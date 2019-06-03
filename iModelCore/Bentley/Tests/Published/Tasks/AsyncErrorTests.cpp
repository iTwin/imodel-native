/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include <Bentley/Tasks/Tasks.h>
#include <Bentley/Tasks/AsyncError.h>

USING_NAMESPACE_BENTLEY_TASKS
//-------------------------------------------------------------------------------------------
// @bsimethod                                          Farhad.Kabir                    11/16
//-------------------------------------------------------------------------------------------
TEST(AsyncErrorTests, Ctor_Empty)
    {
    AsyncError wdw;
    EXPECT_STREQ("", wdw.GetMessage().c_str());
    EXPECT_STREQ("", wdw.GetDescription().c_str());
    }
//-------------------------------------------------------------------------------------------
// @bsimethod                                          Farhad.Kabir                    11/16
//-------------------------------------------------------------------------------------------
TEST(AsyncErrorTests, Ctor_Message) 
    {
    Utf8String message = "Time out";
    AsyncError asyncErr(message);
    EXPECT_STREQ(message.c_str(), asyncErr.GetMessage().c_str());
    EXPECT_STREQ("", asyncErr.GetDescription().c_str());
    }
//-------------------------------------------------------------------------------------------
// @bsimethod                                          Farhad.Kabir                    11/16
//-------------------------------------------------------------------------------------------
TEST(AsyncErrorTests, Ctor_MessageAndDescription) 
    {
    Utf8String message = "Time out";
    Utf8String description = "Current process was stuck in a deadlock condition";
    AsyncError asyncErr(message,description);
    EXPECT_STREQ(message.c_str(), asyncErr.GetMessage().c_str());
    EXPECT_STREQ(description.c_str(), asyncErr.GetDescription().c_str());

    AsyncError wdw;
    wdw.GetMessage();
    EXPECT_STREQ("",wdw.GetMessage().c_str());
    }