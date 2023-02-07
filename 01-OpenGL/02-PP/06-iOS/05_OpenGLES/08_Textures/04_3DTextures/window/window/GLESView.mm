//
//  GLESView.m
//  window
//
//  Created by Kaivalya Deshpande.
//

#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#import "GLESView.h"
#import "vmath.h"

@implementation GLESView
{
@private
    EAGLContext *eaglContext;
    
    // default framebuffer vars
    GLuint defaultFramebuffer;
    GLuint colorRenderbuffer;
    GLuint depthRenderbuffer;
    
    CADisplayLink *displayLink;  // CoreAnimation Display Link
    GLint fps;
    BOOL isAnimating;
    
    // variables for this app
    GLuint shaderProgramObject;
    
    GLuint vaoPyramid;
    GLuint vboPyramidVertices;
    GLuint vboPyramidTexCoords;

    GLuint vaoCube;
    GLuint vboCubeVertices;
    GLuint vboCubeTexCoords;
    
    GLuint textureStone;
    GLuint textureKundali;
    
    GLuint mvpMatrixUniform;
    GLuint textureSamplerUniform;
    
    vmath::mat4 perspectiveProjectionMatrix;
    
    enum {
        KVD_ATTRIBUTE_POSITION = 0,
        KVD_ATTRIBUTE_COLOR,
        KVD_ATTRIBUTE_NORMAL,
        KVD_ATTRIBUTE_TEXTURE0
    };
    
    GLfloat anglePyramid;
    GLfloat angleCube;
}

-(id) initWithFrame:(CGRect)frame
{
    // code
    self = [super initWithFrame:frame];
    if(self)
    {
        // create a drawable surface ('Layer' in iOS)
        // for this to work, one must implement a static method: layerClass
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)[super layer];
        
        // configure the layer
        [eaglLayer setOpaque: YES];  // alpha
        NSNumber *boolNumber = [NSNumber numberWithBool: NO];
        NSDictionary *dictionary = [NSDictionary dictionaryWithObjectsAndKeys:
                                    boolNumber, kEAGLDrawablePropertyRetainedBacking,
                                    kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat,
                                    nil];
        [eaglLayer setDrawableProperties: dictionary];
        
        // create an EAGL context
        eaglContext = [[EAGLContext alloc] initWithAPI: kEAGLRenderingAPIOpenGLES3];
        if(!eaglContext)
        {
            printf("GLESView: initWithFrame: initWithRenderingAPI: failed to create an EAGL Context\n");
            [self uninitialize];
            [self release];
            exit(0);
        }
        
        // make this context current
        [EAGLContext setCurrentContext: eaglContext];
        
        // log GLES info
        [self logGLESInfo];
        
        // create and configure the view's default framebuffer
        glGenFramebuffers(1, &defaultFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
        {
            GLint backingWidth;
            GLint backingHeight;
            
            // create default color renderbuffer
            glGenRenderbuffers(1, &colorRenderbuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
            {
                // allocate storage for the color renderbuffer
                [eaglContext renderbufferStorage: GL_RENDERBUFFER fromDrawable: eaglLayer];
                
                // associate the color buffer with default framebuffer
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderbuffer);
                
                // get color renderbuffer dimensions
                glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth);
                glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight);
            }
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            
            // create default depth renderbuffer
            glGenRenderbuffers(1, &depthRenderbuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
            {
                // allocate storage for the depth renderbuffer
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, backingWidth, backingHeight);
                
                // associate the depth renderbuffer with the default framebuffer
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);
            }
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            
            // check framebuffer creation status
            if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                printf("GLESView: initWidthFrame: glCheckFramebufferStatus(): incomplete framebuffer\n");
                [self uninitialize];
                [self release];
                exit(0);
            }
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        // initialize OpenGL part of this application
        [self initialize];
        
        // 60 FPS started at iOS 8.0
        fps = 60;
        isAnimating = NO;
        
        // add event handlers (all conform to the "Target-Action" Pattern)
        // single tap
        UITapGestureRecognizer *singleTapGestureRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(onSingleTap:)];
        [singleTapGestureRecognizer setNumberOfTapsRequired:1];  // single tap
        [singleTapGestureRecognizer setNumberOfTouchesRequired:1];  // single finger
        
        // inform self(view) and the gesture recognizer object of each other
        [singleTapGestureRecognizer setDelegate:self];  // can pass self because view conforms to the UIGestureRecognizer protocol (see declaration)
        [self addGestureRecognizer:singleTapGestureRecognizer];
        
        // double tap
        UITapGestureRecognizer *doubleTapGestureRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(onDoubleTap:)];
        [doubleTapGestureRecognizer setNumberOfTapsRequired:2];
        [doubleTapGestureRecognizer setNumberOfTouchesRequired:1];
        [doubleTapGestureRecognizer setDelegate:self];
        [self addGestureRecognizer:doubleTapGestureRecognizer];
        
        // differentiate between single and double taps
        // add dependency: when singleTap is recognized, double tap must fail
        [singleTapGestureRecognizer requireGestureRecognizerToFail:doubleTapGestureRecognizer];
        
        // swipe
        UISwipeGestureRecognizer *swipeGestureRecognizer = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(onSwipe:)];
        [self addGestureRecognizer:swipeGestureRecognizer];
        
        // long press
        UILongPressGestureRecognizer *longPressGestureRecognizer = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(onLongPress:)];
        [self addGestureRecognizer:longPressGestureRecognizer];
    }
    return self;
}

