#import <foundation/foundation.h>  // stdio.h
#import <string.h>
#import <stdarg.h>
#import <cocoa/cocoa.h>  // macOS SDK main header
#import <QuartzCore/CVDisplayLink.h>  // CoreVideo display link
#import <OpenGL/gl3.h>
#import <OpenGL/gl3ext.h>
#import <signal.h>
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
    GLuint vao;
    GLuint vbo;
    GLuint texMarble;
    
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
    
    struct Material {
        vmath::vec4 ambient;
        vmath::vec4 diffuse;
        vmath::vec4 specular;
        float shininess;
    } material;
    
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
        GLuint texSampler;  // sampler2D
        GLuint viewerPosition;  // vec4
        GLuint isLight;  // bool
    } uniforms;
    
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
        "in vec4 a_color;\n" \
        "in vec3 a_normal;\n" \
        "in vec2 a_texCoord;\n" \
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
        "   vec4 color;\n" \
        "   vec2 texCoord;\n" \
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
        "   vs_out.color = a_color;\n" \
        "   vs_out.texCoord = a_texCoord;\n" \
        "   \n" \
        "   vec4 eyeCoordinate = modelViewMatrix * a_position;\n"
        "   vs_out.lightDirection = normalize(light.position - eyeCoordinate).xyz;\n" \
        "   vs_out.viewDirection = normalize(viewerPosition - eyeCoordinate).xyz;\n" \
        "   vs_out.normal = normalize(normalMatrix * a_normal);\n" \
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
        "   vec2 texCoord;\n" \
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
        "uniform sampler2D texSampler;\n" \
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
        "   FragColor = fs_in.color * texture(texSampler, fs_in.texCoord) * (ambient + diffuse + specular);\n" \
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
    glBindAttribLocation(shaderProgramObject, KVD_ATTRIBUTE_COLOR, "a_color");
    glBindAttribLocation(shaderProgramObject, KVD_ATTRIBUTE_NORMAL, "a_normal");
    glBindAttribLocation(shaderProgramObject, KVD_ATTRIBUTE_TEXTURE0, "a_texCoord");
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
    uniforms.texSampler = glGetUniformLocation(shaderProgramObject, "texSampler");
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
    uniforms.viewerPosition = glGetUniformLocation(shaderProgramObject, "viewerPosition");

    /* loading vertex & color data into pipeline */
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    {
        // vertex data
        const float cubeVertexDataInterleaved[] = {
            // Positions             // Colors               // Normals                    // TexCoords
            // Top face              // Top face             // Top face                   // Top face
            1.0f,  1.0f, -1.0f,      0.0f, 0.0f, 1.0f,       0.0f,  1.0f,  0.0f,           1.0f, 1.0f,
           -1.0f,  1.0f, -1.0f,      0.0f, 0.0f, 1.0f,       0.0f,  1.0f,  0.0f,           0.0f, 1.0f,
           -1.0f,  1.0f,  1.0f,      0.0f, 0.0f, 1.0f,       0.0f,  1.0f,  0.0f,           0.0f, 0.0f,
            1.0f,  1.0f,  1.0f,      0.0f, 0.0f, 1.0f,       0.0f,  1.0f,  0.0f,           1.0f, 0.0f,

            // Bottom face           // Bottom face          // Bottom face                // Bottom face
            1.0f, -1.0f, -1.0f,      0.0f, 1.0f, 0.0f,       0.0f, -1.0f,  0.0f,           1.0f, 1.0f,
           -1.0f, -1.0f, -1.0f,      0.0f, 1.0f, 0.0f,       0.0f, -1.0f,  0.0f,           0.0f, 1.0f,
           -1.0f, -1.0f,  1.0f,      0.0f, 1.0f, 0.0f,       0.0f, -1.0f,  0.0f,           0.0f, 0.0f,
            1.0f, -1.0f,  1.0f,      0.0f, 1.0f, 0.0f,       0.0f, -1.0f,  0.0f,           1.0f, 0.0f,

            // Front face            // Front face           // Front face                 // Front face
            1.0f,  1.0f,  1.0f,      1.0f, 0.0f, 0.0f,       0.0f,  0.0f,  1.0f,           1.0f, 1.0f,
           -1.0f,  1.0f,  1.0f,      1.0f, 0.0f, 0.0f,       0.0f,  0.0f,  1.0f,           0.0f, 1.0f,
           -1.0f, -1.0f,  1.0f,      1.0f, 0.0f, 0.0f,       0.0f,  0.0f,  1.0f,           0.0f, 0.0f,
            1.0f, -1.0f,  1.0f,      1.0f, 0.0f, 0.0f,       0.0f,  0.0f,  1.0f,           1.0f, 0.0f,

            // Back face             // Back face            // Back face                  // Back face
            1.0f,  1.0f, -1.0f,      0.0f, 1.0f, 1.0f,       0.0f,  0.0f, -1.0f,           1.0f, 1.0f,
           -1.0f,  1.0f, -1.0f,      0.0f, 1.0f, 1.0f,       0.0f,  0.0f, -1.0f,           0.0f, 1.0f,
           -1.0f, -1.0f, -1.0f,      0.0f, 1.0f, 1.0f,       0.0f,  0.0f, -1.0f,           0.0f, 0.0f,
            1.0f, -1.0f, -1.0f,      0.0f, 1.0f, 1.0f,       0.0f,  0.0f, -1.0f,           1.0f, 0.0f,

            // Right face            // Right face           // Right face                 // Right face
            1.0f,  1.0f, -1.0f,      1.0f, 0.0f, 1.0f,       1.0f,  0.0f,  0.0f,           1.0f, 1.0f,
            1.0f,  1.0f,  1.0f,      1.0f, 0.0f, 1.0f,       1.0f,  0.0f,  0.0f,           0.0f, 1.0f,
            1.0f, -1.0f,  1.0f,      1.0f, 0.0f, 1.0f,       1.0f,  0.0f,  0.0f,           0.0f, 0.0f,
            1.0f, -1.0f, -1.0f,      1.0f, 0.0f, 1.0f,       1.0f,  0.0f,  0.0f,           1.0f, 0.0f,

            // Left face             // Left face             // Left face                 // Left face
           -1.0f,  1.0f,  1.0f,      1.0f, 1.0f, 0.0f,      -1.0f,  0.0f,  0.0f,           1.0f, 1.0f,
           -1.0f,  1.0f, -1.0f,      1.0f, 1.0f, 0.0f,      -1.0f,  0.0f,  0.0f,           0.0f, 1.0f,
           -1.0f, -1.0f, -1.0f,      1.0f, 1.0f, 0.0f,      -1.0f,  0.0f,  0.0f,           0.0f, 0.0f,
           -1.0f, -1.0f,  1.0f,      1.0f, 1.0f, 0.0f,      -1.0f,  0.0f,  0.0f,           1.0f, 0.0f
        };

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        {
            glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertexDataInterleaved), cubeVertexDataInterleaved, GL_STATIC_DRAW);
            
            /*
             * sizeof(cubeVertexDataInterleaved) = (3 + 3 + 3 + 2) * 24 * sizeof(float) = 11 * 24 * sizeof(float) = 264 * sizeof(float)
             */
            glVertexAttribPointer(KVD_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void *)(0));
            glVertexAttribPointer(KVD_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void *)(3 * sizeof(float)));
            glVertexAttribPointer(KVD_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void *)(6 * sizeof(float)));
            glVertexAttribPointer(KVD_ATTRIBUTE_TEXTURE0, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void *)(9 * sizeof(float)));
            
            glEnableVertexAttribArray(KVD_ATTRIBUTE_POSITION);
            glEnableVertexAttribArray(KVD_ATTRIBUTE_COLOR);
            glEnableVertexAttribArray(KVD_ATTRIBUTE_NORMAL);
            glEnableVertexAttribArray(KVD_ATTRIBUTE_TEXTURE0);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);
    
    // loading textures
    texMarble = [self loadGLTexture: "Resources/Marble.bmp"];
    if(!texMarble)
    {
        fprintf(logFile, "loadGLTexture: failed to load Marble.bmp\n");
        [self uninitialize];
        [self release];
        [NSApp terminate: self];
    }
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    glClearDepth(1.0f);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    
    transformations.projection = vmath::mat4::identity();
    theta = 0.0f;
    
    // initialize lighting
    light.ambient =  vmath::vec4(0.05f, 0.05f, 0.05f, 1.0f);
    light.diffuse = vmath::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    light.specular = vmath::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    light.position = vmath::vec4(5.0f, 5.0f, 5.0f, 1.0f);
    
    material.ambient = vmath::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    material.diffuse = vmath::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    material.specular = vmath::vec4(1.0f, 1.0f, 1.0f, 1.0f);
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
    // constants
    const vmath::vec3 viewerPosition = vmath::vec3(0.0f, 0.0f, 1.0f);
    
    // code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgramObject);
    {
        // transform
        transformations.model = vmath::translate(0.0f, 0.0f, -5.0f) *
            vmath::rotate(theta, 1.0f, 0.0f, 0.0f) *
            vmath::rotate(theta, 0.0f, 1.0f, 0.0f) *
            vmath::rotate(theta, 0.0f, 0.0f, 1.0f);
        transformations.view = vmath::lookat(viewerPosition, vmath::vec3(0.0f, 0.0f, 0.0f), vmath::vec3(0.0f, 1.0f, 0.0f));
        
        // setup uniforms
        glUniform1i(uniforms.isLight, GL_TRUE);
        glUniform4fv(uniforms.light.ambient, 1, light.ambient);
        glUniform4fv(uniforms.light.diffuse, 1, light.diffuse);
        glUniform4fv(uniforms.light.specular, 1, light.specular);
        glUniform4fv(uniforms.light.position, 1, light.position);
        
        glUniform4fv(uniforms.material.ambient, 1, material.ambient);
        glUniform4fv(uniforms.material.diffuse, 1, material.diffuse);
        glUniform4fv(uniforms.material.specular, 1, material.specular);
        glUniform1f(uniforms.material.shininess, material.shininess);
        
        glUniform4fv(uniforms.viewerPosition, 1, vmath::vec4(viewerPosition, 1.0));
        
        glUniformMatrix4fv(uniforms.transformations.model, 1, GL_FALSE, transformations.model);
        glUniformMatrix4fv(uniforms.transformations.view, 1, GL_FALSE, transformations.view);
        glUniformMatrix4fv(uniforms.transformations.projection, 1, GL_FALSE, transformations.projection);
        
        // set up textures
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(uniforms.texSampler, 0);
        glBindTexture(GL_TEXTURE_2D, texMarble);
        
        // draw
        glBindVertexArray(vao);
        {
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
            glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
            glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
            glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
            glDrawArrays(GL_TRIANGLE_FAN, 20, 4);
        }
        glBindVertexArray(0);

        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glUseProgram(0);
}

