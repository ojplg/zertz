#include "main.h"
#include "move.h"
#include "marble.h"
#include "player.h"
#include "net_to_move.h"

#include "gnet.h"
#include <limits.h> /* PATH_MAX */

#include <stdio.h>  /* stdio included for perror only */
#include <unistd.h> /* unistd: read, close, write */
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>


/*
 * gotta have some global variables
 */

// this is the top level window.
GtkWidget* window;

// an appbar for displaying messages
GnomeAppBar* appbar;

// a canvas to put in the window.
GtkWidget* canvas;

// this is the current state of the game
GameState game_state;
/*
 * The GameState struct contains a Board struct,
 * a linked list to track the islands, the available
 * marble count, the score of each of the players,
 * and a flag to show who is on the move.
 */ 

/*
 * The Zertz_Player structs contain player
 * settings and keep track of the player's
 * score.  Done a tiny bit differently than
 * in the GameState struct.  A bit redundant,
 * but it is a trick that allows marbles to 
 * be moved into the bin easily.  Not ideal. 
 */
Zertz_Player players[2];
#define PL_ONE (players[0])
#define PL_TWO (players[1])
#define PL_CURRENT (players[game_state.mover])

char* remote_addr = NULL;

/*
	These variables define the state that the
	game is in at any particular moment.
*/
int turn_stage; // either PLACING, REMOVING, or JUMPING
int game_mode; // either STOP, USE_GUI, USE_AI, or USE_NET

// so that the move being played at the console
// can be collected
Move* current_move = NULL;

// this is the graphical board
Gtk_Board* gui_board = NULL;

// This is a local copy of the board.
// It is needed to validate and accept
// Moves made on the GUI
Board gui_position;

// the game level, i.e. BEGINNER or TOURNAMENT
int level;

// a record of the moves of the game
GList* turn_list = NULL;

// stream to write game record to
record_stream* game_rec_stream = NULL;

// some other misc net play vars
int ima_netplay_server; /* Am I a server for net play (or a client)? */
int use_net; /* true if either player is a net player */
int net_play_port;
// and, sockets for net play comms
gnet_socket net_play_gnsock, net_play_server_gnsock;

// this key is used to connect the image
// of the marble drawn in the canvas to
// its Marble struct
const char* MARBLE_KEY = "marble_key";

// for displaying goal circles.
GnomeCanvasItem* goal_markers = NULL;
// flag about whether or not to draw them
int display_markers;

/*
 * These static initializers generate the menus.
 */
static GnomeUIInfo game_menu[]={
	GNOMEUIINFO_MENU_NEW_GAME_ITEM( 
			new_game ,
			NULL ) ,
	GNOMEUIINFO_SEPARATOR , 
	GNOMEUIINFO_MENU_UNDO_ITEM(
			undo_event_cb ,
			NULL ) ,
	GNOMEUIINFO_SEPARATOR , 
	GNOMEUIINFO_ITEM( 
			"Net Connect" ,
			"Connect to another zertz player.",
			init_net_play ,
			NULL ) ,
	GNOMEUIINFO_SEPARATOR , 
	GNOMEUIINFO_MENU_EXIT_ITEM( 
			delete_event_cb ,
			NULL ) ,
	GNOMEUIINFO_END
};

static GnomeUIInfo prefs_menu[]={
	GNOMEUIINFO_MENU_PREFERENCES_ITEM( prefs_cb , NULL ),
	GNOMEUIINFO_END
};

static GnomeUIInfo help_menu[]={
	/* GNOMEUIINFO_HELP("./help/C"), */
	GNOMEUIINFO_MENU_ABOUT_ITEM( about_cb, NULL ),
	GNOMEUIINFO_END
};

static GnomeUIInfo main_menu[]={
	GNOMEUIINFO_MENU_GAME_TREE( game_menu ) ,
	GNOMEUIINFO_MENU_SETTINGS_TREE( prefs_menu ) ,
	GNOMEUIINFO_MENU_HELP_TREE( help_menu ),
	GNOMEUIINFO_END
};

/* gdf: Options table for parsing command line: */
static char* opt_p1;
static char* opt_p2;
static int   opt_port = 0;
static int   opt_server; /* force being the game server */
static int   opt_save_prefs = 0;
/* not using poptContext here (yet?) */
static struct poptOption optTable[] =
	{
		/* (a number of options, including --help, are supplied by gnome-init) */
		{"player1", '1', POPT_ARG_STRING, &opt_p1, 0,
		 "Type of player to use for Player 1", "{human,ai,<IP>}"},

		{"player2", '2', POPT_ARG_STRING, &opt_p2, 0,
		 "Type of player to use for Player 2", "{human,ai,<IP>}"},

		{"port", 'P', POPT_ARG_INT, &opt_port, 0,
		 "For net play: port to use", "PORTNUM"},

		{"server", 's', POPT_ARG_NONE, &opt_server, 0,
		 "For net play: be the game server (should be auto-detected)", NULL},

		{"save-prefs", '\0', POPT_ARG_NONE, &opt_save_prefs, 0,
		 "Save options given on command line to Gnome preferences (default is to simply override)", NULL},
                
		{NULL,'\0',0,NULL,0,NULL,NULL} /* end opts table */
	};

int main
( int argc , char* argv[] )
{
	/* "flags is not currently used, but should be specified as 0" */
	gnome_init_with_popt_table( "Zertz" , VERSION , argc , argv,
										 optTable, 0, NULL );
	gdk_rgb_init(); // dunno what does

	/* Check stored preferences, and commandline options */
	load_preferences();
	examine_user_options(); /* possibly override prefs */

	// make the application and title it	
	window = gnome_app_new( "zertz" , "Zertz" );

	// put the little bar on the bottom
	appbar = GNOME_APPBAR ( gnome_appbar_new( FALSE , TRUE , GNOME_PREFERENCES_NEVER ) );
	gnome_app_set_statusbar(GNOME_APP(window), GTK_WIDGET (appbar));
	gnome_appbar_push( appbar , "Welcome to Gnome Zertz." );
	
	// tell the window what to do when closed
	gtk_signal_connect( GTK_OBJECT(window) ,
			"delete_event" ,
			GTK_SIGNAL_FUNC( delete_event_cb ),
			NULL );

	// add the menu
	gnome_app_create_menus_with_data( GNOME_APP( window ) , main_menu , window );
		
	// make the canvas
	canvas = gnome_canvas_new_aa();
	gnome_canvas_set_scroll_region( GNOME_CANVAS(canvas) , 0 , 0 , 400 , 400 );
	
	// put the canvas into the window
	gnome_app_set_contents( GNOME_APP(window) , (GtkWidget*) canvas );
	
	// size and show the window
	gtk_window_set_default_size(GTK_WINDOW(window) , 400 , 453 );
	gtk_widget_show_all( window );

	draw_board_accoutrements();
	gui_board_to_null();

	net_play_gnsock.sock_fd = -1; // this is lame
	gtk_main();
	
	return 0;
}

/*
 * Look at the cmdline opts the user gave, and set the appropriate things.
 */
void examine_user_options()
{
	int i;
	char* opt[2];

	/* Is user forcing server mode?  Check this before --playerX opts. */
	ima_netplay_server = opt_server;

	/* Deal with --player1 and --player2:
	 * Use a for loop cause I'm so lazy.
	 * Allowable values are HUMAN, AI, NET, or a hostname.
	 * Defaults are taken from the loaded app prefs.
	 */
	opt[0] = opt_p1;	
	opt[1] = opt_p2;
	for (i=0; i<2; i++) {
		/* if no cmdline opt was given, leave as the defaults */
		if (opt[i] != NULL) {
			if (!strncasecmp(opt[i], "HUMAN", strlen("HUMAN"))) {
				players[i].type = USE_GUI;
			} else if (!strncasecmp(opt[i], "AI", strlen("AI"))) {
				players[i].type = USE_AI;
			} else if (!strncasecmp(opt[i], "NET", strlen("NET"))) {
				/* net play server for unspecified client */
				players[i].type = USE_NET;
				ima_netplay_server = 1;
			} else {
				/* any other value we take as a hostname for net play... */
				/* save it in the type_data field */
				printf("Expecting player %d to be at '%s'\n", i+1, opt[i]);
				players[i].type = USE_NET;
				//players[i].type_data = calloc( strlen(opt[i])+1, 1 );
				//strncpy( players[i].type_data, opt[i], strlen(opt[i]) );
				if( remote_addr != NULL ){
					free( remote_addr );
				}
				remote_addr = calloc( strlen(opt[i])+1, 1 );
				strncpy( remote_addr, opt[i], strlen(opt[i]) );
			}
		}
	}

	if ( (PL_ONE.type == USE_NET) && (PL_TWO.type == USE_NET) ) {
		// this is SUPER WRONG!!! FIXME  Put up a blocking dialog
		// or anything just NOT this
		g_error("Sorry, P1 and P2 can't both be net players.");
	}

	/* XXX gdf net_play_port should be included in preferences */
	if ( opt_port ) {
		net_play_port = opt_port;
	} else {
		net_play_port = ZN_DEFAULT_PORT;
	}

 	/// XXX if ( opt_save_prefs ) { ???how? }

} /* examine_user_options() */


