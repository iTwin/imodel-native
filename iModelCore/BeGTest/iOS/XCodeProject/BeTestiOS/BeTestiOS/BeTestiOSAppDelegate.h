/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#import "BeTestGLView.h"

@interface BeTestiOSAppDelegate : UIResponder <UIApplicationDelegate>

@property (strong, nonatomic) UIViewController* RootViewController;
@property (strong, nonatomic) UIWindow* Window;
@property (strong, nonatomic) IBOutlet BeTestGLView* View;

@end
