/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/NotificationManager_Test.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     KyleDeeds  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
struct NotificationManagerTest : public testing::Test
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ~NotificationManagerTest ()
    {
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NotificationManagerTest, GetPriority_SetPriority)
    {
    OutputMessagePriority priority = OutputMessagePriority::Fatal;
    Utf8CP brief = "This is fatal";
    NotifyMessageDetails x(priority, brief);
    ASSERT_TRUE(priority == x.GetPriority());
    x.SetPriority(OutputMessagePriority::Warning);
    ASSERT_EQ(OutputMessagePriority::Warning, x.GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NotificationManagerTest, GetBriefMsg_SetBriefMsg)
    {
    OutputMessagePriority priority = OutputMessagePriority::Fatal;
    Utf8CP brief = "This is fatal";
    NotifyMessageDetails x(priority, brief);
    if( brief != x.GetBriefMsg())
        FAIL();
    x.SetBriefMsg("This is a fatal");
    ASSERT_STREQ("This is a fatal", x.GetBriefMsg().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NotificationManagerTest, GetDetailedMsg_SetDetailedMsg)
    {
    OutputMessagePriority priority = OutputMessagePriority::Fatal;
    Utf8CP brief = "This is fatal";
    Utf8CP detailMsg = "This is a long message";
    NotifyMessageDetails x(priority, brief, detailMsg);
    if(detailMsg != x.GetDetailedMsg())
        FAIL();
    Utf8CP tooLongMsg = "This is a too long message";
    x.SetDetailedMsg(tooLongMsg);
    ASSERT_STREQ(tooLongMsg, x.GetDetailedMsg().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dan.East        10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NotificationManagerTest, ActivityMessageDetails_get)
    {
    ActivityMessageDetails x(true, false, true);
    ASSERT_TRUE(x.GetShowProgressBar());
    ASSERT_FALSE(x.GetShowPercentInMessage());
    ASSERT_TRUE(x.GetSupportsCancellation());
    }



