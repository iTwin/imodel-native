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
// columns and overflow, plus a mixin, a navigation property, a link table relationship, structs,
// arrays, an enumeration, a geometry and a class with its own table.
// The shared columns of PhysicalElement's joined table are full with Origin and Tags, so the properties of
// Pipe, Valve and Sensor are in the overflow table. Pipe and Valve both have Material, in different columns.
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
    <ECEnumeration typeName="ValveKind" backingTypeName="int" isStrict="true">
        <ECEnumerator name="Gate" value="1" />
        <ECEnumerator name="Ball" value="2" />
        <ECEnumerator name="Check" value="3" />
    </ECEnumeration>
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
        <ECProperty propertyName="Kind" typeName="ValveKind" />
        <ECNavigationProperty propertyName="AttachedPipe" relationshipName="PipeHasValves" direction="Backward" />
        <ECProperty propertyName="Material" typeName="string" />
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
        <ECProperty propertyName="Footprint" typeName="Bentley.Geometry.Common.IGeometry" />
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
    <ECRelationshipClass typeName="DocumentRefersToNotes" strength="referencing" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
            <Class class="Document" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is referred to by" polymorphic="true">
            <Class class="Note" />
        </Target>
        <ECProperty propertyName="Reason" typeName="string" />
    </ECRelationshipClass>
</ECSchema>)xml");
        return s_baseline;
        }

    //! Expects the import to be accepted, then verifies all data.
    void ExpectAccepted(Utf8StringCR schemaXml, SchemaVersionChange versionChange = SchemaVersionChange::Minor, ExpectedDataChanges const& expectedChanges = ExpectedDataChanges())
        {
        EXPECT_EQ(SUCCESS, UpdateSchema(schemaXml, versionChange));
        VerifyData(expectedChanges);
        }

    //! Expects the import to be rejected with the given issue, then verifies that nothing changed.
    void ExpectRejected(Utf8StringCR schemaXml, Utf8CP issueId, SchemaVersionChange versionChange = SchemaVersionChange::Minor)
        {
        EXPECT_EQ(ERROR, UpdateSchema(schemaXml, versionChange));
        ExpectRejectionReason(issueId);
        VerifyData();
        }

    //! Values of struct and struct array properties no longer have the given struct member.
    static ExpectedDataChanges::ValueConverter WithoutStructMember(Utf8CP memberName)
        {
        Utf8String member(memberName);
        return [member] (Utf8StringCR valueBeforeUpdate)
            {
            BeJsDocument value(valueBeforeUpdate.c_str());
            if (value.isArray())
                {
                value.ForEachArrayMemberValue([&] (uint32_t, BeJsValue element)
                    {
                    element.removeMember(member.c_str());
                    return false;
                    });
                return value.Stringify();
                }

            value.removeMember(member.c_str());
            return value.size() == 0 ? Utf8String() : value.Stringify();
            };
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
// Pipe inherits Length from the inserted base class, so ECSQL with $-> has to find it on existing pipes.
// The harness reads data without $->, so this test checks it separately. For a hierarchy without shared columns
// see MappingCustomAttributeUpdateTests.InsertBaseClassWithNewProperty.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, InsertBaseClassWithNewProperty)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    AppendToSchema(schemaXml, R"xml(
    <ECEntityClass typeName="LinearElement" modifier="Abstract">
        <BaseClass>PhysicalElement</BaseClass>
        <ECProperty propertyName="Length" typeName="double" />
    </ECEntityClass>
)xml");
    ReplaceInClass(schemaXml, "Pipe", "<BaseClass>PhysicalElement</BaseClass>", "<BaseClass>LinearElement</BaseClass>");
    ExpectAccepted(schemaXml);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.Pipe SET Length = 1.5"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM ts.Pipe"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    const int pipeCount = stmt.GetValueInt(0);
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT $->Length FROM ts.Pipe"));
    int rowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ++rowCount;
        EXPECT_EQ(1.5, stmt.GetValueDouble(0));
        }

    EXPECT_LT(0, pipeCount);
    EXPECT_EQ(pipeCount, rowCount);
    m_ecdb.AbandonChanges();
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
    ExpectRejectionReason("ECDb_0386");

    VerifyData();
    }

