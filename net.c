#include "common.h"
#include "log.h"
#include "net.h"

void PacketSend( PacketData_t *pd, const int *sd )
{
	char output[ BUFFER_LEN ];
	int n;

	memset( output, 0, sizeof( output ) );
	memcpy( output, &pd->command, sizeof( pd->command ) );

	printf( "server command: %d\n", (int )pd->command );

	// TODO: Add length !!
	switch( (int) pd->command ) {
		case CMD_LOGIN:
#ifdef _DEBUG
			LogMessage( LOG_NOTICE, "server reply: game login" );
#endif
			//memcpy( output + sizeof( pd->command ), &( ( (ServerByte_t *)pd->data )->byte0 ), sizeof( ( (ServerByte_t *)pd->data )->byte0 ) );
			memcpy( output + sizeof( pd->command ), (char *)pd->data, sizeof( GameLoginSrv_t ) );
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

		case CMD_GAME_MOVEPIECE:
#ifdef _DEBUG
			LogMessage( LOG_NOTICE, "server reply: game move piece" );
#endif
			memcpy( output + sizeof( pd->command ), &( ( (GamePieceMoveSrv_t *)pd->data )->pieceId ), sizeof( (GamePieceMoveSrv_t *)pd->data )->pieceId );
			memcpy( output + sizeof( pd->command ) + sizeof( ( (GamePieceMoveSrv_t *)pd->data )->pieceId ), &( ( (GamePieceMoveSrv_t *)pd->data )->xdest ), sizeof( ( (GamePieceMoveSrv_t *)pd->data )->xdest ) );
			memcpy( output + sizeof( pd->command ) + sizeof( ( (GamePieceMoveSrv_t *)pd->data )->pieceId ) + sizeof( ( (GamePieceMoveSrv_t *)pd->data )->xdest ), &( ( (GamePieceMoveSrv_t *)pd->data )->ydest ), sizeof( ( (GamePieceMoveSrv_t *)pd->data )->ydest ) );
			memcpy( output + sizeof( pd->command ) + sizeof( ( (GamePieceMoveSrv_t *)pd->data )->pieceId ) + sizeof( ( (GamePieceMoveSrv_t *)pd->data )->xdest ) + sizeof( ( (GamePieceMoveSrv_t *)pd->data )->ydest ) , &( ( (GamePieceMoveSrv_t *)pd->data )->checkMate ), sizeof( ( (GamePieceMoveSrv_t *)pd->data )->checkMate ) );
			memcpy( output + sizeof( pd->command ) + sizeof( ( (GamePieceMoveSrv_t *)pd->data )->pieceId ) + sizeof( ( (GamePieceMoveSrv_t *)pd->data )->xdest ) + sizeof( ( (GamePieceMoveSrv_t *)pd->data )->ydest ) + sizeof( ( (GamePieceMoveSrv_t *)pd->data )->checkMate ) , &( ( (GamePieceMoveSrv_t *)pd->data )->next ), sizeof( ( (GamePieceMoveSrv_t *)pd->data )->next ) );

			break;
		case CMD_GAME_INITIAL_PIECES:
			memcpy( output + sizeof( pd->command ), (char *)pd->data, 225 );

			break;
		case CMD_GAME_STAND:
#ifdef _DEBUG
			LogMessage( LOG_NOTICE, "server reply: game stand" );
#endif
			memcpy( output + sizeof( pd->command ), (char *)pd->data, sizeof( GameSitServerData_t ) );
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
#ifdef _DEBUG
		LogMessage( LOG_NOTICE, "boardcastGames: game null" );
#endif
		return;
	}
#ifdef _DEBUG
	LogMessage( LOG_NOTICE, "boardcastGames: send to games" );
#endif

	if( game->player1 != NULL ) {
		PacketSend( pd, &game->player1->socketDesc );
#ifdef _DEBUG
		LogMessage( LOG_NOTICE, "boardcastGames: player1 exists" );
#endif
	}

	if( game->player2 != NULL ) {
		PacketSend( pd, &game->player2->socketDesc );
#ifdef _DEBUG
		LogMessage( LOG_NOTICE, "boardcastGames: player2 exists" );
#endif
	}
	
	int i;
	for( i = 0; i < MAX_SPECTATORS; i++ ) {
		if( game->spectators[ i ] != NULL ) {
			PacketSend( pd, &game->spectators[ i ]->socketDesc );
#ifdef _DEBUG
		char buf[ 0x40 ];
		sprintf( buf, "broadcastGames: spectator exists: %d\n", i );
		LogMessage( LOG_NOTICE, buf );
#endif
		}
	}

	// already locked when called
	// pthread_mutex_unlock( cld->mutex );
}

void BroadcastToPlayers( ClientLocalData_t *cld, PacketData_t *pd )
{
	//pthread_mutex_lock( cld->mutex );
	int i;
	for( i = 0; i < MAX_CLIENTS; i++ ) {
		if( cld->cst->players[ i ].socketDesc != 0 ) {
#ifdef _DEBUG
			char buf[ 0x40 ];
			sprintf( buf, "broadcasttoPlayers: %s\n", cld->cst->players[ i ].username );
			LogMessage( LOG_NOTICE, buf );
#endif

			PacketSend( pd, &cld->cst->players[ i ].socketDesc );
		}
	}

	//pthread_mutex_unlock( cld->mutex );	
}

void ReplyToPlayer( PacketData_t *pd, const int *sd )
{
	PacketSend( pd, sd );
}
