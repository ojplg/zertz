#include "board.h"
#include <gnome.h>

/*
 * Prints an ascii Board to the console.
 * For debugging only. 
 */
void print_board( Board* position )
{
	int row;
	int column;
	char display;
	for( row=0 ; row < WIDTH ; row++ ){
		for( column=0 ; column < WIDTH ; column++ ){
			display = decode_square( position -> square[column][row] );
			printf("%c" , display ) ;
		}
		printf("\n");
	}
}

/*
 * Creates the Board by setting all the 
 * elements pointed to by position to 
 * zero and by then setting the squares
 * that are on the Board to one.
 */
void init_board( Board* position )
{
	int side = WIDTH / 2 + 1;
	int row;
	int column;
	int rowStart;
	int rowEnd;

	// set all to 0
	for( row=0 ; row < WIDTH ; row++ ){
		for( column=0 ; column < WIDTH ; column++ ){
			position->square[column][row] = VOID;
		}
	}
	// set Board squares to 1
	for( row=0 ; row < WIDTH ; row++ ){
		rowStart = row - side + 1  > 0 ? row - side + 1 : 0;
		rowEnd = row + side > WIDTH ? WIDTH : row + side;
		for( column= rowStart ; column < rowEnd ; column++ ){
			position->square[column][row] = EMPTY;
		}
	}
	set_removeable_discs( position );
}

/*
 * Makes a copy of the position on the Board.
 */
void copy_board
( Board* orig , Board* copy )
{
	int i, j;	

	for( i=0 ; i<WIDTH ; i++ ){
		for( j=0 ; j<WIDTH ; j++ ){
			copy->square[i][j] = orig->square[i][j];
		}
	}
	
}

/*
 * Finds the empty discs that can be
 * removed and sets them so.
 */ 
void set_removeable_discs
( Board* position )
{
	int row , column ;
	int rowStart , rowEnd;
	int side = WIDTH / 2 + 1;
	
	// set removeable discs to removeable
	for( row=0 ; row < WIDTH ; row++ ){
		rowStart = row - side + 1  > 0 ? row - side + 1 : 0;
		rowEnd = row + side > WIDTH ? WIDTH : row + side;
		for( column= rowStart ; column < rowEnd ; column++ ){
			if( is_removeable( position , column , row ) ){
				position->square[column][row] = REMOVEABLE;
			}
		}
	}

}

/*
 * returns true if any disc is marked
 * removeable.  false otherwise
 */ 
int has_removeable_disc
( Board* position )
{
	int row , column ;
	int rowStart , rowEnd;
	int side = WIDTH / 2 + 1;

	for( row=0 ; row < WIDTH ; row++ ){
		rowStart = row - side + 1  > 0 ? row - side + 1 : 0;
		rowEnd = row + side > WIDTH ? WIDTH : row + side;
		for( column= rowStart ; column < rowEnd ; column++ ){
			if( is_removeable( position , row , column ) ){
				return TRUE;
			}
		}
	}
	return FALSE;
}

/*
 * Returns true if a disc can be removed, else false.
 */ 
int is_removeable( Board* position , int column , int row ){
	int i;
	
	/* First check to make sure the disc is empty */
	if( get_disc_contents( position , column , row ) == EMPTY ||
		  get_disc_contents( position , column , row ) == REMOVEABLE ){ 
		/* if it is, two consecutive Neighbors must be void */
		for( i=0 ; i<6 ; i++ ){
			if( get_neighbor( position , column , row , i ) == VOID && 
					get_neighbor(position , column , row , (i+1)%6 ) == VOID ){
				return TRUE;
			}
		}
	}
	return FALSE;
}

/*
 * returns a character based on the
 * integer value of a square.
 */
char decode_square( int squareContents )
{
	switch (squareContents) {
		case VOID:
			return ' ';
		case EMPTY:
			return '_';
		case REMOVEABLE:
			return '.';	
		case BLACK:
			return 'B';
		case GREY:
			return 'G';
		case WHITE:
			return 'W';
		default:
			return 'e'; //for error
	}
}

