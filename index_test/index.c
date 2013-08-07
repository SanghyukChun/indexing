#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/types.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "index.h"
#include "hashes.h"

/* TODO change define values */
/*#define ARRAY_SIZE 40000*/
/*#define FILE_PER_INDEXER 1000*/
#define ARRAY_SIZE 4
#define FILE_PER_INDEXER 10
#define FILTER_SIZE 20
#define FILTER_SIZE_BYTES (1 << (FILTER_SIZE - 3))
#define FILTER_BITMASK ((1 << FILTER_SIZE) - 1)
#define NUM_HASHES 7

/*****************************************************************************/
static inline void
sort_array(indexer_context_t *ictx)
{
	index_array_t *ia = &ictx->ic_index[ictx->ic_array_idx];
	QSORT(struct array_node, ia->saddr, ia->cnt, COMPARE_VALUE);
	QSORT(struct array_node, ia->daddr, ia->cnt, COMPARE_VALUE);
	QSORT(struct array_node, ia->sport, ia->cnt, COMPARE_VALUE);
	QSORT(struct array_node, ia->dport, ia->cnt, COMPARE_VALUE);
}
/*****************************************************************************/
static inline void
convert_into_char(unsigned char *char_data, unsigned int data)
{
	char_data[0] = (data >> 24) & 0xFF;
	char_data[1] = (data >> 16) & 0xFF;
	char_data[2] = (data >> 8 ) & 0xFF;
	char_data[3] = (data)       & 0xFF;
}
/*****************************************************************************/
static inline void
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




/*****************************************************************************/
static inline void
init_bloom_filter(index_array_t *ia)
{
	bloom_filter_t *bf = (bloom_filter_t *)malloc(sizeof(bloom_filter_t));

	bf->saddr = (unsigned char *)calloc(FILTER_SIZE_BYTES, sizeof(unsigned char));
	bf->daddr = (unsigned char *)calloc(FILTER_SIZE_BYTES, sizeof(unsigned char));
	bf->sport = (unsigned char *)calloc(FILTER_SIZE_BYTES, sizeof(unsigned char));
	bf->dport = (unsigned char *)calloc(FILTER_SIZE_BYTES, sizeof(unsigned char));

	ia->filter = bf;
}
/*****************************************************************************/
static inline void
init_index_array(indexer_context_t *ictx, index_array_t *ia)
{
	ia = (index_array_t *)calloc(FILE_PER_INDEXER, sizeof(index_array_t));

	int i;
	for (i=0; i<FILE_PER_INDEXER; i++) {
		init_bloom_filter(&ia[i]);
	}

	ia[0].saddr = (array_node_t *)calloc(ARRAY_SIZE * FILE_PER_INDEXER, sizeof(array_node_t *));
	ia[0].daddr = (array_node_t *)calloc(ARRAY_SIZE * FILE_PER_INDEXER, sizeof(array_node_t *));
	ia[0].sport = (array_node_t *)calloc(ARRAY_SIZE * FILE_PER_INDEXER, sizeof(array_node_t *));
	ia[0].dport = (array_node_t *)calloc(ARRAY_SIZE * FILE_PER_INDEXER, sizeof(array_node_t *));
	ia[0].cnt   = 0;
	ictx->ic_index = ia;
}
/*****************************************************************************/
inline void
init_indexer_context(indexer_context_t *ictx)
{
	memset(ictx, 0, sizeof(indexer_context_t));
	index_array_t *ia;
	init_index_array(ictx, ia);

	ictx->ic_array_idx = 0;
	ictx->ic_done = false;
}
/*****************************************************************************/




