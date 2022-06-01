/* standard headers */
#include <stdio.h>  // for standard I/O
#include <stdlib.h> // for exit()
#include <memory.h> // for memset()

/* X11 headers */
#include <X11/Xlib.h>
#include <X11/Xutil.h>  // for XVisualInfo
#include <X11/XKBlib.h> // for the keyboard

/* macros */
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

/* global variables */
Display *display = NULL;
XVisualInfo visualInfo;
Colormap colormap;
Window window;

/* entry-point function */
int main(int argc, char *argv[])
{
    // function declarations
    void uninitialize(void);

    // local variables
    int defaultScreen, defaultDepth;
    XSetWindowAttributes windowAttributes;
    int styleMask = 0;
    Atom wm_delete_window_atom;
    XEvent event;
    KeySym keysym;
    int screenWidth, screenHeight;
    Status status;
    
    // code
    display = XOpenDisplay(NULL);
    if(!display)
    {
        fprintf(stderr, "error: XOpenDisplay() failed\n");
        uninitialize();
        exit(EXIT_FAILURE);
    }

    defaultScreen = XDefaultScreen(display);
    defaultDepth = XDefaultDepth(display, defaultScreen);

    status = XMatchVisualInfo(display, defaultScreen, defaultDepth, TrueColor, &visualInfo);
    if(!status)
    {
        fprintf(stderr, "error: XMatchVisualInfo() failed with code %d\n", status);
        uninitialize();
        exit(EXIT_FAILURE);
    }

    memset(&windowAttributes, 0, sizeof(XSetWindowAttributes));
    windowAttributes.border_pixel = 0;
    windowAttributes.background_pixel = XBlackPixel(display, defaultScreen);
    windowAttributes.background_pixmap = 0;
    windowAttributes.colormap = XCreateColormap(display, RootWindow(display, visualInfo.screen), visualInfo.visual, AllocNone);
    windowAttributes.event_mask |= ExposureMask | KeyPressMask | ButtonPressMask | FocusChangeMask;

    colormap = windowAttributes.colormap;

    styleMask |= CWBorderPixel | CWBackPixel | CWColormap | CWEventMask;

    window = XCreateWindow(
        display,
        RootWindow(display, visualInfo.screen),
        0,
        0,
        WIN_WIDTH,
        WIN_HEIGHT,
        0,
        visualInfo.depth,
        InputOutput,
        visualInfo.visual,
        styleMask,
        &windowAttributes
    );
    if(!window)
    {
        fprintf(stderr, "error: XCreateWindow() failed\n");
        uninitialize();
        exit(EXIT_FAILURE);
    }

    XStoreName(display, window, "Kaivalya Vishwakumar Deshpande: Messages");

    wm_delete_window_atom = XInternAtom(display, "WM_DELETE_WINDOW", True); // True: create the atom in all cases: whether it exists before or not
    XSetWMProtocols(display, window, &wm_delete_window_atom, 1);

    XMapWindow(display, window);

    // centering the window
    screenWidth = XWidthOfScreen(XScreenOfDisplay(display, defaultScreen));
    screenHeight = XHeightOfScreen(XScreenOfDisplay(display, defaultScreen));
    XMoveWindow(display, window, ((screenWidth - WIN_WIDTH) / 2), ((screenHeight - WIN_HEIGHT) / 2));

    // message loop
    while(1)
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
                uninitialize();
                exit(EXIT_SUCCESS);
                break;

            default:
                break;
            }
            break;

        case ButtonPress:
            switch(event.xbutton.button)
            {
            case 1:
                fprintf(stdout, "Event ButtonPress: you clicked the Left Mouse Button\n");
                break;

            case 2:
                fprintf(stdout, "Event ButtonPress: you clicked the Middle Mouse Button\n");
                break;

            case 3:
                fprintf(stdout, "Event ButtonPress: you clicked the Right Mouse Button\n");
                break;

            default:
                break;
            }
            break;

        case FocusIn:
            fprintf(stdout, "Event FocusIn: the window has gained focus\n");
            break;

        case FocusOut:
            fprintf(stdout, "Event ButtonPress: the window has lost focus\n");
            break;

        case 33:    // wm_delete_window_atom is always registered with ID 33; this event occurs when the close button/option is clicked
            uninitialize();
            exit(EXIT_SUCCESS);
            break;

        default:
            break;
        }
    }

    uninitialize();

    return 0;
}

void uninitialize(void)
{
    // code
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
