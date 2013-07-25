Indexing
========
This project is sample indexing architechure for flosis. Repository is https://github.com/SanghyukChun/indexing
Index has two layers: bloom filter and linked list with quick sort

After make, you can get binary named 'index'
default index binary need argument which for size of index array
./index 1000
will create index with size 1000

## main.c
main.c is example code which use index array. it inserts meta data which is created by rand()
main.c runs until array is fully filled and clean 5 times

insert example:
```
// you should assign ctx and bctx first

// init index array with ctx, bctx, and size of array
// bctx is assigned to ctx->bctx, so you do not need to use it directly any more
init_index_array(ctx, bctx, size);

// ... process flow meta

// meta is FlowMeta object
// insert meta into index array
insert_into_index_array(ctx, meta);
```

search example:
```
// you should clearfy which attribute do you look for
// attribute is defined as enum (TYPE_SADDR, TYPE_DADDR, TYPE_SPORT, TYPE_DPORT)
// data is unsigned int value which you want to search in the index array
search_from_index_array(ctx, TYPE_SADDR, data);
```

## bloom_filter.c
bloom_filter.c has init, insert, search, clean and free functions.
Before use bloom filter, you should initialize bloom_filter_context_t *bctx. number of bit in bloom filter is defined in FILTER_SIZE as macro in bloom_filter.c. therefore, if you want to change number of bit in bloom filter, you should change it manually.
When you use insert_into_bloom_filter, you just do not worry about anything except create meta data. It require only context and meta data. It use 7 hashes from http://www.partow.net/programming/hashfunctions/ if you want to change hashes, you can edit it in hash.c before you edit or change hashes, you should re-define NUM_HASHES to number of hashes from 7. get_hashes() in bloom_filter.c also changed. unsigned int array hash has NUM_HASH slots which individually means result of hash functions. Since insert function is void function, you do not worry about return value.
search_from_bloom_filter is basically same as sarch_from_index_array. ctx, type, and data is required. You will get 1 if bloom filter detect input data while returns 0 when there is no input value

change hash function example
```
#define HASH_NUM 4

// ...

// you can define your hash functions in hashed.c
// you do not need to change nothing except this function in bloom_filter.c
static void
get_hashes(unsigned int hash[], unsigned char *data)
{
	hash[0] = hash_0 (data);
	hash[1] = hash_1 (data);
	hash[2] = hash_2 (data);
	hash[3] = hash_3 (data);
}
```

## index.c
index.c has init, insert, search, clean and free functions.
Before use index.c you should initialize index_array_context_t *ctx. If you do not need to set index array size by argument, define it as macro.
insert returns 1 if array is fully filled after insert given input. If array is fully filled, you should sort array, write it in SSD and clean array. Now clean_array is deleted from insert function. Array is sorted using quick sort. It is implemented by qsort.h (http://www.corpit.ru/mjt/qsort/qsort.h)
Seach function first look up bloom filter first and search data using binary search if it may be has data. I returns start and end index of given data because array may can have same index.






## Remaing Works
- Front client and server
