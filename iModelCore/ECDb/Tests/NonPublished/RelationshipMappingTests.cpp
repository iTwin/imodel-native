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
    //! Helper for common schema pattern. It contains one base entity and an element class, so the tests do not have to repeat that part several times.
    Utf8String ConstructTestSchema(Utf8CP content, Utf8CP version = "01.00.00")
        {
        constexpr Utf8CP schemaTemplate = R"xml(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
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
          <ECEntityClass typeName="BaseMixin" modifier="Abstract">
            <ECCustomAttributes>
                <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                    <!-- Only subclasses of ts:Element can implement the BaseMixin interface -->
                    <AppliesToEntityClass>Element</AppliesToEntityClass>
                </IsMixin>
            </ECCustomAttributes>
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
    // Introduces a new nav prop one level above the existing one (existing is on MaterialProfile, new is on MaterialProfileDefinition) using the same property name.
    // Reflects a real scenario of what people attempted to do.
    // Expected result: Failing because the two nav props with the same name use different relationships
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
    // Attempts to move a nav prop to one level above while removing the original nav property so it does not run into the error of the previous test.
    // however, this scenario also does not work because deleting a nav property is under no circumstances permitted in today's code.
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
    TestIssueListener issueListener;
    m_ecdb.AddIssueListener(issueListener);
    ASSERT_EQ(BentleyStatus::ERROR, ImportSchema(schema2));

    ASSERT_FALSE(issueListener.IsEmpty()) << "Should raise an issue.";
    ASSERT_STREQ("ECSchema Upgrade failed. ECClass TestSchema:MaterialProfile: Deleting Navigation ECProperty 'Material' from an ECClass is not supported.", issueListener.GetLastMessage().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, MoveNavUpInHierarchyRemoveAndIgnore)
    {
    //Same as above but setting the DoNotFailForDeletionsOrModifications flag on schema import.
    //The flag currently does not affect nav props, the error is still the same, but it seems possible this could be adjusted in the future.
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
    TestIssueListener issueListener;
    m_ecdb.AddIssueListener(issueListener);
    ASSERT_EQ(BentleyStatus::ERROR, ImportSchema(schema2, SchemaManager::SchemaImportOptions::DoNotFailForDeletionsOrModifications));

    ASSERT_FALSE(issueListener.IsEmpty()) << "Should raise an issue.";
    ASSERT_STREQ("ECSchema Upgrade failed. ECClass TestSchema:MaterialProfile: Deleting Navigation ECProperty 'Material' from an ECClass is not supported.", issueListener.GetLastMessage().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, MoveNavUpInHierarchyAdjustOriginal)
    {
    //Introduce a new nav prop with the same name as an existing one, a level above in the hierarchy
    //This time, adjust the existing nav prop to use the same relationship as the new base nav prop.
    //This currently also fails because changing the relationship class of an existing nav prop is not allowed.
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
    TestIssueListener issueListener;
    m_ecdb.AddIssueListener(issueListener);
    ASSERT_EQ(BentleyStatus::ERROR, ImportSchema(schema2, SchemaManager::SchemaImportOptions::DoNotFailSchemaValidationForLegacyIssues));

    ASSERT_FALSE(issueListener.IsEmpty()) << "Should raise an issue.";
    ASSERT_STREQ("ECSchema Upgrade failed. ECProperty TestSchema:MaterialProfile.Material: Changing the 'Relationship' for a Navigation ECProperty is not supported.", issueListener.GetLastMessage().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, SpatialAnalysisMapProblem)
    {
    // Simulates attempted changes made to SpatialAnalysis which found a bug.
    // The RelClassIdColumn of the overwritten navigation property "Material" ends up being both
    // Physical (for the base relationship) and Virtual (for the overwritten one).
    // The same column cannot be both, so the bug threw an error.
    // Bug was fixed so this should import fine.
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
    // Same as the above test, however without using shared columns.
    // This produces a clash on the foreign-key column which cannot be reused for the overwritten property, but since this is not using
    // shared columns where we could fall back to a new column, this schema gets rejected.

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
    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema1));

    ECInstanceKey materialKey;
    ECClassId relClassId = m_ecdb.Schemas().GetClassId("TestSchema", "MaterialProfileDefinitionRefersToPhysicalMaterial");
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Material(PropMaterial,PropBaseTPH) VALUES('1PropMaterial','1PropBaseTPH')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(materialKey));
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.MaterialProfile(PropMaterialProfile,PropMaterialProfileDefinition,PropBaseTPH,Material) VALUES('2PropMaterialProfile','2PropMaterialProfileDefinition','2PropBaseTPH',?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(1, materialKey.GetInstanceId(), relClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, ManyFKRelsWithSameName)
    {
    // This test simulates 4 different FK relationships going into the same shared table (BaseTPH) while using the same navigation property name.
    // 2 of the nav props should use virtual relClassId columns (sealed relationship) and 2 should use real physical columns
    // The basic scenario is already covered in the tests above, but this here checks with more than 2 clashing properties in the same table
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
TEST_F(RelationshipMappingTestFixture, ManyFKRelsWithSameNameWithoutSharedColumns)
    {
    // Mimics the test above but without Sharing Columns
    Utf8String schemaXml1 = ConstructTestSchema(R"xml(
        <ECEntityClass typeName="BaseTPHNonShared" modifier="Abstract">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
            </ECCustomAttributes>
            <ECProperty propertyName="PropBaseTPH" typeName="string"/>
        </ECEntityClass>

        <ECEntityClass typeName="Material" modifier="Sealed">
            <BaseClass>BaseTPHNonShared</BaseClass>
            <ECProperty propertyName="PropMaterial" typeName="string"/>
        </ECEntityClass>

        <ECEntityClass typeName="SealedEntity1" modifier="Sealed">
            <BaseClass>BaseTPHNonShared</BaseClass>
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
            <BaseClass>BaseTPHNonShared</BaseClass>
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
            <BaseClass>BaseTPHNonShared</BaseClass>
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
            <BaseClass>BaseTPHNonShared</BaseClass>
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
        Utf8PrintfString sql("INSERT INTO ts.Material(PropMaterial,PropBaseTPH) VALUES('%dPropMaterial','%dPropBaseTPH')", index, index, index);
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
        Utf8PrintfString sql("INSERT INTO ts.%s(Prop%s,PropBaseTPH,NavProp) VALUES('%d%s','%dPropBaseTPH',?)", className, className, index+4, className, index+4, index+4);
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, sql.c_str()));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(1, materialKeys[index].GetInstanceId(), relClassId));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(elementKeys[index]));
        };

    insertElement(0, "SealedEntity1");
    insertElement(1, "SealedEntity2");
    insertElement(2, "NonSealedEntity1");
    insertElement(3, "NonSealedEntity2");

    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT PropSealedEntity1, PropBaseTPH, NavProp FROM ts.SealedEntity1");
    ASSERT_EQ(JsonValue(R"json([{"PropSealedEntity1":"4SealedEntity1", "PropBaseTPH": "4PropBaseTPH", "NavProp": {"id":"0x1","relClassName":"TestSchema.SealedEntity1RefersToMaterial"}}])json"),result) << "Verify inserted SealedEntity1 instances";
    }

    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT PropSealedEntity2, PropBaseTPH, NavProp FROM ts.SealedEntity2");
    ASSERT_EQ(JsonValue(R"json([{"PropSealedEntity2":"5SealedEntity2", "PropBaseTPH": "5PropBaseTPH", "NavProp": {"id":"0x2","relClassName":"TestSchema.SealedEntity2RefersToMaterial"}}])json"),result) << "Verify inserted SealedEntity2 instances";
    }

    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT PropNonSealedEntity1, PropBaseTPH, NavProp FROM ts.NonSealedEntity1");
    ASSERT_EQ(JsonValue(R"json([{"PropNonSealedEntity1":"6NonSealedEntity1", "PropBaseTPH": "6PropBaseTPH", "NavProp": {"id":"0x3","relClassName":"TestSchema.NonSealedEntity1RefersToMaterial"}}])json"),result) << "Verify inserted NonSealedEntity1 instances";
    }

    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT PropNonSealedEntity2, PropBaseTPH, NavProp FROM ts.NonSealedEntity2");
    ASSERT_EQ(JsonValue(R"json([{"PropNonSealedEntity2":"7NonSealedEntity2", "PropBaseTPH": "7PropBaseTPH", "NavProp": {"id":"0x4","relClassName":"TestSchema.NonSealedEntity2RefersToMaterial"}}])json"),result) << "Verify inserted NonSealedEntity2 instances";
    }
    }

