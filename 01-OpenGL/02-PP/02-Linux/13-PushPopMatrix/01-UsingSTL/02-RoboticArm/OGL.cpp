/* standard headers */
#include <stdio.h>  // for standard I/O
#include <stdlib.h> // for exit()
#include <memory.h> // for memset()
#include <stack>  // for std::stack

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

#define max(x, y) ((x > y) ? (x) : (y))
#define min(x, y) ((x < y) ? (x) : (y))

/* enums */
enum
{
    KVD_ATTRIBUTE_POSITION = 0,
    KVD_ATTRIBUTE_COLOR,
    KVD_ATTRIBUTE_NORMAL,
    KVD_ATTRIBUTE_TEXTURE0,
};

/* typedefs */
typedef GLXContext (*glXCreateContextAttribsARBProc)(Display *, GLXFBConfig, GLXContext, Bool, const int *);

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
// objects data VRAM
GLuint vaoSphere;
GLuint vboSphereVertices;
GLuint vboSphereNormals;
GLuint vboSphereElements;

GLuint vaoCuboids;
GLuint vboCuboidPositions;
GLuint vboCuboidNormals;

// transformation matrices and uniforms
GLuint modelViewMatrixUniform;
GLuint projectionMatrixUniform;
vmath::mat4 perspectiveProjectionMatrix;
std::stack<vmath::mat4> stackMatModelView;

// lighting
GLuint LdUniform;
GLuint lightPositionUniform;

// object data RAM
float sphereVertices[1146];
float sphereNormals[1146];
float sphereTexCoords[764];
unsigned short sphereElements[2280];
unsigned int numVertices, numElements;

