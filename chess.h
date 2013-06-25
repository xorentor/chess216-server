#ifndef __CHESS_H_
#define __CHESS_H_

#define		PIECE_INPLAY		0x1
#define		PIECE_ISSELECTED 	0x2
#define		PIECE_ISINITIAL		0x4
#define		PIECE_INUSE		0x8

BYTE ClientMovePiece( const BYTE *xsrc, const BYTE *ysrc, const BYTE *x, const BYTE *y, BYTE *piece, const BYTE color );
void InitPieces( Pieces_t *pieces, Move_t *move );	

void AddPiece( Pieces_t *listPieces, const BYTE i, const BYTE x, const BYTE y, const BYTE skin, const BYTE color );
void DestroyPieces();

BYTE CheckMove( Piece_t *piece, const BYTE xdest, const BYTE ydest );
BYTE MoveKnight( Piece_t *piece, const BYTE *xdest, const BYTE *ydest );
BYTE MoveBishop( Piece_t *piece, const BYTE *xdest, const BYTE *ydest );
BYTE MoveRook( Piece_t *piece, const BYTE *xdest, const BYTE *ydest );
BYTE MoveQueen( Piece_t *piece, const BYTE *xdest, const BYTE *ydest );
BYTE MoveKing( Piece_t *piece, const BYTE *xdest, const BYTE *ydest );
BYTE MovePawn( Piece_t *piece, const BYTE *xdest, const BYTE *ydest );

BYTE KingCheck( const BYTE *color );
BYTE KingCheckSimulate( Piece_t *piece, const BYTE xdest, const BYTE ydest );
BYTE KingCheckMate( const BYTE color );
BYTE SquareChecked( const BYTE x, const BYTE y, const BYTE *color );
BYTE SquareFree( const BYTE x, const BYTE y );
BYTE EnPassant( Piece_t *piece, const BYTE *xdest, const BYTE *ydest );

BYTE MovePiece( Piece_t *piece, const BYTE *xdest, const BYTE *ydest );
BYTE MovePieceIter( const BYTE *j, const BYTE *k, const BYTE *xdest, const BYTE *ydest, Piece_t *piece );
void FinalMovePiece( Piece_t *piece, const BYTE *x, const BYTE *y );

#endif
