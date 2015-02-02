/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Client/Connect/BEUniqueIdManager.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#import <Foundation/Foundation.h>

@interface BEUniqueIdManager : NSObject

+ (NSString*) uniqueId;
- (id) initForDevice;
- (NSString*) persistentUniqueId;

//    sample usage
//    BEUniqueIdManager* devmgr = [[BEUniqueIdManager alloc] initForDevice];
//    BEUniqueIdManager* appmgr = [[BEUniqueIdManager alloc] initWithAppName:@"navigator" accessGroupSuffix:accessGroupSuffix];
//    NSString* uuid = [BEUniqueIdManager uniqueId]; // newly generated changes on each execution
//    NSString* deviceuuid = [devmgr persistentUniqueId]; // always unique for a device for Bentley Apps

@end
