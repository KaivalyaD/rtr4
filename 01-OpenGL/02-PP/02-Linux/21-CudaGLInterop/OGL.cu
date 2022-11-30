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

/* Cuda-GL interop headers */
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>

/* macros */
#define WIN_WIDTH 1200
#define WIN_HEIGHT 800

#define MAX_MESH_WIDTH 2048
#define MIN_MESH_WIDTH 128
#define MAX_MESH_HEIGHT 2048
#define MIN_MESH_HEIGHT 128

/* enums */
enum {
    KVD_ATTRIBUTE_POSITION = 0,
    KVD_ATTRIBUTE_COLOR,
    KVD_ATTRIBUTE_NORMAL,
    KVD_ATTRIBUTE_TEXTURE0,
};

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

/* related to the OpenGL programmable pipeline */
GLuint shaderProgramObject;

/* globals for rendering */
GLuint vao;
GLuint vbo;
GLuint gpuVbo;
GLuint mvpMatrixUniform;

vmath::mat4 perspectiveProjectionMatrix;

unsigned int meshWidth = MAX_MESH_WIDTH;
unsigned int meshHeight = MAX_MESH_HEIGHT;
unsigned int arraySize = meshWidth * meshHeight * 4;

float positions[MAX_MESH_HEIGHT][MAX_MESH_WIDTH][4];
float animTime = 0.0f;
bool onGPU = false;

cudaGraphicsResource_t graphicsResource = NULL;

