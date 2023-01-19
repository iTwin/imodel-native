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

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

struct CommonTableExpTestFixture : ECDbTestFixture {};
//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CommonTableExpTestFixture, PureRowConstructorQuery) {
    ASSERT_EQ(SUCCESS, SetupECDb("cte_syntax.ecdb"));


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
    ASSERT_EQ(SUCCESS, SetupECDb("cte_test_meta.ecdb", SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
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
        ASSERT_EQ(73, stmt.GetValueInt(0)) << "aId";
        ASSERT_EQ(true, stmt.IsValueNull(1)) << "aParentId"; 
        ASSERT_EQ(0, stmt.GetValueInt(2)) << "aDepth";
        ASSERT_STREQ("A", stmt.GetValueText(3)) << "aPath";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(74, stmt.GetValueInt(0)) << "aId";
        ASSERT_EQ(73, stmt.GetValueInt(1)) << "aParentId";
        ASSERT_EQ(1, stmt.GetValueInt(2)) << "aDepth";
        ASSERT_STREQ("A/AA", stmt.GetValueText(3)) << "aPath";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(75, stmt.GetValueInt(0)) << "aId";
        ASSERT_EQ(74, stmt.GetValueInt(1)) << "aParentId";
        ASSERT_EQ(2, stmt.GetValueInt(2)) << "aDepth";
        ASSERT_STREQ("A/AA/AAA", stmt.GetValueText(3)) << "aPath";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(76, stmt.GetValueInt(0)) << "aId";
        ASSERT_EQ(74, stmt.GetValueInt(1)) << "aParentId";
        ASSERT_EQ(2, stmt.GetValueInt(2)) << "aDepth";
        ASSERT_STREQ("A/AA/AAB", stmt.GetValueText(3)) << "aPath";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(77, stmt.GetValueInt(0)) << "aId";
        ASSERT_EQ(73, stmt.GetValueInt(1)) << "aParentId";
        ASSERT_EQ(1, stmt.GetValueInt(2)) << "aDepth";
        ASSERT_STREQ("A/AB", stmt.GetValueText(3)) << "aPath";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(78, stmt.GetValueInt(0)) << "aId";
        ASSERT_EQ(77, stmt.GetValueInt(1)) << "aParentId";
        ASSERT_EQ(2, stmt.GetValueInt(2)) << "aDepth";
        ASSERT_STREQ("A/AB/ABA", stmt.GetValueText(3)) << "aPath";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(79, stmt.GetValueInt(0)) << "aId";
        ASSERT_EQ(77, stmt.GetValueInt(1)) << "aParentId";
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
}
 //---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CommonTableExpTestFixture, RecursiveQuery) {
    ASSERT_EQ(SUCCESS, SetupECDb("cte_test.ecdb", SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
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
TEST_F(CommonTableExpTestFixture, SqliteExample) {
    ASSERT_EQ(SUCCESS, SetupECDb("cte_syntax.ecdb"));
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
    EXPECT_TRUE(statement.GetColumnInfo(1).GetDataType().IsPrimitive());

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
    EXPECT_TRUE(statement.GetColumnInfo(1).GetDataType().IsPrimitive());

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
    ASSERT_EQ(SUCCESS, SetupECDb("cte_test_meta.ecdb"));
    if ("simple_nested_no_alias") {
        auto query = R"(
            with recursive
                cte0 (a,b) as ( select 100,200)
            select * from (select a, b from cte0 where a=100 and b=200)
        )";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ(stmt.GetNativeSql(), "WITH cte0(a,b) AS (SELECT 100,200)\nSELECT [K2],[K3] FROM (SELECT cte0.a [K2],cte0.b [K3] FROM cte0 WHERE cte0.a=100 AND cte0.b=200)");
    }     
    if ("simple_nested") {
        auto query = R"(
            with recursive
                cte0 (a,b) as ( select 100,200)
            select * from (select c0.a, c0.b from cte0 c0 where c0.a=100 and c0.b=200)
        )";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ(stmt.GetNativeSql(), "WITH cte0(a,b) AS (SELECT 100,200)\nSELECT [K2],[K3] FROM (SELECT c0.a [K2],c0.b [K3] FROM cte0 c0 WHERE c0.a=100 AND c0.b=200)");
    } 
    if ("simple_wild_nested") {
        auto query = R"(
            with recursive
                cte0 (a,b) as ( select 100,200)
            select * from (select * from cte0 c0 where c0.a=100 and c0.b=200)
        )";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ(stmt.GetNativeSql(), "WITH cte0(a,b) AS (SELECT 100,200)\nSELECT [K2],[K3] FROM (SELECT c0.a K2,c0.b K3 FROM cte0 c0 WHERE c0.a=100 AND c0.b=200)");
    }
  
    if ("simple_wild") {
        auto query = R"(
            with recursive
                cte0 (a,b) as ( select 100,200)
            select * from cte0 c0 where c0.a=100 and c0.b=200
        )";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ(stmt.GetNativeSql(), "WITH cte0(a,b) AS (SELECT 100,200)\nSELECT c0.a,c0.b FROM cte0 c0 WHERE c0.a=100 AND c0.b=200");
    }
    if ("simple_wild_no_alias") {
        auto query = R"(
            with recursive
                cte0 (a,b) as ( select 100,200)
            select * from cte0 where a=100 and b=200
        )";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ(stmt.GetNativeSql(), "WITH cte0(a,b) AS (SELECT 100,200)\nSELECT cte0.a,cte0.b FROM cte0 WHERE cte0.a=100 AND cte0.b=200");
    }
    if ("simple_alias") {
        auto query = R"(
            with recursive
                cte0 (a,b) as ( select 100,200)
            select c0.a, c0.b from cte0 c0 where c0.a=100 and c0.b=200
        )";
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ASSERT_STREQ(stmt.GetNativeSql(), "WITH cte0(a,b) AS (SELECT 100,200)\nSELECT c0.a,c0.b FROM cte0 c0 WHERE c0.a=100 AND c0.b=200");
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
        ASSERT_STREQ(stmt.GetNativeSql(), "WITH cte0(a,b) AS (SELECT 100,200),cte1(c,d) AS (SELECT 100,200)\nSELECT c0.a,c0.b,c1.c,c1.d FROM cte0 c0,cte1 c1 WHERE c0.a=100 AND c0.b=200 AND c1.c=100 AND c1.d=200");
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
        ASSERT_STREQ(stmt.GetNativeSql(), "WITH cte0(a,b) AS (SELECT 100,200),cte1(a,b) AS (SELECT 100,200)\nSELECT c0.a,c0.b,c1.a,c1.b FROM cte0 c0,cte1 c1 WHERE c0.a=100 AND c0.b=200 AND c1.a=100 AND c1.b=200");
    }
}

END_ECDBUNITTESTS_NAMESPACE
