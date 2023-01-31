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
    GLuint vaoCube;
    GLuint vboCubeVertices;
    GLuint vboCubeNormals;
    
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
    
    float theta;
    bool isLight;
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
    
    theta = 0.0f;
    
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
    glGenVertexArrays(1, &vaoCube);
    glBindVertexArray(vaoCube);
    {
        // vertices
        glGenBuffers(1, &vboCubeVertices);
        glBindBuffer(GL_ARRAY_BUFFER, vboCubeVertices);
        {
            // triangle vertices
            const float vertexPositions[] = {
                // top
                1.0f, 1.0f, -1.0f,
               -1.0f, 1.0f, -1.0f,
               -1.0f, 1.0f,  1.0f,
                1.0f, 1.0f,  1.0f,

                // bottom
                1.0f, -1.0f, -1.0f,
               -1.0f, -1.0f, -1.0f,
               -1.0f, -1.0f,  1.0f,
                1.0f, -1.0f,  1.0f,

                // front
                1.0f,  1.0f, 1.0f,
               -1.0f,  1.0f, 1.0f,
               -1.0f, -1.0f, 1.0f,
                1.0f, -1.0f, 1.0f,

                // back
                1.0f,  1.0f, -1.0f,
               -1.0f,  1.0f, -1.0f,
               -1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,

                // right
                1.0f,  1.0f, -1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f, -1.0f,  1.0f,
                1.0f, -1.0f, -1.0f,

                // left
               -1.0f,  1.0f,  1.0f,
               -1.0f,  1.0f, -1.0f,
               -1.0f, -1.0f, -1.0f,
               -1.0f, -1.0f,  1.0f
            };

            glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);
            glVertexAttribPointer(KVD_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(KVD_ATTRIBUTE_POSITION);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        // normals
        glGenBuffers(1, &vboCubeNormals);
        glBindBuffer(GL_ARRAY_BUFFER, vboCubeNormals);
        {
            // vertex normals
            const float vertexNormals[] = {
                // top surface
                0.0f, 1.0f, 0.0f, // top-right of top
                0.0f, 1.0f, 0.0f, // top-left of top
                0.0f, 1.0f, 0.0f, // bottom-left of top
                0.0f, 1.0f, 0.0f, // bottom-right of top

                // bottom surface
                0.0f, -1.0f, 0.0f, // top-right of bottom
                0.0f, -1.0f, 0.0f, // top-left of bottom
                0.0f, -1.0f, 0.0f, // bottom-left of bottom
                0.0f, -1.0f, 0.0f, // bottom-right of bottom

                // front surface
                0.0f, 0.0f, 1.0f,  // top-right of front
                0.0f, 0.0f, 1.0f,  // top-left of front
                0.0f, 0.0f, 1.0f,  // bottom-left of front
                0.0f, 0.0f, 1.0f,  // bottom-right of front

                // back surface
                0.0f, 0.0f, -1.0f, // top-right of back
                0.0f, 0.0f, -1.0f, // top-left of back
                0.0f, 0.0f, -1.0f, // bottom-left of back
                0.0f, 0.0f, -1.0f, // bottom-right of back

                // right surface
                1.0f, 0.0f, 0.0f, // top-right of right
                1.0f, 0.0f, 0.0f, // top-left of right
                1.0f, 0.0f, 0.0f, // bottom-left of right
                1.0f, 0.0f, 0.0f, // bottom-right of right

                // left surface
                -1.0f, 0.0f, 0.0f, // top-right of left
                -1.0f, 0.0f, 0.0f, // top-left of left
                -1.0f, 0.0f, 0.0f, // bottom-left of left
                -1.0f, 0.0f, 0.0f  // bottom-right of left
            };

            glBufferData(GL_ARRAY_BUFFER, sizeof(vertexNormals), vertexNormals, GL_STATIC_DRAW);
            glVertexAttribPointer(KVD_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(KVD_ATTRIBUTE_NORMAL);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    glClearDepth(1.0f);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    
    transformations.projection = vmath::mat4::identity();
    
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
        // transform
        transformations.model = vmath::translate(0.0f, 0.0f, -3.0f) *
            vmath::rotate(theta, 1.0f, 0.0f, 0.0f) *
            vmath::rotate(theta, 0.0f, 1.0f, 0.0f) *
            vmath::rotate(theta, 0.0f, 0.0f, 1.0f);
        transformations.view = vmath::lookat(vmath::vec3(0.0f, 0.0f, 3.0f), vmath::vec3(0.0f, 0.0f, 0.0f), vmath::vec3(0.0f, 1.0f, 0.0f));
        
        // setup uniforms
        if(isLight) {
            glUniform1i(uniforms.isLight, GL_TRUE);
            glUniform4fv(uniforms.light.diffuse, 1, light.diffuse);
            glUniform4fv(uniforms.light.position, 1, light.position);
            glUniform4fv(uniforms.material.diffuse, 1, material.diffuse);
            glUniform1f(uniforms.material.shininess, material.shininess);
        } else {
            glUniform1i(uniforms.isLight, GL_FALSE);
        }
        glUniformMatrix4fv(uniforms.transformations.model, 1, GL_FALSE, transformations.model);
        glUniformMatrix4fv(uniforms.transformations.view, 1, GL_FALSE, transformations.view);
        glUniformMatrix4fv(uniforms.transformations.projection, 1, GL_FALSE, transformations.projection);
        
        // draw
        glBindVertexArray(vaoCube);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
            glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
            glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
            glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
            glDrawArrays(GL_TRIANGLE_FAN, 20, 4);
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
    if(vboCubeNormals)
    {
        glDeleteBuffers(1, &vboCubeNormals);
        vboCubeNormals = 0U;
    }
    
    if(vboCubeVertices)
    {
        glDeleteBuffers(1, &vboCubeVertices);
        vboCubeVertices = 0U;
    }

    if(vaoCube)
    {
        glDeleteVertexArrays(1, &vaoCube);
        vaoCube = 0U;
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
