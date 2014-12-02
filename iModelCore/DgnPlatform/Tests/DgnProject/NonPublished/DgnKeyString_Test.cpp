/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnKeyString_Test.cpp $
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
* @bsiclass                                            Algirdas.Mikoliunas      03/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnKeyStringsTest : public ::testing::Test
{
    ScopedDgnHost           m_host;
    DgnProjectPtr      project;
    
    virtual void SetUp () override
        {
        DgnDbTestDgnManager tdm (L"ElementsSymbologyByLevel.idgndb", __FILE__, OPENMODE_READWRITE);
        project = tdm.GetDgnProjectP();
        ASSERT_TRUE( project != NULL);
        }

};

/*---------------------------------------------------------------------------------**//**
* Work with KeyStrings
* @bsimethod                                    Majd.Uddin                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnKeyStringsTest, WorkWithKeyStrings)
    {
    
    DgnKeyStrings ksTable = project->KeyStrings();
    DgnKeyStringId ksId = ksTable.Insert ("TestAuthor", "Majd");
    EXPECT_TRUE (ksId == ksTable.QueryId ("TestAuthor"));
    }

/*---------------------------------------------------------------------------------**//**
* KeyStrings query
* @bsimethod                                 Algirdas.Mikoliunas                   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnKeyStringsTest, QueryKeyString)
    {
    
    DgnKeyStrings ksTable = project->KeyStrings();

    DgnKeyStringId keyStringId(1);
    Utf8String keyStringName, keyStringValue;
    EXPECT_EQ(BE_SQLITE_ROW, ksTable.Query(keyStringId, &keyStringName, &keyStringValue));

    EXPECT_STREQ("dgn_ModSel", keyStringName.c_str());
    EXPECT_STREQ("", keyStringValue.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* KeyStrings update
* @bsimethod                                 Algirdas.Mikoliunas                   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnKeyStringsTest, UpdateKeyString)
    {
    
    DgnKeyStrings ksTable = project->KeyStrings();

    EXPECT_EQ(BE_SQLITE_DONE, ksTable.Update ("dgn_ModSel", "TestValue"));
    
    DgnKeyStringId keyStringId(1);
    Utf8String keyStringName, keyStringValue;
    EXPECT_EQ(BE_SQLITE_ROW, ksTable.Query(keyStringId, &keyStringName, &keyStringValue));

    EXPECT_STREQ("dgn_ModSel", keyStringName.c_str());
    EXPECT_STREQ("TestValue", keyStringValue.c_str());
    }
