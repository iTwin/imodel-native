/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "SchemaUpdateTestFixture.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// Rules for schemas as a whole: versions, import options, schema attributes and custom
// attributes, references between schemas and schema policies.
//
// Three schemas, like BisCore, a domain schema and a connector's dynamic schema:
// - Base: elements mapped like BisCore elements, TablePerHierarchy with joined tables and shared
//   columns, with a navigation property and a concrete class
// - TestSchema: derives from Base, has a class with its own table and a link table relationship
//   with a Base class as source. Its version 02.01.05 leaves room to lower each digit.
// - Dynamic: a dynamic schema deriving from TestSchema
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct SchemaUpdateRulesTests : SchemaUpdateTestFixture
    {
    static SchemaUpdateBaseline const& GetBaseline()
        {
        static SchemaUpdateBaseline s_baseline("SchemaUpdateRules", bvector<Utf8String>{R"xml(<?xml version="1.0" encoding="utf-8"?>
<ECSchema schemaName="Base" alias="bs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
    <ECEntityClass typeName="Element" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
            <ShareColumns xmlns="ECDbMap.02.00.00">
                <MaxSharedColumnsBeforeOverflow>8</MaxSharedColumnsBeforeOverflow>
                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
            </ShareColumns>
            <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00" />
        </ECCustomAttributes>
        <ECProperty propertyName="Code" typeName="string" />
        <ECNavigationProperty propertyName="Parent" relationshipName="ElementOwnsChildElements" direction="Backward" />
    </ECEntityClass>
    <ECEntityClass typeName="PhysicalElement" modifier="Abstract">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="Origin" typeName="point3d" />
    </ECEntityClass>
    <ECEntityClass typeName="Annotation">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="Text" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsChildElements" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
</ECSchema>)xml",
R"xml(<?xml version="1.0" encoding="utf-8"?>
<ECSchema schemaName="TestSchema" alias="ts" version="02.01.05" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
    <ECSchemaReference name="Base" version="01.00.00" alias="bs" />
    <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA" />
    <ECCustomAttributes>
        <ProductionStatus xmlns="CoreCustomAttributes.01.00.00">
            <SupportedUse>FieldTesting</SupportedUse>
        </ProductionStatus>
    </ECCustomAttributes>
    <ECEntityClass typeName="Pipe">
        <BaseClass>bs:PhysicalElement</BaseClass>
        <ECProperty propertyName="Diameter" typeName="double" />
        <ECProperty propertyName="Material" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Valve">
        <BaseClass>bs:PhysicalElement</BaseClass>
        <ECProperty propertyName="Pressure" typeName="double" />
    </ECEntityClass>
    <ECEntityClass typeName="Note">
        <ECProperty propertyName="Text" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementRefersToNotes" strength="referencing" modifier="Sealed">
        <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
            <Class class="bs:Element" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is referred to by" polymorphic="true">
            <Class class="Note" />
        </Target>
    </ECRelationshipClass>
</ECSchema>)xml",
R"xml(<?xml version="1.0" encoding="utf-8"?>
<ECSchema schemaName="Dynamic" alias="dyn" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
    <ECSchemaReference name="TestSchema" version="02.01.05" alias="ts" />
    <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA" />
    <ECCustomAttributes>
        <DynamicSchema xmlns="CoreCustomAttributes.01.00.00" />
    </ECCustomAttributes>
    <ECEntityClass typeName="DynamicPipe">
        <BaseClass>ts:Pipe</BaseClass>
        <ECProperty propertyName="Rating" typeName="int" />
        <ECProperty propertyName="Vendor" typeName="string" />
    </ECEntityClass>
</ECSchema>)xml"});
        return s_baseline;
        }

    //! The options every iModel import adds.
    static SchemaManager::SchemaImportOptions GetIModelImportOptions()
        {
        return SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade |
            SchemaManager::SchemaImportOptions::AllowMajorSchemaUpgradeForDynamicSchemas |
            SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade;
        }

    //! Expects the import to be accepted, then verifies all data.
    void ExpectAccepted(Utf8StringCR schemaXml, SchemaVersionChange versionChange = SchemaVersionChange::Minor,
        SchemaManager::SchemaImportOptions options = SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade)
        {
        EXPECT_EQ(SUCCESS, UpdateSchema(schemaXml, versionChange, options));
        VerifyData();
        }

    //! Expects the import to be rejected with the given issue, then verifies that nothing changed.
    void ExpectRejected(Utf8StringCR schemaXml, Utf8CP issueId, SchemaVersionChange versionChange = SchemaVersionChange::Minor,
        SchemaManager::SchemaImportOptions options = SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade)
        {
        EXPECT_EQ(ERROR, UpdateSchema(schemaXml, versionChange, options));
        ExpectRejectionReason(issueId);
        VerifyData();
        }

    bool WasReported(Utf8CP issueId) const
        {
        for (ReportedIssue const& issue : GetLastUpdateIssues())
            {
            if (0 == strcmp(issueId, issue.id.m_issueId))
                return true;
            }

        return false;
        }

    //! Replaces the only occurrence of @p oldText in the schema XML.
    static void ReplaceInSchema(Utf8StringR schemaXml, Utf8CP oldText, Utf8CP newText)
        {
        size_t position = schemaXml.find(oldText);
        if (position == Utf8String::npos || schemaXml.find(oldText, position + 1) != Utf8String::npos)
            {
            ADD_FAILURE() << "Expected exactly one occurrence of '" << oldText << "' in the schema XML.";
            return;
            }

        schemaXml.replace(position, strlen(oldText), newText);
        }

    //! Sets the version attribute of the schema element, for updates with SchemaVersionChange::AsInXml.
    static void SetVersionInXml(Utf8StringR schemaXml, Utf8CP version)
        {
        size_t schemaElementStart = schemaXml.find("<ECSchema ");
        size_t versionStart = schemaElementStart == Utf8String::npos ? Utf8String::npos : schemaXml.find(" version=\"", schemaElementStart);
        if (versionStart == Utf8String::npos || versionStart > schemaXml.find('>', schemaElementStart))
            {
            ADD_FAILURE() << "Version of the schema element not found in the schema XML.";
            return;
            }

        versionStart += strlen(" version=\"");
        schemaXml.replace(versionStart, schemaXml.find('"', versionStart) - versionStart, version);
        }

    ECSchemaCP GetPersistedSchema(Utf8CP schemaName) const
        {
        ECSchemaCP schema = m_ecdb.Schemas().GetSchema(schemaName);
        EXPECT_TRUE(schema != nullptr) << schemaName;
        return schema;
        }

    //! Adds a property Diameter to Base:PhysicalElement, which TestSchema:Pipe already has.
    Utf8String AddDiameterToPhysicalElement() const
        {
        Utf8String baseXml = GetBaselineSchemaXml("Base");
        ReplaceInClass(baseXml, "PhysicalElement", R"xml(<ECProperty propertyName="Origin" typeName="point3d" />)xml", R"xml(<ECProperty propertyName="Origin" typeName="point3d" />
        <ECProperty propertyName="Diameter" typeName="double" />)xml");
        return baseXml;
        }

    //! Adds a property Status to Base:PhysicalElement.
    Utf8String AddStatusToPhysicalElement() const
        {
        Utf8String baseXml = GetBaselineSchemaXml("Base");
        ReplaceInClass(baseXml, "PhysicalElement", R"xml(<ECProperty propertyName="Origin" typeName="point3d" />)xml", R"xml(<ECProperty propertyName="Origin" typeName="point3d" />
        <ECProperty propertyName="Status" typeName="string" />)xml");
        return baseXml;
        }
    };

