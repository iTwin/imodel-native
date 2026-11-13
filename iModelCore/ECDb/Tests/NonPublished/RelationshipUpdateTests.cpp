/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "SchemaUpdateTestFixture.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// Relationships the way iModels use them. Elements are mapped like BisCore elements:
// TablePerHierarchy with joined tables and shared columns, plus a mixin.
//
// Mapped to navigation properties:
// - ElementOwnsChildElements: physical foreign key (NoAction) on the root table, with the derived
//   PhysicalElementAssemblesElements, so Parent stores two relationship classes
// - PipeHasValves: sealed, logical foreign key in shared columns, non-polymorphic ends
// - SensorObservesPipe: logical foreign key on the source end, forward navigation property
// - ElementOwnsAspects: physical foreign key with cascading delete on a separate TablePerHierarchy root
// - ElementHasNameplate: 1:1, physical foreign key with cascading delete and a unique index, on a class with its own table
//
// Mapped to link tables:
// - ElementRefersToElements: abstract TablePerHierarchy root without foreign keys, like BisCore,
//   with PipeFeedsValve, which fills all shared columns, and ElementGroupsMembers
// - DocumentRefersToNotes: own table, many to many, with a property
// - NoteAnnotatesElement: own table, one to many, so it has a unique index on the target
// - MaintainableHasDocuments: a mixin as source
// - InstrumentUsesDocuments: two non-polymorphic source classes with an abstract constraint, target lower limit 1
//
// With the generator's selection rule, Pump instances are referenced: Pump1 and Pump2 are Parents of
// other elements, Pump2 is related through ElementGroupsMembers and NoteAnnotatesElement and owns an
// aspect, Pump1 has a nameplate and is the source of MaintainableHasDocuments.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct RelationshipUpdateTests : SchemaUpdateTestFixture
    {
    static SchemaUpdateBaseline const& GetBaseline()
        {
        static SchemaUpdateBaseline s_baseline("Relationships", R"xml(<?xml version="1.0" encoding="utf-8"?>
<ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
    <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA" />
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
        <ECNavigationProperty propertyName="Parent" relationshipName="ElementOwnsChildElements" direction="Backward">
            <ECCustomAttributes>
                <ForeignKeyConstraint xmlns="ECDbMap.02.00.00">
                    <OnDeleteAction>NoAction</OnDeleteAction>
                </ForeignKeyConstraint>
            </ECCustomAttributes>
        </ECNavigationProperty>
    </ECEntityClass>
    <ECEntityClass typeName="PhysicalElement" modifier="Abstract">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="Mass" typeName="double" />
    </ECEntityClass>
    <ECEntityClass typeName="IMaintainable" modifier="Abstract">
        <ECCustomAttributes>
            <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                <AppliesToEntityClass>PhysicalElement</AppliesToEntityClass>
            </IsMixin>
        </ECCustomAttributes>
        <ECProperty propertyName="ServiceInterval" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="Pipe">
        <BaseClass>PhysicalElement</BaseClass>
        <BaseClass>IMaintainable</BaseClass>
        <ECProperty propertyName="Diameter" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="Pump">
        <BaseClass>PhysicalElement</BaseClass>
        <BaseClass>IMaintainable</BaseClass>
        <ECProperty propertyName="Power" typeName="double" />
    </ECEntityClass>
    <ECEntityClass typeName="Valve">
        <BaseClass>PhysicalElement</BaseClass>
        <ECProperty propertyName="Pressure" typeName="double" />
        <ECNavigationProperty propertyName="AttachedPipe" relationshipName="PipeHasValves" direction="Backward" />
    </ECEntityClass>
    <ECEntityClass typeName="Sensor">
        <BaseClass>PhysicalElement</BaseClass>
        <ECProperty propertyName="Reading" typeName="double" />
        <ECNavigationProperty propertyName="ObservedPipe" relationshipName="SensorObservesPipe" direction="Forward" />
    </ECEntityClass>
    <ECEntityClass typeName="Document">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="Title" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Note">
        <ECProperty propertyName="NoteText" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Aspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
            <ShareColumns xmlns="ECDbMap.02.00.00">
                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
            </ShareColumns>
        </ECCustomAttributes>
        <ECNavigationProperty propertyName="Element" relationshipName="ElementOwnsAspects" direction="Backward">
            <ECCustomAttributes>
                <ForeignKeyConstraint xmlns="ECDbMap.02.00.00">
                    <OnDeleteAction>Cascade</OnDeleteAction>
                </ForeignKeyConstraint>
            </ECCustomAttributes>
        </ECNavigationProperty>
    </ECEntityClass>
    <ECEntityClass typeName="InspectionAspect">
        <BaseClass>Aspect</BaseClass>
        <ECProperty propertyName="Inspector" typeName="string" />
        <ECProperty propertyName="InspectedOn" typeName="dateTime" />
    </ECEntityClass>
    <ECEntityClass typeName="Nameplate">
        <ECProperty propertyName="SerialNumber" typeName="string" />
        <ECNavigationProperty propertyName="Element" relationshipName="ElementHasNameplate" direction="Backward">
            <ECCustomAttributes>
                <ForeignKeyConstraint xmlns="ECDbMap.02.00.00">
                    <OnDeleteAction>Cascade</OnDeleteAction>
                </ForeignKeyConstraint>
            </ECCustomAttributes>
        </ECNavigationProperty>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsChildElements" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="Element" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="PhysicalElementAssemblesElements" strength="embedding" modifier="None">
        <BaseClass>ElementOwnsChildElements</BaseClass>
        <Source multiplicity="(0..1)" roleLabel="assembles" polymorphic="true">
            <Class class="PhysicalElement" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is assembled by" polymorphic="true">
            <Class class="PhysicalElement" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="PipeHasValves" strength="referencing" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="has" polymorphic="false">
            <Class class="Pipe" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is attached to" polymorphic="false">
            <Class class="Valve" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="SensorObservesPipe" strength="referencing" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="observes" polymorphic="true">
            <Class class="Sensor" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="is observed by" polymorphic="true">
            <Class class="Pipe" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ElementOwnsAspects" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Aspect" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ElementHasNameplate" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="has nameplate" polymorphic="true">
            <Class class="PhysicalElement" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="is nameplate of" polymorphic="true">
            <Class class="Nameplate" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ElementRefersToElements" strength="referencing" modifier="Abstract">
        <ECCustomAttributes>
            <LinkTableRelationshipMap xmlns="ECDbMap.02.00.00">
                <CreateForeignKeyConstraints>False</CreateForeignKeyConstraints>
            </LinkTableRelationshipMap>
            <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
            <ShareColumns xmlns="ECDbMap.02.00.00">
                <MaxSharedColumnsBeforeOverflow>3</MaxSharedColumnsBeforeOverflow>
                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
            </ShareColumns>
        </ECCustomAttributes>
        <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
            <Class class="Element" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is referenced by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="PipeFeedsValve" strength="referencing" modifier="Sealed">
        <BaseClass>ElementRefersToElements</BaseClass>
        <Source multiplicity="(0..*)" roleLabel="feeds" polymorphic="true">
            <Class class="Pipe" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is fed by" polymorphic="true">
            <Class class="Valve" />
        </Target>
        <ECProperty propertyName="FlowRate" typeName="double" />
        <ECProperty propertyName="Medium" typeName="string" />
        <ECProperty propertyName="IsVerified" typeName="boolean" />
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ElementGroupsMembers" strength="referencing" modifier="None">
        <BaseClass>ElementRefersToElements</BaseClass>
        <Source multiplicity="(0..*)" roleLabel="groups" polymorphic="true">
            <Class class="Element" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is grouped by" polymorphic="true">
            <Class class="Element" />
        </Target>
        <ECProperty propertyName="MemberPriority" typeName="int" />
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
    <ECRelationshipClass typeName="NoteAnnotatesElement" strength="referencing" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="annotates" polymorphic="true">
            <Class class="Note" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is annotated by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="MaintainableHasDocuments" strength="referencing" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="has" polymorphic="true">
            <Class class="IMaintainable" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="documents" polymorphic="true">
            <Class class="Document" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="InstrumentUsesDocuments" strength="referencing" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="uses" polymorphic="false" abstractConstraint="PhysicalElement">
            <Class class="Valve" />
            <Class class="Sensor" />
        </Source>
        <Target multiplicity="(1..*)" roleLabel="is used by" polymorphic="false">
            <Class class="Document" />
        </Target>
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

    ECInstanceKey SelectKey(Utf8CP ecsql) const
        {
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql)) << ecsql;
        EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << ecsql;
        return ECInstanceKey(stmt.GetValueId<ECClassId>(1), stmt.GetValueId<ECInstanceId>(0));
        }

    //! Inserts a link table relationship after the update.
    DbResult InsertLinkTableRelationship(Utf8CP relClassECSqlName, ECInstanceKey const& source, ECInstanceKey const& target)
        {
        Utf8PrintfString ecsql("INSERT INTO %s(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(?,?,?,?)", relClassECSqlName);
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();
        stmt.BindId(1, source.GetInstanceId());
        stmt.BindId(2, source.GetClassId());
        stmt.BindId(3, target.GetInstanceId());
        stmt.BindId(4, target.GetClassId());
        return stmt.Step();
        }
    };

