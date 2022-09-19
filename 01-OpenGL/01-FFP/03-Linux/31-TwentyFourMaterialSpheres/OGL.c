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

// math headers
#include<math.h>

/* macros */
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#define DEG_TO_RAD(x) (((x) * M_PI) / (180.0f))

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

/* for rendering */
Bool gbLightSwitch = False;

GLfloat lightAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat lightPosition[] = { 0.0f, 0.0f, 0.0f, 1.0f };

GLfloat light_model_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
GLfloat light_model_local_viewer[] = { 0.0f };

GLfloat xAngle = 0.0f;
GLfloat yAngle = 0.0f;
GLfloat zAngle = 0.0f;

GLint gKeyPressed = 0;

GLUquadric *quadric = NULL;

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
                case 'F':
                case 'f':
                    /* fullscreen on 'F' or 'f' */
                    toggleFullscreen();
                    break;

                case 'L':
                case 'l':
                    if (!gbLightSwitch)
                    {
                        glEnable(GL_LIGHTING);
                        gbLightSwitch = True;
                    }
                    else
                    {
                        glDisable(GL_LIGHTING);
                        gbLightSwitch = False;
                    }
                    break;

                case 'X':
                case 'x':
                    gKeyPressed = 1;
                    xAngle = 0.0f;	// reset x rotation
                    break;

                case 'Y':
                case 'y':
                    gKeyPressed = 2;
                    yAngle = 0.0f;	// reset y rotation
                    break;

                case 'Z':
                case'z':
                    gKeyPressed = 3;
                    zAngle = 0.0f;	// reset z rotation
                    break;

                default:
                    gKeyPressed = 0;
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
    
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

    // lighting
    glEnable(GL_AUTO_NORMAL);  // generate automatic normals for all geometery
    glEnable(GL_NORMALIZE);  // ensure normals stay of unit length even on transformations

    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_model_ambient);
    glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, light_model_local_viewer);
    glEnable(GL_LIGHT0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // drawing
    quadric = gluNewQuadric();

    // warm-up resize
    resize(WIN_WIDTH, WIN_HEIGHT);

    return 0;
}

void resize(int width, int height)
{
    // variable declarations
    float aspectRatio;

    // code
    if(height == 0)
        height = 1;

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    aspectRatio = (GLfloat)width / (GLfloat)height;
	gluPerspective(25.0f, aspectRatio, 0.1f, 100.0f);
}

void render(void)
{
    // function prototypes
    void draw24Spheres(void);

    // code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(
        3.0f, 2.0f, 35.0f,
        3.0f, 2.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    );

    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    draw24Spheres();

    glXSwapBuffers(display, window);
}

void update(void)
{
    // code
    if (gKeyPressed == 1)
    {
        xAngle = xAngle + 1.0f;
        if (xAngle >= 360.0f)
            xAngle = xAngle - 360.0f;

        lightPosition[0] = 0.0f;
        lightPosition[1] = 50.0f * sinf(DEG_TO_RAD(xAngle));
        lightPosition[2] = 50.0f * cosf(DEG_TO_RAD(xAngle));
        lightPosition[3] = 1.0f;
    }
    else if (gKeyPressed == 2)
    {
        yAngle = yAngle + 1.0f;
        if (yAngle >= 360.0f)
            yAngle = yAngle - 360.0f;

        lightPosition[0] = 50.0f * cosf(DEG_TO_RAD(yAngle));
        lightPosition[1] = 0.0f;
        lightPosition[2] = 50.0f * sinf(DEG_TO_RAD(yAngle));
        lightPosition[3] = 1.0f;
    }
    else if (gKeyPressed == 3)
    {
        zAngle = zAngle + 1.0f;
        if (zAngle >= 360.0f)
            zAngle = zAngle - 360.0f;

        lightPosition[0] = 50.0f * cosf(DEG_TO_RAD(zAngle));
        lightPosition[1] = 50.0f * sinf(DEG_TO_RAD(zAngle));
        lightPosition[2] = 0.0f;
        lightPosition[3] = 1.0f;
    }
    else
    {
        xAngle = DEG_TO_RAD(0.0f);
        yAngle = DEG_TO_RAD(0.0f);
        zAngle = DEG_TO_RAD(0.0f);

        lightPosition[0] = 3.0f;
        lightPosition[1] = 2.0f;
        lightPosition[2] = 50.0f;
        lightPosition[0] = 1.0f;
    }
}

