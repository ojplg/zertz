#include "marble.h"
#include "board.h"

void set_marble
( Marble* marble , GnomeCanvasItem* image , int color , int number )
{
	marble->image = image;
	marble->number = number;
	marble->color = color;
	
	marble->status = AVAILABLE;
	marble->row = -1;
	marble->column = -1;
	marble->place = -1;
	marble->sliding = FALSE;
	marble->slide_request_queue = NULL;	
}

void add_slide_request
( Marble* marble , Slider* slider )
{
	marble->slide_request_queue = g_slist_append( 
			marble->slide_request_queue , slider );
}

Slider* get_next_slide( Marble* marble )
{
	Slider* slider;

	if( marble->slide_request_queue == NULL ){
		return NULL;
	}

	slider = (Slider*) marble->slide_request_queue->data;
	marble->slide_request_queue = g_slist_remove( marble->slide_request_queue , slider );
	return slider;
}

int can_slide( Marble* marble )
{
	if( marble->sliding == TRUE ){
		return FALSE;
	}
	return TRUE;
}

void block_sliding( Marble* marble )
{
	marble->sliding = TRUE;
}

void done_sliding( Marble* marble )
{
	marble->sliding = FALSE;
}

int get_marble_row( Marble* marble )
{
	return marble->row;
}

int set_marble_row( Marble* marble , int row )
{
	if( row<-1 || row> WIDTH ){
		return FALSE;
	}
	else {
		marble->row = row;
		return TRUE;
	}
}

int get_marble_column( Marble* marble )
{
	return marble->column;
}

int set_marble_column( Marble* marble , int column )
{
	if( column<-1 || column> WIDTH ){
		return FALSE;
	}
	else {
		marble->column = column;
		return TRUE;
	}
}

int get_marble_color( Marble* marble )
{
	return marble->color;
}

int set_marble_color( Marble* marble , int color )
{
	if( ! (color==WHITE || color==GREY || color==BLACK) ){
		return FALSE;
	}
	else {
		marble->color = color;
		return TRUE;
	}
}

int get_marble_status( Marble* marble )
{
	return marble->status;
}

int set_marble_status( Marble* marble , int status )
{
	if( ! (status==AVAILABLE || status==PLACED || status==JUMPER 
				|| status==ONE || status==TWO ) ){
		return FALSE;
	}
	else {
		marble->status = status;
		return TRUE;
	}
}

int get_marble_place( Marble* marble )
{
	return marble->place;
}

int set_marble_place( Marble* marble , int place )
{
	if( place<0 ){
		return FALSE;
	}
	else {
		marble->place = place;
		return TRUE;
	}
}

void free_marble( Marble* marble )
{
	gtk_object_destroy( GTK_OBJECT( marble->image ) );
	free( marble );
}

int get_marble_number( Marble* marble )
{
	return marble->number;
}


