#import <foundation/foundation.h>  // stdio.h
#import <cocoa/cocoa.h>  // macOS SDK main header
#import <QuartzCore/CVDisplayLink.h>  // CoreVideo display link
#import <OpenGL/gl3.h>
#import <OpenGL/gl3ext.h>
#import "vmath.h"

#define MAX_STACK_SIZE 32

/* global C function declarations (callback for CoreVideo) */
CVReturn KVDDisplayLinkCallback(CVDisplayLinkRef, const CVTimeStamp *, const CVTimeStamp *, CVOptionFlags, CVOptionFlags *, void *);

// global variable declarations
FILE *logFile = NULL;

// interface declarations
@interface AppDelegate: NSObject <NSApplicationDelegate, NSWindowDelegate>
@end

@interface GLView: NSOpenGLView
@end

@interface OpenGLSphere: NSObject
@end

@interface Mat4Stack: NSObject
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

// sphere class implementation
@implementation OpenGLSphere
{
@private GLuint vao;
@private GLuint vbo;
@private GLuint eabo;
@private unsigned int positionAttribLocation;
@private unsigned int normalAttribLocation;
    
@private float dTheta;
@private float dPhi;
@private int slices;
@private int stacks;
    
@private unsigned short vertexCount;
@private unsigned short faceCount;
    
@private vmath::vec3 *vertices;
@private unsigned short **faceIndices;

@private bool isGenerated;
}
-(id) initWithSlicesAndStacks: (int) _slices : (int) _stacks
{
    // code
    self = [super init];
    isGenerated = false;
    
    if(_slices == 0)
        _slices = 2;
    if(_stacks == 0)
        _stacks = 2;
    
    /* Something is terribly wrong with vertexCount formula */
    slices = _slices;
    stacks = _stacks;
    
    dTheta = M_PI / (float)stacks;
    dPhi = M_PI / (float)slices;  // = 2*PI / 2*(1+slices)
    
    /*
     * How vertexCount was calculated:
     *  a. there are 2 vertices at the poles;
     *  b. there are 2 * slices vertices per slice; and
     *  c. there are (stacks - 1) slices in the sphere.
     */
    vertexCount = 2 + 2 * slices * (stacks - 1);
    fprintf(logFile, "OpenGLSphere: vertexCount = %d\n", vertexCount);
    
    /*
     * How faceCount was calculated:
     *  a. there are 2 * slices triangle faces on each pole;
     *  b. there are 4 * slices triangle faces per stack; and
     *  c. there are (stacks - 2) stacks (excluding polar stacks) in the sphere.
     *
     *  thus,
     *      faceCount = (2 * slices) + (4 * slices * (stacks - 2)) + (2 * slices)
     *   => faceCount = (4 * slices) * (1 + (stacks - 2))
     *   => \__faceCount = 4 * slices * (stacks - 1)__/
     */
    faceCount = 4 * slices * (stacks - 1);
    fprintf(logFile, "OpenGLSphere: faceCount = %d\n", faceCount);
    
    vertices = (vmath::vec3 *)malloc(vertexCount * sizeof(vmath::vec3));
    if(!vertices)
    {
        [self dealloc];
        return nil;
    }
    
    // there are 3 vertices and thus vertex indices per face
    faceIndices = (unsigned short **)malloc(faceCount * sizeof(unsigned short *));
    if(!faceIndices)
    {
        [self dealloc];
        return nil;
    }
    for(int i = 0; i < faceCount; i++)
    {
        faceIndices[i] = (unsigned short *)malloc(3 * sizeof(unsigned short));
        if(!faceIndices[i])
        {
            [self dealloc];
            return nil;
        }
    }
    
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &eabo);
    
    // defaults
    positionAttribLocation = 0;
    normalAttribLocation = 1;
    
    return self;
}

-(void) setPositionAttribLocation: (unsigned int) location
{
    // code
    positionAttribLocation = location;
}

-(void) setNormalAttribLocation: (unsigned int) location
{
    // code
    normalAttribLocation = location;
}

