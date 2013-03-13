#include "memory.h"

#include "common.h"
#include "log.h"
#include "net.h"

void PacketSend( PacketData_t *pd, const int *sd )
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

void BroadcastToGame( ClientLocalData_t *cld, Game_t *game, PacketData_t *pd )
{
	pthread_mutex_lock( cld->mutex );

	if( game == NULL ) {
		pthread_mutex_unlock( cld->mutex );
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

	pthread_mutex_unlock( cld->mutex );
}

void BroadcastToPlayers( ClientLocalData_t *cld, PacketData_t *pd )
{
	pthread_mutex_lock( cld->mutex );

	for( int i = 0; i < MAX_CLIENTS; i++ ) {
		if( cld->cst->players[ i ] != NULL )
			PacketSend( pd, &cld->cst->players[ i ]->socketDesc );
	}

	pthread_mutex_unlock( cld->mutex );	
}

void ReplyToPlayer( PacketData_t *pd, const int *sd )
{
	PacketSend( pd, sd );
}
