#ifndef __PLAYER_H_
#define __PLAYER_H_

Player_t *GetPlayer( ClientLocalData_t* );
Player_t *StorePlayer( ClientLocalData_t* );
void RemovePlayer( ClientLocalData_t* );
void FreePlayer( Player_t* );

#endif
