#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>       
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pcap.h>

#include "main.h"
#include "index.h"

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
init_query_context(query_context_t *qctx, index_array_context_t *ictx)
{
	struct bpf_program *bpf = (struct bpf_program *)malloc(sizeof(struct bpf_program));
	qctx->bpf = bpf;
	qctx->ictx = ictx;
}

static int
compile_pcap(query_context_t *qctx)
{
	bpf_u_int32 bpfnet = 0;
	if (pcap_compile_nopcap(MAX_PACKET_SIZE, DLT_EN10MB, qctx->bpf, qctx->bpf_query, 0, bpfnet)) {
		return -1;
	}
	return 0;
}

static void
bpf_loop(query_context_t *qctx, char buf[])
{
	/* TODO implement */
	/*
	u_char *pkt;
	int len;
	struct bpf_insn *pc = qctx->bpf->bf_insns;
	   if (bpf_filter(pc, pkt, len, MAX_PACKET_SIZE) == 0)
	   printf("bpf\n");
	   */
	/*print_index_array(ctx, TYPE_SADDR);*/
	/*int *search_result = search_from_index_array(ctx, TYPE_SADDR, value);*/

	/*int *search_result = search_range_from_index_array(ctx, TYPE_SADDR, 10000000, 1000000000);*/
	/*if (search_result != NULL)*/
	/*printf("s: %d e: %d\n", search_result[0], search_result[1]);*/

}

static int
parse_query(query_context_t *qctx, char buf[])
{
	int success = 0;
	char *ptr;
	ptr = strtok(buf, ",:-");
	do {
		if (strcmp(ptr, "stime") == 0) {
			ptr = strtok(NULL, ",:-");
			qctx->stime = atoi(ptr);
			success++;
		}

		else if (strcmp(ptr, "etime") == 0) {
			ptr = strtok(NULL, ",:-");
			qctx->etime = atoi(ptr);
			success++;
		}

		else if (strcmp(ptr, "src_ip") == 0) {
			ptr = strtok(NULL, ",:-");
			qctx->fsaddr = atoi(ptr);
			ptr = strtok(NULL, ",:-");
			qctx->lsaddr = atoi(ptr);
			success++;
		}

		else if (strcmp(ptr, "dst_ip") == 0) {
			ptr = strtok(NULL, ",:-");
			qctx->fdaddr = atoi(ptr);
			ptr = strtok(NULL, ",:-");
			qctx->ldaddr = atoi(ptr);
			success++;
		}

		else if (strcmp(ptr, "src_port") == 0) {
			ptr = strtok(NULL, ",:-");
			qctx->sport = atoi(ptr);
			success++;
		}

		else if (strcmp(ptr, "dst_port") == 0) {
			ptr = strtok(NULL, ",:-");
			qctx->dport = atoi(ptr);
			success++;
		}

		else if (strcmp(ptr, "bpf") == 0) {
			ptr = strtok(NULL, ",:-");
			qctx->bpf_query = ptr;
			success++;
		}

		else 
			return -1;
	} while (ptr = strtok(NULL, ",:-"));
	if (success != 7)
		return -1;
	printf("%d, %d, %d, %d, %d, %d, %d, %d, %s\n", qctx->stime, qctx->etime, qctx->fsaddr, qctx->lsaddr, qctx->fdaddr, qctx->ldaddr, qctx->sport, qctx->dport, qctx->bpf_query);
	return 1;
}


/**
 * main for index array index structure
 * @param ctx [description]
 */
static void
insert_rand_data(index_array_context_t *ictx, bloom_filter_context_t *bctx, int size)
{
	init_index_array(ictx, bctx, size);

	bool done = false;

	srand(time(NULL));

	FlowMeta *meta = (FlowMeta *)malloc(sizeof(FlowMeta));
	FlowInfo *info = &meta->flowinfo;

	int cnt = 0;
	while(!done) {
		info->saddr = rand();
		info->daddr = rand();
		info->sport = rand();
		info->dport = rand();

		if (insert_into_index_array(ictx, meta))
			done = true;
	}

	write_index_array(ictx);
}

/**
 * exit for index array index structure
 * @param ctx [description]
 */
static void
index_array_exit(index_array_context_t *ictx)
{
	//free_index_array(ctx);	
}

/**
 * main function
 * @param  argc [description]
 * @param  argv [description]
 * @return      [description]
 */
int
main(const int argc, const char *argv[])
{
	int s, c, len;
	char buf[MAX_LINE];

	s = init_socket(get_port(argc, argv));



	/* Start insert */
	index_array_context_t *ictx = (index_array_context_t *)malloc(sizeof(index_array_context_t));
	bloom_filter_context_t *bctx = (bloom_filter_context_t *)malloc(sizeof(bloom_filter_context_t));
	insert_rand_data(ictx, bctx, 30000);
	/* End insert*/


	query_context_t *qctx = (query_context_t *)malloc(sizeof(query_context_t));
	init_query_context(qctx, ictx);

	while ((c = accept(s, NULL, NULL)) >= 0) {
		while (len = read(c, buf, sizeof(buf)-1)) {
			if (len <= 0) { perror("Error: read() failed\n"); exit(-1); }
			buf[len] = 0;

			if (parse_query(qctx, buf) < 0) {
				fprintf(stderr, "Not enough query options\n");
				write(c, "error", 5); //TODO edit
				break;
			}

			if (compile_pcap(qctx) < 0) {
				fprintf(stderr, "Cannot compile BPF filter\n");
				write(c, "error", 5); //TODO edit
				break;
			}

			bpf_loop(qctx, buf);

		}
		close(c);
	}
	perror("Error:accept() failed");
	close(s);


	index_array_exit(ictx);
	return 0;
}
