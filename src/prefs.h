#ifndef PREFSH
#define PREFSH

#include <gnome.h>

// for turn_stage
#define PLACING 1
#define REMOVING 2
#define JUMPING 3

// for game_mode
#define STOP 0
#define VICTORY_ONE 11
#define VICTORY_TWO 22

// for playerX_state
#define USE_GUI 1
#define USE_AI 2
#define USE_NET 3

#define RANDOM_AI 1
#define SIMPLE_AI 2

#define RANDOM "Random"
#define SIMPLE "Simple"
#define NETWORK "Network"
#define HUMAN "Human"


void prefs_cb( GtkWidget* , gpointer );

void apply_handler( GtkWidget* , gint , gpointer );
void help_handler( GtkWidget* , gint , gpointer );


void p1_set_human( GtkWidget* , gpointer );
void p1_set_random( GtkWidget* , gpointer );
void p1_set_simple( GtkWidget* , gpointer );
void p1_set_network( GtkWidget* , gpointer );
void p2_set_human( GtkWidget* , gpointer );
void p2_set_random( GtkWidget* , gpointer );
void p2_set_simple( GtkWidget* , gpointer );
void p2_set_network( GtkWidget* , gpointer );

void level_beginner_set( GtkWidget* , gpointer );
void level_tournament_set( GtkWidget* , gpointer );
void goal_markers_set( GtkWidget* , gpointer );


void host_changed_cb( GtkWidget* , gpointer );

void load_preferences();
void save_preferences();
void set_buttons();

#endif