//***************************************************************************************
// Schema versions
//***************************************************************************************

//---------------------------------------------------------------------------------------
// ECDb does not enforce version increments, it logs a warning.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateRulesTests, ChangeWithoutVersionIncrement)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("TestSchema");
    ReplaceInClass(schemaXml, "Pipe", R"xml(<ECProperty propertyName="Material" typeName="string" />)xml", R"xml(<ECProperty propertyName="Material" typeName="string" />
        <ECProperty propertyName="Length" typeName="double" />)xml");
    ExpectAccepted(schemaXml, SchemaVersionChange::None);
    }

//---------------------------------------------------------------------------------------
// ECDb treats a schema whose minor version is lower than the persisted one as an older compatible
// version and skips it, including all its changes.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateRulesTests, LowerMinorVersionSkipsTheSchema)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("TestSchema");
    SetVersionInXml(schemaXml, "02.01.03");
    ReplaceInClass(schemaXml, "Pipe", R"xml(<ECProperty propertyName="Material" typeName="string" />)xml", R"xml(<ECProperty propertyName="Material" typeName="string" />
        <ECProperty propertyName="Length" typeName="double" />)xml");
    ExpectAccepted(schemaXml, SchemaVersionChange::AsInXml);

    ECSchemaCP schema = GetPersistedSchema("TestSchema");
    ASSERT_TRUE(schema != nullptr);
    EXPECT_STREQ("02.01.05", schema->GetSchemaKey().GetVersionString().c_str());
    EXPECT_TRUE(schema->GetClassCP("Pipe")->GetPropertyP("Length") == nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateRulesTests, LoweringWriteVersionIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("TestSchema");
    SetVersionInXml(schemaXml, "02.00.09");
    ExpectRejected(schemaXml, "ECDb_0428", SchemaVersionChange::AsInXml);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateRulesTests, LoweringReadVersionIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("TestSchema");
    SetVersionInXml(schemaXml, "01.09.09");
    ExpectRejected(schemaXml, "ECDb_0426", SchemaVersionChange::AsInXml);
    }

