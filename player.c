#include "common.h"
#include "log.h"
#include "player.h"

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
		// send client MAX_CLIENTS reached
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
/*
			p = malloc( sizeof(Player_t) );
			if( p == NULL ) {
#ifdef _DEBUG
				LogMessage( LOG_ERROR, "malloc failed (Player_t)" );
#endif
				return NULL;
			}
#ifdef _DEBUG
				LogMessage( LOG_NOTICE, "Player_t stored" );
#endif
*/
			p->socketDesc = cld->socketDesc;		// copy this!
			p->state = 0;

//			cld->cst->players[ i ] = p;
			( cld->cst->info.playersCount )++;
//			Broadcast( cld, ++( );
#ifdef _DEBUG
			LogMessage( LOG_NOTICE, "Player_t stored" );
#endif
			return p;	
		}
	}

	return NULL;
}

void RemoveGame( ClientLocalData_t *cld, Player_t *p )
{
	Game_t *g;
	int players = 0;
	g = NULL;

	for( int i = 0; i < MAX_GAMES; i++ ) {
		if( cld->cst->games[ i ].player1 == p ) {
			g = &cld->cst->games[ i ];
			cld->cst->games[ i ].player1 = NULL;
			break;
		}

		if( cld->cst->games[ i ].player2 == p  ) {
			g = &cld->cst->games[ i ];
			cld->cst->games[ i ].player2 = NULL;
			break;
		}

		for( int j = 0; j < MAX_SPECTATORS; j++ ) {
			if( cld->cst->games[ i ].spectators[ j ] == p ) {
				g = &cld->cst->games[ i ];
				cld->cst->games[ i ].spectators[ j ] = NULL;
				break;
			}
		}
	}

	if( g != NULL ) {
		if( g->player1 != NULL )
			players++;

		if( g->player2 != NULL )
			players++;

		for( int j = 0; j < MAX_SPECTATORS; j++ ) 
			if( g->spectators[ j ] != NULL )
				players++;
	}

#ifdef _DEBUG
	char buf[ 0x40 ];
	sprintf( buf, "RemoveGame: players count: %d", players );
	LogMessage( LOG_NOTICE, buf );
#endif

	// no player left?
	if( players == 0 )
		memset( g, 0, sizeof( Game_t ) );
}

void RemovePlayer( ClientLocalData_t *cld )
{
	for( int i = 0; i < MAX_CLIENTS; i++ ) {
		if( cld->cst->players[ i ].socketDesc == cld->socketDesc ) {
			// delete game if last player left
			RemoveGame( cld, &( cld->cst->players[ i ] ) );

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
//	if( p != NULL)
//		free( p );
	memset( p, 0, sizeof( Player_t ) );
}
