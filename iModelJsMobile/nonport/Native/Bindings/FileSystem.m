//
//  FileSystem.m
//  TestApp
//
//  Created by Affan Khan on 11/21/18.
//  Copyright © 2018 Affan Khan. All rights reserved.
//

#import "FileSystem.h"


@implementation FileSystem {
    JSContext* _jsContext;
    NSFileManager* _fileManager;
    NSMutableDictionary<NSNumber*, NSFileHandle*>* _fileHandles;
}

- (instancetype)init {
    self = [super init];
    _fileManager = [NSFileManager new];
    _fileHandles = [NSMutableDictionary<NSNumber*, NSFileHandle*> new];
    
    [_fileHandles setObject:[NSFileHandle fileHandleWithStandardInput] forKey:[NSNumber numberWithInt:0]];
    [_fileHandles setObject:[NSFileHandle fileHandleWithStandardOutput] forKey:[NSNumber numberWithInt:1]];
    [_fileHandles setObject:[NSFileHandle fileHandleWithStandardError] forKey:[NSNumber numberWithInt:2]];
    [_fileHandles setObject:[NSFileHandle fileHandleWithNullDevice] forKey:[NSNumber numberWithInt:3]];
    return self;
}
- (void) closeSync: (JSValue*)fd {
    uint32_t fno = fd.toUInt32;
    if ( fno >=0 && fno <= 3)
        return;
    
    NSFileHandle* fh = [_fileHandles objectForKey:[NSNumber numberWithUnsignedInt:fno]];
    if (fh != nil ) {
        [fh closeFile];
        [_fileHandles removeObjectForKey:[NSNumber numberWithUnsignedInt:fno]];
    }
}
- (JSValue*) openSync: (NSString*)path :(NSString*)flags :(JSValue*)mode {
    if ([path caseInsensitiveCompare:@"stdin"] == NSOrderedSame)
        return [JSValue valueWithInt32:0 inContext:_jsContext];
    if ([path caseInsensitiveCompare:@"stdout"] == NSOrderedSame)
        return [JSValue valueWithInt32:1 inContext:_jsContext];
    if ([path caseInsensitiveCompare:@"stderr"] == NSOrderedSame)
        return [JSValue valueWithInt32:2 inContext:_jsContext];
    if ([path caseInsensitiveCompare:@"null"] == NSOrderedSame)
        return [JSValue valueWithInt32:3 inContext:_jsContext];
    
    NSNumber* fd = [NSNumber numberWithUnsignedInt:rand()];
    while ([_fileHandles objectForKey:fd] != nil) {
        fd = [NSNumber numberWithUnsignedInt:rand()];
    }
    
    bool append = [flags hasPrefix:@"a"];
    bool read = [flags hasPrefix:@"r"];;
    bool write = [flags hasPrefix:@"w"];;
    
    NSFileHandle * fileHandle;
    if (read) {
        fileHandle = [NSFileHandle fileHandleForReadingAtPath:path];
    } else if (write) {
        fileHandle = [NSFileHandle fileHandleForWritingAtPath:path];
    } else if (append) {
        fileHandle = [NSFileHandle fileHandleForUpdatingAtPath:path];
    }
    if (_fileHandles == nil) {
        [_jsContext setException:[JSValue valueWithNewErrorFromMessage:@"Unable to open file" inContext:_jsContext]];
        return [JSValue valueWithInt32:-1 inContext:_jsContext];
    }
    [_fileHandles setObject:fileHandle forKey:fd];
    return [JSValue valueWithInt32:fd.intValue inContext:_jsContext];
}

- (bool) existsSync: (NSString*)path {
    bool exists = [_fileManager fileExistsAtPath:path];
    NSLog(@"[FS] existsSync (%@) -> %@", path, exists?@"YES" : @"NO");
    return exists;
}
// only delete files
- (void) unlinkSync: (NSString*)path {
    NSError *err;
    [_fileManager removeItemAtPath:path error:&err];
    if (err) {
        [_jsContext setException:[JSValue valueWithNewErrorFromMessage:err.description inContext:_jsContext]];
    }
}
// delte file and folders
- (void) removeSync: (NSString*)path {
    NSError *err;
    [_fileManager removeItemAtPath:path error:&err];
    if (err) {
       [_jsContext setException:[JSValue valueWithNewErrorFromMessage:err.description inContext:_jsContext]];
    }
}
// delte file and folders
- (void) mkdirSync: (NSString*)path {
    NSError *err;
    [_fileManager createDirectoryAtPath:path withIntermediateDirectories:true attributes:nil error:&err];
    if (err) {
        [_jsContext setException:[JSValue valueWithNewErrorFromMessage:err.description inContext:_jsContext]];
    }
    NSLog(@"[FS] mkdirSync (%@) -> %@", path, !err?@"YES" : @"NO");
}

