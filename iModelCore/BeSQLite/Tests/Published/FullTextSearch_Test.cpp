/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/FullTextSearch_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "BeSQLitePublishedTests.h"

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct FTS5Test : public ::testing::Test
{
    Db      m_db;

    static BeFileName GetDbFilePath(WCharCP name)
        {
        BeFileName dbFileName;
        BeTest::GetHost().GetOutputRoot(dbFileName);
        dbFileName.AppendToPath(name);
        return dbFileName;
        }

    void SetupDb(WCharCP name)
        {
        BeFileName tempDir;
        BeTest::GetHost().GetOutputRoot(tempDir);
        BeSQLiteLib::Initialize(tempDir);

        BeFileName fullname = GetDbFilePath(name);
        if (BeFileName::DoesPathExist(fullname))
            BeFileName::BeDeleteFile(fullname);

        EXPECT_EQ(BE_SQLITE_OK, m_db.CreateNewDb(fullname.GetNameUtf8().c_str()));
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FTS5Test, VirtualTable)
    {
    SetupDb(L"VirtualTable.db");

    // Create an full-text-searchable table
    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("CREATE VIRTUAL TABLE email USING fts5(sender, title, body)"));

    // ###TODO: actually test...
    m_db.CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FTS5Test, ExternalContentTable)
    {
    SetupDb(L"ExternalContentTable.db");

    EXPECT_EQ(BE_SQLITE_OK, m_db.CreateTable("tbl", "a, b, c, d INTEGER PRIMARY KEY"));
    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("CREATE VIRTUAL TABLE fts USING fts5(a, c, content=tbl, content_rowid=d)"));

    m_db.CloseDb();
    }

