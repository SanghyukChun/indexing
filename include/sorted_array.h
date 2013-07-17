typedef struct sorted_array_node
{
	unsigned int file_offset; /* currently unused */
	unsigned int value;
} sorted_array_node_t;

typedef struct sorted_array_context
{
	sorted_array_node_t *head;
	int last_idx;
} sorted_array_context_t;

void init_sorted_array(sorted_array_context_t *ctx);
void insert_into_sorted_array(sorted_array_context_t *ctx, unsigned int data);
void search_from_sorted_array(sorted_array_context_t *ctx, unsigned int data);
void write_sorted_array(sorted_array_context_t *ctx);
void free_sorted_array(sorted_array_context_t *ctx);
