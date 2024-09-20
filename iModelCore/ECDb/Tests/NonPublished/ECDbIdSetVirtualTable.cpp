/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "iostream"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
struct ECDbIdSetVirtualTableTests : ECDbTestFixture {};
//=======================================================================================
//! Virtual Table to tokenize string
// @bsiclass
//=======================================================================================
// struct IdSetModule : ECDbModule {
//     struct IdSetTable : ECDbVirtualTable {
//         struct IdSetCursor : ECDbCursor {
            
//             private:
//                 Utf8String m_text;
//                 IdSet<BeInt64Id>* virtualSetPtr = nullptr;
//                 bset<BeInt64Id> m_idSet;
//                 bset<BeInt64Id>::iterator m_index;

//             public:
//                 IdSetCursor(IdSetTable& vt): ECDbCursor(vt){}
//                 bool Eof() final { return m_index == m_idSet.end() ; }
//                 DbResult Next() final {
//                     ++m_index;
//                     return BE_SQLITE_OK;
//                 }
//                 DbResult GetColumn(int i, Context& ctx) final {
//                     ctx.SetResultInt64((*m_index).GetValue());
//                     return BE_SQLITE_OK;
//                 }
//                 DbResult GetRowId(int64_t& rowId) final {
//                     rowId = (*m_index).GetValue();
//                     return BE_SQLITE_OK;
//                 }
//                 DbResult Filter(int idxNum, const char *idxStr, int argc, DbValue* argv) final {
//                     int i = 0;
//                     int flag = false;
//                     if( idxNum & 1 ){
//                         if(argv[i].GetValueType() == DbValueType::TextVal)
//                             m_text = argv[i++].GetValueText();
//                         else
//                             virtualSetPtr = (IdSet<BeInt64Id>*)argv[i++].GetValuePointer("ID_SET_NAME");
//                         flag = true;
//                     }else{
//                         m_text = "";
//                     }
//                     if(flag )
//                     {
//                         if(m_text.size() > 0)
//                         {
//                             // Parse String to Js Document and iterate through the array and insert int hex ids as int64 in uniqueIds set.
//                             BeJsDocument doc;
//                             doc.Parse(m_text.c_str());
//                             doc.ForEachArrayMember([&](BeJsValue::ArrayIndex, BeJsConst k1)
//                                                 {
//                                 m_idSet.insert(BeInt64Id(k1.asUInt64()));
//                                 return false; });
//                         }
//                         else if(virtualSetPtr != nullptr)
//                         {
//                             IdSet<BeInt64Id> virtualSet = *virtualSetPtr;
//                             for(auto it = virtualSet.begin();it!=virtualSet.end();it++)
//                             {
//                                 m_idSet.insert(*it);
//                             }
//                         }
//                         else
//                         {
//                             return BE_SQLITE_ERROR;
//                         }
//                     }
//                     std::cout << m_idSet.size() << std::endl;  
//                     m_index = m_idSet.begin();
//                     return BE_SQLITE_OK;
//                 }
//         };
//         public:
//             IdSetTable(IdSetModule& module): ECDbVirtualTable(module) {}
//             DbResult Open(DbCursor*& cur) override {
//                 cur = new IdSetCursor(*this);
//                 return BE_SQLITE_OK;
//             }
//              DbResult BestIndex(IndexInfo& indexInfo) final {
//                  int i, j;              /* Loop over constraints */
//                 int idxNum = 0;        /* The query plan bitmask */
//                 int unusableMask = 0;  /* Mask of unusable constraints */
//                 int nArg = 0;          /* Number of arguments that seriesFilter() expects */
//                 int aIdx[2];           /* Constraints on start, stop, and step */
//                 const int SQLITE_SERIES_CONSTRAINT_VERIFY = 0;
//                 aIdx[0] = aIdx[1] = -1;
//                 int nConstraint = indexInfo.GetConstraintCount();

//                 for(i=0; i<nConstraint; i++){
//                     auto pConstraint = indexInfo.GetConstraint(i);
//                     int iCol;    /* 0 for start, 1 for stop, 2 for step */
//                     int iMask;   /* bitmask for those column */
//                     if( pConstraint->GetColumn()< 0) continue;
//                     iCol = pConstraint->GetColumn();
//                     iMask = 1 << iCol;
//                     if (!pConstraint->IsUsable()){
//                         unusableMask |=  iMask;
//                         continue;
//                     } else if (pConstraint->GetOp() == IndexInfo::Operator::EQ ){
//                         idxNum |= iMask;
//                         aIdx[iCol] = i;
//                     }
//                 }
//                 for( i = 0; i < 2; i++) {
//                     if( (j = aIdx[i]) >= 0 ) {
//                         indexInfo.GetConstraintUsage(j)->SetArgvIndex(++nArg);
//                         indexInfo.GetConstraintUsage(j)->SetOmit(!SQLITE_SERIES_CONSTRAINT_VERIFY);
//                     }
//                 }

//                 if ((unusableMask & ~idxNum)!=0 ){
//                     return BE_SQLITE_CONSTRAINT;
//                 }

