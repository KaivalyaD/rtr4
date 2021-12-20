#include <stdio.h>

#define KVD_MAX_STRING_LENGTH 512

int main(void)
{
	// function prototypes
	void MyStrcat(char[], char[]);

	// variable declarations
	char kvd_cArray_One[KVD_MAX_STRING_LENGTH], kvd_cArray_Two[KVD_MAX_STRING_LENGTH];

	// code
	printf("\n\n");

	printf("enter one string: ");
	gets_s(kvd_cArray_One, KVD_MAX_STRING_LENGTH);

	printf("enter another string: ");
	gets_s(kvd_cArray_Two, KVD_MAX_STRING_LENGTH);

	printf("\n\n");

	printf("strings you entered:\n\n");

	printf("\tstring 1:\n\n");
	printf("\t\t\"%s\"\n\n", kvd_cArray_One);

	printf("\tstring 2:\n\n");
	printf("\t\t\"%s\"\n\n", kvd_cArray_Two);

	MyStrcat(kvd_cArray_One, kvd_cArray_Two);

	printf("concatinated string:\n\n");
	printf("\t\"%s\"\n", kvd_cArray_One);

	printf("\n\n");
	return(0);
}

void MyStrcat(char kvd_str_destination[], char kvd_str_source[])
{
	// function prototypes
	int MyStrlen(char[]);

	// variable declarations
	int kvd_i, kvd_destination_length, kvd_source_length;

	// code
	kvd_destination_length = MyStrlen(kvd_str_destination);
	kvd_source_length = MyStrlen(kvd_str_source);

	for (kvd_i = 0; kvd_i < kvd_source_length; kvd_i++)
		kvd_str_destination[kvd_destination_length + kvd_i] = kvd_str_source[kvd_i];

	kvd_str_destination[kvd_source_length + kvd_destination_length] = '\0';
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
