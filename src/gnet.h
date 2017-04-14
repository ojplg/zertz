#ifndef __GNET_H
#define __GNET_H

/*
 * Library encapsulating line-oriented socket communications.
 * $Id: gnet.h,v 1.3 2002/09/27 15:23:41 gdf Exp $ 
 */

/* struct sockaddr, ssize_t, etc: */
#include <sys/types.h>

#define GNET_VERSION = "0.11";

/* typing saver: */
#define SA struct sockaddr

/* XXX define bzero here if necc via configure */


/*
   Structure for reading buffered data from socket.
*/
#define BUFFER_MAX 4096
typedef struct {
  int sock_fd;
  int eof; /* init to 0, true means eof reached */
  char buffer[BUFFER_MAX]; /* BUFFERMAX should be a member here */
  char *cur; /* initialized to buffer */
  char *end; /* initialized to buffer+BUFFER_MAX */
} gnet_socket;

/* 
   Create and set up a socket listening on the given port,
   for game state communications.

   port: port to open master socket on.
   gs: gnet_socket structure to fill with socket info.
   returns -1 on error; initializes gs on success.
   (errors are noisy on stderr)
*/
extern int get_socket_net_master( int port, gnet_socket *gs );

/*
   Create and set up a socket connected to the given host and port,
   for game state communications.

   host: string containing hostname or ip to connect to.
   port: port master process is to be listening on.
   gs: gnet_socket to be filled with socket infos.
   returns -1 on error; initializes gs on success.
*/
extern int get_socket_net_servant( char *host, int port, gnet_socket *gs );

/*
   Close socket, clean up gs.
*/
extern int gnet_socket_close( gnet_socket *gs );

/*
   Accept a connection on srv_gs.  Blocking.
*/
extern int
gnet_socket_accept( gnet_socket *srv_gs,  gnet_socket *cli_gs );

/*
   Fill buffer with a string consisting of the next \n-terminated line
   read from gs, up to maxlen chars.  If EOF is reached, the string
   may not be \n-terminated.

   gs: buffered socket to read from.
   buffer: string buffer to read into.
   maxlen: size of buffer.
   returns chars read into buffer (<=maxlen).

   XXX unsafe: needs a go-over for boundary conditions and fenceposts
*/
extern ssize_t 
gnet_readline( gnet_socket *gs, void *buffer, ssize_t maxlen );


extern ssize_t gnet_writestring( gnet_socket *gs, char *str );

/* gnet_is_readable returns true if a gnet_readline() should
 * return data without blocking.
 * "Should return data without blocking" means
 */
int gnet_is_readable( gnet_socket *gs );


/* 
   sock_path: path to FIFO
   returns socket fd, or -1 on error.
*/
/* extern int get_socket_local_master( char * sock_path ); */




/* XXX */
#ifdef GNET_TESTING
#include <stdio.h>
extern int gnet_dummy_test( void );
extern void gnet_test_echo( int fd );
#endif

#endif
