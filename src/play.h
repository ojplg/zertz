#ifndef PLAY_H
#define PLAY_H


int check_for_win( GameState* , int , int );
int check_for_loss( GameState* , int , int );
void set_state( GameState* , Board* , GSList* , int* , int* , int* , int );
void free_state( GameState* );
void free_turn( Turn* );

void copy_state( GameState* , GameState* );
GSList* find_new_state( GameState* , GameState* , Move* );
void state_after_jump( GameState* , Move* );
GSList* state_after_place( GameState* , Move* );
void decrement_available_marbles( GameState* , int );
void dead_island_scorer( GameState* , Island* );
int determine_future_position( Board* , Jump* );
void print_state( GameState* );
void print_marbles( int* );

void set_up_new_game( GameState* , int );
void init_available_marbles( GameState* , int );
void free_island_list( GSList** );
void free_island( Island* );
int are_free_marbles( GameState* );
int perform_jump( Board* , Jump* );

int check_for_game_over( GameState* , int );
void get_goals( int* , int );

#endif
