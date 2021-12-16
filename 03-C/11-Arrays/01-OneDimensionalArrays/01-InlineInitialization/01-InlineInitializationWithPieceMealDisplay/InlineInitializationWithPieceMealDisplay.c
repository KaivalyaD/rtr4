#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_iArray[] = { 9, 30, 6, 12, 98, 95, 20, 23, 2, 45 };
	int kvd_int_size, kvd_iArray_size, kvd_iArray_num_elements;

	float kvd_fArray[] = { 1.2f, 2.3f, 3.4f, 4.5f, 5.6f, 6.7f, 7.8f, 8.9f };
	int kvd_float_size, kvd_fArray_size, kvd_fArray_num_elements;

	char kvd_cArray[] = { 'A', 'S', 'T', 'R', 'O', 'M', 'E', 'D', 'I', 'C', 'O', 'M', 'P' };
	int kvd_char_size, kvd_cArray_size, kvd_cArray_num_elements;

	// code

	/****** kvd_iArray[] ******/
	printf("\n\n");
	
	printf("inline initialization and piece-meal display of elements of array kvd_iArray[]:\n");
	printf("kvd_iArray[0] (1st element) = %d\n", kvd_iArray[0]);
	printf("kvd_iArray[1] (2nd element) = %d\n", kvd_iArray[1]);
	printf("kvd_iArray[2] (3rd element) = %d\n", kvd_iArray[2]);
	printf("kvd_iArray[3] (4th element) = %d\n", kvd_iArray[3]);
	printf("kvd_iArray[4] (5th element) = %d\n", kvd_iArray[4]);
	printf("kvd_iArray[5] (6th element) = %d\n", kvd_iArray[5]);
	printf("kvd_iArray[6] (7th element) = %d\n", kvd_iArray[6]);
	printf("kvd_iArray[7] (8th element) = %d\n", kvd_iArray[7]);
	printf("kvd_iArray[8] (9th element) = %d\n", kvd_iArray[8]);
	printf("kvd_iArray[9] (10th element) = %d\n\n", kvd_iArray[9]);

	kvd_int_size = sizeof(int);
	kvd_iArray_size = sizeof(kvd_iArray);
	kvd_iArray_num_elements = kvd_iArray_size / kvd_int_size;

	printf("size of the int datatype = %d\n", kvd_int_size);
	printf("size of kvd_iArray[] = %d\n", kvd_iArray_size);
	printf("number of elements in kvd_iArray[] = %d\n", kvd_iArray_num_elements);

	/****** kvd_fArray[] ******/
	printf("\n\n");

	printf("inline initialization and piece-meal display of elements of array kvd_fArray[]:\n");
	printf("kvd_fArray[0] (1st element) = %f\n", kvd_fArray[0]);
	printf("kvd_fArray[1] (2nd element) = %f\n", kvd_fArray[1]);
	printf("kvd_fArray[2] (3rd element) = %f\n", kvd_fArray[2]);
	printf("kvd_fArray[3] (4th element) = %f\n", kvd_fArray[3]);
	printf("kvd_fArray[4] (5th element) = %f\n", kvd_fArray[4]);
	printf("kvd_fArray[5] (6th element) = %f\n", kvd_fArray[5]);
	printf("kvd_fArray[6] (7th element) = %f\n", kvd_fArray[6]);
	printf("kvd_fArray[7] (8th element) = %f\n\n", kvd_fArray[7]);

	kvd_float_size = sizeof(float);
	kvd_fArray_size = sizeof(kvd_fArray);
	kvd_fArray_num_elements = kvd_fArray_size / kvd_float_size;

	printf("size of the float datatype = %d\n", kvd_float_size);
	printf("size of kvd_fArray[] = %d\n", kvd_fArray_size);
	printf("number of elements in kvd_fArray[] = %d\n", kvd_fArray_num_elements);

	/****** kvd_cArray[] ******/
	printf("\n\n");

	printf("inline initialization and piece-meal display of elements of array kvd_cArray[]:\n");
	printf("kvd_cArray[0] (1st element) = %c\n", kvd_cArray[0]);
	printf("kvd_cArray[1] (2nd element) = %c\n", kvd_cArray[1]);
	printf("kvd_cArray[2] (3rd element) = %c\n", kvd_cArray[2]);
	printf("kvd_cArray[3] (4th element) = %c\n", kvd_cArray[3]);
	printf("kvd_cArray[4] (5th element) = %c\n", kvd_cArray[4]);
	printf("kvd_cArray[5] (6th element) = %c\n", kvd_cArray[5]);
	printf("kvd_cArray[6] (7th element) = %c\n", kvd_cArray[6]);
	printf("kvd_cArray[7] (8th element) = %c\n", kvd_cArray[7]);
	printf("kvd_cArray[8] (9th element) = %c\n", kvd_cArray[8]);
	printf("kvd_cArray[9] (10th element) = %c\n", kvd_cArray[9]);
	printf("kvd_fArray[10] (11th element) = %c\n", kvd_cArray[10]);
	printf("kvd_fArray[11] (12th element) = %c\n", kvd_cArray[11]);
	printf("kvd_fArray[12] (13th element) = %c\n\n", kvd_cArray[12]);

	kvd_char_size = sizeof(char);
	kvd_cArray_size = sizeof(kvd_cArray);
	kvd_cArray_num_elements = kvd_cArray_size / kvd_char_size;

	printf("size of the char datatype = %d\n", kvd_char_size);
	printf("size of kvd_cArray[] = %d\n", kvd_cArray_size);
	printf("number of elements in kvd_cArray[] = %d\n", kvd_cArray_num_elements);

	return(0);
}
