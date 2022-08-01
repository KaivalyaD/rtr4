// header files
// standard headers
#include<windows.h>
#include<stdlib.h>	// for exit()
#include<stdio.h>	// for file I/O functions

#include"OGL.h"
#include"Sphere.h"

// GLEW
#include<GL/glew.h>

// OpenGL
#include<GL/gl.h>

// vector math
#include"vmath.h"
using namespace vmath;

// link time libs
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "./lib/bin/x64/Sphere.lib")

// macros
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#define DEG_TO_RAD(x) (((x) * (M_PI)) / (180.0f))

// global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// global variable declarations
FILE *gpLog = NULL;
HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
BOOL gbFullScreen = FALSE;
BOOL gbActiveWindow = FALSE;

// related to the OpenGL programmable pipeline
GLuint shaderProgramObject;

enum
{
	KVD_ATTRIBUTE_POSITION = 0,
	KVD_ATTRIBUTE_COLOR,
	KVD_ATTRIBUTE_NORMAL,
	KVD_ATTRIBUTE_TEXTURE0,
};

// object data VRAM
GLuint vao;
GLuint vboSphereVertices;
GLuint vboSphereNormals;
GLuint vboSphereElements;

// transformation matrices and uniforms
GLint modelMatrixUniform;
GLint viewMatrixUniform;
GLint projectionMatrixUniform;

GLint laUniform;
GLint ldUniform;
GLint lsUniform;
GLint lightPositionUniform;

GLint kaUniform;
GLint kdUniform;
GLint ksUniform;
GLint materialShininessUniform;

GLint lightEnabledUniform;

mat4 perspectiveProjectionMatrix;

// object data RAM
float sphereVertices[1146];
float sphereNormals[1146];
float sphereTexCoords[764];
unsigned short sphereElements[2280];
unsigned int numVertices, numElements;

// light
GLfloat lightAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat lightPosition[] = { 0.0f, 0.0f, 0.0f, 1.0f };

// light switch
BOOL bLight = FALSE;

