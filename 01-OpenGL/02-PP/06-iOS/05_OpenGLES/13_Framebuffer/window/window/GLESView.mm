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

// macros
#define FBO_WIDTH  512
#define FBO_HEIGHT 512

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
    unsigned int viewWidth;
    unsigned int viewHeight;
    
    GLuint fboSphere;
    GLuint rboDepthSphere;
    GLuint texColorSphere;
    
    GLuint shaderProgramObject_Sphere;
    GLuint shaderProgramObject_Cube;
    
    GLuint vaoCube;
    GLuint vboCubePositions;
    GLuint vboCubeTexCoords;
    
    struct Transformations {
        vmath::mat4 model;
        vmath::mat4 view;
    } sphereTransformations, cubeTransformations;
    vmath::mat4 perspectiveProjectionMatrix;
    
    struct Light {
        vmath::vec4 ambient;
        vmath::vec4 diffuse;
        vmath::vec4 specular;
        vmath::vec4 position;
    } light[3];
    
    struct Material {
        vmath::vec4 ambient;
        vmath::vec4 diffuse;
        vmath::vec4 specular;
        float shininess;
    } material;
    
    struct SphereProgramUniforms {
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
        } light[3];
        struct {
            GLuint ambient;  // vec4
            GLuint diffuse;  // vec4
            GLuint specular;  // vec4
            GLuint shininess;  // float
        } material;
        GLuint isLight;  // bool
        GLuint viewerPosition;  // vec4
    } sphereProgramUniforms;
    
    struct CubeProgramUniforms {
        GLuint mvpMatrix;
        GLuint textureSampler;
    } cubeProgramUniforms;
    
    enum {
        KVD_ATTRIBUTE_POSITION = 0,
        KVD_ATTRIBUTE_COLOR,
        KVD_ATTRIBUTE_NORMAL,
        KVD_ATTRIBUTE_TEXTURE0
    };
    
    bool isLight;
    float thetaSphere, thetaCube;
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
        viewWidth = backingWidth;
        viewHeight = backingHeight;
        
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

-(bool) createFBO: (unsigned int) width : (unsigned int) height
{
    // code
    glGenFramebuffers(1, &fboSphere);
    glBindFramebuffer(GL_FRAMEBUFFER, fboSphere);
    {
        // depth attachment
        glGenRenderbuffers(1, &rboDepthSphere);
        glBindRenderbuffer(GL_RENDERBUFFER, rboDepthSphere);
        {
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepthSphere);
        }
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        
        // color attachment
        glGenTextures(1, &texColorSphere);
        glBindTexture(GL_TEXTURE_2D, texColorSphere);
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorSphere, 0);
        }
        glBindTexture(GL_TEXTURE_2D, 0);
        
        // set the drawbuffer
        GLenum drawBuffer = GL_COLOR_ATTACHMENT0;
        glDrawBuffers(1, &drawBuffer);
        
        // check status
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            printf("createFBO: framebuffer is not complete\n");
            return false;
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    return true;
}