/*
 * by default, UIView conforms to the CALayerDelegate protocol
 */
+(Class) layerClass
{
    // code
    return [CAEAGLLayer class];
}

-(void) layoutSubviews
{
    // variable declarations
    GLint backingWidth;
    GLint backingHeight;
    
    // code
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
    {
        glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
        {
            // allocate storage for the color renderbuffer
            [eaglContext renderbufferStorage: GL_RENDERBUFFER fromDrawable: (CAEAGLLayer *)[self layer]];
            
            // get color renderbuffer dimensions
            glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth);
            glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight);
        }
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        
        // create a new depth renderbuffer
        glGenRenderbuffers(1, &depthRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
        {
            // allocate storage for the depth renderbuffer
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, backingWidth, backingHeight);
            
            // associate the depth renderbuffer with the default framebuffer
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);
        }
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        
        // call self's resize
        [self resize: backingWidth : backingHeight];
        
        // check framebuffer creation status
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            printf("GLESView: initWithFrame: glCheckFramebufferStatus(): incomplete framebuffer\n");
            [self uninitialize];
            [self release];
            exit(0);
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // call drawView after resize
    [self drawView: self];
}

-(void) startAnimation
{
    // code
    if(isAnimating == NO)
    {
        // create the display link
        displayLink = [CADisplayLink displayLinkWithTarget: self selector: @selector(drawView:)];
        
        // set display link FPS
        [displayLink setPreferredFramesPerSecond: fps];
        
        // add this display link to the run loop
        [displayLink addToRunLoop: [NSRunLoop currentRunLoop] forMode: NSDefaultRunLoopMode];
        isAnimating = YES;
    }
}

-(void) stopAnimation
{
    // code
    if(isAnimating == YES)
    {
        // remove the display link from run loop
        [displayLink invalidate];
        isAnimating = NO;
    }
}

