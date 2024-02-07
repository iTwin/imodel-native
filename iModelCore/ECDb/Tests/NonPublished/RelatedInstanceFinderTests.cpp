/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <sstream>
#include <rapidjson/ostreamwrapper.h>
#include <random>
#include <filesystem>
#include <ECDb/RelatedInstanceFinder.h>

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE
struct RelatedInstanceFinderFixture : ECDbTestFixture {};


//=======================================================================================
//! Virtual Table to tokenize string
// @bsiclass
//=======================================================================================
// struct RelatedInstanceModule : ECDbModule {
//     struct RelatedInstanceTable : ECDbVirtualTable {
//         struct RelatedInstanceCursor : ECDbCursor {
//             enum class Columns{
//                 toId = 0,
//                 toClassId = 1,
//                 fromId = 2,
//                 fromClassId = 3,
//                 relClassId = 4,
//                 direction =5 ,
//                 id = 6,
//                 classId =7,
//                 dirFilter =8,
//             };
//             private:
//                 int64_t m_iRowid = 0;
//                 ECInstanceId m_id;
//                 ECClassId m_classId;
//                 RelatedInstanceFinder::DirectionFilter m_dirFilter;
//                 Utf8String m_delimiter;
//                 RelatedInstanceFinder::Results m_results;

//             public:
//                 RelatedInstanceCursor(RelatedInstanceTable& vt): ECDbCursor(vt){}
//                 bool Eof() final { return m_iRowid < 1 || m_iRowid > (int64_t)m_results.size() ; }
//                 DbResult Next() final {
//                     ++m_iRowid;
//                     return BE_SQLITE_OK;
//                 }
//                 DbResult GetColumn(int i, Context& ctx) final {

//                     switch( (Columns)i ) {
//                         case Columns::toId:
//                             ctx.SetResultInt64(m_results[m_iRowid].GetTo().GetInstanceId().GetValueUnchecked());
//                             return BE_SQLITE_OK;
//                         case Columns::toClassId:
//                             ctx.SetResultInt64(m_results[m_iRowid].GetTo().GetClassId().GetValueUnchecked());
//                             return BE_SQLITE_OK;
//                         case Columns::fromId:
//                             ctx.SetResultInt64(m_results[m_iRowid].GetFrom().GetInstanceId().GetValueUnchecked());
//                             return BE_SQLITE_OK;
//                         case Columns::fromClassId:
//                             ctx.SetResultInt64(m_results[m_iRowid].GetFrom().GetClassId().GetValueUnchecked());
//                             return BE_SQLITE_OK;
//                         case Columns::relClassId:
//                             ctx.SetResultInt64(m_results[m_iRowid].GetRelationshipClassId().GetValueUnchecked());
//                             return BE_SQLITE_OK;
//                         case Columns::direction:
//                             ctx.SetResultInt((int)m_results[m_iRowid].GetDirection());
//                             return BE_SQLITE_OK;
//                     }
//                     return BE_SQLITE_ERROR;
//                 }
//                 DbResult GetRowId(int64_t& rowId) final {
//                     rowId = m_iRowid;
//                     return BE_SQLITE_OK;
//                 }
//                 DbResult Filter(int idxNum, const char *idxStr, int argc, DbValue* argv) final {
//                     int i = 0;
//                     if( idxNum & 1 ){
//                         m_id = argv[i++].GetValueId<ECInstanceId>();
//                     }else{
//                         m_id = ECInstanceId();
//                     }
//                     if( idxNum & 2 ){
//                         m_classId = argv[i++].GetValueId<ECClassId>();
//                     }else{
//                         m_classId = ECClassId();
//                     }
//                     if( idxNum & 3 ){
//                         Utf8String x = argv[i++].GetValueText();
//                         if (x.EqualsIAscii("forward") || x.EqualsIAscii("f"))
//                             m_dirFilter = RelatedInstanceFinder::DirectionFilter::Forward;
//                         else if (x.EqualsIAscii("backward") || x.EqualsIAscii("b"))
//                             m_dirFilter = RelatedInstanceFinder::DirectionFilter::Backward;
//                         else if (x.EqualsIAscii("both") || x.EqualsIAscii("a"))
//                             m_dirFilter = RelatedInstanceFinder::DirectionFilter::Both;
//                         else
//                             m_dirFilter = RelatedInstanceFinder::DirectionFilter::Both;
//                     } else {
//                         m_dirFilter = RelatedInstanceFinder::DirectionFilter::Both;
//                     }
//                     RelatedInstanceFinder finder(reinterpret_cast<ECDbCR>(GetTable().GetModule().GetDb()));
//                     m_results = finder.FindAll(ECInstanceKey(m_classId, m_id), m_dirFilter);
//                     m_iRowid = 1;
//                     return BE_SQLITE_OK;
//                 }
//         };
//         public:
//             RelatedInstanceTable(RelatedInstanceModule& module): ECDbVirtualTable(module) {}
//             DbResult Open(DbCursor*& cur) override {
//                 cur = new RelatedInstanceCursor(*this);
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
//                     if( pConstraint->GetColumn()< (int)RelatedInstanceCursor::Columns::id) continue;
//                     iCol = pConstraint->GetColumn() - (int)RelatedInstanceCursor::Columns::id;
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
//                 indexInfo.SetIdxNum(idxNum);
//                 return BE_SQLITE_OK;
//              }
//     };
//     public:
//         RelatedInstanceModule(ECDbR db): ECDbModule(
//             db,
//             "related_instances",
//             "CREATE TABLE x(fromId, fromClassId, toId, toClassId, relClassId, direction,id hidden,classId hidden)",
//             R"xml(<?xml version="1.0" encoding="utf-8" ?>
//             <ECSchema
//                     schemaName="rel1"
//                     alias="rel1"
//                     version="1.0.0"
//                     xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
//                 <ECSchemaReference name="ECDbVirtual" version="01.00.00" alias="ecdbvir" />
//                 <ECCustomAttributes>
//                     <VirtualSchema xmlns="ECDbVirtual.01.00.00"/>
//                 </ECCustomAttributes>
//                 <ECEntityClass typeName="related_instances" modifier="Abstract">
//                     <ECCustomAttributes>
//                         <VirtualType xmlns="ECDbVirtual.01.00.00"/>
//                     </ECCustomAttributes>
//                     <ECProperty propertyName="fromId"  typeName="long"/>
//                     <ECProperty propertyName="fromClassId"  typeName="long"/>
//                     <ECProperty propertyName="toId"  typeName="long"/>
//                     <ECProperty propertyName="toClassId"  typeName="long"/>
//                     <ECProperty propertyName="relClassId"  typeName="long"/>
//                     <ECProperty propertyName="direction"  typeName="integer"/>
//                 </ECEntityClass>
//             </ECSchema>)xml") {}
//         DbResult Connect(DbVirtualTable*& out, Config& conf, int argc, const char* const* argv) final {
//             out = new RelatedInstanceTable(*this);
//             conf.SetTag(Config::Tags::Innocuous);
//             return BE_SQLITE_OK;
//         }
// };
SchemaItem GetTestSchema () {
    return SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="2.0" alias="ecdbmap"/>
            <ECEntityClass typeName="Element">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.2.0">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <ECNavigationProperty propertyName="Parent" relationshipName="ElementOwnsChildElements" direction="Backward">
                    <ECCustomAttributes>
                        <ForeignKeyConstraint xmlns="ECDbMap.2.0"/>
                    </ECCustomAttributes>
                </ECNavigationProperty>
            </ECEntityClass>
            <ECRelationshipClass typeName="ElementOwnsChildElements" strength="embedding" modifier="None">
                <Source multiplicity="(0..1)" polymorphic="true" roleLabel="a">
                    <Class class="Element"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="true" roleLabel="a">
                    <Class class="Element"/>
                </Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="ElementRefersToElements" strength="referencing" modifier="None">
                <ECCustomAttributes>
                    <LinkTableRelationshipMap xmlns="ECDbMap.2.0">
                        <CreateForeignKeyConstraints>False</CreateForeignKeyConstraints>
                    </LinkTableRelationshipMap>
                    <ClassMap xmlns="ECDbMap.2.0">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <Source multiplicity="(0..*)" polymorphic="true" roleLabel="a">
                    <Class class="Element"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="true" roleLabel="a">
                    <Class class="Element"/>
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml");
}

