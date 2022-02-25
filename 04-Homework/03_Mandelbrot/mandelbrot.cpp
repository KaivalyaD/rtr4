// header files
#include <windows.h>
#include <stdlib.h>	// for exit()
#include <stdio.h>	// for file I/O
#include <math.h>	// for logf()
#include "res.h"

// OpenGL headers
#include <GL/gl.h>

// OpenGL libraries
#pragma comment(lib, "OpenGL32.lib")

// macros
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#define MAX_ITERATIONS 500

// global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// global variable declarations
FILE *gpLog = NULL;
HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
BOOL gbFullScreen = FALSE;
BOOL gbActiveWindow = FALSE;

// entry-point function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	// function prototypes
	int initialize(void);
	void display(void);
	void update(void);
	void uninitialize(void);

	// local variable declarations
	WNDCLASSEX wndclass{};
	HWND hwnd;
	MSG msg;
	TCHAR szAppname[] = TEXT("GLKaivalya");
	BOOL bDone = FALSE;
	int iRetVal;
	int cxScreen, cyScreen;

	// code
	gpLog = fopen("Log.txt", "w");
	if (!gpLog)
	{
		MessageBox(NULL, TEXT("fopen(): failed to open log file"), TEXT("CRT Error"), MB_OK | MB_ICONERROR);
		exit(0);
	}
	else
	{
		fprintf(gpLog, "fopen(): successfully opened log file\n");
	}

	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NAMASAKARAM));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpfnWndProc = WndProc;
	wndclass.lpszClassName = szAppname;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NAMASAKARAM));

	// register window class
	RegisterClassEx(&wndclass);

	cxScreen = GetSystemMetrics(SM_CXSCREEN);
	cyScreen = GetSystemMetrics(SM_CYSCREEN);

	// create in-core window
	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppname,
		TEXT("KD: Mandelbrot Set"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		(cxScreen - WIN_WIDTH) / 2,
		(cyScreen - WIN_HEIGHT) / 2,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);
	if (!hwnd)
	{
		fprintf(gpLog, "CreateWindow(): failed to create in-core window\n");
		exit(0);
	}
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
		fprintf(gpLog, "initialize(): created OpenGL context and made it current\n");
	}

	// show the window
	ShowWindow(hwnd, iCmdShow);

	// foregrounding and focussing the window
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	// game loop
	while (!bDone)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))	// there are messages in the message queue of this thread
		{
			if (msg.message == WM_QUIT)	// if the received message is WM_QUIT
				bDone = TRUE;	// terminate game loop in the next iteration

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else	// message queue of this thread is empty
		{
			if (gbActiveWindow)	// this window is the active window
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

	return static_cast<int>(msg.wParam);
}

// window callback procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	// function prototypes
	void ToggleFullScreen(void);
	void resize(int, int);
	void uninitialize(void);

	// code
	switch (iMsg)
	{
	case WM_SETFOCUS:
		gbActiveWindow = TRUE;
		fprintf(gpLog, "WndProc() [WM_SETFOCUS]: window in focus\n");
		break;
	case WM_KILLFOCUS:
		gbActiveWindow = FALSE;
		fprintf(gpLog, "WndProc() [WM_KILLFOCUS]: window out of focus\n");
		break;
	case WM_ERASEBKGND:
		return 0;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 27:
			fprintf(gpLog, "WndProc() [WM_KEYDOWN]:destroying after receiving esc\n");
			uninitialize();
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
		PostQuitMessage(0);	// post WM_QUIT to this window's message queue
		break;
	default:
		break;
	}

	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

void ToggleFullScreen(void)
{
	// variable declarations
	static DWORD dwSavedStyle = 0;
	static WINDOWPLACEMENT wpSaved{};
	MONITORINFO miWindowMonitor{};

	// code
	wpSaved.length = sizeof(WINDOWPLACEMENT);

	if (!gbFullScreen)	// window about to change to fullscreen mode
	{
		dwSavedStyle = GetWindowLong(ghwnd, GWL_STYLE);	// save current style before switching mode

		if (dwSavedStyle & WS_OVERLAPPEDWINDOW)	// if the current style covers WS_OVERLAPPEDWINDOW
		{
			miWindowMonitor.cbSize = sizeof(MONITORINFO);

			if (GetWindowPlacement(ghwnd, &wpSaved) &&	// save current window placement
				GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &miWindowMonitor))	// get monitor info of the monitor displaying this window
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwSavedStyle & ~WS_OVERLAPPEDWINDOW);	// remove all components of WS_OVERLAPPEDWINDOW from the window's style
				SetWindowPos(ghwnd, HWND_TOP,
					miWindowMonitor.rcMonitor.left,
					miWindowMonitor.rcMonitor.top,
					miWindowMonitor.rcMonitor.right - miWindowMonitor.rcMonitor.left,
					miWindowMonitor.rcMonitor.bottom - miWindowMonitor.rcMonitor.top,
					SWP_NOZORDER | SWP_FRAMECHANGED);	// set window position so that it occupies the entire monitor on which it resides
			}

			ShowCursor(FALSE);
			gbFullScreen = TRUE;
			fprintf(gpLog, "ToggleFullScreen(): entered fullscreen mode\n");
		}
	}
	else	// window about to change back to restored mode
	{
		SetWindowLong(ghwnd, GWL_STYLE, dwSavedStyle | WS_OVERLAPPEDWINDOW);	// bring all components of an overlapped window back
		SetWindowPlacement(ghwnd, &wpSaved);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);
		gbFullScreen = FALSE;
		fprintf(gpLog, "ToggleFullScreen(): entered restored mode\n");
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
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;

	// get DC
	ghdc = GetDC(ghwnd);

	// choose pixel format
	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
	if (iPixelFormatIndex == 0)
		return -1;

	// set chosen pixel format
	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
		return -2;

	// create an OpenGL rendering context
	ghrc = wglCreateContext(ghdc);
	if (ghrc == NULL)
		return -3;

	// make the rendering context the current context
	if (wglMakeCurrent(ghdc, ghrc) == FALSE)
		return -4;

	// set clear color to black
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// warmup resize
	resize(WIN_WIDTH, WIN_HEIGHT);

	return 0;
}

