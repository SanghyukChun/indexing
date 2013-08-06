#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "index.h"
#include "hashes.h"

// TODO rename as FILE_NUM_PER_INDEX_THREAD
#define FILE_NUM_PER_SSD 10
// TODO remame ARRAY_SIZE
#define ARRAY_SIZE 32768
#define FILTER_SIZE 20
#define NUM_HASHES 7
#define FILTER_SIZE_BYTES (1 << (FILTER_SIZE - 3))
#define FILTER_BITMASK ((1 << FILTER_SIZE) - 1)
#define WORD_BUF_SIZE 32
#define NUM_HASHES 7

//int ARRAY_SIZE;
static inline void sort_array(index_array_context_t *ctx);
static inline int binary_search(index_array_node_t *head, int start, int end, unsigned int data, int type);


/*****************************************************************************/
static void
get_hashes(unsigned int hash[], unsigned char *data)
{
	hash[0] = RSHash  (data, 4);
	hash[1] = DJBHash (data, 4);
	hash[2] = FNVHash (data, 4);
	hash[3] = JSHash  (data, 4);
	hash[4] = PJWHash (data, 4);
	hash[5] = SDBMHash(data, 4);
	hash[6] = DEKHash (data, 4);
}
/*****************************************************************************/
static void
convert_into_char(unsigned char *char_data, unsigned int data)
{
	char_data[0] = (data >> 24) & 0xFF;
	char_data[1] = (data >> 16) & 0xFF;
	char_data[2] = (data >> 8) & 0xFF;
	char_data[3] = (data) & 0xFF;
}
/*****************************************************************************/
static inline void
sort_array(index_array_context_t *ctx)
{
	LOG_MESSAGE("=== start sorting");
	#ifdef PRINT_TIME
	// PRINT_TIME is in util.h
	struct timeval t1,t2;
	gettimeofday(&t1, NULL);
	#endif

	QSORT(struct index_array_node, ctx->saddr, ctx->cnt+1, COMPARE_VALUE);
	QSORT(struct index_array_node, ctx->daddr, ctx->cnt+1, COMPARE_VALUE);
	QSORT(struct index_array_node, ctx->sport, ctx->cnt+1, COMPARE_VALUE);
	QSORT(struct index_array_node, ctx->dport, ctx->cnt+1, COMPARE_VALUE);

	#ifdef PRINT_TIME
	gettimeofday(&t2, NULL);
	double elapsed = (double)(t2.tv_sec - t1.tv_sec) + (double)(t2.tv_usec - t1.tv_usec) * 1e-6;
	printf("elapsed: %f\n", elapsed);
	#endif
	LOG_MESSAGE("=== close sorting");
}
/*****************************************************************************/




/*****************************************************************************/
inline int
init_filter(unsigned char **head)
{
	unsigned char *p = (unsigned char *)calloc(FILTER_SIZE_BYTES * FILE_NUM_PER_SSD, sizeof(unsigned char));

	if (p == NULL)
		return -1;

	*head = p;
	return 0;
}
/*****************************************************************************/
inline int
init_array(index_array_node_t **head)
{
	index_array_node_t *p = (index_array_node_t *)calloc(ARRAY_SIZE * FILE_NUM_PER_SSD, sizeof(index_array_node_t));

	if (p == NULL)
		return -1;

	*head = p;
	return 0;
}
/*****************************************************************************/
inline void
init_index_context(index_context_t *ictx, bloom_filter_context_t *bfctx, index_array_context_t *iactx)
{
	LOG_MESSAGE("=== start init");
	bfctx = (bloom_filter_context_t *)calloc(FILE_NUM_PER_SSD, sizeof(bloom_filter_context_t));
	iactx = (index_array_context_t  *)calloc(FILE_NUM_PER_SSD, sizeof(index_array_context_t));

	ictx->ic_bloom_filter_head = bfctx;
	ictx->ic_index_array_head  = iactx;

	/* allocate memory for bloom filter bitmap */
	if(init_filter(&ictx->ic_bloom_filter_head->saddr) == -1)
		goto fail;
	if(init_filter(&ictx->ic_bloom_filter_head->daddr) == -1)
		goto fail;
	if(init_filter(&ictx->ic_bloom_filter_head->sport) == -1)
		goto fail;
	if(init_filter(&ictx->ic_bloom_filter_head->dport) == -1)
		goto fail;

	/* allocate memory for index array list */
	if (init_array(&ictx->ic_index_array_head->saddr) == -1)
		goto fail;
	if (init_array(&ictx->ic_index_array_head->daddr) == -1)
		goto fail;
	if (init_array(&ictx->ic_index_array_head->sport) == -1)
		goto fail;
	if (init_array(&ictx->ic_index_array_head->dport) == -1)
		goto fail;

	int i;
	bloom_filter_context_t *bf;
	index_array_context_t  *ia;
	for (i = 0; i < FILE_NUM_PER_SSD; i++) {
		bf = &ictx->ic_bloom_filter_head[i];
		bf->saddr = &ictx->ic_bloom_filter_head->saddr[i * FILTER_SIZE_BYTES];
		bf->daddr = &ictx->ic_bloom_filter_head->daddr[i * FILTER_SIZE_BYTES];
		bf->sport = &ictx->ic_bloom_filter_head->sport[i * FILTER_SIZE_BYTES];
		bf->dport = &ictx->ic_bloom_filter_head->dport[i * FILTER_SIZE_BYTES];

		ia = &ictx->ic_index_array_head[i];
		ia->cnt = -1;
		ia->filter = bf;
	}

	ictx->ic_cnt = 0;
	ictx->ic_index_array = &ictx->ic_index_array_head[0];
	LOG_MESSAGE("=== close init");

	return;

	fail:
		fprintf(stderr, "fail to initialize array\n");
		exit(-1);
}
/*****************************************************************************/



