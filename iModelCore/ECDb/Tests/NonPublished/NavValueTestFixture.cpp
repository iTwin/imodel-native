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

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM meta.PropertyCustomAttribute"));
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT NAV(meta.PropertyCustomAttribute.Class, 1, 2)"));
        }
    }

}

END_ECDBUNITTESTS_NAMESPACE