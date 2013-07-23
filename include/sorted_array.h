#include <linux/types.h>
#include <unistd.h>
#include <stdint.h>

#include "util.h"


typedef struct sorted_array_node
{
	uint16_t fileID;
	uint32_t offset;
	unsigned int value;
} sorted_array_node_t;

typedef struct sorted_array_context
{
	sorted_array_node_t *saddr;
	sorted_array_node_t *daddr;
	sorted_array_node_t *sport;
	sorted_array_node_t *dport;
	int last_idx;
} sorted_array_context_t;

inline void init_sorted_array(sorted_array_context_t *ctx, int size);
inline int insert_into_sorted_array(sorted_array_context_t *ctx, FlowMeta *meta);
inline void search_from_sorted_array(sorted_array_context_t *ctx, unsigned int data);
inline void write_sorted_array(sorted_array_context_t *ctx);
inline void free_sorted_array(sorted_array_context_t *ctx);
