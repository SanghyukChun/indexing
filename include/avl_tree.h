typedef struct tree_node
{
	unsigned int file_offset; /* currently unused */
	unsigned int value;
	struct tree_node *parent;
	struct tree_node *left_child;
	struct tree_node *right_child;
} tree_node_t;

typedef struct avl_tree_context
{

} avl_tree_context_t;

void init_avl_tree(avl_tree_context_t *ctx);
void insert_into_avl_tree(avl_tree_context_t *ctx, unsigned int data);
void search_from_avl_tree(avl_tree_context_t *ctx, unsigned int data);
void write_avl_tree(avl_tree_context_t *ctx);
void free_avl_tree(avl_tree_context_t *ctx);
