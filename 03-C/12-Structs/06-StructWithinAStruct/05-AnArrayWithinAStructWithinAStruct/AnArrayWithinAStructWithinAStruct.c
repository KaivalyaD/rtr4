#include <stdio.h>

struct Number
{
	int num;
	int mult_tab[10];
};

struct Tables
{
	struct Number x, y, z;
};

int main(void)
{
	// variable declarations
	struct Tables kvd_tables;
	int kvd_i;

	// code
	kvd_tables.x.num = 2;
	for (kvd_i = 0; kvd_i < 10; kvd_i++)
		kvd_tables.x.mult_tab[kvd_i] = kvd_tables.x.num * (kvd_i + 1);
	
	printf("\n\n");
	printf("table of %d:\n\n", kvd_tables.x.num);
	for (kvd_i = 0; kvd_i < 10; kvd_i++)
		printf("\t%d * %d = %d\n", kvd_tables.x.num, kvd_i + 1, kvd_tables.x.mult_tab[kvd_i]);
	
	kvd_tables.y.num = 3;
	for (kvd_i = 0; kvd_i < 10; kvd_i++)
		kvd_tables.y.mult_tab[kvd_i] = kvd_tables.y.num * (kvd_i + 1);

	printf("\n\n");
	printf("table of %d:\n\n", kvd_tables.y.num);
	for (kvd_i = 0; kvd_i < 10; kvd_i++)
		printf("\t%d * %d = %d\n", kvd_tables.y.num, kvd_i + 1, kvd_tables.y.mult_tab[kvd_i]);

	kvd_tables.z.num = 4;
	for (kvd_i = 0; kvd_i < 10; kvd_i++)
		kvd_tables.z.mult_tab[kvd_i] = kvd_tables.z.num * (kvd_i + 1);

	printf("\n\n");
	printf("table of %d\n", kvd_tables.z.num);
	for (kvd_i = 0; kvd_i < 10; kvd_i++)
		printf("\t%d * %d = %d\n", kvd_tables.z.num, kvd_i + 1, kvd_tables.z.mult_tab[kvd_i]);

	printf("\n\n");
	return(0);
}
