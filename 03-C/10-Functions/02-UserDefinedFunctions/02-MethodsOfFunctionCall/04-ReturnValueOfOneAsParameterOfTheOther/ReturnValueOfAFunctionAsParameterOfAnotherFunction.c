#include <stdio.h>  // for printf()

int main(int kvd_argc, char* kvd_argv[], char* kvd_envp[])
{
	// function prototypes
	int MyAddition(int, int);

	// variable declarations
	int kvd_r, kvd_num_1, kvd_num_2, kvd_num_3, kvd_num_4;

	// code
	kvd_num_1 = 10;
	kvd_num_2 = 20;
	kvd_num_3 = 30;
	kvd_num_4 = 40;

	// the values returned by MyAddition() are used again as the new arguments to itself
	kvd_r = MyAddition(MyAddition(kvd_num_1, kvd_num_2), MyAddition(kvd_num_3, kvd_num_4));

	printf("\n\n");

	printf("%d + %d + %d + %d = %d\n\n", kvd_num_1, kvd_num_2, kvd_num_3, kvd_num_4, kvd_r);

	return(0);
}

int MyAddition(int kvd_x, int kvd_y)
{
	return(kvd_x + kvd_y);
}
