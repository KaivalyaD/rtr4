// header files
#include<windows.h>
#include<stdlib.h>	// for exit()
#include<stdio.h>	// for file I/O functions
#include"OGL.h"

// OpenGL headers
#include<GL/gl.h>
#include<GL/glu.h>	// graphics library utility

// OpenGL libraries
#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "glu32.lib")

// macros
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

// global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// global variable declarations
FILE *gpLog = NULL;
HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
BOOL gbFullScreen = FALSE;
BOOL gbActiveWindow = FALSE;

// for the scene
int shoulder = 0, elbow = 0;
GLUquadric *quadric = NULL;

// entry-point function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	// function prototypes
	int initialize(void);	// declaring according to the order of use
	void display(void);
	void update(void);
	void uninitialize(void);

	// variable declarations
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("MyWindow");
	BOOL bDone = FALSE;
	int iRetVal = 0;
	int cxScreen, cyScreen;

	// code
	if (fopen_s(&gpLog, "Log.txt", "w") != 0)
	{
		MessageBox(NULL, TEXT("fopen_s: failed to open log file"), TEXT("File I/O Error"), MB_OK | MB_ICONERROR);
		exit(0);
	}
	else
	{
		fprintf(gpLog, "fopen_s: log file opened successfully\n");
	}

	// initialization of the wndclass structure
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpfnWndProc = WndProc;
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

	// registering the wndclass
	RegisterClassEx(&wndclass);

	// getting the screen size
	cxScreen = GetSystemMetrics(SM_CXSCREEN);
	cyScreen = GetSystemMetrics(SM_CYSCREEN);

	// create the window
	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppName,
		TEXT("Robotic Arm: Kaivalya Vishwakumar Deshpande"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		(cxScreen - WIN_WIDTH) / 2,
		(cyScreen - WIN_HEIGHT) / 2,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);
	ghwnd = hwnd;
	
	// initialize
	iRetVal = initialize();
	if (iRetVal == -1)
	{
		fprintf(gpLog, "ChoosePixelFormat(): failed\n");
		uninitialize();
	}
	else if (iRetVal == -2)
	{
		fprintf(gpLog, "SetPixelFormat(): failed\n");
		uninitialize();
	}
	else if (iRetVal == -3)
	{
		fprintf(gpLog, "wglCreateContext(): failed\n");
		uninitialize();
	}
	else if (iRetVal == -4)
	{
		fprintf(gpLog, "wglMakeCurrent(): failed\n");
		uninitialize();
	}
	else
	{
		fprintf(gpLog, "created OpenGL context successfully and made it the current context\n");
	}

	// show the window
	ShowWindow(hwnd, iCmdShow);

	// foregrounding and focussing the window
	SetForegroundWindow(hwnd);	// using ghwnd is obviously fine, but by common sense ghwnd is for global use while we have hwnd locally available in WndProc and here
	SetFocus(hwnd);

	// game loop
	while (bDone != TRUE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				bDone = TRUE;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (gbActiveWindow)
			{
				// render the scene
				display();

				// update the scene
				update();
			}
		}
	}

	// uninitialize
	uninitialize();

	return (int)msg.wParam;
}

// callback function
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	// function prototypes
	void ToggleFullScreen(void);
	void resize(int, int);

	// code
	switch (iMsg)
	{
	case WM_SETFOCUS:
		gbActiveWindow = TRUE;
		fprintf(gpLog, "window in focus\n");
		break;
	case WM_KILLFOCUS:
		gbActiveWindow = FALSE;
		fprintf(gpLog, "window out of focus\n");
		break;
	case WM_ERASEBKGND:
		return 0;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 27:
			fprintf(gpLog, "destroying after receiving esc\n");
			DestroyWindow(hwnd);
			break;
		default:
			break;
		}
		break;
	case WM_CHAR:
		switch (wParam)
		{
		case 'F':
		case 'f':
			ToggleFullScreen();
			break;
		case 'e':
			elbow = (elbow + 3) % 360;
			break;
		case 'E':
			elbow = (elbow - 3) % 360;
			break;
		case 's':
			shoulder = (shoulder + 3) % 360;
			break;
		case 'S':
			shoulder = (shoulder - 3) % 360;
			break;
		default:
			break;
		}
		break;
	case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_CLOSE:	// disciplined code: sent as a signal that a window or an application should terminate
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		break;
	}

	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

