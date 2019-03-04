//
//  AppRegistry.h
//  
//
//  Created by Affan Khan on 1/23/19.
//

#import "AppRegistry.h"

@implementation AppRegistry
+ (id)sharedInstance {
    static AppRegistry *sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[self alloc] init];
        NSString* path  = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"Assets/apps.config.json"];
        NSString* jsonString = [[NSString alloc] initWithContentsOfFile:path encoding:NSUTF8StringEncoding error:nil];
        NSData* jsonData = [jsonString dataUsingEncoding:NSUTF8StringEncoding];
        NSError *error = nil;
        NSDictionary  *object = [NSJSONSerialization
                                 JSONObjectWithData:jsonData
                                 options:0
                                 error:&error];
        
        if(!error) {
            NSMutableArray<AppInfo*>* appList = [NSMutableArray<AppInfo*> new];
            sharedInstance.autoRun = [object objectForKey:@"autoRun"];
            NSArray* apps = [object objectForKey:@"apps"];
            for (NSDictionary *app in apps) {
                AppInfo *appInfo = [[AppInfo alloc] init];
                appInfo.appId =  [app valueForKey:@"app-id"];
                appInfo.title =  [app valueForKey:@"title"];
                appInfo.frontend =  [app valueForKey:@"frontend"];
                appInfo.backend =  [app valueForKey:@"backend"];
                appInfo.env =  [app valueForKey:@"env"];
                [appList addObject:appInfo];
            }
            sharedInstance.apps = appList;
        } else {
            NSLog(@"Error in parsing JSON");
        }
    });
    
    return sharedInstance;
}
- (AppInfo*) find: (NSString*)appId {
    for(AppInfo* appInfo in self.apps) {
        if ([appInfo.appId isEqualToString:appId]) {
            return appInfo;
        }
    }
    return nil;
}
@end
@implementation AppInfo

@end
