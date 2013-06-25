#ifndef __NET_H_
#define __NET_H_

void BroadcastToGame( Game_t *game, PacketData_t *pd );
void BroadcastToPlayers( Player_t *players, PacketData_t *pd );
void ReplyToPlayer( PacketData_t *pd, const int *sd );
void PacketSend( PacketData_t *pd, const int *sd );

#endif
