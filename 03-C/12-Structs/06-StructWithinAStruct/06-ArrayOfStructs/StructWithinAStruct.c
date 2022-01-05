#include <stdio.h>

struct Number
{
	int num;
	int tab[10];
};

struct Table
{
	struct Number n;
};

int main(void)
{
	// variable declarations
	struct Table kvd_tables[10];
	int kvd_i, kvd_j;

	// code
	printf("\n\n");

	for (kvd_i = 0; kvd_i < 10; kvd_i++)
		kvd_tables[kvd_i].n.num = kvd_i + 1;

	for (kvd_i = 0; kvd_i < 10; kvd_i++)
	{
		printf("table of %d:\n\n", kvd_i + 1);

		for (kvd_j = 0; kvd_j < 10; kvd_j++)
		{
			kvd_tables[kvd_i].n.tab[kvd_j] = kvd_tables[kvd_i].n.num * (kvd_j + 1);
			printf("\t%d * %d = %d\n", kvd_tables[kvd_i].n.num, kvd_j + 1, kvd_tables[kvd_i].n.tab[kvd_j]);
		}

		printf("\n");
	}

	return(0);
}
