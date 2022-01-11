#include <stdio.h>
#include <stdlib.h>

#define MAX_STRING_LENGTH 512

int main(void)
{
	// function declarations
	char *ReplaceEachVowelWith(char *, char);

	// variable declarations
	char kvd_str[MAX_STRING_LENGTH], *kvd_modified_str = NULL, kvd_replace_by;

	// code
	printf("\n\n");
	printf("enter a string: ");
	gets_s(kvd_str, MAX_STRING_LENGTH);

	printf("you entered:\n\n");
	printf("\t\"%s\"\n\n", kvd_str);

	printf("enter a character: ");
	kvd_replace_by = getchar();
	printf("\n\n");

	kvd_modified_str = ReplaceEachVowelWith(kvd_str, kvd_replace_by);
	if (!kvd_modified_str)
	{
		printf("ReplaceEachVowelWith(): the function has failed\n\n");
		return 1;
	}
	
	printf("if all vowels in the original string were replaced by a '%c', you get:\n\n", kvd_replace_by);
	printf("\t\"%s\"\n\n", kvd_modified_str);

	if (kvd_modified_str)
	{
		free(kvd_modified_str);
		kvd_modified_str = NULL;

		printf("freed and cleaned kvd_modified_str\n\n");
	}

	return 0;
}

char *ReplaceEachVowelWith(char *kvd_str, char kvd_replace_by)
{
	// function prototypes
	void MyStrcpy(char *, char *);
	unsigned int MyStrlen(char *);

	// variable declarations
	char *kvd_modified_string = NULL;
	int kvd_i = 0, kvd_str_length = 0;
	
	// code
	kvd_str_length = MyStrlen(kvd_str);
	
	kvd_modified_string = (char *)malloc(sizeof(char) * kvd_str_length);
	if (!kvd_modified_string)
		return NULL;
	
	MyStrcpy(kvd_modified_string, kvd_str);

	while (*(kvd_modified_string + kvd_i) != '\0')
	{
		switch (*(kvd_modified_string + kvd_i))
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
			*(kvd_modified_string + kvd_i) = kvd_replace_by;
			break;

		default:
			break;
		}
		
		kvd_i++;
	}

	return kvd_modified_string;
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
	unsigned int kvd_length = 0;

	// code
	while (*(kvd_str + kvd_length) != '\0')
		kvd_length++;

	return kvd_length;
}
