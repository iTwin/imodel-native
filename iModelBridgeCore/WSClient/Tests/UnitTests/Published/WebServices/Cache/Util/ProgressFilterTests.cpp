/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Cache/Util/ProgressFilterTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ProgressFilterTests.h"
#include <WebServices/Cache/Util/ProgressFilter.h>

using namespace ::testing;

/*--------------------------------------------------------------------------------------+
* @bsitest                                   Julius.Cepukenas                    02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ProgressFilterTests, Execute_CreatedWithProgressOfNullptr_DoesNothing)
    {
    std::function<void(double)> onProgress = nullptr;
    std::function<void(double)> filteredProgress = ProgressFilter::Create(onProgress);
    filteredProgress(55);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                   Julius.Cepukenas                    02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ProgressFilterTests, Execute_CreatedWithFunctionOfOneArgument_ProgressExcecutes)
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

/*--------------------------------------------------------------------------------------+
* @bsitest                                   Julius.Cepukenas                    02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ProgressFilterTests, Execute_CreatedWithFunctionOfTwoArguments_ProgressExcecutes)
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

/*--------------------------------------------------------------------------------------+
* @bsitest                                   Julius.Cepukenas                    02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ProgressFilterTests, Execute_MultipleTimesAndMinDelayIsZero_ProgressIsCalledEveryTime)
    {
    int count = 0;
    std::function<void(double)> onProgress = [&] (double a)
        {
        count++;
        EXPECT_EQ(count, a);
        };
    std::function<void(double)> filteredProgress = ProgressFilter::Create(onProgress, 0);

    for (int i = 1; i <= 10; i++)
        {
        filteredProgress(i);
        }

    EXPECT_EQ(10, count);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                   Julius.Cepukenas                    02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ProgressFilterTests, Execute_TwiceAndExecuteWithInsufficientDelay_ProgressIsCalledOnce)
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

/*--------------------------------------------------------------------------------------+
* @bsitest                                   Julius.Cepukenas                    02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ProgressFilterTests, Execute_TwiceAndExecuteWithSufficiantDelay_ProgressIsCalledTwice)
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

/*--------------------------------------------------------------------------------------+
* @bsitest                                   Julius.Cepukenas                    02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ProgressFilterTests, Execute_MultipleTimesWhenShouldSkipFilterReturnsTrue_ProgressIsCalledMultipleTimes)
    {
    int count = 0;
    std::function<void(double)> onProgress = [&] (double a)
        {
        count++;
        };
    std::function<bool(double)> shouldSkipFilter = [] (double a)
        {
        return true;
        };
    std::function<void(double)> filteredProgress = ProgressFilter::Create(onProgress, shouldSkipFilter, 100000);
    filteredProgress(1);
    filteredProgress(2);
    filteredProgress(3);
    filteredProgress(4);
    EXPECT_EQ(4, count);
    }
    
/*--------------------------------------------------------------------------------------+
* @bsitest                                   Julius.Cepukenas                    02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ProgressFilterTests, Execute_MultipleTimesWhenShouldSkipFilterReturnsFalse_ProgressIsCalledOnce)
    {
    int count = 0;
    std::function<void(double)> onProgress = [&] (double a)
        {
        count++;
        };
    std::function<bool(double)> shouldSkipFilter = [] (double a)
        {
        return false;
        };
    std::function<void(double)> filteredProgress = ProgressFilter::Create(onProgress, shouldSkipFilter, 100000);
    filteredProgress(1);
    filteredProgress(2);
    filteredProgress(3);
    filteredProgress(4);
    EXPECT_EQ(1, count);
    }
    
/*--------------------------------------------------------------------------------------+
* @bsitest                                   Julius.Cepukenas                    02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ProgressFilterTests, Execute_MultipleTimesAndShouldSkipFilterTrueOnce_ProgressIsCalledTwice)
    {
    int count = 0;
    std::function<void(double)> onProgress = [&] (double a)
        {
        count++;
        };
    std::function<bool(double)> shouldSkipFilter = [&] (double a)
        {
        return count == 1;
        };
    std::function<void(double)> filteredProgress = ProgressFilter::Create(onProgress, shouldSkipFilter, 100000);
    filteredProgress(1);
    filteredProgress(2);
    filteredProgress(3);
    filteredProgress(4);
    EXPECT_EQ(2, count);
    }

