#include <stdio.h>
#include <stdlib.h>

#include "avl_tree.h"
#include "util.h"

#define MAX(a,b) (a > b) ? a : b
#define TREE_SIZE 32768

/**
 * init context for avl tree index
 * @param ctx [description]
 */
void
init_avl_tree(avl_tree_context_t *ctx)
{
	//TODO implement
	LOG_MESSAGE("=== open init avl tree");
	avl_tree_node_t *p = (avl_tree_node_t *)calloc(TREE_SIZE, sizeof(avl_tree_node_t));

	if (p == NULL)
	{
		fprintf(stderr, "fail to initialize avl tree nodes\n");
		exit(-1);
	}

	ctx->root = p;
	ctx->last_node = 0;

	LOG_MESSAGE("=== close init avl tree");
}

void
print_tree_loop(avl_tree_node_t *node)
{
	if (node == NULL)
		printf("NULL\t");
	else 
	{
		print_tree_loop(node->left);
		print_tree_loop(node->right);
		printf("%d\t", node->value);
	}
}

void
print_tree(avl_tree_context_t *ctx)
{
	print_tree_loop(ctx->root);
	printf("\n");
}

/**
 * get height of given node
 * @param  node [description]
 * @return      [description]
 */
int
get_height(avl_tree_node_t *node)
{
	int height = 0;
	if (node != NULL) {
		int left_height = get_height(node->left);
		int right_height = get_height(node->right);
		int max = MAX(left_height, right_height);
		height = 1 + max;
	}
	return height;
}

/**
 * get diff between left node height and right node height of given node
 * @param  node [description]
 * @return      [description]
 */
int
get_diff(avl_tree_node_t *node)
{
	int left_height = get_height(node->left);
	int right_height = get_height(node->right);
	int diff = left_height - right_height;
	return diff;
}

/**
 * when diff of p is -2 right subtree overweights the left one of given node
 * if diff of right child is -1, single left rotation is required
 * @param  p [description]
 * @return   [description]
 */
avl_tree_node_t*
rotate_rr(avl_tree_node_t *p)
{
	avl_tree_node_t *node;
	node = p->right;
	p->right = node->left;
	node->left = p;
	return node;
}

/**
 * when diff of p is +2 left subtree overweights the left one of given node
 * if diff of left child is 1, single right rotation is required
 * @param  p [description]
 * @return   [description]
 */
avl_tree_node_t*
rotate_ll(avl_tree_node_t *p)
{
	avl_tree_node_t *node;
	node = p->left;
	p->left = node->right;
	node->right = p;
	return node;
}

/**
 * when diff of p is -2 right subtree overweights the left one of given node
 * if diff of right child is 1, one right and another left rotation is required
 * @param  p [description]
 * @return   [description]
 */
avl_tree_node_t*
rotate_rl(avl_tree_node_t *p)
{
	avl_tree_node_t *node;
	node = p->right;
	p->right = rotate_ll(node);
	return rotate_rr(p);
}

/**
 * when diff of p is +2 left subtree overweights the left one of given node
 * if diff of left child is -1, one left and another right rotation is required
 * @param  p [description]
 * @return   [description]
 */
avl_tree_node_t*
rotate_lr(avl_tree_node_t *p)
{
	avl_tree_node_t *node;
	node = p->left;
	p->left = rotate_rr(node);
	return rotate_ll(p);
}

/**
 * balance tree by rotation
 * @param  node [description]
 * @return      [description]
 */
avl_tree_node_t*
balancing(avl_tree_node_t *node)
{
	int diff = get_diff(node);
	
	if (diff > 1) 
	{
		if (get_diff(node->left) > 0)
			node = rotate_ll(node);
		else
			node = rotate_lr(node);
	}
	else if (diff < -1)
	{
		if (get_diff(node->right) > 0)
			node = rotate_rl(node);
		else
			node = rotate_rr(node);
	}

	return node;
}

/**
 * insert main loop
 * @param p    [description]
 * @param node [description]
 */
avl_tree_node_t*
insert(avl_tree_node_t *p, avl_tree_node_t *node)
{
	if (p == NULL)
	{
		p = node;
		node->left = NULL;
		node->right = NULL;
		return p;
	}
	else if (node->value < p->value) 
	{
		p->left = insert(p->left, node);
		p = balancing(p);
	}
	else 
	{
		p->right = insert(p->right, node);
		p = balancing(p);
	}

	return p;
}

/**
 * insert data into avl tree
 * @param ctx  [description]
 * @param data [description]
 */
void
insert_into_avl_tree(avl_tree_context_t *ctx, unsigned int data)
{
	LOG_MESSAGE("=== open insert avl tree");
	if (ctx->last_node == 0)
	{
		ctx->root->value = data;
		ctx->last_node++;
		return;
	}

	if (ctx->last_node >= TREE_SIZE - 1)
	{
		//TODO dump and clean
		return;
	}

	avl_tree_node_t *node = &ctx->root[ctx->last_node];
	node->value = data;

	insert(ctx->root, node);
	print_tree(ctx);
	printf("root %d\n", ctx->root->value);
	LOG_MESSAGE("=== close insert avl tree");
}

/**
 * main search loop
 * @param  p    [description]
 * @param  data [description]
 * @return      [description]
 */
avl_tree_node_t*
search(avl_tree_node_t *p, unsigned int data)
{
	if (p == NULL)
		return NULL;

	if (data == p->value)
		return p;

	if (data < p->value)
		return search(p->left, data);

	if (data > p->value)
		return search(p->right, data);
}

/**
 * search data from avl tree
 * @param ctx  [description]
 * @param data [description]
 * @return      [description]
 */
avl_tree_node_t*
search_from_avl_tree(avl_tree_context_t *ctx, unsigned int data)
{
	return search(ctx->root, data);
}

/**
 * write avl tree to file system
 * after write operation end, it clean whole tree to reuse
 * @param ctx [description]
 */
void
write_avl_tree(avl_tree_context_t *ctx)
{
	//TODO implement
}

/**
 * free avl tree
 * @param ctx [description]
 */
void
free_avl_tree(avl_tree_context_t *ctx)
{
	free(ctx->root);
	free(ctx);
}
