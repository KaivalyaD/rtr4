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
GLuint vboSphereVertices;
GLuint vboSphereNormals;
GLuint vboSphereElements;

/* uniform buffer objects */
GLuint uboTransformations;
GLuint uboMaterial;
GLuint uboLight;

/* light switch */
Bool lightEnabled = False;
float theta = 0.0f;
unsigned short rotationAxis = 0;

/* misc */
unsigned int numVertices, numElements;

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

                case 'X':
                case 'x':
                    if(lightEnabled)
                        rotationAxis = 1;
                    break;

                case 'Y':
                case 'y':
                    if(lightEnabled)
                        rotationAxis = 2;
                    break;

                case 'Z':
                case 'z':
                    if(lightEnabled)
                        rotationAxis = 3;
                    break;

                default:
                    if(lightEnabled)
                        rotationAxis = 0;
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
            "vec3 La;  // ambient light base color\n" \
            "vec3 Ld;  // diffuse light base color\n" \
            "vec3 Ls;  // specular light base color\n" \
            "vec3 lightPosition;  // light position in world space\n" \
        "} ub_light;\n" \
        "\n" \
        "layout (location = 0) uniform int u_lightEnabled;\n" \
        "\n" \
        "out vec3 lightDirection;\n" \
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
                "lightDirection = vec3(ub_light.lightPosition - eyeCoordinate);\n" \
                "viewerDirection = -eyeCoordinate;\n" \
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
        "in vec3 lightDirection;\n" \
        "in vec3 transformedNormal;\n" \
        "in vec3 viewerDirection;\n" \
        "\n" \
        "layout (std140, binding = 1) uniform LightBlock\n" \
        "{\n"
            "vec3 La;  // ambient light base color\n" \
            "vec3 Ld;  // diffuse light base color\n" \
            "vec3 Ls;  // specular light base color\n" \
            "vec3 lightPosition;  // light position in world space\n" \
        "} ub_light;\n" \
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
                "vec3 normalizedLightDirection = normalize(lightDirection);\n" \
                "vec3 normalizedTransformedNormal = normalize(transformedNormal);\n" \
                "vec3 normalizedViewerDirection = normalize(viewerDirection);\n" \
                "\n" \
                "vec3 reflectedBeamDirection = reflect(-normalizedLightDirection, normalizedTransformedNormal);\n" \
                "\n" \
                "vec3 ambient = ub_light.La * ub_material.Ka;\n" \
                "vec3 diffuse = ub_light.Ld * ub_material.Kd * max(dot(normalizedLightDirection, normalizedTransformedNormal), 0.0);\n" \
                "vec3 specular = ub_light.Ls * ub_material.Ks * max(pow(dot(normalizedViewerDirection, reflectedBeamDirection), ub_material.materialShininess.x), 0.0);\n" \
                "\n" \
                "FragColor = vec4(ambient + diffuse + specular, 1.0);\n" \
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

    /* loading vertex data into pipeline */
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
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
            glBufferStorage(GL_ARRAY_BUFFER, 4 * sizeof(vmath::vec4), NULL, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
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
    }
    glBindVertexArray(0);

    // setting default light & material params
    void *pMappedBuf;
    pMappedBuf = glMapNamedBufferRange(uboLight, 0, 4 * sizeof(vmath::vec4), GL_MAP_WRITE_BIT);
    {
        ((vmath::vec4 *)(pMappedBuf))[0] = vmath::vec4(0.1f, 0.1f, 0.1f, 1.0f);  // La
        ((vmath::vec4 *)(pMappedBuf))[1] = vmath::vec4(1.0f, 1.0f, 1.0f, 1.0f);  // Ld
        ((vmath::vec4 *)(pMappedBuf))[2] = vmath::vec4(1.0f, 1.0f, 1.0f, 1.0f);  // Ls
        ((vmath::vec4 *)(pMappedBuf))[3] = vmath::vec4(100.0f, 100.0f, 100.0f, 1.0f);  // lightPosition
    }
    glUnmapNamedBuffer(uboLight);
    pMappedBuf = NULL;

    // setting material params
    pMappedBuf = glMapNamedBufferRange(uboMaterial, 0, 4 * sizeof(vmath::vec4), GL_MAP_WRITE_BIT);
    {
        ((vmath::vec4 *)(pMappedBuf))[0] = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);  // Ka
        ((vmath::vec4 *)(pMappedBuf))[1] = vmath::vec4(0.5f, 0.2f, 0.7f, 1.0f);  // Kd
        ((vmath::vec4 *)(pMappedBuf))[2] = vmath::vec4(0.7f, 0.7f, 0.7f, 1.0f);  // Ks
        ((vmath::vec4 *)(pMappedBuf))[3] = vmath::vec4(128.0f, 0.0f, 0.0f, 1.0f);  // material shininess or specular power
    }
    glUnmapNamedBuffer(uboMaterial);
    pMappedBuf = NULL;

    // setting up depth
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // setting the clear color
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);

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

    // setting the projection matrix
    aspectRatio = (GLfloat)width / (GLfloat)height;
    void *pMappedBuf = glMapNamedBufferRange(uboTransformations, 2 * sizeof(vmath::mat4), sizeof(vmath::mat4), GL_MAP_WRITE_BIT);
    {
        *(vmath::mat4 *)(pMappedBuf) = vmath::perspective(
            25.0f,
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
    void draw24Spheres(void);

    // code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    draw24Spheres();

    glXSwapBuffers(display, window);
}

void update(void)
{
    // code
    theta += 2.0f;
    if(theta > 360.0f)
        theta -= 360.0f;
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

void setModelMatrix(vmath::mat4 &modelMat)
{
    // code
    void *pMappedBuf = glMapNamedBufferRange(uboTransformations, 0, 2 * sizeof(vmath::mat4), GL_MAP_WRITE_BIT);
    {
        ((vmath::mat4 *)(pMappedBuf))[0] = modelMat;
    }
    glUnmapNamedBuffer(uboTransformations);
    pMappedBuf = NULL;
}

void setViewMatrix(vmath::mat4 &viewMat)
{
    // code
    void *pMappedBuf = glMapNamedBufferRange(uboTransformations, 0, 2 * sizeof(vmath::mat4), GL_MAP_WRITE_BIT);
    {
        ((vmath::mat4 *)(pMappedBuf))[1] = viewMat;
    }
    glUnmapNamedBuffer(uboTransformations);
    pMappedBuf = NULL;
}

void setLightPosition(vmath::vec4 &position)
{
    // code
    void *pMappedBuf = glMapNamedBufferRange(uboLight, 0, 4 * sizeof(vmath::vec4), GL_MAP_WRITE_BIT);
    {
        ((vmath::vec4 *)(pMappedBuf))[3] = position;
    }
    glUnmapNamedBuffer(uboLight);
    pMappedBuf = NULL;
}

void setMaterial(vmath::vec3& ka, vmath::vec3& kd, vmath::vec3& ks, float shininess)
{
    // code
    // setting material params
    void *pMappedBuf = glMapNamedBufferRange(uboMaterial, 0, 4 * sizeof(vmath::vec4), GL_MAP_WRITE_BIT);
    {
        ((vmath::vec4 *)(pMappedBuf))[0] = vmath::vec4(ka, 1.0f);  // Ka
        ((vmath::vec4 *)(pMappedBuf))[1] = vmath::vec4(kd, 1.0f);  // Kd
        ((vmath::vec4 *)(pMappedBuf))[2] = vmath::vec4(ks, 1.0f);  // Ks
        ((vmath::vec4 *)(pMappedBuf))[3] = vmath::vec4(shininess, 0.0f, 0.0f, 1.0f);  // material shininess or specular power
    }
    glUnmapNamedBuffer(uboMaterial);
    pMappedBuf = NULL;
}

void draw24Spheres(void)
{
    // variable declarations
	vmath::mat4 modelMatrix = vmath::mat4::identity();
	vmath::mat4 viewMatrix = vmath::mat4::identity();
    
    vmath::vec4 lightPosition;
    vmath::vec3 ka, kd, ks;
    float shininess = 0.0f;

    const vmath::vec4 viewerPos = vmath::vec4();

	// view transformation
	viewMatrix = vmath::lookat(
		vmath::vec3(0.0f, 3.75f, 20.0f),
		vmath::vec3(0.0f, 3.75f, 0.0f),
		vmath::vec3(0.0f, 1.0f, 0.0f)
	);

    // code
	if (rotationAxis == 1) {
        lightPosition = vmath::vec4(0.0f, 50.0f * sinf(vmath::radians(theta)), 50.0f * cosf(vmath::radians(theta)), 1.0f);
	}
	else if (rotationAxis == 2) {
		lightPosition = vmath::vec4(50.0f * cosf(vmath::radians(theta)), 0.0f, 50.0f * sinf(vmath::radians(theta)), 1.0f);
	}
	else if (rotationAxis == 3) {
		lightPosition = vmath::vec4(50.0f * cosf(vmath::radians(theta)), 50.0f * sinf(vmath::radians(theta)), 0.0f, 1.0f);
	}
	else {
		lightPosition = vmath::vec4(0.0f, 3.75f, 11.0f, 1.0f);
	}

    glUseProgram(shaderProgramObject);
    {
        glBindVertexArray(vao);
        {
            setViewMatrix(viewMatrix);

            if(lightEnabled)
            {
                setLightPosition(lightPosition);
                glUniform1i(0, 1);

                // [1, 1] material: emerald
                ka = vmath::vec3(0.0215f, 0.1745f, 0.0215f);
                kd = vmath::vec3(0.07568f, 0.61424f, 0.07568f);
                ks = vmath::vec3(0.633f, 0.727811f, 0.633f);
                shininess = 0.6f * 128.0f;
                setMaterial(ka, kd, ks, shininess);
                modelMatrix = vmath::translate(-4.5f, 7.5f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [2, 1] material: jade
                ka = vmath::vec3(0.135f, 0.2225f, 0.1575f);
                kd = vmath::vec3(0.54f, 0.89f, 0.63f);
                ks = vmath::vec3(0.316228f, 0.316228f, 0.316228f);
                shininess = 0.1f * 128.0f;
                setMaterial(ka, kd, ks, shininess);
                modelMatrix = vmath::translate(-4.5f, 6.0f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [3, 1] material: obsidian
                ka = vmath::vec3(0.05375f, 0.05f, 0.06625f);
                kd = vmath::vec3(0.18275f, 0.17f, 0.22525f);
                ks = vmath::vec3(0.332741f, 0.328634f, 0.346435f);
                shininess = 0.3f * 128.0f;
                setMaterial(ka, kd, ks, shininess);
                modelMatrix = vmath::translate(-4.5f, 4.5f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [4, 1] material: pearl
                ka = vmath::vec3(0.25f, 0.20725f, 0.20725f);
                kd = vmath::vec3(1.0f, 0.829f, 0.829f);
                ks = vmath::vec3(0.296648f, 0.296648f, 0.296648f);
                shininess = 0.3f * 128.0f;
                setMaterial(ka, kd, ks, shininess);
                modelMatrix = vmath::translate(-4.5f, 3.0f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [5, 1] material: ruby
                ka = vmath::vec3(0.1745f, 0.01175f, 0.01175f);
                kd = vmath::vec3(0.61424f, 0.04136f, 0.04136f);
                ks = vmath::vec3(0.727811f, 0.727811f, 0.727811f);
                shininess = 0.6f * 128.0f;
                setMaterial(ka, kd, ks, shininess);
                modelMatrix = vmath::translate(-4.5f, 1.5f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [6, 1] material: turquoise
                ka = vmath::vec3(0.1f, 0.18725f, 0.1745f);
                kd = vmath::vec3(0.396f, 0.74151f, 0.69102f);
                ks = vmath::vec3(0.297254f, 0.30829f, 0.306678f);
                shininess = 0.1f * 128.0f;
                setMaterial(ka, kd, ks, shininess);
                modelMatrix = vmath::translate(-4.5f, 0.0f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [1, 2] material: brass
                ka = vmath::vec3(0.329412f, 0.223529f, 0.027451f);
                kd = vmath::vec3(0.780392f, 0.568627f, 0.113725f);
                ks = vmath::vec3(0.992157f, 0.941176f, 0.807843f);
                shininess = 0.21794872f * 128.0f;
                setMaterial(ka, kd, ks, shininess);
                modelMatrix = vmath::translate(-1.5f, 7.5f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [2, 2] material: bronze
                ka = vmath::vec3(0.2125f, 0.1275f, 0.054f);
                kd = vmath::vec3(0.714f, 0.4284f, 0.18144f);
                ks = vmath::vec3(0.393548f, 0.271906f, 0.166721f);
                shininess = 0.2f * 128.0f;
                setMaterial(ka, kd, ks, shininess);
                modelMatrix = vmath::translate(-1.5f, 6.0f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [3, 2] material: chrome
                ka = vmath::vec3(0.25f, 0.25f, 0.25f);
                kd = vmath::vec3(0.4f, 0.4f, 0.4f);
                ks = vmath::vec3(0.774597f, 0.774597f, 0.774597f);
                shininess = 0.6f * 128.0f;
                setMaterial(ka, kd, ks, shininess);
                modelMatrix = vmath::translate(-1.5f, 4.5f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [4, 2] material: copper
                ka = vmath::vec3(0.19125f, 0.0735f, 0.0225f);
                kd = vmath::vec3(0.7038f, 0.27048f, 0.0828f);
                ks = vmath::vec3(0.256777f, 0.137622f, 0.086014f);
                shininess = 0.1f * 128.0f;
                setMaterial(ka, kd, ks, shininess);
                modelMatrix = vmath::translate(-1.5f, 3.0f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [5, 2] material: gold
                ka = vmath::vec3(0.24725f, 0.1995f, 0.0745f);
                kd = vmath::vec3(0.75164f, 0.60648f, 0.22648f);
                ks = vmath::vec3(0.628281f, 0.555802f, 0.366065f);
                shininess = 0.4f * 128.0f;
                setMaterial(ka, kd, ks, shininess);
                modelMatrix = vmath::translate(-1.5f, 1.5f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [6, 2] material: silver
                ka = vmath::vec3(0.19225f, 0.19225f, 0.19225f);
                kd = vmath::vec3(0.50754f, 0.50754f, 0.50754f);
                ks = vmath::vec3(0.508273f, 0.508273f, 0.508273f);
                shininess = 0.4f * 128.0f;
                setMaterial(ka, kd, ks, shininess);
                modelMatrix = vmath::translate(-1.5f, 0.0f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [1, 3] material: black plastic
                ka = vmath::vec3(0.0f, 0.0f, 0.0f);
                kd = vmath::vec3(0.01f, 0.01f, 0.01f);
                ks = vmath::vec3(0.5f, 0.5f, 0.5f);
                shininess = 0.25f * 128.0f;
                setMaterial(ka, kd, ks, shininess);
                modelMatrix = vmath::translate(1.5f, 7.5f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [2, 3] material: cyan plastic
                ka = vmath::vec3(0.0f, 0.1f, 0.06f);
                kd = vmath::vec3(0.0f, 0.50980392f, 0.50980392f);
                ks = vmath::vec3(0.50196078f, 0.50196078f, 0.50196078f);
                shininess = 0.25f * 128.0f;
                setMaterial(ka, kd, ks, shininess);
                modelMatrix = vmath::translate(1.5f, 6.0f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [3, 3] material: green plastic
                ka = vmath::vec3(0.0f, 0.0f, 0.0f);
                kd = vmath::vec3(0.1f, 0.35f, 0.1f);
                ks = vmath::vec3(0.45f, 0.55f, 0.45f);
                shininess = 0.25f * 128.0f;
                setMaterial(ka, kd, ks, shininess);
                modelMatrix = vmath::translate(1.5f, 4.5f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [4, 3] material: red plastic
                ka = vmath::vec3(0.0f, 0.0f, 0.0f);
                kd = vmath::vec3(0.5f, 0.0f, 0.0f);
                ks = vmath::vec3(0.7f, 0.6f, 0.6f);
                shininess = 0.25f * 128.0f;
                setMaterial(ka, kd, ks, shininess);
                modelMatrix = vmath::translate(1.5f, 3.0f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [5, 3] material: white plastic
                ka = vmath::vec3(0.0f, 0.0f, 0.0f);
                kd = vmath::vec3(0.55f, 0.55f, 0.55f);
                ks = vmath::vec3(0.7f, 0.7f, 0.7f);
                shininess = 0.25f * 128.0f;
                setMaterial(ka, kd, ks, shininess);
                modelMatrix = vmath::translate(1.5f, 1.5f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [6, 3] material: yellow plastic
                ka = vmath::vec3(0.0f, 0.0f, 0.0f);
                kd = vmath::vec3(0.5f, 0.5f, 0.0f);
                ks = vmath::vec3(0.6f, 0.6f, 0.5f);
                shininess = 0.25f * 128.0f;
                setMaterial(ka, kd, ks, shininess);
                modelMatrix = vmath::translate(1.5f, 0.0f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [1, 4] material: black rubber
                ka = vmath::vec3(0.02f, 0.02f, 0.02f);
                kd = vmath::vec3(0.01f, 0.01f, 0.01f);
                ks = vmath::vec3(0.4f, 0.4f, 0.4f);
                shininess = 0.078125f * 128.0f;
                setMaterial(ka, kd, ks, shininess);
                modelMatrix = vmath::translate(4.5f, 7.5f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [2, 4] material: cyan rubber
                ka = vmath::vec3(0.0f, 0.05f, 0.05f);
                kd = vmath::vec3(0.4f, 0.5f, 0.5f);
                ks = vmath::vec3(0.04f, 0.7f, 0.7f);
                shininess = 0.078125f * 128.0f;
                setMaterial(ka, kd, ks, shininess);
                modelMatrix = vmath::translate(4.5f, 6.0f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [3, 4] material: green rubber
                ka = vmath::vec3(0.0f, 0.05f, 0.05f);
                kd = vmath::vec3(0.4f, 0.5f, 0.4f);
                ks = vmath::vec3(0.04f, 0.7f, 0.04f);
                shininess = 0.078125f * 128.0f;
                setMaterial(ka, kd, ks, shininess);
                modelMatrix = vmath::translate(4.5f, 4.5f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [4, 4] material: red rubber
                ka = vmath::vec3(0.05f, 0.0f, 0.0f);
                kd = vmath::vec3(0.5f, 0.4f, 0.4f);
                ks = vmath::vec3(0.7f, 0.04f, 0.04f);
                shininess = 0.078125f * 128.0f;
                setMaterial(ka, kd, ks, shininess);
                modelMatrix = vmath::translate(4.5f, 3.0f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [5, 4] material: white rubber
                ka = vmath::vec3(0.05f, 0.05f, 0.05f);
                kd = vmath::vec3(0.5f, 0.5f, 0.5f);
                ks = vmath::vec3(0.7f, 0.7f, 0.7f);
                shininess = 0.078125f * 128.0f;
                setMaterial(ka, kd, ks, shininess);
                modelMatrix = vmath::translate(4.5f, 1.5f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [6, 4] material: yellow rubber
                ka = vmath::vec3(0.05f, 0.05f, 0.0f);
                kd = vmath::vec3(0.5f, 0.5f, 0.4f);
                ks = vmath::vec3(0.7f, 0.7f, 0.04f);
                shininess = 0.078125f * 128.0f;
                setMaterial(ka, kd, ks, shininess);
                modelMatrix = vmath::translate(4.5f, 0.0f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);
            }
            else
            {
                glUniform1i(glGetUniformLocation(shaderProgramObject, "u_lightEnabled"), 0);
            
                // [1, 1]
                modelMatrix = vmath::translate(-4.5f, 7.5f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [2, 1]
                modelMatrix = vmath::translate(-4.5f, 6.0f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [3, 1]
                modelMatrix = vmath::translate(-4.5f, 4.5f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [4, 1]
                modelMatrix = vmath::translate(-4.5f, 3.0f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [5, 1]
                modelMatrix = vmath::translate(-4.5f, 1.5f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [6, 1]
                modelMatrix = vmath::translate(-4.5f, 0.0f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [1, 2]
                modelMatrix = vmath::translate(-1.5f, 7.5f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [2, 2]
                modelMatrix = vmath::translate(-1.5f, 6.0f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [3, 2]
                modelMatrix = vmath::translate(-1.5f, 4.5f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [4, 2]
                modelMatrix = vmath::translate(-1.5f, 3.0f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [5, 2]
                modelMatrix = vmath::translate(-1.5f, 1.5f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [6, 2]
                modelMatrix = vmath::translate(-1.5f, 0.0f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [1, 3]
                modelMatrix = vmath::translate(1.5f, 7.5f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [2, 3]
                modelMatrix = vmath::translate(1.5f, 6.0f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [3, 3]
                modelMatrix = vmath::translate(1.5f, 4.5f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [4, 3]
                modelMatrix = vmath::translate(1.5f, 3.0f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [5, 3]
                modelMatrix = vmath::translate(1.5f, 1.5f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [6, 3]
                modelMatrix = vmath::translate(1.5f, 0.0f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [1, 4]
                modelMatrix = vmath::translate(4.5f, 7.5f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [2, 4]
                modelMatrix = vmath::translate(4.5f, 6.0f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [3, 4]
                modelMatrix = vmath::translate(4.5f, 4.5f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [4, 4]
                modelMatrix = vmath::translate(4.5f, 3.0f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [5, 4]
                modelMatrix = vmath::translate(4.5f, 1.5f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);

                // [6, 4]
                modelMatrix = vmath::translate(4.5f, 0.0f, 0.0f);
                setModelMatrix(modelMatrix);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);
            }
        }
        glBindVertexArray(0);
    }
    glUseProgram(0);
}
