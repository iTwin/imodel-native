//
//  ViewController.m
//  iModelJsApp
//
//  Created by Satyakam Khadilkar on 4/5/18.
//  Copyright © 2018 Bentley Systems. All rights reserved.
//

#import "ViewController.h"
#include <iModelJs/iModelJsServicesTier.h>

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
    
    NSString* html = @"<!DOCTYPE html>"
    "<html lang='en' xmlns='http://www.w3.org/1999/xhtml'>"
    "<head>"
    "    <meta charset='utf-8' />"
    "    <title></title>"
    "    <style>"
    "        .wrapper {"
    "            text-align: center;"
    "        }"
    "        .button {"
    "            position: absolute;"
    "            top: 40%;"
    "            background-color: green;"
    "            padding: 15px 24px;"
    "            text-decoration: none;"
    "            border: none;"
    "            color: ghostwhite;"
    "        }"
    "    </style>"
    "</head>"
    "<body>"
    "    <div class='wrapper'>"
    "        <button id='launchApp' class='button'>Launch Application</button>"
    "    </div>"
    "    <script type='text/javascript'>"
    "        document.getElementById('launchApp').onclick = function () {"
    "            location.href = '$URL';"
    "        };"
    "    </script>"
    "</body>"
    "</html>";
    
    
    auto& gateway = BentleyApi::iModelJs::ServicesTier::MobileGateway::GetInstance();
    
    NSString *appFolderPath = [[NSBundle mainBundle] resourcePath];
    NSString *frontEndIndexPath = [appFolderPath stringByAppendingPathComponent:@"Assets/public/index.html"];
    
    NSURL* url = [NSURL fileURLWithPath:frontEndIndexPath];
    NSURL* url2 = [NSURL fileURLWithPath:appFolderPath];
    NSString* fragment = [NSString stringWithFormat:@"#%u", gateway.GetPort()];
    url = [NSURL URLWithString:fragment relativeToURL:url];
    
    NSURLRequest* request = [NSURLRequest requestWithURL:url];

    NSString *bootstrapHtml = [html stringByReplacingOccurrencesOfString: @"$URL" withString:[[request URL] absoluteString]];
    
    [self.webView loadHTMLString:bootstrapHtml baseURL:url2];
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


@end
