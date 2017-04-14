#ifndef __ZERTZ_NET_H
#define __ZERTZ_NET_H

#include <sys/types.h>

#include "gnet.h"

#define ZERTZ_NET_PROTO_VERSION     "0.9.1"

#define ZN_DEFAULT_PORT             23472
#define ZN_DEFAULT_CLIENT_HOST      "localhost"

/* printf formats for line proto commands */
/* note that scanf formats are not 100% compatible with printf */
/* note also the string length limits, which are pretty arbitrary */
#define ZN_FMT_GREETING "zertznet %10s\n" /* proto-version, mode */
#define ZN_FMT_NEW_GAME "new_game %d %d\n" /* level , player_num */
#define ZN_FMT_REMDISC  "remdisc %d %d\n" /* x, y */
#define ZN_FMT_PLACE    "place %d %d %d\n" /* color, x, y */
#define ZN_FMT_JUMP     "jump %d %d %d %d\n" /* from_x,from_y, to_x,to_y */
#define ZN_FMT_ENDTURN  "done\n"
#define ZN_FMT_ERROR    "abort %30s\n" /* death message */
#define ZN_FMT_ERROR_PFX "abort " /* can't scanf incoming error msgs */

/* Enumeration for zertznet_message. */
enum zn_msg_type {
  ZN_MSG_GREETING,
  ZN_MSG_NEW_GAME,
  ZN_MSG_REMDISC,
  ZN_MSG_PLACE,
  ZN_MSG_JUMP,
  ZN_MSG_ENDTURN,
  ZN_MSG_ERROR
};

/* number of chars per message line, max */
#define MAX_LINE 80

/*
 * This is a little arcane, but hey.
 * This struct gives a common format for the transfer of znet messages.
 * Get at the union members like this:
 *     zertznet_message zm;
 *     zm.msgtype = ZN_MSG_REMDISC;
 *     zm.data.remdisc.x = 2;
 *     zm.data.remdisc.y = 51;
 * Note the (small) length restrictions on the char arrays.
 * Note also that there is no data for ZN_MSG_ENDTURN.
 */
typedef struct {
  enum zn_msg_type msgtype;
  union {
    struct { char verstr[MAX_LINE]; } greeting;
	 struct { int level, player_num;} newgame;
    struct { int x, y; } remdisc;
    struct { int color, x, y; } place;
    struct { int from_x, from_y, to_x, to_y; } jump;
    struct { char errstr[MAX_LINE]; } error;
  } data;
} zertznet_message;

/*
 * Send and recieve znet messages via a gnet_socket (established elsewhere).
 */
extern int zn_sendmsg( gnet_socket *gs, zertznet_message *msg );
extern int zn_recvmsg( gnet_socket *gs, zertznet_message *msg );

extern void zn_dumpmsg( void *stream, zertznet_message *msg );

/* Fns for constructing messages, so you don't have to get your hands dirty. */
extern void
zn_makemsg_greeting( zertznet_message *msg, char *verstr );
extern void zn_makemsg_newgame( zertznet_message* zm , int level , int player_num );
extern void zn_makemsg_remdisc( zertznet_message *zm, int x, int y );
extern void zn_makemsg_place( zertznet_message *zm, int color, int x, int y );
extern void 
zn_makemsg_jump( zertznet_message *zm, int fx, int fy, int tx, int ty );
extern void zn_makemsg_endturn( zertznet_message *zm );
extern void zn_makemsg_error( zertznet_message *zm, char *errmsg );

#endif