//***************************************************************************************
// Moving properties
//***************************************************************************************

//---------------------------------------------------------------------------------------
// Pipe and Valve keep Material in different overflow columns. Moving it to their base class puts
// both into one column. Sensor inherits the property and reads NULL.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, MovePropertyOfSiblingsToBaseClass)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "PhysicalElement", R"xml(<ECArrayProperty propertyName="Tags" typeName="string" />)xml", R"xml(<ECArrayProperty propertyName="Tags" typeName="string" />
        <ECProperty propertyName="Material" typeName="string" />)xml");
    ReplaceInClass(schemaXml, "Pipe", R"xml(<ECProperty propertyName="Material" typeName="string" />)xml", "");
    ReplaceInClass(schemaXml, "Valve", R"xml(<ECProperty propertyName="Material" typeName="string" />)xml", "");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// The opposite of moving a property to a base class: the base class loses the property and
// each subclass declares it. The values belong to the subclasses and should stay.
// Disabled: deleting the base property sets it to NULL with a polymorphic UPDATE, which clears the
// values of all subclasses. Nothing copies them to the new properties, so they read NULL.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, DISABLED_MovePropertyToSubclasses)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "PhysicalElement", R"xml(<ECArrayProperty propertyName="Tags" typeName="string" />)xml", "");
    for (Utf8CP className : {"Pipe", "Valve", "Sensor"})
        ReplaceInClass(schemaXml, className, "</ECEntityClass>", R"xml(    <ECArrayProperty propertyName="Tags" typeName="string" />
    </ECEntityClass>)xml");

    ExpectAccepted(schemaXml, SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, MovePropertyToMixin)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "IHasMass", R"xml(<ECProperty propertyName="Mass" typeName="double" />)xml", R"xml(<ECProperty propertyName="Mass" typeName="double" />
        <ECProperty propertyName="Diameter" typeName="int" />)xml");
    ReplaceInClass(schemaXml, "Pipe", R"xml(<ECProperty propertyName="Diameter" typeName="int" />)xml", "");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// The mixin loses the property and its only implementation declares it.
// The values belong to Pipe and should stay.
// Disabled: rejected with ECDb_0387, same cause as DISABLED_DeletePropertyOfMixin.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, DISABLED_MovePropertyFromMixinToClass)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "IHasMass", R"xml(<ECProperty propertyName="Mass" typeName="double" />)xml", "");
    ReplaceInClass(schemaXml, "Pipe", R"xml(<ECProperty propertyName="Material" typeName="string" />)xml", R"xml(<ECProperty propertyName="Material" typeName="string" />
        <ECProperty propertyName="Mass" typeName="double" />)xml");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, AddPropertyOverride)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Valve", R"xml(<ECProperty propertyName="Pressure" typeName="double" />)xml", R"xml(<ECProperty propertyName="Pressure" typeName="double" />
        <ECArrayProperty propertyName="Tags" typeName="string" displayLabel="Valve tags" />)xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, RemovePropertyOverride)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Valve", R"xml(<ECProperty propertyName="Pressure" typeName="double" />)xml", R"xml(<ECProperty propertyName="Pressure" typeName="double" />
        <ECArrayProperty propertyName="Tags" typeName="string" displayLabel="Valve tags" />)xml");
    ASSERT_EQ(SUCCESS, UpdateSchema(schemaXml));

    ExpectAccepted(GetBaselineSchemaXml());
    }

//***************************************************************************************
// Base classes
//***************************************************************************************

