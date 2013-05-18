#include "common.h"
#include "log.h"
#include "player.h"
#include "net.h"

Player_t *GetPlayer( ClientLocalData_t *cld )
{	
	if( cld->socketDesc == 0 )
		return NULL;
	
	Player_t *p;

	for( int i = 0; i < MAX_CLIENTS; i++ ) {
		if( cld->cst->players[ i ].socketDesc != 0 ) {
			if( cld->cst->players[ i ].socketDesc == cld->socketDesc ) {
#ifdef _DEBUG
				LogMessage( LOG_NOTICE, "Player_t retrieved by socketDescriptor" );
#endif
				return &( cld->cst->players[ i ] );
			}  
		}
	}
	
	if( ( p = StorePlayer( cld ) ) == NULL ) {
		// TODO: send client MAX_CLIENTS reached
#ifdef _DEBUG
		LogMessage( LOG_WARNING, "MAX_CLIENTS reached" );
#endif
		return NULL;	
	}

	return p;
}

Player_t *StorePlayer( ClientLocalData_t *cld )
{
	for( int i = 0; i < MAX_CLIENTS; i++ ) {
		if( cld->cst->players[ i ].socketDesc == 0 ) {
			Player_t *p;
			p = &( cld->cst->players[ i ] );

			p->socketDesc = cld->socketDesc;		// copy this!
			p->state = 0;

			( cld->cst->info.playersCount )++;
#ifdef _DEBUG
			LogMessage( LOG_NOTICE, "Player_t stored" );
#endif
			return p;	
		}
	}

	return NULL;
}

Game_t *FindGameByPlayer( ClientLocalData_t *cld, Player_t *p )
{
	for( int i = 0; i < MAX_GAMES; i++ ) { 
		if( cld->cst->games[ i ].player1 == p ) {
			return &cld->cst->games[ i ];
		}
		if( cld->cst->games[ i ].player2 == p  ) {
			return &cld->cst->games[ i ];
		}

		for( int j = 0; j < MAX_SPECTATORS; j++ ) {
			if( cld->cst->games[ i ].spectators[ j ] == p ) {
				return &cld->cst->games[ i ];
			}
		}
	}

	return NULL;
}

BYTE GetPlayersCount( Game_t *g ) {
	BYTE players = 0;

	assert( g );
	if( g->player1 != NULL )
		players++;

	if( g->player2 != NULL )
		players++;

	for( int j = 0; j < MAX_SPECTATORS; j++ ) 
		if( g->spectators[ j ] != NULL )
			players++;
	
	return players;
}

void RemovePlayerGame( ClientLocalData_t *cld, Player_t *p )
{
	PacketData_t pd;
	ServerTwoBytes_t b;
	Game_t *g = NULL;
	BYTE players = 0;

	if( ( g = FindGameByPlayer( cld, p ) ) == NULL ) {
		printf( "findgamebyplayer null\n" );
		return;
	}

	printf("found game %d\n", g->gameId );

	players = GetPlayersCount( g );	
	printf( "number of players for this game total, before memset0: %d\n", players );

	// remove a player from game
	{ 
	if( g->player1 == p ) 
		g->player1 = NULL;

	if( g->player2 == p  ) 
		g->player2 = NULL;

	for( int j = 0; j < MAX_SPECTATORS; j++ ) {
		if( g->spectators[ j ] == p ) {
			g->spectators[ j ] = NULL;
			break;
		}
	}
	}

	players = GetPlayersCount( g );	

#ifdef _DEBUG
	char buf[ 0x40 ];
	sprintf( buf, "RemovePlayerGame: players count: %d", players );
	LogMessage( LOG_NOTICE, buf );
#endif

	// if no player left, remove game
	if( players == 0 ) {
		pd.command = (char )CMD_GAME_CREATE;
		pd.data = &b;
		b.byte0 = (char )CMD_GAME_CREATE_PARAM_DELETE;
		b.byte1 = (char )g->gameId;
	
		printf("sent game ID: %d\n", (int )b.byte1 );	
		BroadcastToPlayers( cld, &pd );

		memset( g, 0, sizeof( Game_t ) );
	} else
		printf( "number of players for this game total: %d\n", players );
}

void RemovePlayer( ClientLocalData_t *cld )
{
	for( int i = 0; i < MAX_CLIENTS; i++ ) {
		if( cld->cst->players[ i ].socketDesc == cld->socketDesc ) {
			// delete game if last player left
			RemovePlayerGame( cld, &( cld->cst->players[ i ] ) );

			FreePlayer( &cld->cst->players[ i ] );

			( cld->cst->info.playersCount )--;
#ifdef _DEBUG
			LogMessage( LOG_NOTICE, "Player_t released" );
#endif
			return;
		}
	}
}

void FreePlayer( Player_t *p )
{
	memset( p, 0, sizeof( Player_t ) );
}