/*
 * Puts a marble in place.  Marks the square with the color given.
 */
void place_marble
( int color , Board* position , int column , int row ) 
{
	position->square[column][row]=color;
}

/*
 * Determines if a space has
 * an empty disc in it.
 */
int can_accept_marble
( Board* position , int column , int row )
{
	int foo = get_disc_contents( position , column , row );

	if( row < 0 || row > 6 ){
		return FALSE;
	}
	if( column < 0 || column > 6 ){
		return FALSE;
	}

	if( foo == REMOVEABLE || foo == EMPTY ){
		return TRUE;
	}
	return FALSE;
}


/*
 * Removes a marble.  Returns the color of the marble 
 * being removed.
 */
int remove_marble
( Board* position , int column , int row )
{
	int color = position->square[column][row];
	position->square[column][row]=EMPTY;
	return color;
}

/*
 * Removes a disc.
 */ 
void remove_disc
( Board* position , int column , int row )
{
	position->square[column][row]=VOID;
}

/*
 * Returns the value of the square being queried.
 */
int get_disc_contents
( Board* position , int column , int row )
{
	if( row < 0 || row >= WIDTH ){
		return VOID;
	}
	if( column < 0 || column >= WIDTH ){
		return VOID;
	}
	
	return position->square[column][row];
}

/*
 * Determines if there is a jump available, at
 * the point being queried.
 */
int has_jump
( Board* position , int column , int row )
{
	int i;
	int flag;
	Coordinates point;
	Coordinates coords;
	Neighbors myNeighbors;
	
	if( ! is_marbled( get_disc_contents( position , column , row ) ) ){
		return FALSE;
	}
	
	point.x = column;
	point.y = row;
	get_neighbors( &myNeighbors , position , column , row );
	
	flag = FALSE;

	for( i=0 ; i< 6 ; i++ ){
		if( is_marbled( myNeighbors.place[i] ) ){
			get_neighbor_coordinates( &point , i , &coords );
			if( ( get_neighbor( position , coords.x , coords.y , i ) == EMPTY )
			 || ( get_neighbor( position , coords.x , coords.y , i ) == REMOVEABLE ) ){
				flag = TRUE;
			}
		}
	}

	return flag;
}

/*
 * Determines whether a particular jump can be
 * made.
 */
int can_jump_to
( Board* position , int old_column , int old_row , int new_column , int new_row )
{
	int mid_row, mid_column;
	Coordinates mid_coords;
	Coordinates new_coords , old_coords;

	if( ! has_middle_coordinates( old_column , old_row , new_column , new_row ) ){
		return FALSE;
	}
	
	new_coords.x = new_column;
	new_coords.y = new_row;
	old_coords.x = old_column;
	old_coords.y = old_row;
	
	get_middle_coordinates( &new_coords , &old_coords , &mid_coords );
	mid_column = mid_coords.x;
	mid_row = mid_coords.y;
	
	if( is_marbled( get_disc_contents( position , mid_column , mid_row ) ) ){
	  if( can_accept_marble( position , new_column , new_row ) ){
		  return TRUE;
	  }
	}
	
	return FALSE;  	  
}

/*
 * Determines if two points are seperated by
 * exactly one disc and are in a line.
 */
int has_middle_coordinates
( int col1 , int row1 , int col2 , int row2 )
{
	if( ( ( ( col1 == col2 ) && ( abs( row1 - row2 ) == 2 ) ) ||
		( ( abs( col1 - col2 ) == 2 ) && ( row1 == row2 ) ) ) ||
		( ( abs( col1 - col2 ) == 2 ) && ( abs( row1 - row2 ) == 2 ) ) ){
	  return TRUE;
	}
	return FALSE;	
}

/*
 *	computes the value of the Coordinates between
 * the two between the two passed as a and b
 * initializes the mid to the correct thing
 */
