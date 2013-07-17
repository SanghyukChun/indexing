#include <stdio.h>
#include <stdlib.h>

#include "sorted_array.h"
#include "util.h"

#define ARRAY_SIZE 32768

/**
 * init context for sorted array index
 * @param ctx  [description]
 */
void init_sorted_array(sorted_array_context_t *ctx)
{
	//TODO implement
	LOG_MESSAGE("=== open init sorted array");
	sorted_array_node_t *p = (sorted_array_node_t *)calloc(ARRAY_SIZE, sizeof(sorted_array_node_t));

	if (p == NULL)
	{
		fprintf(stderr, "fail to initialize sorted array nodes\n");
		exit(-1);
	}

	LOG_MESSAGE("=== close init sorted array");
}

/**
 * insert data into sorted array using double linked list
 * @param ctx  [description]
 * @param data [description]
 */
void insert_into_sorted_array(sorted_array_context_t *ctx, unsigned int data)
{
	//TODO implement
}

/**
 * search data from sorted array
 * @param ctx  [description]
 * @param data [description]
 */
void search_from_sorted_array(sorted_array_context_t *ctx, unsigned int data)
{
	//TODO implement
}

/**
 * write sorted array to file system
 * after write operation end, it clean whole array to reuse
 * @param ctx [description]
 */
void write_sorted_array(sorted_array_context_t *ctx)
{
	//TODO implement
}

/**
 * free sorted array
 * @param ctx [description]
 */
void free_sorted_array(sorted_array_context_t *ctx)
{
	//TODO implement
}
