#include <stdio.h>

#define MY_PI_VERSION 4

#ifdef MY_PI_VERSION
#if MY_PI_VERSION <= 0
	#define MY_PI 3.14
#elif MY_PI_VERSION == 1
	#define MY_PI 3.1415
#elif MY_PI_VERSION == 2
	#define MY_PI 3.141592
#elif MY_PI_VERSION == 3
	#define MY_PI 3.14159265
#elif MY_PI_VERSION == 4
	#define MY_PI 3.1415926535
#elif MY_PI_VERSION == 5
	#define MY_PI 3.141592653589
#else
	#define MY_PI 3.141592653589793
#endif  // for #if
#endif	// for #ifdef

int main(void)
{
	// code
	printf("\n\n");
	printf("MY_PI version selected = %d\n", MY_PI_VERSION);
	printf("MY_PI = %.15lf\n\n", MY_PI);
	return 0;
}
