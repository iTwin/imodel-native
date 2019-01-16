//
//  AssetHandler.m
//  iModelJsApp
//
//  Created by Affan Khan on 12/20/18.
//  Copyright © 2018 Bentley Systems. All rights reserved.
//

#import "AssetHandler.h"

@implementation AssetHandler
NSString* _assetPath;
NSString* _mainPage;
-(instancetype) initWithAssetPath: (NSString*)path {
    _assetPath = path;
    _mainPage = @"index.html";
    return [self init];
}
/**
 * Method name: initWithAssetPathAndDefaultPage
 * Description: create new instance
 */
-(instancetype) initWithAssetPathAndDefaultPage: (NSString*)path :(NSString*)mainPage {
    _assetPath = path;
    _mainPage = mainPage;
    return [self init];
}
/**
 * Method name: cancelWithFileNotFound
 * Description: cancel URL request with file not found
 */
- (void) cancelWithFileNotFound: (id<WKURLSchemeTask>)urlSchemeTask {
    NSLog(@"FAIL [%@]", urlSchemeTask.request.URL.absoluteString );
    NSURLResponse* taskReponse = [[NSURLResponse alloc]
                                  initWithURL:urlSchemeTask.request.URL
                                  MIMEType:@"text"
                                  expectedContentLength:-1
                                  textEncodingName:@"utf8"];
    [urlSchemeTask didReceiveResponse:taskReponse];
    [urlSchemeTask didReceiveData:[NSData new]];
    //Above step are require otherwise following give accessvoilation. Reported to apple
    [urlSchemeTask didFailWithError:[[NSError alloc] initWithDomain:NSURLErrorDomain code:NSURLErrorFileDoesNotExist userInfo: nil]];
}
/**
 * Method name: tryGetFileUrl
 * Description: Try to get url to disk file if possiable or return nil.
 */
- (NSURL*) tryGetFileUrl: (id<WKURLSchemeTask>)urlSchemeTask {
    NSString *appFolderPath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:_assetPath];
    NSString* fileURLStr = [appFolderPath stringByAppendingPathComponent:urlSchemeTask.request.URL.path];
    if ([urlSchemeTask.request.URL.path isEqualToString:@"/"] || [urlSchemeTask.request.URL.path isEqualToString:@""]) {
        fileURLStr = [fileURLStr stringByAppendingPathComponent:_mainPage];
    }
    
    NSURL* fileURL = [NSURL fileURLWithPath:fileURLStr isDirectory:NO];
    if ([NSFileManager.defaultManager fileExistsAtPath:fileURL.path])
        return fileURL;
    
    return nil;
}
/**
 * Method name: canHandle
 * Description: check if a Url can be handled
 */
- (BOOL) canHandle: (NSURL*)url {
    return NO;
}
/**
 * Method name: handle
 * Description: handle the url
 */
- (void) handle: (id<WKURLSchemeTask>)urlSchemeTask {
}

-(void) respondWithDiskFile:(id<WKURLSchemeTask>)urlSchemeTask :(NSURL*)fileURL {
    __block typeof(urlSchemeTask) urlSchemeTaskRef = urlSchemeTask;
    [[[NSURLSession sharedSession] dataTaskWithURL:fileURL
                                 completionHandler:^(NSData *data,
                                                     NSURLResponse *response,
                                                     NSError *error) {
                                     if(error) {
                                         [self cancelWithFileNotFound:urlSchemeTask];
                                         return;
                                     }
                                     NSURLResponse* taskReponse = [[NSURLResponse alloc]
                                                                   initWithURL:urlSchemeTaskRef.request.URL
                                                                   MIMEType:response.MIMEType
                                                                   expectedContentLength:response.expectedContentLength
                                                                   textEncodingName:response.textEncodingName];
                                     [urlSchemeTaskRef didReceiveResponse:taskReponse];
                                     [urlSchemeTaskRef didReceiveData:data];
                                     [urlSchemeTaskRef didFinish];
                                     NSLog(@"OK [%@]", urlSchemeTaskRef.request.URL.absoluteString );
                                 }] resume];
    
}
- (void)webView:(WKWebView *)webView startURLSchemeTask:(id<WKURLSchemeTask>)urlSchemeTask {
     NSLog(@"REQ [%@]", urlSchemeTask.request.URL.absoluteString );
    NSURL* fileUrl = [self tryGetFileUrl: urlSchemeTask];
    if (fileUrl != nil) {
        [self respondWithDiskFile:urlSchemeTask :fileUrl];
    } else if ([self canHandle:urlSchemeTask.request.URL]) {
        [self handle:urlSchemeTask];
    } else {
        [self cancelWithFileNotFound:urlSchemeTask];
    }
}
- (void)webView:(WKWebView *)webView stopURLSchemeTask:(id<WKURLSchemeTask>)urlSchemeTask {
}
@end
