#include "common.h"
#include "game.h"
#include "player.h"
#include "log.h"

void GameServerSend( PacketData_t *pd, const int *sd )
{
	char output[ BUFFER_LEN ];
	int n;

	memcpy( output, &pd->command, sizeof( pd->command ) );

	switch( (int) pd->command ) {
		case CMD_LOGIN:
			memcpy( output + sizeof( pd->command ), &( ( (ServerByte_t *)pd->data )->byte ), sizeof( ( (ServerByte_t *)pd->data )->byte ) );
			break;
		
		case CMD_GAME_CREATE:
#ifdef _DEBUG
			LogMessage( LOG_NOTICE, "create new game OK" );
#endif
			memcpy( output + sizeof( pd->command ), &( ( (ServerTwoBytes_t *)pd->data )->byte0 ), sizeof( ( (ServerTwoBytes_t *)pd->data )->byte0 ) );
			memcpy( output + sizeof( pd->command ) + sizeof( ( (ServerTwoBytes_t *)pd->data )->byte0 ), &( ( (ServerTwoBytes_t *)pd->data )->byte1 ), sizeof( ( (ServerTwoBytes_t *)pd->data )->byte1 ) );
//			memcpy( output + 2, "0", 1 );
			break;
		default:
			return;
			break;
	}

	n = write( *sd, output, sizeof( output ) );
}

void GameGetInitialData( const int *sd, pthread_mutex_t *mutex )
{
	//short playersCount;
	// 1. online users
	//players = GameGetOnlinePlayers( pthread_mutex_t *mutex );
	
	// 2. list of available games

	// 3. 
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
			
		GameServerSend( &pd, &cld->socketDesc );
		
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
		
	GameServerSend( &pd, &cld->socketDesc );

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

	if( player->state & PLAYER_INGAME ) {
#ifdef _DEBUG
		LogMessage( LOG_NOTICE, "GameCreateNew player in game already" );
#endif
		return;
	}
	
	if( player->state & PLAYER_PLAYING ) {
#ifdef _DEBUG
		LogMessage( LOG_NOTICE, "GameCreateNew player playing" );
#endif
		return;
	}

	if( ( g = GameStore( cld, player ) ) == NULL ) {
		// send response to client
		/*
		pd.command = (char )CMD_GAME_CREATE;
		pd.data = &b;
		b.byte = (char )CMD_GAME_CREATE_PARAM_NOK;
		
		GameServerSend( &pd, sd );
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
		
	GameServerSend( &pd, &cld->socketDesc );
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
			p->state |= PLAYER_INGAME;

			// default game state
			g->gameId = i;
			g->owner = p;
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
