/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

struct ECDbValidityCheckTests : ECDbTestFixture
    {

    };

TEST_F(ECDbValidityCheckTests, LoadAllSchemas) {
    ASSERT_EQ(SUCCESS, SetupECDb("loadAllSchemas.ecdb", SchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema1" alias="ts1" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="Class1">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="A" typeName="string" />
            <ECProperty propertyName="B" typeName="string" />
            <ECProperty propertyName="C" typeName="string" />
            <ECProperty propertyName="D" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="Class2">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="D" typeName="string" />
            <ECProperty propertyName="C" typeName="string" />
            <ECProperty propertyName="B" typeName="string" />
            <ECProperty propertyName="A" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="Class3">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="C" typeName="string" />
            <ECProperty propertyName="A" typeName="string" />
            <ECProperty propertyName="D" typeName="string" />
            <ECProperty propertyName="B" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema")));

    ASSERT_EQ(SUCCESS,  GetHelper().ImportSchema(SchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema2" alias="ts2" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="Class1">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="A" typeName="string" />
            <ECProperty propertyName="B" typeName="string" />
            <ECProperty propertyName="C" typeName="string" />
            <ECProperty propertyName="D" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="Class2">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="D" typeName="string" />
            <ECProperty propertyName="C" typeName="string" />
            <ECProperty propertyName="B" typeName="string" />
            <ECProperty propertyName="A" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="Class3">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="C" typeName="string" />
            <ECProperty propertyName="A" typeName="string" />
            <ECProperty propertyName="D" typeName="string" />
            <ECProperty propertyName="B" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema")));

    auto allSchemas = m_ecdb.Schemas().GetSchemas(true);

    ASSERT_NE(0, allSchemas.size());
}

TEST_F(ECDbValidityCheckTests, LoadAllSchemasFail) {
	ASSERT_EQ(true, false);
}

TEST_F(ECDbValidityCheckTests, LoadAllClassMaps) {
	ASSERT_EQ(true, false);
}

TEST_F(ECDbValidityCheckTests, ClassIdCheck) {
    ASSERT_EQ(SUCCESS, SetupECDb("loadAllSchemas.bim"));

	ECSqlStatement statement;
  	ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb,
    "SELECT ECClassDef.Name, ECClassDef.Schema FROM meta.ClassHasBaseClasses as BaseClasses INNER JOIN meta.ECClassDef ON ECClassDef.ECInstanceId = ClassHasBaseClasses.TargetECInstanceId"));

    // join to class has base classes table

	// SELECT ECClassDef.Name, ECClassDef.Schema
	// FROM meta.ClassHasBaseClasses as BaseClasses 
	// INNER JOIN meta.ECClassDef ON ECClassDef.ECInstanceId = ClassHasBaseClasses.TargetECInstanceId 

	// Navigation properties

  	//SELECT ECInstanceId, Alias FROM meta.ECSchemaDef

  

 	// "SELECT COUNT(*) FROM bis.Element LEFT JOIN meta.ECClassDef on Element.ECClassId = ECClassDef.ECInstanceId WHERE ECClassDef.ECInstanceId IS NULL;" 

	std::vector<BentleyM0200::Utf8CP> names = {};
	std::vector<BentleyM0200::Utf8CP> schemas = {};

	while(BE_SQLITE_ROW == statement.Step())
	{
		names.push_back(statement.GetValueText(0));
		schemas.push_back(statement.GetValueText(0));
	}
	}

END_ECDBUNITTESTS_NAMESPACE