/*
 * Given the initialized player data (prefs and cmdline opts),
 * attempt to set up a network connection for net play.
 * May abort program, if there is a unresolvable connection problem.
 * (Does nothing if neither player is USE_NET)
 */
void init_net_play()
{
	zertznet_message msg;

	if ( net_play_gnsock.sock_fd != -1 ) {
		gnet_socket_close(&net_play_gnsock);
		if (ima_netplay_server) {
			gnet_socket_close(&net_play_server_gnsock);
		}
	}
	
	use_net = (PL_ONE.type==USE_NET || PL_TWO.type==USE_NET);

	/* only one of P1,P2 may be a net player */
	if ( ! use_net ) {
		ima_netplay_server = 0; // just in case --server was given
		return; /* do nothing if no net players */
	}

	/* first, assume we want to be the client */
	if ( ! ima_netplay_server ) {
		printf("Attempting to connect to '%s' port %d...\n",
				 remote_addr, net_play_port);
		if ( get_socket_net_servant( remote_addr, net_play_port,
											  &net_play_gnsock ) < 0 ) {
			/* No server to connect to?  Fine: we'll try to be the server. */
			printf("Couldn't connect to '%s' port %d, will try being server...\n",
					 remote_addr, net_play_port);
			ima_netplay_server = 1;
		} else {
			printf("Connected to %s:%d\n", remote_addr, net_play_port);
			use_net = 1;
		}
	}

	if ( ima_netplay_server ) {
		printf("Attempting to set up master socket on port %d...\n", net_play_port);
		if ( get_socket_net_master( net_play_port, &net_play_server_gnsock )
			  < 0 ) {
			printf("Can't get listen socket (port %d) to be game server: %s\n",
					 net_play_port, strerror(errno));
			exit(-1);
		} else {
			printf("Listening as net play server on port %d\n", net_play_port);
			use_net = 1;
		}
		/* accept client connection */
		if ( gnet_socket_accept(&net_play_server_gnsock,&net_play_gnsock)
			  < 0 ) {
			printf("Error accepting on socket.  Argh!\n");
			exit(-1);
		}
	}

	/* XXX we should probably assure, here, that the other end
	 * also thinks it's the appropriate player (one/two) */
	/* send and recieve the protocol greeting to verify connection */
	zn_makemsg_greeting( &msg, ZERTZ_NET_PROTO_VERSION );
	printf("Sending proto greetzz...\n");
	if ( zn_sendmsg( &net_play_gnsock, &msg ) < 0 ) {
		printf("Error sending zertznet protocol greeting message!\n");
		exit(-1);
	}
	if ( zn_recvmsg( &net_play_gnsock, &msg ) < 0 ) {
		printf("Error reading zertznet protocol greeting message!\n");
		exit(-1);
	}
	zn_dumpmsg( stdout, &msg );
	if (msg.msgtype != ZN_MSG_GREETING) {
		printf("Recieved a bad greeting message (is the other end really playing Zertz?)\n");
		exit(-1);
	}
	/* Here we should verify matching protocol versions and game types. */
	/* Probably, the client side should accept the server settings and reset? */

} /* init_net_play() */



/*
 * Standard function for the about box.
 * What can I say?
 */
void about_cb
( GtkWidget* menu_item , gpointer data )
{
	GtkWidget* about;
	const gchar* authors[] = {
		"Oliver Gugenheim" ,
		"Greg Fast",
		NULL
	};
	
	const gchar* documentors[] = {
		"Oliver Gugenheim" ,
		NULL 
	};
	
	about = gnome_about_new(
			_("Gnome Zertz") , 
			VERSION ,
			"(C) 2002 Oliver Gugenheim" ,
			"no comment",
			authors ,
			documentors ,
			"no credits" ,
			NULL );
	/*
			_("This is a Gnome Client for the Zertz boardgame. "
			"Zertz is part of the GIPF project. More information "
			"about the GIPF project, the official rules to Zertz, "
			"and other information is available at www.gipf.com. "
			"Zertz is the invention of Kris Burm.  The name Zertz "
			"is the Trademark and Copyright of Don & Co NV." ),
			NULL);
			*/

	gtk_widget_show( GTK_WIDGET( about ) );	
}

/*
 * Opens a new file for recording moves.
 * Etc.
 */
void new_game()
{
	char rec_file_name[PATH_MAX];
	int home_player;
	time_t timer;
	zertznet_message send_msg;
	zertznet_message rec_msg;
	
	// seed the random number generator
	time( &timer );
	srand( timer );
	
	// networking stuff
	use_net = (PL_ONE.type==USE_NET || PL_TWO.type==USE_NET);

	if( use_net ){
		printf("This is a network game.\n");

		if( net_play_gnsock.sock_fd == -1 ){
			printf("CANNOT START A NETWORK GAME UNTIL CONNECTION IS ESTABLISHED!\n");
			return;
		}
		
		home_player = PL_ONE.type == USE_NET ? PLAYER_TWO : PLAYER_ONE ;
	
			// do stuff
			printf("Sending new game message\n");
			zn_makemsg_newgame( &send_msg, level , home_player );
			if ( zn_sendmsg( &net_play_gnsock, &send_msg ) < 0 ) {
				printf("Error sending zertznet new game message!\n");
				exit(-1);
			}	
			// do other stuff
			printf("Waiting for new game message\n");
			zn_recvmsg( &net_play_gnsock , &rec_msg ); 
			if( rec_msg.msgtype != ZN_MSG_NEW_GAME ){
				printf("Received some goofy message\n");
				return;
			}
			if( level != rec_msg.data.newgame.level ){
				printf("GRR.  Players are not agreed on what level.");
				return;
			}
			if( home_player == rec_msg.data.newgame.player_num ){
				printf("GRR.  Both players are trying to be %d\n",home_player);
				return;
			}
	}
	

	/* gdf - stop recording (if we are), start recording */
	close_record_file(& game_rec_stream ); /* sets game_rec_stream to NULL */
	if ( get_record_file_name("/var/tmp", 1024, rec_file_name, PATH_MAX) < 0 ) {
		fprintf(stderr, "new_game: Cannot figure out new record file name?\n");
	} 
	else {
		printf("new_game: Recording game in '%s'\n", rec_file_name);
		game_rec_stream = open_record_file(rec_file_name);
	}
	if ( game_rec_stream == NULL ) {
		fprintf(stderr, "new_game: Cannot open record file.  I die.\n");
		exit(-1); /* record_*() calls on NULL will segv, so exit clean here */
	}

	/* keep track of the current move */
	if ( current_move != NULL ) {
		free_move(current_move);
	}
	current_move = (Move*)malloc(sizeof(Move));
	initialize_move(current_move);
	current_move->type = MOVE_TYPE_PLACE_AND_REMOVE;

	begin_game();
}

/*
 * Sets the game state, the score to
 * zero, etc. 
 */
void begin_game()
{
	
	destroy_board();
	
	turn_stage = PLACING;
	game_mode = PL_ONE.type;
	set_up_new_game( &game_state , level );
	init_board( &gui_position );
	
	// set the captured to zeroes
	PL_ONE.captured[0]=0;
	PL_ONE.captured[1]=0;
	PL_ONE.captured[2]=0;
	PL_TWO.captured[0]=0;
	PL_TWO.captured[1]=0;
	PL_TWO.captured[2]=0;

	free_turn_list();
	
	if( display_markers == TRUE ){
		draw_goal_markers();
	}
	else {
		if( goal_markers != NULL ){
			gtk_object_destroy( GTK_OBJECT( goal_markers ) );
			goal_markers = NULL;
		}
	}
			
	draw_discs( &(game_state.position) );
	draw_available_marbles();
	gnome_appbar_clear_stack( appbar );
	set_message( "Place a marble." );

	if( PL_ONE.type == USE_AI ){
		printf("doing first turn with AI\n");
		do_ai_move();

	} else if ( PL_ONE.type == USE_NET ) {
		printf("doing first turn from the net...  waiting for first turn...\n");
		set_message("Waiting for first turn from network...");
		do_network_move();
	}

	gtk_widget_hide_all( canvas );
	gtk_widget_show_all( canvas );
	
}

/*
 * Responds to window closes.
 */ 
gint delete_event_cb
( GtkWidget* window , GdkEventAny* e , gpointer data)
{
	/* gdf - stop recording (if we are) */
	close_record_file( &game_rec_stream );
	/* close net play sockets */
	if (net_play_gnsock.sock_fd!=-1) {
		gnet_socket_close(&net_play_gnsock);
		if (ima_netplay_server) {
			gnet_socket_close(&net_play_server_gnsock);
		}
	}		
	gtk_main_quit();
	return FALSE;
}

/*
 * Call back connected to the undo entry
 * in the game menu.
 *
 * The idea is to go back TWO turns to when
 * the player last had the move!
 */ 
