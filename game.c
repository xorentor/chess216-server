#include "common.h"
#include "game.h"
#include "player.h"
#include "log.h"
#include "net.h"

void GameGetInitialData( const int *sd, pthread_mutex_t *mutex )
{
	//short playersCount;
	// 1. online users
	//players = GameGetOnlinePlayers( pthread_mutex_t *mutex );
	
	// 2. list of available games

	// 3. 
}

void GameJoin( ClientLocalData_t *cld, Player_t *player )
{
	JoinData_t *jd;
	jd = cld->pd->data;

	if( ( player->state & PLAYER_LOGGED ) != PLAYER_LOGGED ) 
		return;

	if( player->state & PLAYER_INGAME )
		return;

#ifdef _DEBUG
	LogMessage( LOG_NOTICE, "game join" );
#endif

	pthread_mutex_lock( cld->mutex );
	for( int i = 0; i < MAX_GAMES; i++ ) {
		if( cld->cst->games[ i ] != NULL ) {
			if( cld->cst->games[ i ]->gameId == (int )jd->gameId && ( cld->cst->games[ i ]->state & GAME_OPENED ) ) {
				for( int k = 0; k < MAX_SPECTATORS; k++ ) {
					if( cld->cst->games[ i ]->spectators[ k ] == NULL ) {
						cld->cst->games[ i ]->spectators[ k ] = player;
						player->state |= PLAYER_INGAME;
						player->state |= PLAYER_SPECTATOR;
						break;
					}
				}
				break;
			}	
		}
	}
	pthread_mutex_unlock( cld->mutex );
}

void GameLogin( ClientLocalData_t *cld, Player_t *player )
{
	LoginData_t *ld;
	ld = (LoginData_t *)(cld->pd->data);	
	PacketData_t pd;
	ServerByte_t b;
	
	// temp
	const char *user, *pass;
	user = "john";
	pass = "doe";

#ifdef _DEBUG
	printf( "user: %s pass: %s \n", ld->username, ld->password );
#endif
	
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
		pd.command = (char )CMD_LOGIN;
		pd.data = &b;
		b.byte = (char )CMD_LOGIN_PARAM_DETAILS_ERR;
			
		ReplyToPlayer( &pd, &cld->socketDesc );
		
		return;
	}

	player->username = ld->username;
	player->state |= PLAYER_LOGGED;

#ifdef _DEBUG
	LogMessage( LOG_NOTICE, "user logged successfully" );
#endif	

	pd.command = (char )CMD_LOGIN;
	pd.data = &b;
	b.byte = (char )CMD_LOGIN_PARAM_DETAILS_OK;
		
	ReplyToPlayer( &pd, &cld->socketDesc );

	// when a login is successful, update client
	GameGetInitialData( &cld->socketDesc, cld->mutex );
}

//void GameCreateNew( void *data, const int *sd, pthread_mutex_t *mutex, int *threadFlag, Player_t *player )
void GameCreateNew( ClientLocalData_t *cld, Player_t *player )
{
	PacketData_t pd;
	ServerTwoBytes_t b;
	Game_t *g;

#ifdef _DEBUG
	char buf[ 0x40 ];
	sprintf( buf, "player->state: %x", player->state );
	LogMessage( LOG_NOTICE, buf );
#endif
	
	if( ( player->state & PLAYER_LOGGED ) != PLAYER_LOGGED ) {
#ifdef _DEBUG
		LogMessage( LOG_NOTICE, "GameCreateNew player not logged" );
#endif

		return;
	}

	if( player->state & PLAYER_CREATED_GAME ) {
#ifdef _DEBUG
		LogMessage( LOG_NOTICE, "Player already created a game" );
#endif
		return;
	}
	
	if( ( g = GameStore( cld, player ) ) == NULL ) {
		// send response to client
		/*
		pd.command = (char )CMD_GAME_CREATE;
		pd.data = &b;
		b.byte = (char )CMD_GAME_CREATE_PARAM_NOK;
		
		ReplyToPlayer( &pd, sd );
		*/
#ifdef _DEBUG
		LogMessage( LOG_WARNING, "GameStore() failed, MAX_GAMES or malloc" );
#endif
		return;
	}

	pd.command = (char )CMD_GAME_CREATE;
	pd.data = &b;
	b.byte0 = (char )CMD_GAME_CREATE_PARAM_OK;
	b.byte1 = (char )g->gameId;
		
//	ReplyToPlayer( &pd, &cld->socketDesc );
	BroadcastToPlayers( cld, &pd );
}

Game_t *GameStore( ClientLocalData_t *cld, Player_t *p )
{
	Game_t *g;
	
	pthread_mutex_lock( cld->mutex );

	for( int i = 2; i < MAX_GAMES; i++ ) {
		if( cld->cst->games[ i ] == NULL ) {
			g = malloc( sizeof( Game_t ) );	
			if( g == NULL ) {
#ifdef _DEBUG
				LogMessage( LOG_ERROR, "malloc failed (Game_t)" );
#endif
				return NULL;
			}
			
			// default player state
			p->state |= PLAYER_CREATED_GAME;

			// default game state
			g->gameId = i;
			g->player1RemTime = 10.0f;
			g->player2RemTime = 10.0f;
			g->state |= GAME_OPENED;
			cld->cst->games[ i ] = g;
	
			pthread_mutex_unlock( cld->mutex );
			return g;
		} 
	}	
	
	pthread_mutex_unlock( cld->mutex );
	return NULL;
}
