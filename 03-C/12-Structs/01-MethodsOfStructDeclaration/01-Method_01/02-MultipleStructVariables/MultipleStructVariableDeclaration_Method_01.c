#include <stdio.h>

// defining a struct
struct MyPoint
{
	int x;
	int y;
} kvd_point_A, kvd_point_B, kvd_point_C, kvd_point_D, kvd_point_E;  // 5 struct variables declared globally

int main(void)
{
	// code
	kvd_point_A.x = 3;
	kvd_point_A.y = 0;

	kvd_point_B.x = 1;
	kvd_point_B.y = 2;

	kvd_point_C.x = 9;
	kvd_point_C.y = 6;

	kvd_point_D.x = 8;
	kvd_point_D.y = 2;

	kvd_point_E.x = 11;
	kvd_point_E.y = 8;

	printf("\n\n");

	printf("kvd_point_A: (%d, %d)\n", kvd_point_A.x, kvd_point_A.y);
	printf("kvd_point_B: (%d, %d)\n", kvd_point_B.x, kvd_point_B.y);
	printf("kvd_point_C: (%d, %d)\n", kvd_point_C.x, kvd_point_C.y);
	printf("kvd_point_D: (%d, %d)\n", kvd_point_D.x, kvd_point_D.y);
	printf("kvd_point_E: (%d, %d)\n", kvd_point_E.x, kvd_point_E.y);

	printf("\n\n");
	return(0);
}
