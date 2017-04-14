#ifndef SIMPLE_AIH
#define SIMPLE_AIH

#include <gnome.h>
#include "board.h"
#include "rand_ai.h"
#include "play.h"

#define BLACK_WEIGHT 5 
#define GREY_WEIGHT 6
#define WHITE_WEIGHT 7.5

#define OPP_DIST_ONE_WEIGHT 2.0
#define OPP_DIST_TWO_WEIGHT 1.0
#define MY_DIST_ONE_WEIGHT 1.0
#define MY_DIST_TWO_WEIGHT 0.5

#define SPLIT_PENALTY 0.5


float eval_game_state( GameState* , int , int );
float find_simple_move( Move* , GameState* , int , int );
int is_interesting_move( Move* , GameState* );
int is_interesting_position( Board* );
int distance_from_victory( int* , int );


#endif