/*****************************************************************************/
static inline void
insert_into_bloom_filter(unsigned char *filter, unsigned int data)
{
	unsigned char char_data[4];
	convert_into_char(char_data, data);
	unsigned int hash[NUM_HASHES];

	get_hashes(hash, char_data);
	int i;
	for (i=0; i<NUM_HASHES; i++) {
		hash[i] = (hash[i] >> FILTER_SIZE) ^ (hash[i] & FILTER_BITMASK);
		filter[hash[i] >> 3] |= 1 << (hash[i] & 7);
	}
}
/*****************************************************************************/
static inline void
insert_into_index_array(index_array_t *ia, array_node_t *head, unsigned int data, FlowMeta *meta)
{
	array_node_t *node = &head[ia->cnt];
	node->value = data;
	node->fileID = meta->fileID;
	node->offset = meta->offset;
}
/*****************************************************************************/
static inline void
insert_index(indexer_context_t *ictx, FlowMeta *meta)
{
	/* insert index into index array */
	index_array_t *ia = &ictx->ic_index[ictx->ic_array_idx];
	insert_into_index_array(ia, ia->saddr, meta->flowinfo.saddr, meta);
	insert_into_index_array(ia, ia->daddr, meta->flowinfo.daddr, meta);
	insert_into_index_array(ia, ia->sport, meta->flowinfo.sport, meta);
	insert_into_index_array(ia, ia->dport, meta->flowinfo.dport, meta);
	ia->cnt += 1;

	/* insert index into bloom filter */
	bloom_filter_t *bf = ia->filter;
	insert_into_bloom_filter(bf->saddr, meta->flowinfo.saddr);
	insert_into_bloom_filter(bf->daddr, meta->flowinfo.daddr);
	insert_into_bloom_filter(bf->sport, meta->flowinfo.sport);
	insert_into_bloom_filter(bf->dport, meta->flowinfo.dport);
}
/*****************************************************************************/
inline void
create_index(indexer_context_t *ictx, FlowMeta *meta_block, int size)
{
	int i;
	for (i=0; i<size; i++) {
		insert_index(ictx, &meta_block[i]);
	}
}
/*****************************************************************************/




