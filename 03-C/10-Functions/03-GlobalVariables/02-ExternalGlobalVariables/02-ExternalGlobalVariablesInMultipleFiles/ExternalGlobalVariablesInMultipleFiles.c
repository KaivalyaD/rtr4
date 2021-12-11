#include <stdio.h>  // for printf()

// declaration (origin) of a global variable
int kvd_global_count = 0;

int main(int kvd_argc, char* kvd_argv[], char* kvd_envp[])
{
	// function prototypes
	void change_count(void);  // defined in this file
	void change_count_one(void);  // defined in File_01.c
	void change_count_two(void);  // defined in File_02.c

	// code
	printf("\n");

	change_count();
	change_count_one();
	change_count_two();

	return(0);
}

void change_count(void)
{
	// code
	kvd_global_count = kvd_global_count + 1;
	printf("change_count(): value of kvd_global_count = %d\n", kvd_global_count);
}
