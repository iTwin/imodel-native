/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#import <Foundation/Foundation.h>
#import <JavaScriptCore/JavaScriptCore.h>


@interface WTWindowTimers : NSObject

- (void)extend:(id)context;

// TODO proper types
@property (nonatomic) NSUInteger tolerance;
@property (readonly, nonatomic) id setTimeout;
@property (readonly, nonatomic) id clearTimeout;
@property (readonly, nonatomic) id setInterval;

@end
