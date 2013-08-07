#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/types.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "index.h"
#include "hashes.h"

/*#define ARRAY_SIZE 40000*/
/*#define FILE_PER_INDEXER 1000*/
#define ARRAY_SIZE 4
#define FILE_PER_INDEXER 3
#define FILTER_SIZE 20
#define FILTER_SIZE_BYTES (1 << (FILTER_SIZE - 3))
#define FILTER_BITMASK ((1 << FILTER_SIZE) - 1)
#define NUM_HASHES 7

/*****************************************************************************/
void sort_array(indexer_context_t *ictx)
{
	index_array_t *ia = &ictx->ic_index[ictx->ic_array_idx];
	QSORT(struct array_node, ia->saddr, ia->cnt, COMPARE_VALUE);
	QSORT(struct array_node, ia->daddr, ia->cnt, COMPARE_VALUE);
	QSORT(struct array_node, ia->sport, ia->cnt, COMPARE_VALUE);
	QSORT(struct array_node, ia->dport, ia->cnt, COMPARE_VALUE);
}
/*****************************************************************************/
void convert_into_char(unsigned char *char_data, unsigned int data)
{
	char_data[0] = (data >> 24) & 0xFF;
	char_data[1] = (data >> 16) & 0xFF;
	char_data[2] = (data >> 8 ) & 0xFF;
	char_data[3] = (data)       & 0xFF;
}
/*****************************************************************************/
void get_hashes(unsigned int hash[], unsigned char *data)
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
void init_bloom_filter(index_array_t *ia)
{
	bloom_filter_t *bf = (bloom_filter_t *)malloc(sizeof(bloom_filter_t));

	bf->saddr = (unsigned char *)calloc(FILTER_SIZE_BYTES, sizeof(unsigned char));
	bf->daddr = (unsigned char *)calloc(FILTER_SIZE_BYTES, sizeof(unsigned char));
	bf->sport = (unsigned char *)calloc(FILTER_SIZE_BYTES, sizeof(unsigned char));
	bf->dport = (unsigned char *)calloc(FILTER_SIZE_BYTES, sizeof(unsigned char));

	ia->filter = bf;
}
/*****************************************************************************/
void init_index_array(indexer_context_t *ictx, index_array_t *ia)
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
	ictx->ic_index = ia;
}
/*****************************************************************************/
void init_indexer_context(indexer_context_t *ictx)
{
	memset(ictx, 0, sizeof(indexer_context_t));
	index_array_t *ia;
	init_index_array(ictx, ia);

	ictx->ic_array_idx = 0;
	ictx->ic_done = false;
}
/*****************************************************************************/




/*****************************************************************************/
int search_in_filter(unsigned char *filter, unsigned int data)
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




/*****************************************************************************/
void insert_into_bloom_filter(unsigned char *filter, unsigned int data)
{
	unsigned char char_data[4];
	convert_into_char(char_data, data);
	unsigned int hash[NUM_HASHES];

	get_hashes(hash, char_data);
	int i;
	for (i=0; i<NUM_HASHES; i++) {
		hash[i] = (hash[i] >> FILTER_SIZE) ^ (hash[i] & FILTER_BITMASK);
		filter[hash[i] >> 3] |=1 << (hash[i] & 7);
	}
}
/*****************************************************************************/
void insert_into_index_array(index_array_t *ia, array_node_t *head, unsigned int data, FlowMeta *meta)
{
	array_node_t *node = &head[ia->cnt];
	node->value = data;
	node->fileID = meta->fileID;
	node->offset = meta->offset;
}
/*****************************************************************************/
void insert_index(indexer_context_t *ictx, FlowMeta *meta)
{
	index_array_t *ia = &ictx->ic_index[ictx->ic_array_idx];
	insert_into_index_array(ia, ia->saddr, meta->flowinfo.saddr, meta);
	insert_into_index_array(ia, ia->daddr, meta->flowinfo.daddr, meta);
	insert_into_index_array(ia, ia->sport, meta->flowinfo.sport, meta);
	insert_into_index_array(ia, ia->dport, meta->flowinfo.dport, meta);
	ia->cnt += 1;

	bloom_filter_t *bf = ia->filter;
	insert_into_bloom_filter(bf->saddr, meta->flowinfo.saddr);
	insert_into_bloom_filter(bf->daddr, meta->flowinfo.daddr);
	insert_into_bloom_filter(bf->sport, meta->flowinfo.sport);
	insert_into_bloom_filter(bf->dport, meta->flowinfo.dport);
}
/*****************************************************************************/
void create_index(indexer_context_t *ictx, FlowMeta *meta_block, int size)
{
	int i;
	for (i=0; i<size; i++) {
		insert_index(ictx, &meta_block[i]);
	}
}
/*****************************************************************************/




// TODO rename function
/*****************************************************************************/
void get_next_file(indexer_context_t *ictx)
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

	ia->saddr = saddr;
	ia->daddr = daddr;
	ia->sport = sport;
	ia->dport = dport;
}
/*****************************************************************************/

int main()
{
	indexer_context_t tmp;
	init_indexer_context(&tmp);
	indexer_context_t *ictx = &tmp;


#include <time.h>
	srand(time(NULL));
	int i,j,k;
	FlowMeta *meta = (FlowMeta *)malloc(sizeof(FlowMeta));
	FlowInfo *info = &meta->flowinfo;

	for (i=0;i<FILE_PER_INDEXER;i++) {
		for (j=0;j<ARRAY_SIZE;j++) {
			info->saddr = rand();
			info->daddr = rand();
			info->sport = rand();
			info->dport = rand();

			printf("%u\n", info->saddr);
			insert_index(ictx, meta);
		}
		get_next_file(ictx);
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
}
