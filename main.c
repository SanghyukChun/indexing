#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "sorted_array.h"
#include "avl_tree.h"
#include "bloom_filter.h"

/**
 * main for sorted array index structure
 * @param ctx [description]
 */
void
sorted_array_main(sorted_array_context_t *ctx)
{
	init_sorted_array(ctx);

	bool done = false;
	unsigned int data = 0;

	srand(time(NULL));

	while(!done) {
		//TODO generate rand data iteratively
		insert_into_sorted_array(ctx, rand());
		if (data++ > 100)
			done = true;
	}

	write_sorted_array(ctx);
}

/**
 * exit for sorted array index structure
 * @param ctx [description]
 */
void
sorted_array_exit(sorted_array_context_t *ctx)
{
	free_sorted_array(ctx);	
}

/**
 * main for bloom filter index structure
 * @param ctx [description]
 */
void
bloom_filter_main(bloom_filter_context_t *ctx)
{
	init_bloom_filter(ctx);

	bool done = false;
	unsigned int data;

	while(!done) {
		//TODO generate rand data iteratively
		insert_into_bloom_filter(ctx, data);
		done = true;
	}

	write_bloom_filter(ctx);
}

/**
 * exit for bloom filter index structure
 * @param ctx [description]
 */
void
bloom_filter_exit(bloom_filter_context_t *ctx)
{
	free_bloom_filter(ctx);
}

/**
 * main for avl tree index structure
 * @param ctx [description]
 */
void
avl_tree_main(avl_tree_context_t *ctx)
{
	init_avl_tree(ctx);

	bool done = false;
	unsigned int data = 0;

	while(!done) {
		//TODO generate rand data iteratively
		insert_into_avl_tree(ctx, data++);
		if (data > 100)
			done = true;
	}

	write_avl_tree(ctx);
}

/**
 * exit for avl tree index structure
 * @param ctx [description]
 */
void
avl_tree_exit(avl_tree_context_t *ctx)
{
	free_avl_tree(ctx);
}

void
usage(char* argv0)
{
		fprintf(stderr, "usage: %s <index structure>\n", argv0);
		fprintf(stderr, "           index structure options\n");
		fprintf(stderr, "           a: index as sorted array\n");
		fprintf(stderr, "           b: index as bloom filter\n");
		fprintf(stderr, "           t: index as avl tree\n");
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
	if (argc == 1)
	{
		sorted_array_context_t *ctx = (sorted_array_context_t *)malloc(sizeof(sorted_array_context_t));
		sorted_array_main(ctx);
		sorted_array_exit(ctx);
		return;
	}

	if (argc != 2)
	{
		usage(argv[0]);
		exit(0);
	}

	if (strcmp(argv[1], "a") == 0)
	{
		sorted_array_context_t *ctx = (sorted_array_context_t *)malloc(sizeof(sorted_array_context_t));
		sorted_array_main(ctx);
		sorted_array_exit(ctx);
	} else if (strcmp(argv[1], "b") == 0) {
		bloom_filter_context_t *ctx = (bloom_filter_context_t *)malloc(sizeof(bloom_filter_context_t));
		bloom_filter_main(ctx);
		bloom_filter_exit(ctx);
	} else if (strcmp(argv[1], "t") == 0) {
		avl_tree_context_t *ctx = (avl_tree_context_t *)malloc(sizeof(avl_tree_context_t));
		avl_tree_main(ctx);
		avl_tree_exit(ctx);
	} else {
		fprintf(stderr, "undefined index structure %s\n", argv[1]);
		usage(argv[0]);
	}
	return 0;
}
