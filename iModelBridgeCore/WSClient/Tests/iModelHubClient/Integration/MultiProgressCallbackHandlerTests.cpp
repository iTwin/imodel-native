/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/MultiProgressCallbackHandlerTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "BaseMockHttpHandlerTest.h"
#include "../../../iModelHubClient/MultiProgressCallbackHandler.h"

USING_NAMESPACE_BENTLEY_IMODELHUB

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             09/2017
//---------------------------------------------------------------------------------------
struct MultiProgressCallbackHandlerTests : public Tests::BaseMockHttpHandlerTest
{
    virtual void SetUp() override
    {
    }
};

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             09/2017
//---------------------------------------------------------------------------------------
TEST_F(MultiProgressCallbackHandlerTests, LockEventTests)
    {
    static double s_transfered = 0.0f;
    static double s_total = 0.0f;

    Http::Request::ProgressCallback mainCallback = [=](double bytesTransfered, double bytesTotal)
        {
        s_transfered = bytesTransfered;
        s_total = bytesTotal;
        };

    MultiProgressCallbackHandler handler(mainCallback);
    Http::Request::ProgressCallback twentyPercentageCallback, eightyPercentageCallback;
    handler.AddCallback(twentyPercentageCallback, 20.0f);
    handler.AddCallback(eightyPercentageCallback, 80.0f);

    eightyPercentageCallback(50.0f, 100.0f);
    EXPECT_EQ(40.0f, s_transfered);
    EXPECT_EQ(100.0f, s_total);

    twentyPercentageCallback(1.0f, 4.0f);
    EXPECT_EQ(45.0f, s_transfered);
    EXPECT_EQ(100.0f, s_total);

    twentyPercentageCallback(4.0f, 4.0f);
    EXPECT_EQ(60.0f, s_transfered);
    EXPECT_EQ(100.0f, s_total);

    eightyPercentageCallback(1.0f, 1.0f);
    EXPECT_EQ(100.0f, s_transfered);
    EXPECT_EQ(100.0f, s_total);
    }
