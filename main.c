#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "sorted_array.h"

/**
 * main for sorted array index structure
 * @param ctx [description]
 */
static void
sorted_array_main(sorted_array_context_t *ctx, int size)
{
	init_sorted_array(ctx, size);

	bool done = false;

	srand(time(NULL));

	FlowMeta *meta = (FlowMeta *)malloc(sizeof(FlowMeta));
	FlowInfo *info = &meta->flowinfo;

	int cnt = 0;
	for (cnt = 0; cnt < 5; cnt++)
	{
		done = false;
		while(!done) {
			info->saddr = rand();
			info->daddr = rand();
			info->sport = rand();
			info->dport = rand();
			//TODO generate rand data iteratively
			/*printf("meta: %u\n", meta->flowinfo.saddr);*/
			if (insert_into_sorted_array(ctx, meta))
				done = true;
		}
	}

	write_sorted_array(ctx);
	search_from_sorted_array(ctx, rand());
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
  sorted_array_context_t *ctx = (sorted_array_context_t *)malloc(sizeof(sorted_array_context_t));
  sorted_array_main(ctx, atoi(argv[1]));
  sorted_array_exit(ctx);
  return 0;
}
