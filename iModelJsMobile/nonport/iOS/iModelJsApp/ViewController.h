//
//  ViewController.h
//  iModelJsApp
//
//  Created by Satyakam Khadilkar on 4/5/18.
//  Copyright © 2018 Bentley Systems. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <WebKit/WebKit.h>
#import <AppAuth/AppAuth.h>
@interface ViewController : UIViewController <WKUIDelegate, WKNavigationDelegate>
@property (nonatomic, weak, nullable, readonly) WKWebView* webView;
// property of the containing class
@property(nonatomic, strong, nullable) OIDAuthState *authState;
@end

