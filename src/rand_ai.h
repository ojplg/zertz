#ifndef RAND_AIH
#define RAND_AIH

#include <gnome.h>
#include "board.h"
#include "move.h"


void get_random_move( Move* , Board* , GSList* , int* , int* , int* , int* , int );
GSList* create_coordinate_trees( Board* );
GSList* find_removeable_discs( Board* );
GSList* find_empty_discs( Board* );
GSList* find_jump_ends( Board* , int , int );
void handle_jumper( Board* , int , int , GNode* );
void print_available_jumps( GSList* );
void print_jump( Jump* );
void print_jump_tree( GNode* );
void print_node( GNode* );
void follow_jump( GNode* );
void clear_coordinate_list( GSList* );
void print_coordinate_list( GSList* );
int same_coordinates( Coordinates* , Coordinates* );
void make_place_list( GSList** , Board* , int* );
void print_move( Move* );
void print_move_list( GSList* );
void make_jump_list( GSList** , Board* ); 
void make_move_list( GSList** , Board* , int* );
void free_move_list( GSList** );
void find_all_moves( GSList** , GameState* );

GNode* create_jump_from_tree( Jump* , GNode* );
GNode* prune_tree( GNode* );

#endif
