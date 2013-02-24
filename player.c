#include "common.h"
#include "log.h"
#include "player.h"

Player_t *GetPlayer( const int *socketDescriptor )
{	
	if( socketDescriptor == NULL )
		return NULL;
	
	Player_t *p;

	for( int i = 0; i < MAX_CLIENTS; i++ ) {
		if( players[ i ] != NULL ) {
			if( *(players[ i ]->sd) == *socketDescriptor ) {
#ifdef _DEBUG
				LogMessage( LOG_NOTICE, "Player_t retrieved by socketDescriptor" );
#endif
				return players[ i ];
			}
		}
	}
	
	if( ( p = StorePlayer( socketDescriptor ) ) == NULL ) {
		// send client MAX_CLIENTS reached
#ifdef _DEBUG
		LogMessage( LOG_WARNING, "MAX_CLIENTS reached" );
#endif
		return NULL;	
	}

	return p;
}

Player_t *StorePlayer( const int *socketDescriptor )
{
	for( int i = 0; i < MAX_CLIENTS; i++ ) {
		if( players[ i ] == NULL ) {
			Player_t *p;
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

			p->sd = socketDescriptor;
			players[ i ] = p;
			return p;	
		}
	}

	return NULL;
}

void RemovePlayer( Player_t *p )
{
	if( p == NULL )
		return;
	
	for( int i = 0; i < MAX_CLIENTS; i++ ) {
		if( players[ i ] == p ) {
			// memset( p, 0, sizeof( Player_t ) );
			FreePlayer( p );
			players[ i ] = NULL;
			return;
		}
	}

}

void FreePlayer( Player_t *p )
{
	if( p != NULL)
		free( p );
}	
