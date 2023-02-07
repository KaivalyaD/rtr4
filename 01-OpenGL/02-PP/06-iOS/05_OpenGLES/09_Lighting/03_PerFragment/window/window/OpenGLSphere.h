//
//  OpenGLSphere.h
//  window
//
//  Created by Kaivalya Deshpande.
//

#ifndef OpenGLSphere_h
#define OpenGLSphere_h

#import <Foundation/Foundation.h>
#import <OpenGLES/ES3/gl.h>
#import "vmath.h"

@interface OpenGLSphere: NSObject
-(id) initWithSlicesAndStacks: (int) _slices : (int) _stacks;
-(void) setPositionAttribLocation: (unsigned int) location;
-(void) setNormalAttribLocation: (unsigned int) location;
-(void) generate;
-(bool) loadVerticesIntoOpenGLPipeline;
-(void) render: (GLenum) mode;
-(void) dealloc;
@end

#endif /* OpenGLSphere_h */