void resize(int width, int height)
{
	// code
	if (height == 0)
		height = 1;	// prevent a divide by 0

	glViewport(0, 0, width, height);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if (height >= width)
	{
		glOrtho(-3.0f,
			3.0f,
			(-3.0f * ((GLfloat)height / (GLfloat)width)),
			(3.0f * ((GLfloat)height / (GLfloat)width)),
			1.0f,
			-1.0f);
	}
	else
	{
		glOrtho((-3.0f * ((GLfloat)width / (GLfloat)height)),
			(3.0f * ((GLfloat)width / (GLfloat)height)),
			-3.0f,
			3.0f,
			1.0f,
			-1.0f);
	}
}

void display(void)
{
	// function prototypes
	GLfloat transform(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
	void lerp(GLfloat[3], GLfloat[3], GLfloat, GLfloat[3]);

	// variable declarations
	static const GLclampf palette[10][3] = {
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f},
		{0.2f, 0.4f, 0.4f},
		{0.3f, 0.6f, 0.6f},
		{0.4f, 0.8f, 0.8f},
		{0.5f, 1.0f, 1.0f},
		{0.0f, 0.8f, 0.8f},
		{0.0f, 0.4f, 0.2f},
		{0.0f, 0.2f, 0.0f},
		{0.0f, 0.0f, 0.1f}
	};
	GLfloat i, j, x0, y0, x, x2, y, y2, iter;
	GLclampf color1[3], color2[3], color[3], index;

	// code
	glClear(GL_COLOR_BUFFER_BIT);
	{
		for (i = 3.0f; i >= -3.0f; i = i - 0.00462f)	// 0.000462
		{
			for (j = -3.0f; j <= 3.0f; j = j + 0.00694f)
			{
				x0 = transform(-2.5f, 2.5f, -2.0f, 0.47f, j);
				y0 = transform(-2.5f, 2.5f, -1.12f, 1.12f, i);

				x = x2 = 0.0f;
				y = y2 = 0.0f;
				iter = 0.0f;
				
				while (x2 + y2 <= 16 && iter < MAX_ITERATIONS)
				{
					y = (x + x) * y + y0;
					x = x2 - y2 + x0;

					x2 = x * x;
					y2 = y * y;

					iter = iter + 1;
				}

				if (iter < MAX_ITERATIONS)
				{
					index = 48 * iter / MAX_ITERATIONS;

					color1[0] = palette[int(index)][0];
					color1[1] = palette[int(index)][1];
					color1[2] = palette[int(index)][2];

					if (int(index + 1.0f) > 10)
						index = 0.0f;

					color2[0] = palette[int(index + 1.0f)][0];
					color2[1] = palette[int(index + 1.0f)][1];
					color2[2] = palette[int(index + 1.0f)][2];

					lerp(color1, color2, index - int(index), color);

					x0 = transform(-2.0f, 0.47f, -1.5f, 1.5f, x0);
					y0 = transform(-1.12f, 1.12f, -1.5f, 1.5f, y0);

					glBegin(GL_POINTS);
					{
						glColor3fv(color);
						glVertex2f(x0, y0);
					}
					glEnd();
				}
			}
		}
	}
	SwapBuffers(ghdc);
}

void update(void)
{
	// code
}

GLfloat transform(GLfloat old_min, GLfloat old_max, GLfloat new_min, GLfloat new_max, GLfloat value)
{
	// variable declarations
	GLfloat old_unit, new_unit;

	// code
	old_unit = 1.0f / (old_max - old_min);
	new_unit = 1.0f / (new_max - new_min);

	return new_min + (old_unit / new_unit) * (value - old_min);
}

void lerp(GLfloat low[3], GLfloat high[3], GLfloat value, GLfloat answer[3])
{
	// code
	answer[0] = (1.0f - value) * low[0] + (value * high[0]);
	answer[1] = (1.0f - value) * low[1] + (value * high[1]);
	answer[2] = (1.0f - value) * low[2] + (value * high[2]);
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
		DestroyWindow(ghwnd);
		ghwnd = NULL;
	}

	if (gpLog)
	{
		fprintf(gpLog, "fclose(): closing log file\n");
		fclose(gpLog);
		gpLog = NULL;
	}
}
