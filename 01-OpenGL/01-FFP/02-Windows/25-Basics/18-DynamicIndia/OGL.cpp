// header files
#include<windows.h>
#include<stdlib.h>	// for exit()
#include<stdio.h>	// for file I/O functions
#include"OGL.h"

#define _USE_MATH_DEFINES
#include <math.h>

// OpenGL headers
#include<GL/gl.h>
#include<GL/glu.h>

// OpenGL libraries
#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "glu32.lib")

// macros
#define ERROR_WIN32_CHOOSEPIXELFORMAT_FAILED -1
#define ERROR_WIN32_SETPIXELFORMAT_FAILED -2
#define ERROR_WGL_WGLCREATECONTEXT_FAILED -3
#define ERROR_WGL_WGLMAKECURRENT_FAILED -4

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#define DEG_TO_RAD(x) ((x) * ((M_PI) / (180.0f)))

#define OFFICIAL_ORANGE 1
#define OFFICIAL_WHITE 2
#define OFFICIAL_GREEN 4
#define DRAWA_NOSTRIPE 1
#define DRAWA_ORANGESTRIPE 2
#define DRAWA_WHITESTRIPE 4
#define DRAWA_GREENSTRIPE 8

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
int state = 0;
bool canProceed = false;
GLfloat xLetter = -1.0f, yLetter = 1.0f;
GLclampf fadeFactorD = 0.0f;

GLfloat revolutionAngleJetOne = -180.0f;
GLfloat xJetOne, xJetTwo = -1.5f, xJetThree;
GLfloat revolutionAngleJetThree = 180.0f;

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
		TEXT("Dynamic India: Kaivalya Deshpande"),
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
	void ToggleFullScreen(void);

	// variable declarations
	PIXELFORMATDESCRIPTOR pfd;
	RECT rc;
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
	pfd.cDepthBits = 8;
	
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

	// depth-related setup
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// toggle fullscreen
	ToggleFullScreen();

	// start BGM
	PlaySound(MAKEINTRESOURCE(IDSOUND_SONE_KI_CHIDIYA), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);

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

	gluPerspective(45.0, (GLfloat)width / (GLfloat)height, 0.1, 100.0);
}

