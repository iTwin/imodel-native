/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <set>
#include <ECObjects/SchemaComparer.h>
#include <Bentley/BeDirectoryIterator.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
struct RelationshipMappingTestFixture : public ECDbTestFixture
    {
    Utf8String ConstructTestSchema(Utf8CP content, Utf8CP version = "01.00.00")
        {
        constexpr Utf8CP schemaTemplate = R"xml(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="BaseTPH" modifier="Abstract">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="PropBaseTPH" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="Element" modifier="Abstract">
            <BaseClass>BaseTPH</BaseClass>
            <ECProperty propertyName="PropElement" typeName="string"/>
          </ECEntityClass>
          %s
        </ECSchema>)xml";
        return Utf8PrintfString(schemaTemplate, version, content);
        };
    };

#define ASSERT_ECSQL(ECDB_OBJ, PREPARESTATUS, STEPSTATUS, ECSQL)   {\
                                                                    ECSqlStatement stmt;\
                                                                    ASSERT_EQ(PREPARESTATUS, stmt.Prepare(ECDB_OBJ, ECSQL));\
                                                                    if (PREPARESTATUS == ECSqlStatus::Success)\
                                                                        ASSERT_EQ(STEPSTATUS, stmt.Step());\
                                                                   }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, MoveNavUpInHierarchy)
    {
    //This should fail because changing the relationship on the nav prop is not allowed
    Utf8String schemaXml1 = ConstructTestSchema(R"xml(
        <ECEntityClass typeName="Material">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropMaterial" typeName="string"/>
        </ECEntityClass>

        <ECEntityClass typeName="MaterialProfileDefinition">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropMaterialProfileDefinition" typeName="string"/>
        </ECEntityClass>

        <ECEntityClass typeName="MaterialProfile">
            <BaseClass>MaterialProfileDefinition</BaseClass>
            <ECProperty propertyName="PropMaterialProfile" typeName="string"/>
            <ECNavigationProperty propertyName="Material" relationshipName="MaterialProfileRefersToMaterial" direction="Forward" />
        </ECEntityClass>

        <ECRelationshipClass typeName="MaterialProfileRefersToMaterial" strength="referencing" strengthDirection="Forward" modifier="Sealed">
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                <Class class="MaterialProfile" />
            </Source>
            <Target multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="true">
                <Class class="Material"/>
            </Target>
        </ECRelationshipClass>
        )xml");
    SchemaItem schema1(schemaXml1);
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDbForCurrentTest(schema1));

    Utf8String schemaXml2 = ConstructTestSchema(R"xml(
        <ECEntityClass typeName="Material">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropMaterial" typeName="string"/>
        </ECEntityClass>

        <ECEntityClass typeName="MaterialProfileDefinition">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropMaterialProfileDefinition" typeName="string"/>
            <ECNavigationProperty propertyName="Material" relationshipName="MaterialProfileDefinitionRefersToMaterial" direction="Forward" />
        </ECEntityClass>

        <ECEntityClass typeName="MaterialProfile">
            <BaseClass>MaterialProfileDefinition</BaseClass>
            <ECProperty propertyName="PropMaterialProfile" typeName="string"/>
            <ECNavigationProperty propertyName="Material" relationshipName="MaterialProfileRefersToMaterial" direction="Forward" />
        </ECEntityClass>

        <ECEntityClass typeName="TaperedMaterialProfile">
            <BaseClass>MaterialProfileDefinition</BaseClass>
            <ECProperty propertyName="PropTaperedMaterialProfileProfile" typeName="string"/>
        </ECEntityClass>

        <ECRelationshipClass typeName="MaterialProfileDefinitionRefersToMaterial" strength="Referencing" strengthDirection="Forward" modifier="None">
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                <Class class="MaterialProfileDefinition" />
            </Source>
            <Target multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="true">
                <Class class="Material"/>
            </Target>
        </ECRelationshipClass>

        <ECRelationshipClass typeName="MaterialProfileRefersToMaterial" strength="Referencing" strengthDirection="Forward" modifier="Sealed">
            <BaseClass>MaterialProfileDefinitionRefersToMaterial</BaseClass>
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                <Class class="MaterialProfile" />
            </Source>
            <Target multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="true">
                <Class class="Material"/>
            </Target>
        </ECRelationshipClass>
        )xml", "01.00.01");
    SchemaItem schema2(schemaXml2);
    ASSERT_EQ(BentleyStatus::ERROR, ImportSchema(schema2));
    //Error during schema deserialization (no issue reported) is:
    //The NavigationECProperty TestSchema:MaterialProfileDefinition:Material cannot be overridden by TestSchema:MaterialProfile:Material because the relationship was changed. A derived property cannot change the referenced relationship.
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, MoveNavUpInHierarchyRemoveOriginal)
    {
    Utf8String schemaXml1 = ConstructTestSchema(R"xml(
        <ECEntityClass typeName="Material">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropMaterial" typeName="string"/>
        </ECEntityClass>

        <ECEntityClass typeName="MaterialProfileDefinition">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropMaterialProfileDefinition" typeName="string"/>
        </ECEntityClass>

        <ECEntityClass typeName="MaterialProfile">
            <BaseClass>MaterialProfileDefinition</BaseClass>
            <ECProperty propertyName="PropMaterialProfile" typeName="string"/>
            <ECNavigationProperty propertyName="Material" relationshipName="MaterialProfileRefersToMaterial" direction="Forward" />
        </ECEntityClass>

        <ECRelationshipClass typeName="MaterialProfileRefersToMaterial" strength="referencing" strengthDirection="Forward" modifier="Sealed">
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                <Class class="MaterialProfile" />
            </Source>
            <Target multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="true">
                <Class class="Material"/>
            </Target>
        </ECRelationshipClass>
        )xml");
    SchemaItem schema1(schemaXml1);
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDbForCurrentTest(schema1));

    Utf8String schemaXml2 = ConstructTestSchema(R"xml(
        <ECEntityClass typeName="Material">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropMaterial" typeName="string"/>
        </ECEntityClass>

        <ECEntityClass typeName="MaterialProfileDefinition">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropMaterialProfileDefinition" typeName="string"/>
            <ECNavigationProperty propertyName="Material" relationshipName="MaterialProfileDefinitionRefersToMaterial" direction="Forward" />
        </ECEntityClass>

        <ECEntityClass typeName="MaterialProfile">
            <BaseClass>MaterialProfileDefinition</BaseClass>
            <ECProperty propertyName="PropMaterialProfile" typeName="string"/>
        </ECEntityClass>

        <ECEntityClass typeName="TaperedMaterialProfile">
            <BaseClass>MaterialProfileDefinition</BaseClass>
            <ECProperty propertyName="PropTaperedMaterialProfileProfile" typeName="string"/>
        </ECEntityClass>

        <ECRelationshipClass typeName="MaterialProfileDefinitionRefersToMaterial" strength="Referencing" strengthDirection="Forward" modifier="None">
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                <Class class="MaterialProfileDefinition" />
            </Source>
            <Target multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="true">
                <Class class="Material"/>
            </Target>
        </ECRelationshipClass>

        <ECRelationshipClass typeName="MaterialProfileRefersToMaterial" strength="Referencing" strengthDirection="Forward" modifier="Sealed">
            <BaseClass>MaterialProfileDefinitionRefersToMaterial</BaseClass>
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                <Class class="MaterialProfile" />
            </Source>
            <Target multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="true">
                <Class class="Material"/>
            </Target>
        </ECRelationshipClass>
        )xml", "01.00.01");
    SchemaItem schema2(schemaXml2);
    ECIssueListener issueListener(m_ecdb);
    ASSERT_EQ(BentleyStatus::ERROR, ImportSchema(schema2));
    auto lastIssue = issueListener.GetIssue();
    ASSERT_TRUE(lastIssue.has_value()) << "Should raise an issue.";
    ASSERT_STREQ("ECSchema Upgrade failed. ECClass TestSchema:MaterialProfile: Deleting Navigation ECProperty 'Material' from an ECClass is not supported.", lastIssue.message.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, MoveNavUpInHierarchyRemoveAndIgnore)
    {
    //Same as above but setting the DoNotFailSchemaValidationForLegacyIssues flag on schema import to work around the previous error
    Utf8String schemaXml1 = ConstructTestSchema(R"xml(
        <ECEntityClass typeName="Material">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropMaterial" typeName="string"/>
        </ECEntityClass>

        <ECEntityClass typeName="MaterialProfileDefinition">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropMaterialProfileDefinition" typeName="string"/>
        </ECEntityClass>

        <ECEntityClass typeName="MaterialProfile">
            <BaseClass>MaterialProfileDefinition</BaseClass>
            <ECProperty propertyName="PropMaterialProfile" typeName="string"/>
            <ECNavigationProperty propertyName="Material" relationshipName="MaterialProfileRefersToMaterial" direction="Forward" />
        </ECEntityClass>

        <ECRelationshipClass typeName="MaterialProfileRefersToMaterial" strength="referencing" strengthDirection="Forward" modifier="Sealed">
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                <Class class="MaterialProfile" />
            </Source>
            <Target multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="true">
                <Class class="Material"/>
            </Target>
        </ECRelationshipClass>
        )xml");
    SchemaItem schema1(schemaXml1);
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDbForCurrentTest(schema1));

    Utf8String schemaXml2 = ConstructTestSchema(R"xml(
        <ECEntityClass typeName="Material">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropMaterial" typeName="string"/>
        </ECEntityClass>

        <ECEntityClass typeName="MaterialProfileDefinition">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropMaterialProfileDefinition" typeName="string"/>
            <ECNavigationProperty propertyName="Material" relationshipName="MaterialProfileDefinitionRefersToMaterial" direction="Forward" />
        </ECEntityClass>

        <ECEntityClass typeName="MaterialProfile">
            <BaseClass>MaterialProfileDefinition</BaseClass>
            <ECProperty propertyName="PropMaterialProfile" typeName="string"/>
        </ECEntityClass>

        <ECEntityClass typeName="TaperedMaterialProfile">
            <BaseClass>MaterialProfileDefinition</BaseClass>
            <ECProperty propertyName="PropTaperedMaterialProfileProfile" typeName="string"/>
        </ECEntityClass>

        <ECRelationshipClass typeName="MaterialProfileDefinitionRefersToMaterial" strength="Referencing" strengthDirection="Forward" modifier="None">
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                <Class class="MaterialProfileDefinition" />
            </Source>
            <Target multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="true">
                <Class class="Material"/>
            </Target>
        </ECRelationshipClass>

        <ECRelationshipClass typeName="MaterialProfileRefersToMaterial" strength="Referencing" strengthDirection="Forward" modifier="Sealed">
            <BaseClass>MaterialProfileDefinitionRefersToMaterial</BaseClass>
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                <Class class="MaterialProfile" />
            </Source>
            <Target multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="true">
                <Class class="Material"/>
            </Target>
        </ECRelationshipClass>
        )xml", "01.00.01");
    SchemaItem schema2(schemaXml2);
    ECIssueListener issueListener(m_ecdb);
    ASSERT_EQ(BentleyStatus::ERROR, ImportSchema(schema2, SchemaManager::SchemaImportOptions::DoNotFailSchemaValidationForLegacyIssues));
    auto lastIssue = issueListener.GetIssue();
    ASSERT_TRUE(lastIssue.has_value()) << "Should raise an issue.";
    ASSERT_STREQ("ECSchema Upgrade failed. ECClass TestSchema:MaterialProfile: Deleting Navigation ECProperty 'Material' from an ECClass is not supported.", lastIssue.message.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, MoveNavUpInHierarchyAdjustOriginal)
    {
    //Adjust the existing nav prop to use the same relationship as the new base nav prop.
    //This currently fails because changing the relationship class of an existing nav prop is not allowed.
    Utf8String schemaXml1 = ConstructTestSchema(R"xml(
        <ECEntityClass typeName="Material">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropMaterial" typeName="string"/>
        </ECEntityClass>

        <ECEntityClass typeName="MaterialProfileDefinition">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropMaterialProfileDefinition" typeName="string"/>
        </ECEntityClass>

        <ECEntityClass typeName="MaterialProfile">
            <BaseClass>MaterialProfileDefinition</BaseClass>
            <ECProperty propertyName="PropMaterialProfile" typeName="string"/>
            <ECNavigationProperty propertyName="Material" relationshipName="MaterialProfileRefersToMaterial" direction="Forward" />
        </ECEntityClass>

        <ECRelationshipClass typeName="MaterialProfileRefersToMaterial" strength="referencing" strengthDirection="Forward" modifier="Sealed">
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                <Class class="MaterialProfile" />
            </Source>
            <Target multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="true">
                <Class class="Material"/>
            </Target>
        </ECRelationshipClass>
        )xml");
    SchemaItem schema1(schemaXml1);
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDbForCurrentTest(schema1));

    Utf8String schemaXml2 = ConstructTestSchema(R"xml(
        <ECEntityClass typeName="Material">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropMaterial" typeName="string"/>
        </ECEntityClass>

        <ECEntityClass typeName="MaterialProfileDefinition">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropMaterialProfileDefinition" typeName="string"/>
            <ECNavigationProperty propertyName="Material" relationshipName="MaterialProfileDefinitionRefersToMaterial" direction="Forward" />
        </ECEntityClass>

        <ECEntityClass typeName="MaterialProfile">
            <BaseClass>MaterialProfileDefinition</BaseClass>
            <ECProperty propertyName="PropMaterialProfile" typeName="string"/>
            <ECNavigationProperty propertyName="Material" relationshipName="MaterialProfileDefinitionRefersToMaterial" direction="Forward" />
        </ECEntityClass>

        <ECEntityClass typeName="TaperedMaterialProfile">
            <BaseClass>MaterialProfileDefinition</BaseClass>
            <ECProperty propertyName="PropTaperedMaterialProfileProfile" typeName="string"/>
        </ECEntityClass>

        <ECRelationshipClass typeName="MaterialProfileDefinitionRefersToMaterial" strength="Referencing" strengthDirection="Forward" modifier="None">
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                <Class class="MaterialProfileDefinition" />
            </Source>
            <Target multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="true">
                <Class class="Material"/>
            </Target>
        </ECRelationshipClass>

        <ECRelationshipClass typeName="MaterialProfileRefersToMaterial" strength="Referencing" strengthDirection="Forward" modifier="Sealed">
            <BaseClass>MaterialProfileDefinitionRefersToMaterial</BaseClass>
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                <Class class="MaterialProfile" />
            </Source>
            <Target multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="true">
                <Class class="Material"/>
            </Target>
        </ECRelationshipClass>
        )xml", "01.00.01");
    SchemaItem schema2(schemaXml2);
    ECIssueListener issueListener(m_ecdb);
    ASSERT_EQ(BentleyStatus::ERROR, ImportSchema(schema2, SchemaManager::SchemaImportOptions::DoNotFailSchemaValidationForLegacyIssues));
    auto lastIssue = issueListener.GetIssue();
    ASSERT_TRUE(lastIssue.has_value()) << "Should raise an issue.";
    ASSERT_STREQ("ECSchema Upgrade failed. ECProperty TestSchema:MaterialProfile.Material: Changing the 'Relationship' for a Navigation ECProperty is not supported.", lastIssue.message.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, SpatialAnalysisMapProblem)
    {
    /*
    Simulates attempted changes made to SpatialAnalysis where a RelClassIdColumn with the same name is supposed to be both
    Physical (for the base relationship) and Virtual (for the derived one).
    Bug was fixed so this should not produce an error.
    */
    Utf8String schemaXml1 = ConstructTestSchema(R"xml(
        <ECEntityClass typeName="PhysicalMaterial" modifier="Abstract">
            <BaseClass>Element</BaseClass>
        </ECEntityClass>

        <ECEntityClass typeName="Material" modifier="Sealed">
            <BaseClass>PhysicalMaterial</BaseClass>
            <ECProperty propertyName="PropMaterial" typeName="string"/>
        </ECEntityClass>

        <ECEntityClass typeName="AnalyticalType" description="Defines a shared set of properties (the 'type') that can be associated with an anlyt:AnalyticalElement. It is not meant to replace a bis:PhysicalType if it is available." displayLabel="Analytical Type" modifier="Abstract">
            <BaseClass>Element</BaseClass>
        </ECEntityClass>

        <ECEntityClass typeName="SurfaceType" modifier="Abstract" displayLabel="Surface Type" description="Defines a Surface Type for IStructuralSurface.">
            <BaseClass>AnalyticalType</BaseClass>
        </ECEntityClass>

        <ECEntityClass typeName="SimpleSurfaceType" modifier="Sealed" description="Simple Surface Type.">
            <BaseClass>SurfaceType</BaseClass>
            <ECProperty propertyName="Thickness" typeName="double" description="Thickness of IStructuralSurfaces."/>
            <ECNavigationProperty propertyName="Material" relationshipName="SimpleSurfaceTypeRefersToMaterial" direction="Forward" displayLabel="Material" description="Material of all IStructuralSurfaces with this type." />
        </ECEntityClass>

        <ECRelationshipClass typeName="SimpleSurfaceTypeRefersToMaterial" strength="referencing" strengthDirection="Forward" modifier="Sealed">
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                <Class class="SimpleSurfaceType" />
            </Source>
            <Target multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="true">
                <Class class="PhysicalMaterial"/>
            </Target>
        </ECRelationshipClass>

        <ECEntityClass typeName="MaterialProfileDefinition" modifier="Abstract">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropMaterialProfileDefinition" typeName="string"/>
            <ECNavigationProperty propertyName="Material" relationshipName="MaterialProfileDefinitionRefersToPhysicalMaterial" direction="Forward" description="Referenced PhysicalMaterial." />
        </ECEntityClass>

        <ECEntityClass typeName="MaterialProfile" modifier="Sealed">
            <BaseClass>MaterialProfileDefinition</BaseClass>
            <ECProperty propertyName="PropMaterialProfile" typeName="string"/>
        </ECEntityClass>

        <ECRelationshipClass typeName="MaterialProfileDefinitionRefersToPhysicalMaterial" strength="referencing" strengthDirection="Forward" modifier="None">
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                <Class class="MaterialProfileDefinition" />
            </Source>
            <Target multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="true">
                <Class class="PhysicalMaterial"/>
            </Target>
        </ECRelationshipClass>

        <ECRelationshipClass typeName="MaterialProfileRefersToMaterial" strength="referencing" strengthDirection="Forward" modifier="Sealed">
            <BaseClass>MaterialProfileDefinitionRefersToPhysicalMaterial</BaseClass>
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                <Class class="MaterialProfile" />
            </Source>
            <Target multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="true">
                <Class class="PhysicalMaterial"/>
            </Target>
        </ECRelationshipClass>
        )xml");
    SchemaItem schema1(schemaXml1);
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDbForCurrentTest(schema1));

    ECInstanceKey materialKey;
    ECClassId relClassId = m_ecdb.Schemas().GetClassId("TestSchema", "MaterialProfileDefinitionRefersToPhysicalMaterial");
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Material(PropMaterial,PropElement,PropBaseTPH) VALUES('1PropMaterial','1PropElement','1PropBaseTPH')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(materialKey));
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.MaterialProfile(PropMaterialProfile,PropMaterialProfileDefinition,PropElement,PropBaseTPH,Material) VALUES('2PropMaterialProfile','2PropMaterialProfileDefinition','2PropElement','2PropBaseTPH',?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(1, materialKey.GetInstanceId(), relClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, SpatialAnalysisMapProblemWithoutSharedColumns)
    {
    /*
    Same as the above test, however without using shared columns.
    This means there should be a MaterialRelClassIdAlt column instead of using a shared column in the final ecdb.
    However, the foreign key column cannot be reused in this case, which causes this test to fail currently.
    */
    Utf8String schemaXml1 = ConstructTestSchema(R"xml(
        <ECEntityClass typeName="BaseTPHNonShared" modifier="Abstract">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
            </ECCustomAttributes>
            <ECProperty propertyName="PropBaseTPH" typeName="string"/>
        </ECEntityClass>

        <ECEntityClass typeName="PhysicalMaterial" modifier="Abstract">
            <BaseClass>BaseTPHNonShared</BaseClass>
        </ECEntityClass>

        <ECEntityClass typeName="Material" modifier="Sealed">
            <BaseClass>PhysicalMaterial</BaseClass>
            <ECProperty propertyName="PropMaterial" typeName="string"/>
        </ECEntityClass>

        <ECEntityClass typeName="AnalyticalType" description="Defines a shared set of properties (the 'type') that can be associated with an anlyt:AnalyticalElement. It is not meant to replace a bis:PhysicalType if it is available." displayLabel="Analytical Type" modifier="Abstract">
            <BaseClass>BaseTPHNonShared</BaseClass>
        </ECEntityClass>

        <ECEntityClass typeName="SurfaceType" modifier="Abstract" displayLabel="Surface Type" description="Defines a Surface Type for IStructuralSurface.">
            <BaseClass>AnalyticalType</BaseClass>
        </ECEntityClass>

        <ECEntityClass typeName="SimpleSurfaceType" modifier="Sealed" description="Simple Surface Type.">
            <BaseClass>SurfaceType</BaseClass>
            <ECProperty propertyName="Thickness" typeName="double" description="Thickness of IStructuralSurfaces."/>
            <ECNavigationProperty propertyName="Material" relationshipName="SimpleSurfaceTypeRefersToMaterial" direction="Forward" displayLabel="Material" description="Material of all IStructuralSurfaces with this type." />
        </ECEntityClass>

        <ECRelationshipClass typeName="SimpleSurfaceTypeRefersToMaterial" strength="referencing" strengthDirection="Forward" modifier="Sealed">
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                <Class class="SimpleSurfaceType" />
            </Source>
            <Target multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="true">
                <Class class="PhysicalMaterial"/>
            </Target>
        </ECRelationshipClass>

        <ECEntityClass typeName="MaterialProfileDefinition" modifier="Abstract">
            <BaseClass>BaseTPHNonShared</BaseClass>
            <ECProperty propertyName="PropMaterialProfileDefinition" typeName="string"/>
            <ECNavigationProperty propertyName="Material" relationshipName="MaterialProfileDefinitionRefersToPhysicalMaterial" direction="Forward" description="Referenced PhysicalMaterial." />
        </ECEntityClass>

        <ECEntityClass typeName="MaterialProfile" modifier="Sealed">
            <BaseClass>MaterialProfileDefinition</BaseClass>
            <ECProperty propertyName="PropMaterialProfile" typeName="string"/>
        </ECEntityClass>

        <ECRelationshipClass typeName="MaterialProfileDefinitionRefersToPhysicalMaterial" strength="referencing" strengthDirection="Forward" modifier="None">
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                <Class class="MaterialProfileDefinition" />
            </Source>
            <Target multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="true">
                <Class class="PhysicalMaterial"/>
            </Target>
        </ECRelationshipClass>

        <ECRelationshipClass typeName="MaterialProfileRefersToMaterial" strength="referencing" strengthDirection="Forward" modifier="Sealed">
            <BaseClass>MaterialProfileDefinitionRefersToPhysicalMaterial</BaseClass>
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                <Class class="MaterialProfile" />
            </Source>
            <Target multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="true">
                <Class class="PhysicalMaterial"/>
            </Target>
        </ECRelationshipClass>
        )xml");
    SchemaItem schema1(schemaXml1);
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDbForCurrentTest());
    ECIssueListener issueListener(m_ecdb);
    ASSERT_EQ(BentleyStatus::ERROR, ImportSchema(schema1));
    auto lastIssue = issueListener.GetIssue();
    ASSERT_TRUE(lastIssue.has_value()) << "Should raise an issue.";
    ASSERT_STREQ("Failed to map ECRelationshipClass 'TestSchema:MaterialProfileDefinitionRefersToPhysicalMaterial'. ForeignKey column name 'MaterialId' is already used by another column in the table 'ts_BaseTPHNonShared'.", lastIssue.message.c_str());

    //Uncomment this part once we start supporting the scenario
    /*ECInstanceKey materialKey;
    ECClassId relClassId = m_ecdb.Schemas().GetClassId("TestSchema", "MaterialProfileDefinitionRefersToPhysicalMaterial");
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Material(PropMaterial,PropElement,PropBaseTPH) VALUES('1PropMaterial','1PropElement','1PropBaseTPH')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(materialKey));
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.MaterialProfile(PropMaterialProfile,PropMaterialProfileDefinition,PropElement,PropBaseTPH,Material) VALUES('2PropMaterialProfile','2PropMaterialProfileDefinition','2PropElement','2PropBaseTPH',?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(1, materialKey.GetInstanceId(), relClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }*/
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, ManyFKRelsWithSameName)
    {
    /* This test simulates 4 different FK relationships going into the same table (BaseTPH) while using the same navigation property name.
        2 of the nav props should use virtual relClassId columns (sealed relationship) and 2 should use real physical columns
    */
    Utf8String schemaXml1 = ConstructTestSchema(R"xml(
        <ECEntityClass typeName="Material" modifier="Sealed">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropMaterial" typeName="string"/>
        </ECEntityClass>

        <ECEntityClass typeName="SealedEntity1" modifier="Sealed">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropSealedEntity1" typeName="string" />
            <ECNavigationProperty propertyName="NavProp" relationshipName="SealedEntity1RefersToMaterial" direction="Forward" />
        </ECEntityClass>

        <ECRelationshipClass typeName="SealedEntity1RefersToMaterial" strength="referencing" strengthDirection="Forward" modifier="Sealed">
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                <Class class="SealedEntity1" />
            </Source>
            <Target multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="true">
                <Class class="Material"/>
            </Target>
        </ECRelationshipClass>

        <ECEntityClass typeName="SealedEntity2" modifier="Sealed">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropSealedEntity2" typeName="string" />
            <ECNavigationProperty propertyName="NavProp" relationshipName="SealedEntity2RefersToMaterial" direction="Forward" />
        </ECEntityClass>

        <ECRelationshipClass typeName="SealedEntity2RefersToMaterial" strength="referencing" strengthDirection="Forward" modifier="Sealed">
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                <Class class="SealedEntity2" />
            </Source>
            <Target multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="true">
                <Class class="Material"/>
            </Target>
        </ECRelationshipClass>

        <ECEntityClass typeName="NonSealedEntity1">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropNonSealedEntity1" typeName="string"/>
            <ECNavigationProperty propertyName="NavProp" relationshipName="NonSealedEntity1RefersToMaterial" direction="Forward" />
        </ECEntityClass>

        <ECRelationshipClass typeName="NonSealedEntity1RefersToMaterial" strength="referencing" strengthDirection="Forward" modifier="None">
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                <Class class="NonSealedEntity1" />
            </Source>
            <Target multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="true">
                <Class class="Material"/>
            </Target>
        </ECRelationshipClass>

        <ECEntityClass typeName="NonSealedEntity2">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropNonSealedEntity2" typeName="string"/>
            <ECNavigationProperty propertyName="NavProp" relationshipName="NonSealedEntity2RefersToMaterial" direction="Forward" />
        </ECEntityClass>

        <ECRelationshipClass typeName="NonSealedEntity2RefersToMaterial" strength="referencing" strengthDirection="Forward" modifier="None">
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                <Class class="NonSealedEntity2" />
            </Source>
            <Target multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="true">
                <Class class="Material"/>
            </Target>
        </ECRelationshipClass>
        )xml");
    SchemaItem schema1(schemaXml1);
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDbForCurrentTest(schema1));

    std::vector<ECInstanceKey> materialKeys(4);
    auto insertMaterial = [&](int index)
        {
        ECSqlStatement stmt;
        Utf8PrintfString sql("INSERT INTO ts.Material(PropMaterial,PropElement,PropBaseTPH) VALUES('%dPropMaterial','%dPropElement','%dPropBaseTPH')", index, index, index);
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, sql.c_str()));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(materialKeys[index]));
        };
    
    for(int i = 0; i < 4; i++)
    {
    insertMaterial(i);
    }

    std::vector<ECInstanceKey> elementKeys(4);
    auto insertElement = [&](int index, Utf8CP className)
        {
        ECClassId relClassId = m_ecdb.Schemas().GetClassId("TestSchema", Utf8PrintfString("%sRefersToMaterial", className));
        ECSqlStatement stmt;
        Utf8PrintfString sql("INSERT INTO ts.%s(Prop%s,PropElement,PropBaseTPH,NavProp) VALUES('%d%s','%dPropElement','%dPropBaseTPH',?)", className, className, index+4, className, index+4, index+4);
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, sql.c_str()));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(1, materialKeys[index].GetInstanceId(), relClassId));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(elementKeys[index]));
        };

    insertElement(0, "SealedEntity1");
    insertElement(1, "SealedEntity2");
    insertElement(2, "NonSealedEntity1");
    insertElement(3, "NonSealedEntity2");

    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT PropSealedEntity1, PropElement, PropBaseTPH, NavProp FROM ts.SealedEntity1");
    ASSERT_EQ(JsonValue(R"json([{"PropSealedEntity1":"4SealedEntity1", "PropElement":"4PropElement", "PropBaseTPH": "4PropBaseTPH", "NavProp": {"id":"0x1","relClassName":"TestSchema.SealedEntity1RefersToMaterial"}}])json"),result) << "Verify inserted SealedEntity1 instances";
    }

    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT PropSealedEntity2, PropElement, PropBaseTPH, NavProp FROM ts.SealedEntity2");
    ASSERT_EQ(JsonValue(R"json([{"PropSealedEntity2":"5SealedEntity2", "PropElement":"5PropElement", "PropBaseTPH": "5PropBaseTPH", "NavProp": {"id":"0x2","relClassName":"TestSchema.SealedEntity2RefersToMaterial"}}])json"),result) << "Verify inserted SealedEntity2 instances";
    }

    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT PropNonSealedEntity1, PropElement, PropBaseTPH, NavProp FROM ts.NonSealedEntity1");
    ASSERT_EQ(JsonValue(R"json([{"PropNonSealedEntity1":"6NonSealedEntity1", "PropElement":"6PropElement", "PropBaseTPH": "6PropBaseTPH", "NavProp": {"id":"0x3","relClassName":"TestSchema.NonSealedEntity1RefersToMaterial"}}])json"),result) << "Verify inserted NonSealedEntity1 instances";
    }

    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT PropNonSealedEntity2, PropElement, PropBaseTPH, NavProp FROM ts.NonSealedEntity2");
    ASSERT_EQ(JsonValue(R"json([{"PropNonSealedEntity2":"7NonSealedEntity2", "PropElement":"7PropElement", "PropBaseTPH": "7PropBaseTPH", "NavProp": {"id":"0x4","relClassName":"TestSchema.NonSealedEntity2RefersToMaterial"}}])json"),result) << "Verify inserted NonSealedEntity2 instances";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, FKRelsWithDifferentNotNullConstraints)
    {
    /* This test simulates 2 different FK relationships going into the same table (BaseTPH) while using the same navigation property name.
        One of them uses a NotNull constraint while the other does not.
    */
    Utf8String schemaXml1 = ConstructTestSchema(R"xml(
        <ECEntityClass typeName="Material" modifier="Sealed">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropMaterial" typeName="string"/>
        </ECEntityClass>

        <ECEntityClass typeName="SealedEntity1" modifier="Sealed">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropSealedEntity1" typeName="string" />
            <ECNavigationProperty propertyName="NavProp" relationshipName="MaterialOwnsSealedEntity1" direction="Backward">
                <ECCustomAttributes>
                    <ForeignKeyConstraint xmlns="ECDbMap.02.00.00">
                        <OnDeleteAction>Cascade</OnDeleteAction>
                    </ForeignKeyConstraint>
                </ECCustomAttributes>
            </ECNavigationProperty>
        </ECEntityClass>

        <ECRelationshipClass typeName="MaterialOwnsSealedEntity1" strength="embedding" modifier="Sealed">
            <Source multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="false">
                <Class class="Material"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                <Class class="SealedEntity1" />
            </Target>
        </ECRelationshipClass>

        <ECEntityClass typeName="SealedEntity2" modifier="Sealed">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropSealedEntity2" typeName="string" />
            <ECNavigationProperty propertyName="NavProp" relationshipName="SealedEntity2RefersToMaterial" direction="Forward" />
        </ECEntityClass>

        <ECRelationshipClass typeName="SealedEntity2RefersToMaterial" strength="referencing" strengthDirection="Forward" modifier="Sealed">
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                <Class class="SealedEntity2" />
            </Source>
            <Target multiplicity="(0..1)" roleLabel="is referenced by" polymorphic="true">
                <Class class="Material"/>
            </Target>
        </ECRelationshipClass>
        )xml");
    SchemaItem schema1(schemaXml1);
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDbForCurrentTest());
    ECIssueListener issueListener(m_ecdb);
    ASSERT_EQ(BentleyStatus::ERROR, ImportSchema(schema1));
    auto lastIssue = issueListener.GetIssue();
    ASSERT_TRUE(lastIssue.has_value()) << "Should raise an issue.";
    ASSERT_STREQ("Failed to map ECRelationshipClass 'TestSchema:SealedEntity2RefersToMaterial'. ForeignKey column name 'NavPropId' is already used by another column in the table 'ts_BaseTPH'.", lastIssue.message.c_str());
    }

END_ECDBUNITTESTS_NAMESPACE