//***************************************************************************************
// Adding relationship classes
//***************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, AddLinkTableRelationship)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    AppendToSchema(schemaXml, R"xml(
    <ECRelationshipClass typeName="NoteMentionsPipes" strength="referencing" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="mentions" polymorphic="true">
            <Class class="Note" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is mentioned by" polymorphic="true">
            <Class class="Pipe" />
        </Target>
        <ECProperty propertyName="Context" typeName="string" />
    </ECRelationshipClass>
)xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// The new relationship class maps into the existing link table and reuses its shared columns.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, AddDerivedLinkTableRelationship)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    AppendToSchema(schemaXml, R"xml(
    <ECRelationshipClass typeName="PumpDrivesValve" strength="referencing" modifier="Sealed">
        <BaseClass>ElementRefersToElements</BaseClass>
        <Source multiplicity="(0..*)" roleLabel="drives" polymorphic="true">
            <Class class="Pump" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is driven by" polymorphic="true">
            <Class class="Valve" />
        </Target>
        <ECProperty propertyName="Speed" typeName="double" />
        <ECProperty propertyName="Ratio" typeName="double" />
        <ECProperty propertyName="Mode" typeName="string" />
    </ECRelationshipClass>
)xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// Existing valves read NULL for the new navigation property.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, AddForeignKeyRelationshipWithNavigationProperty)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    AppendToSchema(schemaXml, R"xml(
    <ECRelationshipClass typeName="SensorControlsValves" strength="referencing" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="controls" polymorphic="true">
            <Class class="Sensor" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is controlled by" polymorphic="true">
            <Class class="Valve" />
        </Target>
    </ECRelationshipClass>
)xml");
    ReplaceInClass(schemaXml, "Valve", R"xml(<ECProperty propertyName="Pressure" typeName="double" />)xml", R"xml(<ECProperty propertyName="Pressure" typeName="double" />
        <ECNavigationProperty propertyName="ControllingSensor" relationshipName="SensorControlsValves" direction="Backward" />)xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// Adds a foreign key column with a foreign key constraint to an existing table that has rows.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, AddForeignKeyRelationshipWithPhysicalForeignKey)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    AppendToSchema(schemaXml, R"xml(
    <ECRelationshipClass typeName="DocumentHasNotes" strength="referencing" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="has note" polymorphic="true">
            <Class class="Document" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is note of" polymorphic="true">
            <Class class="Note" />
        </Target>
    </ECRelationshipClass>
)xml");
    ReplaceInClass(schemaXml, "Note", R"xml(<ECProperty propertyName="NoteText" typeName="string" />)xml", R"xml(<ECProperty propertyName="NoteText" typeName="string" />
        <ECNavigationProperty propertyName="Document" relationshipName="DocumentHasNotes" direction="Backward">
            <ECCustomAttributes>
                <ForeignKeyConstraint xmlns="ECDbMap.02.00.00">
                    <OnDeleteAction>SetNull</OnDeleteAction>
                </ForeignKeyConstraint>
            </ECCustomAttributes>
        </ECNavigationProperty>)xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, AddDerivedForeignKeyRelationship)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    AppendToSchema(schemaXml, R"xml(
    <ECRelationshipClass typeName="DocumentOwnsElements" strength="embedding" modifier="None">
        <BaseClass>ElementOwnsChildElements</BaseClass>
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="true">
            <Class class="Document" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by document" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
)xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// A relationship with properties needs a link table, so it cannot be mapped to a navigation property.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, AddingForeignKeyRelationshipWithPropertyIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    AppendToSchema(schemaXml, R"xml(
    <ECRelationshipClass typeName="DocumentCalibratesSensors" strength="referencing" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="calibrates" polymorphic="true">
            <Class class="Document" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is calibrated by" polymorphic="true">
            <Class class="Sensor" />
        </Target>
        <ECProperty propertyName="CalibratedOn" typeName="dateTime" />
    </ECRelationshipClass>
)xml");
    ReplaceInClass(schemaXml, "Sensor", R"xml(<ECProperty propertyName="Reading" typeName="double" />)xml", R"xml(<ECProperty propertyName="Reading" typeName="double" />
        <ECNavigationProperty propertyName="CalibrationDocument" relationshipName="DocumentCalibratesSensors" direction="Backward" />)xml");
    ExpectRejected(schemaXml, "ECDb_0084");
    }

