#include <stdio.h>

#define MAX_STRING_LENGTH 512

int main(void)
{
	// function prototypes
	int MyStrlen(char[]);

	// variable declarations
	char kvd_cArray[MAX_STRING_LENGTH];
	int kvd_iStringLength = 0;

	// code
	printf("\n\n");

	/* user inputs the string */
	printf("enter a string: ");
	gets_s(kvd_cArray, MAX_STRING_LENGTH);

	/* echo the string at first */
	printf("you entered:\n\n");
	printf("\t\"%s\"\n", kvd_cArray);

	/* now display the string length */
	kvd_iStringLength = MyStrlen(kvd_cArray);
	printf("\tlength of the string = %d characters\n\n", kvd_iStringLength);

	printf("\n\n");
	return(0);
}

int MyStrlen(char kvd_str[])
{
	// variable declarations
	int kvd_i, kvd_string_length = 0;

	// code
	for (kvd_i = 0; kvd_i < MAX_STRING_LENGTH; kvd_i++)
	{
		if (kvd_str[kvd_i] == '\0')
			break;
		else
			kvd_string_length++;
	}

	return(kvd_string_length);
}
