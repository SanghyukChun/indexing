#include <linux/types.h>
#include <unistd.h>
#include <stdint.h>

#include "util.h"


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
	int last_idx;
} index_array_context_t;

inline void init_index_array(index_array_context_t *ctx, int size);
inline int insert_into_index_array(index_array_context_t *ctx, FlowMeta *meta);
inline void search_from_index_array(index_array_context_t *ctx, int type, unsigned int data);
inline void print_index_array(index_array_context_t *ctx, int type);
inline void write_index_array(index_array_context_t *ctx);
inline void clean_index_array(index_array_context_t *ctx);
inline void free_index_array(index_array_context_t *ctx);
