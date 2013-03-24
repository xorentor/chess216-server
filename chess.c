#include "common.h"
#include "log.h"
#include "chess.h"

Move_t lastMove; // FIXME: this is wrong
Pieces_t *listPieces;

INLINE void InitPieces( Pieces_t *pieces ) 
{
	listPieces = pieces;
	BYTE i = 0;

	// white Pawns
	for( ; i < 8; i++ ) {
		AddPiece( listPieces, i, i, 6, WHITE_PAWN, COLOR_WHITE );
	}

	// black Pawns
	for( ; i < 16; i++ ) {
		AddPiece( listPieces, i, (i-8), 1, BLACK_PAWN, COLOR_BLACK );
	}

	// rooks
	AddPiece( listPieces, i++, 7, 7, WHITE_ROOK, COLOR_WHITE );
	AddPiece( listPieces, i++, 0, 7, WHITE_ROOK, COLOR_WHITE );
	AddPiece( listPieces, i++, 7, 0, BLACK_ROOK, COLOR_BLACK );
	AddPiece( listPieces, i++, 0, 0, BLACK_ROOK, COLOR_BLACK );
	
	// knights
	AddPiece( listPieces, i++, 1, 7, WHITE_KNIGHT, COLOR_WHITE );
	AddPiece( listPieces, i++, 6, 7, WHITE_KNIGHT, COLOR_WHITE );
	AddPiece( listPieces, i++, 1, 0, BLACK_KNIGHT, COLOR_BLACK );
	AddPiece( listPieces, i++, 6, 0, BLACK_KNIGHT, COLOR_BLACK );

	// bishops
	AddPiece( listPieces, i++, 2, 7, WHITE_BISHOP, COLOR_WHITE );
	AddPiece( listPieces, i++, 5, 7, WHITE_BISHOP, COLOR_WHITE );
	AddPiece( listPieces, i++, 2, 0, BLACK_BISHOP, COLOR_BLACK );
	AddPiece( listPieces, i++, 5, 0, BLACK_BISHOP, COLOR_BLACK );

	// queens
	AddPiece( listPieces, i++, 3, 7, WHITE_QUEEN, COLOR_WHITE );
	AddPiece( listPieces, i++, 3, 0, BLACK_QUEEN, COLOR_BLACK );

	// kings
	AddPiece( listPieces, i++, 4, 7, WHITE_KING, COLOR_WHITE );
	AddPiece( listPieces, i++, 4, 0, BLACK_KING, COLOR_BLACK );
}

INLINE void AddPiece( Pieces_t *listPieces, const BYTE i, const BYTE x, const BYTE y, const BYTE skin, const BYTE color ) 
{
	// copy them
	listPieces[ i ]->xpos = x;
	listPieces[ i ]->ypos = y;
	listPieces[ i ]->ID = i;
	listPieces[ i ]->skinID = skin;
	listPieces[ i ]->color = color;
	listPieces[ i ]->state = PIECE_INPLAY | PIECE_ISINITIAL | PIECE_INUSE;
}

INLINE void DestroyPieces()
{
	/*
	for( int i = 0; i < 32; i++ ) {
		if( listPieces[ i ] != NULL ) 
			free( listPieces[ i ] );	
	}

	free( listPieces );
	*/
}

INLINE BYTE CheckMove( Piece_t *piece, const BYTE xdest, const BYTE ydest )
{
	if( xdest < 0 || xdest > 7 || ydest < 0 || ydest > 7 )
		return cfalse;

	if( piece->xpos == xdest && piece->ypos == ydest )
		return cfalse;

	switch( piece->skinID ) {
		case WHITE_KNIGHT:
		case BLACK_KNIGHT:
			return MoveKnight( piece, &xdest, &ydest );
			break;

		case WHITE_BISHOP:
		case BLACK_BISHOP:
			return MoveBishop( piece, &xdest, &ydest );	
			break;
		
		case WHITE_ROOK:
		case BLACK_ROOK:
			return MoveRook( piece, &xdest, &ydest );	
			break;

		case WHITE_QUEEN:
		case BLACK_QUEEN:
			return MoveQueen( piece, &xdest, &ydest );
			break;
		case WHITE_KING:
		case BLACK_KING:
			return MoveKing( piece, &xdest, &ydest );
			break;

		case WHITE_PAWN:
		case BLACK_PAWN:
			return MovePawn( piece, &xdest, &ydest );
			break;

		default:
#ifdef _DEBUG_GAME
			LogMessage( LOG_NOTICE, "Unknown piece" );
#endif
			return cfalse;
			break;
	}
}

