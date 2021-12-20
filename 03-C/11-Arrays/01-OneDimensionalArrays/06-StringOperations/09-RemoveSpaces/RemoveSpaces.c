#include <stdio.h>

#define KVD_MAX_STRING_LENGTH 512

int main(void)
{
	// function prototypes
	int MyStrlen(char[]);

	// variable declarations
	char kvd_cArray_Original[KVD_MAX_STRING_LENGTH], kvd_cArray_SpacesRemoved[KVD_MAX_STRING_LENGTH];
	int kvd_i, kvd_j, kvd_iStringLength;

	// code
	printf("\n\n");

	printf("enter a string: ");
	gets_s(kvd_cArray_Original, KVD_MAX_STRING_LENGTH);

	kvd_iStringLength = MyStrlen(kvd_cArray_Original);
	kvd_j = 0;

	for (kvd_i = 0; kvd_i < kvd_iStringLength; kvd_i++)
	{
		if (kvd_cArray_Original[kvd_i] == ' ')
			continue;
		
		kvd_cArray_SpacesRemoved[kvd_j] = kvd_cArray_Original[kvd_i];
		kvd_j++;
	}
	kvd_cArray_SpacesRemoved[kvd_j] = '\0';

	printf("you entered:\n\n");
	printf("\t\"%s\"\n\n", kvd_cArray_Original);

	printf("string with removed spaces:\n\n");
	printf("\t\"%s\"\n\n", kvd_cArray_SpacesRemoved);

	printf("\n\n");
	return(0);
}

int MyStrlen(char kvd_str[])
{
	// variable declarations
	int kvd_string_length = 0;

	while (kvd_str[kvd_string_length])
		kvd_string_length++;

	return(kvd_string_length);
}
