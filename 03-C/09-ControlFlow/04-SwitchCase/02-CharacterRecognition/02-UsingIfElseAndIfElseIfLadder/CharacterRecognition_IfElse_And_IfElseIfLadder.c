#include <stdio.h> // for printf()
#include <conio.h> // for getch()

// ASCII values for 'A' to 'Z': 65 to 90
#define CHAR_ALPHABET_UPPER_CASE_BEGINNING	65
#define CHAR_ALPHABET_UPPER_CASE_END	90

// ASCII values for 'a' to 'z': 97 to 122
#define	CHAR_ALPHABET_LOWER_CASE_BEGINNING	97
#define CHAR_ALPHABET_LOWER_CASE_END	122

// ASCII values for '0' to '9': 48 to 57
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

	if ((kvd_ch == 'A' || kvd_ch == 'a') || (kvd_ch == 'E' || kvd_ch == 'e')
		|| (kvd_ch == 'I' || kvd_ch == 'i') || (kvd_ch == 'O' || kvd_ch == 'o')
		|| (kvd_ch == 'U' || kvd_ch == 'u'))
	{
		printf("'%c' is a vowel\n\n", kvd_ch);
	}
	else
	{
		kvd_ch_value = (int)kvd_ch;

		// if the character's ASCII value lies between 65 and 90 or
		// between 97 and 122, then the it is a consonent inside this scope
		if ((kvd_ch_value >= CHAR_ALPHABET_LOWER_CASE_BEGINNING && (kvd_ch_value <= CHAR_ALPHABET_LOWER_CASE_END)
			|| (kvd_ch_value >= CHAR_ALPHABET_UPPER_CASE_BEGINNING && (kvd_ch_value <= CHAR_ALPHABET_UPPER_CASE_END))))
		{
			printf("'%c' is a consonent\n\n", kvd_ch);
		}
		// if it lies between 48 and 57, it is a digit character
		else if (kvd_ch_value >= CHAR_DIGIT_BEGINNING && kvd_ch_value <= CHAR_DIGIT_END)
		{
			printf("'%c' is a digit\n\n", kvd_ch);
		}
		// otherwise, it is a special character
		else
		{
			printf("'%c' is a special character\n\n", kvd_ch);
		}
	}

	return(0);
}
