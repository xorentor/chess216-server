#include "common.h"
#include "controller.h"
#include "log.h"
#include "player.h"
#include "game.h"

INLINE ParseAction ParseCmd( const char *cmd )
{
	switch( (int )*cmd ) {
		case CMD_LOGIN:
			return &GameLogin;
			break;
		case CMD_GAME_CREATE:
			return &GameCreateNew;	
			break;
		case CMD_GAME_JOIN:
			return &GameJoin;
			break;
		case CMD_GAME_SIT:
			return &GameSit;
			break;
        	case CMD_GAME_MOVEPIECE:
			return &GameMovePiece;
			break;
		case CMD_GAME_STAND:
			return &GameStand;
			break;
		default:
			return NULL;
			break;
	}
}

void Controller( ClientLocalData_t *cld )
{
	Player_t *player;
	player = NULL;

	void (*action)( ClientLocalData_t*, Player_t* ) = NULL;

	if( ( player = GetPlayer( cld ) ) == NULL ) {
		// TODO: tell client max players reached
	}

	if( ( action = ParseCmd( &cld->pd->command ) ) == NULL ) {
		LogMessage( LOG_WARNING, "unknown command" );
		return;
	}

	if( action != NULL ) 
		(*action)( cld, player );
}
