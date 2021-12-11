#include <stdio.h>  // for printf()

int main(int kvd_argc, char* kvd_argv[], char* kvd_envp[])
{
	// function prototypes
	void function_country(void);

	// code
	function_country();

	return(0);
}

// definitions of all user-defined functions
void function_country(void)
{
	// function prototypes
	void function_ofAMC(void);

	// code
	function_ofAMC();  // starting a chain of function calls, showing that calls are pushed onto a stack

	printf("\n\n");

	printf("I am a resident of India.\n\n");
}

void function_ofAMC(void)
{
	// function prototypes
	void function_lastname(void);

	// code
	function_lastname();

	printf("\n\n");

	printf("of AstroMediComp.");
}

void function_lastname(void)
{
	// function prototypes
	void function_middlename(void);

	// code
	function_middlename();

	printf("\n\n");

	printf("Deshpande");
}

void function_middlename(void)
{
	// function prototypes
	void function_firstname(void);

	// code
	function_firstname();

	printf("\n\n");

	printf("Vishwakumar");
}

void function_firstname(void)
{
	// function prototypes
	void function_is(void);

	// code
	function_is();

	printf("\n\n");

	printf("Kaivalya");
}

void function_is(void)
{
	// function prototypes
	void function_name(void);

	// code
	function_name();

	printf("\n\n");

	printf("is");
}

void function_name(void)
{
	// function prototypes
	void function_My(void);

	// code
	function_My();

	printf("\n\n");

	printf("name");
}

void function_My(void)
{
	// function prototypes
	
	// code
	printf("\n\n");

	printf("My");

	// last function in the call chain, whose output appears first, it being on
	// the top of the stack
}
