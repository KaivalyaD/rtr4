#include <stdio.h>
#include <ctype.h>

#define NUM_EMPLOYEES 5
#define MAX_NAME_LENGTH 100

struct Employee
{
	char name[MAX_NAME_LENGTH];
	int age;
	char sex;
	float salary;
	char marital_status;
};

int main(void)
{
	// function prototypes
	void MyGets(char[], int);

	// variable declarations
	struct Employee kvd_emp_records[NUM_EMPLOYEES];
	int kvd_i;

	// code
	printf("\n\n");

	// user-input
	for (kvd_i = 0; kvd_i < NUM_EMPLOYEES; kvd_i++)
	{
		printf("Data Entry for Employee %d\n\n", kvd_i + 1);

		printf("\temployee's name: ");
		MyGets(kvd_emp_records[kvd_i].name, MAX_NAME_LENGTH);

		printf("\temployee's age: ");
		scanf("%d", &kvd_emp_records[kvd_i].age);

		getchar();
		printf("\temployee's sex (M/m for Male, F/f for Female, anything else for others): ");
		kvd_emp_records[kvd_i].sex = getchar();
		
		printf("\temployee's salary: ");
		scanf("%f", &kvd_emp_records[kvd_i].salary);

		getchar();
		printf("\temployee's marital status (Y/y for married, and anything except Y/y otherwise): ");
		kvd_emp_records[kvd_i].marital_status = getchar();

		getchar();
		printf("\n\n");
	}

	// display
	printf("\n\n");
	printf("displaying all employee records\n\n");
	for (kvd_i = 0; kvd_i < NUM_EMPLOYEES; kvd_i++)
	{
		printf("\temployee number %d\n\n", kvd_i + 1);
		printf("\t\tname: %s\n", kvd_emp_records[kvd_i].name);
		printf("\t\tage: %d\n", kvd_emp_records[kvd_i].age);

		if (kvd_emp_records[kvd_i].sex == 'M' || kvd_emp_records[kvd_i].sex == 'm')
			printf("\t\tsex: male\n");
		else if (kvd_emp_records[kvd_i].sex == 'F' || kvd_emp_records[kvd_i].sex == 'f')
			printf("\t\tsex: female\n");
		else
			printf("\t\tsex: other\n");

		printf("\t\tsalary: %.2f\n", kvd_emp_records[kvd_i].salary);

		if (kvd_emp_records[kvd_i].marital_status == 'Y' || kvd_emp_records[kvd_i].marital_status == 'y')
			printf("\t\tmarital status: married\n");
		else
			printf("\t\tmarital status: unmarried\n");

		printf("\n\n");
	}

	return(0);
}

void MyGets(char kvd_str[], int kvd_str_size)
{
	// variable declarations
	int kvd_i;
	char kvd_ch = '\0';

	// code
	kvd_i = 0;

	while ((kvd_ch != '\n') && (kvd_i < kvd_str_size))
	{
		kvd_ch = getchar();
		kvd_str[kvd_i] = kvd_ch;
		kvd_i++;
	}

	kvd_str[kvd_i - 1] = '\0';
}
