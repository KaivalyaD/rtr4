#include <stdio.h>

#define KVD_MAX_STRING_LENGTH 512

int main(void)
{
	// function prototypes
	void MyStrcpy(char[], char[]);

	// variable declarations
	char kvd_cArray[KVD_MAX_STRING_LENGTH], kvd_cArray_copy[KVD_MAX_STRING_LENGTH];

	// code
	printf("\n\n");

	/* prompt user for a string entry */
	printf("enter a string: ");
	gets_s(kvd_cArray, KVD_MAX_STRING_LENGTH);

	// copy
	MyStrcpy(kvd_cArray_copy, kvd_cArray);

	// display
	printf("\n\n");

	printf("original string you entered:\n\n");
	printf("\t\"%s\"\n\n", kvd_cArray);

	printf("copied string:\n\n");
	printf("\t\"%s\"\n", kvd_cArray_copy);

	printf("\n\n");
	return(0);
}

void MyStrcpy(char kvd_str_destination[], char kvd_str_source[])
{
	// function prototypes
	int MyStrlen(char[]);

	// variable declarations
	int kvd_i, kvd_source_length = 0;

	// code
	kvd_source_length = MyStrlen(kvd_str_source);

	for (kvd_i = 0; kvd_i < kvd_source_length; kvd_i++)
		kvd_str_destination[kvd_i] = kvd_str_source[kvd_i];

	kvd_str_destination[kvd_i] = '\0';
}

int MyStrlen(char kvd_str[])
{
	// variable declarations
	int kvd_string_length = 0, kvd_i = 0;

	// code
	while (kvd_str[kvd_i] != '\0')
		kvd_i++;

	kvd_string_length = kvd_i;
	return(kvd_string_length);
}
