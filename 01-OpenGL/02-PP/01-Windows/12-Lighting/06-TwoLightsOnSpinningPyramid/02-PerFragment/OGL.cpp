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
GLuint vboVertexPositions;
GLuint vboVertexNormals;

// transformation matrices and uniforms
GLint modelMatrixUniform;
GLint viewMatrixUniform;
GLint projectionMatrixUniform;

GLint laUniform[2];
GLint ldUniform[2];
GLint lsUniform[2];
GLint lightPositionUniform[2];

GLint kaUniform;
GLint kdUniform;
GLint ksUniform;
GLint materialShininessUniform;

GLint lightEnabledUniform;

mat4 perspectiveProjectionMatrix;

// animation
float angleY;

// ADS light
typedef struct ADSLight
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 position;
} Light;

Light lights[2];

// material
GLfloat materialAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat materialDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat materialSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat materialShininess = 50.0f;

// light switch
BOOL bLight = FALSE;

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
			{
				bLight = FALSE;
			}
			else
			{
				bLight = TRUE;
			}
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
		"uniform vec4 u_lightPosition[2];\n" \
		"uniform int u_lightEnabled;\n" \
		"\n" \
		"out vec3 transformedNormal;\n" \
		"out vec3 lightDirection[2];\n" \
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
				"viewerVector = -eyeCoordinate.xyz;\n" \
				"\n" \
				"for (int i = 0; i < 2; i++)\n" \
				"{\n" \
					"lightDirection[i] = vec3(u_lightPosition[i]) - eyeCoordinate.xyz;\n" \
				"}\n" \
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
		"in vec3 lightDirection[2];\n" \
		"in vec3 viewerVector;\n" \
		"\n" \
		"uniform vec3 u_La[2];\n" \
		"uniform vec3 u_Ld[2];\n" \
		"uniform vec3 u_Ls[2];\n" \
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
			"vec3 phongADSLight = vec3(0.0, 0.0, 0.0);\n" \
			"\n" \
			"if (u_lightEnabled == 1)\n" \
			"{\n" \
				"vec3 normalizedTransformedNormal = normalize(transformedNormal);\n" \
				"vec3 normalizedViewerVector = normalize(viewerVector);\n" \
				"\n" \
				"vec3 componentAmbient[2];\n" \
				"vec3 normalizedLightDirection[2];\n" \
				"vec3 componentDiffuse[2];\n" \
				"vec3 reflectionVector[2];\n" \
				"vec3 componentSpecular[2];\n" \
				"for (int i = 0; i < 2; i++)\n" \
				"{\n" \
					"componentAmbient[i] = u_La[i] * u_Ka;\n" \
					"\n" \
					"normalizedLightDirection[i] = normalize(lightDirection[i]);\n" \
					"componentDiffuse[i] = u_Ld[i] * u_Kd * max(dot(normalizedLightDirection[i], normalizedTransformedNormal), 0.0);\n" \
					"\n" \
					"reflectionVector[i] = reflect(-normalizedLightDirection[i], normalizedTransformedNormal);\n" \
					"componentSpecular[i] = u_Ls[i] * u_Ks * pow(max(dot(reflectionVector[i], normalizedViewerVector), 0.0), u_materialShininess);\n" \
					"\n" \
					"phongADSLight += componentAmbient[i] + componentDiffuse[i] + componentSpecular[i];\n" \
				"}\n" \
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

	laUniform[0] = glGetUniformLocation(shaderProgramObject, "u_La[0]");
	ldUniform[0] = glGetUniformLocation(shaderProgramObject, "u_Ld[0]");
	lsUniform[0] = glGetUniformLocation(shaderProgramObject, "u_Ls[0]");
	lightPositionUniform[0] = glGetUniformLocation(shaderProgramObject, "u_lightPosition[0]");

	laUniform[1] = glGetUniformLocation(shaderProgramObject, "u_La[1]");
	ldUniform[1] = glGetUniformLocation(shaderProgramObject, "u_Ld[1]");
	lsUniform[1] = glGetUniformLocation(shaderProgramObject, "u_Ls[1]");
	lightPositionUniform[1] = glGetUniformLocation(shaderProgramObject, "u_lightPosition[1]");

	kaUniform = glGetUniformLocation(shaderProgramObject, "u_Ka");
	kdUniform = glGetUniformLocation(shaderProgramObject, "u_Kd");
	ksUniform = glGetUniformLocation(shaderProgramObject, "u_Ks");
	materialShininessUniform = glGetUniformLocation(shaderProgramObject, "u_materialShininess");

	lightEnabledUniform = glGetUniformLocation(shaderProgramObject, "u_lightEnabled");
	
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	{
		// position vbo
		glGenBuffers(1, &vboVertexPositions);
		glBindBuffer(GL_ARRAY_BUFFER, vboVertexPositions);
		{
			const GLfloat vertexPositions[] = {
				// front
				0.0f,  1.0f, 0.0f,
			   -1.0f, -1.0f, 1.0f,
				1.0f, -1.0f, 1.0f,

				// right
				0.0f,  1.0f,  0.0f,
				1.0f, -1.0f,  1.0f,
				1.0f, -1.0f, -1.0f,

				// back
				0.0f,  1.0f,  0.0f,
				1.0f, -1.0f, -1.0f,
			   -1.0f, -1.0f, -1.0f,

			   // left
			   0.0f,  1.0f,  0.0f,
			  -1.0f, -1.0f, -1.0f,
			  -1.0f, -1.0f,  1.0f
			};

			glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);
			glVertexAttribPointer(KVD_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
			glEnableVertexAttribArray(KVD_ATTRIBUTE_POSITION);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// normals vbo
		glGenBuffers(1, &vboVertexNormals);
		glBindBuffer(GL_ARRAY_BUFFER, vboVertexNormals);
		{
			const GLfloat vertexNormals[] = {
				0.0f, 0.447214f, 0.894427f, // front-top
				0.0f, 0.447214f, 0.894427f, // front-left
				0.0f, 0.447214f, 0.894427f, // front-right

				0.894427f, 0.447214f, 0.0f, // right-top
				0.894427f, 0.447214f, 0.0f, // right-left
				0.894427f, 0.447214f, 0.0f, // right-right

				0.0f, 0.447214f, -0.894427f, // back-top
				0.0f, 0.447214f, -0.894427f, // back-left
				0.0f, 0.447214f, -0.894427f, // back-right

			   -0.894427f, 0.447214f, 0.0f, // left-top
			   -0.894427f, 0.447214f, 0.0f, // left-left
			   -0.894427f, 0.447214f, 0.0f  // left-right
			};

			glBufferData(GL_ARRAY_BUFFER, sizeof(vertexNormals), vertexNormals, GL_STATIC_DRAW);
			glVertexAttribPointer(KVD_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
			glEnableVertexAttribArray(KVD_ATTRIBUTE_NORMAL);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	glBindVertexArray(0);

	// set clear color
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// depth-related changes
	glClearDepth(1.0f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);

	// lights
	lights[0].ambient = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	lights[0].diffuse = vmath::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	lights[0].specular = vmath::vec4( 1.0f, 0.0f, 0.0f, 1.0f);
	lights[0].position = vmath::vec4(-2.0f, 0.0f, 0.0f, 1.0f);

	lights[1].ambient = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	lights[1].diffuse = vmath::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	lights[1].specular = vmath::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	lights[1].position = vmath::vec4(2.0f, 0.0f, 0.0f, 1.0f);

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
	perspectiveProjectionMatrix = vmath::perspective(45.0f, aspectRatio, 0.1f, 100.0f);
}

void display(void)
{
	// variable declarations
	mat4 translationMatrix = mat4::identity();
	mat4 rotationMatrix = mat4::identity();

	mat4 modelMatrix = mat4::identity();
	mat4 viewMatrix = mat4::identity();

	// code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// transformations
	translationMatrix = vmath::translate(0.0f, 0.0f, -6.0f);
	rotationMatrix = vmath::rotate(angleY, 0.0f, 1.0f, 0.0f);
	modelMatrix = translationMatrix * rotationMatrix;

	// draw
	glUseProgram(shaderProgramObject);
	{
		glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
		glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
		glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

		if (bLight)
		{
			for (int i = 0; i < 2; i++)
			{
				glUniform3fv(laUniform[i], 1, lights[i].ambient);
				glUniform3fv(ldUniform[i], 1, lights[i].diffuse);
				glUniform3fv(lsUniform[i], 1, lights[i].specular);
				glUniform4fv(lightPositionUniform[i], 1, lights[i].position);
			}

			glUniform3fv(kaUniform, 1, materialAmbient);
			glUniform3fv(kdUniform, 1, materialDiffuse);
			glUniform3fv(ksUniform, 1, materialSpecular);
			glUniform1f(materialShininessUniform, materialShininess);

			glUniform1i(lightEnabledUniform, 1);
		}
		else
		{
			glUniform1i(lightEnabledUniform, 0);
		}

		glBindVertexArray(vao);
			glDrawArrays(GL_TRIANGLES, 0, 12);
		glBindVertexArray(0);
	}
	glUseProgram(0);

	SwapBuffers(ghdc);
}

void update(void)
{
	// code
	angleY = angleY + 1.0f;
	if (angleY >= 360.0f)
		angleY = angleY - 360.0f;
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

	if (vboVertexPositions)
	{
		glDeleteBuffers(1, &vboVertexPositions);
		vboVertexPositions = 0;
	}

	if (vboVertexNormals)
	{
		glDeleteBuffers(1, &vboVertexNormals);
		vboVertexNormals = 0;
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
