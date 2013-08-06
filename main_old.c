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
index_main(index_context_t *ictx)
{
	memset(ictx, 0, sizeof(index_context_t));
	bloom_filter_context_t bfctx;
	index_array_context_t  iactx;
	init_index_context(ictx, &bfctx, &iactx);

	srand(time(NULL));

	FlowMeta *meta = (FlowMeta *)malloc(sizeof(FlowMeta));
	FlowInfo *info = &meta->flowinfo;

	int cnt, i;
	for (cnt = 0; cnt < 5; cnt++)
	{
		for (i = 0; i < 30000; i++) {
			info->saddr = rand();
			info->daddr = rand();
			info->sport = rand();
			info->dport = rand();

			
			if ((insert_index(ictx->ic_index_array, meta)) < 0)
				perror("insert error");
		}
		close_file_event(ictx);
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
index_exit(index_context_t *ictx)
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
	index_context_t ictx;
	index_main(&ictx);
	index_exit(&ictx);
	return 0;
}