// delte file and folders
- (void) rmdirSync: (NSString*)path {
    NSError *err;
    [_fileManager createDirectoryAtPath:path withIntermediateDirectories:true attributes:nil error:&err];
    if (err) {
        [_jsContext setException:[JSValue valueWithNewErrorFromMessage:err.description inContext:_jsContext]];
    }
    NSLog(@"[FS] rmdirSync (%@) -> %@", path, !err?@"YES" : @"NO");
}
// olders
- (NSArray<NSString*>*) readdirSync: (NSString*)path {
    NSError *err;
    NSArray<NSString*>* content = [_fileManager contentsOfDirectoryAtPath:path error:&err];
    if (err) {
        [_jsContext setException:[JSValue valueWithNewErrorFromMessage:err.description inContext:_jsContext]];
        return nil;
    }
    NSLog(@"[FS] readdirSync (%@) -> %@, %@", path, !err?@"YES" : @"NO", [content componentsJoinedByString:@",\n"]);
    return content;
}
- (void) writeFileSync: (NSString*)path :(NSString*)content {
    NSError *err;
    [_fileManager createFileAtPath:path contents:nil attributes:nil];
    [content writeToFile:path atomically:true encoding:NSUTF8StringEncoding error:&err];
    if (err) {
        [_jsContext setException:[JSValue valueWithNewErrorFromMessage:err.description inContext:_jsContext]];
    }
}
- (void) copySync: (NSString*)fromPath :(NSString*)toPath {
    NSError *err;
    [_fileManager copyItemAtPath:fromPath toPath:toPath error:&err];
    if (err) {
        [_jsContext setException:[JSValue valueWithNewErrorFromMessage:err.description inContext:_jsContext]];
    }
}
- (NSString*) realpathSync: (JSValue*)path :(JSValue*)options {
    NSError *err;
    NSString* str = [_fileManager destinationOfSymbolicLinkAtPath:path.toString error:&err];
    if (err || str == nil)
        return path.toString;
    
    return str;
}
- (JSValue*) lstatSync: (JSValue*)path {
    NSString* strPath = path.toString;
    BOOL isDir = false;
    if (![_fileManager fileExistsAtPath:strPath isDirectory:&isDir]) {
        return [JSValue valueWithUndefinedInContext:path.context];
    }
    JSValue* ret = [JSValue valueWithNewObjectInContext:path.context ];
    ret[@"IsDirectory"] = [JSValue valueWithBool:isDir inContext:path.context];
    ret[@"IsFile"] = [JSValue valueWithBool:!isDir inContext:path.context];
    bool isSymbolicLink = [_fileManager destinationOfSymbolicLinkAtPath:strPath error:nil] != nil;
    ret[@"IsSymbolicLink"] = [JSValue valueWithBool:isSymbolicLink inContext:path.context];
    ret[@"IsSocket"] = [JSValue valueWithBool:false inContext:path.context];
    NSDictionary<NSFileAttributeKey,id>* attributes = [_fileManager attributesOfItemAtPath:strPath error:nil];
    if (attributes != nil) {
        double birthTimeMs = (double)[[attributes fileCreationDate] timeIntervalSinceReferenceDate]*1000;
        double mtimeMs = (double)[[attributes fileModificationDate] timeIntervalSinceReferenceDate]*1000;
        ret[@"birthTimeMs"] = [JSValue valueWithDouble:birthTimeMs inContext:path.context];
        ret[@"mtimeMs"] = [JSValue valueWithDouble:mtimeMs inContext:path.context];
        ret[@"atimeMs"] = [JSValue valueWithDouble:mtimeMs inContext:path.context];
    }
    return ret;
}
- (JSValue*) istatSync: (JSValue*)path {
    int flags = -1;
    NSString* strPath = path.toString;
    BOOL isDir = false;
    if ([_fileManager fileExistsAtPath:strPath isDirectory:&isDir]) {
        flags = isDir ? 1: 0;
    }
    return [JSValue valueWithInt32:flags inContext:path.context];;
}
- (JSValue*) readFileSync: (JSValue*)path :(JSValue*)options {
    if (!options.isString) {
        @throw @"Error";
    }
    NSURL* file = [NSURL fileURLWithPath:path.toString];
    NSString* content = [[NSString alloc] initWithContentsOfURL:file encoding:NSUTF8StringEncoding error:nil];
    if (content) {
        return [JSValue valueWithObject:content inContext:path.context];
    }
    return [JSValue valueWithUndefinedInContext:path.context];
}

- (void)extend:(id)jsContext {
    if (![jsContext isKindOfClass:[JSContext class]]) {
        NSException *e = [NSException
                          exceptionWithName:@"InvalidTypeException"
                          reason:@"Expecting JSContext"
                          userInfo:nil];
        @throw e;
    }
    JSContext* ctx = _jsContext = ((JSContext*)jsContext);
    ctx[@"fs"] = ctx[@"process"][@"ext"][@"FileSystem"] = self;
    
}
@end
