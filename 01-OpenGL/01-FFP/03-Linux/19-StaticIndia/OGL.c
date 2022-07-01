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

/* for rendering */
GLclampf fadeFactorD = 1.0f;

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
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

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
    // function prototypes
	void drawLetterI(void);
	void drawLetterN(void);
	void drawLetterD(void);
	void drawLetterA(void);

	// code
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glTranslatef(-0.5f, -0.225f, -2.0f);
	
	drawLetterI();

	glTranslatef(0.25f, 0.0f, 0.0f);
	drawLetterN();
	
	glTranslatef(0.25f, 0.0f, 0.0f);
	drawLetterD();
	
	glTranslatef(0.20f, 0.0f, 0.0f);
	drawLetterI();
	
	glTranslatef(0.30f, 0.0f, 0.0f);
	drawLetterA();

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

void drawLetterI(void)
{
	// local variable declarations
	GLfloat xpos = -0.1f;
	GLfloat ypos = 0.4f;

	GLfloat xlimit = 0.2f;
	GLfloat ylimit = 0.4f;

	// code
	glBegin(GL_QUADS);
	{
		// first vertical
		glColor3f(1.0f, 0.6f, 0.2f);
		glVertex3f(xpos + xlimit / 2.0f + 0.0125f, ypos - 0.025f, 0.0f);
		glVertex3f(xpos + xlimit / 2.0f - 0.0125f, ypos - 0.025f, 0.0f);
		
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(xpos + xlimit / 2.0f - 0.0125f, ypos - ylimit / 2.0f, 0.0f);
		glVertex3f(xpos + xlimit / 2.0f + 0.0125f, ypos - ylimit / 2.0f, 0.0f);
		glVertex3f(xpos + xlimit / 2.0f + 0.0125f, ypos - ylimit / 2.0f, 0.0f);
		glVertex3f(xpos + xlimit / 2.0f - 0.0125f, ypos - ylimit / 2.0f, 0.0f);

		glColor3f(0.074f, 0.534f, 0.0313f);
		glVertex3f(xpos + xlimit / 2.0f - 0.0125f, ypos - ylimit + 0.025f, 0.0f);
		glVertex3f(xpos + xlimit / 2.0f + 0.0125f, ypos - ylimit + 0.025f, 0.0f);

		// first horizontal
		glColor3f(1.0f, 0.6f, 0.2f);
		glVertex3f(xpos + xlimit, ypos, 0.0f);
		glVertex3f(xpos, ypos, 0.0f);
		glVertex3f(xpos, ypos - 0.025f, 0.0f);
		glVertex3f(xpos + xlimit, ypos - 0.025f, 0.0f);

		// second horizontal
		glColor3f(0.074f, 0.534f, 0.0313f);
		glVertex3f(xpos + xlimit, ypos - ylimit + 0.025f, 0.0f);
		glVertex3f(xpos, ypos - ylimit + 0.025f, 0.0f);
		glVertex3f(xpos, ypos - ylimit, 0.0f);
		glVertex3f(xpos + xlimit, ypos - ylimit, 0.0f);
	}
	glEnd();
}

