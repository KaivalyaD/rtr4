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
GLuint pv_shaderProgramObject;  // for per-vertex lighting
GLuint pf_shaderProgramObject;  // for per-fragment lighting

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
GLint pv_modelMatrixUniform;
GLint pv_viewMatrixUniform;
GLint pv_projectionMatrixUniform;
GLint pv_laUniform[3];
GLint pv_ldUniform[3];
GLint pv_lsUniform[3];
GLint pv_lightPositionUniform[3];
GLint pv_kaUniform;
GLint pv_kdUniform;
GLint pv_ksUniform;
GLint pv_materialShininessUniform;
GLint pv_lightEnabledUniform;

GLint pf_modelMatrixUniform;
GLint pf_viewMatrixUniform;
GLint pf_projectionMatrixUniform;
GLint pf_laUniform[3];
GLint pf_ldUniform[3];
GLint pf_lsUniform[3];
GLint pf_lightPositionUniform[3];
GLint pf_kaUniform;
GLint pf_kdUniform;
GLint pf_ksUniform;
GLint pf_materialShininessUniform;
GLint pf_lightEnabledUniform;

mat4 perspectiveProjectionMatrix;

// object data RAM
float sphereVertices[1146];
float sphereNormals[1146];
float sphereTexCoords[764];
unsigned short sphereElements[2280];
unsigned int numVertices, numElements;

// ADS light
typedef struct ADSLight
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 position;
} Light;

Light lights[3];

// white
GLfloat materialAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat materialDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat materialSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat materialShininess = 50.0f;

// light switch
BOOL bLights = FALSE;
BOOL bPerVertexLights = TRUE;

