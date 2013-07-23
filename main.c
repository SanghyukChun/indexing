#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "sorted_array.h"
#include "bloom_filter.h"

/**
 * main for sorted array index structure
 * @param ctx [description]
 */
static void
sorted_array_main(sorted_array_context_t *ctx, bloom_filter_context_t *bctx, int size)
{
	init_sorted_array(ctx, size);
	init_bloom_filter(bctx);

	bool done = false;

	srand(time(NULL));

	FlowMeta *meta = (FlowMeta *)malloc(sizeof(FlowMeta));
	FlowInfo *info = &meta->flowinfo;

	int cnt = 0;
	unsigned int value = 0;
	for (cnt = 0; cnt < 5; cnt++)
	{
		done = false;
		while(!done) {
			info->saddr = rand();
			info->daddr = rand();
			info->sport = rand();
			info->dport = rand();

			value = info->saddr;
			//TODO generate rand data iteratively
			/*printf("meta: %u\n", meta->flowinfo.saddr);*/
			insert_into_bloom_filter(bctx, meta);
			if (insert_into_sorted_array(ctx, meta))
				done = true;
		}
		search_from_bloom_filter(bctx, TYPE_SADDR, value);
		search_from_sorted_array(ctx, TYPE_SADDR, value);

		clean_index_array(ctx);
	}

	write_sorted_array(ctx);
}

/**
 * exit for sorted array index structure
 * @param ctx [description]
 */
static void
sorted_array_exit(sorted_array_context_t *ctx)
{
	free_sorted_array(ctx);	
}

/**
 * main function
 * @param  argc [description]
 * @param  argv [description]
 * @return      [description]
 */
int
main(int argc, char *argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s [array_size]\n", argv[0]);
		exit(-1);
	}
	sorted_array_context_t *ctx = (sorted_array_context_t *)malloc(sizeof(sorted_array_context_t));
	bloom_filter_context_t *bctx = (bloom_filter_context_t *)malloc(sizeof(bloom_filter_context_t));
	sorted_array_main(ctx, bctx, atoi(argv[1]));
	sorted_array_exit(ctx);
	return 0;
}
