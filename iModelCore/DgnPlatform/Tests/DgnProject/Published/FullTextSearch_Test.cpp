/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/FullTextSearch_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"

USING_NAMESPACE_BENTLEY_SQLITE

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct SearchableTextTest : public GenericDgnModelTestFixture
{
    DgnDbR m_db;
    uint32_t m_curIdInt = 1;

    typedef DgnSearchableText ST;

    SearchableTextTest() : GenericDgnModelTestFixture(__FILE__, false), m_db(*m_testDgnManager.GetDgnProjectP())
        {
        BeAssert(nullptr != m_testDgnManager.GetDgnProjectP());
        }

    BeInt64Id PeekNextId() const { return BeInt64Id(m_curIdInt); }
    BeInt64Id UseNextId() { return BeInt64Id(m_curIdInt++); }

    ST::Query MakeQuery(Utf8CP text, Utf8CP cat1=nullptr, Utf8CP cat2=nullptr)
        {
        ST::Query query(text, cat1);
        if (nullptr != cat2)
            query.AddTextType(cat2);

        return query;
        }

    typedef std::initializer_list<uint64_t> IdList;

    void ExpectMatches(ST::Query const& query, IdList const& matchIds)
        {
        bset<BeInt64Id> foundIds;
        for (auto const& entry : m_db.SearchableText().QueryRecords(query))
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
        EXPECT_EQ(expectedCount, m_db.SearchableText().QueryCount(MakeQuery(text, cat1, cat2))) << text << "(" << (cat1?cat1:"null") << ") (" << (cat2?cat2:"null") << ")";
        }

    void Insert(Utf8CP category, Utf8CP text)
        {
        ST::Record record(category, UseNextId(), text);
        EXPECT_EQ(BE_SQLITE_OK, m_db.SearchableText().Insert(record));
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchableTextTest, Query)
    {
    // Populate searchable text table
    struct Client { Utf8CP name; Utf8CP address; Utf8CP company; };
    Client clients[] =
        {
            { "John Doe", "123 Milky Way", "Things n' Stuff Inc" },
            { "Lisa Lin", "27 John Doe Ave", "Good Eats Bakery" },
            { "Fred \"Fedderman\" Fud", "622 Main Street", "Main Street Bakery" },
            { "Edd Good", "123 Frankfurt Ave", "Good Stuff Home Furnishings" },
        };

    auto& st = m_db.SearchableText();

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
        queryAll.AddTextType("Address");
        queryAll.AddTextType("Company");
        EXPECT_EQ(totalCount, st.QueryCount(queryAll)) << searchOp.text;

        // Filter by each type
        for (auto i = 0; i < kMax; i++)
            {
            ST::Query query(searchOp.text, searchTypes[i]);
            EXPECT_EQ(searchOp.count[i], st.QueryCount(query)) << searchTypes[i] << " : " << searchOp.text;
            }

        // Filter by two types
        ST::Query nameAddress(searchOp.text, "Name");
        nameAddress.AddTextType("Address");
        EXPECT_EQ(searchOp.count[kName]+searchOp.count[kAddress], st.QueryCount(nameAddress)) << searchOp.text;

        ST::Query nameCompany(searchOp.text, "Name");
        nameCompany.AddTextType("Company");
        EXPECT_EQ(searchOp.count[kName]+searchOp.count[kCompany], st.QueryCount(nameCompany)) << searchOp.text;

        ST::Query addressCompany(searchOp.text, "Address");
        addressCompany.AddTextType("Company");
        EXPECT_EQ(searchOp.count[kAddress]+searchOp.count[kCompany], st.QueryCount(addressCompany)) << searchOp.text;

        // Filter by unknown type
        ST::Query unknown(searchOp.text, "Unknown");
        EXPECT_EQ(0, st.QueryCount(unknown));
        }

    // Query categories
    auto cats = st.QueryTextTypes();
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
            EXPECT_EQ(Utf8String(searchTypes[i]), rec.GetTextType());
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
    }

/*---------------------------------------------------------------------------------**//**
* Matching on category should be exact.
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchableTextTest, CategoryNames)
    {
    auto& st = m_db.SearchableText();
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
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchableTextTest, DropAndUpdate)
    {
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
    auto& st = m_db.SearchableText();
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
    EXPECT_EQ(BE_SQLITE_OK, st.DropTextType("CatB"));
    ExpectCount(1, abc);
    ExpectCount(1, xyz);
    ExpectCount(1, "\"abc def\"");
    ExpectCount(0, abc, "CatB");
    ExpectCount(0, xyz, "CatB");

    // Drop all categories
    EXPECT_EQ(BE_SQLITE_OK, st.DropAll());
    ExpectCount(0, abc);
    ExpectCount(0, xyz);
    }

