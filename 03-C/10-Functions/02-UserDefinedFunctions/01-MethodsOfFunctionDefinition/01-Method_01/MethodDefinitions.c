#include <stdio.h>  // for printf() and scanf()

int main(int kvd_argc, char* kvd_argv, char* kvd_envp)
{
	// this is how you declare a user-defined function
	void MyAddition(void);

	// code
	MyAddition();  // this is how you call a user-defined function
	return(0);
}

// this is how you define a user-defined function
// here, it has no return value and takes no parameters
void MyAddition(void)
{
	// variable declarations
	int kvd_a, kvd_b, kvd_sum;

	// code
	printf("\n\n");

	printf("enter a value for kvd_a: ");
	scanf("%d", &kvd_a);
	printf("enter a value for kvd_b: ");
	scanf("%d", &kvd_b);

	printf("\n\n");

	kvd_sum = kvd_a + kvd_b;

	printf("sum of kvd_a (= %d) and kvd_b (= %d) is %d\n\n", kvd_a, kvd_b, kvd_sum);
}
