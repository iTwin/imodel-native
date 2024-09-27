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
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
// DbResult IdSetModule::IdSetTable::IdSetCursor::FilterRapidJSON(BeJsConst& val) {
//     if(val.isNull())
//     {
//         return BE_SQLITE_ERROR;
//     }
//     if(val.isNumeric())
//     {
//         if(val.asUInt64(-1) == -1)
//             return BE_SQLITE_ERROR;
//         else
//         {
//             uint64_t id = val.asUInt64(-1);
//             m_idSet.insert(id);
//         }
//     }
//     else if(val.isArray())
//     {
//         bool flag = true;
//         val.ForEachArrayMember([&](BeJsValue::ArrayIndex, BeJsConst k1)
//                                                 {
//                                                     if(BE_SQLITE_OK != FilterRapidJSON(k1))
//                                                         flag = true;
//                                                     return false; 
//                                                 });
//         if(flag)
//             return BE_SQLITE_ERROR;
//     }
//     else if(val.isString())
//     {
//         if(val.asUInt64(-1) == -1)
//         {
//             if(val.asString().EqualsIAscii(""))
//                 return BE_SQLITE_ERROR;
//             BeJsDocument doc;
//             doc.Parse(val.asString());
//             if(!doc.isArray())
//                 return BE_SQLITE_ERROR;
//             bool flag = false;
//             doc.ForEachArrayMember([&](BeJsValue::ArrayIndex, BeJsConst k1)
//                                                 {
//                                                     if(BE_SQLITE_OK != FilterRapidJSON(k1))
//                                                         flag = true;
//                                                     return false; 
//                                                 });
//             if(flag)
//                 return BE_SQLITE_ERROR;
//         }
//         else
//            m_idSet.insert(val.asUInt64(-1));
//     }
//     // switch(val.GetType())
//     // {
//     //     case rapidjson::Type::kArrayType:
//     //         {
//     //             for(rapidjson::Value& v: val.GetArray())
//     //             {
//     //                 DbResult res = FilterRapidJSON(v);
//     //                 if(res != BE_SQLITE_OK)
//     //                     return res;
//     //             }
//     //             break;
//     //         }
//     //     case rapidjson::Type::kNumberType:
//     //             {
//     //                 if(!val.IsUint64())
//     //                     return BE_SQLITE_ERROR;
//     //                 int64_t idVal = val.GetUint64();
//     //                 m_idSet.insert(idVal);
//     //                 break;
//     //             }
//     //     case rapidjson::Type::kStringType:
//     //         {
//     //             BeJsDocument d;
//     //             d.Parse(val.GetString());
//     //             d.Stringify(StringifyFormat::Indented);
//     //             bool flag = false;
//     //             if(d.isArray())
//     //             {
//     //                 d.ForEachArrayMember([&](BeJsValue::ArrayIndex, BeJsConst k1)
//     //                                             {
//     //                                                 if(d.asUInt64(-1) == -1)
//     //                                                     flag = true;
//     //                                                 else
//     //                                                     m_idSet.insert(k1.asUInt64());
//     //                                                 return false; 
//     //                                             });
//     //                 if(flag)
//     //                     return BE_SQLITE_ERROR;
//     //             }
//     //             else if(d.isString())
//     //             {
//     //                 if(d.asUInt64(-1) == -1)
//     //                     return BE_SQLITE_ERROR;
//     //                 else
//     //                     m_idSet.insert(d.asUInt64());
//     //             }
//     //             else
//     //                 return BE_SQLITE_ERROR;
//     //             break;
//     //         }
//     //     case rapidjson::Type::kNullType:
//     //         {
//     //             break;
//     //         }
//     //     default:
//     //         // We don't allow any other types here
//     //         return BE_SQLITE_ERROR;
//     // }
//     return BE_SQLITE_OK;
// }
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
                        //     BeJsDocument doc;
                        //     doc.Parse(m_text.c_str());
                        //     doc.ForEachArrayMember([&](BeJsValue::ArrayIndex, BeJsConst k1)
                        //                         {
                        //         m_idSet.insert(BeInt64Id(k1.asUInt64()));
                        //         return false; });
                        // }
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
        ECSqlStatement stmt;
        // ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT x, id FROM (with cte(x) as(select ECInstanceId from meta.ECClassDef) select x from cte),test.IdSet('[1,2,3,4,5]')")));
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT x FROM test.IdSet('[1,2,3,4,5]'), (with cte(x) as(select ECInstanceId from meta.ECClassDef) select x from cte) where id < x group by x")));
        int i = 2;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((i++), stmt.GetValueInt64(0));
        }
        ASSERT_EQ(i, 88);
    }
    //  {
    //     ECSqlStatement stmt;
    //     ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT ECClassId, id FROM meta.ECClassDef,test.IdSet('[1,2,3,4,5]')")));
    //     // ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet('[1,2,3,4,5]')")));
    //     int i = 0;
    //     while (stmt.Step() == BE_SQLITE_ROW)
    //     {
    //         ASSERT_EQ((1+i++), stmt.GetValueInt64(0));
    //     }
    //     ASSERT_EQ(i, 5);
    // }
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
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet('[1,2,3,4,5]') where id = 1")));

        int i = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((1+i++), stmt.GetValueInt64(0));
        }
        ASSERT_EQ(i, 1);
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet('[1,2,3,4,5]') where id > 1")));

        int i = 1;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((1+i++), stmt.GetValueInt64(0));
        }
        ASSERT_EQ(i, 5);
    }
    // {
    //     std::vector<Utf8String> hexIds = std::vector<Utf8String>{"0x1", "0x2", "0x3", "4", "5"};

    //     ECSqlStatement stmt;
    //     ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet(?)")));
    //     IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    //     for(int i =0;i<hexIds.size();i++)
    //     {
    //         IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
    //         ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindText(hexIds[i].c_str(), IECSqlBinder::MakeCopy::No));
    //     }
    //     int i = 0;
    //     while (stmt.Step() == BE_SQLITE_ROW)
    //     {
    //         ASSERT_EQ(BeStringUtilities::ParseHex(hexIds[i++].c_str()), stmt.GetValueInt64(0));
    //     }
    //     ASSERT_EQ(i, hexIds.size());
    // }
    // {
    //     ECSqlStatement stmt;
    //     ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet('[\"0x1\",\"0x2\",3,4,5]')")));
    
    //     int i = 0;
    //     while (stmt.Step() == BE_SQLITE_ROW)
    //     {
    //         ASSERT_EQ((1+i), stmt.GetValueInt64(0));
    //         i++;
    //     }
    //     ASSERT_EQ(i, 5);
    // }
    // {
    //     ECSqlStatement stmt;
    //     ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet('[0x1,0x2,\"3\",\"4\",\"5\"]')")));

    //     // Should fail while converting to json array because hex values with quotes are required so should be empty table
    //     ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    // }
    // {
    //     ECSqlStatement stmt;
    //     ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet('[1,\"2\",3, 4.0, 5.0]')")));
    //     int i = 0;
    //     while (stmt.Step() == BE_SQLITE_ROW)
    //     {
    //         ASSERT_EQ((1+i), stmt.GetValueInt64(0));
    //         i++;
    //     }
    //     ASSERT_EQ(i, 5);
    // }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet('[1,\"2\",3, 4.5, 5.6]')")));
        
        // Will not take into account 4.5 and 5.6 because they are decimal values so should be empty table
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    // {
    //     ECSqlStatement stmt;
    //     ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet(?)")));
    //     ASSERT_EQ(ECSqlStatus::Error,stmt.BindText(1, "[1,\"2\",3, \"abc\"]", IECSqlBinder::MakeCopy::No));

    //     // no binding as we use array ecsql binder so need to call AddArrayElement first
    //     ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    // }
    // {
    //     ECSqlStatement stmt;
    //     ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet(?)")));

    //     // no binding so no data
    //     ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    // }
    // {
    //     ECSqlStatement stmt;
    //     ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet(?)")));
    //     IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    //     IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
    //     ASSERT_EQ(ECSqlStatus::Error, elementBinder.BindText( "[1,\"2\",3, \"abc\"]", IECSqlBinder::MakeCopy::No));

    //     // EmptyArray is Binded so should be empty table
    //     ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    // }
    // {
    //     ECSqlStatement stmt;
    //     ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet(?)")));
    //     IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    //     for(int i = 1;i<=10;i++)
    //     {
    //         IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
    //         ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindInt(i));
    //     }

    //     int i = 0;
    //     while (stmt.Step() == BE_SQLITE_ROW)
    //     {
    //         ASSERT_EQ((1+i), stmt.GetValueInt64(0));
    //         i++;
    //     }
    //     ASSERT_EQ(i, 10);
    // }
    // {
    //     ECSqlStatement stmt;
    //     ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet(?)")));
    //     IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    //     for(int i = 1;i<=10;i++)
    //     {
    //         IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
    //         ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindDouble(i));
    //     }

    //     int i = 0;
    //     while (stmt.Step() == BE_SQLITE_ROW)
    //     {
    //         ASSERT_EQ((1+i), stmt.GetValueInt64(0));
    //         i++;
    //     }
    //     ASSERT_EQ(i, 10);
    // }
    // {
    //     ECSqlStatement stmt;
    //     ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet(?)")));
    //     IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    //     IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
    //     ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindNull());
    //     for(int i = 1;i<=10;i++)
    //     {
    //         IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
    //         ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindDouble(i));
    //     }

    //     // having null as an element so should be empty table
    //     ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    // }
    // {
    //     DPoint2d pArrayOfST1_P2D[] = {DPoint2d::From(-21, 22.1),DPoint2d::From(-85.34, 35.36),DPoint2d::From(-31.34, 12.35)};
    //     DPoint3d pArrayOfST1_P3D[] = {DPoint3d::From(-12.11, -74.1, 12.3),DPoint3d::From(-12.53, 21.76, -32.22),DPoint3d::From(-41.14, -22.45, -31.16)};
    //     ECSqlStatement stmt;
    //     ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet(?)")));
    //     IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    //     for(int i = 0;i<=2;i++)
    //     {
    //         IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
    //         ASSERT_EQ(ECSqlStatus::Error, elementBinder.BindPoint2d(pArrayOfST1_P2D[i]));
    //     }
    //     for(int i = 0;i<=2;i++)
    //     {
    //         IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
    //         ASSERT_EQ(ECSqlStatus::Error, elementBinder.BindPoint3d(pArrayOfST1_P3D[i]));
    //     }

    //     // EmptyArray is Binded so should be empty table
    //     ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    // }
    // {
    //     DPoint2d pArrayOfST1_P2D[] = {DPoint2d::From(-21, 22.1),DPoint2d::From(-85.34, 35.36),DPoint2d::From(-31.34, 12.35)};
    //     DPoint3d pArrayOfST1_P3D[] = {DPoint3d::From(-12.11, -74.1, 12.3),DPoint3d::From(-12.53, 21.76, -32.22),DPoint3d::From(-41.14, -22.45, -31.16)};
    //     double pST1P_ST2P_D2 = 431231.3432;
    //     ECSqlStatement stmt;
    //     ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet(?)")));
    //     IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    //     for(int i = 0;i<=2;i++)
    //     {
    //         IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
    //         ASSERT_EQ(ECSqlStatus::Error, elementBinder["D1"].BindDouble(pST1P_ST2P_D2));
    //         ASSERT_EQ(ECSqlStatus::Error, elementBinder["P2D"].BindPoint2d(pArrayOfST1_P2D[i]));
    //     }
    //     for(int i = 0;i<=2;i++)
    //     {
    //         IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
    //         ASSERT_EQ(ECSqlStatus::Error, elementBinder["D1"].BindDouble(pST1P_ST2P_D2));
    //         ASSERT_EQ(ECSqlStatus::Error, elementBinder["P3D"].BindPoint3d(pArrayOfST1_P3D[i]));
    //     }

    //     // EmptyArray is Binded so should be empty table
    //     ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    // }
    // {
    //     const std::vector<std::vector<uint8_t>> bi_array = {
    //         {0x48, 0x65, 0x6},
    //         {0x48, 0x65, 0x6},
    //         {0x48, 0x65, 0x6}
    //     };
    //     ECSqlStatement stmt;
    //     ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet(?)")));
    //     IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    //     for(auto& m : bi_array)
    //         ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindBlob((void const*)&m[0], (int)m.size(), IECSqlBinder::MakeCopy::No));

    //     // Binary is Binded so should be empty table
    //     ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    // }
    // {
    //     const auto dt = DateTime(DateTime::Kind::Unspecified, 2017, 1, 17, 0, 0);
    //     const auto dtUtc = DateTime(DateTime::Kind::Utc, 2018, 2, 17, 0, 0);
    //     ECSqlStatement stmt;
    //     ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet(?)")));
    //     IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    //     for(int i = 0;i<=1;i++)
    //     {
    //         ASSERT_EQ(ECSqlStatus::Error, arrayBinder.AddArrayElement().BindDateTime(dt));
    //         ASSERT_EQ(ECSqlStatus::Error, arrayBinder.AddArrayElement().BindDateTime(dtUtc));
    //     }

    //     // EmptyArray is Binded so should be empty table
    //     ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    // }
    // {
    //     auto geom = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));
    //     ECSqlStatement stmt;
    //     ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet(?)")));
    //     IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    //     for(int i = 0;i<=1;i++)
    //     {
    //         ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindGeometry(*geom));
    //     }

    //     // Binary is Binded because BindGeometry internally calls so should be empty table 
    //     ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    // }
    // {
    //     ECSqlStatement stmt;
    //     ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet(?)")));
    //     IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    //     for(int i = 0;i<=1;i++)
    //     {
    //         ASSERT_EQ(ECSqlStatus::Error, arrayBinder.AddArrayElement().BindText("ABC",IECSqlBinder::MakeCopy::No));
    //     }

    //     // EmptyArray is Binded so should be empty table
    //     ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    // }
    // {
    //     ECSqlStatement stmt;
    //     ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet(?)")));
    //     IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    //     for(int i = 0;i<=1;i++)
    //     {
    //         ASSERT_EQ(ECSqlStatus::Error, arrayBinder.AddArrayElement().BindText("[abc]",IECSqlBinder::MakeCopy::No));
    //     }

    //     // EmptyArray is Binded so should be empty table
    //     ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    // }
    // {
    //     ECSqlStatement stmt;
    //     ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT id FROM test.IdSet(?)")));
    //     IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    //     for(int i = 0;i<=1;i++)
    //     {
    //         ASSERT_EQ(ECSqlStatus::Error, arrayBinder.AddArrayElement().BindText("[\"abc\"]",IECSqlBinder::MakeCopy::No));
    //     }

    //     // EmptyArray is Binded so should be empty table
    //     ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    // }
}


END_ECDBUNITTESTS_NAMESPACE
