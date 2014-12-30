/*--------------------------------------------------------------------------------------+
|
|     $Source: BentleyLibTest.cpp $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma warning(disable:4505) // unreferenced local function has been removed [in gtest-port.h]

#include <Bentley/Bentley.h>
#include <gtest/gtest.h>

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      03/10
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C" EXPORT_ATTRIBUTE int Run(int argc, char **argv, void*, char const*)
    {
    ::testing::InitGoogleTest (&argc, argv);

    printf ("__START_TESTS__\n");    
    int status = RUN_ALL_TESTS();
    printf ("__END_TESTS__\n");

    return status;
    }

