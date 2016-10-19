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
};

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    08/2016
//---------------------------------------------------------------------------------------
TEST_F(SessionTests, SessionCRUD)
    {
    SetupSeedProject();
    DgnElementId sessionId[3];

    // insert some sample Sessions
    for (int32_t i=0; i<_countof(sessionId); i++)
        {
        Utf8PrintfString sessionName("Session%d", i);
        SessionPtr session = Session::Create(*m_db, sessionName.c_str());
        ASSERT_TRUE(session.IsValid());
        ASSERT_TRUE(session->Insert().IsValid());
        sessionId[i] = session->GetElementId();
        }
    }
