#ifndef __PARSE_H_
#define __PARSE_H_

typedef void (*ParseAction)( void*, const int*, pthread_mutex_t*, int*, Player_t* );

ParseAction ParseCmd( const char *cmd );
void Controller( void *data, const int *sd, pthread_mutex_t *mutex, int *threadFlag );

#endif
