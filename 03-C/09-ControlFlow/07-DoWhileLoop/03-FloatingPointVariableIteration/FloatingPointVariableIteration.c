#include <stdio.h>

int main(void)
{
	// variable declarations
	float kvd_f, kvd_f_num = 3.14f;

	// code
	printf("\n\n");

	kvd_f = kvd_f_num;
	printf("printing numbers from %f to %f:\n\n", kvd_f_num, (kvd_f_num * 10.0f));

	do
	{
		printf("\t%f\n", kvd_f);
		kvd_f = kvd_f + kvd_f_num;
	} while (kvd_f <= (kvd_f_num * 10.0f));

	printf("\n\n");

	return(0);
}
