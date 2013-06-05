#ifndef __GAME_H_
#define __GAME_H_

extern int roundup( const double *f );

void GameLogin( ClientLocalData_t *cld, Player_t *player );
void GameCreateNew( ClientLocalData_t *cld, Player_t *player );
void GameJoin( ClientLocalData_t *cld, Player_t *player );
void GameSit( ClientLocalData_t *cld, Player_t *player );
void GameStand( ClientLocalData_t *cld, Player_t *player );
void GameMovePiece( ClientLocalData_t *cld, Player_t *player );
Game_t *GameStore( ClientLocalData_t *cld, Player_t *p );
Game_t *GameByPlayer( ClientLocalData_t *cld, Player_t *p );
Pieces_t *GetPieces( ClientLocalData_t *cld );

void UpdateGames( ClientLocalData_t *cld );
void GameAutoJoin( ClientLocalData_t *cld, Player_t *player, const int *gameId );
void GameJoinSeatsStatus( ClientLocalData_t *cld, Player_t *player, const int slot );
void GamePiecesStatus( ClientLocalData_t *cld, Pieces_t *pieces );

void UpdateElo( double *w, const double *l, const double *state );
void EndGame( Game_t *g, Player_t *winner );

#endif
