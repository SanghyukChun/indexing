#include <stdio.h>
#include <stdlib.h>
#include "bloom_filter.h"

/**
 * init context for bloom filter
 * @param ctx [description]
 */
void init_bloom_filter(bloom_filter_context_t *ctx)
{
	//TODO implement
}

/**
 * insert data into bloom filter
 * @param ctx [description]
 */
void insert_into_bloom_filter(bloom_filter_context_t *ctx, unsigned int data)
{
	//TODO implement
}

/**
 * search data from bloom filter
 * @param ctx [description]
 */
void search_from_bloom_filter(bloom_filter_context_t *ctx, unsigned int data)
{
	//TODO implement
}

/**
 * write bloom filter to file system
 * after write operation end, it clean whole bloom filter to reuse
 * @param ctx [description]
 */
void write_bloom_filter(bloom_filter_context_t *ctx)
{
	//TODO implement
}

/**
 * free bloom filter
 * @param ctx [description]
 */
void free_bloom_filter(bloom_filter_context_t *ctx)
{
	//TODO implement
}