-(bool) initializeSphere: (unsigned int) texWidth : (unsigned int) texHeight
{
    // code
    if(![self createFBO: FBO_WIDTH : FBO_HEIGHT])
    {
        printf("initializeSphere: createFBO failed\n");
        return false;
    }
    
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
        "} light[3];\n" \
        "\n" \
        "uniform vec4 viewerPosition;\n" \
        "\n" \
        "out VS_OUT {\n" \
        "   vec3 lightDirection[3];\n" \
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
        "   vs_out.lightDirection[0] = normalize(light[0].position - eyeCoordinate).xyz;\n" \
        "   vs_out.lightDirection[1] = normalize(light[1].position - eyeCoordinate).xyz;\n" \
        "   vs_out.lightDirection[2] = normalize(light[2].position - eyeCoordinate).xyz;\n" \
        "   vs_out.viewDirection = normalize(viewerPosition - eyeCoordinate).xyz;\n" \
        "   vs_out.normal = normalize(normalMatrix * a_normal);\n" \
        "}\n";

    GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderObject, 1, (const GLchar **)&vertexShaderSourceCode, NULL);
    glCompileShader(vertexShaderObject);
    glGetShaderiv(vertexShaderObject, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        printf("*** sphere: vertex shader compilation errors ***\n");

        glGetShaderiv(vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            infoLog = (char *)malloc(infoLogLength * sizeof(char));
            if (infoLog)
            {
                GLsizei written = 0;

                glGetShaderInfoLog(vertexShaderObject, infoLogLength * sizeof(char), &written, infoLog);
                printf("sphere: vertex shader compilation log (%d bytes):\n%s\n", written, infoLog);
                free(infoLog);
                infoLog = NULL;
            }
            else
                printf("\tmalloc: cannot allocate memory to hold the compilation log\n");
        }
        else
            printf("\tthere is nothing to print\n");

        return false;
    }
    printf("sphere: vertex shader was compiled without errors\n");

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
        "   vec3 lightDirection[3];\n" \
        "   vec3 viewDirection;\n" \
        "   vec3 normal;\n" \
        "} fs_in;\n" \
        "\n" \
        "uniform struct Light {\n" \
        "   vec4 ambient;\n" \
        "   vec4 diffuse;\n" \
        "   vec4 specular;\n" \
        "   vec4 position;\n" \
        "} light[3];\n" \
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
        "   if(isLight) {\n" \
        "       vec3 lightDirection[3];\n" \
        "       lightDirection[0] = normalize(fs_in.lightDirection[0]);\n" \
        "       lightDirection[1] = normalize(fs_in.lightDirection[1]);\n" \
        "       lightDirection[2] = normalize(fs_in.lightDirection[2]);\n" \
        "       vec3 viewDirection = normalize(fs_in.viewDirection);\n" \
        "       vec3 normal = normalize(fs_in.normal);\n" \
        "       \n" \
        "       vec3 reflectedDirection[3];\n" \
        "       reflectedDirection[0] = reflect(-lightDirection[0], normal);\n" \
        "       reflectedDirection[1] = reflect(-lightDirection[1], normal);\n" \
        "       reflectedDirection[2] = reflect(-lightDirection[2], normal);\n" \
        "       \n" \
        "       vec4 ambient = (light[0].ambient + light[1].ambient + light[2].ambient) * material.ambient;\n" \
        "       \n" \
        "       vec4 diffuse = light[0].diffuse * material.diffuse * max(dot(normal, lightDirection[0]), 0.0);\n" \
        "       diffuse += light[1].diffuse * material.diffuse * max(dot(normal, lightDirection[1]), 0.0);\n" \
        "       diffuse += light[2].diffuse * material.diffuse * max(dot(normal, lightDirection[2]), 0.0);\n" \
        "       \n" \
        "       /* reflect() seems to return positive even for antiparallel reflected vectors wrt normal;\n" \
        "          the if-statement below is a sad fix atleast until I don't find my mistake... */\n" \
        "       vec4 specular = vec4(0.0);\n" \
        "       if(dot(reflectedDirection[0], normal) > 0.0) {\n" \
        "           float specularFactor = pow(max(dot(viewDirection, reflectedDirection[0]), 0.0), material.shininess);\n" \
        "           specular += light[0].specular * material.specular * specularFactor;\n" \
        "       }\n" \
        "       if(dot(reflectedDirection[1], normal) > 0.0) {\n" \
        "           float specularFactor = pow(max(dot(viewDirection, reflectedDirection[1]), 0.0), material.shininess);\n" \
        "           specular += light[1].specular * material.specular * specularFactor;\n" \
        "       }\n" \
        "       if(dot(reflectedDirection[2], normal) > 0.0) {\n" \
        "           float specularFactor = pow(max(dot(viewDirection, reflectedDirection[2]), 0.0), material.shininess);\n" \
        "           specular += light[2].specular * material.specular * specularFactor;\n" \
        "       }\n" \
        "       FragColor = ambient + diffuse + specular;\n" \
        "   } else {\n" \
        "       FragColor = vec4(1.0);\n" \
        "   }\n" \
        "}\n";

    GLuint fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderObject, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);
    glCompileShader(fragmentShaderObject);
    glGetShaderiv(fragmentShaderObject, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        printf("*** sphere: fragment shader compilation errors ***\n");

        glGetShaderiv(fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            infoLog = (char *)malloc(infoLogLength * sizeof(char));
            if (infoLog)
            {
                GLsizei written = 0;

                glGetShaderInfoLog(fragmentShaderObject, infoLogLength, &written, infoLog);
                printf("sphere: fragment shader compilation log (%d bytes):\n%s\n", written, infoLog);
                free(infoLog);
                infoLog = NULL;
            }
            else
                printf("\tmalloc: cannot allocate memory to hold the compilation log\n");
        }
        else
            printf("\tthere is nothing to print\n");

        return false;
    }
    printf("sphere: fragment shader was compiled without errors\n");

    // shader program object
    status = 0;
    infoLogLength = 0;
    infoLog = NULL;

    shaderProgramObject_Sphere = glCreateProgram();
    glAttachShader(shaderProgramObject_Sphere, vertexShaderObject);
    glAttachShader(shaderProgramObject_Sphere, fragmentShaderObject);
    glBindAttribLocation(shaderProgramObject_Sphere, KVD_ATTRIBUTE_POSITION, "a_position");
    glBindAttribLocation(shaderProgramObject_Sphere, KVD_ATTRIBUTE_NORMAL, "a_normal");
    glLinkProgram(shaderProgramObject_Sphere);
    glGetProgramiv(shaderProgramObject_Sphere, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        printf("*** sphere: there were linking errors ***\n");

        glGetProgramiv(shaderProgramObject_Sphere, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            infoLog = (char *)malloc(infoLogLength * sizeof(char));
            if (infoLog)
            {
                GLsizei written = 0;

                glGetProgramInfoLog(shaderProgramObject_Sphere, infoLogLength * sizeof(char), &written, infoLog);
                printf("\tsphere: link time info log (%d bytes):\n%s\n", written, infoLog);
                free(infoLog);
                infoLog = NULL;
            }
            else
                printf("\tmalloc: cannot allocate memory to hold the linking log\n");
        }
        else
            printf("\tthere is nothing to print\n");

        return false;
    }
    printf("sphere: shader program was linked without errors\n");

    /* post link processing */
    sphereProgramUniforms.isLight = glGetUniformLocation(shaderProgramObject_Sphere, "isLight");
    sphereProgramUniforms.transformations.model = glGetUniformLocation(shaderProgramObject_Sphere, "transformations.model");
    sphereProgramUniforms.transformations.view = glGetUniformLocation(shaderProgramObject_Sphere, "transformations.view");
    sphereProgramUniforms.transformations.projection = glGetUniformLocation(shaderProgramObject_Sphere, "transformations.projection");
    sphereProgramUniforms.light[0].ambient = glGetUniformLocation(shaderProgramObject_Sphere, "light[0].ambient");
    sphereProgramUniforms.light[0].diffuse = glGetUniformLocation(shaderProgramObject_Sphere, "light[0].diffuse");
    sphereProgramUniforms.light[0].specular = glGetUniformLocation(shaderProgramObject_Sphere, "light[0].specular");
    sphereProgramUniforms.light[0].position = glGetUniformLocation(shaderProgramObject_Sphere, "light[0].position");
    sphereProgramUniforms.light[1].ambient = glGetUniformLocation(shaderProgramObject_Sphere, "light[1].ambient");
    sphereProgramUniforms.light[1].diffuse = glGetUniformLocation(shaderProgramObject_Sphere, "light[1].diffuse");
    sphereProgramUniforms.light[1].specular = glGetUniformLocation(shaderProgramObject_Sphere, "light[1].specular");
    sphereProgramUniforms.light[1].position = glGetUniformLocation(shaderProgramObject_Sphere, "light[1].position");
    sphereProgramUniforms.light[2].ambient = glGetUniformLocation(shaderProgramObject_Sphere, "light[2].ambient");
    sphereProgramUniforms.light[2].diffuse = glGetUniformLocation(shaderProgramObject_Sphere, "light[2].diffuse");
    sphereProgramUniforms.light[2].specular = glGetUniformLocation(shaderProgramObject_Sphere, "light[2].specular");
    sphereProgramUniforms.light[2].position = glGetUniformLocation(shaderProgramObject_Sphere, "light[2].position");
    sphereProgramUniforms.material.ambient = glGetUniformLocation(shaderProgramObject_Sphere, "material.ambient");
    sphereProgramUniforms.material.diffuse = glGetUniformLocation(shaderProgramObject_Sphere, "material.diffuse");
    sphereProgramUniforms.material.specular = glGetUniformLocation(shaderProgramObject_Sphere, "material.specular");
    sphereProgramUniforms.material.shininess = glGetUniformLocation(shaderProgramObject_Sphere, "material.shininess");
    sphereProgramUniforms.viewerPosition = glGetUniformLocation(shaderProgramObject_Sphere, "viewerPosition");

    /* loading vertex data into pipeline */
    sphere = [[OpenGLSphere alloc] initWithSlicesAndStacks: 25 : 25];
    [sphere setPositionAttribLocation: KVD_ATTRIBUTE_POSITION];
    [sphere setNormalAttribLocation: KVD_ATTRIBUTE_NORMAL];
    [sphere generate];
    [sphere loadVerticesIntoOpenGLPipeline];
    
    // initialize lighting
    isLight = false;
    
    light[0].ambient =  vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    light[0].diffuse = vmath::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    light[0].specular = vmath::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    
    light[1].ambient =  vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    light[1].diffuse = vmath::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    light[1].specular = vmath::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    
    light[2].ambient =  vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    light[2].diffuse = vmath::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    light[2].specular = vmath::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    
    material.ambient = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    material.diffuse = vmath::vec4(0.5f, 0.2f, 0.7f, 1.0f);
    material.specular = vmath::vec4(0.7f, 0.7f, 0.7f, 1.0f);
    material.shininess = 64.0f;
    
    return true;
}

