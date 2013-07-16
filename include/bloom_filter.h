typedef struct bloom_filter_context
{
	unsigned int file_offset; /* currently unused */
} bloom_filter_context_t;

void init_bloom_filter(bloom_filter_context_t *ctx);
void insert_into_bloom_filter(bloom_filter_context_t *ctx, unsigned int data);
void search_from_bloom_filter(bloom_filter_context_t *ctx, unsigned int data);
void write_bloom_filter(bloom_filter_context_t *ctx);
void free_bloom_filter(bloom_filter_context_t *ctx);
