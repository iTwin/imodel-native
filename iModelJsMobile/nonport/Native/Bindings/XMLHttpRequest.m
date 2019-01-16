//
//  XMLHttpRequest.m
//  TestApp
//
//  Created by Affan Khan on 11/6/18.
//  Copyright © 2018 Affan Khan. All rights reserved.
//

#import "XMLHttpRequest.h"

@implementation XMLHttpRequest {
    NSURLSession *_urlSession;
    NSString *_method;
    NSString *_user;
    NSString *_password;
    JSContext *_ctx;
    NSURL *_url;
    bool _async;
    NSMutableDictionary *_requestHeaders;
    NSDictionary *_responseHeaders;
    dispatch_semaphore_t _semaphore;
    NSURLSessionTask *_task;
    bool _aborted;
    NSString* _downloadFile;
    NSError *_error;
};

@synthesize response;
@synthesize responseText;
@synthesize responseType;
@synthesize onreadystatechange;
@synthesize readyState;
@synthesize onload;
@synthesize onerror;
@synthesize status;
@synthesize statusText;
@synthesize ontimeout;
@synthesize responseURL;
@synthesize timeout;

- (instancetype)init {
    // UrlCache
    /*
    NSURLCache* urlCache = [NSURLCache sharedURLCache];
    NSHTTPCookieStorage* cookieStorage = [NSHTTPCookieStorage sharedHTTPCookieStorage];
    NSString *sessionConfigurationIdentifier = [[NSBundle mainBundle] bundleIdentifier];

    //Config
    NSURLSessionConfiguration* configuration = [NSURLSessionConfiguration backgroundSessionConfigurationWithIdentifier:sessionConfigurationIdentifier];
    [configuration setRequestCachePolicy:NSURLRequestReturnCacheDataElseLoad];
    [configuration setTimeoutIntervalForResource:0];
    [configuration setDiscretionary:YES];
    [configuration setSessionSendsLaunchEvents:YES];
    [configuration setURLCache:urlCache];
    [configuration setHTTPCookieStorage:cookieStorage];
    */
    NSURLSessionConfiguration* configuration = NSURLSessionConfiguration.defaultSessionConfiguration;
    configuration.timeoutIntervalForRequest = 30.0;
    configuration.timeoutIntervalForResource = 60.0;
    return [self initWithURLSession:[NSURLSession sessionWithConfiguration:configuration]];
    // return [self initWithURLSession:[NSURLSession sessionWithConfiguration:NSURLSessionConfiguration.defaultSessionConfiguration]];
}

- (instancetype)initWithURLSession:(NSURLSession *)urlSession {
    if (self = [super init]) {
        _urlSession = urlSession;
        _requestHeaders = [NSMutableDictionary new];
        self.readyState = @(UNSENT);
        _semaphore = NULL;
        _task = NULL;
        _aborted =false;
        _error = NULL;
    }
    return self;
}
-(void)dealloc {
    NSLog(@"dealloc() XmlHttpRequest %@", _url.absoluteString);
}

- (void)extend:(id)jsContext {
    if (![jsContext isKindOfClass:[JSContext class]]) {
        NSException *e = [NSException
                          exceptionWithName:@"InvalidTypeException"
                          reason:@"Expecting JSContext"
                          userInfo:nil];
        @throw e;
    }
    JSContext* ctx = ((JSContext*)jsContext);
    

  
    // Simulate the constructor.
    if (ctx[@"XMLHttpRequest"].isUndefined) {
          __block typeof(_ctx) ct = ctx;
   
        ctx[@"XMLHttpRequest"] = ^{
            XMLHttpRequest* xdr = [XMLHttpRequest new];
            [xdr extend:ct];
            return xdr;
        };
        ctx[@"XMLHttpRequest"][@"UNSENT"] = @(UNSENT);
        ctx[@"XMLHttpRequest"][@"OPENED"] = @(OPENED);
        ctx[@"XMLHttpRequest"][@"LOADING"] = @(LOADING);
        ctx[@"XMLHttpRequest"][@"HEADERS"] = @(HEADERS_RECEIVED);
        ctx[@"XMLHttpRequest"][@"DONE"] = @(DONE);
    }

    _ctx = ctx;
}