ECInstanceKey InsertElement(ECDbCR ecdb){
    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.Element(ECInstanceId) VALUES(NULL)"));

    ECInstanceKey key;
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    return key;
}

void SetElementParent(ECDbCR ecdb, ECInstanceKey el, ECInstanceKey parent) {
    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "UPDATE ts.Element SET Parent = ? WHERE ECInstanceId = ?"));
    stmt.BindNavigationValue(1, parent.GetInstanceId());
    stmt.BindId(2, el.GetInstanceId());
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step());
}

ECInstanceKey InsertElementRefersToElements(ECDbCR ecdb, ECInstanceKey source, ECInstanceKey target) {
    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.ElementRefersToElements(SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(?, ?, ?, ? )"));
    stmt.BindId(1, source.GetInstanceId());
    stmt.BindId(2, source.GetClassId());
    stmt.BindId(3, target.GetInstanceId());
    stmt.BindId(4, target.GetClassId());

    ECInstanceKey key;
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    return key;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelatedInstanceFinderFixture, Basic) {
    ASSERT_EQ(SUCCESS, SetupECDb("relatedInstanceFinder.ecdb", GetTestSchema()));

    auto& db = m_ecdb;
    const auto e1 = InsertElement(db);
    const auto e2 = InsertElement(db);

    SetElementParent(db, e1, e2);
    InsertElementRefersToElements(db, e2, e1);
    InsertElementRefersToElements(db, e1, e2);
    InsertElementRefersToElements(db, e1, e1);
    InsertElementRefersToElements(db, e2, e2);
    db.SaveChanges();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "SELECT * FROM rel1.related_instances(?,?, 'forward')"));
    stmt.BindId(1, e1.GetInstanceId());
    stmt.BindId(2, e1.GetClassId());
    stmt.Step();

    auto& finder = db.GetRelatedInstanceFinder();
    const auto e1_forward = finder.FindAll(e1, RelatedInstanceFinder::DirectionFilter::Forward);
    const auto e1_backward = finder.FindAll(e1, RelatedInstanceFinder::DirectionFilter::Backward);
    const auto e1_both= finder.FindAll(e1, RelatedInstanceFinder::DirectionFilter::Both);
    const auto e2_forward = finder.FindAll(e2, RelatedInstanceFinder::DirectionFilter::Forward);
    const auto e2_backward = finder.FindAll(e2, RelatedInstanceFinder::DirectionFilter::Backward);
    const auto e2_both= finder.FindAll(e2, RelatedInstanceFinder::DirectionFilter::Both);

    ASSERT_EQ(e1_forward.size(), 3);
    ASSERT_EQ(e1_backward.size(), 2);
    ASSERT_EQ(e1_both.size(), 5);
    ASSERT_EQ(e2_forward.size(), 2);
    ASSERT_EQ(e2_backward.size(), 3);
    ASSERT_EQ(e2_both.size(), 5);
}

END_ECDBUNITTESTS_NAMESPACE
