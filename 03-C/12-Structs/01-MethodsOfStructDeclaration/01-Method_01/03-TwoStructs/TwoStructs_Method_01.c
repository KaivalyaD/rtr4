#include <stdio.h>
#include <string.h>

// defining struct 1
struct MyPoint
{
	int x;
	int y;
} kvd_point;  // declaring a variable

// defining struct 2
struct MyPointProperties
{
	int quadrant;
	char axis_location[10];
} kvd_point_properties;  // declaring a variable

int main(void)
{
	// code
	// initializing struct members through user input
	printf("\n\n");
	printf("enter the x-coordinate of a point: ");
	scanf("%d", &kvd_point.x);
	printf("enter the y-coordinte of the point: ");
	scanf("%d", &kvd_point.y);

	printf("\n\n");
	printf("the point you entered: (%d, %d):\n\n", kvd_point.x, kvd_point.y);

	if (kvd_point.x == 0 && kvd_point.y == 0)
		printf("\twhich is the origin!\n");
	else
	{
		if (kvd_point.x == 0)
		{
			if (kvd_point.y > 0)
				strcpy(kvd_point_properties.axis_location, "positive Y");
			else if(kvd_point.y < 0)
				strcpy(kvd_point_properties.axis_location, "negative Y");

			kvd_point_properties.quadrant = 0;
			printf("\twhich lies on the %s axis\n", kvd_point_properties.axis_location);
		}
		else if (kvd_point.y == 0)
		{
			if (kvd_point.x > 0)
				strcpy(kvd_point_properties.axis_location, "positive X");
			else if (kvd_point.x < 0)
				strcpy(kvd_point_properties.axis_location, "negative X");
			
			kvd_point_properties.quadrant = 0;
			printf("\twhich lies on the %s axis\n", kvd_point_properties.axis_location);
		}
		else
		{
			kvd_point_properties.axis_location[0] = '\0';

			if (kvd_point.x > 0 && kvd_point.y > 0)
				kvd_point_properties.quadrant = 1;
			else if (kvd_point.x < 0 && kvd_point.y > 0)
				kvd_point_properties.quadrant = 2;
			else if (kvd_point.x < 0 && kvd_point.y < 0)
				kvd_point_properties.quadrant = 3;
			else
				kvd_point_properties.quadrant = 4;

			printf("\twhich lies in quadrant number %d", kvd_point_properties.quadrant);
		}
	}

	printf("\n\n");
	return(0);
}
