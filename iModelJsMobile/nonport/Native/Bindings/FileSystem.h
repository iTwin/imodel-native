//
//  FileSystem.h
//  TestApp
//
//  Created by Affan Khan on 11/21/18.
//  Copyright © 2018 Affan Khan. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <JavaScriptCore/JavaScriptCore.h>
NS_ASSUME_NONNULL_BEGIN

@protocol FileSystem <JSExport>

- (bool) existsSync: (NSString*)path;
- (void) unlinkSync: (NSString*)path;
- (void) removeSync: (NSString*)path;
- (void) mkdirSync: (NSString*)path;
- (void) rmdirSync: (NSString*)path;
- (NSArray<NSString*>*) readdirSync: (NSString*)path;
- (void) writeFileSync: (NSString*)path :(NSString*)content;
- (void) copySync: (NSString*)fromPath :(NSString*)toPath;
- (JSValue*) lstatSync: (JSValue*)path;
- (JSValue*) readFileSync: (JSValue*)path :(JSValue*)options;
- (JSValue*) istatSync: (JSValue*)path;
- (NSString*) realpathSync: (JSValue*)path :(JSValue*)options;
- (void) closeSync: (JSValue*)fd;
- (JSValue*) openSync: (NSString*)path :(NSString*)flags :(JSValue*)mode;
@end


@interface FileSystem : NSObject <FileSystem>
- (void)extend:(id)jsContext;
@end

NS_ASSUME_NONNULL_END
