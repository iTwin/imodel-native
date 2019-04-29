/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <gtest\gtest.h>

int main(int argc, char **argv) 
    {  
    ::testing::InitGoogleTest(&argc, argv);  

    // We do this to cause asserts to always output to a message box.  Without this the gtest process will abort without giving the developer the opportunity to 
    // attach with a debugger and analyze the situation.
    _set_error_mode (_OUT_TO_MSGBOX);

    return RUN_ALL_TESTS();
    }