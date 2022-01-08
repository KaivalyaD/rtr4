#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_num = 23;

	// kvd_ptr (reading from the right to the left) "is a constant pointer to an integer variable"
	int *const kvd_ptr = &kvd_num;

	// code
	printf("\n\n");

	printf("kvd_num = %d\n", kvd_num);
	printf("kvd_ptr = 0x%p\n\n", kvd_ptr);

	// no errors in the following line
	kvd_num++;
	printf("after kvd_num++\n\n");
	printf("\tkvd_num = %d\n", kvd_num);
	printf("\tkvd_ptr = 0x%p\n\n", kvd_ptr);

	// the following line IS an error: kvd_ptr is a 'constant' pointer, meaning
	// once it is assigned any value, reassignment is stricty prohibited. Since
	// it is itself a constant, it can point to one and only one integer throughout
	// throughout its life, but, the integer pointed to can change value directly or
	// even indirectly, i.e. through kvd_ptr, because kvd_ptr is a pointer to a variable,
	// and not a constant integer

	/* kvd_ptr++; */

	// again, no errors here as *kvd_ptr is not a constant
	(*kvd_ptr)++;

	printf("after (*kvd_ptr)++:\n\n");
	printf("\tkvd_ptr = 0x%p\n", kvd_ptr);
	printf("\t*kvd_ptr = %d\n\n", *kvd_ptr);

	return(0);
}
