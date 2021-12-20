#include <stdio.h>
#include <string.h>

#define KVD_MAX_STRING_LENGTH 512

int main(void)
{
	// variable declarations
	char kvd_cArray_One[KVD_MAX_STRING_LENGTH], kvd_cArray_Two[KVD_MAX_STRING_LENGTH];

	// code
	printf("\n\n");

	printf("enter a string: ");
	gets_s(kvd_cArray_One, KVD_MAX_STRING_LENGTH);
	printf("enter another string: ");
	gets_s(kvd_cArray_Two, KVD_MAX_STRING_LENGTH);

	printf("\n\n");

	printf("original strings:\n\n");

	printf("\tstring 1:\n\n");
	printf("\t\t\"%s\"\n\n", kvd_cArray_One);
	
	printf("\tstring 2:\n\n");
	printf("\t\t\"%s\"\n\n", kvd_cArray_Two);

	strcat(kvd_cArray_One, kvd_cArray_Two);

	printf("concatinated strings:\n\n");
	printf("\t\"%s\"\n", kvd_cArray_One);

	printf("\n\n");
	return(0);
}
