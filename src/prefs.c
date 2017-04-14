#include "prefs.h"
#include "board.h"
#include "player.h"

// these are defined in main
extern Zertz_Player players[];
//extern int player1_ai;
//extern int player2_ai;
extern int level;
extern int display_markers;
extern char* remote_addr;


// variables that mimic above
int local_player1_state;
int local_player2_state;
int local_level;
int local_display_markers;
int local_player1_ai;
int local_player2_ai;
char* local_host_name;

GtkWidget* prop_box;

GtkWidget* players_page;
GtkWidget* players_page_label;
GtkWidget* p1_frame;
GtkWidget* p1_box;
GtkWidget* p1_human_but;
GtkWidget* p1_ai_rand_but;
GtkWidget* p1_ai_simp_but;
GtkWidget* p1_net_but;
GtkWidget* p2_frame;
GtkWidget* p2_box;
GtkWidget* p2_human_but;
GtkWidget* p2_ai_rand_but;
GtkWidget* p2_ai_simp_but;
GtkWidget* p2_net_but;

GtkWidget* level_page;
GtkWidget* level_page_label;
GtkWidget* level_frame;
GtkWidget* level_box;
GtkWidget* tournament_but;
GtkWidget* beginner_but;
GtkWidget* goal_frame;
GtkWidget* goal_box;
GtkWidget* goal_but;

GtkWidget* net_page;
GtkWidget* net_page_label;
GtkWidget* host_box;
GtkWidget* host_entry;
GtkWidget* host_entry_label;
GtkWidget* port_box;
GtkWidget* port_entry;
GtkWidget* port_entry_label;

