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
    NSURLSessionDataTask *_task;
    bool _aborted;
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
    return [self initWithURLSession:[NSURLSession sharedSession]];
}

- (instancetype)initWithURLSession:(NSURLSession *)urlSession {
    if (self = [super init]) {
        _urlSession = urlSession;
        _requestHeaders = [NSMutableDictionary new];
        self.readyState = @(UNSENT);
        _semaphore = NULL;
        _task = NULL;
        _aborted =false;
    }
    return self;
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
    ctx[@"XMLHttpRequest"] = ^{
        return self;
    };
    ctx[@"XMLHttpRequest"][@"UNSENT"] = @(UNSENT);
    ctx[@"XMLHttpRequest"][@"OPENED"] = @(OPENED);
    ctx[@"XMLHttpRequest"][@"LOADING"] = @(LOADING);
    ctx[@"XMLHttpRequest"][@"HEADERS"] = @(HEADERS_RECEIVED);
    ctx[@"XMLHttpRequest"][@"DONE"] = @(DONE);
    _ctx = ctx;
}

- (void) open:(JSValue*)method :(JSValue*)url :(JSValue*)async :(JSValue*)user :(JSValue*)password {
    if (_task) {
        [_ctx setException:[JSValue valueWithNewErrorFromMessage:@"there is already a send() in progress" inContext:_ctx]];
        return;
    }
    _password =  [password isUndefined]? NULL : [password toString];
    _user =  [user isUndefined]? NULL : [user toString];
    _async =  [async isUndefined]? true : [async toBool];
    _url =  [url isUndefined]? NULL : [NSURL URLWithString:[url toString]];
    _method =  [method isUndefined]? NULL : [method toString];
    
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

- (void)send:(JSValue*)data {
    _aborted = false;
    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:_url];
    for (NSString *name in _requestHeaders) {
        [request setValue:_requestHeaders[name] forHTTPHeaderField:name];
    }

    if ([data isString]) {
        request.HTTPBody = [[data toString] dataUsingEncoding:NSUTF8StringEncoding];
    }
    else if ([data isArray]) {
        // ToDo
    }

    [request setHTTPMethod:_method];
    __block __weak XMLHttpRequest *weakSelf = self;
    __block JSContext *ctx = _ctx;
    __block dispatch_semaphore_t semaphore = _semaphore;
    __block bool aborted = _aborted;
    // application/octet-stream arrayBuffer
    // application/json         json
    // application/xml
    // text/plain
    id completionHandler = ^(NSData *receivedData, NSURLResponse *response, NSError *error) {
        NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *) response;
        [weakSelf setAllResponseHeaders:[httpResponse allHeaderFields]];
        NSLog(@"MIMEType: %@", [httpResponse MIMEType]);
        self.readyState = @(DONE); // TODO
        weakSelf.status = @(httpResponse.statusCode);
        weakSelf.statusText = [NSString stringWithFormat:@"%ld",httpResponse.statusCode];
        if ([httpResponse statusCode] != 200 /*OK*/ || error != NULL) {
            if (weakSelf.onerror != nil) {
                [weakSelf.onerror callWithArguments:@[]];
            }
            if (error) {
                NSLog(@"Error: %@", [error description]);
            } else {
                NSLog(@"Error: HTTP statusCode = %ld", (long)[httpResponse statusCode]);
            }
            return;
        }
        NSString* responseType = @"";
        NSString* mimeType = [httpResponse MIMEType];
        if ([mimeType containsString:@"text"])
            responseType = @"text";
        else if ([mimeType containsString:@"octet-stream"] || [mimeType containsString:@"image"])
            responseType = @"arraybuffer";
        else if([mimeType containsString:@"json"])
            responseType = @"json";
         //NSLog(@"responseText: %@", weakSelf.responseText);
        weakSelf.responseType = responseType;
        //arraybuffer/blob/document/json/text
        //weakSelf.response = weakSelf.responseText;
        
        if ( [responseType caseInsensitiveCompare:@"blob"] == NSOrderedSame ||
            [responseType caseInsensitiveCompare:@"arraybuffer"] == NSOrderedSame) {
            // create a array
            const NSUInteger length = [receivedData length];
            const void* src = [receivedData bytes];
            JSGlobalContextRef ctxRef = [ctx JSGlobalContextRef];
        
            JSObjectRef binArray = JSObjectMakeTypedArray(ctxRef, kJSTypedArrayTypeUint8Array, length, NULL);
            void* dst = JSObjectGetTypedArrayBytesPtr(ctxRef, binArray, NULL);
            memcpy(dst, src, length);
            
            weakSelf.response = [JSValue valueWithJSValueRef:binArray inContext:ctx];
        } else if ([responseType caseInsensitiveCompare:@"json"] == NSOrderedSame) {
            weakSelf.responseText = [[NSString alloc] initWithData:receivedData
                                                      encoding:NSUTF8StringEncoding];
            NSMutableString* js = [NSMutableString new];
            [js appendString:@"JSON.parse(\""];
            [js appendString:self.responseText];
            [js appendString:@"\");"];
            weakSelf.response = [ctx evaluateScript:js];
        } else {
            weakSelf.responseText = [[NSString alloc] initWithData:receivedData
                                                          encoding:NSUTF8StringEncoding];
        }
        
        // Call this before onreadystatechange or we would have deadlock
        if (semaphore) {
            dispatch_semaphore_signal(semaphore);
        }
        if (weakSelf.onreadystatechange != nil && !aborted) {
            [weakSelf.onreadystatechange callWithArguments:@[]];
        }
        if (weakSelf.onload != nil && ! !aborted) {
            [weakSelf.onload callWithArguments:@[]];
        }
        
    };
    _task = [_urlSession dataTaskWithRequest:request
                                            completionHandler:completionHandler];
    [_task resume];
    if (_semaphore) {
        dispatch_semaphore_wait(_semaphore, DISPATCH_TIME_FOREVER);
    }
}

- (void)abort {
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

- (NSString *)getResponseHeader {
    NSMutableString *responseHeaders = [NSMutableString new];
    for (NSString *key in _responseHeaders) {
        [responseHeaders appendString:key];
        [responseHeaders appendString:@": "];
        [responseHeaders appendString:_responseHeaders[key]];
        [responseHeaders appendString:@"\r\n"];
    }
    return responseHeaders;
}

- (void)overrideMimeType:(NSString *)mimeType {
    
}

@end
