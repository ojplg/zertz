#include <gnome.h>
#include "board.h"
#include "play.h"
#include "move.h"
#include "player.h"
#include "prefs.h"
#include "main.h"

/*
 * This cleans up a state that has been
 * malloced and used.  NOTE: if the state
 * has never been initialized, this will
 * choke on freeing the island list.
 */
void free_state
( GameState* state )
{
	GSList* isles;

	isles = state->islands;
	
	while( isles != NULL ){
		free( isles->data );
		isles = g_slist_next( isles );
	}
	g_slist_free( state->islands );

	free( state );
}

/*
 * This is for debugging.  Prints the state to the
 * console.
 */
void print_state
( GameState* state )
{
	printf("Here is the state:\n");
	print_board( &(state->position) );
	print_islands( (state->islands)	);
	printf("Player one has: ");
	print_marbles( state->one );
	printf("Player two has: ");
	print_marbles( state->two );
	printf("Available marbles are: ");
	print_marbles( state->available );
}

/*
 * Used by above.  More of a MACRO than a function.
 */
void print_marbles
( int* mar )
{
	printf("Black: %d,  Grey: %d, White: %d\n",mar[0],mar[1],mar[2]);
}

/*
 * Makes a copy of the game state.
 */
void copy_state
( GameState* orig , GameState* copy )
{	
	int i;

	copy->islands = NULL;
	copy_islands( orig->islands , &(copy->islands) );
	copy_board( &(orig->position) , &(copy->position) );
	for( i=0 ; i<3 ; i++ ){
		copy->available[i] = orig->available[i];
	}
	for( i=0 ; i<3 ; i++ ){
		copy->one[i] = orig->one[i];
	}
	for( i=0 ; i<3 ; i++ ){
		copy->two[i] = orig->two[i];
	}
	copy->mover=orig->mover;
	
}

/*
 * Looks at the old state, and the move, and
 * fills in the new_state with the situation
 * after the move is made.
 *
 * Returns a list of dead islands. 
 */
GSList* find_new_state
( GameState* old_state , GameState* new_state , Move* move )
{
	GSList* dead = NULL;
	
	// two things ... state after jumps state after place

	copy_state( old_state , new_state );
	
	if( move->type == MOVE_TYPE_JUMP ){
		state_after_jump( new_state , move );
	}	
	else {
		dead = state_after_place( new_state , move );
	}
	
	if( old_state->mover == PLAYER_ONE ){
		new_state->mover = PLAYER_TWO;
	}
	else {
		new_state->mover = PLAYER_ONE;
	}
		
	return dead;
}

/*
 * Determines what the GameState will
 * be after the Move has occured.  Works
 * for jump turns.
 */
void state_after_jump
( GameState* state , Move* move )
{
	Jump* p_jump = NULL;
	int jumped_color;
	Coordinates start;
	Coordinates end;
	Coordinates middle;
	
	p_jump = move->jump;	

	while( p_jump != NULL ){
		// fix board
		jumped_color = perform_jump( &(state->position) , p_jump );
		
		// then fix score
		if( state->mover == PLAYER_ONE ){
			state->one[jumped_color - 3] ++;
		}
		else {
			state->two[jumped_color - 3] ++;
		}
			
		// update the island of the marble removed
		start.x = p_jump->start_column;
		start.y = p_jump->start_row;
		end.x = p_jump->end_column; 
		end.y = p_jump->end_row;		
		get_middle_coordinates( &start , &end , &middle );
		update_islands( state->islands , middle.x , middle.y , REMOVE_MARBLE );
		p_jump = p_jump->multi;
	}

}

/*
 * Updates the board position to
 * what it will be after a jump.
 *
 *	NB:  FOR A SINGLE JUMP.  NOT
 *	A SERIES!
 * 
 * Returns the color of the jumped
 * marble. 
 */
int perform_jump
( Board* future , Jump* jump )
{

	Coordinates start;
	Coordinates end;
	Coordinates middle;
	int jumping_color;
	int jumped_color;
	
	start.x = jump->start_column;	
	start.y = jump->start_row;	
	end.x = jump->end_column;	
	end.y = jump->end_row;	
	get_middle_coordinates( &start , &end , &middle );
		
	jumping_color = remove_marble( future , start.x , start.y );
	jumped_color = remove_marble( future , middle.x , middle.y );
	place_marble( jumping_color , future , end.x , end.y );	

	return jumped_color;
}

/*
 * Determines what the GameState will
 * be after the Move has occured.  Works
 * for place & remove turns.
 *
 * Returns a linked list of islands to dispose.
 */
