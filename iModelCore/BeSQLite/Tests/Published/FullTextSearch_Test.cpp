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

    // Create a full-text-searchable table
    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("CREATE VIRTUAL TABLE email USING fts5(sender, title, body, nosearch UNINDEXED)"));

    // Populate it
    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("INSERT INTO email (sender, title, body, nosearch) VALUES ('paul.connelly@bentley.com', 'DgnDb06Dev', 'Sprint Review', 'this text is secret')"));
    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("INSERT INTO email (sender, title, body, nosearch) VALUES ('paul.connolly@bentley.com', 'Typo', 'Wrong Paul', 'not searchable')"));
    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("INSERT INTO email (sender, title, body, nosearch) VALUES ('paul.schaeffer@nbc.com', 'Retirement', 'Good luck Dave!', 'cc stephen colbert')"));
    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("INSERT INTO email (sender, title, body, nosearch) VALUES ('ingrid@retirement.com', 'Your 401k', 'Paul Michael Connelly, please call me.', 'secret unindexed text')"));

    // Search
    struct SearchOp { int32_t count; Utf8CP where; bool excludeWhere; };

    SearchOp searchOps[] =
        {
            // = operator
            { 4, "email = 'paul'" },
            { 2, "email = 'bentley'" },
            { 0, "email = 'secret'" },  // only un-indexed columns match
            { 2, "email = 'retirement'" },

            // Following 3 syntaxes are supposed to be equivalent
            { 1, "email = 'DgnDb06Dev'" },
            { 1, "email MATCH 'DgnDb06Dev'" },
            { 1, "('DgnDb06Dev')", true },

            // Match keyword
            { 1, "email MATCH '\"401k\"'" },
            { 0, "email MATCH 'sender : \"401k\"'" },
            { 1, "email MATCH 'title : \"401k\"'" },

            // multiple search phrases
            { 2, "email MATCH '\"paul\" \"connelly\"'" },
            { 3, "email MATCH '\"connelly\" OR \"connolly\"'" },

            // prefix
            { 2, "email MATCH 'sender : \"conn\" *'" },
        };

    for (auto const& searchOp : searchOps)
        {
        SqlPrintfString sql("SELECT count(*) FROM email %s%s", searchOp.excludeWhere ? "" : "WHERE ", searchOp.where);
        Statement stmt;
        EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(m_db, sql));
        EXPECT_EQ(BE_SQLITE_ROW, stmt.Step());
        EXPECT_EQ(searchOp.count, stmt.GetValueInt(0)) << "Statement: " << sql.GetUtf8CP();
        }

    m_db.CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FTS5Test, ExternalContentTable)
    {
    SetupDb(L"ExternalContentTable.db");

    // Create the content table
    EXPECT_EQ(BE_SQLITE_OK, m_db.CreateTable("email", "sender, title, body, nosearch"));

    // Create the FTS index referencing the content table
    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("CREATE VIRTUAL TABLE fts using fts5(sender, title, body, content=email, content_rowid=rowid)"));

    // Set up the triggers to keep the index in sync with the content table
    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("CREATE TRIGGER email_ai AFTER INSERT ON email BEGIN"
                                            " INSERT INTO fts(rowid, sender, title, body) VALUES (new.rowid, new.sender, new.title, new.body);"
                                            " END;"));
    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("CREATE TRIGGER email_ad AFTER DELETE ON email BEGIN"
                                            " INSERT INTO fts(fts, rowid, sender, title, body) VALUES('delete', old.rowid, old.sender, old.title, old.body);"
                                            " END;"));
    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("CREATE TRIGGER email_au AFTER UPDATE ON email BEGIN"
                                            " INSERT INTO fts(fts, rowid, sender, title, body) VALUES('delete', old.rowid, old.sender, old.title, old.body);"
                                            " INSERT INTO fts(rowid, sender, title, body) VALUES(new.rowid, new.sender, new.title, new.body);"
                                            " END;"));

    // Populate the content table
    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("INSERT INTO email (sender, title, body, nosearch) VALUES ('paul.connelly@bentley.com', 'DgnDb06Dev', 'Sprint Review', 'this text is secret')"));
    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("INSERT INTO email (sender, title, body, nosearch) VALUES ('paul.connolly@bentley.com', 'Typo', 'Wrong Paul', 'not searchable')"));
    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("INSERT INTO email (sender, title, body, nosearch) VALUES ('paul.schaeffer@nbc.com', 'Retirement', 'Good luck Dave!', 'cc stephen colbert')"));
    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("INSERT INTO email (sender, title, body, nosearch) VALUES ('ingrid@retirement.com', 'Your 401k', 'Paul Michael Connelly, please call me.', 'secret unindexed text')"));

    // Search
    struct SearchOp { int32_t count; Utf8CP where; int32_t countAfterUpdate; };
    SearchOp searchOps[] =
        {
            // = operator
            { 4, "fts = 'paul'", 2 },
            { 2, "fts = 'bentley'", 1 },
            { 0, "fts = 'secret'", 0 },  // only un-indexed columns match
            { 2, "fts = 'retirement'", 2 },

            // Following 3 syntaxes are supposed to be equivalent
            { 1, "fts = 'DgnDb06Dev'", 0 },
            { 1, "fts MATCH 'DgnDb06Dev'", 0 },

            // Match keyword
            { 1, "fts MATCH '\"401k\"'", 1 },
            { 0, "fts MATCH 'sender : \"401k\"'", 0 },
            { 1, "fts MATCH 'title : \"401k\"'", 1 },

            // multiple search phrases
            { 2, "fts MATCH '\"paul\" \"connelly\"'", 0 },
            { 3, "fts MATCH '\"connelly\" OR \"connolly\"'", 1 },

            // prefix
            { 2, "fts MATCH 'sender : \"conn\" *'", 1 },
        };

    for (auto const& searchOp : searchOps)
        {
        SqlPrintfString sql("SELECT count(*) FROM fts WHERE %s", searchOp.where);
        Statement stmt;
        EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(m_db, sql));
        EXPECT_EQ(BE_SQLITE_ROW, stmt.Step());
        EXPECT_EQ(searchOp.count, stmt.GetValueInt(0)) << "Statement: " << sql.GetUtf8CP();
        }

    // Modify content table
        {
        Statement deleteStmt;
        EXPECT_EQ(BE_SQLITE_OK, deleteStmt.Prepare(m_db, "DELETE FROM email WHERE rowid=1"));
        EXPECT_EQ(BE_SQLITE_DONE, deleteStmt.Step());

        Statement updateStmt;
        EXPECT_EQ(BE_SQLITE_OK, updateStmt.Prepare(m_db, "UPDATE email SET sender='enid@retirement.com', title='Your 401k', body='never mind', nosearch='' WHERE rowid=4"));
        EXPECT_EQ(BE_SQLITE_DONE, updateStmt.Step());
        }

    // Query on modified content
    for (auto const& searchOp : searchOps)
        {
        SqlPrintfString sql("SELECT count(*) FROM fts WHERE %s", searchOp.where);
        Statement stmt;
        EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(m_db, sql));
        EXPECT_EQ(BE_SQLITE_ROW, stmt.Step());
        EXPECT_EQ(searchOp.countAfterUpdate, stmt.GetValueInt(0)) << "Statement: " << sql.GetUtf8CP();
        }

    m_db.CloseDb();
    }

