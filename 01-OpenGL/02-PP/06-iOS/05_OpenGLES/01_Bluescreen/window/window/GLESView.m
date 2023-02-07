//
//  GLESView.m
//  window
//
//  Created by Kaivalya Deshpande on 04/02/23.
//

#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#import "GLESView.h"

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
    [self uninitialize];
    [self release];
    exit(0);
}

-(void) onLongPress:(UILongPressGestureRecognizer *)gestureRecognizer
{
    // code
}

-(void) initialize
{
    // code
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    
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
}

-(void) renderScene
{
    // code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

-(void) updateScene
{
    // code
}

-(void) uninitialize
{
    // code
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