TEST_F(RelationshipMappingTestFixture, FKRelsWithSameNameButDifferentNotNullConstraints)
    {
    // This test attempts to produce a problem where 2 nav props of the same name go into the same table (BaseTPH) and share a column.
    // However, the nav props differ in their "not null" constraint on the RelClassId column. Which could in theory end up producing an inconsistent constraint.
    // As the property has base ECClasses mapped to the same table, the not null constraint gets ignored.
    Utf8String schemaXml1 = ConstructTestSchema(R"xml(
        <ECEntityClass typeName="Material" modifier="Sealed">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropMaterial" typeName="string"/>
        </ECEntityClass>

        <ECEntityClass typeName="SealedEntity1" modifier="Sealed">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropSealedEntity1" typeName="string" />
            <ECNavigationProperty propertyName="NavProp" relationshipName="MaterialOwnsSealedEntity1" direction="Forward"/>
        </ECEntityClass>

        <ECRelationshipClass typeName="MaterialOwnsSealedEntity1" strength="referencing" strengthDirection="Forward" modifier="Sealed">
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                <Class class="SealedEntity1" />
            </Source>
            <Target multiplicity="(0..1)" roleLabel="is referenced by" polymorphic="true">
                <Class class="Material"/>
            </Target>
        </ECRelationshipClass>

        <ECEntityClass typeName="SealedEntity2" modifier="Sealed">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropSealedEntity2" typeName="string" />
            <ECNavigationProperty propertyName="NavProp" relationshipName="SealedEntity2RefersToMaterial" direction="Backward"/>
        </ECEntityClass>

        <ECRelationshipClass typeName="SealedEntity2RefersToMaterial" strength="embedding" modifier="Sealed">
            <Source multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="false">
                <Class class="Material"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                <Class class="SealedEntity2" />
            </Target>
        </ECRelationshipClass>
        )xml");
    SchemaItem schema1(schemaXml1);
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDbForCurrentTest());
    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema1));

    ECInstanceKey materialKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Material(PropMaterial,PropElement,PropBaseTPH) VALUES('1PropMaterial','1PropElement','1PropBaseTPH')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(materialKey));  // Insert should succeed even when NavProp is null
    }

    {
    ECSqlStatement stmt;
    ECClassId relClassId = m_ecdb.Schemas().GetClassId("TestSchema", "MaterialOwnsSealedEntity1");
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.SealedEntity1(PropSealedEntity1,PropElement,PropBaseTPH,NavProp) VALUES('1PropSealedEntity1','1PropElement','1PropBaseTPH',?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(1, materialKey.GetInstanceId(), relClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    {
    ECSqlStatement stmt;
    ECClassId relClassId = m_ecdb.Schemas().GetClassId("TestSchema", "SealedEntity2RefersToMaterial");
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.SealedEntity2(PropSealedEntity2,PropElement,PropBaseTPH,NavProp) VALUES('1PropSealedEntity2','2PropElement','2PropBaseTPH',?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(1, materialKey.GetInstanceId(), relClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT PropBaseTPH, PropElement, PropSealedEntity1, NavProp FROM ts.SealedEntity1");
    ASSERT_EQ(JsonValue(R"json([{"PropBaseTPH":"1PropBaseTPH", "PropElement": "1PropElement", "PropSealedEntity1": "1PropSealedEntity1", "NavProp": {"id":"0x1","relClassName":"TestSchema.MaterialOwnsSealedEntity1"}}])json"),result);
    }
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT PropBaseTPH, PropElement, PropSealedEntity2, NavProp FROM ts.SealedEntity2");
    ASSERT_EQ(JsonValue(R"json([{"PropBaseTPH":"2PropBaseTPH", "PropElement": "2PropElement", "PropSealedEntity2": "1PropSealedEntity2", "NavProp": {"id":"0x1","relClassName":"TestSchema.SealedEntity2RefersToMaterial"}}])json"),result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, FKRelsWithDifferentNotNullConstraintsWithoutSharedColumns)
    {
    // This test attempts to produce a problem where 2 nav props of the same name go into the same table (BaseTPH) but without sharing columns
    // However, the nav props differ in their "not null" constraint on the RelClassId column. Which could in theory end up producing an inconsistent constraint.
    // As the property has base ECClasses mapped to the same table, the not null constraint gets ignored.
    Utf8String schemaXml1 = ConstructTestSchema(R"xml(
        <ECEntityClass typeName="BaseTPHNonShared" modifier="Abstract">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
            </ECCustomAttributes>
            <ECProperty propertyName="PropBaseTPH" typeName="string"/>
        </ECEntityClass>

        <ECEntityClass typeName="Material" modifier="Sealed">
            <BaseClass>BaseTPHNonShared</BaseClass>
            <ECProperty propertyName="PropMaterial" typeName="string"/>
        </ECEntityClass>

        <ECEntityClass typeName="SealedEntity1" modifier="Sealed">
            <BaseClass>BaseTPHNonShared</BaseClass>
            <ECProperty propertyName="PropSealedEntity1" typeName="string" />
            <ECNavigationProperty propertyName="NavProp" relationshipName="MaterialOwnsSealedEntity1" direction="Backward" />
        </ECEntityClass>

        <ECRelationshipClass typeName="MaterialOwnsSealedEntity1" strength="referencing" modifier="Sealed">
            <Source multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="false">
                <Class class="BaseTPHNonShared"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                <Class class="SealedEntity1" />
            </Target>
        </ECRelationshipClass>

        <ECEntityClass typeName="SealedEntity2" modifier="Sealed">
            <BaseClass>BaseTPHNonShared</BaseClass>
            <ECProperty propertyName="PropSealedEntity2" typeName="string" />
            <ECNavigationProperty propertyName="NavProp" relationshipName="SealedEntity2RefersToMaterial" direction="Backward" />
        </ECEntityClass>

        <ECRelationshipClass typeName="SealedEntity2RefersToMaterial" strength="embedding" modifier="Sealed">
            <Source multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="false">
                <Class class="BaseTPHNonShared"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                <Class class="SealedEntity2" />
            </Target>
        </ECRelationshipClass>
        )xml");
    SchemaItem schema1(schemaXml1);
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDbForCurrentTest());
    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema1));

    ECInstanceKey materialKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Material(PropMaterial,PropBaseTPH) VALUES('1PropMaterial','1PropBaseTPH')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(materialKey));  // Insert should succeed even when NavProp is null
    }

    {
    ECSqlStatement stmt;
    ECClassId relClassId = m_ecdb.Schemas().GetClassId("TestSchema", "MaterialOwnsSealedEntity1");
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.SealedEntity1(PropSealedEntity1,PropBaseTPH,NavProp) VALUES('1PropSealedEntity1','1PropBaseTPH',?)"));
    stmt.BindNavigationValue(1, materialKey.GetInstanceId(), relClassId);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    {
    ECSqlStatement stmt;
    ECClassId relClassId = m_ecdb.Schemas().GetClassId("TestSchema", "SealedEntity2RefersToMaterial");
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.SealedEntity2(PropSealedEntity2,PropBaseTPH,NavProp) VALUES('1PropSealedEntity2','2PropBaseTPH',?)"));
    stmt.BindNavigationValue(1, materialKey.GetInstanceId(), relClassId);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "delete from ts.Material"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    }

TEST_F(RelationshipMappingTestFixture, IdenticalFKRelsWithDifferentConstraints)
    {
    // This test attempts to produce a problem where 2 nav props of the same name go into the same table (BaseTPH) and share a column.
    // The nav props have different on delete and on update constraints.
    // We end up creating 2 fks each with their own on delete and on update constraints.
    // However, only one version of the constraints is enforced which is the one that was added last (alphabetically the last relationship class added to the map)

    auto schemaXml = SchemaItem(ConstructTestSchema(R"xml(
        <ECEntityClass typeName="Material" modifier="Sealed">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropMaterial" typeName="string"/>
        </ECEntityClass>

        <ECEntityClass typeName="SealedEntity1" modifier="Sealed">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropSealedEntity1" typeName="string" />
            <ECNavigationProperty propertyName="NavProp" relationshipName="MaterialOwnsSealedEntity1" direction="Forward">
                <ECCustomAttributes>
                    <ForeignKeyConstraint xmlns="ECDbMap.2.0.0">
                        <OnDeleteAction>Restrict</OnDeleteAction>
                    </ForeignKeyConstraint>
                </ECCustomAttributes>
            </ECNavigationProperty>
        </ECEntityClass>

        <ECRelationshipClass typeName="MaterialOwnsSealedEntity1" strength="referencing" strengthDirection="Forward" modifier="Sealed">
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                <Class class="SealedEntity1" />
            </Source>
            <Target multiplicity="(0..1)" roleLabel="is referenced by" polymorphic="true">
                <Class class="Material"/>
            </Target>
        </ECRelationshipClass>

        <ECEntityClass typeName="SealedEntity2" modifier="Sealed">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropSealedEntity2" typeName="string" />
            <ECNavigationProperty propertyName="NavProp" relationshipName="SealedEntity2RefersToMaterial" direction="Backward">
                <ECCustomAttributes>
                    <ForeignKeyConstraint xmlns="ECDbMap.2.0.0">
                        <OnDeleteAction>Cascade</OnDeleteAction>
                    </ForeignKeyConstraint>
                </ECCustomAttributes>
            </ECNavigationProperty>
        </ECEntityClass>

        <ECRelationshipClass typeName="SealedEntity2RefersToMaterial" strength="embedding" modifier="Sealed">
            <Source multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="false">
                <Class class="Material"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                <Class class="SealedEntity2" />
            </Target>
        </ECRelationshipClass>
        )xml"));
    
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDbForCurrentTest());
    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schemaXml));

    EXPECT_STREQ(GetHelper().GetDdl("ts_BaseTPH").c_str(), "CREATE TABLE [ts_BaseTPH]([Id] INTEGER PRIMARY KEY, [ECClassId] INTEGER NOT NULL, [ps1] BLOB, [ps2] BLOB, [ps3] BLOB, [NavPropId] INTEGER, "
        "FOREIGN KEY([NavPropId]) REFERENCES [ts_BaseTPH]([Id]) ON DELETE RESTRICT, FOREIGN KEY([NavPropId]) REFERENCES [ts_BaseTPH]([Id]) ON DELETE CASCADE)");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, AddNavUpInHierarchyAndChangeRelationship)
    {
    //Introduce a new nav prop with the same name as an existing one in the base class
    //Also change relationship class Source/Target class to base class
    Utf8String schemaXml1 = ConstructTestSchema(R"xml(
        <ECEntityClass typeName="MaterialProfileDefinition" modifier="Abstract">
            <BaseClass>Element</BaseClass>
        </ECEntityClass>
        <ECEntityClass typeName="Material" modifier="Sealed">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropMaterial" typeName="string"/>
        </ECEntityClass>

        <ECEntityClass typeName="MaterialProfile" modifier="Sealed">
            <BaseClass>MaterialProfileDefinition</BaseClass>
            <ECProperty propertyName="PropMaterialProfile" typeName="string" />
            <ECNavigationProperty propertyName="Material" relationshipName="MaterialProfileRefersToMaterial" direction="Forward" />
        </ECEntityClass>

        <ECRelationshipClass typeName="MaterialProfileRefersToMaterial" strength="referencing" modifier="Sealed">
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                <Class class="MaterialProfile" />
            </Source>
            <Target multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="true">
                <Class class="Element"/>
            </Target>
        </ECRelationshipClass>
        )xml");
    SchemaItem schema1(schemaXml1);
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDbForCurrentTest(schema1));

    ECInstanceKey materialKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Material(PropMaterial) VALUES('MyMaterial')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(materialKey));
    }

    ECClassId relClassId = m_ecdb.Schemas().GetClassId("TestSchema", "MaterialProfileRefersToMaterial");
    ECInstanceKey materialProfileKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.MaterialProfile(PropMaterialProfile,Material) VALUES('MyMaterialProfile',?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(1, materialKey.GetInstanceId(), relClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(materialProfileKey));
    }

    Utf8String schemaXml2 = ConstructTestSchema(R"xml(
        <ECEntityClass typeName="MaterialProfileDefinition" modifier="Abstract">
            <BaseClass>Element</BaseClass>
            <ECNavigationProperty propertyName="Material" relationshipName="MaterialProfileRefersToMaterial" direction="Forward" />
        </ECEntityClass>
        <ECEntityClass typeName="Material" modifier="Sealed">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="PropMaterial" typeName="string"/>
        </ECEntityClass>

        <ECEntityClass typeName="MaterialProfile" modifier="Sealed">
            <BaseClass>MaterialProfileDefinition</BaseClass>
            <ECProperty propertyName="PropMaterialProfile" typeName="string" />
            <ECNavigationProperty propertyName="Material" relationshipName="MaterialProfileRefersToMaterial" direction="Forward" />
        </ECEntityClass>

        <ECRelationshipClass typeName="MaterialProfileRefersToMaterial" strength="referencing" modifier="Sealed">
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                <Class class="MaterialProfileDefinition" />
            </Source>
            <Target multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="true">
                <Class class="Element"/>
            </Target>
        </ECRelationshipClass>
        )xml", "01.00.01");
    SchemaItem schema2(schemaXml2);
    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema2));

    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT m.PropMaterial, mp.PropMaterialProfile FROM ts.MaterialProfile mp JOIN ts.Material m USING ts.MaterialProfileRefersToMaterial");
    ASSERT_EQ(JsonValue(R"json([{"PropMaterial":"MyMaterial", "PropMaterialProfile":"MyMaterialProfile"}])json"), result) << "Verify instances";
    }

    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT m.PropMaterial, mp.PropMaterialProfile FROM ts.MaterialProfile mp JOIN ts.Material m ON mp.Material.Id = m.ECInstanceId");
    ASSERT_EQ(JsonValue(R"json([{"PropMaterial":"MyMaterial", "PropMaterialProfile":"MyMaterialProfile"}])json"), result) << "Verify instances";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, AddNavUpInHierarchyAndRelationshipHasMixin)
    {
    //Introduce a new nav prop with the same name as an existing one in the base class
    //Also change relationship class Source/Target class to base class
    Utf8String schemaXml1 = ConstructTestSchema(R"xml(
        <ECEntityClass typeName="MaterialProfileDefinition" modifier="Abstract">
            <BaseClass>Element</BaseClass>
        </ECEntityClass>
        <ECEntityClass typeName="Material" modifier="Sealed">
            <BaseClass>Element</BaseClass>
            <BaseClass>BaseMixin</BaseClass>
            <ECProperty propertyName="PropMaterial" typeName="string"/>
        </ECEntityClass>

        <ECEntityClass typeName="MaterialProfile" modifier="Sealed">
            <BaseClass>MaterialProfileDefinition</BaseClass>
            <ECProperty propertyName="PropMaterialProfile" typeName="string" />
            <ECNavigationProperty propertyName="Material" relationshipName="MaterialProfileRefersToMaterial" direction="Forward" />
        </ECEntityClass>

        <ECRelationshipClass typeName="MaterialProfileRefersToMaterial" strength="referencing" modifier="Sealed">
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                <Class class="MaterialProfile" />
            </Source>
            <Target multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="true">
                <Class class="BaseMixin"/>
            </Target>
        </ECRelationshipClass>
        )xml");
    SchemaItem schema1(schemaXml1);
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDbForCurrentTest(schema1));

    ECInstanceKey materialKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Material(PropMaterial) VALUES('MyMaterial')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(materialKey));
    }

    ECClassId relClassId = m_ecdb.Schemas().GetClassId("TestSchema", "MaterialProfileRefersToMaterial");
    ECInstanceKey materialProfileKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.MaterialProfile(PropMaterialProfile,Material) VALUES('MyMaterialProfile',?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(1, materialKey.GetInstanceId(), relClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(materialProfileKey));
    }

    Utf8String schemaXml2 = ConstructTestSchema(R"xml(
        <ECEntityClass typeName="MaterialProfileDefinition" modifier="Abstract">
            <BaseClass>Element</BaseClass>
            <ECNavigationProperty propertyName="Material" relationshipName="MaterialProfileRefersToMaterial" direction="Forward" />
        </ECEntityClass>
        <ECEntityClass typeName="Material" modifier="Sealed">
            <BaseClass>Element</BaseClass>
            <BaseClass>BaseMixin</BaseClass>
            <ECProperty propertyName="PropMaterial" typeName="string"/>
        </ECEntityClass>

        <ECEntityClass typeName="MaterialProfile" modifier="Sealed">
            <BaseClass>MaterialProfileDefinition</BaseClass>
            <ECProperty propertyName="PropMaterialProfile" typeName="string" />
            <ECNavigationProperty propertyName="Material" relationshipName="MaterialProfileRefersToMaterial" direction="Forward" />
        </ECEntityClass>

        <ECRelationshipClass typeName="MaterialProfileRefersToMaterial" strength="referencing" modifier="Sealed">
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                <Class class="MaterialProfileDefinition" />
            </Source>
            <Target multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="true">
                <Class class="BaseMixin"/>
            </Target>
        </ECRelationshipClass>
        )xml", "01.00.01");
    SchemaItem schema2(schemaXml2);
    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema2));

    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT m.PropMaterial, mp.PropMaterialProfile FROM ts.MaterialProfile mp JOIN ts.Material m USING ts.MaterialProfileRefersToMaterial");
    ASSERT_EQ(JsonValue(R"json([{"PropMaterial":"MyMaterial", "PropMaterialProfile":"MyMaterialProfile"}])json"), result) << "Verify instances";
    }

    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT m.PropMaterial, mp.PropMaterialProfile FROM ts.MaterialProfile mp JOIN ts.Material m ON mp.Material.Id = m.ECInstanceId");
    ASSERT_EQ(JsonValue(R"json([{"PropMaterial":"MyMaterial", "PropMaterialProfile":"MyMaterialProfile"}])json"), result) << "Verify instances";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, MoveNavUpInHierarchyAndRemapSharedColumns)
    {
    //Move existing nav property up in the base class which derives multiple classes and uses SharedColumn mapping strategy
    //Also change relationship class Source/Target class to be the base class
    Utf8String schemaXml1 = ConstructTestSchema(R"xml(
        <ECEntityClass typeName="MaterialProfileDefinition" modifier="Abstract">
            <BaseClass>Element</BaseClass>
        </ECEntityClass>

        <ECEntityClass typeName="MaterialProfile1">
            <BaseClass>MaterialProfileDefinition</BaseClass>
            <ECProperty propertyName="PropMaterialProfile1" typeName="string" />
            <ECNavigationProperty propertyName="MovingNavProperty" relationshipName="MaterialProfileRefersToMaterial" direction="Forward" />
        </ECEntityClass>
        <ECEntityClass typeName="MaterialProfile2">
            <BaseClass>MaterialProfileDefinition</BaseClass>
            <ECProperty propertyName="PropMaterialProfile2" typeName="string" />
        </ECEntityClass>

        <ECEntityClass typeName="A">
            <ECProperty propertyName="PropA1" typeName="string"/>
        </ECEntityClass>

        <ECRelationshipClass typeName="MaterialProfileRefersToMaterial" strength="referencing" modifier="Sealed">
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                <Class class="MaterialProfile1" />
            </Source>
            <Target multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="false">
                <Class class="A"/>
            </Target>
        </ECRelationshipClass>
        )xml");
    SchemaItem schema1(schemaXml1);
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDbForCurrentTest(schema1));

    ECInstanceKey aKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.A(PropA1) VALUES('PropertyA')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(aKey));
    }

    ECClassId relClassId = m_ecdb.Schemas().GetClassId("TestSchema", "MaterialProfileRefersToMaterial");
    ECInstanceKey materialProfileKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.MaterialProfile1(PropMaterialProfile1,MovingNavProperty) VALUES('MyMaterialProfile',?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(1, aKey.GetInstanceId(), relClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(materialProfileKey));
    }

    Utf8String schemaXml2 = ConstructTestSchema(R"xml(
        <ECEntityClass typeName="MaterialProfileDefinition" modifier="Abstract">
            <BaseClass>Element</BaseClass>
            <ECNavigationProperty propertyName="MovingNavProperty" relationshipName="MaterialProfileRefersToMaterial" direction="Forward" />
        </ECEntityClass>

        <ECEntityClass typeName="MaterialProfile1">
            <BaseClass>MaterialProfileDefinition</BaseClass>
            <ECProperty propertyName="PropMaterialProfile1" typeName="string" />
            <ECNavigationProperty propertyName="MovingNavProperty" relationshipName="MaterialProfileRefersToMaterial" direction="Forward" />
        </ECEntityClass>
        <ECEntityClass typeName="MaterialProfile2">
            <BaseClass>MaterialProfileDefinition</BaseClass>
            <ECProperty propertyName="PropMaterialProfile2" typeName="string" />
        </ECEntityClass>

        <ECEntityClass typeName="A">
            <ECProperty propertyName="PropA1" typeName="string"/>
        </ECEntityClass>

        <ECRelationshipClass typeName="MaterialProfileRefersToMaterial" strength="referencing" modifier="Sealed">
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                <Class class="MaterialProfileDefinition" />
            </Source>
            <Target multiplicity="(1..1)" roleLabel="is referenced by" polymorphic="false">
                <Class class="A"/>
            </Target>
        </ECRelationshipClass>
        )xml", "01.00.01");
    SchemaItem schema2(schemaXml2);
    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema2, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT a.PropA1, mp.PropMaterialProfile1 FROM ts.MaterialProfile1 mp JOIN ts.A a USING ts.MaterialProfileRefersToMaterial");
    ASSERT_EQ(JsonValue(R"json([{"PropA1":"PropertyA", "PropMaterialProfile1":"MyMaterialProfile"}])json"), result) << "Verify instances";
    }

    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT a.PropA1, mp.PropMaterialProfile1 FROM ts.MaterialProfile1 mp JOIN ts.A a ON mp.MovingNavProperty.Id = a.ECInstanceId");
    ASSERT_EQ(JsonValue(R"json([{"PropA1":"PropertyA", "PropMaterialProfile1":"MyMaterialProfile"}])json"), result) << "Verify instances";
    }
    }

END_ECDBUNITTESTS_NAMESPACE
