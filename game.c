#include "common.h"
#include "player.h"
#include "log.h"
#include "net.h"
#include "chess.h"
#include "game.h"
#include "./include/filedb.h"

inline int ISSET( void *mem, int s )
{
        char *t;
        t = (char *)mem;
        while( s > 0 ) {
        	if( t[ --s ] != 0 ) 
                	return 1;
	}
        return 0;
} 

void EndGame( Player_t *players, Game_t *g, Player_t *winner )
{
	PacketData_t pd;
	ServerTwoBytes_t b;
	EloSrv_t elo;

	memset( &elo, 0, sizeof( elo ) );
	memset( &b, 0, sizeof( b ) );

	double win = 1.0f, draw = 0.5f, lose = 0.0f;

	assert( g );
	assert( winner );

	// close the game
	g->state ^= GAME_OPENED;
	g->state ^= GAME_PLAYING;

	// tell clients to remove game from list	
	pd.command = (char )CMD_GAME_CREATE;
	pd.data = &b;
	b.byte0 = (char )CMD_GAME_CREATE_PARAM_DELETE;
	b.byte1 = (char )g->gameId;
	BroadcastToPlayers( players, &pd );
	memset( &b, 0, sizeof( b ) );

	// draw
	if( winner == NULL ) {
		UpdateElo( &g->player1->elorating, &g->player2->elorating, &draw );
		UpdateElo( &g->player2->elorating, &g->player1->elorating, &draw );
		b.byte0 = CMD_GAME_FINISHED_DRAW;
	}
	// somebody won
	else if( winner != NULL ) {
		if( winner == g->player1 ) {
			UpdateElo( &g->player1->elorating, &g->player2->elorating, &win );
			UpdateElo( &g->player2->elorating, &g->player1->elorating, &lose );
			b.byte0 = COLOR_WHITE;
		} else if( winner == g->player2 ) {
			UpdateElo( &g->player2->elorating, &g->player1->elorating, &win );
			UpdateElo( &g->player1->elorating, &g->player2->elorating, &lose );
			b.byte0 = COLOR_BLACK;
		}
	}
	
	setEloByUser( g->player1->username, (int )g->player1->elorating );
	setEloByUser( g->player2->username, (int )g->player2->elorating );

	// reset players' state
	g->player1->state &= PLAYER_LOGGED;
	g->player2->state &= PLAYER_LOGGED;

	// reset spectators' state	
	int i;
	for( i = 0; i < MAX_SPECTATORS; i++ ) {
		if( g->spectators[ i ] != NULL )
			g->spectators[ i ]->state &= PLAYER_LOGGED;
	}

	// tell game participants( players, specs ) it's over and zero init theirs gui
	pd.command = (char )CMD_GAME_FINISHED;
	BroadcastToGame( g, &pd );

	// tell players new elo ratings
	pd.command = (char )CMD_GAME_ELO;
	pd.data = &elo;
	elo.elo_value = (int )g->player1->elorating;
	ReplyToPlayer( &pd, &g->player1->socketDesc );
	elo.elo_value = (int )g->player2->elorating;
	ReplyToPlayer( &pd, &g->player2->socketDesc );

	// remove after all
	RemoveGame( g );
}

void UpdateElo( double *w, const double *l, const double *state )
{
        int k = 32;
        double exp;

	assert( w );
	assert( l );
	assert( state );

        exp = 1 / ( 1 + pow( 10.0f, ( ( *l - *w ) / 400 ) ) );

        if( ( *w >= 2100 && *w < 2401 ) || ( *l >= 2100 && *l < 2401 ) )
                k = 24;
        else if( *w > 2400 && *l > 2400 )
                k = 16;

        *w += k * ( *state - exp );
        *w = roundup( w );

}

void GameGetInitialData( ClientLocalData_t *cld )
{
	// TODO:
	//short playersCount;
	// 1. online users
	//players = GameGetOnlinePlayers( pthread_mutex_t *mutex );
	UpdateGames( cld );
	
	// 2. list of available games
	
	// 3. 
}