void get_middle_coordinates
( Coordinates* a , Coordinates* b , Coordinates* mid )
{
	int mx;
	int my;
	
	if( a->x < b->x){
		mx = b->x - 1;
	}
	else if( a->x == b->x ){
		mx = a->x;
	}
	else {
		mx = a->x - 1;
	}

	if( a->y < b->y){
		my = b->y - 1;
	}
	else if( a->y == b->y ){
		my = a->y;
	}
	else {
		my = a->y - 1;
	}
	
	mid->x = mx;
	mid->y = my;

}

/*
 * Pass a jump and three sets of coordinates.
 * The coordinates will be set appropriately.
 */ 
void set_coordinates_from_jump
( Jump* jump , Coordinates* middle , Coordinates* start , Coordinates* end )
{
	start->x = jump->start_column;
	start->y = jump->start_row;
	end->x = jump->end_column;
	end->y = jump->end_row;

	get_middle_coordinates( start , end , middle );
	
}

/*
 * Checks the entire Board for a jump.
 */
int jump_available
( Board* position )
{
	int i;
	int j;

	for ( i=0 ; i<WIDTH ; i++ ){
		for ( j=0 ; j<WIDTH ; j++ ){
			if( has_jump( position , i , j ) ){
				return TRUE;
			}
		}
	}
	return FALSE;
}

/*
 * Checks the entire Board for a split.
 */
int split_available
( Board* position )
{
	int i;
	int j;

	for ( i=0 ; i<WIDTH ; i++ ){
		for ( j=0 ; j<WIDTH ; j++ ){
			if( will_cause_split( position , i , j ) ){
				return TRUE;
			}
		}
	}
	return FALSE;
}

/*
 * Looks at the board and attempts
 * to find a disc that will cause 
 * a split.  
 *
 * THIS IS NOT USED.
 *  
 */ 
int find_splitter
( Board* position , Coordinates* splitter )
{
	int i, j;
	
	splitter=NULL;
	for ( i=0 ; i<WIDTH ; i++ ){
		for ( j=0 ; j<WIDTH ; j++ ){
			if( will_cause_split( position , i , j ) ){
				
				return TRUE;
			}
		}
	}
	return FALSE;

}


/*
 * Returns true if a disc has a marble.
 */ 
int is_marbled
( int value )
{
	if( value == WHITE || value == GREY || value == BLACK ){
		return TRUE;
	}
	else {
		return FALSE;
	}
}

/*
 * Returns the contents of the disc at the
 * address (0-5) relative to the row and
 * column passed.
 */
int get_neighbor
( Board* position , int column , int row , int address )
{
	Neighbors myNeighbors;
  	int val;
	
	get_neighbors( &myNeighbors , position , column , row );
	val = myNeighbors.place[address];

	return val;
}

/*
 * Determines the coordnates of a neighbor to a
 * point as indicated by the address passed. 
 */
void get_neighbor_coordinates
( Coordinates* point , int address , Coordinates* neighbor)
{
	int column = point->x;
	int row = point->y;
	
	switch( address ){
		case 0:
			neighbor->x = column - 1;
			neighbor->y = row - 1;
			break;
		case 1:
			neighbor->x = column - 1;
			neighbor->y = row;
			break;
		case 2:
			neighbor->x = column;
			neighbor->y = row + 1;
			break;
		case 3:
			neighbor->x = column + 1;
			neighbor->y = row + 1;
			break;
		case 4:
			neighbor->x = column + 1;
			neighbor->y = row;
			break;
		case 5:
			neighbor->x = column;
			neighbor->y = row - 1;
			break;
		default:
			printf("get_neighbor_coordinates has an error!");
			break;
	}
}

/*
 * Initializes the Neighbors* that is passed as the
 * first argument to the values contained in the
 * six Neighbors of the point indicated by the 
 * column and row.
 */