/*****************************************************************************/
static inline int
search_filter(unsigned char *filter, unsigned int data)
{
	unsigned char char_data[4];
	convert_into_char(char_data, data);
	unsigned int hash[NUM_HASHES];

	get_hashes(hash, char_data);
	int i;
	for (i=0; i<NUM_HASHES; i++) {
		hash[i] = (hash[i] >> FILTER_SIZE) ^ (hash[i] & FILTER_BITMASK);
		if (!(filter[hash[i] >> 3] & (1 << (hash[i] & 7))))
			return 0;
	}
	return 1;
}
/*****************************************************************************/
static inline int
search_from_bloom_filter(bloom_filter_t *filter, int type, unsigned int data)
{
	int res = -1;
	switch(type) {
		case TYPE_SADDR:
			res = search_filter(filter->saddr, data);
			break;
		case TYPE_DADDR:
			res = search_filter(filter->daddr, data);
			break;
		case TYPE_SPORT:
			res = search_filter(filter->sport, data);
			break;
		case TYPE_DPORT:
			res = search_filter(filter->dport, data);
			break;
		default:
			fprintf(stderr, "Unknown type\n");
			return -1;
	}

	if (res)
		return -1;
	else
		return 0;
}
/*****************************************************************************/
static inline int
binary_search(array_node_t *head, int start, int end, unsigned int data, int type)
{
	if (start > end) {
		switch(type) {
			case SEARCH_EXACT:
				return -1;
			case SEARCH_MIN:
				return start;
			case SEARCH_MAX:
				return end;
			default:
				fprintf(stderr, "Unknown type\n");
				return -1;
		}
	}

	int mid = (start + end) / 2;
	unsigned int mid_val = head[mid].value;

	printf("s: %d, e: %d, data: %u, midval: %u\n", start, end, data, mid_val);

	if (mid_val > data)
		return binary_search(head, start, mid-1, data, type);
	else if (mid_val < data)
		return binary_search(head, mid+1, end, data, type);

	return mid;
}
/*****************************************************************************/
static inline int
search_backward(array_node_t *head, int idx, unsigned int data)
{
	if (idx == -1)
		return -1;

	while( (head[idx].value == data) && (idx > -1) )
		idx --;

	return idx + 1;
}
/*****************************************************************************/
static inline int
search_forward(array_node_t *head, int idx, unsigned int data, int size)
{
	if (idx == -1)
		return -1;

	while( (head[idx].value == data) && (idx < size) )
		idx ++;

	return idx - 1;
}
/*****************************************************************************/
static inline void
search_single_index(indexer_context_t *ictx, int type, unsigned int val, int res[])
{
	int idx, min_idx, max_idx;
	index_array_t *ia = &ictx->ic_index[ictx->ic_array_idx];

	switch(type) {
		case TYPE_SADDR:
			if (search_from_bloom_filter(ia->filter, TYPE_SADDR, val)) {
				idx     = binary_search  (ia->saddr, 0, ia->cnt, val, SEARCH_EXACT);
				max_idx = search_forward (ia->saddr, idx, val, ia->cnt);
				min_idx = search_backward(ia->saddr, idx, val);
			}
			break;
		case TYPE_DADDR:
			if (search_from_bloom_filter(ia->filter, TYPE_DADDR, val)) {
				idx     = binary_search  (ia->daddr, 0, ia->cnt, val, SEARCH_EXACT);
				max_idx = search_forward (ia->daddr, idx, val, ia->cnt);
				min_idx = search_backward(ia->daddr, idx, val);
			}
			break;
		case TYPE_SPORT:
			if (search_from_bloom_filter(ia->filter, TYPE_SPORT, val)) {
				idx     = binary_search  (ia->sport, 0, ia->cnt, val, SEARCH_EXACT);
				max_idx = search_forward (ia->sport, idx, val, ia->cnt);
				min_idx = search_backward(ia->sport, idx, val);
			}
			break;
		case TYPE_DPORT:
			if (search_from_bloom_filter(ia->filter, TYPE_DPORT, val)) {
				idx     = binary_search  (ia->dport, 0, ia->cnt, val, SEARCH_EXACT);
				printf("idx: %d\n", idx);
				max_idx = search_forward (ia->dport, idx, val, ia->cnt);
				min_idx = search_backward(ia->dport, idx, val);
			}
			break;
		default:
			fprintf(stderr, "Unknown type\n");
			return;
	}

	res[0] = min_idx;
	res[1] = max_idx;
}
/*****************************************************************************/
inline void
search_index(indexer_context_t *ictx, int type, unsigned int min_val, unsigned int max_val, int res[])
{
	/* README usage
	 * int res[2];
	 * search_index(ictx, TYPE_SADDR, min_val, max_val, res);
	 * int min_idx = res[0];
	 * int max_idx = res[1];
	 * */
	if (min_val > max_val)
		return;
	else if (min_val == max_val)
		return search_single_index(ictx, type, min_val, res);

	int min_idx, max_idx;
	index_array_t *ia = &ictx->ic_index[ictx->ic_array_idx];

	switch(type) {
		case TYPE_SADDR:
			min_idx = binary_search(ia->saddr, 0, ia->cnt, min_val, SEARCH_MIN);
			max_idx = binary_search(ia->saddr, 0, ia->cnt, max_val, SEARCH_MAX);
			break;
		case TYPE_DADDR:
			min_idx = binary_search(ia->daddr, 0, ia->cnt, min_val, SEARCH_MIN);
			max_idx = binary_search(ia->daddr, 0, ia->cnt, max_val, SEARCH_MAX);
			break;
		case TYPE_SPORT:
			min_idx = binary_search(ia->sport, 0, ia->cnt, min_val, SEARCH_MIN);
			max_idx = binary_search(ia->sport, 0, ia->cnt, max_val, SEARCH_MAX);
			break;
		case TYPE_DPORT:
			min_idx = binary_search(ia->dport, 0, ia->cnt, min_val, SEARCH_MIN);
			max_idx = binary_search(ia->dport, 0, ia->cnt, max_val, SEARCH_MAX);
			break;
		default:
			fprintf(stderr, "Unknown type\n");
			return;
	}

	res[0] = min_idx;
	res[1] = max_idx;
}
/*****************************************************************************/