// animation
int rotationAxis;
float theta;

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
		TEXT("Kaivalya Vishwakumar Deshpande: OpenGL"),
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
	else if (iRetVal == -5)
	{
		fprintf(gpLog, "glewInit(): failed\n");
		uninitialize();
	}
	else
	{
		fprintf(gpLog, "created OpenGL context successfully and made it the current context\n");
	}

	// show the window
	ShowWindow(hwnd, iCmdShow);

	// foregrounding and focussing the window
	SetForegroundWindow(hwnd);	// using ghwnd is obviously fine, but by common sense, ghwnd is for global use while we have hwnd locally available in WndProc and here
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
		case 'L':
		case 'l':
			if (bLight)
				bLight = FALSE;
			else
				bLight = TRUE;
			break;
		case 'X':
		case 'x':
			rotationAxis = 1;
			break;
		case 'Y':
		case 'y':
			rotationAxis = 2;
			break;
		case 'Z':
		case 'z':
			rotationAxis = 3;
			break;
		default:
			rotationAxis = 0;
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
	void logGLInfo(void);
	void resize(int, int);
	void uninitialize(void);

	// variable declarations
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex = 0;
	GLint status;
	GLint infoLogLength;
	char *log = NULL;
	RECT rc;

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
	ghrc = wglCreateContext(ghdc);
	if (!ghrc)
		return -3;

	// make the rendering context the current context
	if (!wglMakeCurrent(ghdc, ghrc))
		return -4;

	// glew initialization
	if (glewInit() != GLEW_OK)
		return -5;

	// logging OpenGL info
	logGLInfo();

	// vertex shader
	status = 0;
	infoLogLength = 0;
	log = NULL;

	const GLchar *vertexShaderSourceCode =
		"#version 460 core\n" \
		"\n" \
		"in vec4 a_position;\n" \
		"in vec3 a_normal;\n" \
		"\n" \
		"uniform mat4 u_modelMatrix;\n" \
		"uniform mat4 u_viewMatrix;\n" \
		"uniform mat4 u_projectionMatrix;\n" \
		"\n" \
		"uniform vec4 u_lightPosition;\n" \
		"uniform int u_lightEnabled;\n" \
		"\n" \
		"out vec3 transformedNormal;\n" \
		"out vec3 lightDirection;\n" \
		"out vec3 viewerVector;\n" \
		"\n" \
		"void main(void)\n" \
		"{\n" \
			"if (u_lightEnabled == 1)\n" \
			"{\n" \
				"vec4 eyeCoordinate = u_viewMatrix * u_modelMatrix * a_position;\n" \
				"mat3 normalMatrix = mat3(u_viewMatrix * u_modelMatrix);\n" \
				"\n" \
				"transformedNormal = normalMatrix * a_normal;\n" \
				"lightDirection = vec3(u_lightPosition) - eyeCoordinate.xyz;\n" \
				"viewerVector = -eyeCoordinate.xyz;\n" \
			"}\n" \
			"\n" \
			"gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * a_position;\n" \
		"}\n";

	GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderObject, 1, (const GLchar **)&vertexShaderSourceCode, NULL);
	glCompileShader(vertexShaderObject);
	glGetShaderiv(vertexShaderObject, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		fprintf(gpLog, "*** vertex shader compilation errors ***\n");

		glGetShaderiv(vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			log = (char *)malloc(infoLogLength * sizeof(char));
			if (log)
			{
				GLsizei written = 0;

				glGetShaderInfoLog(vertexShaderObject, infoLogLength * sizeof(char), &written, log);
				fprintf(gpLog, "vertex shader compilation log (%d bytes):\n%s\n", written, log);
				free(log);
				log = NULL;
			}
			else
				fprintf(gpLog, "\tmalloc: cannot allocate memory to hold the compilation log\n");
		}
		else
			fprintf(gpLog, "\tthere is nothing to print\n");

		uninitialize();	// application exits as calling DestroyWindow() in uninitialize()
	}
	fprintf(gpLog, "vertex shader was compiled without errors\n");

	// fragment shader
	status = 0;
	infoLogLength = 0;
	log = NULL;

	const GLchar *fragmentShaderSourceCode =
		"#version 460 core\n" \
		"\n" \
		"in vec3 transformedNormal;\n" \
		"in vec3 lightDirection;\n" \
		"in vec3 viewerVector;\n" \
		"\n" \
		"uniform vec3 u_La;\n" \
		"uniform vec3 u_Ld;\n" \
		"uniform vec3 u_Ls;\n" \
		"\n" \
		"uniform vec3 u_Ka;\n" \
		"uniform vec3 u_Kd;\n" \
		"uniform vec3 u_Ks;\n" \
		"uniform float u_materialShininess;\n" \
		"\n" \
		"uniform int u_lightEnabled;\n" \
		"\n" \
		"out vec4 FragColor;\n" \
		"\n" \
		"void main(void)\n" \
		"{\n" \
			"vec3 phongADSLight;\n" \
			"\n" \
			"if (u_lightEnabled == 1)\n" \
			"{\n" \
				"vec3 componentAmbient = u_La * u_Ka;\n" \
				"\n" \
				"vec3 normalizedTransformedNormal = normalize(transformedNormal);\n" \
				"vec3 normalizedLightDirection = normalize(lightDirection);\n" \
				"vec3 componentDiffuse = u_Ld * u_Kd * max(dot(normalizedLightDirection, normalizedTransformedNormal), 0.0);\n" \
				"\n" \
				"vec3 reflectionVector = reflect(-normalizedLightDirection, normalizedTransformedNormal);\n" \
				"vec3 normalizedViewerVector = normalize(viewerVector);\n" \
				"vec3 componentSpecular = u_Ls * u_Ks * pow(max(dot(reflectionVector, normalizedViewerVector), 0.0), u_materialShininess);\n" \
				"\n" \
				"phongADSLight = componentAmbient + componentDiffuse + componentSpecular;\n" \
			"}\n" \
			"else\n" \
			"{\n" \
				"phongADSLight = vec3(1.0, 1.0, 1.0);\n" \
			"}\n" \
			"\n" \
			"FragColor = vec4(phongADSLight, 1.0);\n" \
		"}\n";

	GLuint fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderObject, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);
	glCompileShader(fragmentShaderObject);
	glGetShaderiv(fragmentShaderObject, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		fprintf(gpLog, "*** fragment shader compilation errors ***\n");

		glGetShaderiv(fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			log = (char *)malloc(infoLogLength * sizeof(char));
			if (log)
			{
				GLsizei written = 0;

				glGetShaderInfoLog(fragmentShaderObject, infoLogLength, &written, log);
				fprintf(gpLog, "fragment shader compilation log (%d bytes):\n%s\n", written, log);
				free(log);
				log = NULL;
			}
			else
				fprintf(gpLog, "\tmalloc: cannot allocate memory to hold the compilation log\n");
		}
		else
			fprintf(gpLog, "\tthere is nothing to print\n");

		uninitialize();
	}
	fprintf(gpLog, "fragment shader was compiled without errors\n");

	// shader program object
	status = 0;
	infoLogLength = 0;
	log = NULL;

	shaderProgramObject = glCreateProgram();
	glAttachShader(shaderProgramObject, vertexShaderObject);
	glAttachShader(shaderProgramObject, fragmentShaderObject);
	glBindAttribLocation(shaderProgramObject, KVD_ATTRIBUTE_POSITION, "a_position");
	glBindAttribLocation(shaderProgramObject, KVD_ATTRIBUTE_NORMAL, "a_normal");
	glLinkProgram(shaderProgramObject);

	glGetProgramiv(shaderProgramObject, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		fprintf(gpLog, "*** there were linking errors ***\n");

		glGetProgramiv(shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			log = (char *)malloc(infoLogLength * sizeof(char));
			if (log)
			{
				GLsizei written = 0;

				glGetProgramInfoLog(shaderProgramObject, infoLogLength * sizeof(char), &written, log);
				fprintf(gpLog, "\tlink time info log (%d bytes):\n%s\n", written, log);
				free(log);
				log = NULL;
			}
			else
				fprintf(gpLog, "\tmalloc: cannot allocate memory to hold the linking log\n");
		}
		else
			fprintf(gpLog, "\tthere is nothing to print\n");

		uninitialize();
	}
	fprintf(gpLog, "shader program was linked without errors\n");

	// post-linking processing
	modelMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_modelMatrix");
	viewMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_viewMatrix");
	projectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_projectionMatrix");

	laUniform = glGetUniformLocation(shaderProgramObject, "u_La");
	ldUniform = glGetUniformLocation(shaderProgramObject, "u_Ld");
	lsUniform = glGetUniformLocation(shaderProgramObject, "u_Ls");
	lightPositionUniform = glGetUniformLocation(shaderProgramObject, "u_lightPosition");

	kaUniform = glGetUniformLocation(shaderProgramObject, "u_Ka");
	kdUniform = glGetUniformLocation(shaderProgramObject, "u_Kd");
	ksUniform = glGetUniformLocation(shaderProgramObject, "u_Ks");
	materialShininessUniform = glGetUniformLocation(shaderProgramObject, "u_materialShininess");

	lightEnabledUniform = glGetUniformLocation(shaderProgramObject, "u_lightEnabled");
	
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	{
		// get sphere data
		getSphereVertexData(sphereVertices, sphereNormals, sphereTexCoords, sphereElements);
		numVertices = getNumberOfSphereVertices();
		numElements = getNumberOfSphereElements();

		// positions
		glGenBuffers(1, &vboSphereVertices);
		glBindBuffer(GL_ARRAY_BUFFER, vboSphereVertices);
		{
			glBufferData(GL_ARRAY_BUFFER, sizeof(sphereVertices), sphereVertices, GL_STATIC_DRAW);
			glVertexAttribPointer(KVD_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
			glEnableVertexAttribArray(KVD_ATTRIBUTE_POSITION);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// normals
		glGenBuffers(1, &vboSphereNormals);
		glBindBuffer(GL_ARRAY_BUFFER, vboSphereNormals);
		{
			glBufferData(GL_ARRAY_BUFFER, sizeof(sphereNormals), sphereNormals, GL_STATIC_DRAW);
			glVertexAttribPointer(KVD_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
			glEnableVertexAttribArray(KVD_ATTRIBUTE_NORMAL);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// elements
		glGenBuffers(1, &vboSphereElements);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
		{
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphereElements), sphereElements, GL_STATIC_DRAW);
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	glBindVertexArray(0);

	// set clear color
	glClearColor(0.15f, 0.15f, 0.15f, 1.0f);

	// depth-related changes
	glClearDepth(1.0f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);

	perspectiveProjectionMatrix = mat4::identity();

	// warm-up resize
	GetClientRect(ghwnd, &rc);
	resize(rc.right - rc.left, rc.bottom - rc.top);

	return 0;
}

void logGLInfo(void)
{
	// variable declarations
	GLint numExtensions = 0;

	// code
	fprintf(gpLog, "\n-------------------- OpenGL Properties --------------------\n\n");
	fprintf(gpLog, "OpenGL Vendor   : %s\n", glGetString(GL_VENDOR));
	fprintf(gpLog, "OpenGL Renderer : %s\n", glGetString(GL_RENDERER));
	fprintf(gpLog, "OpenGL Version  : %s\n", glGetString(GL_VERSION));
	fprintf(gpLog, "GLSL Version    : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	fprintf(gpLog, "\n-------------------- OpenGL Extensions --------------------\n\n");
	glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
	fprintf(gpLog, "Number of supported extensions : %d\n\n", numExtensions);
	for (int i = 0; i < numExtensions; i++)
	{
		fprintf(gpLog, "%s\n", glGetStringi(GL_EXTENSIONS, i));
	}
	fprintf(gpLog, "-------------------------------------------------------------\n\n");
}

void resize(int width, int height)
{
	// variable declarations
	GLfloat aspectRatio;

	// code
	if (height == 0)
		height = 1;	// to prevent a divide by zero when calculating the width/height ratio

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

	aspectRatio = (GLfloat)width / (GLfloat)height;
	perspectiveProjectionMatrix = vmath::perspective(25.0f, aspectRatio, 0.1f, 100.0f);
}

void display(void)
{
	// function prototypes
	void draw24Spheres(void);

	// code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	draw24Spheres();

	SwapBuffers(ghdc);
}

void update(void)
{
	// code
	if (bLight && rotationAxis)
	{
		theta = theta + DEG_TO_RAD(1.0f);
		if (theta >= DEG_TO_RAD(360.0f))
			theta = theta - DEG_TO_RAD(360.0f);
	}
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

	if (vboSphereElements)
	{
		glDeleteBuffers(1, &vboSphereElements);
		vboSphereElements = 0;
	}

	if (vboSphereNormals)
	{
		glDeleteBuffers(1, &vboSphereNormals);
		vboSphereNormals = 0;
	}

	if (vboSphereVertices)
	{
		glDeleteBuffers(1, &vboSphereVertices);
		vboSphereVertices = 0;
	}

	if (vao)
	{
		glDeleteVertexArrays(1, &vao);
		vao = 0;
	}
	
	if (shaderProgramObject)
	{
		GLsizei numAttachedShaders = 0;
		GLuint *shaderObjects = NULL;

		glUseProgram(shaderProgramObject);
		
		glGetProgramiv(shaderProgramObject, GL_ATTACHED_SHADERS, &numAttachedShaders);
		shaderObjects = (GLuint *)malloc(numAttachedShaders * sizeof(GLuint));
		glGetAttachedShaders(shaderProgramObject, numAttachedShaders, &numAttachedShaders, shaderObjects);

		for (GLsizei i = 0; i < numAttachedShaders; i++)
		{
			glDetachShader(shaderProgramObject, shaderObjects[i]);
			glDeleteShader(shaderObjects[i]);
			shaderObjects[i] = 0;
		}
		free(shaderObjects);
		shaderObjects = NULL;
		fprintf(gpLog, "detached and deleted %d shader objects\n", numAttachedShaders);

		glUseProgram(0);
		glDeleteProgram(shaderProgramObject);
		shaderProgramObject = 0;
		fprintf(gpLog, "deleted shader program object\n");
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

void draw24Spheres(void)
{
	// variable declarations
	mat4 translationMatrix = mat4::identity();
	mat4 modelMatrix = mat4::identity();
	mat4 viewMatrix = mat4::identity();

	// view transformation
	viewMatrix = vmath::lookat(
		vec3(0.0f, 3.75f, 20.0f),
		vec3(0.0f, 3.75f, 0.0f),
		vec3(0.0f, 1.0f, 0.0f)
	);

	// code
	if (rotationAxis == 1)
	{
		lightPosition[0] = 0.0f;
		lightPosition[1] = 50.0f * sinf(theta);
		lightPosition[2] = 50.0f * cosf(theta);
		lightPosition[3] = 1.0f;
	}
	else if (rotationAxis == 2)
	{
		lightPosition[0] = 50.0f * cosf(theta);
		lightPosition[1] = 0.0f;
		lightPosition[2] = 50.0f * sinf(theta);
		lightPosition[3] = 1.0f;
	}
	else if (rotationAxis == 3)
	{
		lightPosition[0] = 50.0f * cosf(theta);
		lightPosition[1] = 50.0f * sinf(theta);
		lightPosition[2] = 0.0f;
		lightPosition[3] = 1.0f;
	}
	else
	{
		lightPosition[0] = 0.0f;
		lightPosition[1] = 3.75f;
		lightPosition[2] = 11.0f;
		lightPosition[3] = 1.0f;
	}

	glUseProgram(shaderProgramObject);
	{
		glBindVertexArray(vao);
		{
			if (bLight)
			{
				glUniform1i(lightEnabledUniform, 1);

				glUniform3fv(laUniform, 1, lightAmbient);
				glUniform3fv(ldUniform, 1, lightDiffuse);
				glUniform3fv(lsUniform, 1, lightSpecular);
				glUniform4fv(lightPositionUniform, 1, lightPosition);

				// [1, 1] material: emerald
				glUniform3f(kaUniform, 0.0215f, 0.1745f, 0.0215f);
				glUniform3f(kdUniform, 0.07568f, 0.61424f, 0.07568f);
				glUniform3f(ksUniform, 0.633f, 0.727811f, 0.633f);
				glUniform1f(materialShininessUniform, 0.6f * 128.0f);
				translationMatrix = vmath::translate(-4.5f, 7.5f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [2, 1] material: jade
				glUniform3f(kaUniform, 0.135f, 0.2225f, 0.1575f);
				glUniform3f(kdUniform, 0.54f, 0.89f, 0.63f);
				glUniform3f(ksUniform, 0.316228f, 0.316228f, 0.316228f);
				glUniform1f(materialShininessUniform, 0.1f * 128.0f);
				translationMatrix = vmath::translate(-4.5f, 6.0f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [3, 1] material: obsidian
				glUniform3f(kaUniform, 0.05375f, 0.05f, 0.06625f);
				glUniform3f(kdUniform, 0.18275f, 0.17f, 0.22525f);
				glUniform3f(ksUniform, 0.332741f, 0.328634f, 0.346435f);
				glUniform1f(materialShininessUniform, 0.3f * 128.0f);
				translationMatrix = vmath::translate(-4.5f, 4.5f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [4, 1] material: pearl
				glUniform3f(kaUniform, 0.25f, 0.20725f, 0.20725f);
				glUniform3f(kdUniform, 1.0f, 0.829f, 0.829f);
				glUniform3f(ksUniform, 0.296648f, 0.296648f, 0.296648f);
				glUniform1f(materialShininessUniform, 0.3f * 128.0f);
				translationMatrix = vmath::translate(-4.5f, 3.0f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [5, 1] material: ruby
				glUniform3f(kaUniform, 0.1745f, 0.01175f, 0.01175f);
				glUniform3f(kdUniform, 0.61424f, 0.04136f, 0.04136f);
				glUniform3f(ksUniform, 0.727811f, 0.727811f, 0.727811f);
				glUniform1f(materialShininessUniform, 0.6f * 128.0f);
				translationMatrix = vmath::translate(-4.5f, 1.5f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [6, 1] material: turquoise
				glUniform3f(kaUniform, 0.1f, 0.18725f, 0.1745f);
				glUniform3f(kdUniform, 0.396f, 0.74151f, 0.69102f);
				glUniform3f(ksUniform, 0.297254f, 0.30829f, 0.306678f);
				glUniform1f(materialShininessUniform, 0.1f * 128.0f);
				translationMatrix = vmath::translate(-4.5f, 0.0f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [1, 2] material: brass
				glUniform3f(kaUniform, 0.329412f, 0.223529f, 0.027451f);
				glUniform3f(kdUniform, 0.780392f, 0.568627f, 0.113725f);
				glUniform3f(ksUniform, 0.992157f, 0.941176f, 0.807843f);
				glUniform1f(materialShininessUniform, 0.21794872f * 128.0f);
				translationMatrix = vmath::translate(-1.5f, 7.5f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [2, 2] material: bronze
				glUniform3f(kaUniform, 0.2125f, 0.1275f, 0.054f);
				glUniform3f(kdUniform, 0.714f, 0.4284f, 0.18144f);
				glUniform3f(ksUniform, 0.393548f, 0.271906f, 0.166721f);
				glUniform1f(materialShininessUniform, 0.2f * 128.0f);
				translationMatrix = vmath::translate(-1.5f, 6.0f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [3, 2] material: chrome
				glUniform3f(kaUniform, 0.25f, 0.25f, 0.25f);
				glUniform3f(kdUniform, 0.4f, 0.4f, 0.4f);
				glUniform3f(ksUniform, 0.774597f, 0.774597f, 0.774597f);
				glUniform1f(materialShininessUniform, 0.6f * 128.0f);
				translationMatrix = vmath::translate(-1.5f, 4.5f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [4, 2] material: copper
				glUniform3f(kaUniform, 0.19125f, 0.0735f, 0.0225f);
				glUniform3f(kdUniform, 0.7038f, 0.27048f, 0.0828f);
				glUniform3f(ksUniform, 0.256777f, 0.137622f, 0.086014f);
				glUniform1f(materialShininessUniform, 0.1f * 128.0f);
				translationMatrix = vmath::translate(-1.5f, 3.0f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [5, 2] material: gold
				glUniform3f(kaUniform, 0.24725f, 0.1995f, 0.0745f);
				glUniform3f(kdUniform, 0.75164f, 0.60648f, 0.22648f);
				glUniform3f(ksUniform, 0.628281f, 0.555802f, 0.366065f);
				glUniform1f(materialShininessUniform, 0.4f * 128.0f);
				translationMatrix = vmath::translate(-1.5f, 1.5f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [6, 2] material: silver
				glUniform3f(kaUniform, 0.19225f, 0.19225f, 0.19225f);
				glUniform3f(kdUniform, 0.50754f, 0.50754f, 0.50754f);
				glUniform3f(ksUniform, 0.508273f, 0.508273f, 0.508273f);
				glUniform1f(materialShininessUniform, 0.4f * 128.0f);
				translationMatrix = vmath::translate(-1.5f, 0.0f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [1, 3] material: black plastic
				glUniform3f(kaUniform, 0.0f, 0.0f, 0.0f);
				glUniform3f(kdUniform, 0.01f, 0.01f, 0.01f);
				glUniform3f(ksUniform, 0.5f, 0.5f, 0.5f);
				glUniform1f(materialShininessUniform, 0.25f * 128.0f);
				translationMatrix = vmath::translate(1.5f, 7.5f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [2, 3] material: cyan plastic
				glUniform3f(kaUniform, 0.0f, 0.1f, 0.06f);
				glUniform3f(kdUniform, 0.0f, 0.50980392f, 0.50980392f);
				glUniform3f(ksUniform, 0.50196078f, 0.50196078f, 0.50196078f);
				glUniform1f(materialShininessUniform, 0.25f * 128.0f);
				translationMatrix = vmath::translate(1.5f, 6.0f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [3, 3] material: green plastic
				glUniform3f(kaUniform, 0.0f, 0.0f, 0.0f);
				glUniform3f(kdUniform, 0.1f, 0.35f, 0.1f);
				glUniform3f(ksUniform, 0.45f, 0.55f, 0.45f);
				glUniform1f(materialShininessUniform, 0.25f * 128.0f);
				translationMatrix = vmath::translate(1.5f, 4.5f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [4, 3] material: green plastic
				glUniform3f(kaUniform, 0.0f, 0.0f, 0.0f);
				glUniform3f(kdUniform, 0.5f, 0.0f, 0.0f);
				glUniform3f(ksUniform, 0.7f, 0.6f, 0.6f);
				glUniform1f(materialShininessUniform, 0.25f * 128.0f);
				translationMatrix = vmath::translate(1.5f, 3.0f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [5, 3] material: white plastic
				glUniform3f(kaUniform, 0.0f, 0.0f, 0.0f);
				glUniform3f(kdUniform, 0.55f, 0.55f, 0.55f);
				glUniform3f(ksUniform, 0.7f, 0.7f, 0.7f);
				glUniform1f(materialShininessUniform, 0.25f * 128.0f);
				translationMatrix = vmath::translate(1.5f, 1.5f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [6, 3] material: yellow plastic
				glUniform3f(kaUniform, 0.0f, 0.0f, 0.0f);
				glUniform3f(kdUniform, 0.5f, 0.5f, 0.0f);
				glUniform3f(ksUniform, 0.6f, 0.6f, 0.5f);
				glUniform1f(materialShininessUniform, 0.25f * 128.0f);
				translationMatrix = vmath::translate(1.5f, 0.0f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [1, 4] material: black rubber
				glUniform3f(kaUniform, 0.02f, 0.02f, 0.02f);
				glUniform3f(kdUniform, 0.01f, 0.01f, 0.01f);
				glUniform3f(ksUniform, 0.4f, 0.4f, 0.4f);
				glUniform1f(materialShininessUniform, 0.078125f * 128.0f);
				translationMatrix = vmath::translate(4.5f, 7.5f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [2, 4] material: cyan rubber
				glUniform3f(kaUniform, 0.0f, 0.05f, 0.05f);
				glUniform3f(kdUniform, 0.4f, 0.5f, 0.5f);
				glUniform3f(ksUniform, 0.04f, 0.7f, 0.7f);
				glUniform1f(materialShininessUniform, 0.078125f * 128.0f);
				translationMatrix = vmath::translate(4.5f, 6.0f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [3, 4] material: green rubber
				glUniform3f(kaUniform, 0.0f, 0.05f, 0.0f);
				glUniform3f(kdUniform, 0.4f, 0.5f, 0.4f);
				glUniform3f(ksUniform, 0.04f, 0.7f, 0.04f);
				glUniform1f(materialShininessUniform, 0.078125f * 128.0f);
				translationMatrix = vmath::translate(4.5f, 4.5f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [4, 4] material: red rubber
				glUniform3f(kaUniform, 0.05f, 0.0f, 0.0f);
				glUniform3f(kdUniform, 0.5f, 0.4f, 0.4f);
				glUniform3f(ksUniform, 0.7f, 0.04f, 0.04f);
				glUniform1f(materialShininessUniform, 0.078125f * 128.0f);
				translationMatrix = vmath::translate(4.5f, 3.0f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [5, 4] material: white rubber
				glUniform3f(kaUniform, 0.05f, 0.05f, 0.05f);
				glUniform3f(kdUniform, 0.5f, 0.5f, 0.5f);
				glUniform3f(ksUniform, 0.7f, 0.7f, 0.7f);
				glUniform1f(materialShininessUniform, 0.078125f * 128.0f);
				translationMatrix = vmath::translate(4.5f, 1.5f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [6, 4] material: yellow rubber
				glUniform3f(kaUniform, 0.05f, 0.05f, 0.0f);
				glUniform3f(kdUniform, 0.5f, 0.5f, 0.4f);
				glUniform3f(ksUniform, 0.7f, 0.7f, 0.04f);
				glUniform1f(materialShininessUniform, 0.078125f * 128.0f);
				translationMatrix = vmath::translate(4.5f, 0.0f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);
			}
			else
			{
				glUniform1i(lightEnabledUniform, 0);

				// [1, 1]
				translationMatrix = vmath::translate(-4.5f, 7.5f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [2, 1]
				translationMatrix = vmath::translate(-4.5f, 6.0f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [3, 1]
				translationMatrix = vmath::translate(-4.5f, 4.5f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [4, 1]
				translationMatrix = vmath::translate(-4.5f, 3.0f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [5, 1]
				translationMatrix = vmath::translate(-4.5f, 1.5f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [6, 1]
				translationMatrix = vmath::translate(-4.5f, 0.0f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [1, 2]
				translationMatrix = vmath::translate(-1.5f, 7.5f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [2, 2]
				translationMatrix = vmath::translate(-1.5f, 6.0f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [3, 2]
				translationMatrix = vmath::translate(-1.5f, 4.5f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [4, 2]
				translationMatrix = vmath::translate(-1.5f, 3.0f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [5, 2]
				translationMatrix = vmath::translate(-1.5f, 1.5f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [6, 2]
				translationMatrix = vmath::translate(-1.5f, 0.0f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [1, 3]
				translationMatrix = vmath::translate(1.5f, 7.5f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [2, 3]
				translationMatrix = vmath::translate(1.5f, 6.0f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [3, 3]
				translationMatrix = vmath::translate(1.5f, 4.5f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [4, 3]
				translationMatrix = vmath::translate(1.5f, 3.0f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [5, 3]
				translationMatrix = vmath::translate(1.5f, 1.5f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [6, 3]
				translationMatrix = vmath::translate(1.5f, 0.0f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [1, 4]
				translationMatrix = vmath::translate(4.5f, 7.5f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [2, 4]
				translationMatrix = vmath::translate(4.5f, 6.0f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [3, 4]
				translationMatrix = vmath::translate(4.5f, 4.5f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [4, 4]
				translationMatrix = vmath::translate(4.5f, 3.0f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [5, 4]
				translationMatrix = vmath::translate(4.5f, 1.5f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				// [6, 4]
				translationMatrix = vmath::translate(4.5f, 0.0f, 0.0f);
				modelMatrix = translationMatrix;
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
				glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
				glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);
			}
		}
		glBindVertexArray(0);
	}
	glUseProgram(0);
}
