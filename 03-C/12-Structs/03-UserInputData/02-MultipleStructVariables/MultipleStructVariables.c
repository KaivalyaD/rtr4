#include <stdio.h>

struct MyPoint
{
	int x, y;
};

int main(void)
{
	// variable declarations
	struct MyPoint kvd_point_A, kvd_point_B, kvd_point_C, kvd_point_D, kvd_point_E;

	// code
	printf("\n\n");

	printf("enter the x-coordinate of point A: ");
	scanf("%d", &kvd_point_A.x);
	printf("enter the y-coordinate of point A: ");
	scanf("%d", &kvd_point_A.y);

	printf("\n\n");

	printf("enter the x-coordinate of point B: ");
	scanf("%d", &kvd_point_B.x);
	printf("enter the y-coordinate of point B: ");
	scanf("%d", &kvd_point_B.y);

	printf("\n\n");

	printf("enter the x-coordinate of point C: ");
	scanf("%d", &kvd_point_C.x);
	printf("enter the y-coordinate of point C: ");
	scanf("%d", &kvd_point_C.y);

	printf("\n\n");

	printf("enter the x-coordinate of point D: ");
	scanf("%d", &kvd_point_D.x);
	printf("enter the y-coordinate of point D: ");
	scanf("%d", &kvd_point_D.y);

	printf("\n\n");

	printf("enter the x-coordinate of point E: ");
	scanf("%d", &kvd_point_E.x);
	printf("enter the y-coordinate of point E: ");
	scanf("%d", &kvd_point_E.y);

	printf("\n\n");

	printf("points you entered:\n\n");
	printf("\tA :: (%d, %d)\n", kvd_point_A.x, kvd_point_A.y);
	printf("\tB :: (%d, %d)\n", kvd_point_B.x, kvd_point_B.y);
	printf("\tC :: (%d, %d)\n", kvd_point_C.x, kvd_point_C.y);
	printf("\tD :: (%d, %d)\n", kvd_point_D.x, kvd_point_D.y);
	printf("\tE :: (%d, %d)\n", kvd_point_E.x, kvd_point_E.y);
	
	printf("\n\n");
	return(0);
}