void get_neighbors
( Neighbors* X , Board* position , int column , int row )
{
	if ( column == 0 && row == 0 ){
		X->place[0] = VOID;
		X->place[1] = VOID;
		X->place[2] = get_disc_contents( position , column   , row+1 );
		X->place[3] = get_disc_contents( position , column+1 , row+1 );
		X->place[4] = get_disc_contents( position , column+1 , row   );
		X->place[5] = VOID;
	}
	else if ( column == 0 && row < WIDTH-1 ){
		X->place[0] = VOID;
		X->place[1] = VOID;
		X->place[2] = get_disc_contents( position , column   , row+1 );
		X->place[3] = get_disc_contents( position , column+1 , row+1 );
		X->place[4] = get_disc_contents( position , column+1 , row   );
		X->place[5] = get_disc_contents( position , column   , row-1 );
	}
	else if (column < WIDTH-1 && row == 0 ) {
		X->place[0] = VOID;
		X->place[1] = get_disc_contents( position , column-1 , row   );
		X->place[2] = get_disc_contents( position , column   , row+1 );
		X->place[3] = get_disc_contents( position , column+1 , row+1 );
		X->place[4] = get_disc_contents( position , column+1 , row   );
		X->place[5] = VOID;
	}
	else if ( column == WIDTH-1 && row < WIDTH-1 ) {
		X->place[0] = get_disc_contents( position , column-1 , row-1 );
		X->place[1] = get_disc_contents( position , column-1 , row   );
		X->place[2] = get_disc_contents( position , column   , row+1 );
		X->place[3] = VOID;
		X->place[4] = VOID;
		X->place[5] = get_disc_contents( position , column   , row-1 );
	}
	else if ( column < WIDTH-1 && row == WIDTH-1 ) {
		X->place[0] = get_disc_contents( position , column-1 , row-1 );
		X->place[1] = get_disc_contents( position , column-1 , row   );
		X->place[2] = VOID;
		X->place[3] = VOID;
		X->place[4] = get_disc_contents( position , column+1 , row   );
		X->place[5] = get_disc_contents( position , column   , row-1 );
	}
	else if ( column == WIDTH-1 && row == WIDTH-1 ) {
		X->place[0] = get_disc_contents( position , column-1 , row-1 );
		X->place[1] = get_disc_contents( position , column-1 , row   );
		X->place[2] = VOID;
		X->place[3] = VOID;
		X->place[4] = VOID;
		X->place[5] = get_disc_contents( position , column   , row-1 );
	}
	else {
		X->place[0] = get_disc_contents( position , column-1 , row-1 );
		X->place[1] = get_disc_contents( position , column-1 , row   );
		X->place[2] = get_disc_contents( position , column   , row+1 );
		X->place[3] = get_disc_contents( position , column+1 , row+1 );
		X->place[4] = get_disc_contents( position , column+1 , row   );
		X->place[5] = get_disc_contents( position , column   , row-1 );
	}
}

/*
 * Determines whether or not removing
 * a particular disc will cause a split
 * of the Board into two islands.
 */
int will_cause_split
( Board* position , int column , int row )
{
	/* 
	 *	the idea is to loop through the Neighbors,
	 *	and if the Neighbors change from VOID to not VOID
	 *	and back again, it will cause two islands to 
	 *	be formed.
	 */	
	int switches = 0;
	int prevState = -1;
	int currentState = -2;
	int i;
	
	for( i=0 ; i<6 ; i++ ){
		currentState = get_neighbor( position , column , row , i );
		// all we care about is the difference between VOID and not VOID
		if( currentState != VOID ){
			currentState = 1;
		}
		if ( currentState != prevState ){
			switches++;
		}
		prevState = currentState;
	}
	
	if ( switches >= 4 ){
		return TRUE;
	}

	return FALSE;
}

/*
 * The idea here is two find
 * a disc on each side of the split.
 * The removed disc is at the coordinate
 * given by column and row.
 * The pointers are to the Coordinates
 * that will be filled in for each side of
 * the split.
 */
