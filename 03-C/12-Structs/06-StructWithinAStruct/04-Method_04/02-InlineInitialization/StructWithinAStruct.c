#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_length, kvd_breadth, kvd_area;

	struct Point
	{
		int x, y;
	};

	struct Rectangle
	{
		struct Point left_top, right_bottom;
	} kvd_rect = { {3, -4}, {5, -6} };

	// code

	kvd_length = kvd_rect.right_bottom.x - kvd_rect.left_top.x;
	if (kvd_length < 0)
		kvd_length = -kvd_length;

	kvd_breadth = kvd_rect.left_top.y - kvd_rect.right_bottom.y;
	if (kvd_breadth < 0)
		kvd_breadth = -kvd_breadth;

	kvd_area = kvd_length * kvd_breadth;

	printf("\n\n");
	printf("length of the rectangle = %d\n", kvd_length);
	printf("breadth of the rectangle = %d\n", kvd_breadth);
	printf("area of the rectangle = %d\n\n", kvd_area);

	return(0);
}
