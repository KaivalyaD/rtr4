#import <foundation/foundation.h>  // stdio.h
#import <cocoa/cocoa.h>  // macOS SDK main header
#import <QuartzCore/CVDisplayLink.h>  // CoreVideo display link
#import <OpenGL/gl3.h>
#import <OpenGL/gl3ext.h>
#import "vmath.h"

/* global C function declarations (callback for CoreVideo) */
CVReturn KVDDisplayLinkCallback(CVDisplayLinkRef, const CVTimeStamp *, const CVTimeStamp *, CVOptionFlags, CVOptionFlags *, void *);

// global variable declarations
FILE *logFile = NULL;

// interface declarations
@interface AppDelegate: NSObject <NSApplicationDelegate, NSWindowDelegate>
@end

@interface GLView: NSOpenGLView
@end

// entry-point function
int main(int argc, char *argv[])
{
	// code
	// start NSObject auto reference counting
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	// create application object (pre-declared token for all NS applications, now give it memory)
	NSApp = [NSApplication sharedApplication];
	
	// create app delegate object
	AppDelegate *appDelegate = [[AppDelegate alloc] init];
	
	// hand appDelegate over to NSApp
	[NSApp setDelegate: appDelegate];
	
	// start run loop
	[NSApp run];
	
	// once out, release all NSObjects created by this application
	[pool release];
	
	return 0;
}

// implementation for AppDelegate
@implementation AppDelegate
{
@private
	NSWindow *window;
    GLView *view;
}

-(void) applicationDidFinishLaunching: (NSNotification *) notification  // WM_CREATE
{
	// code
    NSBundle *appBundle = [NSBundle mainBundle];
    NSString *appDirectory = [appBundle bundlePath];
    NSString *appParentDirectory = [appDirectory stringByDeletingLastPathComponent];
    
    NSString *logFilePath = [NSString stringWithFormat: @"%@/Log.txt", appParentDirectory];
    if(!logFilePath)
    {
        /* TODO: add message box prompt */
        [self release];
        [NSApp terminate: self];
    }
    const char *cstr_logFilePath = [logFilePath cStringUsingEncoding: NSASCIIStringEncoding];
    
    logFile = fopen(cstr_logFilePath, "w");
    if(!logFile)
    {
        [self release];
        [NSApp terminate: self];
    }
    fprintf(logFile, "application started successfully\n");
    
	NSRect rect = NSMakeRect(0.0, 0.0, 800.0, 600.0);  // C-style call; Cocoa likes a Carbon-based windowing toolkit (CoreGraphics (CG))
	window = [[NSWindow alloc] initWithContentRect:
		rect
		styleMask: NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable
		backing: NSBackingStoreBuffered  // backing store of the window is a GPU buffer
		defer: NO
	];

	[window setTitle: @"KVD: OpenGL"];
	[window setBackgroundColor: [NSColor blackColor]];
	[window center];
    
    view = [[GLView alloc] initWithFrame: rect]; // create a view
    [window setContentView: view];  // set the view
    
    [window setDelegate: self];  // set window's delegate to this object
	[window makeKeyAndOrderFront: self];  // set this view to always be on top
}

-(void) applicationWillTerminate: (NSNotification *) notification  // WM_DESTROY (i.e. on application destruction)
{
	// code
    fprintf(logFile, "application terminated successfully\n");
}

-(void) windowWillClose: (NSNotification *) notification  // WM_CLOSE (i.e. on window close)
{
	// code
	[NSApp terminate: self];  // this also results in a call to [applicationWillTerminate]
}

-(void) dealloc  // called automatically by autoreleaser
{
	// code
    if(view)
    {
        [view release];
        view = nil;
    }
	if(window)
	{
		[window release];
		window = nil;
	}
    if(logFile)
    {
        fclose(logFile);
        logFile = NULL;
    }
	
	[super dealloc];
}
@end

// implementation for GLView
@implementation GLView
{
@private
    CVDisplayLinkRef displayLink;
    
    GLuint shaderProgramObject;
    GLuint vaoPyramid;
    GLuint vboPyramidVertices;
    GLuint mvpMatrixUniform;
    vmath::mat4 perspectiveProjectionMatrix;
    
    enum {
        KVD_ATTRIBUTE_POSITION = 0,
        KVD_ATTRIBUTE_COLOR,
        KVD_ATTRIBUTE_NORMAL,
        KVD_ATTRIBUTE_TEXTURE0
    };
    
    float theta;
}

