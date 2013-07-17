typedef struct avl_tree_node
{
	unsigned int file_offset; /* currently unused */
	unsigned int value;
	struct avl_tree_node *left;
	struct avl_tree_node *right;
} avl_tree_node_t;

typedef struct avl_tree_context
{
	avl_tree_node_t* root;
	int last_node;
} avl_tree_context_t;

void init_avl_tree(avl_tree_context_t *ctx);
void insert_into_avl_tree(avl_tree_context_t *ctx, unsigned int data);
avl_tree_node_t* search_from_avl_tree(avl_tree_context_t *ctx, unsigned int data);
void write_avl_tree(avl_tree_context_t *ctx);
void free_avl_tree(avl_tree_context_t *ctx);
