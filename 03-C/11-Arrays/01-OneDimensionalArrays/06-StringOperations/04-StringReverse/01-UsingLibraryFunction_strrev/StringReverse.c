#include <stdio.h>
#include <string.h>  // for strrev()

#define KVD_MAX_STRING_LENGTH 512

int main(void)
{
	// variable declarations
	char kvd_cArray[KVD_MAX_STRING_LENGTH];

	// code
	printf("\n\n");

	// prompt the user to enter a string
	printf("enter a string: ");
	gets_s(kvd_cArray, KVD_MAX_STRING_LENGTH);

	// display
	printf("\n\n");

	printf("original string you entered:\n\n");
	printf("\t\"%s\"", kvd_cArray);

	printf("\n\n");

	printf("reversed string:\n\n");
	printf("\t\"%s\"", strrev(kvd_cArray));

	printf("\n\n");
	return(0);
}