INLINE BYTE SquareFree( const BYTE x, const BYTE y )
{
	for( BYTE i = 0; i < MAX_PIECES; i++ ) {
		if( !( listPieces[ i ]->state & PIECE_INPLAY ) )
			continue;

		if( x == listPieces[ i ]->xpos && y == listPieces[ i ]->ypos )
			return cfalse;
	}

	return ctrue;
}

INLINE BYTE SquareChecked( const BYTE x, const BYTE y, const BYTE *color )
{
	for( BYTE i = 0; i < MAX_PIECES; i++ ) {
		if( !( listPieces[ i ]->state & PIECE_INPLAY ) )
			continue;

		// check against enemy color
		if( listPieces[ i ]->color == *color )
			continue;

		if( CheckMove( listPieces[ i ], x, y ) )
			return ctrue;
	}

	return cfalse;
}

INLINE BYTE MoveKing( Piece_t *piece, const BYTE *xdest, const BYTE *ydest )
{
	BYTE *x, *y;
	x = &piece->xpos;
	y = &piece->ypos;

	if( *xdest >= ( *x - 1 ) && *xdest <= ( *x + 1 ) && *ydest >= ( *y - 1 ) && *ydest <= ( *y + 1 ) ) 
		return MovePiece( piece, xdest, ydest );

	// To castle:
	
	// king's side
	if( *xdest == ( *x + 2 ) && *ydest == *y && ( piece->state & PIECE_ISINITIAL ) && ( ( piece->skinID == WHITE_KING && ( listPieces[ 16 ]->state & PIECE_ISINITIAL ) ) || ( piece->skinID == BLACK_KING && ( listPieces[ 18 ]->state & PIECE_ISINITIAL ) ) ) ) {
		if( SquareChecked( *x, *y, &piece->color ) )
			return cfalse;

		for( BYTE i = 1; i <= 2; i++ ) {
			if( !SquareFree( *x + i, *y ) || SquareChecked( *x + i, *y, &piece->color ) ) {
				return cfalse;
			}
		}
		return ctrue;
	}

	// queen's side
	if( *xdest == ( *x - 2 ) && *ydest == *y && ( piece->state & PIECE_ISINITIAL ) && ( ( piece->skinID == WHITE_KING && ( listPieces[ 17 ]->state & PIECE_ISINITIAL ) ) || ( piece->skinID == BLACK_KING && ( listPieces[ 19 ]->state & PIECE_ISINITIAL ) ) ) ) {
		if( SquareChecked( *x, *y, &piece->color ) )
			return cfalse;

		for( BYTE i = 1; i <= 3; i++ ) {
			if( !SquareFree( *x - i, *y ) || SquareChecked( *x - i, *y, &piece->color ) ) {
				return cfalse;
			}
		}

		return ctrue;
	}

	return cfalse;
}

INLINE BYTE KingCheck( const BYTE *color )
{
	Piece_t *king;

	// get king of the same colour
	for( BYTE i = 0; i < MAX_PIECES; i++ ) {
		if( !( listPieces[ i ]->state & PIECE_INPLAY ) )
			continue;

		if( *color == COLOR_WHITE && listPieces[ i ]->skinID == WHITE_KING ) {
			king = listPieces[ i ];
			break;
		}
		
		if( *color == COLOR_BLACK && listPieces[ i ]->skinID == BLACK_KING ) {
			king = listPieces[ i ];
			break;
		}
	}

	// see if enemy pieces can reach king
	for( BYTE i = 0; i < MAX_PIECES; i++ ) {
		if( !( listPieces[ i ]->state & PIECE_INPLAY ) )
			continue;
		
		if( listPieces[ i ]->color == *color )
			continue;

		if( CheckMove( listPieces[ i ], king->xpos, king->ypos ) )
			return ctrue;	
	}
	
	return cfalse;
}

