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
/* TODO change define values */
/*#define FILE_PER_INDEXER 1000*/
#define FILE_PER_INDEXER 10
#define INDEX_ARRAY_SIZE FILE_PER_INDEXER * 30000
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
init_bloom_filter(indexer_context_t *ictx)
{
	bloom_filter_t *filters;

	if ((filters = (bloom_filter_t *)calloc(FILE_PER_INDEXER,
					sizeof(bloom_filter_t))) == NULL)
		goto ibf_err1;

	bloom_filter_t *bf;
	int i;
	for(i = 0; i < FILE_PER_INDEXER; i++) {
		bf = &filters[i];
		if ((bf->saddr = calloc(FILTER_SIZE_BYTES, sizeof(u_char))) == NULL)
			goto ibf_err2;
		if ((bf->daddr = calloc(FILTER_SIZE_BYTES, sizeof(u_char))) == NULL)
			goto ibf_err3;
		if ((bf->sport = calloc(FILTER_SIZE_BYTES, sizeof(u_char))) == NULL)
			goto ibf_err4;
		if ((bf->dport = calloc(FILTER_SIZE_BYTES, sizeof(u_char))) == NULL)
			goto ibf_err5;
	}
	ictx->ic_bloom_filter = filters;
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
	index_info_t *info;

	info = (index_info_t *)calloc(FILE_PER_INDEXER, sizeof(index_info_t));
	if (info == NULL)	goto iia_err1;

	init_bloom_filter(ictx);

	ictx->ic_saddr_array = (array_node_t *)calloc(INDEX_ARRAY_SIZE, 
									   sizeof(array_node_t *));
	if (ictx->ic_saddr_array == NULL) goto iia_err2;
	ictx->ic_daddr_array = (array_node_t *)calloc(INDEX_ARRAY_SIZE, 
									   sizeof(array_node_t *));
	if (ictx->ic_daddr_array == NULL) goto iia_err3;
	ictx->ic_sport_array = (array_node_t *)calloc(INDEX_ARRAY_SIZE, 
									   sizeof(array_node_t *));
	if (ictx->ic_sport_array == NULL) goto iia_err4;
	ictx->ic_dport_array = (array_node_t *)calloc(INDEX_ARRAY_SIZE, 
									   sizeof(array_node_t *));
	if (ictx->ic_dport_array == NULL) goto iia_err5;
	ictx->ic_cnt = 0;
	return true;
 iia_err5:
	free(ictx->ic_saddr_array);
 iia_err4:
	free(ictx->ic_daddr_array);
 iia_err3:
	free(ictx->ic_sport_array);
 iia_err2:
	free(info);
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

	switch(type) {
		case TYPE_SADDR:
			if (search_from_bloom_filter(ia->filter, TYPE_SADDR, val)) {
				idx = binary_search(ia->saddr, 0, ia->cnt, val, SEARCH_EXACT);
				res[0] = search_backward(ia->saddr, idx, val);
				res[1] = search_forward(ia->saddr, idx, val, ia->cnt);
			}
			break;
		case TYPE_DADDR:
			if (search_from_bloom_filter(ia->filter, TYPE_DADDR, val)) {
				idx = binary_search(ia->daddr, 0, ia->cnt, val, SEARCH_EXACT);
				res[0] = search_backward(ia->daddr, idx, val);
				res[1] = search_forward(ia->daddr, idx, val, ia->cnt);
			}
			break;
		case TYPE_SPORT:
			if (search_from_bloom_filter(ia->filter, TYPE_SPORT, val)) {
				idx = binary_search(ia->sport, 0, ia->cnt, val, SEARCH_EXACT);
				res[0] = search_backward(ia->sport, idx, val);
				res[1] = search_forward(ia->sport, idx, val, ia->cnt);
			}
			break;
		case TYPE_DPORT:
			if (search_from_bloom_filter(ia->filter, TYPE_DPORT, val)) {
				idx = binary_search(ia->dport, 0, ia->cnt, val, SEARCH_EXACT);
				res[0] = search_backward(ia->dport, idx, val);
				res[1] = search_forward (ia->dport, idx, val, ia->cnt);
			}
			break;
		default:
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
	switch(type) {
		case TYPE_SADDR:
			res[0] = binary_search(ia->saddr, 0, ia->cnt, min_val, SEARCH_MIN);
			res[1] = binary_search(ia->saddr, 0, ia->cnt, max_val, SEARCH_MAX);
			break;
		case TYPE_DADDR:
			res[0] = binary_search(ia->daddr, 0, ia->cnt, min_val, SEARCH_MIN);
			res[1] = binary_search(ia->daddr, 0, ia->cnt, max_val, SEARCH_MAX);
			break;
		case TYPE_SPORT:
			res[0] = binary_search(ia->sport, 0, ia->cnt, min_val, SEARCH_MIN);
			res[1] = binary_search(ia->sport, 0, ia->cnt, max_val, SEARCH_MAX);
			break;
		case TYPE_DPORT:
			res[0] = binary_search(ia->dport, 0, ia->cnt, min_val, SEARCH_MIN);
			res[1] = binary_search(ia->dport, 0, ia->cnt, max_val, SEARCH_MAX);
			break;
		default:
			fprintf(stderr, "Unknown type\n");
			return;
	}
}
/*****************************************************************************/
// TODO rename function
static inline void
get_next_file(indexer_context_t *ictx)
{
	index_array_t *ia;
	array_node_t  *saddr;
	array_node_t  *daddr;
	array_node_t  *sport;
	array_node_t  *dport;

	//TODO assign fileID to bf, ia
	sort_array(ictx);
	ia = &ictx->ic_index[ictx->ic_array_idx];
	saddr = &ia->saddr[ia->cnt];
	daddr = &ia->daddr[ia->cnt];
	sport = &ia->sport[ia->cnt];
	dport = &ia->dport[ia->cnt];
	ictx->ic_array_idx += 1;
	ia = &ictx->ic_index[ictx->ic_array_idx];
	ia->cnt   = 0;
	ia->saddr = saddr;
	ia->daddr = daddr;
	ia->sport = sport;
	ia->dport = dport;
}
/*****************************************************************************/
