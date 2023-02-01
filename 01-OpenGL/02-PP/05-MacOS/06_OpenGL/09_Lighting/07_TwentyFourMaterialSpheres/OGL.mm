#import <foundation/foundation.h>  // stdio.h
#import <string.h>
#import <stdarg.h>
#import <cocoa/cocoa.h>  // macOS SDK main header
#import <QuartzCore/CVDisplayLink.h>  // CoreVideo display link
#import <OpenGL/gl3.h>
#import <OpenGL/gl3ext.h>
#import <signal.h>
#import "vmath.h"

// macros
#define NORMAL_VIEW_ENABLED 0

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

// implementation for GLView
@implementation GLView
{
@private
    CVDisplayLinkRef displayLink;
    
    GLuint shaderProgramObject;
    GLuint vaoSphere;
    GLuint vboSphereVertices;
    GLuint vboSphereNormals;
    
    unsigned int windowWidth;
    unsigned int windowHeight;
    
    struct Transformations {
        vmath::mat4 model;
        vmath::mat4 view;
        vmath::mat4 projection;
    } transformations;
    
    struct Light {
        vmath::vec4 ambient;
        vmath::vec4 diffuse;
        vmath::vec4 specular;
        vmath::vec4 position;
    } light;
    
    struct lightingShaderUniforms {
        struct {
            GLuint model;  // mat4
            GLuint view;  // mat4
            GLuint projection;  // mat4
        } transformations;
        struct {
            GLuint ambient;  // vec4
            GLuint diffuse;  // vec4
            GLuint specular;  // vec4
            GLuint position;  // vec4
        } light;
        struct {
            GLuint ambient;  // vec4
            GLuint diffuse;  // vec4
            GLuint specular;  // vec4
            GLuint shininess;  // float
        } material;
        GLuint isLight;  // bool
        GLuint isViewNormals;  // bool
        GLuint viewerPosition;  // vec4
    } uniforms;
    
    enum {
        KVD_ATTRIBUTE_POSITION = 0,
        KVD_ATTRIBUTE_COLOR,
        KVD_ATTRIBUTE_NORMAL,
        KVD_ATTRIBUTE_TEXTURE0
    };
    
    bool isLight;
    bool isViewNormals;
    float theta;
    int axis;
    OpenGLSphere *sphere;
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
            
    case 'L':
    case 'l':
        isLight = !isLight;
        break;
            
    case 'N':
    case 'n':
        isViewNormals = !isViewNormals;
        break;
        
    case 'X':
    case 'x':
        axis = 1;
        break;
        
    case 'Y':
    case 'y':
        axis = 2;
        break;
        
    case 'Z':
    case 'z':
        axis = 3;
        break;
            
    default:
        axis = 0;
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

#if NORMAL_VIEW_ENABLED
    const GLchar *vertexShaderSourceCode =
        "#version 410 core\n" \
        "\n" \
        "in vec4 a_position;\n" \
        "in vec3 a_normal;\n" \
        "\n" \
        "uniform struct Transformations {\n" \
        "   mat4 model;\n" \
        "   mat4 view;\n" \
        "   mat4 projection;\n" \
        "} transformations;\n" \
        "\n" \
        "uniform struct Light {\n" \
        "   vec4 ambient;\n" \
        "   vec4 diffuse;\n" \
        "   vec4 specular;\n" \
        "   vec4 position;\n" \
        "} light;\n" \
        "\n" \
        "uniform struct Material {\n" \
        "   vec4 ambient;\n" \
        "   vec4 diffuse;\n" \
        "   vec4 specular;\n" \
        "   float shininess;\n" \
        "} material;\n" \
        "\n" \
        "uniform vec4 viewerPosition;\n" \
        "\n" \
        "out VS_OUT {\n" \
        "   vec3 lightDirection;\n" \
        "   vec3 viewDirection;\n" \
        "   vec3 normal;\n" \
        "} vs_out;\n" \
        "\n" \
        "void main(void)\n" \
        "{\n" \
        "   mat4 modelViewMatrix = transformations.view * transformations.model;\n" \
        "   mat3 normalMatrix = mat3(transpose(inverse(modelViewMatrix)));\n" \
        "   \n" \
        "   gl_Position = modelViewMatrix * a_position;\n" \
        "   \n" \
        "   vec4 eyeCoordinate = modelViewMatrix * a_position;\n"
        "   vs_out.lightDirection = normalize(light.position - eyeCoordinate).xyz;\n" \
        "   vs_out.viewDirection = normalize(viewerPosition - eyeCoordinate).xyz;\n" \
        "   vs_out.normal = normalize(normalMatrix * a_normal);\n" \
        "}\n";
#else
    const GLchar *vertexShaderSourceCode =
        "#version 410 core\n" \
        "\n" \
        "in vec4 a_position;\n" \
        "in vec3 a_normal;\n" \
        "\n" \
        "uniform struct Transformations {\n" \
        "   mat4 model;\n" \
        "   mat4 view;\n" \
        "   mat4 projection;\n" \
        "} transformations;\n" \
        "\n" \
        "uniform struct Light {\n" \
        "   vec4 ambient;\n" \
        "   vec4 diffuse;\n" \
        "   vec4 specular;\n" \
        "   vec4 position;\n" \
        "} light;\n" \
        "\n" \
        "uniform struct Material {\n" \
        "   vec4 ambient;\n" \
        "   vec4 diffuse;\n" \
        "   vec4 specular;\n" \
        "   float shininess;\n" \
        "} material;\n" \
        "\n" \
        "uniform vec4 viewerPosition;\n" \
        "\n" \
        "out VS_OUT {\n" \
        "   vec3 lightDirection;\n" \
        "   vec3 viewDirection;\n" \
        "   vec3 normal;\n" \
        "} vs_out;\n" \
        "\n" \
        "void main(void)\n" \
        "{\n" \
        "   mat4 modelViewMatrix = transformations.view * transformations.model;\n" \
        "   mat3 normalMatrix = mat3(transpose(inverse(modelViewMatrix)));\n" \
        "   \n" \
        "   gl_Position = transformations.projection * modelViewMatrix * a_position;\n" \
        "   \n" \
        "   vec4 eyeCoordinate = modelViewMatrix * a_position;\n"
        "   vs_out.lightDirection = normalize(light.position - eyeCoordinate).xyz;\n" \
        "   vs_out.viewDirection = normalize(viewerPosition - eyeCoordinate).xyz;\n" \
        "   vs_out.normal = normalize(normalMatrix * a_normal);\n" \
        "}\n";
#endif

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

#if NORMAL_VIEW_ENABLED
    // geometry shader
    status = 0;
    infoLogLength = 0;
    infoLog = NULL;

    const GLchar *geometryShaderSourceCode =
        "#version 410 core\n" \
        "\n" \
        "layout(triangles) in;\n" \
        "in VS_OUT {\n" \
        "   vec3 lightDirection;\n" \
        "   vec3 viewDirection;\n" \
        "   vec3 normal;\n" \
        "} gs_in[];\n"
        "\n" \
        "uniform struct Transformations {\n" \
        "   mat4 model;\n" \
        "   mat4 view;\n" \
        "   mat4 projection;\n" \
        "} transformations;\n" \
        "\n" \
        "uniform bool isViewNormals;\n" \
        "\n" \
        "layout(line_strip, max_vertices = 9) out;\n" \
        "out GS_OUT {\n" \
        "   vec3 lightDirection;\n" \
        "   vec3 viewDirection;\n" \
        "   vec3 normal;\n" \
        "} gs_out;\n" \
        "\n" \
        "void generateVertex(int index) {\n" \
        "   gl_Position = transformations.projection * gl_in[index].gl_Position;\n" \
        "   gs_out.lightDirection = gs_in[index].lightDirection;\n" \
        "   gs_out.viewDirection = gs_in[index].viewDirection;\n" \
        "   gs_out.normal = gs_in[index].normal;\n" \
        "   EmitVertex();\n" \
        "   \n" \
        "   gl_Position = transformations.projection * (gl_in[index].gl_Position + transformations.view * (vec4(0.5) * vec4(gs_in[index].normal, 1.0)));\n" \
        "   gs_out.lightDirection = gs_in[index].lightDirection;\n" \
        "   gs_out.viewDirection = gs_in[index].viewDirection;\n" \
        "   gs_out.normal = gs_in[index].normal;\n" \
        "   EmitVertex();\n" \
        "   \n" \
        "   EndPrimitive();\n" \
        "}\n" \
        "\n" \
        "void main(void)\n" \
        "{\n" \
        "   if(isViewNormals) {\n" \
        "       generateVertex(0);\n" \
        "       generateVertex(1);\n" \
        "       generateVertex(2);\n" \
        "   }\n" \
        "   \n" \
        "   gl_Position = transformations.projection * gl_in[0].gl_Position;\n" \
        "   gs_out.lightDirection = gs_in[0].lightDirection;\n" \
        "   gs_out.viewDirection = gs_in[0].viewDirection;\n" \
        "   gs_out.normal = gs_in[0].normal;\n" \
        "   EmitVertex();\n" \
        "   \n" \
        "   gl_Position = transformations.projection * gl_in[1].gl_Position;\n" \
        "   gs_out.lightDirection = gs_in[1].lightDirection;\n" \
        "   gs_out.viewDirection = gs_in[1].viewDirection;\n" \
        "   gs_out.normal = gs_in[1].normal;\n" \
        "   EmitVertex();\n" \
        "   \n" \
        "   gl_Position = transformations.projection * gl_in[2].gl_Position;\n" \
        "   gs_out.lightDirection = gs_in[2].lightDirection;\n" \
        "   gs_out.viewDirection = gs_in[2].viewDirection;\n" \
        "   gs_out.normal = gs_in[2].normal;\n" \
        "   EmitVertex();\n" \
        "   \n" \
        "   EndPrimitive();\n" \
        "}\n";

    GLuint geometryShaderObject = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geometryShaderObject, 1, (const GLchar **)&geometryShaderSourceCode, NULL);
    glCompileShader(geometryShaderObject);
    glGetShaderiv(geometryShaderObject, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        fprintf(logFile, "*** geometry shader compilation errors ***\n");

        glGetShaderiv(geometryShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            infoLog = (char *)malloc(infoLogLength * sizeof(char));
            if (infoLog)
            {
                GLsizei written = 0;

                glGetShaderInfoLog(geometryShaderObject, infoLogLength * sizeof(char), &written, infoLog);
                fprintf(logFile, "geometry shader compilation log (%d bytes):\n%s\n", written, infoLog);
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
    fprintf(logFile, "geometry shader was compiled without errors\n");
#endif

    // fragment shader
    status = 0;
    infoLogLength = 0;
    infoLog = NULL;

#if NORMAL_VIEW_ENABLED
    const GLchar *fragmentShaderSourceCode =
        "#version 410 core\n" \
        "\n" \
        "in GS_OUT {\n" \
        "   vec3 lightDirection;\n" \
        "   vec3 viewDirection;\n" \
        "   vec3 normal;\n" \
        "} fs_in;\n" \
        "\n" \
        "uniform struct Light {\n" \
        "   vec4 ambient;\n" \
        "   vec4 diffuse;\n" \
        "   vec4 specular;\n" \
        "   vec4 position;\n" \
        "} light;\n" \
        "\n" \
        "uniform struct Material {\n" \
        "   vec4 ambient;\n" \
        "   vec4 diffuse;\n" \
        "   vec4 specular;\n" \
        "   float shininess;\n" \
        "} material;\n" \
        "\n" \
        "uniform bool isLight;\n" \
        "\n" \
        "out vec4 FragColor;\n" \
        "\n" \
        "void main(void)\n" \
        "{\n" \
        "   vec4 ambient = vec4(1.0);\n" \
        "   vec4 diffuse = vec4(0.0);\n" \
        "   vec4 specular = vec4(0.0);\n" \
        "   \n" \
        "   if(isLight) {\n" \
        "       vec3 lightDirection = normalize(fs_in.lightDirection);\n" \
        "       vec3 viewDirection = normalize(fs_in.viewDirection);\n" \
        "       vec3 normal = normalize(fs_in.normal);\n" \
        "       \n" \
        "       vec3 reflectedDirection = reflect(-lightDirection, normal);\n" \
        "       \n" \
        "       ambient = light.ambient * material.ambient;\n" \
        "       diffuse = light.diffuse * material.diffuse * max(dot(normal, lightDirection), 0.0);\n" \
        "       \n" \
        "       /* reflect() seems to return a positive even for antiparallel reflected vector wrt normal;\n" \
        "          the if-statement below is a sad fix atleast until I don't find my mistake... */\n" \
        "       specular = vec4(0.0);\n" \
        "       if(dot(reflectedDirection, normal) > 0.0) {\n" \
        "           float specularFactor = pow(max(dot(viewDirection, reflectedDirection), 0.0), material.shininess);\n" \
        "           specular = light.specular * material.specular * specularFactor;\n" \
        "       }\n" \
        "   }\n" \
        "   FragColor = ambient + diffuse + specular;\n" \
        "}\n";
#else
    const GLchar *fragmentShaderSourceCode =
        "#version 410 core\n" \
        "\n" \
        "in VS_OUT {\n" \
        "   vec3 lightDirection;\n" \
        "   vec3 viewDirection;\n" \
        "   vec3 normal;\n" \
        "} fs_in;\n" \
        "\n" \
        "uniform struct Light {\n" \
        "   vec4 ambient;\n" \
        "   vec4 diffuse;\n" \
        "   vec4 specular;\n" \
        "   vec4 position;\n" \
        "} light;\n" \
        "\n" \
        "uniform struct Material {\n" \
        "   vec4 ambient;\n" \
        "   vec4 diffuse;\n" \
        "   vec4 specular;\n" \
        "   float shininess;\n" \
        "} material;\n" \
        "\n" \
        "uniform bool isLight;\n" \
        "\n" \
        "out vec4 FragColor;\n" \
        "\n" \
        "void main(void)\n" \
        "{\n" \
        "   vec4 ambient = vec4(1.0);\n" \
        "   vec4 diffuse = vec4(0.0);\n" \
        "   vec4 specular = vec4(0.0);\n" \
        "   \n" \
        "   if(isLight) {\n" \
        "       vec3 lightDirection = normalize(fs_in.lightDirection);\n" \
        "       vec3 viewDirection = normalize(fs_in.viewDirection);\n" \
        "       vec3 normal = normalize(fs_in.normal);\n" \
        "       \n" \
        "       vec3 reflectedDirection = reflect(-lightDirection, normal);\n" \
        "       \n" \
        "       ambient = light.ambient * material.ambient;\n" \
        "       diffuse = light.diffuse * material.diffuse * max(dot(normal, lightDirection), 0.0);\n" \
        "       \n" \
        "       /* reflect() seems to return a positive even for antiparallel reflected vector wrt normal;\n" \
        "          the if-statement below is a sad fix atleast until I don't find my mistake... */\n" \
        "       specular = vec4(0.0);\n" \
        "       if(dot(reflectedDirection, normal) > 0.0) {\n" \
        "           float specularFactor = pow(max(dot(viewDirection, reflectedDirection), 0.0), material.shininess);\n" \
        "           specular = light.specular * material.specular * specularFactor;\n" \
        "       }\n" \
        "   }\n" \
        "   FragColor = ambient + diffuse + specular;\n" \
        "}\n";
#endif

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
#if NORMAL_VIEW_ENABLED
    glAttachShader(shaderProgramObject, geometryShaderObject);
#endif
    glBindAttribLocation(shaderProgramObject, KVD_ATTRIBUTE_POSITION, "a_position");
    glBindAttribLocation(shaderProgramObject, KVD_ATTRIBUTE_NORMAL, "a_normal");
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
    uniforms.isLight = glGetUniformLocation(shaderProgramObject, "isLight");
    uniforms.transformations.model = glGetUniformLocation(shaderProgramObject, "transformations.model");
    uniforms.transformations.view = glGetUniformLocation(shaderProgramObject, "transformations.view");
    uniforms.transformations.projection = glGetUniformLocation(shaderProgramObject, "transformations.projection");
    uniforms.light.ambient = glGetUniformLocation(shaderProgramObject, "light.ambient");
    uniforms.light.diffuse = glGetUniformLocation(shaderProgramObject, "light.diffuse");
    uniforms.light.specular = glGetUniformLocation(shaderProgramObject, "light.specular");
    uniforms.light.position = glGetUniformLocation(shaderProgramObject, "light.position");
    uniforms.material.ambient = glGetUniformLocation(shaderProgramObject, "material.ambient");
    uniforms.material.diffuse = glGetUniformLocation(shaderProgramObject, "material.diffuse");
    uniforms.material.specular = glGetUniformLocation(shaderProgramObject, "material.specular");
    uniforms.material.shininess = glGetUniformLocation(shaderProgramObject, "material.shininess");
#if NORMAL_VIEW_ENABLED
    uniforms.isViewNormals = glGetUniformLocation(shaderProgramObject, "isViewNormals");
#endif
    uniforms.viewerPosition = glGetUniformLocation(shaderProgramObject, "viewerPosition");

    /* generating vertices and loading vertex data into pipeline */
    sphere = [[OpenGLSphere alloc] initWithSlicesAndStacks: 25 : 25];
    [sphere setPositionAttribLocation: KVD_ATTRIBUTE_POSITION];
    [sphere setNormalAttribLocation: KVD_ATTRIBUTE_NORMAL];
    [sphere generate];
    [sphere loadVerticesIntoOpenGLPipeline];
    
    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
    
    glClearDepth(1.0f);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    
    transformations.projection = vmath::mat4::identity();
    theta = 0.0f;
    axis = 0;
    
    // initialize lighting
    isLight = false;
    isViewNormals = false;
    
    light.ambient =  vmath::vec4(0.1f, 0.1f, 0.1f, 1.0f);
    light.diffuse = vmath::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    light.specular = vmath::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    light.position = vmath::vec4(100.0f, 100.0f, 100.0f, 1.0f);
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
    
    windowWidth = width;
    windowHeight = height;
}

-(void) renderScene
{
    // code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(isLight)
    {
        switch(axis)
        {
        // default
        case 0:
            light.position = vmath::vec4(0.0f, 0.0f, 5.0f, 1.0f);
            break;
            
        // rotate light about X-axis
        case 1:
            light.position = vmath::vec4(
              0.0f,
              5.0f * cosf(vmath::radians(theta)),
              5.0f * sinf(vmath::radians(theta)),
              1.0f);
            break;
            
        // rotate light about Y-axis
        case 2:
            light.position = vmath::vec4(
              5.0f * sinf(vmath::radians(theta)),
              0.0f,
              5.0f * cosf(vmath::radians(theta)),
              1.0f);
            break;
            
        // rotate light about Z-axis
        case 3:
            light.position = vmath::vec4(
              5.0f * cosf(vmath::radians(theta)),
              5.0f * sinf(vmath::radians(theta)),
              0.0f,
              1.0f);
            break;
        }
    }
    [self draw24Spheres];
}

-(void) updateScene
{
    // code
    theta += 1.0f;
    if(theta > 360.0f)
        theta -= 360.0f;
}

-(void) uninitialize
{
    // code
    if(sphere)
    {
        [sphere dealloc];
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

-(void) draw24Spheres
{
    // constants
    const vmath::vec3 viewerPosition = vmath::vec3(0.0f, 0.0f, 2.0f);
    int viewportWidth = windowWidth / 4;
    int viewportHeight = windowHeight / 6;
    float aspectRatio = (float)viewportWidth / (float)viewportHeight;
    
    vmath::vec3 ka, kd, ks;
    float shininess;
    
    // code
    glUseProgram(shaderProgramObject);
    {
        // transforms
        transformations.model = vmath::translate(0.0f, 0.0f, -2.0f);
        glUniformMatrix4fv(uniforms.transformations.model, 1, GL_FALSE, transformations.model);
        
        transformations.view = vmath::lookat(viewerPosition, vmath::vec3(0.0f, 0.0f, 0.0f), vmath::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(uniforms.transformations.view, 1, GL_FALSE, transformations.view);
        
        transformations.projection = vmath::perspective(45.0f, aspectRatio, 0.1f, 100.0f);
        glUniformMatrix4fv(uniforms.transformations.projection, 1, GL_FALSE, transformations.projection);
        
        // lighting uniforms
        if(isLight) {
            glUniform1i(uniforms.isLight, GL_TRUE);
            glUniform4fv(uniforms.viewerPosition, 1, vmath::vec4(viewerPosition, 1.0));
            
            glUniform4fv(uniforms.light.ambient, 1, light.ambient);
            glUniform4fv(uniforms.light.diffuse, 1, light.diffuse);
            glUniform4fv(uniforms.light.specular, 1, light.specular);
            glUniform4fv(uniforms.light.position, 1, light.position);
            
            // [1, 1] material: emerald
            glViewport(0, windowHeight - viewportHeight, (GLsizei)viewportWidth, (GLsizei)viewportHeight);
            ka = vmath::vec3(0.0215f, 0.1745f, 0.0215f);
            kd = vmath::vec3(0.07568f, 0.61424f, 0.07568f);
            ks = vmath::vec3(0.633f, 0.727811f, 0.633f);
            shininess = 0.6f * 128.0f;
            glUniform4fv(uniforms.material.ambient, 1, vmath::vec4(ka, 1.0f));
            glUniform4fv(uniforms.material.diffuse, 1, vmath::vec4(kd, 1.0f));
            glUniform4fv(uniforms.material.specular, 1, vmath::vec4(ks, 1.0f));
            glUniform1f(uniforms.material.shininess, shininess);
            [sphere render: GL_TRIANGLES];
            
            // [2, 1] material: jade
            glViewport(0, windowHeight - 2 * viewportHeight, (GLsizei)viewportWidth, (GLsizei)viewportHeight);
            ka = vmath::vec3(0.135f, 0.2225f, 0.1575f);
            kd = vmath::vec3(0.54f, 0.89f, 0.63f);
            ks = vmath::vec3(0.316228f, 0.316228f, 0.316228f);
            shininess = 0.1f * 128.0f;
            glUniform4fv(uniforms.material.ambient, 1, vmath::vec4(ka, 1.0f));
            glUniform4fv(uniforms.material.diffuse, 1, vmath::vec4(kd, 1.0f));
            glUniform4fv(uniforms.material.specular, 1, vmath::vec4(ks, 1.0f));
            glUniform1f(uniforms.material.shininess, shininess);
            [sphere render: GL_TRIANGLES];
            
            // [3, 1] material: obsidian
            glViewport(0, windowHeight - 3 * viewportHeight, (GLsizei)viewportWidth, (GLsizei)viewportHeight);
            ka = vmath::vec3(0.05375f, 0.05f, 0.06625f);
            kd = vmath::vec3(0.18275f, 0.17f, 0.22525f);
            ks = vmath::vec3(0.332741f, 0.328634f, 0.346435f);
            shininess = 0.3f * 128.0f;
            glUniform4fv(uniforms.material.ambient, 1, vmath::vec4(ka, 1.0f));
            glUniform4fv(uniforms.material.diffuse, 1, vmath::vec4(kd, 1.0f));
            glUniform4fv(uniforms.material.specular, 1, vmath::vec4(ks, 1.0f));
            glUniform1f(uniforms.material.shininess, shininess);
            [sphere render: GL_TRIANGLES];
            
            // [4, 1] material: pearl
            glViewport(0, windowHeight - 4 * viewportHeight, (GLsizei)viewportWidth, (GLsizei)viewportHeight);
            ka = vmath::vec3(0.25f, 0.20725f, 0.20725f);
            kd = vmath::vec3(1.0f, 0.829f, 0.829f);
            ks = vmath::vec3(0.296648f, 0.296648f, 0.296648f);
            shininess = 0.3f * 128.0f;
            glUniform4fv(uniforms.material.ambient, 1, vmath::vec4(ka, 1.0f));
            glUniform4fv(uniforms.material.diffuse, 1, vmath::vec4(kd, 1.0f));
            glUniform4fv(uniforms.material.specular, 1, vmath::vec4(ks, 1.0f));
            glUniform1f(uniforms.material.shininess, shininess);
            [sphere render: GL_TRIANGLES];
            
            // [5, 1] material: ruby
            glViewport(0, windowHeight - 5 * viewportHeight, (GLsizei)viewportWidth, (GLsizei)viewportHeight);
            ka = vmath::vec3(0.1745f, 0.01175f, 0.01175f);
            kd = vmath::vec3(0.61424f, 0.04136f, 0.04136f);
            ks = vmath::vec3(0.727811f, 0.727811f, 0.727811f);
            shininess = 0.6f * 128.0f;
            glUniform4fv(uniforms.material.ambient, 1, vmath::vec4(ka, 1.0f));
            glUniform4fv(uniforms.material.diffuse, 1, vmath::vec4(kd, 1.0f));
            glUniform4fv(uniforms.material.specular, 1, vmath::vec4(ks, 1.0f));
            glUniform1f(uniforms.material.shininess, shininess);
            [sphere render: GL_TRIANGLES];
            
            // [6, 1] material: turquoise
            glViewport(0, windowHeight - 6 * viewportHeight, (GLsizei)viewportWidth, (GLsizei)viewportHeight);
            ka = vmath::vec3(0.1f, 0.18725f, 0.1745f);
            kd = vmath::vec3(0.396f, 0.74151f, 0.69102f);
            ks = vmath::vec3(0.297254f, 0.30829f, 0.306678f);
            shininess = 0.1f * 128.0f;
            glUniform4fv(uniforms.material.ambient, 1, vmath::vec4(ka, 1.0f));
            glUniform4fv(uniforms.material.diffuse, 1, vmath::vec4(kd, 1.0f));
            glUniform4fv(uniforms.material.specular, 1, vmath::vec4(ks, 1.0f));
            glUniform1f(uniforms.material.shininess, shininess);
            [sphere render: GL_TRIANGLES];
            
            // [1, 2] material: brass
            glViewport(viewportWidth, windowHeight - viewportHeight, (GLsizei)viewportWidth, (GLsizei)viewportHeight);
            ka = vmath::vec3(0.329412f, 0.223529f, 0.027451f);
            kd = vmath::vec3(0.780392f, 0.568627f, 0.113725f);
            ks = vmath::vec3(0.992157f, 0.941176f, 0.807843f);
            shininess = 0.21794872f * 128.0f;
            glUniform4fv(uniforms.material.ambient, 1, vmath::vec4(ka, 1.0f));
            glUniform4fv(uniforms.material.diffuse, 1, vmath::vec4(kd, 1.0f));
            glUniform4fv(uniforms.material.specular, 1, vmath::vec4(ks, 1.0f));
            glUniform1f(uniforms.material.shininess, shininess);
            [sphere render: GL_TRIANGLES];
            
            // [2, 2] material: bronze
            glViewport(viewportWidth, windowHeight - 2 * viewportHeight, (GLsizei)viewportWidth, (GLsizei)viewportHeight);
            ka = vmath::vec3(0.2125f, 0.1275f, 0.054f);
            kd = vmath::vec3(0.714f, 0.4284f, 0.18144f);
            ks = vmath::vec3(0.393548f, 0.271906f, 0.166721f);
            shininess = 0.2f * 128.0f;
            glUniform4fv(uniforms.material.ambient, 1, vmath::vec4(ka, 1.0f));
            glUniform4fv(uniforms.material.diffuse, 1, vmath::vec4(kd, 1.0f));
            glUniform4fv(uniforms.material.specular, 1, vmath::vec4(ks, 1.0f));
            glUniform1f(uniforms.material.shininess, shininess);
            [sphere render: GL_TRIANGLES];
            
            // [3, 2] material: chrome
            glViewport(viewportWidth, windowHeight - 3 * viewportHeight, (GLsizei)viewportWidth, (GLsizei)viewportHeight);
            ka = vmath::vec3(0.25f, 0.25f, 0.25f);
            kd = vmath::vec3(0.4f, 0.4f, 0.4f);
            ks = vmath::vec3(0.774597f, 0.774597f, 0.774597f);
            shininess = 0.6f * 128.0f;
            glUniform4fv(uniforms.material.ambient, 1, vmath::vec4(ka, 1.0f));
            glUniform4fv(uniforms.material.diffuse, 1, vmath::vec4(kd, 1.0f));
            glUniform4fv(uniforms.material.specular, 1, vmath::vec4(ks, 1.0f));
            glUniform1f(uniforms.material.shininess, shininess);
            [sphere render: GL_TRIANGLES];
            
            // [4, 2] material: copper
            glViewport(viewportWidth, windowHeight - 4 * viewportHeight, (GLsizei)viewportWidth, (GLsizei)viewportHeight);
            ka = vmath::vec3(0.19125f, 0.0735f, 0.0225f);
            kd = vmath::vec3(0.7038f, 0.27048f, 0.0828f);
            ks = vmath::vec3(0.256777f, 0.137622f, 0.086014f);
            shininess = 0.1f * 128.0f;
            glUniform4fv(uniforms.material.ambient, 1, vmath::vec4(ka, 1.0f));
            glUniform4fv(uniforms.material.diffuse, 1, vmath::vec4(kd, 1.0f));
            glUniform4fv(uniforms.material.specular, 1, vmath::vec4(ks, 1.0f));
            glUniform1f(uniforms.material.shininess, shininess);
            [sphere render: GL_TRIANGLES];
            
            // [5, 2] material: gold
            glViewport(viewportWidth, windowHeight - 5 * viewportHeight, (GLsizei)viewportWidth, (GLsizei)viewportHeight);
            ka = vmath::vec3(0.24725f, 0.1995f, 0.0745f);
            kd = vmath::vec3(0.75164f, 0.60648f, 0.22648f);
            ks = vmath::vec3(0.628281f, 0.555802f, 0.366065f);
            shininess = 0.4f * 128.0f;
            glUniform4fv(uniforms.material.ambient, 1, vmath::vec4(ka, 1.0f));
            glUniform4fv(uniforms.material.diffuse, 1, vmath::vec4(kd, 1.0f));
            glUniform4fv(uniforms.material.specular, 1, vmath::vec4(ks, 1.0f));
            glUniform1f(uniforms.material.shininess, shininess);
            [sphere render: GL_TRIANGLES];
            
            // [6, 2] material: silver
            glViewport(viewportWidth, windowHeight - 6 * viewportHeight, (GLsizei)viewportWidth, (GLsizei)viewportHeight);
            ka = vmath::vec3(0.19225f, 0.19225f, 0.19225f);
            kd = vmath::vec3(0.50754f, 0.50754f, 0.50754f);
            ks = vmath::vec3(0.508273f, 0.508273f, 0.508273f);
            shininess = 0.4f * 128.0f;
            glUniform4fv(uniforms.material.ambient, 1, vmath::vec4(ka, 1.0f));
            glUniform4fv(uniforms.material.diffuse, 1, vmath::vec4(kd, 1.0f));
            glUniform4fv(uniforms.material.specular, 1, vmath::vec4(ks, 1.0f));
            glUniform1f(uniforms.material.shininess, shininess);
            [sphere render: GL_TRIANGLES];
            
            // [1, 3] material: black plastic
            glViewport(2 * viewportWidth, windowHeight - viewportHeight, (GLsizei)viewportWidth, (GLsizei)viewportHeight);
            ka = vmath::vec3(0.0f, 0.0f, 0.0f);
            kd = vmath::vec3(0.01f, 0.01f, 0.01f);
            ks = vmath::vec3(0.5f, 0.5f, 0.5f);
            shininess = 0.25f * 128.0f;
            glUniform4fv(uniforms.material.ambient, 1, vmath::vec4(ka, 1.0f));
            glUniform4fv(uniforms.material.diffuse, 1, vmath::vec4(kd, 1.0f));
            glUniform4fv(uniforms.material.specular, 1, vmath::vec4(ks, 1.0f));
            glUniform1f(uniforms.material.shininess, shininess);
            [sphere render: GL_TRIANGLES];
            
            // [2, 3] material: cyan plastic
            glViewport(2 * viewportWidth, windowHeight - 2 * viewportHeight, (GLsizei)viewportWidth, (GLsizei)viewportHeight);
            ka = vmath::vec3(0.0f, 0.1f, 0.06f);
            kd = vmath::vec3(0.0f, 0.50980392f, 0.50980392f);
            ks = vmath::vec3(0.50196078f, 0.50196078f, 0.50196078f);
            shininess = 0.25f * 128.0f;
            glUniform4fv(uniforms.material.ambient, 1, vmath::vec4(ka, 1.0f));
            glUniform4fv(uniforms.material.diffuse, 1, vmath::vec4(kd, 1.0f));
            glUniform4fv(uniforms.material.specular, 1, vmath::vec4(ks, 1.0f));
            glUniform1f(uniforms.material.shininess, shininess);
            [sphere render: GL_TRIANGLES];
            
            // [3, 3] material: green plastic
            glViewport(2 * viewportWidth, windowHeight - 3 * viewportHeight, (GLsizei)viewportWidth, (GLsizei)viewportHeight);
            ka = vmath::vec3(0.0f, 0.0f, 0.0f);
            kd = vmath::vec3(0.1f, 0.35f, 0.1f);
            ks = vmath::vec3(0.45f, 0.55f, 0.45f);
            shininess = 0.25f * 128.0f;
            glUniform4fv(uniforms.material.ambient, 1, vmath::vec4(ka, 1.0f));
            glUniform4fv(uniforms.material.diffuse, 1, vmath::vec4(kd, 1.0f));
            glUniform4fv(uniforms.material.specular, 1, vmath::vec4(ks, 1.0f));
            glUniform1f(uniforms.material.shininess, shininess);
            [sphere render: GL_TRIANGLES];
            
            // [4, 3] material: red plastic
            glViewport(2 * viewportWidth, windowHeight - 4 * viewportHeight, (GLsizei)viewportWidth, (GLsizei)viewportHeight);
            ka = vmath::vec3(0.0f, 0.0f, 0.0f);
            kd = vmath::vec3(0.5f, 0.0f, 0.0f);
            ks = vmath::vec3(0.7f, 0.6f, 0.6f);
            shininess = 0.25f * 128.0f;
            glUniform4fv(uniforms.material.ambient, 1, vmath::vec4(ka, 1.0f));
            glUniform4fv(uniforms.material.diffuse, 1, vmath::vec4(kd, 1.0f));
            glUniform4fv(uniforms.material.specular, 1, vmath::vec4(ks, 1.0f));
            glUniform1f(uniforms.material.shininess, shininess);
            [sphere render: GL_TRIANGLES];
            
            // [5, 3] material: white plastic
            glViewport(2 * viewportWidth, windowHeight - 5 * viewportHeight, (GLsizei)viewportWidth, (GLsizei)viewportHeight);
            ka = vmath::vec3(0.0f, 0.0f, 0.0f);
            kd = vmath::vec3(0.55f, 0.55f, 0.55f);
            ks = vmath::vec3(0.7f, 0.7f, 0.7f);
            shininess = 0.25f * 128.0f;
            glUniform4fv(uniforms.material.ambient, 1, vmath::vec4(ka, 1.0f));
            glUniform4fv(uniforms.material.diffuse, 1, vmath::vec4(kd, 1.0f));
            glUniform4fv(uniforms.material.specular, 1, vmath::vec4(ks, 1.0f));
            glUniform1f(uniforms.material.shininess, shininess);
            [sphere render: GL_TRIANGLES];
            
            // [6, 3] material: white plastic
            glViewport(2 * viewportWidth, windowHeight - 6 * viewportHeight, (GLsizei)viewportWidth, (GLsizei)viewportHeight);
            ka = vmath::vec3(0.0f, 0.0f, 0.0f);
            kd = vmath::vec3(0.5f, 0.5f, 0.0f);
            ks = vmath::vec3(0.6f, 0.6f, 0.5f);
            shininess = 0.25f * 128.0f;
            glUniform4fv(uniforms.material.ambient, 1, vmath::vec4(ka, 1.0f));
            glUniform4fv(uniforms.material.diffuse, 1, vmath::vec4(kd, 1.0f));
            glUniform4fv(uniforms.material.specular, 1, vmath::vec4(ks, 1.0f));
            glUniform1f(uniforms.material.shininess, shininess);
            [sphere render: GL_TRIANGLES];
            
            // [1, 4] material: black rubber
            glViewport(3 * viewportWidth, windowHeight - viewportHeight, (GLsizei)viewportWidth, (GLsizei)viewportHeight);
            ka = vmath::vec3(0.02f, 0.02f, 0.02f);
            kd = vmath::vec3(0.01f, 0.01f, 0.01f);
            ks = vmath::vec3(0.4f, 0.4f, 0.4f);
            shininess = 0.078125f * 128.0f;
            glUniform4fv(uniforms.material.ambient, 1, vmath::vec4(ka, 1.0f));
            glUniform4fv(uniforms.material.diffuse, 1, vmath::vec4(kd, 1.0f));
            glUniform4fv(uniforms.material.specular, 1, vmath::vec4(ks, 1.0f));
            glUniform1f(uniforms.material.shininess, shininess);
            [sphere render: GL_TRIANGLES];
            
            // [2, 4] material: cyan rubber
            glViewport(3 * viewportWidth, windowHeight - 2 * viewportHeight, (GLsizei)viewportWidth, (GLsizei)viewportHeight);
            ka = vmath::vec3(0.0f, 0.05f, 0.05f);
            kd = vmath::vec3(0.4f, 0.5f, 0.5f);
            ks = vmath::vec3(0.04f, 0.7f, 0.7f);
            shininess = 0.078125f * 128.0f;
            glUniform4fv(uniforms.material.ambient, 1, vmath::vec4(ka, 1.0f));
            glUniform4fv(uniforms.material.diffuse, 1, vmath::vec4(kd, 1.0f));
            glUniform4fv(uniforms.material.specular, 1, vmath::vec4(ks, 1.0f));
            glUniform1f(uniforms.material.shininess, shininess);
            [sphere render: GL_TRIANGLES];
            
            // [3, 4] material: green rubber
            glViewport(3 * viewportWidth, windowHeight - 3 * viewportHeight, (GLsizei)viewportWidth, (GLsizei)viewportHeight);
            ka = vmath::vec3(0.0f, 0.05f, 0.05f);
            kd = vmath::vec3(0.4f, 0.5f, 0.4f);
            ks = vmath::vec3(0.04f, 0.7f, 0.04f);
            shininess = 0.078125f * 128.0f;
            glUniform4fv(uniforms.material.ambient, 1, vmath::vec4(ka, 1.0f));
            glUniform4fv(uniforms.material.diffuse, 1, vmath::vec4(kd, 1.0f));
            glUniform4fv(uniforms.material.specular, 1, vmath::vec4(ks, 1.0f));
            glUniform1f(uniforms.material.shininess, shininess);
            [sphere render: GL_TRIANGLES];
            
            // [4, 4] material: red rubber
            glViewport(3 * viewportWidth, windowHeight - 4 * viewportHeight, (GLsizei)viewportWidth, (GLsizei)viewportHeight);
            ka = vmath::vec3(0.05f, 0.0f, 0.0f);
            kd = vmath::vec3(0.5f, 0.4f, 0.4f);
            ks = vmath::vec3(0.7f, 0.04f, 0.04f);
            shininess = 0.078125f * 128.0f;
            glUniform4fv(uniforms.material.ambient, 1, vmath::vec4(ka, 1.0f));
            glUniform4fv(uniforms.material.diffuse, 1, vmath::vec4(kd, 1.0f));
            glUniform4fv(uniforms.material.specular, 1, vmath::vec4(ks, 1.0f));
            glUniform1f(uniforms.material.shininess, shininess);
            [sphere render: GL_TRIANGLES];
            
            // [5, 4] material: red rubber
            glViewport(3 * viewportWidth, windowHeight - 5 * viewportHeight, (GLsizei)viewportWidth, (GLsizei)viewportHeight);
            ka = vmath::vec3(0.05f, 0.05f, 0.05f);
            kd = vmath::vec3(0.5f, 0.5f, 0.5f);
            ks = vmath::vec3(0.7f, 0.7f, 0.7f);
            shininess = 0.078125f * 128.0f;
            glUniform4fv(uniforms.material.ambient, 1, vmath::vec4(ka, 1.0f));
            glUniform4fv(uniforms.material.diffuse, 1, vmath::vec4(kd, 1.0f));
            glUniform4fv(uniforms.material.specular, 1, vmath::vec4(ks, 1.0f));
            glUniform1f(uniforms.material.shininess, shininess);
            [sphere render: GL_TRIANGLES];
            
            // [6, 4] material: yellow rubber
            glViewport(3 * viewportWidth, windowHeight - 6 * viewportHeight, (GLsizei)viewportWidth, (GLsizei)viewportHeight);
            ka = vmath::vec3(0.05f, 0.05f, 0.0f);
            kd = vmath::vec3(0.5f, 0.5f, 0.4f);
            ks = vmath::vec3(0.7f, 0.7f, 0.04f);
            shininess = 0.078125f * 128.0f;
            glUniform4fv(uniforms.material.ambient, 1, vmath::vec4(ka, 1.0f));
            glUniform4fv(uniforms.material.diffuse, 1, vmath::vec4(kd, 1.0f));
            glUniform4fv(uniforms.material.specular, 1, vmath::vec4(ks, 1.0f));
            glUniform1f(uniforms.material.shininess, shininess);
            [sphere render: GL_TRIANGLES];
        } else {
            glUniform1i(uniforms.isLight, GL_FALSE);
            
            for(int x = 0; x < 4; x++)
            {
                for(int y = 1; y <= 6; y++)
                {
                    glViewport(x * viewportWidth, windowHeight - y * viewportHeight, (GLsizei)viewportWidth, (GLsizei)viewportHeight);
                    [sphere render: GL_TRIANGLES];
                }
            }
        }
#if NORMAL_VIEW_ENABLED
        if(isViewNormals) {
            glUniform1i(uniforms.isViewNormals, GL_TRUE);
        } else {
            glUniform1i(uniforms.isViewNormals, GL_FALSE);
        }
#endif
    }
    glUseProgram(0);
}
@end

/* implementing the DisplayLinkCallback */
CVReturn KVDDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *currentTime, const CVTimeStamp *outputTime, CVOptionFlags flagsIn, CVOptionFlags *flagsOut, void *view)
{
    // called by QuartzCore (the CoreVideo driver)
    // code
    return [(GLView *)view getFrameForTime: outputTime];
}
