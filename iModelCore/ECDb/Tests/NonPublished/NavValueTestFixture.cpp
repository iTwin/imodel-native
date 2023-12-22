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
struct NavValueTestFixture : ECDbTestFixture
    {
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
void NavValueTestFixture::SetUp()
    {
    NativeLogging::Logging::SetLogger(&NativeLogging::ConsoleLogger::GetLogger());
    NativeLogging::ConsoleLogger::GetLogger().SetSeverity("ECDb", BentleyApi::NativeLogging::LOG_TRACE);
    NativeLogging::ConsoleLogger::GetLogger().SetSeverity("ECObjectsNative", BentleyApi::NativeLogging::LOG_TRACE);
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
        </ECSchema>)xml"
    )));

    auto* schema = m_ecdb.Schemas().GetSchema("TestSchema");
    m_personClassId = schema->GetClassCP("Person")->GetId();
    m_bookClassId = schema->GetClassCP("Book")->GetId();
    m_bookHasAuthorClassId = schema->GetClassCP("BookHasAuthor")->GetId();

    { //Insert Person
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Person(Name) VALUES('Sanderson')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(m_personInstanceKey));
    }

    { //Insert Order
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Book(Title, Author) VALUES('Mistborn', ?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(1, m_personInstanceKey.GetInstanceId(), m_bookHasAuthorClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(m_bookInstanceKey));
    }
    }

//Select NAV(PropertyCustomAttribute.Property, 1) x FROM foo WHERE x.Id=2 && x.RelECClassId=3
    //    ECPropertyNameExpression