gint undo_event_cb
( GtkWidget* window , GdkEventAny* e , gpointer data)
{
	Turn* turn;
	GameState state;
	Board position;
	GList* link;
	
	if( !((game_mode==USE_GUI) || (game_mode==VICTORY_ONE) || (game_mode==VICTORY_TWO) ) ){
		printf(" NO UNDO AT THIS MOMENT \n");
		return FALSE;
	}
	
	if( PL_ONE.type == USE_NET || PL_TWO.type == USE_NET ){
		printf(" NO UNDO DURING NET PLAY! \n");
		return FALSE;
	}
	
	gtk_widget_hide_all( canvas );
	
	// first simply dispose of one link
	if( turn_list == NULL ){
		new_game();
	}
	link = g_list_first( turn_list );
	turn = link->data;
	turn_list = g_list_remove( turn_list , turn );
	free_turn( turn );
	
	// now dispose of one more link
	if( turn_list == NULL ){
		new_game();
	}
	link = g_list_first( turn_list );
	turn = link->data;
	turn_list = g_list_remove( turn_list , turn );
	free_turn( turn );
	 
	if( 0 == g_list_length( turn_list ) ){
		new_game();
		return FALSE;
	}

	// now get the state that existed before those
	// two turns
	link = g_list_first( turn_list );
	turn = link->data;
	
	unmark_jumper_marbles();
	
	state = turn->state;
	position = state.position;
	
	restore_discs( &position );
	restore_marbles( &state );

	while( gtk_events_pending() ){
		gtk_main_iteration();
	}
	
	gtk_widget_show_all( canvas );

	copy_state( &state , &game_state );
	copy_board( &(state.position) , &gui_position );
	
	// now set the GUI stage to the correct value
	if( jump_available( &(state.position) ) ){
		turn_stage = JUMPING;
		mark_jumper_marbles();
	}
	else {
		turn_stage = PLACING;
		unmark_jumper_marbles();
	}

	print_state( &game_state );
	
	game_mode = USE_GUI;
	
	return FALSE;
}

/*
 * Grabs space for the board and then
 * sets all the pointers to NULL. 
 */
void gui_board_to_null()
{
	if( gui_board != NULL ){
		free( gui_board );
	}
	gui_board = malloc( sizeof (Gtk_Board) );
	discs_to_null();
	marbles_to_null();
}

/*
 * Sets the disc pointers of the board
 * to NULL.
 */
void discs_to_null()
{
	int i,j;

	// get rid of any old discs lying around!	
	for( i=0 ; i<WIDTH ; i++ ){
		for( j=0 ; j<WIDTH ; j++ ){
			gui_board->discs[i][j] = NULL;
		}
	}
}

/*
 * Sets the marble pointers of the board
 * to NULL.
 */
void marbles_to_null()
{
	int i;

	for( i=0 ; i < TOURN_BLACK_AVAIL ; i++ ){
		gui_board->black[i] = NULL;
	}
	for( i=0 ; i < TOURN_GREY_AVAIL ; i++ ){
		gui_board->grey[i] = NULL;
	}
	for( i=0 ; i < TOURN_WHITE_AVAIL ; i++ ){
		gui_board->white[i] = NULL;
	}
}

/*
 * Destroys all the canvas objects that
 * are marbles.
 */
void destroy_marbles()
{
	int i;
	Marble* marble;

	
	for( i=0 ; i < TOURN_BLACK_AVAIL ; i++ ){
		if( ( gui_board->black[i] ) != NULL ){
			marble = gui_board->black[i];
			free_marble( marble );			
			gui_board->black[i] = NULL;
		}
	}
	for( i=0 ; i < TOURN_GREY_AVAIL ; i++ ){
		if( ( gui_board->grey[i] ) != NULL ){
			marble = gui_board->grey[i];
			free_marble( marble );			
			gui_board->grey[i] = NULL;
		}
	}
	for( i=0 ; i < TOURN_WHITE_AVAIL ; i++ ){
		if( ( gui_board->white[i] ) != NULL ){
			marble = gui_board->white[i];
			free_marble( marble );			
			gui_board->white[i] = NULL;
		}
	}
}

/*
 * Destroys all the canvas objects that
 * are discs.
 */
void destroy_discs()
{
	int i,j;

	// get rid of any old discs lying around!	
	for( i=0 ; i<WIDTH ; i++ ){
		for( j=0 ; j<WIDTH ; j++ ){
			if( ( gui_board->discs[i][j] ) != NULL ){
				gtk_object_destroy( GTK_OBJECT( gui_board->discs[i][j] ) );
				gui_board->discs[i][j] = NULL;
			}
		}
	}
}

/*
 * Redraws discs after an undo.
 */
void restore_discs
( Board* position )
{

	int i,j;
	
	for( i=0 ; i<WIDTH ; i++ ){
		for( j=0 ; j<WIDTH ; j++ ){
			if( ( position->square[i][j] != VOID ) && 
				  ( ( gui_board->discs[i][j] ) == NULL ) ){
				draw_disc( i , j );
			}
		}
	}
}

/*
 * Puts marbles back in place after
 * an undo.
 */ 
void restore_marbles
( GameState* state )
{
	int i,j;
	int color;
	Marble* marble = NULL;
	Board local_board;
	int flag = FALSE;
	
	int one_cap[3];
	int two_cap[3];
	int unused[3];
	int on_board[3] = {0,0,0};
	
	copy_board( &(state->position) , &local_board );	
	
	for( i=0 ; i<3 ; i++ ){
		one_cap[i] = state->one[i];
		two_cap[i] = state->two[i];
		unused[i] = state->available[i];
		PL_ONE.captured[i] = 0;
		PL_TWO.captured[i] = 0;
	}
	for( i=0 ; i<WIDTH ; i++ ){
		for( j=0 ; j<WIDTH ; j++ ){
			color = get_disc_contents( &local_board , i , j );
			if( is_marbled( color ) ){
				on_board[color-BLACK]++;
			}
		}
	}

	while( ( marble = get_next_marble( marble ) ) != NULL ){
		gnome_canvas_item_raise_to_top( marble->image );
		color = get_marble_color( marble );
			
		if( unused[color-BLACK] > 0 ){
			replace_marble( marble );
			set_marble_status( marble , AVAILABLE );
			set_marble_row( marble , -1 );
			set_marble_column( marble , -1 );
			set_marble_place( marble , -1 );
			unused[color-BLACK]--;
		}
		else if( one_cap[color-BLACK] > 0 ){
			put_marble_in_bin( marble , PLAYER_ONE );
			set_marble_status( marble , ONE );
			set_marble_row( marble , -1 );
			set_marble_column( marble , -1 );
			set_marble_place( marble , PL_ONE.captured[color-BLACK] );
			one_cap[color-BLACK]--;
			increment_local_score( PLAYER_ONE , color );
		}
		else if( two_cap[color-BLACK] > 0 ){
			put_marble_in_bin( marble , PLAYER_TWO );
			set_marble_status( marble , TWO );
			set_marble_row( marble , -1 );
			set_marble_column( marble , -1 );
			set_marble_place( marble , PL_TWO.captured[color-BLACK] );
			two_cap[color-BLACK]--;
			increment_local_score( PLAYER_TWO , color );
		}
		else if( on_board[color-BLACK] > 0 ){
			flag = TRUE;
			for( i=0 ; i<WIDTH ; i++ ){
				for( j=0 ; j<WIDTH ; j++ ){
					if( (flag == TRUE) && ( color == get_disc_contents( &local_board , i , j ) ) ){
						center_marble( marble , i , j );
						set_marble_status( marble , PLACED );
						set_marble_column( marble , i );
						set_marble_row( marble , j );
						set_marble_place( marble , -1 );
						remove_marble( &local_board , i , j );
						on_board[color-BLACK]--;
						flag = FALSE;
					}
				}
			}
		}
		else {
			printf("THIS MARBLE IS SCREWY %d \n", color);
		}
	}
}

/*
 * Destroys all the canvas items that
 * are marbles or discs.
 */
void destroy_board()
{
	destroy_discs();
	destroy_marbles();
}

/*
 * Draws the bins and their text.
 */ 
