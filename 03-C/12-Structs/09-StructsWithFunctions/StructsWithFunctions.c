#include <stdio.h>

struct Data
{
	int i;
	float f;
	double d;
	char c;
};

int main(void)
{
	// function prototype
	struct Data AddThreeDataStructs(struct Data, struct Data, struct Data);

	// variable declarations
	struct Data kvd_data1, kvd_data2, kvd_data3, kvd_data_answer;

	// code
	printf("\n\n");
	printf("***** data 1 *****\n\n");
	printf("\tenter an integer: ");
	scanf("%d", &kvd_data1.i);
	printf("\tenter a floating point value: ");
	scanf("%f", &kvd_data1.f);
	printf("\tenter a double precision floating point value: ");
	scanf("%lf", &kvd_data1.d);
	getchar();  // for the last remaining new-line character in keyboard buffer
	printf("\tenter a character: ");
	kvd_data1.c = getchar();

	printf("\n\n");
	printf("**** data 2 *****\n\n");
	printf("\tenter an integer: ");
	scanf("%d", &kvd_data2.i);
	printf("\tenter a floating point value: ");
	scanf("%f", &kvd_data2.f);
	printf("\tenter a double precision floating point value: ");
	scanf("%lf", &kvd_data2.d);
	getchar();
	printf("\tenter a character: ");
	kvd_data2.c = getchar();

	printf("\n\n");
	printf("***** data 3 *****\n\n");
	printf("\tenter an integer: ");
	scanf("%d", &kvd_data3.i);
	printf("\tenter a floating point value: ");
	scanf("%f", &kvd_data3.f);
	printf("\tenter a double precision floating point value: ");
	scanf("%lf", &kvd_data3.d);
	getchar();
	printf("\tenter a character: ");
	kvd_data3.c = getchar();

	kvd_data_answer = AddThreeDataStructs(kvd_data1, kvd_data2, kvd_data3);

	printf("\n\n");
	printf("***** answer *****\n\n");
	printf("kvd_data_answer.i = %d\n", kvd_data_answer.i);
	printf("kvd_data_answer.f = %f\n", kvd_data_answer.f);
	printf("kvd_data_answer.d = %lf\n", kvd_data_answer.d);
	printf("kvd_data_answer.c = '%c'\n", kvd_data_answer.c);

	printf("\n\n");
	return(0);
}

struct Data AddThreeDataStructs(struct Data kvd_data1, struct Data kvd_data2, struct Data kvd_data3)
{
	// variable declarations
	struct Data kvd_data_answer;

	kvd_data_answer.i = kvd_data1.i + kvd_data2.i + kvd_data3.i;
	kvd_data_answer.f = kvd_data1.f + kvd_data2.f + kvd_data3.f;
	kvd_data_answer.d = kvd_data1.d + kvd_data2.d + kvd_data3.d;
	
	return kvd_data_answer;
}
