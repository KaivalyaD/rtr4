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

// macros
#define VERTEX_COUNT_CIRCLE 50

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
    
    struct VertexArrayObjects {
        GLuint line;
        GLuint triangle;
        GLuint square;
        GLuint circle;
    } vao;
    
    struct VertexBufferObjects {
        GLuint line;
        GLuint triangle;
        GLuint square;
        GLuint circle;
    } vbo;
    
    struct shaderUniforms {
        GLuint color;
        GLuint mvpMatrix;
    } uniforms;
    
    struct toDisplay {
        bool triangle;
        bool square;
        bool circle;
    } show;
    
    vmath::mat4 perspectiveProjectionMatrix;
    
    enum {
        KVD_ATTRIBUTE_POSITION = 0,
        KVD_ATTRIBUTE_COLOR,
        KVD_ATTRIBUTE_NORMAL,
        KVD_ATTRIBUTE_TEXTURE0
    };
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
    show.triangle = !show.triangle;
}

-(void) onDoubleTap:(UITapGestureRecognizer *)gestureRecognizer
{
    // code
    show.square = !show.square;
}

-(void) onSwipe:(UISwipeGestureRecognizer *)gestureRecognizer
{
    // code
    [self uninitialize];
    [self release];
    exit(0);
}

-(void) onLongPress:(UILongPressGestureRecognizer *)gestureRecognizer
{
    // code
    show.circle = !show.circle;
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
        "in vec4 a_position;\n" \
        "\n" \
        "uniform mat4 u_mvpMatrix;\n" \
        "\n" \
        "void main(void)\n" \
        "{\n" \
        "   gl_Position = u_mvpMatrix * a_position;\n" \
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
        "uniform vec4 u_color;"
        "\n" \
        "out vec4 FragColor;\n" \
        "\n" \
        "void main(void)\n" \
        "{\n" \
        "   FragColor = u_color;\n" \
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
    glBindAttribLocation(shaderProgramObject, KVD_ATTRIBUTE_COLOR, "a_color");
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
    uniforms.mvpMatrix = glGetUniformLocation(shaderProgramObject, "u_mvpMatrix");
    uniforms.color = glGetUniformLocation(shaderProgramObject, "u_color");

    /* loading vertex data into pipeline */
    // line
    glGenVertexArrays(1, &vao.line);
    glBindVertexArray(vao.line);
    {
        glGenBuffers(1, &vbo.line);
        glBindBuffer(GL_ARRAY_BUFFER, vbo.line);
        {
            const float vertexPositions[] = {
                0.0f,  1.0f, 0.0f,
                0.0f, -1.0f, 0.0f
            };

            glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);
            glVertexAttribPointer(KVD_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(KVD_ATTRIBUTE_POSITION);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);
    
    // triangle
    glGenVertexArrays(1, &vao.triangle);
    glBindVertexArray(vao.triangle);
    {
        glGenBuffers(1, &vbo.triangle);
        glBindBuffer(GL_ARRAY_BUFFER, vbo.triangle);
        {
            const float vertexPositions[] = {
                sinf(vmath::radians(  0.0f)), cosf(vmath::radians(  0.0f)), 0.0f,
                sinf(vmath::radians(120.0f)), cosf(vmath::radians(120.0f)), 0.0f,
                sinf(vmath::radians(240.0f)), cosf(vmath::radians(240.0f)), 0.0f,
            };

            glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);
            glVertexAttribPointer(KVD_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(KVD_ATTRIBUTE_POSITION);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);
    
    // square
    glGenVertexArrays(1, &vao.square);
    glBindVertexArray(vao.square);
    {
        glGenBuffers(1, &vbo.square);
        glBindBuffer(GL_ARRAY_BUFFER, vbo.square);
        {
            const float vertexPositions[] = {
                 1.0f,  1.0f, 0.0f,
                -1.0f,  1.0f, 0.0f,
                -1.0f, -1.0f, 0.0f,
                 1.0f, -1.0f, 0.0f
            };

            glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);
            glVertexAttribPointer(KVD_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(KVD_ATTRIBUTE_POSITION);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);
    
    // circle
    glGenVertexArrays(1, &vao.circle);
    glBindVertexArray(vao.circle);
    {
        glGenBuffers(1, &vbo.circle);
        glBindBuffer(GL_ARRAY_BUFFER, vbo.circle);
        {
            vmath::vec2 vertexPositions[VERTEX_COUNT_CIRCLE];

            float theta = 0.0f;
            float dTheta = 360.0f / (float)VERTEX_COUNT_CIRCLE;
            int current = 0;
            
            while(theta < 360.0f) {
                vertexPositions[current++] = vmath::vec2(cosf(vmath::radians(theta)), sinf(vmath::radians(theta)));
                theta += dTheta;
            }
            
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);
            glVertexAttribPointer(KVD_ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(KVD_ATTRIBUTE_POSITION);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    glClearDepthf(1.0f);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    
    perspectiveProjectionMatrix = vmath::mat4::identity();
    
    show.triangle = false;
    show.square = false;
    show.circle = false;
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
    // variable declarations
    vmath::mat4 mvpMatrix;
    
    // code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(shaderProgramObject);
    {
        // blue grid
        glUniform4fv(uniforms.color, 1, vmath::vec4(0.0f, 0.0f, 1.0f, 1.0f));
        
        float x = -1.0f;
        for (int i = 0; i < 41; i++)
        {
            if (i != 20)
            {
                mvpMatrix = perspectiveProjectionMatrix * vmath::translate(x, 0.0f, -2.5f);
                glUniformMatrix4fv(uniforms.mvpMatrix, 1, GL_FALSE, mvpMatrix);

                glBindVertexArray(vao.line);
                glDrawArrays(GL_LINES, 0, 2);
                glBindVertexArray(0);
            }
            x = x + (1.0f / 20.0f);
        }
        
        float y = 1.0f;
        for (int i = 0; i < 41; i++)
        {
            if (i != 20)
            {
                mvpMatrix = perspectiveProjectionMatrix * vmath::translate(0.0f, y, -2.5f) * vmath::rotate(90.0f, 0.0f, 0.0f, 1.0f);
                glUniformMatrix4fv(uniforms.mvpMatrix, 1, GL_FALSE, mvpMatrix);

                glBindVertexArray(vao.line);
                glDrawArrays(GL_LINES, 0, 2);
                glBindVertexArray(0);
            }
            y = y - (1.0f / 20.0f);
        }
        
        // the X-Axis - red
        glUniform4fv(uniforms.color, 1, vmath::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        mvpMatrix = perspectiveProjectionMatrix * vmath::translate(0.0f, 0.0f, -2.5f);
        glUniformMatrix4fv(uniforms.mvpMatrix, 1, GL_FALSE, mvpMatrix);

        glBindVertexArray(vao.line);
        glDrawArrays(GL_LINES, 0, 2);
        glBindVertexArray(0);

        // the Y-Axis - green
        glUniform4fv(uniforms.color, 1, vmath::vec4(0.0f, 1.0f, 0.0f, 1.0f));
        mvpMatrix = perspectiveProjectionMatrix * vmath::translate(0.0f, 0.0f, -2.5f) * vmath::rotate(90.0f, 0.0f, 0.0f, 1.0f);
        glUniformMatrix4fv(uniforms.mvpMatrix, 1, GL_FALSE, mvpMatrix);

        glBindVertexArray(vao.line);
        glDrawArrays(GL_LINES, 0, 2);
        glBindVertexArray(0);
        
        // other geometery
        if(show.triangle)
        {
            mvpMatrix = perspectiveProjectionMatrix * vmath::translate(0.0f, 0.0f, -2.5f) * vmath::scale(0.7f);
            glUniformMatrix4fv(uniforms.mvpMatrix, 1, GL_FALSE, mvpMatrix);
            glUniform4fv(uniforms.color, 1, vmath::vec4(1.0f, 1.0f, 0.0f, 1.0f));
            
            glBindVertexArray(vao.triangle);
            glDrawArrays(GL_LINE_LOOP, 0, 3);
            glBindVertexArray(0);
        }
        
        if(show.square)
        {
            mvpMatrix = perspectiveProjectionMatrix * vmath::translate(0.0f, 0.0f, -2.5f) * vmath::scale(0.7f);
            glUniformMatrix4fv(uniforms.mvpMatrix, 1, GL_FALSE, mvpMatrix);
            glUniform4fv(uniforms.color, 1, vmath::vec4(0.8f, 0.3f, 0.0f, 1.0f));
            
            glBindVertexArray(vao.square);
            glDrawArrays(GL_LINE_LOOP, 0, 4);
            glBindVertexArray(0);
        }
        
        if(show.circle)
        {
            mvpMatrix = perspectiveProjectionMatrix * vmath::translate(0.0f, 0.0f, -2.5f) * vmath::scale(0.7f);
            glUniformMatrix4fv(uniforms.mvpMatrix, 1, GL_FALSE, mvpMatrix);
            glUniform4fv(uniforms.color, 1, vmath::vec4(1.0f, 0.0f, 1.0f, 1.0f));
            
            glBindVertexArray(vao.circle);
            glDrawArrays(GL_LINE_LOOP, 0, VERTEX_COUNT_CIRCLE);
            glBindVertexArray(0);
        }
    }
    glUseProgram(0);
}

-(void) updateScene
{
    // code
}

-(void) uninitialize
{
    // code
    if(vbo.circle)
    {
        glDeleteBuffers(1, &vbo.circle);
        vbo.circle = 0U;
    }
    
    if(vao.circle)
    {
        glDeleteBuffers(1, &vao.circle);
        vao.circle = 0U;
    }
    
    if(vbo.square)
    {
        glDeleteBuffers(1, &vbo.square);
        vbo.square = 0U;
    }
    
    if(vao.square)
    {
        glDeleteBuffers(1, &vao.square);
        vao.square = 0U;
    }

    if(vbo.triangle)
    {
        glDeleteVertexArrays(1, &vbo.triangle);
        vbo.triangle = 0U;
    }
    
    if(vao.triangle)
    {
        glDeleteVertexArrays(1, &vao.triangle);
        vao.triangle = 0U;
    }
    
    if(vbo.line)
    {
        glDeleteVertexArrays(1, &vbo.line);
        vbo.line = 0U;
    }
    
    if(vao.line)
    {
        glDeleteVertexArrays(1, &vao.line);
        vao.line = 0U;
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

@end