/*****************************************************************************/
static void
insert_into_bloom_filter_array(unsigned char *filter, unsigned int data)
{
	unsigned char* char_data = (unsigned char* )calloc(4, sizeof(unsigned char));
	convert_into_char(char_data, data);
	unsigned int hash[NUM_HASHES];
	int i;

	get_hashes(hash, char_data);
	free(char_data);	

	for (i = 0; i < NUM_HASHES; i++) {
		/* xor-fold the hash into FILTER_SIZE bits */
		hash[i] = (hash[i] >> FILTER_SIZE) ^ 
		          (hash[i] & FILTER_BITMASK);
		/* set the bit in the filter */
		filter[hash[i] >> 3] |= 1 << (hash[i] & 7);
	}
}
/*****************************************************************************/
void
insert_into_bloom_filter(bloom_filter_context_t *ctx, FlowMeta *meta)
{
	insert_into_bloom_filter_array(ctx->saddr, meta->flowinfo.saddr);
	insert_into_bloom_filter_array(ctx->daddr, meta->flowinfo.daddr);
	insert_into_bloom_filter_array(ctx->sport, meta->flowinfo.sport);
	insert_into_bloom_filter_array(ctx->dport, meta->flowinfo.dport);
}
/*****************************************************************************/
static inline void
insert_into_index_array(index_array_context_t *ctx, index_array_node_t *head, unsigned int data, FlowMeta *meta)
{
	index_array_node_t *node = &head[ctx->cnt+1];
	node->value  = data;
	node->fileID = meta->fileID;
	node->offset = meta->offset;
}
/*****************************************************************************/
inline int
insert_index(index_array_context_t *ctx, FlowMeta *meta)
{
	/* create_index? insert_index? */
	insert_into_index_array(ctx, ctx->saddr, meta->flowinfo.saddr, meta);
	insert_into_index_array(ctx, ctx->daddr, meta->flowinfo.daddr, meta);
	insert_into_index_array(ctx, ctx->sport, meta->flowinfo.sport, meta);
	insert_into_index_array(ctx, ctx->dport, meta->flowinfo.dport, meta);
	insert_into_bloom_filter(ctx->filter, meta);

	ctx->cnt = ctx->cnt+1;

	if(ctx->cnt >= ARRAY_SIZE * FILE_NUM_PER_SSD -1)
	{
		return 1;
	}

	return 0;
}
/*****************************************************************************/




