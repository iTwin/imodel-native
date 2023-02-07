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

    auto allSchemas = m_ecdb.Schemas().GetSchemas(true);

    ASSERT_NE(0, allSchemas.size());
}

TEST_F(ECDbValidityCheckTests, ClassIdCheck) {
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb"));

    ECSqlStatement checkStatement;

  	ASSERT_EQ(ECSqlStatus::Success, checkStatement.Prepare(m_ecdb, "PRAGMA class_id_check"));

    ASSERT_EQ(BE_SQLITE_ROW, checkStatement.Step());
    ASSERT_STREQ("Passed", checkStatement.GetValueText(0));
    ASSERT_STREQ("", checkStatement.GetValueText(1));
}


TEST_F(ECDbValidityCheckTests, NavigationPropertyIdCheck) {
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb"));

    ECSqlStatement checkStatement;

  	ASSERT_EQ(ECSqlStatus::Success, checkStatement.Prepare(m_ecdb, "PRAGMA nav_prop_id_check"));

    ASSERT_EQ(BE_SQLITE_ROW, checkStatement.Step());
    ASSERT_STREQ("Passed", checkStatement.GetValueText(0));
    ASSERT_STREQ("", checkStatement.GetValueText(1));
}

TEST_F(ECDbValidityCheckTests, FailingNavigationPropertyIdCheck) {
    Utf8String schemaName = "TestSchema";
    Utf8String classA = "ClassA";
    Utf8String classB = "ClassB";
    Utf8String property = "TestProperty";
    Utf8String relClass = "RelClass";

    SchemaItem schemaItem(Utf8PrintfString(
    "<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
    "   <ECEntityClass typeName='ClassA' modifier='Abstract' >"
    "       <ECProperty propertyName='propA' typeName='int' />"
    "   </ECEntityClass>"
    "   <ECEntityClass typeName='ClassB' modifier='Abstract' >"
    "       <ECProperty propertyName='propB' typeName='int' />"
    "       <ECNavigationProperty propertyName='TestProperty' relationshipName='RelClass' direction='Backward' />"
    "   </ECEntityClass>"
    "   <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' strengthDirection='forward' >"
    "       <Source cardinality='(0,1)' polymorphic='True'>"
    "           <Class class='ClassA' />"
    "       </Source>"
    "       <Target cardinality='(0,N)' polymorphic='True'>"
    "           <Class class='ClassB' />"
    "       </Target>"
    "    </ECRelationshipClass>"
    "</ECSchema>"));

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    ECClassId relClassId = m_ecdb.Schemas().GetClassId(schemaName.c_str(), relClass.c_str());
    ECClassId classBId = m_ecdb.Schemas().GetClassId(schemaName.c_str(), classB.c_str());
    ECClassId classAId = m_ecdb.Schemas().GetClassId(schemaName.c_str(), classA.c_str());

    Utf8PrintfString sqlQuery("UPDATE ec_Property SET NavigationRelationshipClassId = %s WHERE ClassId = %s AND Name = '%s'",
        classAId.ToHexStr().c_str(), classBId.ToHexStr().c_str(), "TestProperty");

    ASSERT_EQ( DbResult::BE_SQLITE_OK, m_ecdb.ExecuteSql(sqlQuery.c_str()));

    SqlPrintfString ecsqlQuery("SELECT NavigationRelationshipClass.id FROM meta.ECPropertyDef WHERE class.Id = %s AND Name = '%s'",
        classBId.ToHexStr().c_str(), property.c_str());

	ECSqlStatement illegalModificationCheck;

  	ASSERT_EQ(ECSqlStatus::Success, illegalModificationCheck.Prepare(m_ecdb, ecsqlQuery));

    ASSERT_EQ(BE_SQLITE_ROW, illegalModificationCheck.Step());
    auto newId = illegalModificationCheck.GetValueUInt64(0);
    ASSERT_EQ(classAId.GetValue(), newId);

	ECSqlStatement checkStatement;

  	ASSERT_EQ(ECSqlStatus::Success, checkStatement.Prepare(m_ecdb, "PRAGMA nav_prop_id_check"));

    ASSERT_EQ(BE_SQLITE_ROW, checkStatement.Step());
    ASSERT_STREQ("Failed", checkStatement.GetValueText(0));
    auto details = checkStatement.GetValueText(1);
    ASSERT_STREQ(Utf8PrintfString("Could not find classId of navigation property %s in class %s:%s", property.c_str(), schemaName.c_str(), classB.c_str()).c_str(), details);
	}