void prefs_cb
( GtkWidget* menu_item , gpointer data )
{
	
	prop_box = gnome_property_box_new();

	players_page = gtk_hbox_new( TRUE , 2 );

	p1_frame = gtk_frame_new("Player One");
	p1_box = gtk_vbox_new( TRUE , 2 );
	p1_human_but = gtk_radio_button_new_with_label(NULL,"Human");
	p1_ai_rand_but = gtk_radio_button_new_with_label_from_widget(
			GTK_RADIO_BUTTON(p1_human_but) , "Random AI" );
	p1_ai_simp_but = gtk_radio_button_new_with_label_from_widget(
			GTK_RADIO_BUTTON(p1_human_but) , "Simple AI" );
	p1_net_but = gtk_radio_button_new_with_label_from_widget(
			GTK_RADIO_BUTTON(p1_human_but) , "Network" );
	gtk_box_pack_start( GTK_BOX( p1_box ) , p1_human_but , TRUE , TRUE , 2 );
	gtk_box_pack_start( GTK_BOX( p1_box ) , p1_ai_rand_but , TRUE , TRUE , 2 );
	gtk_box_pack_start( GTK_BOX( p1_box ) , p1_ai_simp_but , TRUE , TRUE , 2 );
	gtk_box_pack_start( GTK_BOX( p1_box ) , p1_net_but , TRUE , TRUE , 2 );
	gtk_container_add( GTK_CONTAINER( p1_frame ) , p1_box );
	gtk_box_pack_start( GTK_BOX( players_page ) , p1_frame , TRUE , TRUE , 2 );
	gtk_signal_connect( GTK_OBJECT( p1_human_but ),
			"clicked",
			GTK_SIGNAL_FUNC( p1_set_human ),
			NULL );

	gtk_signal_connect( GTK_OBJECT( p1_ai_rand_but ),
			"clicked",
			GTK_SIGNAL_FUNC( p1_set_random ),
			NULL );

	gtk_signal_connect( GTK_OBJECT( p1_ai_simp_but ),
			"clicked",
			GTK_SIGNAL_FUNC( p1_set_simple ),
			NULL );
	
	gtk_signal_connect( GTK_OBJECT( p1_net_but ),
			"clicked",
			GTK_SIGNAL_FUNC( p1_set_network ),
			NULL );
	
	p2_frame = gtk_frame_new("Player Two");
	p2_box = gtk_vbox_new( TRUE , 2 );
	p2_human_but = gtk_radio_button_new_with_label(NULL,"Human");
	p2_ai_rand_but = gtk_radio_button_new_with_label_from_widget(
			GTK_RADIO_BUTTON(p2_human_but) , "Random AI" );
	p2_ai_simp_but = gtk_radio_button_new_with_label_from_widget(
			GTK_RADIO_BUTTON(p2_human_but) , "Simple AI" );
	p2_net_but = gtk_radio_button_new_with_label_from_widget(
			GTK_RADIO_BUTTON(p2_human_but) , "Network" );
	gtk_box_pack_start( GTK_BOX( p2_box ) , p2_human_but , TRUE , TRUE , 2 );
	gtk_box_pack_start( GTK_BOX( p2_box ) , p2_ai_rand_but , TRUE , TRUE , 2 );
	gtk_box_pack_start( GTK_BOX( p2_box ) , p2_ai_simp_but , TRUE , TRUE , 2 );
	gtk_box_pack_start( GTK_BOX( p2_box ) , p2_net_but , TRUE , TRUE , 2 );
	gtk_container_add( GTK_CONTAINER( p2_frame ) , p2_box );
	gtk_box_pack_start( GTK_BOX( players_page ) , p2_frame , TRUE , TRUE , 2 );
	gtk_signal_connect( GTK_OBJECT( p2_human_but ),
			"clicked",
			GTK_SIGNAL_FUNC( p2_set_human ),
			NULL );

	gtk_signal_connect( GTK_OBJECT( p2_ai_rand_but ),
			"clicked",
			GTK_SIGNAL_FUNC( p2_set_random ),
			NULL );
	
	gtk_signal_connect( GTK_OBJECT( p2_ai_simp_but ),
			"clicked",
			GTK_SIGNAL_FUNC( p2_set_simple ),
			NULL );
	
	gtk_signal_connect( GTK_OBJECT( p2_net_but ),
			"clicked",
			GTK_SIGNAL_FUNC( p2_set_network ),
			NULL );
	
	players_page_label = gtk_label_new("Players");
	
	gnome_property_box_append_page( GNOME_PROPERTY_BOX( prop_box ) 
			, players_page , players_page_label );

	level_page = gtk_vbox_new( TRUE , 2 );
	level_frame = gtk_frame_new("Level");
	level_box = gtk_vbox_new( TRUE , 2 );
	beginner_but = gtk_radio_button_new_with_label(NULL,"Beginner");
	tournament_but = gtk_radio_button_new_with_label_from_widget(
			GTK_RADIO_BUTTON(beginner_but) , "Tournament" );
	gtk_box_pack_start( GTK_BOX( level_box ) , beginner_but , TRUE , TRUE , 2 ); 
	gtk_box_pack_start( GTK_BOX( level_box ) , tournament_but , TRUE , TRUE , 2 ); 
	gtk_container_add( GTK_CONTAINER( level_frame ) , level_box );
	gtk_box_pack_start( GTK_BOX( level_page ) , level_frame , TRUE , TRUE , 2 );
	level_page_label = gtk_label_new("Levels");
	goal_frame = gtk_frame_new("Goals");
	goal_box = gtk_vbox_new( TRUE , 2 );
	goal_but = gtk_check_button_new_with_label("Indicate Goals");
	gtk_box_pack_start( GTK_BOX( goal_box ) , goal_but , TRUE , TRUE , 2 );
	gtk_container_add( GTK_CONTAINER( goal_frame ) , goal_box );
	gtk_box_pack_start( GTK_BOX( level_page ) , goal_frame , TRUE , TRUE , 2 );
	gnome_property_box_append_page( GNOME_PROPERTY_BOX( prop_box ) ,
			level_page , level_page_label );
	gtk_signal_connect( GTK_OBJECT( beginner_but ),
			"clicked",
			GTK_SIGNAL_FUNC( level_beginner_set ),
			NULL );
	gtk_signal_connect( GTK_OBJECT( tournament_but ),
			"clicked",
			GTK_SIGNAL_FUNC( level_tournament_set ),
			NULL );
	gtk_signal_connect( GTK_OBJECT( goal_but ),
			"clicked",
			GTK_SIGNAL_FUNC( goal_markers_set ),
			NULL );

	
	gtk_signal_connect( GTK_OBJECT(prop_box) , 
			"apply" ,
			GTK_SIGNAL_FUNC(apply_handler),
			NULL );	
	
	gtk_signal_connect( GTK_OBJECT(prop_box) , 
			"help" ,
			GTK_SIGNAL_FUNC(help_handler),
			NULL );	

	// the network configuration
	net_page_label = gtk_label_new("Network");
	net_page = gtk_vbox_new( TRUE , 2 );
	host_entry = gtk_entry_new();
	gtk_entry_set_text( GTK_ENTRY (host_entry) , local_host_name );
	host_entry_label = gtk_label_new("Host Name");
	host_box = gtk_hbox_new( FALSE , 2 );
	gtk_box_pack_start( GTK_BOX( host_box ) , host_entry_label , FALSE , FALSE , 2);
	gtk_box_pack_start( GTK_BOX( host_box ) , host_entry , FALSE , FALSE , 2);
	gtk_box_pack_start( GTK_BOX( net_page ) , host_box , FALSE , FALSE , 2 );	
	
	gnome_property_box_append_page( GNOME_PROPERTY_BOX( prop_box ) ,
			net_page , net_page_label );
	
	gtk_signal_connect( GTK_OBJECT( host_entry ) ,
			"changed" ,
			GTK_SIGNAL_FUNC(host_changed_cb),
			NULL );

	
	set_buttons();
	gtk_widget_show_all( prop_box );
	
}