//***************************************************************************************
// Major changes. Deleting a property in a shared column stands for all changes that need
// the major version gate.
//***************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateRulesTests, DeletePropertyWithReadVersionIncrement)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("TestSchema");
    ReplaceInClass(schemaXml, "Pipe", R"xml(<ECProperty propertyName="Material" typeName="string" />)xml", "");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateRulesTests, DeletingPropertyWithoutReadVersionIncrementIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("TestSchema");
    ReplaceInClass(schemaXml, "Pipe", R"xml(<ECProperty propertyName="Material" typeName="string" />)xml", "");
    ExpectRejected(schemaXml, "ECDb_0384");
    }

//---------------------------------------------------------------------------------------
// iModel imports disallow read version increments, so a regular schema can never take a major change.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateRulesTests, ReadVersionIncrementIsRejectedWithIModelOptions)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("TestSchema");
    ReplaceInClass(schemaXml, "Pipe", R"xml(<ECProperty propertyName="Material" typeName="string" />)xml", "");
    ExpectRejected(schemaXml, "ECDb_0427", SchemaVersionChange::Major, GetIModelImportOptions());
    }

//---------------------------------------------------------------------------------------
// With the iModel options, a dynamic schema takes major changes without a read version increment.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateRulesTests, DeletePropertyOfDynamicSchemaWithIModelOptions)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("Dynamic");
    ReplaceInClass(schemaXml, "DynamicPipe", R"xml(<ECProperty propertyName="Vendor" typeName="string" />)xml", "");
    ExpectAccepted(schemaXml, SchemaVersionChange::Minor, GetIModelImportOptions());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateRulesTests, ReadVersionIncrementOfDynamicSchemaWithIModelOptions)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("Dynamic");
    ReplaceInClass(schemaXml, "DynamicPipe", R"xml(<ECProperty propertyName="Vendor" typeName="string" />)xml", "");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major, GetIModelImportOptions());
    }

//---------------------------------------------------------------------------------------
// Without AllowMajorSchemaUpgradeForDynamicSchemas, dynamic schemas follow the rules of regular schemas.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateRulesTests, DeletingPropertyOfDynamicSchemaWithoutDynamicOptionIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("Dynamic");
    ReplaceInClass(schemaXml, "DynamicPipe", R"xml(<ECProperty propertyName="Vendor" typeName="string" />)xml", "");
    ExpectRejected(schemaXml, "ECDb_0384");
    }

//---------------------------------------------------------------------------------------
// The dynamic flag of the incoming schema counts, so a schema can become dynamic and take a major change
// in the same import. Undecided whether that is intended.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateRulesTests, MakeSchemaDynamicAndDeletePropertyWithIModelOptions)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("TestSchema");
    ReplaceInSchema(schemaXml, "</ProductionStatus>", R"xml(</ProductionStatus>
        <DynamicSchema xmlns="CoreCustomAttributes.01.00.00" />)xml");
    ReplaceInClass(schemaXml, "Pipe", R"xml(<ECProperty propertyName="Material" typeName="string" />)xml", "");
    ExpectAccepted(schemaXml, SchemaVersionChange::Minor, GetIModelImportOptions());
    }

