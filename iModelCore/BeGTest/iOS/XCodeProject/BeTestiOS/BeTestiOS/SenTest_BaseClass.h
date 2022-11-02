/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#import "../BeTestiOSTests/BeTestiOSTests.h"
#import <Bentley/BeAssert.h>
#import <Bentley/BeFileName.h>
#import <Bentley/Logging.h>
#import <stdlib.h>

@interface SenTest_BaseClass : XCTestCase {}
+ (void) begtest_initialize;
+ (void) begtest_uninitialize;
@end
