#include <stdio.h>

int main(void)
{
	// function prototypes
	int AddIntegers(int, int);
	int SubtractIntegers(int, int);
	float AddFloats(float, float);

	// variable declarations
	typedef int (*TwoIntegersFnPtr)(int, int);
	TwoIntegersFnPtr kvd_pTwoInts = NULL;

	typedef float (*TwoFloatsFnPtr)(float, float);
	TwoFloatsFnPtr kvd_pTwoFloats = NULL;

	int kvd_iAnswer;
	float kvd_fAnswer;

	// code
	kvd_pTwoInts = AddIntegers;
	kvd_iAnswer = kvd_pTwoInts(15, 13);
	printf("\n\n");
	printf("sum = %d\n", kvd_iAnswer);

	kvd_pTwoInts = SubtractIntegers;
	kvd_iAnswer = kvd_pTwoInts(15, 13);
	printf("\n\n");
	printf("subtraction = %d\n", kvd_iAnswer);

	kvd_pTwoFloats = AddFloats;
	kvd_fAnswer = kvd_pTwoFloats(1.52f, 3.81f);
	printf("\n\n");
	printf("sum = %f\n\n", kvd_fAnswer);

	return 0;
}

int AddIntegers(int kvd_x, int kvd_y)
{
	return kvd_x + kvd_y;
}

int SubtractIntegers(int kvd_x, int kvd_y)
{
	return kvd_x - kvd_y;
}

float AddFloats(float kvd_x, float kvd_y)
{
	return kvd_x + kvd_y;
}
