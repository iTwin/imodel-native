/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#import "../BeTestiOSTests/BeTestiOSTests.h"
#import <Bentley/BeAssert.h>
#import <Bentley/BeFileName.h>
#import <BeSQLite/L10N.h>
#import <Logging/bentleylogging.h>
#import <stdlib.h>

@interface SenTest_BaseClass : XCTestCase {}
+ (void) begtest_initialize;
+ (void) begtest_uninitialize;
@end
