//
//  ViewController.m
//  iModelJsApp
//
//  Created by Satyakam Khadilkar on 4/5/18.
//  Copyright © 2018 Bentley Systems. All rights reserved.
//

#import "ViewController.h"

@interface ViewController ()

@end

@implementation ViewController

- (void) loadView {
    WKWebViewConfiguration* webConfiguration = [[WKWebViewConfiguration alloc] init];
    WKWebView* webView = [[WKWebView alloc] initWithFrame:CGRectZero configuration:webConfiguration];
    [webView setUIDelegate:self];
    self.view = webView;
    _webView = webView;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    NSString *appFolderPath = [[NSBundle mainBundle] resourcePath];
    NSString *frontEndIndexPath = [appFolderPath stringByAppendingPathComponent:@"FrontEnd/index.html"];
    NSURL* url = [NSURL fileURLWithPath:frontEndIndexPath];
    NSURLRequest* request = [NSURLRequest requestWithURL:url];
    [self.webView loadRequest:request];
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


@end
