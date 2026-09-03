/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
struct ECDbVirtualTableTests : ECDbTestFixture {};
//=======================================================================================
//! Virtual Table to tokenize string
// @bsiclass
//=======================================================================================
struct TokenizeModule : ECDbModule {
    struct TokenizeTable : ECDbVirtualTable {
        struct TokenizeCursor : ECDbCursor {
            enum class Columns{
                Token = 0,
                Text = 1,
                Delimiter =2,
            };
            private:
                int64_t m_iRowid = 0;
                Utf8String m_text;
                Utf8String m_delimiter;
                bvector<Utf8String> m_tokens;

            public:
                TokenizeCursor(TokenizeTable& vt): ECDbCursor(vt){}
                bool Eof() final { return m_iRowid < 1 || m_iRowid > (int64_t)m_tokens.size() ; }
                DbResult Next() final {
                    ++m_iRowid;
                    return BE_SQLITE_OK;
                }
                DbResult GetColumn(int i, Context& ctx) final {
                    Utf8CP x = 0;
                    switch( (Columns)i ){
                        case Columns::Text: x = m_text.c_str(); break;
                        case Columns::Delimiter: x = m_delimiter.c_str(); break;
                        default: x = m_tokens[m_iRowid - 1].c_str(); break;
                    }
                    ctx.SetResultText(x, (int)strlen(x), Context::CopyData::Yes);
                    return BE_SQLITE_OK;
                }
                DbResult GetRowId(int64_t& rowId) final {
                    rowId = m_iRowid;
                    return BE_SQLITE_OK;
                }
                DbResult Filter(int idxNum, const char *idxStr, int argc, DbValue* argv) final {
                    int i = 0;
                    if( idxNum & 1 ){
                        m_text = argv[i++].GetValueText();
                    }else{
                        m_text = "";
                    }
                    if( idxNum & 2 ){
                        m_delimiter = argv[i++].GetValueText();
                    }else{
                        m_delimiter = ";";
                    }
                    m_tokens.clear();
                    BeStringUtilities::Split(m_text.c_str(), m_delimiter.c_str(), m_tokens);
                    if (idxNum & 8)
                        std:: sort(m_tokens.begin(), m_tokens.end(), std::greater <>());
                    else if (idxNum & 16)
                        std:: sort(m_tokens.begin(), m_tokens.end(), std::less <>());

                    m_iRowid = 1;
                    return BE_SQLITE_OK;
                }
        };
        public:
            TokenizeTable(TokenizeModule& module): ECDbVirtualTable(module) {}
            DbResult Open(DbCursor*& cur) override {
                cur = new TokenizeCursor(*this);
                return BE_SQLITE_OK;
            }
             DbResult BestIndex(IndexInfo& indexInfo) final {
                 int i, j;              /* Loop over constraints */
                int idxNum = 0;        /* The query plan bitmask */
                int unusableMask = 0;  /* Mask of unusable constraints */
                int nArg = 0;          /* Number of arguments that seriesFilter() expects */
                int aIdx[2];           /* Constraints on start, stop, and step */
                const int SQLITE_SERIES_CONSTRAINT_VERIFY = 0;
                aIdx[0] = aIdx[1] = -1;
                int nConstraint = indexInfo.GetConstraintCount();

                for(i=0; i<nConstraint; i++){
                    auto pConstraint = indexInfo.GetConstraint(i);
                    int iCol;    /* 0 for start, 1 for stop, 2 for step */
                    int iMask;   /* bitmask for those column */
                    if( pConstraint->GetColumn()< (int)TokenizeCursor::Columns::Text) continue;
                    iCol = pConstraint->GetColumn() - (int)TokenizeCursor::Columns::Text;
                    iMask = 1 << iCol;
                    if (!pConstraint->IsUsable()){
                        unusableMask |=  iMask;
                        continue;
                    } else if (pConstraint->GetOp() == IndexInfo::Operator::EQ ){
                        idxNum |= iMask;
                        aIdx[iCol] = i;
                    }
                }
                for( i = 0; i < 2; i++) {
                    if( (j = aIdx[i]) >= 0 ) {
                        indexInfo.GetConstraintUsage(j)->SetArgvIndex(++nArg);
                        indexInfo.GetConstraintUsage(j)->SetOmit(!SQLITE_SERIES_CONSTRAINT_VERIFY);
                    }
                }

                if ((unusableMask & ~idxNum)!=0 ){
                    return BE_SQLITE_CONSTRAINT;
                }

                indexInfo.SetEstimatedCost(2.0);
                indexInfo.SetEstimatedRows(1000);
                if( indexInfo.GetIndexOrderByCount() >= 1 && indexInfo.GetOrderBy(0)->GetColumn() == 0 ) {
                    if( indexInfo.GetOrderBy(0) ->GetDesc()){
                        idxNum |= 8;
                    } else {
                        idxNum |= 16;
                    }
                    indexInfo.SetOrderByConsumed(true);
                }
                indexInfo.SetIdxNum(idxNum);
                return BE_SQLITE_OK;
             }
    };
    public:
        TokenizeModule(ECDbR db): ECDbModule(
            db,
            "tokenize_text",
            "CREATE TABLE x(token,buffer hidden,delimiter hidden)",
            R"xml(<?xml version="1.0" encoding="utf-8" ?>
            <ECSchema
                    schemaName="test"
                    alias="test"
                    version="1.0.0"
                    xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="ECDbVirtual" version="01.00.00" alias="ecdbvir" />
                <ECCustomAttributes>
                    <VirtualSchema xmlns="ECDbVirtual.01.00.00"/>
                </ECCustomAttributes>
                <ECEntityClass typeName="tokenize_text" modifier="Abstract">
                    <ECCustomAttributes>
                        <VirtualType xmlns="ECDbVirtual.01.00.00"/>
                    </ECCustomAttributes>
                    <ECProperty propertyName="token"  typeName="string"/>
                </ECEntityClass>
            </ECSchema>)xml") {}
        DbResult Connect(DbVirtualTable*& out, Config& conf, int argc, const char* const* argv) final {
            out = new TokenizeTable(*this);
            conf.SetTag(Config::Tags::Innocuous);
            return BE_SQLITE_OK;
        }
};

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbVirtualTableTests, TokenizeModuleTest) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("vtab.ecdb"));
    (new TokenizeModule(m_ecdb))->Register();
    if ("unsorted") {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT token FROM test.tokenize_text('The quick brown fox jumps over the lazy dog', ' ')"));
        auto expected = std::vector<std::string>{"The", "quick", "brown", "fox", "jumps", "over", "the", "lazy", "dog"};
        int i = 0;
        while(stmt.Step() == BE_SQLITE_ROW) {
            ASSERT_STREQ(expected[i++].c_str(), stmt.GetValueText(0));
        }
        ASSERT_EQ(i, 9);
    }
    if ("sorted ascending") {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT token FROM test.tokenize_text('the quick brown fox jumps over the lazy dog', ' ') ORDER BY token"));
        auto expected = std::vector<std::string>{"brown", "dog", "fox", "jumps", "lazy", "over", "quick", "the", "the"};
        int i = 0;
        while(stmt.Step() == BE_SQLITE_ROW) {
            ASSERT_STREQ(expected[i++].c_str(), stmt.GetValueText(0));
        }
        ASSERT_EQ(i, 9);
    }
    if ("sorted descending") {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT token FROM test.tokenize_text('the quick brown fox jumps over the lazy dog', ' ') ORDER BY token DESC"));
        auto expected = std::vector<std::string>{"the", "the", "quick", "over", "lazy", "jumps", "fox", "dog", "brown"};
        int i = 0;
        while(stmt.Step() == BE_SQLITE_ROW) {
            ASSERT_STREQ(expected[i++].c_str(), stmt.GetValueText(0));
        }
        ASSERT_EQ(i, 9);
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbVirtualTableTests, TokenizeModuleTestWithMultipleVTabs) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("vtab.ecdb"));
    (new TokenizeModule(m_ecdb))->Register();
    if ("multiple vtabs should work") {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT a.token, b.token FROM test.tokenize_text('The quick brown fox jumps over the lazy dog', ' ') a, test.tokenize_text('The quick brown fox jumps over the lazy dog', ' ') b"));
        auto expected = std::vector<std::string>{"The", "quick", "brown", "fox", "jumps", "over", "the", "lazy", "dog"};
        int i = -1;
        int j = 0;
        int rowCnt = 0;
        while(stmt.Step() == BE_SQLITE_ROW) {
            if(j % 9 == 0) 
            {
                i++; j=0;
            } 
            std::string expectedFirstValue = expected[i];
            std::string expectedSecondValue = expected[j++];
            ASSERT_STREQ(expectedFirstValue.c_str(), stmt.GetValueText(0));
            ASSERT_STREQ(expectedSecondValue.c_str(), stmt.GetValueText(1));
            rowCnt++;
        }
        ASSERT_EQ(rowCnt, 81);

    }
    if ("multiple vtabs with column alias should work") {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT a.token x, b.token y FROM test.tokenize_text('The quick brown fox jumps over the lazy dog', ' ') a, test.tokenize_text('The quick brown fox jumps over the lazy dog', ' ') b"));
        auto expected = std::vector<std::string>{"The", "quick", "brown", "fox", "jumps", "over", "the", "lazy", "dog"};
        int i = -1;
        int j = 0;
        int rowCnt = 0;
        while(stmt.Step() == BE_SQLITE_ROW) {
            if(j % 9 == 0) 
            {
                i++; j=0;
            } 
            std::string expectedFirstValue = expected[i];
            std::string expectedSecondValue = expected[j++];
            ASSERT_STREQ(expectedFirstValue.c_str(), stmt.GetValueText(0));
            ASSERT_STREQ(expectedSecondValue.c_str(), stmt.GetValueText(1));
            rowCnt++;
        }
        ASSERT_EQ(rowCnt, 81);

        ASSERT_EQ(stmt.GetColumnCount(), 2);
        ASSERT_STREQ("x", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_STREQ("y", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
    }
    if ("multiple json_each vtabs should work") {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM json_each('[1,2,3,4,5]') a, json_each('[1,2,3,4,5]') b"));
        auto expected = std::vector<std::string>{"1", "2", "3", "4", "5"};
        int i = -1;
        int j = 0;
        int rowCnt = 0;
        while(stmt.Step() == BE_SQLITE_ROW) {
            if(j % 5 == 0) 
            {
                i++; j=0;
            } 
            std::string expectedFirstValue = expected[i];
            std::string expectedSecondValue = expected[j++];
            ASSERT_STREQ(expectedFirstValue.c_str(), stmt.GetValueText(1)); // value column
            ASSERT_STREQ(expectedSecondValue.c_str(), stmt.GetValueText(8)); // value column
            rowCnt++;
        }
        ASSERT_EQ(rowCnt, 25);
        
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbVirtualTableTests, ExpandedProperties) {
    SchemaItem schema(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="ExpandedPropertiesTest" alias="ept" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
            <ECEntityClass typeName="Root">
                <ECProperty propertyName="RootA" typeName="string"/>
                <ECProperty propertyName="RootB" typeName="string"/>
            </ECEntityClass>
            <ECEntityClass typeName="Base">
                <BaseClass>Root</BaseClass>
                <ECProperty propertyName="BaseA" typeName="string"/>
                <ECProperty propertyName="Shared" typeName="string"/>
            </ECEntityClass>
            <ECEntityClass typeName="MixinOne" modifier="Abstract">
                <ECCustomAttributes>
                    <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                        <AppliesToEntityClass>Root</AppliesToEntityClass>
                    </IsMixin>
                </ECCustomAttributes>
                <ECProperty propertyName="MixinA" typeName="string"/>
                <ECProperty propertyName="MixinB" typeName="string"/>
            </ECEntityClass>
            <ECEntityClass typeName="MixinTwo" modifier="Abstract">
                <ECCustomAttributes>
                    <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                        <AppliesToEntityClass>Root</AppliesToEntityClass>
                    </IsMixin>
                </ECCustomAttributes>
                <ECProperty propertyName="MixinC" typeName="string"/>
                <ECProperty propertyName="MixinD" typeName="string"/>
            </ECEntityClass>
            <ECEntityClass typeName="Derived">
                <BaseClass>Base</BaseClass>
                <BaseClass>MixinOne</BaseClass>
                <BaseClass>MixinTwo</BaseClass>
                <ECProperty propertyName="Own" typeName="string"/>
                <ECProperty propertyName="Shared" typeName="string"/>
            </ECEntityClass>
        </ECSchema>)xml");
    ASSERT_EQ(SUCCESS, SetupECDb("expanded-properties.ecdb", schema));

    auto assertExpandedProperties = [this]() {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, R"ecsql(
            SELECT ep.ExpandedOrdinal, c.Name DeclaringClass, p.Name Property
            FROM ECVLib.ExpandedProperties(ec_classid('ExpandedPropertiesTest', 'Derived')) ep
            JOIN meta.ECPropertyDef p ON p.ECInstanceId=ep.PropertyId
            JOIN meta.ECClassDef c ON c.ECInstanceId=p.Class.Id
        )ecsql"));

        std::vector<std::string> actual;
        DbResult stepResult;
        while ((stepResult = stmt.Step()) == BE_SQLITE_ROW) {
            Utf8String row;
            row.Sprintf("%d:%s.%s", stmt.GetValueInt(0), stmt.GetValueText(1), stmt.GetValueText(2));
            actual.push_back(row.c_str());
        }
        EXPECT_EQ(BE_SQLITE_DONE, stepResult);

        std::vector<std::string> expected {
            "0:Root.RootA",
            "1:Root.RootB",
            "2:Base.BaseA",
            "3:MixinOne.MixinA",
            "4:MixinOne.MixinB",
            "5:MixinTwo.MixinC",
            "6:MixinTwo.MixinD",
            "7:Derived.Own",
            "8:Derived.Shared",
        };
        EXPECT_EQ(expected, actual);
    };

    assertExpandedProperties();
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    assertExpandedProperties();

    ECSqlStatement descending;
    ASSERT_EQ(ECSqlStatus::Success, descending.Prepare(m_ecdb, R"ecsql(
        SELECT ExpandedOrdinal
        FROM ECVLib.ExpandedProperties(ec_classid('ExpandedPropertiesTest', 'Derived'))
        ORDER BY ExpandedOrdinal DESC
    )ecsql"));
    for (int expected = 8; expected >= 0; --expected) {
        ASSERT_EQ(BE_SQLITE_ROW, descending.Step());
        EXPECT_EQ(expected, descending.GetValueInt(0));
    }
    EXPECT_EQ(BE_SQLITE_DONE, descending.Step());

    ECSqlStatement correlated;
    ASSERT_EQ(ECSqlStatus::Success, correlated.Prepare(m_ecdb, R"ecsql(
        SELECT c.Name, COUNT(*)
        FROM meta.ECSchemaDef s, meta.ECClassDef c, ECVLib.ExpandedProperties(c.ECInstanceId) ep
        WHERE c.Schema.Id=s.ECInstanceId AND s.Name='ExpandedPropertiesTest'
        GROUP BY c.Name
        ORDER BY c.Name
    )ecsql"));
    std::vector<std::pair<std::string, int>> expectedCounts {
        {"Base", 4},
        {"Derived", 9},
        {"MixinOne", 2},
        {"MixinTwo", 2},
        {"Root", 2},
    };
    for (auto const& expected : expectedCounts) {
        ASSERT_EQ(BE_SQLITE_ROW, correlated.Step());
        EXPECT_STREQ(expected.first.c_str(), correlated.GetValueText(0));
        EXPECT_EQ(expected.second, correlated.GetValueInt(1));
    }
    EXPECT_EQ(BE_SQLITE_DONE, correlated.Step());

    ECSqlStatement unknownClass;
    ASSERT_EQ(ECSqlStatus::Success, unknownClass.Prepare(m_ecdb, "SELECT PropertyId FROM ECVLib.ExpandedProperties(0x7fffffffffffffff)"));
    EXPECT_EQ(BE_SQLITE_DONE, unknownClass.Step());
}

END_ECDBUNITTESTS_NAMESPACE
