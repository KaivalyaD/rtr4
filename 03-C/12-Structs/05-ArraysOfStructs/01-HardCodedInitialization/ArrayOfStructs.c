#include <stdio.h>
#include <string.h>

#define NAME_LENGTH 100
#define MARITAL_STATUS 10

struct Employee
{
	char name[NAME_LENGTH];
	int age;
	float salary;
	char sex;
	char marital_status[MARITAL_STATUS];
};

int main(void)
{
	// variable declarations
	struct Employee kvd_emp_record[5];  // an array of 5 struct Employee-s

	char employee_1[] = "Saayujya";
	char employee_2[] = "Kaivalya";
	char employee_3[] = "Mukta";
	char employee_4[] = "Vishwakumar";
	char employee_5[] = "Veer";

	int kvd_i;

	// code
	// employee 1
	strcpy(kvd_emp_record[0].name, employee_1);
	kvd_emp_record[0].age = 24;
	kvd_emp_record[0].sex = 'M';
	kvd_emp_record[0].salary = 50000.00f;
	strcpy(kvd_emp_record[0].marital_status, "unmarried");

	// employee 2
	strcpy(kvd_emp_record[1].name, employee_2);
	kvd_emp_record[1].age = 30;
	kvd_emp_record[1].sex = 'M';
	kvd_emp_record[1].salary = 60000.00f;
	strcpy(kvd_emp_record[1].marital_status, "unmarried");

	// employee 3
	strcpy(kvd_emp_record[2].name, employee_3);
	kvd_emp_record[2].age = 40;
	kvd_emp_record[2].sex = 'F';
	kvd_emp_record[2].salary = 102000.00f;
	strcpy(kvd_emp_record[2].marital_status, "married");

	// employee 4
	strcpy(kvd_emp_record[3].name, employee_4);
	kvd_emp_record[3].age = 47;
	kvd_emp_record[3].sex = 'M';
	kvd_emp_record[3].salary = 150000.00f;
	strcpy(kvd_emp_record[3].marital_status, "married");

	// employee 5
	strcpy(kvd_emp_record[4].name, employee_5);
	kvd_emp_record[4].age = 27;
	kvd_emp_record[4].sex = 'M';
	kvd_emp_record[4].salary = 47000.00f;
	strcpy(kvd_emp_record[4].marital_status, "unmarried");

	// display
	printf("\n\n");

	printf("displaying employee records:\n\n");
	for (kvd_i = 0; kvd_i < 5; kvd_i++)
	{
		printf("\temployee number %d\n\n", kvd_i + 1);
		printf("\t\tname: %s\n", kvd_emp_record[kvd_i].name);
		printf("\t\tage: %d\n", kvd_emp_record[kvd_i].age);
		
		if (kvd_emp_record[kvd_i].sex == 'M' || kvd_emp_record[kvd_i].sex == 'm')
			printf("\t\tsex: Male\n");
		else if (kvd_emp_record[kvd_i].sex == 'F' || kvd_emp_record[kvd_i].sex == 'f')
			printf("\t\tsex: Female\n");

		printf("\t\tsalary: %.2f\n", kvd_emp_record[kvd_i].salary);
		printf("\t\tmarital status: %s\n", kvd_emp_record[kvd_i].marital_status);

		printf("\n\n");
	}

	return(0);
}
