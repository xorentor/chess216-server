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

void Controller( ClientLocalData_t *cld )
{
	Player_t *player;
	void (*action)( ClientLocalData_t*, Player_t* ) = NULL;

	if( ( player = GetPlayer( cld ) ) == NULL ) {
		// max players reached or malloc failed
	}

	if( ( action = ParseCmd( &cld->pd->command ) ) == NULL ) {
		LogMessage( LOG_WARNING, "unknown command" );
		return;
	}

	(*action)( cld, player );
}