void draw_board_accoutrements()
{
	// make a nice green background	
	gnome_canvas_item_new( gnome_canvas_root( GNOME_CANVAS( canvas ) ) , 
			gnome_canvas_rect_get_type(),
			"x1", 0.0 ,
			"y1", 0.0 ,
			"x2", CANVAS_WIDTH ,
			"y2", CANVAS_HEIGHT ,
			"fill_color_rgba" , BACKGROUND ,
			"outline_color" , "black" ,
			"width_pixels" , 1 ,
			NULL ) ;

	// put the dividers and draw some text.
	// Player One Bin
	gnome_canvas_item_new( gnome_canvas_root( GNOME_CANVAS( canvas ) ) ,
			gnome_canvas_rect_get_type() ,
			"x1" , PLAYER1_BIN_LEFT_X ,
			"y1" , PLAYER1_BIN_TOP_Y ,
			"x2" , PLAYER1_BIN_RIGHT_X ,
			"y2" , PLAYER1_BIN_BOTTOM_Y ,
			"fill_color_rgba" , BACKGROUND ,
			"outline_color" , "black" ,
			"width_pixels" , BIN_WIDTH ,
			NULL );
	
	gnome_canvas_item_new( gnome_canvas_root( GNOME_CANVAS( canvas ) ) ,
			gnome_canvas_rect_get_type() ,
			"x1" , UNUSED_BIN_LEFT_X ,
			"y1" , UNUSED_BIN_TOP_Y ,
			"x2" , UNUSED_BIN_RIGHT_X ,
			"y2" , UNUSED_BIN_BOTTOM_Y ,
			"fill_color_rgba" , BACKGROUND ,
			"outline_color" , "black" ,
			"width_pixels" , BIN_WIDTH ,
			NULL );
	
	gnome_canvas_item_new( gnome_canvas_root( GNOME_CANVAS( canvas ) ) ,
			gnome_canvas_rect_get_type() ,
			"x1" , PLAYER2_BIN_LEFT_X ,
			"y1" , PLAYER2_BIN_TOP_Y ,
			"x2" , PLAYER2_BIN_RIGHT_X ,
			"y2" , PLAYER2_BIN_BOTTOM_Y ,
			"fill_color_rgba" , BACKGROUND ,
			"outline_color" , "black" ,
			"width_pixels" , BIN_WIDTH ,
			NULL );

	gnome_canvas_item_new( gnome_canvas_root( GNOME_CANVAS( canvas ) ) ,
			gnome_canvas_text_get_type() ,
			"x" , PLAYER1_TAG_X ,
			"y" , PLAYER1_TAG_Y ,
			"text" , PLAYER1_TAG ,
			"justification" , GTK_JUSTIFY_LEFT , 
			"fill_color" , "black" ,
			"font" , TAG_FONT , 
			NULL );
	
	gnome_canvas_item_new( gnome_canvas_root( GNOME_CANVAS( canvas ) ) ,
			gnome_canvas_text_get_type() ,
			"x" , PLAYER2_TAG_X ,
			"y" , PLAYER2_TAG_Y ,
			"text" , PLAYER2_TAG ,
			"justification" , GTK_JUSTIFY_LEFT , 
			"fill_color" , "black" ,
			"font" , TAG_FONT , 
			NULL );

	gnome_canvas_item_new( gnome_canvas_root( GNOME_CANVAS( canvas ) ) ,
			gnome_canvas_text_get_type() ,
			"x" , UNUSED_TAG_X ,
			"y" , UNUSED_TAG_Y ,
			"text" , UNUSED_TAG ,
			"justification" , GTK_JUSTIFY_LEFT , 
			"fill_color" , "black" ,
			"font" , TAG_FONT , 
			NULL );
}	

/*
 * Draws circles that mark how many
 * marbles need to be collected for
 * victory.
 */
void draw_goal_markers()
{
	int i,j;
	double x, y;
	gchar* outline;
	int goal[3];

	get_goals( goal , level );
	
	if( goal_markers != NULL ){
		gtk_object_destroy( GTK_OBJECT( goal_markers ) );
	}
	
	goal_markers = gnome_canvas_item_new(
			gnome_canvas_root( GNOME_CANVAS( canvas ) ),
			gnome_canvas_group_get_type() ,
			NULL );

	for( i = 0 ; i<3 ; i++ ){
		for( j = 0 ; j<goal[i] ; j++ ){
			x = BASE_BIN_X_OFFSET + UNIT_BIN_X_OFFSET * j;
			y = BASE_BIN_Y_OFFSET + UNIT_BIN_Y_OFFSET * i;
			if( i==0 ){
				outline = "black";
			}
			else if( i==1 ){
				outline = "grey";
			}
			else {
				outline = "white";
			}
			
			gnome_canvas_item_new( GNOME_CANVAS_GROUP( goal_markers ) ,
				  gnome_canvas_ellipse_get_type() ,
				  "x1" , x ,
				  "y1" , y ,
				  "x2" , x + MARBLE_WIDTH ,
				  "y2" , y + MARBLE_WIDTH ,
				  "fill_color_rgba" , BACKGROUND ,
				  "outline_color" , outline ,
				  "width_pixels" , 1 ,
				  NULL );

			if( ( (BEGINNER==level) && (j<BEGIN_ALL_GOAL) ) ||
			 ( (TOURNAMENT==level) && (j<TOURN_ALL_GOAL) ) ){
				gnome_canvas_item_new( GNOME_CANVAS_GROUP( goal_markers ) ,
					  gnome_canvas_ellipse_get_type() ,
					  "x1" , x + MARBLE_WIDTH/2 - 1.0 ,
					  "y1" , y + MARBLE_WIDTH/2 - 1.0 ,
					  "x2" , x + MARBLE_WIDTH/2 + 1.0 , 
					  "y2" , y + MARBLE_WIDTH/2 + 1.0 ,
					  "fill_color_rgba" , BACKGROUND ,
					  "outline_color" , outline ,
					  "width_pixels" , 1 ,
					  NULL );
			}
			
			x += 	PLAYER2_X_OFFSET;
			
			gnome_canvas_item_new( GNOME_CANVAS_GROUP( goal_markers ) ,
				  gnome_canvas_ellipse_get_type() ,
				  "x1" , x ,
				  "y1" , y ,
				  "x2" , x + MARBLE_WIDTH ,
				  "y2" , y + MARBLE_WIDTH ,
				  "fill_color_rgba" , BACKGROUND ,
				  "outline_color" , outline ,
				  "width_pixels" , 1 ,
				  NULL );
			
			if( ( (BEGINNER==level) && (j<BEGIN_ALL_GOAL) ) ||
			 ( (TOURNAMENT==level) && (j<TOURN_ALL_GOAL) ) ){
				gnome_canvas_item_new( GNOME_CANVAS_GROUP( goal_markers ) ,
					  gnome_canvas_ellipse_get_type() ,
					  "x1" , x + MARBLE_WIDTH/2 - 1.0 ,
					  "y1" , y + MARBLE_WIDTH/2 - 1.0 ,
					  "x2" , x + MARBLE_WIDTH/2 + 1.0 , 
					  "y2" , y + MARBLE_WIDTH/2 + 1.0 ,
					  "fill_color_rgba" , BACKGROUND ,
					  "outline_color" , outline ,
					  "width_pixels" , 1 ,
					  NULL );
			}
		}
	}
}

/*
 * Draws all the empty discs. 
 */ 
void draw_discs
( Board* position )
{
	int row;
	int column;

	// put some discs onto the board
	for( row=0 ; row < WIDTH ; row++ ){
		for( column=0 ; column < WIDTH ; column++ ){
			if ( position -> square[column][row] != VOID ){
				draw_disc( column , row );
			}
			else {
				gui_board -> discs[column][row] = NULL;
			}
		}
	}
}

/*
 * Draws an empty disc.
 */
void draw_disc
( int column , int row )
{
	int inner_disc_top_offset;
	int inner_disc_bottom_offset;
	GnomeCanvasItem* foo;
	
	inner_disc_top_offset = ( OUTER_DISC_WIDTH - INNER_DISC_WIDTH ) / 2;
	inner_disc_bottom_offset = OUTER_DISC_WIDTH - inner_disc_top_offset;
	
	gui_board -> discs[column][row] = GNOME_CANVAS_GROUP ( gnome_canvas_item_new( 
				gnome_canvas_root( GNOME_CANVAS( canvas ) ) ,
				gnome_canvas_group_get_type() ,
				NULL ) );
	
	foo = gnome_canvas_item_new( gui_board -> discs[column][row] ,
			gnome_canvas_ellipse_get_type(),
			"x1", calculate_x_position( column , row ) ,
			"y1", calculate_y_position( column , row ) ,
			"x2", OUTER_DISC_WIDTH + calculate_x_position( column , row ) ,
			"y2", OUTER_DISC_WIDTH + calculate_y_position( column , row ) ,
			"fill_color_rgba" , DISC ,
			"outline_color" , "black" ,
			"width_pixels" , 1 ,
			NULL ) ;

	foo = gnome_canvas_item_new( gui_board -> discs[column][row] ,
			gnome_canvas_ellipse_get_type(),
			"x1", inner_disc_top_offset + calculate_x_position( column , row ) ,
			"y1", inner_disc_top_offset + calculate_y_position( column , row ) ,
			"x2", inner_disc_bottom_offset + calculate_x_position( column , row ) ,
			"y2", inner_disc_bottom_offset + calculate_y_position( column , row ) ,
			"fill_color_rgba" , BACKGROUND ,
			"outline_color" , "black" ,
			"width_pixels" , 1 ,
			NULL ) ;

	gtk_signal_connect( GTK_OBJECT( gui_board->discs[column][row] ) , 
			"event" ,
			(GtkSignalFunc) disc_event_cb ,
			NULL ); 
	
}

/*
 * Draws the marbles in columns on the right side
 * of the board.  Connects each marble to the
 * marble_event_cb function.
 */
