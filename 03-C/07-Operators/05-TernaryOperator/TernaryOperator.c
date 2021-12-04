#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_a, kvd_b, kvd_p, kvd_q, kvd_i_result_01, kvd_i_result_02;
	char kvd_ch_result_01, kvd_ch_result_02;

	// code
	printf("\n\n");

	kvd_a = 7;
	kvd_b = 5;
	kvd_ch_result_01 = (kvd_a > kvd_b) ? 'A' : 'B';
	kvd_i_result_01 = (kvd_a > kvd_b) ? kvd_a : kvd_b;
	printf("ternary operator answer 1: %c and %d\n\n", kvd_ch_result_01, kvd_i_result_01);

	kvd_p = kvd_q = 30;
	kvd_ch_result_02 = (kvd_p != kvd_q) ? 'P' : 'Q';
	kvd_i_result_02 = (kvd_p != kvd_q) ? kvd_p : kvd_q;
	printf("ternary operator answer 2: %c and %d\n\n", kvd_ch_result_02, kvd_i_result_02);
}