-(id) initWithFrame: (NSRect) frame
{
    // code
    self = [super initWithFrame: frame];
    
    // create a pixel format attribute array
    NSOpenGLPixelFormatAttribute openGLPixelFormatAttributes[] = {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
        NSOpenGLPFAScreenMask, CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay),  // screen mask is display ID of Core Graphics Library capable of direct (hardware acc.) rendering converted to OpenGL mask
        NSOpenGLPFAColorSize, 24,
        NSOpenGLPFAAlphaSize, 8,
        NSOpenGLPFADepthSize, 24,
        NSOpenGLPFANoRecovery,  // don't fallback to a software context if failed to find a hardware context
        NSOpenGLPFAAccelerated,  // get a hardware context
        NSOpenGLPFADoubleBuffer
    };
    
    // create a pixel format from the array
    NSOpenGLPixelFormat *glPixelFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes: openGLPixelFormatAttributes] autorelease];
    if(glPixelFormat == nil)
    {
        fprintf(logFile, "[NSOpenGLPixelFormat initWithAttributes]: failed\n");
        [self uninitialize];
        [self release];
        [NSApp terminate: self];
    }
    
    // create an OpenGL context with the pixel format
    NSOpenGLContext *glContext = [[[NSOpenGLContext alloc] initWithFormat: glPixelFormat shareContext: nil] autorelease];
    if(glContext == nil)
    {
        fprintf(logFile, "[NSOpenGLContext initWithFormat]: failed\n");
        [self uninitialize];
        [self release];
        [NSApp terminate: self];
    }
    
    // set self's pixel format to created format
    [self setPixelFormat: glPixelFormat];
    
    // set self's OpenGL context to created context
    [self setOpenGLContext: glContext];
    
    return self;
}

/* signature of this method is predefined in NSOpenGLView (not the names) */
-(CVReturn) getFrameForTime: (const CVTimeStamp *) outputTime
{
    // called from the DisplayLinkCallback per frame
    // code
    // this method runs in a different thread and autorelease pools exist per thread
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    [self drawView];
    [pool release];
    return kCVReturnSuccess;
}

/* overridable */
-(void) prepareOpenGL
{
    // code
    [super prepareOpenGL];
    
    // make self's OpenGL context current
    [[self openGLContext] makeCurrentContext];
    
    // set doublebuffer swap interval to 1
    GLint swapInterval = 1;
    [[self openGLContext] setValues: &swapInterval forParameter: NSOpenGLCPSwapInterval];  // CP: Context Parameter
    
    // initialize
    [self initialize];
    
    // To create, configure, and start the display link (eq. to the game loop):
    // 1. create a display link to an active display (i.e. connected to a monitor)
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    
    // 2. set display link output callback for this view (called in a new thread by the driver)
    CVDisplayLinkSetOutputCallback(displayLink, &KVDDisplayLinkCallback, self);
    
    // 3. convert NSOpenGLPixelFormat to CGLPixelFormat (Core Graphics Library)
    CGLPixelFormatObj cglPixelFormat = (CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj];
    
    // 4. convert NSOpenGLContext to CGLContext
    CGLContextObj cglContext = (CGLContextObj)[[self openGLContext] CGLContextObj];
    
    // 5. using this config, set current CG display to CGL pixel format and context
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
    
    // 6. start the display link
    CVDisplayLinkStart(displayLink);
}

/* overridable */
-(void) reshape
{
    // code
    [super reshape];
    
    // this is a critical section hence must be synchronized against concurrent render calls, hence lock the context
    CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
    
    NSRect rect = [self bounds];
    
    // resize
    [self resize: rect.size.width: rect.size.height];
    
    // release the context lock
    CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
}

-(void) drawRect: (NSRect) dirtyRect
{
    // like WM_PAINT for main thread
    // code
    [self drawView];  // to prevent flickering from Yosemite and onwards
}

-(BOOL) acceptsFirstResponder  // GestureDetector.onTouchEvent()
{
    // code
    [[self window] makeFirstResponder: self];  // change first responder from this view's window to this view itself
    
    return YES;  // this view now accepts first responder
}

-(void) keyDown: (NSEvent *) event
{
    // called from a different thread
    // code
    CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
    [[self openGLContext] makeCurrentContext];
    
    int key = (int)[[event characters] characterAtIndex: 0];
    switch(key)
    {
    case 27:
        [self uninitialize];
        [self release];
        [NSApp terminate: self];  // results in a call to applicationWillTerminate after which the pool is released
        break;
    
    case 'F':
    case 'f':
        [[self window] toggleFullScreen: self];
        break;
        
    default:
        break;
    }
    
    CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
}

-(void) drawView
{
    // called from getFrameForTime, so from a new thread
    // like WM_PAINT for the current drawing thread
    // code
    [[self openGLContext] makeCurrentContext];  // make our OpenGL context current for this thread
    
    // lock the context (previous threads might still be running)
    CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
    
    [self updateScene];  // update
    [self renderScene];  // render
    
    // double buffer
    CGLFlushDrawable((CGLContextObj)[[self openGLContext] CGLContextObj]);

    // release the context lock
    CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
}