// run post-move
INLINE BYTE KingCheckMate( const BYTE color )
{
	Piece_t *king;

	if( color == COLOR_WHITE ) 
		king = listPieces[ 30 ];
	else 
		king = listPieces[ 31 ];

	if( !KingCheck( &color ) ) 
		return cfalse;

	if( CheckMove( king, king->xpos - 1, king->ypos - 1 ) ) { 
		if( !KingCheckSimulate( king, king->xpos - 1, king->ypos - 1 ) ) {
			return cfalse;
		}
	}

	if( CheckMove( king, king->xpos, king->ypos - 1 ) ) { 
		if( !KingCheckSimulate( king, king->xpos, king->ypos - 1 ) ) {
			return cfalse;
		}
	}

	if( CheckMove( king, king->xpos + 1, king->ypos - 1 ) ) { 
		if( !KingCheckSimulate( king, king->xpos + 1, king->ypos - 1 ) ) {
			return cfalse;
		}
	}

	if( CheckMove( king, king->xpos - 1, king->ypos ) ) { 
		if( !KingCheckSimulate( king, king->xpos - 1, king->ypos ) ) {
			return cfalse;
		}
	}

	if( CheckMove( king, king->xpos + 1, king->ypos ) ) { 
		if( !KingCheckSimulate( king, king->xpos + 1, king->ypos ) ) {
			return cfalse;
		}
	}

	if( CheckMove( king, king->xpos - 1, king->ypos + 1 ) ) { 
		if( !KingCheckSimulate( king, king->xpos - 1, king->ypos + 1 ) ) {
			return cfalse;
		}
	}

	if( CheckMove( king, king->xpos, king->ypos + 1 ) ) { 
		if( !KingCheckSimulate( king, king->xpos, king->ypos + 1 ) ) {
			return cfalse;
		}
	}

	if( CheckMove( king, king->xpos + 1, king->ypos + 1 ) ) { 
		if( !KingCheckSimulate( king, king->xpos + 1, king->ypos + 1 ) ) {
			return cfalse;
		}
	}

	// if still checked now, we need to:
	// 1. capture the piece that checks AND do not get checked OR
	// 2. move to the square that will block checking AND do not get checked by other piece
	for( BYTE i = 0; i < MAX_PIECES; i++ ) {
		if( !( listPieces[ i ]->state & PIECE_INPLAY ) )
			continue;

		if( listPieces[ i ]->color != color )
			continue;

		for( BYTE x = 0; x < 8; x++ ) {
			for( BYTE y = 0; y < 8; y++ ) {
				if( CheckMove( listPieces[ i ], x, y ) ) {
					if( !KingCheckSimulate( listPieces[ i ], x, y ) ) {
#ifdef _DEBUG_GAME
						char buf[ 0x40 ];
						sprintf( buf, "KingCheckMate: piece %d can prevent checkmate", i );
						LogMessage( LOG_NOTICE, buf );
#endif
						return cfalse;
					}
				}
			}
		}
	}

	return ctrue;
}

INLINE BYTE KingCheckSimulate( Piece_t *piece, const BYTE xdest, const BYTE ydest ){
	BYTE tx, ty;
	Piece_t *tPiece = NULL;
	BYTE checkedSquare = cfalse;

	tx = piece->xpos;
	ty = piece->ypos;

	// capture piece temporarily
	for( BYTE i = 0; i < MAX_PIECES; i++ ) {
		if( !( listPieces[ i ]->state & PIECE_INPLAY ) )
			continue;

		if( listPieces[ i ]->xpos == xdest && listPieces[ i ]->ypos == ydest && listPieces[ i ]->color != piece->color ) {
			tPiece = listPieces[ i ];
			tPiece->state ^= PIECE_INPLAY;	
			break;
		}
	}

	// move it temporarirly	
	piece->xpos = xdest;
	piece->ypos = ydest;	

	if( KingCheck( &piece->color ) )
		checkedSquare = ctrue;

	// restore
	if( tPiece != NULL ) 
		tPiece->state ^= PIECE_INPLAY;
	piece->xpos = tx;
	piece->ypos = ty;
	
	if( checkedSquare )
		return ctrue;

	return cfalse;	
}