void display(void)
{
	// function prototypes
	void drawLetterI(void);
	void drawLetterN(void);
	void drawLetterD(void);
	void drawLetterA(int);
	void drawJet(int);
	
	// code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// set CTM to where the first letter starts

	gluLookAt(
		0.0f, 0.0f, 2.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	);
	glPushMatrix();

	glTranslatef(-0.55f, -0.225f, 0.0f);
	switch (state)
	{
	case 0:
		break;
	case 1:
		/* letters arrive */
		if (xLetter >= 0.0f)
		{
			xLetter = 0.0f;
			canProceed = true;
		}
		glTranslatef((GLfloat)xLetter, 0.0f, 0.0f);
		drawLetterI();
		break;
		
	case 2:
		drawLetterI();

		if (yLetter <= 0.0f)
		{
			yLetter = 0.0f;
			canProceed = true;
		}
		glTranslatef(0.25f, (GLfloat)yLetter, 0.0f);
		drawLetterN();
		break;

	case 3:
		drawLetterI();
		
		glTranslatef(0.25f, 0.0f, 0.0f);
		drawLetterN();

		if (fadeFactorD >= 1.0f)
		{
			fadeFactorD = 1.0f;
			canProceed = true;
		}
		glTranslatef(0.25f, 0.0f, 0.0f);
		drawLetterD();
		break;

	case 4:
		drawLetterI();

		glTranslatef(0.25f, 0.0f, 0.0f);
		drawLetterN();

		glTranslatef(0.25f, 0.0f, 0.0f);
		drawLetterD();

		if (yLetter >= 0.0f)
		{
			yLetter = 0.0f;
			canProceed = true;
		}
		glTranslatef(0.25f, (GLfloat)yLetter, 0.0f);
		drawLetterI();
		break;

	case 5:
		drawLetterI();

		glTranslatef(0.25f, 0.0f, 0.0f);
		drawLetterN();

		glTranslatef(0.25f, 0.0f, 0.0f);
		drawLetterD();

		glTranslatef(0.25f, 0.0f, 0.0f);
		drawLetterI();

		if (xLetter <= 0.30f)
		{
			xLetter = 0.30f;
			canProceed = true;
		}
		glTranslatef((GLfloat)xLetter, 0.0f, 0.0f);
		drawLetterA(DRAWA_NOSTRIPE);
		break;

	case 6:
		drawLetterI();

		glTranslatef(0.25f, 0.0f, 0.0f);
		drawLetterN();

		glTranslatef(0.25f, 0.0f, 0.0f);
		drawLetterD();

		glTranslatef(0.25f, 0.0f, 0.0f);
		drawLetterI();

		glTranslatef(0.30f, 0.0f, 0.0f);
		drawLetterA(DRAWA_NOSTRIPE);
		break;

	case 7:
		drawLetterI();

		glTranslatef(0.25f, 0.0f, 0.0f);
		drawLetterN();

		glTranslatef(0.25f, 0.0f, 0.0f);
		drawLetterD();

		glTranslatef(0.25f, 0.0f, 0.0f);
		drawLetterI();

		glTranslatef(0.30f, 0.0f, 0.0f);
		drawLetterA(DRAWA_NOSTRIPE);

		/* jets arrive */
		glPopMatrix();
		glPushMatrix();
		glTranslatef(-0.72f, 0.80f, 0.0f);
		glRotatef(revolutionAngleJetOne, 0.0f, 0.0f, 1.0f);
		glTranslatef(0.8f, 0.0f, 0.0f);
		glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		drawJet(OFFICIAL_ORANGE);

		glPopMatrix();
		glPushMatrix();
		glTranslatef(xJetTwo, -0.02f, 0.0f);
		drawJet(OFFICIAL_WHITE);

		glPopMatrix();
		if (revolutionAngleJetThree < 90.0f)
		{
			revolutionAngleJetThree = 90.0f;
			canProceed = true;
		}
		glTranslatef(-1.08f, -0.79f, 0.0f);
		glRotatef(revolutionAngleJetThree, 0.0f, 0.0f, 1.0f);
		glTranslatef(0.75f, 0.0f, 0.0f);
		glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
		drawJet(OFFICIAL_GREEN);
		break;

	case 8:
		drawLetterI();

		glTranslatef(0.25f, 0.0f, 0.0f);
		drawLetterN();

		glTranslatef(0.25f, 0.0f, 0.0f);
		drawLetterD();

		glTranslatef(0.25f, 0.0f, 0.0f);
		drawLetterI();

		glTranslatef(0.30f, 0.0f, 0.0f);
		if (xJetOne < 0.5f)
			drawLetterA(DRAWA_NOSTRIPE);
		else
		{
			if (xJetOne > 0.5f && xJetTwo < 0.5f)
				drawLetterA(DRAWA_ORANGESTRIPE);
			else if (xJetTwo > 0.5f && xJetThree < 0.5f)
				drawLetterA(DRAWA_ORANGESTRIPE | DRAWA_GREENSTRIPE);
			else
				drawLetterA(DRAWA_ORANGESTRIPE | DRAWA_WHITESTRIPE | DRAWA_GREENSTRIPE);
		}

		if (xJetOne > 0.72f)
		{
			xJetOne = 0.72f;
			canProceed = true;
		}
		
		glPopMatrix();
		glPushMatrix();
		glTranslatef(xJetOne, 0.0f, 0.0f);
		drawJet(OFFICIAL_ORANGE);

		glPopMatrix();
		glPushMatrix();
		glTranslatef(xJetTwo, -0.02f, 0.0f);
		drawJet(OFFICIAL_WHITE);

		glPopMatrix();
		glTranslatef(xJetThree, -0.04f, 0.0f);
		drawJet(OFFICIAL_GREEN);
		break;

	case 9:
		drawLetterI();

		glTranslatef(0.25f, 0.0f, 0.0f);
		drawLetterN();

		glTranslatef(0.25f, 0.0f, 0.0f);
		drawLetterD();

		glTranslatef(0.25f, 0.0f, 0.0f);
		drawLetterI();

		glTranslatef(0.30f, 0.0f, 0.0f);
		drawLetterA(DRAWA_ORANGESTRIPE | DRAWA_WHITESTRIPE | DRAWA_GREENSTRIPE);

		/* jets depart */
		glPopMatrix();
		glPushMatrix();
		glTranslatef(0.72, 0.80f, 0.0f);
		glRotatef(revolutionAngleJetOne, 0.0f, 0.0f, 1.0f);
		glTranslatef(0.8f, 0.0f, 0.0f);
		glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		drawJet(OFFICIAL_ORANGE);

		glPopMatrix();
		glPushMatrix();
		glTranslatef(xJetTwo, -0.02f, 0.0f);
		drawJet(OFFICIAL_WHITE);

		if (revolutionAngleJetThree < -20.0f)
		{
			canProceed = true;
			revolutionAngleJetThree = 0.0f;
		}

		glPopMatrix();
		glTranslatef(0.72f, -0.79f, 0.0f);
		glRotatef(revolutionAngleJetThree, 0.0f, 0.0f, 1.0f);
		glTranslatef(0.75f, 0.0f, 0.0f);
		glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
		drawJet(OFFICIAL_GREEN);
		break;

	default:
		drawLetterI();

		glTranslatef(0.25f, 0.0f, 0.0f);
		drawLetterN();

		glTranslatef(0.25f, 0.0f, 0.0f);
		drawLetterD();

		glTranslatef(0.25f, 0.0f, 0.0f);
		drawLetterI();

		glTranslatef(0.30f, 0.0f, 0.0f);
		drawLetterA(DRAWA_ORANGESTRIPE | DRAWA_WHITESTRIPE | DRAWA_GREENSTRIPE);
		break;
	}

	SwapBuffers(ghdc);
}

