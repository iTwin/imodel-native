//
//  VM.h
//  TestApp
//
//  Created by Affan Khan on 11/21/18.
//  Copyright © 2018 Affan Khan. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <JavaScriptCore/JavaScriptCore.h>
NS_ASSUME_NONNULL_BEGIN

@protocol VM<JSExport>
- (JSValue*) runInThisContext:(NSString *)code :(JSValue*)fileName;
- (JSValue*) runInNewContext:(NSString *)code :(JSValue*)sandbox :(JSValue*)fileName;
@end


@interface VM : NSObject <VM>
- (void)extend:(id)jsContext;
@end


NS_ASSUME_NONNULL_END
