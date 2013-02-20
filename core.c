#include "common.h"
#include "log.h"
#include "core.h"
#include "controller.h"

pthread_mutex_t mutex =  PTHREAD_MUTEX_INITIALIZER;
int descriptors[ MAX_CLIENTS ];

INLINE void DeletePthread( Thread_t *threads, const pthread_t *pt )
{
	for( int i = 0; i < MAX_THREADS; i++ ) {
		if( threads[ i ].pthread == *pt ) {
#ifdef _DEBUG
			char buf[ 0x40 ];
			sprintf( buf, "removed thread array index: %d pthreadID: %u", i, (unsigned int)pthread_self() );
			LogMessage( LOG_NOTICE, buf );
#endif
			memset( &(threads[ i ]), 0, sizeof( Thread_t ) );	
		}
	}
}

/*
* variables/functions that require mutex:
* - DeletePthread
* - DeleteSD
*/
void *ThreadInit( void *params )
{
    	char readBuffer[ 0x100 ];
	ThreadParam_t *threadParam;
	int buffLen = 0, flag = 0;
	time_t tm;
	
	tm = time( NULL );
	threadParam = (ThreadParam_t *)params;
	memset( readBuffer, 0, sizeof( readBuffer ) );

#ifdef _DEBUG	
	char buf[ 0x40 ];
	sprintf( buf, "socketID: %d", (int)(*threadParam->socketId) );
	LogMessage( LOG_NOTICE, buf );
	sprintf( buf, "PthreadID: %u", (unsigned int)pthread_self() );
	LogMessage( LOG_NOTICE, buf );
#endif

	while( flag != F_QUIT ) {
		memset( readBuffer, 0, sizeof( readBuffer ) );
		buffLen = read( (int)(*threadParam->socketId), readBuffer, 255 );
		if( buffLen <= 0 ) {
#ifdef _DEBUG	
		char buf[ 0x40 ];
		sprintf( buf, "thread: %u quitting...", (unsigned int)pthread_self() );
		LogMessage( LOG_NOTICE, buf );
#endif
			flag |= F_QUIT;
			continue;
		}
#ifdef _DEBUG	
		char buf[ 0x40 ];
		sprintf( buf, "socketID: %d pthreadID: %u buffLen: %d", (int)(*threadParam->socketId), (unsigned int)pthread_self(), buffLen );
		LogMessage( LOG_NOTICE, buf );
#endif
		Controller( &readBuffer, threadParam->socketId, &mutex, &flag );
	}
	
	pthread_mutex_lock( &mutex );
		DeletePthread( (Thread_t *)threadParam->threads, (pthread_t *)pthread_self() );		
		DeleteSD( threadParam->socketId );
	pthread_mutex_unlock( &mutex );

	close( (int)(*threadParam->socketId) );
	pthread_exit( NULL );
}

INLINE void DeleteSD( const int *sd )
{
	// TODO: this might cause latency issues as number of clients increases
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

INLINE const int AddSD( const int *sd )
{
	// TODO: this might cause latency issues as number of clients increases
	for( int i = 0; i < MAX_CLIENTS; i++ ) {
		if( descriptors[ i ] < 1 ) {
#ifdef _DEBUG
			char buf[ 0x40 ];
			sprintf( buf, "socket descriptor allocated index: %d value: %d", i, *sd );
			LogMessage( LOG_NOTICE, buf );
#endif
			descriptors[ i ] = *sd;
			return 0; 
		}
	}
	return -1;
}

INLINE void InitDescriptors()
{
	memset( descriptors, 0, MAX_CLIENTS * sizeof( int ) );
}

INLINE pthread_t *GetPthread( Thread_t *threads )
{
	for( int i = 0; i < MAX_THREADS; i++ ) {
		if( threads[ i ].inuse != 1 ) {
#ifdef	_DEBUG
			char buf[ 0x20 ];
			sprintf( buf, "got new thread location: %d", i );
			LogMessage( LOG_NOTICE, buf );
#endif
			//threads[ i ].pthread = 0;
			threads[ i ].inuse = 1;
			threads[ i ].spawned = time( NULL );
			return &( threads[ i ].pthread );
		}
	}
	return NULL;
}

void Core( int argc, char **argv )
{
     	int sockfd, newsockfd, portno, rc;
     	socklen_t clilen;
     	struct sockaddr_in serv_addr, cli_addr;
	Thread_t threads[ MAX_THREADS ];
	pthread_t *tid = NULL;
	ThreadParam_t threadParam;
	int flag = 0;

     	if( argc < 2 ) {
		LogMessage( LOG_ERROR, "no port provided" );
        	exit(1);
     	}

     	sockfd = socket( AF_INET, SOCK_STREAM, 0 );
  	if( sockfd < 0 ) {
		LogMessage( LOG_ERROR, "opening socket" );
	}

	InitDescriptors();
	memset( (char *)&serv_addr, 0, sizeof( serv_addr ) );
     	portno = atoi( argv[1] );
     	serv_addr.sin_family = AF_INET;
     	serv_addr.sin_addr.s_addr = INADDR_ANY;
     	serv_addr.sin_port = htons( portno );

     	if( bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0 ) 
		LogMessage( LOG_ERROR, "socket binding" );

	listen( sockfd, 5 );
     	clilen = sizeof( cli_addr );

	while( flag != F_QUIT ) {
     		newsockfd = accept( sockfd, (struct sockaddr *) &cli_addr, &clilen );
     		if( newsockfd < 0 )
			LogMessage( LOG_ERROR, "accept" );
		else {
			if( AddSD( &newsockfd ) != 0 ) {
				LogMessage( LOG_WARNING, "max clients reached" );
				continue;
			}

			if( (tid = GetPthread( threads )) == NULL ) {
				LogMessage( LOG_WARNING, "no thread available" );
				continue;
			}

			threadParam.threads = threads;
			threadParam.socketId = &newsockfd;

			if( pthread_create( tid, NULL, &ThreadInit, (void *)&threadParam ) )
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
    	close( sockfd );
}
