#include "common.h"
#include "chess.h"

Move_t lastMove; // FIXME: this is wrong
Pieces_t *listPieces;

void InitPieces( Pieces_t *pieces ) 
{
	// 32 pointers to pieces
	listPieces = pieces;
	int i = 0;

	// white Pawns
	for( ; i < 8; i++ )
	{
		AddPiece( listPieces, i, i, 6, WHITE_PAWN, COLOR_WHITE );
	}

	// black Pawns
	for( ; i < 16; i++ )
	{
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

void DestroyPieces()
{
	for( int i = 0; i < 32; i++ ) {
		if( listPieces[ i ] != NULL ) 
			free( listPieces[ i ] );	
	}

	free( listPieces );
}

int CheckMove( Piece_t *piece, const int xdest, const int ydest )
{
	if( xdest < 0 || xdest > 7 || ydest < 0 || ydest > 7 )
		return cfalse;

	if( piece->xpos == xdest && piece->ypos == ydest )
		return cfalse;

	switch( piece->skinID )
	{
		case WHITE_KNIGHT:
		case BLACK_KNIGHT:
			return MoveKnight( piece, xdest, ydest );
			break;

		case WHITE_BISHOP:
		case BLACK_BISHOP:
			return MoveBishop( piece, xdest, ydest );	
			break;
		
		case WHITE_ROOK:
		case BLACK_ROOK:
			return MoveRook( piece, xdest, ydest );	
			break;

		case WHITE_QUEEN:
		case BLACK_QUEEN:
			return MoveQueen( piece, xdest, ydest );
			break;
		case WHITE_KING:
		case BLACK_KING:
			return MoveKing( piece, xdest, ydest );
			break;

		case WHITE_PAWN:
		case BLACK_PAWN:
			return MovePawn( piece, xdest, ydest );
			break;

		default:
			printf( "not selected\n" );
			return cfalse;
			break;
	}
}

int SquareFree( const int x, const int y )
{
	for( int i = 0; i < 32; i++ ) {
		if( !( listPieces[ i ]->state & PIECE_INPLAY ) )
			continue;

		if( x == listPieces[ i ]->xpos && y == listPieces[ i ]->ypos )
			return cfalse;
	}

	return ctrue;
}

int SquareChecked( const int x, const int y, const int color )
{
	for( int i = 0; i < 32; i++ ) {
		if( !( listPieces[ i ]->state & PIECE_INPLAY ) )
			continue;

		// check against enemy color
		if( listPieces[ i ]->color == color )
			continue;

		if( CheckMove( listPieces[ i ], x, y ) )
			return ctrue;
	}

	return cfalse;
}

int MoveKing( Piece_t *piece, const int xdest, const int ydest )
{
	char *x, *y;
	x = &piece->xpos;
	y = &piece->ypos;

	if( xdest >= ( *x - 1 ) && xdest <= ( *x + 1 ) && ydest >= ( *y - 1 ) && ydest <= ( *y + 1 ) ) 
		return MovePiece( piece, xdest, ydest );

	// To castle:
	
	// king's side
	if( xdest == ( *x + 2 ) && ydest == *y && ( piece->state & PIECE_ISINITIAL ) && ( ( piece->skinID == WHITE_KING && ( listPieces[ 16 ]->state & PIECE_ISINITIAL ) ) || ( piece->skinID == BLACK_KING && ( listPieces[ 18 ]->state & PIECE_ISINITIAL ) ) ) ) {
		if( SquareChecked( *x, *y, piece->color ) )
			return cfalse;

		for( int i = 1; i <= 2; i++ ) {
			if( !SquareFree( *x + i, *y ) || SquareChecked( *x + i, *y, piece->color ) ) {
				printf( "position checked x %d y %d \n", *x + i, *y  );
				return cfalse;
			}
		}
		printf( "castle king's side\n" );
		return ctrue;
	}

	// queen's side
	if( xdest == ( *x - 2 ) && ydest == *y && ( piece->state & PIECE_ISINITIAL ) && ( ( piece->skinID == WHITE_KING && ( listPieces[ 17 ]->state & PIECE_ISINITIAL ) ) || ( piece->skinID == BLACK_KING && ( listPieces[ 19 ]->state & PIECE_ISINITIAL ) ) ) ) {
		if( SquareChecked( *x, *y, piece->color ) )
			return cfalse;

		for( int i = 1; i <= 3; i++ ) {
			if( !SquareFree( *x - i, *y ) || SquareChecked( *x - i, *y, piece->color ) ) {
				printf( "position checked x %d y %d \n", *x - i, *y  );
				return cfalse;
			}
		}

		printf( "castle queen's side\n" );
		return ctrue;
	}

	return cfalse;
}

int KingCheck( const int color )
{
	Piece_t *king;

	// get king of the same colour
	for( int i = 0; i < 32; i++ ) {
		if( !( listPieces[ i ]->state & PIECE_INPLAY ) )
			continue;

		if( color == COLOR_WHITE && listPieces[ i ]->skinID == WHITE_KING ) {
			king = listPieces[ i ];
			break;
		}
		
		if( color == COLOR_BLACK && listPieces[ i ]->skinID == BLACK_KING ) {
			king = listPieces[ i ];
			break;
		}
	}

	// see if enemy pieces can reach king
	for( int i = 0; i < 32; i++ ) {
		if( !( listPieces[ i ]->state & PIECE_INPLAY ) )
			continue;
		
		if( listPieces[ i ]->color == color )
			continue;

		if( CheckMove( listPieces[ i ], king->xpos, king->ypos ) )
			return ctrue;	
	}
	
	return cfalse;
}

// run post-move
int KingCheckMate( const int color )
{
	Piece_t *king;

	if( color == COLOR_WHITE ) {
		king = listPieces[ 30 ];
		printf("white king  \n");
	}
	else {
		king = listPieces[ 31 ];
		printf("black king  \n");
	}

	if( !KingCheck( color ) ) {
		printf("king not checked color: %d \n", color );
		return cfalse;
	}

	printf("king checked 3 \n");
	
	if( CheckMove( king, king->xpos - 1, king->ypos - 1 ) ) { 
		if( !KingCheckSimulate( king, king->xpos - 1, king->ypos - 1 ) ) {
			return cfalse;
		}
	}

	printf("king checked 4 \n");

	if( CheckMove( king, king->xpos, king->ypos - 1 ) ) { 
		if( !KingCheckSimulate( king, king->xpos, king->ypos - 1 ) ) {
			return cfalse;
		}
	}

	printf("king checked 5 \n");

	if( CheckMove( king, king->xpos + 1, king->ypos - 1 ) ) { 
		if( !KingCheckSimulate( king, king->xpos + 1, king->ypos - 1 ) ) {
			return cfalse;
		}
	}

	printf("king checked 6 \n");

	if( CheckMove( king, king->xpos - 1, king->ypos ) ) { 
		if( !KingCheckSimulate( king, king->xpos - 1, king->ypos ) ) {
			return cfalse;
		}
	}

	printf("king checked 7 \n");

	if( CheckMove( king, king->xpos + 1, king->ypos ) ) { 
		if( !KingCheckSimulate( king, king->xpos + 1, king->ypos ) ) {
			return cfalse;
		}
	}

	printf("king checked 8 \n");

	if( CheckMove( king, king->xpos - 1, king->ypos + 1 ) ) { 
		if( !KingCheckSimulate( king, king->xpos - 1, king->ypos + 1 ) ) {
			return cfalse;
		}
	}

	printf("king checked 9 \n");

	if( CheckMove( king, king->xpos, king->ypos + 1 ) ) { 
		if( !KingCheckSimulate( king, king->xpos, king->ypos + 1 ) ) {
			return cfalse;
		}
	}

	printf("king checked 10 \n");

	if( CheckMove( king, king->xpos + 1, king->ypos + 1 ) ) { 
		if( !KingCheckSimulate( king, king->xpos + 1, king->ypos + 1 ) ) {
			return cfalse;
		}
	}

	printf("king checked 11 \n");

	// if still checked now, we need to:
	// 1. capture the piece that checks AND do not get checked OR
	// 2. move to the square that will block checking AND do not get checked by other piece
	for( int i = 0; i < 32; i++ ) {
		if( !( listPieces[ i ]->state & PIECE_INPLAY ) )
			continue;

		if( listPieces[ i ]->color != color )
			continue;

		for( int x = 0; x < 8; x++ ) {
			for( int y = 0; y < 8; y++ ) {
				if( CheckMove( listPieces[ i ], x, y ) ) {
					if( !KingCheckSimulate( listPieces[ i ], x, y ) ) {
						printf( "piece %d can prevent check \n", i );
						return cfalse;
					}
				}
			}
		}
	}

	return ctrue;
}

int KingCheckSimulate( Piece_t *piece, const int xdest, const int ydest )
{
	int tx, ty;
	Piece_t *tPiece = NULL;
	int checkedSquare = cfalse;

	tx = piece->xpos;
	ty = piece->ypos;

	// capture piece temporarily
	for( int i = 0; i < 32; i++ ) {
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

	if( KingCheck( piece->color ) )
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

int MovePiece( Piece_t *piece, const int xdest, const int ydest )
{
	int pawnFlag = cfalse;

	for( int i = 0; i < 32; i++ ) {
		// inplay only
		if( !( listPieces[ i ]->state & PIECE_INPLAY ) )
			continue;

		// ommit itself
		if( listPieces[ i ]->ID == piece->ID )
			continue;

		// capture state - same color
		if( listPieces[ i ]->xpos == xdest && listPieces[ i ]->ypos == ydest && listPieces[ i ]->color == piece->color ) 
			return cfalse;

		// pawn cannot capture a piece at its own X
		if( listPieces[ i ]->xpos == xdest && listPieces[ i ]->ypos == ydest && ( piece->skinID == BLACK_PAWN || piece->skinID == WHITE_PAWN ) && piece->xpos == xdest )
			return cfalse; 

		// pawn can only capture diagonally	
		if( ( piece->skinID == BLACK_PAWN || piece->skinID == WHITE_PAWN ) && piece->xpos != xdest && listPieces[ i ]->xpos == xdest && listPieces[ i ]->ypos == ydest ) 
			pawnFlag = ctrue; 
	}

	// en passant 
	if( EnPassant( piece, xdest, ydest ) )
		pawnFlag = ctrue;

	// pawn cannot move diagonally	
	if( ( piece->skinID == BLACK_PAWN || piece->skinID == WHITE_PAWN ) && piece->xpos != xdest && !pawnFlag )
		return cfalse; 

	// pawn initial move by 2
	if( ( piece->skinID == BLACK_PAWN && ydest == piece->ypos + 2 ) || ( piece->skinID == WHITE_PAWN && ydest == piece->ypos - 2 ) ) {
		for( int i = 0; i < 32; i++ ) {
			// inplay only
			if( !( listPieces[ i ]->state & PIECE_INPLAY ) )
				continue;

			// ommit itself
			if( listPieces[ i ]->ID == piece->ID )
				continue;

			// obstruction?
			if( listPieces[ i ]->xpos == xdest && listPieces[ i ]->ypos == ( piece->ypos + 1 ) && piece->skinID == BLACK_PAWN )
				return cfalse;

			// obstruction?
			if( listPieces[ i ]->xpos == xdest && listPieces[ i ]->ypos == ( piece->ypos - 1 ) && piece->skinID == WHITE_PAWN )
				return cfalse;
		}
	}

	if( pawnFlag )
		return pawnFlag;

	return ctrue;
}

int MovePieceIter( const int j, const int k, const int xdest, const int ydest, Piece_t *piece )
{
	for( int i = 0; i < 32; i++ )
	{		
		// there is another piece between origin and destination
		if( listPieces[ i ]->xpos == j && listPieces[ i ]->ypos == k && ( j != xdest || k != ydest ) && piece->ID != listPieces[ i ]->ID && ( listPieces[ i ]->state & PIECE_INPLAY ) )
		{
			printf("obstruction x: %d y: %d\n", j, k );
			return 0; // return cfalse
		}

		// we've reached the destination and there is no piece obstructing the path
		if( j == xdest && k == ydest )
		{
			if( MovePiece( piece, xdest, ydest ) )
				return 1;	// return ctrue
			else
				return 0;	// return cfalse;
		}
	}
	return 2;	// continue
}

int MovePawn( Piece_t *piece, const int xdest, const int ydest )
{
	// white pawn regular move + capture
	if( xdest >= ( piece->xpos - 1 ) && xdest <= ( piece->xpos + 1 ) && ( piece->ypos - 1 ) == ydest && piece->color == COLOR_WHITE )
		return MovePiece( piece, xdest, ydest );

	// white pawn initial move by 2
	if( piece->xpos == xdest && ( piece->ypos - 2 ) == ydest && ( piece->state & PIECE_ISINITIAL ) && piece->color == COLOR_WHITE )
		return MovePiece( piece, xdest, ydest );

	// black pawn regular move + capture
	if( xdest >= ( piece->xpos - 1 ) && xdest <= ( piece->xpos + 1 ) && ( piece->ypos + 1 ) == ydest && piece->color == COLOR_BLACK )
		return MovePiece( piece, xdest, ydest );

	// black pawn initial move by 2
	if( piece->xpos == xdest && ( piece->ypos + 2 ) == ydest && ( piece->state & PIECE_ISINITIAL ) && piece->color == COLOR_BLACK )
		return MovePiece( piece, xdest, ydest );

	return cfalse;
}

int MoveQueen( Piece_t *piece, const int xdest, const int ydest )
{
	if( ( xdest == piece->xpos && ydest != piece->ypos ) || ( xdest != piece->xpos && ydest == piece->ypos ) )
		return MoveRook( piece, xdest, ydest );
	else
		return MoveBishop( piece, xdest, ydest );

	return cfalse;
}

int MoveBishop( Piece_t *piece, const int xdest, const int ydest )
{
	int j, k;
	j = piece->xpos;
	k = piece->ypos;

	if( piece->xpos < xdest && piece->ypos > ydest )
	{
		for( ; j < 8 && j >= 0 && k < 8 && k >= 0; j++, k-- )
		{
			switch( MovePieceIter( j, k, xdest, ydest, piece ) ) { 
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
	else if( piece->xpos > xdest && piece->ypos > ydest )
	{
		for( ; j < 8 && j >= 0 && k < 8 && k >= 0; j--, k-- )
		{
			switch( MovePieceIter( j, k, xdest, ydest, piece ) ) {
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
	else if( piece->xpos < xdest && piece->ypos < ydest )
	{
		for( ; j < 8 && j >= 0 && k < 8 && k >= 0; j++, k++ )
		{
			switch( MovePieceIter( j, k, xdest, ydest, piece ) ) {
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
	else if( piece->xpos > xdest && piece->ypos < ydest )
	{
		for( ; j < 8 && j >= 0 && k < 8 && k >= 0; j--, k++ )
		{
			switch( MovePieceIter( j, k, xdest, ydest, piece ) ) {
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

int MoveRook( Piece_t *piece, const int xdest, const int ydest )
{
	int j, k;
	j = piece->xpos;
	k = piece->ypos;

	if( piece->xpos == xdest && piece->ypos > ydest )
	{
		for( ; k < 8 && k >= 0; k-- )
		{
			switch( MovePieceIter( j, k, xdest, ydest, piece ) ) {
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
	else if( piece->xpos == xdest && piece->ypos < ydest )
	{
		for( ; k < 8 && k >= 0; k++ )
		{
			switch( MovePieceIter( j, k, xdest, ydest, piece ) ) {
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
	else if( piece->xpos > xdest && piece->ypos == ydest )
	{
		for( ; j < 8 && j >= 0; j-- )
		{
			switch( MovePieceIter( j, k, xdest, ydest, piece ) ) {
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
	else if( piece->xpos < xdest && piece->ypos == ydest )
	{
		for( ; j < 8 && j >= 0; j++ ) {
			switch( MovePieceIter( j, k, xdest, ydest, piece ) ) {
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

int MoveKnight( Piece_t *piece, const int xdest, const int ydest )
{
	char *x, *y;
	x = &(piece->xpos);
	y = &(piece->ypos);

	if( *x == ( xdest - 2 ) && *y == ( ydest - 1 ) )
		return MovePiece( piece, xdest, ydest );
	if( *x == ( xdest - 2 ) && *y == ( ydest + 1 ) )
		return MovePiece( piece, xdest, ydest );
	if( *x == ( xdest - 1 ) && *y == ( ydest + 2 ) )
		return MovePiece( piece, xdest, ydest );
	if( *x == ( xdest + 1 ) && *y == ( ydest + 2 ) )
		return MovePiece( piece, xdest, ydest );
	if( *x == ( xdest + 2 ) && *y == ( ydest - 1 ) )
		return MovePiece( piece, xdest, ydest );
	if( *x == ( xdest + 2 ) && *y == ( ydest + 1 ) )
		return MovePiece( piece, xdest, ydest  );
	if( *x == ( xdest + 1 ) && *y == ( ydest - 2 ) )
		return MovePiece( piece, xdest, ydest );
	if( *x == ( xdest - 1 ) && *y == ( ydest - 2 ) )
		return MovePiece( piece, xdest, ydest );

	return cfalse;
}

int EnPassant( Piece_t *piece, const int xdest, const int ydest )
{
	if( piece->color == COLOR_BLACK ) {
		if( lastMove.skinID == WHITE_PAWN && lastMove.srcY == 6 && lastMove.destY == 4 && piece->skinID == BLACK_PAWN && ( piece->xpos == lastMove.destX + 1 || piece->xpos == lastMove.destX - 1 ) && piece->ypos == lastMove.destY && xdest == lastMove.destX && ydest == ( lastMove.destY + 1 ) ) 
			return ctrue;
	} else {
		if( lastMove.skinID == BLACK_PAWN && lastMove.srcY == 1 && lastMove.destY == 3 && piece->skinID == WHITE_PAWN && ( piece->xpos == lastMove.destX + 1 || piece->xpos == lastMove.destX - 1 ) && piece->ypos == lastMove.destY && xdest == lastMove.destX && ydest == ( lastMove.destY - 1 ) ) 
			return ctrue;
	}

	return cfalse;
}

void FinalMovePiece( Piece_t *piece, const int xdest, const int ydest )
{
	if( EnPassant( piece, xdest, ydest ) ) {
		for( int i = 0; i < 32; i++ ) {
			if( !( listPieces[ i ]->state & PIECE_INPLAY ) )
				continue;

			if( listPieces[ i ]->xpos == xdest && listPieces[ i ]->ypos == piece->ypos ) {
				listPieces[ i ]->state ^= PIECE_INPLAY;
			}
		}		
	}

	lastMove.skinID = piece->skinID;
	lastMove.srcX = piece->xpos;
	lastMove.srcY = piece->ypos;
	lastMove.destX = xdest;
	lastMove.destY = ydest;

	// castle king's side
	if( xdest == ( piece->xpos + 2 ) && ydest == piece->ypos && ( piece->state & PIECE_ISINITIAL ) && ( piece->skinID == BLACK_KING || piece->skinID == WHITE_KING ) ) {
		if( piece->skinID == WHITE_KING ) {
			listPieces[ 16 ]->xpos = piece->xpos + 1;
//			listPieces[ 16 ]->x = BOARD_SQUARE_WH * ( xdest - 1 );
			listPieces[ 16 ]->state ^= PIECE_ISINITIAL;
		}

		if( piece->skinID == BLACK_KING ) {
			listPieces[ 18 ]->xpos = piece->xpos + 1;
//			listPieces[ 18 ]->x = BOARD_SQUARE_WH * ( xdest - 1 );
			listPieces[ 18 ]->state = PIECE_ISINITIAL;
		}

		piece->xpos = xdest;	
//		piece->x = BOARD_SQUARE_WH * xdest;
		return;
	}

	// cast queen's side
	if( xdest == ( piece->xpos - 2 ) && ydest == piece->ypos && ( piece->state & PIECE_ISINITIAL ) && ( piece->skinID == BLACK_KING || piece->skinID == WHITE_KING ) ) {
		if( piece->skinID == WHITE_KING ) {
			listPieces[ 17 ]->xpos = piece->xpos - 1;
//			listPieces[ 17 ]->x = BOARD_SQUARE_WH * ( xdest + 1 );
			listPieces[ 17 ]->state ^= PIECE_ISINITIAL;
		}

		if( piece->skinID == BLACK_KING ) {
			listPieces[ 19 ]->xpos = piece->xpos - 1;
//			listPieces[ 19 ]->x = BOARD_SQUARE_WH * ( xdest + 1 );
			listPieces[ 19 ]->state ^= PIECE_ISINITIAL;
		}

		piece->xpos = xdest;	
//		piece->x = BOARD_SQUARE_WH * xdest;
		piece->state ^= PIECE_ISINITIAL;
		return;
	}

	// turn pawn into queen
	if( ( piece->skinID == BLACK_PAWN && piece->ypos == 6 && ydest == 7 ) || ( piece->skinID == WHITE_PAWN  && piece->ypos == 1 && ydest == 0 ) ) {
		printf("turn pawn\n");
		if( piece->skinID == WHITE_PAWN ) {
			piece->skinID = WHITE_QUEEN;
		} else if ( piece->skinID == BLACK_PAWN ) {
			piece->skinID = BLACK_QUEEN;
		}
	}

	for( int i = 0; i < 32; i++ ) {
		if( !( listPieces[ i ]->state & PIECE_INPLAY ) )
			continue;

		if( listPieces[ i ]->xpos == xdest && listPieces[ i ]->ypos == ydest && listPieces[ i ]->color != piece->color ) {
			
			printf( "capture state piece: %d \n", listPieces[ i ]->ID );	
			listPieces[ i ]->state ^= PIECE_INPLAY;
			piece->xpos = xdest;
			piece->ypos = ydest;
//			piece->x = BOARD_SQUARE_WH * xdest;
//			piece->y = BOARD_SQUARE_WH * ydest;
			piece->state ^= PIECE_ISINITIAL; // FIXME: this is wrong
		
			return;
		}
	}	

	printf( "empty square\n" );
	piece->xpos = xdest;
	piece->ypos = ydest;
//	piece->x = BOARD_SQUARE_WH * xdest;
//	piece->y = BOARD_SQUARE_WH * ydest;
	piece->state ^= PIECE_ISINITIAL; // FIXME: this is wrong
}	

int ClientMovePiece( const int xsrc, const int ysrc, const int x, const int y, int *piece )
{
	for( int i = 0; i < 32; i++ )
	{
		if( listPieces[ i ]->xpos == xsrc && listPieces[ i ]->ypos == ysrc && ( listPieces[ i ]->state & PIECE_INPLAY ) )
		{
			//listPieces[ i ]->state ^= PIECE_ISSELECTED;		// unselect
			if( CheckMove( listPieces[ i ], x, y ) ) {
				if( !KingCheckSimulate( listPieces[ i ], x, y ) ) {
					FinalMovePiece( listPieces[ i ], x, y );
		
					// see if not checkmate
					if( listPieces[ i ]->color == COLOR_WHITE ) {
						if( KingCheckMate( COLOR_BLACK ) ) {
							// reply to clients
							return 2;
						}
					} else {
						if( KingCheckMate( COLOR_WHITE ) ) {
							// reply to clients
							return 3;
						}
					}
					
					// reply to clients - regular move
					*piece = i;
					return 1;
				}
			}
		}	
	}

	return 0;
}

void AddPiece( Pieces_t *listPieces, const int i, const int x, const int y, const int skin, const int color ) 
{
	listPieces[ i ]->xpos = x;
	listPieces[ i ]->ypos = y;
	listPieces[ i ]->ID = i;
	listPieces[ i ]->skinID = skin;
	listPieces[ i ]->color = color;
	listPieces[ i ]->state = PIECE_INPLAY | PIECE_ISINITIAL | PIECE_INUSE;
}
