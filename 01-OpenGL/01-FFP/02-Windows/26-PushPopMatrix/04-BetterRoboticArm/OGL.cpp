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
/* 
 * keys for rotation about:
 * x - *_pitch
 * y - *_yaw
 * z - *_roll
 */
int shoulder_pitch = 0.0f, shoulder_yaw = 0.0f;
int elbow_pitch = 0.0f, elbow_roll = 0.0f;
int wrist_pitch = 0.0f, wrist_roll = 0.0f;
float finger_a = 0.0f, finger_b = 0.0f;

GLfloat lightAmbient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat lightPosition[] = { 0.0f, 100.0f, 0.0f, 100.0f };

GLfloat materialAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat materialDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat materialSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat materialArmShininess = 50.0f;
GLfloat materialJointShininess = 100.0f;

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
		TEXT("Better Robotic Arm: Kaivalya Vishwakumar Deshpande"),
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

		case '1':
			shoulder_yaw = (shoulder_yaw + 3) % 360;
			break;
		case '!':
			shoulder_yaw = (shoulder_yaw - 3) % 360;
			break;

		case '2':
			shoulder_pitch = (shoulder_pitch + 3) % 360;
			break;
		case '@':
			shoulder_pitch = (shoulder_pitch - 3) % 360;
			break;

		case '3':
			elbow_pitch = (elbow_pitch + 3) % 360;
			break;
		case '#':
			elbow_pitch = (elbow_pitch - 3) % 360;
			break;

		case '4':
			elbow_roll = (elbow_roll + 3) % 360;
			break;
		case '$':
			elbow_roll = (elbow_roll - 3) % 360;
			break;

		case '5':
			wrist_pitch = (wrist_pitch + 3) % 360;
			break;
		case '%':
			wrist_pitch = (wrist_pitch - 3) % 360;
			break;

		case '6':
			wrist_roll = (wrist_roll + 3) % 360;
			break;
		case '^':
			wrist_roll = (wrist_roll - 3) % 360;
			break;

		case '7':
			finger_a = min(finger_a + 0.001f, 0.04f);
			break;
		case '&':
			finger_a = max(finger_a - 0.001f, 0.0f);
			break;

		case '8':
			finger_b = min(finger_b + 0.001f, 0.04f);
			break;
		case '*':
			finger_b = max(finger_b - 0.001f, 0.0f);
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
	RECT rc;
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

	// lighting
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glEnable(GL_LIGHT0);

	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

	glEnable(GL_LIGHTING);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// creating the quadric
	quadric = gluNewQuadric();

	// warm-up resize call
	GetClientRect(ghwnd, &rc);
	resize(rc.right - rc.left, rc.bottom - rc.top);

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
	// function prototypes
	void drawCuboid(float, float, float);

	// code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -0.1f);
	
	// camera transformation
	gluLookAt(
		0.0f, 0.0f, 4.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	);

	glTranslatef(0.0f, 1.5f, 0.0f);
	glMaterialf(GL_FRONT, GL_SHININESS, materialJointShininess);
	gluSphere(quadric, 0.1f, 50, 50);
	glRotatef((GLfloat)shoulder_yaw, 0.0f, 1.0f, 0.0f);
	glRotatef((GLfloat)shoulder_pitch, 1.0f, 0.0f, 0.0f);
	glMaterialf(GL_FRONT, GL_SHININESS, materialArmShininess);
	drawCuboid(0.1f, 1.5f, 0.1f);
	
	glTranslatef(0.0f, -1.5f, 0.0f);
	glMaterialf(GL_FRONT, GL_SHININESS, materialJointShininess);
	gluSphere(quadric, 0.1f, 50, 50);
	glRotatef((GLfloat)elbow_pitch, 1.0f, 0.0f, 0.0f);
	glRotatef((GLfloat)elbow_roll, 0.0f, 0.0f, 1.0f);
	glMaterialf(GL_FRONT, GL_SHININESS, materialArmShininess);
	drawCuboid(0.1f, 1.0f, 0.1f);

	glTranslatef(0.0f, -1.0f, 0.0f);
	glMaterialf(GL_FRONT, GL_SHININESS, materialJointShininess);
	gluSphere(quadric, 0.05f, 50, 50);
	glRotatef((GLfloat)wrist_pitch, 1.0f, 0.0f, 0.0f);
	glRotatef((GLfloat)wrist_roll, 0.0f, 0.0f, 1.0f);
	glMaterialf(GL_FRONT, GL_SHININESS, materialArmShininess);
	drawCuboid(0.2f, 0.2f, 0.1f);

	glPushMatrix();
	glTranslatef(-0.08f + (GLfloat)finger_a, -0.2f, 0.0f);
	glMaterialf(GL_FRONT, GL_SHININESS, materialArmShininess);
	drawCuboid(0.05f, 0.1f, 0.1f);

	glPopMatrix();
	glTranslatef(0.08f - (GLfloat)finger_b, -0.2f, 0.0f);
	glMaterialf(GL_FRONT, GL_SHININESS, materialArmShininess);
	drawCuboid(0.05f, 0.1f, 0.1f);

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

	if (quadric)
	{
		gluDeleteQuadric(quadric);
		quadric = NULL;
	}

	if (gpLog)
	{
		fprintf(gpLog, "fclose: closing log file\n");
		fclose(gpLog);
		gpLog = NULL;
	}
}

