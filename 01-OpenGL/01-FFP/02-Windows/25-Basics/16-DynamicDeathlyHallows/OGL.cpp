// header files
#include<windows.h>
#include<stdlib.h>	// for exit()
#include<stdio.h>	// for file I/O functions
#include"OGL.h"

// OpenGL headers
#include<GL/gl.h>
#include<GL/glu.h>

// math headers
#define _USE_MATH_DEFINES
#include<math.h>

// OpenGL libraries
#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "glu32.lib")

// macros
#define ERROR_WIN32_CHOOSEPIXELFORMAT_FAILED -1
#define ERROR_WIN32_SETPIXELFORMAT_FAILED -2
#define ERROR_WGL_WGLCREATECONTEXT_FAILED -3
#define ERROR_WGL_WGLMAKECURRENT_FAILED -4

#define DEG_TO_RAD(x) ( (x) * ( (M_PI) / 180.0f) )

#define WIN_WIDTH 900
#define WIN_HEIGHT 500

// global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// global variable declarations
FILE *gpLog = NULL;
HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
BOOL gbFullScreen = FALSE;
BOOL gbActiveWindow = FALSE;

// for animation
float angleCloakOfInvisibility = 0.0f;
float angleResurrectionStone = 0.0f;
float xCloakOfInvisibility = -1.5f, yCloakOfInvisibility = -0.5f;
float xResurrectionStone = 1.5f, yResurrectionStone = -0.5f;
float yElderWand = 3.0f;

int state = 0;

// entry-point function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	// function prototypes
	int initialize(void);
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
	gpLog = fopen("Log.txt", "w");
	if(!gpLog)
	{
		MessageBox(NULL, TEXT("fopen(): failed to create the log file"), TEXT("CRT Error"), MB_OK | MB_ICONERROR);
		exit(1);
	}
	fprintf(gpLog, "fopen(): successfully created the log file\n");

	// initialization of the WNDCLASSEX structure
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.cbWndExtra = 0;
	wndclass.cbClsExtra = 0;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hInstance = hInstance;
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
		TEXT("Dynamic Deathly Hallows: Kaivalya Deshpande"),
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

	// initialize GL
	iRetVal = initialize();
	switch(iRetVal)
	{
		case ERROR_WIN32_CHOOSEPIXELFORMAT_FAILED:
			fprintf(gpLog, "ChoosePixelFormat(): failed\n");
			uninitialize();
			break;
		case ERROR_WIN32_SETPIXELFORMAT_FAILED:
			fprintf(gpLog, "SetPixelFormat(): failed\n");
			uninitialize();
			break;
		case ERROR_WGL_WGLCREATECONTEXT_FAILED:
			fprintf(gpLog, "wglCreateContext(): failed\n");
			uninitialize();
			break;
		case ERROR_WGL_WGLMAKECURRENT_FAILED:
			fprintf(gpLog, "wglMakeCurrent(): failed\n");
			uninitialize();
			break;
		default:
			fprintf(gpLog, "OpenGL initialized successfully\n");
			break;
	}
	
	// show the window
	ShowWindow(hwnd, iCmdShow);

	// focus and foregrounding the window
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	// game loop
	while(!bDone)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
				bDone = TRUE;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if(gbActiveWindow)
			{
				// render the scene
				display();

				// update the scene
				update();
			}
		}
	}

	// uinitialize GL
	uninitialize();

	return static_cast<int>(msg.wParam);
}

