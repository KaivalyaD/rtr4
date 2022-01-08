#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_num = 82;

	// reading from the right to the left:
	// kvd_vptr_cint is a "variable pointer to an integer constant"
	const int *kvd_ptr = NULL;

	// code
	kvd_ptr = &kvd_num;
	
	printf("\n\n");
	printf("kvd_num = %d\n", kvd_num);
	printf("kvd_ptr = &kvd_num = 0x%p\n\n", kvd_ptr);

	// the following lines aren't erroneous because the value pointed to
	// by kvd_ptr isn't being modified through kvd_ptr
	kvd_num++;
	printf("after kvd_num++, kvd_num = %d\n\n", kvd_num);

	// the next line generates a compile-time error because:
	// kvd_ptr is a pointer variable, but one that points to an integer constant -
	// meaning, it does not care if the value it points to is declared a constant or not,
	// and ALWAYS treats it as constant. When one tries to access the value through
	// such a pointer, the compiler detects the violation, generates an error, and refuses
	// to create the object file.
	
	/* (*kvd_ptr)++; */

	// but this next line will not cause any problems, because kvd_ptr is still
	// a variable
	kvd_ptr++;

	printf("after kvd_ptr++, kvd_ptr = 0x%p\n", kvd_ptr);
	printf("*kvd_ptr = %d\n\n", *kvd_ptr);

	/**
	* In short, we cannot change the "value at address of" kvd_ptr through kvd_ptr,
	* but we can change it without using kvd_ptr.
	*/

	return(0);
}
