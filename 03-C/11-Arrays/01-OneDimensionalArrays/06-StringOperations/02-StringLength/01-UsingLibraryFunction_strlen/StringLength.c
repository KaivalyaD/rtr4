#include <stdio.h>
#include <string.h>  // for strlen()

#define MAX_STRING_LENGTH 512

int main(void)
{
	// variable declarations
	char kvd_cArray[MAX_STRING_LENGTH];
	int kvd_iStringLength = 0;

	// code
	printf("\n\n");

	/* user inputs a string */
	printf("enter a string: ");
	gets_s(kvd_cArray, MAX_STRING_LENGTH);

	/* echo the string first */
	printf("you entered:\n\n");
	printf("\t\"%s\"\n", kvd_cArray);

	/* display the length of string */
	kvd_iStringLength = strlen(kvd_cArray);
	printf("\tlength of string = %d characters\n", kvd_iStringLength);

	printf("\n\n");
	return(0);
}
