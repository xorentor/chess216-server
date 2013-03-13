#ifndef __NET_H_
#define __NET_H_

void BroadcastToGame( ClientLocalData_t *cld, Game_t *game, PacketData_t *pd );
void BroadcastToPlayers( ClientLocalData_t *cld, PacketData_t *pd );
void ReplyToPlayer( PacketData_t *pd, const int *sd );

#endif
