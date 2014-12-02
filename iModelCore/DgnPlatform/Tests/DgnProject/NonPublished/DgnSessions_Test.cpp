/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnSessions_Test.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <Bentley/BeTimeUtilities.h>
#include <DgnPlatform/DgnCore/ColorUtil.h>

#if defined (_MSC_VER)
#pragma warning (disable:4702)
#endif

USING_NAMESPACE_BENTLEY_SQLITE

/*---------------------------------------------------------------------------------**//**
* Test fixture for testing DgnSessions
* @bsimethod                                    Algirdas.Mikoliunas            03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnSessionsTest : public ::testing::Test
    {
    public:
        ScopedDgnHost           m_host;
        DgnProjectPtr      project;

        void SetupProject (WCharCP projFile, FileOpenMode mode);
    };

/*---------------------------------------------------------------------------------**//**
* Set up method that opens an existing .dgndb project file
* @bsimethod                                    Algirdas.Mikoliunas            03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnSessionsTest::SetupProject (WCharCP projFile, FileOpenMode mode)
    {
    
    DgnDbTestDgnManager tdm (projFile, __FILE__, mode);
    project = tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL);
    }

/*---------------------------------------------------------------------------------**//**
* Work with Sessions
* @bsimethod                                    Majd.Uddin                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnSessionsTest, WorkWithSessions)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", OPENMODE_READWRITE);
    
    DgnSessions sessTable = project->Sessions();
    DgnSessions::Iterator sessIter = sessTable.MakeIterator();
    EXPECT_EQ (1, sessIter.QueryCount() ) << "The Session count should be 1, but it is: " << sessIter.QueryCount();
    
    int i = 0;
    FOR_EACH (DgnSessions::Iterator::Entry const & entry, sessIter)
        {
        EXPECT_TRUE (0 == strcmp ("Default", entry.GetName())); //There is only one session with Default
        i++;
        }
    EXPECT_EQ (1, i);
    }

/*---------------------------------------------------------------------------------**//**
* Delete session test
* @bsimethod                                 Algirdas.Mikoliunas                 04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnSessionsTest, DeleteSession)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", OPENMODE_READWRITE);

    DgnSessions sessTable = project->Sessions();
    DgnSessions::Iterator sessIter = sessTable.MakeIterator();
    EXPECT_EQ (1, sessIter.QueryCount()) << "The Session count should be 1, but it is: " << sessIter.QueryCount();
    
    DgnSessionId sessionId(1);
    EXPECT_EQ(BE_SQLITE_DONE, sessTable.DeleteSession(sessionId));
    
    DgnSessions::Iterator iterAfterDelete = sessTable.MakeIterator();
    EXPECT_EQ (0, iterAfterDelete.QueryCount()) << "The Session count should be 0, but it is: " << iterAfterDelete.QueryCount();
    
    DgnSessions::Row sessionRow = sessTable.QuerySessionById(sessionId);
    EXPECT_FALSE(sessionRow.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* Update session test
* @bsimethod                                 Algirdas.Mikoliunas                 04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnSessionsTest, UpdateSession)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", OPENMODE_READWRITE);

    DgnSessionId sessionId(1);
    DgnSessions sessTable = project->Sessions();

    DgnSessions::Row sessionRow = sessTable.QuerySessionById(sessionId);
    EXPECT_TRUE(sessionRow.IsValid());
    EXPECT_STREQ("Default", sessionRow.GetName());
    EXPECT_STREQ("default session", sessionRow.GetDescription());

    DgnSessions::Row updateRow = DgnSessions::Row("TestName", "TestDescription", sessionId);
    EXPECT_EQ(BE_SQLITE_DONE, sessTable.UpdateSession(updateRow));
    
    sessionRow = sessTable.QuerySessionById(sessionId);
    EXPECT_TRUE(sessionRow.IsValid());
    EXPECT_STREQ("TestName", sessionRow.GetName());
    EXPECT_STREQ("TestDescription", sessionRow.GetDescription());
    }

/*---------------------------------------------------------------------------------**//**
* Query session id test
* @bsimethod                                 Algirdas.Mikoliunas                 04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnSessionsTest, QuerySessionId)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", OPENMODE_READWRITE);

    DgnSessions sessTable = project->Sessions();
    DgnSessionId sessionId = sessTable.QuerySessionId("Default");
    EXPECT_EQ(1, sessionId.GetValue());
    
    DgnSessions::Row sessionRow = sessTable.QuerySessionById(sessionId);
    EXPECT_TRUE(sessionRow.IsValid());
    EXPECT_STREQ("Default", sessionRow.GetName());
    EXPECT_STREQ("default session", sessionRow.GetDescription());
    }