/* send latest games to every clientlist if:
	3. if player logs in, send him this as well
*/
void UpdateGames( ClientLocalData_t *cld ) {
	PacketData_t pd;
	ServerTwoBytes_t b;

	int i;
	for( i = 0; i < MAX_GAMES; i++ ) {
		if( cld->cst->games[ i ].gameId != 0 ) {
			pd.command = (char )CMD_GAME_CREATE;
			pd.data = &b;
			b.byte0 = (char )CMD_GAME_CREATE_PARAM_OK;
			b.byte1 = (char )cld->cst->games[ i ].gameId;
		
			ReplyToPlayer( &pd, &cld->socketDesc );
		}
	} 
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

	if( ISSET( &g->pieces, sizeof( g->pieces ) ) == 0 ) {
#ifdef _DEBUG
	LogMessage( LOG_NOTICE, "GameMovePiece: pieces NULL" );
#endif
		return;
	}

	r = ClientMovePiece( &md->xsrc, &md->ysrc, &md->xdest, &md->ydest, &piece, ( g->nextMove == g->player1 ? COLOR_WHITE : COLOR_BLACK ) );

	pd.command = (char )CMD_GAME_MOVEPIECE;

	switch( r ) {
		// regular move
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
		// checkmate - Black
		case 2:
			ps.pieceId = (char )piece;
			ps.xdest = (char )md->xdest;
			ps.ydest = (char )md->ydest;
			ps.checkMate = (char )CMD_GAME_PARAM_CHECKMATE_B;

			BroadcastToGame( g, &pd );
			EndGame( cld->cst->players, g, g->player1 );
			break;
		// checkmate - White
		case 3:
			ps.pieceId = (char )piece;
			ps.xdest = (char )md->xdest;
			ps.ydest = (char )md->ydest;
			ps.checkMate = (char )CMD_GAME_PARAM_CHECKMATE_W;

			BroadcastToGame( g, &pd );
			EndGame( cld->cst->players, g, g->player2 );
			break;
		default:
			// TODO: let client know - illegal turn
			break;
	}
}

void GameStand( ClientLocalData_t *cld, Player_t *player )
{
	GameStandData_t *sd;
	sd = cld->pd->data;
	PacketData_t pd;
	GameStandServerData_t b;

	memset( &b, 0, sizeof( b ) );

	if( ( player->state & PLAYER_LOGGED ) != PLAYER_LOGGED ) {
#ifdef _DEBUG
	LogMessage( LOG_NOTICE, "Player not logged" );
#endif

		return;
	}

	int i, k;
	for( i = 0; i < MAX_GAMES; i++ ) {
		if( cld->cst->games[ i ].gameId != 0 ) {
			if( cld->cst->games[ i ].gameId == (int )sd->gameId && ( cld->cst->games[ i ].state & GAME_OPENED ) ) {
			// player cannot stand if game is already being played
			if( cld->cst->games[ i ].state & GAME_PLAYING )
				return;

				if( ( cld->cst->games[ i ].player1 == player && (int )sd->slot == COLOR_WHITE ) || ( cld->cst->games[ i ].player2 == player && (int )sd->slot == COLOR_BLACK ) ) {
					if( (int )sd->slot == COLOR_WHITE ) {
#ifdef _DEBUG
						LogMessage( LOG_NOTICE, "GameSit: player1 stand" );
#endif
						cld->cst->games[ i ].player1 = NULL;
					}
					else if( (int )sd->slot == COLOR_BLACK ) {
#ifdef _DEBUG
						LogMessage( LOG_NOTICE, "GameSit: player2 stand" );
#endif
						cld->cst->games[ i ].player2 = NULL;
					}

					player->state ^= PLAYER_PLAYING;

					for( k = 0; k < MAX_SPECTATORS; k++ ) {
						if( cld->cst->games[ i ].spectators[ k ] == NULL ) {
							cld->cst->games[ i ].spectators[ k ] = player;
							break; 
						}
					}

					pd.command = (char )CMD_GAME_STAND;
					pd.data = &b;
					b.param = (char )CMD_GAME_STAND_PARAM_OK;
					b.slot = (char )sd->slot;

					BroadcastToGame( &cld->cst->games[ i ], &pd );
					break;
				}
			}	
		}
	}
}

