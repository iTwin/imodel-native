//
//  XMLHttpRequest.m
//  TestApp
//
//  Created by Affan Khan on 11/6/18.
//  Copyright © 2018 Affan Khan. All rights reserved.
//

#import "XMLHttpRequest.h"
#import <JavaScriptCore/JavaScriptCore.h>
#import  <IModelJsHost/IModelJsHost.h>
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
    NSString* _uploadFile;

    JSManagedValue* _onload;
    JSManagedValue* _onerror;
    JSManagedValue* _onreadystatechange;
    JSManagedValue* _withCredentials;
};

-(void)setWithCredentials:(JSValue *)withCredentials {
    _withCredentials = [JSManagedValue managedValueWithValue:withCredentials];
    [[[JSContext currentContext] virtualMachine] addManagedReference:_withCredentials withOwner:self];
}

-(JSValue*)withCredentials { return _withCredentials.value; }


-(void)setOnload:(JSValue *)onload {
    _onload = [JSManagedValue managedValueWithValue:onload];
    [[[JSContext currentContext] virtualMachine] addManagedReference:_onload withOwner:self];
}

-(JSValue*)onload { return _onload.value; }

-(void)setOnerror:(JSValue *)onerror {
    _onerror = [JSManagedValue managedValueWithValue:onerror];
    [[[JSContext currentContext] virtualMachine] addManagedReference:_onerror withOwner:self];
}

-(JSValue*)onerror { return _onerror.value; }

-(void)setOnreadystatechange:(JSValue *)onreadystatechange {
    _onreadystatechange = [JSManagedValue managedValueWithValue:onreadystatechange];
    [[[JSContext currentContext] virtualMachine] addManagedReference:_onreadystatechange withOwner:self];
}
-(JSValue*)onreadystatechange { return _onreadystatechange.value; }

@synthesize response;
@synthesize responseText;
@synthesize responseType;
@synthesize readyState;
@synthesize status;
@synthesize statusText;
@synthesize ontimeout;
@synthesize responseURL;
@synthesize timeout;
@synthesize errorObj;
@synthesize onprogress;
// @synthesize withCredentials;

- (instancetype)init {
    NSURLSessionConfiguration* configuration = NSURLSessionConfiguration.defaultSessionConfiguration;
    configuration.timeoutIntervalForRequest = 500.0;
    configuration.HTTPMaximumConnectionsPerHost = 8;
    return [self initWithURLSession:[NSURLSession sessionWithConfiguration:configuration delegate:self delegateQueue:nil]];
}

