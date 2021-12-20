#include <stdio.h>

#define KVD_MAX_STRING_LENGTH 512

int main(void)
{
	// function prototypes
	int MyStrlen(char[]);
	void MyStrcpy(char[], char[]);

	// variable declarations
	char kvd_cArray_Original[KVD_MAX_STRING_LENGTH], kvd_cArray_VowelsReplaced[KVD_MAX_STRING_LENGTH];
	int kvd_iStringLength, kvd_i;

	// code
	printf("\n\n");

	printf("enter a string: ");
	gets_s(kvd_cArray_Original, KVD_MAX_STRING_LENGTH);

	kvd_iStringLength = MyStrlen(kvd_cArray_Original);
	MyStrcpy(kvd_cArray_VowelsReplaced, kvd_cArray_Original);

	for (kvd_i = 0; kvd_i < kvd_iStringLength; kvd_i++)
	{
		switch (kvd_cArray_Original[kvd_i])
		{
		case 'A':
		case 'a':
		case 'E':
		case 'e':
		case 'I':
		case 'i':
		case 'O':
		case 'o':
		case 'U':
		case 'u':
			kvd_cArray_VowelsReplaced[kvd_i] = '*';
			break;

		default:
			break;
		}
	}

	printf("\n\n");

	printf("original string:\n\n");
	printf("\t\"%s\"\n\n", kvd_cArray_Original);

	printf("string with vowels replaced by an asterisk:\n\n");
	printf("\t\"%s\"\n", kvd_cArray_VowelsReplaced);

	printf("\n\n");
	return(0);
}

void MyStrcpy(char kvd_str_destination[], char kvd_str_source[])
{
	// function prototypes
	int MyStrlen(char[]);

	// variable declarations
	int kvd_i, kvd_source_length;

	// code
	kvd_source_length = MyStrlen(kvd_str_source);
	for (kvd_i = 0; kvd_i < kvd_source_length; kvd_i++)
		kvd_str_destination[kvd_i] = kvd_str_source[kvd_i];

	kvd_str_destination[kvd_i] = '\0';
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

