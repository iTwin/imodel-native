/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#if defined (BENTLEY_WIN32)
    // WIP_BEFILENAME_PORTABILITY - need better way of setting temp directory
    #include <windows.h>
#endif

#include <Bentley/BeTest.h>
#include <Bentley/BeFileName.h>
#include <Logging/bentleylogging.h>
#include <Bentley/Desktop/FileSystem.h>
#include "iModelDmsSupportTests.h"

/*---------------------------------------------------------------------------------**//**
* This is the TestListener that can act on certain events
* @bsiclass                                    Majd.Uddin                      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
class BeGTestListener : public ::testing::EmptyTestEventListener
    {
    virtual void OnTestProgramEnd(const ::testing::UnitTest& unit_test)
        {
        if (unit_test.failed_test_count() == 0)
            fprintf(stdout, "\n\nAll test(s) passed (count=%d)\n", unit_test.successful_test_count());
        else
            fprintf(stderr, "\n\n *** %d test(s) failed ***\n", unit_test.failed_test_count());
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vishal.Shingare                   02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C" int main(int argc, char **argv)
    {
    ::testing::InitGoogleTest(&argc, argv);

    iModelDmsSupportTests::SetArgcArgv(argc, argv);
    //listener with test failure output
    ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new BeGTestListener);
    BeTest::SetRunningUnderGtest();
    return RUN_ALL_TESTS();
    }
