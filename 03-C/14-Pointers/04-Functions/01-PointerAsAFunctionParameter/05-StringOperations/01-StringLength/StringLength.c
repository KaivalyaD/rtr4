#include <stdio.h>
#include <stdlib.h>

#define MAX_STRING_LENGTH 512

int main(void)
{
	// function prototypes
	int MyStrlen(char *);

	// variable declarations
	char *kvd_cArray = NULL;
	unsigned int kvd_string_length = 0;

	// code
	printf("\n\n");
	kvd_cArray = (char *)malloc(sizeof(char) * MAX_STRING_LENGTH);
	if (!kvd_cArray)
	{
		printf("malloc(): failed to allocate storage for %d characters\n", MAX_STRING_LENGTH);
		return 1;
	}
	printf("memory allocated successfully\n\n");

	printf("enter a string: ");
	gets_s(kvd_cArray, MAX_STRING_LENGTH);

	printf("\n\n");

	printf("you entered:\n\n");
	printf("\t\"%s\"\n\n", kvd_cArray);

	kvd_string_length = MyStrlen(kvd_cArray);
	printf("length of the string = %d\n\n", kvd_string_length);

	if (kvd_cArray)
	{
		free(kvd_cArray);
		kvd_cArray = NULL;

		printf("freed and cleaned all dynamically allocated memory\n\n");
	}

	return 0;
}

int MyStrlen(char *kvd_str)
{
	// variable declarations
	int kvd_length = 0;

	while (*(kvd_str + kvd_length) != '\0')
		kvd_length++;

	return kvd_length;
}
