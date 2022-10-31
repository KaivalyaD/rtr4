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

#define FBO_WIDTH 512
#define FBO_HEIGHT 512

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

int winWidth;
int winHeight;

// shader program object
GLuint cubeShaderProgramObject;

enum
{
	KVD_ATTRIBUTE_POSITION = 0,
	KVD_ATTRIBUTE_COLOR,
	KVD_ATTRIBUTE_NORMAL,
	KVD_ATTRIBUTE_TEXTURE0,
};

/* scene objects */
GLuint vaoCube;
GLuint vboVertices;
GLuint vboTexCoords;

// transformation matrices and uniforms
GLint mvpMatrixUniform_Cube;
GLint textureSamplerUniform;
mat4 perspectiveProjectionMatrix_Cube;

// animation
GLfloat theta_Cube = 0.0f;

// FBO-related variables
GLuint fbo;
GLuint rbo;
GLuint fboTexture;
bool fboResult = false;

/******* texture-scene variables begin ********/
GLuint sphereShaderProgramObject;

// object data VRAM
GLuint vaoSphere;
GLuint vboSphereVertices;
GLuint vboSphereNormals;
GLuint vboSphereElements;

GLint modelMatrixUniform_Sphere;
GLint viewMatrixUniform_Sphere;
GLint projectionMatrixUniform_Sphere;
GLint laUniform_Sphere[3];
GLint ldUniform_Sphere[3];
GLint lsUniform_Sphere[3];
GLint lightPositionUniform_Sphere[3];
GLint kaUniform_Sphere;
GLint kdUniform_Sphere;
GLint ksUniform_Sphere;
GLint materialShininessUniform_Sphere;
GLint lightEnabledUniform_Sphere;

mat4 perspectiveProjectionMatrix_Sphere;

// object data RAM
float sphereVertices[1146];
float sphereNormals[1146];
float sphereTexCoords[764];
unsigned short sphereElements[2280];
unsigned int sphereNumVertices, sphereNumElements;

// ADS light
typedef struct ADSLight
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 position;
} Light;

Light lights_Sphere[3];

