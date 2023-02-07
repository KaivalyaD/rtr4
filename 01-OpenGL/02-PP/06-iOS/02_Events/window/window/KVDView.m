//
//  KVDView.m
//  window
//
//  Created by Kaivalya Deshpande on 24/12/22.
//

#import "KVDView.h"

@implementation KVDView

-(id) initWithFrame:(CGRect)frame
{
    // code
    self = [super initWithFrame:frame];
    if(self)
    {
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
    printf("Single tap detected\n");
}

-(void) onDoubleTap:(UITapGestureRecognizer *)gestureRecognizer
{
    // code
    printf("Double tap detected\n");
}

-(void) onSwipe:(UISwipeGestureRecognizer *)gestureRecognizer
{
    // code
    printf("Swipe detected... exiting\n");
    [self release];
    exit(0);
}

-(void) onLongPress:(UILongPressGestureRecognizer *)gestureRecognizer
{
    // code
    printf("Long press detected\n");
}

-(void) dealloc
{
    // code
    [super dealloc];
}

@end
