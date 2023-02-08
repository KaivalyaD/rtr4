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
        GLuint isLight;  // bool
        GLuint viewerPosition;  // vec4
    } uniforms;
    
    enum {
        KVD_ATTRIBUTE_POSITION = 0,
        KVD_ATTRIBUTE_COLOR,
        KVD_ATTRIBUTE_NORMAL,
        KVD_ATTRIBUTE_TEXTURE0
    };
    
    bool isLight;
    int axis;
    float theta;
    OpenGLSphere *sphere;
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
    axis = isLight ? (axis + 1) % 4 : axis;
}

-(void) onDoubleTap:(UITapGestureRecognizer *)gestureRecognizer
{
    // code
    isLight = !isLight;
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
    uniforms.light.ambient = glGetUniformLocation(shaderProgramObject, "light.ambient");
    uniforms.light.diffuse = glGetUniformLocation(shaderProgramObject, "light.diffuse");
    uniforms.light.specular = glGetUniformLocation(shaderProgramObject, "light.specular");
    uniforms.light.position = glGetUniformLocation(shaderProgramObject, "light.position");
    uniforms.material.ambient = glGetUniformLocation(shaderProgramObject, "material.ambient");
    uniforms.material.diffuse = glGetUniformLocation(shaderProgramObject, "material.diffuse");
    uniforms.material.specular = glGetUniformLocation(shaderProgramObject, "material.specular");
    uniforms.material.shininess = glGetUniformLocation(shaderProgramObject, "material.shininess");
    uniforms.viewerPosition = glGetUniformLocation(shaderProgramObject, "viewerPosition");

    /* loading vertex data into pipeline */
    sphere = [[OpenGLSphere alloc] initWithSlicesAndStacks: 25 : 25];
    [sphere setPositionAttribLocation: KVD_ATTRIBUTE_POSITION];
    [sphere setNormalAttribLocation: KVD_ATTRIBUTE_NORMAL];
    [sphere generate];
    [sphere loadVerticesIntoOpenGLPipeline];
    
    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
    
    glClearDepthf(1.0f);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    
    transformations.projection = vmath::mat4::identity();
    theta = 0.0f;
    
    // initialize lighting
    isLight = false;
    
    light.ambient =  vmath::vec4(0.1f, 0.1f, 0.1f, 1.0f);
    light.diffuse = vmath::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    light.specular = vmath::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    light.position = vmath::vec4(5.0f, 5.0f, 5.0f, 1.0f);
}

-(void) resize: (int) width : (int) height
{
    // code
    if(height <= 0)
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
    theta += 2.0f;
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
    }
    glUseProgram(0);
}
@end
