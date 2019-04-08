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
* Testing SQLite's FTS5 module using virtual table.
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
* Testing SQLite's FTS5 module using external content table.
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

/*---------------------------------------------------------------------------------**//**
* Testing features required for BeSQLite FTS5 API.
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FTS5Test, FilterExternalContentTable)
    {
    SetupDb(L"FilterExternalContentTable");

    // The content table consists of:
    //  * Text - searchable text from a row in some other table
    //  * Type - a string identifying the type of the data in the referenced table
    //  * Id - the ID of the data in the referenced table
    // Callers wants to perform full-text search on our content table, optionally filtered by one or more Types.
    // The combination of the Type+Id must be unique
    EXPECT_EQ(BE_SQLITE_OK, m_db.CreateTable("fts", "Type NOT NULL, Id INTEGER NOT NULL, Text NOT NULL, PRIMARY KEY (Type,Id)"));

    // The virtual table indexes searchable text for the "Text" column only.
    // ###TODO: probably should create a separate index on "Type"
    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("CREATE VIRTUAL TABLE fts_idx using fts5(Type UNINDEXED, Id UNINDEXED, Text, content=fts, content_rowid=rowid)"));
    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("CREATE TRIGGER fts_ai AFTER INSERT ON fts BEGIN"
                                            " INSERT INTO fts_idx(rowid, Type, Id, Text) VALUES(new.rowid, new.Type, new.Id, new.Text);"
                                            " END;"));
    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("CREATE TRIGGER fts_ad AFTER DELETE ON fts BEGIN"
                                            " INSERT INTO fts_idx(fts_idx, rowid, Type, Id, Text) VALUES('delete', old.rowid, old.Type, old.Id, old.Text);"
                                            " END;"));
    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("CREATE TRIGGER fts_au AFTER UPDATE ON fts BEGIN"
                                            " INSERT INTO fts_idx(fts_idx, rowid, Type, Id, Text) VALUES('delete', old.rowid, old.Type, old.Id, old.Text);"
                                            " INSERT INTO fts_idx(rowid, Type, Id, Text) VALUES(new.rowid, new.Type, new.Id, new.Text);"
                                            " END;"));

    // Populate the content table
        {
        struct Client { Utf8CP name; Utf8CP address; Utf8CP company; };
        Client clients[] =
            {
                { "John Doe", "123 Milky Way", "Things n Stuff Inc" },
                { "Lisa Lin", "27 John Doe Ave", "Good Eats Bakery" },
                { "Fred Fud", "622 Main Street", "Main Street Bakery" },
                { "Edd Good", "123 Frankfurt Ave", "Good Stuff Home Furnishings" },
            };

        Statement stmt;
        EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(m_db, "INSERT INTO fts (Type, Id, Text) VALUES(?,?,?)"));
        int32_t id = 1;
        for (auto const& client : clients)
            {
            stmt.BindText(1, "Company", Statement::MakeCopy::No);
            stmt.BindInt(2, id++);
            stmt.BindText(3, client.company, Statement::MakeCopy::No);
            EXPECT_EQ(BE_SQLITE_DONE, stmt.Step());

            stmt.Reset();
            stmt.BindText(1, "Address", Statement::MakeCopy::No);
            stmt.BindText(3, client.address, Statement::MakeCopy::No);
            EXPECT_EQ(BE_SQLITE_DONE, stmt.Step());

            stmt.Reset();
            stmt.BindText(1, "Name", Statement::MakeCopy::No);
            stmt.BindText(3, client.name, Statement::MakeCopy::No);
            EXPECT_EQ(BE_SQLITE_DONE, stmt.Step());

            stmt.Reset();
            }
        }

    struct DisableAssertionFailures
        {
        DisableAssertionFailures() { BeTest::SetFailOnAssert(false); }
        ~DisableAssertionFailures() { BeTest::SetFailOnAssert(true); }
        };

    // Verify primary key
        {
        DisableAssertionFailures V_V_V_;
        EXPECT_EQ(BE_SQLITE_CONSTRAINT_PRIMARYKEY, m_db.ExecuteSql("INSERT INTO fts (Type, Id, Text) VALUES('Company', 1, 'my text')"));
        }

    // Query
    enum { kName=0, kAddress, kCompany, kMax };
    struct SearchOp { Utf8CP text; int32_t count[kMax]; };
    SearchOp searchOps[] =
        {
            // text         name    address company
            { "John Doe",   {1,      1,      0} },
            { "Ave",        {0,      2,      0} },
            { "Bakery",     {0,      0,      2} },
            { "Good",       {1,      0,      2} },
            { "Main Street",{0,      1,      1} },
        };

    Utf8CP searchTypes[] = { "Name", "Address", "Company" };
    for (auto const& searchOp : searchOps)
        {
        Utf8PrintfString where("fts_idx MATCH '\"%s\"'", searchOp.text);
        
        // unfiltered
        auto totalCount = searchOp.count[0] + searchOp.count[1] + searchOp.count[2];
        Statement stmt;
        Utf8PrintfString selectUnfiltered("SELECT count(*) FROM fts_idx WHERE %s", where.c_str());
        EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(m_db, selectUnfiltered.c_str()));
        EXPECT_EQ(BE_SQLITE_ROW, stmt.Step());
        EXPECT_EQ(totalCount, stmt.GetValueInt(0)) << "Statement: " << selectUnfiltered.c_str();

        // filtered by all types
        stmt.Finalize();
        Utf8PrintfString selectAll("SELECT count(*) FROM fts_idx WHERE %s AND Type IN ('Name','Address','Company')", where.c_str());
        EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(m_db, selectAll.c_str()));
        EXPECT_EQ(BE_SQLITE_ROW, stmt.Step());
        EXPECT_EQ(totalCount, stmt.GetValueInt(0)) << "Statement: " << selectAll.c_str();

        // Filter by each type
        for (auto i = 0; i < kMax; i++)
            {
            Utf8PrintfString sql("SELECT count(*) FROM fts_idx WHERE Type = '%s' AND %s", searchTypes[i], where.c_str());
            stmt.Finalize();
            EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(m_db, sql.c_str()));
            EXPECT_EQ(BE_SQLITE_ROW, stmt.Step());
            EXPECT_EQ(searchOp.count[i], stmt.GetValueInt(0)) << "Statement: " << sql.c_str();
            }

        // Filter by two types
        stmt.Finalize();
        Utf8PrintfString selectNameAndAddress("SELECT count(*) FROM fts_idx WHERE (Type = 'Name' OR Type = 'Address') AND %s", where.c_str());
        EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(m_db, selectNameAndAddress.c_str()));
        EXPECT_EQ(BE_SQLITE_ROW, stmt.Step());
        EXPECT_EQ(searchOp.count[kName] + searchOp.count[kAddress], stmt.GetValueInt(0)) << selectNameAndAddress.c_str();

        stmt.Finalize();
        Utf8PrintfString selectNameAndCompany("SELECT count(*) FROM fts_idx WHERE (Type='Name' OR Type='Company') AND %s", where.c_str());
        EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(m_db, selectNameAndCompany.c_str()));
        EXPECT_EQ(BE_SQLITE_ROW, stmt.Step());
        EXPECT_EQ(searchOp.count[kName] + searchOp.count[kCompany], stmt.GetValueInt(0)) << selectNameAndCompany.c_str();

        stmt.Finalize();
        Utf8PrintfString selectAddressAndCompany("SELECT count(*) FROM fts_idx WHERE (Type='Address' OR Type='Company') AND %s", where.c_str());
        EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(m_db, selectAddressAndCompany.c_str()));
        EXPECT_EQ(BE_SQLITE_ROW, stmt.Step());
        EXPECT_EQ(searchOp.count[kAddress] + searchOp.count[kCompany], stmt.GetValueInt(0)) << selectAddressAndCompany.c_str();

        // Filter by unknown type
        stmt.Finalize();
        Utf8PrintfString selectNone("SELECT count(*) FROM fts_idx WHERE Type='Unknown' AND %s", where.c_str());
        EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(m_db, selectNone.c_str()));
        EXPECT_EQ(BE_SQLITE_ROW, stmt.Step());
        EXPECT_EQ(0, stmt.GetValueInt(0)) << selectNone.c_str();
        }

    m_db.CloseDb();
    }

