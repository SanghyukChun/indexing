#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "index.h"

#define PRINT_TIME
/*#define ARRAY_SIZE 32768*/

int ARRAY_SIZE;
static inline void sort_array(index_array_context_t *ctx);
static inline int binary_search(index_array_node_t *head, int start, int end, unsigned int data);

/**
 * allocate memory to given head node
 * @param head node structure which indicate first of index array
 */
static inline int
init_array(index_array_node_t **head)
{
	index_array_node_t *p = (index_array_node_t *)calloc(ARRAY_SIZE, sizeof(index_array_node_t));

	if (p == NULL)
		return -1;

	*head = p;
	return 0;
}

/**
 * init context for index array index
 * init array for each index field (source ip/port, destination ip/port)
 * @param ctx  context
 */
inline void
init_index_array(index_array_context_t *ctx, bloom_filter_context_t *bctx, int size)
{
	ARRAY_SIZE = size;
	LOG_MESSAGE("=== open init index array");

	// XXX if you want to initialize only specific type, you can use TYPE.
	// type = TYPE_SADDR | TYPE_DADDR | .....
	// ex) if (type | TYPE_SADDR) {init_array(&ctx->saddr);}
	
	if (init_array(&ctx->saddr) == -1)
		goto fail;
	if (init_array(&ctx->daddr) == -1)
		goto fail;
	if (init_array(&ctx->sport) == -1)
		goto fail;
	if (init_array(&ctx->dport) == -1)
		goto fail;

	init_bloom_filter(bctx);
	ctx->bctx = bctx;
	ctx->last_idx = 0;

	LOG_MESSAGE("=== close init index array");

	fail:
		fprintf(stderr, "fail to initialize array\n");
		exit(-1);
	
}

/**
 * insert data to given array structure
 * assign values to unused node
 * data and file information (fileID and offset) of metadata is assigned to node
 * @param ctx  context
 * @param head node structure which indicate first of index array
 * @param data index data such as ip and port
 * @param meta required to assign fileID and offset to node
 */
static inline void
insert_index(index_array_context_t *ctx, index_array_node_t *head, unsigned int data, FlowMeta *meta)
{
	index_array_node_t *node = &head[ctx->last_idx];
	node->value  = data;
	node->fileID = meta->fileID;
	node->offset = meta->offset;
}

/**
 * insert metadata into index array using double linked list
 * @param ctx  context
 * @param meta flow metadata to indexing
 * @return     it always return 0 except error occured
 */
inline int
insert_into_index_array(index_array_context_t *ctx, FlowMeta *meta)
{
	insert_index(ctx, ctx->saddr, meta->flowinfo.saddr, meta);
	insert_index(ctx, ctx->daddr, meta->flowinfo.daddr, meta);
	insert_index(ctx, ctx->sport, meta->flowinfo.sport, meta);
	insert_index(ctx, ctx->dport, meta->flowinfo.dport, meta);
	insert_into_bloom_filter(ctx->bctx, meta);

	ctx->last_idx = ctx->last_idx+1;

	if(ctx->last_idx >= ARRAY_SIZE)
	{
		sort_array(ctx);
		write_index_array(ctx);
		return 1;
	}

	return 0;
}

/**
 * simple binary search which always compare middle of given start and end index to given data
 * @param  head  node structure which indicate first of index array
 * @param  start first index to look up
 * @param  end   last index to look up
 * @param  data  data to find
 * @return       return 0 if there is no given data
 *               if there is, it return index of given data
 */
static inline int
binary_search(index_array_node_t *head, int start, int end, unsigned int data)
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

static inline int
search_backward(index_array_node_t *head, int idx, unsigned int data) {
	if (idx == -1)
		return -1;

	while ((&head[idx])->value == data || idx > -1)
		idx --;

	return idx + 1;
}

