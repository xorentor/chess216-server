#include "common.h"
#include "controller.h"
#include "game.h"
#include "log.h"


void CmdLogin( void *data, const int *sd, pthread_mutex_t *mutex, int *threadFlag )
{
	LoginData_t *ld;
	ld = (LoginData_t *)data;	
	
	// temp
	const char *user, *pass;
	user = "john";
	pass = "doe";
	
	if( strlen( ld->username ) < 2 || strlen( ld->password ) < 2 ) {

#ifdef _DEBUG
		LogMessage( LOG_WARNING, "login data incomplete" );
#endif
		return;
	}

	if( strcmp( ld->username, user ) != 0 || strcmp( ld->password, pass ) != 0 ) {

#ifdef _DEBUG
		LogMessage( LOG_NOTICE, "incorrect login details" );
#endif
		return;
	}
		
}

INLINE ParseAction ParseCmd( const char *cmd )
{
	switch( (int)*cmd ) {
		case CMD_LOGIN:
			return &CmdLogin;
			break;
		default:
			return NULL;
			break;
	}
}

void Controller( void *data, const int *sd, pthread_mutex_t *mutex, int *threadFlag )
{
	PacketData_t *d;
	d = (PacketData_t *)data;
	void (*action)( void*, const int*, pthread_mutex_t*, int* ) = NULL;

	if( (action = ParseCmd( &d->command )) == NULL ) {
		LogMessage( LOG_WARNING, "unknown command" );
		return;
	}

	(*action)( &d->data, sd, mutex, threadFlag );

}
