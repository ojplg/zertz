#include "rand_ai.h"
#include "move.h"
#include "player.h"
#include "play.h"

/*
 * Yuck what an enormous parameter list.
 * This includes everything an AI might
 * need in order to determine its next move.
 */
void get_random_move
( Move* move , Board* position , GSList* islands , int* goal , 
  int* available , int* one , int* two , int player_num )
{
	int n,p;
	GSList* moves = NULL;
	Move* mv;

	if( *available + *(available+1) + *(available+2) > 0 ){
		make_move_list( &moves , position , available );	
	}
	else {
		if( player_num == PLAYER_ONE ){
			make_move_list( &moves , position , one );
		}
		else {
			make_move_list( &moves , position , two );
		}
	}

	
	n = g_slist_length( moves );
	p = rand() % n;
	mv = (Move*) g_slist_nth_data( moves , p );

	if( mv->type == MOVE_TYPE_PLACE_AND_REMOVE ){	
		move->type = MOVE_TYPE_PLACE_AND_REMOVE;
		move->place_color = mv->place_color;	
		move->place_column = mv->place_column;	
		move->place_row = mv->place_row;	
		move->remove_column = mv->remove_column;	
		move->remove_row = mv->remove_row;	
	}
	else {
		move->type = MOVE_TYPE_JUMP;
		move->jump = (Jump*) malloc( sizeof( Jump ) );
		copy_jump( mv->jump , move->jump );
	}
	free_move_list( &moves );
}

/*
 * Makes a complete list of all moves for
 * the given state.
 */
void find_all_moves
( GSList** moves , GameState* state )
{

	if( are_free_marbles( state ) ){
		make_move_list( moves , &(state->position) , state->available );
	}
	else {
		if( state->mover == PLAYER_ONE ){
			make_move_list( moves , &(state->position) , state->one );
		}
		else { 
			make_move_list( moves , &(state->position) , state->two );
		}
	}
	
}

/*
 * Makes a list of moves available on the
 * board, based on the available marbles
 * passed.
 *
 * NOTE:  You must pass the correct thing
 * for avail!  If there are no available
 * marbles, then pass the marbles won by
 * the player.  For the purpose of determining
 * the move list, it is irrelevant where 
 * the marble is coming from.
 */
void make_move_list
( GSList** moves , Board* position , int* avail )
{
	if( jump_available( position ) ){
		make_jump_list( moves , position );
	}
	else {
		make_place_list( moves , position , avail );
	}
}

/*
 * Destroys the linked list of
 * moves and frees all the memory it took.
 */ 
void free_move_list
( GSList** moves )
{
	Move* mv;
	GSList* start;
	start = *moves;

	while( *moves != NULL ){
		mv = (Move*) (*moves)->data;
		free_move( mv );
		*moves = g_slist_next( *moves );
	}
	g_slist_free( start );
	
}


/*
 * Makes a list of possible placing moves.
 * If the place does not allow for a remove,
 * the remove coordinates will be reported
 * as -1,-1.
 */
void make_place_list
( GSList** moves , Board* position , int* avail )
{
	// three loops are required
	// one for the color marbles
	// one for the places to place
	// one for the discs to remove
	GSList* empty_discs;
	GSList* tmp_empty;
	GSList* removeable_discs;
	GSList* tmp_remove;
	int color;
	Move* move;
	Coordinates* disc_empty;
	Coordinates* disc_remove;
	
	empty_discs = find_empty_discs( position );
	removeable_discs = find_removeable_discs( position );

	/***** FIXME This set of loops is revolting.  FIXME ******/
	
	for( color=BLACK ; color<=WHITE ; color++ ){
		// only enter if the color is available
		tmp_empty = empty_discs;
		if( *(avail+color-BLACK) > 0 ){
			while( tmp_empty != NULL ){
				
				disc_empty = (Coordinates*) tmp_empty->data;
				tmp_remove = removeable_discs;
			
				/* if there are no removals
					add to the list and move on */
				if( tmp_remove == NULL ){
					move = (Move*) malloc( sizeof( Move ) );
					initialize_move( move );
					move_set_place_data( move , color , disc_empty->x , disc_empty->y );
					*moves = g_slist_prepend( *moves , move );
				}
					
				while( tmp_remove != NULL ){
					disc_remove = (Coordinates*) tmp_remove->data;
					
					move = (Move*) malloc( sizeof( Move ) );
					initialize_move( move );
					move_set_place_data( move , color , disc_empty->x , disc_empty->y );

					/* only add to the list if we have a remove,
						or if there is no remove because we just
						took it away with the place. */
					if( ! same_coordinates( disc_remove , disc_empty ) ){	
						move_set_remove_data( move , disc_remove->x , disc_remove->y );
						*moves = g_slist_prepend( *moves , move ); 
					}
					else if( g_slist_length( removeable_discs ) == 1 ){
						*moves = g_slist_prepend( *moves , move ); 
					}
					else {
						free_move( move );
					}

					tmp_remove = g_slist_next( tmp_remove );
				}
				tmp_empty = g_slist_next( tmp_empty );
			}
		}
	}
	//check out the memory management baby
	clear_coordinate_list( empty_discs );
	clear_coordinate_list( removeable_discs );

}

