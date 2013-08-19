#ifndef _INDEX_H_
#define _INDEX_H_

#include <linux/types.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "util.h"
#include "qsort.h"
#define COMPARE_VALUE(a,b) ( (a)->value < (b)->value )

typedef struct bloom_filter
{
	unsigned char *saddr;  /* current saddr */
	unsigned char *daddr;  /* current saddr */
	unsigned char *sport;  /* current saddr */
	unsigned char *dport;  /* current saddr */

	uint16_t fileID;       /* file id of given bloom filter */
} bloom_filter_t;

typedef struct index_argument {
	int ia_cpu;
	int ia_idx;
	//char wa_buf[CONFIG_BUF_SIZE];
	//struct engine_context *ic_ectx;
} index_argument_t;

typedef struct array_node
{
	uint16_t fileID;
	uint32_t offset;
	unsigned int value;
} array_node_t;

typedef struct index_array
{
	array_node_t *saddr;
	array_node_t *daddr;
	array_node_t *sport;
	array_node_t *dport;
	bloom_filter_t *filter;
	int cnt; /* number of current nodes in array (size of array in current state) */
} index_array_t;

typedef struct indexer_context {
	int ic_cpu;
	int ic_idx;
	int ic_evfd;
	char *ic_buf;
	//struct engine_context *ic_ectx;
	int ic_epollfd;

	struct index_array *ic_index;
	int  ic_remain_node; /* number of remain node */
	int  ic_array_idx; /* index of current index_array */
	bool ic_done;
} indexer_context_t;

enum {
	TYPE_SADDR = 1,
	TYPE_DADDR = 2,
	TYPE_SPORT = 4,
	TYPE_DPORT = 8
};

enum {
	SEARCH_EXACT,
	SEARCH_MIN,
	SEARCH_MAX
};

/*
inline void init_index_array(index_array_t *ctx, bloom_filter_t *bctx, int size);
inline int insert_into_index_array(index_array_t *ctx, FlowMeta *meta);
inline int* search_from_index_array(index_array_t *ctx, int type, unsigned int data);
inline int* search_range_from_index_array(index_array_t *ctx, int type, unsigned int start, unsigned int end);
inline void print_index_array(index_array_t *ctx, int type);
inline void write_index_array(index_array_t *ctx);
inline void clean_index_array(index_array_t *ctx);
inline void free_index_array(index_array_t *ctx);
inline void close_file_event(index_array_t *ctx);
*/
#endif