//---------------------------------------------------------------------------------------
// NoteAnnotatesElement is mapped to a link table. A navigation property would need a foreign key column instead.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, AddingNavigationPropertyToLinkTableRelationshipIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Element", R"xml(<ECProperty propertyName="Code" typeName="string" />)xml", R"xml(<ECProperty propertyName="Code" typeName="string" />
        <ECNavigationProperty propertyName="AnnotatingNote" relationshipName="NoteAnnotatesElement" direction="Backward" />)xml");
    ExpectRejected(schemaXml, "ECDb_0066");
    }

//***************************************************************************************
// Properties of relationship classes
//***************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, AddPropertyToLinkTableRelationship)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "DocumentRefersToNotes", R"xml(<ECProperty propertyName="Reason" typeName="string" />)xml", R"xml(<ECProperty propertyName="Reason" typeName="string" />
        <ECProperty propertyName="Weight" typeName="double" />)xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, AddPropertyToDerivedLinkTableRelationship)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "ElementGroupsMembers", R"xml(<ECProperty propertyName="MemberPriority" typeName="int" />)xml", R"xml(<ECProperty propertyName="MemberPriority" typeName="int" />
        <ECProperty propertyName="MemberRole" typeName="string" />)xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// The root does not use shared columns, so the property gets its own column in the link table.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, AddPropertyToAbstractLinkTableRelationship)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "ElementRefersToElements", R"xml(</Target>)xml", R"xml(</Target>
        <ECProperty propertyName="Comment" typeName="string" />)xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// A relationship mapped to a navigation property has no place to store properties.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, AddingPropertyToForeignKeyRelationshipIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "PipeHasValves", R"xml(</Target>)xml", R"xml(</Target>
        <ECProperty propertyName="InstalledOn" typeName="dateTime" />)xml");
    ExpectRejected(schemaXml, "ECDb_0165");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, AddingPropertyToDerivedForeignKeyRelationshipIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "PhysicalElementAssemblesElements", R"xml(</Target>)xml", R"xml(</Target>
        <ECProperty propertyName="AssemblyStep" typeName="int" />)xml");
    ExpectRejected(schemaXml, "ECDb_0165");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, DeletingPropertyOfLinkTableRelationshipWithOwnTableIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "DocumentRefersToNotes", R"xml(<ECProperty propertyName="Reason" typeName="string" />)xml", "");
    ExpectRejected(schemaXml, "ECDb_0386", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, DeletePropertyOfDerivedLinkTableRelationship)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "PipeFeedsValve", R"xml(<ECProperty propertyName="Medium" typeName="string" />)xml", "");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// PipeFeedsValve fills all shared columns of the link table, so the new property goes to the overflow table.
