#include <stdio.h>
#include <stdlib.h>
#include "bloom_filter.h"
#include "hashes.h"

#define FILTER_SIZE 20
#define NUM_HASHES 7
#define FILTER_SIZE_BYTES (1 << (FILTER_SIZE - 3))
#define FILTER_BITMASK ((1 << FILTER_SIZE) - 1)
#define WORD_BUF_SIZE 32
#define NUM_HASHES 7
/**
 * init context for bloom filter
 * @param ctx [description]
 */
void
init_bloom_filter(bloom_filter_context_t *ctx)
{
	ctx->saddr = (unsigned char *)calloc(FILTER_SIZE_BYTES, sizeof(unsigned char));
	ctx->daddr = (unsigned char *)calloc(FILTER_SIZE_BYTES, sizeof(unsigned char));
	ctx->sport = (unsigned char *)calloc(FILTER_SIZE_BYTES, sizeof(unsigned char));
	ctx->dport = (unsigned char *)calloc(FILTER_SIZE_BYTES, sizeof(unsigned char));
}

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

static void
convert_into_char(unsigned char *char_data, unsigned int data)
{
	char_data[0] = (data >> 24) & 0xFF;
	char_data[1] = (data >> 16) & 0xFF;
	char_data[2] = (data >> 8) & 0xFF;
	char_data[3] = (data) & 0xFF;
}

static void
insert_into_bloom_filter_array(unsigned char *filter, unsigned int data)
{
	unsigned char* char_data = (unsigned char* )calloc(4, sizeof(unsigned char));
	convert_into_char(char_data, data);
	unsigned int hash[NUM_HASHES];
	int i;

	get_hashes(hash, char_data);	

	for (i = 0; i < NUM_HASHES; i++) {
		/* xor-fold the hash into FILTER_SIZE bits */
		hash[i] = (hash[i] >> FILTER_SIZE) ^ 
		          (hash[i] & FILTER_BITMASK);
		/* set the bit in the filter */
		filter[hash[i] >> 3] |= 1 << (hash[i] & 7);
	}
}

/**
 * insert data into bloom filter
 * @param ctx [description]
 */
void
insert_into_bloom_filter(bloom_filter_context_t *ctx, FlowMeta *meta)
{
	insert_into_bloom_filter_array(ctx->saddr, meta->flowinfo.saddr);
	insert_into_bloom_filter_array(ctx->daddr, meta->flowinfo.daddr);
	insert_into_bloom_filter_array(ctx->sport, meta->flowinfo.sport);
	insert_into_bloom_filter_array(ctx->dport, meta->flowinfo.dport);
}

int
find_in_filter(unsigned char *filter, unsigned int data)
{
	unsigned char* char_data = (unsigned char* )calloc(4, sizeof(unsigned char));
	convert_into_char(char_data, data);
	unsigned int hash[NUM_HASHES];
	get_hashes(hash, char_data);
	int i;

	for (i = 0; i < NUM_HASHES; i++) {
		hash[i] = (hash[i] >> FILTER_SIZE) ^ 
		          (hash[i] & FILTER_BITMASK);
		if (!(filter[hash[i] >> 3] & (1 << (hash[i] & 7))))
			return 0;
	} 
	return 1;
}

/**
 * search data from bloom filter
 * @param ctx [description]
 */
void
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
		printf("maybe in\n");
	else
		printf("there is no given data\n");
	//TODO implement
}

/**
 * write bloom filter to file system
 * after write operation end, it clean whole bloom filter to reuse
 * @param ctx [description]
 */
void
write_bloom_filter(bloom_filter_context_t *ctx)
{
	//TODO implement
}

/**
 * free bloom filter
 * @param ctx [description]
 */
void
free_bloom_filter(bloom_filter_context_t *ctx)
{
	//TODO implement
}
