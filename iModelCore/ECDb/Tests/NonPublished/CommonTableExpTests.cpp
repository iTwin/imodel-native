/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "NestedStructArrayTestSchemaHelper.h"
#include <Bentley/Base64Utilities.h>
#include <cmath>
#include <algorithm>
#include <set>
#include <BeRapidJson/BeRapidJson.h>
#include "iostream"

#define CLASS_ID(S,C) (int)m_ecdb.Schemas().GetClassId( #S, #C, SchemaLookupMode::AutoDetect).GetValueUnchecked()

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

struct CommonTableExpTestFixture : ECDbTestFixture {};
//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CommonTableExpTestFixture, PureRowConstructorQuery) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("cte_syntax.ecdb"));


    if (true) {
    /* Test if none class base queries work
     *
     */
    auto query = R"(
        VALUES (11, 12)
        UNION
        VALUES (22, 33)
        UNION
        SELECT  45, 55
        UNION
        SELECT  66, 77
    )";

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_EQ(11, stmt.GetValueInt(0)); ASSERT_EQ(12, stmt.GetValueInt(1));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_EQ(22, stmt.GetValueInt(0)); ASSERT_EQ(33, stmt.GetValueInt(1));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_EQ(45, stmt.GetValueInt(0)); ASSERT_EQ(55, stmt.GetValueInt(1));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_EQ(66, stmt.GetValueInt(0)); ASSERT_EQ(77, stmt.GetValueInt(1));
    }
}
//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CommonTableExpTestFixture, MetaQuery) {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("cte_test_meta.ecdb", SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
    <ECSchema schemaName='TestSchema' alias='ts' version='10.10.10' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
        <ECEntityClass typeName='A' >
            <ECProperty propertyName="a_prop" typeName="string" />
        </ECEntityClass>
        <ECEntityClass typeName='AA' >
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="aa_prop" typeName="string" />
        </ECEntityClass>
        <ECEntityClass typeName='AB' >
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="ab_prop" typeName="string" />
        </ECEntityClass>
        <ECEntityClass typeName='AAA' >
            <BaseClass>AA</BaseClass>
            <ECProperty propertyName="aaa_prop" typeName="string" />
        </ECEntityClass>
        <ECEntityClass typeName='AAB' >
            <BaseClass>AA</BaseClass>
            <ECProperty propertyName="aab_prop" typeName="string" />
        </ECEntityClass>
        <ECEntityClass typeName='ABA' >
            <BaseClass>AB</BaseClass>
            <ECProperty propertyName="aba_prop" typeName="string" />
        </ECEntityClass>
        <ECEntityClass typeName='ABB' >
            <BaseClass>AB</BaseClass>
            <ECProperty propertyName="abb_prop" typeName="string" />
        </ECEntityClass>
        <ECEntityClass typeName='B' >
            <ECProperty propertyName="b_prop" typeName="string" />
        </ECEntityClass>
        <ECEntityClass typeName='BA' >
            <BaseClass>B</BaseClass>
            <ECProperty propertyName="ba_prop" typeName="string" />
        </ECEntityClass>
        <ECEntityClass typeName='BB' >
            <BaseClass>B</BaseClass>
            <ECProperty propertyName="bb_prop" typeName="string" />
        </ECEntityClass>
        <ECEntityClass typeName='BAA' >
            <BaseClass>BA</BaseClass>
            <ECProperty propertyName="baa_prop" typeName="string" />
        </ECEntityClass>
        <ECEntityClass typeName='BAB' >
            <BaseClass>BA</BaseClass>
            <ECProperty propertyName="bab_prop" typeName="string" />
        </ECEntityClass>
        <ECEntityClass typeName='BBA' >
            <BaseClass>BB</BaseClass>
            <ECProperty propertyName="bba_prop" typeName="string" />
        </ECEntityClass>
        <ECEntityClass typeName='BBB' >
        <BaseClass>BB</BaseClass>
            <ECProperty propertyName="bbb_prop" typeName="string" />
        </ECEntityClass>
   </ECSchema>)xml")));
    if (true) {
        auto query = R"(
            WITH RECURSIVE
                base_classes (aId, aParentId, aPath, aDepth) AS (
                    SELECT c.ECInstanceId, null, c.Name, 0  FROM meta.ECClassDef c WHERE c.Name='A'
                    UNION ALL
                    SELECT c.ECInstanceId, cbc.TargetECInstanceId, aPath || '/' || c.Name, aDepth + 1
                        FROM meta.ECClassDef c
                            JOIN meta.ClassHasBaseClasses cbc ON cbc.SourceECInstanceId = c.ECInstanceId
                            JOIN base_classes  ON aId = cbc.TargetECInstanceId
                    ORDER BY 1
                )
            SELECT aId, aParentId, aDepth, aPath  from base_classes
        )";

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ("aId", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_STREQ("aParentId", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        ASSERT_STREQ("aDepth", stmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
        ASSERT_STREQ("aPath", stmt.GetColumnInfo(3).GetProperty()->GetName().c_str());
        /*
        aId aParentId aDepth aPath
        --- --------- ------ --------
        73    (null) 0      A
        74        73 1      A/AA
        75        74 2      A/AA/AAA
        76        74 2      A/AA/AAB
        77        73 1      A/AB
        78        77 2      A/AB/ABA
        79        77 2      A/AB/ABB
        */

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(CLASS_ID(ts,A), stmt.GetValueInt(0)) << "aId";
        ASSERT_EQ(true, stmt.IsValueNull(1)) << "aParentId";
        ASSERT_EQ(0, stmt.GetValueInt(2)) << "aDepth";
        ASSERT_STREQ("A", stmt.GetValueText(3)) << "aPath";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(CLASS_ID(ts,AA), stmt.GetValueInt(0)) << "aId";
        ASSERT_EQ(CLASS_ID(ts,A), stmt.GetValueInt(1)) << "aParentId";
        ASSERT_EQ(1, stmt.GetValueInt(2)) << "aDepth";
        ASSERT_STREQ("A/AA", stmt.GetValueText(3)) << "aPath";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(CLASS_ID(ts,AAA), stmt.GetValueInt(0)) << "aId";
        ASSERT_EQ(CLASS_ID(ts,AA), stmt.GetValueInt(1)) << "aParentId";
        ASSERT_EQ(2, stmt.GetValueInt(2)) << "aDepth";
        ASSERT_STREQ("A/AA/AAA", stmt.GetValueText(3)) << "aPath";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(CLASS_ID(ts,AAB), stmt.GetValueInt(0)) << "aId";
        ASSERT_EQ(CLASS_ID(ts,AA), stmt.GetValueInt(1)) << "aParentId";
        ASSERT_EQ(2, stmt.GetValueInt(2)) << "aDepth";
        ASSERT_STREQ("A/AA/AAB", stmt.GetValueText(3)) << "aPath";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(CLASS_ID(ts,AB), stmt.GetValueInt(0)) << "aId";
        ASSERT_EQ(CLASS_ID(ts,A), stmt.GetValueInt(1)) << "aParentId";
        ASSERT_EQ(1, stmt.GetValueInt(2)) << "aDepth";
        ASSERT_STREQ("A/AB", stmt.GetValueText(3)) << "aPath";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(CLASS_ID(ts,ABA), stmt.GetValueInt(0)) << "aId";
        ASSERT_EQ(CLASS_ID(ts,AB), stmt.GetValueInt(1)) << "aParentId";
        ASSERT_EQ(2, stmt.GetValueInt(2)) << "aDepth";
        ASSERT_STREQ("A/AB/ABA", stmt.GetValueText(3)) << "aPath";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(CLASS_ID(ts,ABB), stmt.GetValueInt(0)) << "aId";
        ASSERT_EQ(CLASS_ID(ts,AB), stmt.GetValueInt(1)) << "aParentId";
        ASSERT_EQ(2, stmt.GetValueInt(2)) << "aDepth";
        ASSERT_STREQ("A/AB/ABB", stmt.GetValueText(3)) << "aPath";

    }

    // get all property in for a given class
    if (true) {
        auto query = R"(
            WITH RECURSIVE
                base_classes (aId, aParentId, aPath, aDepth) AS (
                    SELECT c.ECInstanceId, null, c.Name, 0  FROM meta.ECClassDef c WHERE c.Name=?
                    UNION ALL
                    SELECT c.ECInstanceId, cbc.TargetECInstanceId, aPath || '/' || c.Name, aDepth + 1
                        FROM meta.ECClassDef c
                            JOIN meta.ClassHasBaseClasses cbc ON cbc.SourceECInstanceId = c.ECInstanceId
                            JOIN base_classes  ON aId = cbc.TargetECInstanceId
                    ORDER BY 1
                )
            SELECT group_concat( DISTINCT p.Name)  from base_classes join meta.ECPropertyDef p on p.Class.id = aId
        )";

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        stmt.BindText(1, "A", IECSqlBinder::MakeCopy::No);
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("a_prop,aa_prop,aaa_prop,aab_prop,ab_prop,aba_prop,abb_prop", stmt.GetValueText(0));
    }
    if (true) {
        auto query = R"(SELECT aPath FROM(
            WITH RECURSIVE
                base_classes (aId, aParentId, aPath, aDepth) AS (
                    SELECT c.ECInstanceId, null, c.Name, 0  FROM meta.ECClassDef c WHERE c.Name='A'
                    UNION ALL
                    SELECT c.ECInstanceId, cbc.TargetECInstanceId, aPath || '/' || c.Name, aDepth + 1
                        FROM meta.ECClassDef c
                            JOIN meta.ClassHasBaseClasses cbc ON cbc.SourceECInstanceId = c.ECInstanceId
                            JOIN base_classes  ON aId = cbc.TargetECInstanceId
                    ORDER BY 1
                )
            SELECT aId, aParentId, aDepth, aPath  from base_classes
        ))";

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ("aPath", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        /*
        aId aParentId aDepth aPath
        --- --------- ------ --------
        73    (null) 0      A
        74        73 1      A/AA
        75        74 2      A/AA/AAA
        76        74 2      A/AA/AAB
        77        73 1      A/AB
        78        77 2      A/AB/ABA
        79        77 2      A/AB/ABB
        */

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("A", stmt.GetValueText(0)) << "aPath";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("A/AA", stmt.GetValueText(0)) << "aPath";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("A/AA/AAA", stmt.GetValueText(0)) << "aPath";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("A/AA/AAB", stmt.GetValueText(0)) << "aPath";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("A/AB", stmt.GetValueText(0)) << "aPath";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("A/AB/ABA", stmt.GetValueText(0)) << "aPath";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("A/AB/ABB", stmt.GetValueText(0)) << "aPath";

    }

    // get all property in for a given class
    if (true) {
        auto query = R"(SELECT * FROM(
            WITH RECURSIVE
                base_classes (aId, aParentId, aPath, aDepth) AS (
                    SELECT c.ECInstanceId, null, c.Name, 0  FROM meta.ECClassDef c WHERE c.Name=?
                    UNION ALL
                    SELECT c.ECInstanceId, cbc.TargetECInstanceId, aPath || '/' || c.Name, aDepth + 1
                        FROM meta.ECClassDef c
                            JOIN meta.ClassHasBaseClasses cbc ON cbc.SourceECInstanceId = c.ECInstanceId
                            JOIN base_classes  ON aId = cbc.TargetECInstanceId
                    ORDER BY 1
                )
            SELECT group_concat( DISTINCT p.Name)  from base_classes join meta.ECPropertyDef p on p.Class.id = aId
        ))";

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        stmt.BindText(1, "A", IECSqlBinder::MakeCopy::No);
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("a_prop,aa_prop,aaa_prop,aab_prop,ab_prop,aba_prop,abb_prop", stmt.GetValueText(0));
    }
}

 //---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CommonTableExpTestFixture, RecursiveQuery) {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("cte_test.ecdb", SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
    <ECSchema schemaName='TestSchema' alias='ts' version='10.10.10' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
        <ECEntityClass typeName='Element' >
            <ECProperty propertyName="Subject" typeName="string" description="" />
            <ECNavigationProperty propertyName="Parent" description="" relationshipName="ElementOwnsChildElements" direction="backward">
                <ECCustomAttributes>
                   <HiddenProperty xmlns="CoreCustomAttributes.01.00.03"/>
                   <ForeignKeyConstraint xmlns="ECDbMap.02.00.00">
                        <OnDeleteAction>NoAction</OnDeleteAction>
                   </ForeignKeyConstraint>
                </ECCustomAttributes>
            </ECNavigationProperty>
        </ECEntityClass>
        <ECRelationshipClass typeName="ElementOwnsChildElements" description="" modifier="None" strength="embedding">
            <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
                <Class class="Element"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
                <Class class="Element"/>
            </Target>
        </ECRelationshipClass>
    </ECSchema>)xml")));

    ECSqlStatementCache cache(20);
    auto relClassId = m_ecdb.Schemas().GetClassId("TestSchema", "ElementOwnsChildElements");

    auto findElementBySubject = [&](BeInt64Id parentId, Utf8CP subject) {
        auto stmt = parentId.IsValid() ? cache.GetPreparedStatement(m_ecdb, "SELECT  ECInstanceId FROM ts.Element WHERE Parent.Id = ? AND Subject = ?") : cache.GetPreparedStatement(m_ecdb, "SELECT  ECInstanceId FROM ts.Element WHERE Parent.Id IS NULL AND Subject = ?");
        if (parentId.IsValid()) {
            stmt->BindId(1, BeInt64Id(parentId));
            stmt->BindText(2, subject, IECSqlBinder::MakeCopy::No);
        } else {
            stmt->BindText(1, subject, IECSqlBinder::MakeCopy::No);
        }

        if (BE_SQLITE_ROW == stmt->Step()) {
            return stmt->GetValueId<BeInt64Id>(0);
        }
        return BeInt64Id(0);
    };

    auto addElement = [&](Utf8CP subject, BeInt64Id parentId) {
        auto subjectId = findElementBySubject(parentId, subject);
        if (subjectId.IsValid()) {
            return subjectId;
        }

        auto stmt = cache.GetPreparedStatement(m_ecdb, "INSERT INTO ts.Element(Parent, Subject) VALUES(?, ?)");
        if (parentId.IsValid()) {
            stmt->BindNavigationValue(1, parentId, relClassId);
        }

        stmt->BindText(2, subject, IECSqlBinder::MakeCopy::No);
        ECInstanceKey key;
        if (stmt->Step(key) != BE_SQLITE_DONE) {
            return BeInt64Id(0);
        }

        return (BeInt64Id)key.GetInstanceId();
    };

    auto addElementPath = [&](Utf8CP path, Utf8CP delimiter = "/") {
        bvector<Utf8String> subjects;
        BeStringUtilities::Split(path, delimiter, subjects);
        BeInt64Id parentId(0);
        for(auto& subject : subjects) {
            parentId = addElement(subject.c_str(), parentId);
        }
        return parentId;
    };

    addElementPath("Drive/Document/Doc1");
    addElementPath("Drive/Document/Doc2");
    addElementPath("Drive/Document/Doc3");
    addElementPath("Drive/Pictures/Pic1");
    addElementPath("Drive/Pictures/Pic2");
    addElementPath("Book/SciFi/Book1");