/*
 * Prints out each move in a list of
 * possible moves. 
 */
void print_move_list
( GSList* moves )
{
	Move* move;
	int cnt = 0;

	
	while( moves != NULL ){
		move = (Move*) moves->data;
		printf("%d ", cnt++);
		print_move( move );
		moves = g_slist_next( moves );
	}
	
}

/* 
 * Returns TRUE if the coordinates point
 * to the same place, FALSE otherwise. 
 */
int same_coordinates
( Coordinates* a , Coordinates* b )
{
	if( (a->x == b->x) && (a->y == b->y) ){
		return TRUE;
	}
	return FALSE;
}

/*
 * Prints an individual move.
 */
void print_move
( Move* move )
{
	if( move->type == MOVE_TYPE_PLACE_AND_REMOVE ){
		printf("Place %d at %d,%d and remove at %d,%d\n",
			move->place_color,
			move->place_column,
			move->place_row,
			move->remove_column,
			move->remove_row
				);
	}
	else if( move->type == MOVE_TYPE_JUMP ){
		print_jump( move->jump );
	}
}

/*
 * Makes a list of all possible jumps.	
 */
void make_jump_list
( GSList** moves , Board* position )
{
	GSList* jump_trees;
	GSList* first_jump_tree;
	GSList* jumps = NULL;
	GNode* jump_tree;
	GNode* leaf;
	Jump* jump;
	Move* move;	
	
	// the first step is to make the coordinate trees 
	jump_trees = create_coordinate_trees( position );
	first_jump_tree = jump_trees;
	
	// then make the trees into a list of jumps	
	while( jump_trees != NULL ){

		jump_tree = jump_trees->data;	
		
		leaf = NULL;
		while( g_node_first_child( jump_tree ) != NULL ){
			jump = (Jump*) malloc( sizeof( Jump ) );
			leaf = create_jump_from_tree( jump , jump_tree );
			jump_tree = prune_tree( leaf );
			jumps = g_slist_prepend( jumps , jump );
		}	
		
		jump_trees =  jump_trees -> next;
	}

	/* all the memory pointed to in the trees
		was cleared in prune_tree */
	g_slist_free( first_jump_tree );
		
	// takes the jump list and turns it into 
	// a moves list
	while( jumps != NULL ){
		move = (Move*) malloc( sizeof( Move ) );
		move->type = MOVE_TYPE_JUMP;
		move->jump = (Jump*)( jumps->data );
		*moves = g_slist_prepend( *moves , move );
		jumps = jumps -> next;
	}

	/* no need to free the jumps, that memory
	   is pointed to by the moves list */	
	g_slist_free( jumps );
	
}

/*
 * frees all the memory in a singly
 * linked list of coordinates.
 *
 * NOTE:  DEPENDS ON CASTS.  passing
 * an arbitrary list is VERY dangerous.
 */ 
void clear_coordinate_list
( GSList* list )
{
	GSList* tmp;
	Coordinates* coord;
	
	tmp = list;
	while( tmp != NULL ){
	  	coord = (Coordinates*) tmp->data;
		free( coord );

		tmp = g_slist_next( tmp );
	}

	g_slist_free( list );
}

/*
 * Debugging purposes ONLY.  Prints
 * a list of coordinates.
 *
 * NOTE:  DEPENDS ON CASTS.  passing
 * an arbitrary list is VERY dangerous.
 */
void print_coordinate_list
( GSList* list )
{
	GSList* tmp;
	Coordinates* coord;
	int cnt = 0;
	
	printf("rand_ai:  printing a coordinate list :\n");
	
	tmp = list;
	while( tmp != NULL ){
	  	coord = (Coordinates*) tmp->data;
		printf("   %d)  %d,%d\n", cnt++ , coord->x , coord->y );
		tmp = g_slist_next( tmp );
	}
}

/*
 * Returns a linked list of pointers 
 * to strange trees of jumps.  VERY CONFUSING!
 * Each node of the tree holds a coordinate pointer.
 * The root node holds the coordinate of the
 * spot that has the jump.  Each of the children
 * shows a landing place.  OK?
 */