void drawLetterN(void)
{
	// local variable declarations
	GLfloat xpos = -0.1f;
	GLfloat ypos = 0.4f;

	GLfloat v_lineThickness = 0.05f;
	GLfloat v_lineHeight = 0.4f;
	GLfloat h_lineThickness = 0.2f;
	GLfloat h_lineHeight = 0.05f;

	// code
	glBegin(GL_QUADS);
	{
		// first vertical
		glColor3f(1.0f, 0.6f, 0.2f);
		glVertex3f(xpos, ypos, 0.0f);

		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(xpos, ypos - v_lineHeight / 2.0f, 0.0f);
		glVertex3f(xpos + v_lineThickness, ypos - v_lineHeight / 2.0f, 0.0f);
		
		glColor3f(1.0f, 0.6f, 0.2f);
		glVertex3f(xpos + v_lineThickness, ypos, 0.0f);

		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(xpos, ypos - v_lineHeight / 2.0f, 0.0f);

		glColor3f(0.074f, 0.534f, 0.0313f);
		glVertex3f(xpos, ypos - v_lineHeight, 0.0f);
		glVertex3f(xpos + v_lineThickness, ypos - v_lineHeight, 0.0f);

		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(xpos + v_lineThickness, ypos - v_lineHeight / 2.0f, 0.0f);

		// first horizontal
		glColor3f(1.0f, 0.6f, 0.2f);
		glVertex3f(xpos + v_lineThickness, ypos, 0.0f);
		glVertex3f(xpos, ypos, 0.0f);

		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(xpos + h_lineThickness / 1.6f - v_lineThickness, ypos - v_lineHeight / 2.0f, 0.0f);
		glVertex3f(xpos + h_lineThickness / 1.6f, ypos - v_lineHeight / 2.0f, 0.0f);
		glVertex3f(xpos + h_lineThickness / 1.6f - v_lineThickness, ypos - v_lineHeight / 2.0f, 0.0f);

		glColor3f(0.074f, 0.534f, 0.0313f);
		glVertex3f(xpos + h_lineThickness - v_lineThickness, ypos - v_lineHeight, 0.0f);
		glVertex3f(xpos + h_lineThickness, ypos - v_lineHeight, 0.0f);

		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(xpos + h_lineThickness / 1.6f, ypos - v_lineHeight / 2.0f, 0.0f);
		
		// second vertical
		glColor3f(1.0f, 0.6f, 0.2f);
		glVertex3f(xpos + h_lineThickness, ypos, 0.0f);
		glVertex3f(xpos + h_lineThickness - v_lineThickness, ypos, 0.0f);

		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(xpos + h_lineThickness - v_lineThickness, ypos - v_lineHeight / 2.0f, 0.0f);
		glVertex3f(xpos + h_lineThickness, ypos - v_lineHeight / 2.0f, 0.0f);
		glVertex3f(xpos + h_lineThickness, ypos - v_lineHeight / 2.0f, 0.0f);
		glVertex3f(xpos + h_lineThickness - v_lineThickness, ypos - v_lineHeight / 2.0f, 0.0f);

		glColor3f(0.074f, 0.534f, 0.0313f);
		glVertex3f(xpos + h_lineThickness - v_lineThickness, ypos - v_lineHeight, 0.0f);
		glVertex3f(xpos + h_lineThickness, ypos - v_lineHeight, 0.0f);
	}
	glEnd();
}