- (void) open:(JSValue*)method :(JSValue*)url :(JSValue*)async :(JSValue*)user :(JSValue*)password {
    NSLog(@"open: %@", url.toString);
    if (_task) {
        [_ctx setException:[JSValue valueWithNewErrorFromMessage:@"there is already a send() in progress" inContext:_ctx]];
        return;
    }
    _password =  [password isUndefined]? NULL : [password toString];
    _user =  [user isUndefined]? NULL : [user toString];
    _async =  [async isUndefined]? true : [async toBool];
    _url =  [url isUndefined]? NULL : [NSURL URLWithString:[url toString]];
    _method =  [method isUndefined]? NULL : [method toString];
    _error = NULL;
    if (!_url) {
        [_ctx setException:[JSValue valueWithNewErrorFromMessage:@"url is required parameter" inContext:_ctx]];
        return;
    }
    if (!_method) {
        [_ctx setException:[JSValue valueWithNewErrorFromMessage:@"method is required parameter" inContext:_ctx]];
        return;
    }
    
    if (!_async) {
        _semaphore = dispatch_semaphore_create(0);
    }
    self.readyState = @(OPENED);
}

- (void)setRequestHeader:(NSString *)name :(NSString *)value {
    _requestHeaders[name] = value;
}

- (void)setAllResponseHeaders:(NSDictionary *)responseHeaders {
    _responseHeaders = responseHeaders;
}

