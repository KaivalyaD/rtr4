#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_num;

	// code
	printf("\n\n");
	printf("enter a value for kvd_num: ");
	scanf("%d", &kvd_num);

	if (kvd_num < 0) // if-01
	{
		printf("kvd_num is negative\n\n", kvd_num);
	}
	else // else-01
	{
		if ((kvd_num > 0) && (kvd_num <= 100)) // if-02
		{
			printf("kvd_num (= %d) lies between 0 and 100\n\n", kvd_num);
		}
		else // else-02
		{
			if ((kvd_num > 100) && (kvd_num <= 200)) // if-03
			{
				printf("kvd_num (= %d) lies between 100 and 200\n\n", kvd_num);
			}
			else // else-03
			{
				if ((kvd_num > 200) && (kvd_num <= 300)) // if-04
				{
					printf("kvd_num (= %d) lies between 200 and 300\n\n", kvd_num);
				}
				else // else-04
				{
					if ((kvd_num > 300) && (kvd_num <= 400)) // if-05
					{
						printf("kvd_num (= %d) lies between 300 and 400\n\n", kvd_num);
					}
					else // else-05
					{
						if ((kvd_num > 400) && (kvd_num <= 500)) // if-06
						{
							printf("kvd_num (= %d) lies between 400 and 500\n\n", kvd_num);
						}
						else // else-06
						{
							printf("kvd_num (= %d) is greater than 500\n\n", kvd_num);
						} // closing else-06

					} // closing else-05
				
				} // closing else-04
			
			} // closing else-03
		
		} // closing else-02
	
	} // closing else-01

	return(0);
}
