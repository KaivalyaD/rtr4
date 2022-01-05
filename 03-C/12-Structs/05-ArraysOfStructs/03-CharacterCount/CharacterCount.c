#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAX_STRING_LENGTH 1024

struct CharacterCount
{
	char c;
	int count;
} kvd_character_count[] = {
	{'A', 0},
	{'B', 0},
	{'C', 0},
	{'D', 0},
	{'E', 0},
	{'F', 0},
	{'G', 0},
	{'H', 0},
	{'I', 0},
	{'J', 0},
	{'K', 0},
	{'L', 0},
	{'M', 0},
	{'N', 0},
	{'O', 0},
	{'P', 0},
	{'Q', 0},
	{'R', 0},
	{'S', 0},
	{'T', 0},
	{'U', 0},
	{'V', 0},
	{'W', 0},
	{'X', 0},
	{'Y', 0},
	{'Z', 0} };

#define STRUCT_ARRAY_SIZE sizeof(kvd_character_count)
#define SINGULAR_STRUCT_ELEMENTAL_SIZE sizeof(kvd_character_count[0])
#define ARRAY_ELEMENT_COUNT ((STRUCT_ARRAY_SIZE) / (SINGULAR_STRUCT_ELEMENTAL_SIZE))

int main(void)
{
	// variable declarations
	char kvd_str[MAX_STRING_LENGTH];
	int kvd_i, kvd_j, kvd_actual_string_length = 0;

	// code
	printf("\n\n");

	printf("enter a string: ");
	gets_s(kvd_str, MAX_STRING_LENGTH);

	kvd_actual_string_length = strlen(kvd_str);

	printf("\n\n");

	printf("you entered:\n\n");
	printf("\t\"%s\"", kvd_str);

	printf("\n\n");
	for (kvd_i = 0; kvd_i < kvd_actual_string_length; kvd_i++)
	{
		for (kvd_j = 0; kvd_j < ARRAY_ELEMENT_COUNT; kvd_j++)
		{
			kvd_str[kvd_i] = toupper(kvd_str[kvd_i]);
			if (kvd_str[kvd_i] == kvd_character_count[kvd_j].c)
				kvd_character_count[kvd_j].count++;
		}
	}

	printf("\n\n");
	printf("the number of times all characters from the alphabet occurred:\n\n");
	for (kvd_i = 0; kvd_i < ARRAY_ELEMENT_COUNT; kvd_i++)
		printf("\t'%c': %d\n", kvd_character_count[kvd_i].c, kvd_character_count[kvd_i].count);
	
	printf("\n\n");
	return(0);
}
