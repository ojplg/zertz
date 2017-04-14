#include "main.h"

/*
 * Sets values to -1 so that everything is
 * recognizably empty. 
 */
void initialize_move
( Move* move )
{
	move->type=-1;
	move->place_color=-1;
	move->place_column=-1;
	move->place_row=-1;
	move->remove_column=-1;
	move->remove_row=-1;

	move->jump = NULL;
	
} /* initialize_move() */

/* Free move struct pointer, and any chained jumps */
void
free_move( Move* move )
{	
	free_chained_jumps(move->jump);
	free(move);

} /* free_move */

/* Recursively free chained jumps in a Move struct */
void free_chained_jumps
( Jump* jump )
{
	if ( jump!=NULL ) {
		if ( jump->multi != NULL ) {
			free_chained_jumps( jump->multi );
		}
		free( jump ); /* free this pointer */
	}

} /* free_chained_jumps() */


void move_add_jump
( Move* move, int start_col, int start_row, int end_col, int end_row )
{
	Jump *newjump, *cur;

	/* allocate and add a new Jump* to the jump chain */
	newjump = malloc( sizeof(Jump) );
	newjump->start_column = start_col;
	newjump->start_row = start_row;
	newjump->end_column = end_col;
	newjump->end_row = end_row;
	newjump->multi = NULL;
	
	/* find end of jump chain and append newjump */
	if ( move->jump != NULL ) {
		cur = move->jump;
		while( cur->multi != NULL ) {
			cur = cur->multi;
		}
		cur->multi = newjump;
	} else {
		move->jump = newjump;
	}
	move->type = MOVE_TYPE_JUMP;

} /* move_add_jump() */


void move_set_place_data
( Move* move, int color, int column, int row )
{
	move->place_color = color;
	move->place_column = column;
	move->place_row = row;
	move->type = MOVE_TYPE_PLACE_AND_REMOVE;

} /* move_set_place_data() */


void move_set_remove_data
( Move* move, int column, int row )
{
	move->remove_column = column;
	move->remove_row = row;
	move->type = MOVE_TYPE_PLACE_AND_REMOVE;

} /* move_set_remove_data() */

/*
 * Makes a copy of a jump.
 */
int copy_jump
( Jump* original , Jump* copy )
{
	int i;
	Jump* next;
	
	copy->start_column = original->start_column;
	copy->start_row = original->start_row;
	copy->end_column = original->end_column;
	copy->end_row = original->end_row;
	copy->multi = NULL;
	
	i=1;
	next = original->multi;
	
	while( next != NULL ){
			
		copy->multi = (Jump*) malloc( sizeof( Jump ) );
		copy = copy->multi;
		
		copy->start_column = next->start_column;
		copy->start_row = next->start_row;
		copy->end_column = next->end_column;
		copy->end_row = next->end_row;
		copy->multi = NULL;

		next = next->multi;
		i++;
	}
	
	return i;	
}

/* Dump move to stdout */
void
dump_move( Move* m )
{
	Jump* cur;
	switch (m->type) {
	case MOVE_TYPE_PLACE_AND_REMOVE:
		printf("MOVE> (P&R) place=(color=%d, pos=(%d,%d)) remove=(%d,%d)\n",
				 m->place_color, m->place_column, m->place_row,
				 m->remove_column, m->remove_row);
		break;
	case MOVE_TYPE_JUMP:
		printf("MOVE> (JMP) ");
		for(cur=m->jump; cur!=NULL; cur=cur->multi) {
			printf("[(%d,%d)-(%d,%d)] ",
					 cur->start_column, cur->start_row,
					 cur->end_column, cur->end_row);
		}
		printf("\n");
		break;
	default:
		printf("Augh, unknown move type for dump_move()!\n");
	}

} /* dump_move */


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
