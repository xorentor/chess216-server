#include "common.h"
#include "main.h"
#include "log.h"
#include "controller.h"
#include "chess.h"
#include "game.h"
#include "player.h"

static pthread_mutex_t mutex_sd = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_threads = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_games = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_players = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_pieces = PTHREAD_MUTEX_INITIALIZER;
int descriptors[ MAX_CLIENTS ];

INLINE void DeletePthread( Thread_t *threads, const pthread_t *pt )
{
	int i;
	pthread_mutex_lock( &mutex_threads );
	for( i = 0; i < MAX_THREADS; i++ ) {
		if( threads[ i ].pthread == *pt ) {
			memset( &(threads[ i ]), 0, sizeof( Thread_t ) );	
		}
	}
	pthread_mutex_unlock( &mutex_threads );
}

int SockOptErr( const int *sd )
{
	int error, r;
	socklen_t len; 

	assert( sd );
	error = 0;
	len = sizeof( error );
	r = getsockopt( *sd, SOL_SOCKET, SO_ERROR, &error, &len );
	return error;
}

INLINE void *ClientInit( void *params )
{
	ClientLocalData_t data; 

    	char readBuffer[ BUFFER_LEN ];
	int buffLen = 0, quitFlag = 0;

	memset( &data, 0, sizeof( data ) );
	data.socketDesc = *( (ThreadParam_t *)params )->socketId;	// copy this!
	data.quitFlag = &quitFlag;
	data.cst = ( (ThreadParam_t *)params )->cst;

#ifdef _DEBUG	
	char buf[ 0x40 ];
	sprintf( buf, "socketID: %d", data.socketDesc );
	LogMessage( LOG_NOTICE, buf );
	sprintf( buf, "PthreadID: %u", (unsigned int)pthread_self() );
	LogMessage( LOG_NOTICE, buf );
#endif
	
	while( quitFlag != F_QUIT ) {
		memset( readBuffer, 0, sizeof( readBuffer ) );
		
		buffLen = read( data.socketDesc, readBuffer, BUFFER_LEN );
		if( buffLen <= 0 ) {
#ifdef _DEBUG	
			char buf[ 0x40 ];
			sprintf( buf, "thread: %u quitting...", (unsigned int)pthread_self() );
			LogMessage( LOG_NOTICE, buf );
#endif
			quitFlag |= F_QUIT;
			continue;
		}
#ifdef _DEBUG	
		char buf[ 0x40 ];
		sprintf( buf, "socketID: %d pthreadID: %u buffLen: %d", data.socketDesc, (unsigned int)pthread_self(), buffLen );
		LogMessage( LOG_NOTICE, buf );
#endif
		// this could be removed if we point this structure to readBuffer
		PacketData_t pd;
		memcpy( &pd.command, &readBuffer, sizeof( pd.command ) );
		pd.data = readBuffer + sizeof( pd.command ) + sizeof( pd.length );

		data.pd = &pd;
		Controller( &data );
		
		usleep( 100000 );
	}
	
	RemovePlayer( &data );
	DeletePthread( ( (ThreadParam_t *)params )->threads, (pthread_t *)pthread_self() );		
	DeleteSD( &data.socketDesc );

	close( data.socketDesc );
	pthread_exit( NULL );
	return NULL;
}

INLINE void DeleteSD( const int *sd )
{
	// TODO: this might cause latency issues as number of clients increases
	int i;
	pthread_mutex_lock( &mutex_sd );
	for( i = 0; i < MAX_CLIENTS; i++ ) {
		if( descriptors[ i ] == *sd ) {
#ifdef _DEBUG
			char buf[ 0x40 ];
			sprintf( buf, "socket descriptor deallocated index: %d value: %d", i, *sd );
			LogMessage( LOG_NOTICE, buf );
#endif
			memset( &(descriptors[ i ]), 0, sizeof( descriptors[ i ] ) );
			break;
		}
	}
	pthread_mutex_unlock( &mutex_sd );
}

INLINE const int AddSD( const int *sd )
{
	// TODO: this might cause latency issues as number of clients increases
	int i, r = -1;
	pthread_mutex_lock( &mutex_sd );
	for( i = 0; i < MAX_CLIENTS; i++ ) {
		if( descriptors[ i ] < 1 ) {
#ifdef _DEBUG
			char buf[ 0x40 ];
			sprintf( buf, "socket descriptor allocated index: %d value: %d", i, *sd );
			LogMessage( LOG_NOTICE, buf );
#endif
			descriptors[ i ] = *sd;
			r = 0;
			break; 
		}
	}

	pthread_mutex_unlock( &mutex_sd );
	return r;
}

INLINE pthread_t *GetPthread( Thread_t *threads )
{
	int i;
	pthread_t *r = NULL;
	pthread_mutex_lock( &mutex_threads );
	for( i = 0; i < MAX_THREADS; i++ ) {
		if( threads[ i ].inuse != 1 ) {
#ifdef	_DEBUG
			char buf[ 0x20 ];
			sprintf( buf, "got new thread location: %d", i );
			LogMessage( LOG_NOTICE, buf );
#endif
			//threads[ i ].pthread = 0;
			threads[ i ].inuse = 1;
			threads[ i ].spawned = time( NULL );
			r = &( threads[ i ].pthread );
			break;
		}
	}
	pthread_mutex_unlock( &mutex_threads );
	return r;
}


