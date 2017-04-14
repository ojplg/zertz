#include "zertz_net.h"
#include "gnet.h"

#include <stdio.h>
#include <string.h>

/*
 * Send a zertznet message over a gnet_socket, which is specified in zm.
 * Returns <0 on error.
 */
extern int
zn_sendmsg( gnet_socket *gs, zertznet_message *zm )
{
  char msgline[MAX_LINE];
  
  /* create string to send, fill in data appropriate for the message type */
  switch (zm->msgtype) {

  case ZN_MSG_GREETING:
    snprintf(msgline, MAX_LINE, ZN_FMT_GREETING, 
	     zm->data.greeting.verstr );
    break;

	case ZN_MSG_NEW_GAME:
	 snprintf(msgline, MAX_LINE, ZN_FMT_NEW_GAME,
			 zm->data.newgame.level , zm->data.newgame.player_num );
	 break;
	 
  case ZN_MSG_REMDISC:
    snprintf(msgline, MAX_LINE, ZN_FMT_REMDISC,
	     zm->data.remdisc.x, zm->data.remdisc.y);
    break;

  case ZN_MSG_PLACE:
    snprintf(msgline, MAX_LINE, ZN_FMT_PLACE,
	     zm->data.place.color, zm->data.place.x, zm->data.place.y);
    break;

  case ZN_MSG_JUMP:
    snprintf(msgline, MAX_LINE, ZN_FMT_JUMP,
	     zm->data.jump.from_x, zm->data.jump.from_y,
	     zm->data.jump.to_x, zm->data.jump.to_y);
    break;

  case ZN_MSG_ENDTURN:
    snprintf(msgline, MAX_LINE, ZN_FMT_ENDTURN); /* no data */
    break;

  case ZN_MSG_ERROR:
    snprintf(msgline, MAX_LINE, ZN_FMT_ERROR, zm->data.error.errstr);
    break;

  default:
    fprintf(stderr, "Attempt to call zn_sendmsg() with unknown message type (how did you get here?)\n");
    return -1;
    
  } /* switch */

  return gnet_writestring(gs, msgline);  

} /* zn_sendmsg */


/*
 * Recieve a zertznet message on gnet_socket gs. 
 * Fills zm with the parsed-out data from the message.
 * [Will block until the next message is sent over gs.]
 * Returns <0 on error.
 */
extern int
zn_recvmsg( gnet_socket *gs, zertznet_message *zm )
{
  char line[MAX_LINE];
  int bytes_read;
  // for parsing error messages:
  char* linemsg;
  int leadlen;

  /* get a line off the socket */
  if ( (bytes_read=gnet_readline( gs, line, MAX_LINE )) < 0 ) {
    return -1;
  }

  /* ugly sscanf tree */
  if ( sscanf(line, ZN_FMT_GREETING, 
	      zm->data.greeting.verstr ) > 0
       ) {
    zm->msgtype = ZN_MSG_GREETING;
    
  } else if( sscanf( line, ZN_FMT_NEW_GAME,
			 &zm->data.newgame.level , &zm->data.newgame.player_num ) > 0
		  ) {
	  zm->msgtype = ZN_MSG_NEW_GAME;
	} else if ( sscanf(line, ZN_FMT_REMDISC,
		     &zm->data.remdisc.x, &zm->data.remdisc.y) > 0
	      ) {
    zm->msgtype = ZN_MSG_REMDISC;

  } else if ( sscanf(line, ZN_FMT_PLACE,
		     &zm->data.place.color, &zm->data.place.x, &zm->data.place.y) > 0
	      ) {
    zm->msgtype = ZN_MSG_PLACE;

  } else if ( sscanf(line, ZN_FMT_JUMP,
		     &zm->data.jump.from_x, &zm->data.jump.from_y,
		     &zm->data.jump.to_x, &zm->data.jump.to_y) > 0
	      ) {
    zm->msgtype = ZN_MSG_JUMP;

  } else if ( ! strncmp(line, ZN_FMT_ERROR_PFX, strlen(ZN_FMT_ERROR_PFX)) ) {
    /* XXX should trim trailing \n */
    linemsg = line + strlen(ZN_FMT_ERROR_PFX);
    leadlen = strspn(linemsg, " \t\n"); // advance over leading whitespace
    linemsg = linemsg + leadlen;
    strncpy( zm->data.error.errstr, linemsg, MAX_LINE-1 );
    zm->msgtype = ZN_MSG_ERROR;

  } else if ( ! strncmp(line, ZN_FMT_ENDTURN, strlen(ZN_FMT_ENDTURN)) ) {
    /* ENDTURN is special, since there's no message data */
    /* XXX unsafe */
    zm->msgtype = ZN_MSG_ENDTURN;

  } else {
    fprintf(stderr, "Recieved unknown message type.\n");
    return -1;
  }
  /* end ugly sscanf tree */

  return 0; /* if we made it this far, zm is all filled in, hooray */

} /* zn_recvmsg */


