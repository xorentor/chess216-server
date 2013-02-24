#ifndef __CORE_H_
#define __CORE_H_

void Core( int argc, char **argv );
pthread_t *GetPthread( Thread_t *threads );
const int AddSD( const int *sd );
void DeleteSD( const int *sd );
void *ThreadInit( void *params );
void DeletePthread( Thread_t *threads, const pthread_t *pt );
void Error( const char *msg );
static void *ProxyCall( void *params );
void InitDescriptors();

void *ClientThread( void *params );
void *BroadcastThread( void *params );

#endif