/*****************************************************************************/
int
find_in_filter(unsigned char *filter, unsigned int data)
{
	unsigned char* char_data = (unsigned char* )calloc(4, sizeof(unsigned char));
	convert_into_char(char_data, data);
	unsigned int hash[NUM_HASHES];

	get_hashes(hash, char_data);
	free(char_data);
	int i;

	for (i = 0; i < NUM_HASHES; i++) {
		hash[i] = (hash[i] >> FILTER_SIZE) ^ 
		          (hash[i] & FILTER_BITMASK);
		if (!(filter[hash[i] >> 3] & (1 << (hash[i] & 7))))
			return 0;
	} 
	return 1;
}
/*****************************************************************************/
int
search_from_bloom_filter(bloom_filter_context_t *ctx, int type, unsigned int data)
{
	int res = -1;
	if (type | TYPE_SADDR)
		res = find_in_filter(ctx->saddr, data);
	else if (type | TYPE_DADDR)
		res = find_in_filter(ctx->daddr, data);
	else if (type | TYPE_SPORT)
		res = find_in_filter(ctx->sport, data);
	else if (type | TYPE_DPORT)
		res = find_in_filter(ctx->dport, data);
	
	if (res)
		return 1;
	else
		return 0;
	//TODO implement
}
/*****************************************************************************/
static inline int
binary_search(index_array_node_t *head, int start, int end, unsigned int data, int type)
{
	if (start > end) {
		if (type == SEARCH_EXACT) {
			return -1;
		} else if (type == SEARCH_MIN) {
			return start;
		} else if (type == SEARCH_MAX) {
			return end;
		}
	}

	int mid = (start + end) / 2;
	unsigned int mid_val = (&head[mid])->value;

	if (mid_val > data) {
		return binary_search(head, start, mid-1, data, type);
	} else if (mid_val < data) {
		return binary_search(head, mid+1, end, data, type);
	}

	return mid;
}
/*****************************************************************************/
static inline int
search_backward(index_array_node_t *head, int idx, unsigned int data) {
	if (idx == -1)
		return -1;

	while ( ((&head[idx])->value == data) && (idx > -1) )
		idx --;

	return idx + 1;
}
/*****************************************************************************/
static inline int
search_forward(index_array_node_t *head, int idx, unsigned int data) {
	if (idx == -1)
		return -1;

	while ( ((&head[idx])->value == data) && (idx < ARRAY_SIZE) )
		idx ++;

	return idx - 1;
}
/*****************************************************************************/
inline int *
search_index_single(index_array_context_t *ctx, int type, unsigned int data)
{
	static int ret[2];

	int idx   = -1;
	int start = -1;
	int end   = -1;

	switch (type) {
		case TYPE_SADDR:
			if (search_from_bloom_filter(ctx->filter, TYPE_SADDR, data)) {
				idx   = binary_search  (ctx->saddr, 0, ctx->cnt, data, SEARCH_EXACT);
				start = search_backward(ctx->saddr, idx, data);	
				end   = search_forward (ctx->saddr, idx, data);
			}
			break;
		case TYPE_DADDR:
			if (search_from_bloom_filter(ctx->filter, TYPE_DADDR, data)) {
				idx   = binary_search  (ctx->daddr, 0, ctx->cnt, data, SEARCH_EXACT);
				start = search_backward(ctx->daddr, idx, data);
				end   = search_forward (ctx->daddr, idx, data);
			}
			break;
		case TYPE_SPORT:
			if (search_from_bloom_filter(ctx->filter, TYPE_SPORT, data)){
				idx   = binary_search  (ctx->sport, 0, ctx->cnt, data, SEARCH_EXACT);
				start = search_backward(ctx->sport, idx, data);
				end   = search_forward (ctx->sport, idx, data);
			}
			break;
		case TYPE_DPORT:
			if (search_from_bloom_filter(ctx->filter, TYPE_DPORT, data)){
				idx   = binary_search  (ctx->dport, 0, ctx->cnt, data, SEARCH_EXACT);
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

	return ret;
}
/*****************************************************************************/
inline int *
search_index(index_array_context_t *ctx, int type, unsigned int start, unsigned int end)
{
	if (start > end) {
		return NULL;
	} else if (start == end) {
		return search_index_single(ctx, type, start);
	}

	int s_start, e_end;
	static int ret[2];

	s_start = 0;
	e_end = ARRAY_SIZE;

	switch (type) {
		case TYPE_SADDR:
			s_start = binary_search(ctx->saddr, 0, ctx->cnt, start, SEARCH_MIN);
			e_end   = binary_search(ctx->saddr, 0, ctx->cnt, end, SEARCH_MAX);
			break;
		case TYPE_DADDR:
			s_start = binary_search(ctx->daddr, 0, ctx->cnt, start, SEARCH_MIN);
			e_end   = binary_search(ctx->daddr, 0, ctx->cnt, end, SEARCH_MAX);
			break;
		case TYPE_SPORT:
			s_start = binary_search(ctx->sport, 0, ctx->cnt, start, SEARCH_MIN);
			e_end   = binary_search(ctx->sport, 0, ctx->cnt, end, SEARCH_MAX);
			break;
		case TYPE_DPORT:
			s_start = binary_search(ctx->dport, 0, ctx->cnt, start, SEARCH_MIN);
			e_end   = binary_search(ctx->dport, 0, ctx->cnt, end, SEARCH_MAX);
			break;
		default:
			printf("Unknown type\n");
			return NULL;
	}
	if (s_start > e_end)
		return NULL;

	ret[0] = s_start;
	ret[1] = e_end;

	return ret;
}
/*****************************************************************************/




/*****************************************************************************/
void
write_bloom_filter(bloom_filter_context_t *ctx)
{
	//TODO implement
}
/*****************************************************************************/
inline void
write_index_array(index_array_context_t *ctx)
{
	write_bloom_filter(ctx->filter);
	//TODO implement
}
/*****************************************************************************/




/*****************************************************************************/
void
clean_filter(unsigned char *filter)
{
	int i;
	for(i = 0; i < FILTER_SIZE_BYTES / 32; i++)
	{
		filter[i] = 0;
	}
}
/*****************************************************************************/
void
clean_bloom_filter(bloom_filter_context_t *ctx)
{
	clean_filter(ctx->saddr);
	clean_filter(ctx->daddr);
	clean_filter(ctx->sport);
	clean_filter(ctx->dport);
}
/*****************************************************************************/
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
/*****************************************************************************/
inline void 
clean_index_array(index_array_context_t *ctx)
{
	LOG_MESSAGE("=== start clean");

	clean_array(ctx->saddr);
	clean_array(ctx->daddr);
	clean_array(ctx->sport);
	clean_array(ctx->dport);

	clean_bloom_filter(ctx->filter);

	ctx->cnt = -1;

	LOG_MESSAGE("=== start clean");
}
/*****************************************************************************/




/*****************************************************************************/
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
/*****************************************************************************/
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
/*****************************************************************************/




/*****************************************************************************/
void
free_bloom_filter(bloom_filter_context_t *ctx)
{
	free(ctx->saddr);
	free(ctx->daddr);
	free(ctx->sport);
	free(ctx->dport);

	free(ctx);
}
/*****************************************************************************/
inline void
free_index_array(index_array_context_t *ctx)
{
	LOG_MESSAGE("=== start free");

	free_bloom_filter(ctx->filter);
	free(ctx->saddr);
	free(ctx->daddr);
	free(ctx->sport);
	free(ctx->dport);
	free(ctx);

	LOG_MESSAGE("=== start free");
}
/*****************************************************************************/




/*****************************************************************************/
inline void close_file_event(index_context_t *ictx)
{
	sort_array(ictx->ic_index_array);
	
	index_array_context_t *old_ia = ictx->ic_index_array;
	ictx->ic_cnt += 1;
	index_array_context_t *new_ia = &ictx->ic_index_array_head[ictx->ic_cnt];
	
	new_ia->saddr = &old_ia->saddr[old_ia->cnt];
	new_ia->daddr = &old_ia->daddr[old_ia->cnt];
	new_ia->sport = &old_ia->sport[old_ia->cnt];
	new_ia->dport = &old_ia->dport[old_ia->cnt];

	ictx->ic_index_array = new_ia;
	/*
	write_index_array(ctx);
	ctx->saddr = &(ctx->saddr[ctx->cnt+1]);
	ctx->daddr = &(ctx->daddr[ctx->cnt+1]);
	ctx->sport = &(ctx->sport[ctx->cnt+1]);
	ctx->dport = &(ctx->dport[ctx->cnt+1]);
	ctx->filter->saddr = &(ctx->filter->saddr[FILTER_SIZE_BYTES+1]);
	ctx->filter->daddr = &(ctx->filter->daddr[FILTER_SIZE_BYTES+1]);
	ctx->filter->sport = &(ctx->filter->sport[FILTER_SIZE_BYTES+1]);
	ctx->filter->dport = &(ctx->filter->dport[FILTER_SIZE_BYTES+1]);
	*/
}
/*****************************************************************************/