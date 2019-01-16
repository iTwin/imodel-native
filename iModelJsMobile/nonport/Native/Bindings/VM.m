//
//  VM.m
//  TestApp
//
//  Created by Affan Khan on 11/21/18.
//  Copyright © 2018 Affan Khan. All rights reserved.
//

#import "VM.h"


@implementation VM {
    JSContext* _context;
}

- (void)extend:(id)jsContext {
    if (![jsContext isKindOfClass:[JSContext class]]) {
        NSException *e = [NSException
                          exceptionWithName:@"InvalidTypeException"
                          reason:@"Expecting JSContext"
                          userInfo:nil];
        @throw e;
    }
    _context = ((JSContext*)jsContext);
   _context[@"vm"] = _context[@"process"][@"ext"][@"VM"] = self;
}
- (JSValue*) runInThisContext:(NSString *)code :(JSValue*)fileName {
    JSValue* fn = [fileName objectForKeyedSubscript:@"filename"];
    NSURL* url = [fileName isUndefined] ? nil : [[NSURL alloc]  initWithString:[fn toString]];
    return [_context evaluateScript:code withSourceURL:url ];
}
- (JSValue*) runInNewContext:(NSString *)code :(JSValue*)sandbox :(JSValue*)fileName {
    JSValue* fn = [fileName objectForKeyedSubscript:@"filename"];
    NSURL* url = [fileName isUndefined] ? nil : [[NSURL alloc]  initWithString:[fn toString]];
    JSContext* newContext = [[JSContext alloc] initWithVirtualMachine:_context.virtualMachine];
    if (!sandbox.isUndefined) {
        for(NSString* key in sandbox.toArray) {
            newContext[key] = sandbox[key];
        }
    }
    return [newContext evaluateScript:code withSourceURL:url];
}
@end
