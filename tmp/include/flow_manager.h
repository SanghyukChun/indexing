#ifndef _UTILH_
#define _UTILH_

#define PRINT_TIME
#define LOG
#ifdef LOG
#define LOG_MESSAGE(msg) printf("%s\n", msg)
#else
#define LOG_MESSAGE(msg)
#endif

#include <linux/types.h>
#include <unistd.h>
#include <stdint.h>

/*****************************************************************************/
/* Configurations */

#define NUM_CPU 16
#define NUM_ENG 24
#define NUM_XGE 4
#define NUM_WRITER_PER_ENG 2
#define PS_CHUNK_SIZE 128
#define PRINT_TIMER 1
#define MAX_WBUF_NUM 100
#define MAX_WBUF_SIZE (512 * 1024)
#define MAX_FCAP_SIZE (10 * 1024 * 1024 * 1024L)
#define MAX_META_SIZE (4 * 1024 * 1024 * 1024L)
/* META_ARRAY_SIZE should be 2^n */
#define META_ARRAY_SIZE (1024 * 1024L)
//#define MAX_FCAP_NUM 2746
#define MAX_FCAP_NUM 274




typedef struct FlowInfo {
	__be32 saddr; /* source IP address. */
	__be32 daddr; /* destination IP address. */
	__be16 sport; /* source port number. */
	__be16 dport; /* destination port number. */
	u_char protocol; /* transport layer protocol. */
	struct timeval start; /* first packet arriving time. */
	struct timeval end; /* last packet arribing time. */
} FlowInfo;

typedef struct FlowMeta {
    FlowInfo flowinfo;
    uint16_t fileID;
    uint32_t offset;
} FlowMeta;

/*****************************************************************************/
typedef struct indexer_context {
	int ic_cpu;
	struct engine_context *ic_ectx;
	int ic_sockd;

	struct array_node_t *ic_saddr_array;
	struct array_node_t *ic_daddr_array;
	struct array_node_t *ic_sport_array;
	struct array_node_t *ic_dport_array;
	struct array_node_t *ic_saddr_array;
	bloom_filter_t *ic_bloom_filter;
	index_info_t *ic_index_info;
	int ic_cnt;

	bool ic_done;
	FlowMeta *ic_meta;
} indexer_context_t;
/*****************************************************************************/
typedef struct engine_context {
} engine_context_t;


#endif