void drawLetterD()
{
	// local variable declarations
	GLfloat xpos = -0.1f;
	GLfloat ypos = 0.4f;

	GLfloat xlimit = 0.2f;
	GLfloat ylimit = 0.4f;

	// code
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_QUADS);
	{
		// first vertical
		glColor3f(fadeFactorD * 1.0f, fadeFactorD * 0.6f, fadeFactorD * 0.2f);
		glVertex3f(xpos, ypos - 0.025f, 0.0f);

		glColor3f(fadeFactorD * 1.0f, fadeFactorD * 1.0f, fadeFactorD * 1.0f);
		glVertex3f(xpos, ypos - ylimit / 2.0f, 0.0f);
		glVertex3f(xpos + 0.025f, ypos - ylimit / 2.0f, 0.0f);

		glColor3f(fadeFactorD * 1.0f, fadeFactorD * 0.6f, fadeFactorD * 0.2f);
		glVertex3f(xpos + 0.025f, ypos - 0.025f, 0.0f);

		glColor3f(fadeFactorD * 1.0f, fadeFactorD * 1.0f, fadeFactorD * 1.0f);
		glVertex3f(xpos + 0.025f, ypos - ylimit / 2.0f, 0.0f);
		glVertex3f(xpos, ypos - ylimit / 2.0f, 0.0f);

		glColor3f(fadeFactorD * 0.074f, fadeFactorD * 0.534f, fadeFactorD * 0.0313f);
		glVertex3f(xpos, ypos - ylimit + 0.025f, 0.0f);
		glVertex3f(xpos + 0.025f, ypos - ylimit + 0.025f, 0.0f);

		// first horizontal
		glColor3f(fadeFactorD * 1.0f, fadeFactorD * 0.6f, fadeFactorD * 0.2f);
		glVertex3f(xpos + (xlimit / 2.0f) + 0.025, ypos, 0.0f);
		glVertex3f(xpos, ypos, 0.0f);
		glVertex3f(xpos, ypos - 0.025f, 0.0f);
		glVertex3f(xpos + (xlimit / 2.0f) + 0.025f, ypos - 0.025f, 0.0f);

		// second vertical
		glColor3f(fadeFactorD * 1.0f, fadeFactorD * 0.8f, fadeFactorD * 0.6f);
		glVertex3f(xpos + xlimit, ypos - ylimit / 4.0f, 0.0f);
		glVertex3f(xpos + xlimit - 0.025f, ypos - ylimit / 4.0f, 0.0f);

		glColor3f(fadeFactorD * 1.0f, fadeFactorD * 1.0f, fadeFactorD * 1.0f);
		glVertex3f(xpos + xlimit - 0.025f, ypos - (3.0f * ylimit / 10.0f) / 1.6f - 0.125f, 0.0f);
		glVertex3f(xpos + xlimit, ypos - (3.0f * ylimit / 4.0f) / 1.6f - 0.0125f, 0.0f);
		glVertex3f(xpos + xlimit, ypos - (3.0f * ylimit / 4.0f) / 1.6f - 0.0125f, 0.0f);
		glVertex3f(xpos + xlimit - 0.025f, ypos - (3.0f * ylimit / 4.0f) / 1.6f - 0.0125f, 0.0f);

		glColor3f(fadeFactorD * 0.74f, fadeFactorD * 0.834f, fadeFactorD * 0.633f);
		glVertex3f(xpos + xlimit - 0.025f, ypos - (3.0f * ylimit / 4.0f) - 0.0125f, 0.0f);
		glVertex3f(xpos + xlimit, ypos - (3.0f * ylimit / 4.0f) - 0.0125f, 0.0f);

		// second horizontal
		glColor3f(fadeFactorD * 0.074f, fadeFactorD * 0.534f, fadeFactorD * 0.0313f);
		glVertex3f(xpos + xlimit / 2.0f + 0.025, ypos - ylimit + 0.025f, 0.0f);
		glVertex3f(xpos, ypos - ylimit + 0.025f, 0.0f);
		glVertex3f(xpos, ypos - ylimit, 0.0f);
		glVertex3f(xpos + xlimit / 2.0f + 0.025, ypos - ylimit, 0.0f);
	}
	glEnd();

	glBegin(GL_POLYGON);
	{
		// first forward slant
		glColor3f(fadeFactorD * 1.0f, fadeFactorD * 0.6f, fadeFactorD * 0.2f);
		glVertex3f(xpos + xlimit / 2.0f + 0.045f, ypos, 0.0f);
		glVertex3f(xpos + xlimit / 2.0f + 0.025f, ypos, 0.0f);
		glVertex3f(xpos + xlimit / 2.0f + 0.025f, ypos - 0.025f, 0.0f);

		glColor3f(fadeFactorD * 1.0f, fadeFactorD * 0.8f, fadeFactorD * 0.6f);
		glVertex3f(xpos + xlimit - 0.025f, ypos - ylimit / 4.0f, 0.0f);
		glVertex3f(xpos + xlimit, ypos - ylimit / 4.0f, 0.0f);
		glVertex3f(xpos + xlimit, ypos - ylimit / 4.0f + 0.02f, 0.0f);
	}
	glEnd();

	glBegin(GL_POLYGON);
	{
		// first backward slant
		glColor3f(fadeFactorD * 0.74f, fadeFactorD * 0.834f, fadeFactorD * 0.633f);
		glVertex3f(xpos + xlimit, ypos - (3.0f * ylimit / 4.0f) - 0.0125f, 0.0f);
		glVertex3f(xpos + xlimit - 0.025f, ypos - (3.0f * ylimit / 4.0f) - 0.0125f, 0.0f);

		glColor3f(fadeFactorD * 0.074f, fadeFactorD * 0.534f, fadeFactorD * 0.0313f);
		glVertex3f(xpos + xlimit / 2.0f + 0.025f, ypos - ylimit + 0.025f, 0.0f);
		glVertex3f(xpos + xlimit / 2.0f + 0.025f, ypos - ylimit, 0.0f);
		glVertex3f(xpos + xlimit / 2.0f + 0.045f, ypos - ylimit, 0.0f);

		glColor3f(fadeFactorD * 0.74f, fadeFactorD * 0.834f, fadeFactorD * 0.633f);
		glVertex3f(xpos + xlimit, ypos - (3.0f * ylimit / 4.0f) - 0.03f, 0.0f);
		glVertex3f(xpos + xlimit, ypos - (3.0f * ylimit / 4.0f) - 0.0125f, 0.0f);
	}
	glEnd();
}

