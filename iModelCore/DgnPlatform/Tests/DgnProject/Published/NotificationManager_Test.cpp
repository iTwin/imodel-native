/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/NotificationManager_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     KyleDeeds  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
class NotificationManagerTest : public GenericDgnModelTestFixture
{
public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
NotificationManagerTest()
    : GenericDgnModelTestFixture (__FILE__, true, false)
    {}

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
    NotifyMessageDetails x(priority, brief, NULL, 0, openAlert);
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
//    OutputMessageAlert openAlert = OutputMessageAlert::None;
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
TEST_F(NotificationManagerTest, GetMsgAttributes_default)
    {
    OutputMessagePriority priority = OutputMessagePriority::Fatal;
//    OutputMessageAlert openAlert = OutputMessageAlert::None;
    Utf8CP brief = "This is fatal";
    NotifyMessageDetails x(priority, brief);
    ASSERT_EQ(0, x.GetMsgAttributes());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NotificationManagerTest, GetMsgAttributes_setAttr)
    {
    OutputMessagePriority priority = OutputMessagePriority::Fatal;
    OutputMessageAlert openAlert = OutputMessageAlert::None;
    Utf8CP brief = "This is fatal";
    long attr = 1;
    NotifyMessageDetails x(priority, brief, NULL, attr, openAlert);
    ASSERT_EQ(attr, x.GetMsgAttributes());
    x.SetMsgAttributes(12);
    ASSERT_EQ(12, x.GetMsgAttributes());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NotificationManagerTest, GetDetailedMsg_SetDetailedMsg)
    {
    OutputMessagePriority priority = OutputMessagePriority::Fatal;
    OutputMessageAlert openAlert = OutputMessageAlert::None;
    Utf8CP brief = "This is fatal";
    Utf8CP detailMsg = "This is a long message";
    long attr = 1;
    NotifyMessageDetails x(priority, brief, detailMsg, attr, openAlert);
    if(detailMsg != x.GetDetailedMsg())
        FAIL();
    x.SetDetailedMsg("This is a too long message");
    ASSERT_STREQ("This is a too long message", x.GetDetailedMsg().c_str());
    }

