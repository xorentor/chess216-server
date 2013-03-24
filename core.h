#ifndef __CORE_H_
#define __CORE_H_

const int AddSD( const int *sd );
void DeleteSD( const int *sd );
void DeletePthread( Thread_t *threads, const pthread_t *pt );
void *ClientInit( void *params );
void InitDescriptors();
pthread_t *GetPthread( Thread_t *threads );
void *ClientThread( void *params );
void *ServerThread( void *params );
void Core( int argc, char **argv );

#endif
