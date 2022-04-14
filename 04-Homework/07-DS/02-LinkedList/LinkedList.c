#include <stdio.h>
#include <stdlib.h>

// struct for node
struct node {
	int data;
	struct node *next;
} head;

void initList(void)
{
	// code
	head.data = 0;
	head.next = NULL;
}

void append(int data)
{
	// variable declarations
	struct node *end;
	struct node *new;

	// code
	new = (struct node *)malloc(sizeof(struct node));
	new->data = data;
	new->next = NULL;

	for (end = &head; end->next != NULL; end = end->next);

	end->next = new;
}

void insert(int data, int index)
{
	// variable declaration
	struct node *current;
	struct node *new;
	int i = 0;

	// code
	new = (struct node *)malloc(sizeof(struct node));
	new->data = data;
	
	for (current = &head; current->next != NULL; current = current->next)
	{
		if (i == index)
			break;
		i++;
	}

	new->next = current->next;
	current->next = new;
}

int remove_kvd(int index)
{
	// variable declarations
	struct node *current;
	struct node *toDelete;
	int i = 0, data;

	// code
	if (head.next == NULL)
		return -1;

	current = &head;
	for (toDelete = head.next; toDelete->next != NULL; toDelete = toDelete->next, current = current->next)
	{
		if (i == index)
			break;
		i++;
	}
	
	data = toDelete->data;
	current->next = toDelete->next;

	free(toDelete);
	toDelete = NULL;

	return data;
}

void display(void)
{
	// variable declarations
	struct node *current;
	int index = 0;

	// code
	for (current = head.next; current != NULL; current = current->next)
	{
		printf("at index = %d, data = %d\n", index, current->data);
		index++;
	}

	printf("list ends here\n");
}

void delete(void)
{
	// variable declarations
	struct node *prev;
	struct node *current;

	// code
	for (current = head.next; current != NULL;)
	{
		prev = current;
		current = current->next;
		
		free(prev);
		prev = NULL;
	}

	head.next = NULL;
}

int main(int argc, char *argv[], char *envp[])
{
	// variable declarations
	int ch = 0;
	int data, index;

	// code
	initList();
	
	while (ch != 5)
	{
		printf("1. Append\n2. Insert\n3. Remove\n4. Display\n5. End\nEnter Operation Code: ");
		scanf("%d", &ch);

		switch (ch)
		{
		case 1:
			printf("data: ");
			scanf("%d", &data);
			append(data);
			printf("inserted data %d\n\n", data);
			break;

		case 2:
			printf("data: ");
			scanf("%d", &data);
			printf("index: ");
			scanf("%d", &index);
			insert(data, index);
			printf("inserted %d at index %d\n\n", data, index);
			break;

		case 3:
			printf("index: ");
			scanf("%d", &index);
			data = remove_kvd(index);
			printf("removed %d from index %d\n\n", data, index);
			break;

		case 4:
			printf("current list:\n");
			display();
			printf("\n");
			break;

		default:
			if (ch != 5)
				printf("invalid choice\n\n");
			break;
		}
	}

	delete();
	display();
	
	return 0;
}
