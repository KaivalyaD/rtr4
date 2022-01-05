#include <stdio.h>
#include <string.h>

#define INT_ARRAY_SIZE 10
#define FLOAT_ARRAY_SIZE 5
#define CHAR_ARRAY_SIZE 26

#define NUM_STRINGS 10
#define MAX_CHARACTERS_PER_STRING 20

#define ALPHABET_BEGINNING 65

struct MyDataOne
{
	int iArray[INT_ARRAY_SIZE];
	float fArray[FLOAT_ARRAY_SIZE];
};

struct MyDataTwo
{
	char cArray[CHAR_ARRAY_SIZE];
	char strArray[NUM_STRINGS][MAX_CHARACTERS_PER_STRING];
};

int main(void)
{
	// variable declarations
	struct MyDataOne kvd_data_one;
	struct MyDataTwo kvd_data_two;
	int kvd_i;

	// code
	// piece-meal assignment, hard-coded
	kvd_data_one.fArray[0] = 0.01f;
	kvd_data_one.fArray[1] = 5.12f;
	kvd_data_one.fArray[2] = 9.63f;
	kvd_data_one.fArray[3] = 7.15f;
	kvd_data_one.fArray[4] = 12.46f;
	kvd_data_one.fArray[5] = 4.54f;

	// user-defined loop assignment
	printf("\n\n");

	printf("enter %d integers:\n\n", INT_ARRAY_SIZE);
	for (kvd_i = 0; kvd_i < INT_ARRAY_SIZE; kvd_i++)
	{
		printf("\t");
		scanf("%d", &kvd_data_one.iArray[kvd_i]);
	}

	// hard-coded loop assignment
	for (kvd_i = 0; kvd_i < CHAR_ARRAY_SIZE; kvd_i++)
		kvd_data_two.cArray[kvd_i] = (char)(kvd_i + ALPHABET_BEGINNING);

	// piece-meal assignment, hard-coded
	strcpy(kvd_data_two.strArray[0], "Welcome!");
	strcpy(kvd_data_two.strArray[1], " This");
	strcpy(kvd_data_two.strArray[2], " is");
	strcpy(kvd_data_two.strArray[3], " AstroMediComp's");
	strcpy(kvd_data_two.strArray[4], " Real");
	strcpy(kvd_data_two.strArray[5], " Time");
	strcpy(kvd_data_two.strArray[6], " Rendering");
	strcpy(kvd_data_two.strArray[7], " Batch");
	strcpy(kvd_data_two.strArray[8], " of");
	strcpy(kvd_data_two.strArray[9], " 2020-21.");

	// displaying struct MyDataOne's members
	printf("\n\n");

	printf("struct MyDataOne kvd_data_one:\n\n");
	for (kvd_i = 0; kvd_i < INT_ARRAY_SIZE; kvd_i++)
		printf("\tkvd_data_one.iArray[%d] = %d\n", kvd_i, kvd_data_one.iArray[kvd_i]);
	
	printf("\n\n");
	for (kvd_i = 0; kvd_i < FLOAT_ARRAY_SIZE; kvd_i++)
		printf("\tkvd_data_one.fArray[%d] = %f\n", kvd_i, kvd_data_one.fArray[kvd_i]);

	// displaying struct MyDataTwo's members
	printf("\n\n");

	printf("struct MyDataTwo kvd_data_two:\n\n");
	for (kvd_i = 0; kvd_i < CHAR_ARRAY_SIZE; kvd_i++)
		printf("\tkvd_data_two.cArray[%d] = '%c'\n", kvd_i, kvd_data_two.cArray[kvd_i]);

	printf("\n\n");
	for (kvd_i = 0; kvd_i < NUM_STRINGS; kvd_i++)
		printf("\tkvd_data_two.strArray[%d] = \"%s\"\n", kvd_i, kvd_data_two.strArray[kvd_i]);

	printf("\n\n");
	return(0);
}
