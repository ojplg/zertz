#ifndef __PLAYER_H
#define __PLAYER_H

/*
 * Defines structure encompassing the data for a single Zertz player.
 */


/* Indexes to the players[] array in main.c */
#define PLAYER_ONE 0
#define PLAYER_TWO 1

#define TOGGLE(A) ((A==0)?1:0) 


typedef struct {
	int type; /* kind of player this is: USE_GUI, USE_AI, USE_NET */
	int ai_type; /* if the player is USE_AI, this flag should be set. */
	char * type_data; /* additional data depending on type
							 * eg, address for USE_NET */
	/* type_data is UNUSED!  The address for USE_NET
	 * is kept in main.c in remote_addr.  There is no
	 * need for it here, since we only allow one
	 * net player anyway!
	 */
	int captured[3]; /* captured marbles of each color */

} Zertz_Player;

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
