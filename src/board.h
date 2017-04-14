#ifndef BOARDH
#define BOARDH

#include <stdio.h>
#include <stdlib.h>
#include <gnome.h>
#include "move.h"

#define WIDTH 7

#define VOID 0
#define EMPTY 1
#define REMOVEABLE 2
#define BLACK 3
#define GREY 4
#define WHITE 5

#ifndef TRUE
# define TRUE 1
#endif
#ifndef FALSE
# define FALSE 0
#endif
#define WHITE_COUNT 0
#define GREY_COUNT 1
#define BLACK_COUNT 2

#define BEGINNER 0
#define TOURNAMENT 1

#define BEGIN_WHITE_GOAL 3
#define BEGIN_GREY_GOAL 4
#define BEGIN_BLACK_GOAL 5
#define BEGIN_ALL_GOAL 2

#define BEGIN_WHITE_AVAIL 5
#define BEGIN_GREY_AVAIL 7
#define BEGIN_BLACK_AVAIL 9

#define TOURN_WHITE_GOAL 4
#define TOURN_GREY_GOAL 5
#define TOURN_BLACK_GOAL 6
#define TOURN_ALL_GOAL 3

#define TOURN_WHITE_AVAIL 6
#define TOURN_GREY_AVAIL 8
#define TOURN_BLACK_AVAIL 10

/*
 * Island update flags.	
 */
#define REMOVE_MARBLE 45
#define ADD_MARBLE 251
#define REMOVE_DISC 386

typedef struct {
	int square[WIDTH][WIDTH];
} Board;

typedef struct {
	int numberDiscs;
	int numberFilled;

	Board* map;
} Island;

typedef struct {
	int place[6];
} Neighbors;

typedef struct {
	int x;
	int y;
} Coordinates;

typedef struct{
	Coordinates place[6];
} Neighborhood;

typedef struct GameState {
	Board position;
	GSList* islands;
	int available[3];
	int one[3];
	int two[3];
	int mover; /* the player who has the next move */
	/* USE EITHER PLAYER_ONE or PLAYER_TWO macros */
} GameState;

typedef struct Turn {
	Move move;  // this is the move
	GameState state; // this is the position after the move 
} Turn;


void print_board( Board* );
void init_board( Board* );
void copy_board( Board* , Board* );
char decode_square( int );
void place_marble( int , Board* , int , int );
int remove_marble(  Board* , int , int );
void remove_disc( Board* , int , int );
int is_removeable( Board* , int , int );
void set_removeable_discs( Board* );
int has_removeable_disc ( Board* );
int get_disc_contents( Board* , int , int );
int can_accept_marble( Board* , int , int );
void get_neighbors( Neighbors* , Board* , int , int );
int get_neighbor( Board* , int , int , int );
int has_jump( Board* , int, int );
int can_jump_to( Board* , int , int , int , int );
int is_marbled( int );
void get_neighbor_coordinates( Coordinates* , int , Coordinates* );
int will_cause_split( Board* , int , int );
int is_in_island( Island* , int , int );
int jump_available( Board* );
int split_available( Board* );
void get_middle_coordinates( Coordinates* , Coordinates* , Coordinates* );
int has_middle_coordinates( int , int , int , int );
void set_island( Board* , Island* , int , int );
int find_test_square( Island* , int* , int* );
void mark_neighbors( Island* , Coordinates* , Board* );
void set_split_neighbors( Board* , int , int , int* , int* , int* , int* );
void mark_island( int , Island* , int , int );
void print_island( Island* );
int is_alive( Island* );

void clean_up_dead_island( Island* , Board* );
void deal_with_split( GSList** , Board* , int , int );
void update_islands( GSList* , int , int , int );
Island* is_already_in_island( GSList* , int , int );
GSList* seek_bad_islands( GSList** , Board* );
void print_islands( GSList* );

void copy_island( Island* , Island* );
void copy_islands( GSList* , GSList** );
void set_coordinates_from_jump( Jump* , Coordinates* , Coordinates* , Coordinates* );


#endif
