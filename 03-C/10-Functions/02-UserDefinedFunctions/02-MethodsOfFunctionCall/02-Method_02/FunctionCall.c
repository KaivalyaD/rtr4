#include <stdio.h>  // for printf()

int main(int kvd_argc, char* kvd_argv[], char* kvd_envp[])
{
	// function prototypes
	void display_information(void);
	void function_country(void);

	// code
	display_information();
	function_country();

	printf("\n\n");

	return(0);
}

// user-defined function definitions
void display_information(void)
{
	// function prototypes
	void function_My(void);
	void function_name(void);
	void function_is(void);
	void function_firstname(void);
	void function_middlename(void);
	void function_lastname(void);
	void function_ofAMC(void);

	// calling even more functions within a function called from the
	// entry point -> all these functions and functions called within
	// them will eventually trace their calls to main() and be executed.
	function_My();
	function_name();
	function_is();
	function_firstname();
	function_middlename();
	function_lastname();
	function_ofAMC();
}

void function_My(void)
{
	// code
	printf("\n\n");

	printf("My");
}

void function_name(void)
{
	// code
	printf("\n\n");

	printf("name");
}

void function_is(void)
{
	// code
	printf("\n\n");

	printf("is");
}

void function_firstname(void)
{
	// code
	printf("\n\n");

	printf("Kaivalya");
}

void function_middlename(void)
{
	// code
	printf("\n\n");

	printf("Vishwakumar");
}

void function_lastname(void)
{
	// code
	printf("\n\n");

	printf("Deshpande");
}

void function_ofAMC(void)
{
	// code
	printf("\n\n");

	printf("of AstroMediComp.");
}

void function_country(void)
{
	// code
	printf("\n\n");

	printf("I am a resident of India.");
}
