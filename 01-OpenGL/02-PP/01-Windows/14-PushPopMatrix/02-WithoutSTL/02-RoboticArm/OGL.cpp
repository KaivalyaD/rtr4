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

#define max(x, y) ((x > y) ? (x) : (y))
#define min(x, y) ((x < y) ? (x) : (y))

#define MAX_MV_MATRIX_STACK_SIZE 32

typedef struct Stack
{
	vmath::mat4 matMV[MAX_MV_MATRIX_STACK_SIZE];
	int top = -1;
} Stack;

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
GLuint vaoSphere;
GLuint vboSphereVertices;
GLuint vboSphereNormals;
GLuint vboSphereElements;

GLuint vaoCuboids;
GLuint vboCuboidPositions;
GLuint vboCuboidNormals;

// transformation matrices and uniforms
GLint modelMatrixUniform;
GLint viewMatrixUniform;
GLint projectionMatrixUniform;
mat4 perspectiveProjectionMatrix;

// lighting
GLuint LdUniform;
GLuint lightPositionUniform;

// controls and inverse kinematics
float shoulder_yaw, shoulder_pitch;
float elbow_pitch, elbow_roll;
float wrist_pitch, wrist_roll;
float finger_a, finger_b;
Stack stackMatModelView;

// object data RAM
float sphereVertices[1146];
float sphereNormals[1146];
float sphereTexCoords[764];
unsigned short sphereElements[2280];
unsigned int numVertices, numElements;

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

		case '1':
			shoulder_yaw = (shoulder_yaw + 3.0f);
			break;
		case '!':
			shoulder_yaw = (shoulder_yaw - 3.0f);
			break;

		case '2':
			shoulder_pitch = (shoulder_pitch + 3.0f);
			break;
		case '@':
			shoulder_pitch = (shoulder_pitch - 3.0f);
			break;

		case '3':
			elbow_pitch = (elbow_pitch + 3.0f);
			break;
		case '#':
			elbow_pitch = (elbow_pitch - 3.0f);
			break;

		case '4':
			elbow_roll = (elbow_roll + 3.0f);
			break;
		case '$':
			elbow_roll = (elbow_roll - 3.0f);
			break;

		case '5':
			wrist_pitch = (wrist_pitch + 3.0f);
			break;
		case '%':
			wrist_pitch = (wrist_pitch - 3.0f);
			break;

		case '6':
			wrist_roll = (wrist_roll + 3.0f);
			break;
		case '^':
			wrist_roll = (wrist_roll - 3.0f);
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
	BOOL LoadGLTexture(GLuint *, TCHAR[]);
	GLuint generateCuboids(GLuint *, GLuint *, const size_t, const vmath::vec3 *);

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
		"in vec4 a_normal;\n" \
		"\n" \
		"uniform mat4 u_modelMatrix;\n" \
		"uniform mat4 u_viewMatrix;\n" \
		"uniform mat4 u_projectionMatrix;\n" \
		"uniform vec4 u_lightPosition;\n" \
		"\n" \
		"out vec3 a_normal_out;\n" \
		"out vec3 lightDirection;\n" \
		"\n"
		"void main(void)\n" \
		"{\n" \
			"mat4 modelViewMatrix = u_viewMatrix * u_modelMatrix;\n" \
			"gl_Position = u_projectionMatrix * modelViewMatrix * a_position;\n" \
			"\n" \
			"vec3 eyeCoordinate = vec3(modelViewMatrix * a_position);\n" \
			"lightDirection = vec3(u_lightPosition) - eyeCoordinate;\n" \
			"a_normal_out = mat3(transpose(inverse(modelViewMatrix))) * a_normal.xyz;\n" \
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
		"in vec3 a_normal_out;\n" \
		"in vec3 lightDirection;\n" \
		"\n" \
		"uniform vec3 u_Ld;\n" \
		"\n" \
		"out vec4 FragColor;\n" \
		"\n" \
		"void main(void)\n" \
		"{\n" \
			"vec3 normalizedLightDirection = normalize(lightDirection);\n" \
			"vec3 normal = normalize(a_normal_out);\n"
			"vec3 diffuseColor = u_Ld * max(dot(normalizedLightDirection, normal), 0.0);\n" \
			"FragColor = vec4(diffuseColor, 1.0);\n" \
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
	lightPositionUniform = glGetUniformLocation(shaderProgramObject, "u_lightPosition");
	LdUniform = glGetUniformLocation(shaderProgramObject, "u_Ld");
	
	glGenVertexArrays(1, &vaoSphere);
	glBindVertexArray(vaoSphere);
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

	// generate cuboids
	const vmath::vec3 cuboidDimensions[] = {
		//           w  ,  h  ,  d      //
		vmath::vec3(0.1f, 1.5f, 0.1f),  // forearm
		vmath::vec3(0.1f, 1.0f, 0.1f),  // arm
		vmath::vec3(0.2f, 0.2f, 0.1f),  // wrist
		vmath::vec3(0.05f, 0.1f, 0.1f)  // fingers
	};
	vaoCuboids = generateCuboids(&vboCuboidPositions, &vboCuboidNormals, sizeof(cuboidDimensions) / sizeof(cuboidDimensions[0]), cuboidDimensions);

	// set clear color
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

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
	perspectiveProjectionMatrix = vmath::perspective(45.0f, aspectRatio, 0.1f, 100.0f);
}

