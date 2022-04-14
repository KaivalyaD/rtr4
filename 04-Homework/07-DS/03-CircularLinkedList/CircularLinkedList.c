// headers
#include<stdio.h>
#include<stdlib.h>

// struct definitions
struct node {
	int data;
	struct node *next;
} head;	// if need be to treat the head specially, it helps to not malloc

void init(void)
{
	head.data = 0;
	head.next = &head;
}

void append_node(int data)
{
	// variable declarations
	struct node *node;
	struct node *new;

	// code
	new = (struct node *)malloc(sizeof(struct node));
	new->data = data;

	node = &head;
	while (node->next != &head)
	{
		node = node->next;
	}
	new->next = &head;	// node->next == &head
	node->next = new;
}

void insert_node(int index, int data)
{
	// variable declarations
	struct node *node;
	struct node *new;

	// code
	new = (struct node *)malloc(sizeof(struct node));
	new->data = data;
	
	node = &head;
	while (index > 0)
	{
		node = node->next;
		index--;
	}
	new->next = node->next;
	node->next = new;
}

int remove_node(int index)
{
	// variable declarations
	struct node *node;
	struct node *toRemove;
	int value;

	// code
	node = &head;
	while (index > 0)
	{
		node = node->next;
		index--;
	}

	if (node->next == &head)
		return 0;
	
	toRemove = node->next;
	node->next = toRemove->next;

	value = toRemove->data;
	toRemove->next = NULL;	// its important to complete the story
	free(toRemove);

	return value;
}

void delete_list(void)
{
	// variable declarations
	struct node *node;
	struct node *toDelete;
	int index = 0;

	// code
	node = head.next;
	while (node != &head)
	{
		printf("deleting %d from index %d\n", node->data, index++);

		toDelete = node;
		node = node->next;
		free(toDelete);
	}
}

void display_list(void)
{
	// variable declarations
	struct node *node;
	int index = 0;

	// code
	printf("current list:\n");
	node = head.next;
	while (node != &head)
	{
		printf("\t%d: %d\n", index++, node->data);
		node = node->next;
	}
}

int main(void)
{
	// variable declarations
	int value;

	// code
	printf("\n");

	init();

	append_node(5);
	append_node(10);
	append_node(25);
	append_node(30);
	
	display_list();
	printf("\n");

	insert_node(2, 15);
	printf("inserted %d into index %d\n", 15, 2);
	display_list();
	printf("\n");

	insert_node(3, 20);
	printf("inserted %d into index %d\n", 20, 3);
	display_list();
	printf("\n");

	value = remove_node(3);
	printf("removed %d from index %d\n", value, 3);
	display_list();
	printf("\n");

	value = remove_node(0);
	printf("removed %d from index %d\n", value, 0);
	display_list();
	printf("\n");

	value = remove_node(3);
	printf("removed %d from index %d\n", value, 3);
	display_list();
	printf("\n");

	value = remove_node(2021);
	printf("removed %d from index %d\n", value, 2021);
	display_list();
	printf("\n");
	
	delete_list();
	
	return 0;
}