INLINE BYTE MovePiece( Piece_t *piece, const BYTE *xdest, const BYTE *ydest )
{
	BYTE pawnFlag = cfalse;

	for( BYTE i = 0; i < MAX_PIECES; i++ ) {
		// inplay only
		if( !( listPieces[ i ]->state & PIECE_INPLAY ) )
			continue;

		// ommit itself
		if( listPieces[ i ]->ID == piece->ID )
			continue;

		// capture state - same color
		if( listPieces[ i ]->xpos == *xdest && listPieces[ i ]->ypos == *ydest && listPieces[ i ]->color == piece->color ) 
			return cfalse;

		// pawn cannot capture a piece at its own X
		if( listPieces[ i ]->xpos == *xdest && listPieces[ i ]->ypos == *ydest && ( piece->skinID == BLACK_PAWN || piece->skinID == WHITE_PAWN ) && piece->xpos == *xdest )
			return cfalse; 

		// pawn can only capture diagonally	
		if( ( piece->skinID == BLACK_PAWN || piece->skinID == WHITE_PAWN ) && piece->xpos != *xdest && listPieces[ i ]->xpos == *xdest && listPieces[ i ]->ypos == *ydest ) 
			pawnFlag = ctrue; 
	}

	// en passant 
	if( EnPassant( piece, xdest, ydest ) )
		pawnFlag = ctrue;

	// pawn cannot move diagonally	
	if( ( piece->skinID == BLACK_PAWN || piece->skinID == WHITE_PAWN ) && piece->xpos != *xdest && !pawnFlag )
		return cfalse; 

	// pawn initial move by 2
	if( ( piece->skinID == BLACK_PAWN && *ydest == piece->ypos + 2 ) || ( piece->skinID == WHITE_PAWN && *ydest == piece->ypos - 2 ) ) {
		for( BYTE i = 0; i < MAX_PIECES; i++ ) {
			// inplay only
			if( !( listPieces[ i ]->state & PIECE_INPLAY ) )
				continue;

			// ommit itself
			if( listPieces[ i ]->ID == piece->ID )
				continue;

			// obstruction?
			if( listPieces[ i ]->xpos == *xdest && listPieces[ i ]->ypos == ( piece->ypos + 1 ) && piece->skinID == BLACK_PAWN )
				return cfalse;

			// obstruction?
			if( listPieces[ i ]->xpos == *xdest && listPieces[ i ]->ypos == ( piece->ypos - 1 ) && piece->skinID == WHITE_PAWN )
				return cfalse;
		}
	}

	if( pawnFlag )
		return pawnFlag;

	return ctrue;
}

INLINE BYTE MovePieceIter( const BYTE *j, const BYTE *k, const BYTE *xdest, const BYTE *ydest, Piece_t *piece )
{
	for( BYTE i = 0; i < MAX_PIECES; i++ ) {		
		// there is another piece between origin and destination
		if( listPieces[ i ]->xpos == *j && listPieces[ i ]->ypos == *k && ( *j != *xdest || *k != *ydest ) && piece->ID != listPieces[ i ]->ID && ( listPieces[ i ]->state & PIECE_INPLAY ) ) {
			return 0; // return cfalse
		}

		// we've reached the destination and there is no piece obstructing the path
		if( *j == *xdest && *k == *ydest ) {
			if( MovePiece( piece, xdest, ydest ) )
				return 1;	// return ctrue
			else
				return 0;	// return cfalse;
		}
	}
	return 2;	// continue
}