void apply_handler
( GtkWidget* prop_box , gint page , gpointer data )
{
	G_CONST_RETURN gchar* tmp;
	
	printf("apply signal caught\n");
	
	players[0].type = local_player1_state;
	players[1].type = local_player2_state;
	players[0].ai_type = local_player1_ai;
	players[1].ai_type = local_player2_ai;
	level = local_level;
	display_markers = local_display_markers;
	
	tmp = gtk_entry_get_text( GTK_ENTRY( host_entry ) );
	remote_addr = calloc( strlen(tmp) + 1, 1 );
	strncpy( remote_addr , tmp , strlen(tmp) );
	local_host_name = calloc( strlen(tmp) + 1, 1 );
	strncpy( local_host_name , tmp , strlen(tmp) );

	printf("host name was set to: %s\n", local_host_name );
	
	save_preferences();
}


void help_handler
( GtkWidget* prop_box , gint page , gpointer data )
{
	printf("THERE IS NO HELP\n");
}

void host_changed_cb
( GtkWidget* entry , gpointer data )
{
	G_CONST_RETURN gchar* tmp;
	tmp = gtk_entry_get_text( GTK_ENTRY( host_entry ) );

	if( strcmp( local_host_name , tmp ) != 0 ){
		gnome_property_box_changed( GNOME_PROPERTY_BOX( prop_box ) );
	}

}



void p1_set_human
( GtkWidget* button , gpointer data )
{
	if( GTK_TOGGLE_BUTTON( button )->active ){
		printf("p1_set_human called\n");
		local_player1_state = USE_GUI;
		if( local_player1_state != players[0].type ){
			gnome_property_box_changed( GNOME_PROPERTY_BOX( prop_box ) );
		}	
	}
}

void p1_set_random
( GtkWidget* button , gpointer data )
{
	if( GTK_TOGGLE_BUTTON( button )->active ){
		printf("p1_set_random called\n");
		local_player1_state = USE_AI;
		local_player1_ai = RANDOM_AI;
		if( local_player1_state != players[0].type || 
				local_player1_ai != players[0].ai_type ){
			gnome_property_box_changed( GNOME_PROPERTY_BOX( prop_box ) );
		}	
	}
}

void p1_set_simple
( GtkWidget* button , gpointer data )
{
	if( GTK_TOGGLE_BUTTON( button )->active ){
		printf("p1_set_simple called\n");
		local_player1_state = USE_AI;
		local_player1_ai = SIMPLE_AI;
		if( local_player1_state != players[0].type || 
				local_player1_ai != players[0].ai_type ){
			gnome_property_box_changed( GNOME_PROPERTY_BOX( prop_box ) );
		}	
	}
}

void p1_set_network
( GtkWidget* button , gpointer data )
{
	if( GTK_TOGGLE_BUTTON( button )->active ){
		printf("p1_set_network called\n");
		local_player1_state = USE_NET;
		if( local_player1_state != players[0].type ){
			gnome_property_box_changed( GNOME_PROPERTY_BOX( prop_box ) );
		}	
	}
}


