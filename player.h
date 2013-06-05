#ifndef __PLAYER_H_
#define __PLAYER_H_

void RemoveGame( Game_t *g ) ;
Player_t *GetPlayer( ClientLocalData_t* );
Player_t *StorePlayer( ClientLocalData_t* );
void RemovePlayer( ClientLocalData_t* );
void RemovePlayerGame( ClientLocalData_t *cld, Player_t *p );
void FreePlayer( Player_t* );
Game_t *FindGameByPlayer( ClientLocalData_t *cld, Player_t *p );

#endif
