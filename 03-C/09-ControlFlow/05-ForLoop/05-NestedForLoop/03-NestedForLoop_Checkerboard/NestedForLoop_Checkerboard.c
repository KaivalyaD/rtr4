#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_i, kvd_j, kvd_c;

	// code
	printf("\n\n");

	for (kvd_i = 0; kvd_i < 64; kvd_i++)
	{
		for (kvd_j = 0; kvd_j < 64; kvd_j++)
		{
			kvd_c = ((kvd_i & 0x04) == 0) ^ ((kvd_j & 0x08) == 0);

			if (kvd_c == 0)
				printf("# ");

			if (kvd_c == 1)
				printf("  ");
		}

		printf("\n\n");
	}

	return(0);
}
