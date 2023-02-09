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
#import "OpenGLSphere.h"
#import "Mat4Stack.h"

#define max(x, y) ((x > y) ? (x) : (y))
#define min(x, y) ((x < y) ? (x) : (y))

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
    
    int limbToMove;
    int axisOfRotation;
    
    bool isLight;
    OpenGLSphere *sphere;
    Mat4Stack *stackMatModelView;
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
    limbToMove = (limbToMove + 1) % 5;
}

-(void) onDoubleTap:(UITapGestureRecognizer *)gestureRecognizer
{
    // code
    axisOfRotation = 0;
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
    axisOfRotation = 1;
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
    glBindAttribLocation(shaderProgramObject, KVD_ATTRIBUTE_NORMAL, "a_normal");
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
    uniforms.isLight = glGetUniformLocation(shaderProgramObject, "isLight");
    uniforms.transformations.model = glGetUniformLocation(shaderProgramObject, "transformations.model");
    uniforms.transformations.view = glGetUniformLocation(shaderProgramObject, "transformations.view");
    uniforms.transformations.projection = glGetUniformLocation(shaderProgramObject, "transformations.projection");
    uniforms.light.diffuse = glGetUniformLocation(shaderProgramObject, "light.diffuse");
    uniforms.light.position = glGetUniformLocation(shaderProgramObject, "light.position");
    uniforms.material.diffuse = glGetUniformLocation(shaderProgramObject, "material.diffuse");
    uniforms.material.shininess = glGetUniformLocation(shaderProgramObject, "material.shininess");

    /* loading vertex data into pipeline */
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
    
    // allocate and initialize the stack
    stackMatModelView = [[Mat4Stack alloc] init];
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    glClearDepthf(1.0f);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    
    transformations.projection = vmath::mat4::identity();
    limbToMove = -1;
    axisOfRotation = 0;
    
    // initialize lighting
    isLight = false;
    light.diffuse = vmath::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    light.position = vmath::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    material.diffuse = vmath::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    material.shininess = 128.0f;
}

-(void) resize: (int) width : (int) height
{
    // code
    if(height <= 0)
        height = 1;
    
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    
    float aspectRatio = (GLfloat)width / (GLfloat)height;
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

-(void) updateScene
{
    // code
    switch(limbToMove)
    {
    case 0:  // shoulder
        if(axisOfRotation == 0)  // Y-axis
        {
            shoulder_yaw = (shoulder_yaw + 3.0f);
        }
        else  // X-axis
        {
            shoulder_pitch = (shoulder_pitch + 3.0f);
        }
        break;
        
    case 1:  // elbow
        if(axisOfRotation == 0)  // Y-axis
        {
            elbow_pitch = (elbow_pitch + 3.0f);
        }
        else  // X-axis
        {
            elbow_roll = (elbow_roll + 3.0f);
        }
        break;
        
    case 2:  // wrist
        if(axisOfRotation == 0)  // Y-axis
        {
            wrist_pitch = (wrist_pitch + 3.0f);
        }
        else  // X-axis
        {
            wrist_roll = (wrist_roll + 3.0f);
        }
        break;
        
    case 3:  // finger-a
        if(axisOfRotation == 0)  // inward motion
        {
            finger_a = min(finger_a + 0.001f, 0.04f);
        }
        else  // outward motion
        {
            finger_a = max(finger_a - 0.001f, 0.0f);
        }
        break;
        
    case 4:  // finger-b
        if(axisOfRotation == 0)  // inward motion
        {
            finger_b = min(finger_b + 0.001f, 0.04f);
        }
        else  // outward motion
        {
            finger_b = max(finger_b - 0.001f, 0.0f);
        }
        break;
        
    default:
        break;
    }
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