static inline int
search_forward(index_array_node_t *head, int idx, unsigned int data) {
	if (idx == -1)
		return -1;

	while ((&head[idx])->value == data || idx < ARRAY_SIZE)
		idx ++;

	return idx - 1;
}

/**
 * search data from array
 * @param ctx  context
 * @param data data to find
 */
inline int *
search_from_index_array(index_array_context_t *ctx, int type, unsigned int data)
{
	int ret[2];
	int *ptr;

	int idx = -1;
	int start = 0;
	int end = ARRAY_SIZE-1;

	switch (type) {
		case TYPE_SADDR:
			if (search_from_bloom_filter(ctx->bctx, TYPE_SADDR, data)) {
				idx   = binary_search  (ctx->saddr, 0, ARRAY_SIZE-1, data);
				start = search_backward(ctx->saddr, idx, data);	
				end   = search_forward (ctx->saddr, idx, data);
			}
			break;
		case TYPE_DADDR:
			if (search_from_bloom_filter(ctx->bctx, TYPE_DADDR, data)) {
				idx   = binary_search  (ctx->daddr, 0, ARRAY_SIZE-1, data);
				start = search_backward(ctx->daddr, idx, data);
				end   = search_forward (ctx->daddr, idx, data);
			}
			break;
		case TYPE_SPORT:
			if (search_from_bloom_filter(ctx->bctx, TYPE_SPORT, data)){
				idx   = binary_search  (ctx->sport, 0, ARRAY_SIZE-1, data);
				start = search_backward(ctx->sport, idx, data);
				end   = search_forward (ctx->sport, idx, data);
			}
			break;
		case TYPE_DPORT:
			if (search_from_bloom_filter(ctx->bctx, TYPE_DPORT, data)){
				idx   = binary_search  (ctx->dport, 0, ARRAY_SIZE-1, data);
				start = search_backward(ctx->dport, idx, data);
				end   = search_forward (ctx->dport, idx, data);
			}
			break;
		default:
			printf("Unknown type\n");
			return NULL;
	}

	if (idx == -1)
		return NULL;	

	ret[0] = start;
	ret[1] = end;

	ptr = ret;

	return ptr;
}

inline int *
search_range_from_index_array(index_array_context_t *ctx, int type, unsigned int start, unsigned int end)
{
	int s_idx, s_start, e_idx, e_end;
	int ret[2];
	int *ptr;
	
	s_idx = -1;
	e_idx = -1;

	s_start = 0;
	e_end = ARRAY_SIZE;

	switch (type) {
		case TYPE_SADDR:
			s_idx   = binary_search  (ctx->saddr, 0, ARRAY_SIZE-1, start);
			s_start = search_backward(ctx->saddr, s_idx, start);	

			e_idx   = binary_search  (ctx->saddr, 0, ARRAY_SIZE-1, end);
			e_end   = search_forward (ctx->saddr, e_idx, end);
			break;
		case TYPE_DADDR:
			s_idx   = binary_search  (ctx->daddr, 0, ARRAY_SIZE-1, start);
			s_start = search_backward(ctx->daddr, s_idx, start);

			e_idx   = binary_search  (ctx->daddr, 0, ARRAY_SIZE-1, end);
			e_end   = search_forward (ctx->daddr, e_idx, end);
			break;
		case TYPE_SPORT:
			s_idx   = binary_search  (ctx->sport, 0, ARRAY_SIZE-1, start);
			s_start = search_backward(ctx->sport, s_idx, start);

			e_idx   = binary_search  (ctx->sport, 0, ARRAY_SIZE-1, start);
			e_end   = search_forward (ctx->sport, e_idx, end);
			break;
		case TYPE_DPORT:
			s_idx   = binary_search  (ctx->dport, 0, ARRAY_SIZE-1, start);
			s_start = search_backward(ctx->dport, s_idx, start);

			e_idx   = binary_search  (ctx->dport, 0, ARRAY_SIZE-1, start);
			e_end   = search_forward (ctx->dport, e_idx, end);
			break;
		default:
			printf("Unknown type\n");
			return NULL;
	}

	if (s_idx == -1 || e_idx == -1)
		return NULL;

	ret[0] = s_start;
	ret[1] = e_end;

	ptr = ret;

	return ptr;
}