// Disabled: ECSQL cannot read link table properties in an overflow table, ECDb maps them there anyway.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, DISABLED_AddPropertyToFullLinkTableRelationship)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "PipeFeedsValve", R"xml(<ECProperty propertyName="IsVerified" typeName="boolean" />)xml", R"xml(<ECProperty propertyName="IsVerified" typeName="boolean" />
        <ECProperty propertyName="Pressure" typeName="double" />)xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// The values move from a shared column of the derived class to a column of the root.
// PipeFeedsValve inherits the property and reads NULL.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, MovePropertyOfDerivedLinkTableRelationshipToBase)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "ElementGroupsMembers", R"xml(<ECProperty propertyName="MemberPriority" typeName="int" />)xml", "");
    ReplaceInClass(schemaXml, "ElementRefersToElements", R"xml(</Target>)xml", R"xml(</Target>
        <ECProperty propertyName="MemberPriority" typeName="int" />)xml");
    ExpectAccepted(schemaXml);
    }

//***************************************************************************************
// Deleting relationship classes
//***************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, DeleteDerivedLinkTableRelationship)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    RemoveClass(schemaXml, "ElementGroupsMembers");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major, ExpectedDataChanges().DeleteInstancesOf("TestSchema:ElementGroupsMembers"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, DeleteLinkTableRelationshipWithOwnTable)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    RemoveClass(schemaXml, "DocumentRefersToNotes");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major, ExpectedDataChanges().DeleteInstancesOf("TestSchema:DocumentRefersToNotes"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, DeletingLinkTableRelationshipWithoutMajorVersionIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    RemoveClass(schemaXml, "DocumentRefersToNotes");
    ExpectRejected(schemaXml, "ECDb_0376");
    }

//---------------------------------------------------------------------------------------
// DoNotFailForDeletionsOrModifications skips the deletion and keeps the relationship class and its instances.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, DeletingLinkTableRelationshipWithoutMajorVersionIsSkippedWithDoNotFail)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    RemoveClass(schemaXml, "DocumentRefersToNotes");
    ASSERT_EQ(SUCCESS, UpdateSchema(schemaXml, SchemaVersionChange::Minor,
        SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade | SchemaManager::SchemaImportOptions::DoNotFailForDeletionsOrModifications));
    ASSERT_TRUE(m_ecdb.Schemas().GetClass("TestSchema", "DocumentRefersToNotes") != nullptr);
    VerifyData();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, DeletingDerivedForeignKeyRelationshipIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    RemoveClass(schemaXml, "PhysicalElementAssemblesElements");
    ExpectRejected(schemaXml, "ECDb_0381", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, DeletingForeignKeyRelationshipWithItsNavigationPropertyIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    RemoveClass(schemaXml, "PipeHasValves");
    ReplaceInClass(schemaXml, "Valve", R"xml(<ECNavigationProperty propertyName="AttachedPipe" relationshipName="PipeHasValves" direction="Backward" />)xml", "");
    ExpectRejected(schemaXml, "ECDb_0381", SchemaVersionChange::Major);
    }

//***************************************************************************************
// Deleting classes that take part in relationships
//***************************************************************************************

//---------------------------------------------------------------------------------------
// Pumps are Parents of other elements, and the foreign key on Parent has OnDeleteAction NoAction.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, DeletingClassWhoseInstancesAreParentsIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    RemoveClass(schemaXml, "Pump");
    ExpectRejected(schemaXml, "ECDb_0382", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// Relationships to the deleted instances go away with them: aspects and nameplates through their
// cascading foreign keys, link table rows through the foreign keys of their link tables.
// The test removes the Parent values and the rows of ElementRefersToElements that point to pumps first,
// other tests cover those.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, DeleteClassWhoseInstancesAreReferenced)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    ChangeDataBeforeUpdate("UPDATE ts.Element SET Parent=NULL");
    ChangeDataBeforeUpdate("DELETE FROM ts.ElementRefersToElements WHERE SourceECInstanceId IN (SELECT ECInstanceId FROM ts.Pump) OR TargetECInstanceId IN (SELECT ECInstanceId FROM ts.Pump)");

    Utf8String schemaXml = GetBaselineSchemaXml();
    RemoveClass(schemaXml, "Pump");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major, ExpectedDataChanges()
        .DeleteInstancesOf("TestSchema:Pump")
        .DeleteReferencingInstancesOf("TestSchema:InspectionAspect")
        .DeleteReferencingInstancesOf("TestSchema:Nameplate")
        .DeleteReferencingInstancesOf("TestSchema:NoteAnnotatesElement")
        .DeleteReferencingInstancesOf("TestSchema:MaintainableHasDocuments"));
    }

