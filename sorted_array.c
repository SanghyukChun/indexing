#include <stdio.h>
#include <stdlib.h>

#include "sorted_array.h"

#define ARRAY_SIZE 32768

static inline void sort_array(sorted_array_context_t *ctx);
static inline int binary_search(sorted_array_node_t *head, int start, int end, unsigned int data);

/**
 * allocate memory to given head node
 * @param head node structure which indicate first of index array
 */
static inline void
init_array(sorted_array_node_t **head)
{
	sorted_array_node_t *p = (sorted_array_node_t *)calloc(ARRAY_SIZE, sizeof(sorted_array_node_t));

	if (p == NULL)
	{
		fprintf(stderr, "fail to initialize array\n");
		exit(-1);
	}

	*head = p;
}

/**
 * init context for sorted array index
 * init array for each index field (source ip/port, destination ip/port)
 * @param ctx  context
 */
inline void
init_sorted_array(sorted_array_context_t *ctx)
{
	LOG_MESSAGE("=== open init sorted array");

	init_array(&ctx->saddr);
	init_array(&ctx->daddr);
	init_array(&ctx->sport);
	init_array(&ctx->dport);

	ctx->last_idx = 0;

	LOG_MESSAGE("=== close init sorted array");
}

/**
 * insert data to given array structure
 * assign values to unused node
 * data and file information (fileID and offset) of metadata is assigned to node
 * @param ctx  [description]
 * @param head [description]
 * @param data [description]
 * @param meta [description]
 */
static inline void
insert_index(sorted_array_context_t *ctx, sorted_array_node_t *head, unsigned int data, FlowMeta *meta)
{
	sorted_array_node_t *node = &head[ctx->last_idx];
	node->value = data;
	node->fileID = meta->fileID;
	node->offset = meta->offset;
}

/**
 * insert metadata into sorted array using double linked list
 * @param ctx  [description]
 * @param meta [description]
 * @return     [description]
 */
inline int
insert_into_sorted_array(sorted_array_context_t *ctx, FlowMeta *meta)
{
	insert_index(ctx, ctx->saddr, meta->flowinfo.saddr, meta);
	insert_index(ctx, ctx->daddr, meta->flowinfo.daddr, meta);
	insert_index(ctx, ctx->sport, meta->flowinfo.sport, meta);
	insert_index(ctx, ctx->dport, meta->flowinfo.dport, meta);

	ctx->last_idx = ctx->last_idx+1;

	if(ctx->last_idx >= ARRAY_SIZE)
	{
		sort_array(ctx);
		write_sorted_array(ctx);
		clean_index_array(ctx);
		return 0;
	}

	return 0;
}

/**
 * binary search
 * @param  head  [description]
 * @param  start [description]
 * @param  end   [description]
 * @param  data  [description]
 * @return       [description]
 */
static inline int
binary_search(sorted_array_node_t *head, int start, int end, unsigned int data)
{
	if (start > end)
		return -1;

	int mid = (start + end) / 2;
	unsigned int mid_val = (&head[mid])->value;

	if (mid_val > data) {
		return binary_search(head, start, mid-1, data);
	} else if (mid_val < data) {
		return binary_search(head, mid+1, end, data);
	}

	return mid;
}

/**
 * search data from sorted array
 * @param ctx  [description]
 * @param data [description]
 */
inline void
search_from_sorted_array(sorted_array_context_t *ctx, unsigned int data)
{
	//TODO change function to use ENUM field marker
	int res = binary_search(ctx->saddr, 0, ARRAY_SIZE-1, data);
	
	if (res == -1) {
		printf("%u do not exist in the array\n", data);
		return;		
	}

	printf("%u is exist in array idx: %d\n", data, res);
}

/**
 * swap node a and b
 * @param a [description]
 * @param b [description]
 */
static inline void
swap(sorted_array_node_t *a, sorted_array_node_t *b)
{
	sorted_array_node_t temp;
	temp = *a; *a = *b;	*b = temp;
}

/**
 * sort parition
 * @param  head [description]
 * @param  l    [description]
 * @param  r    [description]
 * @return      [description]
 */
static inline int
partition(sorted_array_node_t *head, int l, int r)
{
	unsigned int pivot, i, j;
	pivot = (&head[l])->value;
	i = l; j = r+1;

	while(1)
	{
		do ++i; while( (&head[i])->value <= pivot && i <= r );
		do --j; while( (&head[j])->value > pivot );
		if (i >= j) break;
		swap(&head[i], &head[j]);
	}
	swap(&head[l], &head[j]);
	return j;
}

/**
 * do quick sort
 * @param head [description]
 * @param l    [description]
 * @param r    [description]
 */
static inline void
quick_sort(sorted_array_node_t *head, int l, int r)
{
	int j;
	if (l < r)
	{
		j = partition(head, l, r);
		quick_sort(head, l, j-1);
		quick_sort(head, j+1, r);
	}
}

/**
 * sort array by quick sort function
 * @param ctx [description]
 */
static inline void
sort_array(sorted_array_context_t *ctx)
{
	LOG_MESSAGE("=== start sorting");
	quick_sort(ctx->saddr, 0, ctx->last_idx-1);
	quick_sort(ctx->daddr, 0, ctx->last_idx-1);
	quick_sort(ctx->sport, 0, ctx->last_idx-1);
	quick_sort(ctx->dport, 0, ctx->last_idx-1);
	LOG_MESSAGE("=== close sorting");
}

/**
 * print array
 * @param ctx [description]
 */
inline void
print_sorted_array(sorted_array_context_t *ctx)
{
	/**
	sorted_array_node_t *head = ctx->head;
	int i;
	for (i = 0; i <ARRAY_SIZE; i++)
	{
		sorted_array_node_t *node = &head[i];
		if (node->value != 0)
			printf("%d\n", node->value);
	}
	**/
}

/**
 * write sorted array to file system
 * after write operation end, it clean whole array to reuse
 * @param ctx [description]
 */
inline void
write_sorted_array(sorted_array_context_t *ctx)
{
	//TODO implement
}

/**
 * clean given array
 * @param head [description]
 */
inline static void 
clean_array(sorted_array_node_t *head)
{
	int i;
	sorted_array_node_t *node;
	for (i = 0; i < ARRAY_SIZE; i++)
	{
		node = &head[i];
		node->fileID = 0;
		node->offset = 0;
		node->value = 0;
	}
}

/**
 * clean whole arrayes to reuse
 * @param ctx context
 */
inline static void 
clean_index_array(sorted_array_context_t *ctx)
{
	LOG_MESSAGE("=== start clean");
	clean_array(ctx->saddr);
	clean_array(ctx->daddr);
	clean_array(ctx->sport);
	clean_array(ctx->dport);
	LOG_MESSAGE("=== start clean");
}

/**
 * free sorted array
 * @param ctx [description]
 */
inline void
free_sorted_array(sorted_array_context_t *ctx)
{
	LOG_MESSAGE("=== start free");
	free(ctx->saddr);
	free(ctx->daddr);
	free(ctx->sport);
	free(ctx->dport);
	free(ctx);
	LOG_MESSAGE("=== start free");
}