/**
 * swap node a and b
 * @param a [description]
 * @param b [description]
 */
static inline void
swap(index_array_node_t *a, index_array_node_t *b)
{
	index_array_node_t temp;
	temp = *a; *a = *b;	*b = temp;
}

/**
 * parition sorting algorithm for quick sort
 * @param  head node structure which indicate first of index array
 * @param  l    [description]
 * @param  r    [description]
 * @return      [description]
 */
static inline int
partition(index_array_node_t *head, int l, int r)
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
 * @param head node structure which indicate first of index array
 * @param l    [description]
 * @param r    [description]
 */
static inline void
quick_sort(index_array_node_t *head, int l, int r)
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
 * @param ctx context
 */
static inline void
sort_array(index_array_context_t *ctx)
{
	LOG_MESSAGE("=== start sorting");
	#ifdef PRINT_TIME
	struct timeval t1,t2;
	gettimeofday(&t1, NULL);
	#endif

	quick_sort(ctx->saddr, 0, ctx->last_idx-1);
	quick_sort(ctx->daddr, 0, ctx->last_idx-1);
	quick_sort(ctx->sport, 0, ctx->last_idx-1);
	quick_sort(ctx->dport, 0, ctx->last_idx-1);

	#ifdef PRINT_TIME
	gettimeofday(&t2, NULL);
	double elapsed = (double)(t2.tv_sec - t1.tv_sec) + (double)(t2.tv_usec - t1.tv_usec) * 1e-6;
	printf("elapsed: %f\n", elapsed);
	#endif
	LOG_MESSAGE("=== close sorting");
}

static inline void
print_array(index_array_node_t *head)
{
	int i;
	for (i = 0; i <ARRAY_SIZE; i++)
	{
		index_array_node_t *node = &head[i];
		printf("%u\n", node->value);
	}
}

/**
 * print array
 * @param ctx context
 */
inline void
print_index_array(index_array_context_t *ctx, int type)
{
	if (type | TYPE_SADDR)
		print_array(ctx->saddr);
	else if (type | TYPE_DADDR)
		print_array(ctx->daddr);
	else if (type | TYPE_SPORT)
		print_array(ctx->sport);
	else if (type | TYPE_DPORT)
		print_array(ctx->dport);
}

/**
 * write index array to file system
 * after write operation end, it clean whole array to reuse
 * @param ctx [description]
 */
inline void
write_index_array(index_array_context_t *ctx)
{
	//TODO implement
}

/**
 * clean given array to reuse
 * @param head node structure which indicate first of index array
 */
inline static void 
clean_array(index_array_node_t *head)
{
	int i;
	index_array_node_t *node;
	for (i = 0; i < ARRAY_SIZE; i++)
	{
		node = &head[i];
		node->fileID = 0;
		node->offset = 0;
		node->value = 0;
	}
}

/**
 * clean whole arrays to reuse
 * @param ctx context
 */
inline void 
clean_index_array(index_array_context_t *ctx)
{
	LOG_MESSAGE("=== start clean");

	clean_array(ctx->saddr);
	clean_array(ctx->daddr);
	clean_array(ctx->sport);
	clean_array(ctx->dport);

	clean_bloom_filter(ctx->bctx);

	ctx->last_idx = 0;

	LOG_MESSAGE("=== start clean");
}

/**
 * free index array
 * @param ctx context
 */
inline void
free_index_array(index_array_context_t *ctx)
{
	LOG_MESSAGE("=== start free");

	free(ctx->saddr);
	free(ctx->daddr);
	free(ctx->sport);
	free(ctx->dport);
	free(ctx);

	LOG_MESSAGE("=== start free");
}
