#include "common.h"
#include "controller.h"
#include "log.h"
#include "player.h"
#include "game.h"

INLINE ParseAction ParseCmd( const char *cmd )
{
	switch( (int)*cmd ) {
		case CMD_LOGIN:
			return &GameLogin;
			break;
		
		case CMD_CREATE_GAME:
			return &GameCreateNew;	
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
	Player_t *player;		

	player = GetPlayer( sd );

	void (*action)( void*, const int*, pthread_mutex_t*, int*, Player_t* ) = NULL;

	if( (action = ParseCmd( &d->command )) == NULL ) {
		LogMessage( LOG_WARNING, "unknown command" );
		return;
	}

	(*action)( &d->data, sd, mutex, threadFlag, player );
}