-(void) generate
{
    // variable declarations
    float x, y, z, phi, theta, sin_theta;
    int currentIndex;
    const float PI = M_PI;
    const float TWO_PI = 2.0f * M_PI;
    
    // code
    if(isGenerated)
        return;
    
    // vertex array buffer
    currentIndex = 0;
    vertices[currentIndex++] = vmath::vec3(0.0f, 1.0f, 0.0f);
    
    theta = dTheta;
    while(theta < PI)
    {
        y = cosf(theta);
        sin_theta = sinf(theta);
        
        phi = 0.0f;
        while(phi < TWO_PI)
        {
            x = sin_theta * cosf(phi);
            z = sin_theta * sinf(phi);
            vertices[currentIndex++] = vmath::vec3(x, y, z);
            phi += dPhi;
        }
        
        theta += dTheta;
    }
    
    vertices[currentIndex++] = vmath::vec3(0.0f, -1.0f, 0.0f);
    isGenerated = true;
    
    // element array buffer
    int verticesPerSlice = 2 * slices;
    int currentFaceVertexIndex = 0;
    currentIndex = 0;
    
    // top polar cap indices
    int stageOne = verticesPerSlice;
    while(currentIndex < stageOne)
    {
        faceIndices[currentIndex][0] = 0;
        faceIndices[currentIndex][1] = currentFaceVertexIndex + 1;
        faceIndices[currentIndex][2] = (currentFaceVertexIndex + 2 > 2 * slices) ? 1 : (currentFaceVertexIndex + 2);
        
        currentFaceVertexIndex++;
        currentIndex++;
    }
    
    // main sphere body indices
    currentFaceVertexIndex = 1;
    
    int stageTwo = stageOne + (4 * slices * (stacks - 2));
    while(currentIndex < stageTwo)
    {
        faceIndices[currentIndex][0] = currentFaceVertexIndex;
        faceIndices[currentIndex][1] = currentFaceVertexIndex + verticesPerSlice;
        faceIndices[currentIndex][2] = ((currentFaceVertexIndex + 1) % verticesPerSlice == 1) ?
            (currentFaceVertexIndex - verticesPerSlice + 1) :
            (currentFaceVertexIndex + 1);
        
        faceIndices[currentIndex + 1][0] = faceIndices[currentIndex][2];
        faceIndices[currentIndex + 1][1] = faceIndices[currentIndex][1];
        faceIndices[currentIndex + 1][2] = ((currentFaceVertexIndex + verticesPerSlice + 1) % verticesPerSlice == 1) ?
            (faceIndices[currentIndex + 1][1] - verticesPerSlice + 1) :
            (currentFaceVertexIndex + verticesPerSlice + 1);
        
        currentFaceVertexIndex++;
        currentIndex += 2;
    }
    
    // bottom polar cap indices
    currentFaceVertexIndex = vertexCount - verticesPerSlice - 1;
    
    int stageThree = stageTwo + verticesPerSlice;
    while(currentIndex < stageThree)
    {
        faceIndices[currentIndex][0] = currentFaceVertexIndex;
        faceIndices[currentIndex][1] = vertexCount - 1;
        faceIndices[currentIndex][2] = (currentFaceVertexIndex + 1 > vertexCount - 2 ) ?
            (vertexCount - verticesPerSlice - 1) :
            (currentFaceVertexIndex + 1);
        
        currentFaceVertexIndex++;
        currentIndex++;
    }
}

