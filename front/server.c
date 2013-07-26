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

#include "server.h"

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

static int
get_port(const int argc, const char **argv)
{
  int port;

  /* check the command line arguments */
  if (argc < 2 ||                       /* # of argument too small */
      (port = atoi(argv[1])) < 1024 ||  /* port should be in 1024-65535 */
      port > 65535) {
    usage(argv[0]);
    exit(-1);
  }
  return port;
}

static int
init_socket(int port)
{
  int s;
  struct sockaddr_in saddr = {0}; /* {0} intializes all fields to 0 */

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

  return s;
}

static void
init_bpf_context(bpf_context_t *ctx)
{
  struct bpf_program *bpf = (struct bpf_program *)malloc(sizeof(struct bpf_program));
  ctx->bpf = bpf;
}

static int
compile_pcap(bpf_context_t *ctx, char *bpf_query)
{
  bpf_u_int32 bpfnet = 0;
  if (pcap_compile_nopcap(MAX_PACKET_SIZE, DLT_EN10MB, ctx->bpf, bpf_query, 0, bpfnet)) {
    return -1;
  }
  return 0;
}

static void
bpf_loop(bpf_context_t *ctx, char buf[])
{
  /* TODO implement */
  u_char *pkt;
  int len;
  struct bpf_insn *pc = ctx->bpf->bf_insns;
  /*
  if (bpf_filter(pc, pkt, len, MAX_PACKET_SIZE) == 0)
    printf("bpf\n");
  */
 
  printf("%s\n", buf);
}

/*----------------------------------------------------*/
int 
main(const int argc, const char **argv)
{
  int s, c, len;
  char buf[MAX_LINE];

  s = init_socket(get_port(argc, argv));

  /* initialize bpf */
  bpf_context_t *ctx = (bpf_context_t *)malloc(sizeof(bpf_context_t));
  init_bpf_context(ctx);

  /* accept a connection and handle it in a forked process */
  while ((c = accept(s, NULL, NULL)) >= 0) {
    /* read a line from client */
    while (len = read(c, buf, sizeof(buf)-1)) {
      if (len <= 0) { perror("Error: read() failed\n"); exit(-1); }

      char *bpf_query = buf;
      if (compile_pcap(ctx, bpf_query) < 0) {
        fprintf(stderr, "Cannot compile BPF filter\n");
        write(c, "error", 5); //TODO edit
        break;
      }

      write(c, buf, len);
      buf[len] = 0;

      /* bpf loop */
      bpf_loop(ctx, buf);

    }
    printf("close\n");
    close(c);
  }
  perror("Error:accept() failed");
  close(s);
  return(-1);
}
