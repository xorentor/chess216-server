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
		
		case CMD_GAME_CREATE:
			return &GameCreateNew;	
			break;

		default:
			return NULL;
			break;
	}
}

void Controller( void *pd, const int *sd, pthread_mutex_t *mutex, int *threadFlag )
{
	Player_t *player;
	void (*action)( void*, const int*, pthread_mutex_t*, int*, Player_t* ) = NULL;

	if( ( player = GetPlayer( sd ) ) == NULL ) {
		// max players reached or malloc failed
	}

	if( ( action = ParseCmd( &( (PacketData_t *)pd )->command ) ) == NULL ) {
		LogMessage( LOG_WARNING, "unknown command" );
		return;
	}

	(*action)( ( (PacketData_t *)pd )->data, sd, mutex, threadFlag, player );
}
