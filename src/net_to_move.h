#ifndef __NET_TO_MOVE_H
#define __NET_TO_MOVE_H

/** 
 *  Functions to read and send zertznet messages on gnet_sockets,
 *  based on the data contained in Move structs.
 */

#include "move.h"
#include "zertz_net.h"

/* Note: these currently abort the program on net errors (!) */
void recv_network_move( gnet_socket* gs, Move* move );
void send_network_move( gnet_socket* gs, Move* move );

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
