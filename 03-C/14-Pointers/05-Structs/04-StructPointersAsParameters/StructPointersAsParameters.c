#include <stdio.h>
#include <stdlib.h>

struct Data
{
	int i;
	float f;
	double d;
};

int main(void)
{
	// function prototypes
	void ChangeValues(struct Data *);

	// variable declarations
	struct Data *kvd_pData = NULL;

	// code
	printf("\n\n");

	kvd_pData = (struct Data *)malloc(sizeof(struct Data) * 1);
	if (!kvd_pData)
	{
		printf("malloc(): failed to allocate memory to 1 struct Data\n\n");
		return 1;
	}
	printf("memory allocated successfully\n\n");

	kvd_pData->i = 39;
	kvd_pData->f = 3.141f;
	kvd_pData->d = 892.32748;

	printf("before ChangeData(kvd_pData), members in kvd_pData:\n\n");
	printf("\t->i = %d\n", kvd_pData->i);
	printf("\t->f = %f\n", kvd_pData->f);
	printf("\t->d = %lf\n\n", kvd_pData->d);

	ChangeValues(kvd_pData);

	printf("after ChangeValues(kvd_pData), members in kvd_pData:\n\n");
	printf("\t->i = %d\n", kvd_pData->i);
	printf("\t->f = %f\n", kvd_pData->f);
	printf("\t->d = %lf\n\n", kvd_pData->d);

	if (kvd_pData)
	{
		free(kvd_pData);
		kvd_pData = NULL;

		printf("freed and cleaned all dynamically allocated memory\n\n");
	}

	return 0;
}

void ChangeValues(struct Data *kvd_pData)
{
	// code
	kvd_pData->i = 26;			// (*kvd_pData).i = 26;
	kvd_pData->f = 2.71f;		// (*kvd_pData).f = 2.71f;
	kvd_pData->d = 23.88923;	// (*kvd_pData).d = 23.88923;
}