//---------------------------------------------------------------------------------------
// Sensor becomes a Valve. Its values move out of the overflow columns Valve uses, and it inherits
// the Valve properties, which read NULL.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, MoveClassBelowSibling)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Sensor", "<BaseClass>PhysicalElement</BaseClass>", "<BaseClass>Valve</BaseClass>");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// Only a subclass of the old base class can replace it.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, ChangingBaseClassToAncestorIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Sensor", "<BaseClass>PhysicalElement</BaseClass>", "<BaseClass>Element</BaseClass>");
    ExpectRejected(schemaXml, "ECDb_0357", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, ChangingBaseClassToUnrelatedClassIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Sensor", "<BaseClass>PhysicalElement</BaseClass>", "<BaseClass>Document</BaseClass>");
    ExpectRejected(schemaXml, "ECDb_0357", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// Note has its own table and no base class.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, AddingBaseClassToRootClassIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Note", R"xml(<ECProperty propertyName="NoteText" typeName="string" />)xml", R"xml(<BaseClass>Element</BaseClass>
        <ECProperty propertyName="NoteText" typeName="string" />)xml");
    ExpectRejected(schemaXml, "ECDb_0358", SchemaVersionChange::Major);
    }

//***************************************************************************************
// Mixins
//***************************************************************************************

//---------------------------------------------------------------------------------------
// Existing valves read NULL for Mass and now come back from IHasMass queries.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, AddMixinToClass)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Valve", "<BaseClass>PhysicalElement</BaseClass>", R"xml(<BaseClass>PhysicalElement</BaseClass>
        <BaseClass>IHasMass</BaseClass>)xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// Pipe loses Mass. Accepted without a major version, although deleting a property needs one.
// The values stay in their columns, see DISABLED_RemoveMixinFromClassThenAddItAgain.
// Undecided whether that is intended.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, RemoveMixinWithPropertiesFromClass)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Pipe", "<BaseClass>IHasMass</BaseClass>", "");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// Mass values were dropped with the mixin, so they must not come back with it.
// Disabled: removing the mixin leaves the values in their columns, and adding it again makes them readable.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, DISABLED_RemoveMixinFromClassThenAddItAgain)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Pipe", "<BaseClass>IHasMass</BaseClass>", "");
    ASSERT_EQ(SUCCESS, UpdateSchema(schemaXml));

    ExpectAccepted(GetBaselineSchemaXml(), SchemaVersionChange::Minor, ExpectedDataChanges()
        .ConvertValuesOf("TestSchema:Pipe", "Mass", [] (Utf8StringCR) { return Utf8String(); }));
    }

//---------------------------------------------------------------------------------------
// Deletes the mixin and removes it from Pipe in one update. Pipe instances stay and lose Mass.
// Disabled: rejected with ECDb_0378. ECDb processes the changed classes by name, so it deletes IHasMass
// while Pipe still derives from it.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, DISABLED_DeleteMixin)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    RemoveClass(schemaXml, "IHasMass");
    ReplaceInClass(schemaXml, "Pipe", "<BaseClass>IHasMass</BaseClass>", "");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// Pipe loses Mass.
// Disabled: rejected with ECDb_0387. Deleting a property sets it to NULL with an ECSQL UPDATE on the
// class that declares it, and ECSQL cannot modify a mixin.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, DISABLED_DeletePropertyOfMixin)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "IHasMass", R"xml(<ECProperty propertyName="Mass" typeName="double" />)xml", "");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, GeneralizeMixinAppliesToEntityClass)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "IHasMass", "<AppliesToEntityClass>PhysicalElement</AppliesToEntityClass>", "<AppliesToEntityClass>Element</AppliesToEntityClass>");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// Pipe is the only class that has the mixin, so all data still fits the narrower rule.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, SpecializingMixinAppliesToEntityClassIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "IHasMass", "<AppliesToEntityClass>PhysicalElement</AppliesToEntityClass>", "<AppliesToEntityClass>Pipe</AppliesToEntityClass>");
    ExpectRejected(schemaXml, "ECDb_0356");
    }

//***************************************************************************************
// Class modifiers
//***************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, MakeAbstractClassConcrete)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "PhysicalElement", R"xml(modifier="Abstract")xml", R"xml(modifier="None")xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// PhysicalElement, a subclass of Element, is abstract.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, MakingClassWithAbstractSubclassConcreteIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Element", R"xml(modifier="Abstract")xml", R"xml(modifier="None")xml");
    ExpectRejected(schemaXml, "ECDb_0360");
    }

