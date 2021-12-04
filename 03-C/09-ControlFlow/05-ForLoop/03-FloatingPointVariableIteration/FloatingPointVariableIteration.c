#include <stdio.h>

int main(void)
{
	// variable declarations
	float kvd_f = 1.57f, kvd_f_num = 3.14f;

	// code
	printf("\n\n");

	printf("printing numbers from %f to %f: \n\n", kvd_f, kvd_f_num);
	for (kvd_f = 1.57; kvd_f <= (kvd_f_num * 10.0f); kvd_f = kvd_f + kvd_f_num)
	{
		printf("\t%f\n", kvd_f);
	}

	printf("\n\n");

	return(0);
}