//                 indexInfo.SetEstimatedCost(2.0);
//                 indexInfo.SetEstimatedRows(1000);
//                 if( indexInfo.GetIndexOrderByCount() >= 1 && indexInfo.GetOrderBy(0)->GetColumn() == 0 ) {
//                     if( indexInfo.GetOrderBy(0) ->GetDesc()){
//                         idxNum |= 8;
//                     } else {
//                         idxNum |= 16;
//                     }
//                     indexInfo.SetOrderByConsumed(true);
//                 }
//                 indexInfo.SetIdxNum(idxNum);
//                 return BE_SQLITE_OK;
//              }
//     };
//     public:
//         IdSetModule(ECDbR db): ECDbModule(
//             db,
//             "IdSet",
//             "CREATE TABLE x(id hidden)",
//             R"xml(<?xml version="1.0" encoding="utf-8" ?>
//             <ECSchema
//                     schemaName="test"
//                     alias="test"
//                     version="1.0.0"
//                     xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
//                 <ECSchemaReference name="ECDbVirtual" version="01.00.00" alias="ecdbvir" />
//                 <ECCustomAttributes>
//                     <VirtualSchema xmlns="ECDbVirtual.01.00.00"/>
//                 </ECCustomAttributes>
//                 <ECEntityClass typeName="IdSet" modifier="Abstract">
//                     <ECCustomAttributes>
//                         <VirtualType xmlns="ECDbVirtual.01.00.00"/>
//                     </ECCustomAttributes>
//                     <ECProperty propertyName="id"  typeName="int"/>
//                 </ECEntityClass>
//             </ECSchema>)xml") {}
//         DbResult Connect(DbVirtualTable*& out, Config& conf, int argc, const char* const* argv) final {
//             out = new IdSetTable(*this);
//             conf.SetTag(Config::Tags::Innocuous);
//             return BE_SQLITE_OK;
//         }
// };

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbIdSetVirtualTableTests, IdSetModuleTest) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("vtab.ecdb"));
    {
        std::vector<Utf8String> hexIds = std::vector<Utf8String>{"0x1", "0x2", "0x3", "4", "5"};
        Utf8String jsonArrayString = "[";
        int k = 0;
        for (k; k < hexIds.size() - 1; k++)
        {
            jsonArrayString += ("\"" + hexIds[k] + "\", ");
        }
        jsonArrayString += ("\"" + hexIds[k] + "\"]");

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet(?)")));
        stmt.BindText(1, jsonArrayString.c_str(), IECSqlBinder::MakeCopy::No);

        int i = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ(BeStringUtilities::ParseHex(hexIds[i++].c_str()), stmt.GetValueInt64(0));
        }
        ASSERT_EQ(i, hexIds.size());
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet(?)")));
        stmt.BindText(1, "[1,2,3,4,5]", IECSqlBinder::MakeCopy::No);

        int i = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((1+i++), stmt.GetValueInt64(0));
        }
        ASSERT_EQ(i, 5);
    }
    {
        bvector<BeInt64Id> v;
        v.push_back(BeInt64Id(1)); 
        std::shared_ptr<IdSet<BeInt64Id>> seedIdSet = std::make_shared<IdSet<BeInt64Id>>();
        for(auto id: v){
            seedIdSet->insert(id);
        }
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet(?)")));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindVirtualSet(1, seedIdSet));
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ(1, stmt.GetValueInt64(0));
        } 
    }
    {
        bvector<BeInt64Id> v;
        v.push_back(BeInt64Id(1)); 
        std::shared_ptr<ECInstanceIdSet> seedIdSet = std::make_shared<ECInstanceIdSet>();
        for(auto id: v){
            seedIdSet->insert(ECInstanceId(id));
        }
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet(?)")));
        ASSERT_EQ(ECSqlStatus::Error, stmt.BindVirtualSet(1, seedIdSet));
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet('[1,2,3,4,5]')")));

        int i = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((1+i++), stmt.GetValueInt64(0));
        }
        ASSERT_EQ(i, 5);
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet('[\"0x1\",\"0x2\",3,4,5]')")));
    
        int i = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((1+i), stmt.GetValueInt64(0));
            i++;
        }
        ASSERT_EQ(i, 5);
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet('[0x1,0x2,\"3\",\"4\",\"5\"]')")));

        // Should fail while converting to json array because hex values with quotes are required
        int i = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((1+i), stmt.GetValueInt64(0));
            i++;
        }
        ASSERT_EQ(i, 0);
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet(?)")));
        stmt.BindText(1, "[1,\"2\",3, 4.0, 5.0]", IECSqlBinder::MakeCopy::No);

        int i = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((1+i), stmt.GetValueInt64(0));
            i++;
        }
        ASSERT_EQ(i, 5);
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet(?)")));
        stmt.BindText(1, "[1,\"2\",3, 4.5, 5.6]", IECSqlBinder::MakeCopy::No);

        // Will not take into account 4.5 and 5.6 because they are decimal values
        int i = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((1+i), stmt.GetValueInt64(0));
            i++;
        }
        ASSERT_EQ(i, 3);
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet(?)")));
        stmt.BindText(1, "[1,\"2\",3, \"Soham\"]", IECSqlBinder::MakeCopy::No);

        // Will not take into account "Soham" because it is a string
        int i = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((1+i), stmt.GetValueInt64(0));
            i++;
        }
        ASSERT_EQ(i, 3);
    }
}


END_ECDBUNITTESTS_NAMESPACE
