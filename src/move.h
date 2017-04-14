#ifndef __MOVE_H
#define __MOVE_H


/* Move types for Move struct: */
enum move_type {
	MOVE_TYPE_JUMP,
	MOVE_TYPE_PLACE_AND_REMOVE
};


/* Struct for storing jump (and/or a list of jumps) */
typedef struct Jump {
	int start_column, start_row;
	int end_column, end_row;
	/* In case the landing place of this jump allows multiple jumps: */
	struct Jump* multi;

} Jump;


/* Struct for a single move:
 * either a jump, or a place-and-remove
 */
typedef struct {
	enum move_type type;
	
	int place_color;
	int place_column, place_row;
	int remove_column, remove_row;

	Jump* jump;

} Move;

void initialize_move( Move* );
void free_move( Move* move );
void free_chained_jumps( Jump* jump );
/* these also set move->type appropriately */
void move_add_jump(Move* move, int start_c, int start_r, int end_c, int end_r);
void move_set_remove_data( Move* move, int column, int row );
void move_set_place_data( Move* move, int color, int column, int row );
void dump_move( Move* );
int copy_jump( Jump* , Jump* );

#endif


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