-(void) updateScene  // NSOpenGLView has its own update not meant for animation
{
    // code
    theta += 1.0f;
    if(theta > 1.0f)
        theta -= 360.0f;
}

-(void) uninitialize
{
    // code
    if(texMarble)
    {
        glDeleteTextures(1, &texMarble);
        texMarble = 0;
    }
    
    if(vbo)
    {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }
    
    if(vao)
    {
        glDeleteBuffers(1, &vao);
        vao = 0;
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

-(GLuint) loadGLTexture: (const char *)cstr_relTexFilePath
{
    // code
    NSBundle *appBundle = [NSBundle mainBundle];
    NSString *appDirectory = [appBundle bundlePath];
    NSString *appParentDirectory = [appDirectory stringByDeletingLastPathComponent];
    NSString *texFilePath = [NSString stringWithFormat: @"%@/%s", appParentDirectory, cstr_relTexFilePath];
    if(!texFilePath)
    {
        fprintf(logFile, "loadGLTexture: cannot create NSString* from cstring: '%s'\n", cstr_relTexFilePath);
        return 0U;
    }

    // get NSImage from texture image
    NSImage *nsTexImage = [[NSImage alloc] initWithContentsOfFile: texFilePath];
    if(!nsTexImage)
    {
        fprintf(logFile, "loadGLTexture: [NSImageObj initWithContentsOfFile] failed\n");
        return 0U;
    }
    
    // get CGImage from NSImage
    CGImageRef cgTexImage = [nsTexImage CGImageForProposedRect:nil context:nil hints:nil];  // context is a GraphicsContext, not OpenGL context and hints are extra properties passed as a dictionary
    
    // from CGImage, get image width and height
    int imgWidth = (int)CGImageGetWidth(cgTexImage);
    int imgHeight = (int)CGImageGetHeight(cgTexImage);
    
    // from CGImage, get CGDataProvider
    CGDataProviderRef cgDataProvider = CGImageGetDataProvider(cgTexImage);
    
    // get image data from CGDataProvider as CFData (Core Foundation)
    CFDataRef cfImgData = CGDataProviderCopyData(cgDataProvider);
    
    // convert format from CFData to void *
    void *imgData = (void *)CFDataGetBytePtr(cfImgData);
    
    // usual texture setup
    GLuint newTex = 0U;
    glGenTextures(1, &newTex);
    glBindTexture(GL_TEXTURE_2D, newTex);
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgWidth, imgHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, imgData);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    glBindTexture(GL_TEXTURE_2D, 0U);
    
    CFRelease(cfImgData);
    cfImgData = NULL;
    imgData = NULL;  // since cfImgData is now NULL, imgData must not remain dangling
    CGDataProviderRelease(cgDataProvider);
    cgDataProvider = NULL;
    
    return newTex;
}
@end

/* implementing the DisplayLinkCallback */
CVReturn KVDDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *currentTime, const CVTimeStamp *outputTime, CVOptionFlags flagsIn, CVOptionFlags *flagsOut, void *view)
{
    // called by QuartzCore (the CoreVideo driver)
    // code
    return [(GLView *)view getFrameForTime: outputTime];
}
