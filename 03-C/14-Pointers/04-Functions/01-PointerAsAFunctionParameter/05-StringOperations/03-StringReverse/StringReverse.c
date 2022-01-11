#include <stdio.h>
#include <stdlib.h>

#define MAX_STRING_LENGTH 512

int main(void)
{
	// function prototypes
	void MyStrrev(char *, char *);
	unsigned int MyStrlen(char *);

	// variable declarations
	char *kvd_cArray_original = NULL, *kvd_cArray_rev = NULL;
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
	kvd_cArray_rev = (char *)malloc(sizeof(char) * kvd_original_string_length);
	if (!kvd_cArray_rev)
	{
		printf("malloc(): failed to allocate memory for %d characters\n\n", kvd_original_string_length);
		return 1;
	}
	printf("memory for the reversed string allocated successfully (%d characters)\n\n", kvd_original_string_length);

	printf("you entered:\n\n");
	printf("\t\"%s\"\n\n", kvd_cArray_original);

	MyStrrev(kvd_cArray_rev, kvd_cArray_original);

	printf("reversed kvd_cArray_original:\n\n");
	printf("\t\"%s\"\n\n", kvd_cArray_rev);

	if (kvd_cArray_rev)
	{
		free(kvd_cArray_rev);
		kvd_cArray_rev = NULL;

		printf("freed and cleaned kvd_cArray_rev\n");
	}
	if (kvd_cArray_original)
	{
		free(kvd_cArray_original);
		kvd_cArray_original = NULL;

		printf("freed and cleaned kvd_cArray_original\n\n");
	}

	return 0;
}

void MyStrrev(char *kvd_destination, char *kvd_source)
{
	// variable declarations
	int kvd_i = 0, kvd_source_length = 0;

	kvd_source_length = MyStrlen(kvd_source);

	// code
	while (*(kvd_source + kvd_i) != '\0')
	{
		*(kvd_destination + kvd_i) = *(kvd_source + kvd_source_length - kvd_i - 1);
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
