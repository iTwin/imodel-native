//
//  main.cpp
//  NAPI-JSC-Tests
//
//  Created by Satyakam Khadilkar on 2/16/18.
//  Copyright © 2018 Bentley Systems. All rights reserved.
//

#include <iostream>
#include <iModelJs/iModelJs.h>

int main(int argc, const char * argv[]) {
    // insert code here...
//    std::cout << "Hello, World!\n";
    
    BentleyB0200::iModelJs::Js::Runtime runtime;
    Napi::Env& env = runtime.Env();

    return 0;
}
