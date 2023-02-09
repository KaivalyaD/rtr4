//
//  Mat4Stack.h
//  window
//
//  Created by Kaivalya Deshpande.
//

#ifndef Mat4Stack_h
#define Mat4Stack_h

#import <Foundation/Foundation.h>
#import "vmath.h"

// macros
#define MAX_STACK_SIZE 32

// Mat4Stack interface declaration
@interface Mat4Stack: NSObject
-(id) init;
-(vmath::mat4) top;
-(void) push: (vmath::mat4) mat;
-(void) pop;
-(void) dealloc;
@end

#endif /* Mat4Stack_h */
