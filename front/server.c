#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>       
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <pcap.h>

#define MAX_LINE 4096 
#define MAX_PACKET_SIZE 4096 
#define FILTER_RULE "tcp"

/*----------------------------------------------------*/
static void
usage(const char *prog) 
{
  fprintf(stderr, "Usage: %s port\n"
	  "\tThe port number should be between 1024-65535.\n", 
	  prog);
}

void
init_pcap()
{
  bpf_u_int32 bpfnet = 0;
  struct bpf_program *bpf;
  if (pcap_compile_nopcap(MAX_PACKET_SIZE, DLT_EN10MB, bpf, FILTER_RULE, 0, bpfnet)) {
    fprintf(stderr, "Cannot compile BPF filter\n");
  }
}

/*----------------------------------------------------*/
int 
main(const int argc, const char **argv)
{
  int port, s, c, len;
  struct sockaddr_in saddr = {0}; /* {0} intializes all fields to 0 */
  char buf[MAX_LINE];

  /* check the command line arguments */
  if (argc < 2 ||                       /* # of argument too small */
      (port = atoi(argv[1])) < 1024 ||  /* port should be in 1024-65535 */
      port > 65535) {
    usage(argv[0]);
    exit(-1);
  }

  /* create the listening socket */
  if ((s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    perror("Error:socket() failed");
    exit(-1);
  }

  /* bind on the specified port */
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = INADDR_ANY;
  saddr.sin_port = htons(port);
  if (bind(s, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
    perror("Error:bind() failed");
    exit(-1);
  }

  /* listen on the port */
  if (listen(s, 1024) < 0) {
    perror("Error: listen() failed");
    exit(-1);
  }

  init_pcap();
  /* accept a connection and handle it in a forked process */
  while ((c = accept(s, NULL, NULL)) >= 0) {
    /* read a line from client */
    while (len = read(c, buf, sizeof(buf)-1)) {
      if (len <= 0) {
        perror("Error: read() failed\n");
        exit(-1);
      }
      /* TODO implement */
      printf("%s\n", buf);
    }
    close(c);
  }
  perror("Error:accept() failed");
  close(s);
  return(-1);
}
