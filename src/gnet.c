#include <stdio.h>  /* stdio included for perror only */
#include <unistd.h> /* unistd: read, close, write */
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

#include "gnet.h"


void
gnet_socket_init( int sock_fd, gnet_socket *gs )
{
  gs->sock_fd = sock_fd;
  gs->eof = 0;
  gs->end = gs->buffer + BUFFER_MAX;
  gs->cur = gs->end; /* want to start with no data */

} /* gnet_socket_init */


extern int
get_socket_net_master( int port, gnet_socket *gs )
{
  int s;
  int i_opt;
  struct sockaddr_in saddr;

  if ( (s=socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
    perror("Can't create socket");
    return -1;
  }
  /* set socket options */
  i_opt = 1;
  if ( (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &i_opt, sizeof(int)) < 0) ) {
    perror("Can't set socket options!");
    return -1;
  }
  /* set up socket addr/port info */
  bzero(&saddr, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = htonl(INADDR_ANY); /* let kernel assign IP */
  saddr.sin_port = htons( port );
  if ( bind(s, (SA*) &saddr, sizeof(saddr)) < 0 ) {
    perror("Failed bind");
    return -1;
  }
  /* put socket into listen mode */
  /* this "1024" gets truncated to SO_MAXCONN, `man listen' for details.
     (probably not portable beyond Linux) */
  if ( listen(s, 1024) < 0 ) {
    perror("Error on listen");
    return -1;
  }

  gnet_socket_init( s, gs );
  return gs->sock_fd; /* user shouldn't use this return value, but rather gs */

} /* get_socket_net_master */


extern int
gnet_socket_close( gnet_socket *gs )
{
  int rv;
  rv = close( gs->sock_fd );
  gs->sock_fd = -1; /* "invalid" */
  if ( rv<0 ) {
    perror("Error on close");
  }
  return rv;

} /* gnet_socket_close */


extern int
gnet_socket_accept( gnet_socket *srv_gs, gnet_socket *cli_gs )
{
  struct sockaddr_in cl_addr;
  socklen_t clen;
  int c;

  clen = sizeof(cl_addr);
  if ( (c=accept(srv_gs->sock_fd, (SA*) &cl_addr, &clen)) < 0 ) {
    perror("Error on accept");
    return -1;
  }
  
  gnet_socket_init( c, cli_gs );
  return cli_gs->sock_fd;

} /* gnet_socket_accept */


int
gnet_socket_buffer_read( gnet_socket *gs )
{
  int bread;

  if (gs->eof) return 0; /* don't try to read past eof */
  bread = read( gs->sock_fd, gs->buffer, BUFFER_MAX );
  if (bread==0) {
    gs->eof = 1;
  }
  if (bread <= 0) { /* 0 == EOF */
    return bread;
  } else {
    gs->cur = gs->buffer;
    gs->end = gs->cur + bread;
    return bread;
  }

} /* gnet_socket_buffer_read */  


int
gnet_is_readable( gnet_socket *gs )
{
  struct pollfd ufds[1];
  int ret_fd;

  /* Data remaining in the buffer means we can read: */
  if ( gs->cur < gs->end ) return 1;

  /* EOF'ed socket means we *can* read (it'll just fail!): */
  if ( gs->eof ) return 1;
  
  /* Otherwise, check if the socket's got readable data: */
  ufds[0].fd = gs->sock_fd;
  ufds[0].events = POLLIN | POLLPRI; /* interested in incoming data */
  ufds[0].revents = 0;
  ret_fd = poll(ufds, 1, 0); /* poll sock_fd, returning immediately */
  if ( ret_fd > 0 ) {
    /* events or errors reported in ufds */
    if ( (ufds[0].revents & (POLLIN|POLLPRI)) ) {
      return 1;
    }
  }

  /* We get here if sock_fd isn't ready to read (or if poll croaked) */
  return 0; /* backstop */

} /* gnet_is_readable() */


extern ssize_t
gnet_readline( gnet_socket *gs, void *retbuf, ssize_t maxlen )
{
  int n, bread;
  char c, *ptr;

  ptr = retbuf;
  for( n=0; n<maxlen; n++ ) {
    if ( gs->cur >= gs->end ) { /* at end of buffer, need to read more */
      if (gs->eof) break; /* stop after eof, once buffer is expended  */
      bread = gnet_socket_buffer_read( gs );
      if (bread<0) {
	perror("Error reading socket");
	return -1; /* XXX should we NULL *ptr before returning? */
      } 
    }

    c = *(gs->cur++); /* two-step, so we can examine copied char */
    *ptr++ = c;
    if ( c == '\n' ) {
      break;
    }
  }
  *ptr = 0; /* terminate with NULL */
  return (n);

} /* gs_test_readline */

extern ssize_t
gnet_writestring( gnet_socket *gs, char *str )
{
  return write( gs->sock_fd, str, strlen(str) );
}


extern int 
get_socket_net_servant( char *host, int port, gnet_socket *gs )
{
  int sock;
  struct sockaddr_in saddr;
  struct hostent *ent;
  char ip[INET_ADDRSTRLEN];
  u_char *p;

  /* resolve hostname to ip addr */
  if ( (ent = gethostbyname(host)) == NULL ) {
    printf("Can't resolve hostname '%s'.\n", host);
    return -1;
  }
  /* there must be a better way: */
  p = (u_char *)(ent->h_addr_list[0]);
  snprintf(ip, INET_ADDRSTRLEN, "%d.%d.%d.%d", p[0],p[1],p[2],p[3]);
  /*printf("Resolved addr to %s\n", ip);*/
  
  if ( (sock=socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
    perror("Can't create socket");
    return -1;
  }
  /* set up addr/port info for server to connect to */
  bzero(&saddr, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons( port );
  if (! inet_aton( ip, &saddr.sin_addr ) ) {
    perror("Can't figure out that host ip");
    return -1;
  }
  /* connect */
  if ( connect(sock, (SA*)&saddr, sizeof(saddr)) < 0 ) {
    perror("Can't connect");
    return -1;
  }

  gnet_socket_init( sock, gs );
  return sock;  
  
} /* get_socket_net_servant */


#ifdef GNET_TESTING

extern int
gnet_dummy_test( void )
{  return 251; }

extern void
gnet_test_echo( int fd )
{

  ssize_t n;
  char buf[4096];
  char *p;
  
  p = buf;
  for(;;) {
    n = read(fd, p, (buf+4096-p) ); /* totally not safe XXX prolly ob1 */
    if (n<0) {
      perror("error on read");
      return;
    } else if (n==0) {
      /* eof */
      break;
    } else {
      p+=n;
    }
  }
  printf("got: %s\n", buf);

}

#endif /* ifdef GNET_TESTING */
