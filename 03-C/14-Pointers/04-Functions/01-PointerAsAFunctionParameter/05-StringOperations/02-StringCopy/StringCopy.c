#include <stdio.h>
#include <stdlib.h>

#define MAX_STRING_LENGTH 512

int main(void)
{
	// function prototypes
	void MyStrcpy(char *, char *);
	unsigned int MyStrlen(char *);

	// variable declarations
	char *kvd_cArray_original = NULL, *kvd_cArray_copy = NULL;
	int kvd_original_string_length = 0;

	// code
	printf("\n\n");

	kvd_cArray_original = (char *)malloc(sizeof(char) * MAX_STRING_LENGTH);
	if (!kvd_cArray_original)
	{
		printf("malloc(): failed to allocate memory for %d characters\n\n", MAX_STRING_LENGTH);
		return 1;
	}
	printf("memory for the original string allocated successfully\n\n");

	printf("enter a string: ");
	gets_s(kvd_cArray_original, MAX_STRING_LENGTH);
	printf("\n\n");

	kvd_original_string_length = MyStrlen(kvd_cArray_original);
	kvd_cArray_copy = (char *)malloc(sizeof(char) * kvd_original_string_length);
	if (!kvd_cArray_copy)
	{
		printf("malloc(): failed to allocate memory for %d characters\n\n", kvd_original_string_length);
		return 1;
	}
	printf("memory for the copy string allocated successfully (%d characters)\n\n", kvd_original_string_length);

	printf("you entered:\n\n");
	printf("\t\"%s\"\n\n", kvd_cArray_original);

	MyStrcpy(kvd_cArray_copy, kvd_cArray_original);

	printf("copied kvd_cArray_original into kvd_cArray_copy\n\n");

	printf("kvd_cArray_copy now contains:\n\n");
	printf("\t\"%s\"\n\n", kvd_cArray_copy);

	if (kvd_cArray_copy)
	{
		free(kvd_cArray_copy);
		kvd_cArray_copy = NULL;

		printf("freed and cleaned kvd_cArray_copy\n");
	}
	if (kvd_cArray_original)
	{
		free(kvd_cArray_original);
		kvd_cArray_original = NULL;

		printf("freed and cleaned kvd_cArray_original\n\n");
	}

	return 0;
}

void MyStrcpy(char *kvd_destination, char *kvd_source)
{
	// variable declarations
	int kvd_i = 0;

	// code
	while (*(kvd_source + kvd_i) != '\0')
	{
		*(kvd_destination + kvd_i) = *(kvd_source + kvd_i);
		kvd_i++;
	}

	*(kvd_destination + kvd_i) = '\0';
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
