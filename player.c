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

void RemovePlayer( ClientLocalData_t *cld )
{
	for( int i = 0; i < MAX_CLIENTS; i++ ) {
		if( cld->cst->players[ i ].socketDesc == cld->socketDesc ) {
			// memset( p, 0, sizeof( Player_t ) );
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
