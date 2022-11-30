/* standard headers */
#include <stdio.h>  // for standard I/O
#include <stdlib.h> // for exit()
#include <memory.h> // for memset()

/* vector math */
#include "vmath.h"

/* X11 headers */
#include <X11/Xlib.h>
#include <X11/Xutil.h>  // for XVisualInfo
#include <X11/XKBlib.h> // for the keyboard

/* GLEW headers */
#include <GL/glew.h>

/* OpenGL headers */
#include <GL/gl.h>  // the OpenGL API
#include <GL/glx.h> // API for GL/XServer bridging

/* sphere lib */
#include "Sphere.h"

/* macros */
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#define FBO_WIDTH 512
#define FBO_HEIGHT 512

/* typedefs */
typedef GLXContext (*glXCreateContextAttribsARBProc)(Display *, GLXFBConfig, GLXContext, Bool, const int*);

/* global variables */
/* windowing */
Display *display = NULL;
XVisualInfo *visualInfo = NULL;
Colormap colormap;
Window window;
Bool fullScreen = False;
Bool activeWindow = False;
FILE *logFile = NULL;

/* glX variables */
glXCreateContextAttribsARBProc glXCreateContextAttribsARB = NULL;
GLXFBConfig glxFBConfig;
GLXContext glxContext = NULL;

int winWidth;
int winHeight;

/* related to the OpenGL programmable pipeline */
GLuint cubeShaderProgramObject;

/* scene objects */
GLuint vaoCube;
GLuint vboVertices;
GLuint vboTexCoords;

// transformation matrices and uniforms
vmath::mat4 perspectiveProjectionMatrix_Cube;

// animation
GLfloat theta_Cube = 0.0f;

// FBO-related variables
GLuint fbo;
GLuint rbo;
GLuint fboTexture;
bool fboResult = false;

/******* texture-scene variables begin ********/
GLuint sphereShaderProgramObject;

// object data RAM
GLuint vaoSphere;
GLuint vboSphereVertices;
GLuint vboSphereNormals;
GLuint vboSphereElements;
unsigned int numVertices, numElements;

/* uniform buffer objects */
GLuint uboTransformations;
GLuint uboMaterial;
GLuint uboLight;
GLuint uboViewer;

/* light switch */
Bool lightEnabled = False;

// light switch
Bool bLights = False;

// animation
GLfloat theta_Sphere = 0.0f;
/******* texture-scene variables end ********/

