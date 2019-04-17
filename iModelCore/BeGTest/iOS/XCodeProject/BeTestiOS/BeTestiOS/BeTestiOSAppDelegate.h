/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#import "BeTestGLView.h"

@interface BeTestiOSAppDelegate : UIResponder <UIApplicationDelegate>

@property (strong, nonatomic) UIViewController* RootViewController;
@property (strong, nonatomic) UIWindow* Window;
@property (strong, nonatomic) IBOutlet BeTestGLView* View;

@end
