#include "common.h"
#include "game.h"
#include "player.h"
#include "log.h"

void GameLogin( void *data, const int *sd, pthread_mutex_t *mutex, int *threadFlag, Player_t *player )
{
	LoginData_t *ld;
	ld = (LoginData_t *)data;	
	
	// temp
	const char *user, *pass;
	user = "john";
	pass = "doe";
	
	if( strlen( ld->username ) < 2 || strlen( ld->password ) < 2 ) {

#ifdef _DEBUG
		// send response to client as well
		LogMessage( LOG_WARNING, "login data incomplete" );
#endif
		return;
	}

	if( strcmp( ld->username, user ) != 0 || strcmp( ld->password, pass ) != 0 ) {

#ifdef _DEBUG
		// send response to client as well
		LogMessage( LOG_NOTICE, "incorrect login details" );
#endif
		return;
	}

	player->state |= PLAYER_LOGGED;
}

void GameCreateNew( void *data, const int *sd, pthread_mutex_t *mutex, int *threadFlag, Player_t *player )
{
	if( player->state & PLAYER_LOGGED != PLAYER_LOGGED )
		return;

	if( player->state & PLAYER_INGAME )
		return;
	
	if( player->state & PLAYER_PLAYING )
		return;
	
	pthread_mutex_lock( mutex );
		if( GameStore( player ) != 0 ) {
			// send response to client
#ifdef _DEBUG
			LogMessage( LOG_WARNING, "GameStore failed, MAX_GAMES or malloc" );
#endif
		}
	pthread_mutex_unlock( mutex );
}

int GameStore( Player_t *p )
{
	Game_t *g;
	for( int i = 0; i < MAX_GAMES; i++ ) {
		if( games[ i ] == NULL ) {
			g = malloc( sizeof( Game_t ) );	
			if( g == NULL ) {
#ifdef _DEBUG
				LogMessage( LOG_ERROR, "malloc failed (Game_t)" );
#endif
				return -1;
			}
			
			// default owner state
			p->state |= PLAYER_INGAME;

			// default game state
			g->owner = p;
			g->player1RemTime = 10.0f;
			g->player2RemTime = 10.0f;
			g->state |= GAME_OPENED;
			games[ i ] = g;
			return 0;
		} 
	}	
	return -1;
}
