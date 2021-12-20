#include <stdio.h>

#define KVD_MAX_STRING_LENGTH 512

int main(void)
{
	// function prototypes
	int MyStrlen(char[]);

	// variable declarations
	char kvd_cArray[KVD_MAX_STRING_LENGTH];
	int kvd_vowel_count[5] = {0, 0, 0, 0, 0};
	int kvd_i, kvd_string_length;

	// code
	printf("\n\n");

	printf("enter a string: ");
	gets_s(kvd_cArray, KVD_MAX_STRING_LENGTH);

	printf("\n\n");

	printf("you entered:\n\n");
	printf("\t\"%s\"", kvd_cArray);

	printf("\n\n");

	kvd_string_length = MyStrlen(kvd_cArray);
	for (kvd_i = 0; kvd_i < kvd_string_length; kvd_i++)
	{
		switch (kvd_cArray[kvd_i])
		{
		case 'A':
		case 'a':
			kvd_vowel_count[0]++;
			break;

		case 'E':
		case 'e':
			kvd_vowel_count[1]++;
			break;

		case 'I':
		case 'i':
			kvd_vowel_count[2]++;
			break;

		case 'O':
		case 'o':
			kvd_vowel_count[3]++;
			break;

		case 'U':
		case 'u':
			kvd_vowel_count[4]++;
			break;

		default:
			break;
		}
	}

	printf("vowel counts:\n\n");
	printf("\tA-s: %d\n", kvd_vowel_count[0]);
	printf("\tE-s: %d\n", kvd_vowel_count[1]);
	printf("\tI-s: %d\n", kvd_vowel_count[2]);
	printf("\tO-s: %d\n", kvd_vowel_count[3]);
	printf("\tU-s: %d\n", kvd_vowel_count[4]);

	printf("\n\n");
	return(0);
}

int MyStrlen(char kvd_str[])
{
	// variable declarations
	int kvd_string_length = 0;

	while (kvd_str[kvd_string_length] != '\0')
		kvd_string_length++;

	return(kvd_string_length);
}