/*
    Id Subject  ParentId
    -- -------- --------
     1 Drive      (null)
     2 Document        1
     3 Doc1            2
     4 Doc2            2
     5 Doc3            2
     6 Pictures        1
     7 Pic1            6
     8 Pic2            6
     9 Book       (null)
    10 SciFi           9
    11 Book1          10
*/
    if (true) {
        auto query = R"(
            WITH RECURSIVE
                cnt (aId, aParentId, aPath, aDepth) AS (
                    SELECT ECInstanceId, Parent.Id, Subject, 0 FROM ts.Element WHERE ECInstanceId = 1
                    UNION ALL
                    SELECT ECInstanceId, Parent.Id, aPath || '/' || Subject, aDepth + 1 FROM ts.Element, cnt WHERE Parent.Id = cnt.aId
                    ORDER BY 1
                )
            SELECT aId, aParentId, aDepth, aPath  from cnt
        )";

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ("aId", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_STREQ("aParentId", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        ASSERT_STREQ("aDepth", stmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
        ASSERT_STREQ("aPath", stmt.GetColumnInfo(3).GetProperty()->GetName().c_str());

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0)) << "aId";
        ASSERT_EQ(true, stmt.IsValueNull(1)) << "aParentId";
        ASSERT_EQ(0, stmt.GetValueInt(2)) << "aDepth";
        ASSERT_STREQ("Drive", stmt.GetValueText(3)) << "aPath";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(2, stmt.GetValueInt(0)) << "aId";
        ASSERT_EQ(1, stmt.GetValueInt(1)) << "aParentId";
        ASSERT_EQ(1, stmt.GetValueInt(2)) << "aDepth";
        ASSERT_STREQ("Drive/Document", stmt.GetValueText(3)) << "aPath";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(3, stmt.GetValueInt(0)) << "aId";
        ASSERT_EQ(2, stmt.GetValueInt(1)) << "aParentId";
        ASSERT_EQ(2, stmt.GetValueInt(2)) << "aDepth";
        ASSERT_STREQ("Drive/Document/Doc1", stmt.GetValueText(3)) << "aPath";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(4, stmt.GetValueInt(0)) << "aId";
        ASSERT_EQ(2, stmt.GetValueInt(1)) << "aParentId";
        ASSERT_EQ(2, stmt.GetValueInt(2)) << "aDepth";
        ASSERT_STREQ("Drive/Document/Doc2", stmt.GetValueText(3)) << "aPath";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(5, stmt.GetValueInt(0)) << "aId";
        ASSERT_EQ(2, stmt.GetValueInt(1)) << "aParentId";
        ASSERT_EQ(2, stmt.GetValueInt(2)) << "aDepth";
        ASSERT_STREQ("Drive/Document/Doc3", stmt.GetValueText(3)) << "aPath";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(6, stmt.GetValueInt(0)) << "aId";
        ASSERT_EQ(1, stmt.GetValueInt(1)) << "aParentId";
        ASSERT_EQ(1, stmt.GetValueInt(2)) << "aDepth";
        ASSERT_STREQ("Drive/Pictures", stmt.GetValueText(3)) << "aPath";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(7, stmt.GetValueInt(0)) << "aId";
        ASSERT_EQ(6, stmt.GetValueInt(1)) << "aParentId";
        ASSERT_EQ(2, stmt.GetValueInt(2)) << "aDepth";
        ASSERT_STREQ("Drive/Pictures/Pic1", stmt.GetValueText(3)) << "aPath";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(8, stmt.GetValueInt(0)) << "aId";
        ASSERT_EQ(6, stmt.GetValueInt(1)) << "aParentId";
        ASSERT_EQ(2, stmt.GetValueInt(2)) << "aDepth";
        ASSERT_STREQ("Drive/Pictures/Pic2", stmt.GetValueText(3)) << "aPath";
    }
    if (true) {
        auto query = R"(
            WITH RECURSIVE
                cnt (aId, aParentId, aPath, aDepth) AS (
                    SELECT ECInstanceId, Parent.Id, Subject, 0 FROM ts.Element WHERE ECInstanceId = 9
                    UNION ALL
                    SELECT ECInstanceId, Parent.Id, aPath || '/' || Subject, aDepth + 1 FROM ts.Element, cnt WHERE Parent.Id = cnt.aId
                    ORDER BY 1
                )
            SELECT aId, aParentId, aDepth, aPath  from cnt
        )";

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ("aId", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_STREQ("aParentId", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        ASSERT_STREQ("aDepth", stmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
        ASSERT_STREQ("aPath", stmt.GetColumnInfo(3).GetProperty()->GetName().c_str());


        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(9, stmt.GetValueInt(0)) << "aId";
        ASSERT_EQ(true, stmt.IsValueNull(1)) << "aParentId";
        ASSERT_EQ(0, stmt.GetValueInt(2)) << "aDepth";
        ASSERT_STREQ("Book", stmt.GetValueText(3)) << "aPath";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(10, stmt.GetValueInt(0)) << "aId";
        ASSERT_EQ(9, stmt.GetValueInt(1)) << "aParentId";
        ASSERT_EQ(1, stmt.GetValueInt(2)) << "aDepth";
        ASSERT_STREQ("Book/SciFi", stmt.GetValueText(3)) << "aPath";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(11, stmt.GetValueInt(0)) << "aId";
        ASSERT_EQ(10, stmt.GetValueInt(1)) << "aParentId";
        ASSERT_EQ(2, stmt.GetValueInt(2)) << "aDepth";
        ASSERT_STREQ("Book/SciFi/Book1", stmt.GetValueText(3)) << "aPath";
    }
}

 //---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CommonTableExpTestFixture, RecursiveQueryWithinSubQuery) {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("subquery_cte_test.ecdb", SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
    <ECSchema schemaName='TestSchema' alias='ts' version='10.10.10' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
        <ECEntityClass typeName='Element' >
            <ECProperty propertyName="Subject" typeName="string" description="" />
            <ECNavigationProperty propertyName="Parent" description="" relationshipName="ElementOwnsChildElements" direction="backward">
                <ECCustomAttributes>
                   <HiddenProperty xmlns="CoreCustomAttributes.01.00.03"/>
                   <ForeignKeyConstraint xmlns="ECDbMap.02.00.00">
                        <OnDeleteAction>NoAction</OnDeleteAction>
                   </ForeignKeyConstraint>
                </ECCustomAttributes>
            </ECNavigationProperty>
        </ECEntityClass>
        <ECRelationshipClass typeName="ElementOwnsChildElements" description="" modifier="None" strength="embedding">
            <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
                <Class class="Element"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
                <Class class="Element"/>
            </Target>
        </ECRelationshipClass>
    </ECSchema>)xml")));

    ECSqlStatementCache cache(20);
    auto relClassId = m_ecdb.Schemas().GetClassId("TestSchema", "ElementOwnsChildElements");

    auto findElementBySubject = [&](BeInt64Id parentId, Utf8CP subject) {
        auto stmt = parentId.IsValid() ? cache.GetPreparedStatement(m_ecdb, "SELECT  ECInstanceId FROM ts.Element WHERE Parent.Id = ? AND Subject = ?") : cache.GetPreparedStatement(m_ecdb, "SELECT  ECInstanceId FROM ts.Element WHERE Parent.Id IS NULL AND Subject = ?");
        if (parentId.IsValid()) {
            stmt->BindId(1, BeInt64Id(parentId));
            stmt->BindText(2, subject, IECSqlBinder::MakeCopy::No);
        } else {
            stmt->BindText(1, subject, IECSqlBinder::MakeCopy::No);
        }

        if (BE_SQLITE_ROW == stmt->Step()) {
            return stmt->GetValueId<BeInt64Id>(0);
        }
        return BeInt64Id(0);
    };

    auto addElement = [&](Utf8CP subject, BeInt64Id parentId) {
        auto subjectId = findElementBySubject(parentId, subject);
        if (subjectId.IsValid()) {
            return subjectId;
        }

        auto stmt = cache.GetPreparedStatement(m_ecdb, "INSERT INTO ts.Element(Parent, Subject) VALUES(?, ?)");
        if (parentId.IsValid()) {
            stmt->BindNavigationValue(1, parentId, relClassId);
        }

        stmt->BindText(2, subject, IECSqlBinder::MakeCopy::No);
        ECInstanceKey key;
        if (stmt->Step(key) != BE_SQLITE_DONE) {
            return BeInt64Id(0);
        }

        return (BeInt64Id)key.GetInstanceId();
    };

    auto addElementPath = [&](Utf8CP path, Utf8CP delimiter = "/") {
        bvector<Utf8String> subjects;
        BeStringUtilities::Split(path, delimiter, subjects);
        BeInt64Id parentId(0);
        for(auto& subject : subjects) {
            parentId = addElement(subject.c_str(), parentId);
        }
        return parentId;
    };

    addElementPath("Drive/Document/Doc1");
    addElementPath("Drive/Document/Doc2");
    addElementPath("Drive/Document/Doc3");
    addElementPath("Drive/Pictures/Pic1");
    addElementPath("Drive/Pictures/Pic2");
    addElementPath("Book/SciFi/Book1");