INLINE BYTE MovePawn( Piece_t *piece, const BYTE *xdest, const BYTE *ydest )
{
	// white pawn regular move + capture
	if( *xdest >= ( piece->xpos - 1 ) && *xdest <= ( piece->xpos + 1 ) && ( piece->ypos - 1 ) == *ydest && piece->color == COLOR_WHITE )
		return MovePiece( piece, xdest, ydest );

	// white pawn initial move by 2
	if( piece->xpos == *xdest && ( piece->ypos - 2 ) == *ydest && ( piece->state & PIECE_ISINITIAL ) && piece->color == COLOR_WHITE )
		return MovePiece( piece, xdest, ydest );

	// black pawn regular move + capture
	if( *xdest >= ( piece->xpos - 1 ) && *xdest <= ( piece->xpos + 1 ) && ( piece->ypos + 1 ) == *ydest && piece->color == COLOR_BLACK )
		return MovePiece( piece, xdest, ydest );

	// black pawn initial move by 2
	if( piece->xpos == *xdest && ( piece->ypos + 2 ) == *ydest && ( piece->state & PIECE_ISINITIAL ) && piece->color == COLOR_BLACK )
		return MovePiece( piece, xdest, ydest );

	return cfalse;
}

INLINE BYTE MoveQueen( Piece_t *piece, const BYTE *xdest, const BYTE *ydest )
{
	if( ( *xdest == piece->xpos && *ydest != piece->ypos ) || ( *xdest != piece->xpos && *ydest == piece->ypos ) )
		return MoveRook( piece, xdest, ydest );
	else
		return MoveBishop( piece, xdest, ydest );

	return cfalse;
}

INLINE BYTE MoveBishop( Piece_t *piece, const BYTE *xdest, const BYTE *ydest )
{
	BYTE j, k;
	j = piece->xpos;
	k = piece->ypos;

	if( piece->xpos < *xdest && piece->ypos > *ydest ) {
		for( ; j < 8 && j >= 0 && k < 8 && k >= 0; j++, k-- ) {
			switch( MovePieceIter( &j, &k, xdest, ydest, piece ) ) { 
				case 0:	
					return cfalse;
					break;
				case 1:
					return ctrue;				
					break;
				case 2:
					continue;
					break;
			}
		}
	}
	else if( piece->xpos > *xdest && piece->ypos > *ydest ) {
		for( ; j < 8 && j >= 0 && k < 8 && k >= 0; j--, k-- ) {
			switch( MovePieceIter( &j, &k, xdest, ydest, piece ) ) {
				case 0:	
					return cfalse;
					break;
				case 1:
					return ctrue;				
					break;
				case 2:
					continue;
					break;
			}
		}
	}
	else if( piece->xpos < *xdest && piece->ypos < *ydest ) {
		for( ; j < 8 && j >= 0 && k < 8 && k >= 0; j++, k++ ) {
			switch( MovePieceIter( &j, &k, xdest, ydest, piece ) ) {
				case 0:	
					return cfalse;
					break;
				case 1:
					return ctrue;				
					break;
				case 2:
					continue;
					break;
			}
		}
	}
	else if( piece->xpos > *xdest && piece->ypos < *ydest ) {
		for( ; j < 8 && j >= 0 && k < 8 && k >= 0; j--, k++ ) {
			switch( MovePieceIter( &j, &k, xdest, ydest, piece ) ) {
				case 0:	
					return cfalse;
					break;
				case 1:
					return ctrue;				
					break;
				case 2:
					continue;
					break;
			}
		}
	}

	return cfalse;
}

INLINE BYTE MoveRook( Piece_t *piece, const BYTE *xdest, const BYTE *ydest )
{
	BYTE j, k;
	j = piece->xpos;
	k = piece->ypos;

	if( piece->xpos == *xdest && piece->ypos > *ydest ) {
		for( ; k < 8 && k >= 0; k-- ) {
			switch( MovePieceIter( &j, &k, xdest, ydest, piece ) ) {
				case 0:	
					return cfalse;
					break;
				case 1:
					return ctrue;				
					break;
				case 2:
					continue;
					break;
			}

		}
	}
	else if( piece->xpos == *xdest && piece->ypos < *ydest ) {
		for( ; k < 8 && k >= 0; k++ ) {
			switch( MovePieceIter( &j, &k, xdest, ydest, piece ) ) {
				case 0:	
					return cfalse;
					break;
				case 1:
					return ctrue;				
					break;
				case 2:
					continue;
					break;
			}
		}
	}
	else if( piece->xpos > *xdest && piece->ypos == *ydest ) {
		for( ; j < 8 && j >= 0; j-- ) {
			switch( MovePieceIter( &j, &k, xdest, ydest, piece ) ) {
				case 0:	
					return cfalse;
					break;
				case 1:
					return ctrue;				
					break;
				case 2:
					continue;
					break;
			}

		}
	}
	else if( piece->xpos < *xdest && piece->ypos == *ydest ) {
		for( ; j < 8 && j >= 0; j++ ) {
			switch( MovePieceIter( &j, &k, xdest, ydest, piece ) ) {
				case 0:	
					return cfalse;
					break;
				case 1:
					return ctrue;				
					break;
				case 2:
					continue;
					break;
			}
		}
	}

	return cfalse;
}

