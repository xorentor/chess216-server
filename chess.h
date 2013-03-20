#ifndef __CHESS_H_
#define __CHESS_H_

#define		PIECE_INPLAY		0x1
#define		PIECE_ISSELECTED 	0x2
#define		PIECE_ISINITIAL		0x4
#define		PIECE_INUSE		0x8

typedef struct Move_s
{
	char skinID;
	char srcX;
	char srcY;
	char destX;
	char destY;
} Move_t;

int ClientMovePiece( const int xsrc, const int ysrc, const int x, const int y, int *piece );
void InitPieces( Pieces_t *pieces );	

void AddPiece( Pieces_t *listPieces, const int i, const int x, const int y, const int skin, const int color );
int MoveKnight( Piece_t *piece, const int xdest, const int ydest );
void DestroyPieces();

int CheckMove( Piece_t *piece, const int xdest, const int ydest );
int MoveBishop( Piece_t *piece, const int xdest, const int ydest );
int MoveRook( Piece_t *piece, const int xdest, const int ydest );
int MoveQueen( Piece_t *piece, const int xdest, const int ydest );
int MoveKing( Piece_t *piece, const int xdest, const int ydest );
int MovePawn( Piece_t *piece, const int xdest, const int ydest );

int KingCheck( const int color );
int KingCheckSimulate( Piece_t *piece, const int xdest, const int ydest );
int KingCheckMate( const int color );
int SquareChecked( const int x, const int y, const int color );
int SquareFree( const int x, const int y );
int EnPassant( Piece_t *piece, const int xdest, const int ydest );

int MovePiece( Piece_t *piece, const int xdest, const int ydest );
int MovePieceIter( const int j, const int k, const int xdest, const int ydest, Piece_t *piece );
void FinalMovePiece( Piece_t *piece, const int x, const int y );
int CheckSpecials( Piece_t *piece, const int xdest, const int ydest );

#endif
