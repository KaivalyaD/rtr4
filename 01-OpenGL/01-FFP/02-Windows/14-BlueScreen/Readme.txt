** This application onwards, Window.c is OGL.cpp (although no special feature of C++ will be used). **
   [Why? After a few more programs, a new header written in C++ syntax shall be included.]

### Changes ###

# Headers #
1. A new header is included: <GL\gl.h>.
   [Why?]
   
//////////////////////////////////////////////////////////////////////////////////////////////////////

# Macros #

//////////////////////////////////////////////////////////////////////////////////////////////////////

# Pragma Directives #
2. A library is loaded programmatically for this application: "OpenGL32.lib"
   [Why?]

//////////////////////////////////////////////////////////////////////////////////////////////////////

# Global Function Prototypes #

//////////////////////////////////////////////////////////////////////////////////////////////////////

# Global Variables #
3. 2 new global variables are declared:
	a. HDC ghdc: to store the device context of the device displaying current window
	b. HGLRC ghrc: to store the rendering context of the device displaying the current window

//////////////////////////////////////////////////////////////////////////////////////////////////////

## Entry Point ##

# Function Prototypes #
4. A new function is used: uninitialize(). Whenever there is a critical error during or after
   initialize(), so that continuing execution is either of no use or is harmful, uninitialize() to
   terminate the application instead of returning or calling exit().

# Local Variables #

# Code #
5. During initialize(), 4 critical function calls may fail - ChoosePixelFormat(), SetPixelFormat(),
   wglCreateContext(), wglMakeCurrent(). If any of these fail, log the failure and uninitialize();
   else log the success and continue execution.
   
//////////////////////////////////////////////////////////////////////////////////////////////////////

## Window Procedure ##

//////////////////////////////////////////////////////////////////////////////////////////////////////

## User Defined Functions ##

//////////////////////////////////////////////////////////////////////////////////////////////////////

Notes:
1. When the window is being resized, it turns black and remains black until the mouse is released;
   and then turns blue again.