//***************************************************************************************
// Data transforms
//***************************************************************************************

//---------------------------------------------------------------------------------------
// Pipe.Diameter becomes an override of the new base property and moves to its column.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateRulesTests, MovingDataWithoutDataTransformOptionIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    ExpectRejected(AddDiameterToPhysicalElement(), "ECDb_0590", SchemaVersionChange::Minor, SchemaManager::SchemaImportOptions::None);
    }

//---------------------------------------------------------------------------------------
// Pipe.Diameter becomes an override of the new base property and keeps its values. Valve inherits
// the property and reads NULL.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateRulesTests, AddPropertyToReferencedBaseClassThatSubclassAlreadyHas)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    ExpectAccepted(AddDiameterToPhysicalElement());
    }

//***************************************************************************************
// Schema attributes and custom attributes
//***************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateRulesTests, ChangeSchemaLabelAndDescription)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("TestSchema");
    ReplaceInSchema(schemaXml, R"xml(alias="ts" version="02.01.05")xml", R"xml(alias="ts" version="02.01.05" displayLabel="Test schema" description="Pipes and valves")xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateRulesTests, ChangingAliasIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("TestSchema");
    ReplaceInSchema(schemaXml, R"xml(alias="ts" version="02.01.05")xml", R"xml(alias="tst" version="02.01.05")xml");
    ExpectRejected(schemaXml, "ECDb_0430");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateRulesTests, ChangingAliasIsSkippedWithDoNotFail)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("TestSchema");
    ReplaceInSchema(schemaXml, R"xml(alias="ts" version="02.01.05")xml", R"xml(alias="tst" version="02.01.05")xml");
    ExpectAccepted(schemaXml, SchemaVersionChange::Minor,
        SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade | SchemaManager::SchemaImportOptions::DoNotFailForDeletionsOrModifications);

    ECSchemaCP schema = GetPersistedSchema("TestSchema");
    ASSERT_TRUE(schema != nullptr);
    EXPECT_STREQ("ts", schema->GetAlias().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateRulesTests, MakeSchemaDynamic)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("TestSchema");
    ReplaceInSchema(schemaXml, "</ProductionStatus>", R"xml(</ProductionStatus>
        <DynamicSchema xmlns="CoreCustomAttributes.01.00.00" />)xml");
    ExpectAccepted(schemaXml);

    ECSchemaCP schema = GetPersistedSchema("TestSchema");
    ASSERT_TRUE(schema != nullptr);
    EXPECT_TRUE(schema->IsDynamicSchema());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateRulesTests, MakeSchemaNonDynamic)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("Dynamic");
    ReplaceInSchema(schemaXml, R"xml(<DynamicSchema xmlns="CoreCustomAttributes.01.00.00" />)xml", "");
    ExpectAccepted(schemaXml);

    ECSchemaCP schema = GetPersistedSchema("Dynamic");
    ASSERT_TRUE(schema != nullptr);
    EXPECT_FALSE(schema->IsDynamicSchema());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateRulesTests, AddSchemaCustomAttribute)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("TestSchema");
    ReplaceInSchema(schemaXml, "</ProductionStatus>", R"xml(</ProductionStatus>
        <HiddenSchema xmlns="CoreCustomAttributes.01.00.00" />)xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateRulesTests, ChangeSchemaCustomAttributeValue)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("TestSchema");
    ReplaceInSchema(schemaXml, "<SupportedUse>FieldTesting</SupportedUse>", "<SupportedUse>Production</SupportedUse>");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateRulesTests, RemoveSchemaCustomAttribute)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("TestSchema");
    ReplaceInSchema(schemaXml, R"xml(<ProductionStatus xmlns="CoreCustomAttributes.01.00.00">
            <SupportedUse>FieldTesting</SupportedUse>
        </ProductionStatus>)xml", "");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// A SchemaMap cannot be added to an existing schema. The import also has valid custom attribute changes
