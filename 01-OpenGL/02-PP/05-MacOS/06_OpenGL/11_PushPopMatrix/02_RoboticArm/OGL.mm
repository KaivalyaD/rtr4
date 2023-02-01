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
#define MAX_STACK_SIZE 32

#define max(x, y) ((x > y) ? (x) : (y))
#define min(x, y) ((x < y) ? (x) : (y))

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

    GLuint vaoCuboids;
    GLuint vboCuboidPositions;
    GLuint vboCuboidNormals;
    
    struct Transformations {
        vmath::mat4 model;
        vmath::mat4 view;
        vmath::mat4 projection;
    } transformations;
    
    struct Light {
        vmath::vec4 diffuse;
        vmath::vec4 position;
    } light;
    
    struct Material {
        vmath::vec4 diffuse;
        float shininess;
    } material;
    
    struct lightingShaderUniforms {
        struct {
            GLuint model;  // mat4
            GLuint view;  // mat4
            GLuint projection;  // mat4
        } transformations;
        struct {
            GLuint diffuse;  // vec4
            GLuint position;  // vec4
        } light;
        struct {
            GLuint diffuse;  // vec4
            GLuint shininess;  // float
        } material;
        GLuint isLight;  // bool
    } uniforms;
    
    enum {
        KVD_ATTRIBUTE_POSITION = 0,
        KVD_ATTRIBUTE_COLOR,
        KVD_ATTRIBUTE_NORMAL,
        KVD_ATTRIBUTE_TEXTURE0
    };
    
    // arm controls
    float shoulder_yaw, shoulder_pitch;
    float elbow_pitch, elbow_roll;
    float wrist_pitch, wrist_roll;
    float finger_a, finger_b;
    
    bool isLight;
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
            
    case 'L':
    case 'l':
        isLight = !isLight;
        break;
            
    case '1':
        shoulder_yaw = (shoulder_yaw + 3.0f);
        break;
    case '!':
        shoulder_yaw = (shoulder_yaw - 3.0f);
        break;

    case '2':
        shoulder_pitch = (shoulder_pitch + 3.0f);
        break;
    case '@':
        shoulder_pitch = (shoulder_pitch - 3.0f);
        break;

    case '3':
        elbow_pitch = (elbow_pitch + 3.0f);
        break;
    case '#':
        elbow_pitch = (elbow_pitch - 3.0f);
        break;

    case '4':
        elbow_roll = (elbow_roll + 3.0f);
        break;
    case '$':
        elbow_roll = (elbow_roll - 3.0f);
        break;

    case '5':
        wrist_pitch = (wrist_pitch + 3.0f);
        break;
    case '%':
        wrist_pitch = (wrist_pitch - 3.0f);
        break;

    case '6':
        wrist_roll = (wrist_roll + 3.0f);
        break;
    case '^':
        wrist_roll = (wrist_roll - 3.0f);
        break;

    case '7':
        finger_a = min(finger_a + 0.001f, 0.04f);
        break;
    case '&':
        finger_a = max(finger_a - 0.001f, 0.0f);
        break;

    case '8':
        finger_b = min(finger_b + 0.001f, 0.04f);
        break;
    case '*':
        finger_b = max(finger_b - 0.001f, 0.0f);
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
        "in vec3 a_normal;\n" \
        "\n" \
        "uniform struct Transformations {\n" \
        "   mat4 model;\n" \
        "   mat4 view;\n" \
        "   mat4 projection;\n" \
        "} transformations;\n" \
        "\n" \
        "uniform struct Light {\n" \
        "   vec4 diffuse;\n" \
        "   vec4 position;\n" \
        "} light;\n" \
        "\n" \
        "uniform struct Material {\n" \
        "   vec4 diffuse;\n" \
        "   float shininess;\n" \
        "} material;\n" \
        "\n" \
        "uniform bool isLight;\n" \
        "\n" \
        "out VS_OUT {\n" \
        "   vec4 color;\n" \
        "} vs_out;\n" \
        "\n" \
        "void main(void)\n" \
        "{\n" \
        "   mat4 modelViewMatrix = transformations.view * transformations.model;\n" \
        "   \n" \
        "   gl_Position = transformations.projection * modelViewMatrix * a_position;\n" \
        "   if(isLight) {\n" \
        "       mat3 normalMatrix = mat3(transpose(inverse(modelViewMatrix)));\n" \
        "       vec4 eyeCoordinate = modelViewMatrix * a_position;\n"
        "       \n" \
        "       vec3 lightDirection = normalize(light.position - eyeCoordinate).xyz;\n"
        "       vec3 transformedNormal = normalize(normalMatrix * a_normal);\n" \
        "       vs_out.color = light.diffuse * material.diffuse * max(dot(transformedNormal, lightDirection), 0.0);\n" \
        "   } else {\n" \
        "       vs_out.color = vec4(1.0);\n" \
        "   }\n" \
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
        "in VS_OUT {\n" \
        "   vec4 color;\n" \
        "} fs_in;\n" \
        "\n" \
        "out vec4 FragColor;\n" \
        "\n" \
        "void main(void)\n" \
        "{\n" \
            "FragColor = fs_in.color;\n" \
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
    uniforms.light.diffuse = glGetUniformLocation(shaderProgramObject, "light.diffuse");
    uniforms.light.position = glGetUniformLocation(shaderProgramObject, "light.position");
    uniforms.material.diffuse = glGetUniformLocation(shaderProgramObject, "material.diffuse");
    uniforms.material.shininess = glGetUniformLocation(shaderProgramObject, "material.shininess");

    /* loading vertex & color data into pipeline */
    // sphere
    sphere = [[OpenGLSphere alloc] initWithSlicesAndStacks: 25 : 25];
    [sphere setPositionAttribLocation: KVD_ATTRIBUTE_POSITION];
    [sphere setNormalAttribLocation: KVD_ATTRIBUTE_NORMAL];
    [sphere generate];
    [sphere loadVerticesIntoOpenGLPipeline];
    
    // cuboids
    const vmath::vec3 cuboidDimensions[] = {
        //           w  ,  h  ,  d      //
        vmath::vec3(0.1f, 1.5f, 0.1f),  // forearm
        vmath::vec3(0.1f, 1.0f, 0.1f),  // arm
        vmath::vec3(0.2f, 0.2f, 0.1f),  // wrist
        vmath::vec3(0.05f, 0.1f, 0.1f)  // fingers
    };
    vaoCuboids = [self generateCuboids: &vboCuboidPositions : &vboCuboidNormals : sizeof(cuboidDimensions) / sizeof(cuboidDimensions[0]) : cuboidDimensions];
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    glClearDepth(1.0f);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    
    transformations.projection = vmath::mat4::identity();
    
    // initialize the stack
    stackMatModelView = [[Mat4Stack alloc] init];
    
    // initialize lighting
    isLight = false;
    light.diffuse = vmath::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    light.position = vmath::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    material.diffuse = vmath::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    material.shininess = 128.0f;
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
    transformations.projection = vmath::perspective(
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
        // transforms
        transformations.view = vmath::mat4::identity();
        
        // lighting uniforms
        glUniform1i(uniforms.isLight, GL_TRUE);
        glUniform4fv(uniforms.light.diffuse, 1, light.diffuse);
        glUniform4fv(uniforms.light.position, 1, light.position);
        glUniform4fv(uniforms.material.diffuse, 1, material.diffuse);
        glUniform1f(uniforms.material.shininess, material.shininess);
        
        // transform uniforms
        glUniformMatrix4fv(uniforms.transformations.view, 1, GL_FALSE, transformations.view);
        glUniformMatrix4fv(uniforms.transformations.projection, 1, GL_FALSE, transformations.projection);
        
        // draw
        // push initial transformation
        [stackMatModelView push: vmath::translate(0.0f, 0.0f, -4.0f)];
        {
            // push forearm transformation
            [stackMatModelView push:
                              vmath::translate(0.0f, 1.4f, 0.0f) *
                              vmath::rotate(shoulder_yaw, 0.0f, 1.0f, 0.0f) *
                              vmath::rotate(shoulder_pitch, 1.0f, 0.0f, 0.0f)];
            {
                // draw shoulder
                glUniformMatrix4fv(uniforms.transformations.model, 1, GL_FALSE, [stackMatModelView top] * vmath::scale(0.1f));
                [sphere render: GL_TRIANGLES];
                
                // draw forearm
                glUniformMatrix4fv(uniforms.transformations.model, 1, GL_FALSE, [stackMatModelView top]);
                glBindVertexArray(vaoCuboids);
                    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
                    glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
                    glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
                    glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
                    glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
                    glDrawArrays(GL_TRIANGLE_FAN, 20, 4);
                glBindVertexArray(0);
                
                // push arm transformation
                [stackMatModelView push:
                                  vmath::translate(0.0f, -1.5f, 0.0f) *
                                  vmath::rotate(elbow_pitch, 0.0f, 1.0f, 0.0f) *
                                  vmath::rotate(elbow_roll, 1.0f, 0.0f, 0.0f)];
                {
                    // draw elbow
                    glUniformMatrix4fv(uniforms.transformations.model, 1, GL_FALSE, [stackMatModelView top] * vmath::scale(0.1f));
                    [sphere render: GL_TRIANGLES];
                    
                    // draw arm
                    glUniformMatrix4fv(uniforms.transformations.model, 1, GL_FALSE, [stackMatModelView top]);
                    glBindVertexArray(vaoCuboids);
                        glDrawArrays(GL_TRIANGLE_FAN, 24, 4);
                        glDrawArrays(GL_TRIANGLE_FAN, 28, 4);
                        glDrawArrays(GL_TRIANGLE_FAN, 32, 4);
                        glDrawArrays(GL_TRIANGLE_FAN, 36, 4);
                        glDrawArrays(GL_TRIANGLE_FAN, 40, 4);
                        glDrawArrays(GL_TRIANGLE_FAN, 44, 4);
                    glBindVertexArray(0);
                    
                    // push palm transformation
                    [stackMatModelView push:
                                      vmath::translate(0.0f, -1.0f, 0.0f) *
                                      vmath::rotate(wrist_pitch, 0.0f, 1.0f, 0.0f) *
                                      vmath::rotate(wrist_roll, 1.0f, 0.0f, 0.0f)];
                    {
                        // draw wrist
                        glUniformMatrix4fv(uniforms.transformations.model, 1, GL_FALSE, [stackMatModelView top] * vmath::scale(0.1f));
                        [sphere render: GL_TRIANGLES];
                        
                        // draw palm
                        glUniformMatrix4fv(uniforms.transformations.model, 1, GL_FALSE, [stackMatModelView top]);
                        glBindVertexArray(vaoCuboids);
                            glDrawArrays(GL_TRIANGLE_FAN, 48, 4);
                            glDrawArrays(GL_TRIANGLE_FAN, 52, 4);
                            glDrawArrays(GL_TRIANGLE_FAN, 56, 4);
                            glDrawArrays(GL_TRIANGLE_FAN, 60, 4);
                            glDrawArrays(GL_TRIANGLE_FAN, 64, 4);
                            glDrawArrays(GL_TRIANGLE_FAN, 68, 4);
                        glBindVertexArray(0);
                        
                        // push finger 'a' transformation
                        [stackMatModelView push: vmath::translate(-0.08f + finger_a, -0.2f, 0.0f)];
                        {
                            // draw finger 'a'
                            glUniformMatrix4fv(uniforms.transformations.model, 1, GL_FALSE, [stackMatModelView top]);
                            glBindVertexArray(vaoCuboids);
                                glDrawArrays(GL_TRIANGLE_FAN, 72, 4);
                                glDrawArrays(GL_TRIANGLE_FAN, 76, 4);
                                glDrawArrays(GL_TRIANGLE_FAN, 80, 4);
                                glDrawArrays(GL_TRIANGLE_FAN, 84, 4);
                                glDrawArrays(GL_TRIANGLE_FAN, 88, 4);
                                glDrawArrays(GL_TRIANGLE_FAN, 92, 4);
                            glBindVertexArray(0);
                        }
                        [stackMatModelView pop];  // finger 'a' transformation
                        
                        // push finger 'b' transformation
                        [stackMatModelView push: vmath::translate(0.08f - finger_b, -0.2f, 0.0f)];
                        {
                            // draw finger 'b'
                            glUniformMatrix4fv(uniforms.transformations.model, 1, GL_FALSE, [stackMatModelView top]);
                            glBindVertexArray(vaoCuboids);
                                glDrawArrays(GL_TRIANGLE_FAN, 72, 4);
                                glDrawArrays(GL_TRIANGLE_FAN, 76, 4);
                                glDrawArrays(GL_TRIANGLE_FAN, 80, 4);
                                glDrawArrays(GL_TRIANGLE_FAN, 84, 4);
                                glDrawArrays(GL_TRIANGLE_FAN, 88, 4);
                                glDrawArrays(GL_TRIANGLE_FAN, 92, 4);
                            glBindVertexArray(0);
                        }
                        [stackMatModelView pop];  // finger 'b' transformation
                    }
                    [stackMatModelView pop];  // wrist transformation
                }
                [stackMatModelView pop];  // arm transformation
            }
            [stackMatModelView pop];  // forearm transformation
        }
        [stackMatModelView pop];  // initial transformation
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
    
    if(vboCuboidNormals)
    {
        glDeleteBuffers(1, &vboCuboidNormals);
        vboCuboidNormals = 0;
    }
    
    if(vboCuboidPositions)
    {
        glDeleteBuffers(1, &vboCuboidPositions);
        vboCuboidPositions = 0;
    }
    
    if(vaoCuboids)
    {
        glDeleteVertexArrays(1, &vaoCuboids);
        vaoCuboids = 0;
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

-(GLuint) generateCuboids: (GLuint *) pVboPositions : (GLuint *) pVboNormals : (const size_t) count : (const vmath::vec3 *) dimensions
{
    // variable declarations
    GLuint retVao = 0;

    // code
    glGenVertexArrays(1, &retVao);
    glBindVertexArray(retVao);
    {
        // positions vbo
        glGenBuffers(1, pVboPositions);
        glBindBuffer(GL_ARRAY_BUFFER, *pVboPositions);
        {
            // initialize a dynamic data store first
            glBufferData(
                GL_ARRAY_BUFFER,
                count * 24 * 3 * sizeof(float),
                NULL,
                GL_DYNAMIC_DRAW
            );
            glVertexAttribPointer(KVD_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);

            for(int i = 0; i < count; i++)
            {
                float currentWidth = dimensions[i][0];
                float currentHeight = dimensions[i][1];
                float currentDepth = dimensions[i][2];

                float faceVertices[] = {
                    // front
                    currentWidth / 2.0f,  0.0f,  currentDepth / 2.0f,
                   -currentWidth / 2.0f,  0.0f,  currentDepth / 2.0f,
                   -currentWidth / 2.0f, -currentHeight,  currentDepth / 2.0f,
                    currentWidth / 2.0f, -currentHeight,  currentDepth / 2.0f,

                    // right
                    currentWidth / 2.0f,  0.0f, -currentDepth / 2.0f,
                    currentWidth / 2.0f,  0.0f,  currentDepth / 2.0f,
                    currentWidth / 2.0f, -currentHeight,  currentDepth / 2.0f,
                    currentWidth / 2.0f, -currentHeight, -currentDepth / 2.0f,

                    // back
                   -currentWidth / 2.0f,  0.0f, -currentDepth / 2.0f,
                    currentWidth / 2.0f,  0.0f, -currentDepth / 2.0f,
                    currentWidth / 2.0f, -currentHeight, -currentDepth / 2.0f,
                   -currentWidth / 2.0f, -currentHeight, -currentDepth / 2.0f,

                    // left
                   -currentWidth / 2.0f,  0.0f,  currentDepth / 2.0f,
                   -currentWidth / 2.0f,  0.0f, -currentDepth / 2.0f,
                   -currentWidth / 2.0f, -currentHeight, -currentDepth / 2.0f,
                   -currentWidth / 2.0f, -currentHeight,  currentDepth / 2.0f,

                    // top
                    currentWidth / 2.0f,  0.0f,  currentDepth / 2.0f,
                    currentWidth / 2.0f,  0.0f, -currentDepth / 2.0f,
                   -currentWidth / 2.0f,  0.0f, -currentDepth / 2.0f,
                   -currentWidth / 2.0f,  0.0f,  currentDepth / 2.0f,

                    // bottom
                    currentWidth / 2.0f, -currentHeight,  currentDepth / 2.0f,
                   -currentWidth / 2.0f, -currentHeight,  currentDepth / 2.0f,
                   -currentWidth / 2.0f, -currentHeight, -currentDepth / 2.0f,
                    currentWidth / 2.0f, -currentHeight, -currentDepth / 2.0f
                };

                glBufferSubData(GL_ARRAY_BUFFER, i * 24 * 3 * sizeof(float), 24 * 3 * sizeof(float), faceVertices);
            }

            // copy data to vertex shader now
            glEnableVertexAttribArray(KVD_ATTRIBUTE_POSITION);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // normals vbo
        glGenBuffers(1, pVboNormals);
        glBindBuffer(GL_ARRAY_BUFFER, *pVboNormals);
        {
            glBufferData(
                GL_ARRAY_BUFFER,
                count * 24 * 3 * sizeof(float),
                NULL,
                GL_DYNAMIC_DRAW
            );
            glVertexAttribPointer(KVD_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);

            for(int i = 0; i < count; i++)
            {
                float faceNormals[] = {
                    // front
                    0.0f,  0.0f,  1.0f,
                    0.0f,  0.0f,  1.0f,
                    0.0f,  0.0f,  1.0f,
                    0.0f,  0.0f,  1.0f,

                    // right
                    1.0f,  0.0f,  0.0f,
                    1.0f,  0.0f,  0.0f,
                    1.0f,  0.0f,  0.0f,
                    1.0f,  0.0f,  0.0f,

                    // back
                    0.0f,  0.0f, -1.0f,
                    0.0f,  0.0f, -1.0f,
                    0.0f,  0.0f, -1.0f,
                    0.0f,  0.0f, -1.0f,

                    // left
                   -1.0f,  0.0f,  0.0f,
                   -1.0f,  0.0f,  0.0f,
                   -1.0f,  0.0f,  0.0f,
                   -1.0f,  0.0f,  0.0f,

                    // top
                    0.0f,  1.0f,  0.0f,
                    0.0f,  1.0f,  0.0f,
                    0.0f,  1.0f,  0.0f,
                    0.0f,  1.0f,  0.0f,

                    // bottom
                    0.0f, -1.0f, 0.0f,
                    0.0f, -1.0f, 0.0f,
                    0.0f, -1.0f, 0.0f,
                    0.0f, -1.0f, 0.0f
                };

                glBufferSubData(GL_ARRAY_BUFFER, i * 24 * 3 * sizeof(float), 24 * 3 * sizeof(float), faceNormals);
            }

            glEnableVertexAttribArray(KVD_ATTRIBUTE_NORMAL);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);

    return retVao;
}
@end

/* implementing the DisplayLinkCallback */
CVReturn KVDDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *currentTime, const CVTimeStamp *outputTime, CVOptionFlags flagsIn, CVOptionFlags *flagsOut, void *view)
{
    // called by QuartzCore (the CoreVideo driver)
    // code
    return [(GLView *)view getFrameForTime: outputTime];
}