/* entry-point function */
int main(int argc, char *argv[])
{
    // function declarations
    void toggleFullscreen(void);
    int initialize(void);
    void resize(int, int);
    void update(void);
    void render(void);
    void uninitialize(void);

    // local variables
    int defaultScreen, defaultDepth;
    GLXFBConfig *glxFBConfigs = NULL;
    GLXFBConfig bestGLXFBConfig;
    XVisualInfo *tempXVisualInfo = NULL;
    int cntFBConfigs;
    XSetWindowAttributes windowAttributes;
    int styleMask = 0;
    Atom wm_delete_window_atom;
    XEvent event;
    KeySym keysym;
    int screenWidth, screenHeight;
    char keys[26];

    int framebufferAttributes[] = {
        GLX_X_RENDERABLE, True,              // hardware accelerated renderable
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,   // draw to a window
        GLX_RENDER_TYPE, GLX_RGBA_BIT,       // the render type is an RGBA frame
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_STENCIL_SIZE, 8,
        GLX_DEPTH_SIZE, 24,
        GLX_DOUBLEBUFFER, True,
        None
    };
    Bool done = False;
    int ret;
    int winWidth, winHeight;

    // code
    logFile = fopen("log.txt", "w");
    if(!logFile)
    {
        fprintf(stderr, "fopen: failed to open log.txt\n");
        uninitialize();
        exit(EXIT_FAILURE);
    }
    fprintf(logFile, "successfully created log file\n");

    display = XOpenDisplay(NULL);
    if(!display)
    {
        fprintf(stderr, "error: XOpenDisplay() failed\n");
        uninitialize();
        exit(EXIT_FAILURE);
    }

    defaultScreen = XDefaultScreen(display);
    defaultDepth = XDefaultDepth(display, defaultScreen);

    // getting all conforming FB configurations
    glxFBConfigs = glXChooseFBConfig(display, defaultScreen, framebufferAttributes, &cntFBConfigs);
    if(!glxFBConfigs)
    {
        fprintf(logFile, "error: glXChooseFBConfig() failed\n");
        uninitialize();
        exit(EXIT_FAILURE);
    }
    fprintf(logFile, "found %d conforming framebuffer configurations:\n", cntFBConfigs);

    // choosing the best FB configuration but getting both -the best and the worst
    int bestFramebufferConfigIdx = -1;
    int worstFramebufferConfigIdx = -1;
    int bestCountOfSamples = -1;
    int worstCountOfSamples = 999;

    for(int i = 0; i < cntFBConfigs; i++)
    {
        tempXVisualInfo = glXGetVisualFromFBConfig(display, glxFBConfigs[i]);
        if(!tempXVisualInfo)
            continue;       // if an FB config has no XVisualInfo, it cannot be the best

        int samples, sampleBuffers;
        glXGetFBConfigAttrib(display, glxFBConfigs[i], GLX_SAMPLE_BUFFERS, &sampleBuffers);
        glXGetFBConfigAttrib(display, glxFBConfigs[i], GLX_SAMPLES, &samples);

        fprintf(logFile, "[%d] XVisualInfo (visualid 0x%lx) has: %d sample buffers and %d samples\n", (i + 1), tempXVisualInfo->visualid, sampleBuffers, samples);

        if((bestFramebufferConfigIdx < 0) || (sampleBuffers && (samples > bestCountOfSamples)))
        {
            bestFramebufferConfigIdx = i;
            bestCountOfSamples = samples;
        }

        if((worstFramebufferConfigIdx >= 999) || !sampleBuffers || (samples < worstCountOfSamples))
        {
            // this may be not the worst of the worst, but only as good as unusable
            worstFramebufferConfigIdx = i;
            worstCountOfSamples = samples;
        }

        XFree(tempXVisualInfo);
        tempXVisualInfo = NULL;
    }
    bestGLXFBConfig = glxFBConfigs[bestFramebufferConfigIdx];
    glxFBConfig = bestGLXFBConfig;

    XFree(glxFBConfigs);
    glxFBConfigs = NULL;

    // obtaining visual info from the best FB configuration
    visualInfo = glXGetVisualFromFBConfig(display, bestGLXFBConfig);
    fprintf(logFile, "visualid of the best XVisualInfo found: 0x%lx\n", visualInfo->visualid);

    memset(&windowAttributes, 0, sizeof(XSetWindowAttributes));
    windowAttributes.border_pixel = 0;
    windowAttributes.background_pixel = XBlackPixel(display, defaultScreen);
    windowAttributes.background_pixmap = 0;
    windowAttributes.colormap = XCreateColormap(display, RootWindow(display, visualInfo->screen), visualInfo->visual, AllocNone);
    windowAttributes.event_mask |= ExposureMask | KeyPressMask | StructureNotifyMask | FocusChangeMask;

    colormap = windowAttributes.colormap;

    styleMask |= CWBorderPixel | CWBackPixel | CWColormap | CWEventMask;

    window = XCreateWindow(
        display,
        RootWindow(display, visualInfo->screen),
        0,
        0,
        WIN_WIDTH,
        WIN_HEIGHT,
        0,
        visualInfo->depth,
        InputOutput,
        visualInfo->visual,
        styleMask,
        &windowAttributes
    );
    if(!window)
    {
        fprintf(stderr, "error: XCreateWindow() failed\n");
        uninitialize();
        exit(EXIT_FAILURE);
    }

    XStoreName(display, window, "Kaivalya Vishwakumar Deshpande: OpenGL");

    wm_delete_window_atom = XInternAtom(display, "WM_DELETE_WINDOW", True); // True: create the atom in all cases: whether it exists before or not
    XSetWMProtocols(display, window, &wm_delete_window_atom, 1);

    XMapWindow(display, window);

    // centering the window
    screenWidth = XWidthOfScreen(XScreenOfDisplay(display, defaultScreen));
    screenHeight = XHeightOfScreen(XScreenOfDisplay(display, defaultScreen));
    XMoveWindow(display, window, ((screenWidth - WIN_WIDTH) / 2), ((screenHeight - WIN_HEIGHT) / 2));

    // initialize OpenGL
    ret = initialize();
    if(ret == -1)
    {
        fprintf(logFile, "error: glXGetProcAddressARB() failed\n");
        uninitialize();
        exit(EXIT_FAILURE);
    }
    fprintf(logFile, "OpenGL initialized successfully\n");

    // game loop
    while(!done)
    {
        while(XPending(display))
        {
            XNextEvent(display, &event);
            switch(event.type)
            {
            case MapNotify:
                break;

            case KeyPress:
                keysym = XkbKeycodeToKeysym(display, event.xkey.keycode, 0, 0);
                switch(keysym)
                {
                case XK_Escape:
                    done = True;
                    break;

                default:
                    break;
                }

                XLookupString(&event.xkey, keys, sizeof(keys), NULL, NULL);
                switch(*keys)
                {
                case 'F':
                case 'f':
                    toggleFullscreen();
                    break;

                case 'L':
                case 'l':
                    lightEnabled = !lightEnabled;
                    break;

                default:
                    break;
                }
                break;

            case FocusIn:   /* WM_SETFOCUS in Win32 */
                activeWindow = True;
                break;

            case FocusOut:  /* WM_KILLFOCUS in Win32 */
                activeWindow = False;
                break;

            case ConfigureNotify:
                winWidth = event.xconfigure.width;
                winHeight = event.xconfigure.height;
                resize(winWidth, winHeight);
                break;

            case 33:
                done = True;
                break;

            default:
                break;
            }
        }

        if(activeWindow)
        {
            // update the scene
            update();

            // render the scene
            render();
        }
    }
    uninitialize();

    return 0;
}

