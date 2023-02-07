//
//  AppDelegate.m
//  window
//
//  Created by Kaivalya Deshpande on 24/12/22.
//

#import "AppDelegate.h"
#import "ViewController.h"
#import "KVDView.h"

@implementation AppDelegate
{
@private
    UIWindow *window;
    ViewController *viewController;  // AIG recommends view control be in hands of ViewController instead of the owning window unlike in MacOS
    KVDView *view;
}

-(BOOL) application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    // code
    window = [UIWindow new];
    [window setBackgroundColor:[UIColor blackColor]];
    
    viewController = [[ViewController alloc] init];
    [window setRootViewController:viewController];
    
    // CGRect rect = window.screen.bounds;  // modern call
    CGRect rect = [[window screen] bounds];
    view = [[KVDView alloc] initWithFrame:rect];
    
    [viewController setView:view];  // mutator: ref count of view increments here
    [view release];  // decrement it artificially
    
    [window makeKeyAndVisible];
    return YES;
}

-(void) applicationWillResignActive:(UIApplication *)application
{
    // code
}

-(void) applicationDidEnterBackground:(UIApplication *)application
{
    // code
}

-(void) applicationWillEnterForeground:(UIApplication *)application
{
    // code
}

-(void) applicationDidBecomeActive:(UIApplication *)application
{
    // code
}

-(void) applicationWillTerminate:(UIApplication *)application
{
    // code
}

-(void) dealloc
{
    // code
    [super dealloc];
    
    if(view)
    {
        [view release];
        view = nil;
    }
    if(viewController)
    {
        [viewController release];
        viewController = nil;
    }
    if(window)
    {
        [window release];
        window = nil;
    }
}

@end;
