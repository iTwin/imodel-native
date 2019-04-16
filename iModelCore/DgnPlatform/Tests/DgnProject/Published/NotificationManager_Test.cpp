/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
TEST_F(NotificationManagerTest, GetOpenAlert_default)
    {
    OutputMessagePriority priority = OutputMessagePriority::Fatal;
    OutputMessageAlert openAlert = OutputMessageAlert::None;
    Utf8CP brief = "This is fatal";
    NotifyMessageDetails x(priority, brief);
    ASSERT_TRUE(openAlert == x.GetOpenAlert());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NotificationManagerTest, GetOpenAlert_setAlert)
    {
    OutputMessagePriority priority = OutputMessagePriority::Fatal;
    OutputMessageAlert openAlert = OutputMessageAlert::Dialog;
    Utf8CP brief = "This is fatal";
    NotifyMessageDetails x(priority, brief, NULL, OutputMessageType::Alert, openAlert);
    ASSERT_TRUE(openAlert == x.GetOpenAlert());
    x.SetOpenAlert(OutputMessageAlert::Dialog);
    ASSERT_EQ(OutputMessageAlert::Dialog, x.GetOpenAlert());
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
* @bsimethod                                                    Dan.East        10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NotificationManagerTest, GetMsgType_default)
    {
    OutputMessagePriority priority = OutputMessagePriority::Fatal;
    Utf8CP brief = "This is fatal";
    NotifyMessageDetails x(priority, brief);
    ASSERT_EQ(OutputMessageType::Toast, x.GetMsgType());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dan.East        10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NotificationManagerTest, GetMsgType_setType)
    {
    OutputMessagePriority priority = OutputMessagePriority::Fatal;
    Utf8CP brief = "This is fatal";
    OutputMessageType type = OutputMessageType::Pointer;
    NotifyMessageDetails x(priority, brief, NULL, type);
    ASSERT_EQ(type, x.GetMsgType());

    type = OutputMessageType::Sticky;
    x.SetMsgType(type);
    ASSERT_EQ(type, x.GetMsgType());
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
TEST_F(NotificationManagerTest, PointerTypeDetails_defaults)
    {
    OutputMessagePriority priority = OutputMessagePriority::Info;
    Utf8CP brief = "This is a test";
    NotifyMessageDetails x(priority, brief);
    ASSERT_EQ(3500, x.GetDisplayTime());    // 3.5 seconds is the default
    ASSERT_EQ(nullptr, x.GetPointerTypeViewport());
    ASSERT_EQ(0, x.GetPointerTypeDisplayPoint().x);
    ASSERT_EQ(0, x.GetPointerTypeDisplayPoint().y);
    ASSERT_EQ(RelativePosition::TopRight, x.GetPointerTypeRelativePosition());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dan.East        10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NotificationManagerTest, SetDisplayTime_get)
    {
    OutputMessagePriority priority = OutputMessagePriority::Info;
    Utf8CP brief = "This is a test";
    NotifyMessageDetails x(priority, brief);
    x.SetDisplayTime(5000);
    ASSERT_EQ(5000, x.GetDisplayTime());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dan.East        10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NotificationManagerTest, SetPointerTypeDetails_get)
    {
    OutputMessagePriority priority = OutputMessagePriority::Info;
    Utf8CP brief = "This is a test";
    Point2d testPoint = Point2d::From(10, 11);
    NotifyMessageDetails x(priority, brief);
    x.SetPointerTypeDetails((DgnViewportCP)0x1111, testPoint, RelativePosition::BottomRight);

    ASSERT_EQ((DgnViewportCP)0x1111, x.GetPointerTypeViewport());
    ASSERT_EQ(10, x.GetPointerTypeDisplayPoint().x);
    ASSERT_EQ(11, x.GetPointerTypeDisplayPoint().y);
    ASSERT_EQ(RelativePosition::BottomRight, x.GetPointerTypeRelativePosition());
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



