#include <stdio.h>
#include <conio.h>

int main(void)
{
	// function prototypes
	char toupper(char);

	// variable declarations
	char kvd_c, kvd_c_i;
	unsigned int kvd_ascii_c = 0;

	// code
	printf("\n\n");
	printf("enter the first character of your name: ");
	kvd_c = getch();
	printf("%c\n\n", kvd_c);

	kvd_c = (kvd_c < 97) ? kvd_c : kvd_c - 32;

	for (kvd_c_i = 'A'; kvd_c_i <= 'Z'; kvd_c_i++)
	{
		if (kvd_c == kvd_c_i)
		{
			kvd_ascii_c = (unsigned int)kvd_c;
			goto print_result;  // direct jump to label: print_result
		}
	}

	printf("\n\n");
	printf("you did not enter an english alphabet\n\n");
	return 0;

print_result:
	printf("ascii value of '%c' = %u\n\n", kvd_c, kvd_ascii_c);
	return 0;
}

char toupper(char kvd_c)
{
	// code
	if (kvd_c <= 97)
		return kvd_c;
	else
		return kvd_c - 32;
}
