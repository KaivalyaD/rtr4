#include <stdio.h>

#define KVD_MAX_STRING_LENGTH 512

int main(void)
{
	// function prototypes
	int MyStrlen(char[]);
	void MyStrcpy(char[], char[]);

	// variable declarations
	char kvd_cArray[KVD_MAX_STRING_LENGTH];
	int kvd_iStringLength = 0, kvd_word_count = 0, kvd_space_count = 0, kvd_i;
	int kvd_fWordBegins = 1;

	// code
	printf("\n\n");

	printf("enter a string: ");
	gets_s(kvd_cArray, KVD_MAX_STRING_LENGTH);

	kvd_iStringLength = MyStrlen(kvd_cArray);

	for (kvd_i = 0; kvd_i < kvd_iStringLength; kvd_i++)
	{
		if (kvd_cArray[kvd_i] == '.' || kvd_cArray[kvd_i] == ' ' || kvd_cArray[kvd_i] == ',' || kvd_cArray[kvd_i] == '?' || kvd_cArray[kvd_i] == '!')
		{
			if(kvd_cArray[kvd_i] == ' ')
				kvd_space_count++;

			kvd_fWordBegins = 1;
		}
		else
		{
			if (kvd_fWordBegins)
			{
				kvd_word_count++;
				kvd_fWordBegins = 0;
			}
		}
	}

	printf("\n\n");

	printf("you entered:\n\n");
	printf("\t\"%s\"\n\n", kvd_cArray);

	printf("breakdown:\n\n");
	printf("\t1. Space Count = %d\n", kvd_space_count);
	printf("\t2. Word Count  = %d\n", kvd_word_count);

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
	int kvd_string_length = 0;

	while (kvd_str[kvd_string_length])
		kvd_string_length++;

	return(kvd_string_length);
}
