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
        GLuint isLight;  // bool
        GLuint texSampler;  // sampler2D
        GLuint viewerPosition;  // vec4
    } uniforms;
    
    enum {
        KVD_ATTRIBUTE_POSITION = 0,
        KVD_ATTRIBUTE_COLOR,
        KVD_ATTRIBUTE_NORMAL,
        KVD_ATTRIBUTE_TEXTURE0
    };
    
    float theta;
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
        "   FragColor = fs_in.color * (ambient + diffuse + specular) * texture(texSampler, fs_in.texCoord);\n" \
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
    glBindAttribLocation(shaderProgramObject, KVD_ATTRIBUTE_NORMAL, "a_normal");
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

    /* loading vertex data into pipeline */
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
    texMarble = [self loadGLTexture: "Marble"];
    if(!texMarble)
    {
        printf("loadGLTexture: failed to load Marble\n");
        [self uninitialize];
        [self release];
        exit(0);
    }
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    glClearDepthf(1.0f);
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
    material.shininess = 64.0f;
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

-(void) updateScene
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
