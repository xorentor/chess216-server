#ifndef __CORE_H_
#define __CORE_H_

#define		PORTNO			5770
#define		MAX_THREADS		1024
#define		MAX_CLIENTS		1024

void Init( int argc, char **argv );
pthread_t *GetThreadID( Thread_t *threads, const long *tid );
const bool PushSD( const int *sd );
void PopSD( const int *sd );
void *ThreadInit( void *params );
void RemovePthread( Thread_t *threads, const pthread_t *pt );
void Error( const char *msg );
static void *ProxyCall( void *params );
void InitClients();

#endif
