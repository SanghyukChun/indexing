typedef struct array_node
{
	unsigned int file_offset; /* currently unused */
	unsigned int value;
	struct array_node *prev;
	struct array_node *next;
} array_node_t;

typedef struct sorted_array_context
{
	
} sorted_array_context_t;

void init_sorted_array(sorted_array_context_t *ctx);
void insert_into_sorted_array(sorted_array_context_t *ctx, unsigned int data);
void search_from_sorted_array(sorted_array_context_t *ctx, unsigned int data);
void write_sorted_array(sorted_array_context_t *ctx);
void free_sorted_array(sorted_array_context_t *ctx);