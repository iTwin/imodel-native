//
//  AssetHandler.h
//  iModelJsApp
//
//  Created by Affan Khan on 12/20/18.
//  Copyright © 2018 Bentley Systems. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <WebKit/WebKit.h>
NS_ASSUME_NONNULL_BEGIN

@interface AssetHandler : NSObject<WKURLSchemeHandler>
- (void) cancelWithFileNotFound: (id<WKURLSchemeTask>)urlSchemeTask;
-(instancetype) initWithAssetPath: (NSString*)path;
-(instancetype) initWithAssetPathAndDefaultPage: (NSString*)path :(NSString*)mainPage;
@end

NS_ASSUME_NONNULL_END