void update(void)
{
	// code
	switch (state)
	{
	case 0:
		Sleep(4000);
		state++;
		break;
	case 1:
		if (canProceed)
		{
			yLetter = 1.0f;	// setting yLetter for the next 'N'
			
			state++;
			canProceed = false;
			
			Sleep(1500);
			break;
		}
		xLetter = xLetter + 0.00025f;
		break;

	case 2:
		if (canProceed)
		{
			state++;
			canProceed = false;
			
			Sleep(1500);
			break;
		}
		yLetter = yLetter - 0.00025f;
		break;

	case 3:
		if (canProceed)
		{
			fadeFactorD = 1.0f;	// setting D opaque till the demo ends
			yLetter = -1.0f; // setting yLetter for the next 'I'

			state++;
			canProceed = false;

			Sleep(1500);
		}
		fadeFactorD = fadeFactorD + 0.00025f;
		break;

	case 4:
		if (canProceed)
		{
			xLetter = 1.5f;	// setting xLetter for the next 'A'

			state++;
			canProceed = false;

			Sleep(1500);
			break;
		}
		yLetter = yLetter + 0.00025f;
		break;

	case 5:
		if (canProceed)
		{
			state++;
			canProceed = false;

			Sleep(1500);
			break;
		}
		xLetter = xLetter - 0.00025f;
		break;

	case 6:
		state++;
		break;

	case 7:
		if (canProceed)
		{
			state++;
			xJetOne = xJetTwo + 0.18f;
			xJetThree = xJetTwo - 0.18f;
			canProceed = false;
			
			break;
		}
		revolutionAngleJetOne = revolutionAngleJetOne + 0.01f;
		xJetTwo = xJetTwo + 0.000066f;
		revolutionAngleJetThree = revolutionAngleJetThree - 0.01f;
		break;

	case 8:
		if (canProceed)
		{
			state++;
			canProceed = false;

			break;
		}
		xJetOne = xJetOne + 0.00016f;
		xJetTwo = xJetTwo + 0.00018f;
		xJetThree = xJetThree + 0.00020f;
		break;

	case 9:
		if (canProceed)
		{
			state++;
			canProceed = false;

			break;
		}
		revolutionAngleJetOne = revolutionAngleJetOne + 0.01f;
		xJetTwo = xJetTwo + 0.00016f;
		revolutionAngleJetThree = revolutionAngleJetThree - 0.01f;
		break;

	default:
		Sleep(1000);
		SendMessage(ghwnd, WM_CLOSE, NULL, NULL);
		break;
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

void drawLetterI(void)
{
	// local variable declarations
	GLfloat xpos = -0.1f;
	GLfloat ypos = 0.4f;

	GLfloat xlimit = 0.2f;
	GLfloat ylimit = 0.4f;

	// code
	glBegin(GL_QUADS);
	{
		// first vertical
		glColor3f(1.0f, 0.6f, 0.2f);
		glVertex3f(xpos + xlimit / 2.0f + 0.0125f, ypos - 0.025f, 0.0f);
		glVertex3f(xpos + xlimit / 2.0f - 0.0125f, ypos - 0.025f, 0.0f);
		
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(xpos + xlimit / 2.0f - 0.0125f, ypos - ylimit / 2.0f, 0.0f);
		glVertex3f(xpos + xlimit / 2.0f + 0.0125f, ypos - ylimit / 2.0f, 0.0f);
		glVertex3f(xpos + xlimit / 2.0f + 0.0125f, ypos - ylimit / 2.0f, 0.0f);
		glVertex3f(xpos + xlimit / 2.0f - 0.0125f, ypos - ylimit / 2.0f, 0.0f);

		glColor3f(0.074f, 0.534f, 0.0313f);
		glVertex3f(xpos + xlimit / 2.0f - 0.0125f, ypos - ylimit + 0.025f, 0.0f);
		glVertex3f(xpos + xlimit / 2.0f + 0.0125f, ypos - ylimit + 0.025f, 0.0f);

		// first horizontal
		glColor3f(1.0f, 0.6f, 0.2f);
		glVertex3f(xpos + xlimit, ypos, 0.0f);
		glVertex3f(xpos, ypos, 0.0f);
		glVertex3f(xpos, ypos - 0.025f, 0.0f);
		glVertex3f(xpos + xlimit, ypos - 0.025f, 0.0f);

		// second horizontal
		glColor3f(0.074f, 0.534f, 0.0313f);
		glVertex3f(xpos + xlimit, ypos - ylimit + 0.025f, 0.0f);
		glVertex3f(xpos, ypos - ylimit + 0.025f, 0.0f);
		glVertex3f(xpos, ypos - ylimit, 0.0f);
		glVertex3f(xpos + xlimit, ypos - ylimit, 0.0f);
	}
	glEnd();
}

void drawLetterN(void)
{
	// local variable declarations
	GLfloat xpos = -0.1f;
	GLfloat ypos = 0.4f;

	GLfloat v_lineThickness = 0.05f;
	GLfloat v_lineHeight = 0.4f;
	GLfloat h_lineThickness = 0.2f;
	GLfloat h_lineHeight = 0.05f;

	// code
	glBegin(GL_QUADS);
	{
		// first vertical
		glColor3f(1.0f, 0.6f, 0.2f);
		glVertex3f(xpos, ypos, 0.0f);

		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(xpos, ypos - v_lineHeight / 2.0f, 0.0f);
		glVertex3f(xpos + v_lineThickness, ypos - v_lineHeight / 2.0f, 0.0f);
		
		glColor3f(1.0f, 0.6f, 0.2f);
		glVertex3f(xpos + v_lineThickness, ypos, 0.0f);

		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(xpos, ypos - v_lineHeight / 2.0f, 0.0f);

		glColor3f(0.074f, 0.534f, 0.0313f);
		glVertex3f(xpos, ypos - v_lineHeight, 0.0f);
		glVertex3f(xpos + v_lineThickness, ypos - v_lineHeight, 0.0f);

		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(xpos + v_lineThickness, ypos - v_lineHeight / 2.0f, 0.0f);

		// first horizontal
		glColor3f(1.0f, 0.6f, 0.2f);
		glVertex3f(xpos + v_lineThickness, ypos, 0.0f);
		glVertex3f(xpos, ypos, 0.0f);

		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(xpos + h_lineThickness / 1.6f - v_lineThickness, ypos - v_lineHeight / 2.0f, 0.0f);
		glVertex3f(xpos + h_lineThickness / 1.6f, ypos - v_lineHeight / 2.0f, 0.0f);
		glVertex3f(xpos + h_lineThickness / 1.6f - v_lineThickness, ypos - v_lineHeight / 2.0f, 0.0f);

		glColor3f(0.074f, 0.534f, 0.0313f);
		glVertex3f(xpos + h_lineThickness - v_lineThickness, ypos - v_lineHeight, 0.0f);
		glVertex3f(xpos + h_lineThickness, ypos - v_lineHeight, 0.0f);

		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(xpos + h_lineThickness / 1.6f, ypos - v_lineHeight / 2.0f, 0.0f);
		
		// second vertical
		glColor3f(1.0f, 0.6f, 0.2f);
		glVertex3f(xpos + h_lineThickness, ypos, 0.0f);
		glVertex3f(xpos + h_lineThickness - v_lineThickness, ypos, 0.0f);

		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(xpos + h_lineThickness - v_lineThickness, ypos - v_lineHeight / 2.0f, 0.0f);
		glVertex3f(xpos + h_lineThickness, ypos - v_lineHeight / 2.0f, 0.0f);
		glVertex3f(xpos + h_lineThickness, ypos - v_lineHeight / 2.0f, 0.0f);
		glVertex3f(xpos + h_lineThickness - v_lineThickness, ypos - v_lineHeight / 2.0f, 0.0f);

		glColor3f(0.074f, 0.534f, 0.0313f);
		glVertex3f(xpos + h_lineThickness - v_lineThickness, ypos - v_lineHeight, 0.0f);
		glVertex3f(xpos + h_lineThickness, ypos - v_lineHeight, 0.0f);
	}
	glEnd();
}

void drawLetterD()
{
	// local variable declarations
	GLfloat xpos = -0.1f;
	GLfloat ypos = 0.4f;

	GLfloat xlimit = 0.2f;
	GLfloat ylimit = 0.4f;

	// code
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_QUADS);
	{
		// first vertical
		glColor3f(fadeFactorD * 1.0f, fadeFactorD * 0.6f, fadeFactorD * 0.2f);
		glVertex3f(xpos, ypos - 0.025f, 0.0f);

		glColor3f(fadeFactorD * 1.0f, fadeFactorD * 1.0f, fadeFactorD * 1.0f);
		glVertex3f(xpos, ypos - ylimit / 2.0f, 0.0f);
		glVertex3f(xpos + 0.025f, ypos - ylimit / 2.0f, 0.0f);

		glColor3f(fadeFactorD * 1.0f, fadeFactorD * 0.6f, fadeFactorD * 0.2f);
		glVertex3f(xpos + 0.025f, ypos - 0.025f, 0.0f);

		glColor3f(fadeFactorD * 1.0f, fadeFactorD * 1.0f, fadeFactorD * 1.0f);
		glVertex3f(xpos + 0.025f, ypos - ylimit / 2.0f, 0.0f);
		glVertex3f(xpos, ypos - ylimit / 2.0f, 0.0f);

		glColor3f(fadeFactorD * 0.074f, fadeFactorD * 0.534f, fadeFactorD * 0.0313f);
		glVertex3f(xpos, ypos - ylimit + 0.025f, 0.0f);
		glVertex3f(xpos + 0.025f, ypos - ylimit + 0.025f, 0.0f);

		// first horizontal
		glColor3f(fadeFactorD * 1.0f, fadeFactorD * 0.6f, fadeFactorD * 0.2f);
		glVertex3f(xpos + (xlimit / 2.0f) + 0.025, ypos, 0.0f);
		glVertex3f(xpos, ypos, 0.0f);
		glVertex3f(xpos, ypos - 0.025f, 0.0f);
		glVertex3f(xpos + (xlimit / 2.0f) + 0.025f, ypos - 0.025f, 0.0f);

		// second vertical
		glColor3f(fadeFactorD * 1.0f, fadeFactorD * 0.8f, fadeFactorD * 0.6f);
		glVertex3f(xpos + xlimit, ypos - ylimit / 4.0f, 0.0f);
		glVertex3f(xpos + xlimit - 0.025f, ypos - ylimit / 4.0f, 0.0f);

		glColor3f(fadeFactorD * 1.0f, fadeFactorD * 1.0f, fadeFactorD * 1.0f);
		glVertex3f(xpos + xlimit - 0.025f, ypos - (3.0f * ylimit / 10.0f) / 1.6f - 0.125f, 0.0f);
		glVertex3f(xpos + xlimit, ypos - (3.0f * ylimit / 4.0f) / 1.6f - 0.0125f, 0.0f);
		glVertex3f(xpos + xlimit, ypos - (3.0f * ylimit / 4.0f) / 1.6f - 0.0125f, 0.0f);
		glVertex3f(xpos + xlimit - 0.025f, ypos - (3.0f * ylimit / 4.0f) / 1.6f - 0.0125f, 0.0f);

		glColor3f(fadeFactorD * 0.74f, fadeFactorD * 0.834f, fadeFactorD * 0.633f);
		glVertex3f(xpos + xlimit - 0.025f, ypos - (3.0f * ylimit / 4.0f) - 0.0125f, 0.0f);
		glVertex3f(xpos + xlimit, ypos - (3.0f * ylimit / 4.0f) - 0.0125f, 0.0f);

		// second horizontal
		glColor3f(fadeFactorD * 0.074f, fadeFactorD * 0.534f, fadeFactorD * 0.0313f);
		glVertex3f(xpos + xlimit / 2.0f + 0.025, ypos - ylimit + 0.025f, 0.0f);
		glVertex3f(xpos, ypos - ylimit + 0.025f, 0.0f);
		glVertex3f(xpos, ypos - ylimit, 0.0f);
		glVertex3f(xpos + xlimit / 2.0f + 0.025, ypos - ylimit, 0.0f);
	}
	glEnd();

	glBegin(GL_POLYGON);
	{
		// first forward slant
		glColor3f(fadeFactorD * 1.0f, fadeFactorD * 0.6f, fadeFactorD * 0.2f);
		glVertex3f(xpos + xlimit / 2.0f + 0.045f, ypos, 0.0f);
		glVertex3f(xpos + xlimit / 2.0f + 0.025f, ypos, 0.0f);
		glVertex3f(xpos + xlimit / 2.0f + 0.025f, ypos - 0.025f, 0.0f);

		glColor3f(fadeFactorD * 1.0f, fadeFactorD * 0.8f, fadeFactorD * 0.6f);
		glVertex3f(xpos + xlimit - 0.025f, ypos - ylimit / 4.0f, 0.0f);
		glVertex3f(xpos + xlimit, ypos - ylimit / 4.0f, 0.0f);
		glVertex3f(xpos + xlimit, ypos - ylimit / 4.0f + 0.02f, 0.0f);
	}
	glEnd();

	glBegin(GL_POLYGON);
	{
		// first backward slant
		glColor3f(fadeFactorD * 0.74f, fadeFactorD * 0.834f, fadeFactorD * 0.633f);
		glVertex3f(xpos + xlimit, ypos - (3.0f * ylimit / 4.0f) - 0.0125f, 0.0f);
		glVertex3f(xpos + xlimit - 0.025f, ypos - (3.0f * ylimit / 4.0f) - 0.0125f, 0.0f);

		glColor3f(fadeFactorD * 0.074f, fadeFactorD * 0.534f, fadeFactorD * 0.0313f);
		glVertex3f(xpos + xlimit / 2.0f + 0.025f, ypos - ylimit + 0.025f, 0.0f);
		glVertex3f(xpos + xlimit / 2.0f + 0.025f, ypos - ylimit, 0.0f);
		glVertex3f(xpos + xlimit / 2.0f + 0.045f, ypos - ylimit, 0.0f);

		glColor3f(fadeFactorD * 0.74f, fadeFactorD * 0.834f, fadeFactorD * 0.633f);
		glVertex3f(xpos + xlimit, ypos - (3.0f * ylimit / 4.0f) - 0.03f, 0.0f);
		glVertex3f(xpos + xlimit, ypos - (3.0f * ylimit / 4.0f) - 0.0125f, 0.0f);
	}
	glEnd();
}

void drawLetterA(int horz_desc)
{
	// local variable declarations
	GLfloat xpos = -0.15f;
	GLfloat ypos = 0.4f;

	GLfloat xlimit = 0.3f;
	GLfloat ylimit = 0.4f;
	GLfloat slope = ylimit / (xlimit / 2.0f - 0.05f);

	// code
	glBegin(GL_QUADS);
	{
		// first horizontal
		if (horz_desc & DRAWA_ORANGESTRIPE)
		{
			glColor3f(1.0f, 0.6f, 0.2f);
			glVertex3f(xpos + (ylimit / 2.0f + 0.025f) / slope + 0.05f, ypos - ylimit / 2.0f + 0.032f, 0.0f);
			
			glVertex3f(xpos + (ylimit / 2.0f - 0.025f) / slope + 0.045f, ypos - ylimit / 2.0f + 0.012f, 0.0f);
			glVertex3f(xpos + (-1.5f * ylimit - 0.025) / -slope + 0.055f, ypos - ylimit / 2.0f + 0.012f, 0.0f);
			
			glVertex3f(xpos + (-1.5f * ylimit + 0.025) / -slope + 0.05f, ypos - ylimit / 2.0f + 0.032f, 0.0f);
		}
		if (horz_desc & DRAWA_WHITESTRIPE)
		{
			glColor3f(1.0f, 1.0f, 1.0f);
			glVertex3f(xpos + (ylimit / 2.0f + 0.025f) / slope + 0.05f, ypos - ylimit / 2.0f + 0.015f, 0.0f);

			glVertex3f(xpos + (ylimit / 2.0f - 0.025f) / slope + 0.045f, ypos - ylimit / 2.0f - 0.005f, 0.0f);
			glVertex3f(xpos + (-1.5f * ylimit - 0.025) / -slope + 0.055f, ypos - ylimit / 2.0f - 0.005f, 0.0f);

			glVertex3f(xpos + (-1.5f * ylimit + 0.025) / -slope + 0.05f, ypos - ylimit / 2.0f + 0.015, 0.0f);
		}
		if (horz_desc & DRAWA_GREENSTRIPE)
		{
			glColor3f(0.074f, 0.534f, 0.0313f);
			glVertex3f(xpos + (ylimit / 2.0f + 0.025f) / slope + 0.04f, ypos - ylimit / 2.0f - 0.005f, 0.0f);

			glVertex3f(xpos + (ylimit / 2.0f - 0.025f) / slope + 0.045f, ypos - ylimit / 2.0f - 0.025f, 0.0f);
			glVertex3f(xpos + (-1.5f * ylimit - 0.025) / -slope + 0.055f, ypos - ylimit / 2.0f - 0.025f, 0.0f);

			glVertex3f(xpos + (-1.5f * ylimit + 0.025) / -slope + 0.06f, ypos - ylimit / 2.0f - 0.005, 0.0f);
		}

		// first slant
		glColor3f(1.0f, 0.6f, 0.2f);
		glVertex3f(xpos + xlimit / 2.0f + 0.025f, ypos, 0.0f);
		glVertex3f(xpos + xlimit / 2.0f - 0.025f, ypos, 0.0f);

		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(xpos / 2.0f - 0.025f, ypos - ylimit / 2.0f, 0.0f);
		glVertex3f(xpos / 2.0f + 0.025f, ypos - ylimit / 2.0f, 0.0f);
		glVertex3f(xpos / 2.0f + 0.025f, ypos - ylimit / 2.0f, 0.0f);
		glVertex3f(xpos / 2.0f - 0.025f, ypos - ylimit / 2.0f, 0.0f);

		glColor3f(0.074f, 0.534f, 0.0313f);
		glVertex3f(xpos - 0.025f, ypos - ylimit, 0.0f);
		glVertex3f(xpos + 0.025f, ypos - ylimit, 0.0f);

		// second slant
		glColor3f(1.0f, 0.6f, 0.2f);
		glVertex3f(xpos + xlimit / 2.0f + 0.025, ypos, 0.0f);
		glVertex3f(xpos + xlimit / 2.0f - 0.025, ypos, 0.0f);

		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f((xpos + xlimit) / 2.0f - 0.025f, ypos - ylimit / 2.0f, 0.0f);
		glVertex3f((xpos + xlimit) / 2.0f + 0.025f, ypos - ylimit / 2.0f, 0.0f);
		glVertex3f((xpos + xlimit) / 2.0f + 0.025f, ypos - ylimit / 2.0f, 0.0f);
		glVertex3f((xpos + xlimit) / 2.0f - 0.025f, ypos - ylimit / 2.0f, 0.0f);

		glColor3f(0.074f, 0.534f, 0.0313f);
		glVertex3f(xpos + xlimit - 0.025f, ypos - ylimit, 0.0f);
		glVertex3f(xpos + xlimit + 0.025f, ypos - ylimit, 0.0f);
	}
	glEnd();
}

void drawJet(int smoke_color)
{
	// code
	glPushMatrix();	// save CTM to prevent unnecessary changes due to following transformations after returning

	glTranslatef(-0.47f, 0.0f, 0.0f);	// for centering the jet
	glColor3f(0.360f, 0.553f, 0.682);
	glBegin(GL_QUADS);
	{
		glVertex3f(0.58f, 0.0f, 0.0f);
		glVertex3f(0.55f, 0.01f, 0.0f);
		glVertex3f(0.5f, 0.01f, 0.0f);
		glVertex3f(0.5f, 0.0f, 0.0f);

		glVertex3f(0.5f, 0.01f, 0.0f);
		glVertex3f(0.398f, 0.01f, 0.0f);
		glVertex3f(0.398f, -0.01f, 0.0f);
		glVertex3f(0.5f, -0.01f, 0.0f);

		glVertex3f(0.58f, 0.0f, 0.0f);
		glVertex3f(0.55f, -0.01f, 0.0f);
		glVertex3f(0.5f, -0.01f, 0.0f);
		glVertex3f(0.5f, 0.0f, 0.0f);

		glVertex3f(0.5f, 0.01f, 0.0f);
		glVertex3f(0.46f, 0.1f, 0.0f);
		glVertex3f(0.45f, 0.1f, 0.0f);
		glVertex3f(0.44f, 0.01f, 0.0f);
		
		glVertex3f(0.5f, -0.01f, 0.0f);
		glVertex3f(0.46f, -0.1f, 0.0f);
		glVertex3f(0.45f, -0.1f, 0.0f);
		glVertex3f(0.44f, -0.01f, 0.0f);

		glVertex3f(0.43f, 0.01f, 0.0f);
		glVertex3f(0.41f, 0.03f, 0.0f);
		glVertex3f(0.40f, 0.03f, 0.0f);
		glVertex3f(0.40f, 0.01f, 0.0f);

		glVertex3f(0.43f, -0.01f, 0.0f);
		glVertex3f(0.41f, -0.03f, 0.0f);
		glVertex3f(0.40f, -0.03f, 0.0f);
		glVertex3f(0.40f, -0.01f, 0.0f);
		
		if (smoke_color & OFFICIAL_ORANGE)
			glColor3f(1.0f, 0.6f, 0.2f);
		else if (smoke_color & OFFICIAL_WHITE)
			glColor3f(1.0f, 1.0f, 1.0f);
		else if(smoke_color & OFFICIAL_GREEN)
			glColor3f(0.074f, 0.534f, 0.0313f);
		glVertex3f(0.40f, 0.01f, 0.0f);

		glColor3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.34f, 0.01f, 0.0f);
		glVertex3f(0.34f, -0.01f, 0.0f);
		
		if (smoke_color & OFFICIAL_ORANGE)
			glColor3f(1.0f, 0.6f, 0.2f);
		else if (smoke_color & OFFICIAL_WHITE)
			glColor3f(1.0f, 1.0f, 1.0f);
		else if (smoke_color & OFFICIAL_GREEN)
			glColor3f(0.074f, 0.534f, 0.0313f);
		glVertex3f(0.40f, -0.01f, 0.0f);
	}
	glEnd();

	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_POLYGON);
	{
		glVertex3f(0.57f, 0.0f, 0.0f);
		glVertex3f(0.55f, 0.007f, 0.0f);
		glVertex3f(0.53f, 0.007f, 0.0f);
		glVertex3f(0.52f, 0.0f, 0.0f);
		glVertex3f(0.53f, -0.007f, 0.0f);
		glVertex3f(0.55f, -0.007f, 0.0f);
	}
	glEnd();

	glTranslatef(0.445f, 0.025f, 0.0f);
	glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
	glLineWidth(1.5f);
	glColor3f(0.0f, 0.082, 0.157f);

	glBegin(GL_LINES);
	{
		glVertex3f(-0.012f, 0.005f, 0.0f);
		glVertex3f(0.012f, 0.005f, 0.0f);

		glVertex3f(0.0f, 0.004f, 0.0f);
		glVertex3f(0.0f, 0.032f, 0.0f);

		glVertex3f(-0.012f, 0.033f, 0.0f);
		glVertex3f(0.012f, 0.033f, 0.0f);
	}
	glEnd();

	glTranslatef(0.03f, 0.0f, 0.0f);
	glBegin(GL_LINES);
	{
		glVertex3f(-0.012f, 0.005f, 0.0f);
		glVertex3f(0.0021f, 0.033f, 0.0f);
		
		glVertex3f(0.0021f, 0.033f, 0.0f);
		glVertex3f(0.012f, 0.005f, 0.0f);

		glVertex3f(-0.006f, 0.016f, 0.0f);
		glVertex3f(0.008f, 0.016f, 0.0f);
	}
	glEnd();

	glTranslatef(0.03f, 0.0f, 0.0f);
	glBegin(GL_LINES);
	{
		glVertex3f(-0.005f, 0.005f, 0.0f);
		glVertex3f(-0.005f, 0.03f, 0.0f);

		glVertex3f(-0.005f, 0.03f, 0.0f);
		glVertex3f(0.0105f, 0.03f, 0.0f);

		glVertex3f(-0.005f, 0.02f, 0.0f);
		glVertex3f(0.0105f, 0.02f, 0.0f);
	}
	glEnd();

	glPopMatrix();	// pop saved CTM
}
