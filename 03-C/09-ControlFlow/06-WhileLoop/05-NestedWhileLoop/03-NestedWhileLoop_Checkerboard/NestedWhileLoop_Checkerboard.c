#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_i, kvd_j, kvd_c;

	// code
	printf("\n\n");

	kvd_i = 0;
	while (kvd_i < 64)
	{
		kvd_j = 0;
		while (kvd_j < 64)
		{
			kvd_c = ((kvd_i & 0x04) == 0) ^ ((kvd_j & 0x08) == 0);

			if (kvd_c == 0)
				printf("$ ");
			
			if (kvd_c == 1)
				printf("  ");

			kvd_j++;
		}

		printf("\n\n");
		kvd_i++;
	}

	return(0);
}