void draw24Spheres(void)
{
    // variable declarations
    GLfloat materialAmbient[4];
    GLfloat materialDiffuse[4];
    GLfloat materialSpecular[4];
    GLfloat materialShininess;

    // code
    // ***** 1st sphere on 1st column (gems), emerald *****
    // ambient material
    materialAmbient[0] = 0.0215; // r
    materialAmbient[1] = 0.1745; // g
    materialAmbient[2] = 0.0215; // b
    materialAmbient[3] = 1.0f;   // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.07568; // r
    materialDiffuse[1] = 0.61424; // g
    materialDiffuse[2] = 0.07568; // b
    materialDiffuse[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.633;    // r
    materialSpecular[1] = 0.727811; // g
    materialSpecular[2] = 0.633;    // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.6 * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glTranslatef(-4.5f, 8.25f, 0.0f);
    gluSphere(quadric, 1.0f, 30, 30);
    // *******************************************************

    // ***** 2nd sphere on 1st column, jade *****
    // ambient material
    materialAmbient[0] = 0.135;  // r
    materialAmbient[1] = 0.2225; // g
    materialAmbient[2] = 0.1575; // b
    materialAmbient[3] = 1.0f;   // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.54; // r
    materialDiffuse[1] = 0.89; // g
    materialDiffuse[2] = 0.63; // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.316228; // r
    materialSpecular[1] = 0.316228; // g
    materialSpecular[2] = 0.316228; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.1 * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glTranslatef(0.0f, -2.5f, 0.0f);
    gluSphere(quadric, 1.0f, 30, 30);
    // *******************************************************

    // ***** 3rd sphere on 1st column, obsidian *****
    // ambient material
    materialAmbient[0] = 0.05375; // r
    materialAmbient[1] = 0.05;    // g
    materialAmbient[2] = 0.06625; // b
    materialAmbient[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.18275; // r
    materialDiffuse[1] = 0.17;    // g
    materialDiffuse[2] = 0.22525; // b
    materialDiffuse[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.332741; // r
    materialSpecular[1] = 0.328634; // g
    materialSpecular[2] = 0.346435; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.3 * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glTranslatef(0.0f, -2.5f, 0.0f);
    gluSphere(quadric, 1.0f, 30, 30);
    // *******************************************************

    // ***** 4th sphere on 1st column, pearl *****
    // ambient material
    materialAmbient[0] = 0.25;    // r
    materialAmbient[1] = 0.20725; // g
    materialAmbient[2] = 0.20725; // b
    materialAmbient[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 1.0;   // r
    materialDiffuse[1] = 0.829; // g
    materialDiffuse[2] = 0.829; // b
    materialDiffuse[3] = 1.0f;  // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.296648; // r
    materialSpecular[1] = 0.296648; // g
    materialSpecular[2] = 0.296648; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.088 * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glTranslatef(0.0f, -2.5f, 0.0f);
    gluSphere(quadric, 1.0f, 30, 30);
    // *******************************************************

    // ***** 5th sphere on 1st column, ruby *****
    // ambient material
    materialAmbient[0] = 0.1745;  // r
    materialAmbient[1] = 0.01175; // g
    materialAmbient[2] = 0.01175; // b
    materialAmbient[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.61424; // r
    materialDiffuse[1] = 0.04136; // g
    materialDiffuse[2] = 0.04136; // b
    materialDiffuse[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.727811; // r
    materialSpecular[1] = 0.626959; // g
    materialSpecular[2] = 0.626959; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.6 * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glTranslatef(0.0f, -2.5f, 0.0f);
    gluSphere(quadric, 1.0f, 30, 30);
    // *******************************************************

    // ***** 6th sphere on 1st column, turquoise *****
    // ambient material
    materialAmbient[0] = 0.1;     // r
    materialAmbient[1] = 0.18725; // g
    materialAmbient[2] = 0.1745;  // b
    materialAmbient[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.396;   // r
    materialDiffuse[1] = 0.74151; // g
    materialDiffuse[2] = 0.69102; // b
    materialDiffuse[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.297254; // r
    materialSpecular[1] = 0.30829;  // g
    materialSpecular[2] = 0.306678; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.1 * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glTranslatef(0.0f, -2.5f, 0.0f);
    gluSphere(quadric, 1.0f, 30, 30);
    // *******************************************************
    // *******************************************************
    // *******************************************************

    // ***** 1st sphere on 2nd column (metals), brass *****
    // ambient material
    materialAmbient[0] = 0.329412; // r
    materialAmbient[1] = 0.223529; // g
    materialAmbient[2] = 0.027451; // b
    materialAmbient[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.780392; // r
    materialDiffuse[1] = 0.568627; // g
    materialDiffuse[2] = 0.113725; // b
    materialDiffuse[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.992157; // r
    materialSpecular[1] = 0.941176; // g
    materialSpecular[2] = 0.807843; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.21794872 * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glTranslatef(5.0f, 12.5f, 0.0f);
    gluSphere(quadric, 1.0f, 30, 30);
    // *******************************************************

    // ***** 2nd sphere on 2nd column, bronze *****
    // ambient material
    materialAmbient[0] = 0.2125; // r
    materialAmbient[1] = 0.1275; // g
    materialAmbient[2] = 0.054;  // b
    materialAmbient[3] = 1.0f;   // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.714;   // r
    materialDiffuse[1] = 0.4284;  // g
    materialDiffuse[2] = 0.18144; // b
    materialDiffuse[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.393548; // r
    materialSpecular[1] = 0.271906; // g
    materialSpecular[2] = 0.166721; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.2 * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glTranslatef(0.0f, -2.5f, 0.0f);
    gluSphere(quadric, 1.0f, 30, 30);
    // *******************************************************

    // ***** 3rd sphere on 2nd column, chrome *****
    // ambient material
    materialAmbient[0] = 0.25; // r
    materialAmbient[1] = 0.25; // g
    materialAmbient[2] = 0.25; // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.4;  // r
    materialDiffuse[1] = 0.4;  // g
    materialDiffuse[2] = 0.4;  // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.774597; // r
    materialSpecular[1] = 0.774597; // g
    materialSpecular[2] = 0.774597; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.6 * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glTranslatef(0.0f, -2.5f, 0.0f);
    gluSphere(quadric, 1.0f, 30, 30);
    // *******************************************************

    // ***** 4th sphere on 2nd column, copper *****
    // ambient material
    materialAmbient[0] = 0.19125; // r
    materialAmbient[1] = 0.0735;  // g
    materialAmbient[2] = 0.0225;  // b
    materialAmbient[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.7038;  // r
    materialDiffuse[1] = 0.27048; // g
    materialDiffuse[2] = 0.0828;  // b
    materialDiffuse[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.256777; // r
    materialSpecular[1] = 0.137622; // g
    materialSpecular[2] = 0.086014; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.1 * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glTranslatef(0.0f, -2.5f, 0.0f);
    gluSphere(quadric, 1.0f, 30, 30);
    // *******************************************************

    // ***** 5th sphere on 2nd column, gold *****
    // ambient material
    materialAmbient[0] = 0.24725; // r
    materialAmbient[1] = 0.1995;  // g
    materialAmbient[2] = 0.0745;  // b
    materialAmbient[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.75164; // r
    materialDiffuse[1] = 0.60648; // g
    materialDiffuse[2] = 0.22648; // b
    materialDiffuse[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.628281; // r
    materialSpecular[1] = 0.555802; // g
    materialSpecular[2] = 0.366065; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.4 * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glTranslatef(0.0f, -2.5f, 0.0f);
    gluSphere(quadric, 1.0f, 30, 30);
    // *******************************************************

    // ***** 6th sphere on 2nd column, silver *****
    // ambient material
    materialAmbient[0] = 0.19225; // r
    materialAmbient[1] = 0.19225; // g
    materialAmbient[2] = 0.19225; // b
    materialAmbient[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.50754; // r
    materialDiffuse[1] = 0.50754; // g
    materialDiffuse[2] = 0.50754; // b
    materialDiffuse[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.508273; // r
    materialSpecular[1] = 0.508273; // g
    materialSpecular[2] = 0.508273; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.4 * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glTranslatef(0.0f, -2.5f, 0.0f);
    gluSphere(quadric, 1.0f, 30, 30);
    // *******************************************************
    // *******************************************************
    // *******************************************************

    // ***** 1st sphere on 3rd column (plastic), black *****
    // ambient material
    materialAmbient[0] = 0.0;  // r
    materialAmbient[1] = 0.0;  // g
    materialAmbient[2] = 0.0;  // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.01; // r
    materialDiffuse[1] = 0.01; // g
    materialDiffuse[2] = 0.01; // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.50; // r
    materialSpecular[1] = 0.50; // g
    materialSpecular[2] = 0.50; // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.25 * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glTranslatef(5.0f, 12.5f, 0.0f);
    gluSphere(quadric, 1.0f, 30, 30);
    // *******************************************************

    // ***** 2nd sphere on 3rd column, cyan *****
    // ambient material
    materialAmbient[0] = 0.0;  // r
    materialAmbient[1] = 0.1;  // g
    materialAmbient[2] = 0.06; // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.0;        // r
    materialDiffuse[1] = 0.50980392; // g
    materialDiffuse[2] = 0.50980392; // b
    materialDiffuse[3] = 1.0f;       // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.50196078; // r
    materialSpecular[1] = 0.50196078; // g
    materialSpecular[2] = 0.50196078; // b
    materialSpecular[3] = 1.0f;       // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.25 * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glTranslatef(0.0f, -2.5f, 0.0f);
    gluSphere(quadric, 1.0f, 30, 30);
    // *******************************************************

    // ***** 3rd sphere on 3rd column, green *****
    // ambient material
    materialAmbient[0] = 0.0;  // r
    materialAmbient[1] = 0.0;  // g
    materialAmbient[2] = 0.0;  // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.1;  // r
    materialDiffuse[1] = 0.35; // g
    materialDiffuse[2] = 0.1;  // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.45; // r
    materialSpecular[1] = 0.55; // g
    materialSpecular[2] = 0.45; // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.25 * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glTranslatef(0.0f, -2.5f, 0.0f);
    gluSphere(quadric, 1.0f, 30, 30);
    // *******************************************************

    // ***** 4th sphere on 3rd column, red *****
    // ambient material
    materialAmbient[0] = 0.0;  // r
    materialAmbient[1] = 0.0;  // g
    materialAmbient[2] = 0.0;  // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.5;  // r
    materialDiffuse[1] = 0.0;  // g
    materialDiffuse[2] = 0.0;  // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.7;  // r
    materialSpecular[1] = 0.6;  // g
    materialSpecular[2] = 0.6;  // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.25 * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glTranslatef(0.0f, -2.5f, 0.0f);
    gluSphere(quadric, 1.0f, 30, 30);
    // *******************************************************

    // ***** 5th sphere on 3rd column, white *****
    // ambient material
    materialAmbient[0] = 0.0;  // r
    materialAmbient[1] = 0.0;  // g
    materialAmbient[2] = 0.0;  // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.55; // r
    materialDiffuse[1] = 0.55; // g
    materialDiffuse[2] = 0.55; // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.70; // r
    materialSpecular[1] = 0.70; // g
    materialSpecular[2] = 0.70; // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.25 * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glTranslatef(0.0f, -2.5f, 0.0f);
    gluSphere(quadric, 1.0f, 30, 30);
    // *******************************************************

    // ***** 6th sphere on 3rd column, yellow *****
    // ambient material
    materialAmbient[0] = 0.0;  // r
    materialAmbient[1] = 0.0;  // g
    materialAmbient[2] = 0.0;  // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.5;  // r
    materialDiffuse[1] = 0.5;  // g
    materialDiffuse[2] = 0.0;  // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.60; // r
    materialSpecular[1] = 0.60; // g
    materialSpecular[2] = 0.50; // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.25 * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glTranslatef(0.0f, -2.5f, 0.0f);
    gluSphere(quadric, 1.0f, 30, 30);
    // *******************************************************
    // *******************************************************
    // *******************************************************

    // ***** 1st sphere on 4th column (rubber), black *****
    // ambient material
    materialAmbient[0] = 0.02; // r
    materialAmbient[1] = 0.02; // g
    materialAmbient[2] = 0.02; // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.01; // r
    materialDiffuse[1] = 0.01; // g
    materialDiffuse[2] = 0.01; // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.4;  // r
    materialSpecular[1] = 0.4;  // g
    materialSpecular[2] = 0.4;  // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.078125 * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glTranslatef(5.0f, 12.5f, 0.0f);
    gluSphere(quadric, 1.0f, 30, 30);
    // *******************************************************

    // ***** 2nd sphere on 4th column, cyan *****
    // ambient material
    materialAmbient[0] = 0.0;  // r
    materialAmbient[1] = 0.05; // g
    materialAmbient[2] = 0.05; // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.4;  // r
    materialDiffuse[1] = 0.5;  // g
    materialDiffuse[2] = 0.5;  // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.04; // r
    materialSpecular[1] = 0.7;  // g
    materialSpecular[2] = 0.7;  // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.078125 * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glTranslatef(0.0f, -2.5f, 0.0f);
    gluSphere(quadric, 1.0f, 30, 30);
    // *******************************************************

    // ***** 3rd sphere on 4th column, green *****
    // ambient material
    materialAmbient[0] = 0.0;  // r
    materialAmbient[1] = 0.05; // g
    materialAmbient[2] = 0.0;  // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.4;  // r
    materialDiffuse[1] = 0.5;  // g
    materialDiffuse[2] = 0.4;  // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.04; // r
    materialSpecular[1] = 0.7;  // g
    materialSpecular[2] = 0.04; // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.078125 * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glTranslatef(0.0f, -2.5f, 0.0f);
    gluSphere(quadric, 1.0f, 30, 30);
    // *******************************************************

    // ***** 4th sphere on 4th column, red *****
    // ambient material
    materialAmbient[0] = 0.05; // r
    materialAmbient[1] = 0.0;  // g
    materialAmbient[2] = 0.0;  // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.5;  // r
    materialDiffuse[1] = 0.4;  // g
    materialDiffuse[2] = 0.4;  // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.7;  // r
    materialSpecular[1] = 0.04; // g
    materialSpecular[2] = 0.04; // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.078125 * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glTranslatef(0.0f, -2.5f, 0.0f);
    gluSphere(quadric, 1.0f, 30, 30);
    // *******************************************************

    // ***** 5th sphere on 4th column, white *****
    // ambient material
    materialAmbient[0] = 0.05; // r
    materialAmbient[1] = 0.05; // g
    materialAmbient[2] = 0.05; // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.5;  // r
    materialDiffuse[1] = 0.5;  // g
    materialDiffuse[2] = 0.5;  // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.7;  // r
    materialSpecular[1] = 0.7;  // g
    materialSpecular[2] = 0.7;  // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.078125 * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glTranslatef(0.0f, -2.5f, 0.0f);
    gluSphere(quadric, 1.0f, 30, 30);
    // *******************************************************

    // ***** 6th sphere on 4th column, yellow *****
    // ambient material
    materialAmbient[0] = 0.05; // r
    materialAmbient[1] = 0.05; // g
    materialAmbient[2] = 0.0;  // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.5;  // r
    materialDiffuse[1] = 0.5;  // g
    materialDiffuse[2] = 0.4;  // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.7;  // r
    materialSpecular[1] = 0.7;  // g
    materialSpecular[2] = 0.04; // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.078125 * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glTranslatef(0.0f, -2.5f, 0.0f);
    gluSphere(quadric, 1.0f, 30, 30);
    // *******************************************************
    // *******************************************************
    // *******************************************************
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
