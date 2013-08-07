#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/types.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#define NODE_NUM 100
#define LIST_NUM 10
#define NODE_PER_LIST NODE_NUM / LIST_NUM

typedef struct node {
	unsigned int value;
} node_t;

typedef struct test {
	node_t *head;
} test_t;

typedef struct container {
	test_t *t;
	int cnt;
} c_t;

int main(void)
{
	c_t c;
	memset(&c, 0, sizeof(c_t));
	node_t *head = (node_t *)calloc(NODE_NUM, sizeof(node_t));
	test_t *test_list = (test_t *)calloc(LIST_NUM, sizeof(test_t));

	c_t *container = &c;
	test_list->head = head;
	container->t = test_list;
	container->cnt = 0;

	int i,j;
	for (j=0; j<LIST_NUM; j++)
	{
		node_t *tmp = &container->t->head[container->cnt];
		
		for (i=0; i<NODE_PER_LIST; i++)
		{
			tmp[i].value = j;
			container->cnt += 1;
		}
	}
	/*
	for (j=0; j<LIST_NUM; j++)
	{
		node_t *tmp = &head[j * NODE_PER_LIST];
		
		for (i=0; i<NODE_PER_LIST; i++)
		{
			tmp[i].value = j;

		}
	}
	*/

	for (i=0;i<NODE_NUM;i++)
	{
		printf("%u", head[i].value);
	}
}
