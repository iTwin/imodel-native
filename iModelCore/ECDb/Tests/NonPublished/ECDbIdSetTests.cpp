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
struct ECDbIdSetTests : ECDbTestFixture {};

//=======================================================================================
//! Virtual Table to tokenize array ids
// @bsiclass
//=======================================================================================
struct TokenizeIdModule : ECDbModule {
    struct TokenizeIdTable : ECDbVirtualTable {
        struct TokenizeIdCursor : ECDbCursor {
            enum class Columns{
                Id = 0,
                Text = 1,
                Delimiter =2,
            };
            private:
                int64_t m_iRowid = 0;
                Utf8String m_text;
                Utf8String m_delimiter = ",";
                bvector<int64_t> m_intId;

            public:
                TokenizeIdCursor(TokenizeIdTable& vt): ECDbCursor(vt){}
                bool Eof() final { return m_iRowid < 1 || m_iRowid > (int64_t)m_intId.size() ; }
                DbResult Next() final {
                    ++m_iRowid;
                    return BE_SQLITE_OK;
                }
                DbResult GetColumn(int i, Context& ctx) final {
                    Utf8CP x = 0;
                    switch( (Columns)i ){
                        case Columns::Text:
                            x = m_text.c_str(); 
                            ctx.SetResultText(x, (int)strlen(x), Context::CopyData::Yes);
                            break;
                        case Columns::Delimiter: 
                            x = m_delimiter.c_str(); 
                            ctx.SetResultText(x, (int)strlen(x), Context::CopyData::Yes);
                            break;
                        default: 
                            ctx.SetResultInt64(m_intId[m_iRowid - 1]);
                            break;
                    }  
                    return BE_SQLITE_OK;
                }
                DbResult GetRowId(int64_t& rowId) final {
                    rowId = m_iRowid;
                    return BE_SQLITE_OK;
                }
                DbResult Filter(int idxNum, const char *idxStr, int argc, DbValue* argv) final {
                    int i = 0;
                    bvector<Utf8String> tokens;

                    if( idxNum & 1 ){
                        m_text = argv[i++].GetValueText();
                    }else{
                        m_text = "";
                    }

                    tokens.clear();
                    BeJsDocument doc;
                    doc.Parse(m_text);
                    m_text.ReplaceAll("[","");
                    m_text.ReplaceAll("]","");
                    BeStringUtilities::ParseArguments(tokens,m_text.c_str(),m_delimiter.c_str());

                    for (int j = 0;j<tokens.size();j++) {
                        m_intId.push_back(BeStringUtilities::ParseHex(tokens[j].c_str()));
                    }

                    m_iRowid = 1;
                    return BE_SQLITE_OK;
                }
        };
        public:
            TokenizeIdTable(TokenizeIdModule& module): ECDbVirtualTable(module) {}
            DbResult Open(DbCursor*& cur) override {
                cur = new TokenizeIdCursor(*this);
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
                    if( pConstraint->GetColumn()< (int)TokenizeIdCursor::Columns::Text) continue;
                    iCol = pConstraint->GetColumn() - (int)TokenizeIdCursor::Columns::Text;
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
        TokenizeIdModule(ECDbR db): ECDbModule(
            db,
            "IdSet",
            "CREATE TABLE x(id,buffer hidden,delimiter hidden)",
            R"xml(<?xml version="1.0" encoding="utf-8" ?>
            <ECSchema
                    schemaName="test"
                    alias="test"
                    version="1.0.0"
                    xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="ECDbVirtual" version="01.00.00" alias="ecdbvirid" />
                <ECCustomAttributes>
                    <VirtualSchema xmlns="ECDbVirtual.01.00.00"/>
                </ECCustomAttributes>
                <ECEntityClass typeName="IdSet" modifier="Abstract">
                    <ECCustomAttributes>
                        <VirtualType xmlns="ECDbVirtual.01.00.00"/>
                    </ECCustomAttributes>
                    <ECProperty propertyName="id"  typeName="string"/>
                </ECEntityClass>
            </ECSchema>)xml") {}
        DbResult Connect(DbVirtualTable*& out, Config& conf, int argc, const char* const* argv) final {
            out = new TokenizeIdTable(*this);
            conf.SetTag(Config::Tags::Innocuous);
            return BE_SQLITE_OK;
        }
};

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbIdSetTests, TokenizeIdsTest) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("vtab1212.ecdb"));
    (new TokenizeIdModule(m_ecdb))->Register();
    // if ("unsorted") {
    //     ECSqlStatement stmt;
    //     ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, R"j(SELECT id FROM test.IdSet('["0x1", "0x2", "0x3"]'))j"));
    //     std::vector<std::int64_t> expected = std::vector<std::int64_t>{1,2,3};
    //     int i = 0;
    //     while(stmt.Step() == BE_SQLITE_ROW) {
    //         int64_t temp = stmt.GetValueInt64(0);
    //         ASSERT_EQ(expected[i++], temp);
    //     }
    //     ASSERT_EQ(i, 3);
    // }
    std::vector<std::int64_t> expected = std::vector<std::int64_t>{1,2,3};
    
    ECSqlStatement stmt;

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet(?)")));
    std::shared_ptr<IdSet<BeInt64Id>> ids = std::make_shared<IdSet<BeInt64Id>>();
    
    ids->insert((BeInt64Id)1);
    ids->insert((BeInt64Id)2);
    ids->insert((BeInt64Id)3);
    stmt.BindText(1, "[\"0x1\", \"0x2\", \"0x3\"]",IECSqlBinder::MakeCopy::No);
    
    // stmt.BindVirtualSet(1,ids);    
    
    // stmt.GetBinder(1);
    // IECSqlBinder& primArrayBinder = stmt.GetBinder(1);
    // primArrayBinder.BindNull();
    // for (int i = 0; i < expected.size(); i++)
    // {
    //     ASSERT_EQ(ECSqlStatus::Success,primArrayBinder.AddArrayElement().BindInt64(expected[i]));
    // }

    
    int i = 0;
    while(stmt.Step() == BE_SQLITE_ROW) {
        int64_t temp = stmt.GetValueInt64(0);
        ASSERT_EQ(expected[i++], temp);
    }
    ASSERT_EQ(i, 3);
    
}

END_ECDBUNITTESTS_NAMESPACE