//---------------------------------------------------------------------------------------
// Rejected even though only the instances would be in the way.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, MakingConcreteClassAbstractIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Sensor", R"xml(typeName="Sensor">)xml", R"xml(typeName="Sensor" modifier="Abstract">)xml");
    ExpectRejected(schemaXml, "ECDb_0363");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, SealClassThenUnsealIt)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Sensor", R"xml(typeName="Sensor">)xml", R"xml(typeName="Sensor" modifier="Sealed">)xml");
    ASSERT_EQ(SUCCESS, UpdateSchema(schemaXml));

    ExpectAccepted(GetBaselineSchemaXml());
    }

//***************************************************************************************
// Deleting classes and properties
//***************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, DeletingClassWithoutMajorVersionIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    RemoveClass(schemaXml, "Sensor");
    ExpectRejected(schemaXml, "ECDb_0376");
    }

//---------------------------------------------------------------------------------------
// ECDb sees a deletion plus an addition: the instances of Sensor are deleted, Detector starts empty.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, RenameClassWithReadVersionIncrement)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Sensor", R"xml(typeName="Sensor")xml", R"xml(typeName="Detector")xml");
    ASSERT_EQ(SUCCESS, UpdateSchema(schemaXml, SchemaVersionChange::Major));
    VerifyData(ExpectedDataChanges().DeleteInstancesOf("TestSchema:Sensor"));
    }

//---------------------------------------------------------------------------------------
// Origin uses three shared columns. All subclasses lose it.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, DeletePropertyOfBaseClass)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "PhysicalElement", R"xml(<ECProperty propertyName="Origin" typeName="point3d" />)xml", "");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// The root class does not share columns, so Code has a column of its own.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, DeletingPropertyOfRootClassIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Element", R"xml(<ECProperty propertyName="Code" typeName="string" />)xml", "");
    ExpectRejected(schemaXml, "ECDb_0386", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, DeleteStructProperty)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Document", R"xml(<ECStructProperty propertyName="Cover" typeName="Segment" />)xml", "");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, DeleteStructArrayProperty)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Pipe", R"xml(<ECStructArrayProperty propertyName="Segments" typeName="Segment" />)xml", "");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major);
    }

//***************************************************************************************
// Struct members. Segment is used by the struct property Document.Cover and the struct array Pipe.Segments.
//***************************************************************************************

//---------------------------------------------------------------------------------------
// Existing covers read NULL for the new member.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, AddStructMember)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Segment", R"xml(<ECProperty propertyName="SegmentLabel" typeName="string" />)xml", R"xml(<ECProperty propertyName="SegmentLabel" typeName="string" />
        <ECProperty propertyName="SegmentWidth" typeName="double" />)xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, DeleteStructMember)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Segment", R"xml(<ECProperty propertyName="SegmentLabel" typeName="string" />)xml", "");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major, ExpectedDataChanges()
        .ConvertValuesOf("TestSchema:Document", "Cover", WithoutStructMember("SegmentLabel"))
        .ConvertValuesOf("TestSchema:Pipe", "Segments", WithoutStructMember("SegmentLabel")));
    }

//---------------------------------------------------------------------------------------
// The deleted values must not come back with the member.
// Disabled: deleting the member only hides the values. They stay in the columns of the struct property
// and in the stored struct arrays, and adding the member again makes them readable.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassHierarchyUpdateTests, DISABLED_DeleteStructMemberThenAddItAgain)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Segment", R"xml(<ECProperty propertyName="SegmentLabel" typeName="string" />)xml", "");
    ASSERT_EQ(SUCCESS, UpdateSchema(schemaXml, SchemaVersionChange::Major));

    ExpectAccepted(GetBaselineSchemaXml(), SchemaVersionChange::Minor, ExpectedDataChanges()
        .ConvertValuesOf("TestSchema:Document", "Cover", WithoutStructMember("SegmentLabel"))
        .ConvertValuesOf("TestSchema:Pipe", "Segments", WithoutStructMember("SegmentLabel")));
    }

END_ECDBUNITTESTS_NAMESPACE