TEST_F(ECDbValidityCheckTests, ValidateCheck) {
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb"));

    ECSqlStatement checkStatement;

  	ASSERT_EQ(ECSqlStatus::Success, checkStatement.Prepare(m_ecdb, "PRAGMA validate"));

    while(BE_SQLITE_ROW == checkStatement.Step()) {
        ASSERT_STREQ("Passed", checkStatement.GetValueText(1));
    }
}

TEST_F(ECDbValidityCheckTests, ValidateCheckFailingNavPropertyCheck) {
    Utf8String schemaName = "TestSchema";
    Utf8String classA = "ClassA";
    Utf8String classB = "ClassB";
    Utf8String property = "TestProperty";
    Utf8String relClass = "RelClass";

    SchemaItem schemaItem(Utf8PrintfString(
    "<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
    "   <ECEntityClass typeName='ClassA' modifier='Abstract' >"
    "       <ECProperty propertyName='propA' typeName='int' />"
    "   </ECEntityClass>"
    "   <ECEntityClass typeName='ClassB' modifier='Abstract' >"
    "       <ECProperty propertyName='propB' typeName='int' />"
    "       <ECNavigationProperty propertyName='TestProperty' relationshipName='RelClass' direction='Backward' />"
    "   </ECEntityClass>"
    "   <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' strengthDirection='forward' >"
    "       <Source cardinality='(0,1)' polymorphic='True'>"
    "           <Class class='ClassA' />"
    "       </Source>"
    "       <Target cardinality='(0,N)' polymorphic='True'>"
    "           <Class class='ClassB' />"
    "       </Target>"
    "    </ECRelationshipClass>"
    "</ECSchema>"));

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    ECClassId relClassId = m_ecdb.Schemas().GetClassId(schemaName.c_str(), relClass.c_str());
    ECClassId classBId = m_ecdb.Schemas().GetClassId(schemaName.c_str(), classB.c_str());
    ECClassId classAId = m_ecdb.Schemas().GetClassId(schemaName.c_str(), classA.c_str());

    Utf8PrintfString sqlQuery("UPDATE ec_Property SET NavigationRelationshipClassId = %s WHERE ClassId = %s AND Name = '%s'",
        classAId.ToHexStr().c_str(), classBId.ToHexStr().c_str(), "TestProperty");

    ASSERT_EQ( DbResult::BE_SQLITE_OK, m_ecdb.ExecuteSql(sqlQuery.c_str()));

    SqlPrintfString ecsqlQuery("SELECT NavigationRelationshipClass.id FROM meta.ECPropertyDef WHERE class.Id = %s AND Name = '%s'",
        classBId.ToHexStr().c_str(), property.c_str());

	ECSqlStatement illegalModificationCheck;

  	ASSERT_EQ(ECSqlStatus::Success, illegalModificationCheck.Prepare(m_ecdb, ecsqlQuery));

    ASSERT_EQ(BE_SQLITE_ROW, illegalModificationCheck.Step());
    auto newId = illegalModificationCheck.GetValueUInt64(0);
    ASSERT_EQ(classAId.GetValue(), newId);

    ECSqlStatement checkStatement;

  	ASSERT_EQ(ECSqlStatus::Success, checkStatement.Prepare(m_ecdb, "PRAGMA validate"));

	struct CheckData {
		Utf8String name;
		Utf8String status;
	};

    std::vector<CheckData> expectedResults = {
		{"nav_prop_id_check", "Failed"},
		{"class_id_check", "Did not run"},
	};

	for(auto result: expectedResults) {
		checkStatement.Step();
		ASSERT_STREQ(result.name.c_str(), checkStatement.GetValueText(0));
		ASSERT_STREQ(result.status.c_str(), checkStatement.GetValueText(1));
	}
}

END_ECDBUNITTESTS_NAMESPACE