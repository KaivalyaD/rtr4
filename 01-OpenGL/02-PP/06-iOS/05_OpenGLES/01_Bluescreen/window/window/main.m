//
//  main.m
//  window
//
//  Created by Kaivalya Deshpande on 24/12/22.
//

#import <UIKit/UIKit.h>
#import "AppDelegate.h"

int main(int argc, char *argv[])
{
    // code
    // start NSObject auto reference counting
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    NSString *delegateClassName = NSStringFromClass([AppDelegate class]);
    
    int result = UIApplicationMain(argc, argv, nil, delegateClassName);
    
    [pool release];
    
    return result;
}
