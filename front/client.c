#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>       
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>

#define MAX_LINE 4096

/*----------------------------------------------------*/
static void
usage(const char *prog) 
{
  fprintf(stderr, "Usage: %s server-name port\n"
	  "\tThe port number should be between 1024-65535.\n", 
	  prog);
}

/*----------------------------------------------------*/
int 
main(const int argc, const char **argv)
{
  int port;
  int s;
  int len, res;
  struct sockaddr_in saddr = {0}; /* {0} intializes all fields to 0 */
  struct hostent *hp;
  char buf[MAX_LINE];

  /* check the command line arguments */
  if (argc < 3 ||                       /* # of argument too small */
      (port = atoi(argv[2])) < 1024 ||  /* port should be in 1024-65535 */
      port > 65535) {
    usage(argv[0]);
    exit(-1);
  }

  /* create a socket */
  if ((s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    perror("Error:socket() failed");
    exit(-1);
  }

  /* do a name lookup and get the IP address */
  if ((hp = gethostbyname(argv[1])) == NULL) {
	perror("Error: gethostbyname() failed\n");
	exit(-1);
  }

  /* connect to the server */
  saddr.sin_family = AF_INET;
  memcpy(&saddr.sin_addr.s_addr, hp->h_addr, hp->h_length);
  saddr.sin_port = htons(port);
  if (connect(s, (const struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
    perror("Error: connect() failed");
    exit(-1);
  }

  /* read one line from stdin */
  if (fgets(buf, sizeof(buf), stdin) == NULL) {
	perror("Error: fgets() failed");
    exit(-1);
  }

  /* send the line to server */
  len = strlen(buf);
  if (write (s, buf, len) <= 0) {
	perror("Error: write() failed\n");
	exit(-1);
  }
  
  /* receive the line from server and print it */
  if ((res = read(s, buf, sizeof(buf)-1)) <= 0) {
	perror("Error: read() failed\n");
	exit(-1);
  }
  buf[res] = 0;
  printf("received: %s", buf);

  close(s);
  return(0);
}