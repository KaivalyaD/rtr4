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
    GLuint pv_shaderProgramObject;
    GLuint pf_shaderProgramObject;
    
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
        float shininess[2];   // can't see per-vertex light properly for large shininess on iOS emulator
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
    } uniforms[2];
    
    enum {
        KVD_ATTRIBUTE_POSITION = 0,
        KVD_ATTRIBUTE_COLOR,
        KVD_ATTRIBUTE_NORMAL,
        KVD_ATTRIBUTE_TEXTURE0
    };
    
    bool isLight;
    bool isPerVertexLight;
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
    isPerVertexLight = isLight ? !isPerVertexLight : isPerVertexLight;
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
    // per-vertex light vertex shader
    int status = 0;
    int infoLogLength = 0;
    char *infoLog = NULL;
    
    const GLchar *pv_vertexShaderSourceCode =
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
        "uniform bool isLight;\n" \
        "\n" \
        "out VS_OUT {\n" \
        "   vec4 color;\n" \
        "} vs_out;\n" \
        "\n" \
        "void main(void)\n" \
        "{\n" \
        "   mat4 modelViewMatrix = transformations.view * transformations.model;\n" \
        "   mat3 normalMatrix = mat3(transpose(inverse(modelViewMatrix)));\n" \
        "   \n" \
        "   gl_Position = transformations.projection * modelViewMatrix * a_position;\n" \
        "   vec3 transformedNormal = normalize(normalMatrix * a_normal);\n" \
        "   \n" \
        "   if(isLight) {\n" \
        "       vec4 eyeCoordinate = modelViewMatrix * a_position;\n"
        "       \n" \
        "       vec3 lightDirection = normalize(light.position - eyeCoordinate).xyz;\n"
        "       vec3 reflectedDirection = reflect(-lightDirection, transformedNormal);\n" \
        "       vec3 viewDirection = normalize(viewerPosition - eyeCoordinate).xyz;\n" \
        "       \n" \
        "       vec4 ambient = light.ambient * material.ambient;\n" \
        "       vec4 diffuse = light.diffuse * material.diffuse * max(dot(transformedNormal, lightDirection), 0.0);\n" \
        "       \n" \
        "       /* reflect() seems to return a positive even for antiparallel reflected vector wrt normal;\n" \
        "          the if-statement below is a sad fix atleast until I don't find my mistake... */\n" \
        "       vec4 specular = vec4(0.0);\n" \
        "       if(dot(reflectedDirection, transformedNormal) > 0.0) {\n" \
        "           vec3 viewDirection = normalize(viewerPosition - eyeCoordinate).xyz;\n" \
        "           float specularFactor = pow(max(dot(viewDirection, reflectedDirection), 0.0), material.shininess);\n" \
        "           specular = light.specular * material.specular * specularFactor;\n" \
        "       }\n" \
        "       \n" \
        "       vs_out.color = ambient + diffuse + specular;\n" \
        "   } else {\n" \
        "       vs_out.color = vec4(1.0);\n" \
        "   }\n" \
        "}\n";
    
    GLuint pv_vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(pv_vertexShaderObject, 1, (const GLchar **)&pv_vertexShaderSourceCode, NULL);
    glCompileShader(pv_vertexShaderObject);
    glGetShaderiv(pv_vertexShaderObject, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        printf("*** per-vertex vertex shader compilation errors ***\n");

        glGetShaderiv(pv_vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            infoLog = (char *)malloc(infoLogLength * sizeof(char));
            if (infoLog)
            {
                GLsizei written = 0;

                glGetShaderInfoLog(pv_vertexShaderObject, infoLogLength * sizeof(char), &written, infoLog);
                printf("per-vertex vertex shader compilation log (%d bytes):\n%s\n", written, infoLog);
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
    printf("per-vertex vertex shader was compiled without errors\n");
    
    // per-vertex light fragment shader
    status = 0;
    infoLogLength = 0;
    infoLog = NULL;
    
    const GLchar *pv_fragmentShaderSourceCode =
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
        "   FragColor = fs_in.color;\n" \
        "}\n";
    
    GLuint pv_fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(pv_fragmentShaderObject, 1, (const GLchar **)&pv_fragmentShaderSourceCode, NULL);
    glCompileShader(pv_fragmentShaderObject);
    glGetShaderiv(pv_fragmentShaderObject, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        printf("*** per-vertex fragment shader compilation errors ***\n");

        glGetShaderiv(pv_fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            infoLog = (char *)malloc(infoLogLength * sizeof(char));
            if (infoLog)
            {
                GLsizei written = 0;

                glGetShaderInfoLog(pv_fragmentShaderObject, infoLogLength, &written, infoLog);
                printf("per-vertex fragment shader compilation log (%d bytes):\n%s\n", written, infoLog);
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
    printf("per-vertex fragment shader was compiled without errors\n");
    
    // per-vertex lighting shader program object
    status = 0;
    infoLogLength = 0;
    infoLog = NULL;

    pv_shaderProgramObject = glCreateProgram();
    glAttachShader(pv_shaderProgramObject, pv_vertexShaderObject);
    glAttachShader(pv_shaderProgramObject, pv_fragmentShaderObject);
    glBindAttribLocation(pv_shaderProgramObject, KVD_ATTRIBUTE_POSITION, "a_position");
    glBindAttribLocation(pv_shaderProgramObject, KVD_ATTRIBUTE_NORMAL, "a_normal");
    glLinkProgram(pv_shaderProgramObject);
    glGetProgramiv(pv_shaderProgramObject, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        printf("*** per-vertex program: there were linking errors ***\n");

        glGetProgramiv(pv_shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            infoLog = (char *)malloc(infoLogLength * sizeof(char));
            if (infoLog)
            {
                GLsizei written = 0;

                glGetProgramInfoLog(pv_shaderProgramObject, infoLogLength * sizeof(char), &written, infoLog);
                printf("\tper-vertex program: link time info log (%d bytes):\n%s\n", written, infoLog);
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
    printf("per-vertex lighting program was linked without errors\n");
    
    // ************************************************************************************** //
    
    // per-fragment light vertex shader
    status = 0;
    infoLogLength = 0;
    infoLog = NULL;

    const GLchar *pf_vertexShaderSourceCode =
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

    GLuint pf_vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(pf_vertexShaderObject, 1, (const GLchar **)&pf_vertexShaderSourceCode, NULL);
    glCompileShader(pf_vertexShaderObject);
    glGetShaderiv(pf_vertexShaderObject, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        printf("*** per-fragment lighting vertex shader compilation errors ***\n");

        glGetShaderiv(pf_vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            infoLog = (char *)malloc(infoLogLength * sizeof(char));
            if (infoLog)
            {
                GLsizei written = 0;

                glGetShaderInfoLog(pf_vertexShaderObject, infoLogLength * sizeof(char), &written, infoLog);
                printf("per-fragment lighting vertex shader compilation log (%d bytes):\n%s\n", written, infoLog);
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
    printf("per-fragment lighting vertex shader was compiled without errors\n");

    // per-fragment lighting fragment shader
    status = 0;
    infoLogLength = 0;
    infoLog = NULL;

    const GLchar *pf_fragmentShaderSourceCode =
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

    GLuint pf_fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(pf_fragmentShaderObject, 1, (const GLchar **)&pf_fragmentShaderSourceCode, NULL);
    glCompileShader(pf_fragmentShaderObject);
    glGetShaderiv(pf_fragmentShaderObject, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        printf("*** per-fragment lighting fragment shader compilation errors ***\n");

        glGetShaderiv(pf_fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            infoLog = (char *)malloc(infoLogLength * sizeof(char));
            if (infoLog)
            {
                GLsizei written = 0;

                glGetShaderInfoLog(pf_fragmentShaderObject, infoLogLength, &written, infoLog);
                printf("per-fragment lighting fragment shader compilation log (%d bytes):\n%s\n", written, infoLog);
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
    printf("per-fragment lighting fragment shader was compiled without errors\n");

    // per-fragment lighting shader program object
    status = 0;
    infoLogLength = 0;
    infoLog = NULL;

    pf_shaderProgramObject = glCreateProgram();
    glAttachShader(pf_shaderProgramObject, pf_vertexShaderObject);
    glAttachShader(pf_shaderProgramObject, pf_fragmentShaderObject);
    glBindAttribLocation(pf_shaderProgramObject, KVD_ATTRIBUTE_POSITION, "a_position");
    glBindAttribLocation(pf_shaderProgramObject, KVD_ATTRIBUTE_NORMAL, "a_normal");
    glLinkProgram(pf_shaderProgramObject);
    glGetProgramiv(pf_shaderProgramObject, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        printf("*** per-fragment lighting program: there were linking errors ***\n");

        glGetProgramiv(pf_shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            infoLog = (char *)malloc(infoLogLength * sizeof(char));
            if (infoLog)
            {
                GLsizei written = 0;

                glGetProgramInfoLog(pf_shaderProgramObject, infoLogLength * sizeof(char), &written, infoLog);
                printf("\tper-fragment lighting program: link time info log (%d bytes):\n%s\n", written, infoLog);
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
    printf("per-fragment lighting shader program was linked without errors\n");

    /* per-vertex lighting post link processing */
    uniforms[0].isLight = glGetUniformLocation(pv_shaderProgramObject, "isLight");
    uniforms[0].transformations.model = glGetUniformLocation(pv_shaderProgramObject, "transformations.model");
    uniforms[0].transformations.view = glGetUniformLocation(pv_shaderProgramObject, "transformations.view");
    uniforms[0].transformations.projection = glGetUniformLocation(pv_shaderProgramObject, "transformations.projection");
    uniforms[0].light.ambient = glGetUniformLocation(pv_shaderProgramObject, "light.ambient");
    uniforms[0].light.diffuse = glGetUniformLocation(pv_shaderProgramObject, "light.diffuse");
    uniforms[0].light.specular = glGetUniformLocation(pv_shaderProgramObject, "light.specular");
    uniforms[0].light.position = glGetUniformLocation(pv_shaderProgramObject, "light.position");
    uniforms[0].material.ambient = glGetUniformLocation(pv_shaderProgramObject, "material.ambient");
    uniforms[0].material.diffuse = glGetUniformLocation(pv_shaderProgramObject, "material.diffuse");
    uniforms[0].material.specular = glGetUniformLocation(pv_shaderProgramObject, "material.specular");
    uniforms[0].material.shininess = glGetUniformLocation(pv_shaderProgramObject, "material.shininess");
    uniforms[0].viewerPosition = glGetUniformLocation(pv_shaderProgramObject, "viewerPosition");
    
    /* per-fragment lighting post link processing */
    uniforms[1].isLight = glGetUniformLocation(pf_shaderProgramObject, "isLight");
    uniforms[1].transformations.model = glGetUniformLocation(pf_shaderProgramObject, "transformations.model");
    uniforms[1].transformations.view = glGetUniformLocation(pf_shaderProgramObject, "transformations.view");
    uniforms[1].transformations.projection = glGetUniformLocation(pf_shaderProgramObject, "transformations.projection");
    uniforms[1].light.ambient = glGetUniformLocation(pf_shaderProgramObject, "light.ambient");
    uniforms[1].light.diffuse = glGetUniformLocation(pf_shaderProgramObject, "light.diffuse");
    uniforms[1].light.specular = glGetUniformLocation(pf_shaderProgramObject, "light.specular");
    uniforms[1].light.position = glGetUniformLocation(pf_shaderProgramObject, "light.position");
    uniforms[1].material.ambient = glGetUniformLocation(pf_shaderProgramObject, "material.ambient");
    uniforms[1].material.diffuse = glGetUniformLocation(pf_shaderProgramObject, "material.diffuse");
    uniforms[1].material.specular = glGetUniformLocation(pf_shaderProgramObject, "material.specular");
    uniforms[1].material.shininess = glGetUniformLocation(pf_shaderProgramObject, "material.shininess");
    uniforms[1].viewerPosition = glGetUniformLocation(pf_shaderProgramObject, "viewerPosition");

    /* loading vertex data into pipeline */
    sphere = [[OpenGLSphere alloc] initWithSlicesAndStacks: 25 : 25];
    [sphere setPositionAttribLocation: KVD_ATTRIBUTE_POSITION];
    [sphere setNormalAttribLocation: KVD_ATTRIBUTE_NORMAL];
    [sphere generate];
    [sphere loadVerticesIntoOpenGLPipeline];
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    glClearDepthf(1.0f);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    
    transformations.projection = vmath::mat4::identity();
    
    // initialize lighting
    isLight = false;
    isPerVertexLight = true;
    
    light.ambient =  vmath::vec4(0.1f, 0.1f, 0.1f, 1.0f);
    light.diffuse = vmath::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    light.specular = vmath::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    light.position = vmath::vec4(5.0f, 5.0f, 5.0f, 1.0f);
    
    material.ambient = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    material.diffuse = vmath::vec4(0.5f, 0.2f, 0.7f, 1.0f);
    material.specular = vmath::vec4(0.7f, 0.7f, 0.7f, 1.0f);
    material.shininess[0] = 5.0f;
    material.shininess[1] = 64.0f;
}

-(void) resize: (int) width : (int) height
{
    // code
    if(height <= 0)
        height = 1;
    
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    
    float aspectRatio = (GLfloat)width / (GLfloat)height;
    transformations.projection = vmath::perspective(45.0f, aspectRatio, 0.1f, 100.0f);
}

-(void) renderScene
{
    // constants
    const vmath::vec3 viewerPosition = vmath::vec3(0.0f, 0.0f, 2.0f);
    
    // variables
    GLuint useProgram;
    unsigned int useIndex;
    
    // code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if(isPerVertexLight)
    {
        useProgram = pv_shaderProgramObject;
        useIndex = 0;
    }
    else
    {
        useProgram = pf_shaderProgramObject;
        useIndex = 1;
    }
    
    glUseProgram(useProgram);
    {
        // transform
        transformations.model = vmath::translate(0.0f, 0.0f, -2.0f);
        transformations.view = vmath::lookat(viewerPosition, vmath::vec3(0.0f, 0.0f, 0.0f), vmath::vec3(0.0f, 1.0f, 0.0f));
        
        // setup uniforms
        if(isLight) {
            glUniform1i(uniforms[useIndex].isLight, GL_TRUE);
        } else {
            glUniform1i(uniforms[useIndex].isLight, GL_FALSE);
        }
        
        glUniform4fv(uniforms[useIndex].light.ambient, 1, light.ambient);
        glUniform4fv(uniforms[useIndex].light.diffuse, 1, light.diffuse);
        glUniform4fv(uniforms[useIndex].light.specular, 1, light.specular);
        glUniform4fv(uniforms[useIndex].light.position, 1, light.position);
        
        glUniform4fv(uniforms[useIndex].material.ambient, 1, material.ambient);
        glUniform4fv(uniforms[useIndex].material.diffuse, 1, material.diffuse);
        glUniform4fv(uniforms[useIndex].material.specular, 1, material.specular);
        glUniform1f(uniforms[useIndex].material.shininess, material.shininess[useIndex]);
        
        glUniformMatrix4fv(uniforms[useIndex].transformations.model, 1, GL_FALSE, transformations.model);
        glUniformMatrix4fv(uniforms[useIndex].transformations.view, 1, GL_FALSE, transformations.view);
        glUniformMatrix4fv(uniforms[useIndex].transformations.projection, 1, GL_FALSE, transformations.projection);
        
        // draw
        [sphere render: GL_TRIANGLES];
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
    if(sphere)
    {
        [sphere dealloc];
    }

    if (pf_shaderProgramObject)
    {
        GLsizei numAttachedShaders = 0;
        GLuint *shaderObjects = NULL;

        glUseProgram(pf_shaderProgramObject);
        
        glGetProgramiv(pf_shaderProgramObject, GL_ATTACHED_SHADERS, &numAttachedShaders);
        shaderObjects = (GLuint *)malloc(numAttachedShaders * sizeof(GLuint));
        glGetAttachedShaders(pf_shaderProgramObject, numAttachedShaders, &numAttachedShaders, shaderObjects);

        for (GLsizei i = 0; i < numAttachedShaders; i++)
        {
            glDetachShader(pf_shaderProgramObject, shaderObjects[i]);
            glDeleteShader(shaderObjects[i]);
            shaderObjects[i] = 0;
        }
        free(shaderObjects);
        shaderObjects = NULL;
        printf("per-fragment lighting program: detached and deleted %d shader objects\n", numAttachedShaders);

        glUseProgram(0);
        glDeleteProgram(pf_shaderProgramObject);
        pf_shaderProgramObject = 0;
        printf("deleted per-fragment lighting shader program object\n");
    }
    
    if (pv_shaderProgramObject)
    {
        GLsizei numAttachedShaders = 0;
        GLuint *shaderObjects = NULL;

        glUseProgram(pv_shaderProgramObject);
        
        glGetProgramiv(pv_shaderProgramObject, GL_ATTACHED_SHADERS, &numAttachedShaders);
        shaderObjects = (GLuint *)malloc(numAttachedShaders * sizeof(GLuint));
        glGetAttachedShaders(pv_shaderProgramObject, numAttachedShaders, &numAttachedShaders, shaderObjects);

        for (GLsizei i = 0; i < numAttachedShaders; i++)
        {
            glDetachShader(pv_shaderProgramObject, shaderObjects[i]);
            glDeleteShader(shaderObjects[i]);
            shaderObjects[i] = 0;
        }
        free(shaderObjects);
        shaderObjects = NULL;
        printf("per-vertex lighting program: detached and deleted %d shader objects\n", numAttachedShaders);

        glUseProgram(0);
        glDeleteProgram(pv_shaderProgramObject);
        pv_shaderProgramObject = 0;
        printf("deleted per-vertex lighting shader program object\n");
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