void draw_available_marbles()
{
	int i;
	GnomeCanvasItem* tmp;
	
	for( i=0 ; i < NUMBER_BLACK ; i++ ){
		tmp = gnome_canvas_item_new( 
				gnome_canvas_root( GNOME_CANVAS( canvas ) ) , 
				gnome_canvas_ellipse_get_type(),
				"x1", BASE_MARBLE_X_OFFSET ,
				"y1", BASE_MARBLE_Y_OFFSET + i*MARBLE_SPACE ,
				"x2", BASE_MARBLE_X_OFFSET + MARBLE_WIDTH ,
				"y2", BASE_MARBLE_Y_OFFSET + MARBLE_WIDTH + i*MARBLE_SPACE,
				"fill_color_rgba" , BLACK_MARBLE ,
				"outline_color" , "black" ,
				"width_pixels" , MARBLE_BORDER_WIDTH ,
				NULL ) ;
		gtk_signal_connect( GTK_OBJECT( tmp ) , 
				"event" ,
				(GtkSignalFunc) marble_event_cb ,
				NULL );
		
		gui_board->black[i] = (Marble*) malloc( sizeof( Marble ) );
		set_marble( gui_board->black[i] , tmp , BLACK , i );
		gtk_object_set_data( GTK_OBJECT( tmp ) , MARBLE_KEY , gui_board->black[i] );
		
	}

	for( i=0 ; i < NUMBER_GREY ; i++ ){
		tmp = gnome_canvas_item_new(
				gnome_canvas_root( GNOME_CANVAS( canvas ) ) , 
				gnome_canvas_ellipse_get_type(),
				"x1", BASE_MARBLE_X_OFFSET + MARBLE_SPACE ,
				"y1", BASE_MARBLE_Y_OFFSET + i*MARBLE_SPACE ,
				"x2", BASE_MARBLE_X_OFFSET + MARBLE_WIDTH + MARBLE_SPACE ,
				"y2", BASE_MARBLE_Y_OFFSET + MARBLE_WIDTH + i*MARBLE_SPACE,
				"fill_color_rgba" , GREY_MARBLE ,
				"outline_color" , "black" ,
				"width_pixels" , MARBLE_BORDER_WIDTH ,
				NULL ) ;
		gtk_signal_connect( GTK_OBJECT( tmp ) , 
				"event" ,
				(GtkSignalFunc) marble_event_cb ,
				NULL );
	
		gui_board->grey[i] = (Marble*) malloc( sizeof( Marble ) );
		set_marble( gui_board->grey[i] , tmp , GREY , i );
		gtk_object_set_data( GTK_OBJECT( tmp ) , MARBLE_KEY , gui_board->grey[i] );
		
	}

	for( i=0 ; i < NUMBER_WHITE ; i++ ){
		tmp = gnome_canvas_item_new(
				gnome_canvas_root( GNOME_CANVAS( canvas ) ) , 
				gnome_canvas_ellipse_get_type(),

				"x1", BASE_MARBLE_X_OFFSET + MARBLE_SPACE*2 ,
				"y1", BASE_MARBLE_Y_OFFSET + i*MARBLE_SPACE ,
				"x2", BASE_MARBLE_X_OFFSET + MARBLE_WIDTH + MARBLE_SPACE*2 ,
				"y2", BASE_MARBLE_Y_OFFSET + MARBLE_WIDTH + i*MARBLE_SPACE,
				"fill_color_rgba" , WHITE_MARBLE ,
				"outline_color" , "black" ,
				"width_pixels" , MARBLE_BORDER_WIDTH ,
				NULL ) ;
		gtk_signal_connect( GTK_OBJECT( tmp ) , 
				"event" ,
				(GtkSignalFunc) marble_event_cb ,
				NULL );
		
		gui_board->white[i] = (Marble*) malloc( sizeof( Marble ) );
		set_marble( gui_board->white[i] , tmp , WHITE , i );
		gtk_object_set_data( GTK_OBJECT( tmp ) , MARBLE_KEY , gui_board->white[i] );
	}

}

/*
 * Deletes a disc upon receipt of a double-click.
 */
gint disc_event_cb
( GnomeCanvasItem* disc , GdkEvent* event , gpointer data )
{
	int column, row;
	char error;
	
	// only accept input under correct conditions
	if( game_mode != USE_GUI ){
		return FALSE;
	}
	if( turn_stage != REMOVING ){
		return FALSE;
	}
	
	column = calculate_column( event->button.x , event->button.y );
	row = calculate_row( event->button.x , event->button.y );
	
	switch (event->type){
		case GDK_2BUTTON_PRESS:
			if( is_removeable( &gui_position , column , row ) ){
				gtk_object_destroy( GTK_OBJECT( disc ) );
				gui_board->discs[column][row] = NULL;
				remove_disc( &gui_position , column , row );
				move_set_remove_data( current_move, column, row );
				advance_player_turn();
			}
			else {
				error = decode_square ( get_disc_contents( &gui_position , column , row ) ) ;
				printf("main: Tried to remove %d , %d. Not allowed contains: %c.\n" , column , row , error);
			}
			return FALSE;
		default:
			return FALSE;
	}

	return FALSE;
}

/*
 * Function that responds to events that befall 
 * a marble.
 */ 
gint marble_event_cb
( GnomeCanvasItem* item , GdkEvent* event , gpointer data )
{
	static double x, y;
	double new_x, new_y;
	GdkCursor *fleur;
	static int dragging;
	static int orig_row, orig_column, orig_place;
	double item_x, item_y;
	int row , column;
	int marble_color;	
	char marble_status;
	static int multi_jump = FALSE;
	Marble* marble;
	
	marble = get_marble_from_canvas( item );
	
	// only respond if game is in GUI mode
	if( game_mode != USE_GUI ){
		return FALSE;
	}
	
	// check if marble can even be moved
	if( ! is_moveable_marble( marble ) ){
		return FALSE;
	}
	
	marble_status = get_marble_status( marble );
	marble_color = get_marble_color( marble );

	item_x = event->button.x;
	item_y = event->button.y;
	gnome_canvas_item_w2i(item->parent, &item_x, &item_y);

	switch (event->type) { 
		case GDK_BUTTON_PRESS: 
			if( turn_stage == JUMPING ){
				orig_row = get_marble_row( marble );
				orig_column = get_marble_column( marble );
			}
			else if( ( turn_stage == PLACING ) && ( ONE == marble_status || TWO == marble_status ) ){
				orig_place = get_marble_place( marble );
			}
			x = item_x;
			y = item_y;
			fleur = gdk_cursor_new(GDK_FLEUR);
			gnome_canvas_item_grab(item,
				GDK_POINTER_MOTION_MASK | 
				GDK_BUTTON_RELEASE_MASK,
				fleur,
				event->button.time);
			gdk_cursor_destroy(fleur);
			dragging = TRUE;
			break;
		case GDK_MOTION_NOTIFY: 
			if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
				new_x = item_x;
				new_y = item_y;
				gnome_canvas_item_move(item, new_x - x , new_y - y );
				x = new_x;
				y = new_y;
			}
			break;
		case GDK_BUTTON_RELEASE: 
			gnome_canvas_item_ungrab(item, event->button.time);
			dragging = FALSE;
			column = find_item_column( item );
			row = find_item_row( item );
			if( turn_stage == JUMPING ){
				if( can_jump_to( &gui_position , orig_column , orig_row , column ,row ) ){
					
					if( multi_jump == FALSE ){
						record_move_start_capture(game_rec_stream); // hMMM???
					}
					
					center_marble( marble , column , row );
					set_marble_column( marble , column );
					set_marble_row( marble , row );
					set_marble_place( marble , -1 );
					move_jumped_marble( game_state.mover , orig_column , orig_row , column , row );
					move_add_jump(current_move, orig_column, orig_row, column, row);
					unmark_jumper_marbles();
					
					// update bits of the local model to check for 
					// multiple jumps
					remove_marble( &gui_position , orig_column , orig_row );
					place_marble( marble_color , &gui_position , column , row );
					// removal of the jumped marble is taken care
					// of by move_jumped_marble
					
					if( has_jump( &gui_position , column , row ) ){
						multi_jump = TRUE;
						set_marble_status( marble , JUMPER );
					}
					else {
						record_move_end_capture(game_rec_stream, column,row);
						set_removeable_discs( &(gui_position) );
						advance_player_turn();
						multi_jump = FALSE;
					}
				}
				// clean up if the player tried to 
				// jump someplace not allowed
				else {
					center_marble( marble , orig_column , orig_row );
				}
			}			
			else if( turn_stage == PLACING ){
				if( can_accept_marble( &gui_position , column , row ) ){
					center_marble( marble , column , row );	
					set_marble_status( marble , PLACED );
					set_marble_column( marble , column );
					set_marble_row( marble , row );
					place_marble( marble_color , &gui_position , column , row );
					set_removeable_discs( &gui_position );
					move_set_place_data(current_move, marble_color, column, row);
					next_gui_stage();	
					if( ! are_free_marbles( &game_state ) ){
						decrement_local_score( game_state.mover , marble_color );
					}
				}
				// clean up if marble dropped someplace wrong
				else if( are_free_marbles( &game_state ) ){
					replace_marble( marble );
				}
				else {
					slide_marble_to_bin( marble , game_state.mover );
				}
			}
			break;
		default: 
			break;
	}
				
	return TRUE;
}

