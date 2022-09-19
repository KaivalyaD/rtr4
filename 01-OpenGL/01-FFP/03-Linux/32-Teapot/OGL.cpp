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

/* SOIL */
#include <SOIL/SOIL.h>

/* the Utah Teapot model */
#include "Teapot.h"

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

/* objects */
GLuint textureMarble;

// lighting
GLfloat lightAmbient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat lightPosition[] = { 0.0f, 0.0f, 2.0f, 1.0f };

Bool lightSwitch = False;

// texture
Bool textureSwitch = False;

// animation
float theta;

/* entry-point */
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
        GLX_DEPTH_SIZE, 24,
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

    case -3:
        fprintf(stderr, "loadGLTexture: failed for textureMarble\n");
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
                    if (!lightSwitch)
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

                case 'T':
                case 't':
                    if (!textureSwitch)
                    {
                        glEnable(GL_TEXTURE_2D);
                        textureSwitch = True;
                    }
                    else
                    {
                        glDisable(GL_TEXTURE_2D);
                        textureSwitch = False;
                    }
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
    Bool loadGLTexture(GLuint *, const char *);
    void uninitialize(void);

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

    // load textures
    if(!loadGLTexture(&textureMarble, "./resources/Marble.png"))
        return -3;
    
    glEnable(GL_TEXTURE_2D);

    // OpenGL has been initialized
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // depth-related changes
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // changes concerning lighting
    glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT1, GL_POSITION, lightPosition);
    glEnable(GL_LIGHT1);

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
    // variable declarations
    int vertIdx, normIdx, texCoordIdx;

    // code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(0.0f, 0.0f, -1.2f);
	glRotatef(theta, 0.0f, 1.0f, 0.0f);

    if (textureSwitch)
        glBindTexture(GL_TEXTURE_2D, textureMarble);
    else
        glBindTexture(GL_TEXTURE_2D, 0);

    glBegin(GL_TRIANGLES);
    {
        for (int i = 0; i < sizeof(face_indicies) / sizeof(face_indicies[0]); i++)
        {
            for (int j = 0; j < 3; j++)
            {
                vertIdx = face_indicies[i][j];
                normIdx = face_indicies[i][j + 3];
                texCoordIdx = face_indicies[i][j + 6];

                glNormal3f(normals[normIdx][0], normals[normIdx][1], normals[normIdx][2]);
                glTexCoord2f(textures[texCoordIdx][0], textures[texCoordIdx][1]);
                glVertex3f(vertices[vertIdx][0], vertices[vertIdx][1], vertices[vertIdx][2]);
            }
        }
    }
    glEnd();

    glXSwapBuffers(display, window);
}

void update(void)
{
    // code
    theta = theta + 1.0f;
	if (theta > 360.0f)
		theta = theta - 360.0f;
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

    if(textureMarble)
    {
        glDeleteTextures(1, &textureMarble);
        textureMarble = 0U;
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

Bool loadGLTexture(GLuint *texture, const char *filename)
{
    // variable declarations
    int imgWidth = 0, imgHeight = 0;
    unsigned char *imgData = NULL;

    // code
    imgData = SOIL_load_image(
        filename,
        &imgWidth,
        &imgHeight,
        NULL,
        SOIL_LOAD_RGBA
    );
    if(!imgData)
        return False;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        gluBuild2DMipmaps(
            GL_TEXTURE_2D,
            GL_RGBA,
            imgWidth,
            imgHeight,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            imgData
        );
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    SOIL_free_image_data(imgData);
    imgData = NULL;

    return True;
}
