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

	srand(time(NULL));

	FlowMeta *meta = (FlowMeta *)malloc(sizeof(FlowMeta));
	FlowInfo *info = &meta->flowinfo;

	int cnt, i;
	unsigned int value = 0;
	for (cnt = 0; cnt < 5; cnt++)
	{
		for (i = 0; i < 30000; i++) {
			info->saddr = rand();
			info->daddr = rand();
			info->sport = rand();
			info->dport = rand();

			value = info->saddr;
			if ((insert_into_index_array(ctx, meta)) < 0)
				perror("insert error");
		}
		close_file_event(ctx);
		/*print_index_array(ctx, TYPE_SADDR);*/
		/*int *search_result = search_from_index_array(ctx, TYPE_SADDR, value);*/
		/*
		int *search_result = search_range_from_index_array(ctx, TYPE_SADDR, 10000000, 1000000000);
		if (search_result != NULL)
			printf("s: %d e: %d\n", search_result[0], search_result[1]);
		*/
	}

	//write_index_array(ctx);
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
	index_array_context_t *ctx = (index_array_context_t *)malloc(sizeof(index_array_context_t));
	bloom_filter_context_t *bctx = (bloom_filter_context_t *)malloc(sizeof(bloom_filter_context_t));
	index_array_main(ctx, bctx, 0);
	index_array_exit(ctx);
	return 0;
}
