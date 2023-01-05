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

	  ECSqlStatement classDefStatement;

  	// ASSERT_EQ(ECSqlStatus::Success, classDefStatement.Prepare(m_ecdb,
    // SqlPrintfString("SELECT DISTINCT ec_classname(ECClassDef.ECInstanceId, 's.c') FROM meta.ClassHasAllBaseClasses INNER JOIN meta.ECClassDef ON ECClassDef.ECInstanceId = ClassHasAllBaseClasses.TargetECInstanceId WHERE ECClassDef.Type != %d AND ECClassDef.Type != %d AND ec_classname(ECClassDef.ECInstanceId, 's') != 'ECDbSystem'",
		// ECClassType::CustomAttribute, ECClassType::Struct)));

    ASSERT_EQ(ECSqlStatus::Success, classDefStatement.Prepare(m_ecdb, "SELECT ec_classname(ECInstanceId) FROM meta.ECClassDef"));

	  std::vector<BentleyM0200::Utf8CP> classNames = {};

	while(BE_SQLITE_ROW == classDefStatement.Step())
		{
		classNames.push_back(classDefStatement.GetValueText(0));
		}

	for(size_t i = 0; i < classNames.size(); i++)
		{
		ECSqlStatement CheckStatement;
		ASSERT_EQ(ECSqlStatus::Success, CheckStatement.Prepare(m_ecdb,
    	SqlPrintfString("SELECT COUNT(*) FROM %s LEFT JOIN meta.ECClassDef on Element.ECClassId = ECClassDef.ECInstanceId WHERE ECClassDef.ECInstanceId IS NULL", classNames[i])));
		ASSERT_EQ(BE_SQLITE_ROW, CheckStatement.Step());
		ASSERT_EQ(0, CheckStatement.GetValueInt(0));
		}
	}

END_ECDBUNITTESTS_NAMESPACE