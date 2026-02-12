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
TEST_F(RelatedInstanceFinderFixture, FinderAPI) {
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

    auto reformatJson = [](Utf8CP js) -> Utf8String {
            BeJsDocument doc;
            doc.Parse(js);
            return doc.Stringify(StringifyFormat::Indented);
    };

    auto getRelatedInstanceJson = [&](ECInstanceKeyCR key, Utf8CP direction) -> Utf8String {
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(db, R"s(
            SELECT JSON_GROUP_ARRAY(
                JSON_OBJECT(
                    'ECInstanceId', ECInstanceId,
                    'ECClassId', ec_className(ECClassId),
                    'RelECClassId', ec_className(RelECClassId),
                    'Direction', Direction)
                ) out
            FROM rel1.related_instances(?,?,?) ORDER BY ECInstanceId OPTIONS ENABLE_EXPERIMENTAL_FEATURES;
        )s"));

        stmt.BindId(1, e1.GetInstanceId());
        stmt.BindId(2, e1.GetClassId());
        stmt.BindText(3, direction, IECSqlBinder::MakeCopy::No);
        if (stmt.Step() == BE_SQLITE_ROW) {
            return reformatJson(stmt.GetValueText(0));
        }
        return "";
    };

    if ("e1 - forward") {
        const auto expected = reformatJson(R"([
            {
                "ECInstanceId": 2,
                "ECClassId": "TestSchema:Element",
                "RelECClassId": "TestSchema:ElementOwnsChildElements",
                "Direction": 1
            },
            {
                "ECInstanceId": 2,
                "ECClassId": "TestSchema:Element",
                "RelECClassId": "TestSchema:ElementRefersToElements",
                "Direction": 1
            },
            {
                "ECInstanceId": 1,
                "ECClassId": "TestSchema:Element",
                "RelECClassId": "TestSchema:ElementRefersToElements",
                "Direction": 1
            }
        ])");
        // printf("%s\n", getRelatedInstanceJson(e1, "forward").c_str());
        EXPECT_STREQ(expected.c_str(), getRelatedInstanceJson(e1, "forward").c_str());
    }
    if ("e1 - backward") {
        const auto expected = reformatJson(R"([
            {
                "ECInstanceId": 1,
                "ECClassId": "TestSchema:Element",
                "RelECClassId": "TestSchema:ElementRefersToElements",
                "Direction": 2
            },
            {
                "ECInstanceId": 2,
                "ECClassId": "TestSchema:Element",
                "RelECClassId": "TestSchema:ElementRefersToElements",
                "Direction": 2
            }
        ])");

        // printf("%s\n", getRelatedInstanceJson(e1, "backward").c_str());
        EXPECT_STREQ(expected.c_str(), getRelatedInstanceJson(e1, "backward").c_str());
    }
    if ("e1 - both") {
        const auto expected = reformatJson(R"([
            {
                "ECInstanceId": 2,
                "ECClassId": "TestSchema:Element",
                "RelECClassId": "TestSchema:ElementOwnsChildElements",
                "Direction": 1
            },
            {
                "ECInstanceId": 2,
                "ECClassId": "TestSchema:Element",
                "RelECClassId": "TestSchema:ElementRefersToElements",
                "Direction": 1
            },
            {
                "ECInstanceId": 1,
                "ECClassId": "TestSchema:Element",
                "RelECClassId": "TestSchema:ElementRefersToElements",
                "Direction": 1
            },
            {
                "ECInstanceId": 1,
                "ECClassId": "TestSchema:Element",
                "RelECClassId": "TestSchema:ElementRefersToElements",
                "Direction": 2
            },
            {
                "ECInstanceId": 2,
                "ECClassId": "TestSchema:Element",
                "RelECClassId": "TestSchema:ElementRefersToElements",
                "Direction": 2
            }
        ])");
        // printf("%s\n", getRelatedInstanceJson(e1, "both").c_str());
        EXPECT_STREQ(expected.c_str(), getRelatedInstanceJson(e1, "both").c_str());
    }

    // printf("%s\n", getRelatedInstanceJson(e1, "forward").c_str());

}

END_ECDBUNITTESTS_NAMESPACE