/*
 * Returns TRUE if the marble can be moved based
 * on the current conditions.  FALSE otherwise.
 */
int is_moveable_marble
( Marble* marble )
{

	int marble_status = get_marble_status( marble );
	
	// Only move marbles during place stages and  
	// jumping stages
	if( ! ( turn_stage == JUMPING || turn_stage == PLACING ) ){
		return FALSE;
	}
	
	// do not allow placed marbles to be moved
	if( PLACED == marble_status ){
		return FALSE;
	}

	// if there is a jump available only allow
	// jumping marbles to be moved
	if( turn_stage == JUMPING ){
		if( JUMPER != marble_status ) {
			return FALSE;
		}
	}
	
	// do not allow captured marbles to be moved
	// unless we are all out of marbles
	// or allow player one to move player two's marbles
	// or visa versa
	if( are_free_marbles( &game_state ) ){
		if( ONE == marble_status || TWO == marble_status ){
			return FALSE;
		}
	}
	else {
		if( ONE == marble_status && game_state.mover == PLAYER_TWO ) {
			return FALSE;
		}
		if( TWO == marble_status && game_state.mover == PLAYER_ONE ) {
			return FALSE;
		}
	}

	return TRUE;
}


/*
 * Pass the beginning and ending coordinates of a jump.
 *
 * Moves the jumped marble into the bin, and removes 
 * it from the model.  It also updates the model
 * about the jumping marble.
 */
void move_jumped_marble
( int player , int start_column , int start_row , int end_column , int end_row )
{
	
	int jumped_color;
	Coordinates old_coord, new_coord;
	Coordinates mid_coord;
	Marble* jumped_marble;
	
	// find the middle
	old_coord.x = start_column;
	old_coord.y = start_row;
	new_coord.x = end_column;
	new_coord.y = end_row;
	get_middle_coordinates( &old_coord , &new_coord , &mid_coord );

	// take marble off the local copy of the board
	remove_marble( &gui_position , mid_coord.x , mid_coord.y ); 
	
	// get the middle marble
	jumped_marble = get_marble_from_board( mid_coord.x , mid_coord.y );
	jumped_color = get_marble_color( jumped_marble );
	// Recording!
	record_move_capture_step(game_rec_stream, 
				 old_coord.x, old_coord.y, jumped_color);
	// move it to the correct player's bin
	put_marble_in_bin( jumped_marble , player );
	increment_local_score( player , jumped_color );

}

/*
 * Changes the value of turn_stage
 * to the appropriate value.  This
 * value is only relevant during GUI
 * turns.
 */
void next_gui_stage()
{

	switch( turn_stage ) {
		case PLACING:
			// if possible move to removing stage
			if( has_removeable_disc( &gui_position ) ){
				turn_stage = REMOVING;
				set_message("Remove a disc.");
			}
			// no removal? go to next player's turn
			else {
				advance_player_turn();
				if( ! ( (VICTORY_ONE == game_mode) || (VICTORY_TWO == game_mode) ) ) {
					if( jump_available( &(game_state.position) ) ){
						turn_stage = JUMPING;
						mark_jumper_marbles();
					}
					else {		
						turn_stage = PLACING;
					}
				}
			}
			break;
		case REMOVING:
			advance_player_turn();
			break;
		case JUMPING:
			advance_player_turn();
			break;
		default:
			printf("\n\n------ turn step problem -------\n\n");
			break;
	}
}

/*
 * This function makes a move that is determined
 * by the AI.
 *
 */
void do_ai_move()
{
	float eval;
	int goal[3];
	
	get_goals( goal , level );

	if( PL_CURRENT.ai_type == SIMPLE_AI ){	
		eval = find_simple_move( current_move , &game_state , level , game_state.mover );
		/*
		printf("HERE IS THE AI MOVE: ");
		dump_move( current_move );
		printf("HERE IS THE EVAL:  %f\n",eval);
		*/
	}
	else {
		get_random_move( current_move ,
				&(game_state.position) ,
				game_state.islands ,
				goal , 
				game_state.available ,
				PL_ONE.captured ,
				PL_TWO.captured ,
				game_state.mover
				);
	}
	
	if( current_move->type == MOVE_TYPE_JUMP ){
		display_jump_move( current_move );	
	}
	else {
		display_plain_move( current_move );
	}
	
	advance_player_turn();
		
}

void do_network_move()
{
	/* until there's a network move to read, just update the gui */
	while (! gnet_is_readable(&net_play_gnsock)) {
		while (gtk_events_pending()) {
			gtk_main_iteration();
		}
		sleep(1);
	}
	recv_network_move( &net_play_gnsock, current_move );

	if( current_move->type == MOVE_TYPE_JUMP ){
		display_jump_move( current_move );	
	}
	else {
		display_plain_move( current_move );
	}

	advance_player_turn();
	
} /* do_network_move() */

/*
 * Moves the jumping and the jumped marbles.
 */
void display_jump_move( Move* move )
{
	Marble* marble;
	Jump* next;
	Coordinates start, end, middle;

	next = move->jump;

	// find the jumping marble
	marble = get_marble_from_board( next->start_column , next->start_row );

	record_move_start_capture( game_rec_stream );

	// find the final end of the jump
	while( next != NULL ){
		set_coordinates_from_jump( next , &middle , &start , &end );	
		move_jumped_marble( game_state.mover , start.x , start.y , end.x , end.y );
		slide_center_marble( marble , end.x , end.y );
		next = next	-> multi;
	}
	
	record_move_end_capture( game_rec_stream , end.x , end.y ); 
	// put jumping marble at the end
	slide_center_marble( marble , end.x , end.y );
	set_marble_column( marble , end.x );
	set_marble_row( marble , end.y );
	set_marble_place( marble , -1 );
}

/*
 * Does the updates to the GUI
 * required by a plain turn. 
 */
void display_plain_move
( Move* move )
{
	Marble* marble;
	int color, column, row;

	color = move->place_color;
	column = move->place_column;
	row = move->place_row;

	if( are_free_marbles( &game_state ) ){
		marble = grab_available_marble( color );
	}
	else {
		printf("grabbing captured marble\n");
		marble = grab_captured_marble( color , game_state.mover );
		decrement_local_score( game_state.mover , color );
	}
	
	set_marble_status( marble , PLACED );
	set_marble_column( marble , column );
	set_marble_row( marble , row );
	set_marble_place( marble , -1 );

	slide_center_marble( marble , column , row );	
	
	// trying to help unexplained crashes
	while( gtk_events_pending() ){
		gtk_main_iteration();
	}

	// now do remove
	column = move->remove_column;
	row = move->remove_row;

	if( column >= 0 && column < WIDTH && row >= 0 && row < WIDTH ){	
		take_off_disc( column , row );
	}
}

void take_off_disc
( int column , int row )
{
	Flasher* flasher;

	flasher = (Flasher*) malloc( sizeof( Flasher ) );
	flasher->count = 0;
	flasher->column = column;
	flasher->row = row;

	gtk_timeout_add( 150 , (GtkFunction) flash_disc , (gpointer) flasher );
}

gint flash_disc( gpointer data )
{
	GnomeCanvasGroup* disc;
	Flasher* flasher;
	
	flasher = (Flasher*) data;
	
	disc = gui_board->discs[flasher->column][flasher->row];
	if( flasher->count > 8 ){
		gtk_object_destroy( GTK_OBJECT( disc ) );
		gui_board->discs[flasher->column][flasher->row] = NULL;
		free( flasher );
		return FALSE;
	}
	else if( flasher->count % 2 == 0 ){
		gnome_canvas_item_hide( GNOME_CANVAS_ITEM( disc ) );
		(flasher->count)++;
		return TRUE;
	}
	else {
		gnome_canvas_item_show( GNOME_CANVAS_ITEM( disc ) );
		(flasher->count)++;
		return TRUE;
	}

}


/*
 * Cute function.  Returns a pointer to the lowest 
 *	numbered marble (Canvas Item) in the unused bin
 * that is of the correct color.
 */
Marble* grab_available_marble
( int color )
{
	Marble* marble=NULL;
	
	while( ( marble = get_next_marble( marble ) ) != NULL ){
		if( (color==get_marble_color(marble)) && (AVAILABLE == get_marble_status( marble ) ) ){
			return marble;
		}
	}
	return NULL;
}

/*
 * Cute function.  Returns a pointer to the lowest 
 *	numbered marble (Canvas Item) in the player's bin
 * that is of the correct color.
 */
