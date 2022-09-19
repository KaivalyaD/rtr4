/* standard headers */
#include <stdio.h>  // for standard I/O
#include <stdlib.h> // for exit()
#include <memory.h> // for memcpy()

/* X11 headers */
#include <X11/Xlib.h>
#include <X11/Xutil.h>  // for XVisualInfo
#include <X11/XKBlib.h> // for keyboard input

/* OpenGL headers */
#include <GL/gl.h>  // OpenGL
#include <GL/glu.h> // GLU
#include <GL/glx.h> // glX

/* macros */
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

/* global variables */
/* windowing */
Display *display = NULL;
XVisualInfo *visualInfo = NULL;
Colormap colormap;
Window window;
Bool fullScreen = False;
Bool activeWindow = False;

/* glX */
GLXContext glxContext = NULL;

/* global spotlight controls */
GLfloat spotDirection[3] = { 0.0f, 0.0f, -1.0f };
GLfloat spotExponent = 64.0f;
GLfloat spotCutoffAngle = 45.0f;
GLfloat spotAttenConstant = 1.0f;
GLfloat spotAttenLinear = 0.0f;
GLfloat spotAttenQuadratic = 0.0f;

/* for rendering */
GLUquadric *quadric;
Bool lightSwitch = False;

/* entry-point */
int main(int argc, char *argv)
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
    XSetWindowAttributes windowAttributes;
    int styleMask = 0;
    Atom wm_delete_window_atom;
    XEvent event;
    KeySym keysym;
    int screenWidth, screenHeight;
    int winWidth, winHeight;
    char keys[26];

    int framebufferAttributes[] = {
        GLX_DOUBLEBUFFER, True,
        GLX_RGBA,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 24,  // 32 is another option
        None
    };

    Bool done = False;
    int ret;

    // code
    // get connection to local XServer
    display = XOpenDisplay(NULL);
    if(!display)
    {
        fprintf(stderr, "error: XOpenDisplay() failed: check your network or the address passed to this API\n");
        uninitialize();
        exit(EXIT_FAILURE);
    }

    // get the default screen and depth of XServer
    defaultScreen = XDefaultScreen(display);
    defaultDepth = XDefaultDepth(display, defaultScreen);

    /* 
     * if a visual as described by `framebufferAttributes' is available on the system
     * whose connection is referenced by `display', fetch all its information
     */
    visualInfo = glXChooseVisual(display, defaultScreen, framebufferAttributes);
    if(!visualInfo)
    {
        fprintf(stderr, "error: glXChooseVisual() failed: (possibly) any of your displays don't have support for this application to run\n");
        uninitialize();
        exit(EXIT_FAILURE);
    }

    // set window attributes
    memset(&windowAttributes, 0, sizeof(XSetWindowAttributes));
    windowAttributes.border_pixel = 0;
    windowAttributes.background_pixel = XBlackPixel(display, visualInfo->screen);
    windowAttributes.background_pixmap = 0;
    windowAttributes.colormap = XCreateColormap(display, RootWindow(display, visualInfo->screen), visualInfo->visual, AllocNone);
    windowAttributes.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask | FocusChangeMask;
    
    colormap = windowAttributes.colormap;

    styleMask |= CWBorderPixel | CWBackPixel | CWColormap | CWEventMask;

    // create in-core window
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

    // give title to window
    XStoreName(display, window, "Kaivalya Vishwakumar Deshpande: OpenGL");

    // register on-close protocol into the window manager
    wm_delete_window_atom = XInternAtom(display, "WM_DELETE_ATOM", True);
    XSetWMProtocols(display, window, &wm_delete_window_atom, 1);

    // show the window
    XMapWindow(display, window);

    // center the window
    screenWidth = XWidthOfScreen(XScreenOfDisplay(display, visualInfo->screen));
    screenHeight = XHeightOfScreen(XScreenOfDisplay(display, visualInfo->screen));
    XMoveWindow(display, window, ((screenWidth - WIN_WIDTH) / 2), ((screenHeight - WIN_HEIGHT) / 2));

    // initialize OpenGL
    ret = initialize();
    switch(ret)
    {
    case -1:
        fprintf(stderr, "glXCreateContext(): failed\n");
        uninitialize();
        exit(EXIT_FAILURE);

    case -2:
        fprintf(stderr, "glXMakeCurrent(): failed\n");
        uninitialize();
        exit(EXIT_FAILURE);

    default:
        // announce successful initialization
        fprintf(stdout, "successfully initialized OpenGL\n");
        break;
    }

    // game loop
    while(!done)
    {
        /* check first for pending events; if any, handle them on priority */
        while(XPending(display))
        {
            XNextEvent(display, &event);
            switch(event.type)
            {
            case MapNotify:
                /* unhandled */
                break;

            case KeyPress:
                keysym = XkbKeycodeToKeysym(display, event.xkey.keycode, 0, 0);
                switch(keysym)
                {
                case XK_Escape:
                    /* close on esc */
                    done = True;
                    break;

                default:
                    /* unhandled */
                    break;
                }

                XLookupString(&event.xkey, keys, sizeof(keys) / sizeof(keys[0]), NULL, NULL);
                switch(*keys)
                {
                /**
                 * controls summary:
                 * 
                 * L/l toggles light
                 * A/D controls spot direction along X
                 * W/S controls spot direction along Y
                 * E/e controls spot exponent
                 * C/c controls spot cutoff
                 * 1/! controls constant attenuation
                 * 2/@ controls linear attenuation
                 * 3/# controls quadratic attenuation
                 * 
                 * F/f toggles fullscreen mode
                 */
                case 'A':
                case 'a':
                    spotDirection[0] -= 0.01f;
                    break;

                case 'D':
                case 'd':
                    spotDirection[0] += 0.01f;
                    break;

                case 'E':
                    spotExponent += 1.0f;
                    if(spotExponent > 128.0f)
                        spotExponent = 128.0f;
                    break;

                case 'e':
                    spotExponent -= 1.0f;
                    break;

                case 'F':
                case 'f':
                    /* fullscreen on 'F' or 'f' */
                    toggleFullscreen();
                    break;

                case 'L':
                case 'l':
                    if(!lightSwitch)
                    {
                        glEnable(GL_LIGHTING);
                        lightSwitch = True;
                    }
                    else
                    {
                        glDisable(GL_LIGHTING);
                        lightSwitch = False;
                    }
                    break;
                
                case 'S':
                case 's':
                    spotDirection[1] -= 0.01f;
                    break;

                case 'C':
                    spotCutoffAngle += 0.01f;
                    if(spotCutoffAngle > 90.0f)
                        spotCutoffAngle = 90.0f;
                    break;

                case 'c':
                    spotCutoffAngle -= 0.01f;
                    if(spotCutoffAngle < 0.0f)
                        spotCutoffAngle = 0.0f;
                    break;

                case 'W':
                case 'w':
                    spotDirection[1] += 0.01f;
                    break;

                case '1':
                    spotAttenConstant += 0.05f;
                    break;

                case '!':
                    spotAttenConstant -= 0.05f;
                    if(spotAttenConstant < 0.0f)
                        spotAttenConstant = 0.0f;
                    break;

                case '2':
                    spotAttenLinear += 0.05f;
                    break;

                case '@':
                    spotAttenLinear -= 0.05f;
                    if(spotAttenLinear < 0.0f)
                        spotAttenLinear = 0.0f;
                    break;

                case '3':
                    spotAttenQuadratic += 0.05f;
                    break;

                case '#':
                    spotAttenQuadratic -= 0.05f;
                    if(spotAttenQuadratic < 0.0f)
                        spotAttenQuadratic = 0.0f;
                    break;

                default:
                    /* unhandled */
                    break;
            }
            break;

            case FocusIn:
                activeWindow = True;
                fprintf(stdout, "rendering active: window is in focus\n");
                break;

            case FocusOut:
                activeWindow = False;
                fprintf(stdout, "rendering paused: window is out of focus\n");
                break;

            case ConfigureNotify:
                /* change viewport bounds on resize */
                winWidth = event.xconfigure.width;
                winHeight = event.xconfigure.height;
                resize(winWidth, winHeight);
                break;

            case 33:
                /* close on exit */
                done = True;
                break;

            default:
                /* unhandled */
                break;
            }
        }

        /* if here, there are no more pending events; if the window is active, render next scene */
        if(activeWindow)
        {
            // update the scene
            update();

            // render the scene
            render();
        }
    }
    
    // application can exit successfully; uninitialize
    uninitialize();

    // announce successful cleanup and exit
    fprintf(stdout, "cleaned everything up, exiting...\n");

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
    {
        fullScreen = False;
        fprintf(stdout, "fullscreen mode off\n");
    }
    else
    {
        fullScreen = True;
        fprintf(stdout, "fullscreen mode on\n");
    }

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
    void resize(int, int);

    // local variables
    /* lighting */
    GLfloat lightAmbient[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat lightDiffuse[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat lightSpecular[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat lightPosition[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

    GLfloat materialAmbient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat materialDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat materialSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat materialShininess = 128.0f;
    int ret;

    // code
    /*
     * create an unshared rendering context on `display' with
     * `visualInfo', capable of using hardware acceleration
     */
    glxContext = glXCreateContext(display, visualInfo, NULL, True);
    if(!glxContext)
        return -1;

    // make the context current
    ret = glXMakeCurrent(display, window, glxContext);
    if(!ret)
        return -2;

    // OpenGL has been initialized
    // settings for using depth
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    
    // setting up lights
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    glEnable(GL_LIGHT0);

    // setting up the material
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, materialShininess);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // initializing quadric
    quadric = gluNewQuadric();

    // warm-up resize
    resize(WIN_WIDTH, WIN_HEIGHT);

    return 0;
}

void resize(int width, int height)
{
    // local variables
    float aspectRatio;

    // code
    if(height == 0)
        height = 1;

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    aspectRatio = (float)width / (float)height;
    gluPerspective(
        45.0f,
        (GLfloat)aspectRatio,
        0.1f,
        100.0f
    );
}

void render(void)
{
    // code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPushMatrix();
        glTranslatef(-1.0f, 0.5f, -4.5f);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        gluCylinder(quadric, 0.0f, 0.5f, 1.0f, 100, 100);
    glPopMatrix();

    glPushMatrix();
        glTranslatef(1.0f, 1.0f, -4.5f);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        gluCylinder(quadric, 0.5f, 0.5f, 1.5f, 100, 100);
    glPopMatrix();

    glPushMatrix();
        glTranslatef(0.0f, -1.0f, -4.5f);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        gluSphere(quadric, 0.5f, 100, 100);
    glPopMatrix();

    glXSwapBuffers(display, window);
}

void update(void)
{
    // code
    // update light params
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spotDirection);
    glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, spotExponent);
    glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, spotCutoffAngle);
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, spotAttenConstant);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, spotAttenLinear);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, spotAttenQuadratic);
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

    if(quadric)
    {
        gluDeleteQuadric(quadric);
        quadric = NULL;
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
}