// animation
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
			ToggleFullScreen();
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
			/* if the lights are on, switch to per-fragment lighting */
			if (bLights && bPerVertexLights)
				bPerVertexLights = FALSE;
			break;

		case 'L':
		case 'l':
			/* toggle the lights on or off */
			if (bLights)
				bLights = FALSE;
			else
				bLights = TRUE;
			break;

		case 'Q':
		case 'q':
			/* quit */
			fprintf(gpLog, "destroying after receiving 'Q'\n");
			DestroyWindow(hwnd);
			break;

		case 'V':
		case 'v':
			/* if the lights are on, switch to per-vertex lighting */
			if (bLights && !bPerVertexLights)
				bPerVertexLights = TRUE;
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

	// per-vertex lighting vertex shader
	status = 0;
	infoLogLength = 0;
	log = NULL;

	const GLchar *pv_vertexShaderSourceCode =
		"#version 460 core\n" \
		"\n" \
		"in vec4 a_position;\n" \
		"in vec3 a_normal;\n" \
		"\n" \
		"uniform mat4 u_modelMatrix;\n" \
		"uniform mat4 u_viewMatrix;\n" \
		"uniform mat4 u_projectionMatrix;\n" \
		"\n" \
		"uniform vec3 u_La[3];\n" \
		"uniform vec3 u_Ld[3];\n" \
		"uniform vec3 u_Ls[3];\n" \
		"uniform vec4 u_lightPosition[3];\n" \
		"\n" \
		"uniform vec3 u_Ka;\n" \
		"uniform vec3 u_Kd;\n" \
		"uniform vec3 u_Ks;\n" \
		"uniform float u_materialShininess;\n" \
		"\n" \
		"uniform int u_lightEnabled;\n" \
		"\n" \
		"out vec3 phongADSLight;\n" \
		"\n" \
		"void main(void)\n" \
		"{\n" \
			"if (u_lightEnabled == 1)\n" \
			"{\n" \
				"vec4 eyeCoordinate = u_viewMatrix * u_modelMatrix * a_position;\n" \
				"mat3 normalMatrix = mat3(u_viewMatrix * u_modelMatrix);\n" \
				"vec3 transformedNormal = normalize(normalMatrix * a_normal);\n" \
				"\n" \
				"vec3 viewerVector = normalize(-eyeCoordinate.xyz);\n" \
				"\n" \
				"vec3 componentAmbient[3];\n" \
				"vec3 lightDirection[3];\n" \
				"vec3 componentDiffuse[3];\n" \
				"vec3 reflectionVector[3];\n" \
				"vec3 componentSpecular[3];\n" \
				"for (int i = 0; i < 3; i++)\n" \
				"{\n" \
					"componentAmbient[i] = u_La[i] * u_Ka;\n" \
					"\n" \
					"lightDirection[i] = normalize(vec3(u_lightPosition[i]) - eyeCoordinate.xyz);\n" \
					"componentDiffuse[i] = u_Ld[i] * u_Kd * max(dot(lightDirection[i], transformedNormal), 0.0);\n" \
					"\n" \
					"reflectionVector[i] = reflect(-lightDirection[i], transformedNormal);\n" \
					"componentSpecular[i] = u_Ls[i] * u_Ks * pow(max(dot(reflectionVector[i], viewerVector), 0.0), u_materialShininess);\n" \
					"\n" \
					"phongADSLight += componentAmbient[i] + componentDiffuse[i] + componentSpecular[i];\n" \
				"}\n" \
			"}\n" \
			"else\n" \
			"{\n" \
				"phongADSLight = vec3(1.0, 1.0, 1.0);\n" \
			"}\n" \
			"\n" \
			"gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * a_position;\n" \
		"}\n";

	GLuint pv_vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(pv_vertexShaderObject, 1, (const GLchar **)&pv_vertexShaderSourceCode, NULL);
	glCompileShader(pv_vertexShaderObject);
	glGetShaderiv(pv_vertexShaderObject, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		fprintf(gpLog, "*** per-vertex: vertex shader compilation errors ***\n");

		glGetShaderiv(pv_vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			log = (char *)malloc(infoLogLength * sizeof(char));
			if (log)
			{
				GLsizei written = 0;

				glGetShaderInfoLog(pv_vertexShaderObject, infoLogLength * sizeof(char), &written, log);
				fprintf(gpLog, "perv-vertex: vertex shader compilation log (%d bytes):\n%s\n", written, log);
				free(log);
				log = NULL;
			}
			else
				fprintf(gpLog, "\tper-vertex: malloc: cannot allocate memory to hold the compilation log\n");
		}
		else
			fprintf(gpLog, "\tper-vertex: there is nothing to print\n");

		uninitialize();	// application exits as calling DestroyWindow() in uninitialize()
	}
	fprintf(gpLog, "per-vertex: vertex shader was compiled without errors\n");

	// per-vertex lighting fragment shader
	status = 0;
	infoLogLength = 0;
	log = NULL;

	const GLchar *pv_fragmentShaderSourceCode =
		"#version 460 core\n" \
		"\n" \
		"in vec3 phongADSLight;\n" \
		"\n" \
		"out vec4 FragColor;\n" \
		"\n" \
		"void main(void)\n" \
		"{\n" \
			"FragColor = vec4(phongADSLight, 1.0);\n" \
		"}\n";

	GLuint pv_fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(pv_fragmentShaderObject, 1, (const GLchar **)&pv_fragmentShaderSourceCode, NULL);
	glCompileShader(pv_fragmentShaderObject);
	glGetShaderiv(pv_fragmentShaderObject, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		fprintf(gpLog, "*** per-vertex: fragment shader compilation errors ***\n");

		glGetShaderiv(pv_fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			log = (char *)malloc(infoLogLength * sizeof(char));
			if (log)
			{
				GLsizei written = 0;

				glGetShaderInfoLog(pv_fragmentShaderObject, infoLogLength, &written, log);
				fprintf(gpLog, "per-vertex: fragment shader compilation log (%d bytes):\n%s\n", written, log);
				free(log);
				log = NULL;
			}
			else
				fprintf(gpLog, "\tper-vertex: malloc: cannot allocate memory to hold the compilation log\n");
		}
		else
			fprintf(gpLog, "\tper-vertex: there is nothing to print\n");

		uninitialize();
	}
	fprintf(gpLog, "per-vertex: fragment shader was compiled without errors\n");

	// per-vertex lighting shader program
	status = 0;
	infoLogLength = 0;
	log = NULL;

	pv_shaderProgramObject = glCreateProgram();
	glAttachShader(pv_shaderProgramObject, pv_vertexShaderObject);
	glAttachShader(pv_shaderProgramObject, pv_fragmentShaderObject);
	glBindAttribLocation(pv_shaderProgramObject, KVD_ATTRIBUTE_POSITION, "a_position");
	glBindAttribLocation(pv_shaderProgramObject, KVD_ATTRIBUTE_NORMAL, "a_normal");
	glLinkProgram(pv_shaderProgramObject);

	glGetProgramiv(pv_shaderProgramObject, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		fprintf(gpLog, "*** per-vertex: there were linking errors ***\n");

		glGetProgramiv(pv_shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			log = (char *)malloc(infoLogLength * sizeof(char));
			if (log)
			{
				GLsizei written = 0;

				glGetProgramInfoLog(pv_shaderProgramObject, infoLogLength * sizeof(char), &written, log);
				fprintf(gpLog, "\tper-vertex: link time info log (%d bytes):\n%s\n", written, log);
				free(log);
				log = NULL;
			}
			else
				fprintf(gpLog, "\tper-vertex: malloc: cannot allocate memory to hold the linking log\n");
		}
		else
			fprintf(gpLog, "\tper-vertex: there is nothing to print\n");

		uninitialize();
	}
	fprintf(gpLog, "per-vertex: shader program was linked without errors\n");

	// per-vertex lighting shader program post-linking processing
	pv_modelMatrixUniform = glGetUniformLocation(pv_shaderProgramObject, "u_modelMatrix");
	pv_viewMatrixUniform = glGetUniformLocation(pv_shaderProgramObject, "u_viewMatrix");
	pv_projectionMatrixUniform = glGetUniformLocation(pv_shaderProgramObject, "u_projectionMatrix");

	pv_laUniform[0] = glGetUniformLocation(pv_shaderProgramObject, "u_La[0]");
	pv_ldUniform[0] = glGetUniformLocation(pv_shaderProgramObject, "u_Ld[0]");
	pv_lsUniform[0] = glGetUniformLocation(pv_shaderProgramObject, "u_Ls[0]");
	pv_lightPositionUniform[0] = glGetUniformLocation(pv_shaderProgramObject, "u_lightPosition[0]");

	pv_laUniform[1] = glGetUniformLocation(pv_shaderProgramObject, "u_La[1]");
	pv_ldUniform[1] = glGetUniformLocation(pv_shaderProgramObject, "u_Ld[1]");
	pv_lsUniform[1] = glGetUniformLocation(pv_shaderProgramObject, "u_Ls[1]");
	pv_lightPositionUniform[1] = glGetUniformLocation(pv_shaderProgramObject, "u_lightPosition[1]");

	pv_laUniform[2] = glGetUniformLocation(pv_shaderProgramObject, "u_La[2]");
	pv_ldUniform[2] = glGetUniformLocation(pv_shaderProgramObject, "u_Ld[2]");
	pv_lsUniform[2] = glGetUniformLocation(pv_shaderProgramObject, "u_Ls[2]");
	pv_lightPositionUniform[2] = glGetUniformLocation(pv_shaderProgramObject, "u_lightPosition[2]");

	pv_kaUniform = glGetUniformLocation(pv_shaderProgramObject, "u_Ka");
	pv_kdUniform = glGetUniformLocation(pv_shaderProgramObject, "u_Kd");
	pv_ksUniform = glGetUniformLocation(pv_shaderProgramObject, "u_Ks");
	pv_materialShininessUniform = glGetUniformLocation(pv_shaderProgramObject, "u_materialShininess");

	pv_lightEnabledUniform = glGetUniformLocation(pv_shaderProgramObject, "u_lightEnabled");

	// per-fragment lighting vertex shader
	status = 0;
	infoLogLength = 0;
	log = NULL;

	const GLchar *pf_vertexShaderSourceCode =
		"#version 460 core\n" \
		"\n" \
		"in vec4 a_position;\n" \
		"in vec3 a_normal;\n" \
		"\n" \
		"uniform mat4 u_modelMatrix;\n" \
		"uniform mat4 u_viewMatrix;\n" \
		"uniform mat4 u_projectionMatrix;\n" \
		"\n" \
		"uniform vec4 u_lightPosition[3];\n" \
		"uniform int u_lightEnabled;\n" \
		"\n" \
		"out vec3 transformedNormal;\n" \
		"out vec3 lightDirection[3];\n" \
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
				"for (int i = 0; i < 3; i++)\n" \
				"{\n" \
					"lightDirection[i] = vec3(u_lightPosition[i]) - eyeCoordinate.xyz;\n" \
				"}\n" \
			"}\n" \
			"\n" \
			"gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * a_position;\n" \
		"}\n";

	GLuint pf_vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(pf_vertexShaderObject, 1, (const GLchar **)&pf_vertexShaderSourceCode, NULL);
	glCompileShader(pf_vertexShaderObject);
	glGetShaderiv(pf_vertexShaderObject, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		fprintf(gpLog, "*** per-fragment: vertex shader compilation errors ***\n");

		glGetShaderiv(pf_vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			log = (char *)malloc(infoLogLength * sizeof(char));
			if (log)
			{
				GLsizei written = 0;

				glGetShaderInfoLog(pf_vertexShaderObject, infoLogLength * sizeof(char), &written, log);
				fprintf(gpLog, "per-fragment: vertex shader compilation log (%d bytes):\n%s\n", written, log);
				free(log);
				log = NULL;
			}
			else
				fprintf(gpLog, "\tper-fragment: malloc: cannot allocate memory to hold the compilation log\n");
		}
		else
			fprintf(gpLog, "\tper-fragment: there is nothing to print\n");

		uninitialize();	// application exits as calling DestroyWindow() in uninitialize()
	}
	fprintf(gpLog, "per-fragment: vertex shader was compiled without errors\n");

	// per-fragment lighting fragment shader
	status = 0;
	infoLogLength = 0;
	log = NULL;

	const GLchar *pf_fragmentShaderSourceCode =
		"#version 460 core\n" \
		"\n" \
		"in vec3 transformedNormal;\n" \
		"in vec3 lightDirection[3];\n" \
		"in vec3 viewerVector;\n" \
		"\n" \
		"uniform vec3 u_La[3];\n" \
		"uniform vec3 u_Ld[3];\n" \
		"uniform vec3 u_Ls[3];\n" \
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
				"vec3 componentAmbient[3];\n" \
				"vec3 normalizedLightDirection[3];\n" \
				"vec3 componentDiffuse[3];\n" \
				"vec3 reflectionVector[3];\n" \
				"vec3 componentSpecular[3];\n" \
				"for (int i = 0; i < 3; i++)\n" \
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

	GLuint pf_fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(pf_fragmentShaderObject, 1, (const GLchar **)&pf_fragmentShaderSourceCode, NULL);
	glCompileShader(pf_fragmentShaderObject);
	glGetShaderiv(pf_fragmentShaderObject, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		fprintf(gpLog, "*** per-fragment: fragment shader compilation errors ***\n");

		glGetShaderiv(pf_fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			log = (char *)malloc(infoLogLength * sizeof(char));
			if (log)
			{
				GLsizei written = 0;

				glGetShaderInfoLog(pf_fragmentShaderObject, infoLogLength, &written, log);
				fprintf(gpLog, "per-fragment: fragment shader compilation log (%d bytes):\n%s\n", written, log);
				free(log);
				log = NULL;
			}
			else
				fprintf(gpLog, "\tper-fragment: malloc: cannot allocate memory to hold the compilation log\n");
		}
		else
			fprintf(gpLog, "\tper-fragment: there is nothing to print\n");

		uninitialize();
	}
	fprintf(gpLog, "per-fragment: fragment shader was compiled without errors\n");

	// per-fragment lighting shader program
	status = 0;
	infoLogLength = 0;
	log = NULL;

	pf_shaderProgramObject = glCreateProgram();
	glAttachShader(pf_shaderProgramObject, pf_vertexShaderObject);
	glAttachShader(pf_shaderProgramObject, pf_fragmentShaderObject);
	glBindAttribLocation(pf_shaderProgramObject, KVD_ATTRIBUTE_POSITION, "a_position");
	glBindAttribLocation(pf_shaderProgramObject, KVD_ATTRIBUTE_NORMAL, "a_normal");
	glLinkProgram(pf_shaderProgramObject);

	glGetProgramiv(pf_shaderProgramObject, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		fprintf(gpLog, "*** per-fragment: there were linking errors ***\n");

		glGetProgramiv(pf_shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			log = (char *)malloc(infoLogLength * sizeof(char));
			if (log)
			{
				GLsizei written = 0;

				glGetProgramInfoLog(pf_shaderProgramObject, infoLogLength * sizeof(char), &written, log);
				fprintf(gpLog, "\tper-fragment: link time info log (%d bytes):\n%s\n", written, log);
				free(log);
				log = NULL;
			}
			else
				fprintf(gpLog, "\tper-fragment: malloc: cannot allocate memory to hold the linking log\n");
		}
		else
			fprintf(gpLog, "\tper-fragment: there is nothing to print\n");

		uninitialize();
	}
	fprintf(gpLog, "per-fragment: shader program was linked without errors\n");

	// per-fragment lighting shader program post-linking processing
	pf_modelMatrixUniform = glGetUniformLocation(pf_shaderProgramObject, "u_modelMatrix");
	pf_viewMatrixUniform = glGetUniformLocation(pf_shaderProgramObject, "u_viewMatrix");
	pf_projectionMatrixUniform = glGetUniformLocation(pf_shaderProgramObject, "u_projectionMatrix");

	pf_laUniform[0] = glGetUniformLocation(pf_shaderProgramObject, "u_La[0]");
	pf_ldUniform[0] = glGetUniformLocation(pf_shaderProgramObject, "u_Ld[0]");
	pf_lsUniform[0] = glGetUniformLocation(pf_shaderProgramObject, "u_Ls[0]");
	pf_lightPositionUniform[0] = glGetUniformLocation(pf_shaderProgramObject, "u_lightPosition[0]");

	pf_laUniform[1] = glGetUniformLocation(pf_shaderProgramObject, "u_La[1]");
	pf_ldUniform[1] = glGetUniformLocation(pf_shaderProgramObject, "u_Ld[1]");
	pf_lsUniform[1] = glGetUniformLocation(pf_shaderProgramObject, "u_Ls[1]");
	pf_lightPositionUniform[1] = glGetUniformLocation(pf_shaderProgramObject, "u_lightPosition[1]");

	pf_laUniform[2] = glGetUniformLocation(pf_shaderProgramObject, "u_La[2]");
	pf_ldUniform[2] = glGetUniformLocation(pf_shaderProgramObject, "u_Ld[2]");
	pf_lsUniform[2] = glGetUniformLocation(pf_shaderProgramObject, "u_Ls[2]");
	pf_lightPositionUniform[2] = glGetUniformLocation(pf_shaderProgramObject, "u_lightPosition[2]");

	pf_kaUniform = glGetUniformLocation(pf_shaderProgramObject, "u_Ka");
	pf_kdUniform = glGetUniformLocation(pf_shaderProgramObject, "u_Kd");
	pf_ksUniform = glGetUniformLocation(pf_shaderProgramObject, "u_Ks");
	pf_materialShininessUniform = glGetUniformLocation(pf_shaderProgramObject, "u_materialShininess");

	pf_lightEnabledUniform = glGetUniformLocation(pf_shaderProgramObject, "u_lightEnabled");
	
	// vao
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
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// depth-related changes
	glClearDepth(1.0f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);

	// lights
	lights[0].ambient = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	lights[0].diffuse = vmath::vec4(1.0f, 0.0f, 0.0f, 1.0f);  // red
	lights[0].specular = vmath::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	lights[0].position = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	lights[1].ambient = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	lights[1].diffuse = vmath::vec4(0.0f, 1.0f, 0.0f, 1.0f);  // blue
	lights[1].specular = vmath::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	lights[1].position = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	lights[2].ambient = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	lights[2].diffuse = vmath::vec4(0.0f, 0.0f, 1.0f, 1.0f);  // green
	lights[2].specular = vmath::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	lights[2].position = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);

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
	mat4 modelMatrix = mat4::identity();
	mat4 viewMatrix = mat4::identity();

	// code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// transformations
	translationMatrix = vmath::translate(0.0f, 0.0f, -2.0f);
	modelMatrix = translationMatrix;

	// draw
	if (bPerVertexLights)
	{
		glUseProgram(pv_shaderProgramObject);
		{
			glUniformMatrix4fv(pv_modelMatrixUniform, 1, GL_FALSE, modelMatrix);
			glUniformMatrix4fv(pv_viewMatrixUniform, 1, GL_FALSE, viewMatrix);
			glUniformMatrix4fv(pv_projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

			if (bLights)
			{
				lights[0].position = vmath::vec4(0.0f, 10.0f * sinf(DEG_TO_RAD(theta)), 10.0f * cosf(DEG_TO_RAD(theta)), 1.0f);
				lights[1].position = vmath::vec4(10.0f * cosf(DEG_TO_RAD(theta)), 0.0f, 10.0f * sinf(DEG_TO_RAD(theta)), 1.0f);
				lights[2].position = vmath::vec4(10.0f * cosf(DEG_TO_RAD(theta)), 10.0f * sinf(DEG_TO_RAD(theta)), 0.0f, 1.0f);

				for (int i = 0; i < 3; i++)
				{
					glUniform3fv(pv_laUniform[i], 1, lights[i].ambient);
					glUniform3fv(pv_ldUniform[i], 1, lights[i].diffuse);
					glUniform3fv(pv_lsUniform[i], 1, lights[i].specular);
					glUniform4fv(pv_lightPositionUniform[i], 1, lights[i].position);
				}

				glUniform3fv(pv_kaUniform, 1, materialAmbient);
				glUniform3fv(pv_kdUniform, 1, materialDiffuse);
				glUniform3fv(pv_ksUniform, 1, materialSpecular);
				glUniform1f(pv_materialShininessUniform, materialShininess);

				glUniform1i(pv_lightEnabledUniform, 1);
			}
			else
			{
				glUniform1i(pv_lightEnabledUniform, 0);
			}

			glBindVertexArray(vao);
			{
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);
			}
			glBindVertexArray(0);
		}
		glUseProgram(0);
	}
	else
	{
		glUseProgram(pf_shaderProgramObject);
		{
			glUniformMatrix4fv(pf_modelMatrixUniform, 1, GL_FALSE, modelMatrix);
			glUniformMatrix4fv(pf_viewMatrixUniform, 1, GL_FALSE, viewMatrix);
			glUniformMatrix4fv(pf_projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

			if (bLights)
			{
				lights[0].position = vmath::vec4(0.0f, 10.0f * sinf(DEG_TO_RAD(theta)), 10.0f * cosf(DEG_TO_RAD(theta)), 1.0f);
				lights[1].position = vmath::vec4(10.0f * cosf(DEG_TO_RAD(theta)), 0.0f, 10.0f * sinf(DEG_TO_RAD(theta)), 1.0f);
				lights[2].position = vmath::vec4(10.0f * cosf(DEG_TO_RAD(theta)), 10.0f * sinf(DEG_TO_RAD(theta)), 0.0f, 1.0f);

				for (int i = 0; i < 3; i++)
				{
					glUniform3fv(pf_laUniform[i], 1, lights[i].ambient);
					glUniform3fv(pf_ldUniform[i], 1, lights[i].diffuse);
					glUniform3fv(pf_lsUniform[i], 1, lights[i].specular);
					glUniform4fv(pf_lightPositionUniform[i], 1, lights[i].position);
				}

				glUniform3fv(pf_kaUniform, 1, materialAmbient);
				glUniform3fv(pf_kdUniform, 1, materialDiffuse);
				glUniform3fv(pf_ksUniform, 1, materialSpecular);
				glUniform1f(pf_materialShininessUniform, materialShininess);

				glUniform1i(pf_lightEnabledUniform, 1);
			}
			else
			{
				glUniform1i(pf_lightEnabledUniform, 0);
			}

			glBindVertexArray(vao);
			{
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);
			}
			glBindVertexArray(0);
		}
		glUseProgram(0);
	}

	SwapBuffers(ghdc);
}

