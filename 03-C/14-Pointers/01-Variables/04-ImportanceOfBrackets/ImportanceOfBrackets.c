#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_num, *kvd_ptr = NULL, kvd_ans;

	// code
	kvd_num = 45;
	kvd_ptr = &kvd_num;

	printf("\n\n");
	printf("kvd_num = %d\n", kvd_num);
	printf("&kvd_num = 0x%p\n", &kvd_num);
	printf("*(&kvd_num) = %d\n", *(&kvd_num));
	printf("kvd_ptr = 0x%p\n", kvd_ptr);
	printf("*kvd_ptr = %d\n\n", *kvd_ptr);

	// let's go 10 bytes beyond kvd_ptr in memory! But you cannot always see what is
	// located there (i.e. you cannot always dereference the pointer to it).
	// Again, but! Address is just a number, so there's no problem in changing
	// the value of kvd_ptr.
	printf("kvd_ptr + 10 = %p\n", kvd_ptr + 10);

	// the following line might crash the program because the accessed address may
	// lie outside the process's address space. It is of course, BAD PRACTICE to do this
	// note the positions of '*' and '()'
	printf("*(kvd_ptr + 10) = %d\n", *(kvd_ptr + 10));

	// note again, the positions of '*' and '()'
	printf("(*kvd_ptr + 10) = %d\n\n", (*kvd_ptr + 10));
	
	// this is how NOT to post-increment a variable through its pointer
	// see the brackets! (This line may also crash the program as we're
	// trying to access unallocated memory)
	*kvd_ptr++;
	printf("*kvd_ptr++ = %d\n", *kvd_ptr);

	// this is how to post-increment a variable through its pointer
	// The problem is precedence of ++ over *, and thus the solution
	// is to override the precedence by placement of ()
	(*kvd_ptr)++;
	printf("(*kvd_ptr)++ = %d\n\n", *kvd_ptr);

	return(0);
}
