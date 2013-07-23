#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "index.h"

/**
 * main for index array index structure
 * @param ctx [description]
 */
static void
index_array_main(index_array_context_t *ctx, bloom_filter_context_t *bctx, int size)
{
	init_index_array(ctx, bctx, size);

	bool done = false;

	srand(time(NULL));

	FlowMeta *meta = (FlowMeta *)malloc(sizeof(FlowMeta));
	FlowInfo *info = &meta->flowinfo;

	int cnt = 0;
	unsigned int value = 0;
	for (cnt = 0; cnt < 1; cnt++)
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
			if (insert_into_index_array(ctx, meta))
				done = true;
		}
		print_index_array(ctx, TYPE_SADDR);
		search_from_index_array(ctx, TYPE_SADDR, value);

		clean_bloom_filter(bctx);
		clean_index_array(ctx);
	}

	write_index_array(ctx);
}

/**
 * exit for index array index structure
 * @param ctx [description]
 */
static void
index_array_exit(index_array_context_t *ctx)
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
main(int argc, char *argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s [array_size]\n", argv[0]);
		exit(-1);
	}
	index_array_context_t *ctx = (index_array_context_t *)malloc(sizeof(index_array_context_t));
	bloom_filter_context_t *bctx = (bloom_filter_context_t *)malloc(sizeof(bloom_filter_context_t));
	index_array_main(ctx, bctx, atoi(argv[1]));
	index_array_exit(ctx);
	return 0;
}
