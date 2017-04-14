#ifndef MARBLE_H
#define MARBLE_H

#include <gnome.h>

#define AVAILABLE 1
#define PLACED 2
#define JUMPER 3
#define ONE 4
#define TWO 5

/* 
 * The Marble struct contains a pointer
 * to the Canvas Item that represents
 * the marble and a wealth of data about
 * the marble, its location, status, etc.
 *
 * The CanvasItem contains a pointer to 
 * this struct.  So, things are reflective.
 */
typedef struct Marble{
	GnomeCanvasItem* image;

	// either AVAILABLE, PLACED, JUMPER, ONE, or TWO
	int status;

	// set when on the board, else -1
	int row;
	int column;
	
	// unique permanent identifiers
	// for this marble
	int color;
	int number;
	
	// -1 until captured
	// then represents position in the
	// player's bin	
	int place;	

	// either TRUE or FALSE
	// allows the marble to block
	// movement requests if it needs to
	int sliding;

	// if the marble is sliding, and needs
	// to slide again, add a request
	// to the queue
	GSList* slide_request_queue;
} Marble;

typedef struct Slider{
	Marble* marble;
	double start_x;
	double start_y;
	double end_x;
	double end_y;
} Slider;

void set_marble( Marble* marble , GnomeCanvasItem* image , int color , int number );
void free_marble( Marble* marble );

void block_sliding( Marble* marble );
void done_sliding( Marble* marble );
int can_slide( Marble* marble );
void add_slide_request( Marble* marble , Slider* slider );
Slider* get_next_slide( Marble* marble );

// various getters and setters
// All return false if passed a bad value
int get_marble_row( Marble* marble );
int set_marble_row( Marble* marble , int row );
int get_marble_column( Marble* marble );
int set_marble_column( Marble* marble , int column );
int get_marble_color( Marble* marble );
int set_marble_color( Marble* marble , int color );
int get_marble_status( Marble* marble );
int set_marble_status( Marble* marble , int status );
int get_marble_place( Marble* marble );
int set_marble_place( Marble* marble , int place );
int get_marble_number( Marble* marble );

#endif