void set_split_neighbors
( Board* position , int column , int row , int* one_col , int* one_row , int* two_col , int* two_row )
{
	int i , j;
	// find the (possibly three) Neighbors
	int neighbor[3] ;
	Coordinates disc;
	Coordinates a_neighbor;

	disc.x = column;
	disc.y = row;
	
	// find the Neighbors that have discs
	j = 0;
	for( i=0 ; i<6 ; i++ ){
		if( get_neighbor( position , column , row , i ) != VOID ){
			neighbor[j] = i;
			if( j < 2 )/*this should never fail, but who wants a segfault? */ {
				j++;
			}
		}
	}

	//set the first neighbor Coordinates
	get_neighbor_coordinates( &disc , neighbor[0] , &a_neighbor );
	*one_col = a_neighbor.x;
	*one_row = a_neighbor.y;
	// if the second neighbor is next to the first, ditch it, else use it
	if( neighbor[0] == neighbor[1] - 1 ){
		get_neighbor_coordinates( &disc , neighbor[2] , &a_neighbor ) ;
		*two_col = a_neighbor.x;
		*two_row = a_neighbor.y;
	}
	else {
		get_neighbor_coordinates( &disc , neighbor[1] , &a_neighbor ) ;
		*two_col = a_neighbor.x;
		*two_row = a_neighbor.y;
	}

}
 
/* 
 * Returns TRUE if the island is alive 
 * ie, contains empty discs, or FALSE 
 * otherwise.
 * Very unsophisticated: no error checking.
 */
int is_alive
( Island* isle )
{
	if( isle->numberFilled == isle->numberDiscs ){
		return FALSE;
	}
	else {
		return TRUE;
	}
}

/*
 * Determines whether a particular point
 * is in the island or not.
 */
int is_in_island
( Island* isle , int column , int row )
{
	return isle->map->square[column][row];
}

/*
 * This function sets all the points in the 
 * Board* island to either 0 or 1.  If the point
 * is in the "island" containing the Coordinates
 * base, then it will be set to 1, else to 0.
 *
 * Reading this function can cause confusion --- BEWARE!
 */
void set_island
( Board* position , Island* isle , int column , int row )
{

	int i , j;
	int side = WIDTH / 2 + 1;
	int row_start , row_end;
	int test_column, test_row;
	Coordinates test_coordinates;
   Coordinates tmp_coordinates;	
	
	isle->numberDiscs = 0;
	isle->numberFilled = 0;
	
	isle->map = (Board*) malloc( sizeof ( Board ) );

	// set all to zero
	for( i=0 ; i<WIDTH ; i++ ){
		for( j=0 ; j<WIDTH ; j++ ){
			isle->map->square[j][i] = 0;
		}
	}
	
	// set all that could be on the Board to -1 
	for( i=0 ; i<WIDTH ; i++ ){
		row_start = i - side + 1 > 0 ? i -side + 1 : 0;
		row_end = i + side > WIDTH ? WIDTH : i + side;
		for( j=row_start ; j<row_end ; j++ ){
			isle->map->square[j][i] = -1;
		}
	}

	// set the value of the base to 2
	isle->map->square[column][row]=2;

	// keep looping while there are squares
	// set to be tested.
	while( find_test_square( isle , &test_column , &test_row ) ){
		// loop through the squares's Neighbors
		for( i=0 ; i<6 ; i++ ){
			// we only care about the Neighbors that have not been marked
			if( -1 == get_neighbor( isle->map , test_column , test_row , i ) ){
				test_coordinates.x = test_column;
				test_coordinates.y = test_row;
				// find the Coordinates of the neighbor
				get_neighbor_coordinates( &test_coordinates , i , &tmp_coordinates );		
				// if the neighbor is not VOID mark it for testing
				if( VOID != get_neighbor( position , test_column , test_row , i ) ){
					mark_island( 2 , isle , tmp_coordinates.x , tmp_coordinates.y );		
				}
				else {
					//mark it as void
					mark_island( 0 , isle , tmp_coordinates.x , tmp_coordinates.y );	
				}
			}
		}
		// now mark the square itself as in the
		// island and testing complete
		mark_island( 1 , isle , test_column , test_row );
		// increment the number of discs in the island
		isle->numberDiscs++;
	  	// if there is a marble on it, increment marble count
		if( is_marbled( get_disc_contents( position , test_column , test_row ) ) ){
			isle->numberFilled++;
		}	
	}
	
	//if there are any "-1" left, they must be disconnected
	// so we set them to zero
	for( i=0 ; i<WIDTH ; i++ ){
		for( j=0 ; j<WIDTH ; j++ ){
			if( -1 == isle->map->square[j][i]  ){
				isle->map->square[j][i] = 0;
			}
		}
	}
	
}

