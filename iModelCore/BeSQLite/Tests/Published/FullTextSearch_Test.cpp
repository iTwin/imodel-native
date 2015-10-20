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
            { "John Doe",   1,      1,      0 },
            { "Ave",        0,      2,      0 },
            { "Bakery",     0,      0,      2 },
            { "Good",       1,      0,      2 },
            { "Main Street",0,      1,      1 },
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

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct SearchableTextTest : FTS5Test
{
    typedef SearchableText ST;

    uint32_t m_curIdInt = 1;

    BeInt64Id PeekNextId() const { return BeInt64Id(m_curIdInt); }
    BeInt64Id UseNextId() { return BeInt64Id(m_curIdInt++); }

    ST::Query MakeQuery(Utf8CP text, Utf8CP cat1=nullptr, Utf8CP cat2=nullptr)
        {
        ST::Query query(text, cat1);
        if (nullptr != cat2)
            query.AddCategory(cat2);

        return query;
        }

    typedef std::initializer_list<uint64_t> IdList;

    void ExpectMatches(ST::Query const& query, IdList const& matchIds)
        {
        bset<BeInt64Id> foundIds;
        for (auto const& entry : m_db.GetSearchableText().QueryRecords(query))
            {
            EXPECT_TRUE(foundIds.end() == foundIds.find(entry.GetId()));
            EXPECT_TRUE(matchIds.end() != std::find_if(matchIds.begin(), matchIds.end(), [&](uint64_t arg) { return BeInt64Id(arg) == entry.GetId(); }));
            foundIds.insert(entry.GetId());
            }

        EXPECT_EQ(matchIds.size(), foundIds.size());
        }

    void ExpectMatches(Utf8CP text, IdList const& matchIds, Utf8CP cat1=nullptr, Utf8CP cat2=nullptr)
        {
        return ExpectMatches(MakeQuery(text, cat1, cat2), matchIds);
        }

    void ExpectCount(size_t expectedCount, Utf8CP text, Utf8CP cat1=nullptr, Utf8CP cat2=nullptr)
        {
        EXPECT_EQ(expectedCount, m_db.GetSearchableText().QueryCount(MakeQuery(text, cat1, cat2))) << text << "(" << (cat1?cat1:"null") << ") (" << (cat2?cat2:"null") << ")";
        }

    void Insert(Utf8CP category, Utf8CP text)
        {
        ST::Record record(category, UseNextId(), text);
        EXPECT_EQ(BE_SQLITE_OK, m_db.GetSearchableText().Insert(record));
        }
};

