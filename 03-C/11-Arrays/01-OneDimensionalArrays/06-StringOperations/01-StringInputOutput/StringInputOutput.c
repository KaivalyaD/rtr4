#include <stdio.h>

#define MAX_STRING_LENGTH 512

int main(void)
{
	// variable declarations
	char kvd_cArray[MAX_STRING_LENGTH];  // a string is a character array

	// code
	printf("\n\n");

	printf("enter a string: ");
	gets_s(kvd_cArray, MAX_STRING_LENGTH);

	printf("\n\n");

	printf("you entered the following string:\n\n");
	printf("\t\"%s\"", kvd_cArray);

	printf("\n\n");
	return(0);
}