Marble* grab_captured_marble
( int color , int player_num )
{
	int code;
	int place;
	Marble* marble=NULL;
	
	printf("Player %d is grabbing a %d marble\n" , player_num+1 , color );
	
	if( PLAYER_ONE == player_num ){
		code = ONE;
		place = PL_ONE.captured[color-BLACK]-1;
	}
	else if( PLAYER_TWO == player_num ){
		code = TWO;
		place = PL_TWO.captured[color-BLACK]-1;
	}
	else {
		return NULL;
	}

	while( ( marble = get_next_marble( marble ) ) != NULL ){
		if( (color==get_marble_color( marble )) 
				&& (code==get_marble_status( marble )) 
				&& (place==get_marble_place( marble)) ){
			return marble;
		}
	}

	
	printf("COUND NOT FIND THE CAPTURED MARBLE\n");
	printf("LOOKED FOR MARBLE IN PLACE %d \n",place);
	return NULL;
}


/*
 * Sets condition key for every Marble on the board
 * that has an available jump to JUMPER.
 */
void mark_jumper_marbles()
{
	Marble* marble = NULL;
	int column, row;
	
	while( ( marble = get_next_marble( marble ) ) != NULL ){
		if( PLACED == get_marble_status( marble ) ){
			column = get_marble_column( marble );
			row = get_marble_row( marble );
			if( has_jump( &(game_state.position) , column , row ) ){
				set_marble_status( marble , JUMPER );
			}
		}
	}
}

/*
 * Finds the jumper marbles and resets their status
 * to placed.  Called after a jump has happened.
 */
void 
unmark_jumper_marbles()
{
	Marble* marble = NULL;

	while( ( marble = get_next_marble( marble ) ) != NULL ){
		if( JUMPER == get_marble_status(marble) ){
			set_marble_status( marble , PLACED );
		}
	}
}

/*
 * Centers a marble in exactly the middle
 * of the disc.
 */ 
void center_marble
( Marble* marble , int column , int row )
{
	int x, y;
	double top_x , top_y , bot_x , bot_y;
	double center;

	// this makes things right --- but it might be wrong
	// thinking about this is getting me confused.
	center = ( UNIT_Y_OFFSET - MARBLE_WIDTH ) / 2;
	
	x = calculate_x_position( column , row );
	y = calculate_y_position( column , row );
	gnome_canvas_item_get_bounds( marble->image , &top_x , &top_y , &bot_x , &bot_y );
	gnome_canvas_item_move( marble->image , x-top_x+center , y-top_y+center );
}

/*
 * Moves a marble to the correct position
 * after it has been placed.
 */ 
void slide_center_marble
( Marble* marble , int column , int row )
{
	
	int x, y;
	
	x = calculate_x_position( column , row );
	y = calculate_y_position( column , row );

	slide_marble( marble , x , y );

}

/*
 * Graphically slides a marble to the given
 * coordinates.
 */ 
void slide_marble
( Marble* marble , double end_x , double end_y )
{
	Slider* slider;
	double top_x , top_y , bot_x , bot_y;

	gnome_canvas_item_get_bounds( marble->image , &top_x , &top_y , &bot_x , &bot_y );
	
	slider = (Slider*) malloc( sizeof( Slider ) );
	slider->marble = marble;
	slider->start_x = top_x;
	slider->start_y = top_y;
	slider->end_x = end_x;
	slider->end_y = end_y;

	if( can_slide( marble ) ){
		block_sliding( marble );
		gtk_timeout_add( 3 , (GtkFunction) increment_slide , (gpointer)slider );
	}
	else {
		add_slide_request( marble , slider );
	}
	
}

/*
 * Function to move the marble a delta
 * amount along its sliding journey.
 */ 
gint increment_slide
( gpointer data )
{
	float delta_x, delta_y;
	double top_x , top_y , bot_x , bot_y;
	Marble* marble;
	Slider* my_slider;
  	Slider* next_slider;
	double center;
	
	my_slider = (Slider*) data;
	
	marble = my_slider->marble;
	gnome_canvas_item_get_bounds( marble->image , &top_x , &top_y , &bot_x , &bot_y );

	center = ( UNIT_Y_OFFSET - MARBLE_WIDTH ) / 2;

	delta_x = (my_slider->end_x - my_slider->start_x)/100;
	delta_y = (my_slider->end_y - my_slider->start_y)/100;
	
	// when we get pretty close, move to exact
	// free the slider, mark the marble correctly
	// return FALSE
	if( abs(top_x-my_slider->end_x)<.1 && abs(top_y-my_slider->end_y)<.1 ){
		gnome_canvas_item_move( marble->image , my_slider->end_x-top_x+center, my_slider->end_y-top_y+center );
		done_sliding( marble );
		free( my_slider );

		if( ( next_slider = get_next_slide(marble) ) != NULL ){
			slide_marble( marble , next_slider->end_x , next_slider->end_y );
			free( next_slider );	
		}
		
		mark_jumper_marbles();
		return FALSE;
	}
	else {
		gnome_canvas_item_move( marble->image , delta_x , delta_y );  
		return TRUE;
	}
}

/*
 * Returns a marble to its unused location if it
 * was dropped in the wrong place.
 */
void replace_marble
( Marble* marble )
{
	double x, y;
	double top_x , top_y , bot_x , bot_y;
	int num;
	int marble_color;
	
	num = get_marble_number( marble );
	marble_color = get_marble_color( marble );
	
	y = BASE_MARBLE_Y_OFFSET + MARBLE_SPACE * num;
 	
	if( BLACK == marble_color ){
		x = BASE_MARBLE_X_OFFSET; 
	}
	else if( GREY == marble_color ){
		x = BASE_MARBLE_X_OFFSET + MARBLE_SPACE; 
	}
	else if( WHITE == marble_color ){
		x = BASE_MARBLE_X_OFFSET + MARBLE_SPACE * 2; 
	}
	
	//printf("replacing\n");
	gnome_canvas_item_get_bounds( marble->image , &top_x , &top_y , &bot_x , &bot_y );
	gnome_canvas_item_move( marble->image , x-top_x , y-top_y );
}

/*
 * Finds the Marble* pointer that represents
 * the Marble shown by the Canvas item passed.
 */
Marble* get_marble_from_canvas
( GnomeCanvasItem* item )
{
	Marble* marble = NULL;

	marble = (Marble*) gtk_object_get_data( GTK_OBJECT( item ) , MARBLE_KEY );
	
	return marble;
}

/*
 * Returns the correct marble from the board
 * when passed the coordinates.
 */ 
Marble* get_marble_from_board
( int column , int row )
{
	Marble* marble = NULL;

	while( ( marble = get_next_marble( marble ) ) != NULL ){
		if( column == get_marble_column( marble ) &&
				row == get_marble_row( marble ) ){
				return marble;
		}
	}

	printf("ERROR: COULD NOT FIND THE MARBLE ON THE BOARD!\n");
	
	return NULL;
}

/*
 * If the island needs to be destroyed
 * this moves the marbles appropriately
 * and destroys the discs.
 */
void remove_island
( Island* isle )
{
	int i,j;
	int color;	
	Marble* marble;
	int started_capturing = FALSE; /* lame hack */
	
	for( i=0 ; i<WIDTH ; i++ ){
		for( j=0 ; j<WIDTH ; j++ ){
			if( is_in_island( isle , j , i ) ){
				//printf("%d,%d is in the island\n",j,i);
				//deal with the marble
				marble = get_marble_from_board( j , i );
				// note the TOGGLE, since clean up happens
				// after the game_state is updated to the
				// next player's turn
				if( marble != NULL ){
					color = get_marble_color( marble );
					put_marble_in_bin( marble , TOGGLE(game_state.mover) );
					increment_local_score( TOGGLE(game_state.mover) , color );
					if ( ! started_capturing ) {
						record_move_start_capture(game_rec_stream);
						started_capturing=TRUE;
					}
					record_move_capture_isolate(game_rec_stream, color, j, i);
				}
				else {
					printf("HUGE PROBLEM:  a marble to be removed was NULL\n");
					exit( -1 );
				}
					
				// deal with the disc
				gtk_object_destroy( GTK_OBJECT( gui_board->discs[j][i] ) );
				gui_board->discs[j][i] = NULL; // this is key!
			}
		}
	}
}


/*
 * Moves a marble from the board into
 * one of the players' captured marble
 * bins.
 * 
 * Also updates the local score and
 * sets the marble status.
 */ 
void put_marble_in_bin
( Marble* marble , int player )
{

	if( PLAYER_ONE == player ){
		set_marble_status( marble , ONE );
		set_marble_place( marble , PL_ONE.captured[get_marble_color(marble)-BLACK] );
	}
	else if( PLAYER_TWO == player ){
		set_marble_status( marble , TWO );
		set_marble_place( marble , PL_TWO.captured[get_marble_color(marble)-BLACK] );
	}

	set_marble_column( marble , -1 );
	set_marble_row( marble , -1 );
	
	slide_marble_to_bin( marble , player );
	
}

/*
 * Used when a player has to reach into
 * his own bin to place a marble.
 */
