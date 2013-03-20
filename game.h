#ifndef __GAME_H_
#define __GAME_H_

void GameLogin( ClientLocalData_t *cld, Player_t *player );
void GameCreateNew( ClientLocalData_t *cld, Player_t *player );
void GameJoin( ClientLocalData_t *cld, Player_t *player );
void GameSit( ClientLocalData_t *cld, Player_t *player );
void GameMovePiece( ClientLocalData_t *cld, Player_t *player );
Game_t *GameStore( ClientLocalData_t *cld, Player_t *p );
Game_t *GameByPlayer( ClientLocalData_t *cld, Player_t *p );
Pieces_t *GetPieces( ClientLocalData_t *cld );

#endif