// that ECDb processes before and after it, ordered by custom attribute class name.
// Disabled: the update ignores that the schema custom attribute was rejected, returns success and does not
// store it. Only the persisted schema check, which is off by default, fails the import.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateRulesTests, DISABLED_AddingSchemaMapIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("TestSchema");
    ReplaceInSchema(schemaXml, R"xml(<ECSchemaReference name="Base" version="01.00.00" alias="bs" />)xml", R"xml(<ECSchemaReference name="Base" version="01.00.00" alias="bs" />
    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
    <ECSchemaReference name="ECDbSchemaPolicies" version="01.00.00" alias="ecdbpol" />)xml");
    ReplaceInSchema(schemaXml, "<SupportedUse>FieldTesting</SupportedUse>", "<SupportedUse>Production</SupportedUse>");
    ReplaceInSchema(schemaXml, "</ProductionStatus>", R"xml(</ProductionStatus>
        <HiddenSchema xmlns="CoreCustomAttributes.01.00.00" />
        <NoAdditionalLinkTables xmlns="ECDbSchemaPolicies.01.00.00" />
        <SchemaMap xmlns="ECDbMap.02.00.00">
            <TablePrefix>tst</TablePrefix>
        </SchemaMap>)xml");
    EXPECT_EQ(ERROR, UpdateSchema(schemaXml));
    ExpectRejectionReason("ECDb_0354");
    EXPECT_FALSE(WasReported("ECDb_0754")) << "The update has to reject the custom attribute itself, not the persisted schema check.";
    VerifyData();
    }

//***************************************************************************************
// References
//***************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateRulesTests, AddReferenceToSchemaInFile)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("TestSchema");
    ReplaceInSchema(schemaXml, R"xml(<ECSchemaReference name="Base" version="01.00.00" alias="bs" />)xml", R"xml(<ECSchemaReference name="Base" version="01.00.00" alias="bs" />
    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />)xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// The referenced schema is new and imported in the same call.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateRulesTests, AddReferenceToSchemaImportedTogether)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String materialsXml(R"xml(<?xml version="1.0" encoding="utf-8"?>
<ECSchema schemaName="Materials" alias="mat" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
    <ECEnumeration typeName="MaterialKind" backingTypeName="int" isStrict="true">
        <ECEnumerator name="Steel" value="1" />
        <ECEnumerator name="Copper" value="2" />
    </ECEnumeration>
</ECSchema>)xml");
    Utf8String schemaXml = GetBaselineSchemaXml("TestSchema");
    ReplaceInSchema(schemaXml, R"xml(<ECSchemaReference name="Base" version="01.00.00" alias="bs" />)xml", R"xml(<ECSchemaReference name="Base" version="01.00.00" alias="bs" />
    <ECSchemaReference name="Materials" version="01.00.00" alias="mat" />)xml");
    ReplaceInClass(schemaXml, "Valve", R"xml(<ECProperty propertyName="Pressure" typeName="double" />)xml", R"xml(<ECProperty propertyName="Pressure" typeName="double" />
        <ECProperty propertyName="BodyMaterial" typeName="mat:MaterialKind" />)xml");
    EXPECT_EQ(SUCCESS, UpdateSchemas({{materialsXml, SchemaVersionChange::AsInXml}, {schemaXml, SchemaVersionChange::Minor}}));
    VerifyData();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateRulesTests, RemoveReference)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("TestSchema");
    ReplaceInSchema(schemaXml, R"xml(<ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA" />)xml", "");
    ReplaceInSchema(schemaXml, R"xml(<ECCustomAttributes>
        <ProductionStatus xmlns="CoreCustomAttributes.01.00.00">
            <SupportedUse>FieldTesting</SupportedUse>
        </ProductionStatus>
    </ECCustomAttributes>)xml", "");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// TestSchema and Dynamic classes inherit the new property and read NULL.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateRulesTests, UpdateReferencedSchemaOnly)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    ExpectAccepted(AddStatusToPhysicalElement());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateRulesTests, UpdateReferencedAndReferencingSchemasTogether)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("TestSchema");
    ReplaceInClass(schemaXml, "Valve", R"xml(<ECProperty propertyName="Pressure" typeName="double" />)xml", R"xml(<ECProperty propertyName="Pressure" typeName="double" />
        <ECProperty propertyName="Rating" typeName="int" />)xml");
    EXPECT_EQ(SUCCESS, UpdateSchemas({{AddStatusToPhysicalElement(), SchemaVersionChange::Minor}, {schemaXml, SchemaVersionChange::Minor}}));
    VerifyData();
    }