//---------------------------------------------------------------------------------------
// The link table of ElementRefersToElements has no foreign keys, like in BisCore.
// Disabled: its rows that point to the deleted pumps stay in the table. ECSQL skips them when
// SourceECClassId or TargetECClassId is selected and returns them otherwise.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, DISABLED_DeleteClassWhoseInstancesAreReferencedByLinkTableWithoutForeignKeys)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    ChangeDataBeforeUpdate("UPDATE ts.Element SET Parent=NULL");

    Utf8String schemaXml = GetBaselineSchemaXml();
    RemoveClass(schemaXml, "Pump");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major, ExpectedDataChanges()
        .DeleteInstancesOf("TestSchema:Pump")
        .DeleteReferencingInstancesOf("TestSchema:InspectionAspect")
        .DeleteReferencingInstancesOf("TestSchema:Nameplate")
        .DeleteReferencingInstancesOf("TestSchema:NoteAnnotatesElement")
        .DeleteReferencingInstancesOf("TestSchema:MaintainableHasDocuments")
        .DeleteReferencingInstancesOf("TestSchema:ElementGroupsMembers"));
    }

//---------------------------------------------------------------------------------------
// Both relationships that use Note are deleted in the same import.
// Disabled: ECDb deletes classes in alphabetical order and checks each one against the relationship
// constraints stored at that point. Note comes before NoteAnnotatesElement, so the import fails with ECDb_0380.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, DISABLED_DeleteClassTogetherWithItsLinkTableRelationships)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    RemoveClass(schemaXml, "Note");
    RemoveClass(schemaXml, "DocumentRefersToNotes");
    RemoveClass(schemaXml, "NoteAnnotatesElement");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major, ExpectedDataChanges()
        .DeleteInstancesOf("TestSchema:Note")
        .DeleteInstancesOf("TestSchema:DocumentRefersToNotes")
        .DeleteInstancesOf("TestSchema:NoteAnnotatesElement"));
    }

//***************************************************************************************
// Multiplicity
//***************************************************************************************

//---------------------------------------------------------------------------------------
// The schema allows several notes per element afterwards.
// Disabled: the unique index built for the old multiplicity stays and rejects a second note.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, DISABLED_IncreaseUpperMultiplicityOfLinkTableRelationship)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "NoteAnnotatesElement", R"xml(<Source multiplicity="(0..1)")xml", R"xml(<Source multiplicity="(0..*)")xml");
    ExpectAccepted(schemaXml);

    ECInstanceKey annotatedElement = SelectKey("SELECT TargetECInstanceId, TargetECClassId FROM ts.NoteAnnotatesElement ORDER BY ECInstanceId LIMIT 1");
    ECInstanceKey otherNote = SelectKey("SELECT ECInstanceId, ECClassId FROM ts.Note WHERE ECInstanceId<>(SELECT SourceECInstanceId FROM ts.NoteAnnotatesElement ORDER BY ECInstanceId LIMIT 1) LIMIT 1");
    EXPECT_EQ(BE_SQLITE_DONE, InsertLinkTableRelationship("ts.NoteAnnotatesElement", otherNote, annotatedElement));
    }

//---------------------------------------------------------------------------------------
// The schema allows several nameplates per element afterwards.
// Disabled: the foreign key column keeps the unique index built for the 1:1 relationship, and the
// mapping check rejects the import with ECDb_0219.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, DISABLED_IncreaseUpperMultiplicityOfOneToOneForeignKey)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "ElementHasNameplate", R"xml(<Target multiplicity="(0..1)")xml", R"xml(<Target multiplicity="(0..*)")xml");
    ExpectAccepted(schemaXml);

    ECSqlStatement select;
    ASSERT_EQ(ECSqlStatus::Success, select.Prepare(m_ecdb, "SELECT Element.Id, Element.RelECClassId FROM ts.Nameplate WHERE Element.Id IS NOT NULL ORDER BY ECInstanceId LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, select.Step());
    ECSqlStatement insert;
    ASSERT_EQ(ECSqlStatus::Success, insert.Prepare(m_ecdb, "INSERT INTO ts.Nameplate(SerialNumber, Element) VALUES('second nameplate', ?)"));
    insert.BindNavigationValue(1, select.GetValueId<ECInstanceId>(0), select.GetValueId<ECClassId>(1));
    EXPECT_EQ(BE_SQLITE_DONE, insert.Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, IncreasingLowerMultiplicityIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "DocumentRefersToNotes", R"xml(<Target multiplicity="(0..*)")xml", R"xml(<Target multiplicity="(1..*)")xml");
    ExpectRejected(schemaXml, "ECDb_0345", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// Relaxing the lower limit cannot invalidate existing data, ECDb still rejects it.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, DecreasingLowerMultiplicityIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "InstrumentUsesDocuments", R"xml(<Target multiplicity="(1..*)")xml", R"xml(<Target multiplicity="(0..*)")xml");
    ExpectRejected(schemaXml, "ECDb_0345");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, DecreasingUpperMultiplicityIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "DocumentRefersToNotes", R"xml(<Target multiplicity="(0..*)")xml", R"xml(<Target multiplicity="(0..1)")xml");
    ExpectRejected(schemaXml, "ECDb_0346", SchemaVersionChange::Major);
    }

//***************************************************************************************
// Polymorphic flag
//***************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, MakeLinkTableConstraintPolymorphic)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "InstrumentUsesDocuments", R"xml(roleLabel="is used by" polymorphic="false")xml", R"xml(roleLabel="is used by" polymorphic="true")xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, MakeForeignKeyConstraintPolymorphic)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "PipeHasValves", R"xml(roleLabel="is attached to" polymorphic="false")xml", R"xml(roleLabel="is attached to" polymorphic="true")xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, MakingConstraintNonPolymorphicIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "DocumentRefersToNotes", R"xml(roleLabel="refers to" polymorphic="true")xml", R"xml(roleLabel="refers to" polymorphic="false")xml");
    ExpectRejected(schemaXml, "ECDb_0349", SchemaVersionChange::Major);
    }

