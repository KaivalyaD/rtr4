#include <stdio.h>  // for printf()
#include <stdlib.h>  // for exit()

int main(int kvd_argc, char* kvd_argv[], char* kvd_envp[])
{
	// function declarations
	void MyAddition(void);
	int MySubtraction(void);
	void MyMultiplication(int, int);
	int MyDivision(int, int);

	// variable declarations
	int kvd_result_subtraction;
	int kvd_a_multiplication, kvd_b_multiplication;
	int kvd_a_division, kvd_b_division, kvd_division_result;

	// code
	MyAddition();  // calling a function that takes nothing and returns nothing

	kvd_result_subtraction = MySubtraction();  // calling a function that takes nothing, but returns an integer value
	printf("\n\n");
	printf("subtraction yields %d", kvd_result_subtraction);

	printf("\n\n");

	printf("enter an integer for kvd_a_multiplication: ");
	scanf("%d", &kvd_a_multiplication);
	printf("enter an integer for kvd_b_multiplication: ");
	scanf("%d", &kvd_b_multiplication);

	MyMultiplication(kvd_a_multiplication, kvd_b_multiplication);  // calling a function that takes 2 arguments, but returns nothing

	printf("\n\n");

	printf("enter a value for kvd_a_division: ");
	scanf("%d", &kvd_a_division);
	printf("enter a value for kvd_b_division: ");
	scanf("%d", &kvd_b_division);

	kvd_division_result = MyDivision(kvd_a_division, kvd_b_division);  // calling a function that takes 2 arguments and returns an integer value

	printf("division of kvd_a_division (= %d) by kvd_b_division (= %d) yields %d", kvd_a_division, kvd_b_division, kvd_division_result);

	printf("\n\n");
	return(0);
}

// definition of MyAddition()
void MyAddition(void)
{
	// variable declarations
	int kvd_a_addition, kvd_b_addition, kvd_sum_addition;

	// code
	printf("\n\n");

	printf("enter a value for kvd_a_addition: ");
	scanf("%d", &kvd_a_addition);
	printf("enter a value for kvd_b_addition: ");
	scanf("%d", &kvd_b_addition);

	kvd_sum_addition = kvd_a_addition + kvd_b_addition;

	printf("addition of kvd_a_addition (= %d) and kvd_b_addition (= %d) is %d\n\n", kvd_a_addition, kvd_b_addition, kvd_sum_addition);
}

// definition of MySubtraction()
int MySubtraction(void)
{
	// variable declarations
	int kvd_a_subtraction, kvd_b_subtraction;

	// code
	printf("\n\n");

	printf("enter a value for kvd_a_subtraction: ");
	scanf("%d", &kvd_a_subtraction);
	printf("enter a value for kvd_b_subtraction: ");
	scanf("%d", &kvd_b_subtraction);

	return(kvd_a_subtraction - kvd_b_subtraction);
}

// definition of MyMultiplication()
void MyMultiplication(int kvd_x, int kvd_y)
{
	// variable declarations
	int kvd_multiplication_result = kvd_x * kvd_y;

	printf("\n\n");

	printf("multiplication of kvd_x (= %d) and kvd_y (= %d) is %d", kvd_x, kvd_y, kvd_multiplication_result);
}

// definition of MyDivision()
int MyDivision(int kvd_x, int kvd_y)
{
	if (kvd_y == 0)
	{
		printf("cannot divide by 0\n");
		exit(1);
	}

	return(kvd_x / kvd_y);
}
