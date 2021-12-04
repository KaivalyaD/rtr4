#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_i, kvd_j, kvd_c;

	// code
	printf("\n\n");

	kvd_i = 0;
	do
	{
		kvd_j = 0;
		do
		{
			kvd_c = ((kvd_i & 0x04) == 0) ^ ((kvd_j & 0x08) == 0);

			if (kvd_c == 0)
				printf("@ ");
			else
				printf("  ");

			kvd_j++;
		} while (kvd_j < 64);

		printf("\n\n");
		kvd_i++;
	} while (kvd_i < 64);

	return(0);
}
