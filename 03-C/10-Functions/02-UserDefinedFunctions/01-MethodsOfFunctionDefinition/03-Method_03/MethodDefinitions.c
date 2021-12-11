#include <stdio.h>  // for printf() and scanf()

int main(int kvd_argc, char* kvd_argv[], char* kvd_envp[])
{
	// function declarations
	void MyAddition(int, int);

	// variable declarations
	int kvd_a, kvd_b;

	// code
	printf("\n\n");

	printf("enter a value for kvd_a: ");
	scanf("%d", &kvd_a);
	printf("enter a value for kvd_b: ");
	scanf("%d", &kvd_b);

	MyAddition(kvd_a, kvd_b);  // call

	return(0);
}

// this time, the function returns nothing, but takes 2 arguments,
// sums them up, and displays the result
void MyAddition(int kvd_x, int kvd_y)
{
	// variable declarations
	int kvd_sum;

	// code
	kvd_sum = kvd_x + kvd_y;

	printf("\n\n");

	printf("sum of kvd_a (= %d) and kvd_b (= %d) is %d\n\n", kvd_x, kvd_y, kvd_sum);
}
