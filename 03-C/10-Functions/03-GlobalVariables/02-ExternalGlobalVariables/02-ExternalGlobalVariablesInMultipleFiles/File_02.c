#include <stdio.h>  // for printf()

void change_count_two(void)
{
	// code

	/* This is how to declare an external global variable, as a variable,
	   local to a function.

	   [ Although kvd_global_count is declared in local scope, the linker
	   places the address of the global variable in its place, thus making
	   it look like a static variable.
	   
	   BUT there is a difference: this address is allocated not when the
	   function is called for the first time, but as soon as the program
	   is invoked! In other words, kvd_global_count remains a global
	   variable, visible only to the local scope of change_count_two(). ] */

	extern int kvd_global_count;

	kvd_global_count = kvd_global_count + 1;
	printf("change_count_two(): value of kvd_global_count in File_02.c = %d\n", kvd_global_count);
}