//Select Nav(Foo:Parent, Parent.Id, Parent.RelECClassId) parent2 FROM foo WHERE parent2.Id=2 && parent2.RelECClassId=3

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavValueTestFixture, SimpleSelectNavValue) {
    {
        { // construct from 3 static parameters
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT NAV(ts.Book.Author, 1, 2)"));
        printf("%s\n", stmt.GetNativeSql());
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);

        ECClassId relClassId;
        ECInstanceId instId = stmt.GetValueNavigation<ECInstanceId>(0, &relClassId);
        ASSERT_EQ(ECClassId(2ull), relClassId);
        ASSERT_EQ(ECInstanceId(1ull), instId);
        auto& colInfo = stmt.GetColumnInfo(0);
        ASSERT_TRUE(colInfo.IsValid());
        ASSERT_TRUE(colInfo.IsGeneratedProperty());
        ASSERT_TRUE(colInfo.GetDataType().IsNavigation());

        auto* property = colInfo.GetProperty();
        ASSERT_TRUE(property != nullptr);
        ASSERT_STREQ("Author", property->GetName().c_str());
        ASSERT_TRUE(property->GetIsNavigation());
        }

        { // construct from 2 static parameters (RelClassId should be taken from nav prop)
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT NAV(ts.Book.Author, 1)"));
        printf("%s\n", stmt.GetNativeSql());
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);

        ECClassId relClassId;
        ECInstanceId instId = stmt.GetValueNavigation<ECInstanceId>(0, &relClassId);
        ASSERT_EQ(m_bookHasAuthorClassId, relClassId);
        ASSERT_EQ(ECInstanceId(1ull), instId);
        auto& colInfo = stmt.GetColumnInfo(0);
        ASSERT_TRUE(colInfo.IsValid());
        ASSERT_TRUE(colInfo.IsGeneratedProperty());
        ASSERT_TRUE(colInfo.GetDataType().IsNavigation());

        auto* property = colInfo.GetProperty();
        ASSERT_TRUE(property != nullptr);
        ASSERT_STREQ("Author", property->GetName().c_str());
        ASSERT_TRUE(property->GetIsNavigation());
        }

        { // construct from parameters
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT NAV(ts.Book.Author, ?, ?)"));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, m_personInstanceKey.GetInstanceId()));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, m_bookHasAuthorClassId));
        printf("%s\n", stmt.GetNativeSql());
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

        { // construct from actual nav property row
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT NAV(ts.Book.Author, Author.Id, Author.RelECClassId) [MyNavProp] FROM ts.Book LIMIT 1"));
        printf("%s\n", stmt.GetNativeSql());
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

        { // check if ECSql handles duplicate names in NAV function
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT NAV(ts.Book.Author, 1, 2), NAV(ts.Book.Author, 3, 4)"));
        printf("%s\n", stmt.GetNativeSql());
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);

        ECClassId relClassId;
        ECInstanceId instId = stmt.GetValueNavigation<ECInstanceId>(0, &relClassId);
        ASSERT_EQ(ECClassId(2ull), relClassId);
        ASSERT_EQ(ECInstanceId(1ull), instId);
        auto& firstColInfo = stmt.GetColumnInfo(0);
        ASSERT_TRUE(firstColInfo.IsValid());
        ASSERT_TRUE(firstColInfo.IsGeneratedProperty());
        ASSERT_TRUE(firstColInfo.GetDataType().IsNavigation());

        auto* property = firstColInfo.GetProperty();
        ASSERT_TRUE(property != nullptr);
        ASSERT_STREQ("Author", property->GetName().c_str());
        ASSERT_TRUE(property->GetIsNavigation());

        instId = stmt.GetValueNavigation<ECInstanceId>(1, &relClassId);
        ASSERT_EQ(ECClassId(4ull), relClassId);
        ASSERT_EQ(ECInstanceId(3ull), instId);
        auto& secondColInfo = stmt.GetColumnInfo(1);
        ASSERT_TRUE(secondColInfo.IsValid());
        ASSERT_TRUE(secondColInfo.IsGeneratedProperty());
        ASSERT_TRUE(secondColInfo.GetDataType().IsNavigation());

        property = secondColInfo.GetProperty();
        ASSERT_TRUE(property != nullptr);
        ASSERT_STREQ("Author_1", property->GetName().c_str());
        ASSERT_TRUE(property->GetIsNavigation());
        }

        // TODO: create a ECClass with NAV function in another test and move next two test to the new test suite.
        { // check if fields are added correctly
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Property, Property.Id, Property, Property.RelECClassId, Property from meta.PropertyCustomAttribute"));
        printf("%s\n", stmt.GetNativeSql());
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);

        ECClassId relClassId;
        ECInstanceId instId = stmt.GetValueNavigation<ECInstanceId>(0, &relClassId);
        ASSERT_EQ(stmt.GetValueId<ECInstanceId>(3), relClassId);
        ASSERT_EQ(stmt.GetValueId<ECInstanceId>(1), instId);
        auto& firstColInfo = stmt.GetColumnInfo(0);
        ASSERT_TRUE(firstColInfo.IsValid());
        ASSERT_TRUE(firstColInfo.IsGeneratedProperty());
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
        ASSERT_STREQ("Property", property->GetName().c_str());
        ASSERT_TRUE(property->GetIsNavigation());

        property = stmt.GetColumnInfo(1).GetProperty();
        ASSERT_TRUE(property != nullptr);
        ASSERT_STREQ("Property.Id", property->GetDisplayLabel().c_str());
        ASSERT_TRUE(property->GetIsPrimitive());

        property = thirdColInfo.GetProperty();
        ASSERT_TRUE(property != nullptr);
        ASSERT_STREQ("Property_1", property->GetName().c_str());
        ASSERT_TRUE(property->GetIsNavigation());

        property = stmt.GetColumnInfo(3).GetProperty();
        ASSERT_TRUE(property != nullptr);
        ASSERT_STREQ("Property.RelECClassId", property->GetDisplayLabel().c_str());
        ASSERT_TRUE(property->GetIsPrimitive());

        property = fifthColInfo.GetProperty();
        ASSERT_TRUE(property != nullptr);
        ASSERT_STREQ("Property_2", property->GetName().c_str());
        ASSERT_TRUE(property->GetIsNavigation());
        }

        { // check if SELECT * works with NAV functions
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * from meta.PropertyCustomAttribute"));
        printf("%s\n", stmt.GetNativeSql());
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);
        }

        { // check if NAV function works in select
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Author, Author.Id, Author.RelECClassId FROM (SELECT NAV(ts.Book.Author, 1, 2)) WHERE Author.Id = 1 AND Author.RelECClassId = 2"));
        printf("%s\n", stmt.GetNativeSql());
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);

        ECClassId relClassId;
        ECInstanceId instId = stmt.GetValueId<ECInstanceId>(0);
        instId = stmt.GetValueNavigation<ECInstanceId>(0, &relClassId);
        ASSERT_EQ(ECInstanceId(2ull), relClassId);
        ASSERT_EQ(ECInstanceId(1ull), instId);
        auto& firstColInfo = stmt.GetColumnInfo(0);
        ASSERT_TRUE(firstColInfo.IsValid());
        ASSERT_TRUE(firstColInfo.IsGeneratedProperty());
        ASSERT_TRUE(firstColInfo.GetDataType().IsNavigation());

        auto* property = firstColInfo.GetProperty();
        ASSERT_TRUE(property != nullptr);
        ASSERT_STREQ("Author", property->GetName().c_str());
        ASSERT_TRUE(property->GetIsNavigation());

        instId = stmt.GetValueId<ECInstanceId>(1);
        ASSERT_EQ(ECInstanceId(1ull), instId);
        auto& secondColInfo = stmt.GetColumnInfo(1);
        ASSERT_TRUE(secondColInfo.IsValid());
        ASSERT_TRUE(secondColInfo.IsGeneratedProperty());
        ASSERT_TRUE(secondColInfo.GetDataType().IsPrimitive());

        property = secondColInfo.GetProperty();
        ASSERT_TRUE(property != nullptr);
        ASSERT_STREQ("Author.Id", property->GetDisplayLabel().c_str());
        ASSERT_TRUE(property->GetIsPrimitive());

        instId = stmt.GetValueId<ECInstanceId>(2);
        ASSERT_EQ(ECInstanceId(2ull), instId);
        auto& thirdColInfo = stmt.GetColumnInfo(2);
        ASSERT_TRUE(thirdColInfo.IsValid());
        ASSERT_TRUE(thirdColInfo.IsGeneratedProperty());
        ASSERT_TRUE(thirdColInfo.GetDataType().IsPrimitive());

        property = thirdColInfo.GetProperty();
        ASSERT_TRUE(property != nullptr);
        ASSERT_STREQ("Author.RelECClassId", property->GetDisplayLabel().c_str());
        ASSERT_TRUE(property->GetIsPrimitive());
        }

        // TODO: maybe create seperate test for error scenarios?
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "with a(NavProp) AS (SELECT NAV(ts.Book.Author, 1, 2)) SELECT * FROM a"));
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "with a(id, relid) AS (SELECT NAV(ts.Book.Author, 1, 2)) SELECT * FROM a"));
        }
    }

}

END_ECDBUNITTESTS_NAMESPACE