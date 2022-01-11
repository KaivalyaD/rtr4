#include <stdio.h>
#include <stdlib.h>

#define MAX_STRING_LENGTH 512

int main(void)
{
	// function prototypes
	void MyStrcat(char *, char *);
	unsigned int MyStrlen(char *);

	// variable declarations
	char *kvd_cArray_1 = NULL, *kvd_cArray_2 = NULL;

	// code
	printf("\n\n");

	kvd_cArray_1 = (char *)malloc(sizeof(char) * MAX_STRING_LENGTH);
	if (!kvd_cArray_1)
	{
		printf("malloc(): failed to allocate memory for %d characters\n\n", MAX_STRING_LENGTH);
		return 1;
	}
	printf("memory for the first string allocated successfully\n\n");
	kvd_cArray_2 = (char *)malloc(sizeof(char) * MAX_STRING_LENGTH);
	if (!kvd_cArray_2)
	{
		printf("malloc(): failed to allocate memory for %d characters\n\n", MAX_STRING_LENGTH);
		return 1;
	}
	printf("memory for the second string allocated successfully\n\n");

	printf("enter a string: ");
	gets_s(kvd_cArray_1, MAX_STRING_LENGTH);
	printf("enter another string: ");
	gets_s(kvd_cArray_2, MAX_STRING_LENGTH);
	
	printf("\n\n");
	printf("you entered:\n\n");
	printf("\t\"%s\"; and\n", kvd_cArray_1);
	printf("\t\"%s\"\n\n", kvd_cArray_2);

	MyStrcat(kvd_cArray_1, kvd_cArray_2);

	printf("kvd_cArray_1 + kvd_cArray_2:\n\n");
	printf("\t\"%s\"\n\n", kvd_cArray_1);

	if (kvd_cArray_2)
	{
		free(kvd_cArray_2);
		kvd_cArray_2 = NULL;

		printf("freed and cleaned kvd_cArray_2\n");
	}
	if (kvd_cArray_1)
	{
		free(kvd_cArray_1);
		kvd_cArray_1 = NULL;

		printf("freed and cleaned kvd_cArray_1\n\n");
	}

	return 0;
}

void MyStrcat(char *kvd_str_1, char *kvd_str_2)
{
	// variable declarations
	unsigned int kvd_i = 0, kvd_str_1_length = 0;

	kvd_str_1_length = MyStrlen(kvd_str_1);

	// code
	while (*(kvd_str_2 + kvd_i) != '\0' && kvd_i < MAX_STRING_LENGTH)
	{
		*(kvd_str_1 + kvd_str_1_length + kvd_i) = *(kvd_str_2 + kvd_i);
		kvd_i++;
	}

	*(kvd_str_1 + kvd_str_1_length + MyStrlen(kvd_str_2)) = '\0';
}

unsigned int MyStrlen(char *kvd_str)
{
	// variable declarations
	int kvd_string_length = 0;

	// code
	while (*(kvd_str + kvd_string_length) != '\0')
		kvd_string_length++;

	return kvd_string_length;
}
