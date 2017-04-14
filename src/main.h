#ifndef MAINH
#define MAINH

#include <gnome.h>
#include "board.h"
#include "marble.h"
#include "rand_ai.h"
#include "recording.h"
#include "prefs.h"
#include "gnet.h"
#include "zertz_net.h"
#include "simple_ai.h"
#include "play.h"

// some color definitions
#define BLACK_MARBLE 0x000000FF
#define GREY_MARBLE 0x666666FF
#define WHITE_MARBLE 0xFFFFFFFF
#define BACKGROUND 0x006400FF
#define DISC 0x4876FFFF

#define NUMBER_BLACK ( (TOURNAMENT==level) ? TOURN_BLACK_AVAIL : BEGIN_BLACK_AVAIL )
#define NUMBER_GREY ( (TOURNAMENT==level) ? TOURN_GREY_AVAIL : BEGIN_GREY_AVAIL )
#define NUMBER_WHITE ( (TOURNAMENT==level) ? TOURN_WHITE_AVAIL : BEGIN_WHITE_AVAIL )

typedef struct GtkBoard {
	GnomeCanvasGroup* discs[WIDTH][WIDTH];
	Marble* white[TOURN_WHITE_AVAIL];
	Marble* grey[TOURN_GREY_AVAIL];
	Marble* black[TOURN_BLACK_AVAIL];
} Gtk_Board;

typedef struct Flasher {
	int count;
	int column;
	int row;
} Flasher;


void examine_user_options();

gint delete_event_cb( GtkWidget* , GdkEventAny* , gpointer );
gint undo_event_cb( GtkWidget* , GdkEventAny* , gpointer );
void about_cb( GtkWidget* , gpointer );
void draw_discs( Board* );
float calculate_x_position( int , int );
float calculate_y_position( int , int );
int calculate_row( float , float );
int calculate_column( float , float );
void draw_available_marbles();
gint marble_event_cb( GnomeCanvasItem* marble_image , GdkEvent* event , gpointer data );
void center_marble( Marble* , int , int );
void replace_marble( Marble* );
gint disc_event_cb( GnomeCanvasItem* disc , GdkEvent* event , gpointer data );
void mark_jumper_marbles();
void unmark_jumper_marbles();
int is_moveable_marble( Marble* );

void draw_disc( int , int );

gint board_signal( GnomeCanvas* , GdkEvent* , gpointer );
void put_marble_in_bin( Marble* , int );

void move_jumped_marble( int , int , int , int , int );

void display_jump_move( Move* );
void display_plain_move( Move* );

void gui_board_to_null();
void discs_to_null();
void marbles_to_null();
void destroy_discs();
void destroy_board();
void destroy_marbles();
void begin_game();
void next_gui_stage();
void advance_player_turn();
void remove_island( Island* );
void new_game();
void draw_board_accoutrements();
void draw_goal_markers();

void set_message( char* );
int marbles_available();
void slide_marble_to_bin( Marble* , int );
void decrement_local_score( int , int );
void increment_local_score( int , int );
void shuffle_won_marbles( int );

Marble* get_marble_from_canvas( GnomeCanvasItem* item );
Marble* get_marble_from_board( int column , int row );

void do_ai_move();
void do_network_move();
Marble* grab_available_marble( int );
Marble* grab_captured_marble( int , int );
Marble* get_next_marble( Marble* );

void get_marble_count( char , int* , int* , int* );
void restore_marbles( GameState* );
void restore_discs( Board* );
void free_turn_list();

gint increment_slide( gpointer data );
void slide_marble( Marble* marble , double end_x , double end_y );
void slide_center_marble( Marble* marble , int column , int row );

int find_item_row( GnomeCanvasItem* item );
int find_item_column( GnomeCanvasItem* item );

void take_off_disc( int column , int row );
gint flash_disc( gpointer data );

void init_net_play();

// definitions for various magic number coordinates

#define CANVAS_HEIGHT 400.0
#define CANVAS_WIDTH 400.0

// These definitions are for drawing the various bins
#define UNUSED_BIN_LEFT_X 280.0
#define UNUSED_BIN_RIGHT_X 380.0
#define UNUSED_BIN_TOP_Y 5.0
#define UNUSED_BIN_BOTTOM_Y 250.0

#define PLAYER1_BIN_LEFT_X 10.0
#define PLAYER1_BIN_RIGHT_X 180.0 
#define PLAYER1_BIN_TOP_Y 250.0
#define PLAYER1_BIN_BOTTOM_Y 380.0

#define PLAYER2_BIN_LEFT_X 210.0
#define PLAYER2_BIN_RIGHT_X 380.0
#define PLAYER2_BIN_TOP_Y 250.0
#define PLAYER2_BIN_BOTTOM_Y 380.0

#define BIN_WIDTH 2

#define PLAYER1_TAG "Player One Bin"
#define PLAYER1_TAG_X 80.0
#define PLAYER1_TAG_Y 260.0
#define PLAYER2_TAG "Player Two Bin"
#define PLAYER2_TAG_X 270.0
#define PLAYER2_TAG_Y 260.0
#define UNUSED_TAG "Unplayed Bin"
#define UNUSED_TAG_X 330.0 
#define UNUSED_TAG_Y 15.0

#define TAG_FONT "-*-clean-bold-r-normal-*-16-*-*-*-*-*-*"

// These definitions are for drawing the discs and marbles
#define OUTER_DISC_WIDTH 30.0
#define INNER_DISC_WIDTH 12.0

#define BASE_X_OFFSET 20.0 // how far the discs are from the left edge
#define BASE_Y_OFFSET 20.0 // how far from the top edge
#define UNIT_X_OFFSET 32.0 // center-to-center measure between discs horizontally
#define UNIT_Y_OFFSET 29.0 // center-to-center measure between discs vertically

#define MARBLE_WIDTH 18.0
#define BASE_MARBLE_X_OFFSET 300.0 // distance from top of the initial marble array
#define BASE_MARBLE_Y_OFFSET 30.0 // distance from left of the initial marble array
#define MARBLE_SPACE 22.0 // center-to-center marble distance
#define MARBLE_BORDER_WIDTH 1

// These definitions are for putting the marbles in the players' bins
#define PLAYER2_X_OFFSET 200.0
#define BASE_BIN_X_OFFSET 22.0
#define BASE_BIN_Y_OFFSET 280.0
#define UNIT_BIN_X_OFFSET 25.0
#define UNIT_BIN_Y_OFFSET 28.0



#endif