void p2_set_human
( GtkWidget* button , gpointer data )
{
	if( GTK_TOGGLE_BUTTON( button )->active ){
		printf("p2_set_human called\n");
		local_player2_state = USE_GUI;
		if( local_player2_state != players[1].type ){
			gnome_property_box_changed( GNOME_PROPERTY_BOX( prop_box ) );
		}	
	}
}

void p2_set_random
( GtkWidget* button , gpointer data )
{
	if( GTK_TOGGLE_BUTTON( button )->active ){
		printf("p2_set_random called\n");
		local_player2_state = USE_AI;
		local_player2_ai = RANDOM_AI;
		if( local_player2_state != players[1].type ||
				local_player2_ai != players[1].ai_type ){
			gnome_property_box_changed( GNOME_PROPERTY_BOX( prop_box ) );
		}	
	}
}

void p2_set_simple
( GtkWidget* button , gpointer data )
{
	if( GTK_TOGGLE_BUTTON( button )->active ){
		printf("p2_set_random called\n");
		local_player2_state = USE_AI;
		local_player2_ai = SIMPLE_AI;
		if( local_player2_state != players[1].type ||
				local_player2_ai != players[1].ai_type ){
			gnome_property_box_changed( GNOME_PROPERTY_BOX( prop_box ) );
		}	
	}
}

void p2_set_network
( GtkWidget* button , gpointer data )
{
	if( GTK_TOGGLE_BUTTON( button )->active ){
		printf("p2_set_network called\n");
		local_player2_state = USE_NET;
		if( local_player2_state != players[1].type ){
			gnome_property_box_changed( GNOME_PROPERTY_BOX( prop_box ) );
		}	
	}
}

void level_beginner_set
( GtkWidget* button , gpointer data )
{
	if( GTK_TOGGLE_BUTTON( button )->active ){
		local_level = BEGINNER;
		if( local_level != level ){
			gnome_property_box_changed( GNOME_PROPERTY_BOX( prop_box ) );
		}	
	}
}

void level_tournament_set
( GtkWidget* button , gpointer data )
{
	if( GTK_TOGGLE_BUTTON( button )->active ){
		local_level = TOURNAMENT;
		if( local_level != level ){
			gnome_property_box_changed( GNOME_PROPERTY_BOX( prop_box ) );
		}	
	}
}

void goal_markers_set
( GtkWidget* button , gpointer data )
{
	if( GTK_TOGGLE_BUTTON( button )->active ){
		local_display_markers = TRUE;
	}
	else {
		local_display_markers = FALSE;
	}
	
	if( local_display_markers != display_markers ){
		gnome_property_box_changed( GNOME_PROPERTY_BOX( prop_box ) );
	}	
}

void load_preferences
()
{
	gchar* player1_str;
	gchar* player2_str;
	
	player1_str = gnome_config_get_string("/zertz/players/PlayerOne=Human");
	player2_str = gnome_config_get_string("/zertz/players/PlayerTwo=Human");

	if( strcmp( player1_str , HUMAN ) == 0 ){
		printf("Player one is human\n");
		players[0].type = USE_GUI;
	}
	else if( strcmp( player1_str , RANDOM ) == 0 ){
		printf("Player one is random\n");
		players[0].type = USE_AI;
		players[0].ai_type = RANDOM_AI;
	}
	else if( strcmp( player1_str , SIMPLE ) == 0 ){
		printf("Player one is simple\n");
		players[0].type = USE_AI;
		players[0].ai_type = SIMPLE_AI;
	}
	else if( strcmp( player1_str , NETWORK ) == 0 ){
		printf("Player one is network\n");
		players[0].type = USE_NET;
	}

	if( strcmp( player2_str , HUMAN ) == 0 ){
		printf("Player two is human\n");
		players[1].type = USE_GUI;
	}
	else if( strcmp( player2_str , RANDOM ) == 0 ){
		printf("Player two is random\n");
		players[1].type = USE_AI;
		players[1].ai_type = RANDOM_AI;
	}
	else if( strcmp( player2_str , SIMPLE ) == 0 ){
		printf("Player two is simple\n");
		players[1].type = USE_AI;
		players[1].ai_type = SIMPLE_AI;
	}
	else if( strcmp( player2_str , NETWORK ) == 0 ){
		printf("Player two is network\n");
		players[1].type = USE_NET;
	}

	local_player1_state = players[0].type;
	local_player2_state = players[1].type;
	local_player1_ai = players[0].ai_type;
	local_player2_ai = players[1].ai_type;
	
	level = gnome_config_get_int( "/zertz/levels/level=BEGINNER" );
	local_level = level;	
	
	display_markers = gnome_config_get_int( "/zertz/levels/goal_markers=TRUE" );
	local_display_markers = display_markers;
	
	local_host_name = gnome_config_get_string("/zertz/net/host=localhost" );
	
	remote_addr = calloc( strlen(local_host_name) + 1, 1 );
	strncpy( remote_addr , local_host_name , strlen(local_host_name) );
	
	if( players[0].type == USE_NET ){
		printf("Player one is using the net host: %s\n",local_host_name);
	}
	if( players[1].type == USE_NET ){
		printf("Player two is using the net host: %s\n",local_host_name);
	}
		
	
}

