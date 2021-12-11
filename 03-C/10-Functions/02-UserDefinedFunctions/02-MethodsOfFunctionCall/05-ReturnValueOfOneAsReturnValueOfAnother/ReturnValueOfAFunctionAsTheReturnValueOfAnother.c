#include <stdio.h>  // for printf()

int main(int kvd_argc, char* kvd_argv[], char* kvd_envp[])
{
	// function prototypes
	int MyAddition(int, int);

	// variable declarations
	int kvd_r, kvd_num_1, kvd_num_2;

	// code
	kvd_num_1 = 10;
	kvd_num_2 = 20;

	printf("\n\n");

	printf("%d + %d = %d\n\n", kvd_num_1, kvd_num_2, MyAddition(kvd_num_1, kvd_num_2));

	return(0);
}

int MyAddition(int kvd_x, int kvd_y)
{
	// function prototypes
	int Add(int, int);

	// code
	// returning the return value of Add() to the caller, as the return value
	// of MyAddition()
	return(Add(kvd_x, kvd_y));
}

int Add(int kvd_x, int kvd_y)
{
	// legitimately performing the operation
	return(kvd_x + kvd_y);
}