// white
GLfloat materialAmbient_Sphere[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat materialDiffuse_Sphere[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat materialSpecular_Sphere[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat materialShininess_Sphere = 50.0f;

// light switch
BOOL bLights = FALSE;

// animation
GLfloat theta_Sphere = 0.0f;
/******* texture-scene variables end ********/

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
	else if (iRetVal == -6)
	{
		fprintf(gpLog, "createFBO(): failed\n");
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
			/* toggle the lights on or off */
			if (bLights)
				bLights = FALSE;
			else
				bLights = TRUE;
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
	bool createFBO(GLint, GLint);
	int initializeSphere(int, int);
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
		"in vec2 a_texCoord;\n" \
		"\n" \
		"uniform mat4 u_mvpMatrix;\n" \
		"\n" \
		"out vec2 a_texCoord_out;\n" \
		"\n" \
		"void main(void)\n" \
		"{\n" \
			"gl_Position = u_mvpMatrix * a_position;\n" \
			"a_texCoord_out = a_texCoord;\n" \
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
		"in vec2 a_texCoord_out;\n" \
		"\n" \
		"uniform sampler2D u_textureSampler;\n" \
		"\n" \
		"out vec4 FragColor;\n" \
		"\n" \
		"void main(void)\n" \
		"{\n" \
			"FragColor = texture(u_textureSampler, a_texCoord_out);\n" \
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

	cubeShaderProgramObject = glCreateProgram();
	glAttachShader(cubeShaderProgramObject, vertexShaderObject);
	glAttachShader(cubeShaderProgramObject, fragmentShaderObject);
	glBindAttribLocation(cubeShaderProgramObject, KVD_ATTRIBUTE_POSITION, "a_position");
	glBindAttribLocation(cubeShaderProgramObject, KVD_ATTRIBUTE_TEXTURE0, "a_texCoord");
	glLinkProgram(cubeShaderProgramObject);

	glGetProgramiv(cubeShaderProgramObject, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		fprintf(gpLog, "*** there were linking errors ***\n");

		glGetProgramiv(cubeShaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			log = (char *)malloc(infoLogLength * sizeof(char));
			if (log)
			{
				GLsizei written = 0;

				glGetProgramInfoLog(cubeShaderProgramObject, infoLogLength * sizeof(char), &written, log);
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
	mvpMatrixUniform_Cube = glGetUniformLocation(cubeShaderProgramObject, "u_mvpMatrix");
	textureSamplerUniform = glGetUniformLocation(cubeShaderProgramObject, "u_textureSampler");
	
	// vao
	glGenVertexArrays(1, &vaoCube);
	glBindVertexArray(vaoCube);
	{
		// position vbo
		glGenBuffers(1, &vboVertices);
		glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
		{
			const GLfloat vertexPositions[] = {
				// top
				1.0f, 1.0f, -1.0f,
			   -1.0f, 1.0f, -1.0f,
			   -1.0f, 1.0f,  1.0f,
				1.0f, 1.0f,  1.0f,

				// bottom
				1.0f, -1.0f, -1.0f,
			   -1.0f, -1.0f, -1.0f,
			   -1.0f, -1.0f,  1.0f,
			    1.0f, -1.0f,  1.0f,

				// front
				1.0f,  1.0f, 1.0f,
			   -1.0f,  1.0f, 1.0f,
			   -1.0f, -1.0f, 1.0f,
				1.0f, -1.0f, 1.0f,

				// back
				1.0f,  1.0f, -1.0f,
			   -1.0f,  1.0f, -1.0f,
			   -1.0f, -1.0f, -1.0f,
				1.0f, -1.0f, -1.0f,

				// right
				1.0f,  1.0f, -1.0f,
				1.0f,  1.0f,  1.0f,
				1.0f, -1.0f,  1.0f,
				1.0f, -1.0f, -1.0f,

				// left
			   -1.0f,  1.0f,  1.0f,
			   -1.0f,  1.0f, -1.0f,
			   -1.0f, -1.0f, -1.0f,
			   -1.0f, -1.0f,  1.0f
			};

			glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);
			glVertexAttribPointer(KVD_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
			glEnableVertexAttribArray(KVD_ATTRIBUTE_POSITION);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// color vbo
		glGenBuffers(1, &vboTexCoords);
		glBindBuffer(GL_ARRAY_BUFFER, vboTexCoords);
		{
			const GLfloat vertexTexCoords[] = {
				0.0f, 0.0f,
				1.0f, 0.0f,
				1.0f, 1.0f,
				0.0f, 1.0f,

				0.0f, 0.0f,
				1.0f, 0.0f,
				1.0f, 1.0f,
				0.0f, 1.0f,

				0.0f, 0.0f,
				1.0f, 0.0f,
				1.0f, 1.0f,
				0.0f, 1.0f,

				0.0f, 0.0f,
				1.0f, 0.0f,
				1.0f, 1.0f,
				0.0f, 1.0f,

				0.0f, 0.0f,
				1.0f, 0.0f,
				1.0f, 1.0f,
				0.0f, 1.0f,

				0.0f, 0.0f,
				1.0f, 0.0f,
				1.0f, 1.0f,
				0.0f, 1.0f
			};

			glBufferData(GL_ARRAY_BUFFER, sizeof(vertexTexCoords), vertexTexCoords, GL_STATIC_DRAW);
			glVertexAttribPointer(KVD_ATTRIBUTE_TEXTURE0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
			glEnableVertexAttribArray(KVD_ATTRIBUTE_TEXTURE0);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	glBindVertexArray(0);

	// set clear color
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	// depth-related changes
	glClearDepth(1.0f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_TEXTURE_2D);

	perspectiveProjectionMatrix_Cube = mat4::identity();

	// warm-up resize
	GetClientRect(ghwnd, &rc);
	resize(rc.right - rc.left, rc.bottom - rc.top);

	/**************** FBO ****************/
	fboResult = createFBO(FBO_WIDTH, FBO_HEIGHT);
	if (fboResult == true)
	{
		int ret = initializeSphere(FBO_WIDTH, FBO_HEIGHT);
		switch (ret)
		{
		default:
			fprintf(gpLog, "successfully initialized the sphere FBO\n");
			break;
		}
	}
	else
		return -6;

	return 0;
}

int initializeSphere(int width, int height)
{
	// function prototypes
	void resizeSphere(int, int);
	void uninitializeSphere(void);

	// variable declarations
	GLint status;
	GLint infoLogLength;
	char *log = NULL;

	// code
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

	GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderObject, 1, (const GLchar **)&vertexShaderSourceCode, NULL);
	glCompileShader(vertexShaderObject);
	glGetShaderiv(vertexShaderObject, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		fprintf(gpLog, "*** sphere: vertex shader compilation errors ***\n");

		glGetShaderiv(vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			log = (char *)malloc(infoLogLength * sizeof(char));
			if (log)
			{
				GLsizei written = 0;

				glGetShaderInfoLog(vertexShaderObject, infoLogLength * sizeof(char), &written, log);
				fprintf(gpLog, "sphere vertex shader compilation log (%d bytes):\n%s\n", written, log);
				free(log);
				log = NULL;
			}
			else
				fprintf(gpLog, "\tsphere: malloc: cannot allocate memory to hold the compilation log\n");
		}
		else
			fprintf(gpLog, "\tsphere: there is nothing to print\n");

		uninitializeSphere();	// application exits as calling DestroyWindow() in uninitialize()
	}
	fprintf(gpLog, "sphere: vertex shader was compiled without errors\n");

	// fragment shader
	status = 0;
	infoLogLength = 0;
	log = NULL;

	const GLchar *fragmentShaderSourceCode =
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

	GLuint fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderObject, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);
	glCompileShader(fragmentShaderObject);
	glGetShaderiv(fragmentShaderObject, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		fprintf(gpLog, "*** sphere: fragment shader compilation errors ***\n");

		glGetShaderiv(fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			log = (char *)malloc(infoLogLength * sizeof(char));
			if (log)
			{
				GLsizei written = 0;

				glGetShaderInfoLog(fragmentShaderObject, infoLogLength, &written, log);
				fprintf(gpLog, "sphere: fragment shader compilation log (%d bytes):\n%s\n", written, log);
				free(log);
				log = NULL;
			}
			else
				fprintf(gpLog, "\tsphere: malloc: cannot allocate memory to hold the compilation log\n");
		}
		else
			fprintf(gpLog, "\tsphere: there is nothing to print\n");

		uninitializeSphere();
	}
	fprintf(gpLog, "sphere: fragment shader was compiled without errors\n");

	// shader program
	status = 0;
	infoLogLength = 0;
	log = NULL;

	sphereShaderProgramObject = glCreateProgram();
	glAttachShader(sphereShaderProgramObject, vertexShaderObject);
	glAttachShader(sphereShaderProgramObject, fragmentShaderObject);
	glBindAttribLocation(sphereShaderProgramObject, KVD_ATTRIBUTE_POSITION, "a_position");
	glBindAttribLocation(sphereShaderProgramObject, KVD_ATTRIBUTE_NORMAL, "a_normal");
	glLinkProgram(sphereShaderProgramObject);

	glGetProgramiv(sphereShaderProgramObject, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		fprintf(gpLog, "*** sphere: there were linking errors ***\n");

		glGetProgramiv(sphereShaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			log = (char *)malloc(infoLogLength * sizeof(char));
			if (log)
			{
				GLsizei written = 0;

				glGetProgramInfoLog(sphereShaderProgramObject, infoLogLength * sizeof(char), &written, log);
				fprintf(gpLog, "\tsphere: link time info log (%d bytes):\n%s\n", written, log);
				free(log);
				log = NULL;
			}
			else
				fprintf(gpLog, "\tsphere: malloc: cannot allocate memory to hold the linking log\n");
		}
		else
			fprintf(gpLog, "\tsphere: there is nothing to print\n");

		uninitializeSphere();
	}
	fprintf(gpLog, "sphere: shader program was linked without errors\n");

	// per-fragment lighting shader program post-linking processing
	modelMatrixUniform_Sphere = glGetUniformLocation(sphereShaderProgramObject, "u_modelMatrix");
	viewMatrixUniform_Sphere = glGetUniformLocation(sphereShaderProgramObject, "u_viewMatrix");
	projectionMatrixUniform_Sphere = glGetUniformLocation(sphereShaderProgramObject, "u_projectionMatrix");

	laUniform_Sphere[0] = glGetUniformLocation(sphereShaderProgramObject, "u_La[0]");
	ldUniform_Sphere[0] = glGetUniformLocation(sphereShaderProgramObject, "u_Ld[0]");
	lsUniform_Sphere[0] = glGetUniformLocation(sphereShaderProgramObject, "u_Ls[0]");
	lightPositionUniform_Sphere[0] = glGetUniformLocation(sphereShaderProgramObject, "u_lightPosition[0]");

	laUniform_Sphere[1] = glGetUniformLocation(sphereShaderProgramObject, "u_La[1]");
	ldUniform_Sphere[1] = glGetUniformLocation(sphereShaderProgramObject, "u_Ld[1]");
	lsUniform_Sphere[1] = glGetUniformLocation(sphereShaderProgramObject, "u_Ls[1]");
	lightPositionUniform_Sphere[1] = glGetUniformLocation(sphereShaderProgramObject, "u_lightPosition[1]");

	laUniform_Sphere[2] = glGetUniformLocation(sphereShaderProgramObject, "u_La[2]");
	ldUniform_Sphere[2] = glGetUniformLocation(sphereShaderProgramObject, "u_Ld[2]");
	lsUniform_Sphere[2] = glGetUniformLocation(sphereShaderProgramObject, "u_Ls[2]");
	lightPositionUniform_Sphere[2] = glGetUniformLocation(sphereShaderProgramObject, "u_lightPosition[2]");

	kaUniform_Sphere = glGetUniformLocation(sphereShaderProgramObject, "u_Ka");
	kdUniform_Sphere = glGetUniformLocation(sphereShaderProgramObject, "u_Kd");
	ksUniform_Sphere = glGetUniformLocation(sphereShaderProgramObject, "u_Ks");
	materialShininessUniform_Sphere = glGetUniformLocation(sphereShaderProgramObject, "u_materialShininess");

	lightEnabledUniform_Sphere = glGetUniformLocation(sphereShaderProgramObject, "u_lightEnabled");

	// vao
	glGenVertexArrays(1, &vaoSphere);
	glBindVertexArray(vaoSphere);
	{
		// get sphere data
		getSphereVertexData(sphereVertices, sphereNormals, sphereTexCoords, sphereElements);
		sphereNumVertices = getNumberOfSphereVertices();
		sphereNumElements = getNumberOfSphereElements();

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
	lights_Sphere[0].ambient = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	lights_Sphere[0].diffuse = vmath::vec4(1.0f, 0.0f, 0.0f, 1.0f);  // red
	lights_Sphere[0].specular = vmath::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	lights_Sphere[0].position = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	lights_Sphere[1].ambient = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	lights_Sphere[1].diffuse = vmath::vec4(0.0f, 1.0f, 0.0f, 1.0f);  // blue
	lights_Sphere[1].specular = vmath::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	lights_Sphere[1].position = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	lights_Sphere[2].ambient = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	lights_Sphere[2].diffuse = vmath::vec4(0.0f, 0.0f, 1.0f, 1.0f);  // green
	lights_Sphere[2].specular = vmath::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	lights_Sphere[2].position = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	perspectiveProjectionMatrix_Sphere = mat4::identity();

	// warm-up resize
	resizeSphere(FBO_WIDTH, FBO_HEIGHT);

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

	winWidth = width;
	winHeight = height;

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

	aspectRatio = (GLfloat)width / (GLfloat)height;
	perspectiveProjectionMatrix_Cube = vmath::perspective(45.0f, aspectRatio, 0.1f, 100.0f);
}

void resizeSphere(int width, int height)
{
	// variable declarations
	GLfloat aspectRatio;

	// code
	if (height == 0)
		height = 1;	// to prevent a divide by zero when calculating the width/height ratio

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

	aspectRatio = (GLfloat)width / (GLfloat)height;
	perspectiveProjectionMatrix_Sphere = vmath::perspective(45.0f, aspectRatio, 0.1f, 100.0f);
}

void display(void)
{
	// function prototypes
	void displaySphere(GLint, GLint);
	void updateSphere(void);

	// variable declarations
	mat4 translationMatrix = mat4::identity();
	mat4 rotationMatrix_X = mat4::identity();
	mat4 rotationMatrix_Y = mat4::identity();
	mat4 rotationMatrix_Z = mat4::identity();
	mat4 rotationMatrix = mat4::identity();
	mat4 scaleMatrix = mat4::identity();

	mat4 modelViewMatrix = mat4::identity();
	mat4 modelViewProjectionMatrix = mat4::identity();

	// code
	/****************** FBO setup *****************/
	if (fboResult)
	{
		displaySphere(FBO_WIDTH, FBO_HEIGHT);
		updateSphere();
	}

	/****************** Default *****************/
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	resize(winWidth, winHeight);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(cubeShaderProgramObject);
	{
		// transformations
		translationMatrix = vmath::translate(0.0f, 0.0f, -4.0f);
		rotationMatrix_X = vmath::rotate(theta_Cube, 1.0f, 0.0f, 0.0f);
		rotationMatrix_Y = vmath::rotate(theta_Cube, 0.0f, 1.0f, 0.0f);
		rotationMatrix_Z = vmath::rotate(theta_Cube, 0.0f, 0.0f, 1.0f);
		rotationMatrix = rotationMatrix_X * rotationMatrix_Y * rotationMatrix_Z;
		scaleMatrix = vmath::scale(0.75f, 0.75f, 0.75f);

		modelViewMatrix = translationMatrix * rotationMatrix * scaleMatrix;
		modelViewProjectionMatrix = perspectiveProjectionMatrix_Cube * modelViewMatrix;

		// setting transformation uniforms
		glUniformMatrix4fv(mvpMatrixUniform_Cube, 1, GL_FALSE, modelViewProjectionMatrix);
		
		// setting texture params
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(textureSamplerUniform, 0);
		glBindTexture(GL_TEXTURE_2D, fboTexture);

		// drawing
		glBindVertexArray(vaoCube);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
			glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
			glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
			glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
			glDrawArrays(GL_TRIANGLE_FAN, 20, 4);
		glBindVertexArray(0);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glUseProgram(0);

	SwapBuffers(ghdc);
}

void update(void)
{
	// code
	theta_Cube = theta_Cube + 0.5f;
	if (theta_Cube >= 360.0f)
		theta_Cube = theta_Cube - 360.0f;
}

void displaySphere(GLint textureWidth, GLint textureHeight)
{
	// variable declarations
	mat4 translationMatrix = mat4::identity();
	mat4 modelMatrix = mat4::identity();
	mat4 viewMatrix = mat4::identity();

	// code
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	resizeSphere(textureWidth, textureHeight);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// transformations
	translationMatrix = vmath::translate(0.0f, 0.0f, -2.0f);
	modelMatrix = translationMatrix;

	// draw
	glUseProgram(sphereShaderProgramObject);
	{
		glUniformMatrix4fv(modelMatrixUniform_Sphere, 1, GL_FALSE, modelMatrix);
		glUniformMatrix4fv(viewMatrixUniform_Sphere, 1, GL_FALSE, viewMatrix);
		glUniformMatrix4fv(projectionMatrixUniform_Sphere, 1, GL_FALSE, perspectiveProjectionMatrix_Sphere);

		if (bLights)
		{
			lights_Sphere[0].position = vmath::vec4(0.0f, 10.0f * sinf(DEG_TO_RAD(theta_Sphere)), 10.0f * cosf(DEG_TO_RAD(theta_Sphere)), 1.0f);
			lights_Sphere[1].position = vmath::vec4(10.0f * cosf(DEG_TO_RAD(theta_Sphere)), 0.0f, 10.0f * sinf(DEG_TO_RAD(theta_Sphere)), 1.0f);
			lights_Sphere[2].position = vmath::vec4(10.0f * cosf(DEG_TO_RAD(theta_Sphere)), 10.0f * sinf(DEG_TO_RAD(theta_Sphere)), 0.0f, 1.0f);

			for (int i = 0; i < 3; i++)
			{
				glUniform3fv(laUniform_Sphere[i], 1, lights_Sphere[i].ambient);
				glUniform3fv(ldUniform_Sphere[i], 1, lights_Sphere[i].diffuse);
				glUniform3fv(lsUniform_Sphere[i], 1, lights_Sphere[i].specular);
				glUniform4fv(lightPositionUniform_Sphere[i], 1, lights_Sphere[i].position);
			}

			glUniform3fv(kaUniform_Sphere, 1, materialAmbient_Sphere);
			glUniform3fv(kdUniform_Sphere, 1, materialDiffuse_Sphere);
			glUniform3fv(ksUniform_Sphere, 1, materialSpecular_Sphere);
			glUniform1f(materialShininessUniform_Sphere, materialShininess_Sphere);

			glUniform1i(lightEnabledUniform_Sphere, 1);
		}
		else
		{
			glUniform1i(lightEnabledUniform_Sphere, 0);
		}

		glBindVertexArray(vaoSphere);
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
			glDrawElements(GL_TRIANGLES, sphereNumElements, GL_UNSIGNED_SHORT, 0);
		}
		glBindVertexArray(0);
	}
	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void updateSphere(void)
{
	// code
	// deliberately kept separate from update() for isolation
	theta_Sphere = theta_Sphere + 1.5f;
	if (theta_Sphere >= 360.0f)
		theta_Sphere = theta_Sphere - 360.0f;
}

void uninitialize(void)
{
	// function prototypes
	void ToggleFullScreen(void);
	void uninitializeSphere(void);

	// code
	if (gbFullScreen)
	{
		ToggleFullScreen();
	}

	uninitializeSphere();

	if (fboTexture)
	{
		glDeleteTextures(1, &fboTexture);
		fboTexture = 0;
	}

	if (rbo)
	{
		glDeleteRenderbuffers(1, &rbo);
		rbo = 0;
	}

	if (fbo)
	{
		glDeleteBuffers(1, &fbo);
		fbo = 0;
	}

	if (vboTexCoords)
	{
		glDeleteBuffers(1, &vboTexCoords);
		vboTexCoords = 0;
	}

	if (vboVertices)
	{
		glDeleteBuffers(1, &vboVertices);
		vboVertices = 0;
	}

	if (vaoCube)
	{
		glDeleteVertexArrays(1, &vaoCube);
		vaoCube = 0;
	}

	if (cubeShaderProgramObject)
	{
		GLsizei numAttachedShaders = 0;
		GLuint *shaderObjects = NULL;

		glUseProgram(cubeShaderProgramObject);
		
		glGetProgramiv(cubeShaderProgramObject, GL_ATTACHED_SHADERS, &numAttachedShaders);
		shaderObjects = (GLuint *)malloc(numAttachedShaders * sizeof(GLuint));
		glGetAttachedShaders(cubeShaderProgramObject, numAttachedShaders, &numAttachedShaders, shaderObjects);

		for (GLsizei i = 0; i < numAttachedShaders; i++)
		{
			glDetachShader(cubeShaderProgramObject, shaderObjects[i]);
			glDeleteShader(shaderObjects[i]);
			shaderObjects[i] = 0;
		}
		free(shaderObjects);
		shaderObjects = NULL;
		fprintf(gpLog, "detached and deleted %d shader objects\n", numAttachedShaders);

		glUseProgram(0);
		glDeleteProgram(cubeShaderProgramObject);
		cubeShaderProgramObject = 0;
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

void uninitializeSphere(void)
{
	// code
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

	if (vaoSphere)
	{
		glDeleteVertexArrays(1, &vaoSphere);
		vaoSphere = 0;
	}

	if (sphereShaderProgramObject)
	{
		GLsizei numAttachedShaders = 0;
		GLuint *shaderObjects = NULL;

		glUseProgram(sphereShaderProgramObject);

		glGetProgramiv(sphereShaderProgramObject, GL_ATTACHED_SHADERS, &numAttachedShaders);
		shaderObjects = (GLuint *)malloc(numAttachedShaders * sizeof(GLuint));
		glGetAttachedShaders(sphereShaderProgramObject, numAttachedShaders, &numAttachedShaders, shaderObjects);

		for (GLsizei i = 0; i < numAttachedShaders; i++)
		{
			glDetachShader(sphereShaderProgramObject, shaderObjects[i]);
			glDeleteShader(shaderObjects[i]);
			shaderObjects[i] = 0;
		}
		free(shaderObjects);
		shaderObjects = NULL;
		fprintf(gpLog, "sphere: detached and deleted %d shader objects\n", numAttachedShaders);

		glUseProgram(0);
		glDeleteProgram(sphereShaderProgramObject);
		sphereShaderProgramObject = 0;
		fprintf(gpLog, "sphere: deleted shader program object\n");
	}
}

bool createFBO(GLint textureWidth, GLint textureHeight)
{
	// variable declarations
	int maxRenderbufferSize;

	// code
	// check available renderbuffer size 
	glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &maxRenderbufferSize);
	if (maxRenderbufferSize < textureWidth || maxRenderbufferSize < textureHeight)
	{
		fprintf(
			gpLog,
			"*** textureWidth(%d) or textureHeight(%d) exceeded maximum Renderbuffer size(%d) ***\n",
			textureWidth,
			textureHeight,
			maxRenderbufferSize
		);
		return false;
	}

	// create a framebuffer object
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	{
		// create renderbuffer object(s)
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);

		// specify the format and storage for the renderbuffer
		// using COPONENT16 for GLES applications, has nothing to do with depth buffer, but with color depth
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, textureWidth, textureHeight);

		// create an empty texture for the target scene
		glGenTextures(1, &fboTexture);
		glBindTexture(GL_TEXTURE_2D, fboTexture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		// again, a GLES-safe call (5 + 6 + 5 = 16 bit of color depth as promised when allocating storage)
		// Aftab Munshi and Ginsberg, both suggest use 565 instead of any other combinations for better quality textures
		// why more bits to green? human eyes are more susceptible to the green spectrum
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
			
		// hand above texture over to the FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);

		// hand above renderbuffer over to the FBO
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

		// check whether FBO creation is successful or not
		GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (result != GL_FRAMEBUFFER_COMPLETE)
		{
			fprintf(gpLog, "*** framebuffer is not complete ***\n");
			return false;
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);  // renderbuffer and texture objects associated with an FBO are unbound automatically

	return true;
}
