#include <stdio.h>

struct Point
{
	int x, y;
};

struct Rectangle
{
	struct Point left_top, right_bottom;
};

int main(void)
{
	// variable declarations
	int kvd_length, kvd_breadth, kvd_area;
	struct Rectangle kvd_rect;

	// code
	printf("\n\n");

	printf("enter the left-most x-coordinate for the rectangle: ");
	scanf("%d", &kvd_rect.left_top.x);
	printf("enter the top-most y-coordinate for the rectangle: ");
	scanf("%d", &kvd_rect.left_top.y);

	printf("\n\n");
	printf("enter the right-most x-coordinate for the rectangle: ");
	scanf("%d", &kvd_rect.right_bottom.x);
	printf("enter the right-most y-coordinate for the rectangle: ");
	scanf("%d", &kvd_rect.right_bottom.y);

	kvd_length = kvd_rect.right_bottom.x - kvd_rect.left_top.x;
	if (kvd_length < 0)
		kvd_length = -1 * kvd_length;

	kvd_breadth = kvd_rect.left_top.y - kvd_rect.right_bottom.y;
	if (kvd_breadth < 0)
		kvd_breadth = -1 * kvd_breadth;

	kvd_area = kvd_length * kvd_breadth;

	printf("\n\n");
	printf("length of the rectangle = %d\n", kvd_length);
	printf("breadth of the rectangle = %d\n", kvd_breadth);
	printf("area of the rectangle = %d\n", kvd_area);

	return(0);
}
