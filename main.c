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
#include "util.h"

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
/*----------------------------------------------------*/



/*----------------------------------------------------*/
static void
init_query_context(query_context_t *qctx, index_array_context_t *ictx)
{
	struct bpf_program *bpf = (struct bpf_program *)malloc(sizeof(struct bpf_program));
	qctx->bpf = bpf;
	qctx->ictx = ictx;
	qctx->no_bpf = false;
}

static int
compile_pcap(query_context_t *qctx)
{
	if (qctx->no_bpf)
		return 0;

	bpf_u_int32 bpfnet = 0;
	if (pcap_compile_nopcap(MAX_PACKET_SIZE, DLT_EN10MB, qctx->bpf, qctx->bpf_query, 0, bpfnet)) {
		return -1;
	}
	return 0;
}

static int
parse_query(query_context_t *qctx, char buf[])
{
	int success = 0;
	char *ptr;
	ptr = strtok(buf, ",:/");
	do {
			printf("%s\n",ptr);
		if (strcmp(ptr, "stime") == 0) {
			ptr = strtok(NULL, ",:/");
			qctx->stime = strtoul(ptr, NULL, 0);
			success++;
		}

		else if (strcmp(ptr, "etime") == 0) {
			ptr = strtok(NULL, ",:/");
			qctx->etime = strtoul(ptr, NULL, 0);
			success++;
		}

		else if (strcmp(ptr, "src_ip") == 0) {
			ptr = strtok(NULL, ",:/");
			if (strtoul(ptr, NULL, 0) < 0) {
				qctx->fsaddr = -1;
				qctx->lsaddr = -1;
			} else {
				qctx->fsaddr = strtoul(ptr, NULL, 0);
				ptr = strtok(NULL, ",:/");
				qctx->lsaddr = strtoul(ptr, NULL, 0);
			}
			success++;
		}

		else if (strcmp(ptr, "dst_ip") == 0) {
			ptr = strtok(NULL, ",:/");
			if (atoi(ptr) < strtoul(ptr, NULL, 0)) {
				qctx->fdaddr = -1;
				qctx->ldaddr = -1;
			} else {
				qctx->fdaddr = strtoul(ptr, NULL, 0);
				ptr = strtok(NULL, ",:/");
				qctx->ldaddr = strtoul(ptr, NULL, 0);
			}
			success++;
		}

		else if (strcmp(ptr, "src_port") == 0) {
			ptr = strtok(NULL, ",:/");
			if (strtoul(ptr, NULL, 0) < 0) {
				qctx->fsport = -1;
				qctx->lsport = -1;
			} else {
				qctx->fsport = strtoul(ptr, NULL, 0);
				ptr = strtok(NULL, ",:/");
				qctx->lsport = strtoul(ptr, NULL, 0);
			}
			success++;
		}

		else if (strcmp(ptr, "dst_port") == 0) {
			ptr = strtok(NULL, ",:/");
			if (atoi(ptr) < 0) {
				qctx->fdport = -1;
				qctx->ldport = -1;
			} else {
				qctx->fdport = strtoul(ptr, NULL, 0);
				ptr = strtok(NULL, ",:/");
				qctx->ldport = strtoul(ptr, NULL, 0);
			}
			success++;
		}

		else if (strcmp(ptr, "bpf") == 0) {
			ptr = strtok(NULL, ",:/");
			qctx->bpf_query = ptr;
			success++;
		}

		else 
			return -1;
	} while ( (ptr = strtok(NULL, ",:/")) != NULL );
	if (success != 7)
		return -1;
	if (atoi(qctx->bpf_query) < 0)
		qctx->no_bpf = true;
	printf("%d %d %d, %d, %d, %d, %d, %d, %d, %d, %s\n", qctx->stime, qctx->etime, qctx->fsaddr, qctx->lsaddr, qctx->fdaddr, qctx->ldaddr, qctx->fsport, qctx->lsport, qctx->fdport, qctx->ldport, qctx->bpf_query);
	return 1;
}
/*----------------------------------------------------*/



/*----------------------------------------------------*/
static void
insert_rand_data(index_array_context_t *ictx, bloom_filter_context_t *bctx, int size)
{
	init_index_array(ictx, bctx, size);

	bool done = false;

	srand(time(NULL));

	FlowMeta *meta = (FlowMeta *)malloc(sizeof(FlowMeta));
	FlowInfo *info = &meta->flowinfo;

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

static void
index_array_exit(index_array_context_t *ictx)
{
	//free_index_array(ctx);	
}
/*----------------------------------------------------*/



/*----------------------------------------------------*/
static void
get_file_info(query_context_t *qctx)
{
	// TODO implement
	// qctx->stime; qctx->etime;
	return;
}

static void
search_with_query(query_context_t *qctx)
{
	int *res;
	get_file_info(qctx);

	res = 
	search_range_from_index_array(qctx->ictx, TYPE_SADDR, qctx->fsaddr, qctx->lsaddr);

	if (res != NULL)
		printf("saddr: %d %d\n", res[0], res[1]);

	res = 
	search_range_from_index_array(qctx->ictx, TYPE_DADDR, qctx->fsaddr, qctx->lsaddr);

	if (res != NULL)
		printf("daddr: %d %d\n", res[0], res[1]);

	res = 
	search_range_from_index_array(qctx->ictx, TYPE_SPORT, qctx->fsaddr, qctx->lsaddr);

	if (res != NULL)
		printf("sport: %d %d\n", res[0], res[1]);

	res = 
	search_range_from_index_array(qctx->ictx, TYPE_DPORT, qctx->fsaddr, qctx->lsaddr);

	if (res != NULL)
		printf("dport: %d %d\n", res[0], res[1]);


	u_char *pkt;
	int len;

	struct bpf_insn *pc = qctx->bpf->bf_insns;
	if (bpf_filter(pc, pkt, len, MAX_PACKET_SIZE) == 0)
		return; // TODO implement
}
/*----------------------------------------------------*/


/*----------------------------------------------------*/
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
		while ((len = read(c, buf, sizeof(buf)-1))) {
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

			search_with_query(qctx);

		}
		close(c);
	}
	perror("Error:accept() failed");
	close(s);


	index_array_exit(ictx);
	return 0;
}
/*----------------------------------------------------*/