void display(void)
{
	// function prototypes
	vmath::mat4 top(Stack &);
	void push(vmath::mat4 &, Stack &);
	void pop(Stack &);

	// variable declarations
	mat4 modelMatrix;
	const mat4 viewMatrix = vmath::translate(0.0f, 0.0f, -4.0f);

	// code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// draw
	glUseProgram(shaderProgramObject);
	{
		glUniform3f(LdUniform, 1.0f, 1.0f, 1.0f);
		glUniform4f(lightPositionUniform, 0.0f, 1.0f, 0.0f, 1.0f);
		glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
		glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

		// push forearm transformations
		modelMatrix = vmath::translate(0.0f, 1.5f, 0.0f) *
			vmath::rotate(shoulder_yaw, 0.0f, 1.0f, 0.0f) *
			vmath::rotate(shoulder_pitch, 1.0f, 0.0f, 0.0f);

		push(modelMatrix, stackMatModelView);
		{
			glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, top(stackMatModelView) * vmath::scale(0.2f));
			glBindVertexArray(vaoSphere);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);
			glBindVertexArray(0);

			glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, top(stackMatModelView));
			glBindVertexArray(vaoCuboids);
				glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
				glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
				glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
				glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
				glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
				glDrawArrays(GL_TRIANGLE_FAN, 20, 4);
			glBindVertexArray(0);

			// push arm transformation
			modelMatrix = top(stackMatModelView) *
				vmath::translate(0.0f, -1.5f, 0.0f) *
				vmath::rotate(elbow_pitch, 0.0f, 1.0f, 0.0f) *
				vmath::rotate(elbow_roll, 1.0f, 0.0f, 0.0f);

			push(modelMatrix, stackMatModelView);
			{
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, top(stackMatModelView) * vmath::scale(0.2f));
				glBindVertexArray(vaoSphere);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
					glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, top(stackMatModelView));
				glBindVertexArray(vaoCuboids);
					glDrawArrays(GL_TRIANGLE_FAN, 24, 4);
					glDrawArrays(GL_TRIANGLE_FAN, 28, 4);
					glDrawArrays(GL_TRIANGLE_FAN, 32, 4);
					glDrawArrays(GL_TRIANGLE_FAN, 36, 4);
					glDrawArrays(GL_TRIANGLE_FAN, 40, 4);
					glDrawArrays(GL_TRIANGLE_FAN, 44, 4);
				glBindVertexArray(0);

				// push wrist transformation
				modelMatrix = top(stackMatModelView) *
					vmath::translate(0.0f, -1.0f, 0.0f) *
					vmath::rotate(wrist_pitch, 0.0f, 1.0f, 0.0f) *
					vmath::rotate(wrist_roll, 1.0f, 0.0f, 0.0f);

				push(modelMatrix, stackMatModelView);
				{
					glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, top(stackMatModelView) * vmath::scale(0.2f));
					glBindVertexArray(vaoSphere);
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
						glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

					glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, top(stackMatModelView));
					glBindVertexArray(vaoCuboids);
						glDrawArrays(GL_TRIANGLE_FAN, 48, 4);
						glDrawArrays(GL_TRIANGLE_FAN, 52, 4);
						glDrawArrays(GL_TRIANGLE_FAN, 56, 4);
						glDrawArrays(GL_TRIANGLE_FAN, 60, 4);
						glDrawArrays(GL_TRIANGLE_FAN, 64, 4);
						glDrawArrays(GL_TRIANGLE_FAN, 68, 4);
					glBindVertexArray(0);

					// push finger 'a' transformation
					modelMatrix = top(stackMatModelView) *
						vmath::translate(-0.08f + finger_a, -0.2f, 0.0f);

					push(modelMatrix, stackMatModelView);
					{
						glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, top(stackMatModelView));
						glBindVertexArray(vaoCuboids);
							glDrawArrays(GL_TRIANGLE_FAN, 72, 4);
							glDrawArrays(GL_TRIANGLE_FAN, 76, 4);
							glDrawArrays(GL_TRIANGLE_FAN, 80, 4);
							glDrawArrays(GL_TRIANGLE_FAN, 84, 4);
							glDrawArrays(GL_TRIANGLE_FAN, 88, 4);
							glDrawArrays(GL_TRIANGLE_FAN, 92, 4);
						glBindVertexArray(0);
					}
					pop(stackMatModelView);  // pop finger 'a' transformation

					// push finger 'b' transformation
					modelMatrix = top(stackMatModelView) *
						vmath::translate(0.08f - finger_b, -0.2f, 0.0f);

					push(modelMatrix, stackMatModelView);
					{
						glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, top(stackMatModelView));
						glBindVertexArray(vaoCuboids);
							glDrawArrays(GL_TRIANGLE_FAN, 72, 4);
							glDrawArrays(GL_TRIANGLE_FAN, 76, 4);
							glDrawArrays(GL_TRIANGLE_FAN, 80, 4);
							glDrawArrays(GL_TRIANGLE_FAN, 84, 4);
							glDrawArrays(GL_TRIANGLE_FAN, 88, 4);
							glDrawArrays(GL_TRIANGLE_FAN, 92, 4);
						glBindVertexArray(0);
					}
					pop(stackMatModelView);  // pop finger 'b' transformation
				}
				pop(stackMatModelView);  // pop wrist transformation
			}
			pop(stackMatModelView);  // pop arm transformation
		}
		pop(stackMatModelView);  // pop forearm transformation
	}
	glUseProgram(0);

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

	if (vboCuboidNormals)
	{
		glDeleteBuffers(1, &vboCuboidNormals);
		vboCuboidNormals = 0;
	}

	if (vboCuboidPositions)
	{
		glDeleteBuffers(1, &vboCuboidPositions);
		vboCuboidPositions = 0;
	}

	if (vaoCuboids)
	{
		glDeleteVertexArrays(1, &vaoCuboids);
		vaoCuboids = 0;
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

	if (vaoSphere)
	{
		glDeleteVertexArrays(1, &vaoSphere);
		vaoSphere = 0;
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

GLuint generateCuboids(GLuint *pVboPositions, GLuint *pVboNormals, const size_t count, const vmath::vec3 *dimensions)
{
	// variable declarations
	GLuint retVao = 0;

	// code
	glGenVertexArrays(1, &retVao);
	glBindVertexArray(retVao);
	{
		// positions vbo
		glGenBuffers(1, pVboPositions);
		glBindBuffer(GL_ARRAY_BUFFER, *pVboPositions);
		{
			// initialize a dynamic data store first
			glBufferStorage(
				GL_ARRAY_BUFFER,
				count * 24 * 3 * sizeof(float),
				NULL,
				GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT
			);
			glVertexAttribPointer(KVD_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);

			for (int i = 0; i < count; i++)
			{
				float currentWidth = dimensions[i][0];
				float currentHeight = dimensions[i][1];
				float currentDepth = dimensions[i][2];

				float faceVertices[] = {
					// front
					currentWidth / 2.0f,  0.0f,  currentDepth / 2.0f,
				   -currentWidth / 2.0f,  0.0f,  currentDepth / 2.0f,
				   -currentWidth / 2.0f, -currentHeight,  currentDepth / 2.0f,
					currentWidth / 2.0f, -currentHeight,  currentDepth / 2.0f,

					// right
					currentWidth / 2.0f,  0.0f, -currentDepth / 2.0f,
					currentWidth / 2.0f,  0.0f,  currentDepth / 2.0f,
					currentWidth / 2.0f, -currentHeight,  currentDepth / 2.0f,
					currentWidth / 2.0f, -currentHeight, -currentDepth / 2.0f,

					// back
				   -currentWidth / 2.0f,  0.0f, -currentDepth / 2.0f,
					currentWidth / 2.0f,  0.0f, -currentDepth / 2.0f,
					currentWidth / 2.0f, -currentHeight, -currentDepth / 2.0f,
				   -currentWidth / 2.0f, -currentHeight, -currentDepth / 2.0f,

				   // left
				   -currentWidth / 2.0f,  0.0f,  currentDepth / 2.0f,
				   -currentWidth / 2.0f,  0.0f, -currentDepth / 2.0f,
				   -currentWidth / 2.0f, -currentHeight, -currentDepth / 2.0f,
				   -currentWidth / 2.0f, -currentHeight,  currentDepth / 2.0f,

				    // top
				    currentWidth / 2.0f,  0.0f,  currentDepth / 2.0f,
				    currentWidth / 2.0f,  0.0f, -currentDepth / 2.0f,
				   -currentWidth / 2.0f,  0.0f, -currentDepth / 2.0f,
				   -currentWidth / 2.0f,  0.0f,  currentDepth / 2.0f,

				    // bottom
				    currentWidth / 2.0f, -currentHeight,  currentDepth / 2.0f,
				   -currentWidth / 2.0f, -currentHeight,  currentDepth / 2.0f,
				   -currentWidth / 2.0f, -currentHeight, -currentDepth / 2.0f,
				    currentWidth / 2.0f, -currentHeight, -currentDepth / 2.0f
				};

				glBufferSubData(GL_ARRAY_BUFFER, i * 24 * 3 * sizeof(float), 24 * 3 * sizeof(float), faceVertices);
			}

			// copy data to vertex shader now
			glEnableVertexAttribArray(KVD_ATTRIBUTE_POSITION);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// normals vbo
		glGenBuffers(1, pVboNormals);
		glBindBuffer(GL_ARRAY_BUFFER, *pVboNormals);
		{
			glBufferStorage(
				GL_ARRAY_BUFFER,
				count * 24 * 3 * sizeof(float),
				NULL,
				GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT
			);
			glVertexAttribPointer(KVD_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);

			for (int i = 0; i < count; i++)
			{
				float faceNormals[] = {
					// front
					0.0f,  0.0f,  1.0f,
					0.0f,  0.0f,  1.0f,
					0.0f,  0.0f,  1.0f,
					0.0f,  0.0f,  1.0f,

					// right
					1.0f,  0.0f,  0.0f,
					1.0f,  0.0f,  0.0f,
					1.0f,  0.0f,  0.0f,
					1.0f,  0.0f,  0.0f,

					// back
					0.0f,  0.0f, -1.0f,
					0.0f,  0.0f, -1.0f,
					0.0f,  0.0f, -1.0f,
					0.0f,  0.0f, -1.0f,

					// left
				   -1.0f,  0.0f,  0.0f,
				   -1.0f,  0.0f,  0.0f,
				   -1.0f,  0.0f,  0.0f,
				   -1.0f,  0.0f,  0.0f,

				    // top
				    0.0f,  1.0f,  0.0f,
				    0.0f,  1.0f,  0.0f,
				    0.0f,  1.0f,  0.0f,
				    0.0f,  1.0f,  0.0f,

				    // bottom
				    0.0f, -1.0f,  0.0f,
				    0.0f, -1.0f,  0.0f,
				    0.0f, -1.0f,  0.0f,
				    0.0f, -1.0f,  0.0f
				};

				glBufferSubData(GL_ARRAY_BUFFER, i * 24 * 3 * sizeof(float), 24 * 3 * sizeof(float), faceNormals);
			}

			glEnableVertexAttribArray(KVD_ATTRIBUTE_NORMAL);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	glBindVertexArray(0);

	return retVao;
}

/* implementing stack functions */
vmath::mat4 top(Stack &stack)
{
	if (stack.top > -1)
		return stack.matMV[stack.top];

	return vmath::mat4(
		vmath::vec4(0.0f, 0.0f, 0.0f, 0.0f),
		vmath::vec4(0.0f, 0.0f, 0.0f, 0.0f),
		vmath::vec4(0.0f, 0.0f, 0.0f, 0.0f),
		vmath::vec4(0.0f, 0.0f, 0.0f, 0.0f)
	);
}

void push(vmath::mat4 &mat, Stack &stack)
{
	stack.top += 1;

	if (stack.top > MAX_MV_MATRIX_STACK_SIZE)
	{
		stack.top -= 1;
		return;
	}

	stack.matMV[stack.top] = mat;
}

void pop(Stack &stack)
{
	if (stack.top > -1)
		stack.top -= 1;
}
