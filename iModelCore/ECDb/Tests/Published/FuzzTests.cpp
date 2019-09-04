/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

struct FuzzTestFixture : ECDbTestFixture {};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  03/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FuzzTestFixture, FuzzedECInstanceXml)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("fuzz_getderivedclasses.ecdb"));
    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA" />
                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                <ECEntityClass typeName="MyBase" >
                 <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                  </ECCustomAttributes>
                <ECProperty propertyName="Name" typeName="string" />
              </ECEntityClass>
              <ECEntityClass typeName="MySub" >
                 <ECCustomAttributes>
                    <ClassHasCurrentTimeStampProperty xmlns="CoreCustomAttributes.01.00">
                        <PropertyName>LastMod</PropertyName>
                    </ClassHasCurrentTimeStampProperty>
                  </ECCustomAttributes>
                <BaseClass>MyBase</BaseClass>
                <ECProperty propertyName="LastMod" typeName="dateTime" readOnly="True">
                 <ECCustomAttributes>
                    <DateTimeInfo xmlns="CoreCustomAttributes.01.00">
                        <DateTimeKind>Utc</DateTimeKind>
                    </DateTimeInfo>
                  </ECCustomAttributes>
                </ECProperty>
              </ECEntityClass>
            </ECSchema>)xml")));

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.MyBase(Name) VALUES('Base-1')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.MySub(Name) VALUES('Sub-1')"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());

    ECClassId subClassId;
    {
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb()); // reopen to clear all caches
    ECClassCP subClass = m_ecdb.Schemas().GetClass("TestSchema", "MySub");
    ASSERT_TRUE(subClass != nullptr);
    subClassId = subClass->GetId();
    }

    {
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb()); // reopen to clear all caches
    ECClassCP baseClass = m_ecdb.Schemas().GetClass("TestSchema", "MyBase");
    ASSERT_TRUE(baseClass != nullptr);
    ASSERT_TRUE(baseClass->GetDerivedClasses().empty()) << "Derived classes not loaded explicitly";
    ASSERT_EQ(1, m_ecdb.Schemas().GetDerivedClasses(*baseClass).size()) << "Derived classes loaded explicitly";

    ASSERT_EQ(JsonValue(R"json([{"cnt": 2}])json"), GetHelper().ExecuteSelectECSql("SELECT count(*) cnt FROM ts.MyBase"));
    }

    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb()); // reopen to clear all caches
    {
    //Corrupt the CA XML of the subclass
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "UPDATE ec_CustomAttribute SET Instance='<ClassHasCurrentTimeStampProperty' WHERE ContainerId=? And ContainerType=?"));
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindId(1, subClassId));
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindInt(2, (int) ECN::CustomAttributeContainerType::AnyClass));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    ASSERT_EQ(1, m_ecdb.GetModifiedRowCount()) << "Only one CA XML should have been corrupted";
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    }

    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

    //check various API calls how to react to the fuzzed CA XML
    
    {
    //working against sub class having the fuzzed CA XML
    ScopedDisableFailOnAssertion disableFailOnAssertion;
    EXPECT_TRUE(m_ecdb.Schemas().GetClass("TestSchema", "MySub") == nullptr) << "Loading class is expected to fail";

    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT count(*) FROM ts.MySub"));
    }

    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    
    //working against base class of the sub class having the fuzzed CA XML
    {
    ECClassCP baseClass = m_ecdb.Schemas().GetClass("TestSchema", "MyBase");
    ASSERT_TRUE(baseClass != nullptr);
    EXPECT_TRUE(baseClass->GetDerivedClasses().empty()) << "Derived classes not loaded explicitly";

    ScopedDisableFailOnAssertion disableFailOnAssertion;
    EXPECT_TRUE(m_ecdb.Schemas().GetDerivedClasses(*baseClass).empty()) << "Loading derived classes is expected to fail, so derived class list remains empty";
    }

    {
    ScopedDisableFailOnAssertion disableFailOnAssertion;
    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT count(*) FROM ts.MyBase")) << "Expected to fail because subclass of MyBase has fuzzed CustomAttribute";
    }
    }

END_ECDBUNITTESTS_NAMESPACE
