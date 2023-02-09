//
//  Mat4Stack.mm
//  window
//
//  Created by Kaivalya Deshpande.
//

#import "Mat4Stack.h"

// implementation for Mat4Stack
@implementation Mat4Stack
{
@private
    vmath::mat4 matMV[MAX_STACK_SIZE];
    int topCntr;
}
-(id) init
{
    // code
    self = [super init];
    if(!self)
        return nil;
    
    topCntr = -1;
    return self;
}

-(vmath::mat4) top
{
    // code
    if(topCntr > -1)
        return matMV[topCntr];
    
    return vmath::mat4::identity();
}

-(void) push: (vmath::mat4) mat
{
    // code
    vmath::mat4 newTop = [self top] * mat;
    
    topCntr += 1;
    if(topCntr > MAX_STACK_SIZE - 1)
    {
        topCntr = MAX_STACK_SIZE - 1;
        return;
    }
    
    matMV[topCntr] = newTop;
}

-(void) pop
{
    // code
    if(topCntr > -1)
        topCntr -= 1;
}

-(void) dealloc
{
    // code
    [super dealloc];
}
@end
