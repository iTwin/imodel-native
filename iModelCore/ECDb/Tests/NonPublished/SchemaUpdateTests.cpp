/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "SchemaUpdateTestFixture.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// Class hierarchy mapped like BisCore elements: TablePerHierarchy with joined tables, shared
// columns and overflow, plus a mixin, a navigation property, structs, arrays and a class with
// its own table.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ClassHierarchyUpdateTests : SchemaUpdateTestFixture
    {
    static SchemaUpdateBaseline const& GetBaseline()
        {
        static SchemaUpdateBaseline s_baseline("ClassHierarchy", R"xml(<?xml version="1.0" encoding="utf-8"?>
<ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
    <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA" />
    <ECStructClass typeName="Segment">
        <ECProperty propertyName="SegmentLength" typeName="double" />
        <ECProperty propertyName="SegmentLabel" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="Element" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
            <ShareColumns xmlns="ECDbMap.02.00.00">
                <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
            </ShareColumns>
            <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00" />
        </ECCustomAttributes>
        <ECProperty propertyName="Code" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="PhysicalElement" modifier="Abstract">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="Origin" typeName="point3d" />
        <ECArrayProperty propertyName="Tags" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="IHasMass" modifier="Abstract">
        <ECCustomAttributes>
            <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                <AppliesToEntityClass>PhysicalElement</AppliesToEntityClass>
            </IsMixin>
        </ECCustomAttributes>
        <ECProperty propertyName="Mass" typeName="double" />
    </ECEntityClass>
    <ECEntityClass typeName="Pipe">
        <BaseClass>PhysicalElement</BaseClass>
        <BaseClass>IHasMass</BaseClass>
        <ECProperty propertyName="Diameter" typeName="int" />
        <ECProperty propertyName="Material" typeName="string" />
        <ECStructArrayProperty propertyName="Segments" typeName="Segment" />
    </ECEntityClass>
    <ECEntityClass typeName="Valve">
        <BaseClass>PhysicalElement</BaseClass>
        <ECProperty propertyName="Pressure" typeName="double" />
        <ECProperty propertyName="ValveType" typeName="long" />
        <ECNavigationProperty propertyName="AttachedPipe" relationshipName="PipeHasValves" direction="Backward" />
    </ECEntityClass>
    <ECEntityClass typeName="Sensor">
        <BaseClass>PhysicalElement</BaseClass>
        <ECProperty propertyName="Reading" typeName="double" />
        <ECProperty propertyName="SerialNumber" typeName="binary" />
        <ECProperty propertyName="InstalledOn" typeName="dateTime" />
    </ECEntityClass>
    <ECEntityClass typeName="Document">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="Title" typeName="string" />
        <ECProperty propertyName="PageCount" typeName="int" />
        <ECProperty propertyName="Anchor" typeName="point2d" />
        <ECStructProperty propertyName="Cover" typeName="Segment" />
    </ECEntityClass>
    <ECEntityClass typeName="Note">
        <ECProperty propertyName="NoteText" typeName="string" />
        <ECProperty propertyName="Priority" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="PipeHasValves" strength="referencing" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="has" polymorphic="false">
            <Class class="Pipe" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is attached to" polymorphic="false">
            <Class class="Valve" />
        </Target>
    </ECRelationshipClass>
</ECSchema>)xml");
        return s_baseline;
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, AddPropertyToDerivedClass)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Pipe", R"xml(<ECProperty propertyName="Material" typeName="string" />)xml", R"xml(
        <ECProperty propertyName="Material" typeName="string" />
        <ECProperty propertyName="Length" typeName="double" />)xml");
    ASSERT_EQ(SUCCESS, UpdateSchema(schemaXml));

    VerifyData();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, InsertBaseClassAndMovePropertyToIt)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    AppendToSchema(schemaXml, R"xml(
    <ECEntityClass typeName="LinearElement" modifier="Abstract">
        <BaseClass>PhysicalElement</BaseClass>
        <ECProperty propertyName="Diameter" typeName="int" />
    </ECEntityClass>
)xml");
    ReplaceInClass(schemaXml, "Pipe", "<BaseClass>PhysicalElement</BaseClass>", "<BaseClass>LinearElement</BaseClass>");
    ReplaceInClass(schemaXml, "Pipe", R"xml(<ECProperty propertyName="Diameter" typeName="int" />)xml", "");
    ASSERT_EQ(SUCCESS, UpdateSchema(schemaXml));

    // Every baseline instance reads as before. Pipe instances now also come back from LinearElement queries.
    VerifyData();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, DeletePropertyFromDerivedClass)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Pipe", R"xml(<ECProperty propertyName="Material" typeName="string" />)xml", "");
    ASSERT_EQ(SUCCESS, UpdateSchema(schemaXml, SchemaVersionChange::Major));

    VerifyData();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, DeleteClassWithInstances)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    RemoveClass(schemaXml, "Sensor");
    ASSERT_EQ(SUCCESS, UpdateSchema(schemaXml, SchemaVersionChange::Major));

    VerifyData(ExpectedDataChanges().DeleteInstancesOf("TestSchema:Sensor"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, DeletingPropertyOfClassWithOwnTableIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Note", R"xml(<ECProperty propertyName="Priority" typeName="int" />)xml", "");
    ASSERT_EQ(ERROR, UpdateSchema(schemaXml, SchemaVersionChange::Major));

    VerifyData();
    }

END_ECDBUNITTESTS_NAMESPACE
