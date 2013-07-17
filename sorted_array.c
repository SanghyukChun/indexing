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
		fprintf(stderr, "fail to initialize array\n");
		exit(-1);
	}

	ctx->head = p;
	ctx->last_idx = 0;

	LOG_MESSAGE("=== close init sorted array");
}

/**
 * insert data into sorted array using double linked list
 * @param ctx  [description]
 * @param data [description]
 */
void insert_into_sorted_array(sorted_array_context_t *ctx, unsigned int data)
{
	sorted_array_node_t *node = &ctx->head[ctx->last_idx];
	node->value = data;
	//node->file_offset = offset;
	ctx->last_idx = ctx->last_idx+1;
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

int partition(sorted_array_node_t *head, int l, int r)
{
	int pivot, i, j, t;
	pivot = (&head[l])->value;
	i = l; j = r+1;

	while(1)
	{
		do ++i; while( (&head[i])->value <= pivot && i <= r );
		do --j; while( (&head[j])->value > pivot );
		if (i >= j) break;
		t = (&head[i])->value; (&head[i])->value = (&head[j])->value; (&head[j])->value = t;
	}
	t = (&head[l])->value; (&head[l])->value = (&head[j])->value; (&head[j])->value = t;
	return j;
}

void quick_sort(sorted_array_node_t *head, int l, int r)
{
	int j;
	if (l < r)
	{
		j = partition(head, l, r);
		quick_sort(head, l, j-1);
		quick_sort(head, j+1, r);
	}
}

void sort_array(sorted_array_context_t *ctx)
{
	sorted_array_node_t *head = ctx->head;
	quick_sort(head, 0, 1000);
}

void print_sorted_array(sorted_array_context_t *ctx)
{
	sorted_array_node_t *head = ctx->head;
	int i;
	for (i = 0; i <ARRAY_SIZE; i++)
	{
		sorted_array_node_t *node = &head[i];
		if (node->value != 0)
			printf("%d\n", node->value);
	}
}

/**
 * write sorted array to file system
 * after write operation end, it clean whole array to reuse
 * @param ctx [description]
 */
void write_sorted_array(sorted_array_context_t *ctx)
{
	//TODO implement
	sort_array(ctx);
	print_sorted_array(ctx);
}

/**
 * free sorted array
 * @param ctx [description]
 */
void free_sorted_array(sorted_array_context_t *ctx)
{
	//TODO implement
}
