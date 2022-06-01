/* standard headers */
#include <stdio.h>  // for standard I/O
#include <stdlib.h> // for exit()
#include <memory.h> // for memset()
#include <string.h> // for strlen()

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
Bool fullScreen = False;

/* entry-point function */
int main(int argc, char *argv[])
{
    // function declarations
    void toggleFullscreen(void);
    void uninitialize(void);

    // local variables
    int defaultScreen, defaultDepth;
    XSetWindowAttributes windowAttributes;
    int styleMask = 0;
    Atom wm_delete_window_atom;
    XEvent event;
    KeySym keysym;
    int screenWidth, screenHeight;
    char keys[26];
    Status status;

    static GC gc;
    XGCValues gcValues;
    static XFontStruct *fontStruct = NULL;
    XColor fontColor;
    char helloWorld[] = "Hello World!!!";
    int strLength;
    int fontWidth;
    int fontHeight;
    static int winWidth;
    static int winHeight;
    
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
    windowAttributes.event_mask |= ExposureMask | KeyPressMask | StructureNotifyMask;

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

    XStoreName(display, window, "Kaivalya Vishwakumar Deshpande: Hello World");

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
            fontStruct = XLoadQueryFont(display, "fixed");
            break;

        case KeyPress:
            keysym = XkbKeycodeToKeysym(display, event.xkey.keycode, 0, 0);
            switch(keysym)
            {
            case XK_Escape:
                XUnloadFont(display, fontStruct->fid);
                XFreeGC(display, gc);
                uninitialize();
                exit(EXIT_SUCCESS);
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

        case ConfigureNotify:
            winWidth = event.xconfigure.width;
            winHeight = event.xconfigure.height;
            break;

        case Expose:
            gc = XCreateGC(display, window, 0, &gcValues);

            XSetFont(display, gc, fontStruct->fid);

            XAllocNamedColor(display, colormap, "green", &fontColor, &fontColor);
            XSetForeground(display, gc, fontColor.pixel);

            strLength = strlen(helloWorld);
            fontWidth = XTextWidth(fontStruct, helloWorld, strLength);
            fontHeight = fontStruct->ascent + fontStruct->descent;  // these 2 values are measured from the 'baseline' located at center of a character

            XDrawString(display, window, gc, ((winWidth - fontWidth) / 2), ((winHeight - fontHeight) / 2), helloWorld, strLength);
            break;

        case 33:    // wm_delete_window_atom is always registered with ID 33; this event occurs when the close button/option is clicked
            XUnloadFont(display, fontStruct->fid);
            XFreeGC(display, gc);
            uninitialize();
            exit(EXIT_SUCCESS);
            break;

        default:
            break;
        }
    }

    XUnloadFont(display, fontStruct->fid);
    XFreeGC(display, gc);
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

    XSendEvent(display, RootWindow(display, visualInfo.screen), False, SubstructureNotifyMask, &event);
}

void uninitialize(void)
{
    // code
    if(fullScreen)
        toggleFullscreen();

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