/* 
 * This is for debugging purposes, mainly.
 * Dumps a report of the contents of the given message to the stream.
 */
extern void
zn_dumpmsg( void *stream, zertznet_message *zm ) {

  switch (zm->msgtype) {

  case ZN_MSG_GREETING:
    fprintf(stream, "GREETING (verstr='%s')\n", 
	    zm->data.greeting.verstr );
    break;

  case ZN_MSG_NEW_GAME:
    fprintf(stream, "NEW_GAME (level=%d  player=%d)\n", 
	    zm->data.newgame.level , zm->data.newgame.player_num );
    break;

  case ZN_MSG_REMDISC:
    fprintf(stream, "REMDISC (pos=(%d, %d))\n", 
	    zm->data.remdisc.x, zm->data.remdisc.y);
    break;

  case ZN_MSG_PLACE:
    fprintf(stream, "PLACE (color=%d, pos=(%d, %d))\n",
	    zm->data.place.color, zm->data.place.x, zm->data.place.y);
    break;

  case ZN_MSG_JUMP:
    fprintf(stream, "JUMP (from=(%d, %d), to=(%d, %d))\n",
	    zm->data.jump.from_x, zm->data.jump.from_y,
	    zm->data.jump.to_x, zm->data.jump.to_y);
    break;

  case ZN_MSG_ENDTURN:
    fprintf(stream, "ENDTURN (no message data)\n");
    break;

  case ZN_MSG_ERROR:
    fprintf(stream, "ERROR (msg='%s')\n", zm->data.error.errstr);
    break;

  default:
    fprintf(stream, "Attempt to call zn_dumpmsg() with unknown message type (how did you get here?)\n");
    
  } /* switch */
  
} /* zn_dumpmsg */

extern void
zn_makemsg_greeting( zertznet_message *zm, char *verstr )
{
  zm->msgtype = ZN_MSG_GREETING;
  strncpy( zm->data.greeting.verstr, verstr, MAX_LINE );
}

extern void
zn_makemsg_newgame( zertznet_message *zm, int level , int player_num )
{
  zm->msgtype = ZN_MSG_NEW_GAME;
  zm->data.newgame.level = level;
  zm->data.newgame.player_num = player_num;
}

extern void
zn_makemsg_remdisc( zertznet_message *zm, int x, int y )
{
  zm->msgtype = ZN_MSG_REMDISC;
  zm->data.remdisc.x = x;
  zm->data.remdisc.y = y;
}

extern void
zn_makemsg_place( zertznet_message *zm, int color, int x, int y )
{
  zm->msgtype = ZN_MSG_PLACE;
  zm->data.place.color = color;
  zm->data.place.x = x;
  zm->data.place.y = y;
}

extern void 
zn_makemsg_jump( zertznet_message *zm, int fx, int fy, int tx, int ty )
{
  zm->msgtype = ZN_MSG_JUMP;
  zm->data.jump.from_x = fx;
  zm->data.jump.from_y = fy;
  zm->data.jump.to_x = tx;
  zm->data.jump.to_y = ty;
}

extern void
zn_makemsg_endturn( zertznet_message *zm )
{
  zm->msgtype = ZN_MSG_ENDTURN;
}

extern void
zn_makemsg_error( zertznet_message *zm, char *errmsg )
{
  zm->msgtype = ZN_MSG_ERROR;
  strncpy( zm->data.error.errstr, errmsg, MAX_LINE );
}

