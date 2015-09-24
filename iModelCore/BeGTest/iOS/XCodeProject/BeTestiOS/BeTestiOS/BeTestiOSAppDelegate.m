#import "BeTestiOSAppDelegate.h"

@implementation BeTestiOSAppDelegate

@synthesize RootViewController;
@synthesize Window;
@synthesize View;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    CGRect screenBounds = [[UIScreen mainScreen] bounds];

    self.RootViewController = [[UIViewController alloc] init];
    self.Window = [[UIWindow alloc] initWithFrame:screenBounds];
    self.View = [[BeTestGLView alloc] initWithFrame:screenBounds];

    [self.Window addSubview:self.View];
    [self.Window setRootViewController:self.RootViewController];
    [self.Window makeKeyAndVisible];
    
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
}

- (void)applicationWillTerminate:(UIApplication *)application
{
}

@end
