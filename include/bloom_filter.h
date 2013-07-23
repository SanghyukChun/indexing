#include <linux/types.h>
#include <unistd.h>
#include <stdint.h>

#include "util.h"

typedef struct bloom_filter_context
{
	unsigned char *saddr;
	unsigned char *daddr;
	unsigned char *sport;
	unsigned char *dport;
	uint16_t fileID;
} bloom_filter_context_t;

void init_bloom_filter(bloom_filter_context_t *ctx);
void insert_into_bloom_filter(bloom_filter_context_t *ctx, FlowMeta *meta);
void search_from_bloom_filter(bloom_filter_context_t *ctx, int type, unsigned int data);
void write_bloom_filter(bloom_filter_context_t *ctx);
void clean_bloom_filter(bloom_filter_context_t *ctx);
void free_bloom_filter(bloom_filter_context_t *ctx);