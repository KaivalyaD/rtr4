#include <stdio.h>	// for printf()
#include <conio.h>	// for getch()

// ASCII value bounds for the upper-case alphabet ('A' to 'Z': 65 to 90)
#define CHAR_ALPHABET_UPPER_CASE_BEGINNING	65
#define CHAR_ALPHABET_UPPER_CASE_END	90

// ASCII value bounds for the lower-case alphabet ('a' to 'z': 97 to 122)
#define CHAR_ALPHABET_LOWER_CASE_BEGINNING	97
#define CHAR_ALPHABET_LOWER_CASE_END	122

// ASCII value bounds for decimal digits '0' to '9'
#define CHAR_DIGIT_BEGINNING	48
#define CHAR_DIGIT_END	57

int main(void)
{
	// variable declarations
	char kvd_ch;
	int kvd_ch_value;

	// code
	printf("\n\n");

	printf("enter a character: ");
	kvd_ch = getch();

	printf("\n");

	switch (kvd_ch)
	{
	case 'A':	// fall through as a blessing
	case 'a':

	case 'E':
	case 'e':

	case 'I':
	case 'i':

	case 'O':
	case 'o':

	case 'U':
	case 'u':
		printf("'%c' is a vowel\n\n", kvd_ch);
		break;

	default:
		kvd_ch_value = (int)kvd_ch;

		// if the character has its ASCII code between 65 and 90, or
		// between 97 and 122, then it is a consonent
		if ( (kvd_ch_value >= CHAR_ALPHABET_LOWER_CASE_BEGINNING && kvd_ch_value <= CHAR_ALPHABET_LOWER_CASE_END)
			 || (kvd_ch_value >= CHAR_ALPHABET_UPPER_CASE_BEGINNING && kvd_ch_value <= CHAR_ALPHABET_UPPER_CASE_END) )
		{
			printf("'%c' is a consonent\n\n", kvd_ch);
		}
		// if it lies between 48 and 57, it is a digit
		else if ((kvd_ch_value >= CHAR_DIGIT_BEGINNING) && (kvd_ch_value <= CHAR_DIGIT_END))
		{
			printf("'%c' is a digit\n\n", kvd_ch);
		}
		// otherwise, it is a special character
		else
		{
			printf("'%c' is a special character\n\n", kvd_ch);
		}
		break;
	}

	printf("switch block completed\n\n");

	return(0);
}
