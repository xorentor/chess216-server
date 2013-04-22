#include "common.h"
#include "player.h"
#include "log.h"
#include "net.h"
#include "chess.h"
#include "game.h"

void GameGetInitialData( const int *sd, pthread_mutex_t *mutex )
{
	// TODO:
	//short playersCount;
	// 1. online users
	//players = GameGetOnlinePlayers( pthread_mutex_t *mutex );
	
	// 2. list of available games

	// 3. 
}

void GameMovePiece( ClientLocalData_t *cld, Player_t *player )
{
	BYTE r, piece = 0;
	Game_t *g;	
	MovePieceData_t *md;
	PacketData_t pd;
	GamePieceMoveSrv_t ps;

	memset( &ps, 0, sizeof( ps ) );
	pd.data = &ps;

	md = cld->pd->data;
#ifdef _DEBUG
	LogMessage( LOG_NOTICE, "GameMovePiece: start" );
#endif

	if( ( g = GameByPlayer( cld, player ) ) == NULL ) {
#ifdef _DEBUG
	LogMessage( LOG_NOTICE, "GameMovePiece: Game not found" );
#endif
		return;
	}

	if( g->nextMove != player ) {
#ifdef _DEBUG
	LogMessage( LOG_NOTICE, "GameMovePiece: not player's move" );
#endif
		return;
	}

	if( g->listPieces == NULL ) {
#ifdef _DEBUG
	LogMessage( LOG_NOTICE, "GameMovePiece: listPieces NULL" );
#endif
		return;
	}

	r = ClientMovePiece( &md->xsrc, &md->ysrc, &md->xdest, &md->ydest, &piece, ( g->nextMove == g->player1 ? COLOR_WHITE : COLOR_BLACK ) );

	pd.command = (char )CMD_GAME_MOVEPIECE;

	switch( r ) {
		case 1:
			ps.pieceId = (char )piece;
			ps.xdest = (char )md->xdest;
			ps.ydest = (char )md->ydest;
	
			if( g->player1 == player ) {	
				g->nextMove = g->player2;
				ps.next = COLOR_BLACK;
			} else {
				g->nextMove = g->player1;
				ps.next = COLOR_WHITE;
			}

			BroadcastToGame( g, &pd );
			break;
		case 2:
			ps.pieceId = (char )piece;
			ps.xdest = (char )md->xdest;
			ps.ydest = (char )md->ydest;
			ps.checkMate = (char )COLOR_WHITE;

			BroadcastToGame( g, &pd );
			break;
		case 3:
			ps.pieceId = (char )piece;
			ps.xdest = (char )md->xdest;
			ps.ydest = (char )md->ydest;
			ps.checkMate = (char )COLOR_BLACK;

			BroadcastToGame( g, &pd );
			break;
		default:
			// TODO: let client know - illegal turn
			break;
	}
}

void GameSit( ClientLocalData_t *cld, Player_t *player )
{
	GameSitData_t *sd;
	Pieces_t *pieces;
	sd = cld->pd->data;
	PacketData_t pd;
	GameSitServerData_t b;

	memset( &b, 0, sizeof( b ) );

	if( ( player->state & PLAYER_LOGGED ) != PLAYER_LOGGED ) {
#ifdef _DEBUG
	LogMessage( LOG_NOTICE, "Gamesit: not logged" );
#endif

		return;
	}

	if( player->state & PLAYER_PLAYING ) {
#ifdef _DEBUG
	LogMessage( LOG_NOTICE, "Gamesit: player playing" );
#endif
		return;
	}

#ifdef _DEBUG
	LogMessage( LOG_NOTICE, "game sit" );
#endif

//	pthread_mutex_lock( cld->mutex );
	for( int i = 0; i < MAX_GAMES; i++ ) {
		if( cld->cst->games[ i ].gameId != 0 ) {
			if( cld->cst->games[ i ].gameId == (int )sd->gameId && ( cld->cst->games[ i ].state & GAME_OPENED ) ) {
				if( ( cld->cst->games[ i ].player1 == NULL && (int )sd->slot == COLOR_WHITE ) || ( cld->cst->games[ i ].player2 == NULL && (int )sd->slot == COLOR_BLACK ) ) {
					for( int k = 0; k < MAX_SPECTATORS; k++ ) {
						if( cld->cst->games[ i ].spectators[ k ] == player ) {
							cld->cst->games[ i ].spectators[ k ] = NULL;
							player->state ^= PLAYER_SPECTATOR;
							break;
						}
					}
					
					if( (int )sd->slot == COLOR_WHITE ) {
#ifdef _DEBUG
						LogMessage( LOG_NOTICE, "GameSit: player1 sit" );
#endif
						cld->cst->games[ i ].player1 = player;
						cld->cst->games[ i ].nextMove = player;
					}

					if( (int )sd->slot == COLOR_BLACK ) {
#ifdef _DEBUG
						LogMessage( LOG_NOTICE, "GameSit: player2 sit" );
#endif
						cld->cst->games[ i ].player2 = player;
					}
					player->state |= PLAYER_PLAYING;

					pd.command = (char )CMD_GAME_SIT;
					pd.data = &b;
					memcpy( &b.username, &player->username, strlen( player->username ) );
					b.slot = (char )sd->slot;
					// check if match can begin
					if( cld->cst->games[ i ].player1 != NULL && cld->cst->games[ i ].player2 != NULL ) {
						cld->cst->games[ i ].state |= GAME_PLAYING;
						b.gameBegin = (char )CMD_GAME_BEGIN_PARAM_OK;
						if( ( pieces = GetPieces( cld ) ) != NULL ) {
							cld->cst->games[ i ].listPieces = pieces;
							InitPieces( pieces );
						} else {	
							// TODO: cancel game
							cld->cst->games[ i ].listPieces = NULL;
							cld->cst->games[ i ].state ^= GAME_PLAYING;
						}
#ifdef _DEBUG
						char buf[ 0x40 ];
						sprintf( buf, "GameSit: listPieces: %p", cld->cst->games[ i ].listPieces );
						LogMessage( LOG_NOTICE, buf );
#endif
					}	
												

					BroadcastToGame( &cld->cst->games[ i ], &pd );
					break;
				}
			}
		}
	}	

//	pthread_mutex_unlock( cld->mutex );

	// TODO: respond to client - sit request failed
}

