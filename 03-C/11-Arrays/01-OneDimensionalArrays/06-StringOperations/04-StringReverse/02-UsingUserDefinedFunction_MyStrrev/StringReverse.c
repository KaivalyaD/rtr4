#include <stdio.h>

#define KVD_MAX_STRING_LENGTH 512

int main(void)
{
	// function prototypes
	int MyStrrev(char[], char[]);

	// variable declarations
	char kvd_cArray[KVD_MAX_STRING_LENGTH], kvd_cArray_reversed[KVD_MAX_STRING_LENGTH];

	// code
	printf("\n\n");

	printf("enter a string: ");
	gets_s(kvd_cArray, KVD_MAX_STRING_LENGTH);

	printf("\n\n");

	printf("you entered:\n\n");
	printf("\t\"%s\"", kvd_cArray);

	printf("\n\n");
	
	MyStrrev(kvd_cArray_reversed, kvd_cArray);
	printf("reversed string:\n\n");
	printf("\t\"%s\"", kvd_cArray_reversed);

	printf("\n\n");
	return(0);
}

int MyStrrev(char kvd_str_destination[], char kvd_str_source[])
{
	// function prototypes
	int MyStrlen(char[]);

	// variable declarations
	int kvd_i, kvd_source_length, kvd_destination_length;

	// code
	kvd_source_length = MyStrlen(kvd_str_source);
	
	for (kvd_i = 0; kvd_i < kvd_source_length; kvd_i++)
		kvd_str_destination[kvd_i] = kvd_str_source[kvd_source_length - 1 - kvd_i];

	kvd_str_destination[kvd_i] = '\0';

	return(kvd_i);
}

int MyStrlen(char kvd_str[])
{
	// variable declarations
	int kvd_string_length = 0;

	// code
	while (kvd_str[kvd_string_length] != '\0')
		kvd_string_length++;

	return(kvd_string_length);
}
