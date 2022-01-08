#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_num = 91;

	// kvd_ptr is a (reading from the right to the left), "constant pointer to an integer constant"
	const int *const kvd_ptr = &kvd_num;

	// code
	printf("\n\n");

	printf("kvd_num = %d\n", kvd_num);
	printf("kvd_ptr = 0x%p\n\n", kvd_ptr);

	// the following line produces no error because kvd_num isn't a constant
	kvd_num++;
	printf("after kvd_num++:\n\n");
	printf("\tkvd_num = %d\n", kvd_num);
	printf("\tkvd_ptr = %p\n\n", kvd_ptr);

	// the following lines ARE BOTH erroneous: kvd_ptr is itself a constant pointer
	// and one that points to a constant integer. This means, you can neither change
	// the value of kvd_ptr, nor change the value pointed to by kvd_ptr using kvd_ptr.
	
	/* kvd_ptr++; */
	/* (*kvd_ptr)++; */

	return(0);
}