-(void) logGLESInfo
{
    // variable declarations
    GLint numExtensions = 0;

    // code
    printf("\n-------------------- OpenGLES Properties --------------------\n\n");
    printf("OpenGLES Vendor   : %s\n", glGetString(GL_VENDOR));
    printf("OpenGLES Version  : %s\n", glGetString(GL_VERSION));
    printf("GLSL ES Version    : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    printf("\n-------------------- OpenGLES Extensions --------------------\n\n");
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    printf("Number of supported extensions : %d\n\n", numExtensions);
    for (int i = 0; i < numExtensions; i++)
    {
        printf("%s\n", glGetStringi(GL_EXTENSIONS, i));
    }
    printf("-------------------------------------------------------------\n\n");
}

/*
 * -(void) drawRect: (CGRect) dirtyRect
 * {
 *    // code
 * }
 */

-(void) drawView: (id) sender
{
    // !!! this method runs in a different thread !!!
    // code
    [EAGLContext setCurrentContext: eaglContext];
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
    
    // update this frame
    [self updateScene];
    
    // render this frame
    [self renderScene];
    
    // present this color buffer and swap
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    [eaglContext presentRenderbuffer: GL_RENDERBUFFER];
}

-(BOOL) acceptsFirstResponder
{
    // code
    return YES;
}

// all touch events pass through this 'root' event
-(void) touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    // code
}

-(void) onSingleTap:(UITapGestureRecognizer *)gestureRecognizer
{
    // code
}

-(void) onDoubleTap:(UITapGestureRecognizer *)gestureRecognizer
{
    // code
}

-(void) onSwipe:(UISwipeGestureRecognizer *)gestureRecognizer
{
    // code
}

-(void) onLongPress:(UILongPressGestureRecognizer *)gestureRecognizer
{
    // code
    [self uninitialize];
    [self release];
    exit(0);
}