//***************************************************************************************
// Constraint classes
//***************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, GeneralizeLinkTableConstraintClass)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "DocumentRefersToNotes", R"xml(<Class class="Document" />)xml", R"xml(<Class class="Element" />)xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// The navigation property on Sensor can afterwards point to any physical element.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, GeneralizeReferencedEndOfForeignKeyRelationship)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "SensorObservesPipe", R"xml(<Class class="Pipe" />)xml", R"xml(<Class class="PhysicalElement" />)xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// Adding a class to a constraint keeps every existing relationship valid.
// Disabled: ECDb writes the first new class into every existing constraint class row, which violates
// a unique index, and the import fails without reporting an issue.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, DISABLED_AddConstraintClass)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "InstrumentUsesDocuments", R"xml(<Class class="Sensor" />)xml", R"xml(<Class class="Sensor" />
            <Class class="Pump" />)xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// Two constraint classes are replaced by their common base class.
// Disabled: ECDb writes the new class into both existing constraint class rows, which violates a
// unique index, and the import fails without reporting an issue.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, DISABLED_ReplaceConstraintClassesWithBaseClass)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "InstrumentUsesDocuments", R"xml(<Source multiplicity="(0..*)" roleLabel="uses" polymorphic="false" abstractConstraint="PhysicalElement">
            <Class class="Valve" />
            <Class class="Sensor" />)xml", R"xml(<Source multiplicity="(0..*)" roleLabel="uses" polymorphic="true">
            <Class class="PhysicalElement" />)xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// Disabled: ECDb does not store the new abstract constraint, so the check of the persisted schema
// rejects the import with ECDb_0754.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, DISABLED_ChangeAbstractConstraint)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "InstrumentUsesDocuments", R"xml(abstractConstraint="PhysicalElement")xml", R"xml(abstractConstraint="Element")xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// Existing relationships with a Pipe as source would no longer be valid.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, NarrowingConstraintClassIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "MaintainableHasDocuments", R"xml(<Class class="IMaintainable" />)xml", R"xml(<Class class="Pump" />)xml");
    ExpectRejected(schemaXml, "ECDb_0352", SchemaVersionChange::Major);
    }

//***************************************************************************************
// Role labels, strength and strength direction
//***************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, ChangeRoleLabels)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "DocumentRefersToNotes", R"xml(roleLabel="refers to")xml", R"xml(roleLabel="cites")xml");
    ReplaceInClass(schemaXml, "DocumentRefersToNotes", R"xml(roleLabel="is referred to by")xml", R"xml(roleLabel="is cited by")xml");
    ReplaceInClass(schemaXml, "ElementOwnsChildElements", R"xml(roleLabel="owns child")xml", R"xml(roleLabel="is parent of")xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// Disabled: after a multiplicity change ECDb skips the other changes on the same constraint, so the
// check of the persisted schema rejects the import with ECDb_0754.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, DISABLED_ChangeRoleLabelTogetherWithMultiplicity)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "NoteAnnotatesElement", R"xml(<Source multiplicity="(0..1)" roleLabel="annotates")xml", R"xml(<Source multiplicity="(0..*)" roleLabel="comments on")xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, ChangingStrengthOfLinkTableRelationshipIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "DocumentRefersToNotes", R"xml(strength="referencing")xml", R"xml(strength="holding")xml");
    ExpectRejected(schemaXml, "ECDb_0365", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, ChangingStrengthOfForeignKeyRelationshipIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "SensorObservesPipe", R"xml(strength="referencing")xml", R"xml(strength="holding")xml");
    ExpectRejected(schemaXml, "ECDb_0365", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// DoNotFailForDeletionsOrModifications does not apply to strength changes.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, ChangingStrengthIsRejectedWithDoNotFail)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "DocumentRefersToNotes", R"xml(strength="referencing")xml", R"xml(strength="holding")xml");
    EXPECT_EQ(ERROR, UpdateSchema(schemaXml, SchemaVersionChange::Minor,
        SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade | SchemaManager::SchemaImportOptions::DoNotFailForDeletionsOrModifications));
    ExpectRejectionReason("ECDb_0365");
    VerifyData();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, ChangingStrengthDirectionIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "DocumentRefersToNotes", R"xml(strength="referencing")xml", R"xml(strength="referencing" strengthDirection="Backward")xml");
    ExpectRejected(schemaXml, "ECDb_0366", SchemaVersionChange::Major);
    }