- (instancetype)initWithURLSession:(NSURLSession *)urlSession {
    if (self = [super init]) {
        _urlSession = urlSession;
        _requestHeaders = [NSMutableDictionary new];
        self.readyState = @(UNSENT);
        _semaphore = NULL;
        _task = NULL;
        _aborted =false;
        self.errorObj = NULL;
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
    NSLog(@"open: %@ %@", method.toString, url.toString);
    if (_task) {
        [_ctx setException:[JSValue valueWithNewErrorFromMessage:@"there is already a send() in progress" inContext:_ctx]];
        return;
    }
    _password =  [password isUndefined]? NULL : [password toString];
    _user =  [user isUndefined]? NULL : [user toString];
    _async =  [async isUndefined]? true : [async toBool];
     NSString *unescaped = [[url toString] stringByReplacingOccurrencesOfString:@" " withString:@"%20"];
    _url =  [url isUndefined]? NULL : [NSURL URLWithString:unescaped];
    _method =  [method isUndefined]? NULL : [method toString];
    self.errorObj = NULL;
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

- (void)uploadChunk: (NSString*)fileName :(JSValue*)blkNo :(JSValue*)blkSize {
    _aborted = false;
    self.errorObj = nil;
    _uploadFile  = fileName;
    size_t blockNo = [blkNo toUInt32];
    size_t blockSize = [blkSize toUInt32];
    NSData* blockData = [self dataWithContentsOfFile:_uploadFile atOffset:blockNo*blockSize withSize:blockSize];
    if (!blockData) {
        self.status = [NSNumber  numberWithInt:200];
        if (self.onerror != nil) {
            [self.onerror callWithArguments:@[]];
        }
        NSLog(@"[XHR] DOES NOT EXISTS URL: %@,  PATH: %@", _url.absoluteString, _uploadFile);
        return;
    }
    
    
    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:_url
                                        cachePolicy:NSURLRequestUseProtocolCachePolicy
                                        timeoutInterval:500.0];
   for (NSString *name in _requestHeaders) {
       [request setValue:_requestHeaders[name] forHTTPHeaderField:name];
   }
       
   NSLog(@"[XHR] UPLOADING %@ ", _url.absoluteString);
   [request setHTTPMethod:_method];
   __block __weak typeof(self) weakSelf = self;
   id completionHandler = ^(NSData *data, NSURLResponse *response, NSError *error){
           if (error) {
               self.errorObj = error;
               if (weakSelf.onerror != nil) {
                   [weakSelf.onerror callWithArguments:@[]];
               }
               NSLog(@"[XHR] ERROR uploadingTaskWithRequest failed: %@", error);
               return;
           }
           
       weakSelf.status = [NSNumber  numberWithInt:200];
       if (weakSelf.onreadystatechange != nil) {
           [weakSelf.onreadystatechange callWithArguments:@[]];
       }
       if (weakSelf.onload != nil) {
           [weakSelf.onload callWithArguments:@[]];
       }
   };
 
    _task = [_urlSession uploadTaskWithRequest:request fromData:blockData
                        completionHandler:completionHandler];
   [_task resume];
}

- (NSData *) dataWithContentsOfFile:(NSString *)path atOffset:(off_t)offset withSize:(size_t)bytes {
    FILE *file = fopen([path UTF8String], "rb");
    if(file == NULL)
        return nil;

    void *data = malloc(bytes);  // check for NULL!
    fseeko(file, offset, SEEK_SET);
    size_t nread = fread(data, 1, bytes, file);  // check return value, in case read was short!
    if (nread == 0) {
        fclose(file);
        return nil;
    }
    if(nread < bytes) {
      data = realloc(data, nread);
    }
    fclose(file);
    // NSData takes ownership and will call free(data) when it's released
    return [NSData dataWithBytesNoCopy:data length:nread];
}

- (void)URLSession:(NSURLSession *)session downloadTask:(NSURLSessionDownloadTask *)downloadTask didWriteData:(int64_t)bytesWritten totalBytesWritten:(int64_t)totalBytesWritten totalBytesExpectedToWrite:(int64_t)totalBytesExpectedToWrite {
    // NSLog(@"[XHR] PROGRESS [bytesWritten:%lld] [totalBytesWritten:%lld] [totalBytesExpectedToWrite:%lld]",
    //      bytesWritten, totalBytesWritten, totalBytesExpectedToWrite);
    if (self.onprogress != nil) {
        JSContext* context = self.onprogress.context;
        JSValue* doneBytes = [JSValue valueWithDouble:(double)totalBytesWritten inContext:context];
        JSValue* totalBytes = [JSValue valueWithDouble:(double)totalBytesExpectedToWrite inContext:context];
        [[IModelJsHost sharedInstance] exec:self.onprogress arguments:@[doneBytes, totalBytes]];
    }
}

- (void)URLSession:(nonnull NSURLSession *)session downloadTask:(nonnull NSURLSessionDownloadTask *)downloadTask didFinishDownloadingToURL:(nonnull NSURL *)location {
    self.errorObj = nil;
    self.status = [NSNumber  numberWithInt:200];
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSURL *fileURL = [NSURL fileURLWithPath:_downloadFile];
    NSError *moveError;
    if (![fileManager moveItemAtURL:location toURL:fileURL error:&moveError]) {
      if (self.onerror != nil) {
          [self.onerror callWithArguments:@[]];
      }
      NSLog(@"[XHR] ERROR moveItemAtURL failed: %@", moveError);
      return;
    }
    NSLog(@"[XHR] DOWNLOADED %@ ", location.absoluteString);
    NSLog(@"[XHR] MOVING %@", fileURL.absoluteString);
    self.responseType = @"file";
    self.responseText =location.absoluteString;
    if (self.onreadystatechange != nil) {
      [self.onreadystatechange callWithArguments:@[]];
    }
    if (self.onload != nil) {
      [self.onload callWithArguments:@[]];
    }
    if (_semaphore) {
        dispatch_semaphore_signal(_semaphore);
    }
}

- (void)downloadFile:(NSString*)fileName {
    _downloadFile  = fileName;
    _aborted = false;
    self.errorObj = nil;
    
    bool exists = [[NSFileManager defaultManager] fileExistsAtPath:_downloadFile];
    if (exists) {
        self.status = [NSNumber  numberWithInt:200];
        if (self.onerror != nil) {
            [self.onerror callWithArguments:@[]];
        }
        NSLog(@"[XHR] EXISTS URL: %@,  PATH: %@", _url.absoluteString, _downloadFile);
        return;
    }

    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:_url
                                     cachePolicy:NSURLRequestUseProtocolCachePolicy
                                     timeoutInterval:500.0];
    for (NSString *name in _requestHeaders) {
        [request setValue:_requestHeaders[name] forHTTPHeaderField:name];
    }
    
    NSLog(@"[XHR] DOWNLOADING %@ ", _url.absoluteString);
    [request setHTTPMethod:_method];
    __block __weak typeof(self) weakSelf = self;
    __block typeof(_downloadFile) fileUrl = _downloadFile;
    id completionHandler = ^(NSURL *location, NSURLResponse *response, NSError *error){
        if (error) {
            self.errorObj = error;
            if (weakSelf.onerror != nil) {
                [weakSelf.onerror callWithArguments:@[]];
            }
            NSLog(@"[XHR] ERROR downloadTaskWithRequest failed: %@", error);
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
            NSLog(@"[XHR] ERROR moveItemAtURL failed: %@", moveError);
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
    if (self.onprogress) {
        _task = [_urlSession downloadTaskWithRequest:request];
    } else {
        _task = [_urlSession downloadTaskWithRequest:request
                                   completionHandler:completionHandler];
    }
    [_task resume];
}
- (void)URLSession:(NSURLSession *)session
              task:(NSURLSessionTask *)task
didCompleteWithError:(NSError *)error {
    self.readyState = @(DONE);
    if (_aborted)
        self.status = @(5000);
    else
        self.status = @(503);
    if (self.onerror != nil) {
        [self.onerror callWithArguments:@[]];
    }
    if (self.onreadystatechange != nil ) {
        [self.onreadystatechange callWithArguments:@[]];
    }
    NSLog(@"Error: %@", [error description]);
}
- (void)cancel {
    if (_task != nil) {
        [_task cancel];
        _aborted = true;
        self.status = [NSNumber  numberWithInt:5000];
        if (self.onreadystatechange != nil) {
            [self.onreadystatechange callWithArguments:@[]];
        }
        if (_semaphore) {
            dispatch_semaphore_signal(_semaphore);
        }
    }
}

- (void)completionHandlerEx:(NSData*)receivedData :(NSURLResponse*)clientResponse :(NSError*)error {
    NSLog(@"complete: %@", _url.absoluteString);
    if (_semaphore) {
        dispatch_semaphore_signal(_semaphore);
    }
    if (error) {
        self.errorObj = error;
        NSLog(@"Error1: %@   %@", _url.absoluteString , error.description);

        return;
    }
    NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *) clientResponse;
    [self setAllResponseHeaders:[httpResponse allHeaderFields]];
    self.readyState = @(DONE); // TODO
    self.status = @(httpResponse.statusCode);
    self.statusText = [NSString stringWithFormat:@"%ld",httpResponse.statusCode];
    if (error != NULL) {
        if (self.onerror != nil) {
            [self.onerror callWithArguments:@[]];
        }
        if (self.onreadystatechange != nil ) {
            [self.onreadystatechange callWithArguments:@[]];
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

    if (self.onreadystatechange != nil && !_aborted) {
        [self.onreadystatechange callWithArguments:@[]];
    }
    if (self.onload != nil && ! !_aborted) {
        [self.onload callWithArguments:@[]];
    }
}

- (NSString *)base64String:(NSString *)str
{
    NSData *theData = [str dataUsingEncoding: NSASCIIStringEncoding];
    const uint8_t* input = (const uint8_t*)[theData bytes];
    NSInteger length = [theData length];

    static char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

    NSMutableData* data = [NSMutableData dataWithLength:((length + 2) / 3) * 4];
    uint8_t* output = (uint8_t*)data.mutableBytes;

    NSInteger i;
    for (i=0; i < length; i += 3) {
        NSInteger value = 0;
        NSInteger j;
        for (j = i; j < (i + 3); j++) {
            value <<= 8;

            if (j < length) {
                value |= (0xFF & input[j]);
            }
        }

        NSInteger theIndex = (i / 3) * 4;
        output[theIndex + 0] =                    table[(value >> 18) & 0x3F];
        output[theIndex + 1] =                    table[(value >> 12) & 0x3F];
        output[theIndex + 2] = (i + 1) < length ? table[(value >> 6)  & 0x3F] : '=';
        output[theIndex + 3] = (i + 2) < length ? table[(value >> 0)  & 0x3F] : '=';
    }
    return [[NSString alloc] initWithData:data encoding:NSASCIIStringEncoding];
}

- (void)send:(JSValue*)data {
    _aborted = false;
    self.errorObj = nil;
    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:_url];
    if (_user) {
        NSString *authStr = [NSString stringWithFormat:@"%@:%@", _user, _password];
        NSString *authValue = [NSString stringWithFormat:@"Basic %@", [self base64String: authStr]];
        [request setValue:authValue forHTTPHeaderField:@"Authorization"];
    }
    for (NSString *name in _requestHeaders) {
        [request setValue:_requestHeaders[name] forHTTPHeaderField:name];
    }
    NSLog(@"DATA %@", data.toString);
    if ([data isString]) {
        request.HTTPBody =  [[data toString] dataUsingEncoding:NSUTF8StringEncoding];
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
    __block __weak typeof(self) weakSelf = self;
    _task = [_urlSession dataTaskWithRequest:request
                                            completionHandler: ^(NSData* receivedData, NSURLResponse* clientResponse, NSError* error ){
                                                [weakSelf completionHandlerEx:receivedData :clientResponse :error];
                                            }];

    [_task resume];
    
    if (_semaphore) {
        dispatch_semaphore_wait(_semaphore, DISPATCH_TIME_FOREVER);
        if (self.errorObj) {
            [_ctx setException:[JSValue valueWithNewErrorFromMessage:self.errorObj.description inContext:_ctx]];
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
    NSLog(@"Mime Type override : %@", mimeType);
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
