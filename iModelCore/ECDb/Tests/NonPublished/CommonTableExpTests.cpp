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
        ASSERT_STREQ(stmt.GetNativeSql(), "SELECT [K4] FROM (WITH RECURSIVE cte0(a,b) AS (SELECT 100,200)\nSELECT [K2] [K4],[K3] FROM (SELECT c0.a K2,c0.b K3 FROM cte0 c0 WHERE c0.a=100 AND c0.b=200))");
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);
        ASSERT_STREQ("a", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_STREQ("100", stmt.GetValueText(0));
    }
    if ("simple_wild_nested_with_unmatched_values") {
        auto query = R"(select a from(
            with recursive
                cte0 (a,b) as ( select 100,200)
            select * from (select * from cte0 c0 where c0.a=300 and c0.b=400)
        ))";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ(stmt.GetNativeSql(), "SELECT [K4] FROM (WITH RECURSIVE cte0(a,b) AS (SELECT 100,200)\nSELECT [K2] [K4],[K3] FROM (SELECT c0.a K2,c0.b K3 FROM cte0 c0 WHERE c0.a=300 AND c0.b=400))");
        ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
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
    if ("simple_select_cte_wit_defined_columns_outside") {
        auto ecsql = R"(with cte as (select Parent from ts.Element) select Parent.Id PiD from cte)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("PiD", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(true, stmt.IsValueNull(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(2, stmt.GetValueInt(0));
    }
    if ("simple_select_cte_wit_defined_columns_outside_within_subquery") {
        auto ecsql = R"(with cte as (select Parent from ts.Element) select * from (select Parent.Id PiD from cte))";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("PiD", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(true, stmt.IsValueNull(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(2, stmt.GetValueInt(0));
    }
    if ("simple_select_cte_wit_defined_columns_outside_within_subquery_with_lias") {
        auto ecsql = R"(with cte as (select Parent from ts.Element) select * from (select Parent.Id PiD from cte) X)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("PiD", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(true, stmt.IsValueNull(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(2, stmt.GetValueInt(0));
    }
    if ("simple_select_cte_wit_defined_columns_outside_within_multiple_subquery") {
        auto ecsql = R"(with cte as (select Parent from ts.Element) select * from (select * from (select Parent.Id PiD from cte)))";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("PiD", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(true, stmt.IsValueNull(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(2, stmt.GetValueInt(0));
    }
    if ("simple_select_cte_wit_defined_columns_outside_within_multiple_subquery_with_first_asterisk") {
        auto ecsql = R"(with cte as (select Parent from ts.Element) select * from (select PiD from (select Parent.Id PiD from cte)))";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("PiD", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(true, stmt.IsValueNull(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(2, stmt.GetValueInt(0));
    }
    if ("simple_select_cte_wit_defined_columns_outside_within_multiple_subquery_with_middle_asterisk") {
        auto ecsql = R"(with cte as (select Parent from ts.Element) select PiD from (select * from (select Parent.Id PiD from cte)))";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("PiD", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(true, stmt.IsValueNull(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(2, stmt.GetValueInt(0));
    }
    if ("simple_select_cte_wit_defined_columns_outside_without alias") {
        auto ecsql = R"(with cte as (select Parent from ts.Element) select Parent.Id from cte)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("Id", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(true, stmt.IsValueNull(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(2, stmt.GetValueInt(0));
    }
    if ("simple_select_cte_wit_defined_columns_outside_within_subquery_without_alias") {
        auto ecsql = R"(with cte as (select Parent from ts.Element) select * from (select Parent.Id from cte))";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("Id", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(true, stmt.IsValueNull(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(2, stmt.GetValueInt(0));
    }
    if ("simple_select_cte_wit_defined_columns_outside_within_subquery_with_table_alias_without_column_alias") {
        auto ecsql = R"(with cte as (select Parent from ts.Element) select * from (select Parent.Id from cte) X)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("Id", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(true, stmt.IsValueNull(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(2, stmt.GetValueInt(0));
    }
    if ("simple_select_cte_wit_defined_columns_outside_within_multiple_subquery_without_alias") {
        auto ecsql = R"(with cte as (select Parent from ts.Element) select * from (select * from (select Parent.Id from cte)))";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("Id", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(true, stmt.IsValueNull(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(2, stmt.GetValueInt(0));
    }
    if ("simple_select_cte_wit_defined_columns_outside_within_multiple_subquery_with_first_asterisk_without_alias") {
        auto ecsql = R"(with cte as (select Parent from ts.Element) select * from (select Parent.Id from (select Parent.Id from cte)))";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("Id", stmt.GetColumnInfo(0).GetProperty()->GetDisplayLabel().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(true, stmt.IsValueNull(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(2, stmt.GetValueInt(0));
    }
    if ("simple_select_cte_wit_defined_columns_outside_within_multiple_subquery_with_middle_asterisk_without_alias") {
        auto ecsql = R"(with cte as (select Parent from ts.Element) select Parent.Id from (select * from (select Parent.Id from cte)))";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("Id", stmt.GetColumnInfo(0).GetProperty()->GetDisplayLabel().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(true, stmt.IsValueNull(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(2, stmt.GetValueInt(0));
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
    if ("selecting_*_inside_with_sepcified_columns_outside_within_subquery") {
        auto ecsql = R"(with cte as (select * from ts.Element) select * from (select Subject, Parent.Id pId from cte))";
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
    if ("selecting_*_inside_with_sepcified_columns_outside_within_subquery_with alias") {
        auto ecsql = R"(with cte as (select * from ts.Element) select * from (select Subject, Parent.Id pId from cte) X)";
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
TEST_F(CommonTableExpTestFixture, asterisk_resolution_in_cte) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, OpenECDbTestDataFile("test.bim"));

    if("using table alias while asterisk resolution for cte with sub columns"){
        auto ecsql = R"(WITH e(a,b) AS (SELECT f.* FROM (select 100, 200) f) SELECT a, b FROM e)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(2, stmt.GetColumnCount());
        ASSERT_STREQ("a", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_STREQ("b", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("100", stmt.GetValueText(0));
        ASSERT_STREQ("200", stmt.GetValueText(1));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    if("using table alias while asterisk resolution for cte without sub columns"){
        auto ecsql = R"(WITH e AS (SELECT f.* FROM (select 100, 200) f) SELECT * FROM e)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(2, stmt.GetColumnCount());
        ASSERT_STREQ("100", stmt.GetColumnInfo(0).GetProperty()->GetDisplayLabel().c_str());
        ASSERT_STREQ("200", stmt.GetColumnInfo(1).GetProperty()->GetDisplayLabel().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("100", stmt.GetValueText(0));
        ASSERT_STREQ("200", stmt.GetValueText(1));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    if("using table alias while asterisk resolution and selecting SELECT statements"){
        auto ecsql = R"(WITH e AS (SELECT f.* FROM Bis.Element f) SELECT (SELECT ECInstanceId FROM Bis.Model m WHERE m.ECInstanceId = e.Model.Id LIMIT 3) FROM e)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("1", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("1", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("1", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    }
    if("using table alias while asterisk resolution for cte without sub columns and selecting compound prop with limit outside"){
        auto ecsql = R"( WITH e AS (SELECT f.* FROM Bis.Element f) SELECT Model.Id FROM e limit 1)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("1", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    if("using table alias while asterisk resolution for cte without sub columns subquery and selecting compound prop"){
        auto ecsql = R"( select * from (WITH e AS (SELECT f.* FROM Bis.Element f) SELECT Model.Id FROM e limit 1))";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("1", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    if("using table alias while asterisk resolution for cte without sub columns subquery and selecting compound prop with limit inside"){
        auto ecsql = R"( WITH e AS (SELECT f.* FROM Bis.Element f limit 1) SELECT Model.Id FROM e)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("1", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    if("using table alias while asterisk resolution for cte without sub columns aliased subquery"){
        auto ecsql = R"( select a.* from (WITH e AS (SELECT f.* FROM Bis.Element f limit 1) SELECT Model.Id FROM e)a)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("1", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    if("using table alias while asterisk resolution for cte without sub columns subquery and selecting SELECT value exps for aliased ctes"){
        auto ecsql = R"(WITH e AS (SELECT f.* FROM Bis.Element f) SELECT (SELECT ECInstanceId FROM (SELECT C.ECInstanceId FROM Meta.ECClassDef C WHERE C.ECInstanceId = E.ECClassId limit 1)) a FROM e)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("a", stmt.GetColumnInfo(0).GetProperty()->GetDisplayLabel().c_str());
        ASSERT_STREQ("a", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("76", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    }
    if("using table alias while asterisk resolution for cte without sub columns subquery and selecting SELECT value exps for aliased ctes with limit inside"){
        auto ecsql = R"(select g.* from (WITH e AS (SELECT f.* FROM Bis.Element f) SELECT (SELECT ECInstanceId FROM (SELECT C.ECInstanceId FROM Meta.ECClassDef C WHERE C.ECInstanceId = E.ECClassId limit 1)) a FROM e) g)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("a", stmt.GetColumnInfo(0).GetProperty()->GetDisplayLabel().c_str());
        ASSERT_STREQ("a", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("76", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    }
    if("using table alias while asterisk resolution for cte without sub columns selecting SELECT value exps for aliased ctes with limit outside"){
        auto ecsql = R"(WITH e AS (SELECT f.* FROM Bis.Element f) SELECT (SELECT ECInstanceId FROM (SELECT C.ECInstanceId FROM Meta.ECClassDef C WHERE C.ECInstanceId = E.ECClassId) limit 1) a FROM e)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("a", stmt.GetColumnInfo(0).GetProperty()->GetDisplayLabel().c_str());
        ASSERT_STREQ("a", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("76", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    }
    if("using table alias while asterisk resolution for cte without sub columns subquery and selecting SELECT value exps for aliased ctes with limit aliased"){
        auto ecsql = R"(select a from (WITH e AS (SELECT f.* FROM Bis.Element f) SELECT (SELECT ECInstanceId FROM (SELECT C.ECInstanceId FROM Meta.ECClassDef C WHERE C.ECInstanceId = E.ECClassId) limit 1) a FROM e))";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("a", stmt.GetColumnInfo(0).GetProperty()->GetDisplayLabel().c_str());
        ASSERT_STREQ("a", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("76", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    }
    if("using table alias while asterisk resolution for cte without sub columns subquery and aliasing the subquery and also the CTE select query"){
        auto ecsql = R"(select y.a from (WITH e AS (SELECT f.* FROM Bis.Element f) SELECT (SELECT ECInstanceId FROM (SELECT C.ECInstanceId FROM Meta.ECClassDef C WHERE C.ECInstanceId = E.ECClassId) limit 1) a FROM e) y)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("a", stmt.GetColumnInfo(0).GetProperty()->GetDisplayLabel().c_str());
        ASSERT_STREQ("a", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("76", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    }
    if("using table alias while asterisk resolution for cte without sub columns subquery and aliasing the subquery and also the CTE select query and selecting asterisk with alias outside"){
        auto ecsql = R"(select y.* from (WITH e AS (SELECT f.* FROM Bis.Element f) SELECT (SELECT ECInstanceId FROM (SELECT C.ECInstanceId FROM Meta.ECClassDef C WHERE C.ECInstanceId = E.ECClassId) limit 1) a FROM e) y)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("a", stmt.GetColumnInfo(0).GetProperty()->GetDisplayLabel().c_str());
        ASSERT_STREQ("a", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("76", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    }
    if("using asterisk with nav props should fail"){
        auto ecsql = R"(WITH e AS (SELECT Model.* FROM Bis.Element f) SELECT Model FROM e)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, ecsql));
    }
    if("using asterisk with nav props should fail"){
        auto ecsql = R"(WITH e AS (SELECT Model.* FROM Bis.Element f) SELECT * FROM e)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, ecsql));
    }
    if("using asterisk with nav props and aliased class should fail"){
        auto ecsql = R"(WITH e AS (SELECT f.Model.* FROM Bis.Element f) SELECT * FROM e)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, ecsql));
    }
    if("using nav props with cte with sub columns should fail"){
        auto ecsql = R"(WITH e(m) AS (SELECT f.Model FROM Bis.Element f) SELECT * FROM e)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, ecsql));
    }
    if("using nav props with cte with one sub column should fail"){
        auto ecsql = R"(WITH e(m) AS (SELECT f.Model FROM Bis.Element f) SELECT m.Id FROM e)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, ecsql));
    }
    if("using nav props with cte with two sub columns should fail"){
        auto ecsql = R"(WITH e(m, n) AS (SELECT f.Model FROM Bis.Element f) SELECT * FROM e)";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, ecsql));
    }
}
//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CommonTableExpTestFixture, SubQueryBlock_With_cte_with_no_columns) {
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
            WITH models AS (
                SELECT foo.ECInstanceId i FROM ts.Foo foo)
            SELECT i FROM models WHERE models.i = 1
        )";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_STREQ(SqlPrintfString("WITH models AS (SELECT [foo].[ECInstanceId] [K0] FROM (SELECT [Id] ECInstanceId,89 ECClassId FROM [main].[ts_Foo]) [foo])\nSELECT [K0] FROM models WHERE [K0]=1", fooClassId.ToString().c_str()), stmt.GetNativeSql());
    }
    if ("select_property_in_cte_block") {
        auto ecsql = R"(
            WITH models AS (
                SELECT foo.Code i FROM ts.Foo foo)
            SELECT i FROM models WHERE models.i IN (?)
        )";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_STREQ(SqlPrintfString("WITH models AS (SELECT [foo].[Code] [K0] FROM (SELECT [Id] ECInstanceId,89 ECClassId,[Code] FROM [main].[ts_Foo]) [foo])\nSELECT [K0] FROM models WHERE [K0] IN (:_ecdb_sqlparam_ix1_col1)", fooClassId.ToString().c_str()), stmt.GetNativeSql());
    }
    if ("select_id_in_cte_block") {
        auto ecsql = R"(
            WITH models AS (
                SELECT foo.ECInstanceId i FROM ts.Foo foo)
            SELECT i FROM models WHERE models.i IN (?)
        )";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_STREQ(SqlPrintfString("WITH models AS (SELECT [foo].[ECInstanceId] [K0] FROM (SELECT [Id] ECInstanceId,89 ECClassId FROM [main].[ts_Foo]) [foo])\nSELECT [K0] FROM models WHERE [K0] IN (:_ecdb_sqlparam_ix1_col1)", fooClassId.ToString().c_str()), stmt.GetNativeSql());
    }
    if ("nested_select_id_in_cte_block") {
        auto ecsql = R"(
            WITH models AS (
                SELECT (SELECT foo.ECInstanceId i FROM ts.Foo foo) AS ecId)
            SELECT i FROM models WHERE models.i IN (?)
        )";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, ecsql));
    }
    if ("select_link_table_in_cte_block") {
        auto ecsql = R"(
            WITH models AS (
                SELECT
                    r.ECInstanceId i,
                    r.ECClassId c,
                    r.SourceECInstanceId si,
                    r.SourceECClassId sc,
                    r.TargetECInstanceId ti,
                    r.TargetECClassId tc
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
        ASSERT_STREQ(SqlPrintfString("WITH models AS (SELECT [r].[ECInstanceId] [K0],[r].[ECClassId] [K1],[r].[SourceECInstanceId] [K2],[r].[SourceECClassId] [K3],[r].[TargetECInstanceId] [K4],[r].[TargetECClassId] [K5] FROM (SELECT [ts_Rel].[Id] [ECInstanceId],[ts_Rel].[ECClassId],[ts_Rel].[SourceId] [SourceECInstanceId],90 [SourceECClassId],[ts_Rel].[TargetId] [TargetECInstanceId],88 [TargetECClassId] FROM [main].[ts_Rel]) [r])\nSELECT [K0],[K1],[K2],[K3],[K4],[K5] FROM models m WHERE [K0]=:_ecdb_sqlparam_ix1_col1 AND [K1]=:_ecdb_sqlparam_ix2_col1 AND [K2]=:_ecdb_sqlparam_ix3_col1 AND [K3]=:_ecdb_sqlparam_ix4_col1 AND [K4]=:_ecdb_sqlparam_ix5_col1 AND [K5]=:_ecdb_sqlparam_ix6_col1 AND [K0] IN (:_ecdb_sqlparam_ix7_col1) AND [K1] IN (:_ecdb_sqlparam_ix8_col1) AND [K2] IN (:_ecdb_sqlparam_ix9_col1) AND [K3] IN (:_ecdb_sqlparam_ix10_col1) AND [K4] IN (:_ecdb_sqlparam_ix11_col1) AND [K5] IN (:_ecdb_sqlparam_ix12_col1)",
                    parentClassId.ToString().c_str(), childClassId.ToString().c_str()), stmt.GetNativeSql());
    }
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CommonTableExpTestFixture, FindingCompoundDataProperty_For_CTE_Without_SubColumns) {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("FindingCompoundDataProperty_For_CTE_Without_SubColumns.ecdb", SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
    <ECSchema schemaName='TestSchema' alias='ts' version='10.10.10' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
        <ECEntityClass typeName='Element' >
            <ECProperty propertyName="p2d" typeName="point2d"/>
            <ECProperty propertyName="p3d" typeName="point3d"/>
        </ECEntityClass>
    </ECSchema>)xml")));
    ECSqlStatementCache cache(5);
    std::vector<std::vector<double>> pointList = {{200.0,-440.0,345.6}, {300.0,-240.0,120.0}, {220.0,-180.0,330.6}, {270.0,-450.0,340.7}, {200.0,-230.0,220.5} };

    auto addElement = [&](DPoint2d point2d, DPoint3d point3d) {
        auto stmt = cache.GetPreparedStatement(m_ecdb, "INSERT INTO ts.Element(p2d, p3d) VALUES(?, ?)");
        stmt->BindPoint2d(1, point2d);

        stmt->BindPoint3d(2, point3d);
        ECInstanceKey key;
        if (stmt->Step(key) != BE_SQLITE_DONE) {
            return BeInt64Id(0);
        }

        return (BeInt64Id)key.GetInstanceId();
    };
    for(std::vector<double> vec: pointList)
    {
        addElement(DPoint2d::From(vec[0], vec[1]), DPoint3d::From(vec[0], vec[1], vec[2]));
    }
    if("selecting point2d from inside cte"){
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus:: Success, stmt.Prepare(m_ecdb, "WITH cte AS (SELECT p2d FROM ts.Element) SELECT * FROM cte"));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("p2d", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(DPoint2d::From(pointList[0][0], pointList[0][1]), stmt.GetValuePoint2d(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(DPoint2d::From(pointList[1][0], pointList[1][1]), stmt.GetValuePoint2d(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(DPoint2d::From(pointList[2][0], pointList[2][1]), stmt.GetValuePoint2d(0));
    }
    if("selecting point2d from inside cte and aliasing it outside"){
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus:: Success, stmt.Prepare(m_ecdb, "WITH cte AS (SELECT p2d FROM ts.Element) SELECT cte.p2d FROM cte"));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("p2d", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(DPoint2d::From(pointList[0][0], pointList[0][1]), stmt.GetValuePoint2d(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(DPoint2d::From(pointList[1][0], pointList[1][1]), stmt.GetValuePoint2d(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(DPoint2d::From(pointList[2][0], pointList[2][1]), stmt.GetValuePoint2d(0));
    }
    if("selecting point2d from inside cte and aliasing it outside with class aliased"){
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus:: Success, stmt.Prepare(m_ecdb, "WITH cte AS (SELECT p2d FROM ts.Element) SELECT c.p2d FROM cte c"));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("p2d", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(DPoint2d::From(pointList[0][0], pointList[0][1]), stmt.GetValuePoint2d(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(DPoint2d::From(pointList[1][0], pointList[1][1]), stmt.GetValuePoint2d(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(DPoint2d::From(pointList[2][0], pointList[2][1]), stmt.GetValuePoint2d(0));
    }
    if("selecting point2d from inside cte and selecting point2d X prop outside"){
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus:: Success, stmt.Prepare(m_ecdb, "WITH cte AS (SELECT p2d FROM ts.Element) SELECT p2d.X FROM cte"));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("X", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("200.0", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("300.0", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("220.0", stmt.GetValueText(0));
    }
    if("selecting point2d from inside cte and selecting point2d X prop outside with class"){
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus:: Success, stmt.Prepare(m_ecdb, "WITH cte AS (SELECT p2d FROM ts.Element) SELECT cte.p2d.X FROM cte"));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("X", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("200.0", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("300.0", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("220.0", stmt.GetValueText(0));
    }
    if("selecting point2d X prop from inside cte and selecting same outside"){
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus:: Success, stmt.Prepare(m_ecdb, "WITH cte AS (SELECT p2d.X FROM ts.Element) SELECT cte.p2d.X FROM cte"));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("X", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("200.0", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("300.0", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("220.0", stmt.GetValueText(0));
    }
    if("selecting point2d from inside cte and selecting point2d X prop outside with aliased class"){
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus:: Success, stmt.Prepare(m_ecdb, "WITH cte AS (SELECT p2d FROM ts.Element) SELECT c.p2d.X FROM cte c"));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("X", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("200.0", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("300.0", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("220.0", stmt.GetValueText(0));
    }
    if("selecting point2d from inside cte and selecting point2d Y prop outside"){
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus:: Success, stmt.Prepare(m_ecdb, "WITH cte AS (SELECT p2d FROM ts.Element) SELECT p2d.Y FROM cte"));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("Y", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("-440.0", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("-240.0", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("-180.0", stmt.GetValueText(0));
    }
    if("selecting point2d from inside cte and selecting point2d Y prop outside with class"){
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus:: Success, stmt.Prepare(m_ecdb, "WITH cte AS (SELECT p2d FROM ts.Element) SELECT cte.p2d.Y FROM cte"));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("Y", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("-440.0", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("-240.0", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("-180.0", stmt.GetValueText(0));
    }
    if("selecting point2d from inside cte and selecting point2d Y prop outside with aliased class"){
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus:: Success, stmt.Prepare(m_ecdb, "WITH cte AS (SELECT p2d FROM ts.Element) SELECT c.p2d.Y FROM cte c"));
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_STREQ("Y", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("-440.0", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("-240.0", stmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("-180.0", stmt.GetValueText(0));
    }
    if("selecting point3d from inside cte and selecting point3d all prop outside"){
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus:: Success, stmt.Prepare(m_ecdb, "WITH cte AS (SELECT p3d FROM ts.Element) SELECT p3d.X, p3d.Y, p3d.Z FROM cte"));
        ASSERT_EQ(3, stmt.GetColumnCount());
        ASSERT_STREQ("X", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_STREQ("Y", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        ASSERT_STREQ("Z", stmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("200.0", stmt.GetValueText(0));
        ASSERT_STREQ("-440.0", stmt.GetValueText(1));
        ASSERT_STREQ("345.6", stmt.GetValueText(2));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("300.0", stmt.GetValueText(0));
        ASSERT_STREQ("-240.0", stmt.GetValueText(1));
        ASSERT_STREQ("120.0", stmt.GetValueText(2));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("220.0", stmt.GetValueText(0));
        ASSERT_STREQ("-180.0", stmt.GetValueText(1));
        ASSERT_STREQ("330.6", stmt.GetValueText(2));
    }
    if("selecting point3d from inside cte and selecting point3d all prop outside with class"){
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus:: Success, stmt.Prepare(m_ecdb, "WITH cte AS (SELECT p3d FROM ts.Element) SELECT cte.p3d.X, cte.p3d.Y, cte.p3d.Z FROM cte"));
        ASSERT_EQ(3, stmt.GetColumnCount());
        ASSERT_STREQ("X", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_STREQ("Y", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        ASSERT_STREQ("Z", stmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("200.0", stmt.GetValueText(0));
        ASSERT_STREQ("-440.0", stmt.GetValueText(1));
        ASSERT_STREQ("345.6", stmt.GetValueText(2));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("300.0", stmt.GetValueText(0));
        ASSERT_STREQ("-240.0", stmt.GetValueText(1));
        ASSERT_STREQ("120.0", stmt.GetValueText(2));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("220.0", stmt.GetValueText(0));
        ASSERT_STREQ("-180.0", stmt.GetValueText(1));
        ASSERT_STREQ("330.6", stmt.GetValueText(2));
    }
    if("selecting point3d from inside cte and selecting point3d all prop outside with aliased class"){
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus:: Success, stmt.Prepare(m_ecdb, "WITH cte AS (SELECT p3d FROM ts.Element) SELECT c.p3d.X, c.p3d.Y, c.p3d.Z FROM cte c"));
        ASSERT_EQ(3, stmt.GetColumnCount());
        ASSERT_STREQ("X", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_STREQ("Y", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        ASSERT_STREQ("Z", stmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("200.0", stmt.GetValueText(0));
        ASSERT_STREQ("-440.0", stmt.GetValueText(1));
        ASSERT_STREQ("345.6", stmt.GetValueText(2));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("300.0", stmt.GetValueText(0));
        ASSERT_STREQ("-240.0", stmt.GetValueText(1));
        ASSERT_STREQ("120.0", stmt.GetValueText(2));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("220.0", stmt.GetValueText(0));
        ASSERT_STREQ("-180.0", stmt.GetValueText(1));
        ASSERT_STREQ("330.6", stmt.GetValueText(2));
    }
    if("selecting point3d all props from inside cte and selecting same outside"){
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus:: Success, stmt.Prepare(m_ecdb, "WITH cte AS (SELECT p3d.X, p3d.Y, p3d.Z FROM ts.Element) SELECT p3d.X, p3d.Y, p3d.Z FROM cte"));
        ASSERT_EQ(3, stmt.GetColumnCount());
        ASSERT_STREQ("X", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_STREQ("Y", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        ASSERT_STREQ("Z", stmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("200.0", stmt.GetValueText(0));
        ASSERT_STREQ("-440.0", stmt.GetValueText(1));
        ASSERT_STREQ("345.6", stmt.GetValueText(2));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("300.0", stmt.GetValueText(0));
        ASSERT_STREQ("-240.0", stmt.GetValueText(1));
        ASSERT_STREQ("120.0", stmt.GetValueText(2));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("220.0", stmt.GetValueText(0));
        ASSERT_STREQ("-180.0", stmt.GetValueText(1));
        ASSERT_STREQ("330.6", stmt.GetValueText(2));
    }
    if("selecting point3d all props from inside cte and selecting same outside with class"){
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus:: Success, stmt.Prepare(m_ecdb, "WITH cte AS (SELECT p3d.X, p3d.Y, p3d.Z FROM ts.Element) SELECT cte.p3d.X, cte.p3d.Y, cte.p3d.Z FROM cte"));
        ASSERT_EQ(3, stmt.GetColumnCount());
        ASSERT_STREQ("X", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_STREQ("Y", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        ASSERT_STREQ("Z", stmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("200.0", stmt.GetValueText(0));
        ASSERT_STREQ("-440.0", stmt.GetValueText(1));
        ASSERT_STREQ("345.6", stmt.GetValueText(2));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("300.0", stmt.GetValueText(0));
        ASSERT_STREQ("-240.0", stmt.GetValueText(1));
        ASSERT_STREQ("120.0", stmt.GetValueText(2));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("220.0", stmt.GetValueText(0));
        ASSERT_STREQ("-180.0", stmt.GetValueText(1));
        ASSERT_STREQ("330.6", stmt.GetValueText(2));
    }
    if("selecting point3d all props from inside cte and selecting same outside with aliased class"){
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus:: Success, stmt.Prepare(m_ecdb, "WITH cte AS (SELECT p3d.X, p3d.Y, p3d.Z FROM ts.Element) SELECT c.p3d.X, c.p3d.Y, c.p3d.Z FROM cte c"));
        ASSERT_EQ(3, stmt.GetColumnCount());
        ASSERT_STREQ("X", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_STREQ("Y", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        ASSERT_STREQ("Z", stmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("200.0", stmt.GetValueText(0));
        ASSERT_STREQ("-440.0", stmt.GetValueText(1));
        ASSERT_STREQ("345.6", stmt.GetValueText(2));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("300.0", stmt.GetValueText(0));
        ASSERT_STREQ("-240.0", stmt.GetValueText(1));
        ASSERT_STREQ("120.0", stmt.GetValueText(2));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("220.0", stmt.GetValueText(0));
        ASSERT_STREQ("-180.0", stmt.GetValueText(1));
        ASSERT_STREQ("330.6", stmt.GetValueText(2));
    }
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CommonTableExpTestFixture, CTEWithStructBinding)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("InsertWithStructBinding.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb(10));

    ECClassCP pStructClass = m_ecdb.Schemas().GetClass("ECSqlTest", "PStruct");
    ASSERT_TRUE(pStructClass != nullptr && pStructClass->IsStructClass());

    //**** Test 1 *****
    {
    Json::Value expectedJson;
    ASSERT_EQ(SUCCESS, TestUtilities::ParseJson(expectedJson,R"json(
         { "b" : true,
         "d" : 3.0,
         "dt" : "2014-03-27T12:00:00.000",
         "dtUtc" : "2014-03-27T12:00:00.000Z",
         "i" : 44444,
         "l" : 444444444,
         "s" : "Hello, world",
         "p2d" : { "x" : 3.0, "y" : 5.0 },
        "p3d" : { "x" : 3.0, "y" : 5.0, "z" : -6.0}
        })json"));

    ECSqlStatement insertStmt;
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA (I, PStructProp) VALUES (?, ?)"));
    ASSERT_EQ(ECSqlStatus::Success, JsonECSqlBinder::BindStructValue(insertStmt.GetBinder(2), expectedJson, *pStructClass->GetStructClassCP())) << insertStmt.GetECSql();
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step(key)) << insertStmt.GetECSql();
    insertStmt.Finalize();
    {
        ECSqlStatement selStmt;
        ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "WITH cte AS(SELECT PStructProp FROM ecsql.PSA WHERE ECInstanceId=?) SELECT * FROM cte"));
        ASSERT_EQ(ECSqlStatus::Success, selStmt.BindId(1, key.GetInstanceId())) << selStmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_ROW, selStmt.Step());
        JsonECSqlSelectAdapter jsonAdapter(selStmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal, ECJsonInt64Format::AsNumber));
        Json::Value actualJson;
        ASSERT_EQ(SUCCESS, jsonAdapter.GetRow(actualJson)) << selStmt.GetECSql();
        ASSERT_TRUE(actualJson.isMember("PStructProp"));
        ASSERT_EQ(JsonValue(expectedJson), JsonValue(actualJson["PStructProp"]));
    }
    {
        ECSqlStatement selStmt;
        ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "WITH cte AS(SELECT PStructProp FROM ecsql.PSA WHERE ECInstanceId=?) SELECT PStructProp FROM cte"));
        ASSERT_EQ(ECSqlStatus::Success, selStmt.BindId(1, key.GetInstanceId())) << selStmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_ROW, selStmt.Step());
        JsonECSqlSelectAdapter jsonAdapter(selStmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal, ECJsonInt64Format::AsNumber));
        Json::Value actualJson;
        ASSERT_EQ(SUCCESS, jsonAdapter.GetRow(actualJson)) << selStmt.GetECSql();
        ASSERT_TRUE(actualJson.isMember("PStructProp"));
        ASSERT_EQ(JsonValue(expectedJson), JsonValue(actualJson["PStructProp"]));
    }
    {
        ECSqlStatement selStmt;
        ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "WITH cte AS(SELECT PStructProp FROM ecsql.PSA WHERE ECInstanceId=?) SELECT cte.PStructProp FROM cte"));
        ASSERT_EQ(ECSqlStatus::Success, selStmt.BindId(1, key.GetInstanceId())) << selStmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_ROW, selStmt.Step());
        JsonECSqlSelectAdapter jsonAdapter(selStmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal, ECJsonInt64Format::AsNumber));
        Json::Value actualJson;
        ASSERT_EQ(SUCCESS, jsonAdapter.GetRow(actualJson)) << selStmt.GetECSql();
        ASSERT_TRUE(actualJson.isMember("PStructProp"));
        ASSERT_EQ(JsonValue(expectedJson), JsonValue(actualJson["PStructProp"]));
    }
    {
        ECSqlStatement selStmt;
        ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "WITH cte AS(SELECT PStructProp FROM ecsql.PSA WHERE ECInstanceId=?) SELECT c.PStructProp FROM cte c"));
        ASSERT_EQ(ECSqlStatus::Success, selStmt.BindId(1, key.GetInstanceId())) << selStmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_ROW, selStmt.Step());
        JsonECSqlSelectAdapter jsonAdapter(selStmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal, ECJsonInt64Format::AsNumber));
        Json::Value actualJson;
        ASSERT_EQ(SUCCESS, jsonAdapter.GetRow(actualJson)) << selStmt.GetECSql();
        ASSERT_TRUE(actualJson.isMember("PStructProp"));
        ASSERT_EQ(JsonValue(expectedJson), JsonValue(actualJson["PStructProp"]));
    }
    {
        ECSqlStatement selStmt;
        ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "SELECT PStructProp.p2d FROM (SELECT PStructProp.p2d FROM ecsql.PSA WHERE ECInstanceId=?)"));
        ASSERT_EQ(ECSqlStatus::Success, selStmt.BindId(1, key.GetInstanceId())) << selStmt.GetECSql();
        ASSERT_EQ(1, selStmt.GetColumnCount());
        ASSERT_STREQ("p2d", selStmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, selStmt.Step());
        ASSERT_EQ(DPoint2d::From(3.0, 5.0), selStmt.GetValuePoint2d(0));
    }
    {
        ECSqlStatement selStmt;
        ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "WITH cte AS(SELECT PStructProp.p2d FROM ecsql.PSA WHERE ECInstanceId=?) SELECT PStructProp.p2d FROM cte"));
        ASSERT_EQ(ECSqlStatus::Success, selStmt.BindId(1, key.GetInstanceId())) << selStmt.GetECSql();
        ASSERT_EQ(1, selStmt.GetColumnCount());
        ASSERT_STREQ("p2d", selStmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, selStmt.Step());
        ASSERT_EQ(DPoint2d::From(3.0, 5.0), selStmt.GetValuePoint2d(0));
    }
    {
        ECSqlStatement selStmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, selStmt.Prepare(m_ecdb, "SELECT PStructProp.p2d.X FROM (SELECT PStructProp.p2d FROM ecsql.PSA WHERE ECInstanceId=?)")); // TODO: Should be supported
    }
    {
        ECSqlStatement selStmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, selStmt.Prepare(m_ecdb, "WITH cte AS(SELECT PStructProp.p2d FROM ecsql.PSA WHERE ECInstanceId=?) SELECT PStructProp.p2d.X FROM cte")); // TODO: Should be supported
    }
    {
        ECSqlStatement selStmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, selStmt.Prepare(m_ecdb, "WITH cte AS(SELECT PStructProp.p3d FROM ecsql.PSA WHERE ECInstanceId=?) SELECT PStructProp.p3d.X, PStructProp.p3d.Y, PStructProp.p3d.Z FROM cte")); // TODO: Should be supported
    }
    {
        ECSqlStatement selStmt;
        ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "WITH cte AS(SELECT PStructProp FROM ecsql.PSA WHERE ECInstanceId=?) SELECT PStructProp FROM cte"));
        ASSERT_EQ(ECSqlStatus::Success, selStmt.BindId(1, key.GetInstanceId())) << selStmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_ROW, selStmt.Step());
        JsonECSqlSelectAdapter jsonAdapter(selStmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal, ECJsonInt64Format::AsNumber));
        Json::Value actualJson;
        ASSERT_EQ(SUCCESS, jsonAdapter.GetRow(actualJson)) << selStmt.GetECSql();
        ASSERT_TRUE(actualJson.isMember("PStructProp"));
        ASSERT_EQ(JsonValue(expectedJson), JsonValue(actualJson["PStructProp"]));
    }
    {
        ECSqlStatement selStmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, selStmt.Prepare(m_ecdb, "WITH cte(str) AS(SELECT PStructProp FROM ecsql.PSA WHERE ECInstanceId=?) SELECT * FROM cte"));
    }
    {
        ECSqlStatement selStmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, selStmt.Prepare(m_ecdb, "WITH cte(str) AS(SELECT PStructProp FROM ecsql.PSA WHERE ECInstanceId=?) SELECT str FROM cte"));
    }
    {
        ECSqlStatement selStmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, selStmt.Prepare(m_ecdb, "WITH cte(str) AS(SELECT PStructProp FROM ecsql.PSA WHERE ECInstanceId=?) SELECT cte.str FROM cte"));
    }
    {
        ECSqlStatement selStmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, selStmt.Prepare(m_ecdb, "WITH cte(str) AS(SELECT PStructProp FROM ecsql.PSA WHERE ECInstanceId=?) SELECT c.str FROM cte c"));
    }
    if("binder_change_test_with_columns"){
        ECSqlStatement selStmt;
        ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "WITH cte(Id) AS(SELECT ECInstanceId FROM ecsql.PSA) SELECT * FROM cte c WHERE Id = ?"));
        ASSERT_EQ(ECSqlStatus::Success, selStmt.BindId(1, key.GetInstanceId())) << selStmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_ROW, selStmt.Step());
        ASSERT_STREQ("281", selStmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_DONE, selStmt.Step());
    }
    if("binder_change_test_without_columns"){
        ECSqlStatement selStmt;
        ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "WITH cte AS(SELECT ECInstanceId FROM ecsql.PSA) SELECT * FROM cte c WHERE ECInstanceId = ?"));
        ASSERT_EQ(ECSqlStatus::Success, selStmt.BindId(1, key.GetInstanceId())) << selStmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_ROW, selStmt.Step());
        ASSERT_STREQ("281", selStmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_DONE, selStmt.Step());
    }
    
    }

    //**** Test 2 *****
    {
    Json::Value expectedJson;
    ASSERT_EQ(SUCCESS, TestUtilities::ParseJson(expectedJson, R"json(
        { "PStructProp" :
        { "b" : true,
         "d" : 3.0,
         "dt" : "2014-03-27T12:00:00.000",
         "dtUtc" : "2014-03-27T12:00:00.000Z",
         "i" : 44444,
         "l" : 444444444,
         "s" : "Hello, world",
         "p2d" : { "x" : 3.0, "y" : 5.0 },
        "p3d" : { "x" : 3.0, "y" : 5.0, "z" : -6.0}
        }})json"));
    ECClassCP saStructClass = m_ecdb.Schemas().GetClass("ECSqlTest", "SAStruct");
    ASSERT_TRUE(saStructClass != nullptr && saStructClass->IsStructClass());

    ECSqlStatement insertStmt;
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.Prepare(m_ecdb, "INSERT INTO ecsql.SA(SAStructProp) VALUES(?)"));
    ASSERT_EQ(ECSqlStatus::Success, JsonECSqlBinder::BindStructValue(insertStmt.GetBinder(1), expectedJson, *saStructClass->GetStructClassCP())) << insertStmt.GetECSql();
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step(key)) << insertStmt.GetECSql();
    insertStmt.Finalize();
    
    {
        ECSqlStatement selStmt;
        ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "WITH cte AS(SELECT SAStructProp FROM ecsql.SA WHERE ECInstanceId=?) SELECT * FROM cte"));
        ASSERT_EQ(ECSqlStatus::Success, selStmt.BindId(1, key.GetInstanceId())) << selStmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_ROW, selStmt.Step());
        JsonECSqlSelectAdapter jsonAdapter(selStmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal, ECJsonInt64Format::AsNumber));
        Json::Value actualJson;
        ASSERT_EQ(SUCCESS, jsonAdapter.GetRow(actualJson)) << selStmt.GetECSql();
        ASSERT_TRUE(actualJson.isMember("SAStructProp"));
        ASSERT_EQ(JsonValue(expectedJson), JsonValue(actualJson["SAStructProp"]));
    }
    {
        ECSqlStatement selStmt;
        ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "WITH cte AS(SELECT SAStructProp FROM ecsql.SA WHERE ECInstanceId=?) SELECT SAStructProp FROM cte"));
        ASSERT_EQ(ECSqlStatus::Success, selStmt.BindId(1, key.GetInstanceId())) << selStmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_ROW, selStmt.Step());
        JsonECSqlSelectAdapter jsonAdapter(selStmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal, ECJsonInt64Format::AsNumber));
        Json::Value actualJson;
        ASSERT_EQ(SUCCESS, jsonAdapter.GetRow(actualJson)) << selStmt.GetECSql();
        ASSERT_TRUE(actualJson.isMember("SAStructProp"));
        ASSERT_EQ(JsonValue(expectedJson), JsonValue(actualJson["SAStructProp"]));
    }
    {
        ECSqlStatement selStmt;
        ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "WITH cte AS(SELECT SAStructProp FROM ecsql.SA WHERE ECInstanceId=?) SELECT cte.SAStructProp FROM cte"));
        ASSERT_EQ(ECSqlStatus::Success, selStmt.BindId(1, key.GetInstanceId())) << selStmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_ROW, selStmt.Step());
        JsonECSqlSelectAdapter jsonAdapter(selStmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal, ECJsonInt64Format::AsNumber));
        Json::Value actualJson;
        ASSERT_EQ(SUCCESS, jsonAdapter.GetRow(actualJson)) << selStmt.GetECSql();
        ASSERT_TRUE(actualJson.isMember("SAStructProp"));
        ASSERT_EQ(JsonValue(expectedJson), JsonValue(actualJson["SAStructProp"]));
    }
    {
        ECSqlStatement selStmt;
        ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "WITH cte AS(SELECT SAStructProp FROM ecsql.SA WHERE ECInstanceId=?) SELECT c.SAStructProp FROM cte c"));
        ASSERT_EQ(ECSqlStatus::Success, selStmt.BindId(1, key.GetInstanceId())) << selStmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_ROW, selStmt.Step());
        JsonECSqlSelectAdapter jsonAdapter(selStmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal, ECJsonInt64Format::AsNumber));
        Json::Value actualJson;
        ASSERT_EQ(SUCCESS, jsonAdapter.GetRow(actualJson)) << selStmt.GetECSql();
        ASSERT_TRUE(actualJson.isMember("SAStructProp"));
        ASSERT_EQ(JsonValue(expectedJson), JsonValue(actualJson["SAStructProp"]));
    }
    {
        ECSqlStatement selStmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, selStmt.Prepare(m_ecdb, "SELECT SAStructProp.p2d FROM (SELECT SAStructProp.p2d FROM ecsql.SA WHERE ECInstanceId=?)")); // TODO: Should be supported
    }
    {
        ECSqlStatement selStmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, selStmt.Prepare(m_ecdb, "WITH cte AS(SELECT SAStructProp.p2d FROM ecsql.SA WHERE ECInstanceId=?) SELECT SAStructProp.p2d FROM cte")); // TODO: Should be supported
    }
    {
        ECSqlStatement selStmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, selStmt.Prepare(m_ecdb, "SELECT SAStructProp.p2d.X FROM (SELECT SAStructProp.p2d FROM ecsql.SA WHERE ECInstanceId=?)")); // TODO: Should be supported
    }
    {
        ECSqlStatement selStmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, selStmt.Prepare(m_ecdb, "WITH cte AS(SELECT SAStructProp.p2d FROM ecsql.SA WHERE ECInstanceId=?) SELECT SAStructProp.p2d.X FROM cte")); // TODO: Should be supported
    }
    {
        ECSqlStatement selStmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, selStmt.Prepare(m_ecdb, "WITH cte AS(SELECT SAStructProp.p3d FROM ecsql.PSA WHERE ECInstanceId=?) SELECT SAStructProp.p3d.X, SAStructProp.p3d.Y, SAStructProp.p3d.Z FROM cte")); // TODO: Should be supported
    }
    {
        ECSqlStatement selStmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, selStmt.Prepare(m_ecdb, "WITH cte(str) AS(SELECT SAStructProp FROM ecsql.SA WHERE ECInstanceId=?) SELECT * FROM cte"));
    }
    {
        ECSqlStatement selStmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, selStmt.Prepare(m_ecdb, "WITH cte(str) AS(SELECT SAStructProp FROM ecsql.SA WHERE ECInstanceId=?) SELECT str FROM cte"));
    }
    {
        ECSqlStatement selStmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, selStmt.Prepare(m_ecdb, "WITH cte(str) AS(SELECT SAStructProp FROM ecsql.SA WHERE ECInstanceId=?) SELECT cte.str FROM cte"));
    }
    {
        ECSqlStatement selStmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, selStmt.Prepare(m_ecdb, "WITH cte(str) AS(SELECT SAStructProp FROM ecsql.SA WHERE ECInstanceId=?) SELECT c.str FROM cte c"));
    }
    if("binder_change_test_with_columns"){
        ECSqlStatement selStmt;
        ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "WITH cte(Id) AS(SELECT ECInstanceId FROM ecsql.SA) SELECT * FROM cte c WHERE Id = ?"));
        ASSERT_EQ(ECSqlStatus::Success, selStmt.BindId(1, key.GetInstanceId())) << selStmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_ROW, selStmt.Step());
        ASSERT_STREQ("282", selStmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_DONE, selStmt.Step());
    }
    if("binder_change_test_without_columns"){
        ECSqlStatement selStmt;
        ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "WITH cte AS(SELECT ECInstanceId FROM ecsql.SA) SELECT * FROM cte c WHERE ECInstanceId = ?"));
        ASSERT_EQ(ECSqlStatus::Success, selStmt.BindId(1, key.GetInstanceId())) << selStmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_ROW, selStmt.Step());
        ASSERT_STREQ("282", selStmt.GetValueText(0));
        ASSERT_EQ(BE_SQLITE_DONE, selStmt.Step());
    }
    
    }
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CommonTableExpTestFixture, multiple_ctes_without_subcolumns)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("multiple_ctes_without_subcolumns.ecdb"));
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "WITH [cte1] AS ( SELECT 1 AS KEYID, 'BeepBoo' AS Noise ), [cte2] AS ( SELECT 1 AS KEYID, 'Robot' AS Name ) SELECT * FROM cte1 [c1] JOIN cte2 [c2] ON c1.KEYID = c2.KEYID"));
    ASSERT_STREQ(stmt.GetNativeSql(), "WITH cte1 AS (SELECT 1 [K0],'BeepBoo' [K2]),cte2 AS (SELECT 1 [K1],'Robot' [K3])\nSELECT [K0],[K2],[K1],[K3] FROM cte1 c1 INNER JOIN cte2 c2 ON [K0]=[K1] ");
    ASSERT_EQ(4, stmt.GetColumnCount());
    ASSERT_STREQ("KEYID", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
    ASSERT_STREQ("Noise", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
    ASSERT_STREQ("KEYID_1", stmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
    ASSERT_STREQ("Name", stmt.GetColumnInfo(3).GetProperty()->GetName().c_str());
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ("1", stmt.GetValueText(0));
    ASSERT_STREQ("BeepBoo", stmt.GetValueText(1));
    ASSERT_STREQ("1", stmt.GetValueText(2));
    ASSERT_STREQ("Robot", stmt.GetValueText(3));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CommonTableExpTestFixture, multiple_ctes_without_subcolumns_without_asterisk)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("multiple_ctes_without_subcolumns_without_asterisk.ecdb"));
   ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "WITH [cte1] AS ( SELECT 1 AS KEYID, 'BeepBoo' AS Noise ), [cte2] AS ( SELECT 1 AS KEYID, 'Robot' AS Name ) SELECT c1.KEYID, c1.Noise ,c2.KEYID, c2.Name FROM cte1 [c1] JOIN cte2 [c2] ON c1.KEYID = c2.KEYID"));
    ASSERT_STREQ(stmt.GetNativeSql(), "WITH cte1 AS (SELECT 1 [K0],'BeepBoo' [K2]),cte2 AS (SELECT 1 [K1],'Robot' [K3])\nSELECT [K0],[K2],[K1],[K3] FROM cte1 c1 INNER JOIN cte2 c2 ON [K0]=[K1] ");
    ASSERT_EQ(4, stmt.GetColumnCount());
    ASSERT_STREQ("KEYID", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
    ASSERT_STREQ("Noise", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
    ASSERT_STREQ("KEYID_1", stmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
    ASSERT_STREQ("Name", stmt.GetColumnInfo(3).GetProperty()->GetName().c_str());
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ("1", stmt.GetValueText(0));
    ASSERT_STREQ("BeepBoo", stmt.GetValueText(1));
    ASSERT_STREQ("1", stmt.GetValueText(2));
    ASSERT_STREQ("Robot", stmt.GetValueText(3));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CommonTableExpTestFixture, multiple_ctes_without_subcolumns_with_asterisk)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("multiple_ctes_without_subcolumns_without_asterisk.ecdb"));
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "WITH [cte1] AS ( SELECT 1 AS KEYID, 'BeepBoo' AS Noise ), cte2(KEYID, Name) AS ( SELECT 1, 'Robot' ) SELECT * FROM cte1 [c1] JOIN cte2 [c2] ON c1.KEYID = c2.KEYID"));
    ASSERT_STREQ(stmt.GetNativeSql(), "WITH cte1 AS (SELECT 1 [K0],'BeepBoo' [K2]),cte2(KEYID,Name) AS (SELECT 1,'Robot')\nSELECT [K0],[K2],c2.KEYID,c2.Name FROM cte1 c1 INNER JOIN cte2 c2 ON [K0]=c2.KEYID ");
    ASSERT_EQ(4, stmt.GetColumnCount());
    ASSERT_STREQ("KEYID", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
    ASSERT_STREQ("Noise", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
    ASSERT_STREQ("KEYID_1", stmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
    ASSERT_STREQ("Name", stmt.GetColumnInfo(3).GetProperty()->GetName().c_str());
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ("1", stmt.GetValueText(0));
    ASSERT_STREQ("BeepBoo", stmt.GetValueText(1));
    ASSERT_STREQ("1", stmt.GetValueText(2));
    ASSERT_STREQ("Robot", stmt.GetValueText(3));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CommonTableExpTestFixture, mixing_ctes_with_columns_and_without_columns)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("multiple_ctes_without_subcolumns_without_asterisk.ecdb"));
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "WITH [cte1] AS ( SELECT 1 AS KEYID, 'BeepBoo' AS Noise ), cte2(KEYID, Name) AS ( SELECT 1, 'Robot' ) SELECT c1.KEYID, Noise, c2.KEYID, Name FROM cte1 [c1] JOIN cte2 [c2] ON c1.KEYID = c2.KEYID"));
    ASSERT_STREQ(stmt.GetNativeSql(), "WITH cte1 AS (SELECT 1 [K0],'BeepBoo' [K2]),cte2(KEYID,Name) AS (SELECT 1,'Robot')\nSELECT [K0],[K2],c2.KEYID,c2.Name FROM cte1 c1 INNER JOIN cte2 c2 ON [K0]=c2.KEYID ");
    ASSERT_EQ(4, stmt.GetColumnCount());
    ASSERT_STREQ("KEYID", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
    ASSERT_STREQ("Noise", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
    ASSERT_STREQ("c2__x002E__KEYID", stmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
    EXPECT_STREQ("c2.KEYID", stmt.GetColumnInfo(2).GetProperty()->GetDisplayLabel().c_str());
    ASSERT_STREQ("Name", stmt.GetColumnInfo(3).GetProperty()->GetName().c_str());
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ("1", stmt.GetValueText(0));
    ASSERT_STREQ("BeepBoo", stmt.GetValueText(1));
    ASSERT_STREQ("1", stmt.GetValueText(2));
    ASSERT_STREQ("Robot", stmt.GetValueText(3));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CommonTableExpTestFixture, mixing_ctes_with_columns_and_without_columns_and_selecting_in_random_order)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("mixing_ctes_with_columns_and_without_columns_and_selecting_in_random_order.ecdb"));
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "WITH [cte1] AS ( SELECT 1 AS KEYID, 'BeepBoo' AS Noise ), cte2(KEYID, Name) AS ( SELECT 1, 'Robot' ) SELECT c2.KEYID, Noise, c1.KEYID, Name FROM cte1 [c1] JOIN cte2 [c2] ON c1.KEYID = c2.KEYID"));
    ASSERT_STREQ(stmt.GetNativeSql(), "WITH cte1 AS (SELECT 1 [K0],'BeepBoo' [K2]),cte2(KEYID,Name) AS (SELECT 1,'Robot')\nSELECT c2.KEYID,[K2],[K0],c2.Name FROM cte1 c1 INNER JOIN cte2 c2 ON [K0]=c2.KEYID ");
    ASSERT_EQ(4, stmt.GetColumnCount());
    ASSERT_STREQ("KEYID", stmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
    ASSERT_STREQ("Noise", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
    ASSERT_STREQ("c2__x002E__KEYID", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
    EXPECT_STREQ("c2.KEYID", stmt.GetColumnInfo(0).GetProperty()->GetDisplayLabel().c_str());
    ASSERT_STREQ("Name", stmt.GetColumnInfo(3).GetProperty()->GetName().c_str());
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ("1", stmt.GetValueText(0));
    ASSERT_STREQ("BeepBoo", stmt.GetValueText(1));
    ASSERT_STREQ("1", stmt.GetValueText(2));
    ASSERT_STREQ("Robot", stmt.GetValueText(3));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CommonTableExpTestFixture, unions_inside_select_inside_cte_and_selecting_1_column_after_left_join)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("unions_inside_select_inside_cte_and_selecting_1_column_after_left_join.ecdb"));
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, " WITH edges AS ( SELECT 1 AS [Id], 2 AS [ParentId], 'Hi' UNION ALL SELECT 3 AS [Id], 4 AS [ParentId], 'Hello'), nodes AS ( SELECT 3 AS [Id], 2 AS [ParentId], 'A'), joinToParent AS (SELECT p.Id FROM nodes [p] LEFT JOIN edges [c] ON [p].Id = [c].ParentId) SELECT * FROM joinToParent"));
    ASSERT_STREQ(stmt.GetNativeSql(), "WITH edges AS (SELECT 1,2 [K1],'Hi' UNION ALL SELECT 3,4,'Hello'),nodes AS (SELECT 3 [K0],2,'A'),joinToParent AS (SELECT [K0] [K2] FROM nodes p LEFT OUTER JOIN edges c ON [K0]=[K1] )\nSELECT [K2] FROM joinToParent");
    ASSERT_EQ(1, stmt.GetColumnCount());
    ASSERT_STREQ("Id", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ("3", stmt.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CommonTableExpTestFixture, unions_inside_select_inside_cte_and_selecting_multiple_columns_after_left_join)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("unions_inside_select_inside_cte_and_selecting_multiple_columns_after_left_join.ecdb"));
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, " WITH edges AS ( SELECT 1 AS [Id], 2 AS [ParentId], 'Hi' UNION ALL SELECT 3 AS [Id], 4 AS [ParentId], 'Hello'), nodes AS ( SELECT 3 AS [Id], 2 AS [ParentId], 'A'), joinToParent AS (SELECT * FROM nodes [p] LEFT JOIN edges [c] ON [p].Id = [c].ParentId) SELECT * FROM joinToParent"));
    ASSERT_STREQ(stmt.GetNativeSql(), "WITH edges AS (SELECT 1 [K4],2 [K1],'Hi' [K5] UNION ALL SELECT 3,4,'Hello'),nodes AS (SELECT 3 [K0],2 [K2],'A' [K3]),joinToParent AS (SELECT [K0] [K6],[K2] [K7],[K3] [K8],[K4] [K9],[K1] [K10],[K5] [K11] FROM nodes p LEFT OUTER JOIN edges c ON [K0]=[K1] )\nSELECT [K6],[K7],[K8],[K9],[K10],[K11] FROM joinToParent");
    ASSERT_EQ(6, stmt.GetColumnCount());
    ASSERT_STREQ("Id", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
    ASSERT_STREQ("ParentId", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
    ASSERT_STREQ("'A'", stmt.GetColumnInfo(2).GetProperty()->GetDisplayLabel().c_str());
    ASSERT_STREQ("Id_1", stmt.GetColumnInfo(3).GetProperty()->GetDisplayLabel().c_str());
    ASSERT_STREQ("ParentId_1", stmt.GetColumnInfo(4).GetProperty()->GetDisplayLabel().c_str());
    ASSERT_STREQ("'Hi'", stmt.GetColumnInfo(5).GetProperty()->GetDisplayLabel().c_str());
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ("3", stmt.GetValueText(0));
    ASSERT_STREQ("2", stmt.GetValueText(1));
    ASSERT_STREQ("A", stmt.GetValueText(2));
    ASSERT_EQ(true, stmt.IsValueNull(3));
    ASSERT_EQ(true, stmt.IsValueNull(4));
    ASSERT_EQ(true, stmt.IsValueNull(5));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CommonTableExpTestFixture, multiple_ctes_with_aliased_asterisk_resolution_inside)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("multiple_ctes_with_aliased_asterisk_resolution_inside.ecdb"));
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, " WITH edges AS ( SELECT 1 AS [Id], 2 AS [ParentId], 'Hi' UNION ALL SELECT 3 AS [Id], 4 AS [ParentId], 'Hello'), nodes AS ( SELECT 3 AS [Id], 2 AS [ParentId], 'A'), joinToParent AS (SELECT p.* FROM nodes [p] LEFT JOIN edges [c] ON [p].Id = [c].ParentId) SELECT * FROM joinToParent"));
    ASSERT_STREQ(stmt.GetNativeSql(), "WITH edges AS (SELECT 1,2 [K1],'Hi' UNION ALL SELECT 3,4,'Hello'),nodes AS (SELECT 3 [K0],2 [K2],'A' [K3]),joinToParent AS (SELECT [K0] [K4],[K2] [K5],[K3] [K6] FROM nodes p LEFT OUTER JOIN edges c ON [K0]=[K1] )\nSELECT [K4],[K5],[K6] FROM joinToParent");
    ASSERT_EQ(3, stmt.GetColumnCount());
    ASSERT_STREQ("Id", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
    ASSERT_STREQ("ParentId", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
    ASSERT_STREQ("'A'", stmt.GetColumnInfo(2).GetProperty()->GetDisplayLabel().c_str());
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ("3", stmt.GetValueText(0));
    ASSERT_STREQ("2", stmt.GetValueText(1));
    ASSERT_STREQ("A", stmt.GetValueText(2));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CommonTableExpTestFixture, multiple_ctes_with_aliased_asterisk_resolution_inside_and_outside)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("multiple_ctes_with_aliased_asterisk_resolution_inside_and_outside.ecdb"));
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, " WITH edges AS ( SELECT 1 AS [Id], 2 AS [ParentId], 'Hi' UNION ALL SELECT 3 AS [Id], 4 AS [ParentId], 'Hello'), nodes AS ( SELECT 3 AS [Id], 2 AS [ParentId], 'A'), joinToParent AS (SELECT * FROM edges [c] LEFT JOIN nodes [p] ON [c].ParentId = [p].Id) SELECT * FROM joinToParent"));
    ASSERT_STREQ(stmt.GetNativeSql(), "WITH edges AS (SELECT 1 [K2],2 [K0],'Hi' [K3] UNION ALL SELECT 3,4,'Hello'),nodes AS (SELECT 3 [K1],2 [K4],'A' [K5]),joinToParent AS (SELECT [K2] [K6],[K0] [K7],[K3] [K8],[K1] [K9],[K4] [K10],[K5] [K11] FROM edges c LEFT OUTER JOIN nodes p ON [K0]=[K1] )\nSELECT [K6],[K7],[K8],[K9],[K10],[K11] FROM joinToParent");
    ASSERT_EQ(6, stmt.GetColumnCount());
    ASSERT_STREQ("Id", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
    ASSERT_STREQ("ParentId", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
    ASSERT_STREQ("'Hi'", stmt.GetColumnInfo(2).GetProperty()->GetDisplayLabel().c_str());
    ASSERT_STREQ("Id_1", stmt.GetColumnInfo(3).GetProperty()->GetDisplayLabel().c_str());
    ASSERT_STREQ("ParentId_1", stmt.GetColumnInfo(4).GetProperty()->GetDisplayLabel().c_str());
    ASSERT_STREQ("'A'", stmt.GetColumnInfo(5).GetProperty()->GetDisplayLabel().c_str());
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ("1", stmt.GetValueText(0));
    ASSERT_STREQ("2", stmt.GetValueText(1));
    ASSERT_STREQ("Hi", stmt.GetValueText(2));
    ASSERT_EQ(true, stmt.IsValueNull(3));
    ASSERT_EQ(true, stmt.IsValueNull(4));
    ASSERT_EQ(true, stmt.IsValueNull(5));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ("3", stmt.GetValueText(0));
    ASSERT_STREQ("4", stmt.GetValueText(1));
    ASSERT_STREQ("Hello", stmt.GetValueText(2));
    ASSERT_EQ(true, stmt.IsValueNull(3));
    ASSERT_EQ(true, stmt.IsValueNull(4));
    ASSERT_EQ(true, stmt.IsValueNull(5));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CommonTableExpTestFixture, multiple_ctes_with_aliased_asterisk_resolution_inside_and_outside_with_values_matching_for_on_clause)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("multiple_ctes_with_aliased_asterisk_resolution_inside_and_outside_with_values_matching_for_on_clause.ecdb"));
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, " WITH edges AS ( SELECT 1 AS [Id], 2 AS [ParentId], 'Hi' UNION ALL SELECT 3 AS [Id], 4 AS [ParentId], 'Hello'), nodes AS ( SELECT 4 AS [Id], 2 AS [ParentId], 'A'), joinToParent AS (SELECT * FROM edges [c] LEFT JOIN nodes [p] ON [c].ParentId = [p].Id) SELECT * FROM joinToParent"));
    ASSERT_STREQ(stmt.GetNativeSql(), "WITH edges AS (SELECT 1 [K2],2 [K0],'Hi' [K3] UNION ALL SELECT 3,4,'Hello'),nodes AS (SELECT 4 [K1],2 [K4],'A' [K5]),joinToParent AS (SELECT [K2] [K6],[K0] [K7],[K3] [K8],[K1] [K9],[K4] [K10],[K5] [K11] FROM edges c LEFT OUTER JOIN nodes p ON [K0]=[K1] )\nSELECT [K6],[K7],[K8],[K9],[K10],[K11] FROM joinToParent");
    ASSERT_EQ(6, stmt.GetColumnCount());
    ASSERT_STREQ("Id", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
    ASSERT_STREQ("ParentId", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
    ASSERT_STREQ("'Hi'", stmt.GetColumnInfo(2).GetProperty()->GetDisplayLabel().c_str());
    ASSERT_STREQ("Id_1", stmt.GetColumnInfo(3).GetProperty()->GetDisplayLabel().c_str());
    ASSERT_STREQ("ParentId_1", stmt.GetColumnInfo(4).GetProperty()->GetDisplayLabel().c_str());
    ASSERT_STREQ("'A'", stmt.GetColumnInfo(5).GetProperty()->GetDisplayLabel().c_str());
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ("1", stmt.GetValueText(0));
    ASSERT_STREQ("2", stmt.GetValueText(1));
    ASSERT_STREQ("Hi", stmt.GetValueText(2));
    ASSERT_EQ(true, stmt.IsValueNull(3));
    ASSERT_EQ(true, stmt.IsValueNull(4));
    ASSERT_EQ(true, stmt.IsValueNull(5));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ("3", stmt.GetValueText(0));
    ASSERT_STREQ("4", stmt.GetValueText(1));
    ASSERT_STREQ("Hello", stmt.GetValueText(2));
    ASSERT_STREQ("4", stmt.GetValueText(3));
    ASSERT_STREQ("2", stmt.GetValueText(4));
    ASSERT_STREQ("A", stmt.GetValueText(5));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CommonTableExpTestFixture, ambiguous_column_for_multiple_CTEs)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("ambiguous_column_for_multiple_CTEs.ecdb"));
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "WITH [cte1] AS ( SELECT 1 AS KEYID, 'BeepBoo' AS Noise ), cte2(KEYID, Name) AS ( SELECT 1, 'Robot' ) SELECT KEYID, Noise, Name FROM cte1 [c1] JOIN cte2 [c2] ON c1.KEYID = c2.KEYID"));
    }

END_ECDBUNITTESTS_NAMESPACE
