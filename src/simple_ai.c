#include "simple_ai.h"
#include "move.h"
#include "player.h"
#include "play.h"

/*
 * Finds a simple move.
 *
 * Mostly looks at the position after the place
 * and determines if it looks better or worse.
 * But, has a sneaky recursion to allow it
 * to evaluate, primitively, cascading jumps.
 */
float find_simple_move
( Move* move , GameState* state , int level , int player_num )
{

	float top_eval = -101.0;
	float tmp = -101.0;
	Move* mv;
  	Move*	best;
	GSList* moves = NULL;
	GSList* mvs = NULL;
	GameState new_state;
	Move* stub = NULL;
	int cnt = 1;
	
	find_all_moves( &moves , state ); 	
	mvs = moves;
	
	while( moves != NULL ){
		mv = (Move*) moves->data;
		find_new_state( state , &new_state , mv );
		tmp = eval_game_state( &new_state , level , player_num );
		if( jump_available( &(new_state.position) ) && abs(tmp) != 100.0 ){
			stub = (Move*)	malloc( sizeof( Move ) );
			tmp = -find_simple_move( stub , &(new_state) , level , TOGGLE(player_num) );
			free_move( stub );
		}
				
		// perhaps switch if equal
		if( tmp == top_eval ){
			if( 0 == (rand()%cnt) ){
				top_eval = tmp;
				best = mv;
			}
			cnt++;
		}
		
		// always switch if better
		if( tmp > top_eval ){
			top_eval = tmp;
			best = mv;
		}
		free_island_list( &(new_state.islands ) );
		moves = g_slist_next( moves );
	}

	if( best->type == MOVE_TYPE_PLACE_AND_REMOVE ){	
		move->type = MOVE_TYPE_PLACE_AND_REMOVE;
		move->place_color = best->place_color;	
		move->place_column = best->place_column;	
		move->place_row = best->place_row;	
		move->remove_column = best->remove_column;	
		move->remove_row = best->remove_row;	
	}
	else {
		move->type = MOVE_TYPE_JUMP;	
		move->jump = (Jump*) malloc( sizeof( Jump ) );
		copy_jump( best->jump , move->jump );
	}

	free_move_list( &mvs );
	
	return top_eval;		
	
}

/*
 * UNUSED FOR THE MOMENT.
 */
int is_interesting_move
( Move* move , GameState* state )
{

	GameState new_state;
	find_new_state( state , &new_state , move );
	if( is_interesting_position( &(new_state.position) ) ){
		return TRUE;
	}
	return FALSE;	
	
}

/*
 * UNUSED FOR THE MOMENT.
 */
int is_interesting_position
( Board* pos )
{

	if( jump_available( pos ) ){
		return TRUE;
	}
	else if( split_available( pos ) ){
		return TRUE;
	}
	return FALSE;
}

/*
 * Makes an evaluation of the state of the 
 * game.
 *
 * Very simple minded:  compares the number of
 * won marbles.
 */ 
float eval_game_state
( GameState* state , int level , int player_num )
{

	int my_black , my_white , my_grey;
	int opp_black , opp_white , opp_grey;
	float sum;
	int my_dist , opp_dist;

	// the first part of the
	// evaluation simply compares
	// the weighted value of the 
	// marbles won	
	if( player_num == PLAYER_ONE ){
		my_black = *(state->one);
		my_grey = *(state->one + 1);
		my_white = *(state->one + 2);
		opp_black = *(state->two);
		opp_grey = *(state->two + 1);
		opp_white = *(state->two + 2);
		my_dist = distance_from_victory( state->one , level );	
		opp_dist = distance_from_victory( state->two , level );	
	}
	else {
		my_black = *(state->two);
		my_grey = *(state->two + 1);
		my_white = *(state->two + 2);
		opp_black = *(state->one);
		opp_grey = *(state->one + 1);
		opp_white = *(state->one + 2);
		my_dist = distance_from_victory( state->two , level );	
		opp_dist = distance_from_victory( state->one , level );	
	}
	
	sum = my_black * BLACK_WEIGHT +
		my_grey * GREY_WEIGHT +
		my_white * WHITE_WEIGHT -
		opp_black * BLACK_WEIGHT -
		opp_grey * GREY_WEIGHT -
		opp_white * WHITE_WEIGHT;

	// then that sum is adjusted for the 
	// distance from victory factor
	// The idea is to alert the AI that
	// sometimes it is better to give up
	// a more valuable marble in case the
	// opponent is too close to a particular
	// victory condition
	if( 2 == opp_dist ){
		sum -= OPP_DIST_TWO_WEIGHT;
	}
	else if( 1 == opp_dist ){
		sum -= OPP_DIST_ONE_WEIGHT;
	}
	if( 2 == my_dist ){
		sum += MY_DIST_TWO_WEIGHT;
	}
	else if( 1 == my_dist ){
		sum += MY_DIST_ONE_WEIGHT;
	}
	
	// adjust for split availability	
	if( split_available( &(state->position ) ) ){
		sum -= SPLIT_PENALTY;
	}
	
	// A winning or losing position is worth 
	// more (or less) than anything
	if( check_for_win( state , level , player_num ) ) {
		sum = 100.0;
	}
	else if( check_for_loss( state , level , player_num ) ){
		sum = -100.0;
	}
	
	return sum;
}

/*
 * Computes how many marbles away
 * from the closest victory condition
 * a player is.  NOTE:  Does not account
 * for the possibility that marbles may
 * not be available for that condition.
 */
int distance_from_victory
( int* won , int level )
{
	int goals[3];
	int tmp = 0;
	int dist = 6;
	int i;

	get_goals( goals , level );
	
	if( level == TOURNAMENT ){
		//goals[0] = TOURN_BLACK_GOAL;
		//goals[1] = TOURN_GREY_GOAL;
		//goals[2] = TOURN_WHITE_GOAL;
		
		for( i=0 ; i<3 ; i++ ){
			tmp += TOURN_ALL_GOAL - won[i] > 0 ? TOURN_ALL_GOAL - won[i] : 0;
		}
	}
	else {
		//goals[0] = BEGIN_BLACK_GOAL;
		//goals[1] = BEGIN_GREY_GOAL;
		//goals[2] = BEGIN_WHITE_GOAL;
		
		for( i=0 ; i<3 ; i++ ){
			tmp += BEGIN_ALL_GOAL - won[i] > 0 ? BEGIN_ALL_GOAL - won[i] : 0;
		}
	}

	if( tmp < dist ){
		dist = tmp;
	}
	tmp = goals[0]  - won[0];
	if( tmp < dist ){
		dist = tmp;
	}
	tmp = goals[1]  - won[1];
	if( tmp < dist ){
		dist = tmp;
	}
	tmp = goals[2]  - won[2];
	if( tmp < dist ){
		dist = tmp;
	}
	
	return dist;
	
}