void toggleFullscreen(void)
{
    // local variables
    Atom wm_current_state_atom;
    Atom wm_fullscreen_state_atom;
    XEvent event;

    // code
    if(fullScreen)
        fullScreen = False;
    else
        fullScreen = True;

    wm_current_state_atom = XInternAtom(display, "_NET_WM_STATE", False);
    wm_fullscreen_state_atom = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);

    memset(&event, 0, sizeof(XEvent));
    event.type = ClientMessage;
    event.xclient.window = window;
    event.xclient.message_type = wm_current_state_atom;
    event.xclient.format = 32;
    event.xclient.data.l[0] = fullScreen ? 1 : 0;
    event.xclient.data.l[1] = wm_fullscreen_state_atom;

    XSendEvent(display, RootWindow(display, visualInfo->screen), False, SubstructureNotifyMask, &event);
}

int initialize(void)
{
    // function prototypes
    void logGLInfo(void);
    void uninitialize(void);
    void resize(int, int);
    int initializeSphere(int, int);
    bool createFBO(int, int);

    // variable declarations
    GLint contextAttributes[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 4,   // for an OpenGL 4.6 context
        GLX_CONTEXT_MINOR_VERSION_ARB, 6,
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB, // core-profile OpenGL
        None
    };

    // code
    glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((GLubyte *)"glXCreateContextAttribsARB");
    if(!glXCreateContextAttribsARB)
        return -1;

    // passing the 2nd last argument as True means enabling hardware support for rendering
    glxContext = glXCreateContextAttribsARB(display, glxFBConfig, NULL, True, contextAttributes);
    if(!glxContext)
    {
        GLint contextAttributesFallback[] = {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 1,   // this does not mean you get OpenGL 1.0 strictly, but the highest supported version
            GLX_CONTEXT_MINOR_VERSION_ARB, 0,
            None
        };

        glxContext = glXCreateContextAttribsARB(display, glxFBConfig, NULL, True, contextAttributesFallback);
        fprintf(logFile, "cannot support OpenGL version 4.6; falling back to the default\n");
    }
    else
        fprintf(logFile, "OpenGL 4.6 is fully supported\n");

    // checking whether hardware/direct rendering is supported or not
    if(glXIsDirect(display, glxContext))
        fprintf(logFile, "hardware rendering is supported\n");
    else
        fprintf(logFile, "hardware rendering is not supported\n");

    glXMakeCurrent(display, window, glxContext);

    // glew initialization
	if (glewInit() != GLEW_OK)
		return -5;

	// logging OpenGL info
	logGLInfo();

	// vertex shader
	int status = 0;
	int infoLogLength = 0;
	char *infoLog = NULL;

    const GLchar *vertexShaderSourceCode =
		"#version 460 core\n" \
		"\n" \
		"layout(location = 0) in vec4 a_position;\n" \
		"layout(location = 1) in vec2 a_texCoord;\n" \
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
		fprintf(logFile, "*** vertex shader compilation errors ***\n");

		glGetShaderiv(vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			infoLog = (char *)malloc(infoLogLength * sizeof(char));
			if (infoLog)
			{
				GLsizei written = 0;

				glGetShaderInfoLog(vertexShaderObject, infoLogLength * sizeof(char), &written, infoLog);
				fprintf(logFile, "vertex shader compilation log (%d bytes):\n%s\n", written, infoLog);
				free(infoLog);
				infoLog = NULL;
			}
			else
				fprintf(logFile, "\tmalloc: cannot allocate memory to hold the compilation log\n");
		}
		else
			fprintf(logFile, "\tthere is nothing to print\n");

		uninitialize();
        exit(EXIT_FAILURE);
	}
	fprintf(logFile, "vertex shader was compiled without errors\n");

	// fragment shader
	status = 0;
	infoLogLength = 0;
	infoLog = NULL;
    
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
		fprintf(logFile, "*** fragment shader compilation errors ***\n");

		glGetShaderiv(fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			infoLog = (char *)malloc(infoLogLength * sizeof(char));
			if (infoLog)
			{
				GLsizei written = 0;

				glGetShaderInfoLog(fragmentShaderObject, infoLogLength, &written, infoLog);
				fprintf(logFile, "fragment shader compilation log (%d bytes):\n%s\n", written, infoLog);
				free(infoLog);
				infoLog = NULL;
			}
			else
				fprintf(logFile, "\tmalloc: cannot allocate memory to hold the compilation log\n");
		}
		else
			fprintf(logFile, "\tthere is nothing to print\n");

		uninitialize();
		exit(EXIT_FAILURE);
	}
	fprintf(logFile, "fragment shader was compiled without errors\n");

	// shader program object
	status = 0;
	infoLogLength = 0;
	infoLog = NULL;

	cubeShaderProgramObject = glCreateProgram();
	glAttachShader(cubeShaderProgramObject, vertexShaderObject);
	glAttachShader(cubeShaderProgramObject, fragmentShaderObject);
	glLinkProgram(cubeShaderProgramObject);
	glGetProgramiv(cubeShaderProgramObject, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		fprintf(logFile, "*** there were linking errors ***\n");

		glGetProgramiv(cubeShaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			infoLog = (char *)malloc(infoLogLength * sizeof(char));
			if (infoLog)
			{
				GLsizei written = 0;

				glGetProgramInfoLog(cubeShaderProgramObject, infoLogLength * sizeof(char), &written, infoLog);
				fprintf(logFile, "\tlink time info log (%d bytes):\n%s\n", written, infoLog);
				free(infoLog);
				infoLog = NULL;
			}
			else
				fprintf(logFile, "\tmalloc: cannot allocate memory to hold the linking log\n");
		}
		else
			fprintf(logFile, "\tthere is nothing to print\n");

		uninitialize();
		exit(EXIT_FAILURE);
	}
	fprintf(logFile, "shader program was linked without errors\n");

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
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
			glEnableVertexAttribArray(0);
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
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
			glEnableVertexAttribArray(1);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	glBindVertexArray(0);

    // setting up depth
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // setting the clear color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // initialize projection matrix
    perspectiveProjectionMatrix_Cube = vmath::mat4::identity();

    /**************** FBO ****************/
	fboResult = createFBO(FBO_WIDTH, FBO_HEIGHT);
	if (fboResult == true)
	{
		int ret = initializeSphere(FBO_WIDTH, FBO_HEIGHT);
		switch (ret)
		{
		default:
			fprintf(logFile, "successfully initialized the sphere FBO\n");
			break;
		}
	}
    else
    {
        fprintf(logFile, "createFBO(): failed\n");
		uninitialize();
        exit(EXIT_FAILURE);
    }

    // warm-up resize
    resize(WIN_WIDTH, WIN_HEIGHT);

    return 0;
}