GSList* create_coordinate_trees
( Board* position )
{
	int i, j;
	GSList* jump_trees;
	GNode* jump_tree;
	
	jump_trees = NULL;	
	for( i=0 ; i<WIDTH ; i++ ){
		for( j=0 ; j<WIDTH ; j++ ){
			if( has_jump( position , j , i ) ){
				jump_tree = g_node_new( NULL );
			  	handle_jumper( position , j , i , jump_tree );	
			 	jump_trees = g_slist_prepend( jump_trees , jump_tree );
			}
		}
	}

	return jump_trees;
}

/*
 * This function follows a path along the
 * coordinate tree, it always goes to the 
 * first child, and creates a Jump from
 * the coordinates it finds.
 *
 *	Returns a pointer to the leaf node
 *	that was the end of the Jump created.
 */
GNode* create_jump_from_tree
( Jump* jump , GNode* tree )
{
	GNode* child;
	GNode* parent;
	Coordinates* par_coord_ptr;
	Coordinates* chi_coord_ptr;
	
	if( tree == NULL ){
		return NULL;
	}
	
	// set up parent and child
	parent = tree;
	child = g_node_first_child( parent );

	// get data from them	
	par_coord_ptr = (Coordinates*) parent->data;
	chi_coord_ptr = (Coordinates*) child->data;
	
	// enter data into jump
	jump->start_column = par_coord_ptr->x;
	jump->start_row = par_coord_ptr->y;
	jump->end_column = chi_coord_ptr->x;
	jump->end_row = chi_coord_ptr->y;
	jump->multi = NULL;		
	
	parent = child;
	// deal with multiple jumps
	while( ( child = g_node_first_child( parent ) ) != NULL ){
		
		jump->multi = (Jump*) malloc( sizeof( Jump ) );
		jump = jump -> multi;

		par_coord_ptr = (Coordinates*) parent->data;
		chi_coord_ptr = (Coordinates*) child->data;
		
		jump->start_column = par_coord_ptr->x;
		jump->start_row = par_coord_ptr->y;
		jump->end_column = chi_coord_ptr->x;
		jump->end_row = chi_coord_ptr->y;
		jump->multi = NULL;		
	
		parent = child;
	
	}
		
	return parent;	
}

/*
 * This removes the nodes from the tree that
 * lead solely to the leaf.  Any nodes that
 * have multiple children are left.
 * Returns a pointer to the tree's root
 * after pruning.
 */ 
GNode* prune_tree( GNode* leaf )
{
	GNode* momma;
	GNode* kiddie;
	
	kiddie = leaf;
	momma = leaf->parent;	
	
	while( g_node_n_children( momma ) == 1 ){
		//printf("inside while\n");
		
		if( G_NODE_IS_ROOT(momma) ){
			//printf("momma is root!\n");
			free( kiddie->data );
			free( momma->data );
			g_node_destroy( momma );
			return NULL;
		}
		
		free( kiddie->data );
		kiddie = momma;
		momma = momma->parent;
		
	}
	
	//printf("dropped out of while \n");	
	if( kiddie != NULL ) {
		free( kiddie->data );
		g_node_destroy( kiddie );
	}
	
	return g_node_get_root( momma );
	
}

/*
 * This attempts to make a tree of the jumps from  
 * the passed point.  The data in each node of
 * the tree is a coordinate pointer.
 * Follow the tree to make a jump.
 */
void handle_jumper
( Board* position , int column , int row , GNode* leaf )
{
	GSList* landings;
	GSList* landings_start;
	Coordinates* landing;
	Coordinates* take_off;
	Board future;
	GNode* child;
	Jump jump;
	
	// make the coordinates that this leaf will
	// point to
	// this must be a pointer, so the memory
	// exists after the recursive call
	// the memory will be freed only after
	// prune_tree is called ... I hope
	take_off = (Coordinates*) malloc( sizeof( Coordinates ) );
	take_off->x = column;
	take_off->y = row;
	
	leaf->data = take_off;
	
	// find all the places that the marble could land	
	landings = find_jump_ends( position , column , row );
	landings_start = landings;
	
	// loop through them
	while( landings != NULL ){
		
		landing = (Coordinates*) landings->data;
		child = g_node_append_data( leaf , landing );	
		
		copy_board( position , &future );
		
		jump.start_column = column;
		jump.start_row = row;
		jump.end_column = landing->x;
		jump.end_row = landing->y;
		jump.multi = NULL;
		
		perform_jump( &future , &jump );
	
		handle_jumper( &future , jump.end_column , jump.end_row , child );
	
		// this was malloced by find_jump_ends
		free( landing );
		
		landings = g_slist_next( landings );
	}
	
	g_slist_free( landings_start );
	
}