// controls
float shoulder_yaw, shoulder_pitch;
float elbow_pitch, elbow_roll;
float wrist_pitch, wrist_roll;
float finger_a, finger_b;

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
        GLX_X_RENDERABLE, True,            // hardware accelerated renderable
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT, // draw to a window
        GLX_RENDER_TYPE, GLX_RGBA_BIT,     // the render type is an RGBA frame
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_STENCIL_SIZE, 8,
        GLX_DEPTH_SIZE, 24,
        GLX_DOUBLEBUFFER, True,
        None};
    Bool done = False;
    int ret;
    int winWidth, winHeight;

    // code
    logFile = fopen("log.txt", "w");
    if (!logFile)
    {
        fprintf(stderr, "fopen: failed to open log.txt\n");
        uninitialize();
        exit(EXIT_FAILURE);
    }
    fprintf(logFile, "successfully created log file\n");

    display = XOpenDisplay(NULL);
    if (!display)
    {
        fprintf(stderr, "error: XOpenDisplay() failed\n");
        uninitialize();
        exit(EXIT_FAILURE);
    }

    defaultScreen = XDefaultScreen(display);
    defaultDepth = XDefaultDepth(display, defaultScreen);

    // getting all conforming FB configurations
    glxFBConfigs = glXChooseFBConfig(display, defaultScreen, framebufferAttributes, &cntFBConfigs);
    if (!glxFBConfigs)
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

    for (int i = 0; i < cntFBConfigs; i++)
    {
        tempXVisualInfo = glXGetVisualFromFBConfig(display, glxFBConfigs[i]);
        if (!tempXVisualInfo)
            continue; // if an FB config has no XVisualInfo, it cannot be the best

        int samples, sampleBuffers;
        glXGetFBConfigAttrib(display, glxFBConfigs[i], GLX_SAMPLE_BUFFERS, &sampleBuffers);
        glXGetFBConfigAttrib(display, glxFBConfigs[i], GLX_SAMPLES, &samples);

        fprintf(logFile, "[%d] XVisualInfo (visualid 0x%lx) has: %d sample buffers and %d samples\n", (i + 1), tempXVisualInfo->visualid, sampleBuffers, samples);

        if ((bestFramebufferConfigIdx < 0) || (sampleBuffers && (samples > bestCountOfSamples)))
        {
            bestFramebufferConfigIdx = i;
            bestCountOfSamples = samples;
        }

        if ((worstFramebufferConfigIdx >= 999) || !sampleBuffers || (samples < worstCountOfSamples))
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
        &windowAttributes);
    if (!window)
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
    if (ret == -1)
    {
        fprintf(logFile, "error: glXGetProcAddressARB() failed\n");
        uninitialize();
        exit(EXIT_FAILURE);
    }
    fprintf(logFile, "OpenGL initialized successfully\n");

    // game loop
    while (!done)
    {
        while (XPending(display))
        {
            XNextEvent(display, &event);
            switch (event.type)
            {
            case MapNotify:
                break;

            case KeyPress:
                keysym = XkbKeycodeToKeysym(display, event.xkey.keycode, 0, 0);
                switch (keysym)
                {
                case XK_Escape:
                    done = True;
                    break;

                default:
                    break;
                }

                XLookupString(&event.xkey, keys, sizeof(keys), NULL, NULL);
                switch (*keys)
                {
                case 'F':
                case 'f':
                    toggleFullscreen();
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

            case FocusIn: /* WM_SETFOCUS in Win32 */
                activeWindow = True;
                break;

            case FocusOut: /* WM_KILLFOCUS in Win32 */
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

        if (activeWindow)
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
    if (fullScreen)
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
    GLuint generateCuboids(GLuint *, GLuint *, const size_t, const vmath::vec3 *);

    // variable declarations
    GLint contextAttributes[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 4, // for an OpenGL 4.6 context
        GLX_CONTEXT_MINOR_VERSION_ARB, 6,
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB, // core-profile OpenGL
        None};

    // code
    glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((GLubyte *)"glXCreateContextAttribsARB");
    if (!glXCreateContextAttribsARB)
        return -1;

    // passing the 2nd last argument as True means enabling hardware support for rendering
    glxContext = glXCreateContextAttribsARB(display, glxFBConfig, NULL, True, contextAttributes);
    if (!glxContext)
    {
        GLint contextAttributesFallback[] = {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 1, // this does not mean you get OpenGL 1.0 strictly, but the highest supported version
            GLX_CONTEXT_MINOR_VERSION_ARB, 0,
            None};

        glxContext = glXCreateContextAttribsARB(display, glxFBConfig, NULL, True, contextAttributesFallback);
        fprintf(logFile, "cannot support OpenGL version 4.6; falling back to the default\n");
    }
    else
        fprintf(logFile, "OpenGL 4.6 is fully supported\n");

    // checking whether hardware/direct rendering is supported or not
    if (glXIsDirect(display, glxContext))
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
        "in vec4 a_position;\n" \
        "in vec4 a_normal;\n" \
        "\n" \
        "uniform mat4 u_modelViewMatrix;\n" \
        "uniform mat4 u_projectionMatrix;\n" \
        "uniform vec4 u_lightPosition;\n" \
        "\n" \
        "out vec3 a_normal_out;\n" \
        "out vec3 lightDirection;\n" \
        "\n"
        "void main(void)\n" \
        "{\n" \
            "gl_Position = u_projectionMatrix * u_modelViewMatrix * a_position;\n" \
            "\n" \
            "vec3 eyeCoordinate = vec3(u_modelViewMatrix * a_position);\n" \
            "lightDirection = vec3(u_lightPosition) - eyeCoordinate;\n" \
            "a_normal_out = mat3(transpose(inverse(u_modelViewMatrix))) * a_normal.xyz;\n" \
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
    glBindAttribLocation(shaderProgramObject, KVD_ATTRIBUTE_NORMAL, "a_normal");
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
    modelViewMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_modelViewMatrix");
    projectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_projectionMatrix");
    lightPositionUniform = glGetUniformLocation(shaderProgramObject, "u_lightPosition");
    LdUniform = glGetUniformLocation(shaderProgramObject, "u_Ld");

    /* loading vertex data into pipeline */
    // spheres
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

    // setting up depth
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // setting the clear color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // initializing the projection matrix
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
    if (height == 0)
        height = 1;

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    aspectRatio = (GLfloat)width / (GLfloat)height;
    perspectiveProjectionMatrix = vmath::perspective(45.0f, aspectRatio, 0.1f, 100.0f);
}

void render(void)
{
    // code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw
    glUseProgram(shaderProgramObject);
    {
        glUniform3f(LdUniform, 1.0f, 1.0f, 1.0f);
        glUniform4f(lightPositionUniform, 0.0f, 1.0f, 0.0f, 1.0f);
        glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

        // push initial transformation
        stackMatModelView.push(vmath::mat4::identity() * vmath::translate(0.0f, 0.0f, -4.0f));
        {
            // push forearm transformation
            stackMatModelView.push(
                stackMatModelView.top() *
                vmath::translate(0.0f, 1.5f, 0.0f) *
                vmath::rotate(shoulder_yaw, 0.0f, 1.0f, 0.0f) *
                vmath::rotate(shoulder_pitch, 1.0f, 0.0f, 0.0f)
            );
            {
                glUniformMatrix4fv(modelViewMatrixUniform, 1, GL_FALSE, stackMatModelView.top() * vmath::scale(0.2f));
                glBindVertexArray(vaoSphere);
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
                    glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);
                
                glUniformMatrix4fv(modelViewMatrixUniform, 1, GL_FALSE, stackMatModelView.top());
                glBindVertexArray(vaoCuboids);
                    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
                    glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
                    glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
                    glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
                    glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
                    glDrawArrays(GL_TRIANGLE_FAN, 20, 4);
                glBindVertexArray(0);

                // push arm transformation
                stackMatModelView.push(
                    stackMatModelView.top() *
                    vmath::translate(0.0f, -1.5f, 0.0f) *
                    vmath::rotate(elbow_pitch, 0.0f, 1.0f, 0.0f) *
                    vmath::rotate(elbow_roll, 1.0f, 0.0f, 0.0f)
                );
                {
                    glUniformMatrix4fv(modelViewMatrixUniform, 1, GL_FALSE, stackMatModelView.top() * vmath::scale(0.2f));
                    glBindVertexArray(vaoSphere);
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
                    glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);
                
                    glUniformMatrix4fv(modelViewMatrixUniform, 1, GL_FALSE, stackMatModelView.top());
                    glBindVertexArray(vaoCuboids);
                        glDrawArrays(GL_TRIANGLE_FAN, 24, 4);
                        glDrawArrays(GL_TRIANGLE_FAN, 28, 4);
                        glDrawArrays(GL_TRIANGLE_FAN, 32, 4);
                        glDrawArrays(GL_TRIANGLE_FAN, 36, 4);
                        glDrawArrays(GL_TRIANGLE_FAN, 40, 4);
                        glDrawArrays(GL_TRIANGLE_FAN, 44, 4);
                    glBindVertexArray(0);

                    // push wrist transformation
                    stackMatModelView.push(
                        stackMatModelView.top() *
                        vmath::translate(0.0f, -1.0f, 0.0f) *
                        vmath::rotate(wrist_pitch, 0.0f, 1.0f, 0.0f) *
                        vmath::rotate(wrist_roll, 1.0f, 0.0f, 0.0f)
                    );
                    {
                        glUniformMatrix4fv(modelViewMatrixUniform, 1, GL_FALSE, stackMatModelView.top() * vmath::scale(0.2f));
                        glBindVertexArray(vaoSphere);
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
                        glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);
                
                        glUniformMatrix4fv(modelViewMatrixUniform, 1, GL_FALSE, stackMatModelView.top());
                        glBindVertexArray(vaoCuboids);
                            glDrawArrays(GL_TRIANGLE_FAN, 48, 4);
                            glDrawArrays(GL_TRIANGLE_FAN, 52, 4);
                            glDrawArrays(GL_TRIANGLE_FAN, 56, 4);
                            glDrawArrays(GL_TRIANGLE_FAN, 60, 4);
                            glDrawArrays(GL_TRIANGLE_FAN, 64, 4);
                            glDrawArrays(GL_TRIANGLE_FAN, 68, 4);
                        glBindVertexArray(0);

                        // push finger 'a' transformation
                        stackMatModelView.push(
                            stackMatModelView.top() *
                            vmath::translate(-0.08f + finger_a, -0.2f, 0.0f)
                        );
                        {
                            glUniformMatrix4fv(modelViewMatrixUniform, 1, GL_FALSE, stackMatModelView.top());
                            glBindVertexArray(vaoCuboids);
                                glDrawArrays(GL_TRIANGLE_FAN, 72, 4);
                                glDrawArrays(GL_TRIANGLE_FAN, 76, 4);
                                glDrawArrays(GL_TRIANGLE_FAN, 80, 4);
                                glDrawArrays(GL_TRIANGLE_FAN, 84, 4);
                                glDrawArrays(GL_TRIANGLE_FAN, 88, 4);
                                glDrawArrays(GL_TRIANGLE_FAN, 92, 4);
                            glBindVertexArray(0);
                        }
                        stackMatModelView.pop();  // pop finger 'a' transformation

                        // push finger 'b' transformation
                        stackMatModelView.push(
                            stackMatModelView.top() *
                            vmath::translate(0.08f - finger_b, -0.2f, 0.0f)
                        );
                        {                    
                            glUniformMatrix4fv(modelViewMatrixUniform, 1, GL_FALSE, stackMatModelView.top());
                            glBindVertexArray(vaoCuboids);
                                glDrawArrays(GL_TRIANGLE_FAN, 72, 4);
                                glDrawArrays(GL_TRIANGLE_FAN, 76, 4);
                                glDrawArrays(GL_TRIANGLE_FAN, 80, 4);
                                glDrawArrays(GL_TRIANGLE_FAN, 84, 4);
                                glDrawArrays(GL_TRIANGLE_FAN, 88, 4);
                                glDrawArrays(GL_TRIANGLE_FAN, 92, 4);
                            glBindVertexArray(0);
                        }
                        stackMatModelView.pop();  // pop finger 'b' transformation
                    }
                    stackMatModelView.pop();  // pop wrist transformation
                }
                stackMatModelView.pop();  // pop arm transformation
            }
            stackMatModelView.pop();  // pop forearm transformation
        }
        stackMatModelView.pop();  // pop initial transformation
    }
    glUseProgram(0);

    glXSwapBuffers(display, window);
}