- (void)downloadFile:(NSString*)fileName {
    _downloadFile  = fileName;
    _aborted = false;
    _error = nil;
    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:_url];
    for (NSString *name in _requestHeaders) {
        [request setValue:_requestHeaders[name] forHTTPHeaderField:name];
    }
    [request setHTTPMethod:_method];
    __block __weak typeof(self) weakSelf = self;
    __block typeof(_downloadFile) fileUrl = _downloadFile;
    id completionHandler = ^(NSURL *location, NSURLResponse *response, NSError *error){
        if (error) {
            _error = error;
            if (weakSelf.onerror != nil) {
                [weakSelf.onerror callWithArguments:@[]];
            }
            NSLog(@"downloadTaskWithRequest failed: %@", error);
            return;
        }
        
        weakSelf.status = [NSNumber  numberWithInt:200];
        NSFileManager *fileManager = [NSFileManager defaultManager];
        NSURL *fileURL = [NSURL fileURLWithPath:fileUrl];
        NSError *moveError;
        if (![fileManager moveItemAtURL:location toURL:fileURL error:&moveError]) {
            if (weakSelf.onerror != nil) {
                [weakSelf.onerror callWithArguments:@[]];
            }
            NSLog(@"moveItemAtURL failed: %@", moveError);
            return;
        }
        NSLog(@"[XHR] DOWNLOADED %@ ", location.absoluteString);
        NSLog(@"[XHR] MOVING %@", fileURL.absoluteString);
        weakSelf.responseType = @"file";
        weakSelf.responseText =location.absoluteString;
        if (weakSelf.onreadystatechange != nil) {
            [weakSelf.onreadystatechange callWithArguments:@[]];
        }
        if (weakSelf.onload != nil) {
            [weakSelf.onload callWithArguments:@[]];
        }
    };
 
   _task = [_urlSession downloadTaskWithRequest:request
                         completionHandler:completionHandler];
    [_task resume];
}
- (void)completionHandlerEx:(NSData*)receivedData :(NSURLResponse*)clientResponse :(NSError*)error {
 NSLog(@"complete: %@", _url.absoluteString);
        if (error) {
            _error = error;
            NSLog(@"Error1: %@   %@", _url.absoluteString , error.description);
            if (_semaphore) {              
                dispatch_semaphore_signal(_semaphore);
            }
            return;
        }
        NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *) clientResponse;
        [self setAllResponseHeaders:[httpResponse allHeaderFields]];
        self.readyState = @(DONE); // TODO
        self.status = @(httpResponse.statusCode);
        self.statusText = [NSString stringWithFormat:@"%ld",httpResponse.statusCode];
        if ([httpResponse statusCode] != 200 /*OK*/ || error != NULL) {
            if (self.onerror != nil) {
                [self.onerror callWithArguments:@[]];
            }
            if (error) {
                NSLog(@"Error: %@", [error description]);
            } else {
                NSLog(@"Error: HTTP statusCode = %ld", (long)[httpResponse statusCode]);
            }
            return;
        }
        NSString* responseTypeStr = @"";
        NSString* mimeType = [httpResponse MIMEType];
        if ([mimeType containsString:@"text"])
            responseTypeStr = @"text";
        else if ([mimeType containsString:@"octet-stream"] || [mimeType containsString:@"image"])
            responseTypeStr = @"arraybuffer";
        else if([mimeType containsString:@"json"])
            responseTypeStr = @"json";
         //NSLog(@"responseText: %@", weakSelf.responseText);
        self.responseType = responseTypeStr;
        //arraybuffer/blob/document/json/text
        //weakSelf.response = weakSelf.responseText;
        
        if ( [responseTypeStr caseInsensitiveCompare:@"blob"] == NSOrderedSame ||
            [responseTypeStr caseInsensitiveCompare:@"arraybuffer"] == NSOrderedSame) {
            // create a array
            const NSUInteger length = [receivedData length];
            const void* src = [receivedData bytes];
            JSGlobalContextRef ctxRef = [_ctx JSGlobalContextRef];
        
            JSObjectRef binArray = JSObjectMakeTypedArray(ctxRef, kJSTypedArrayTypeUint8Array, length, NULL);
            void* dst = JSObjectGetTypedArrayBytesPtr(ctxRef, binArray, NULL);
            memcpy(dst, src, length);
            
            self.response = [JSValue valueWithJSValueRef:binArray inContext:_ctx];
        } else if ([responseTypeStr caseInsensitiveCompare:@"json"] == NSOrderedSame) {
            if (receivedData.length > 0 ) {
            NSString* json= [[NSString alloc] initWithData:receivedData
                                                      encoding:NSUTF8StringEncoding];
            json = [json stringByReplacingOccurrencesOfString:@"\"\\\""
                                                     withString:@"\""];
            json = [json stringByReplacingOccurrencesOfString:@"\\\"\""
                                                   withString:@"\""];
                
            [JSValue valueWithObject:json inContext:_ctx];
            self.response =  [JSValue valueWithObject:json inContext:_ctx];
            }
          
        } else {
            self.responseText = [[NSString alloc] initWithData:receivedData
                                                          encoding:NSUTF8StringEncoding];
        }
        
        // Call this before onreadystatechange or we would have deadlock
        if (_semaphore) {
            dispatch_semaphore_signal(_semaphore);
        }
        if (self.onreadystatechange != nil && !_aborted) {
            [self.onreadystatechange callWithArguments:@[]];
        }
        if (self.onload != nil && ! !_aborted) {
            [self.onload callWithArguments:@[]];
        }
}  
- (void)send:(JSValue*)data {
    _aborted = false;
    _error = nil;
    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:_url];
    for (NSString *name in _requestHeaders) {
        [request setValue:_requestHeaders[name] forHTTPHeaderField:name];
    }

    if ([data isString]) {
        request.HTTPBody = [[data toString] dataUsingEncoding:NSUTF8StringEncoding];
    }
    else if ([data isUndefined]) {

    }
    else if ([data isNull]) {

    }
    
    else if ([data isObject]) {
        JSValue* func = [_ctx evaluateScript:@"JSON.stringify"];
        NSString* str= [func callWithArguments:@[data]].toString;
        request.HTTPBody = [str dataUsingEncoding:NSUTF8StringEncoding];
        // ToDo
    }
    else if ([data isArray]) {
        // ToDo
    }

    [request setHTTPMethod:_method];

    // application/octet-stream arrayBuffer
    // application/json         json
    // application/xml
    // text/plain
    __block __weak typeof(self) weakSelf = self;
    _task = [_urlSession dataTaskWithRequest:request
                                            completionHandler: ^(NSData* receivedData, NSURLResponse* clientResponse, NSError* error ){
                                                [weakSelf completionHandlerEx:receivedData :clientResponse :error];
                                            }];

    [_task resume];
    
    if (_semaphore) {
        dispatch_semaphore_wait(_semaphore, DISPATCH_TIME_FOREVER);
        if (_error) {
            [_ctx setException:[JSValue valueWithNewErrorFromMessage:_error.description inContext:_ctx]];
        }
    }
}

- (void)abort {
    NSLog(@"Aborted %@", _url.absoluteString);
    if (_aborted) {
        [_ctx setException:[JSValue valueWithNewErrorFromMessage:@"xhr already aboarted" inContext:_ctx]];
        return;
    }
    if (!_task) {
        [_ctx setException:[JSValue valueWithNewErrorFromMessage:@"no send() in progress" inContext:_ctx]];
        return;
    }
    _aborted = true;
    if (_task) {
        [_task cancel];
    }
}

- (void)overrideMimeType:(NSString *)mimeType {
}
- (NSString *)getAllResponseHeaders {
    NSMutableString *responseHeaders = [NSMutableString new];
    for (NSString *key in _responseHeaders) {
        [responseHeaders appendString:key];
        [responseHeaders appendString:@": "];
        [responseHeaders appendString:_responseHeaders[key]];
        [responseHeaders appendString:@"\r\n"];
    }
     return responseHeaders;
}

- (NSString *)getResponseHeader:(NSString *)name {
    return _responseHeaders[name];
}



@end