/*
 * This loops through the squares
 * pointed to by island, 
 * sets the pointer to the first square
 * that requires testing.
 * If nothing needs testing will return
 * false.
 */
int find_test_square
( Island* isle , int* column_ptr , int* row_ptr )
{
	int i , j;
		
	for( i=0 ; i<WIDTH ; i++ ){
		for( j=0 ; j<WIDTH ; j++ ){
			if( isle->map->square[i][j] == 2 ){
				*column_ptr = i;
				*row_ptr = j;
				return TRUE;	
			}	
		}
	}
	
	return FALSE;	
}

/*
 * Put a mark on the island just like the Board.
 */
void mark_island
( int mark , Island* isle , int column , int row )
{
	isle->map->square[column][row] = mark;
}

/* 
 * Prints all the islands in a linked list.	
 */
void print_islands
( GSList* islands )
{
	Island* isle;
	
	printf("There are %d islands\n" , g_slist_length( islands ) );
	
	while( islands != NULL ){
		isle = (Island*) islands->data;
		print_island( isle );
		islands = g_slist_next( islands );
	}
	
}

/*
 * Prints out the island to the console.
 */
void print_island( Island* isle )
{
	int i,j;
	printf("Island has %d discs and %d marbles\n" , isle->numberDiscs , isle->numberFilled );
	for( i=0 ; i<WIDTH ; i++ ){
		for( j=0 ; j<WIDTH ; j++ ){
			printf("%d",isle->map->square[j][i]);
		}
		printf("\n");
	}
}

/*
 * This function updates the island list with 
 * the new situation.  It either marks the island
 * as having another disc filled, in case of 
 * a marble placement, or reduces the number
 * of discs in the island, in the case of a
 * disc removal, or reduces the number of marbles
 * in the case of a jump removing a marble.
 */
void update_islands
( GSList* islands , int column , int row , int flag )
{
	Island* isle;

	// loop through the islands on the list	
	// to find the correct island to update
	while( islands != NULL ) {
		isle = (Island*) islands->data;
	
		if( is_in_island( isle , column , row ) ){
			// perform the update
			if( flag == REMOVE_DISC ){
				mark_island( 0 , isle , column , row );
				isle->numberDiscs--;
			}
			else if( flag == ADD_MARBLE ){
				isle->numberFilled++;
			}
			else if( flag == REMOVE_MARBLE ){
				isle->numberFilled--;
			}
			else {
				printf("update island problem\n");
			}
		}
	
		islands = g_slist_next( islands );
	}
}

/*
 * Looks to see if the split is happening
 * to something that is already an island.
 * Returns NULL if there is no island.
 * Which should NEVER happen.  The Board
 * itself is an island.  Remember?
 */
Island* is_already_in_island
( GSList* islands , int column , int row )
{
	Island* isle;

	// loop through the islands on the list	
	while( islands != NULL ) {
		isle = (Island*) islands->data;
	
		if( is_in_island( isle , column , row ) ){
			//printf("main: %d,%d is already in an island\n" , column , row );
			return isle;
		}
		islands = g_slist_next( islands );
	}

	return NULL;
}

