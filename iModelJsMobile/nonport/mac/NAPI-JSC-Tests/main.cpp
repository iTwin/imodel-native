//
//  main.cpp
//  NAPI-JSC-Tests
//
//  Created by Satyakam Khadilkar on 2/16/18.
//  Copyright © 2018 Bentley Systems. All rights reserved.
//

#import <Foundation/Foundation.h>
#include "Test.h"

int main(int argc, const char * argv[]) {
    NSString *appFolderPath = [[NSBundle mainBundle] resourcePath];
    Test(appFolderPath.UTF8String);
    return 0;
}