void drawLetterA(void)
{
	// local variable declarations
	GLfloat xpos = -0.15f;
	GLfloat ypos = 0.4f;

	GLfloat xlimit = 0.3f;
	GLfloat ylimit = 0.4f;
	GLfloat slope = ylimit / (xlimit / 2.0f - 0.05f);

	// code
	glBegin(GL_QUADS);
	{
		// first horizontal
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(xpos + (ylimit / 2.0f + 0.025f) / slope + 0.05f, ypos - ylimit / 2.0f + 0.025f, 0.0f);

		glColor3f(0.8f, 0.9f, 0.65f);
		glVertex3f(xpos + (ylimit / 2.0f - 0.025f) / slope + 0.045f, ypos - ylimit / 2.0f - 0.0125f, 0.0f);
		glVertex3f(xpos + (-1.5f * ylimit - 0.025) / -slope + 0.055f, ypos - ylimit / 2.0f - 0.0125f, 0.0f);
		
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(xpos + (-1.5f * ylimit + 0.025) / -slope + 0.05f, ypos - ylimit / 2.0f + 0.025f, 0.0f);

		// first slant
		glColor3f(1.0f, 0.6f, 0.2f);
		glVertex3f(xpos + xlimit / 2.0f + 0.025f, ypos, 0.0f);
		glVertex3f(xpos + xlimit / 2.0f - 0.025f, ypos, 0.0f);

		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(xpos / 2.0f - 0.025f, ypos - ylimit / 2.0f, 0.0f);
		glVertex3f(xpos / 2.0f + 0.025f, ypos - ylimit / 2.0f, 0.0f);
		glVertex3f(xpos / 2.0f + 0.025f, ypos - ylimit / 2.0f, 0.0f);
		glVertex3f(xpos / 2.0f - 0.025f, ypos - ylimit / 2.0f, 0.0f);

		glColor3f(0.074f, 0.534f, 0.0313f);
		glVertex3f(xpos - 0.025f, ypos - ylimit, 0.0f);
		glVertex3f(xpos + 0.025f, ypos - ylimit, 0.0f);

		// second slant
		glColor3f(1.0f, 0.6f, 0.2f);
		glVertex3f(xpos + xlimit / 2.0f + 0.025, ypos, 0.0f);
		glVertex3f(xpos + xlimit / 2.0f - 0.025, ypos, 0.0f);

		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f((xpos + xlimit) / 2.0f - 0.025f, ypos - ylimit / 2.0f, 0.0f);
		glVertex3f((xpos + xlimit) / 2.0f + 0.025f, ypos - ylimit / 2.0f, 0.0f);
		glVertex3f((xpos + xlimit) / 2.0f + 0.025f, ypos - ylimit / 2.0f, 0.0f);
		glVertex3f((xpos + xlimit) / 2.0f - 0.025f, ypos - ylimit / 2.0f, 0.0f);

		glColor3f(0.074f, 0.534f, 0.0313f);
		glVertex3f(xpos + xlimit - 0.025f, ypos - ylimit, 0.0f);
		glVertex3f(xpos + xlimit + 0.025f, ypos - ylimit, 0.0f);
	}
	glEnd();
}