// callback function
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	// function prototypes
	void ToggleFullScreen(void);
	void resize(int, int);

	// code
	switch(iMsg)
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
			switch(wParam)
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
			switch(wParam)
			{
				case 'F':
				case 'f':
					ToggleFullScreen();
					break;
				default:
					break;
			}
			break;
		case WM_SIZE:
			resize(LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_CLOSE:
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

	if(!gbFullScreen)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

		if(dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi.cbSize = sizeof(MONITORINFO);

			if(GetWindowPlacement(ghwnd, &wp) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(ghwnd,
					HWND_TOP,
					mi.rcMonitor.left, mi.rcMonitor.top,
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

		gbFullScreen = FALSE;
	}
}

// GL init
int initialize(void)
{
	// function prototypes
	void resize(int, int);

	// variable declarations
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex = 0;

	// code
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 32;
	
	// get the Windows' DC
	ghdc = GetDC(ghwnd);

	// choose format descriptor
	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
	if(!iPixelFormatIndex)
		return ERROR_WIN32_CHOOSEPIXELFORMAT_FAILED;
	
	// set chosen pixel format
	if(!SetPixelFormat(ghdc, iPixelFormatIndex, &pfd))
		return ERROR_WIN32_SETPIXELFORMAT_FAILED;
	
	// create an OpenGL rendering context
	ghrc = wglCreateContext(ghdc);
	if(!ghrc)
		return ERROR_WGL_WGLCREATECONTEXT_FAILED;
	
	if(!wglMakeCurrent(ghdc, ghrc))
		return ERROR_WGL_WGLMAKECURRENT_FAILED;

	// OpenGL begins here
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// setting up depth
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// warm-up resize
	resize(WIN_WIDTH - 16, WIN_HEIGHT - 39);

	return 0;
}

void resize(int width, int height)
{
	// code
	if(height == 0)
		height = 1;	// to prevent a divide-by-zero

	glViewport(0.0f, 0.0f, (GLsizei)width, (GLsizei)height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);
}

void display(void)
{
	// variable declarations
	const float sideCloakOfInvisibility = 1.0f;
	const float radiusResurrectionStone = sideCloakOfInvisibility * sqrtf(3) / 6.0f;
	const float c = sqrtf(3) / 4.0f;

	// code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glLineWidth(2.0f);
	glColor3f(1.0f, 1.0f, 1.0f);

	// animating the Cloak of Invisibility
	switch(state)
	{
	case 0:
		glTranslatef(-1.5f, -0.5f, -3.0f);
		glRotatef(angleCloakOfInvisibility, 0.0f, 1.0f, 0.0f);
		break;
	case 1:
		if (xCloakOfInvisibility < 0.0f && yCloakOfInvisibility < 0.0f)
		{
			glTranslatef(xCloakOfInvisibility, yCloakOfInvisibility, -3.0f);
		}
		else
		{
			glTranslatef(0.0f, 0.0f, -3.0f);
		}
		glRotatef(angleCloakOfInvisibility, 0.0f, 1.0f, 0.0f);
		break;
	default:
		glTranslatef(0.0f, 0.0f, -3.0f);
		glRotatef(angleCloakOfInvisibility, 0.0f, 1.0f, 0.0f);
		break;
	}
	glBegin(GL_LINE_LOOP);
	{
		glVertex3f(0.0f, sideCloakOfInvisibility * c, 0.0f);
		glVertex3f(-1 * sideCloakOfInvisibility / 2.0f, -1 * sideCloakOfInvisibility * c, 0.0f);
		glVertex3f(sideCloakOfInvisibility / 2.0f, -1 * sideCloakOfInvisibility * c, 0.0f);
	}
	glEnd();
	
	glLoadIdentity();

	// animating the Resurrection Stone
	switch (state)
	{
	case 0:
	case 1:
	case 2:
		glTranslatef(1.5f, -0.5f, -3.0f);
		glRotatef(angleResurrectionStone, 0.0f, 1.0f, 0.0f);
		break;
	case 3:
		if (xResurrectionStone > 0.0f && yResurrectionStone < 0.0f)
		{
			glTranslatef(xResurrectionStone, yResurrectionStone, -3.0f);
		}
		else
		{
			glTranslatef(0.0f, 0.0f, -3.0f);
		}
		glRotatef(angleResurrectionStone, 0.0f, 1.0f, 0.0f);
		break;
	default:
		glTranslatef(0.0f, 0.0f, -3.0f);
		glRotatef(angleResurrectionStone, 0.0f, 1.0f, 0.0f);
		break;
	}
	glBegin(GL_LINE_LOOP);
	{
		for (int i = 0; i < 360; i++)
		{
			glVertex3f(radiusResurrectionStone * cosf(DEG_TO_RAD(i)), (radiusResurrectionStone * sinf(DEG_TO_RAD(i))) - (sideCloakOfInvisibility * c / 3.0f), 0.0f);
		}
	}
	glEnd();

	glLoadIdentity();

	// animating the Elder Wand
	switch (state)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
		glTranslatef(0.0f, 3.0f, -3.0f);
		break;
	case 5:
		if (yElderWand > 0.0f)
		{
			glTranslatef(0.0f, yElderWand, -3.0f);
		}
		else
		{
			glTranslatef(0.0f, 0.0f, -3.0f);
		}
		break;
	default:
		glTranslatef(0.0f, 0.0f, -3.0f);
		break;
	}
	glBegin(GL_LINES);
	{
		glVertex3f(0.0f, sideCloakOfInvisibility * c, 0.0f);
		glVertex3f(0.0f, -1 * sideCloakOfInvisibility * c, 0.0f);
		glVertex3f(0.001f, sideCloakOfInvisibility * c, 0.0f);
		glVertex3f(0.001f, -1 * sideCloakOfInvisibility * c, 0.0f);
	}
	glEnd();

	SwapBuffers(ghdc);
}

void update(void)
{
	// variable declarations
	static int countRotationsCloakOfInvisibility = 0;
	static int countRotationsResurrectionStone = 0;
	static float diffAngleCloakOfInvisibility = 0.0f;
	static float diffAngleResurrectionStone = 0.0f;

	// code
	if (state < 6)
	{
		angleCloakOfInvisibility = angleCloakOfInvisibility + 0.1f;
		if (angleCloakOfInvisibility - 360.0f >= 0.0f)
		{
			angleCloakOfInvisibility = angleCloakOfInvisibility - 360.0f;
			countRotationsCloakOfInvisibility = countRotationsCloakOfInvisibility + 1;
		}

		angleResurrectionStone = angleResurrectionStone + 0.2f;
		if (angleResurrectionStone - 360.0f >= 0.0f)
		{
			angleResurrectionStone = angleResurrectionStone - 360.0f;
			if (countRotationsCloakOfInvisibility >= 6)
				countRotationsResurrectionStone = countRotationsResurrectionStone + 1;
		}
	}
	else
	{
		diffAngleCloakOfInvisibility = fabs(360.0f - angleCloakOfInvisibility);

		if(angleCloakOfInvisibility < 360.0f)
			angleCloakOfInvisibility = angleCloakOfInvisibility + (diffAngleCloakOfInvisibility / 1000.0f);
		else if (angleCloakOfInvisibility > 360.0f)
			angleCloakOfInvisibility = angleCloakOfInvisibility - (diffAngleCloakOfInvisibility / 1000.0f);

		if (diffAngleCloakOfInvisibility < 0.001f)
			diffAngleCloakOfInvisibility = 0.0f;

		diffAngleResurrectionStone = fabs(360.0f - angleResurrectionStone);
		
		if (angleResurrectionStone < 360.0f)
			angleResurrectionStone = angleResurrectionStone + (diffAngleResurrectionStone / 1000.0f);
		else if (angleResurrectionStone > 360.0f)
			angleResurrectionStone = angleResurrectionStone - (diffAngleResurrectionStone / 1000.0f);

		if (diffAngleResurrectionStone < 0.001f)
			diffAngleResurrectionStone = 0.0f;
	}

	if (countRotationsCloakOfInvisibility < 3)
	{
		state = 0;
	}
	else if(countRotationsCloakOfInvisibility < 5)
	{
		xCloakOfInvisibility = xCloakOfInvisibility + 0.0003f;
		yCloakOfInvisibility = yCloakOfInvisibility + 0.0001f;

		state = 1;
	}
	else if(countRotationsResurrectionStone < 1)
	{
		state = 2;
	}
	else if (countRotationsResurrectionStone < 5)
	{
		xResurrectionStone = xResurrectionStone - 0.0003f;
		yResurrectionStone = yResurrectionStone + 0.0001f;

		state = 3;
	}
	else if (countRotationsResurrectionStone < 9)
	{
		state = 4;
	}
	else if (countRotationsResurrectionStone < 18)
	{
		yElderWand = yElderWand - 0.0003f;

		state = 5;
	}
	else
	{
		state = 6;
	}
}

void uninitialize(void)
{
	// function prototypes
	void ToggleFullScreen(void);

	// code
	if(gbFullScreen)
		ToggleFullScreen();

	if(wglGetCurrentContext() == ghrc)
		wglMakeCurrent(NULL, NULL);
	
	if(ghrc)
	{
		wglDeleteContext(ghrc);
		ghrc = NULL;
	}
	if(ghdc)
	{
		DeleteDC(ghdc);
		ghdc = NULL;
	}
	if(ghwnd)
	{
		DestroyWindow(ghwnd);
		ghwnd = NULL;
	}
	if(gpLog)
	{
		fprintf(gpLog, "fclose(): closing the log file\n");
		fclose(gpLog);
		gpLog = NULL;
	}
}
