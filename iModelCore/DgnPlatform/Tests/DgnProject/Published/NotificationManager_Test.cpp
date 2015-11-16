/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/NotificationManager_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
TEST_F(NotificationManagerTest, GetPriority)
    {
    OutputMessagePriority priority = OutputMessagePriority::Fatal;
    Utf8CP brief = "This is fatal";
    NotifyMessageDetails x(priority, brief);
    ASSERT_TRUE(priority == x.GetPriority());
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
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NotificationManagerTest, GetBriefMsg)
    {
    OutputMessagePriority priority = OutputMessagePriority::Fatal;
//    OutputMessageAlert openAlert = OutputMessageAlert::None;
    Utf8CP brief = "This is fatal";
    NotifyMessageDetails x(priority, brief);
    if( brief != x.GetBriefMsg())
        FAIL();
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
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NotificationManagerTest, GetDetailedMsg)
    {
    OutputMessagePriority priority = OutputMessagePriority::Fatal;
    OutputMessageAlert openAlert = OutputMessageAlert::None;
    Utf8CP brief = "This is fatal";
    Utf8CP detailMsg = "This is a long message";
    long attr = 1;
    NotifyMessageDetails x(priority, brief, detailMsg, attr, openAlert);
    if(detailMsg != x.GetDetailedMsg())
        FAIL();
    }
