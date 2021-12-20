#include <stdio.h>
#include <ctype.h>  // for toupper()

#define KVD_MAX_STRING_LENGTH 512

typedef enum _BOOL
{
	FALSE,
	TRUE
} BOOL;

int main(void)
{
	// function prototypes
	int MyStrlen(char[]);

	// variable declarations
	char kvd_cArray_Original[KVD_MAX_STRING_LENGTH], kvd_cArray_Capitalized[KVD_MAX_STRING_LENGTH];
	int kvd_iStringLength = 0, kvd_i;
	BOOL kvd_bWordBegins = TRUE;

	// code
	printf("\n\n");

	printf("enter a string: ");
	gets_s(kvd_cArray_Original, KVD_MAX_STRING_LENGTH);

	kvd_iStringLength = MyStrlen(kvd_cArray_Original);

	for (kvd_i = 0; kvd_i < kvd_iStringLength; kvd_i++)
	{
		if (kvd_cArray_Original[kvd_i] == '.' || kvd_cArray_Original[kvd_i] == ' ' || kvd_cArray_Original[kvd_i] == ',' || kvd_cArray_Original[kvd_i] == '?' || kvd_cArray_Original[kvd_i] == '!')
		{
			kvd_cArray_Capitalized[kvd_i] = kvd_cArray_Original[kvd_i];
			kvd_bWordBegins = TRUE;
		}
		else
		{
			if (kvd_bWordBegins)
			{
				kvd_cArray_Capitalized[kvd_i] = toupper(kvd_cArray_Original[kvd_i]);
				kvd_bWordBegins = FALSE;
			}
			else
				kvd_cArray_Capitalized[kvd_i] = kvd_cArray_Original[kvd_i];
		}
	}
	kvd_cArray_Capitalized[kvd_i] = '\0';

	printf("\n\n");

	printf("you entered:\n\n");
	printf("\t\"%s\"\n\n", kvd_cArray_Original);

	printf("capitalized string:\n\n");
	printf("\t\"%s\"\n", kvd_cArray_Capitalized);

	printf("\n\n");
	return(0);
}

int MyStrlen(char kvd_str[])
{
	// variable declarations
	int kvd_string_length = 0;

	// code
	while (kvd_str[kvd_string_length])
		kvd_string_length++;

	return(kvd_string_length);
}