//***************************************************************************************
// Class modifiers
//***************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, SealLinkTableRelationship)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "DocumentRefersToNotes", R"xml(modifier="None")xml", R"xml(modifier="Sealed")xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// A link table relationship with its own table must stay sealed, subclasses would need TablePerHierarchy.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, UnsealingLinkTableRelationshipWithOwnTableIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "NoteAnnotatesElement", R"xml(modifier="Sealed")xml", R"xml(modifier="None")xml");
    ExpectRejected(schemaXml, "ECDb_0178");
    }

//---------------------------------------------------------------------------------------
// Instances of a sealed relationship have no RelECClassId column, an unsealed one would need it.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, UnsealingForeignKeyRelationshipIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "PipeHasValves", R"xml(modifier="Sealed")xml", R"xml(modifier="None")xml");
    ExpectRejected(schemaXml, "ECDb_0211");
    }

//---------------------------------------------------------------------------------------
// A sealed relationship must not have a RelECClassId column, and the existing one cannot be dropped.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, SealingForeignKeyRelationshipIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "ElementHasNameplate", R"xml(modifier="None")xml", R"xml(modifier="Sealed")xml");
    ExpectRejected(schemaXml, "ECDb_0210");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, MakeAbstractLinkTableRelationshipConcrete)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "ElementRefersToElements", R"xml(modifier="Abstract")xml", R"xml(modifier="None")xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, MakingRelationshipAbstractIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "DocumentRefersToNotes", R"xml(modifier="None")xml", R"xml(modifier="Abstract")xml");
    ExpectRejected(schemaXml, "ECDb_0363", SchemaVersionChange::Major);
    }

//***************************************************************************************
// Relationship class hierarchies
//***************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, InsertBaseClassIntoLinkTableRelationshipHierarchy)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    AppendToSchema(schemaXml, R"xml(
    <ECRelationshipClass typeName="PhysicalElementRefersToElements" strength="referencing" modifier="Abstract">
        <BaseClass>ElementRefersToElements</BaseClass>
        <Source multiplicity="(0..*)" roleLabel="physically refers to" polymorphic="true">
            <Class class="PhysicalElement" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is physically referenced by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
)xml");
    ReplaceInClass(schemaXml, "PipeFeedsValve", "<BaseClass>ElementRefersToElements</BaseClass>", "<BaseClass>PhysicalElementRefersToElements</BaseClass>");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// Parent values keep their relationship classes.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, InsertBaseClassIntoForeignKeyRelationshipHierarchy)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    AppendToSchema(schemaXml, R"xml(
    <ECRelationshipClass typeName="PhysicalElementOwnsElements" strength="embedding" modifier="None">
        <BaseClass>ElementOwnsChildElements</BaseClass>
        <Source multiplicity="(0..1)" roleLabel="physically owns" polymorphic="true">
            <Class class="PhysicalElement" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is physically owned by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
)xml");
    ReplaceInClass(schemaXml, "PhysicalElementAssemblesElements", "<BaseClass>ElementOwnsChildElements</BaseClass>", "<BaseClass>PhysicalElementOwnsElements</BaseClass>");
    ExpectAccepted(schemaXml);
    }

//***************************************************************************************
// Navigation properties
//***************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, ChangeNavigationPropertyLabel)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Valve", R"xml(propertyName="AttachedPipe")xml", R"xml(propertyName="AttachedPipe" displayLabel="Attached pipe" description="The pipe the valve sits on")xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, PointingNavigationPropertyToAnotherRelationshipIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    AppendToSchema(schemaXml, R"xml(
    <ECRelationshipClass typeName="PipeSuppliesValves" strength="referencing" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="supplies" polymorphic="false">
            <Class class="Pipe" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is supplied by" polymorphic="false">
            <Class class="Valve" />
        </Target>
    </ECRelationshipClass>
)xml");
    ReplaceInClass(schemaXml, "Valve", R"xml(relationshipName="PipeHasValves")xml", R"xml(relationshipName="PipeSuppliesValves")xml");
    ExpectRejected(schemaXml, "ECDb_0339");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, DeletingNavigationPropertyIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Sensor", R"xml(<ECNavigationProperty propertyName="ObservedPipe" relationshipName="SensorObservesPipe" direction="Forward" />)xml", "");
    ExpectRejected(schemaXml, "ECDb_0385", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// Removing the navigation property from Sensor counts as deleting it, even though PhysicalElement now declares it.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, MovingNavigationPropertyToBaseClassIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Sensor", R"xml(<ECNavigationProperty propertyName="ObservedPipe" relationshipName="SensorObservesPipe" direction="Forward" />)xml", "");
    ReplaceInClass(schemaXml, "PhysicalElement", R"xml(<ECProperty propertyName="Mass" typeName="double" />)xml", R"xml(<ECProperty propertyName="Mass" typeName="double" />
        <ECNavigationProperty propertyName="ObservedPipe" relationshipName="SensorObservesPipe" direction="Forward" />)xml");
    ReplaceInClass(schemaXml, "SensorObservesPipe", R"xml(<Class class="Sensor" />)xml", R"xml(<Class class="PhysicalElement" />)xml");
    ExpectRejected(schemaXml, "ECDb_0385");
    }

