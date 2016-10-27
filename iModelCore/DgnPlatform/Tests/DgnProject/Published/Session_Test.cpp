/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/Session_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"

//=======================================================================================
// @bsiclass                                    Shaun.Sewall                    10/2016
//=======================================================================================
struct SessionTests : public DgnDbTestFixture
{
    void Test();
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SessionTests::Test()
    {
    // insert some sample Sessions
    for (int32_t i=0; i<3; ++i)
        {
        Utf8PrintfString sessionName("Session%d", i);
        SessionPtr session = Session::Create(*m_db, sessionName.c_str());
        ASSERT_TRUE(session.IsValid());
        ASSERT_TRUE(session->Insert().IsValid());
        }

    auto& sessions= m_db->Sessions();
    EXPECT_TRUE(sessions.GetCurrent().GetName() == "");
    sessions.SetCurrent("NotFound");
    EXPECT_TRUE(sessions.GetCurrent().GetName() == "");

    {
    EXPECT_TRUE(sessions.GetByName("Session1").IsValid());
    sessions.SetCurrent("Session2");
    auto& current = sessions.GetCurrent();
    EXPECT_TRUE(current.GetName() == "Session2");

    Json::Value val;
    val["val100"]  = 100;
    val["val200"]  = 200;
    current.SetVariable("myval", val);

    Json::Value val2;
    val2["val30"]  = 30;
    val2["val40"]  = 40;
    current.SetVariable("myval2", val2);
    EXPECT_TRUE(current.Update().IsValid());
    m_db->SaveChanges();
    }

    sessions.SetCurrent("Session1");
    EXPECT_TRUE(sessions.GetCurrent().GetName() == "Session1");
    sessions.SetCurrent("Session2");

    {
    auto& myVal = sessions.GetCurrent().GetVariable("myval");
    EXPECT_TRUE(!myVal.isNull());
    EXPECT_TRUE(myVal["val100"].asInt() == 100);
    EXPECT_TRUE(myVal["val200"].asInt() == 200);
    }

    sessions.GetCurrent().RemoveVariable("myval");
    {
    auto& myVal = sessions.GetCurrent().GetVariable("myval");
    EXPECT_TRUE(myVal.isNull());
    auto& myVal2 = sessions.GetCurrent().GetVariable("myval2");
    EXPECT_TRUE(!myVal2.isNull());
    EXPECT_TRUE(myVal2["val30"].asInt() == 30);
    EXPECT_TRUE(myVal2["val40"].asInt() == 40);
    }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    08/2016
//---------------------------------------------------------------------------------------
TEST_F(SessionTests, Sessions)
    {
    SetupSeedProject();
    Test();
    }
