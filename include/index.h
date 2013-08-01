#include <linux/types.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "util.h"
#include "bloom_filter.h"
#include "qsort.h"
#define COMPARE_VALUE(a,b) ( (a)->value < (b)->value )

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
	int ic_epollfd;
	bool ic_done;
} index_context_t;

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
	bloom_filter_context_t *bctx;
	int last_idx;
} index_array_context_t;

inline void init_index_array(index_array_context_t *ctx, bloom_filter_context_t *bctx, int size);
inline int insert_into_index_array(index_array_context_t *ctx, FlowMeta *meta);
inline int* search_from_index_array(index_array_context_t *ctx, int type, unsigned int data);
inline int* search_range_from_index_array(index_array_context_t *ctx, int type, unsigned int start, unsigned int end);
inline void print_index_array(index_array_context_t *ctx, int type);
inline void write_index_array(index_array_context_t *ctx);
inline void clean_index_array(index_array_context_t *ctx);
inline void free_index_array(index_array_context_t *ctx);
