#include <stdio.h>
#include <string.h>  // for strcpy()

#define KVD_MAX_STRING_LENGTH 512

int main(void)
{
	// variable declarations
	char kvd_cArray[KVD_MAX_STRING_LENGTH], kvd_cArray_copy[KVD_MAX_STRING_LENGTH];

	// code
	printf("\n\n");

	// prompt the user for string input
	printf("enter a string: ");
	gets_s(kvd_cArray, KVD_MAX_STRING_LENGTH);

	// copy kvd_cArray to kvd_cArray_copy
	strcpy(kvd_cArray_copy, kvd_cArray);
	
	// display
	printf("\n\n");

	printf("original string you entered:\n\n");
	printf("\t\"%s\"\n\n", kvd_cArray);

	printf("copied string:\n\n");
	printf("\t\"%s\"\n", kvd_cArray_copy);

	printf("\n\n");
	return(0);
}
