#include "net_to_move.h"
#include "zertz_net.h"
#include "move.h"

#include <stdio.h>
#include <stdlib.h>

/* A single move will be composed of multiple zertznet messages */
/* Dies (!) on zn_{send,recv}msg() failures */
void
recv_network_move( gnet_socket* gs, Move* move )
{
	zertznet_message msg;
	int rcvd_end_turn = 0;

	/* read messages until a ZN_MSG_ENDTURN is recieved,
	 * assembling the messages into the Move struct */
	initialize_move( move );
	while( ! rcvd_end_turn ) {
		if ( zn_recvmsg( gs, &msg ) < 0 ) { 
			fprintf(stderr, "Error recieving net player turn!\n");
			exit(-1);
		}
		printf(">reading<\n");
		zn_dumpmsg( stdout, &msg ); // XXX debug remove!

		switch ( msg.msgtype ) {
		case ZN_MSG_REMDISC:
			move_set_remove_data( move,
										 msg.data.remdisc.x, msg.data.remdisc.y );
			break;
		case ZN_MSG_PLACE:
			move_set_place_data( move,	msg.data.place.color,
										msg.data.place.x, msg.data.place.y );
			break;
		case ZN_MSG_JUMP:
			move_add_jump( move,
								msg.data.jump.from_x, msg.data.jump.from_y,
								msg.data.jump.to_x, msg.data.jump.to_y );
			break;
		case ZN_MSG_ENDTURN:
			rcvd_end_turn = 1;
			break;
		case ZN_MSG_ERROR:
			fprintf(stderr, "Error indicated from client end!\n");
			zn_dumpmsg( stderr, &msg );
			/* no break here: fall thru to default and croak */
		case ZN_MSG_GREETING:
		default:
			fprintf(stderr, "Got an invalid message from net player!\n");
			exit(-1);
		}
	}

} /* recv_network_move() */


/** For use by send_network_move() only.
 *  Handles failures and saves local redundancy. */
static void
Sendmsg( gnet_socket* gs, zertznet_message* msg )
{
	// printf("<sending>\n"); // XXX remove!
	// zn_dumpmsg( stdout, msg ); // XXX remove!
	if ( zn_sendmsg(gs,msg) < 0 ) {
		fprintf(stderr,"Error sending to remove net player!\n");
		exit(-1);
	}
}

/*
 * Build and send out the appropriate zertznet_messages for the given move.
 * A single move is represented by multiple ZN messages.
 */
void
send_network_move( gnet_socket* gs, Move* move )
{
	zertznet_message msg;
	Jump* jump;

	if ( move->type == MOVE_TYPE_JUMP ) {
		jump = move->jump;
		while( jump != NULL ) {
			zn_makemsg_jump( &msg,
								  jump->start_column, jump->start_row,
								  jump->end_column, jump->end_row );
			Sendmsg( gs, &msg );
			jump = jump->multi;
		}

	} else { /* place and remove */
		zn_makemsg_place( &msg, 
								move->place_color,
								move->place_column, move->place_row );
		Sendmsg( gs, &msg );
		zn_makemsg_remdisc( &msg, move->remove_column, move->remove_row );
		Sendmsg( gs, &msg );

	}
	/* send endturn message for Closure */
	zn_makemsg_endturn( &msg );
	Sendmsg( gs, &msg );

} /* send_network_move() */




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
