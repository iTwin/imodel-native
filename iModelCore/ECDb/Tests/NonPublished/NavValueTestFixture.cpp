/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// @bsistruct
//=======================================================================================
struct NavValueTestFixture : ECDbTestFixture {
   protected:
    ECClassId m_personClassId;
    ECClassId m_bookClassId;
    ECClassId m_bookHasAuthorClassId;
    ECInstanceKey m_personInstanceKey;
    ECInstanceKey m_bookInstanceKey;
    void SetUp() override;
};

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void NavValueTestFixture::SetUp() {
    ECDbTestFixture::SetUp();
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("NavValueTests.ecdb", SchemaItem(
                                                                          R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.03" alias="ecdbmap"/>
            <ECEntityClass typeName="Person">
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Book">
                <ECProperty propertyName="Title" typeName="string" />
                <ECNavigationProperty propertyName="Author" relationshipName="BookHasAuthor" direction="Forward"/>
            </ECEntityClass>
            <ECRelationshipClass typeName="BookHasAuthor" modifier="Sealed" strength="referencing">
                <Source multiplicity="(0..*)" roleLabel="A" polymorphic="false">
                    <Class class="Book"/>
                </Source>
                <Target multiplicity="(1..1)" roleLabel="B" polymorphic="false">
                    <Class class="Person"/>
                </Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="CustomBookHasAuthor" modifier="Sealed" strength="referencing">
                <Source multiplicity="(0..*)" roleLabel="A" polymorphic="false">
                    <Class class="CustomBook"/>
                </Source>
                <Target multiplicity="(1..1)" roleLabel="B" polymorphic="false">
                    <Class class="Person"/>
                </Target>
            </ECRelationshipClass>
            <ECEntityClass typeName="CustomBook" modifier="abstract">
                <ECCustomAttributes>
                    <QueryView>
                        <Query>
                            SELECT
                                [ECInstanceId],
                                ec_classid('TestSchema', 'CustomBook') as [ECClassId],
                                NAVIGATION_VALUE(ts.Book.Author, 1, 2)
                            FROM ts.Book
                        </Query>
                    </QueryView>
                </ECCustomAttributes>
                <ECNavigationProperty propertyName="Author" relationshipName="CustomBookHasAuthor" direction="Forward"/>
            </ECEntityClass>
        </ECSchema>)xml")));

    auto* schema = m_ecdb.Schemas().GetSchema("TestSchema");
    m_personClassId = schema->GetClassCP("Person")->GetId();
    m_bookClassId = schema->GetClassCP("Book")->GetId();
    m_bookHasAuthorClassId = schema->GetClassCP("BookHasAuthor")->GetId();

    {  // Insert Person
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Person(Name) VALUES('Sanderson')"));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(m_personInstanceKey));
    }

    {  // Insert Order
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Book(Title, Author) VALUES('Mistborn', ?)"));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(1, m_personInstanceKey.GetInstanceId(), m_bookHasAuthorClassId));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(m_bookInstanceKey));
    }
}

/*---------------------------------------------------------------------------------**/ /**
 * @bsimethod
 +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavValueTestFixture, SimpleSelectNavValue) {
    {  // construct from 3 static parameters
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT NAVIGATION_VALUE(ts.Book.Author, 1, 2)"));
        // printf("%s\n", stmt.GetNativeSql());
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);

        ECClassId relClassId;
        ECInstanceId instId = stmt.GetValueNavigation<ECInstanceId>(0, &relClassId);
        ASSERT_EQ(ECClassId(UINT64_C(2)), relClassId);
        ASSERT_EQ(ECInstanceId(UINT64_C(1)), instId);
        auto& colInfo = stmt.GetColumnInfo(0);
        ASSERT_TRUE(colInfo.IsValid());
        ASSERT_TRUE(colInfo.IsGeneratedProperty());
        ASSERT_TRUE(colInfo.GetDataType().IsNavigation());

        auto* property = colInfo.GetProperty();
        ASSERT_TRUE(property != nullptr);
        ASSERT_STREQ("Author", property->GetName().c_str());
        ASSERT_TRUE(property->GetIsNavigation());
    }

    {  // construct from 2 static parameters (RelClassId should be taken from nav prop)
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT NAVIGATION_VALUE(ts.Book.Author, 1)"));
        //("%s\n", stmt.GetNativeSql());
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);

        ECClassId relClassId;
        ECInstanceId instId = stmt.GetValueNavigation<ECInstanceId>(0, &relClassId);
        ASSERT_EQ(m_bookHasAuthorClassId, relClassId);
        ASSERT_EQ(ECInstanceId(UINT64_C(1)), instId);
        auto& colInfo = stmt.GetColumnInfo(0);
        ASSERT_TRUE(colInfo.IsValid());
        ASSERT_TRUE(colInfo.IsGeneratedProperty());
        ASSERT_TRUE(colInfo.GetDataType().IsNavigation());

        auto* property = colInfo.GetProperty();
        ASSERT_TRUE(property != nullptr);
        ASSERT_STREQ("Author", property->GetName().c_str());
        ASSERT_TRUE(property->GetIsNavigation());
    }

    {  // construct from parameters
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT NAVIGATION_VALUE(ts.Book.Author, ?, ?)"));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, m_personInstanceKey.GetInstanceId()));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, m_bookHasAuthorClassId));
        // printf("%s\n", stmt.GetNativeSql());
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);

        ECClassId relClassId;
        ECInstanceId instId = stmt.GetValueNavigation<ECInstanceId>(0, &relClassId);
        ASSERT_EQ(m_bookHasAuthorClassId, relClassId);
        ASSERT_EQ(m_personInstanceKey.GetInstanceId(), instId);
        auto& colInfo = stmt.GetColumnInfo(0);
        ASSERT_TRUE(colInfo.IsValid());
        ASSERT_TRUE(colInfo.IsGeneratedProperty());
        ASSERT_TRUE(colInfo.GetDataType().IsNavigation());

        auto* property = colInfo.GetProperty();
        ASSERT_TRUE(property != nullptr);
        ASSERT_STREQ("Author", property->GetName().c_str());
        ASSERT_TRUE(property->GetIsNavigation());
    }

    {  // construct from actual nav property row
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT NAVIGATION_VALUE(ts.Book.Author, Author.Id, Author.RelECClassId) [MyNavProp] FROM ts.Book LIMIT 1"));
        // printf("%s\n", stmt.GetNativeSql());
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);

        ECClassId relClassId;
        ECInstanceId instId = stmt.GetValueNavigation<ECInstanceId>(0, &relClassId);
        ASSERT_EQ(m_bookHasAuthorClassId, relClassId);
        ASSERT_EQ(m_personInstanceKey.GetInstanceId(), instId);
        auto& colInfo = stmt.GetColumnInfo(0);
        ASSERT_TRUE(colInfo.IsValid());
        ASSERT_TRUE(colInfo.IsGeneratedProperty());
        ASSERT_TRUE(colInfo.GetDataType().IsNavigation());

        auto* property = colInfo.GetProperty();
        ASSERT_TRUE(property != nullptr);
        ASSERT_STREQ("MyNavProp", property->GetName().c_str());
        ASSERT_TRUE(property->GetIsNavigation());
    }

    {  // check if ECSql handles duplicate names in NAV function
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT NAVIGATION_VALUE(ts.Book.Author, 1, 2), NAVIGATION_VALUE(ts.Book.Author, 3, 4)"));
        // printf("%s\n", stmt.GetNativeSql());
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);

        ECClassId relClassId;
        ECInstanceId instId = stmt.GetValueNavigation<ECInstanceId>(0, &relClassId);
        ASSERT_EQ(ECClassId(UINT64_C(2)), relClassId);
        ASSERT_EQ(ECInstanceId(UINT64_C(1)), instId);
        auto& firstColInfo = stmt.GetColumnInfo(0);
        ASSERT_TRUE(firstColInfo.IsValid());
        ASSERT_TRUE(firstColInfo.IsGeneratedProperty());
        ASSERT_TRUE(firstColInfo.GetDataType().IsNavigation());

        auto* property = firstColInfo.GetProperty();
        ASSERT_TRUE(property != nullptr);
        ASSERT_STREQ("Author", property->GetName().c_str());
        ASSERT_TRUE(property->GetIsNavigation());

        instId = stmt.GetValueNavigation<ECInstanceId>(1, &relClassId);
        ASSERT_EQ(ECClassId(UINT64_C(4)), relClassId);
        ASSERT_EQ(ECInstanceId(UINT64_C(3)), instId);
        auto& secondColInfo = stmt.GetColumnInfo(1);
        ASSERT_TRUE(secondColInfo.IsValid());
        ASSERT_TRUE(secondColInfo.IsGeneratedProperty());
        ASSERT_TRUE(secondColInfo.GetDataType().IsNavigation());

        property = secondColInfo.GetProperty();
        ASSERT_TRUE(property != nullptr);
        ASSERT_STREQ("Author_1", property->GetName().c_str());
        ASSERT_TRUE(property->GetIsNavigation());
    }

    {  // check if fields are added correctly
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Author, Author.Id, Author, Author.RelECClassId, Author from ts.CustomBook"));
        // printf("%s\n", stmt.GetNativeSql());
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);

        ECClassId relClassId;
        ECInstanceId instId = stmt.GetValueNavigation<ECInstanceId>(0, &relClassId);
        ASSERT_EQ(stmt.GetValueId<ECInstanceId>(3), relClassId);
        ASSERT_EQ(stmt.GetValueId<ECInstanceId>(1), instId);
        auto& firstColInfo = stmt.GetColumnInfo(0);
        ASSERT_TRUE(firstColInfo.IsValid());
        ASSERT_FALSE(firstColInfo.IsGeneratedProperty());
        ASSERT_TRUE(firstColInfo.GetDataType().IsNavigation());

        instId = stmt.GetValueNavigation<ECInstanceId>(2, &relClassId);
        ASSERT_EQ(stmt.GetValueId<ECInstanceId>(3), relClassId);
        ASSERT_EQ(stmt.GetValueId<ECInstanceId>(1), instId);
        auto& thirdColInfo = stmt.GetColumnInfo(2);
        ASSERT_TRUE(thirdColInfo.IsValid());
        ASSERT_TRUE(thirdColInfo.IsGeneratedProperty());
        ASSERT_TRUE(thirdColInfo.GetDataType().IsNavigation());

        instId = stmt.GetValueNavigation<ECInstanceId>(4, &relClassId);
        ASSERT_EQ(stmt.GetValueId<ECInstanceId>(3), relClassId);
        ASSERT_EQ(stmt.GetValueId<ECInstanceId>(1), instId);
        auto& fifthColInfo = stmt.GetColumnInfo(4);
        ASSERT_TRUE(fifthColInfo.IsValid());
        ASSERT_TRUE(fifthColInfo.IsGeneratedProperty());
        ASSERT_TRUE(fifthColInfo.GetDataType().IsNavigation());

        auto* property = firstColInfo.GetProperty();
        ASSERT_TRUE(property != nullptr);
        ASSERT_STREQ("Author", property->GetName().c_str());
        ASSERT_TRUE(property->GetIsNavigation());

        property = stmt.GetColumnInfo(1).GetProperty();
        ASSERT_TRUE(property != nullptr);
        ASSERT_STREQ("Author.Id", property->GetDisplayLabel().c_str());
        ASSERT_TRUE(property->GetIsPrimitive());

        property = thirdColInfo.GetProperty();
        ASSERT_TRUE(property != nullptr);
        ASSERT_STREQ("Author_1", property->GetName().c_str());
        ASSERT_TRUE(property->GetIsNavigation());

        property = stmt.GetColumnInfo(3).GetProperty();
        ASSERT_TRUE(property != nullptr);
        ASSERT_STREQ("Author.RelECClassId", property->GetDisplayLabel().c_str());
        ASSERT_TRUE(property->GetIsPrimitive());

        property = fifthColInfo.GetProperty();
        ASSERT_TRUE(property != nullptr);
        ASSERT_STREQ("Author_2", property->GetName().c_str());
        ASSERT_TRUE(property->GetIsNavigation());
    }

    {  // check if SELECT * works with NAV functions
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * from meta.PropertyCustomAttribute"));
        // printf("%s\n", stmt.GetNativeSql());
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);
    }

    {  // check if NAV function works in select
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Author, Author.Id, Author.RelECClassId FROM (SELECT NAVIGATION_VALUE(ts.Book.Author, 1, 2)) WHERE Author.Id = 1 AND Author.RelECClassId = 2"));
        // printf("%s\n", stmt.GetNativeSql());
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);

        ECClassId relClassId;
        ECInstanceId instId = stmt.GetValueId<ECInstanceId>(0);
        instId = stmt.GetValueNavigation<ECInstanceId>(0, &relClassId);
        ASSERT_EQ(ECInstanceId(UINT64_C(2)), relClassId);
        ASSERT_EQ(ECInstanceId(UINT64_C(1)), instId);
        auto& firstColInfo = stmt.GetColumnInfo(0);
        ASSERT_TRUE(firstColInfo.IsValid());
        ASSERT_TRUE(firstColInfo.IsGeneratedProperty());
        ASSERT_TRUE(firstColInfo.GetDataType().IsNavigation());

        auto* property = firstColInfo.GetProperty();
        ASSERT_TRUE(property != nullptr);
        ASSERT_STREQ("Author", property->GetName().c_str());
        ASSERT_TRUE(property->GetIsNavigation());

        instId = stmt.GetValueId<ECInstanceId>(1);
        ASSERT_EQ(ECInstanceId(UINT64_C(1)), instId);
        auto& secondColInfo = stmt.GetColumnInfo(1);
        ASSERT_TRUE(secondColInfo.IsValid());
        ASSERT_TRUE(secondColInfo.IsGeneratedProperty());
        ASSERT_TRUE(secondColInfo.GetDataType().IsPrimitive());

        property = secondColInfo.GetProperty();
        ASSERT_TRUE(property != nullptr);
        ASSERT_STREQ("Author.Id", property->GetDisplayLabel().c_str());
        ASSERT_TRUE(property->GetIsPrimitive());

        instId = stmt.GetValueId<ECInstanceId>(2);
        ASSERT_EQ(ECInstanceId(UINT64_C(2)), instId);
        auto& thirdColInfo = stmt.GetColumnInfo(2);
        ASSERT_TRUE(thirdColInfo.IsValid());
        ASSERT_TRUE(thirdColInfo.IsGeneratedProperty());
        ASSERT_TRUE(thirdColInfo.GetDataType().IsPrimitive());

        property = thirdColInfo.GetProperty();
        ASSERT_TRUE(property != nullptr);
        ASSERT_STREQ("Author.RelECClassId", property->GetDisplayLabel().c_str());
        ASSERT_TRUE(property->GetIsPrimitive());
    }

    {  // check if NAV function can take a query as arguments
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT NAVIGATION_VALUE(ts.Book.Author, (SELECT COUNT(*) FROM ts.Book), ((SELECT COUNT(*) FROM ts.Book) + 1))"));
        // printf("%s\n", stmt.GetNativeSql());
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);

        ECClassId relClassId;
        ECInstanceId instId = stmt.GetValueNavigation<ECInstanceId>(0, &relClassId);
        ASSERT_EQ(ECClassId(UINT64_C(2)), relClassId);
        ASSERT_EQ(ECInstanceId(UINT64_C(1)), instId);
        auto& colInfo = stmt.GetColumnInfo(0);
        ASSERT_TRUE(colInfo.IsValid());
        ASSERT_TRUE(colInfo.IsGeneratedProperty());
        ASSERT_TRUE(colInfo.GetDataType().IsNavigation());

        auto* property = colInfo.GetProperty();
        ASSERT_TRUE(property != nullptr);
        ASSERT_STREQ("Author", property->GetName().c_str());
        ASSERT_TRUE(property->GetIsNavigation());
    }
}

/*---------------------------------------------------------------------------------**/ /**
 * @bsimethod
 +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavValueTestFixture, SelectNavValueErrorScenarios) {
    {  // NAV function should fail in CTE
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "with a(NavProp) AS (SELECT NAVIGATION_VALUE(ts.Book.Author, 1, 2)) SELECT * FROM a"));
    }

    {  // NAV function should fail in CTE
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "with a(id, relid) AS (SELECT NAVIGATION_VALUE(ts.Book.Author, 1, 2)) SELECT * FROM a"));
    }

    {  // first arg is not a relationship property
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT NAVIGATION_VALUE(ts.Book.Title, 1, 2)"));
    }

    {  // more than 3 args
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT NAVIGATION_VALUE(ts.Book.Author, 1, 2, 3)"));
    }

    {  // schema does not exist
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT NAVIGATION_VALUE(NonExistingSchema.Book.Author, 1, 2)"));
    }

    {  // class does not exist
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT NAVIGATION_VALUE(ts.NonExistingClass.Author, 1, 2)"));
    }

    {  // property does not exist
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT NAVIGATION_VALUE(ts.book.NonExistingProperty, 1, 2)"));
    }

    {  // string instead of id
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT NAVIGATION_VALUE(ts.Book.Author, 'one', 'two')"));
    }

    {  // id's should be >= 0
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT NAVIGATION_VALUE(ts.Book.Author, -1)"));
    }

    {  // id's should be >= 0
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT NAVIGATION_VALUE(ts.Book.Author, 5, 0)"));
    }

    {  // NAV function should fail if id's are double type
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT NAVIGATION_VALUE(ts.Book.Author, 1.15, 7.15)"));
    }

    {  // should fail if first argument is not a property path
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT NAVIGATION_VALUE(1, 1, 1"));
    }

    {  // should fail if first argument is not a property path
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT NAVIGATION_VALUE('value', 1, 1"));
    }
}

END_ECDBUNITTESTS_NAMESPACE