-(void) initialize
{
    // code
    // vertex shader
    int status = 0;
    int infoLogLength = 0;
    char *infoLog = NULL;

    const GLchar *vertexShaderSourceCode =
        "#version 300 core\n" \
        "\n" \
        "#define AAPL_TEXCOORD_FIX mat2(vec2(0.0, -1.0), vec2(1.0, 0.0))\n" \
        "\n" \
        "in vec4 a_position;\n" \
        "in vec2 a_texCoord;\n" \
        "\n" \
        "uniform mat4 u_mvpMatrix;\n" \
        "\n" \
        "out vec2 a_texCoord_out;\n" \
        "\n" \
        "void main(void)\n" \
        "{\n" \
        "   gl_Position = u_mvpMatrix * a_position;\n" \
        "   a_texCoord_out = AAPL_TEXCOORD_FIX * a_texCoord;\n" \
        "}\n";

    GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderObject, 1, (const GLchar **)&vertexShaderSourceCode, NULL);
    glCompileShader(vertexShaderObject);
    glGetShaderiv(vertexShaderObject, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        printf("*** vertex shader compilation errors ***\n");

        glGetShaderiv(vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            infoLog = (char *)malloc(infoLogLength * sizeof(char));
            if (infoLog)
            {
                GLsizei written = 0;

                glGetShaderInfoLog(vertexShaderObject, infoLogLength * sizeof(char), &written, infoLog);
                printf("vertex shader compilation log (%d bytes):\n%s\n", written, infoLog);
                free(infoLog);
                infoLog = NULL;
            }
            else
                printf("\tmalloc: cannot allocate memory to hold the compilation log\n");
        }
        else
            printf("\tthere is nothing to print\n");

        [self uninitialize];
        [self release];
        exit(0);
    }
    printf("vertex shader was compiled without errors\n");

    // fragment shader
    status = 0;
    infoLogLength = 0;
    infoLog = NULL;

    const GLchar *fragmentShaderSourceCode =
        "#version 300 core\n" \
        "\n" \
        "precision highp float;\n" \
        "\n" \
        "in vec2 a_texCoord_out;\n" \
        "\n" \
        "uniform sampler2D u_textureSampler;\n" \
        "\n" \
        "out vec4 FragColor;\n" \
        "\n" \
        "void main(void)\n" \
        "{\n" \
        "   FragColor = texture(u_textureSampler, a_texCoord_out);\n" \
        "}\n";

    GLuint fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderObject, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);
    glCompileShader(fragmentShaderObject);
    glGetShaderiv(fragmentShaderObject, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        printf("*** fragment shader compilation errors ***\n");

        glGetShaderiv(fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            infoLog = (char *)malloc(infoLogLength * sizeof(char));
            if (infoLog)
            {
                GLsizei written = 0;

                glGetShaderInfoLog(fragmentShaderObject, infoLogLength, &written, infoLog);
                printf("fragment shader compilation log (%d bytes):\n%s\n", written, infoLog);
                free(infoLog);
                infoLog = NULL;
            }
            else
                printf("\tmalloc: cannot allocate memory to hold the compilation log\n");
        }
        else
            printf("\tthere is nothing to print\n");

        [self uninitialize];
        [self release];
        exit(0);
    }
    printf("fragment shader was compiled without errors\n");

    // shader program object
    status = 0;
    infoLogLength = 0;
    infoLog = NULL;

    shaderProgramObject = glCreateProgram();
    glAttachShader(shaderProgramObject, vertexShaderObject);
    glAttachShader(shaderProgramObject, fragmentShaderObject);
    glBindAttribLocation(shaderProgramObject, KVD_ATTRIBUTE_POSITION, "a_position");
    glBindAttribLocation(shaderProgramObject, KVD_ATTRIBUTE_TEXTURE0, "a_texCoord");
    glLinkProgram(shaderProgramObject);
    glGetProgramiv(shaderProgramObject, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        printf("*** there were linking errors ***\n");

        glGetProgramiv(shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            infoLog = (char *)malloc(infoLogLength * sizeof(char));
            if (infoLog)
            {
                GLsizei written = 0;

                glGetProgramInfoLog(shaderProgramObject, infoLogLength * sizeof(char), &written, infoLog);
                printf("\tlink time info log (%d bytes):\n%s\n", written, infoLog);
                free(infoLog);
                infoLog = NULL;
            }
            else
                printf("\tmalloc: cannot allocate memory to hold the linking log\n");
        }
        else
            printf("\tthere is nothing to print\n");

        [self uninitialize];
        [self release];
        exit(0);
    }
    printf("shader program was linked without errors\n");

    /* post link processing */
    mvpMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_mvpMatrix");
    textureSamplerUniform = glGetUniformLocation(shaderProgramObject, "u_textureSampler");

    /* loading vertex data into pipeline */
    // pyramid vao
    glGenVertexArrays(1, &vaoPyramid);
    glBindVertexArray(vaoPyramid);
    {
        // position vbo
        glGenBuffers(1, &vboPyramidVertices);
        glBindBuffer(GL_ARRAY_BUFFER, vboPyramidVertices);
        {
            const GLfloat pyramidVertexPositions[] = {
                // front
                0.0f,  1.0f, 0.0f,
               -1.0f, -1.0f, 1.0f,
                1.0f, -1.0f, 1.0f,

                // right
                0.0f,  1.0f,  0.0f,
                1.0f, -1.0f,  1.0f,
                1.0f, -1.0f, -1.0f,

                // back
                0.0f,  1.0f,  0.0f,
                1.0f, -1.0f, -1.0f,
               -1.0f, -1.0f, -1.0f,

                // left
                0.0f, 1.0f,   0.0f,
               -1.0f, -1.0f, -1.0f,
               -1.0f, -1.0f,  1.0f
            };

            glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertexPositions), pyramidVertexPositions, GL_STATIC_DRAW);
            glVertexAttribPointer(KVD_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(KVD_ATTRIBUTE_POSITION);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // texCoord vbo
        glGenBuffers(1, &vboPyramidTexCoords);
        glBindBuffer(GL_ARRAY_BUFFER, vboPyramidTexCoords);
        {
            const GLfloat pyramidVertexTexCoords[] = {
                0.5f, 1.0f, // front-top
                0.0f, 0.0f, // front-left
                1.0f, 0.0f, // front-right

                0.5f, 1.0f, // right-top
                1.0f, 0.0f, // right-left
                0.0f, 0.0f, // right-right

                0.5f, 1.0f, // back-top
                1.0f, 0.0f, // back-left
                0.0f, 0.0f, // back-right

                0.5f, 1.0f, // left-top
                0.0f, 0.0f, // left-left
                1.0f, 0.0f  // left-right
            };
            
            glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertexTexCoords), pyramidVertexTexCoords, GL_STATIC_DRAW);
            glVertexAttribPointer(KVD_ATTRIBUTE_TEXTURE0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(KVD_ATTRIBUTE_TEXTURE0);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);

    // cube vao
    glGenVertexArrays(1, &vaoCube);
    glBindVertexArray(vaoCube);
    {
        // position vbo
        glGenBuffers(1, &vboCubeVertices);
        glBindBuffer(GL_ARRAY_BUFFER, vboCubeVertices);
        {
            const GLfloat cubeVertexPositions[] = {
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

            glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertexPositions), cubeVertexPositions, GL_STATIC_DRAW);
            glVertexAttribPointer(KVD_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(KVD_ATTRIBUTE_POSITION);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // texCoord vbo
        glGenBuffers(1, &vboCubeTexCoords);
        glBindBuffer(GL_ARRAY_BUFFER, vboCubeTexCoords);
        {
            const GLfloat cubeVertexTexCoords[] = {
                1.0f, 0.0f,
                0.0f, 0.0f,
                0.0f, 1.0f,
                1.0f, 1.0f,

                1.0f, 0.0f,
                0.0f, 0.0f,
                0.0f, 1.0f,
                1.0f, 1.0f,

                1.0f, 0.0f,
                0.0f, 0.0f,
                0.0f, 1.0f,
                1.0f, 1.0f,

                1.0f, 0.0f,
                0.0f, 0.0f,
                0.0f, 1.0f,
                1.0f, 1.0f,

                1.0f, 0.0f,
                0.0f, 0.0f,
                0.0f, 1.0f,
                1.0f, 1.0f,

                1.0f, 0.0f,
                0.0f, 0.0f,
                0.0f, 1.0f,
                1.0f, 1.0f
            };

            glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertexTexCoords), cubeVertexTexCoords, GL_STATIC_DRAW);
            glVertexAttribPointer(KVD_ATTRIBUTE_TEXTURE0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(KVD_ATTRIBUTE_TEXTURE0);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);
    
    textureStone = [self loadGLTexture: "Stone"];
    if(!textureStone)
    {
        printf("loadGLTexture: failed to load Stone\n");
        [self uninitialize];
        [self release];
        exit(0);
    }
    
    textureKundali = [self loadGLTexture: "Vijay_Kundali"];
    if(!textureKundali)
    {
        printf("loadGLTexture: failed to load Vijay_Kundali\n");
        [self uninitialize];
        [self release];
        exit(0);
    }
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    glClearDepthf(1.0f);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    
    perspectiveProjectionMatrix = vmath::mat4::identity();
    anglePyramid = 0.0f;
    angleCube = 0.0f;
}

