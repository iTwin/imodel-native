//
//  AppDelegate.h
//  iModelJsApp
//
//  Created by Satyakam Khadilkar on 4/5/18.
//  Copyright © 2018 Bentley Systems. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "XMLHTTPRequest.h"
#import "WindowTimers.h"
@interface AppDelegate : UIResponder <UIApplicationDelegate>

@property (strong, nonatomic) UIWindow *window;
@property (nonatomic) XMLHttpRequest *xmlHttpRequest;
@property (nonatomic) WTWindowTimers *windowTimers;
@end

