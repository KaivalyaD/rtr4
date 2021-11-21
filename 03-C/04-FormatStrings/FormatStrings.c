#include <stdio.h>

int main(void)
{
	// code
	printf("\n\n");
	printf("**************************************************************************");
	printf("\n\n");

	printf("Hello World !!!\n\n");

	int a = 13;
	printf("integer decimal value of 'a' = %d\n", a);
	printf("integer octal value of 'a' = %o\n", a);
	printf("integer hexadecimal value of 'a' in lower-case hex-digits = %x\n", a);
	printf("integer hexadecimal value of 'a' in upper-case hex-digits = %X\n\n", a);

	char ch = 'A';
	printf("character 'ch' = %c\n", ch);
	char str[] = "AstroMediComp's Real Time Rendering, Batch 4, 2021";
	printf("string 'str' = %s\n\n", str);

	long num = 30121995L;
	printf("long integer 'num' = %ld\n\n", num);

	unsigned int b = 7;
	printf("unsigned integer 'b' = %u\n\n", b);

	float f_num = 3012.1995f;
	printf("floating point number 'f_num' with just %%f = %f\n", f_num);
	printf("floating point number 'f_num' with %%4.2f = %4.2f\n", f_num);
	printf("floating point nunber 'f_num' with %%6.5f = %6.5f\n\n", f_num);

	double d_pi = 3.14159265358979323846;
	printf("double precision floating point number without exponential = %g\n", d_pi);
	printf("double precision floating point number with lower-case exponential = %e\n", d_pi);
	printf("double precision floating point number with upper-case exponential = %E\n", d_pi);
	printf("double precision lower-case hexadecimal value of 'd_pi' = %a\n", d_pi);
	printf("double precision upper-case hexadecimal value of d_pi = %A\n\n", d_pi);

	printf("**************************************************************************");
	printf("\n\n");
	return(0);
}
