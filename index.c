#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/types.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include "index.h"
#include "hashes.h"
/*****************************************************************************/
/*#define INDEX_PER_FILE 40000*/
/*#define FILE_PER_INDEXER 1000*/
#define INDEX_PER_FILE 4
#define FILE_PER_INDEXER 10
#define INDEX_ARRAY_SIZE INDEX_PER_FILE * FILE_PER_INDEXER
#define FILTER_SIZE 20
#define FILTER_SIZE_BYTES (1 << (FILTER_SIZE - 3))
#define FILTER_BITMASK ((1 << FILTER_SIZE) - 1)
#define NUM_HASHES 7
/*****************************************************************************/
inline void
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
static inline void
init_bloom_filter(index_array_t *ia)
{
	bloom_filter_t *bf;

	if ((bf = (bloom_filter_t *)malloc(sizeof(bloom_filter_t))) == NULL)
		goto ibf_err1;
	if ((bf->saddr = calloc(FILTER_SIZE_BYTES, sizeof(u_char))) == NULL)
		goto ibf_err2;
	if ((bf->daddr = calloc(FILTER_SIZE_BYTES, sizeof(u_char))) == NULL)
		goto ibf_err3;
	if ((bf->sport = calloc(FILTER_SIZE_BYTES, sizeof(u_char))) == NULL)
		goto ibf_err4;
	if ((bf->dport = calloc(FILTER_SIZE_BYTES, sizeof(u_char))) == NULL)
		goto ibf_err5;
	ia->filter = bf;
	return;
 ibf_err5:
	free(bf->sport);
 ibf_err4:
	free(bf->daddr);
 ibf_err3:
	free(bf->saddr);
 ibf_err2:
	free(bf);
 ibf_err1:
	perror("malloc");
	exit(0);
}
/*****************************************************************************/
inline bool
init_index_array(indexer_context_t *ictx)
{
	int i;
	index_array_t *ia;

	ia = (index_array_t *)calloc(FILE_PER_INDEXER, sizeof(index_array_t));
	if (ia == NULL)	goto iia_err1;

	for (i = 0; i < FILE_PER_INDEXER; i++)
		init_bloom_filter(&ia[i]);

	ia->saddr = 
		(array_node_t *)calloc(INDEX_ARRAY_SIZE, sizeof(array_node_t *));
	if (ia->saddr == NULL) goto iia_err2;
	ia->daddr =
		(array_node_t *)calloc(INDEX_ARRAY_SIZE, sizeof(array_node_t *));
	if (ia->daddr == NULL) goto iia_err3;
	ia->sport = 
		(array_node_t *)calloc(INDEX_ARRAY_SIZE, sizeof(array_node_t *));
	if (ia->sport == NULL) goto iia_err4;
	ia->dport =
		(array_node_t *)calloc(INDEX_ARRAY_SIZE, sizeof(array_node_t *));
	if (ia->dport == NULL) goto iia_err5;
	ia->cnt = 0;
	ictx->ic_index = ia;
	ictx->ic_remain_node = INDEX_ARRAY_SIZE;
	return true;
 iia_err5:
	free(ia->sport);
 iia_err4:
	free(ia->daddr);
 iia_err3:
	free(ia->saddr);
 iia_err2:
	free(ia);
 iia_err1:
	perror("malloc");
	return false;
}
/*****************************************************************************/
static inline void
insert_into_bloom_filter(unsigned char *filter, unsigned int data)
{
	unsigned char char_data[4];
	unsigned int hash[NUM_HASHES];
	int i;

	convert_into_char(char_data, data);
	get_hashes(hash, char_data);
	for (i = 0; i < NUM_HASHES; i++) {
		hash[i] = (hash[i] >> FILTER_SIZE) ^ (hash[i] & FILTER_BITMASK);
		filter[hash[i] >> 3] |= 1 << (hash[i] & 7);
	}
}
/*****************************************************************************/
static inline void
insert_into_index_array(index_array_t *ia, array_node_t *head, 
						unsigned int data, FlowMeta *meta)
{
	array_node_t *node = &head[ia->cnt];

	node->value = data;
	node->fileID = meta->fileID;
	node->offset = meta->offset;
}
/*****************************************************************************/
inline void
insert_index(indexer_context_t *ictx, FlowMeta *meta)
{
	index_array_t *ia = &ictx->ic_index[ictx->ic_array_idx];
	bloom_filter_t *bf = ia->filter;

	/* insert index into index array */
	insert_into_index_array(ia, ia->saddr, meta->flowinfo.saddr, meta);
	insert_into_index_array(ia, ia->daddr, meta->flowinfo.daddr, meta);
	insert_into_index_array(ia, ia->sport, meta->flowinfo.sport, meta);
	insert_into_index_array(ia, ia->dport, meta->flowinfo.dport, meta);
	ia->cnt++;
	ictx->ic_remain_node--;

	/* insert index into bloom filter */
	insert_into_bloom_filter(bf->saddr, meta->flowinfo.saddr);
	insert_into_bloom_filter(bf->daddr, meta->flowinfo.daddr);
	insert_into_bloom_filter(bf->sport, meta->flowinfo.sport);
	insert_into_bloom_filter(bf->dport, meta->flowinfo.dport);
}
/*****************************************************************************/
static inline int
search_filter(unsigned char *filter, unsigned int data)
{
	unsigned char char_data[4];
	unsigned int hash[NUM_HASHES];
	int i;

	convert_into_char(char_data, data);
	get_hashes(hash, char_data);
	for (i = 0; i < NUM_HASHES; i++) {
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

	if (type == TYPE_SADDR)
		res = search_filter(filter->saddr, data);
	else if (type == TYPE_DADDR)
		res = search_filter(filter->daddr, data);
	else if (type == TYPE_SPORT)
		res = search_filter(filter->sport, data);
	else if (type == TYPE_DPORT)
		res = search_filter(filter->dport, data);
	else {
		fprintf(stderr, "Unknown type\n");
		return -1;
	}

	return (res? -1 : 0);
}
/*****************************************************************************/
static inline int
binary_search(array_node_t *head, int start, int end, 
			  unsigned int data, int type)
{
	int mid;
	unsigned int mid_val;

	if (start > end) {
		if (type == SEARCH_EXACT)
			return -1;
		else if (type == SEARCH_MIN)
			return start;
		else if (type == SEARCH_MAX)
			return end;
		else {
			fprintf(stderr, "Unknown type\n");
			return -1;
		}
	}
	mid = (start + end) / 2;
	mid_val = head[mid].value;
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
	while ((head[idx].value == data) && (idx > -1))
		idx--;
	return idx + 1;
}
/*****************************************************************************/
static inline int
search_forward(array_node_t *head, int idx, unsigned int data, int size)
{
	if (idx == -1)
		return -1;
	while((head[idx].value == data) && (idx < size))
		idx++;
	return idx - 1;
}
/*****************************************************************************/
static inline void
search_single_index(indexer_context_t *ictx, int type, 
					unsigned int val, int res[])
{
	int idx;
	index_array_t *ia = &ictx->ic_index[ictx->ic_array_idx];

	if (type == TYPE_SADDR) {
		if (search_from_bloom_filter(ia->filter, TYPE_SADDR, val)) {
			idx = binary_search(ia->saddr, 0, ia->cnt, val, SEARCH_EXACT);
			res[0] = search_backward(ia->saddr, idx, val);
			res[1] = search_forward(ia->saddr, idx, val, ia->cnt);
		}
	} else if (type == TYPE_DADDR) {
		if (search_from_bloom_filter(ia->filter, TYPE_DADDR, val)) {
			idx = binary_search(ia->daddr, 0, ia->cnt, val, SEARCH_EXACT);
			res[0] = search_backward(ia->daddr, idx, val);
			res[1] = search_forward(ia->daddr, idx, val, ia->cnt);
		}
	} else if (type == TYPE_SPORT) {
		if (search_from_bloom_filter(ia->filter, TYPE_SPORT, val)) {
			idx = binary_search(ia->sport, 0, ia->cnt, val, SEARCH_EXACT);
			res[0] = search_backward(ia->sport, idx, val);
			res[1] = search_forward(ia->sport, idx, val, ia->cnt);
		}
	} else if (type == TYPE_DPORT) {
		if (search_from_bloom_filter(ia->filter, TYPE_DPORT, val)) {
			idx = binary_search(ia->dport, 0, ia->cnt, val, SEARCH_EXACT);
			res[0] = search_backward(ia->dport, idx, val);
			res[1] = search_forward (ia->dport, idx, val, ia->cnt);
		}
	} else {
		fprintf(stderr, "Unknown type\n");
		return;
	}
}
/*****************************************************************************/
/* README usage
 * int res[2]
 * search_index(ictx, TYPE_SADDR, min_val, max_val, res);
 * res[0]: minimum index
 * res[1]: maximum index
 */
inline void
search_index(indexer_context_t *ictx, int type, 
			 unsigned int min_val, unsigned int max_val, int res[])
{
	index_array_t *ia;

	if (min_val > max_val)
		return;
	else if (min_val == max_val)
		return search_single_index(ictx, type, min_val, res);

	ia = &ictx->ic_index[ictx->ic_array_idx];
	if (type == TYPE_SADDR) {
		res[0] = binary_search(ia->saddr, 0, ia->cnt, min_val, SEARCH_MIN);
		res[1] = binary_search(ia->saddr, 0, ia->cnt, max_val, SEARCH_MAX);
	} else if (type == TYPE_DADDR) {
		res[0] = binary_search(ia->daddr, 0, ia->cnt, min_val, SEARCH_MIN);
		res[1] = binary_search(ia->daddr, 0, ia->cnt, max_val, SEARCH_MAX);
	} else if (type == TYPE_SPORT) {
		res[0] = binary_search(ia->sport, 0, ia->cnt, min_val, SEARCH_MIN);
		res[1] = binary_search(ia->sport, 0, ia->cnt, max_val, SEARCH_MAX);
	} else if (type == TYPE_DPORT) {
		res[0] = binary_search(ia->dport, 0, ia->cnt, min_val, SEARCH_MIN);
		res[1] = binary_search(ia->dport, 0, ia->cnt, max_val, SEARCH_MAX);
	} else {
		fprintf(stderr, "Unknown type\n");
		return;
	}
}
/*****************************************************************************/
// TODO rename function
inline void
get_next_file(indexer_context_t *ictx)
{
	index_array_t *ia;
	sort_array(ictx);

	if (ictx->ic_remain_node < INDEX_PER_FILE) {
		ictx->ic_array_idx = 0;
		ictx->ic_remain_node = INDEX_ARRAY_SIZE;
		int i;
		for (i = 0; i < FILE_PER_INDEXER; i++) {
			ia = &ictx->ic_index[i];
			ia->cnt = 0;
		}
	} else {
		ia = &ictx->ic_index[ictx->ic_array_idx];
		array_node_t *old_saddr = &ia->saddr[ia->cnt];
		array_node_t *old_daddr = &ia->daddr[ia->cnt];
		array_node_t *old_sport = &ia->sport[ia->cnt];
		array_node_t *old_dport = &ia->dport[ia->cnt];
		ictx->ic_array_idx += 1;

		ia = &ictx->ic_index[ictx->ic_array_idx];
		ia->cnt   = 0;
		ia->saddr = old_saddr;
		ia->daddr = old_daddr;
		ia->sport = old_sport;
		ia->dport = old_dport;
	}
}
/*****************************************************************************/