-(bool) loadVerticesIntoOpenGLPipeline
{
    // code
    if(!isGenerated)
        return false;
    
    glBindVertexArray(vao);
        // vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(vmath::vec3), vertices, GL_STATIC_DRAW);
            
            glVertexAttribPointer(positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(positionAttribLocation);
            
            // normal and position vectors are the same for a sphere
            glVertexAttribPointer(normalAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(normalAttribLocation);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        // index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eabo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * faceCount * sizeof(unsigned short), NULL, GL_DYNAMIC_DRAW);
        
        int mappedIndex = 0;
        int faceIndex = 0;
        while(faceIndex < faceCount)
        {
            void *pMappedBuf = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, mappedIndex * sizeof(unsigned short), 3 * sizeof(unsigned short), GL_MAP_WRITE_BIT);
            
            ((unsigned short *)pMappedBuf)[0] = faceIndices[faceIndex][0];
            ((unsigned short *)pMappedBuf)[1] = faceIndices[faceIndex][1];
            ((unsigned short *)pMappedBuf)[2] = faceIndices[faceIndex][2];
            
            glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
            mappedIndex += 3;
            faceIndex += 1;
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    return true;
}

-(void) render: (GLenum) mode
{
    // code
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eabo);
    glDrawElements(mode, 3 * faceCount, GL_UNSIGNED_SHORT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

-(void) dealloc
{
    // code
    if(eabo)
    {
        glDeleteBuffers(1, &eabo);
        eabo = 0U;
    }
    if(vbo)
    {
        glDeleteBuffers(1, &vbo);
        vbo = 0U;
    }
    if(vao)
    {
        glDeleteVertexArrays(1, &vao);
        vao = 0U;
    }
    if(faceIndices)
    {
        for(int i = 0; i < faceCount; i++)
        {
            if(faceIndices[i])
            {
                free(faceIndices[i]);
                faceIndices[i] = NULL;
            }
        }
        free(faceIndices);
        faceIndices = NULL;
    }
    if(vertices)
    {
        free(vertices);
        vertices = NULL;
    }
    if(isGenerated)
    {
        isGenerated = false;
    }
    
    [super dealloc];
}
@end

// implementation for Mat4Stack
@implementation Mat4Stack
{
@private
    vmath::mat4 matMV[MAX_STACK_SIZE];
    int topCntr;
}
-(id) init
{
    // code
    self = [super init];
    if(!self)
        return nil;
    
    topCntr = -1;
    return self;
}

-(vmath::mat4) top
{
    // code
    if(topCntr > -1)
        return matMV[topCntr];
    
    return vmath::mat4::identity();
}

-(void) push: (vmath::mat4) mat
{
    // code
    vmath::mat4 newTop = [self top] * mat;
    
    topCntr += 1;
    if(topCntr > MAX_STACK_SIZE - 1)
    {
        topCntr = MAX_STACK_SIZE - 1;
        return;
    }
    
    matMV[topCntr] = newTop;
}

-(void) pop
{
    // code
    if(topCntr > -1)
        topCntr -= 1;
}

-(void) dealloc
{
    // code
    [super dealloc];
}
@end

// implementation for GLView
@implementation GLView
{
@private
    CVDisplayLinkRef displayLink;
    
    GLuint shaderProgramObject;
    GLuint mvpMatrixUniform;
    GLuint colorUniform;
    vmath::mat4 perspectiveProjectionMatrix;
    
    enum {
        KVD_ATTRIBUTE_POSITION = 0,
        KVD_ATTRIBUTE_COLOR,
        KVD_ATTRIBUTE_NORMAL,
        KVD_ATTRIBUTE_TEXTURE0
    };
    
    float thetaRevEarth;
    float thetaRotEarth;
    float thetaRevMoon;
    float thetaRotMoon;
    
    OpenGLSphere *sphere;
    Mat4Stack *stackMatModelView;
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
            
    case 'y':
        thetaRevEarth += 3.0f;
        break;

    case 'Y':
        thetaRevEarth -= 3.0f;
        break;

    case 'd':
        thetaRotEarth += 3.0f;
        break;

    case 'D':
        thetaRotEarth -= 3.0f;
        break;

    case 'm':
        thetaRevMoon += 3.0f;
        break;

    case 'M':
        thetaRevMoon -= 3.0f;
        break;

    case 'n':
        thetaRotMoon += 3.0f;
        break;

    case 'N':
        thetaRotMoon -= 3.0f;
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
    // log GL info
    [self logGLInfo];
    
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
        "uniform vec4 u_color;"
        "\n" \
        "out vec4 FragColor;\n" \
        "\n" \
        "void main(void)\n" \
        "{\n" \
            "FragColor = u_color;\n" \
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
    glBindAttribLocation(shaderProgramObject, KVD_ATTRIBUTE_POSITION, "a_position");
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
    colorUniform = glGetUniformLocation(shaderProgramObject, "u_color");

    /* loading vertex data into pipeline */
    sphere = [[OpenGLSphere alloc] initWithSlicesAndStacks: 25 : 25];
    [sphere setPositionAttribLocation: KVD_ATTRIBUTE_POSITION];
    [sphere setNormalAttribLocation: KVD_ATTRIBUTE_NORMAL];
    [sphere generate];
    [sphere loadVerticesIntoOpenGLPipeline];
    
    // initialize the stack
    stackMatModelView = [[Mat4Stack alloc] init];
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
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
    // code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgramObject);
    {
        // transformations for the sun
        [stackMatModelView push: vmath::translate(0.0f, 0.0f, -5.0f)];
        {
            glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix * [stackMatModelView top]);
            glUniform4fv(colorUniform, 1, vmath::vec4(1.0f, 0.5f, 0.0f, 1.0f));
            [sphere render: GL_TRIANGLES];

            // transformations for the earth
            [stackMatModelView push:
                              vmath::rotate(thetaRevEarth, 0.0f, 1.0f, 0.0f) *
                              vmath::translate(2.0f, 0.0f, 0.0f) *
                              vmath::rotate(thetaRotEarth, 0.0f, 1.0f, 0.0f) *
                              vmath::scale(0.25f, 0.25f, 0.25f)];
            {
                glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix * [stackMatModelView top]);
                glUniform4fv(colorUniform, 1, vmath::vec4(0.2f, 0.3f, 0.8f, 1.0f));
                [sphere render: GL_TRIANGLES];
            }
            [stackMatModelView pop];  // pop transformations for the earth

            // transformations for the moon
            [stackMatModelView push:
                              vmath::rotate(thetaRevEarth, 0.0f, 1.0f, 0.0f) *
                              vmath::translate(2.0f, 0.0f, 0.0f) *
                              vmath::rotate(thetaRevMoon, 0.0f, 1.0f, 0.0f) *
                              vmath::translate(0.5f, 0.0f, 0.0f) *
                              vmath::rotate(thetaRotMoon, 0.0f, 1.0f, 0.0f) *
                              vmath::scale(0.05f, 0.05f, 0.05f)];
            {
                glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix * [stackMatModelView top]);
                glUniform4fv(colorUniform, 1, vmath::vec4(0.8f, 0.8f, 0.8f, 1.0f));
                [sphere render: GL_TRIANGLES];
            }
            [stackMatModelView pop];  // pop transformations for the moon
        }
        [stackMatModelView pop];  // pop transformations for the sun
    }
    glUseProgram(0);
}

-(void) updateScene  // NSOpenGLView has its own update not meant for animation
{
    // code
}

-(void) uninitialize
{
    // code
    if(sphere)
    {
        [sphere dealloc];
        sphere = nil;
    }
    
    if(stackMatModelView)
    {
        [stackMatModelView dealloc];
        stackMatModelView = nil;
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
