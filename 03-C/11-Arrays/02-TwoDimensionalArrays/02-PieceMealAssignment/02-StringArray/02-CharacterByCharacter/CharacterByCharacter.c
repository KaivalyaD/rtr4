#include <stdio.h>

#define MAX_STRING_LENGTH 512

int main(void)
{
	// variable declarations
	char kvd_strArray[5][10];
	int kvd_strArray_size, kvd_strArray_row_count, kvd_strArray_col_count, kvd_strArray_element_count;
	int kvd_i;
	
	// code
	printf("\n\n");

	kvd_strArray_size = sizeof(kvd_strArray);
	printf("size of the 2D array of characters kvd_strArray[][] = %d\n", kvd_strArray_size);

	kvd_strArray_row_count = kvd_strArray_size / sizeof(kvd_strArray[0]);
	printf("max. number of rows in kvd_strArray[][] = %d\n", kvd_strArray_row_count);

	kvd_strArray_col_count = sizeof(kvd_strArray[0]) / sizeof(kvd_strArray[0][0]);
	printf("max. number of columns in kvd_strArray[][] = %d\n", kvd_strArray_col_count);

	kvd_strArray_element_count = kvd_strArray_row_count * kvd_strArray_col_count;
	printf("max. number of elements that can fit into kvd_strArray[][] = %d\n", kvd_strArray_element_count);

	/* character-by-character piece meal assignment */
	// row 1 / string 1
	kvd_strArray[0][0] = 'M';
	kvd_strArray[0][1] = 'y';
	kvd_strArray[0][2] = '\0';

	// row 2 / string 2
	kvd_strArray[1][0] = 'n';
	kvd_strArray[1][1] = 'a';
	kvd_strArray[1][2] = 'm';
	kvd_strArray[1][3] = 'e';
	kvd_strArray[1][4] = '\0';

	// row 3 / string 3
	kvd_strArray[2][0] = 'i';
	kvd_strArray[2][1] = 's';
	kvd_strArray[2][2] = '\0';

	// row 4 / string 4
	kvd_strArray[3][0] = 'K';
	kvd_strArray[3][1] = 'a';
	kvd_strArray[3][2] = 'i';
	kvd_strArray[3][3] = 'v';
	kvd_strArray[3][4] = 'a';
	kvd_strArray[3][5] = 'l';
	kvd_strArray[3][6] = 'y';
	kvd_strArray[3][7] = 'a';
	kvd_strArray[3][8] = '\0';

	// row 5 / string 5
	kvd_strArray[4][0] = 'D';
	kvd_strArray[4][1] = 'e';
	kvd_strArray[4][2] = 's';
	kvd_strArray[4][3] = 'h';
	kvd_strArray[4][4] = 'p';
	kvd_strArray[4][5] = 'a';
	kvd_strArray[4][6] = 'n';
	kvd_strArray[4][7] = 'd';
	kvd_strArray[4][8] = 'e';
	kvd_strArray[4][9] = '\0';

	printf("\n\n");

	printf("displaying all strings in kvd_strArray[][]:\n\n\t");
	for (kvd_i = 0; kvd_i < kvd_strArray_row_count; kvd_i++)
		printf("%s ", kvd_strArray[kvd_i]);

	printf("\n\n");
	return(0);
}