/* cuda kernel to animate a wave on the point mesh */
__global__ void animateSineWaveOnGPU(float4 *positions, unsigned int width, unsigned int height, float phase)
{
    float freq = 4.0f;
    unsigned int z = blockIdx.y * blockDim.y + threadIdx.y;
    unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;

    if(x < width && z < height)
    {
        float u = (float)x / (float)width;
        float v = (float)z / (float)height;

        u = (2.0f * u) - 1.0f;
        v = (2.0f * v) - 1.0f;

        positions[(x * width) + z] = make_float4(u, 0.5f * sinf((u * freq) + phase) * cosf((v * freq) + phase), v, 1.0f);
    }
}

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
    defaultDepth;

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

                case XK_space:
                    onGPU = !onGPU;
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
    void checkCudaError(const char *, int);

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

    // cuda initialization
    int devCount = 0;
    cudaGetDeviceCount(&devCount); checkCudaError("cudaGetDeviceCount", __LINE__);
    if(devCount <= 0)
    {
        fprintf(logFile, "fatal error: initialize(): no cuda supporting devices with compute capability >= 2.0 found on this machine");
        uninitialize();
        exit(EXIT_FAILURE);
    }
    cudaSetDevice(0); checkCudaError("cudaSetDevice", __LINE__);  // select 0th device as default

	// logging OpenGL info
	logGLInfo();

	// vertex shader
	int status = 0;
	int infoLogLength = 0;
	char *infoLog = NULL;

	const GLchar *vertexShaderSourceCode =
		"#version 460 core\n" \
        "\n" \
        "in vec4 a_position;\n" \
        "\n" \
        "uniform mat4 u_mvpMatrix;\n" \
        "\n" \
		"void main(void)\n" \
		"{\n" \
			"gl_Position = u_mvpMatrix * a_position;\n" \
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
        "out vec4 FragColor;\n" \
        "\n" \
		"void main(void)\n" \
		"{\n" \
			"FragColor = vec4(1.0, 0.5, 0.0, 1.0);\n" \
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

	shaderProgramObject = glCreateProgram();
	glAttachShader(shaderProgramObject, vertexShaderObject);
	glAttachShader(shaderProgramObject, fragmentShaderObject);
    glBindAttribLocation(shaderProgramObject, KVD_ATTRIBUTE_POSITION, "a_position");
	glLinkProgram(shaderProgramObject);
	glGetProgramiv(shaderProgramObject, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		fprintf(logFile, "*** there were linking errors ***\n");

		glGetProgramiv(shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			infoLog = (char *)malloc(infoLogLength * sizeof(char));
			if (infoLog)
			{
				GLsizei written = 0;

				glGetProgramInfoLog(shaderProgramObject, infoLogLength * sizeof(char), &written, infoLog);
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

    /* post link processing */
    mvpMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_mvpMatrix");

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    {
        // buffer for cpu-computed sine wave
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, arraySize * sizeof(float), NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // buffer for gpu-computed sine wave
        glGenBuffers(1, &gpuVbo);
        glBindBuffer(GL_ARRAY_BUFFER, gpuVbo);
            glBufferData(GL_ARRAY_BUFFER, arraySize * sizeof(float), NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);

    // bring vertex positions array into a known state
    for(unsigned int i = 0; i < meshHeight; i++)
    {
        for(unsigned int j = 0; j < meshWidth; j++)
        {
            positions[i][j][0] = 0.0f;
            positions[i][j][1] = 0.0f;
            positions[i][j][2] = 0.0f;
            positions[i][j][3] = 0.0f;
        }
    }

    // create cuda/opengl interop resource
    // register gpuVbo as a writable graphics resource to cuda; discard memory after being used
    cudaGraphicsGLRegisterBuffer(&graphicsResource, gpuVbo, cudaGraphicsMapFlagsWriteDiscard); checkCudaError("cudaGraphicsGLRegisterBuffer", __LINE__);
    
    // setting up depth
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // setting the clear color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // initializing the perspective projection matrix
    perspectiveProjectionMatrix = vmath::mat4::identity();

    // warm-up resize
    resize(WIN_WIDTH, WIN_HEIGHT);

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

void resize(int width, int height)
{
    // variable declarations
    GLfloat aspectRatio;

    // code
    if(height == 0)
        height = 1;
    
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    aspectRatio = (GLfloat)width / (GLfloat)height;
    perspectiveProjectionMatrix = vmath::perspective(
        45.0f,
        aspectRatio,
        0.1f,
        100.0f
    );
}

void render(void)
{
    // function prototypes
    void animateSineWaveOnCPU(unsigned int, unsigned int, float);
    void checkCudaError(const char *, int);

    // variable declarations
    vmath::mat4 modelViewMatrix;
    vmath::mat4 modelViewProjectionMatrix;
    float4 *pPositions = NULL;
    size_t numMappedBytes = 0U;

    static char title[255];

    // code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgramObject);
    {
        // transformations
        modelViewMatrix = vmath::translate(0.0f, 0.0f, -1.5f);
        modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
        glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionMatrix);

        glBindVertexArray(vao);
        {
            if(!onGPU)
            {
                sprintf(title, "Kaivalya Deshpande: OpenGL-CUDA | Points Rendered = %d | Running on CPU", meshWidth * meshHeight);
                XStoreName(display, window, title);

                animateSineWaveOnCPU(meshWidth, meshHeight, animTime);
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                    glBufferData(GL_ARRAY_BUFFER, arraySize * sizeof(float), positions, GL_DYNAMIC_DRAW);
                    glVertexAttribPointer(KVD_ATTRIBUTE_POSITION, 4, GL_FLOAT, GL_FALSE, 0, NULL);
                    glEnableVertexAttribArray(KVD_ATTRIBUTE_POSITION);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
            else
            {
                sprintf(title, "Kaivalya Deshpande: OpenGL-CUDA | Points Rendered = %d | Running on GPU", meshWidth * meshHeight);
                XStoreName(display, window, title);

                // map cuda graphics resource
                cudaGraphicsMapResources(1, &graphicsResource, 0); checkCudaError("cudaGraphicsMapResources", __LINE__);

                // get pointer to the mapped resource
                cudaGraphicsResourceGetMappedPointer((void **)&pPositions, &numMappedBytes, graphicsResource); checkCudaError("cudaGraphicsResourceGetMappedPointer", __LINE__);

                // configure and launch the kernel
                dim3 block(8, 8, 1);
                dim3 grid((meshWidth / block.x) + 1, (meshHeight / block.y) + 1, 1);
                animateSineWaveOnGPU<<< grid, block >>>(pPositions, meshWidth, meshHeight, animTime); checkCudaError("(launcher) animateSineWaveOnGPU<<< >>>", __LINE__);

                // unmap graphics resource
                cudaGraphicsUnmapResources(1, &graphicsResource, 0); checkCudaError("cudaGraphicsUnmapResources", __LINE__);
            
                // perform drawing rituals as usual
                glBindBuffer(GL_ARRAY_BUFFER, gpuVbo);
                    glVertexAttribPointer(KVD_ATTRIBUTE_POSITION, 4, GL_FLOAT, GL_FALSE, 0, NULL);
                    glEnableVertexAttribArray(KVD_ATTRIBUTE_POSITION);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }

            glDrawArrays(GL_POINTS, 0, meshWidth * meshHeight);
        }
        glBindVertexArray(0);
    }
	glUseProgram(0);

    glXSwapBuffers(display, window);
}

void update(void)
{
    // code
    animTime += 0.01f;
    if(animTime > 360.0f)
        animTime -= 360.0f;
}

void uninitialize(void)
{
    // local variables
    GLXContext currentContext;

    // code
    if(fullScreen)
    {
        toggleFullscreen();
    }

    if(gpuVbo)
    {
        if(graphicsResource)
        {
            cudaGraphicsUnregisterResource(graphicsResource);
            graphicsResource = NULL;
        }
        glDeleteBuffers(1, &gpuVbo);
        gpuVbo = 0U;
    }

    if(vbo)
    {
        glDeleteBuffers(1, &vbo);
        vbo = 0U;
    }

    if(vao)
    {
        glDeleteVertexArrays(1, &vao);
        vao = 0U;
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
		fprintf(logFile, "detached and deleted %d shader objects\n", numAttachedShaders);

		glUseProgram(0);
		glDeleteProgram(shaderProgramObject);
		shaderProgramObject = 0;
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

void animateSineWaveOnCPU(unsigned int width, unsigned int height, float phase)
{
    // local variables
    float u, v;
    float freq = 4.0f;

    // code
    for(unsigned int i = 0; i < height; i++)
    {
        for(unsigned int j = 0; j < width; j++)
        {
            // u, v -> [0, 1]
            u = (float)j / (float)width;
            v = (float)i / (float)height;

            // u, v -> [-1, 1]
            u = (2.0f * u) - 1.0f;
            v = (2.0f * v) - 1.0f;

            positions[i][j][0] = u;
            positions[i][j][1] = 0.5f * sinf((u * freq) + phase) * cosf((v * freq) + phase);
            positions[i][j][2] = v;
            positions[i][j][3] = 1.0f;
        }
    }
}

/* cuda error handling utility function */
void checkCudaError(const char *func, int lineNum)
{
    // code
    cudaError_t code = cudaGetLastError();
    if(code != cudaSuccess)
    {
        fprintf(logFile, "cuda error: %f() : line %d : %s(%d)\n", func, lineNum, cudaGetErrorString(code), code);
        uninitialize();
        exit(EXIT_FAILURE);
    }
}