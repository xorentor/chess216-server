#ifndef __CORE_H_
#define __CORE_H_

#define		PORTNO			5770
#define		MAX_THREADS		1024
#define		MAX_CLIENTS		1024

void Core( int argc, char **argv );
pthread_t *GetPthread( Thread_t *threads );
const int AddSD( const int *sd );
void DeleteSD( const int *sd );
void *ThreadInit( void *params );
void DeletePthread( Thread_t *threads, const pthread_t *pt );
void Error( const char *msg );
static void *ProxyCall( void *params );
void InitDescriptors();

#endif