void GameSit( ClientLocalData_t *cld, Player_t *player )
{
	GameSitData_t *sd;
	//Pieces_t *pieces;
	sd = cld->pd->data;
	PacketData_t pd;
	GameSitServerData_t b;

	memset( &b, 0, sizeof( b ) );

	if( ( player->state & PLAYER_LOGGED ) != PLAYER_LOGGED ) {
#ifdef _DEBUG
	LogMessage( LOG_NOTICE, "Player not logged" );
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
	printf("gamesit: gameId: %d slot: %d", (int )sd->gameId, (int )sd->slot );

//	pthread_mutex_lock( cld->mutex );
	int i, k;
	for( i = 0; i < MAX_GAMES; i++ ) {
		if( cld->cst->games[ i ].gameId != 0 ) {
			if( cld->cst->games[ i ].gameId == (int )sd->gameId && ( cld->cst->games[ i ].state & GAME_OPENED ) ) {
				if( ( cld->cst->games[ i ].player1 == NULL && (int )sd->slot == COLOR_WHITE ) || ( cld->cst->games[ i ].player2 == NULL && (int )sd->slot == COLOR_BLACK ) ) {
					// player sits down - remove him from spectators
					for( k = 0; k < MAX_SPECTATORS; k++ ) {
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
					player->state ^= PLAYER_SPECTATOR;

					pd.command = (char )CMD_GAME_SIT;
					pd.data = &b;
					memcpy( &b.username, &player->username, strlen( player->username ) );
					b.slot = (char )sd->slot;
					// check if match can begin
					if( cld->cst->games[ i ].player1 != NULL && cld->cst->games[ i ].player2 != NULL ) {
						cld->cst->games[ i ].state |= GAME_PLAYING;
						b.gameBegin = (char )CMD_GAME_BEGIN_PARAM_OK;
						InitPieces( &cld->cst->games[ i ].pieces, &cld->cst->games[ i ].lastMove );
//						cld->cst->games[ i ].listPieces = &cld->cst->games[ i ].pieces;
/*
						if( ( pieces = GetPieces( cld ) ) != NULL ) {
							cld->cst->games[ i ].listPieces = pieces;
							InitPieces( pieces, &cld->cst->games[ i ].lastMove );
						} else {	
							// TODO: cancel game
							cld->cst->games[ i ].listPieces = NULL;
							cld->cst->games[ i ].state ^= GAME_PLAYING;
						}
*/
#ifdef _DEBUG
						char buf[ 0x40 ];
						sprintf( buf, "GameSit: pieces: %p", cld->cst->games[ i ].pieces );
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

void GameAutoJoin( ClientLocalData_t *cld, Player_t *player, const int *gameId )
{
	//JoinData_t *jd;
	//jd = cld->pd->data;
	PacketData_t pd;
	ServerTwoBytes_t b;

	if( ( player->state & PLAYER_LOGGED ) != PLAYER_LOGGED ) 
		return;

	if( player->state & PLAYER_PLAYING )
		return;
	
	int i, k;
	for( i = 1; i < MAX_GAMES; i++ ) {
		if( cld->cst->games[ i ].gameId != 0 ) {
			if( cld->cst->games[ i ].gameId == *gameId && ( cld->cst->games[ i ].state & GAME_OPENED ) ) {
				for( k = 0; k < MAX_SPECTATORS; k++ ) {
					if( cld->cst->games[ i ].spectators[ k ] == NULL ) {

						printf( "game join OK, reply now, player as spectator\n" );
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

void GameJoin( ClientLocalData_t *cld, Player_t *player )
{
	JoinData_t *jd;
	jd = cld->pd->data;
	PacketData_t pd;
	ServerTwoBytes_t b;
	Game_t *g;

	if( ( player->state & PLAYER_LOGGED ) != PLAYER_LOGGED ) 
		return;

	if( player->state & PLAYER_PLAYING )
		return;

#ifdef _DEBUG
	LogMessage( LOG_NOTICE, "game join" );
#endif

	// player is already in another game?
	if( ( g = FindGameByPlayer( cld, player ) ) != NULL ) {
		// player wants to join the same game
		if( g->gameId == (int )jd->gameId ) {
			// remove player from existing game before he joins a new one
			RemovePlayerGame( cld, player );
			return;
		}
	}

//	pthread_mutex_lock( cld->mutex );
	int i, k;
	for( i = 1; i < MAX_GAMES; i++ ) {
		if( cld->cst->games[ i ].gameId != 0 ) {
			if( cld->cst->games[ i ].gameId == (int )jd->gameId && ( cld->cst->games[ i ].state & GAME_OPENED ) ) {
				for( k = 0; k < MAX_SPECTATORS; k++ ) {
					if( cld->cst->games[ i ].spectators[ k ] == NULL ) {

						printf( "game join OK, reply now" );
						cld->cst->games[ i ].spectators[ k ] = player;
						player->state |= PLAYER_INGAME;
						player->state |= PLAYER_SPECTATOR;

						pd.command = (char )CMD_GAME_JOIN;
						pd.data = &b;
						b.byte0 = (char )CMD_GAME_JOIN_PARAM_OK;
						b.byte1 = (char )cld->cst->games[ i ].gameId;
		
						ReplyToPlayer( &pd, &cld->socketDesc );

						if( cld->cst->games[ i ].player1 != NULL ) 
							GameJoinSeatsStatus( cld, cld->cst->games[ i ].player1, COLOR_WHITE );
						if( cld->cst->games[ i ].player2 != NULL ) 
							GameJoinSeatsStatus( cld, cld->cst->games[ i ].player2, COLOR_BLACK );

						if( ISSET( cld->cst->games[ i ].pieces, sizeof( cld->cst->games[ i ].pieces ) ) != 0 )
							GamePiecesStatus( cld, &cld->cst->games[ i ].pieces );

#ifdef _DEBUG
						char buf[ 0x40 ];
						sprintf( buf, "GameSit: pieces: %p", cld->cst->games[ i ].pieces );
						LogMessage( LOG_NOTICE, buf );
#endif
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

void GamePiecesStatus( ClientLocalData_t *cld, Pieces_t *pieces )
{
	PacketData_t pd;
	char b[ 225 ];
	int index = 0;

	memset( &b, 0, sizeof( b ) );

	pd.command = (char )CMD_GAME_INITIAL_PIECES;
	pd.data = &b;

	int i;
	for( i = 0; i < 32; i++ ) {
		memcpy( &b[ index++ ], &pieces[ i ]->xpos, sizeof( char ) );
		memcpy( &b[ index++ ], &pieces[ i ]->ypos, sizeof( char ) );
		memcpy( &b[ index++ ], &pieces[ i ]->ID, sizeof( char ) );
		memcpy( &b[ index++ ], &pieces[ i ]->skinID, sizeof( char ) );
		memcpy( &b[ index++ ], &pieces[ i ]->state, sizeof( char ) );
	}

	ReplyToPlayer( &pd, &cld->socketDesc );
}

void GameJoinSeatsStatus( ClientLocalData_t *cld, Player_t *player, const int slot )
{
	PacketData_t pd;
	GameSitServerData_t b;

	memset( &b, 0, sizeof( b ) );

	pd.command = (char )CMD_GAME_SIT;
	pd.data = &b;
	memcpy( &b.username, &player->username, strlen( player->username ) );
	b.slot = slot;

	ReplyToPlayer( &pd, &cld->socketDesc );
}

void GameLogin( ClientLocalData_t *cld, Player_t *player )
{
	LoginData_t *ld;
	ld = (LoginData_t *)(cld->pd->data);	
	PacketData_t pd;
	GameLoginSrv_t b;

	memset( &b, 0, sizeof( b ) );	
	
	//if( strlen( ld->username ) < 2 || strlen( ld->password ) < 2 ) {
	if( strlen( ld->username ) < 2 ) {
#ifdef _DEBUG
		// send response to client as well
		LogMessage( LOG_WARNING, "login data incomplete" );
#endif
		return;
	}

	if( verifyUser( ld->username, ld->password ) != 0 ) {

#ifdef _DEBUG
		// send response to client as well
		LogMessage( LOG_NOTICE, "incorrect login details" );
#endif	
		pd.command = (char )CMD_LOGIN;
		pd.data = &b;
		//b.byte0 = (char )CMD_LOGIN_PARAM_DETAILS_ERR;
		b.param = (char )CMD_LOGIN_PARAM_DETAILS_ERR;

		ReplyToPlayer( &pd, &cld->socketDesc );
		
		return;
	}

	memset( player->username, 0, sizeof( player->username ) );
	if( strlen( ld->username ) > sizeof( player->username ) )
		memcpy( player->username, ld->username, sizeof( player->username ) );
	else
		memcpy( player->username, ld->username, strlen( ld->username ) );
	player->state |= PLAYER_LOGGED;
	player->elorating = getEloByUser( player->username );

#ifdef _DEBUG
	LogMessage( LOG_NOTICE, "user logged successfully" );
#endif	

	pd.command = (char )CMD_LOGIN;
	pd.data = &b;
//	b.byte0 = (char )CMD_LOGIN_PARAM_DETAILS_OK;
	b.param = (char )CMD_LOGIN_PARAM_DETAILS_OK;
	memcpy( &b.username, player->username, strlen( player->username ) );
	memcpy( &b.elorating, (char *)&player->elorating, sizeof( player->elorating ) );
	printf("ELOrating: %f\n", player->elorating );

	ReplyToPlayer( &pd, &cld->socketDesc );

	// when a login is successful, update client
	GameGetInitialData( cld );
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
		
	BroadcastToPlayers( cld->cst->players, &pd );

	// immediately join
	GameAutoJoin( cld, player, &g->gameId );
}

Game_t *GameByPlayer( ClientLocalData_t *cld, Player_t *p )
{
//	pthread_mutex_lock( cld->mutex );
	if( p == NULL )
		return NULL;
	int i;
	for( i = 0; i < MAX_GAMES; i++ ) {
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

	int i;
	for( i = 1; i < MAX_GAMES; i++ ) {
		if( cld->cst->games[ i ].gameId == 0 ) {
			g = &( cld->cst->games[ i ] );

			// default player state
			p->state |= PLAYER_CREATED_GAME;

			// init pointers
//			g->listPieces = NULL;
			g->player1 = NULL;
			g->player2 = NULL;
			g->nextMove = NULL;
			memset( &g->lastMove, 0, sizeof( g->lastMove ) );
			memset( &g->pieces, 0, sizeof( g->pieces ) );

			g->gameId = i;
			g->p1_min = 0;
			g->p1_sec = 2;
			g->p2_min = 0;
			g->p2_sec = 2;
			g->state |= GAME_OPENED;
			g->t0 = g->t1 = g->t2 = g->t3 = g->t4 = g->t5 = g->t6 = 0;

			int k;
			for( k = 0; k < MAX_SPECTATORS; k++ ) {
				g->spectators[ k ] = NULL;
			}
	
//			pthread_mutex_unlock( cld->mutex );
			return g;
		} 
	}	
	
//	pthread_mutex_unlock( cld->mutex );
	return NULL;
}

/*
Pieces_t *GetPieces( ClientLocalData_t *cld )
{
	int i;
	for( i = 0; i < MAX_GAMES; i++ ) {
		if( !( cld->cst->pieces[ i ][ 0 ].state & PIECE_INUSE ) ) {
			 return &( cld->cst->pieces[ i ] );
		}
	}

	return NULL;
}
*/
void RemoveGame( Game_t *g ) 
{
	memset( g, 0, sizeof( Game_t ) );
}
