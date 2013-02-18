#include "common.h"
#include "log.h"
#include "core.h"

pthread_mutex_t mutex;
int descriptors[ MAX_CLIENTS ];

INLINE void RemovePthread( Thread_t *threads, const pthread_t *pt )
{
	for( int i = 0; i < MAX_THREADS; i++ ) {
		if( threads[ i ].pthread == *pt ) {
			memset( &(threads[ i ]), 0, sizeof( Thread_t ) );	
#ifdef _DEBUG
			char buf[ 0x40 ];
			sprintf( buf, "removed thread array i: %d", i );
			LogMessage( LOG_NOTICE, buf );
#endif
		}
	}
}

void *ThreadInit( void *params )
{
    	char readBuffer[ 0x100 ];
	ThreadParam_t *threadParam;
	int socketId, buffLen;
	bool threadquit = false;
	time_t tm;
	Thread_t *threads;		
	
	threadParam = (ThreadParam_t *)params;
	threads = (Thread_t *)threadParam->threads;
	socketId = (int)(*threadParam->socketId);
	tm = time( NULL );
//	bzero( readBuffer, 0x100 );
	memset( readBuffer, 0, sizeof( readBuffer ) );
#ifdef _DEBUG	
	char buf[ 0x40 ];
	sprintf( buf, "socketID: %d", socketId );
	LogMessage( LOG_NOTICE, buf );
	sprintf( buf, "threadID: %u", (unsigned int)pthread_self() );
	LogMessage( LOG_NOTICE, buf );
#endif

	while( !threadquit )
	{
		buffLen = read( socketId, readBuffer, 255 );
		if( buffLen <= 0 )
			threadquit = true;
#ifdef _DEBUG	
		char buf[ 0x40 ];
		sprintf( buf, "thread: %d buffLen: %d", socketId, buffLen );
		LogMessage( LOG_NOTICE, buf );
#endif
	}
	
	close( socketId );
	pthread_mutex_lock( &mutex );
		RemovePthread( threads, (pthread_t *)pthread_self() );		
		PopSD( &socketId );
	pthread_mutex_unlock( &mutex );
	close( socketId );
	pthread_exit( NULL );
}

INLINE void PopSD( const int *sd )
{
	// TODO: this might cause latency issues
	for( int i = 0; i < MAX_CLIENTS; i++ ) {
		if( descriptors[ i ] == *sd ) {
#ifdef _DEBUG
			char buf[ 0x40 ];
			sprintf( buf, "socket descriptor deallocated index: %d value: %d", i, *sd );
			LogMessage( LOG_NOTICE, buf );
#endif
			memset( &(descriptors[ i ]), 0, sizeof( int ) );
			break;
		}
	}
}

INLINE const bool PushSD( const int *sd )
{
	// TODO: this might cause latency issues
	for( int i = 0; i < MAX_CLIENTS; i++ ) {
		if( descriptors[ i ] < 1 ) {
#ifdef _DEBUG
			char buf[ 0x40 ];
			sprintf( buf, "socket descriptor allocated index: %d value: %d", i, *sd );
			LogMessage( LOG_NOTICE, buf );
#endif
			descriptors[ i ] = *sd;
			return true; 
		}
	}
	return false;
}

INLINE void InitClients()
{
	memset( descriptors, 0, MAX_CLIENTS * sizeof( int ) );
}

INLINE pthread_t *GetThreadID( Thread_t *threads, const long *tid )
{
	for( int i = 0; i < MAX_THREADS; i++ ) {
		if( !( threads[ i ].inuse ) ) {
#ifdef	_DEBUG
			char buf[ 0x20 ];
			sprintf( buf, "got new thread location: %d", i );
			LogMessage( LOG_NOTICE, buf );
#endif
			//threads[ i ].pthread = 0;
			threads[ i ].inuse = true;
			threads[ i ].spawned = time( NULL );
			return &( threads[ i ].pthread );
		}
	}
	return NULL;
}

void Init( int argc, char **argv )
{
     	int sockfd, newsockfd, portno;
     	socklen_t clilen;
     	struct sockaddr_in serv_addr, cli_addr;
	Thread_t threads[ MAX_THREADS ];
	pthread_t *tid = NULL;
	long threadPool = 0;	
	ThreadParam_t threadParam;

     	if( argc < 2 ) {
		LogMessage( LOG_ERROR, "no port provided" );
        	exit(1);
     	}

     	sockfd = socket( AF_INET, SOCK_STREAM, 0 );
  	if( sockfd < 0 ) {
		LogMessage( LOG_ERROR, "opening socket" );
	}

//     	bzero( (char *) &serv_addr, sizeof(serv_addr) );
	memset( (char *)&serv_addr, 0, sizeof( serv_addr ) );
     	portno = atoi( argv[1] );
     	serv_addr.sin_family = AF_INET;
     	serv_addr.sin_addr.s_addr = INADDR_ANY;
     	serv_addr.sin_port = htons( portno );

     	if( bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0 ) 
		LogMessage( LOG_ERROR, "socket binding" );

	listen( sockfd, 5 );
     	clilen = sizeof( cli_addr );

	InitClients();

	while( true ) {
     		newsockfd = accept( sockfd, (struct sockaddr *) &cli_addr, &clilen );
     		if( newsockfd < 0 )
			LogMessage( LOG_ERROR, "accept" );
		else {
			if( !PushSD( &newsockfd ) ) {
				LogMessage( LOG_WARNING, "max clients reached" );
				continue;
			}

			if( (tid = GetThreadID( threads, &threadPool )) == NULL ) {
				LogMessage( LOG_WARNING, "no thread available" );
				continue;
			}

			threadParam.threads = threads;
			threadParam.socketId = &newsockfd;

			pthread_create( tid, NULL, &ThreadInit, (void *)&threadParam );
#ifdef	_DEBUG
			char buf[ 0x20 ];
			sprintf( buf, "thread ID: %u", (unsigned int)(*tid) );
			LogMessage( LOG_NOTICE, buf );
#endif
			threadPool++;
		}
	}
    	close(sockfd);
}
