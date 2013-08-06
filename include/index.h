#ifndef _INDEX_H_
#define _INDEX_H_

#include <linux/types.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "util.h"
#include "qsort.h"
#define COMPARE_VALUE(a,b) ( (a)->value < (b)->value )

typedef struct bloom_filter_context
{
	unsigned char *saddr;
	unsigned char *daddr;
	unsigned char *sport;
	unsigned char *dport;

	uint16_t fileID;      /* file id of given bloom filter */
} bloom_filter_context_t;

typedef struct index_array_node
{
	uint16_t fileID;
	uint32_t offset;
	unsigned int value;
} index_array_node_t;

typedef struct index_array_context
{
	index_array_node_t *saddr;
	index_array_node_t *daddr;
	index_array_node_t *sport;
	index_array_node_t *dport;
	bloom_filter_context_t *filter;
	int cnt;

	uint16_t fileID;                /* file id of given index array */
} index_array_context_t;

typedef struct index_argument {
	int ia_cpu;
	int ia_idx;
	//char wa_buf[CONFIG_BUF_SIZE];
	//struct engine_context *ic_ectx;
} index_argument_t;

typedef struct index_context {
	int ic_cpu;
	int ic_idx;
	int ic_evfd;
	char *ic_buf;
	//struct engine_context *ic_ectx;
	
	struct bloom_filter_context *ic_bloom_filter_head;
	struct bloom_filter_context *ic_bloom_filter;
	struct index_array_context  *ic_index_array_head;
	struct index_array_context  *ic_index_array;
	int ic_cnt;

	int  ic_socket;
	bool ic_done;
} index_context_t;

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

inline void init_index_context(index_context_t *ictx, bloom_filter_context_t *bfctx, index_array_context_t *iactx);
inline int insert_index(index_array_context_t *ctx, FlowMeta *meta);
inline int* search_index(index_array_context_t *ctx, int type, unsigned int start, unsigned int end);
inline void print_index_array(index_array_context_t *ctx, int type);
inline void write_index_array(index_array_context_t *ctx);
inline void clean_index_array(index_array_context_t *ctx);
inline void free_index_array(index_array_context_t *ctx);
inline void close_file_event(index_context_t *ictx);

#endif