-(void) resize: (int) width : (int) height
{
    // code
    if(height <= 0)
        height = 1;
    
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    
    float aspectRatio = (GLfloat)width / (GLfloat)height;
    perspectiveProjectionMatrix = vmath::perspective(45.0f, aspectRatio, 0.1f, 100.0f);
}

-(void) renderScene
{
    // code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(shaderProgramObject);
    {
        // transformations for the pyramid
        vmath::mat4 modelViewProjectionMatrix = perspectiveProjectionMatrix *
            vmath::translate(-1.5f, 0.0f, -6.0f) *
            vmath::rotate(-anglePyramid, 0.0f, 1.0f, 0.0f);

        glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureStone);
        glUniform1i(textureSamplerUniform, 0);
        glBindVertexArray(vaoPyramid);
            glDrawArrays(GL_TRIANGLES, 0, 12);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);

        // transformations for the cube
        modelViewProjectionMatrix = perspectiveProjectionMatrix *
            vmath::translate(1.5f, 0.0f, -6.0f) *
            vmath::rotate(angleCube, 1.0f, 0.0f, 0.0f) *
            vmath::rotate(angleCube, 0.0f, 1.0f, 0.0f) *
            vmath::rotate(angleCube, 0.0f, 0.0f, 1.0f) *
            vmath::scale(0.75f);

        glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureKundali);
        glUniform1i(textureSamplerUniform, 0);
        glBindVertexArray(vaoCube);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
            glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
            glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
            glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
            glDrawArrays(GL_TRIANGLE_FAN, 20, 4);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glUseProgram(0);
}

