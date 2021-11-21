#include <stdio.h>

#define MY_PI 3.1415926535897932

#define AMC_STRING "AstroMediComp RTR, Batch 4, 2021"

// un-named enumerations
enum
{
	SUNDAY,
	MONDAY,
	TUESDAY,
	WEDNESDAY,
	THURSDAY,
	FRIDAY,
	SATURDAY,
};

enum
{
	JANUARY = 1,
	FEBRUARY,
	MARCH,
	APRIL,
	MAY,
	JUNE,
	JULY,
	AUGUST,
	SEPTEMBER,
	OCTOBER,
	NOVEMBER,
	DECEMBER
};

// named enumerations
enum boolean
{
	TRUE = 1,
	FALSE = 0
};

enum numbers
{
	ONE,
	TWO,
	THREE,
	FOUR,
	FIVE = 5,
	SIX,
	SEVEN,
	EIGHT,
	NINE,
	TEN
};

int main(void)
{
	// local constant declarations
	const double epsilon = 10e-7;

	// code
	printf("\n\n");
	printf("local constant epsilon = %lf\n\n", epsilon);

	printf("sunday is day\t%d\n", SUNDAY);
	printf("monday is day\t%d\n", MONDAY);
	printf("tuesday is day\t%d\n", TUESDAY);
	printf("wednesday is day\t%d\n", WEDNESDAY);
	printf("thursday is day\t%d\n", THURSDAY);
	printf("friday is day\t%d\n", FRIDAY);
	printf("saturday is day\t%d\n\n", SATURDAY);

	printf("ONE is enum number\t%d\n", ONE);
	printf("TWO is enum number\t%d\n", TWO);
	printf("THREE is enum number\t%d\n", THREE);
	printf("FOUR is enum number\t%d\n", FOUR);
	printf("FIVE is enum number\t%d\n", FIVE);
	printf("SIX is enum number\t%d\n", SIX);
	printf("SEVEN is enum number\t%d\n", SEVEN);
	printf("EIGHT is enum number\t%d\n", EIGHT);
	printf("NINE is enum number\t%d\n", NINE);
	printf("TEN is enum number\t%d\n\n", TEN);

	printf("JANUARY is enum number\t%d\n", JANUARY);
	printf("FEBURARY is enum number\t%d\n", FEBRUARY);
	printf("MARCH is enum number\t%d\n", MARCH);
	printf("APRIL is enum number\t%d\n", APRIL);
	printf("MAY is enum number\t%d\n", MAY);
	printf("JUNE is enum number\t%d\n", JUNE);
	printf("JULY is enum number\t%d\n", JULY);
	printf("AUGUST is enum number\t%d\n", AUGUST);
	printf("SEPTEMBER is enum number\t%d\n", SEPTEMBER);
	printf("OCTOBER is enum number\t%d\n", OCTOBER);
	printf("NOVEMBER is enum number\t%d\n", NOVEMBER);
	printf("DECEMBER is enum number\t%d\n\n", DECEMBER);

	printf("value of TRUE\t= %d\n", TRUE);
	printf("value of FALSE\t= %d\n\n", FALSE);

	printf("MY_PI macro value = %.10lf\n\n", MY_PI);
	printf("area of circle with radius 2 units = %f\n\n", (MY_PI * 2.0f * 2.0f));

	printf("\n\n");

	printf(AMC_STRING);
	printf("\n\n");

	printf("AMC_STRING = \"%s\"\n\n", AMC_STRING);

	return(0);
}