void drawCuboid(float width, float height, float depth)
{
	// variable declarations
	GLfloat xStart = (GLfloat)(-width / 2.0f);
	GLfloat xEnd = (GLfloat)(width / 2.0f);
	GLfloat zStart = (GLfloat)(depth / 2.0f);
	GLfloat zEnd = (GLfloat)(-depth / 2.0f);

	// code
	glBegin(GL_QUADS);
	{
		/* front face */
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(xStart, 0.0f, zStart);
		glVertex3f(xStart, (GLfloat)(-height), zStart);
		glVertex3f(xEnd, (GLfloat)(-height), zStart);
		glVertex3f(xEnd, 0.0f, zStart);

		/* right face */
		glNormal3f(1.0f, 0.0f, 0.0f);
		glVertex3f(xEnd, 0.0f, zStart);
		glVertex3f(xEnd, (GLfloat)(-height), zStart);
		glVertex3f(xEnd, (GLfloat)(-height), zEnd);
		glVertex3f(xEnd, 0.0f, zEnd);

		/* rear face */
		glNormal3f(0.0f, 0.0f, -1.0f);
		glVertex3f(xEnd, 0.0f, zEnd);
		glVertex3f(xEnd, (GLfloat)(-height), zEnd);
		glVertex3f(xStart, (GLfloat)(-height), zEnd);
		glVertex3f(xStart, 0.0f, zEnd);

		/* left face */
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(xStart, 0.0f, zEnd);
		glVertex3f(xStart, (GLfloat)(-height), zEnd);
		glVertex3f(xStart, (GLfloat)(-height), zStart);
		glVertex3f(xStart, 0.0f, zStart);

		/* top face */
		glNormal3f(0.0f, 1.0f, 0.0f);
		glVertex3f(xStart, 0.0f, zStart);
		glVertex3f(xEnd, 0.0f, zStart);
		glVertex3f(xEnd, 0.0f, zEnd);
		glVertex3f(xStart, 0.0f, zEnd);

		/* bottom face */
		glNormal3f(0.0f, -1.0f, 0.0f);
		glVertex3f(xStart, (GLfloat)(-height), zStart);
		glVertex3f(xStart, (GLfloat)(-height), zEnd);
		glVertex3f(xEnd, (GLfloat)(-height), zEnd);
		glVertex3f(xEnd, (GLfloat)(-height), zStart);
	}
	glEnd();
}