-(void) initialize
{
    // code
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    
    // log GL info
    [self logGLInfo];
    
    theta = 0.0f;
    
    // vertex shader
    int status = 0;
    int infoLogLength = 0;
    char *infoLog = NULL;

    const GLchar *vertexShaderSourceCode =
        "#version 410 core\n" \
        "\n" \
        "in vec4 a_position;\n" \
        "\n" \
        "uniform mat4 u_mvpMatrix;\n" \
        "\n" \
        "void main(void)\n" \
        "{\n" \
            "gl_Position = u_mvpMatrix * a_position;\n" \
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

        [self uninitialize];
        [self release];
        [NSApp terminate: self];
    }
    fprintf(logFile, "vertex shader was compiled without errors\n");

    // fragment shader
    status = 0;
    infoLogLength = 0;
    infoLog = NULL;

    const GLchar *fragmentShaderSourceCode =
        "#version 410 core\n" \
        "\n" \
        "out vec4 FragColor;\n" \
        "\n" \
        "void main(void)\n" \
        "{\n" \
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

        [self uninitialize];
        [self release];
        [NSApp terminate: self];
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

        [self uninitialize];
        [self release];
        [NSApp terminate: self];
    }
    fprintf(logFile, "shader program was linked without errors\n");

    /* post link processing */
    mvpMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_mvpMatrix");

    /* loading vertex & color data into pipeline */
    glGenVertexArrays(1, &vaoPyramid);
    glBindVertexArray(vaoPyramid);
    {
        // vertices
        glGenBuffers(1, &vboPyramidVertices);
        glBindBuffer(GL_ARRAY_BUFFER, vboPyramidVertices);
        {
            // triangle vertices
            const float vertexPositions[] = {
                // front
                 0.0f,  1.0f,  0.0f,
                -1.0f, -1.0f,  1.0f,
                 1.0f, -1.0f,  1.0f,

                // right
                 0.0f,  1.0f,  0.0f,
                 1.0f, -1.0f,  1.0f,
                 1.0f, -1.0f, -1.0f,

                // rear
                 0.0f,  1.0f,  0.0f,
                 1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f, -1.0f,

                // left
                 0.0f,  1.0f,  0.0f,
                -1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f,  1.0f
            };

            glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);
            glVertexAttribPointer(KVD_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(KVD_ATTRIBUTE_POSITION);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);
    
    glClearDepth(1.0f);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    
    perspectiveProjectionMatrix = vmath::mat4::identity();
}

-(void) logGLInfo
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

-(void) resize: (int) width : (int) height
{
    // variable declarations
    GLfloat aspectRatio = 0.0f;
    
    // code
    if(height < 0)
        height = 1;
    
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    
    aspectRatio = (GLfloat)width / (GLfloat)height;
    perspectiveProjectionMatrix = vmath::perspective(
        45.0f,
        aspectRatio,
        0.1f,
        100.0f
    );
}

-(void) renderScene
{
    // variable declarations
    vmath::mat4 modelViewMatrix;
    vmath::mat4 modelViewProjectionMatrix;

    // code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgramObject);
    {
        // transformations
        modelViewMatrix = vmath::translate(0.0f, 0.0f, -6.0f) * vmath::rotate(theta, 0.0f, 1.0f, 0.0f);
        modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
        glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionMatrix);
        
        glBindVertexArray(vaoPyramid);
            glDrawArrays(GL_TRIANGLES, 0, 12);
        glBindVertexArray(0);
    }
    glUseProgram(0);
}

-(void) updateScene  // NSOpenGLView has its own update not meant for animation
{
    // code
    theta += 1.0f;
    if(theta > 360.0f)
        theta = theta - 360.0f;
}

-(void) uninitialize
{
    // code
    if(vboPyramidVertices)
    {
        glDeleteBuffers(1, &vboPyramidVertices);
        vboPyramidVertices = 0U;
    }

    if(vaoPyramid)
    {
        glDeleteVertexArrays(1, &vaoPyramid);
        vaoPyramid = 0U;
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
}

-(void) dealloc
{
    // code
    [super dealloc];
    
    if(displayLink)
    {
        CVDisplayLinkStop(displayLink);
        CVDisplayLinkRelease(displayLink);
        displayLink = nil;
    }
}
@end

/* implementing the DisplayLinkCallback */
CVReturn KVDDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *currentTime, const CVTimeStamp *outputTime, CVOptionFlags flagsIn, CVOptionFlags *flagsOut, void *view)
{
    // called by QuartzCore (the CoreVideo driver)
    // code
    return [(GLView *)view getFrameForTime: outputTime];
}
