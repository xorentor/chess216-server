#ifndef __PARSE_H_
#define __PARSE_H_

#define		CMD_LOGIN		0x30

typedef struct LoginData_s
{
	char username[ 64 ];
	char password[ 64 ];
} LoginData_t;

typedef void (*ParseAction)( void*, const int*, pthread_mutex_t*, int* );

ParseAction ParseCmd( const char *cmd );
void Controller( void *data, const int *sd, pthread_mutex_t *mutex, int *threadFlag );

#endif
