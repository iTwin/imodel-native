/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/nativeatp/ECObjectsTestPCH.cpp $
|
|  $Copyright: (c) 2009 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"

#if !defined (COMPILING_PUBLISHED_TESTS) && !defined(COMPILING_NONPUBLISHED_TESTS)
    #error One of COMPILING_PUBLISHED_TESTS or COMPILING_NONPUBLISHED_TESTS must be defined when building ECObjects native tests.
#endif

#if defined(COMPILING_PUBLISHED_TESTS) && defined (NON_PUBLISHED_HEADER_INCLUDED)
   #error The ECObjects Published API Tests are compiling against the non published header files.  They are intended to compile against the published headers.  It is likely the include path for dll mke has been changed in error.
#endif

#if defined(COMPILING_NONPUBLISHED_TESTS) && !defined (NON_PUBLISHED_HEADER_INCLUDED) 
   #error The ECObjects NonPublished API Tests are compiling against the published header files.  They are intended to compile against the nonpublished headers.  It is likely the include path for dll mke has been changed in error.
#endif

int main(int argc, char **argv) 
    {  
    ::testing::InitGoogleTest(&argc, argv);  

    // We do this to cause asserts to always output to a message box.  Without this the gtest process will abort without giving the developer the opportunity to 
    // attach with a debugger and analyze the situation.
    _set_error_mode (_OUT_TO_MSGBOX);

    return RUN_ALL_TESTS();
    }