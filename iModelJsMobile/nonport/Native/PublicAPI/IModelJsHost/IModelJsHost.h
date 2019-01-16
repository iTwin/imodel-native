#import <Foundation/Foundation.h>
#import <JavaScriptCore/JavaScriptCore.h>


@interface IModelJsHost : NSObject

- (void)loadBackend: (NSURL*)backendUrl :(NSArray<NSString*>*)searchPaths :(NSString*)currentDir;
- (int)getPort;
- (JSContext*)getContext;
- (BOOL)isReady;
+ (id)sharedInstance;

@end