-(void) updateScene
{
    // code
    anglePyramid = anglePyramid + 1.0f;
    if (anglePyramid > 360.0f)
        anglePyramid = anglePyramid - 360.0f;

    angleCube = angleCube + 1.0f;
    if (angleCube > 360.0f)
        angleCube = angleCube - 360.0f;
}

-(void) uninitialize
{
    // code
    if (textureKundali)
    {
        glDeleteTextures(1, &textureKundali);
        textureKundali = 0;
    }

    if (textureStone)
    {
        glDeleteTextures(1, &textureStone);
        textureStone = 0;
    }

    if (vboCubeTexCoords)
    {
        glDeleteBuffers(1, &vboCubeTexCoords);
        vboCubeTexCoords = 0;
    }

    if (vboCubeVertices)
    {
        glDeleteBuffers(1, &vboCubeVertices);
        vboCubeVertices = 0;
    }

    if (vaoCube)
    {
        glDeleteVertexArrays(1, &vaoCube);
        vaoCube = 0;
    }

    if (vboPyramidTexCoords)
    {
        glDeleteBuffers(1, &vboPyramidTexCoords);
        vboPyramidTexCoords = 0;
    }

    if (vboPyramidVertices)
    {
        glDeleteBuffers(1, &vboPyramidVertices);
        vboPyramidVertices = 0;
    }

    if (vaoPyramid)
    {
        glDeleteVertexArrays(1, &vaoPyramid);
        vaoPyramid = 0;
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
        printf("detached and deleted %d shader objects\n", numAttachedShaders);

        glUseProgram(0);
        glDeleteProgram(shaderProgramObject);
        shaderProgramObject = 0;
        printf("deleted shader program object\n");
    }
    
    if(depthRenderbuffer)
    {
        glDeleteRenderbuffers(1, &depthRenderbuffer);
        depthRenderbuffer = 0;
    }
    
    if(colorRenderbuffer)
    {
        glDeleteRenderbuffers(1, &colorRenderbuffer);
        colorRenderbuffer = 0;
    }
    
    if(defaultFramebuffer)
    {
        glDeleteFramebuffers(1, &defaultFramebuffer);
        defaultFramebuffer = 0;
    }
    
    if([EAGLContext currentContext] == eaglContext)
    {
        [eaglContext release];
        eaglContext = nil;
    }
}

-(void) dealloc
{
    // code
    // [self uninitialize];
    
    if(displayLink)
    {
        [displayLink release];
        displayLink = nil;
    }
    
    [super dealloc];
}

-(GLuint) loadGLTexture: (const char *)texAssetName
{
    // code
    // get UIImage from texture image
    UIImage *uiTexImage = [UIImage imageNamed:@(texAssetName)];
    if(!uiTexImage)
    {
        printf("loadGLTexture: [UIImage imageNamed] failed\n");
        return 0U;
    }
    
    // get CGImage from UIImage
    CGImageRef cgTexImage = uiTexImage.CGImage;
    
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