//---------------------------------------------------------------------------------------
// Sensor keeps its navigation property as an override. Other physical elements inherit it and read NULL.
// Disabled: the import moves the property of Sensor to new columns, clears the old ones and copies no
// values, so the navigation values of all sensors are lost.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, DISABLED_AddNavigationPropertyToBaseClass)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "PhysicalElement", R"xml(<ECProperty propertyName="Mass" typeName="double" />)xml", R"xml(<ECProperty propertyName="Mass" typeName="double" />
        <ECNavigationProperty propertyName="ObservedPipe" relationshipName="SensorObservesPipe" direction="Forward" />)xml");
    ReplaceInClass(schemaXml, "SensorObservesPipe", R"xml(<Class class="Sensor" />)xml", R"xml(<Class class="PhysicalElement" />)xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, AddingForeignKeyConstraintIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Valve", R"xml(<ECNavigationProperty propertyName="AttachedPipe" relationshipName="PipeHasValves" direction="Backward" />)xml",
        R"xml(<ECNavigationProperty propertyName="AttachedPipe" relationshipName="PipeHasValves" direction="Backward">
            <ECCustomAttributes>
                <ForeignKeyConstraint xmlns="ECDbMap.02.00.00" />
            </ECCustomAttributes>
        </ECNavigationProperty>)xml");
    ExpectRejected(schemaXml, "ECDb_0354");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, ChangingForeignKeyOnDeleteActionIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Aspect", "<OnDeleteAction>Cascade</OnDeleteAction>", "<OnDeleteAction>NoAction</OnDeleteAction>");
    ExpectRejected(schemaXml, "ECDb_0354");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, RemovingForeignKeyConstraintIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Nameplate", R"xml(
            <ECCustomAttributes>
                <ForeignKeyConstraint xmlns="ECDbMap.02.00.00">
                    <OnDeleteAction>Cascade</OnDeleteAction>
                </ForeignKeyConstraint>
            </ECCustomAttributes>
        )xml", "");
    ExpectRejected(schemaXml, "ECDb_0354");
    }

//***************************************************************************************
// Link table mapping custom attributes
//***************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, AddingLinkTableRelationshipMapIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "DocumentRefersToNotes", R"xml(<Source multiplicity="(0..*)" roleLabel="refers to")xml", R"xml(<ECCustomAttributes>
            <LinkTableRelationshipMap xmlns="ECDbMap.02.00.00">
                <CreateForeignKeyConstraints>False</CreateForeignKeyConstraints>
            </LinkTableRelationshipMap>
        </ECCustomAttributes>
        <Source multiplicity="(0..*)" roleLabel="refers to")xml");
    ExpectRejected(schemaXml, "ECDb_0354");
    }

//---------------------------------------------------------------------------------------
// AllowDuplicateRelationships is the only LinkTableRelationshipMap value that may change.
// ElementRefersToElements is mapped TablePerHierarchy, which ignores the flag also when the class is
// first mapped, so duplicates stay rejected.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, SetAllowDuplicateRelationships)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "ElementRefersToElements", "<CreateForeignKeyConstraints>False</CreateForeignKeyConstraints>", R"xml(<CreateForeignKeyConstraints>False</CreateForeignKeyConstraints>
                <AllowDuplicateRelationships>True</AllowDuplicateRelationships>)xml");
    ExpectAccepted(schemaXml);

    ECInstanceKey source = SelectKey("SELECT SourceECInstanceId, SourceECClassId FROM ts.PipeFeedsValve ORDER BY ECInstanceId LIMIT 1");
    ECInstanceKey target = SelectKey("SELECT TargetECInstanceId, TargetECClassId FROM ts.PipeFeedsValve ORDER BY ECInstanceId LIMIT 1");
    EXPECT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, InsertLinkTableRelationship("ts.PipeFeedsValve", source, target));
    }

//***************************************************************************************
// Subclasses of constraint classes
//***************************************************************************************

//---------------------------------------------------------------------------------------
// The new class shares the table of the constraint class.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, AddSubclassOfConstraintClass)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    AppendToSchema(schemaXml, R"xml(
    <ECEntityClass typeName="PressurePipe">
        <BaseClass>Pipe</BaseClass>
        <ECProperty propertyName="Rating" typeName="int" />
    </ECEntityClass>
)xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// The new class inherits a navigation property.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, AddSubclassOfNavigationPropertyHolder)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    AppendToSchema(schemaXml, R"xml(
    <ECEntityClass typeName="FlowSensor">
        <BaseClass>Sensor</BaseClass>
        <ECProperty propertyName="FlowRange" typeName="double" />
    </ECEntityClass>
)xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// Note has its own table and so would its subclass. Link table ends must map to a single table.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipUpdateTests, AddingSubclassInOwnTableToLinkTableConstraintIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    AppendToSchema(schemaXml, R"xml(
    <ECEntityClass typeName="StickyNote">
        <BaseClass>Note</BaseClass>
        <ECProperty propertyName="Color" typeName="string" />
    </ECEntityClass>
)xml");
    ExpectRejected(schemaXml, "ECDb_0195");
    }

END_ECDBUNITTESTS_NAMESPACE
