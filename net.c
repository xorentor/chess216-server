#include "memory.h"

#include "common.h"
#include "log.h"
#include "net.h"

void PacketSend( PacketData_t *pd, const int *sd )
{
	char output[ BUFFER_LEN ];
	int n;

	memcpy( output, &pd->command, sizeof( pd->command ) );

	// TODO: Add length !!
	switch( (int) pd->command ) {
		case CMD_LOGIN:
#ifdef _DEBUG
			LogMessage( LOG_NOTICE, "server reply: game login" );
#endif
			memcpy( output + sizeof( pd->command ), &( ( (ServerByte_t *)pd->data )->byte ), sizeof( ( (ServerByte_t *)pd->data )->byte ) );
			break;
		
		case CMD_GAME_CREATE:
#ifdef _DEBUG
			LogMessage( LOG_NOTICE, "server reply: game create" );
#endif
			memcpy( output + sizeof( pd->command ), &( ( (ServerTwoBytes_t *)pd->data )->byte0 ), sizeof( ( (ServerTwoBytes_t *)pd->data )->byte0 ) );
			memcpy( output + sizeof( pd->command ) + sizeof( ( (ServerTwoBytes_t *)pd->data )->byte0 ), &( ( (ServerTwoBytes_t *)pd->data )->byte1 ), sizeof( ( (ServerTwoBytes_t *)pd->data )->byte1 ) );
//			memcpy( output + 2, "0", 1 );
			break;
		case CMD_GAME_JOIN:
#ifdef _DEBUG
			LogMessage( LOG_NOTICE, "server reply: game join" );
#endif
			memcpy( output + sizeof( pd->command ), &( ( (ServerTwoBytes_t *)pd->data )->byte0 ), sizeof( ( (ServerTwoBytes_t *)pd->data )->byte0 ) );
			memcpy( output + sizeof( pd->command ) + sizeof( ( (ServerTwoBytes_t *)pd->data )->byte0 ), &( ( (ServerTwoBytes_t *)pd->data )->byte1 ), sizeof( ( (ServerTwoBytes_t *)pd->data )->byte1 ) );
			break;

		case CMD_GAME_SIT:
#ifdef _DEBUG
			LogMessage( LOG_NOTICE, "server reply: game sit" );
#endif
			memcpy( output + sizeof( pd->command ), &( ( (GameSitServerData_t *)pd->data )->slot ), sizeof( (GameSitServerData_t *)pd->data )->slot );
			memcpy( output + sizeof( pd->command ) + sizeof( ( (GameSitServerData_t *)pd->data )->slot ), &( ( (GameSitServerData_t *)pd->data )->username ), sizeof( ( (GameSitServerData_t *)pd->data )->username ) );
			memcpy( output + sizeof( pd->command ) + sizeof( ( (GameSitServerData_t *)pd->data )->slot ) + sizeof( ( (GameSitServerData_t *)pd->data )->username ), &( ( (GameSitServerData_t *)pd->data )->gameBegin ), sizeof( ( (GameSitServerData_t *)pd->data )->gameBegin ) );
			break;
		default:
			return;
			break;
	}

	n = write( *sd, output, sizeof( output ) );
}

void BroadcastToGame( Game_t *game, PacketData_t *pd )
{
	// already locked when called
	//pthread_mutex_lock( cld->mutex );

	if( game == NULL ) {
		//pthread_mutex_unlock( cld->mutex );
		return;
	}

	if( game->player1 != NULL )
		PacketSend( pd, &game->player1->socketDesc );

	if( game->player2 != NULL )
		PacketSend( pd, &game->player2->socketDesc );
	
	for( int i = 0; i < MAX_SPECTATORS; i++ ) {
		if( game->spectators[ i ] != NULL )
			PacketSend( pd, &game->spectators[ i ]->socketDesc );
	}

	// already locked when called
	// pthread_mutex_unlock( cld->mutex );
}

void BroadcastToPlayers( ClientLocalData_t *cld, PacketData_t *pd )
{
	//pthread_mutex_lock( cld->mutex );

	for( int i = 0; i < MAX_CLIENTS; i++ ) {
		if( cld->cst->players[ i ] != NULL ) {
#ifdef _DEBUG
			char buf[ 0x40 ];
			sprintf( buf, "broadcasttoPlayers: %s\n", cld->cst->players[ i ]->username );
			LogMessage( LOG_NOTICE, buf );
#endif

			PacketSend( pd, &cld->cst->players[ i ]->socketDesc );
		}
	}

	//pthread_mutex_unlock( cld->mutex );	
}

void ReplyToPlayer( PacketData_t *pd, const int *sd )
{
	PacketSend( pd, sd );
}