void GameJoin( ClientLocalData_t *cld, Player_t *player )
{
	JoinData_t *jd;
	jd = cld->pd->data;
	PacketData_t pd;
	ServerTwoBytes_t b;

	if( ( player->state & PLAYER_LOGGED ) != PLAYER_LOGGED ) 
		return;

	if( player->state & PLAYER_PLAYING )
		return;

#ifdef _DEBUG
	LogMessage( LOG_NOTICE, "game join" );
#endif

//	pthread_mutex_lock( cld->mutex );
	for( int i = 0; i < MAX_GAMES; i++ ) {
		if( cld->cst->games[ i ].gameId != 0 ) {
			if( cld->cst->games[ i ].gameId == (int )jd->gameId && ( cld->cst->games[ i ].state & GAME_OPENED ) ) {
				for( int k = 0; k < MAX_SPECTATORS; k++ ) {
					if( cld->cst->games[ i ].spectators[ k ] == NULL ) {
						cld->cst->games[ i ].spectators[ k ] = player;
						player->state |= PLAYER_INGAME;
						player->state |= PLAYER_SPECTATOR;

						pd.command = (char )CMD_GAME_JOIN;
						pd.data = &b;
						b.byte0 = (char )CMD_GAME_JOIN_PARAM_OK;
						b.byte1 = (char )cld->cst->games[ i ].gameId;
		
						ReplyToPlayer( &pd, &cld->socketDesc );

						break;
					}
				}
				break;
			}	
		}
	}
//	pthread_mutex_unlock( cld->mutex );

	// TODO: game couldn't be found, respond to client
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
		b.byte0 = (char )CMD_LOGIN_PARAM_DETAILS_ERR;
			
		ReplyToPlayer( &pd, &cld->socketDesc );
		
		return;
	}

	memset( player->username, 0, sizeof( player->username ) );
	memcpy( player->username, ld->username, strlen( ld->username ) );
	player->state |= PLAYER_LOGGED;

#ifdef _DEBUG
	LogMessage( LOG_NOTICE, "user logged successfully" );
#endif	

	pd.command = (char )CMD_LOGIN;
	pd.data = &b;
	b.byte0 = (char )CMD_LOGIN_PARAM_DETAILS_OK;
		
	ReplyToPlayer( &pd, &cld->socketDesc );

	// when a login is successful, update client
	GameGetInitialData( &cld->socketDesc, cld->mutex );
}

void GameCreateNew( ClientLocalData_t *cld, Player_t *player )
{
	PacketData_t pd;
	ServerTwoBytes_t b;
	Game_t *g;

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

	if( player->state & PLAYER_INGAME )
		return;
	
	if( ( g = GameStore( cld, player ) ) == NULL ) {
		// TODO: send response to client
#ifdef _DEBUG
		LogMessage( LOG_WARNING, "GameStore() failed, MAX_GAMES reached" );
#endif
		return;
	}

	pd.command = (char )CMD_GAME_CREATE;
	pd.data = &b;
	b.byte0 = (char )CMD_GAME_CREATE_PARAM_OK;
	b.byte1 = (char )g->gameId;
		
	BroadcastToPlayers( cld, &pd );
}

Game_t *GameByPlayer( ClientLocalData_t *cld, Player_t *p )
{
//	pthread_mutex_lock( cld->mutex );
	if( p == NULL )
		return NULL;

	for( int i = 0; i < MAX_GAMES; i++ ) {
		if( cld->cst->games[ i ].gameId != 0 ) {
			if( cld->cst->games[ i ].player1 == p || cld->cst->games[ i ].player2 == p ) {
				//pthread_mutex_unlock( cld->mutex );
				return &cld->cst->games[ i ];
			}
		}		
	}

//	pthread_mutex_unlock( cld->mutex );
	return NULL;
}

Game_t *GameStore( ClientLocalData_t *cld, Player_t *p )
{
	Game_t *g;

	if( p == NULL )
		return NULL;
	
//	pthread_mutex_lock( cld->mutex );

	for( int i = 1; i < MAX_GAMES; i++ ) {
		if( cld->cst->games[ i ].gameId == 0 ) {
			g = &( cld->cst->games[ i ] );

			// default player state
			p->state |= PLAYER_CREATED_GAME;

			// default game state
			g->gameId = i;
			g->player1RemTime = 10.0f;
			g->player2RemTime = 10.0f;
			g->state |= GAME_OPENED;
			g->nextMove = NULL;
	
//			pthread_mutex_unlock( cld->mutex );
			return g;
		} 
	}	
	
//	pthread_mutex_unlock( cld->mutex );
	return NULL;
}

Pieces_t *GetPieces( ClientLocalData_t *cld )
{
	for( int i = 0; i < MAX_GAMES; i++ ) {
		if( !( cld->cst->pieces[ i ][ 0 ].state & PIECE_INUSE ) ) {
			 return &( cld->cst->pieces[ i ] );
		}
	}

	return NULL;
}
