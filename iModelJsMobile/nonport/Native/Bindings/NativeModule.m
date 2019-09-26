//
//  NativeModule.m
//  TestApp
//
//  Created by Affan Khan on 11/21/18.
//  Copyright © 2018 Affan Khan. All rights reserved.
//

#import "NativeModule.h"

@implementation NativeModule {
    JSContext* _ctx;
    NSArray<NSString*>* _nodePath;
}
- (NSArray<NSString*>*) getSearchPaths {
    if (!_nodePath) {
        JSValue* nodePath = [_ctx evaluateScript:@"process.env.NODE_PATH"];
        if (nodePath.isString) {
            _nodePath = [nodePath.toString componentsSeparatedByString:@":"];
        } else {
            [_ctx setException:[JSValue valueWithNewErrorFromMessage:@"'process.env.NODE_PATH' must be of type string and must be set" inContext:_ctx]];
        }
    }
    return _nodePath;
}
- (JSValue*) polyfill: (JSValue*)str {
    NSString* request = str.toString;
    if ([request isEqualToString:@"stream"] ) {
        return [JSValue valueWithObject:@"stream-browserify" inContext:_ctx];
    } else if ([request isEqualToString:@"util"] ) {
        return [JSValue valueWithObject:@"node-util" inContext:_ctx];
    } else if ([request isEqualToString:@"tty"] ) {
        return [JSValue valueWithObject:@"tty-browserify" inContext:_ctx];
    }
    
    return str;
}

- (JSValue*) load:(NSString*) str {
    for(NSString* searchPath in [self getSearchPaths]) {
        NSString* file = [[searchPath stringByAppendingPathComponent:str] stringByAppendingPathExtension:@"js"];
  
        if ([NSFileManager.defaultManager fileExistsAtPath:file]) {
            NSString* dirName = [file stringByDeletingLastPathComponent];
            NSString* fileName = [file lastPathComponent];
            NSString* content = [[NSString alloc] initWithContentsOfFile:file encoding:NSUTF8StringEncoding error:nil];
            NSMutableString* wrapperArray = [NSMutableString new];
            [wrapperArray appendString:@"(function (__filename, __dirname) { "];
            [wrapperArray appendString:@"let module = { exports: {}};\n"];
            [wrapperArray appendString:@"let exports = module.exports;\n"];
            [wrapperArray appendString:content];
            [wrapperArray appendString:@"\n return module.exports;});"];
            JSValue* module = [_ctx evaluateScript:wrapperArray];
            return [module callWithArguments:@[fileName, dirName]];
        }
    }
    NSLog(@"unable to find require('%@')", str);
    return [JSValue valueWithUndefinedInContext:_ctx];
}

- (JSValue*) require: (NSString*)request {
    if ([request isEqualToString:@"vm"])
        return _ctx[@"vm"];
    else if ([request isEqualToString:@"IModelJsFs"]
             || [request isEqualToString:@"./IModelJsFs"]
             || [request isEqualToString:@"../IModelJsFs"]
             || [request isEqualToString:@"./lib/IModelJsFs.js"]
             || [request isEqualToString:@"../../IModelJsFs"])
             
        return _ctx[@"IModelJsFsModule"];
    else if ([request isEqualToString:@"util_ex"])
        return _ctx[@"util_ex"];
    else if ([request isEqualToString:@"assert"])
        return _ctx[@"assert"];
    else if ([request isEqualToString:@"native_module"] || [request isEqualToString:@"internal/bootstrap/loaders"])
        return _ctx[@"native_module"];
    else if ([request isEqualToString:@"fs"])
        return _ctx[@"fs"];
    else if ([request isEqualToString:@"multiparty"]
             || [request isEqualToString:@"form-data"]
             || [request isEqualToString:@"form-data"]
             || [request isEqualToString:@"https"]
             || [request isEqualToString:@"http"]
             || [request isEqualToString:@"zlib"]
             || [request isEqualToString:@"net"] || [request isEqualToString:@"internal/net"]
             || [request isEqualToString:@"crypto"]
        ) {
        return [JSValue valueWithUndefinedInContext:_ctx];
    }
    return [self load:request];
}

- (JSValue*) runScript: (NSString*)str {
    for(NSString* searchPath in [self getSearchPaths]) {
        NSString* file = [[searchPath stringByAppendingPathComponent:str] stringByAppendingPathExtension:@"js"];
        if ([NSFileManager.defaultManager fileExistsAtPath:file]) {
            NSString* content = [[NSString alloc] initWithContentsOfFile:file encoding:NSUTF8StringEncoding error:nil];
            return [_ctx evaluateScript:content withSourceURL:[[NSURL alloc] initWithString:file]];
        }
    }
    return [JSValue valueWithUndefinedInContext:_ctx];
}

- (bool) nonInternalExists: (NSString*)path {
    const NSURL* fileUrl =  [[NSURL alloc] initWithString:path];
    const NSString* request = [fileUrl lastPathComponent];
    if ([request isEqualToString:@"./IModelJsFs"]
         || [request isEqualToString:@"IModelJsFs"]
         || [request isEqualToString:@"../IModelJsFs"]
         || [request isEqualToString:@"../../IModelJsFs"]
         || [request isEqualToString:@"fs"]
        || [request isEqualToString:@"multiparty"]
        || [request isEqualToString:@"form-data"]
        || [request isEqualToString:@"form-data"]
        || [request isEqualToString:@"https"]
        || [request isEqualToString:@"http"]
        || [request isEqualToString:@"zlib"]
        || [request isEqualToString:@"net"]
        || [request isEqualToString:@"crypto"]
        //|| [request isEqualToString:@"os"]
        ) {
        return true;
    }
    return false;
}
- (void)extend:(id)jsContext {
    if (![jsContext isKindOfClass:[JSContext class]]) {
        NSException *e = [NSException
                          exceptionWithName:@"InvalidTypeException"
                          reason:@"Expecting JSContext"
                          userInfo:nil];
        @throw e;
    }
    _ctx = ((JSContext*)jsContext);
    _ctx[@"native_module"] = _ctx[@"process"][@"ext"][@"NativeModule"] = self;
}
@end
