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
            enum class Columns {
                Token = 0,
                Text = 1,
                Delimiter = 2,
            };

           private:
            int64_t m_iRowid = 0;
            Utf8String m_text;
            Utf8String m_delimiter;
            bvector<Utf8String> m_tokens;

           public:
            TokenizeCursor(TokenizeTable& vt) : ECDbCursor(vt) {}
            bool Eof() final { return m_iRowid < 1 || m_iRowid > (int64_t)m_tokens.size(); }
            DbResult Next() final {
                ++m_iRowid;
                return BE_SQLITE_OK;
            }
            DbResult GetColumn(int i, Context& ctx) final {
                Utf8CP x = 0;
                switch ((Columns)i) {
                    case Columns::Text:
                        x = m_text.c_str();
                        break;
                    case Columns::Delimiter:
                        x = m_delimiter.c_str();
                        break;
                    default:
                        x = m_tokens[m_iRowid - 1].c_str();
                        break;
                }
                ctx.SetResultText(x, (int)strlen(x), Context::CopyData::Yes);
                return BE_SQLITE_OK;
            }
            DbResult GetRowId(int64_t& rowId) final {
                rowId = m_iRowid;
                return BE_SQLITE_OK;
            }
            DbResult Filter(int idxNum, const char* idxStr, int argc, DbValue* argv) final {
                int i = 0;
                if (idxNum & 1) {
                    m_text = argv[i++].GetValueText();
                } else {
                    m_text = "";
                }
                if (idxNum & 2) {
                    m_delimiter = argv[i++].GetValueText();
                } else {
                    m_delimiter = ";";
                }
                m_tokens.clear();
                BeStringUtilities::Split(m_text.c_str(), m_delimiter.c_str(), m_tokens);
                if (idxNum & 8)
                    std::sort(m_tokens.begin(), m_tokens.end(), std::greater<>());
                else if (idxNum & 16)
                    std::sort(m_tokens.begin(), m_tokens.end(), std::less<>());

                m_iRowid = 1;
                return BE_SQLITE_OK;
            }
        };

       public:
        TokenizeTable(TokenizeModule& module) : ECDbVirtualTable(module) {}
        DbResult Open(DbCursor*& cur) override {
            cur = new TokenizeCursor(*this);
            return BE_SQLITE_OK;
        }
        DbResult BestIndex(IndexInfo& indexInfo) final {
            int i, j;             /* Loop over constraints */
            int idxNum = 0;       /* The query plan bitmask */
            int unusableMask = 0; /* Mask of unusable constraints */
            int nArg = 0;         /* Number of arguments that seriesFilter() expects */
            int aIdx[2];          /* Constraints on start, stop, and step */
            const int SQLITE_SERIES_CONSTRAINT_VERIFY = 0;
            aIdx[0] = aIdx[1] = -1;
            int nConstraint = indexInfo.GetConstraintCount();

            for (i = 0; i < nConstraint; i++) {
                auto pConstraint = indexInfo.GetConstraint(i);
                int iCol;  /* 0 for start, 1 for stop, 2 for step */
                int iMask; /* bitmask for those column */
                if (pConstraint->GetColumn() < (int)TokenizeCursor::Columns::Text) continue;
                iCol = pConstraint->GetColumn() - (int)TokenizeCursor::Columns::Text;
                iMask = 1 << iCol;
                if (!pConstraint->IsUsable()) {
                    unusableMask |= iMask;
                    continue;
                } else if (pConstraint->GetOp() == IndexInfo::Operator::EQ) {
                    idxNum |= iMask;
                    aIdx[iCol] = i;
                }
            }
            for (i = 0; i < 2; i++) {
                if ((j = aIdx[i]) >= 0) {
                    indexInfo.GetConstraintUsage(j)->SetArgvIndex(++nArg);
                    indexInfo.GetConstraintUsage(j)->SetOmit(!SQLITE_SERIES_CONSTRAINT_VERIFY);
                }
            }

            if ((unusableMask & ~idxNum) != 0) {
                return BE_SQLITE_CONSTRAINT;
            }

            indexInfo.SetEstimatedCost(2.0);
            indexInfo.SetEstimatedRows(1000);
            if (indexInfo.GetIndexOrderByCount() >= 1 && indexInfo.GetOrderBy(0)->GetColumn() == 0) {
                if (indexInfo.GetOrderBy(0)->GetDesc()) {
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
    TokenizeModule(ECDbR db) : ECDbModule(
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
        while (stmt.Step() == BE_SQLITE_ROW) {
            ASSERT_STREQ(expected[i++].c_str(), stmt.GetValueText(0));
        }
        ASSERT_EQ(i, 9);
    }
    if ("sorted ascending") {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT token FROM test.tokenize_text('the quick brown fox jumps over the lazy dog', ' ') ORDER BY token"));
        auto expected = std::vector<std::string>{"brown", "dog", "fox", "jumps", "lazy", "over", "quick", "the", "the"};
        int i = 0;
        while (stmt.Step() == BE_SQLITE_ROW) {
            ASSERT_STREQ(expected[i++].c_str(), stmt.GetValueText(0));
        }
        ASSERT_EQ(i, 9);
    }
    if ("sorted descending") {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT token FROM test.tokenize_text('the quick brown fox jumps over the lazy dog', ' ') ORDER BY token DESC"));
        auto expected = std::vector<std::string>{"the", "the", "quick", "over", "lazy", "jumps", "fox", "dog", "brown"};
        int i = 0;
        while (stmt.Step() == BE_SQLITE_ROW) {
            ASSERT_STREQ(expected[i++].c_str(), stmt.GetValueText(0));
        }
        ASSERT_EQ(i, 9);
    }
}

END_ECDBUNITTESTS_NAMESPACE