/*
    Id Subject  ParentId Depth
    -- -------- -------- ------
     1 Drive      (null)  0
     2 Document        1  1
     3 Doc1            2  2
     4 Doc2            2  2
     5 Doc3            2  2
     6 Pictures        1  1
     7 Pic1            6  2
     8 Pic2            6  2
     9 Book       (null)  0
    10 SciFi           9  1
    11 Book1          10  2
*/
    if (true) {
        auto query = R"(SELECT aParentId FROM(
            WITH RECURSIVE
                cnt (aId, aParentId, aPath, aDepth) AS (
                    SELECT ECInstanceId, Parent.Id, Subject, 0 FROM ts.Element WHERE ECInstanceId = 1
                    UNION ALL
                    SELECT ECInstanceId, Parent.Id, aPath || '/' || Subject, aDepth + 1 FROM ts.Element, cnt WHERE Parent.Id = cnt.aId
                    ORDER BY 1
                )
            SELECT aId, aParentId, aDepth, aPath  from cnt
        ))";

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ("aParentId", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(true, stmt.IsValueNull(0)) << "aParentId";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0)) << "aParentId";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(2, stmt.GetValueInt(0)) << "aParentId";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(2, stmt.GetValueInt(0)) << "aParentId";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(2, stmt.GetValueInt(0)) << "aParentId";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0)) << "aParentId";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(6, stmt.GetValueInt(0)) << "aParentId";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(6, stmt.GetValueInt(0)) << "aParentId";
    }
    if (true) {
        auto query = R"(SELECT aDepth FROM(
            WITH RECURSIVE
                cnt (aId, aParentId, aPath, aDepth) AS (
                    SELECT ECInstanceId, Parent.Id, Subject, 0 FROM ts.Element WHERE ECInstanceId = 9
                    UNION ALL
                    SELECT ECInstanceId, Parent.Id, aPath || '/' || Subject, aDepth + 1 FROM ts.Element, cnt WHERE Parent.Id = cnt.aId
                    ORDER BY 1
                )
            SELECT aId, aParentId, aDepth, aPath  from cnt
        ))";

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ("aDepth", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());


        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(0, stmt.GetValueInt(0)) << "aDepth";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0)) << "aDepth";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(2, stmt.GetValueInt(0)) << "aDepth";
    }
    if (true) {
        auto query = R"(SELECT DISTINCT aDepth FROM(
            WITH RECURSIVE
                cnt (aId, aParentId, aPath, aDepth) AS (
                    SELECT ECInstanceId, Parent.Id, Subject, 0 FROM ts.Element WHERE ECInstanceId = 9
                    UNION ALL
                    SELECT ECInstanceId, Parent.Id, aPath || '/' || Subject, aDepth + 1 FROM ts.Element, cnt WHERE Parent.Id = cnt.aId
                    ORDER BY 1
                )
            SELECT aId, aParentId, aDepth, aPath  from cnt
        ))";

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ("aDepth", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());


        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(0, stmt.GetValueInt(0)) << "aDepth";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0)) << "aDepth";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(2, stmt.GetValueInt(0)) << "aDepth";
    }
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CommonTableExpTestFixture, SqliteExampleWithinSubquery) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("cte_subquery_syntax.ecdb"));
    // FROM ONLY cnt should fail.
    // x in following has to be primitive value
    if (true) {
        auto query = R"(SELECT x FROM(
            WITH RECURSIVE
                cnt (x,y) AS (
                    SELECT 100, 200
                    UNION ALL
                    SELECT x+1, 200 FROM cnt WHERE x<210
                )
            SELECT * from cnt
        ))";

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ("x", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());


        for (int i = 100; i < 210; ++i)  {
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
            ASSERT_EQ(i, stmt.GetValueInt(0));
        }
    }
    if (true) {
        auto query = R"(SELECT x FROM(
            WITH RECURSIVE
                cnt (x) AS (
                    VALUES(100)
                    UNION ALL
                    SELECT x+1 FROM cnt WHERE x<210
                )
            SELECT x from cnt
        ))";


        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        for (int i = 100; i < 210; ++i)  {
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
            ASSERT_EQ(i, stmt.GetValueInt(0));
        }
    }
    if (true) {
        auto query = R"(SELECT y FROM(
            WITH
                cnt (x,y) AS (
                    SELECT 100, 200
                )
            SELECT * from cnt
        ))";


        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ("y", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(200, stmt.GetValueInt(0));
    }
    if (true) {
        auto query = R"(SELECT y,b FROM(
            WITH
                cte_1 (x,y) AS (
                    SELECT 100, 200
                ),
                cte_2 (a,b) AS (
                    SELECT 100, 400
                )
            SELECT * from cte_1, cte_2 where cte_1.x=cte_2.a
        ))";

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ("y", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_STREQ("b", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(200, stmt.GetValueInt(0));
        ASSERT_EQ(400, stmt.GetValueInt(1));
    }
    if (true) {
        auto query = R"(SELECT y,b FROM(
            WITH
                cte_1 (x,y) AS (
                    SELECT 100, 200
                ),
                cte_2 (a,b) AS (
                    SELECT 100, 400
                )
            SELECT * from cte_1 aa, cte_2 bb where aa.x=bb.a
        ))";

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ("y", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_STREQ("b", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(200, stmt.GetValueInt(0));
        ASSERT_EQ(400, stmt.GetValueInt(1));
    }

   if (true) {
        auto query = R"(SELECT * FROM(
            WITH
               cte_1 (a,b,c) AS (
                    SELECT 100, 400
                )
            SELECT * from cte_1
        ))";

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, query));
   }
   if (true) {
        auto query = R"(SELECT * FROM(
            WITH
               cte_1 (a,a) AS (
                    SELECT 100, 400
                )
            SELECT * from cte_1
        ))";

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, query));
   }
   if (true) {
        auto query = R"(SELECT * FROM(
            WITH
               cte_1 (a,b) AS (
                    SELECT 100, 400, 300
                )
            SELECT * from cte_1
        ))";

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, query));
   }
   if (true) {
        auto query = R"(SELECT * FROM(
            WITH
               cte_1 (a,b,c) AS (
                    SELECT 100, 400, 300
                ),
               cte_1 (a,b,c) AS (
                    SELECT 100, 400, 300
                )
            SELECT * from cte_1
        ))";

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, query));
   }
    {
    // Test Case 1 : CTE with one bind parameter
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, R"(SELECT * FROM(
        WITH RECURSIVE
            cte (x) AS (
                VALUES(:test1)
                UNION ALL
                SELECT x + 1 FROM cte WHERE x < 30)
        SELECT x from cte))"));

    EXPECT_STREQ("x", statement.GetColumnInfo(0).GetProperty()->GetName().c_str());
    EXPECT_TRUE(statement.GetColumnInfo(0).GetDataType().IsPrimitive());
    EXPECT_EQ(statement.GetColumnInfo(0).GetDataType().GetPrimitiveType(), PrimitiveType::PRIMITIVETYPE_Double);

    statement.BindInt(1, 1);
    for (auto i = 1; i <= 30; ++i)
        {
        EXPECT_EQ(BE_SQLITE_ROW, statement.Step());
        EXPECT_EQ(i, statement.GetValueInt(0));
        }
    statement.Finalize();
    }
    {
    // Test Case 2 : CTE with 2 bind parameters
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, R"(SELECT y FROM(
        WITH RECURSIVE
            cte (x, y) AS (
                VALUES(:test1, :test2)
                UNION ALL
                SELECT x + 1, y + 2 FROM cte WHERE x < 4 and y < 6)
        SELECT * from cte))"));

    EXPECT_STREQ("y", statement.GetColumnInfo(0).GetProperty()->GetName().c_str());

    EXPECT_TRUE(statement.GetColumnInfo(0).GetDataType().IsPrimitive());
    EXPECT_EQ(statement.GetColumnInfo(0).GetDataType().GetPrimitiveType(), PrimitiveType::PRIMITIVETYPE_Double);

    statement.BindInt(1, 1);
    statement.BindInt(2, 2);
    for (const auto& testValues : std::vector<int> {2, 4, 6})
        {
        EXPECT_EQ(BE_SQLITE_ROW, statement.Step());
        EXPECT_EQ(testValues, statement.GetValueInt(0));
        }
    statement.Finalize();
    }
    {
    // Test Case 3 : CTE with 1 bind parameters and 1 literal
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, R"(SELECT x FROM(
        WITH RECURSIVE
            cte (x, y) AS (
                VALUES(:test1, 200)
                UNION ALL
                SELECT x + 1, y FROM cte WHERE x < 3)
        SELECT * from cte))"));

    EXPECT_STREQ("x", statement.GetColumnInfo(0).GetProperty()->GetName().c_str());

    EXPECT_TRUE(statement.GetColumnInfo(0).GetDataType().IsPrimitive());
    EXPECT_EQ(statement.GetColumnInfo(0).GetDataType().GetPrimitiveType(), PrimitiveType::PRIMITIVETYPE_Double);

    statement.BindInt(1, 1);
    for (const auto& testValues : std::vector<int>{1, 2, 3})
        {
        EXPECT_EQ(BE_SQLITE_ROW, statement.Step());
        EXPECT_EQ(testValues, statement.GetValueInt(0));
        }
    statement.Finalize();
    }
    {
    // Test Case 4 : CTE with one bind parameter
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, R"(SELECT * FROM(
        WITH RECURSIVE
            cte (x) AS (
                SELECT ECInstanceId FROM meta.ECSchemaDef WHERE ECInstanceId = :test1
                UNION ALL
                SELECT x + 1 FROM cte WHERE x < 3)
        SELECT x from cte))"));

    EXPECT_STREQ("x", statement.GetColumnInfo(0).GetProperty()->GetName().c_str());
    EXPECT_TRUE(statement.GetColumnInfo(0).GetDataType().IsPrimitive());
    EXPECT_EQ(statement.GetColumnInfo(0).GetDataType().GetPrimitiveType(), PrimitiveType::PRIMITIVETYPE_Long);

    statement.BindInt(1, 1);
    for (auto i = 1; i <= 3; ++i)
        {
        EXPECT_EQ(BE_SQLITE_ROW, statement.Step());
        EXPECT_EQ(i, statement.GetValueInt(0));
        }
    statement.Finalize();
    }
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CommonTableExpTestFixture, SqliteExample) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("cte_syntax.ecdb"));
    // FROM ONLY cnt should fail.
    // x in following has to be primitive value
    if (true) {
        auto query = R"(
            WITH RECURSIVE
                cnt (x,y) AS (
                    SELECT 100, 200
                    UNION ALL
                    SELECT x+1, 200 FROM cnt WHERE x<210
                )
            SELECT * from cnt
        )";

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ("x", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_STREQ("y", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());


        for (int i = 100; i < 210; ++i)  {
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
            ASSERT_EQ(i, stmt.GetValueInt(0));
            ASSERT_EQ(200, stmt.GetValueInt(1));
        }
    }
    if (true) {
        auto query = R"(
            WITH RECURSIVE
                cnt (x,y) AS (
                    SELECT 100, 200
                    UNION ALL
                    SELECT x+1, 200 FROM cnt WHERE x<210
                )
            SELECT x,y from cnt
        )";


        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        for (int i = 100; i < 210; ++i)  {
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
            ASSERT_EQ(i, stmt.GetValueInt(0));
            ASSERT_EQ(200, stmt.GetValueInt(1));
        }
    }
    if (true) {
        auto query = R"(
            WITH RECURSIVE
                cnt (x) AS (
                    VALUES(100)
                    UNION ALL
                    SELECT x+1 FROM cnt WHERE x<210
                )
            SELECT x from cnt
        )";


        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        for (int i = 100; i < 210; ++i)  {
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
            ASSERT_EQ(i, stmt.GetValueInt(0));
        }
    }
    if (true) {
        auto query = R"(
            WITH
                cnt (x,y) AS (
                    SELECT 100, 200
                )
            SELECT * from cnt
        )";


        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ("x", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_STREQ("y", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(100, stmt.GetValueInt(0));
        ASSERT_EQ(200, stmt.GetValueInt(1));
    }
    if (true) {
        auto query = R"(
            WITH
                cte_1 (x,y) AS (
                    SELECT 100, 200
                ),
                cte_2 (a,b) AS (
                    SELECT 100, 400
                )
            SELECT * from cte_1, cte_2 where cte_1.x=cte_2.a
        )";

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ("x", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_STREQ("y", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        ASSERT_STREQ("a", stmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
        ASSERT_STREQ("b", stmt.GetColumnInfo(3).GetProperty()->GetName().c_str());

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(100, stmt.GetValueInt(0));
        ASSERT_EQ(200, stmt.GetValueInt(1));
        ASSERT_EQ(100, stmt.GetValueInt(2));
        ASSERT_EQ(400, stmt.GetValueInt(3));
    }
    if (true) {
        auto query = R"(
            WITH
                cte_1 (x,y) AS (
                    SELECT 100, 200
                ),
                cte_2 (a,b) AS (
                    SELECT 100, 400
                )
            SELECT * from cte_1 aa, cte_2 bb where aa.x=bb.a
        )";

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ("x", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_STREQ("y", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        ASSERT_STREQ("a", stmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
        ASSERT_STREQ("b", stmt.GetColumnInfo(3).GetProperty()->GetName().c_str());

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(100, stmt.GetValueInt(0));
        ASSERT_EQ(200, stmt.GetValueInt(1));
        ASSERT_EQ(100, stmt.GetValueInt(2));
        ASSERT_EQ(400, stmt.GetValueInt(3));
    }

   if (true) {
        auto query = R"(
            WITH
               cte_1 (a,b,c) AS (
                    SELECT 100, 400
                )
            SELECT * from cte_1
        )";

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, query));
   }
   if (true) {
        auto query = R"(
            WITH
               cte_1 (a,a) AS (
                    SELECT 100, 400
                )
            SELECT * from cte_1
        )";

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, query));
   }
   if (true) {
        auto query = R"(
            WITH
               cte_1 (a,b) AS (
                    SELECT 100, 400, 300
                )
            SELECT * from cte_1
        )";

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, query));
   }
   if (true) {
        auto query = R"(
            WITH
               cte_1 (a,b,c) AS (
                    SELECT 100, 400, 300
                ),
               cte_1 (a,b,c) AS (
                    SELECT 100, 400, 300
                )
            SELECT * from cte_1
        )";

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, query));
   }
    {
    // Test Case 1 : CTE with one bind parameter
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, R"(
        WITH RECURSIVE
            cte (x) AS (
                VALUES(:test1)
                UNION ALL
                SELECT x + 1 FROM cte WHERE x < 3)
        SELECT x from cte)"));

    EXPECT_STREQ("x", statement.GetColumnInfo(0).GetProperty()->GetName().c_str());
    EXPECT_TRUE(statement.GetColumnInfo(0).GetDataType().IsPrimitive());
    EXPECT_EQ(statement.GetColumnInfo(0).GetDataType().GetPrimitiveType(), PrimitiveType::PRIMITIVETYPE_Double);

    statement.BindInt(1, 1);
    for (auto i = 1; i <= 3; ++i)
        {
        EXPECT_EQ(BE_SQLITE_ROW, statement.Step());
        EXPECT_EQ(i, statement.GetValueInt(0));
        }
    statement.Finalize();
    }
    {
    // Test Case 2 : CTE with 2 bind parameters
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, R"(
        WITH RECURSIVE
            cte (x, y) AS (
                VALUES(:test1, :test2)
                UNION ALL
                SELECT x + 1, y + 2 FROM cte WHERE x < 4 and y < 6)
        SELECT * from cte)"));

    EXPECT_STREQ("x", statement.GetColumnInfo(0).GetProperty()->GetName().c_str());
    EXPECT_STREQ("y", statement.GetColumnInfo(1).GetProperty()->GetName().c_str());

    EXPECT_TRUE(statement.GetColumnInfo(0).GetDataType().IsPrimitive());
    EXPECT_EQ(statement.GetColumnInfo(0).GetDataType().GetPrimitiveType(), PrimitiveType::PRIMITIVETYPE_Double);
    EXPECT_TRUE(statement.GetColumnInfo(1).GetDataType().IsPrimitive());
    EXPECT_EQ(statement.GetColumnInfo(1).GetDataType().GetPrimitiveType(), PrimitiveType::PRIMITIVETYPE_Double);

    statement.BindInt(1, 1);
    statement.BindInt(2, 2);
    for (const auto& testValues : std::vector<std::pair<int, int>> {{1, 2}, {2, 4}, {3, 6}})
        {
        EXPECT_EQ(BE_SQLITE_ROW, statement.Step());
        EXPECT_EQ(testValues.first, statement.GetValueInt(0));
        EXPECT_EQ(testValues.second, statement.GetValueInt(1));
        }
    statement.Finalize();
    }
    {
    // Test Case 3 : CTE with 1 bind parameters and 1 literal
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, R"(
        WITH RECURSIVE
            cte (x, y) AS (
                VALUES(:test1, 200)
                UNION ALL
                SELECT x + 1, y FROM cte WHERE x < 3)
        SELECT * from cte)"));

    EXPECT_STREQ("x", statement.GetColumnInfo(0).GetProperty()->GetName().c_str());
    EXPECT_STREQ("y", statement.GetColumnInfo(1).GetProperty()->GetName().c_str());

    EXPECT_TRUE(statement.GetColumnInfo(0).GetDataType().IsPrimitive());
    EXPECT_EQ(statement.GetColumnInfo(0).GetDataType().GetPrimitiveType(), PrimitiveType::PRIMITIVETYPE_Double);
    EXPECT_TRUE(statement.GetColumnInfo(1).GetDataType().IsPrimitive());
    EXPECT_EQ(statement.GetColumnInfo(1).GetDataType().GetPrimitiveType(), PrimitiveType::PRIMITIVETYPE_Long);  // Literals get created as Long Primitives

    statement.BindInt(1, 1);
    for (const auto& testValues : std::vector<int>{1, 2, 3})
        {
        EXPECT_EQ(BE_SQLITE_ROW, statement.Step());
        EXPECT_EQ(testValues, statement.GetValueInt(0));
        EXPECT_EQ(200, statement.GetValueInt(1));
        }
    statement.Finalize();
    }
    {
    // Test Case 4 : CTE with one bind parameter
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, R"(
        WITH RECURSIVE
            cte (x) AS (
                SELECT ECInstanceId FROM meta.ECSchemaDef WHERE ECInstanceId = :test1
                UNION ALL
                SELECT x + 1 FROM cte WHERE x < 3)
        SELECT x from cte)"));

    EXPECT_STREQ("x", statement.GetColumnInfo(0).GetProperty()->GetName().c_str());
    EXPECT_TRUE(statement.GetColumnInfo(0).GetDataType().IsPrimitive());
    EXPECT_EQ(statement.GetColumnInfo(0).GetDataType().GetPrimitiveType(), PrimitiveType::PRIMITIVETYPE_Long);

    statement.BindInt(1, 1);
    for (auto i = 1; i <= 3; ++i)
        {
        EXPECT_EQ(BE_SQLITE_ROW, statement.Step());
        EXPECT_EQ(i, statement.GetValueInt(0));
        }
    statement.Finalize();
    }
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CommonTableExpTestFixture, alias_to_cte) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("cte_test_meta.ecdb"));
    if ("simple_nested_no_alias") {
        auto query = R"(
            with recursive
                cte0 (a,b) as ( select 100,200)
            select * from (select a, b from cte0 where a=100 and b=200)
        )";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ(stmt.GetNativeSql(), "WITH RECURSIVE cte0(a,b) AS (SELECT 100,200)\nSELECT [K2],[K3] FROM (SELECT cte0.a [K2],cte0.b [K3] FROM cte0 WHERE cte0.a=100 AND cte0.b=200)");
    }
    if ("simple_nested") {
        auto query = R"(
            with recursive
                cte0 (a,b) as ( select 100,200)
            select * from (select c0.a, c0.b from cte0 c0 where c0.a=100 and c0.b=200)
        )";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ(stmt.GetNativeSql(), "WITH RECURSIVE cte0(a,b) AS (SELECT 100,200)\nSELECT [K2],[K3] FROM (SELECT c0.a [K2],c0.b [K3] FROM cte0 c0 WHERE c0.a=100 AND c0.b=200)");
    }
    if ("simple_wild_nested") {
        auto query = R"(
            with recursive
                cte0 (a,b) as ( select 100,200)
            select * from (select * from cte0 c0 where c0.a=100 and c0.b=200)
        )";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ(stmt.GetNativeSql(), "WITH RECURSIVE cte0(a,b) AS (SELECT 100,200)\nSELECT [K2],[K3] FROM (SELECT c0.a K2,c0.b K3 FROM cte0 c0 WHERE c0.a=100 AND c0.b=200)");
    }

    if ("simple_wild") {
        auto query = R"(
            with recursive
                cte0 (a,b) as ( select 100,200)
            select * from cte0 c0 where c0.a=100 and c0.b=200
        )";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ(stmt.GetNativeSql(), "WITH RECURSIVE cte0(a,b) AS (SELECT 100,200)\nSELECT c0.a,c0.b FROM cte0 c0 WHERE c0.a=100 AND c0.b=200");
    }
    if ("simple_wild_no_alias") {
        auto query = R"(
            with recursive
                cte0 (a,b) as ( select 100,200)
            select * from cte0 where a=100 and b=200
        )";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ(stmt.GetNativeSql(), "WITH RECURSIVE cte0(a,b) AS (SELECT 100,200)\nSELECT cte0.a,cte0.b FROM cte0 WHERE cte0.a=100 AND cte0.b=200");
    }
    if ("simple_alias") {
        auto query = R"(
            with recursive
                cte0 (a,b) as ( select 100,200)
            select c0.a, c0.b from cte0 c0 where c0.a=100 and c0.b=200
        )";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ(stmt.GetNativeSql(), "WITH RECURSIVE cte0(a,b) AS (SELECT 100,200)\nSELECT c0.a,c0.b FROM cte0 c0 WHERE c0.a=100 AND c0.b=200");
    }
    if ("ambiguous_col") {
        auto query = R"(
            with recursive
                cte0 (a,b) as ( select 100,200),
                cte1 (c,d) as ( select 100,200)
            select * from cte0 c0, cte1 c1 where c0.a=100 and c0.b=200 and c1.c=100 and c1.d=200
        )";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ(stmt.GetNativeSql(), "WITH RECURSIVE cte0(a,b) AS (SELECT 100,200),cte1(c,d) AS (SELECT 100,200)\nSELECT c0.a,c0.b,c1.c,c1.d FROM cte0 c0,cte1 c1 WHERE c0.a=100 AND c0.b=200 AND c1.c=100 AND c1.d=200");
    }
    if ("ambiguous_col_2") {
        auto query = R"(
            with recursive
                cte0 (a,b) as ( select 100,200),
                cte1 (a,b) as ( select 100,200)
            select * from cte0 c0, cte1 c1 where c0.a=100 and c0.b=200 and c1.a=100 and c1.b=200
        )";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ(stmt.GetNativeSql(), "WITH RECURSIVE cte0(a,b) AS (SELECT 100,200),cte1(a,b) AS (SELECT 100,200)\nSELECT c0.a,c0.b,c1.a,c1.b FROM cte0 c0,cte1 c1 WHERE c0.a=100 AND c0.b=200 AND c1.a=100 AND c1.b=200");
    }
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CommonTableExpTestFixture, alias_to_cte_within_subquery) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("cte_subquery_test_meta.ecdb"));
    if ("simple_nested_no_alias") {
        auto query = R"(select * from(
            with recursive
                cte0 (a,b) as ( select 100,200)
            select * from (select a, b from cte0 where a=100 and b=200)
        ))";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ(stmt.GetNativeSql(), "SELECT [K4],[K5] FROM (WITH RECURSIVE cte0(a,b) AS (SELECT 100,200)\nSELECT [K2] [K4],[K3] [K5] FROM (SELECT cte0.a [K2],cte0.b [K3] FROM cte0 WHERE cte0.a=100 AND cte0.b=200))");
    }
    if ("simple_nested") {
        auto query = R"(SELECT * FROM(
            with recursive
                cte0 (a,b) as ( select 100,200)
            select * from (select c0.a, c0.b from cte0 c0 where c0.a=100 and c0.b=200)
        ))";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ(stmt.GetNativeSql(), "SELECT [K4],[K5] FROM (WITH RECURSIVE cte0(a,b) AS (SELECT 100,200)\nSELECT [K2] [K4],[K3] [K5] FROM (SELECT c0.a [K2],c0.b [K3] FROM cte0 c0 WHERE c0.a=100 AND c0.b=200))");
    }
    if ("simple_wild_nested") {
        auto query = R"(select a from(
            with recursive
                cte0 (a,b) as ( select 100,200)
            select * from (select * from cte0 c0 where c0.a=100 and c0.b=200)
        ))";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ(stmt.GetNativeSql(), "SELECT [a] FROM (WITH RECURSIVE cte0(a,b) AS (SELECT 100,200)\nSELECT [K2] [a],[K3] FROM (SELECT c0.a K2,c0.b K3 FROM cte0 c0 WHERE c0.a=100 AND c0.b=200))");
    }

    if ("simple_wild") {
        auto query = R"(select b from(
            with recursive
                cte0 (a,b) as ( select 100,200)
            select * from cte0 c0 where c0.a=100 and c0.b=200
        ))";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ(stmt.GetNativeSql(), "SELECT [K2] FROM (WITH RECURSIVE cte0(a,b) AS (SELECT 100,200)\nSELECT c0.a,c0.b K2 FROM cte0 c0 WHERE c0.a=100 AND c0.b=200)");
    }
    if ("simple_wild_no_alias") {
        auto query = R"(select * from(
            with recursive
                cte0 (a,b) as ( select 100,200)
            select * from cte0 where a=100 and b=200
        ))";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ(stmt.GetNativeSql(), "SELECT [K2],[K3] FROM (WITH RECURSIVE cte0(a,b) AS (SELECT 100,200)\nSELECT cte0.a K2,cte0.b K3 FROM cte0 WHERE cte0.a=100 AND cte0.b=200)");
    }
    if ("simple_alias") {
        auto query = R"(select c0.a from(
            with recursive
                cte0 (a,b) as ( select 100,200)
            select c0.a, c0.b from cte0 c0 where c0.a=100 and c0.b=200
        ))";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ(stmt.GetNativeSql(), "SELECT [K2] FROM (WITH RECURSIVE cte0(a,b) AS (SELECT 100,200)\nSELECT c0.a [K2],c0.b FROM cte0 c0 WHERE c0.a=100 AND c0.b=200)");
    }
    if ("ambiguous_col") {
        auto query = R"(select a,b from(
            with recursive
                cte0 (a,b) as ( select 100,200),
                cte1 (c,d) as ( select 300,400)
            select * from cte0 c0, cte1 c1 where c0.a=100 and c0.b=200
        ))";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ(stmt.GetNativeSql(), "SELECT [K2],[K3] FROM (WITH RECURSIVE cte0(a,b) AS (SELECT 100,200),cte1(c,d) AS (SELECT 300,400)\nSELECT c0.a K2,c0.b K3,c1.c,c1.d FROM cte0 c0,cte1 c1 WHERE c0.a=100 AND c0.b=200)");
    }
    if ("ambiguous_col_2") {
        auto query = R"(select a,b from(
            with recursive
                cte0 (a,b) as ( select 100,200),
                cte1 (a,b) as ( select 300,400)
            select * from cte0 c0, cte1 c1 where c0.a=100 and c0.b=200 and c1.a=300 and c1.b=400
        ))";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ(stmt.GetNativeSql(), "SELECT [K4],[K5] FROM (WITH RECURSIVE cte0(a,b) AS (SELECT 100,200),cte1(a,b) AS (SELECT 300,400)\nSELECT c0.a K4,c0.b K5,c1.a,c1.b FROM cte0 c0,cte1 c1 WHERE c0.a=100 AND c0.b=200 AND c1.a=300 AND c1.b=400)");
    }
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CommonTableExpTestFixture, SubQueryBlock) {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("SubQueryBlock.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEntityClass typeName="Parent">
                    <ECProperty propertyName="Name" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Child">
                    <ECProperty propertyName="Name" typeName="string" />
                </ECEntityClass>
                <ECRelationshipClass typeName="Rel" modifier="None">
                    <Source multiplicity="(0..*)" polymorphic="True" roleLabel="is parent of">
                        <Class class="Parent" />
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is child of">
                        <Class class="Child"/>
                    </Target>
                </ECRelationshipClass>
                <ECEntityClass typeName="Foo">
                    <ECProperty propertyName="Code" typeName="int" />
                </ECEntityClass>
            </ECSchema>)xml")));

    ECClassId fooClassId = m_ecdb.Schemas().GetClassId("TestSchema", "Foo");
    ASSERT_TRUE(fooClassId.IsValid());
    ECClassId parentClassId = m_ecdb.Schemas().GetClassId("TestSchema", "Parent");
    ASSERT_TRUE(parentClassId.IsValid());
    ECClassId childClassId = m_ecdb.Schemas().GetClassId("TestSchema", "Child");
    ASSERT_TRUE(childClassId.IsValid());
    if ("simple_select_query") {
        auto ecsql = R"(
            WITH models(i) AS (
                SELECT foo.ECInstanceId FROM ts.Foo foo)
            SELECT i FROM models WHERE models.i = 1
        )";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_STREQ(SqlPrintfString("WITH models(i) AS (SELECT [foo].[ECInstanceId] FROM (SELECT [Id] ECInstanceId,%s ECClassId FROM [main].[ts_Foo]) [foo])\nSELECT models.i FROM models WHERE models.i=1", fooClassId.ToString().c_str()), stmt.GetNativeSql());
    }
    if ("select_property_in_cte_block") {
        auto ecsql = R"(
            WITH models(i) AS (
                SELECT foo.Code FROM ts.Foo foo)
            SELECT i FROM models WHERE models.i IN (?)
        )";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_STREQ(SqlPrintfString("WITH models(i) AS (SELECT [foo].[Code] FROM (SELECT [Id] ECInstanceId,%s ECClassId,[Code] FROM [main].[ts_Foo]) [foo])\nSELECT models.i FROM models WHERE models.i IN (:_ecdb_sqlparam_ix1_col1)", fooClassId.ToString().c_str()), stmt.GetNativeSql());
    }
    if ("select_id_in_cte_block") {
        auto ecsql = R"(
            WITH models(i) AS (
                SELECT foo.ECInstanceId FROM ts.Foo foo)
            SELECT i FROM models WHERE models.i IN (?)
        )";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_STREQ(SqlPrintfString("WITH models(i) AS (SELECT [foo].[ECInstanceId] FROM (SELECT [Id] ECInstanceId,%s ECClassId FROM [main].[ts_Foo]) [foo])\nSELECT models.i FROM models WHERE models.i IN (:_ecdb_sqlparam_ix1_col1)", fooClassId.ToString().c_str()), stmt.GetNativeSql());
    }
    if ("nested_select_id_in_cte_block") {
        auto ecsql = R"(
            WITH models(i) AS (
                SELECT (SELECT foo.ECInstanceId FROM ts.Foo foo) AS ecId)
            SELECT i FROM models WHERE models.i IN (?)
        )";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_STREQ(SqlPrintfString("WITH models(i) AS (SELECT (SELECT [foo].[ECInstanceId] FROM (SELECT [Id] ECInstanceId,%s ECClassId FROM [main].[ts_Foo]) [foo]) [ecId])\nSELECT models.i FROM models WHERE models.i IN (:_ecdb_sqlparam_ix1_col1)", fooClassId.ToString().c_str()), stmt.GetNativeSql());
    }
    if ("select_link_table_in_cte_block") {
        auto ecsql = R"(
            WITH models(i, c, si, sc, ti, tc) AS (
                SELECT
                    r.ECInstanceId,
                    r.ECClassId,
                    r.SourceECInstanceId,
                    r.SourceECClassId,
                    r.TargetECInstanceId,
                    r.TargetECClassId
                FROM ts.Rel r)
            SELECT
                *
            FROM
                models m
            WHERE
                m.i = ? AND m.c = ? AND m.si = ? AND m.sc = ? AND m.ti = ? AND m.tc = ?
                AND m.i IN (?) AND m.c IN (?) AND m.si IN (?) AND m.sc IN (?) AND m.ti IN (?) AND m.tc IN (?)
        )";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_STREQ(SqlPrintfString("WITH models(i,c,si,sc,ti,tc) AS (SELECT [r].[ECInstanceId],[r].[ECClassId],[r].[SourceECInstanceId],[r].[SourceECClassId],[r].[TargetECInstanceId],[r].[TargetECClassId] FROM (SELECT [ts_Rel].[Id] [ECInstanceId],[ts_Rel].[ECClassId],[ts_Rel].[SourceId] [SourceECInstanceId],%s [SourceECClassId],[ts_Rel].[TargetId] [TargetECInstanceId],%s [TargetECClassId] FROM [main].[ts_Rel]) [r])\nSELECT m.i,m.c,m.si,m.sc,m.ti,m.tc FROM models m WHERE m.i=:_ecdb_sqlparam_ix1_col1 AND m.c=:_ecdb_sqlparam_ix2_col1 AND m.si=:_ecdb_sqlparam_ix3_col1 AND m.sc=:_ecdb_sqlparam_ix4_col1 AND m.ti=:_ecdb_sqlparam_ix5_col1 AND m.tc=:_ecdb_sqlparam_ix6_col1 AND m.i IN (:_ecdb_sqlparam_ix7_col1) AND m.c IN (:_ecdb_sqlparam_ix8_col1) AND m.si IN (:_ecdb_sqlparam_ix9_col1) AND m.sc IN (:_ecdb_sqlparam_ix10_col1) AND m.ti IN (:_ecdb_sqlparam_ix11_col1) AND m.tc IN (:_ecdb_sqlparam_ix12_col1)",
                    parentClassId.ToString().c_str(), childClassId.ToString().c_str()), stmt.GetNativeSql());
    }
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CommonTableExpTestFixture, CTE_Subquery_Tests) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("CTESubqueryTests.ecdb"));

    if ("simple_select_cte_subquery") {
        auto ecsql = R"(select * from meta.ECClassDef where 
        ECInstanceId >= SOME(with a(ECClassId) AS (select ECClassId from meta.ECPropertyDef) select * from a) 
        LIMIT 1)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_STREQ(stmt.GetNativeSql(),"SELECT [ECClassDef].[ECInstanceId],[ECClassDef].[ECClassId],[ECClassDef].[SchemaId],[ECClassDef].[SchemaRelECClassId],[ECClassDef].[Name],[ECClassDef].[DisplayLabel],[ECClassDef].[Description],[ECClassDef].[Type],[ECClassDef].[Modifier],[ECClassDef].[CustomAttributeContainerType],[ECClassDef].[RelationshipStrength],[ECClassDef].[RelationshipStrengthDirection] FROM (SELECT [Id] ECInstanceId,37 ECClassId,[SchemaId],(CASE WHEN [SchemaId] IS NULL THEN NULL ELSE 38 END) [SchemaRelECClassId],[Name],[DisplayLabel],[Description],[Type],[Modifier],[CustomAttributeContainerType],[RelationshipStrength],[RelationshipStrengthDirection] FROM [main].[ec_Class]) [ECClassDef] WHERE EXISTS(WITH a(ECClassId) AS (SELECT [ECPropertyDef].[ECClassId] FROM (SELECT [Id] ECInstanceId,44 ECClassId FROM [main].[ec_Property]) [ECPropertyDef])\nSELECT a.ECClassId FROM a WHERE [ECClassDef].[ECInstanceId] >= a.ECClassId)  LIMIT 1" );
    }
    if ("simple_select_subquery") {
        auto ecsql = R"(
            select * from meta.ECClassDef where ECInstanceId >= SOME(select ECClassId from meta.ECPropertyDef) LIMIT 1
        )";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_STREQ(stmt.GetNativeSql(),"SELECT [ECClassDef].[ECInstanceId],[ECClassDef].[ECClassId],[ECClassDef].[SchemaId],[ECClassDef].[SchemaRelECClassId],[ECClassDef].[Name],[ECClassDef].[DisplayLabel],[ECClassDef].[Description],[ECClassDef].[Type],[ECClassDef].[Modifier],[ECClassDef].[CustomAttributeContainerType],[ECClassDef].[RelationshipStrength],[ECClassDef].[RelationshipStrengthDirection] FROM (SELECT [Id] ECInstanceId,37 ECClassId,[SchemaId],(CASE WHEN [SchemaId] IS NULL THEN NULL ELSE 38 END) [SchemaRelECClassId],[Name],[DisplayLabel],[Description],[Type],[Modifier],[CustomAttributeContainerType],[RelationshipStrength],[RelationshipStrengthDirection] FROM [main].[ec_Class]) [ECClassDef] WHERE EXISTS(SELECT [ECPropertyDef].[ECClassId] FROM (SELECT [Id] ECInstanceId,44 ECClassId FROM [main].[ec_Property]) [ECPropertyDef] WHERE [ECClassDef].[ECInstanceId] >= [ECPropertyDef].[ECClassId])  LIMIT 1");
    }
    if ("checking_result_equality_between_simple_select_and_cte_in_subquery") {
        auto ecsqlSelect = R"(select * from meta.ECClassDef where 
        ECInstanceId >= SOME(with a(ECClassId) AS (select ECClassId from meta.ECPropertyDef) select * from a) 
        LIMIT 1)";
        ECSqlStatement stmtSelect;
        ASSERT_EQ(ECSqlStatus::Success, stmtSelect.Prepare(m_ecdb, ecsqlSelect));

        auto ecsqlCTE = R"(
            select * from meta.ECClassDef where ECInstanceId >= SOME(select ECClassId from meta.ECPropertyDef) LIMIT 1
        )";
        ECSqlStatement stmtCTE;
        ASSERT_EQ(ECSqlStatus::Success, stmtCTE.Prepare(m_ecdb, ecsqlCTE));
        EXPECT_STREQ(stmtCTE.GetColumnInfo(0).GetProperty()->GetName().c_str(), stmtSelect.GetColumnInfo(0).GetProperty()->GetName().c_str());
        EXPECT_STREQ(stmtCTE.GetColumnInfo(1).GetProperty()->GetName().c_str(), stmtSelect.GetColumnInfo(1).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmtCTE.Step());
        ASSERT_EQ(BE_SQLITE_ROW, stmtSelect.Step());
        ASSERT_EQ(stmtSelect.GetValueInt(0), stmtCTE.GetValueInt(0));
        ASSERT_EQ(stmtSelect.GetValueInt(1), stmtCTE.GetValueInt(1));
    }
    if ("simple_select_cte_subquery_with_alias") {
        auto ecsql = R"(select a.ECInstanceId from meta.ECClassDef a where a.ECInstanceId >= ALL(with a(Id) as (select a.ECClassId from meta.ECPropertyDef a) select a.Id from a) LIMIT 1)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_STREQ(stmt.GetNativeSql(),"SELECT [a].[ECInstanceId] FROM (SELECT [Id] ECInstanceId,37 ECClassId FROM [main].[ec_Class]) [a] WHERE NOT EXISTS(WITH a(Id) AS (SELECT [a].[ECClassId] FROM (SELECT [Id] ECInstanceId,44 ECClassId FROM [main].[ec_Property]) [a])\nSELECT a.Id FROM a WHERE a.Id >= [a].[ECInstanceId])  LIMIT 1" );
    }
    if ("simple_select_cte_subquery_with_alias_without_ALL") {
        auto ecsql = R"(select a.ECInstanceId from meta.ECClassDef a where ECInstanceId >= (with a(ECClassId) AS (select a.ECClassId from meta.ECPropertyDef a) select a.ECClassId from a) LIMIT 1)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_STREQ(stmt.GetNativeSql(),"SELECT [a].[ECInstanceId] FROM (SELECT [Id] ECInstanceId,37 ECClassId FROM [main].[ec_Class]) [a] WHERE [a].[ECInstanceId]>=(WITH a(ECClassId) AS (SELECT [a].[ECClassId] FROM (SELECT [Id] ECInstanceId,44 ECClassId FROM [main].[ec_Property]) [a])\nSELECT [a].[ECClassId] FROM a)  LIMIT 1" );
    }
    if ("simple_select_cte_subquery_with_multiple_column_checks") {
        auto ecsql = R"(select * from meta.ECClassDef where ECInstanceId >= ALL(with a(a,b,c,d,e,f,g) AS (select * from meta.ClassHasBaseClasses) select * from a) LIMIT 1)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_STREQ(stmt.GetNativeSql(),"SELECT [ECClassDef].[ECInstanceId],[ECClassDef].[ECClassId],[ECClassDef].[SchemaId],[ECClassDef].[SchemaRelECClassId],[ECClassDef].[Name],[ECClassDef].[DisplayLabel],[ECClassDef].[Description],[ECClassDef].[Type],[ECClassDef].[Modifier],[ECClassDef].[CustomAttributeContainerType],[ECClassDef].[RelationshipStrength],[ECClassDef].[RelationshipStrengthDirection] FROM (SELECT [Id] ECInstanceId,37 ECClassId,[SchemaId],(CASE WHEN [SchemaId] IS NULL THEN NULL ELSE 38 END) [SchemaRelECClassId],[Name],[DisplayLabel],[Description],[Type],[Modifier],[CustomAttributeContainerType],[RelationshipStrength],[RelationshipStrengthDirection] FROM [main].[ec_Class]) [ECClassDef] WHERE NOT EXISTS(WITH a(a,b,c,d,e,f,g) AS (SELECT [ClassHasBaseClasses].[ECInstanceId],[ClassHasBaseClasses].[ECClassId],[ClassHasBaseClasses].[Ordinal],[ClassHasBaseClasses].[SourceECInstanceId],[ClassHasBaseClasses].[SourceECClassId],[ClassHasBaseClasses].[TargetECInstanceId],[ClassHasBaseClasses].[TargetECClassId] FROM (SELECT [ec_ClassHasBaseClasses].[Id] [ECInstanceId],42 [ECClassId],[ec_ClassHasBaseClasses].[ClassId] [SourceECInstanceId],37 [SourceECClassId],[ec_ClassHasBaseClasses].[BaseClassId] [TargetECInstanceId],37 [TargetECClassId],[Ordinal] FROM [main].[ec_ClassHasBaseClasses]) [ClassHasBaseClasses])\nSELECT a.a,a.b,a.c,a.d,a.e,a.f,a.g FROM a WHERE a.a >= [ECClassDef].[ECInstanceId] AND a.b >= [ECClassDef].[ECInstanceId] AND a.c >= [ECClassDef].[ECInstanceId] AND a.d >= [ECClassDef].[ECInstanceId] AND a.e >= [ECClassDef].[ECInstanceId] AND a.f >= [ECClassDef].[ECInstanceId] AND a.g >= [ECClassDef].[ECInstanceId])  LIMIT 1" );
    }
    if ("simple_select_subquery_with_multiple_column_checks") {
        auto ecsql = R"(
            select * from meta.ECClassDef where ECInstanceId >= ALL(select * from meta.ClassHasBaseClasses) LIMIT 1
        )";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_STREQ(stmt.GetNativeSql(),"SELECT [ECClassDef].[ECInstanceId],[ECClassDef].[ECClassId],[ECClassDef].[SchemaId],[ECClassDef].[SchemaRelECClassId],[ECClassDef].[Name],[ECClassDef].[DisplayLabel],[ECClassDef].[Description],[ECClassDef].[Type],[ECClassDef].[Modifier],[ECClassDef].[CustomAttributeContainerType],[ECClassDef].[RelationshipStrength],[ECClassDef].[RelationshipStrengthDirection] FROM (SELECT [Id] ECInstanceId,37 ECClassId,[SchemaId],(CASE WHEN [SchemaId] IS NULL THEN NULL ELSE 38 END) [SchemaRelECClassId],[Name],[DisplayLabel],[Description],[Type],[Modifier],[CustomAttributeContainerType],[RelationshipStrength],[RelationshipStrengthDirection] FROM [main].[ec_Class]) [ECClassDef] WHERE NOT EXISTS(SELECT [ClassHasBaseClasses].[ECInstanceId],[ClassHasBaseClasses].[ECClassId],[ClassHasBaseClasses].[Ordinal],[ClassHasBaseClasses].[SourceECInstanceId],[ClassHasBaseClasses].[SourceECClassId],[ClassHasBaseClasses].[TargetECInstanceId],[ClassHasBaseClasses].[TargetECClassId] FROM (SELECT [ec_ClassHasBaseClasses].[Id] [ECInstanceId],42 [ECClassId],[ec_ClassHasBaseClasses].[ClassId] [SourceECInstanceId],37 [SourceECClassId],[ec_ClassHasBaseClasses].[BaseClassId] [TargetECInstanceId],37 [TargetECClassId],[Ordinal] FROM [main].[ec_ClassHasBaseClasses]) [ClassHasBaseClasses] WHERE [ClassHasBaseClasses].[ECInstanceId] >= [ECClassDef].[ECInstanceId] AND [ClassHasBaseClasses].[ECClassId] >= [ECClassDef].[ECInstanceId] AND [ClassHasBaseClasses].[Ordinal] >= [ECClassDef].[ECInstanceId] AND [ClassHasBaseClasses].[SourceECInstanceId] >= [ECClassDef].[ECInstanceId] AND [ClassHasBaseClasses].[SourceECClassId] >= [ECClassDef].[ECInstanceId] AND [ClassHasBaseClasses].[TargetECInstanceId] >= [ECClassDef].[ECInstanceId] AND [ClassHasBaseClasses].[TargetECClassId] >= [ECClassDef].[ECInstanceId])  LIMIT 1");
    }
    if ("checking_result_equality_between_simple_select_and_cte_in_subquery_for_multiple_column_checks") {
        auto ecsqlCTE = R"(select * from meta.ECClassDef where ECInstanceId >= ALL(with a(a,b,c,d,e,f,g) AS (select * from meta.ClassHasBaseClasses) select * from a) LIMIT 1)";
        ECSqlStatement stmtCTE;
        ASSERT_EQ(ECSqlStatus::Success, stmtCTE.Prepare(m_ecdb, ecsqlCTE));

        auto ecsqlSelect = R"(
            select * from meta.ECClassDef where ECInstanceId >= ALL(select * from meta.ClassHasBaseClasses) LIMIT 1
        )";
        ECSqlStatement  stmtSelect;
        ASSERT_EQ(ECSqlStatus::Success,  stmtSelect.Prepare(m_ecdb, ecsqlSelect));
        EXPECT_STREQ(stmtCTE.GetColumnInfo(0).GetProperty()->GetName().c_str(), stmtSelect.GetColumnInfo(0).GetProperty()->GetName().c_str());
        EXPECT_STREQ(stmtCTE.GetColumnInfo(1).GetProperty()->GetName().c_str(), stmtSelect.GetColumnInfo(1).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmtCTE.Step());
        ASSERT_EQ(BE_SQLITE_ROW, stmtSelect.Step());
        ASSERT_EQ(stmtSelect.GetValueInt(0), stmtCTE.GetValueInt(0));
        ASSERT_EQ(stmtSelect.GetValueInt(1), stmtCTE.GetValueInt(1));
    }
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CommonTableExpTestFixture, CTE_Without_Columns_Subquery_Tests) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("CTEWithoutColumnsSubqueryTests.ecdb"));

    if ("checking_result_equality_between_simple_select_and_cte_without_columns_in_subquery") {
        auto ecsqlSelect = R"(select * from meta.ECClassDef where ECInstanceId >= SOME(with a AS (select ECClassId from meta.ECPropertyDef) select * from a))";
        ECSqlStatement stmtSelect;
        ASSERT_EQ(ECSqlStatus::Success, stmtSelect.Prepare(m_ecdb, ecsqlSelect));
        ASSERT_EQ(11, stmtSelect.GetColumnCount());
        

        auto ecsqlCTE = R"(
            select * from meta.ECClassDef where ECInstanceId >= SOME(select ECClassId from meta.ECPropertyDef)
        )";
        ECSqlStatement stmtCTE;
        ASSERT_EQ(ECSqlStatus::Success, stmtCTE.Prepare(m_ecdb, ecsqlCTE));
        ASSERT_EQ(11, stmtCTE.GetColumnCount());
        EXPECT_STREQ(stmtCTE.GetColumnInfo(0).GetProperty()->GetName().c_str(), stmtSelect.GetColumnInfo(0).GetProperty()->GetName().c_str());
        EXPECT_STREQ(stmtCTE.GetColumnInfo(1).GetProperty()->GetName().c_str(), stmtSelect.GetColumnInfo(1).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmtCTE.Step());
        ASSERT_EQ(BE_SQLITE_ROW, stmtSelect.Step());
        ASSERT_EQ(stmtSelect.GetValueInt(0), stmtCTE.GetValueInt(0));
        ASSERT_EQ(stmtSelect.GetValueInt(1), stmtCTE.GetValueInt(1));
    }
    if ("checking_result_equality_between_simple_select_and_cte_without_columns_in_subquery_for_multiple_column_checks") {
        auto ecsqlCTE = R"(select * from meta.ECClassDef where ECInstanceId >= ALL(with a AS (select * from meta.ClassHasBaseClasses) select * from a))";
        ECSqlStatement stmtCTE;
        ASSERT_EQ(ECSqlStatus::Success, stmtCTE.Prepare(m_ecdb, ecsqlCTE));
        ASSERT_EQ(11, stmtCTE.GetColumnCount());

        auto ecsqlSelect = R"(
            select * from meta.ECClassDef where ECInstanceId >= ALL(select * from meta.ClassHasBaseClasses)
        )";
        ECSqlStatement  stmtSelect;
        ASSERT_EQ(ECSqlStatus::Success,  stmtSelect.Prepare(m_ecdb, ecsqlSelect));
        ASSERT_EQ(11, stmtSelect.GetColumnCount());
        EXPECT_STREQ(stmtCTE.GetColumnInfo(0).GetProperty()->GetName().c_str(), stmtSelect.GetColumnInfo(0).GetProperty()->GetName().c_str());
        EXPECT_STREQ(stmtCTE.GetColumnInfo(1).GetProperty()->GetName().c_str(), stmtSelect.GetColumnInfo(1).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmtCTE.Step());
        ASSERT_EQ(BE_SQLITE_ROW, stmtSelect.Step());
        ASSERT_EQ(stmtSelect.GetValueInt(0), stmtCTE.GetValueInt(0));
        ASSERT_EQ(stmtSelect.GetValueInt(1), stmtCTE.GetValueInt(1));
    }
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CommonTableExpTestFixture, CTE_Without_SubColumns) {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("CTEWithoutSubColumns.ecdb", SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
    <ECSchema schemaName='TestSchema' alias='ts' version='10.10.10' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
        <ECEntityClass typeName='Element' >
            <ECProperty propertyName="Subject" typeName="string" description="" />
            <ECNavigationProperty propertyName="Parent" description="" relationshipName="ElementOwnsChildElements" direction="backward">
                <ECCustomAttributes>
                   <HiddenProperty xmlns="CoreCustomAttributes.01.00.03"/>
                   <ForeignKeyConstraint xmlns="ECDbMap.02.00.00">
                        <OnDeleteAction>NoAction</OnDeleteAction>
                   </ForeignKeyConstraint>
                </ECCustomAttributes>
            </ECNavigationProperty>
        </ECEntityClass>
        <ECRelationshipClass typeName="ElementOwnsChildElements" description="" modifier="None" strength="embedding">
            <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
                <Class class="Element"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
                <Class class="Element"/>
            </Target>
        </ECRelationshipClass>
    </ECSchema>)xml")));

    ECSqlStatementCache cache(20);
    auto relClassId = m_ecdb.Schemas().GetClassId("TestSchema", "ElementOwnsChildElements");

    auto findElementBySubject = [&](BeInt64Id parentId, Utf8CP subject) {
        auto stmt = parentId.IsValid() ? cache.GetPreparedStatement(m_ecdb, "SELECT  ECInstanceId FROM ts.Element WHERE Parent.Id = ? AND Subject = ?") : cache.GetPreparedStatement(m_ecdb, "SELECT  ECInstanceId FROM ts.Element WHERE Parent.Id IS NULL AND Subject = ?");
        if (parentId.IsValid()) {
            stmt->BindId(1, BeInt64Id(parentId));
            stmt->BindText(2, subject, IECSqlBinder::MakeCopy::No);
        } else {
            stmt->BindText(1, subject, IECSqlBinder::MakeCopy::No);
        }

        if (BE_SQLITE_ROW == stmt->Step()) {
            return stmt->GetValueId<BeInt64Id>(0);
        }
        return BeInt64Id(0);
    };

    auto addElement = [&](Utf8CP subject, BeInt64Id parentId) {
        auto subjectId = findElementBySubject(parentId, subject);
        if (subjectId.IsValid()) {
            return subjectId;
        }

        auto stmt = cache.GetPreparedStatement(m_ecdb, "INSERT INTO ts.Element(Parent, Subject) VALUES(?, ?)");
        if (parentId.IsValid()) {
            stmt->BindNavigationValue(1, parentId, relClassId);
        }

        stmt->BindText(2, subject, IECSqlBinder::MakeCopy::No);
        ECInstanceKey key;
        if (stmt->Step(key) != BE_SQLITE_DONE) {
            return BeInt64Id(0);
        }

        return (BeInt64Id)key.GetInstanceId();
    };

    auto addElementPath = [&](Utf8CP path, Utf8CP delimiter = "/") {
        bvector<Utf8String> subjects;
        BeStringUtilities::Split(path, delimiter, subjects);
        BeInt64Id parentId(0);
        for(auto& subject : subjects) {
            parentId = addElement(subject.c_str(), parentId);
        }
        return parentId;
    };

    addElementPath("Drive/Document/Doc1");
    addElementPath("Drive/Document/Doc2");
    addElementPath("Drive/Document/Doc3");
    addElementPath("Drive/Pictures/Pic1");
    addElementPath("Drive/Pictures/Pic2");
    addElementPath("Book/SciFi/Book1");

    /*
    Id Subject  ParentId Depth
    -- -------- -------- ------
     1 Drive      (null)  0
     2 Document        1  1
     3 Doc1            2  2
     4 Doc2            2  2
     5 Doc3            2  2
     6 Pictures        1  1
     7 Pic1            6  2
     8 Pic2            6  2
     9 Book       (null)  0
    10 SciFi           9  1
    11 Book1          10  2
*/

    if ("simple_select_cte") {
        auto ecsql = R"(with cte as (select * from ts.Element) select * from cte)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(4, stmt.GetColumnCount());
        ASSERT_STREQ("ECInstanceId", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_STREQ("ECClassId", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        ASSERT_STREQ("Subject", stmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
        ASSERT_STREQ("Parent", stmt.GetColumnInfo(3).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0));
        ASSERT_STREQ("Drive", stmt.GetValueText(2));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(2, stmt.GetValueInt(0));
        ASSERT_STREQ("Document", stmt.GetValueText(2));
    }
    if ("simple_select_cte_wit_defined_columns_inside") {
        auto ecsql = R"(with cte as (select Subject, Parent.Id PiD from ts.Element) select * from cte)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(2, stmt.GetColumnCount());
        ASSERT_STREQ("Subject", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_STREQ("PiD", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("Drive", stmt.GetValueText(0));
        ASSERT_EQ(true, stmt.IsValueNull(1));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("Document", stmt.GetValueText(0));
        ASSERT_EQ(1, stmt.GetValueInt(1));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("Doc1", stmt.GetValueText(0));
        ASSERT_EQ(2, stmt.GetValueInt(1));
    }
    if ("selecting_*_inside_with_sepcified_columns_outside") {
        auto ecsql = R"(with cte as (select * from ts.Element) select Subject, Parent.Id pId from cte)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(2, stmt.GetColumnCount());
        ASSERT_STREQ("Subject", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_STREQ("pId", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("Drive", stmt.GetValueText(0));
        ASSERT_EQ(true, stmt.IsValueNull(1));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("Document", stmt.GetValueText(0));
        ASSERT_EQ(1, stmt.GetValueInt(1));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("Doc1", stmt.GetValueText(0));
        ASSERT_EQ(2, stmt.GetValueInt(1));
    }
    if ("multiple_cte_blocks_without_columns") {
        auto ecsql = R"(with a AS (select * from ts.Element), b AS (select Parent.Id pId from ts.Element) select a.ECInstanceId, Subject, pId from a,b)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(3, stmt.GetColumnCount());
        ASSERT_STREQ("ECInstanceId", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_STREQ("Subject", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        ASSERT_STREQ("pId", stmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0));
        ASSERT_STREQ("Drive", stmt.GetValueText(1));
        ASSERT_EQ(true, stmt.IsValueNull(2));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0));
        ASSERT_STREQ("Drive", stmt.GetValueText(1));
        ASSERT_EQ(1, stmt.GetValueInt(2));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0));
        ASSERT_STREQ("Drive", stmt.GetValueText(1));
        ASSERT_EQ(2, stmt.GetValueInt(2));
    }
    if ("multiple_cte_blocks_with_columns_&_without_columns") {
        auto ecsql = R"(with a AS (select * from ts.Element), b(Id) AS (select Parent.Id from ts.Element) select ECInstanceId, Subject, Id pId from a,b)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(3, stmt.GetColumnCount());
        ASSERT_STREQ("ECInstanceId", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_STREQ("Subject", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        ASSERT_STREQ("pId", stmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0));
        ASSERT_STREQ("Drive", stmt.GetValueText(1));
        ASSERT_EQ(true, stmt.IsValueNull(2));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0));
        ASSERT_STREQ("Drive", stmt.GetValueText(1));
        ASSERT_EQ(1, stmt.GetValueInt(2));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0));
        ASSERT_STREQ("Drive", stmt.GetValueText(1));
        ASSERT_EQ(2, stmt.GetValueInt(2));
    }
    if ("multiple_cte_blocks_with_columns_&_without_columns_ambiguous_columns") {
        auto ecsql = R"(with a AS (select * from ts.Element), b(ECInstanceId) AS (select Parent.Id from ts.Element) select a.ECInstanceId, Subject, b.ECInstanceId pId from a,b)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(3, stmt.GetColumnCount());
        ASSERT_STREQ("ECInstanceId", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_STREQ("Subject", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        ASSERT_STREQ("pId", stmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0));
        ASSERT_STREQ("Drive", stmt.GetValueText(1));
        ASSERT_EQ(true, stmt.IsValueNull(2));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0));
        ASSERT_STREQ("Drive", stmt.GetValueText(1));
        ASSERT_EQ(1, stmt.GetValueInt(2));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0));
        ASSERT_STREQ("Drive", stmt.GetValueText(1));
        ASSERT_EQ(2, stmt.GetValueInt(2));
    }
    if ("alias_cte_without_subColumns") {
        auto ecsql = R"(with cte0 as ( select 100,200) select * from (select * from cte0 c0))";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(2, stmt.GetColumnCount());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(100, stmt.GetValueInt(0));
        ASSERT_EQ(200, stmt.GetValueInt(1));
    }
    if ("alias_cte_without_subColumns_with_column_alias") {
        auto ecsqlalias_1 = R"(with cte0 as ( select 100 a,200 b) select * from (select * from cte0 c0 where c0.a = 100 and c0.b = 200))";
        ECSqlStatement stmt_alias_1;
        ASSERT_EQ(ECSqlStatus::Success, stmt_alias_1.Prepare(m_ecdb, ecsqlalias_1));
        ASSERT_EQ(2, stmt_alias_1.GetColumnCount());

        auto ecsqlalias_2 = R"(with cte0 as ( select 100 a,200 b) select * from (select * from cte0 where cte0.a = 100 and b = 200))";
        ECSqlStatement stmt_alias_2;
        ASSERT_EQ(ECSqlStatus::Success, stmt_alias_2.Prepare(m_ecdb, ecsqlalias_2));
        ASSERT_EQ(2, stmt_alias_2.GetColumnCount());

        ASSERT_STREQ(stmt_alias_1.GetColumnInfo(0).GetProperty()->GetName().c_str(), stmt_alias_2.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_STREQ(stmt_alias_1.GetColumnInfo(1).GetProperty()->GetName().c_str(), stmt_alias_2.GetColumnInfo(1).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt_alias_1.Step());
        ASSERT_EQ(BE_SQLITE_ROW, stmt_alias_2.Step());
        ASSERT_EQ(stmt_alias_1.GetValueInt(0), stmt_alias_2.GetValueInt(0));
        ASSERT_EQ(stmt_alias_1.GetValueInt(1), stmt_alias_2.GetValueInt(1));

        
    }
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CommonTableExpTestFixture, FindingProperty_For_CTE_Without_SubColumns) {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("CTEWithoutSubColumns.ecdb", SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
    <ECSchema schemaName='TestSchema' alias='ts' version='10.10.10' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
        <ECEntityClass typeName='Element' >
            <ECProperty propertyName="Subject" typeName="string" description="" />
            <ECNavigationProperty propertyName="Parent" description="" relationshipName="ElementOwnsChildElements" direction="backward">
                <ECCustomAttributes>
                   <HiddenProperty xmlns="CoreCustomAttributes.01.00.03"/>
                   <ForeignKeyConstraint xmlns="ECDbMap.02.00.00">
                        <OnDeleteAction>NoAction</OnDeleteAction>
                   </ForeignKeyConstraint>
                </ECCustomAttributes>
            </ECNavigationProperty>
        </ECEntityClass>
        <ECRelationshipClass typeName="ElementOwnsChildElements" description="" modifier="None" strength="embedding">
            <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
                <Class class="Element"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
                <Class class="Element"/>
            </Target>
        </ECRelationshipClass>
    </ECSchema>)xml")));

    ECSqlStatementCache cache(20);
    auto relClassId = m_ecdb.Schemas().GetClassId("TestSchema", "ElementOwnsChildElements");

    auto findElementBySubject = [&](BeInt64Id parentId, Utf8CP subject) {
        auto stmt = parentId.IsValid() ? cache.GetPreparedStatement(m_ecdb, "SELECT  ECInstanceId FROM ts.Element WHERE Parent.Id = ? AND Subject = ?") : cache.GetPreparedStatement(m_ecdb, "SELECT  ECInstanceId FROM ts.Element WHERE Parent.Id IS NULL AND Subject = ?");
        if (parentId.IsValid()) {
            stmt->BindId(1, BeInt64Id(parentId));
            stmt->BindText(2, subject, IECSqlBinder::MakeCopy::No);
        } else {
            stmt->BindText(1, subject, IECSqlBinder::MakeCopy::No);
        }

        if (BE_SQLITE_ROW == stmt->Step()) {
            return stmt->GetValueId<BeInt64Id>(0);
        }
        return BeInt64Id(0);
    };

    auto addElement = [&](Utf8CP subject, BeInt64Id parentId) {
        auto subjectId = findElementBySubject(parentId, subject);
        if (subjectId.IsValid()) {
            return subjectId;
        }

        auto stmt = cache.GetPreparedStatement(m_ecdb, "INSERT INTO ts.Element(Parent, Subject) VALUES(?, ?)");
        if (parentId.IsValid()) {
            stmt->BindNavigationValue(1, parentId, relClassId);
        }

        stmt->BindText(2, subject, IECSqlBinder::MakeCopy::No);
        ECInstanceKey key;
        if (stmt->Step(key) != BE_SQLITE_DONE) {
            return BeInt64Id(0);
        }

        return (BeInt64Id)key.GetInstanceId();
    };

    auto addElementPath = [&](Utf8CP path, Utf8CP delimiter = "/") {
        bvector<Utf8String> subjects;
        BeStringUtilities::Split(path, delimiter, subjects);
        BeInt64Id parentId(0);
        for(auto& subject : subjects) {
            parentId = addElement(subject.c_str(), parentId);
        }
        return parentId;
    };

    addElementPath("Drive/Document/Doc1");
    addElementPath("Drive/Document/Doc2");
    addElementPath("Drive/Document/Doc3");
    addElementPath("Drive/Pictures/Pic1");
    addElementPath("Drive/Pictures/Pic2");
    addElementPath("Book/SciFi/Book1");

    /*
    Id Subject  ParentId Depth
    -- -------- -------- ------
     1 Drive      (null)  0
     2 Document        1  1
     3 Doc1            2  2
     4 Doc2            2  2
     5 Doc3            2  2
     6 Pictures        1  1
     7 Pic1            6  2
     8 Pic2            6  2
     9 Book       (null)  0
    10 SciFi           9  1
    11 Book1          10  2
*/

    if ("simple_select_cte") {
        auto ecsql = R"(with cte as (select * from ts.Element) select * from cte)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(4, stmt.GetColumnCount());
        ASSERT_STREQ("ECInstanceId", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_STREQ("ECClassId", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        ASSERT_STREQ("Subject", stmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
        ASSERT_STREQ("Parent", stmt.GetColumnInfo(3).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0));
        ASSERT_STREQ("Drive", stmt.GetValueText(2));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(2, stmt.GetValueInt(0));
        ASSERT_STREQ("Document", stmt.GetValueText(2));
    }
    if ("simple_select_cte_wit_defined_columns_inside") {
        auto ecsql = R"(with cte as (select Subject from ts.Element) select * from cte)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("Subject", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("Drive", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("Document", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("Doc1", stmt.GetValueText(0));
    }
    if ("selecting_*_inside_with_sepcified_columns_outside") {
        auto ecsql = R"(select Subject from ts.Element where ECInstanceId = (with cte as (select * from ts.Element) select Parent.Id from cte))";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("Subject", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    if ("selecting_*_outside_with_sepcified_columns_inside") {
        auto ecsql = R"(select Subject from ts.Element where ECInstanceId = (with cte as (select Parent.Id from ts.Element) select * from cte))";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("Subject", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    if ("cte_without_subColumns_with WHERE") {
        auto ecsql = R"(with cte as (select * from ts.Element) select Subject from cte where Parent.Id = (select ECInstanceId from ts.Element))";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("Subject", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("Document", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("Pictures", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    if ("cte_without_subColumns_with WHERE_using_alias") {
        auto ecsql = R"(with cte as (select * from ts.Element) select Subject from cte c where c.Parent.Id = (select ECInstanceId from ts.Element))";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("Subject", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("Document", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("Pictures", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    if ("cte_without_subColumns_with WHERE_using_cte_name_as_alias") {
        auto ecsql = R"(with cte as (select * from ts.Element) select Subject from cte where cte.Parent.Id = (select ECInstanceId from ts.Element))";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("Subject", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("Document", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("Pictures", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    if ("cte_without_subColumns_in_subquery_ref_with alias") {
        auto ecsql = R"(select a.x from (with tmp(x) as (SELECT e.Subject FROM ts.Element e order by e.Subject LIMIT 1) select x from tmp) a)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("x", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("Book", stmt.GetValueText(0));
    }
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CommonTableExpTestFixture, Invalid_SQL_Tests) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("InvalidCTETestsDb.ecdb"));

    if ("mismatch_in_columns_within_CTE") {
        auto ecsql = R"(select * from meta.ECClassDef where 
        ECInstanceId >= SOME(with a(ECClassId) AS (select * from meta.ECPropertyDef) select * from a) 
        LIMIT 1)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, ecsql));
    }
    if ("mismatch_in_number_of_columns_CTE") {
        auto ecsql = R"(select * from meta.ECClassDef where ECInstanceId >= ALL(with a(a,b,c,d) AS (select * from meta.ClassHasBaseClasses) select * from a) LIMIT 1)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, ecsql));
    }
    if ("testing_recursive_without_columns_cte") {
        auto ecsql = R"(with recursive a AS (select * from meta.ClassHasBaseClasses) select * from a)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, ecsql));
    }
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CommonTableExpTestFixture, Debug_Tests) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("Debug_Tests.ecdb"));
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM ( SELECT 1 AS KEYID, 'BeepBoo' AS Noise ) [c1] JOIN ( SELECT 1 AS KEYID, 'Robot' AS Name ) [c2] ON c1.KEYID = c2.KEYID"));
        std::cout << stmt.GetNativeSql() << std::endl;
        }
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "WITH [cte1] AS ( SELECT 1 AS KEYID, 'BeepBoo' AS Noise ), [cte2] AS ( SELECT 1 AS KEYID, 'Robot' AS Name ) SELECT * FROM cte1 [c1] JOIN cte2 [c2] ON c1.KEYID = c2.KEYID"));
        std::cout << stmt.GetNativeSql() << std::endl;
        }
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "WITH edges AS (SELECT [sif].[ECInstanceId] AS [Id], [sif].[Parent].[Id] AS [ParentId], [sif].[CodeValue], [sif].[UserLabel], [sif].[EntryPriority] FROM [bis].[SheetIndexFolder] [sif] UNION ALL SELECT [si].[ECInstanceId] AS [Id], [si].[Parent].[Id] AS [ParentId], [si].[CodeValue], [si].[UserLabel], -1 AS [EntryPriority] FROM [bis].[SheetIndex] [si]), nodes AS ( SELECT [sr].[ECInstanceId] AS [Id], [sr].[Parent].[Id] AS [ParentId], [sr].[CodeValue], [sr].[UserLabel], [sr].[EntryPriority] FROM [bis].[SheetReference] [sr] ), joinToParent AS ( SELECT p.Id FROM nodes [p] LEFT JOIN edges [c] ON [p].Id = [c].ParentId) SELECT * FROM joinToParent"));
        std::cout << stmt.GetNativeSql() << std::endl;
        }
}

END_ECDBUNITTESTS_NAMESPACE
