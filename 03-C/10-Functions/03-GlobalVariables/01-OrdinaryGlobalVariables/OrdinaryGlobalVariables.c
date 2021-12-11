#include <stdio.h>  // for printf()

/*
 * If not explicitly initialized, global variables are initialized
 * by default to their null values (0 for int, 0.0f for float, etc.).
 *
 * However, it remains a good practice to initialize any variable,
 * irrespective of its scope, once it is declared.
 */

 /* this scope is global */

int kvd_global_count = 0;

int main(int kvd_argc, char* kvd_argv[], char* kvd_envp[])
{
	/* this scope is local to main() */

	// function prototypes
	void change_count_one(void);
	void change_count_two(void);
	void change_count_three(void);

	// code
	printf("\n\n");

	printf("main(): value of kvd_global_count = %d\n", kvd_global_count);

	change_count_one();
	change_count_two();
	change_count_three();

	printf("\n");
	return(0);
}

/* this scope is once again, global */

void change_count_one(void)
{
	/* this scope is local to change_count_one()  */

	// code
	kvd_global_count = kvd_global_count + 1;
	printf("change_count_one(): value of kvd_global_count = %d\n", kvd_global_count);
}

/* global again */

void change_count_two(void)
{
	/* scope local to change_count_two */

	// code
	kvd_global_count = kvd_global_count + 2;
	printf("change_count_two(): value of kvd_global_count = %d\n", kvd_global_count);
}

/* global, yet again */

void change_count_three(void)
{
	/* scope local to change_count_three */

	// code
	kvd_global_count = kvd_global_count + 3;
	printf("change_count_three(): value of kvd_global_count = %d\n", kvd_global_count);
}

/* global forever! ( atleast until you write another block ;) ) */