void copy_islands
( GSList* orig , GSList** copy )
{
	Island* old;
	Island* new;
	
	
	while( orig != NULL ){
		old = (Island*) orig->data;
		new = (Island*) malloc( sizeof( Island ) );
		copy_island( old , new );

		*copy = g_slist_prepend( *copy , new );
		
		orig = g_slist_next( orig );
	}	
}

void copy_island
( Island* orig , Island* copy )
{
	Board* new_board;
	int i,j;
	
	//printf("copying island\n");
	
	new_board = (Board*) malloc( sizeof( Board ) );
	for( i=0 ; i<WIDTH ; i++ ){
		for( j=0 ; j<WIDTH ; j++ ){
			new_board->square[i][j] = orig->map->square[i][j];
		}
	}
	copy->map = new_board;
	copy->numberDiscs = orig->numberDiscs;
	copy->numberFilled = orig->numberFilled;
}

/*
 * Deals with the issue of splitting 
 * an island into two.
 *
 * This function takes the island that
 * is being split and removes it from the
 * list.  Then, it creates two new islands
 * and adds them each to the list.
 * The two ints passed are the column and row
 * of the disc that was removed that caused
 * the split.
 */
void deal_with_split
( GSList** islands , Board* position , int column , int row )
{
	// the values of the two bases for the
	// islands
	int column1 , row1 , column2 , row2;
	Island* isle1;
	Island* isle2;
	Island* tmp;
	
	// delete the old island from the list
	if( ( tmp = is_already_in_island( *islands , column , row ) ) != NULL ){
		*islands = g_slist_remove( *islands , tmp );
		free( tmp );
	}

	// find the bases of the new islands
	set_split_neighbors( position , column , row , &column1 , &row1 , &column2 , &row2 );	

	// set up the islands
	// and add them to the list 
	isle1 = (Island*) malloc( sizeof( Island ) );
	set_island( position , isle1 , column1 , row1 );
	*islands = g_slist_append( *islands , isle1 );	

	isle2 = (Island*) malloc( sizeof( Island ) );
	set_island( position , isle2 , column2 , row2 );
	*islands = g_slist_append( *islands , isle2 );
}

/*
 * This function removes discs from the Board
 * that are in the island passed to it.
 *
 * ALSO NOTE THE INCREDIBLY DIRTY TRICK!!!
 * HINT:  involves mark_island! 
 * OMIGOD THIS IS SO PERVERTEDLY BAD!
 *
 * The trick is, the marbles on the
 * dead island are copied from the board
 * onto the island.
 */
void clean_up_dead_island
( Island* isle , Board* position )
{

	int i,j;
	int color;

	for( i=0 ; i<WIDTH ; i++ ){
		for( j=0 ; j<WIDTH ; j++ ){
			if( is_in_island( isle , j , i ) ){
				color = get_disc_contents( position , j , i );
				remove_disc( position , j , i );
				mark_island( color , isle , j , i );	
			}
		}
	}
	set_removeable_discs( position );

}

/*
 * This function looks through the list of
 * islands to determine if there are any dead 
 * ones.  If so it constructs a list of
 * dead islands and returns a pointer to it.
 *
 * It also removes dead islands from the list
 * passed to it.
 */
GSList* seek_bad_islands( GSList** islands , Board* position )
{
	GSList* tmp;
	Island* isle;
	GSList* dead_list = NULL;
	
	tmp = *islands;
	while( tmp != NULL ){
		isle = (Island*) tmp->data;
		if( ! is_alive( isle ) ){
			dead_list = g_slist_prepend( dead_list , isle );
		}
		tmp = g_slist_next( tmp );
	}
	
	tmp = dead_list;
	while( tmp != NULL ){
		isle = (Island*) tmp->data;
		*islands = g_slist_remove( *islands , isle );
		tmp = g_slist_next( tmp );
	}

	return dead_list;	
}

/* ...set emacs options for proper style (keep this at the end of the file)...
 *
 * Local Variables:
 * c-tab-always-indent: nil
 * indent-tabs-mode: t
 * c-basic-offset: 3
 * tab-width: 3
 * End:
 *
 */
