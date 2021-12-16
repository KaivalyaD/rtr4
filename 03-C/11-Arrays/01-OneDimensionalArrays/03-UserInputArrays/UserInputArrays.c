#include <stdio.h>
#include <conio.h>

// To change the fixed size of an array, simply change values of these
// macros and rebuild!

#define INT_ARRAY_ELEMENT_COUNT 5
#define FLOAT_ARRAY_ELEMENT_COUNT 7
#define CHAR_ARRAY_ELEMENT_COUNT 15

int main(void)
{
	// variable declarations
	int kvd_iArray[INT_ARRAY_ELEMENT_COUNT];
	float kvd_fArray[FLOAT_ARRAY_ELEMENT_COUNT];
	char kvd_cArray[CHAR_ARRAY_ELEMENT_COUNT];
	int kvd_i;

	// code
	printf("\n\n");

	/* arrays initialized by user inputs */
	printf("enter %d elements for kvd_iArray[]:\n", INT_ARRAY_ELEMENT_COUNT);
	for (kvd_i = 0; kvd_i < INT_ARRAY_ELEMENT_COUNT; kvd_i++)
		scanf("%d", &kvd_iArray[kvd_i]);

	printf("\n\n");

	printf("enter %d elements for kvd_fArray[]:\n", FLOAT_ARRAY_ELEMENT_COUNT);
	for (kvd_i = 0; kvd_i < FLOAT_ARRAY_ELEMENT_COUNT; kvd_i++)
		scanf("%f", &kvd_fArray[kvd_i]);

	printf("\n\n");

	printf("enter %d elements for kvd_cArray[]:\n", CHAR_ARRAY_ELEMENT_COUNT);
	for (kvd_i = 0; kvd_i < CHAR_ARRAY_ELEMENT_COUNT; kvd_i++)
	{
		// logic for echoing a user-entered character
		kvd_cArray[kvd_i] = getch();
		printf("%c\n", kvd_cArray[kvd_i]);
	}

	printf("\n\n");

	/* arrays displayed */
	printf("kvd_iArray[]:\n");
	for (kvd_i = 0; kvd_i < INT_ARRAY_ELEMENT_COUNT; kvd_i++)
		printf("\t%d\n", kvd_iArray[kvd_i]);

	printf("\n\n");

	printf("kvd_fArray[]:\n");
	for (kvd_i = 0; kvd_i < FLOAT_ARRAY_ELEMENT_COUNT; kvd_i++)
		printf("\t%f\n", kvd_fArray[kvd_i]);

	printf("\n\n");

	printf("kvd_cArray[]:\n");
	for (kvd_i = 0; kvd_i < CHAR_ARRAY_ELEMENT_COUNT; kvd_i++)
		printf("\t%c\n", kvd_cArray[kvd_i]);

	printf("\n\n");
	return(0);
}