void update(void)
{
	// code
	theta = theta + 1.0f;
	if (theta >= 360.0f)
		theta = theta - 360.0f;
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

	if (pf_shaderProgramObject)
	{
		GLsizei numAttachedShaders = 0;
		GLuint *shaderObjects = NULL;

		glUseProgram(pf_shaderProgramObject);

		glGetProgramiv(pf_shaderProgramObject, GL_ATTACHED_SHADERS, &numAttachedShaders);
		shaderObjects = (GLuint *)malloc(numAttachedShaders * sizeof(GLuint));
		glGetAttachedShaders(pf_shaderProgramObject, numAttachedShaders, &numAttachedShaders, shaderObjects);

		for (GLsizei i = 0; i < numAttachedShaders; i++)
		{
			glDetachShader(pf_shaderProgramObject, shaderObjects[i]);
			glDeleteShader(shaderObjects[i]);
			shaderObjects[i] = 0;
		}
		free(shaderObjects);
		shaderObjects = NULL;
		fprintf(gpLog, "per-fragment: detached and deleted %d shader objects\n", numAttachedShaders);

		glUseProgram(0);
		glDeleteProgram(pf_shaderProgramObject);
		pf_shaderProgramObject = 0;
		fprintf(gpLog, "per-fragment: deleted shader program object\n");
	}
	
	if (pv_shaderProgramObject)
	{
		GLsizei numAttachedShaders = 0;
		GLuint *shaderObjects = NULL;

		glUseProgram(pv_shaderProgramObject);
		
		glGetProgramiv(pv_shaderProgramObject, GL_ATTACHED_SHADERS, &numAttachedShaders);
		shaderObjects = (GLuint *)malloc(numAttachedShaders * sizeof(GLuint));
		glGetAttachedShaders(pv_shaderProgramObject, numAttachedShaders, &numAttachedShaders, shaderObjects);

		for (GLsizei i = 0; i < numAttachedShaders; i++)
		{
			glDetachShader(pv_shaderProgramObject, shaderObjects[i]);
			glDeleteShader(shaderObjects[i]);
			shaderObjects[i] = 0;
		}
		free(shaderObjects);
		shaderObjects = NULL;
		fprintf(gpLog, "per-vertex: detached and deleted %d shader objects\n", numAttachedShaders);

		glUseProgram(0);
		glDeleteProgram(pv_shaderProgramObject);
		pv_shaderProgramObject = 0;
		fprintf(gpLog, "per-vertex: deleted shader program object\n");
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