INLINE BYTE MoveKnight( Piece_t *piece, const BYTE *xdest, const BYTE *ydest )
{
	BYTE *x, *y;
	x = &piece->xpos;
	y = &piece->ypos;

	if( *x == ( *xdest - 2 ) && *y == ( *ydest - 1 ) )
		return MovePiece( piece, xdest, ydest );
	if( *x == ( *xdest - 2 ) && *y == ( *ydest + 1 ) )
		return MovePiece( piece, xdest, ydest );
	if( *x == ( *xdest - 1 ) && *y == ( *ydest + 2 ) )
		return MovePiece( piece, xdest, ydest );
	if( *x == ( *xdest + 1 ) && *y == ( *ydest + 2 ) )
		return MovePiece( piece, xdest, ydest );
	if( *x == ( *xdest + 2 ) && *y == ( *ydest - 1 ) )
		return MovePiece( piece, xdest, ydest );
	if( *x == ( *xdest + 2 ) && *y == ( *ydest + 1 ) )
		return MovePiece( piece, xdest, ydest  );
	if( *x == ( *xdest + 1 ) && *y == ( *ydest - 2 ) )
		return MovePiece( piece, xdest, ydest );
	if( *x == ( *xdest - 1 ) && *y == ( *ydest - 2 ) )
		return MovePiece( piece, xdest, ydest );

	return cfalse;
}

INLINE BYTE EnPassant( Piece_t *piece, const BYTE *xdest, const BYTE *ydest )
{
	if( piece->color == COLOR_BLACK ) {
		if( lastMove.skinID == WHITE_PAWN && lastMove.srcY == 6 && lastMove.destY == 4 && piece->skinID == BLACK_PAWN && ( piece->xpos == lastMove.destX + 1 || piece->xpos == lastMove.destX - 1 ) && piece->ypos == lastMove.destY && *xdest == lastMove.destX && *ydest == ( lastMove.destY + 1 ) ) 
			return ctrue;
	} else {
		if( lastMove.skinID == BLACK_PAWN && lastMove.srcY == 1 && lastMove.destY == 3 && piece->skinID == WHITE_PAWN && ( piece->xpos == lastMove.destX + 1 || piece->xpos == lastMove.destX - 1 ) && piece->ypos == lastMove.destY && *xdest == lastMove.destX && *ydest == ( lastMove.destY - 1 ) ) 
			return ctrue;
	}

	return cfalse;
}

