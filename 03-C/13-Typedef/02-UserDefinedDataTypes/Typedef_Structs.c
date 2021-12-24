#include <stdio.h>

#define MAX_NAME_LENGTH 100

struct Employee
{
	char name[MAX_NAME_LENGTH];
	unsigned int age;
	char sex;
	double salary;
};

struct MyData
{
	int i;
	float f;
	double d;
	char c;
};

int main(void)
{
	// typedefs
	typedef struct Employee MY_EMPLOYEE_TYPE;
	typedef struct MyData MY_DATA_TYPE;

	// variable declarations
	struct Employee kvd_emp = { "Funny", 25, 'M', 10000.00 };
	MY_EMPLOYEE_TYPE kvd_emp_typedef = { "Bunny", 23, 'F', 20400.00 };

	struct MyData kvd_md = { 30, 11.45f, 28.041977, 'm' };
	MY_DATA_TYPE kvd_md_typedef;

	// code
	kvd_md_typedef.i = 9;
	kvd_md_typedef.f = 1.5f;
	kvd_md_typedef.d = 5.011973;
	kvd_md_typedef.c = 'v';

	printf("\n\n");
	printf("struct Employee:\n\n");
	printf("\tkvd_emp.name = %s\n", kvd_emp.name);
	printf("\tkvd_emp.age = %d\n", kvd_emp.age);
	printf("\tkvd_emp.sex = %c\n", kvd_emp.sex);
	printf("\tkvd_emp.salary = %lf\n", kvd_emp.salary);

	printf("\n\n");
	printf("MY_EMPLOYEE_TYPE:\n\n");
	printf("\tkvd_emp_typedef.name = %s\n", kvd_emp_typedef.name);
	printf("\tkvd_emp_typedef.age = %d\n", kvd_emp_typedef.age);
	printf("\tkvd_emp_typedef.sex = %c\n", kvd_emp_typedef.sex);
	printf("\tkvd_emp_typedef.salary = %lf\n", kvd_emp_typedef.salary);

	printf("\n\n");
	printf("struct MyData:\n\n");
	printf("\tkvd_md.i = %d\n", kvd_md.i);
	printf("\tkvd_md.f = %f\n", kvd_md.f);
	printf("\tkvd_md.d = %lf\n", kvd_md.d);
	printf("\tkvd_md.c = '%c'\n", kvd_md.c);

	printf("\n\n");
	printf("MY_DATA_TYPE:\n\n");
	printf("\tkvd_md_typedef.i = %d\n", kvd_md_typedef.i);
	printf("\tkvd_md_typedef.f = %f\n", kvd_md_typedef.f);
	printf("\tkvd_md_typedef.d = %lf\n", kvd_md_typedef.d);
	printf("\tkvd_md_typedef.c = '%c'\n", kvd_md_typedef.c);

	printf("\n\n");
	return(0);
}
