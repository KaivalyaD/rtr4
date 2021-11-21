#include <stdio.h>

int main(void)
{
	// variable declarations
	float kvd_f, kvd_f_explicit, kvd_f_result;
	int kvd_i, kvd_i_explicit, kvd_i_result;
	char kvd_c;

	// code
	printf("\n\n");

	/* interconversion (implicit type-casts) between char and int */
	kvd_i = 68;
	kvd_c = kvd_i;
	printf("when kvd_i=%d is implicitly converted down to a char, we see kvd_c=%c\n\n", kvd_i, kvd_c);

	kvd_c = 'Q';
	kvd_i = kvd_c;
	printf("when kvd_c=%c is implicitly converted up to an int, we see kvd_i=%d\n\n", kvd_c, kvd_i);

	/* casting from an int to a float implicitly */
	kvd_i = 5;
	kvd_f = 7.8f;
	kvd_f_result = kvd_i + kvd_f;
	printf("sum of (int)kvd_i=%d and (float)kvd_f=%f yields a (float)kvd_f_result=%f without any data loss\n\n", kvd_i, kvd_f, kvd_f_result);

	/* casting from a float to an int implicitly: COMPILER WARNING (cl did not give one, but not all compilers are the same)! TRUNCATION! COERCION! DATA LOSS! */
	kvd_i_result = kvd_i + kvd_f;
	printf("sum of (int)kvd_i=%d and (float)kvd_f=%f yields a (int)kvd_i_result=%d with data loss\n\n", kvd_i, kvd_f, kvd_i_result);

	/* casting from a float to an int explicitly: TRUNCATION! COERCION! DATA LOSS! */
	kvd_f_explicit = 30.121995f;
	kvd_i_explicit = (int)kvd_f_explicit;
	printf("the floating point value that will undergo a type-cast to int: %f\n", kvd_f_explicit);
	printf("result of such a cast: %d\n\n", kvd_i_explicit);
}