INLINE void FinalMovePiece( Piece_t *piece, const BYTE *xdest, const BYTE *ydest )
{
	if( EnPassant( piece, xdest, ydest ) ) {
		for( BYTE i = 0; i < MAX_PIECES; i++ ) {
			if( !( listPieces[ i ]->state & PIECE_INPLAY ) )
				continue;

			if( listPieces[ i ]->xpos == *xdest && listPieces[ i ]->ypos == piece->ypos ) {
				listPieces[ i ]->state ^= PIECE_INPLAY;
			}
		}		
	}

	// copy them
	lastMove.skinID = piece->skinID;
	lastMove.srcX = piece->xpos;
	lastMove.srcY = piece->ypos;
	lastMove.destX = *xdest;
	lastMove.destY = *ydest;

	// castle king's side
	if( *xdest == ( piece->xpos + 2 ) && *ydest == piece->ypos && ( piece->state & PIECE_ISINITIAL ) && ( piece->skinID == BLACK_KING || piece->skinID == WHITE_KING ) ) {
		if( piece->skinID == WHITE_KING ) {
			listPieces[ 16 ]->xpos = piece->xpos + 1;
			listPieces[ 16 ]->state ^= PIECE_ISINITIAL;
		}

		if( piece->skinID == BLACK_KING ) {
			listPieces[ 18 ]->xpos = piece->xpos + 1;
			listPieces[ 18 ]->state = PIECE_ISINITIAL;
		}

		piece->xpos = *xdest;	
		return;
	}

	// cast queen's side
	if( *xdest == ( piece->xpos - 2 ) && *ydest == piece->ypos && ( piece->state & PIECE_ISINITIAL ) && ( piece->skinID == BLACK_KING || piece->skinID == WHITE_KING ) ) {
		if( piece->skinID == WHITE_KING ) {
			listPieces[ 17 ]->xpos = piece->xpos - 1;
			listPieces[ 17 ]->state ^= PIECE_ISINITIAL;
		}

		if( piece->skinID == BLACK_KING ) {
			listPieces[ 19 ]->xpos = piece->xpos - 1;
			listPieces[ 19 ]->state ^= PIECE_ISINITIAL;
		}

		piece->xpos = *xdest;	
		piece->state ^= PIECE_ISINITIAL;
		return;
	}

	// turn pawn into queen
	if( ( piece->skinID == BLACK_PAWN && piece->ypos == 6 && *ydest == 7 ) || ( piece->skinID == WHITE_PAWN  && piece->ypos == 1 && *ydest == 0 ) ) {
#ifdef _DEBUG_GAME
		LogMessage( LOG_NOTICE, "turn pawn into queen" );
#endif

		if( piece->skinID == WHITE_PAWN ) {
			piece->skinID = WHITE_QUEEN;
		} else if ( piece->skinID == BLACK_PAWN ) {
			piece->skinID = BLACK_QUEEN;
		}
	}

	for( BYTE i = 0; i < MAX_PIECES; i++ ) {
		if( !( listPieces[ i ]->state & PIECE_INPLAY ) )
			continue;

		if( listPieces[ i ]->xpos == *xdest && listPieces[ i ]->ypos == *ydest && listPieces[ i ]->color != piece->color ) {
#ifdef _DEBUG_GAME
			char buf[ 0x40 ];
			sprintf( buf, "capture state, piece %d", i );
			LogMessage( LOG_NOTICE, buf );
#endif
			listPieces[ i ]->state ^= PIECE_INPLAY;
			piece->xpos = *xdest;
			piece->ypos = *ydest;
			piece->state ^= PIECE_ISINITIAL; // FIXME: this is wrong
		
			return;
		}
	}	

#ifdef _DEBUG_GAME
	LogMessage( LOG_NOTICE, "Move to empty square" );
#endif

	piece->xpos = *xdest;
	piece->ypos = *ydest;
	piece->state ^= PIECE_ISINITIAL; // FIXME: this is wrong
}	

INLINE BYTE ClientMovePiece( const BYTE *xsrc, const BYTE *ysrc, const BYTE *x, const BYTE *y, BYTE *piece, const BYTE color )
{
	for( BYTE i = 0; i < MAX_PIECES; i++ ) {
		if( listPieces[ i ]->xpos == *xsrc && listPieces[ i ]->ypos == *ysrc && ( listPieces[ i ]->state & PIECE_INPLAY ) && listPieces[ i ]->color == color ) {
			if( CheckMove( listPieces[ i ], *x, *y ) ) {
				if( !KingCheckSimulate( listPieces[ i ], *x, *y ) ) {
					FinalMovePiece( listPieces[ i ], x, y );
					*piece = i;

					if( listPieces[ i ]->color == COLOR_WHITE ) 
						if( KingCheckMate( COLOR_BLACK ) ) 
							return 2;
					else 
						if( KingCheckMate( COLOR_WHITE ) ) 
							return 3;
					
					return 1;
				}
			}
		}
	}

	return 0;
}
