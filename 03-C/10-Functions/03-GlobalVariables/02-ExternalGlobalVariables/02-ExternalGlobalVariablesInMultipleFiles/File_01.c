#include <stdio.h>  // for printf()

/* This is how to declare an external global variable, as a variable,
   global to a file. */

extern int kvd_global_count;

void change_count_one(void)
{
	// code
	kvd_global_count = kvd_global_count + 1;
	printf("change_count_one(): value of kvd_global_count in File_01.c = %d\n", kvd_global_count);
}
