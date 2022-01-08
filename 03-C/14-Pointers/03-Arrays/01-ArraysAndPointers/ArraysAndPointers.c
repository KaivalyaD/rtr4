#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_iArray[] = { 2, 4, 6, 8, 10, 12, 14, 16, 18, 20 };
	float kvd_fArray[] = { 1.2f, 2.3f, 3.4f, 4.5f, 5.6f, 6.7f, 7.8f, 8.9f, 9.1f, 0.2f };
	char kvd_cArray[] = { 'A', 'S', 'T', 'R', 'O', 'M', 'E', 'D', 'I', 'C', 'O', 'M', 'P', '\0' };

	// code
	printf("\n\n");

	printf("elements and their addresses in kvd_iArray[]:\n\n");
	printf("\tkvd_iArray[0] = %d; at address = 0x%p\n", *(kvd_iArray + 0), (kvd_iArray + 0));
	printf("\tkvd_iArray[1] = %d; at address = 0x%p\n", *(kvd_iArray + 1), (kvd_iArray + 1));
	printf("\tkvd_iArray[2] = %d; at address = 0x%p\n", *(kvd_iArray + 2), (kvd_iArray + 2));
	printf("\tkvd_iArray[3] = %d; at address = 0x%p\n", *(kvd_iArray + 3), (kvd_iArray + 3));
	printf("\tkvd_iArray[4] = %d; at address = 0x%p\n", *(kvd_iArray + 4), (kvd_iArray + 4));
	printf("\tkvd_iArray[5] = %d; at address = 0x%p\n", *(kvd_iArray + 5), (kvd_iArray + 5));
	printf("\tkvd_iArray[6] = %d; at address = 0x%p\n", *(kvd_iArray + 6), (kvd_iArray + 6));
	printf("\tkvd_iArray[7] = %d; at address = 0x%p\n", *(kvd_iArray + 7), (kvd_iArray + 7));
	printf("\tkvd_iArray[8] = %d; at address = 0x%p\n", *(kvd_iArray + 8), (kvd_iArray + 8));
	printf("\tkvd_iArray[9] = %d; at address = 0x%p\n\n", *(kvd_iArray + 9), (kvd_iArray + 9));

	printf("elements and their address in kvd_fArray[]:\n\n");
	printf("\tkvd_fArray[0] = %f; at address = 0x%p\n", *(kvd_fArray + 0), (kvd_fArray + 0));
	printf("\tkvd_fArray[1] = %f; at address = 0x%p\n", *(kvd_fArray + 1), (kvd_fArray + 1));
	printf("\tkvd_fArray[2] = %f; at address = 0x%p\n", *(kvd_fArray + 2), (kvd_fArray + 2));
	printf("\tkvd_fArray[3] = %f; at address = 0x%p\n", *(kvd_fArray + 3), (kvd_fArray + 3));
	printf("\tkvd_fArray[4] = %f; at address = 0x%p\n", *(kvd_fArray + 4), (kvd_fArray + 4));
	printf("\tkvd_fArray[5] = %f; at address = 0x%p\n", *(kvd_fArray + 5), (kvd_fArray + 5));
	printf("\tkvd_fArray[6] = %f; at address = 0x%p\n", *(kvd_fArray + 6), (kvd_fArray + 6));
	printf("\tkvd_fArray[7] = %f; at address = 0x%p\n", *(kvd_fArray + 7), (kvd_fArray + 7));
	printf("\tkvd_fArray[8] = %f; at address = 0x%p\n", *(kvd_fArray + 8), (kvd_fArray + 8));
	printf("\tkvd_fArray[9] = %f; at address = 0x%p\n\n", *(kvd_fArray + 9), (kvd_fArray + 9));

	printf("elements and their address in kvd_cArray[]:\n\n");
	printf("\tkvd_cArray[0] = '%c'; at address = 0x%p\n", *(kvd_cArray + 0), (kvd_cArray + 0));
	printf("\tkvd_cArray[1] = '%c'; at address = 0x%p\n", *(kvd_cArray + 1), (kvd_cArray + 1));
	printf("\tkvd_cArray[2] = '%c'; at address = 0x%p\n", *(kvd_cArray + 2), (kvd_cArray + 2));
	printf("\tkvd_cArray[3] = '%c'; at address = 0x%p\n", *(kvd_cArray + 3), (kvd_cArray + 3));
	printf("\tkvd_cArray[4] = '%c'; at address = 0x%p\n", *(kvd_cArray + 4), (kvd_cArray + 4));
	printf("\tkvd_cArray[5] = '%c'; at address = 0x%p\n", *(kvd_cArray + 5), (kvd_cArray + 5));
	printf("\tkvd_cArray[6] = '%c'; at address = 0x%p\n", *(kvd_cArray + 6), (kvd_cArray + 6));
	printf("\tkvd_cArray[7] = '%c'; at address = 0x%p\n", *(kvd_cArray + 7), (kvd_cArray + 7));
	printf("\tkvd_cArray[8] = '%c'; at address = 0x%p\n", *(kvd_cArray + 8), (kvd_cArray + 8));
	printf("\tkvd_cArray[9] = '%c'; at address = 0x%p\n", *(kvd_cArray + 9), (kvd_cArray + 9));
	printf("\tkvd_cArray[10] = '%c'; at address = 0x%p\n", *(kvd_cArray + 10), (kvd_cArray + 10));
	printf("\tkvd_cArray[11] = '%c'; at address = 0x%p\n", *(kvd_cArray + 11), (kvd_cArray + 11));
	printf("\tkvd_cArray[12] = '%c'; at address = 0x%p\n", *(kvd_cArray + 12), (kvd_cArray + 12));
	printf("\tkvd_cArray[13] = '%c'; at address = 0x%p\n", *(kvd_cArray + 13), (kvd_cArray + 13));

	return(0);
}