void *ServerThread( void *params )
{

	int flag = 0, i;
	PacketData_t pd;
 	GameTimerSrv_t b;
	CrossThread_t *cst;
	Game_t *g = NULL;

	memset( &b, 0, sizeof( b ) );
	pd.command = CMD_GAME_TIMER;
	pd.data = &b;
	
	cst = ( ( ClientThread_t *)params )->cst;
	while( flag != F_QUIT )
	{
		//players;
		//games;
		for( i = 0; i < MAX_GAMES; i++ ) {
			if( cst->games[ i ].gameId < 1 )
				continue;

			g = &cst->games[ i ];
			if( cst->games[ i ].state & GAME_PLAYING ) {
			
				if( cst->games[ i ].nextMove == cst->games[ i ].player1 ) {
					if( cst->games[ i ].p1_sec == 0 ) {
						cst->games[ i ].p1_sec = 59;
						cst->games[ i ].p1_min--;
					} else {
						cst->games[ i ].p1_sec--;
					}
				}

				if( cst->games[ i ].nextMove == cst->games[ i ].player2 ) {
					if( cst->games[ i ].p2_sec == 0 ) {
						cst->games[ i ].p2_sec = 59;
						cst->games[ i ].p2_min--;
					} else {
						cst->games[ i ].p2_sec--;
					}
				}
				b.p1_sec = cst->games[ i ].p1_sec;
				b.p1_min = cst->games[ i ].p1_min;
				b.p2_sec = cst->games[ i ].p2_sec;
				b.p2_min = cst->games[ i ].p2_min;
				
				BroadcastToGame( &cst->games[ i ], &pd );

				if( g->p1_sec == 0 && g->p1_min == 0 ) {
					EndGame( cst->players, g, g->player2 );
					continue;
				}
				if( g->p2_sec == 0 && g->p2_min == 0 ) {
					EndGame( cst->players, g, g->player1 );
					continue;
				}
			}
		}		
		
		sleep( 1 );
	}

	pthread_exit( NULL );
	return NULL;
}

void *ClientsThread( void *params )
{
	ThreadParam_t threadParam;
	ClientThread_t *ct = params;
	pthread_t *tid = NULL;
	int newsockfd, flag = 0;

	while( flag != F_QUIT ) {
		
     		newsockfd = accept( *(ct->sockfd), (struct sockaddr *) ct->cli_addr, ct->clilen );
     		if( newsockfd < 0 )
			LogMessage( LOG_ERROR, "accept" );
		else {
			
			if( AddSD( &newsockfd ) != 0 ) {
				// TODO: send response to client
				LogMessage( LOG_WARNING, "max clients reached" );
				continue;
			}

			if( (tid = GetPthread( ct->threads )) == NULL ) {
				// TODO: send resopnse to client
				LogMessage( LOG_WARNING, "no thread available" );
				continue;
			}

			threadParam.threads = ct->threads;
			threadParam.socketId = &newsockfd;
			threadParam.cst = ct->cst;
			if( pthread_create( tid, NULL, &ClientInit, (void *)&threadParam ) )
				LogMessage( LOG_ERROR, "thread creation failed" );
/*
#ifdef	_DEBUG
			char buf[ 0x20 ];
			sprintf( buf, "thread ID: %u", (unsigned int)(*tid) );
			LogMessage( LOG_NOTICE, buf );
#endif
*/	
		}
	}

	pthread_exit( NULL );
	return NULL;
}

int main( int argc, char **argv )
{
     	int sockfd, portno; 
	pthread_t tid[ 2 ];
     	socklen_t clilen;
     	struct sockaddr_in serv_addr, cli_addr;
	Thread_t threads[ MAX_THREADS ];
	ClientThread_t ct;
	CrossThread_t cst;

	if( argv[ 1 ] == NULL ) {
		LogMessage( LOG_ERROR, "port unspecified" );
		exit(0);
	}

     	sockfd = socket( AF_INET, SOCK_STREAM, 0 );
  	if( sockfd < 0 ) {
		LogMessage( LOG_ERROR, "opening socket" );
	}

	memset( descriptors, 0, sizeof( descriptors ) );
	memset( (char *)&serv_addr, 0, sizeof( serv_addr ) );
	memset( &cst, 0, sizeof( cst ) );
	
	cst.info.playersCount = 0;
	cst.info.gamesCount = 0;

	ct.threads = threads;
	ct.cli_addr = &cli_addr;
	ct.clilen = &clilen;
	ct.sockfd = &sockfd;
	ct.cst = &cst;

     	portno = atoi( argv[1] ); 
     	serv_addr.sin_family = AF_INET;
     	serv_addr.sin_addr.s_addr = INADDR_ANY;
     	serv_addr.sin_port = htons( portno );

     	if( bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0 ) 
		LogMessage( LOG_ERROR, "socket binding" );

	listen( sockfd, 5 );
     	clilen = sizeof( cli_addr );

        pthread_create( &(tid[ 0 ]), NULL, &ClientsThread, (void *)&ct  );
        pthread_create( &(tid[ 1 ]), NULL, &ServerThread, (void *)&ct );

        pthread_join( tid[ 0 ], NULL );
        pthread_join( tid[ 1 ], NULL );

    	close( sockfd );

	return 0;
}