int initializeSphere(int width, int height)
{
    // function prototypes
	void resizeSphere(int, int);
    void uninitialize(void);

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
        "#define COUNT_LIGHTS 3\n" \
        "\n" \
        "layout (location = 0) in vec4 a_position;\n" \
        "layout (location = 1) in vec3 a_normal;\n" \
        "\n" \
        "layout (std140, binding = 0) uniform TransformationsBlock\n" \
        "{\n" \
            "mat4 modelMatrix;\n" \
            "mat4 viewMatrix;\n" \
            "mat4 projMatrix;\n" \
        "} ub_transformations;\n" \
        "\n" \
        "layout (std140, binding = 1) uniform LightBlock\n" \
        "{\n"
            "vec3 La[COUNT_LIGHTS];  // ambient light base color\n" \
            "vec3 Ld[COUNT_LIGHTS];  // diffuse light base color\n" \
            "vec3 Ls[COUNT_LIGHTS];  // specular light base color\n" \
            "vec3 lightPosition[COUNT_LIGHTS];  // light position in world space\n" \
        "} ub_lights;\n" \
        "\n" \
        "layout (std140, binding = 3) uniform ViewerBlock\n" \
        "{\n" \
            "vec3 viewerPosition;  // viewer position in world space\n" \
        "} ub_viewer;\n" \
        "\n" \
        "layout (location = 0) uniform int u_lightEnabled;\n" \
        "\n" \
        "out vec3 lightDirection[COUNT_LIGHTS];\n" \
        "out vec3 transformedNormal;\n" \
        "out vec3 viewerDirection;\n" \
        "\n" \
        "void main(void)\n" \
        "{\n" \
            "if (u_lightEnabled == 1)\n" \
            "{\n" \
                "mat4 modelViewMatrix = ub_transformations.viewMatrix * ub_transformations.modelMatrix;\n" \
                "mat3 normalMatrix = mat3(transpose(inverse(modelViewMatrix)));\n" \
                "\n" \
                "vec3 eyeCoordinate = vec3(modelViewMatrix * a_position);\n" \
                "\n" \
                "transformedNormal = vec3(normalMatrix * a_normal);\n" \
                "for (int i = 0; i < COUNT_LIGHTS; i++)\n" \
                    "lightDirection[i] = vec3(ub_lights.lightPosition[i] - eyeCoordinate);\n" \
                "viewerDirection = vec3(ub_viewer.viewerPosition - eyeCoordinate);\n" \
            "}\n" \
            "\n" \
            "gl_Position = ub_transformations.projMatrix * " \
                            "ub_transformations.viewMatrix * " \
                            "ub_transformations.modelMatrix * " \
                            "a_position;\n" \
        "}\n";

    GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderObject, 1, (const GLchar **)&vertexShaderSourceCode, NULL);
	glCompileShader(vertexShaderObject);
	glGetShaderiv(vertexShaderObject, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		fprintf(logFile, "*** sphere vertex shader compilation errors ***\n");

		glGetShaderiv(vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			log = (char *)malloc(infoLogLength * sizeof(char));
			if (log)
			{
				GLsizei written = 0;

				glGetShaderInfoLog(vertexShaderObject, infoLogLength * sizeof(char), &written, log);
				fprintf(logFile, "sphere vertex shader compilation log (%d bytes):\n%s\n", written, log);
				free(log);
				log = NULL;
			}
			else
				fprintf(logFile, "\tmalloc: cannot allocate memory to hold the compilation log\n");
		}
		else
			fprintf(logFile, "\tthere is nothing to print\n");

		uninitialize();
        exit(EXIT_FAILURE);
	}
	fprintf(logFile, "sphere vertex shader was compiled without errors\n");

    const GLchar *fragmentShaderSourceCode =
        "#version 460 core\n" \
        "\n" \
        "#define COUNT_LIGHTS 3\n" \
        "\n" \
        "in vec3 lightDirection[COUNT_LIGHTS];\n" \
        "in vec3 transformedNormal;\n" \
        "in vec3 viewerDirection;\n" \
        "\n" \
        "layout (std140, binding = 1) uniform LightBlock\n" \
        "{\n"
            "vec3 La[COUNT_LIGHTS];  // ambient light base color\n" \
            "vec3 Ld[COUNT_LIGHTS];  // diffuse light base color\n" \
            "vec3 Ls[COUNT_LIGHTS];  // specular light base color\n" \
            "vec3 lightPosition[COUNT_LIGHTS];  // light position in world space\n" \
        "} ub_lights;\n" \
        "\n" \
        "layout (std140, binding = 2) uniform MaterialBlock\n" \
        "{\n" \
            "vec3 Ka;  // material constant for ambient light\n" \
            "vec3 Kd;  // material constant for diffuse light\n" \
            "vec3 Ks;  // material constant for specular light\n" \
            "vec3 materialShininess;  // X-coord is material shininess\n" \
        "} ub_material;\n" \
        "\n" \
        "layout (location = 0) uniform int u_lightEnabled;\n" \
        "\n" \
        "out vec4 FragColor;\n" \
        "\n" \
        "void main(void)\n" \
        "{\n" \
            "if (u_lightEnabled == 1)\n" \
            "{\n" \
                "vec3 normalizedLightDirection;\n" \
                "vec3 reflectedBeamDirection;\n" \
                "vec3 normalizedTransformedNormal = normalize(transformedNormal);\n" \
                "vec3 normalizedViewerDirection = normalize(viewerDirection);\n" \
                "\n" \
                "vec3 ambient_i, diffuse_i, specular_i;\n" \
                "for (int i = 0; i < COUNT_LIGHTS; i++)\n" \
                "{\n" \
                    "normalizedLightDirection = normalize(lightDirection[i]);\n" \
                    "reflectedBeamDirection = reflect(-normalizedLightDirection, normalizedTransformedNormal);\n" \
                    "\n" \
                    "ambient_i = ub_lights.La[i] * ub_material.Ka;\n" \
                    "diffuse_i = ub_lights.Ld[i] * ub_material.Kd * max(dot(normalizedLightDirection, normalizedTransformedNormal), 0.0);\n" \
                    "specular_i = ub_lights.Ls[i] * ub_material.Ks * max(pow(dot(normalizedViewerDirection, reflectedBeamDirection), ub_material.materialShininess.x), 0.0);\n" \
                    "\n" \
                    "FragColor += vec4(ambient_i + diffuse_i + specular_i, 1.0);\n" \
                "}\n" \
            "}\n" \
            "else\n" \
                "FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n" \
        "}\n";

    GLuint fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderObject, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);
	glCompileShader(fragmentShaderObject);
	glGetShaderiv(fragmentShaderObject, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		fprintf(logFile, "*** sphere fragment shader compilation errors ***\n");

		glGetShaderiv(fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			log = (char *)malloc(infoLogLength * sizeof(char));
			if (log)
			{
				GLsizei written = 0;

				glGetShaderInfoLog(fragmentShaderObject, infoLogLength, &written, log);
				fprintf(logFile, "sphere fragment shader compilation log (%d bytes):\n%s\n", written, log);
				free(log);
				log = NULL;
			}
			else
				fprintf(logFile, "\tmalloc: cannot allocate memory to hold the compilation log\n");
		}
		else
			fprintf(logFile, "\tthere is nothing to print\n");

		uninitialize();
		exit(EXIT_FAILURE);
	}
	fprintf(logFile, "sphere fragment shader was compiled without errors\n");

    // shader program
	status = 0;
	infoLogLength = 0;
	log = NULL;

	sphereShaderProgramObject = glCreateProgram();
	glAttachShader(sphereShaderProgramObject, vertexShaderObject);
	glAttachShader(sphereShaderProgramObject, fragmentShaderObject);
	glLinkProgram(sphereShaderProgramObject);

	glGetProgramiv(sphereShaderProgramObject, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		fprintf(logFile, "*** sphere: there were linking errors ***\n");

		glGetProgramiv(sphereShaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			log = (char *)malloc(infoLogLength * sizeof(char));
			if (log)
			{
				GLsizei written = 0;

				glGetProgramInfoLog(sphereShaderProgramObject, infoLogLength * sizeof(char), &written, log);
				fprintf(logFile, "\tsphere: link time info log (%d bytes):\n%s\n", written, log);
				free(log);
				log = NULL;
			}
			else
				fprintf(logFile, "\tsphere: malloc: cannot allocate memory to hold the linking log\n");
		}
		else
			fprintf(logFile, "\tsphere: there is nothing to print\n");

		uninitialize();
	}
	fprintf(logFile, "sphere: shader program was linked without errors\n");

    /* loading vertex data into pipeline */
    glGenVertexArrays(1, &vaoSphere);
    glBindVertexArray(vaoSphere);
    {
        // containers for sphere data
        float sphereVertices[1146];
        float sphereNormals[1146];
        float sphereTexCoords[764];
        unsigned short sphereElements[2280];

        // get sphere data
        getSphereVertexData(sphereVertices, sphereNormals, sphereTexCoords, sphereElements);
        numVertices = getNumberOfSphereVertices();
        numElements = getNumberOfSphereElements();

        // positions
        glGenBuffers(1, &vboSphereVertices);
        glBindBuffer(GL_ARRAY_BUFFER, vboSphereVertices);
        {
            glBufferData(GL_ARRAY_BUFFER, sizeof(sphereVertices), sphereVertices, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(0);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // normals
        glGenBuffers(1, &vboSphereNormals);
        glBindBuffer(GL_ARRAY_BUFFER, vboSphereNormals);
        {
            glBufferData(GL_ARRAY_BUFFER, sizeof(sphereNormals), sphereNormals, GL_STATIC_DRAW);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(1);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // elements
        glGenBuffers(1, &vboSphereElements);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
        {
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphereElements), sphereElements, GL_STATIC_DRAW);
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        // uniform buffer objects
        // for transformations
        glCreateBuffers(1, &uboTransformations);
        glBindBuffer(GL_ARRAY_BUFFER, uboTransformations);
        {
            glBufferStorage(GL_ARRAY_BUFFER, 3 * sizeof(vmath::mat4), NULL, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
            glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboTransformations);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // for lighting
        glCreateBuffers(1, &uboLight);
        glBindBuffer(GL_ARRAY_BUFFER, uboLight);
        {
            glBufferStorage(GL_ARRAY_BUFFER, 4 * 3 * sizeof(vmath::vec4), NULL, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
            glBindBufferBase(GL_UNIFORM_BUFFER, 1, uboLight);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // for materials
        glCreateBuffers(1, &uboMaterial);
        glBindBuffer(GL_ARRAY_BUFFER, uboMaterial);
        {
            glBufferStorage(GL_ARRAY_BUFFER, 4 * sizeof(vmath::vec4), NULL, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
            glBindBufferBase(GL_UNIFORM_BUFFER, 2, uboMaterial);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // for viewer
        glCreateBuffers(1, &uboViewer);
        glBindBuffer(GL_ARRAY_BUFFER, uboViewer);
        {
            glBufferStorage(GL_ARRAY_BUFFER, 1 * sizeof(vmath::vec4), NULL, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
            glBindBufferBase(GL_UNIFORM_BUFFER, 3, uboViewer);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);

    // setting light params
    void *pMappedBuf;
    pMappedBuf = glMapNamedBufferRange(uboLight, 0, 9 * sizeof(vmath::vec4), GL_MAP_WRITE_BIT);
    {
        ((vmath::vec4 *)(pMappedBuf))[0] = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);  // La[0]
        ((vmath::vec4 *)(pMappedBuf))[1] = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);  // La[1]
        ((vmath::vec4 *)(pMappedBuf))[2] = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);  // La[2]

        ((vmath::vec4 *)(pMappedBuf))[3] = vmath::vec4(1.0f, 0.0f, 0.0f, 1.0f);  // Ld[0]
        ((vmath::vec4 *)(pMappedBuf))[4] = vmath::vec4(0.0f, 1.0f, 0.0f, 1.0f);  // Ld[1]
        ((vmath::vec4 *)(pMappedBuf))[5] = vmath::vec4(0.0f, 0.0f, 1.0f, 1.0f);  // Ld[2]

        ((vmath::vec4 *)(pMappedBuf))[6] = vmath::vec4(1.0f, 0.0f, 0.0f, 1.0f);  // Ls[0]
        ((vmath::vec4 *)(pMappedBuf))[7] = vmath::vec4(0.0f, 1.0f, 0.0f, 1.0f);  // Ls[1]
        ((vmath::vec4 *)(pMappedBuf))[8] = vmath::vec4(0.0f, 0.0f, 1.0f, 1.0f);  // Ls[2]
    }
    glUnmapNamedBuffer(uboLight);
    pMappedBuf = NULL;

    // setting material params
    pMappedBuf = glMapNamedBufferRange(uboMaterial, 0, 4 * sizeof(vmath::vec4), GL_MAP_WRITE_BIT);
    {
        ((vmath::vec4 *)(pMappedBuf))[0] = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);  // Ka
        ((vmath::vec4 *)(pMappedBuf))[1] = vmath::vec4(1.0f, 1.0f, 1.0f, 1.0f);  // Kd
        ((vmath::vec4 *)(pMappedBuf))[2] = vmath::vec4(1.0f, 1.0f, 1.0f, 1.0f);  // Ks
        ((vmath::vec4 *)(pMappedBuf))[3] = vmath::vec4(128.0f, 0.0f, 0.0f, 1.0f);  // material shininess or specular power
    }
    glUnmapNamedBuffer(uboMaterial);
    pMappedBuf = NULL;

    // set clear color
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// depth-related changes
	glClearDepth(1.0f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);

    // warm-up resize
    resizeSphere(FBO_WIDTH, FBO_HEIGHT);

    return 0;
}

void logGLInfo(void)
{
	// variable declarations
	GLint numExtensions = 0;

	// code
	fprintf(logFile, "\n-------------------- OpenGL Properties --------------------\n\n");
	fprintf(logFile, "OpenGL Vendor   : %s\n", glGetString(GL_VENDOR));
	fprintf(logFile, "OpenGL Renderer : %s\n", glGetString(GL_RENDERER));
	fprintf(logFile, "OpenGL Version  : %s\n", glGetString(GL_VERSION));
	fprintf(logFile, "GLSL Version    : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	fprintf(logFile, "\n-------------------- OpenGL Extensions --------------------\n\n");
	glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
	fprintf(logFile, "Number of supported extensions : %d\n\n", numExtensions);
	for (int i = 0; i < numExtensions; i++)
	{
		fprintf(logFile, "%s\n", glGetStringi(GL_EXTENSIONS, i));
	}
	fprintf(logFile, "-------------------------------------------------------------\n\n");
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
			logFile,
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
		// using COMPONENT16 for GLES applications, has nothing to do with depth buffer, but with color depth
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
			fprintf(logFile, "*** framebuffer is not complete ***\n");
			return false;
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);  // renderbuffer and texture objects associated with an FBO are unbound automatically

	return true;
}

void resize(int width, int height)
{
    // variable declarations
    GLfloat aspectRatio;

    // code
    winWidth = width;
    winHeight = height;

    if(height == 0)
        height = 1;

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    // setting the projection matrix
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
    void *pMappedBuf = glMapNamedBufferRange(uboTransformations, 2 * sizeof(vmath::mat4), sizeof(vmath::mat4), GL_MAP_WRITE_BIT);
    {
        *(vmath::mat4 *)(pMappedBuf) = vmath::perspective(
            45.0f,
            aspectRatio,
            0.1f,
            100.0f
        );
    }
    glUnmapNamedBuffer(uboTransformations);
    pMappedBuf = NULL;
}

void render(void)
{
    // function prototypes
    void renderSphere(GLint, GLint);
    void updateSphere(void);

    // variable declarations
    vmath::mat4 modelViewMatrix;

    // code
    /****************** FBO setup *****************/
	if (fboResult)
	{
		renderSphere(FBO_WIDTH, FBO_HEIGHT);
		updateSphere();
	}

	/****************** Default *****************/
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	resize(winWidth, winHeight);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(cubeShaderProgramObject);
    {
        // transformations for this frame
        modelViewMatrix = vmath::translate(0.0f, 0.0f, -4.0f) *
            vmath::rotate(theta_Cube, 1.0f, 0.0f, 0.0f) *
            vmath::rotate(theta_Cube, 0.0f, 1.0f, 0.0f) *
            vmath::rotate(theta_Cube, 0.0f, 0.0f, 1.0f) *
            vmath::scale(0.75f, 0.75f, 0.75f);

        // setting transformation uniforms
        glUniformMatrix4fv(glGetUniformLocation(cubeShaderProgramObject, "u_mvpMatrix"), 1, GL_FALSE, perspectiveProjectionMatrix_Cube * modelViewMatrix);

        // setting texture params
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(cubeShaderProgramObject, "u_textureSampler"), 0);
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

    glXSwapBuffers(display, window);
}

void renderSphere(int textureWidth, int textureHeight)
{
    // variable declarations
    vmath::mat4 modelMatrix = vmath::mat4::identity();
    const vmath::vec3 viewerPosition = vmath::vec3(0.0f, 0.0f, 1.0f);
    const vmath::mat4 viewMatrix = vmath::lookat(
        viewerPosition,
        vmath::vec3(0.0f, 0.0f, 0.0f),
        vmath::vec3(0.0f, 1.0f, 0.0f)
    );

    // code
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	resizeSphere(textureWidth, textureHeight);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw
    glUseProgram(sphereShaderProgramObject);
    {
        // transformations on the sphere
        modelMatrix = vmath::translate(0.0f, 0.0f, -2.0f);

        // update per-frame uniforms
        if(lightEnabled)
            glUniform1i(0, 1);
        else
            glUniform1i(0, 0);

        void *pMappedBuf = NULL;
        pMappedBuf = glMapNamedBufferRange(uboTransformations, 0, 2 * sizeof(vmath::mat4), GL_MAP_WRITE_BIT);
        {
            ((vmath::mat4 *)(pMappedBuf))[0] = modelMatrix;
            ((vmath::mat4 *)(pMappedBuf))[1] = viewMatrix;
        }
        glUnmapNamedBuffer(uboTransformations);
        pMappedBuf = NULL;

        // setting light positions
        pMappedBuf = glMapNamedBufferRange(uboLight, 9 * sizeof(vmath::vec4), 3 * sizeof(vmath::vec4), GL_MAP_WRITE_BIT);
        {
            ((vmath::vec4 *)(pMappedBuf))[0] = vmath::vec4(
                0.0f,
                10.0f * sinf(vmath::radians(theta_Sphere)),
                10.0f * cosf(vmath::radians(theta_Sphere)),
                1.0f
            );

            ((vmath::vec4 *)(pMappedBuf))[1] = vmath::vec4(
                10.0f * cosf(vmath::radians(theta_Sphere)),
                0.0f,
                10.0f * sinf(vmath::radians(theta_Sphere)),
                1.0f
            );

            ((vmath::vec4 *)(pMappedBuf))[2] = vmath::vec4(
                10.0f * cosf(vmath::radians(theta_Sphere)),
                10.0f * sinf(vmath::radians(theta_Sphere)),
                0.0f,
                1.0f
            );
        }
        glUnmapNamedBuffer(uboLight);
        pMappedBuf = NULL;

        // setting viewer params
        pMappedBuf = glMapNamedBufferRange(uboViewer, 0, 1 * sizeof(vmath::vec4), GL_MAP_WRITE_BIT);
        {
            ((vmath::vec4 *)(pMappedBuf))[0] = vmath::vec4(viewerPosition, 1.0f);  // viewerPosition
        }
        glUnmapNamedBuffer(uboViewer);
        pMappedBuf = NULL;

        // draw the frame
        glBindVertexArray(vaoSphere);
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
            glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);
        }
        glBindVertexArray(0);
    }
    glUseProgram(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void update(void)
{
    // code
    theta_Cube += 0.4f;
    if(theta_Cube > 360.0f)
        theta_Cube -= 360.0f;
}

void updateSphere(void)
{
    // code
    theta_Sphere += 1.6f;
    if(theta_Sphere > 360.0f)
        theta_Sphere -= 360.0f;
}

void uninitialize(void)
{
    // function prototypes
    void uninitializeSphere(void);

    // local variables
    GLXContext currentContext;

    // code
    if(fullScreen)
    {
        toggleFullscreen();
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
        fprintf(logFile, "detached and deleted %d shader objects\n", numAttachedShaders);

        glUseProgram(0);
        glDeleteProgram(cubeShaderProgramObject);
        cubeShaderProgramObject = 0;
        fprintf(logFile, "deleted shader program object\n");
    }

    currentContext = glXGetCurrentContext();
    if(currentContext && currentContext == glxContext)
    {
        glXMakeCurrent(display, 0, 0);
        currentContext = NULL;
    }

    if(glxContext)
    {
        glXDestroyContext(display, glxContext);
        glxContext = NULL;
    }

    if(visualInfo)
    {
        free(visualInfo);
        visualInfo = NULL;
    }

    if(window)
    {
        XDestroyWindow(display, window);
        window = 0;
    }

    if(colormap)
    {
        XFreeColormap(display, colormap);
        colormap = 0;
    }

    if(display)
    {
        XCloseDisplay(display);
        display = NULL;
    }

    if(logFile)
    {
        fprintf(logFile, "closing log file\n");
        fclose(logFile);
        logFile = NULL;
    }
}

void uninitializeSphere(void)
{
    // code
    if(uboViewer)
    {
        glDeleteBuffers(1, &uboViewer);
        uboViewer = 0U;
    }

    if(uboMaterial)
    {
        glDeleteBuffers(1, &uboMaterial);
        uboMaterial = 0U;
    }

    if(uboLight)
    {
        glDeleteBuffers(1, &uboLight);
        uboLight = 0U;
    }

    if(uboTransformations)
    {
        glDeleteBuffers(1, &uboTransformations);
        uboTransformations = 0U;
    }

    if(vboSphereNormals)
    {
        glDeleteBuffers(1, &vboSphereNormals);
        vboSphereNormals = 0U;
    }

    if(vboSphereVertices)
    {
        glDeleteBuffers(1, &vboSphereVertices);
        vboSphereVertices = 0U;
    }

    if(vaoSphere)
    {
        glDeleteVertexArrays(1, &vaoSphere);
        vaoSphere = 0U;
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
        fprintf(logFile, "detached and deleted %d shader objects\n", numAttachedShaders);

        glUseProgram(0);
        glDeleteProgram(sphereShaderProgramObject);
        sphereShaderProgramObject = 0;
        fprintf(logFile, "deleted shader program object\n");
    }
}