GSList* state_after_place
( GameState* state , Move* move )
{
	Island* dead = NULL;
	GSList* dead_list = NULL;
	GSList* tmp = NULL;
	
	// First decrement the marble total appropriately. 
	if( are_free_marbles( state ) ){
		decrement_available_marbles( state , move->place_color );
	}
	else {
		if( state->mover == PLAYER_ONE ){
			state->one[move->place_color - BLACK] --;
		}
		else {
			state->two[move->place_color - BLACK] --;
		}
	}

	// TAKE CARE OF THE PLACE
	place_marble( move->place_color , &(state->position) , move->place_column , move->place_row );
	// update islands
	update_islands( state->islands , move->place_column , move->place_row , ADD_MARBLE );
	
	// NOW DO THE REMOVE	
	if( move->remove_column != -1 ){
		remove_disc( &(state->position) , move->remove_column , move->remove_row ); 
	}
	// check for islands created
	if( will_cause_split( &(state->position) , move->remove_column , move->remove_row ) ){
		deal_with_split( &(state->islands) , &(state->position) , move->remove_column , move->remove_row );
	}
	else {
		update_islands( state->islands , move->remove_column , move->remove_row , REMOVE_DISC );
	}
	
	// look for dead islands
	dead_list = seek_bad_islands( &(state->islands) , &(state->position) );
	tmp = dead_list;
	
	while( tmp != NULL ){
		dead = (Island*) tmp->data;	
		clean_up_dead_island( dead , &(state->position) );
		dead_island_scorer( state , dead );
		tmp = g_slist_next( tmp );
	}
	
	return dead_list;
	
}

/*
 * Reduces the count of available marbles	
 * of the passed color by one.
 */
void decrement_available_marbles
( GameState* state , int color )
{
	if( color == BLACK ){
		state->available[0]--;
	}
	else if( color == GREY ){
		state->available[1]--;
	}
	else if( color == WHITE ){
		state->available[2]--;
	}
	else {
		printf("UGG. ERROR. Marble of unknown color\n");
	}

}

/*
 * Sets the state pointed to by the first argument	
 * according to the information in the rest.
 */
void set_state
( GameState* state , Board* position , GSList* islands , int* avail , int* player1 , int* player2 , int mover )
{
	int i;
	
	copy_board( position , &(state->position) );
	state->islands = NULL;
	copy_islands( islands , &(state->islands) );
	for( i=0 ; i<3 ; i++ ){
		state->available[i] = avail[i];
	}
	for( i=0 ; i<3 ; i++ ){
		state->one[i] = player1[i];
	}
	for( i=0 ; i<3 ; i++ ){
		state->two[i] = player2[i];
	}
	state->mover = mover;
}

/*
 * Updates the game_state by changing the
 * score after the island disappears.
 */
void dead_island_scorer
( GameState* state , Island* isle )
{
	int i,j;
	int color;

	for( i=0 ; i<WIDTH ; i++ ){
		for( j=0 ; j<WIDTH ; j++ ){
			if( is_in_island( isle , i , j ) ){
				color = get_disc_contents( (isle->map) , i , j );
				if( state->mover == PLAYER_ONE ){
					state->one[color - 3] ++;
				}
				else {
					state->two[color - 3] ++;
				}
			}
		}
	}
}

/* 
 * Initializes the game state to the starting
 * situation.
 */
void set_up_new_game
( GameState* state , int level )
{
	Island* isle_ptr;
	
	// set scores to zero
	state->one[0] = 0;
	state->one[1] = 0;
	state->one[2] = 0;
	state->two[0] = 0;
	state->two[1] = 0;
	state->two[2] = 0;
	
	// set mover to player one
	state->mover = PLAYER_ONE;

	// set up the board
	init_board( &(state->position) );
	
	// set up the marbles
	init_available_marbles( state , level );
	
	// set up the island list by adding
	// the entire board to the list as one island	
	free_island_list( &(state->islands) );
	isle_ptr = malloc( sizeof( Island ));
	set_island( &(state->position) , isle_ptr , 0 , 0 );
	state->islands = g_slist_append( state->islands , isle_ptr );
	
}

/*
 * Frees the memory held by a Turn struct
 * pointer.
 */
void free_turn( Turn* turn )
{
	free_island_list( &(turn->state.islands) );
	free( turn );
}

/*
 * Restores the island list to NULL, and
 * frees all the memory held by the island
 * pointers. 
 */