void ToggleFullScreen(void)
{
	// variable declarations
	static DWORD dwStyle;
	static WINDOWPLACEMENT wp;
	MONITORINFO mi;

	// code
	wp.length = sizeof(WINDOWPLACEMENT);

	if (gbFullScreen == FALSE)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi.cbSize = sizeof(MONITORINFO);

			if (GetWindowPlacement(ghwnd, &wp) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top,
					SWP_NOZORDER | SWP_FRAMECHANGED);
			}

			ShowCursor(FALSE);
			gbFullScreen = TRUE;
			fprintf(gpLog, "fullscreen mode on\n");
		}
	}
	else
	{
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wp);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);
		gbFullScreen = FALSE;
		fprintf(gpLog, "fullscreen mode off\n");
	}
}

int initialize(void)
{
	// function prototypes
	void resize(int, int);

	// variable declarations
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex = 0;

	// code
	// initialize PIXELFORMATDESCRIPTOR
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;	// R
	pfd.cGreenBits = 8;	// G
	pfd.cBlueBits = 8;	// B
	pfd.cAlphaBits = 8;	// A
	pfd.cDepthBits = 32;

	// get DC
	ghdc = GetDC(ghwnd);

	// choose pixel format
	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
	if (iPixelFormatIndex == 0)
		return -1;

	// set chosen pixel format
	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
		return -2;

	// create OpenGL rendering context
	ghrc = wglCreateContext(ghdc);	// calling my first bridging API
	if (ghrc == NULL)
		return -3;

	// make the rendering context the current context
	if (wglMakeCurrent(ghdc, ghrc) == FALSE)	// the second bridging API
		return -4;

	// here starts the OpenGL code
	// clear the screen using black colour
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// enabling depth test
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// warm-up resize call
	resize(WIN_WIDTH, WIN_HEIGHT);

	return 0;
}

void resize(int width, int height)
{
	// code
	if (height == 0)
		height = 1;	// to prevent a divide by zero when calculating the width/height ratio

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

	glMatrixMode(GL_PROJECTION); // show a projection
	glLoadIdentity();

	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

void display(void)
{
	// code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -0.1f);
	
	// camera transformation
	gluLookAt(
		0.0f, 0.0f, 5.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	);
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(0.5f, 0.35f, 0.05f);

	// mechanics for the shoulder
	glTranslatef(0.0f, 0.0f, -6.0f);	// for better visibility
	glPushMatrix();	// shoulder joint pushed

	glRotatef((GLfloat)shoulder, 0.0f, 0.0f, 1.0f);	// rotate the arm
	glTranslatef(1.0f, 0.0f, 0.0f);	// translate 1 unit ahead of the shoulder in the appropriate direction
	glPushMatrix();	// push unscaled CTM

	glScalef(2.0f, 0.5f, 1.0f);	// all x- and y-coordinates henceforth specified until re-scaled shall be multiplied by 2 and 0.5 resp.
	/* 
	 * draw the arm:
	 * 
	 * glTranslatef() gets the CTM to (1, 0, 0) i.e. (0, 0, 0) is mapped onto the real (1, 0, 0);
	 * glScalef() as called above scales all x-values and y-values by a factor of 2 and 0.5 respectively, while all z-values remain unscaled  
	 * 
	 * thus, the call to gluSphere() below results in an ellipsoid instead of a sphere
	 */
	quadric = gluNewQuadric();
	gluSphere(quadric, 0.5f, 10, 10);
	gluDeleteQuadric(quadric);

	glPopMatrix();	// pop unscaled CTM
	
	// mechanics for the forearm
	glTranslatef(1.0f, 0.0f, 0.0f);	// go to the elbow

	glRotatef((GLfloat)elbow, 0.0f, 0.0f, 1.0f);
	glTranslatef(1.0f, 0.0f, 0.0f);	// translate 1 unit ahead of the elbow in the appropriate direction
	glPushMatrix();	// push the unscaled CTM

	glScalef(2.0f, 0.5f, 1.0f);
	quadric = gluNewQuadric();
	gluSphere(quadric, 0.6f, 10, 10);
	gluDeleteQuadric(quadric);

	// go back to shoulder joint
	glPopMatrix();
	glPopMatrix();

	SwapBuffers(ghdc);
}

void update(void)
{
	// code
}

void uninitialize(void)
{
	// function prototypes
	void ToggleFullScreen(void);

	// code
	if (gbFullScreen)
	{
		ToggleFullScreen();
	}

	if (wglGetCurrentContext() == ghrc)
	{
		wglMakeCurrent(NULL, NULL);
	}

	if (ghrc)
	{
		wglDeleteContext(ghrc);
		ghrc = NULL;
	}

	if (ghdc)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	if (ghwnd)
	{
		DestroyWindow(ghwnd);	// if unitialize() was not called from WM_DESTROY
		ghwnd = NULL;
	}

	if (gpLog)
	{
		fprintf(gpLog, "fclose: closing log file\n");
		fclose(gpLog);
		gpLog = NULL;
	}
}
