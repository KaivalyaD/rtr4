#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#define LENGTH_NAME 100
#define MARITAL_STATUS 10

struct Employee
{
	char name[LENGTH_NAME];
	int age;
	char sex, marital_status;
	float salary;
};

int main(void)
{
	// function prototypes
	void MyGets(char *, int);

	// variable declarations
	struct Employee *kvd_pEmployeeRecord = NULL;
	int kvd_employee_count, kvd_i;

	// code
	printf("\n\n");
	printf("enter the number of employees: ");
	scanf("%d", &kvd_employee_count);

	printf("\n\n");
	kvd_pEmployeeRecord = (struct Employee *)malloc(sizeof(struct Employee) * kvd_employee_count);
	if (!kvd_pEmployeeRecord)
	{
		printf("malloc(): failed to allocate memory for %d struct Employee-s\n\n", kvd_employee_count);
		return 1;
	}
	printf("memory allocated successfully\n\n");

	for (kvd_i = 0; kvd_i < kvd_employee_count; kvd_i++)
	{
		printf("data entry for employee %d\n\n", kvd_i + 1);

		printf("\tname: ");
		MyGets((kvd_pEmployeeRecord + kvd_i)->name, LENGTH_NAME);

		printf("\tage: ");
		scanf("%d", &((kvd_pEmployeeRecord + kvd_i)->age));

		getchar();
		printf("\tsex (M/m for male and F/f for female, anything else for none): ");
		(kvd_pEmployeeRecord + kvd_i)->sex = getchar();

		printf("\tsalary in INR: ");
		scanf("%f", &((kvd_pEmployeeRecord + kvd_i)->salary));

		getchar();
		printf("\tmarital status (Y/y for married, anything else for unmarried): ");
		(kvd_pEmployeeRecord + kvd_i)->marital_status = getchar();

		printf("\n\n");
	}

	printf("\n\n");
	printf("all employees you entered:\n\n");
	for (kvd_i = 0; kvd_i < kvd_employee_count; kvd_i++)
	{
		printf("employee %d:\n\n", kvd_i);
		printf("\tname: %s\n", (kvd_pEmployeeRecord + kvd_i)->name);
		printf("\tage: %d\n", (kvd_pEmployeeRecord + kvd_i)->age);
		printf("\tsex: ");
		if ((kvd_pEmployeeRecord + kvd_i)->sex == 'M' || (kvd_pEmployeeRecord + kvd_i)->sex == 'm')
			printf("male\n");
		else if ((kvd_pEmployeeRecord + kvd_i)->sex == 'F' || (kvd_pEmployeeRecord + kvd_i)->sex == 'f')
			printf("female\n");
		else
			printf("other\n");
		printf("\tsalary: %f\n", (kvd_pEmployeeRecord + kvd_i)->salary);
		printf("\tmarital status: ");
		if ((kvd_pEmployeeRecord + kvd_i)->marital_status == 'Y' || (kvd_pEmployeeRecord + kvd_i)->marital_status == 'y')
			printf("married\n\n");
		else
			printf("unmarried\n\n");
	}

	if (kvd_pEmployeeRecord)
	{
		free(kvd_pEmployeeRecord);
		kvd_pEmployeeRecord = NULL;

		printf("freed and cleaned all dynamically allocated memory\n\n");
	}

	return 0;
}

// a rudimentary gets_s-like implementation
// reason: gets_s(), fgets(), and fscanf() work differently on different platforms
// warning: backspace (char deletion), carat movements are not implemented
void MyGets(char *kvd_str, int kvd_str_size)
{
	// variable declarations
	int kvd_i;
	char kvd_c = '\0';

	// code
	kvd_i = 0;
	do
	{
		kvd_c = getch();
		kvd_str[kvd_i] = kvd_c;
		printf("%c", kvd_c);
		kvd_i++;
	} while (kvd_c != '\r' && kvd_i < kvd_str_size);

	printf("\n");

	if (kvd_i == kvd_str_size)
		kvd_str[kvd_i - 1] = '\0';
	else
		kvd_str[kvd_i] = '\0';
}