// TODO rename function
/*****************************************************************************/
static inline void
get_next_file(indexer_context_t *ictx)
{
	//TODO assign fileID to bf, ia
	sort_array(ictx);
	index_array_t *ia = &ictx->ic_index[ictx->ic_array_idx];

	array_node_t *saddr = &ia->saddr[ia->cnt];
	array_node_t *daddr = &ia->daddr[ia->cnt];
	array_node_t *sport = &ia->sport[ia->cnt];
	array_node_t *dport = &ia->dport[ia->cnt];

	ictx->ic_array_idx += 1;
	ia = &ictx->ic_index[ictx->ic_array_idx];

	ia->cnt   = 0;
	ia->saddr = saddr;
	ia->daddr = daddr;
	ia->sport = sport;
	ia->dport = dport;
}
/*****************************************************************************/




/* TODO delete sample program*/
int main()
{
	indexer_context_t tmp;
	init_indexer_context(&tmp);
	indexer_context_t *ictx = &tmp;


#include <time.h>
	srand(time(NULL));
	int i,j;
	FlowMeta *meta = (FlowMeta *)malloc(sizeof(FlowMeta));
	FlowInfo *info = &meta->flowinfo;

	unsigned int value;
	for (i=0;i<FILE_PER_INDEXER;i++) {
		for (j=0;j<ARRAY_SIZE;j++) {
			info->saddr = rand();
			info->daddr = rand();
			info->sport = rand();
			info->dport = rand();
			value = info->saddr;

			printf("%u\n", info->saddr);
			insert_index(ictx, meta);
		}
		if (i != FILE_PER_INDEXER-1)
			get_next_file(ictx);
		else
			sort_array(ictx);
	}
	index_array_t *ia;
	array_node_t *head;
	array_node_t *index_array;
	/*head = ictx->ic_index[0].saddr;*/
	/*unsigned char *bitmap;*/
	/*int i,j;*/
	/*for (i=0;i<FILE_PER_INDEXER;i++) {*/
		/*ia = &ictx->ic_index[i];*/
		/*bitmap = ia->filter->saddr;*/
		/*for (j=0;j<FILTER_SIZE_BYTES;j++) {*/
			/*bitmap[j] = i;*/
		/*}*/
	/*}*/

	/*head = ictx->ic_index[1].saddr;*/
	/*unsigned char *bitmap;*/
	/*for (i=0;i<FILE_PER_INDEXER;i++) {*/
		/*ia = &ictx->ic_index[i];*/
		/*bitmap = ia->filter->saddr;*/
		/*for (j=0;j<FILTER_SIZE_BYTES;j++) {*/
			/*printf("%d",bitmap[j]);*/
		/*}*/
	/*}*/

	/*for (i=0;i<FILE_PER_INDEXER;i++) {*/
		/*ia = &ictx->ic_index[i];*/
		/*ia->saddr = head;*/
		/*index_array = ia->saddr;*/
		/*for (j=0;j<ARRAY_SIZE;j++) {*/
			/*index_array[j].value = i;*/
		/*}*/
		/*head = &index_array[j];*/
	/*}*/


	printf("=========================\n");
	printf("value: %u\n", value);
	int res[2];
	search_index(ictx, TYPE_SADDR, value, value, res);
	/*search_index(ictx, TYPE_SADDR, 0, value, res);*/
	int min_idx = res[0];
	int max_idx = res[1];

	printf("min: %d, max:%d\n", min_idx, max_idx);

	printf("=========================\n");
	head = ictx->ic_index[0].saddr;
	for (i=0;i<FILE_PER_INDEXER;i++) {
		ia = &ictx->ic_index[i];
		ia->saddr = head;
		index_array = ia->saddr;
		for (j=0;j<ia->cnt;j++) {
			printf("%u\n",index_array[j].value);
		}
		head = &index_array[j];
	}
	return 0;
}