void decrement_local_score
( int player , int color )
{
	if( PLAYER_ONE == player ){
		PL_ONE.captured[color-BLACK]--;
	}
	else if( PLAYER_TWO == player ){
		PL_TWO.captured[color-BLACK]--;
	}
	else {
		printf("OH HIDEOUS DECREMEMT\n");
	}
}

/*
 * Updates the local copy of the score 
 * kept in the player struct.
 */
void increment_local_score
( int player , int color )
{
	if( PLAYER_ONE == player ){
		PL_ONE.captured[color-BLACK]++;
	}
	else if( PLAYER_TWO == player ){
		PL_TWO.captured[color-BLACK]++;
	}
	else {
		printf("OH HIDEOUS INCREMENT\n");
	}
}

/*
 * Puts the marble in the bin without adjusting
 * the score.
 */
void slide_marble_to_bin
( Marble* marble , int player )
{
	double x, y;
	double top_x , top_y , bot_x , bot_y;
	int color = get_marble_color( marble );
	int ver_offset = color - BLACK;
	int place = get_marble_place( marble );
	double center;

	center = ( UNIT_Y_OFFSET - MARBLE_WIDTH ) / 2;
	gnome_canvas_item_get_bounds( marble->image , &top_x , &top_y , &bot_x , &bot_y );
	
	x = BASE_BIN_X_OFFSET + UNIT_BIN_X_OFFSET * place - center;
	y = BASE_BIN_Y_OFFSET + UNIT_BIN_Y_OFFSET * ver_offset - center;

	if( PLAYER_TWO == player ){
		x += PLAYER2_X_OFFSET;	
	}

	slide_marble( marble , x , y );

}

/*
 * When passed a marble this function
 * finds the row it is in.
 */
int find_item_row
( GnomeCanvasItem* item )
{
	double top_x , top_y , bot_x , bot_y;
	gnome_canvas_item_get_bounds( item , &top_x , &top_y , &bot_x , &bot_y );
	return calculate_row( top_x , top_y );	
}

/*
 * When passed a marble this function
 * finds the column it is in.
 */
int find_item_column
( GnomeCanvasItem* item )
{
	double top_x , top_y , bot_x , bot_y;
	gnome_canvas_item_get_bounds( item , &top_x , &top_y , &bot_x , &bot_y );
	return calculate_column( top_x , top_y );	
}

/*
 * Makes things appear in a nice hexagon.
 * That is, translates row and column into
 * an x-coordinate for the canvas.
 */ 
float calculate_x_position
( int x , int y )
{
	float base = BASE_X_OFFSET + UNIT_X_OFFSET * x;
	base = base + ( (WIDTH-1)/2 - y ) * ( UNIT_X_OFFSET / 2 ) ;	
	return base;
}

/*
 * Makes things appear in a nice hexagon.
 * That is, translates row and column into
 * an y-coordinate for the canvas.
 */ 
float calculate_y_position
( int x , int y )
{
	return BASE_Y_OFFSET + UNIT_Y_OFFSET * y;
}

/*
 * Translates canvas coordinates to zertz board
 * coordinates.
 */
int calculate_row
( float x , float y )
{
	int row;
	
	row = (int) (y - BASE_Y_OFFSET) / UNIT_Y_OFFSET;
	
	return row;
}

/*
 * Translates canvas coordinates to zertz board
 * coordinates.
 */
int calculate_column
( float x , float y )
{
	int column;
	int row;
	float adjusted_x;
	
	row = calculate_row( x , y );
	
	adjusted_x = x - ( ( (WIDTH-1)/2 -row ) * ( UNIT_X_OFFSET / 2 ) );
	
	column = (int) (adjusted_x - BASE_X_OFFSET ) / UNIT_X_OFFSET;
	
	return column;
}

/*
 * Puts a message on the app bar on the
 * bottom of the window.
 */
void set_message
( char* message )
{
	char * pX_message; // "P1: " + message, or "P2: " + message

	int strsize = 5 + strlen(message);
	pX_message = (char*)calloc(strsize, 1);
	if (game_state.mover==PLAYER_ONE) {
		strncpy(pX_message, "P1: ", strsize);
	} else {
		strncpy(pX_message, "P2: ", strsize);
	}
	strcat(pX_message, message);

	gnome_appbar_push(GNOME_APPBAR (appbar), pX_message);

	free(pX_message);
}

/*
 * Provides a mechanism for looping through
 * all the marbles.  Pass NULL to get the
 * first marble.
 */
Marble* get_next_marble
( Marble* marble )
{
	int color;
	int number;
	
	// first take care of the first marble	
	if( marble == NULL ){
		return gui_board->black[0];
	}

	color = get_marble_color( marble );
	number = get_marble_number( marble ); 

	if( (BLACK==color) && (number<NUMBER_BLACK-1) ){
		return gui_board->black[number+1];
	}
	else if( (BLACK==color) && (number==NUMBER_BLACK-1) ){
		return gui_board->grey[0];
	}
	else if( (GREY==color) && (number<NUMBER_GREY-1) ){
		return gui_board->grey[number+1];
	}
	else if( (GREY==color) && (number==NUMBER_GREY-1) ){
		return gui_board->white[0];
	}
	else if( (WHITE==color) && (number<NUMBER_WHITE-1) ){
		return gui_board->white[number+1];
	}
	else if( (WHITE==color) && (number==NUMBER_WHITE-1) ){
		return NULL;
	}

	// should never happen
	printf("main: ERROR looking for a marble past the end!");
	return NULL;	
	
}

/*
 * Changes the value of game_state.mover.	
 */
void advance_player_turn()
{

	Turn* turn = NULL;
	GSList* dead_isles = NULL;
	Island* isle = NULL;
	GameState new_state;
	
	while (gtk_events_pending()) {
		gtk_main_iteration();
	}

	
	// send current_move to remove net players
	if ( use_net && PL_CURRENT.type!=USE_NET ) {
		// handle any pending gui events before blocking on net output
		while (gtk_events_pending()) {
			gtk_main_iteration();
		}
		send_network_move( &net_play_gnsock, current_move );
	}		
	
	// update the GameState
	dead_isles = find_new_state( &game_state , &new_state , current_move );
	copy_state( &new_state , &game_state );

	// put the move that got us here and the new
	// state onto the turn list	
	turn = (Turn*) malloc( sizeof( Turn ) );
	turn->move = *(current_move); 
	copy_state( &new_state , &(turn->state) );
	turn_list = g_list_prepend( turn_list , turn );	

	// update the gui_position
	copy_board( &(game_state.position) , &gui_position );

	//dump_move(current_move);
	//print_state( &game_state );

	// this is out of place
	// but recording must happen
	// FIXME rewrite recording API	
	if( current_move->type == MOVE_TYPE_PLACE_AND_REMOVE ){
		record_move_place(game_rec_stream, current_move->place_color, current_move->place_column, current_move->place_row);
		record_move_removal(game_rec_stream, current_move->remove_column, current_move->remove_row);
	}
	record_move_end( game_rec_stream );
	
	// make room for a new move	
	current_move = NULL;
	current_move = (Move*) malloc( sizeof( Move ) );
	initialize_move( current_move );

	// kill any dead islands
	while( dead_isles != NULL ){
		isle = (Island*) dead_isles->data;
		remove_island( isle );
		dead_isles = g_slist_next( dead_isles );
	}	

	// advance game_mode
	if( ! ( game_mode == VICTORY_TWO || game_mode == VICTORY_ONE ) ){
		if( game_state.mover == PLAYER_ONE ){
			game_mode = PL_ONE.type;
		}
		else {
			game_mode = PL_TWO.type;
		}
	}

	// look to see if a victory has happened
	if( check_for_game_over( &game_state , level  ) ){
		game_mode = check_for_game_over( &game_state , level );
	}
	
	// now act appropriately
	if( game_mode == USE_GUI ){
		free_chained_jumps( current_move->jump );
		initialize_move( current_move );
		if( jump_available( &(game_state.position) ) ){
			mark_jumper_marbles();
			set_message("You must jump.");
			turn_stage = JUMPING;
		}
		else {
			set_message("Place a marble.");
			turn_stage = PLACING;
		}
	}
	else if( game_mode == USE_AI ){
		// do an ai turn
		do_ai_move();
	}
	else if( game_mode == USE_NET ){
		set_message("Waiting for a move from the net player...");
		// handle any pending gui events before blocking on net input
		while (gtk_events_pending()) {
			gtk_main_iteration();
		}
		do_network_move();
	}
	else if( game_mode == VICTORY_ONE ){
		set_message("Player 1 has won!");
	}
	else if( game_mode == VICTORY_TWO ){
		set_message("Player 2 has won!");
	}
	
	print_state( &game_state );
	
}

/*
 * Free up the turn list at the end of a game.
 */ 
void free_turn_list()
{
	Turn* tmp;
	
	while( turn_list != NULL ){
		tmp = (Turn*) turn_list->data;
		turn_list = g_list_remove( turn_list , tmp );
		free_island_list( &(tmp->state.islands) );
		
		free( tmp );
	}

	turn_list = NULL;
}
		
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
