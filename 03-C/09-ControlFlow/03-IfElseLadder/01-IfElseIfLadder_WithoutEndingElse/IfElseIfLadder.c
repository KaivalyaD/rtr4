#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_num;

	// code
	printf("\n\n");

	printf("enter a value for kvd_num: ");
	scanf("%d", &kvd_num);

	// if-else if ladder begins
	if (kvd_num < 0)
		printf("kvd_num (= %d) is less than 0\n\n", kvd_num);

	else if ((kvd_num > 0) && (kvd_num <= 100))
		printf("kvd_num (= %d) lies between 0 and 100\n\n", kvd_num);

	else if ((kvd_num > 100) && (kvd_num <= 200))
		printf("kvd_num (= %d) lies between 100 and 200\n\n", kvd_num);

	else if ((kvd_num > 200) && (kvd_num <= 300))
		printf("kvd_num (= %d) lies between 200 and 300\n\n", kvd_num);

	else if ((kvd_num > 300) && (kvd_num <= 400))
		printf("kvd_num (= %d) lies between 300 and 400\n\n", kvd_num);

	else if ((kvd_num > 400) && (kvd_num <= 500))
		printf("kvd_num (= %d) lies between 400 and 500\n\n", kvd_num);

	else if (kvd_num > 500)
		printf("kvd_num (= %d) is greater than 500\n\n", kvd_num);

	return(0);
}
