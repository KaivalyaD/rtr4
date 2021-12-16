#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_iArray[] = { 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
	int kvd_int_size, kvd_iArray_size, kvd_iArray_num_elements;

	float kvd_fArray[] = { 1.2f, 2.3f, 3.4f, 4.5f, 5.6f, 6.7f, 7.8f, 8.9f };
	int kvd_float_size, kvd_fArray_size, kvd_fArray_num_elements;

	char kvd_cArray[] = { 'A', 'S', 'T', 'R', 'O', 'M', 'E', 'D', 'I', 'C', 'O', 'M', 'P' };
	int kvd_char_size, kvd_cArray_size, kvd_cArray_num_elements;

	int kvd_i;

	// code

	/***** kvd_iArray[] *****/
	printf("\n\n");

	printf("inline initialization and loop display of elements in kvd_iArray[]:\n\n");
	
	kvd_int_size = sizeof(int);
	kvd_iArray_size = sizeof(kvd_iArray);
	kvd_iArray_num_elements = kvd_iArray_size / kvd_int_size;
	for (kvd_i = 0; kvd_i < kvd_iArray_num_elements; kvd_i++)
	{
		printf("kvd_iArray[%d] (element number %d) = %d\n", kvd_i, kvd_i+1, kvd_iArray[kvd_i]);
	}

	printf("\n\n");

	printf("size of the int datatype = %d\n", kvd_int_size);
	printf("size of kvd_iArray[] = %d\n", kvd_iArray_size);
	printf("number of elements in kvd_iArray[] = %d\n", kvd_iArray_num_elements);

	/***** kvd_fArray[] *****/
	printf("\n\n");

	printf("inline initialization and loop display of elements in kvd_fArray[]:\n\n");

	kvd_float_size = sizeof(float);
	kvd_fArray_size = sizeof(kvd_fArray);
	kvd_fArray_num_elements = kvd_fArray_size / kvd_float_size;
	kvd_i = 0;
	while(kvd_i < kvd_fArray_num_elements)
	{
		printf("kvd_fArray[%d] (element number %d) = %f\n", kvd_i, kvd_i + 1, kvd_fArray[kvd_i]);
		kvd_i++;
	}

	printf("\n\n");

	printf("size of the float datatype = %d\n", kvd_float_size);
	printf("size of kvd_fArray[] = %d\n", kvd_fArray_size);
	printf("number of elements in kvd_fArray[] = %d\n", kvd_fArray_num_elements);

	/***** kvd_cArray[] *****/
	printf("\n\n");

	printf("inline initialization and loop display of elements in kvd_cArray[]:\n\n");

	kvd_char_size = sizeof(char);
	kvd_cArray_size = sizeof(kvd_cArray);
	kvd_cArray_num_elements = kvd_cArray_size / kvd_char_size;
	kvd_i = 0;
	do
	{
		printf("kvd_cArray[%d] (element number %d) = %c\n", kvd_i, kvd_i + 1, kvd_cArray[kvd_i]);
		kvd_i++;
	} while (kvd_i < kvd_cArray_num_elements);

	printf("\n\n");

	printf("size of the char datatype = %d\n", kvd_char_size);
	printf("size of kvd_cArray[] = %d\n", kvd_cArray_size);
	printf("number of elements in kvd_cArray[] = %d\n", kvd_cArray_num_elements);

	printf("\n\n");
	return(0);
}