void update(void)
{
    // code
}

void uninitialize(void)
{
    // local variables
    GLXContext currentContext;

    // code
    if (fullScreen)
    {
        toggleFullscreen();
    }

    if(vboCuboidNormals)
    {
        glDeleteBuffers(1, &vboCuboidNormals);
        vboCuboidNormals = 0;
    }

    if(vboCuboidPositions)
    {
        glDeleteBuffers(1, &vboCuboidPositions);
        vboCuboidPositions = 0;
    }

    if(vaoCuboids)
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
        fprintf(logFile, "detached and deleted %d shader objects\n", numAttachedShaders);

        glUseProgram(0);
        glDeleteProgram(shaderProgramObject);
        shaderProgramObject = 0;
        fprintf(logFile, "deleted shader program object\n");
    }

    currentContext = glXGetCurrentContext();
    if (currentContext && currentContext == glxContext)
    {
        glXMakeCurrent(display, 0, 0);
        currentContext = NULL;
    }

    if (glxContext)
    {
        glXDestroyContext(display, glxContext);
        glxContext = NULL;
    }

    if (visualInfo)
    {
        free(visualInfo);
        visualInfo = NULL;
    }

    if (window)
    {
        XDestroyWindow(display, window);
        window = 0;
    }

    if (colormap)
    {
        XFreeColormap(display, colormap);
        colormap = 0;
    }

    if (display)
    {
        XCloseDisplay(display);
        display = NULL;
    }

    if (logFile)
    {
        fprintf(logFile, "closing log file\n");
        fclose(logFile);
        logFile = NULL;
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

            for(int i = 0; i < count; i++)
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

            for(int i = 0; i < count; i++)
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
                    0.0f, -1.0f, 0.0f,
                    0.0f, -1.0f, 0.0f,
                    0.0f, -1.0f, 0.0f,
                    0.0f, -1.0f, 0.0f
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