void save_preferences
()
{
	
	switch( players[0].type ){
		case USE_GUI:
			gnome_config_set_string( "/zertz/players/PlayerOne" , HUMAN );
			break;
		case USE_AI:
			if( players[0].ai_type == RANDOM_AI ){
				gnome_config_set_string( "/zertz/players/PlayerOne" , RANDOM );
			}
			else if( players[0].ai_type == SIMPLE_AI ){
				gnome_config_set_string( "/zertz/players/PlayerOne" , SIMPLE );
			}
			break;
		case USE_NET:
			gnome_config_set_string( "/zertz/players/PlayerOne" , NETWORK );
			break;
		default:
			gnome_config_set_string( "/zertz/players/PlayerOne" , HUMAN );
			break;
	}

	switch( players[1].type ){
		case USE_GUI:
			gnome_config_set_string( "/zertz/players/PlayerTwo" , HUMAN );
			break;
		case USE_AI:
			if( players[1].ai_type == RANDOM_AI ){
				gnome_config_set_string( "/zertz/players/PlayerTwo" , RANDOM );
			}
			else if( players[1].ai_type == SIMPLE_AI ){
				gnome_config_set_string( "/zertz/players/PlayerTwo" , SIMPLE );
			}
			break;
		case USE_NET:
			gnome_config_set_string( "/zertz/players/PlayerTwo" , NETWORK );
			break;
		default:
			gnome_config_set_string( "/zertz/players/PlayerTwo" , HUMAN );
			break;
	}
	
	gnome_config_set_int( "/zertz/levels/level" , level );
	gnome_config_set_int( "/zertz/levels/goal_markers" , display_markers );
	
	gnome_config_sync();
	
	gnome_config_set_string( "/zertz/net/host" , local_host_name );
	
}

void set_buttons
()
{
		
	switch( players[0].type ){
		case USE_GUI:
			gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(p1_human_but) , TRUE );
			break;
		case USE_AI:
			if( players[0].ai_type == RANDOM_AI ){
				gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(p1_ai_rand_but) , TRUE );
			}
			else if( players[0].ai_type == SIMPLE_AI ){
				gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(p1_ai_simp_but) , TRUE );
			}
			break;
		case USE_NET:
			gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(p1_net_but) , TRUE );
			break;
		default:
			//gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(p1_human_but) , TRUE );
			break;
	}

	switch( players[1].type ){
		case USE_GUI:
			gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(p2_human_but) , TRUE );
			break;
		case USE_AI:
			if( players[1].ai_type == RANDOM_AI ){
				gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(p2_ai_rand_but) , TRUE );
			}
			else if( players[1].ai_type == SIMPLE_AI ){
				gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(p2_ai_simp_but) , TRUE );
			}
			break;
		case USE_NET:
			gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(p2_net_but) , TRUE );
			break;
		default:
			//gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(p2_human_but) , TRUE );
			break;
	}
	
	if( level == BEGINNER ){
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(beginner_but) , TRUE );
	}
	else {
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(tournament_but) , TRUE );
	}

	if( display_markers == TRUE ){
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(goal_but) , TRUE );
	}
	
}