/*
 * Prints out the jumps avaiable from a 
 * certain point.
 */
void print_jump_tree
( GNode* tree )
{
	int row, column;
	Coordinates* coord_ptr;
	
	coord_ptr = (Coordinates*) tree->data;
	column = coord_ptr->x;
	row = coord_ptr->y;

	printf("rand_ai.c The marble at %d,%d is a jumper.\n" , column , row );
	
	follow_jump( tree );
	printf("\nrand_ai.c jump tree all printed.\n");
}

/*
 * Stupid function used only by print_jump_tree.
 */ 
void follow_jump
( GNode* node )
{
	int num_children;
	int i;
	GNode* child;

	num_children = g_node_n_children( node );	
	if( num_children > 0 ){
		printf("Jumps to ");
	}
	for( i=0 ; i<num_children ; i++ ){
		child = g_node_nth_child( node , i );
		print_node( child );	
		if( i+1<num_children ){
			printf(" or to ");
		}
		follow_jump( child );
	}

}

/* 
 * Prints the contents of a node
 * that contains a coordinates pointer. 
 */
void print_node
( GNode* node )
{
	int row, column;
	Coordinates* coord_ptr;
	
	coord_ptr = (Coordinates*) node->data;
	column = coord_ptr->x;
	row = coord_ptr->y;

	printf(" %d,%d ", column , row );	
}

/*
 * Pass a point and the board position.
 * This function will fill the list
 * landings with coordinate pointers
 * to places the marble can land.
 */
GSList* find_jump_ends
( Board* position , int column , int row )
{
	
	int dir;
	Coordinates jumper;
	Coordinates jumpee;
	Coordinates* landing_ptr;
	GSList* landings;
	
	jumper.x = column;
	jumper.y = row;
	landings = NULL;

	for( dir=0 ; dir<6 ; dir++ ){
		get_neighbor_coordinates( &jumper , dir , &jumpee );
		landing_ptr = (Coordinates*) malloc( sizeof( Coordinates ) );
		get_neighbor_coordinates( &jumpee , dir , landing_ptr );
		if( can_jump_to( position , column , row , landing_ptr->x , landing_ptr->y ) ){
			landings = g_slist_prepend( landings , landing_ptr );		
		}
		else {
			free( landing_ptr );
		}
	}
	return landings;
}

/*
 * Prints out all the jumps in
 * a linked list.  MIGHT WORK.
 * THERE ARE NO LINKED LISTS OF
 * JUMPS TO PRINT AT THE MOMENT!
 */
void print_available_jumps
( GSList* jumps )
{
	Jump* jump_ptr;
	int i = 1;

	while( jumps != NULL ){
		jump_ptr = (Jump*) jumps->data;
		printf("rand_ai:  Jump ---%d--- " , i );  
		print_jump( jump_ptr );
		i++;
		jumps = g_slist_next( jumps );
	}
}

/*
 * Debugging.
 */
void print_jump
( Jump* jump_ptr )
{
	printf("Jump from %d,%d " , jump_ptr->start_column , jump_ptr->start_row );
	printf("to %d,%d" , jump_ptr->end_column , jump_ptr->end_row );
	if( jump_ptr->multi != NULL ){
		printf(" AND thence: "); 
		print_jump( jump_ptr->multi );
	}
	else {
		printf(" DONE\n");
	}
}


/*
 * Builds a linked list of
 * all the empty discs on the
 * board.
 */
GSList* find_empty_discs
( Board* position )
{
	int i,j;
	Coordinates* coord_ptr;
	GSList* empty_discs = NULL;
	
	for( i=0 ; i<WIDTH ; i++ ){
		for( j=0 ; j<WIDTH ; j++ ){
			if( can_accept_marble( position , j , i ) ){
				coord_ptr = (Coordinates*) malloc( sizeof( Coordinates ) );
				coord_ptr->x = j;
				coord_ptr->y = i;
				empty_discs = g_slist_prepend( empty_discs , coord_ptr );
			}
		}
	}
	return empty_discs;
}

/*
 * Builds a linked list of
 * all the removeable discs on the
 * board.
 */
GSList* find_removeable_discs
( Board* position )
{
	int i,j;
	Coordinates* coord_ptr;
	GSList* removeable_discs = NULL;
	
	for( i=0 ; i<WIDTH ; i++ ){
		for( j=0 ; j<WIDTH ; j++ ){
			if( is_removeable( position , j , i ) ){
				coord_ptr = (Coordinates*) malloc( sizeof( Coordinates ) );
				coord_ptr->x = j;
				coord_ptr->y = i;
				removeable_discs = g_slist_append( removeable_discs , coord_ptr );
			}
		}
	}
	return removeable_discs;
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
