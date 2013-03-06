#ifndef __GAME_H_
#define __GAME_H_

void GameLogin( void *data, const int *sd, pthread_mutex_t *mutex, int *threadFlag, Player_t *player );
void GameCreateNew( void *data, const int *sd, pthread_mutex_t *mutex, int *threadFlag, Player_t *player );
Game_t *GameStore( Player_t *p, pthread_mutex_t *mutex );

#endif
