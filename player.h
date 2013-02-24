#ifndef __PLAYER_H_
#define __PLAYER_H_

Player_t *GetPlayer( const int *socketDescriptor );
Player_t *StorePlayer( const int *socketDescriptor );
void RemovePlayer( Player_t *p );
void FreePlayer( Player_t *p );

#endif
