#include <stdio.h>

// general typedef
typedef int MY_INT;  // now, using MY_INT has a type means to use int as the type

int main(void)
{
	// function prototypes
	MY_INT add(MY_INT, MY_INT);

	// more typedefs
	typedef int MY_INT;  // redefined MY_INT
	typedef float KVD_FLOAT;  // KVD_FLOAT -> float
	typedef char CHARACTER;  // CHARACTER -> char
	typedef double MY_DOUBLE;  // MY_DOUBLE -> double

	// copying the Win32 SDK!
	typedef unsigned int UINT;
	typedef UINT HANDLE;
	typedef HANDLE HWND;
	typedef HANDLE HINSTANCE;

	// variable declarations
	MY_INT kvd_a = 10, kvd_i;
	MY_INT kvd_iArray[] = { 9, 18, 27, 36, 45, 54, 63, 72, 81, 90 };

	KVD_FLOAT kvd_f_e = 2.71f;
	const KVD_FLOAT kvd_f_pi = 3.14f;

	CHARACTER kvd_c = '*';
	CHARACTER kvd_cArray_01[] = "Hello";
	CHARACTER kvd_cArray_02[][10] = { "RTR", "Batch", "2020-21" };

	MY_DOUBLE kvd_d = 8.19112001;

	UINT kvd_uint = 1993;
	HANDLE kvd_handle = 19209912;  // please do NOT assign values to handles in Win32 SDK like shown
	HWND kvd_hwnd = 78374;  // obtain them via functions returning them instead.
	HINSTANCE kvd_hInstance = 26878367;
	
	// code
	printf("\n\n");
	printf("variable kvd_a of type MY_INT = %d\n", kvd_a);

	printf("\n\n");

	for (kvd_i = 0; kvd_i < (sizeof(kvd_iArray) / sizeof(MY_INT)); kvd_i++)
		printf("variable kvd_iArray[%d] of type MY_INT = %d\n", kvd_i, kvd_iArray[kvd_i]);

	printf("\n\n");

	printf("variable kvd_f_e of type KVD_FLOAT = %f\n", kvd_f_e);
	printf("variable kvd_f_pi of type KVD_FLOAT = %f\n", kvd_f_pi);

	printf("\n\n");
	printf("variable kvd_d of type MY_DOUBLE = %lf\n", kvd_d);

	printf("\n\n");
	printf("variable kvd_c of type CHARACTER = '%c'\n", kvd_c);

	printf("\n\n");
	printf("string kvd_cArray_01 of type CHARACTER[] = %s\n", kvd_cArray_01);

	printf("\n\n");
	for (kvd_i = 0; kvd_i < (sizeof(kvd_cArray_02) / sizeof(kvd_cArray_02[0])); kvd_i++)
		printf("string kvd_cArray_02[%d] of type CHARACTER[] = %s\n", kvd_i, kvd_cArray_02[kvd_i]);

	printf("\n\n");
	printf("variable kvd_uint of type UINT = %u\n", kvd_uint);
	printf("variable kvd_handle of type HANDLE = %u\n", kvd_handle);
	printf("variable kvd_hwnd of type HWND = %d\n", kvd_hwnd);
	printf("variable kvd_hInstance of type HINSTANCE = %d\n", kvd_hInstance);

	printf("\n\n");

	MY_INT kvd_x = 90;
	MY_INT kvd_y = 30;
	MY_INT kvd_ret;

	kvd_ret = add(kvd_x, kvd_y);
	printf("ret = %d\n", kvd_ret);

	printf("\n\n");
	return(0);
}

MY_INT add(MY_INT kvd_a, MY_INT kvd_b)
{
	// code
	return(kvd_a + kvd_b);
}
