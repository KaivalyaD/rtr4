#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_iArrayOne[10], kvd_iArrayTwo[10];

	// code

	/***** kvd_iArrayOne[] *****/
	kvd_iArrayOne[0] = 3;
	kvd_iArrayOne[1] = 6;
	kvd_iArrayOne[2] = 9;
	kvd_iArrayOne[3] = 12;
	kvd_iArrayOne[4] = 15;
	kvd_iArrayOne[5] = 18;
	kvd_iArrayOne[6] = 21;
	kvd_iArrayOne[7] = 24;
	kvd_iArrayOne[8] = 27;
	kvd_iArrayOne[9] = 30;

	printf("\n\n");
	
	printf("piece-meal (hard-coded) assignment to and display of elements within kvd_iArrayOne[]:\n\n");

	printf("kvd_iArrayOne[0] (1st element) = %d\n", kvd_iArrayOne[0]);
	printf("kvd_iArrayOne[1] (2nd element) = %d\n", kvd_iArrayOne[1]);
	printf("kvd_iArrayOne[2] (3rd element) = %d\n", kvd_iArrayOne[2]);
	printf("kvd_iArrayOne[3] (4th element) = %d\n", kvd_iArrayOne[3]);
	printf("kvd_iArrayOne[4] (5th element) = %d\n", kvd_iArrayOne[4]);
	printf("kvd_iArrayOne[5] (6th element) = %d\n", kvd_iArrayOne[5]);
	printf("kvd_iArrayOne[6] (7th element) = %d\n", kvd_iArrayOne[6]);
	printf("kvd_iArrayOne[7] (8th element) = %d\n", kvd_iArrayOne[7]);
	printf("kvd_iArrayOne[8] (9th element) = %d\n", kvd_iArrayOne[8]);
	printf("kvd_iArrayOne[9] (10th element) = %d\n", kvd_iArrayOne[9]);

	/***** kvd_iArrayTwo[] *****/
	printf("\n\n");

	printf("enter the 1st element into kvd_iArrayTwo[]: ");
	scanf("%d", &kvd_iArrayTwo[0]);
	printf("enter the 2nd element into kvd_iArrayTwo[]: ");
	scanf("%d", &kvd_iArrayTwo[1]);
	printf("enter the 3rd element into kvd_iArrayTwo[]: ");
	scanf("%d", &kvd_iArrayTwo[2]);
	printf("enter the 4th element into kvd_iArrayTwo[]: ");
	scanf("%d", &kvd_iArrayTwo[3]);
	printf("enter the 5th element into kvd_iArrayTwo[]: ");
	scanf("%d", &kvd_iArrayTwo[4]);
	printf("enter the 6th element into kvd_iArrayTwo[]: ");
	scanf("%d", &kvd_iArrayTwo[5]);
	printf("enter the 7th element into kvd_iArrayTwo[]: ");
	scanf("%d", &kvd_iArrayTwo[6]);
	printf("enter the 8th element into kvd_iArrayTwo[]: ");
	scanf("%d", &kvd_iArrayTwo[7]);
	printf("enter the 9th element into kvd_iArrayTwo[]: ");
	scanf("%d", &kvd_iArrayTwo[8]);
	printf("enter the 10th element into kvd_iArrayTwo[]: ");
	scanf("%d", &kvd_iArrayTwo[9]);

	printf("\n\n");

	printf("piece-meal (user-input) assignment to and display of elements to iArrayTwo[]:\n");
	printf("iArrayTwo[0] (1st element) = %d\n", kvd_iArrayTwo[0]);
	printf("iArrayTwo[1] (2nd element) = %d\n", kvd_iArrayTwo[1]);
	printf("iArrayTwo[2] (3rd element) = %d\n", kvd_iArrayTwo[2]);
	printf("iArrayTwo[3] (4th element) = %d\n", kvd_iArrayTwo[3]);
	printf("iArrayTwo[4] (5th element) = %d\n", kvd_iArrayTwo[4]);
	printf("iArrayTwo[5] (6th element) = %d\n", kvd_iArrayTwo[5]);
	printf("iArrayTwo[6] (7th element) = %d\n", kvd_iArrayTwo[6]);
	printf("iArrayTwo[7] (8th element) = %d\n", kvd_iArrayTwo[7]);
	printf("iArrayTwo[8] (9th element) = %d\n", kvd_iArrayTwo[8]);
	printf("iArrayTwo[9] (10th element) = %d\n", kvd_iArrayTwo[9]);

	printf("\n\n");
	return(0);
}
