/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Cache/Util/ProgressFilterTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ProgressFilterTests.h"
#include <WebServices/Cache/Util/ProgressFilter.h>

using namespace ::testing;

TEST_F(ProgressFilterTests, ExecuteFilteredProgress_CreatedWithProgressOfNullptr_DoesNothing)
    {
    std::function<void(double)> onProgress = nullptr;
    std::function<void(double)> filteredProgress = ProgressFilter::Create(onProgress);
    filteredProgress(55);
    }

TEST_F(ProgressFilterTests, ExecuteFilteredProgress_CreatedWithFunctionOfOneArgument_ProgressExcecutes)
    {
    int count = 0;
    std::function<void(double)> onProgress = [&] (double a) 
        {
        count++;
        EXPECT_EQ(55, a);
        };
    std::function<void(double)> filteredProgress = ProgressFilter::Create(onProgress);
    filteredProgress(55);
    EXPECT_EQ(1, count);
    }

TEST_F(ProgressFilterTests, ExecuteFilteredProgress_CreatedWithFunctionOfTwoArguments_ProgressExcecutes)
    {
    int count = 0;
    std::function<void(int, Utf8String)> onProgress = [&] (int a, Utf8String b)
        {
        count++;
        EXPECT_EQ(55, a);
        EXPECT_EQ("A", b);
        };
    std::function<void(int, Utf8String)> filteredProgress = ProgressFilter::Create(onProgress);
    filteredProgress(55, "A");
    EXPECT_EQ(1, count);
    }

TEST_F(ProgressFilterTests, ExecuteFilteredProgressTwice_ExecuteWithInsufficientDelay_ProgressIsCalledOnce)
    {
    int count = 0;
    std::function<void(double)> onProgress = [&] (double a)
        {
        count++;
        EXPECT_EQ(55, a);
        };
    std::function<void(double)> filteredProgress = ProgressFilter::Create(onProgress, 100000);
    filteredProgress(55);
    BeThreadUtilities::BeSleep(0);
    filteredProgress(75);
    EXPECT_EQ(1, count);
    }

TEST_F(ProgressFilterTests, ExecuteFilteredProgressTwice_ExecuteWithSufficiantDelay_ProgressIsCalledTwice)
    {
    int count = 0;
    std::function<void(double)> onProgress = [&] (double a)
        {
        count++;
        };
    std::function<void(double)> filteredProgress = ProgressFilter::Create(onProgress, 1);
    filteredProgress(55);
    BeThreadUtilities::BeSleep(2);
    filteredProgress(75);
    EXPECT_EQ(2, count);
    }