/*---------------------------------------------------------------------------------**//**
* Testing full-text search queries using BeSQLite::SearchableText API.
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchableTextTest, Query)
    {
    SetupDb(L"SearchableTextApi.db");

    // Populate searchable text table
    struct Client { Utf8CP name; Utf8CP address; Utf8CP company; };
    Client clients[] =
        {
            { "John Doe", "123 Milky Way", "Things n' Stuff Inc" },
            { "Lisa Lin", "27 John Doe Ave", "Good Eats Bakery" },
            { "Fred \"Fedderman\" Fud", "622 Main Street", "Main Street Bakery" },
            { "Edd Good", "123 Frankfurt Ave", "Good Stuff Home Furnishings" },
        };

    SearchableText st = m_db.GetSearchableText();

    for (auto const& client : clients)
        {
        BeInt64Id id = UseNextId();
        ST::Record company("Company", id, client.company);
        EXPECT_EQ(BE_SQLITE_OK, st.Insert(company));
        ST::Record address("Address", id, client.address);
        EXPECT_EQ(BE_SQLITE_OK, st.Insert(address));
        ST::Record name("Name", id, client.name);
        EXPECT_EQ(BE_SQLITE_OK, st.Insert(name));
        }

    struct DisableAssertionFailures
        {
        DisableAssertionFailures() { BeTest::SetFailOnAssert(false); }
        ~DisableAssertionFailures() { BeTest::SetFailOnAssert(true); }
        };

    // Verify primary key
        {
        DisableAssertionFailures V_V_V_;
        EXPECT_EQ(BE_SQLITE_CONSTRAINT_PRIMARYKEY, st.Insert(ST::Record("Name", BeInt64Id(1LL), "Jane Doe")));
        }

    // Query counts
    enum { kName=0, kAddress, kCompany, kMax };
    struct SearchOp { Utf8CP text; int32_t count[kMax]; };
    SearchOp searchOps[] =
        {
            // text             name    address company
            { "\"John Doe\"",   1,      1,      0 },
            { "\"Ave\"",        0,      2,      0 },
            { "\"Bakery\"",     0,      0,      2 },
            { "\"Good\"",       1,      0,      2 },
            { "\"Main Street\"",0,      1,      1 },
        };

    Utf8CP searchTypes[] = { "Name", "Address", "Company" };
    for (auto const& searchOp : searchOps)
        {
        // unfiltered
        auto totalCount = searchOp.count[0] + searchOp.count[1] + searchOp.count[2];
        EXPECT_EQ(totalCount, st.QueryCount(ST::Query(searchOp.text))) << searchOp.text;
        
        // include all categories
        ST::Query queryAll(searchOp.text, "Name");
        queryAll.AddCategory("Address");
        queryAll.AddCategory("Company");
        EXPECT_EQ(totalCount, st.QueryCount(queryAll)) << searchOp.text;

        // Filter by each type
        for (auto i = 0; i < kMax; i++)
            {
            ST::Query query(searchOp.text, searchTypes[i]);
            EXPECT_EQ(searchOp.count[i], st.QueryCount(query)) << searchTypes[i] << " : " << searchOp.text;
            }

        // Filter by two types
        ST::Query nameAddress(searchOp.text, "Name");
        nameAddress.AddCategory("Address");
        EXPECT_EQ(searchOp.count[kName]+searchOp.count[kAddress], st.QueryCount(nameAddress)) << searchOp.text;

        ST::Query nameCompany(searchOp.text, "Name");
        nameCompany.AddCategory("Company");
        EXPECT_EQ(searchOp.count[kName]+searchOp.count[kCompany], st.QueryCount(nameCompany)) << searchOp.text;

        ST::Query addressCompany(searchOp.text, "Address");
        addressCompany.AddCategory("Company");
        EXPECT_EQ(searchOp.count[kAddress]+searchOp.count[kCompany], st.QueryCount(addressCompany)) << searchOp.text;

        // Filter by unknown type
        ST::Query unknown(searchOp.text, "Unknown");
        EXPECT_EQ(0, st.QueryCount(unknown));
        }

    // Query categories
    auto cats = st.QueryCategories();
    EXPECT_EQ(3, cats.size());
    EXPECT_TRUE(cats.end() != std::find(cats.begin(), cats.end(), "Name"));
    EXPECT_TRUE(cats.end() != std::find(cats.begin(), cats.end(), "Address"));
    EXPECT_TRUE(cats.end() != std::find(cats.begin(), cats.end(), "Company"));

    // Query individual records
    uint64_t idInt = 1;
    for (auto const& client : clients)
        {
        BeInt64Id id(idInt++);
        for (auto i = 0; i < kMax; i++)
            {
            ST::Record rec = st.QueryRecord(ST::Key(searchTypes[i], id));
            EXPECT_TRUE(rec.IsValid());
            Utf8String cmp(kName == i ? client.name : (kAddress == i ? client.address : client.company));
            EXPECT_EQ(cmp, rec.GetText());
            EXPECT_EQ(Utf8String(searchTypes[i]), rec.GetCategory());
            EXPECT_EQ(rec.GetId(), id);
            }
        }

    // Query records
    IdList emptyIds;
    ExpectMatches("\"John Doe\"", IdList{1,2});
    ExpectMatches("\"John Doe\"", IdList{1}, "Name");
    ExpectMatches("\"John Doe\"", IdList{2}, "Address");
    ExpectMatches("\"John Doe\"", emptyIds, "Company");
    ExpectMatches("\"Bakery\"", IdList{2,3});

    m_db.CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* Matching on category should be exact.
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchableTextTest, CategoryNames)
    {
    SetupDb(L"CategoryNames.db");

    SearchableText st = m_db.GetSearchableText();
    EXPECT_EQ(BE_SQLITE_OK, st.Insert(ST::Record("My Category", UseNextId(), "stuff")));
    EXPECT_EQ(BE_SQLITE_OK, st.Insert(ST::Record("My Category B", UseNextId(), "stuff")));
    EXPECT_EQ(BE_SQLITE_OK, st.Insert(ST::Record("Category", UseNextId(), "stuff")));
    EXPECT_EQ(BE_SQLITE_OK, st.Insert(ST::Record("Category B", UseNextId(), "stuff")));
    EXPECT_EQ(BE_SQLITE_OK, st.Insert(ST::Record("My Text", UseNextId(), "stuff")));

    Utf8CP text = "\"stuff\"";
    EXPECT_EQ(5, st.QueryCount(MakeQuery(text)));
    EXPECT_EQ(1, st.QueryCount(MakeQuery(text, "My Category")));
    EXPECT_EQ(1, st.QueryCount(MakeQuery(text, "Category")));
    EXPECT_EQ(0, st.QueryCount(MakeQuery(text, "My")));
    EXPECT_EQ(2, st.QueryCount(MakeQuery(text, "Category", "Category B")));

    // MATCH expression should not match on category names
    EXPECT_EQ(0, st.QueryCount(MakeQuery("\"Category\"")));

    m_db.CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchableTextTest, DropAndUpdate)
    {
    SetupDb(L"DropAndUpdate.db");

    //     Cat     Text         ID
    Insert("CatA", "abc");      // 1
    Insert("CatA", "abc def");  // 2
    Insert("CatB", "abc def");  // 3
    Insert("CatB", "def");      // 4
    Insert("CatB", "xyz");      // 5
    Insert("CatA", "xyz");      // 6

    Utf8CP abc = "\"abc\"";
    Utf8CP xyz = "\"xyz\"";

    ExpectCount(3, abc);
    ExpectCount(2, abc, "CatA");
    ExpectCount(1, abc, "CatB");
    ExpectCount(2, xyz);
    ExpectCount(1, xyz, "CatA");
    ExpectCount(1, xyz, "CatB");

    // Update the text associated with one record
    // Result:
    //     Cat     Text         ID
    //     "CatA"  "xyz"         1  <= changed text
    //     "CatA"  "abc def"     2
    //     "CatB"  "abc def"     3
    //     "CatB"  "def"         4
    //     "CatB"  "xyz"         5
    //     "CatA"  "xyz"         6
    auto st = m_db.GetSearchableText();
    EXPECT_EQ(BE_SQLITE_OK, st.Update(ST::Record("CatA", BeInt64Id(1LL), "xyz"), nullptr));
    ExpectCount(2, abc);
    ExpectCount(3, xyz);
    ExpectCount(2, xyz, "CatA");
    ExpectCount(1, xyz, "CatB");

    // Change category + ID. Note two entries with same ID can exist as long as in different categories.
    // Result:
    //     Cat     Text         ID
    //     "CatB"  "xyz"         2  <= changed category and ID
    //     "CatA"  "abc def"     2
    //     "CatB"  "abc def"     3
    //     "CatB"  "def"         4
    //     "CatB"  "xyz"         5
    //     "CatA"  "xyz"         6
    ST::Key key("CatA", BeInt64Id(1LL));
    EXPECT_EQ(BE_SQLITE_OK, st.Update(ST::Record("CatB", BeInt64Id(2LL), "xyz"), &key));
    ExpectCount(1, xyz, "CatA");
    ExpectCount(2, xyz, "CatB");
    ExpectCount(3, xyz);

    // Drop a single record
    // Result:
    //     Cat     Text         ID
    //     "CatA"  "abc def"     2
    //     "CatB"  "abc def"     3
    //     "CatB"  "def"         4
    //     "CatB"  "xyz"         5
    //     "CatA"  "xyz"         6
    key = ST::Key("CatB", BeInt64Id(2LL));
    EXPECT_EQ(BE_SQLITE_OK, st.DropRecord(key));
    ExpectCount(2, xyz);

    // Drop a category
    // Result:
    //     Cat     Text         ID
    //     "CatA"  "abc def"     2
    //     "CatA"  "xyz"         6
    EXPECT_EQ(BE_SQLITE_OK, st.DropCategory("CatB"));
    ExpectCount(1, abc);
    ExpectCount(1, xyz);
    ExpectCount(1, "\"abc def\"");
    ExpectCount(0, abc, "CatB");
    ExpectCount(0, xyz, "CatB");

    // Drop all categories
    EXPECT_EQ(BE_SQLITE_OK, st.DropAll());
    ExpectCount(0, abc);
    ExpectCount(0, xyz);

    m_db.CloseDb();
    }