//---------------------------------------------------------------------------------------
// Subclasses in TestSchema and Dynamic lose the values of the deleted property.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateRulesTests, DeletePropertyOfReferencedBaseClass)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String baseXml = GetBaselineSchemaXml("Base");
    ReplaceInClass(baseXml, "PhysicalElement", R"xml(<ECProperty propertyName="Origin" typeName="point3d" />)xml", "");
    ExpectAccepted(baseXml, SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateRulesTests, DeletingBaseClassOfReferencingSchemaIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String baseXml = GetBaselineSchemaXml("Base");
    RemoveClass(baseXml, "PhysicalElement");
    ExpectRejected(baseXml, "ECDb_0378", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// Two versions of a referenced schema end up in one import when the schemas were read against different
// versions of it, e.g. one from the file and one from the application. Schemas in the file reference other
// schemas by name, so importing the newer version would serve both.
// Disabled: ECDb computes the changes for one version and applies them to the other. The import fails
// without reporting an issue.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateRulesTests, DISABLED_ImportTwoVersionsOfReferencedSchema)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String materialsXml(R"xml(<?xml version="1.0" encoding="utf-8"?>
<ECSchema schemaName="Materials" alias="mat" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
    <ECSchemaReference name="Base" version="01.00.00" alias="bs" />
    <ECEntityClass typeName="MaterialSample">
        <BaseClass>bs:Element</BaseClass>
        <ECProperty propertyName="Density" typeName="double" />
    </ECEntityClass>
</ECSchema>)xml");
    Utf8String newerBaseXml = AddStatusToPhysicalElement();
    SetVersionInXml(newerBaseXml, "01.00.01");
    Utf8String schemaXml = GetBaselineSchemaXml("TestSchema");
    ReplaceInSchema(schemaXml, R"xml(<ECSchemaReference name="Base" version="01.00.00" alias="bs" />)xml", R"xml(<ECSchemaReference name="Base" version="01.00.01" alias="bs" />)xml");

    EXPECT_EQ(SUCCESS, UpdateSchemas({
        {GetBaselineSchemaXml("Base"), SchemaVersionChange::AsInXml},
        {materialsXml, SchemaVersionChange::AsInXml},
        {newerBaseXml, SchemaVersionChange::AsInXml},
        {schemaXml, SchemaVersionChange::Minor}}));
    VerifyData();

    ECSchemaCP baseSchema = GetPersistedSchema("Base");
    ASSERT_TRUE(baseSchema != nullptr);
    EXPECT_STREQ("01.00.01", baseSchema->GetSchemaKey().GetVersionString().c_str());
    }

//***************************************************************************************
// Schema policies
//***************************************************************************************

//---------------------------------------------------------------------------------------
// Base opts in to NoAdditionalRootEntityClasses after TestSchema already has the root class Note.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateRulesTests, AddingRootClassAfterPolicyOptInIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String baseXml = GetBaselineSchemaXml("Base");
    ReplaceInSchema(baseXml, R"xml(<ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />)xml", R"xml(<ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
    <ECSchemaReference name="ECDbSchemaPolicies" version="01.00.00" alias="ecdbpol" />
    <ECCustomAttributes>
        <NoAdditionalRootEntityClasses xmlns="ECDbSchemaPolicies.01.00.00" />
    </ECCustomAttributes>)xml");
    ASSERT_EQ(SUCCESS, UpdateSchema(baseXml));
    VerifyData();

    Utf8String schemaXml = GetBaselineSchemaXml("TestSchema");
    AppendToSchema(schemaXml, R"xml(
    <ECEntityClass typeName="Gauge">
        <ECProperty propertyName="Reading" typeName="double" />
    </ECEntityClass>
)xml");
    ExpectRejected(schemaXml, "ECDb_0268");
    }

END_ECDBUNITTESTS_NAMESPACE
