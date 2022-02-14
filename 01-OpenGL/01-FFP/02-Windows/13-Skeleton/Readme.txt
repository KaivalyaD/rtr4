** This code will be used as is for OpenGL as well as DirectX applications on Windows **

### Changes ###

# Headers #

//////////////////////////////////////////////////////////////////////////////////////////////////////

# Macros #
1. Integer variables cxClient, cyClient changed to macro definitions WIN_WIDTH and WIN_HEIGHT.
   It is in general a good practice to define macros for values that aren't going to change.
   Although the window width and height may change in future, their default values when the
   application is run is constant.

//////////////////////////////////////////////////////////////////////////////////////////////////////

# Pragma Directives #

//////////////////////////////////////////////////////////////////////////////////////////////////////

# Global Function Prototypes #

//////////////////////////////////////////////////////////////////////////////////////////////////////

# Global Variables #

//////////////////////////////////////////////////////////////////////////////////////////////////////

## Entry Point ##

# Function Prototypes #
2. 3 new user-defined functions used: initialize(), display(), update(). Although their order
   doesn't matter to the compiler, they are declared in the same order as the sequence of their
   first use as a discipline.

# Variable Declarations #
3. Integer variable iRetVal declared to store the return value of initialize(). This value
   shall be used in future for checking and handling errors that might have occurred during
   initialization.

# Code #
4. wndclass.hbrBackground changed back to BLACK_BRUSH.

5. cxClient, cyClient replaced by WIN_WIDTH, WIN_HEIGHT respectively.

6. Call to UpdateWindow() removed.
	a. [Why SetForegroundWindow()?]
	b. [Why SetFocus()?]
	c. Why avoid using ghwnd in WinMain()?
	   WinMain() is the first function to receive the window handle of this window and
	   already has a variable to store it. Because the stack is easier and faster to access
	   than the global memory it is better to use the local variable when a choice is available.

# Game Loop #
7. display() and update() called in that order in the case when:
   a. There is no message available in the message queue of this window's thread; and
   b. This window is the active window.

//////////////////////////////////////////////////////////////////////////////////////////////////////

## Window Procedure ##

# Function Prototypes #
8. 2 new functions used: resize(), uninitialize(). Although their order doesn't matter to
   the compiler, they are declared in the same order as the sequence of their first use as
   a discipline.

# Message Handlers #
9. WM_ERASEBKGND:
   While in WM_PAINT there is no need to explicitly state whether or not to erase background
   as PAINTSTRUCT::fErase is always true, because this is an OpenGL application and not a Win32
   application, there is no one to erase the background. OpenGL is responsible for this operation.

  [Not applicable to current Windows:
   We don't want the default window procedure to process this message because
   we don't want the OS to erase the background after we have already processed that
   message, which explains "return 0" instead of "break".]

10. WM_SIZE:
	LOWORD(lParam) and HIWORD(lParam) respectively contain the new width and height of the window
	whose WndProc() receives the WM_SIZE message when it is resized (maximized, minimized, or using
	the mouse). Because this is an OpenGL application, it has to know about this change in size,
	which we handle in the resize() function. WM_SIZE is actually processed inside resize() but is
	called from WM_SIZE handler.

11. WM_CLOSE:
	Sent as a signal that a window or an application should terminate. Code for prompts asking the user
	to confirm termination should be written here. We chose to terminate straight away assuming the user
	knows what he/she is doing.

12. WM_DESTROY:
	uninitialize() is called before PostQuitMessage().

//////////////////////////////////////////////////////////////////////////////////////////////////////

## User-Defined Functions ##

# initialize() #
  arguments: none
  returns: none
  description:

# resize() #
  arguments: [in] new width of the window (int)
			 [in] new height of the window (int)
  returns: 0 in case of success [int]
		   non-zero value in case of error [int]
  description:
	a. In future, there will be cases when the ratio of window's width and the window's height is
	   necessary. If the height were to be zero, calculating this ratio will generate a fault and
	   the application will terminate abruptly. Thus, before this error propagates anywhere, we
	   alter the value of height at the first moment we see that it is zero which occurs in the
	   WM_SIZE message handler, where we have called resize().

# display() #
  arguments: none
  returns: none
  description:

# update() #
  arguments: none
  returns: none
  description:

# uninitialize() #
  arguments: none
  returns: none
  description:
	a. As a discipline, before terminating always check if the application is in fullscreen mode
	   or not. If it is fullscreen, first switch back to the normal (restored) state and then
	   continue with termination.

	b. As a discipline, before invalidating a pointer, always check if the pointer is already
	   invalid. If not, first invalidate (Ex. through free()) and then reset its value to NULL.
