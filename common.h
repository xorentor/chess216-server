#ifndef __COMMON_H_
#define __COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <pthread.h>
#include <memory.h>

#define 	_DEBUG			1
#define		_DEBUG_GAME		1
#define		INLINE			inline
#define		BYTE			char
#define		ctrue			1
#define		cfalse			0

#define		PORTNO			5770
#define		F_QUIT			0x0001
#define		BUFFER_LEN		0x100

// max
#define		MAX_SPECTATORS		4
#define		MAX_GAMES		256
#define		MAX_PIECES		32
#define		MAX_THREADS		256
#define		MAX_CLIENTS		256

// game colours
#define         COLOR_WHITE             1
#define         COLOR_BLACK             2

// player state
#define		PLAYER_LOGGED		0x1
#define		PLAYER_CREATED_GAME	0x2
#define		PLAYER_PLAYING		0x4
#define		PLAYER_CHATTING		0x8
#define		PLAYER_AWAY		0x10
#define		PLAYER_OFFER_DRAW	0x20
#define		PLAYER_ACCEPT_DRAW	0x40
#define		PLAYER_RESIGN		0x80
#define		PLAYER_SPECTATOR	0x100
#define		PLAYER_OFFER_REMATCH	0x200
#define		PLAYER_ACCEPT_REMATCH	0x400
#define		PLAYER_REFUSE_DRAW	0x800
#define		PLAYER_REFUSE_REMATCH	0x1000
#define		PLAYER_LOST		0x2000
#define		PLAYER_WON		0x4000
#define		PLAYER_INGAME		0x8000

// game state
#define		GAME_OPENED		0x1
#define		GAME_PLAYING		0x2
#define		GAME_PAUSED		0x4
#define		GAME_FINISHED		0x8
#define		GAME_CANCELLED		0x10

#ifndef NULL
#define 	NULL ((void *)0)
#endif

// as same as server ***
enum {
	CMD_LOGIN = 1,
	CMD_GAME_CREATE,
	CMD_GAME_JOIN,
	CMD_GAME_SIT,
        CMD_GAME_MOVEPIECE,
};      
                                          
enum {
	CMD_LOGIN_PARAM_DETAILS_OK = 0,
	CMD_LOGIN_PARAM_DETAILS_ERR,
	CMD_GAME_CREATE_PARAM_OK,
	CMD_GAME_CREATE_PARAM_NOK,
	CMD_GAME_JOIN_PARAM_OK,
	CMD_GAME_JOIN_PARAM_NOK,
	CMD_GAME_BEGIN_PARAM_OK,
	CMD_GAME_PARAM_CHECKMATE,
};

enum
{
        BLACK_KING = 0,
        WHITE_KING,
        BLACK_QUEEN,
        WHITE_QUEEN,
        BLACK_ROOK,
        WHITE_ROOK,
        BLACK_BISHOP,
        WHITE_BISHOP,
        BLACK_KNIGHT,
        WHITE_KNIGHT,
        BLACK_PAWN,
        WHITE_PAWN
};
// ***  

typedef struct Thread_s
{
	pthread_t pthread;
	int inuse;
	time_t spawned;
} Thread_t;

typedef struct PacketData_s
{
	char command;
	char length;	
	void *data;
} PacketData_t;

typedef struct ServerByte_s
{
	char byte0;
} ServerByte_t;

typedef struct ServerTwoBytes_s
{
	char byte0;
	char byte1;
} ServerTwoBytes_t;

typedef struct Player_s
{
	time_t loggedTime;
	char username[ 32 ];
	int socketDesc;
	int state;		// logged, sitting, playing, chatting, away, ...
} Player_t;

typedef struct Piece_s
{
// leave these to client
//	int x;	
//	int y;
	char xpos;
	char ypos;
	char ID;
	char skinID;
	char color;
	int state;
} Piece_t;

typedef Piece_t Pieces_t[ 32 ];

typedef struct JoinData_s
{       
        char gameId;
} JoinData_t;

typedef struct MovePieceData_s
{
        char xsrc;      
        char ysrc;
        char xdest;     
        char ydest;
} MovePieceData_t;

typedef struct GameSitServerData_s
{                       
        char slot;
        char username[ 32 ];
	char gameBegin;
} GameSitServerData_t;  

typedef struct GameSitData_s
{
        char gameId;
        char slot;
} GameSitData_t;

typedef struct LoginData_s
{
	char username[ 32 ];
	char password[ 32 ];
} LoginData_t;

typedef struct GamePieceMoveSrv_s
{
	char pieceId;
	char xdest;
	char ydest;
	char checkMate;
} GamePieceMoveSrv_t;

typedef struct Game_s
{
	int gameId;
	Pieces_t *listPieces;
	Player_t *player1;
	Player_t *player2;
	Player_t *spectators[ MAX_SPECTATORS ];
	Player_t *nextMove;
	float player1RemTime;
	float player2RemTime;
	int state;
} Game_t;

typedef struct Info_s
{
	BYTE playersCount;	// int despite that this can't be higher than MAX_CLIENTS
	BYTE gamesCount;		// int despite that this can't be higher than MAX_GAMES
} Info_t; 

typedef struct CrossThread_s
{	
	Piece_t pieces[ MAX_GAMES ][ 32 ];
	Game_t games[ MAX_GAMES ];
	Player_t players[ MAX_CLIENTS ];
	Info_t info;
} CrossThread_t;

typedef struct ClientLocalData_s
{
	int socketDesc;
	int *quitFlag;
	pthread_mutex_t *mutex;
	PacketData_t *pd;
	CrossThread_t *cst;
} ClientLocalData_t;

typedef struct ThreadParam_s
{
	int *socketId;
	Thread_t *threads;
	CrossThread_t *cst;
} ThreadParam_t;

typedef struct ClientThread_s
{
	Thread_t *threads;
	struct sockaddr_in *cli_addr;
	socklen_t *clilen;
	int *sockfd;
	CrossThread_t *cst;
} ClientThread_t;

#endif
