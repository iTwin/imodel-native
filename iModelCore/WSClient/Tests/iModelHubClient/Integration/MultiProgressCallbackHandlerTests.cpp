/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../Helpers/BaseMockHttpHandlerTest.h"
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
    static double s_transfered = 0.0;
    static double s_total = 0.0;

    Http::Request::ProgressCallback mainCallback = [=](double bytesTransfered, double bytesTotal)
        {
        s_transfered = bytesTransfered;
        s_total = bytesTotal;
        };

    MultiProgressCallbackHandler handler(mainCallback, 20.0);
    Http::Request::ProgressCallback twentyPercentageCallback, eightyPercentageCallback;
    handler.AddCallback(twentyPercentageCallback);
    handler.AddCallback(eightyPercentageCallback);

    eightyPercentageCallback(8.0, 16.0);
    EXPECT_EQ(8.0, s_transfered);
    EXPECT_EQ(20.0, s_total);

    twentyPercentageCallback(1.0, 4.0);
    EXPECT_EQ(9.0, s_transfered);
    EXPECT_EQ(20.0, s_total);

    twentyPercentageCallback(4.0, 4.0);
    EXPECT_EQ(12.0, s_transfered);
    EXPECT_EQ(20.0, s_total);

    eightyPercentageCallback(16.0, 16.0);
    EXPECT_EQ(20.0, s_transfered);
    EXPECT_EQ(20.0, s_total);
    }