void free_island_list
( GSList** islands )
{
	Island* isle_ptr;
	
	while( *islands != NULL ){
		isle_ptr = (Island*) (*islands)->data;
		*islands = g_slist_remove( *islands , isle_ptr );
		if( isle_ptr != NULL ){
			free_island( isle_ptr );
		}
	}	
}

/*
 * Properly frees a malloc Island struct
 * including the embedded Board struct.
 */ 
void free_island( Island* isle )
{
	if( isle == NULL ){
		return;
	}

	free( isle->map );
	free( isle );
	
}

/*
 *	set the available marbles based on the game level
 */
void init_available_marbles( GameState* state , int level )
{
	if( level == BEGINNER ) {
		state->available[0] = BEGIN_BLACK_AVAIL; 
		state->available[1] = BEGIN_GREY_AVAIL; 
		state->available[2] = BEGIN_WHITE_AVAIL; 
	}
	else if( level == TOURNAMENT ){
		state->available[0] = TOURN_BLACK_AVAIL; 
		state->available[1] = TOURN_GREY_AVAIL; 
		state->available[2] = TOURN_WHITE_AVAIL; 
	}
}

/*
 * Returns true if there are marbles in the
 * unused bin.  False otherwise.
 */
int are_free_marbles
( GameState* state )
{
	int i;
	int cnt=0;
	
	for( i=0 ; i<3 ; i++ ){
		cnt += state->available[i];
	}
	
	if( cnt > 0 ){
		return TRUE;
	}
	return FALSE;
}

/*
 * Compares the player's won marbles
 * with the number needed to see if
 * that player has achieved the victory
 * conditions.
 */ 
int check_for_win
( GameState* state , int level , int player_num )
{
	int curr_black , curr_white , curr_grey;

	if( player_num == PLAYER_ONE ){
		curr_black = *(state->one);
		curr_grey = *(state->one + 1);
		curr_white = *(state->one + 2);
	}
	else {
		curr_black = *(state->two);
		curr_grey = *(state->two + 1);
		curr_white = *(state->two + 2);
	}
	
	if( level == BEGINNER ){
		if( curr_black >= BEGIN_ALL_GOAL && 
				curr_grey >= BEGIN_ALL_GOAL &&
				curr_white >= BEGIN_ALL_GOAL ){
			return TRUE;
		}
		else if( curr_black >= BEGIN_BLACK_GOAL || 
				curr_grey >= BEGIN_GREY_GOAL ||
				curr_white >= BEGIN_WHITE_GOAL ){
			return TRUE;
		}
		else {
			return FALSE;
		}
	}	
	else if ( level == TOURNAMENT ){
		if( curr_black >= TOURN_ALL_GOAL && 
				curr_grey >= TOURN_ALL_GOAL &&
				curr_white >= TOURN_ALL_GOAL ){
			return TRUE;
		}
		else if( curr_black >= TOURN_BLACK_GOAL || 
				curr_grey >= TOURN_GREY_GOAL ||
				curr_white >= TOURN_WHITE_GOAL ){
			return TRUE;
		}
		else {
			return FALSE;
		}
	}

	/* should never happen! */
	return FALSE;

}

/*
 * Check to see if someone has won.
 */
int check_for_game_over( GameState* state , int level )
{
	if( check_for_win( state , level , PLAYER_ONE ) ){
		printf(">>> Player One has Won! <<<\n");
		return VICTORY_ONE;
	}
	if( check_for_win( state , level , PLAYER_TWO ) ){
		printf(">>> Player Two has Won! <<<\n");
		return VICTORY_TWO;
	}
	return FALSE;
}

/*
 * Checks to see if the opposing player
 * has achieved victory.
 */ 
int check_for_loss
( GameState* state , int level , int player_num )
{
	return check_for_win( state , level , TOGGLE( player_num ) );	
}

/*
 * Sets the goals array passed it to
 * the correct values.
 */ 
void get_goals
( int* goals , int level )
{
	if( level == BEGINNER ){
		goals[0] = BEGIN_BLACK_GOAL;
		goals[1] = BEGIN_GREY_GOAL;
		goals[2] = BEGIN_WHITE_GOAL;
	}
	else if( level == TOURNAMENT ){
		goals[0] = TOURN_BLACK_GOAL;
		goals[1] = TOURN_GREY_GOAL;
		goals[2] = TOURN_WHITE_GOAL;
	}
}

void next_turn( GameState* state , Move* move )
{


}
		


/*
 * Frees all the memory held 
 * by a GameSate pointer.
 */ 
void free_game_state
( GameState* state )
{
	free_island_list( &(state->islands) );
	free( state );	
}