-(void) initialize
{
    // code
    if(![self initializeSphere: FBO_WIDTH: FBO_HEIGHT])
    {
        printf("initialize: initializeSphere failed\n");
        [self uninitialize];
        [self release];
        exit(0);
    }
    
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
        printf("*** cube: vertex shader compilation errors ***\n");

        glGetShaderiv(vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            infoLog = (char *)malloc(infoLogLength * sizeof(char));
            if (infoLog)
            {
                GLsizei written = 0;

                glGetShaderInfoLog(vertexShaderObject, infoLogLength * sizeof(char), &written, infoLog);
                printf("cube: vertex shader compilation log (%d bytes):\n%s\n", written, infoLog);
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
    printf("cube: vertex shader was compiled without errors\n");

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
        printf("*** cube: fragment shader compilation errors ***\n");

        glGetShaderiv(fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            infoLog = (char *)malloc(infoLogLength * sizeof(char));
            if (infoLog)
            {
                GLsizei written = 0;

                glGetShaderInfoLog(fragmentShaderObject, infoLogLength, &written, infoLog);
                printf("cube: fragment shader compilation log (%d bytes):\n%s\n", written, infoLog);
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
    printf("cube: fragment shader was compiled without errors\n");

    // shader program object
    status = 0;
    infoLogLength = 0;
    infoLog = NULL;

    shaderProgramObject_Cube = glCreateProgram();
    glAttachShader(shaderProgramObject_Cube, vertexShaderObject);
    glAttachShader(shaderProgramObject_Cube, fragmentShaderObject);
    glBindAttribLocation(shaderProgramObject_Cube, KVD_ATTRIBUTE_POSITION, "a_position");
    glBindAttribLocation(shaderProgramObject_Cube, KVD_ATTRIBUTE_TEXTURE0, "a_texCoord");
    glLinkProgram(shaderProgramObject_Cube);
    glGetProgramiv(shaderProgramObject_Cube, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        printf("*** cube: there were linking errors ***\n");

        glGetProgramiv(shaderProgramObject_Cube, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            infoLog = (char *)malloc(infoLogLength * sizeof(char));
            if (infoLog)
            {
                GLsizei written = 0;

                glGetProgramInfoLog(shaderProgramObject_Cube, infoLogLength * sizeof(char), &written, infoLog);
                printf("\tcube: link time info log (%d bytes):\n%s\n", written, infoLog);
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
    printf("cube: shader program was linked without errors\n");

    /* post link processing */
    cubeProgramUniforms.mvpMatrix = glGetUniformLocation(shaderProgramObject_Cube, "u_mvpMatrix");
    cubeProgramUniforms.textureSampler = glGetUniformLocation(shaderProgramObject_Cube, "u_textureSampler");
    
    /* loading data into pipeline */
    glGenVertexArrays(1, &vaoCube);
    glBindVertexArray(vaoCube);
    {
        // position vbo
        glGenBuffers(1, &vboCubePositions);
        glBindBuffer(GL_ARRAY_BUFFER, vboCubePositions);
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
    
    perspectiveProjectionMatrix = vmath::mat4::identity();
    
    glClearDepthf(1.0f);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
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

-(void) renderSphereScene
{
    // constants
    const vmath::vec3 viewerPosition = vmath::vec3(0.0f, 0.0f, 2.0f);
    
    // code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgramObject_Sphere);
    {
        // transforms
        sphereTransformations.model = vmath::translate(0.0f, 0.0f, -2.0f);
        sphereTransformations.view = vmath::lookat(viewerPosition, vmath::vec3(0.0f, 0.0f, 0.0f), vmath::vec3(0.0f, 1.0f, 0.0f));
        
        light[0].position = vmath::vec4(
            0.0f,
            10.0f * sinf(vmath::radians(thetaSphere)),
            10.0f * cosf(vmath::radians(thetaSphere)),
            1.0f
        );
        light[1].position = vmath::vec4(
            10.0f * cosf(vmath::radians(thetaSphere)),
            0.0f,
            10.0f * sinf(vmath::radians(thetaSphere)),
            1.0f
        );
        light[2].position = vmath::vec4(
            10.0f * cosf(vmath::radians(thetaSphere)),
            10.0f * sinf(vmath::radians(thetaSphere)),
            0.0f,
            1.0f
        );
        
        // setup uniforms
        if(isLight) {
            glUniform1i(sphereProgramUniforms.isLight, GL_TRUE);
            
            for(int i = 0; i < 3; i++) {
                glUniform4fv(sphereProgramUniforms.light[i].ambient, 1, light[i].ambient);
                glUniform4fv(sphereProgramUniforms.light[i].diffuse, 1, light[i].diffuse);
                glUniform4fv(sphereProgramUniforms.light[i].specular, 1, light[i].specular);
                glUniform4fv(sphereProgramUniforms.light[i].position, 1, light[i].position);
            }
            
            glUniform4fv(sphereProgramUniforms.material.ambient, 1, material.ambient);
            glUniform4fv(sphereProgramUniforms.material.diffuse, 1, material.diffuse);
            glUniform4fv(sphereProgramUniforms.material.specular, 1, material.specular);
            glUniform1f(sphereProgramUniforms.material.shininess, material.shininess);
            
            glUniform4fv(sphereProgramUniforms.viewerPosition, 1, vmath::vec4(viewerPosition, 1.0));
        } else {
            glUniform1i(sphereProgramUniforms.isLight, GL_FALSE);
        }
        
        glUniformMatrix4fv(sphereProgramUniforms.transformations.model, 1, GL_FALSE, sphereTransformations.model);
        glUniformMatrix4fv(sphereProgramUniforms.transformations.view, 1, GL_FALSE, sphereTransformations.view);
        glUniformMatrix4fv(sphereProgramUniforms.transformations.projection, 1, GL_FALSE, perspectiveProjectionMatrix);
        
        // draw
        [sphere render: GL_TRIANGLES];
    }
    glUseProgram(0);
}

-(void) renderScene
{
    // code
    // render to sphere FBO (texture)
    [self resize: FBO_WIDTH : FBO_HEIGHT];
    glBindFramebuffer(GL_FRAMEBUFFER, fboSphere);
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    [self renderSphereScene];
    
    // render to default FBO
    [self resize: viewWidth : viewHeight];
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgramObject_Cube);
    {
        cubeTransformations.model = vmath::translate(0.0f, 0.0f, -4.0f) *
            vmath::rotate(thetaCube, 1.0f, 0.0f, 0.0f) *
            vmath::rotate(thetaCube, 0.0f, 1.0f, 0.0f) *
            vmath::rotate(thetaCube, 0.0f, 0.0f, 1.0f) *
            vmath::scale(0.75f);
        
        glUniformMatrix4fv(cubeProgramUniforms.mvpMatrix, 1, GL_FALSE, perspectiveProjectionMatrix * cubeTransformations.model);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texColorSphere);  // use the render target of sphere program as a texture for cube
        glUniform1i(cubeProgramUniforms.textureSampler, 0);
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

-(void) updateSphereScene
{
    // code
    thetaSphere += 1.6f;
    if(thetaSphere > 360.0f)
        thetaSphere -= 360.0f;
}

-(void) updateScene
{
    // code
    [self updateSphereScene];
    
    thetaCube += 0.4f;
    if(thetaCube > 360.0f)
        thetaCube -= 360.0f;
}

-(void) uninitializeSphere
{
    // code
    if(sphere)
    {
        [sphere dealloc];
    }
    
    if(texColorSphere)
    {
        glDeleteTextures(1, &texColorSphere);
        texColorSphere = 0;
    }
    
    if(rboDepthSphere)
    {
        glDeleteRenderbuffers(1, &rboDepthSphere);
        rboDepthSphere = 0;
    }
    
    if(fboSphere)
    {
        glDeleteFramebuffers(1, &fboSphere);
        fboSphere = 0;
    }

    if (shaderProgramObject_Sphere)
    {
        GLsizei numAttachedShaders = 0;
        GLuint *shaderObjects = NULL;

        glUseProgram(shaderProgramObject_Sphere);
        
        glGetProgramiv(shaderProgramObject_Sphere, GL_ATTACHED_SHADERS, &numAttachedShaders);
        shaderObjects = (GLuint *)malloc(numAttachedShaders * sizeof(GLuint));
        glGetAttachedShaders(shaderProgramObject_Sphere, numAttachedShaders, &numAttachedShaders, shaderObjects);

        for (GLsizei i = 0; i < numAttachedShaders; i++)
        {
            glDetachShader(shaderProgramObject_Sphere, shaderObjects[i]);
            glDeleteShader(shaderObjects[i]);
            shaderObjects[i] = 0;
        }
        free(shaderObjects);
        shaderObjects = NULL;
        printf("sphere program: detached and deleted %d shader objects\n", numAttachedShaders);

        glUseProgram(0);
        glDeleteProgram(shaderProgramObject_Sphere);
        shaderProgramObject_Sphere = 0;
        printf("sphere program: deleted shader program object\n");
    }
}

-(void) uninitialize
{
    // code
    [self uninitializeSphere];
    
    if(vboCubeTexCoords)
    {
        glDeleteBuffers(1, &vboCubeTexCoords);
        vboCubeTexCoords = 0;
    }
    
    if(vboCubePositions)
    {
        glDeleteBuffers(1, &vboCubePositions);
        vboCubePositions = 0;
    }
    
    if(vaoCube)
    {
        glDeleteVertexArrays(1, &vaoCube);
        vaoCube = 0;
    }
    
    if (shaderProgramObject_Cube)
    {
        GLsizei numAttachedShaders = 0;
        GLuint *shaderObjects = NULL;

        glUseProgram(shaderProgramObject_Cube);
        
        glGetProgramiv(shaderProgramObject_Cube, GL_ATTACHED_SHADERS, &numAttachedShaders);
        shaderObjects = (GLuint *)malloc(numAttachedShaders * sizeof(GLuint));
        glGetAttachedShaders(shaderProgramObject_Cube, numAttachedShaders, &numAttachedShaders, shaderObjects);

        for (GLsizei i = 0; i < numAttachedShaders; i++)
        {
            glDetachShader(shaderProgramObject_Cube, shaderObjects[i]);
            glDeleteShader(shaderObjects[i]);
            shaderObjects[i] = 0;
        }
        free(shaderObjects);
        shaderObjects = NULL;
        printf("cube program: detached and deleted %d shader objects\n", numAttachedShaders);

        glUseProgram(0);
        glDeleteProgram(shaderProgramObject_Cube);
        shaderProgramObject_Cube = 0;
        printf("cube program: deleted shader program object\n");
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
