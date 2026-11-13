/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "SchemaUpdateTestFixture.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// Scope of the import option DoNotFailForDeletionsOrModifications. DgnDb::ImportV8LegacySchemas
// sets it, so it applies to every schema the MicroStation connector imports.
// With the option, some rejected changes are skipped with an info message and the rest of the
// import goes on. Other rejected changes still fail the import.
//
// Two schemas, like a domain schema and a connector's dynamic schema:
// - Domain: elements mapped like BisCore elements, TablePerHierarchy with shared columns,
//   a class with its own table, enumerations and a KindOfQuantity
// - Dynamic: a dynamic schema deriving from Domain
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct DoNotFailUpdateTests : SchemaUpdateTestFixture
    {
    static SchemaUpdateBaseline const& GetBaseline()
        {
        static SchemaUpdateBaseline s_baseline("DoNotFail", bvector<Utf8String>{R"xml(<?xml version="1.0" encoding="utf-8"?>
<ECSchema schemaName="Domain" alias="dom" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
    <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA" />
    <ECSchemaReference name="Units" version="01.00.00" alias="u" />
    <ECSchemaReference name="Formats" version="01.00.00" alias="f" />
    <ECEnumeration typeName="Status" backingTypeName="int" isStrict="true">
        <ECEnumerator name="Planned" value="1" />
        <ECEnumerator name="Built" value="2" />
        <ECEnumerator name="Removed" value="3" />
    </ECEnumeration>
    <ECEnumeration typeName="Color" backingTypeName="string" isStrict="false"><ECEnumerator name="Red" value="red" /><ECEnumerator name="Blue" value="blue" /></ECEnumeration>
    <KindOfQuantity typeName="Length" persistenceUnit="u:M" relativeError="0.0001" />
    <ECEntityClass typeName="Element" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
            <ShareColumns xmlns="ECDbMap.02.00.00">
                <MaxSharedColumnsBeforeOverflow>8</MaxSharedColumnsBeforeOverflow>
                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
            </ShareColumns>
        </ECCustomAttributes>
        <ECProperty propertyName="Code" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Pipe">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="Diameter" typeName="double" kindOfQuantity="Length" />
        <ECProperty propertyName="Material" typeName="string" />
        <ECProperty propertyName="Status" typeName="Status" />
    </ECEntityClass>
    <ECEntityClass typeName="Valve">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="Pressure" typeName="double" />
    </ECEntityClass>
    <ECEntityClass typeName="Note">
        <ECProperty propertyName="Text" typeName="string" />
    </ECEntityClass>
</ECSchema>)xml",
R"xml(<?xml version="1.0" encoding="utf-8"?>
<ECSchema schemaName="Dynamic" alias="dyn" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
    <ECSchemaReference name="Domain" version="01.00.00" alias="dom" />
    <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA" />
    <ECCustomAttributes>
        <DynamicSchema xmlns="CoreCustomAttributes.01.00.00" />
    </ECCustomAttributes>
    <ECEntityClass typeName="DynamicPipe">
        <BaseClass>dom:Pipe</BaseClass>
        <ECProperty propertyName="Rating" typeName="int" />
        <ECProperty propertyName="Vendor" typeName="string" />
    </ECEntityClass>
</ECSchema>)xml"});
        return s_baseline;
        }

    //! The options of DgnDb::ImportV8LegacySchemas, plus the ones every iModel import adds.
    static SchemaManager::SchemaImportOptions GetV8LegacyImportOptions()
        {
        return SchemaManager::SchemaImportOptions::DoNotFailSchemaValidationForLegacyIssues |
            SchemaManager::SchemaImportOptions::DoNotFailForDeletionsOrModifications |
            SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade |
            SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade |
            SchemaManager::SchemaImportOptions::AllowMajorSchemaUpgradeForDynamicSchemas;
        }

    //! Expects the import to be accepted with the given info message for the skipped change, then verifies all data.
    void ExpectSkipped(Utf8StringCR schemaXml, Utf8CP issueId)
        {
        EXPECT_EQ(SUCCESS, UpdateSchema(schemaXml, SchemaVersionChange::Minor, GetV8LegacyImportOptions()));
        EXPECT_TRUE(WasReported(issueId)) << issueId;
        VerifyData();
        }

    //! Expects the import to be rejected with the given issue, then verifies that nothing changed.
    void ExpectRejected(Utf8StringCR schemaXml, Utf8CP issueId)
        {
        EXPECT_EQ(ERROR, UpdateSchema(schemaXml, SchemaVersionChange::Minor, GetV8LegacyImportOptions()));
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

    ECClassCP GetPersistedClass(Utf8CP schemaName, Utf8CP className) const
        {
        ECClassCP ecClass = m_ecdb.Schemas().GetClass(schemaName, className);
        EXPECT_TRUE(ecClass != nullptr) << schemaName << ":" << className;
        return ecClass;
        }

    ECPropertyCP GetPersistedProperty(Utf8CP schemaName, Utf8CP className, Utf8CP propertyName) const
        {
        ECClassCP ecClass = GetPersistedClass(schemaName, className);
        ECPropertyCP property = ecClass == nullptr ? nullptr : ecClass->GetPropertyP(propertyName, false);
        EXPECT_TRUE(property != nullptr) << schemaName << ":" << className << "." << propertyName;
        return property;
        }
    };

//***************************************************************************************
// Skipped changes. The import succeeds, the change is not stored and the other changes are.
//***************************************************************************************

//---------------------------------------------------------------------------------------
// Items are matched by name case-insensitively, so a rename can only change the case.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DoNotFailUpdateTests, ChangingCaseOfClassNameIsSkipped)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("Domain");
    ReplaceInClass(schemaXml, "Valve", R"xml(typeName="Valve")xml", R"xml(typeName="VALVE" description="Controls flow")xml");
    ExpectSkipped(schemaXml, "ECDb_0639");

    ECClassCP valve = GetPersistedClass("Domain", "Valve");
    ASSERT_TRUE(valve != nullptr);
    EXPECT_STREQ("Valve", valve->GetName().c_str());
    EXPECT_STREQ("Controls flow", valve->GetDescription().c_str());
    }

//---------------------------------------------------------------------------------------
// Disabled: skipping the rename returns before the other changes of the property are written, so the display label is lost.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DoNotFailUpdateTests, DISABLED_ChangingCaseOfPropertyNameIsSkipped)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("Domain");
    ReplaceInClass(schemaXml, "Pipe", R"xml(<ECProperty propertyName="Material" typeName="string" />)xml",
        R"xml(<ECProperty propertyName="MATERIAL" typeName="string" displayLabel="Pipe material" />)xml");
    ExpectSkipped(schemaXml, "ECDb_0632");

    ECPropertyCP material = GetPersistedProperty("Domain", "Pipe", "Material");
    ASSERT_TRUE(material != nullptr);
    EXPECT_STREQ("Material", material->GetName().c_str());
    EXPECT_STREQ("Pipe material", material->GetDisplayLabel().c_str());
    }

//---------------------------------------------------------------------------------------
// Disabled: skipping the type change returns before the other changes of the property are written, so the display label is lost.
// A change to a point type is rejected even with a major version change.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DoNotFailUpdateTests, DISABLED_UnsupportedPropertyTypeChangeIsSkipped)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("Domain");
    ReplaceInClass(schemaXml, "Pipe", R"xml(<ECProperty propertyName="Material" typeName="string" />)xml",
        R"xml(<ECProperty propertyName="Material" typeName="point3d" displayLabel="Pipe material" />)xml");
    ExpectSkipped(schemaXml, "ECDb_0633");

    ECPropertyCP material = GetPersistedProperty("Domain", "Pipe", "Material");
    ASSERT_TRUE(material != nullptr && material->GetIsPrimitive());
    EXPECT_EQ(PRIMITIVETYPE_String, material->GetAsPrimitiveProperty()->GetType());
    EXPECT_STREQ("Pipe material", material->GetDisplayLabel().c_str());
    }

//---------------------------------------------------------------------------------------
// Disabled: skipping the KindOfQuantity change returns before the other changes of the property are written, so the display label is lost.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DoNotFailUpdateTests, DISABLED_ChangingKindOfQuantityOfPropertyWithoutAllowUnitChangeIsSkipped)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("Domain");
    ReplaceInSchema(schemaXml, R"xml(<KindOfQuantity typeName="Length" persistenceUnit="u:M" relativeError="0.0001" />)xml",
        R"xml(<KindOfQuantity typeName="Length" persistenceUnit="u:M" relativeError="0.0001" />
    <KindOfQuantity typeName="SmallLength" persistenceUnit="u:MM" relativeError="0.0001" />)xml");
    ReplaceInClass(schemaXml, "Pipe", R"xml(<ECProperty propertyName="Diameter" typeName="double" kindOfQuantity="Length" />)xml",
        R"xml(<ECProperty propertyName="Diameter" typeName="double" kindOfQuantity="SmallLength" displayLabel="Outer diameter" />)xml");
    ExpectSkipped(schemaXml, "ECDb_0343");

    ECPropertyCP diameter = GetPersistedProperty("Domain", "Pipe", "Diameter");
    ASSERT_TRUE(diameter != nullptr && diameter->GetKindOfQuantity() != nullptr);
    EXPECT_STREQ("Length", diameter->GetKindOfQuantity()->GetName().c_str());
    EXPECT_STREQ("Outer diameter", diameter->GetDisplayLabel().c_str());
    }

//---------------------------------------------------------------------------------------
// Disabled: skipping the persistence unit change skips the whole update of the KindOfQuantity, so the description is lost.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DoNotFailUpdateTests, DISABLED_ChangingPersistenceUnitOfKindOfQuantityIsSkipped)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("Domain");
    ReplaceInSchema(schemaXml, R"xml(<KindOfQuantity typeName="Length" persistenceUnit="u:M" relativeError="0.0001" />)xml",
        R"xml(<KindOfQuantity typeName="Length" persistenceUnit="u:MM" relativeError="0.0001" description="Lengths of parts" />)xml");
    ExpectSkipped(schemaXml, "ECDb_0645");

    KindOfQuantityCP length = m_ecdb.Schemas().GetKindOfQuantity("Domain", "Length");
    ASSERT_TRUE(length != nullptr && length->GetPersistenceUnit() != nullptr);
    EXPECT_STREQ("M", length->GetPersistenceUnit()->GetName().c_str());
    EXPECT_STREQ("Lengths of parts", length->GetDescription().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DoNotFailUpdateTests, DeletingPropertyWithoutReadVersionIncrementIsSkipped)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("Domain");
    ReplaceInClass(schemaXml, "Pipe", R"xml(<ECProperty propertyName="Material" typeName="string" />)xml", "");
    ExpectSkipped(schemaXml, "ECDb_0641");

    EXPECT_TRUE(GetPersistedProperty("Domain", "Pipe", "Material") != nullptr);
    }

//---------------------------------------------------------------------------------------
// The option does not protect dynamic schemas: iModel imports allow major changes to them.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DoNotFailUpdateTests, DeletingPropertyOfDynamicSchemaIsNotSkipped)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("Dynamic");
    ReplaceInClass(schemaXml, "DynamicPipe", R"xml(<ECProperty propertyName="Vendor" typeName="string" />)xml", "");
    EXPECT_EQ(SUCCESS, UpdateSchema(schemaXml, SchemaVersionChange::Minor, GetV8LegacyImportOptions()));
    EXPECT_FALSE(WasReported("ECDb_0641"));
    VerifyData();

    ECClassCP dynamicPipe = GetPersistedClass("Dynamic", "DynamicPipe");
    ASSERT_TRUE(dynamicPipe != nullptr);
    EXPECT_TRUE(dynamicPipe->GetPropertyP("Vendor", false) == nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DoNotFailUpdateTests, DeletingClassWithoutReadVersionIncrementIsSkipped)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("Domain");
    RemoveClass(schemaXml, "Valve");
    ExpectSkipped(schemaXml, "ECDb_0640");

    EXPECT_TRUE(GetPersistedClass("Domain", "Valve") != nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DoNotFailUpdateTests, AddingBaseClassIsSkipped)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("Domain");
    ReplaceInClass(schemaXml, "Note", R"xml(<ECProperty propertyName="Text" typeName="string" />)xml", R"xml(<BaseClass>Element</BaseClass>
        <ECProperty propertyName="Text" typeName="string" />)xml");
    ExpectSkipped(schemaXml, "ECDb_0638");

    ECClassCP note = GetPersistedClass("Domain", "Note");
    ASSERT_TRUE(note != nullptr);
    EXPECT_FALSE(note->HasBaseClasses());
    }

//---------------------------------------------------------------------------------------
// Disabled: the mixin change rewrites all base classes of Note from the incoming schema, so the skipped base class is stored too.
// Note then derives from the TablePerHierarchy root Element while its instances stay in their own table.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DoNotFailUpdateTests, DISABLED_AddingBaseClassTogetherWithMixinIsSkipped)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("Domain");
    AppendToSchema(schemaXml, R"xml(
    <ECEntityClass typeName="IAnnotated" modifier="Abstract">
        <ECCustomAttributes>
            <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                <AppliesToEntityClass>Note</AppliesToEntityClass>
            </IsMixin>
        </ECCustomAttributes>
    </ECEntityClass>
)xml");
    ReplaceInClass(schemaXml, "Note", R"xml(<ECProperty propertyName="Text" typeName="string" />)xml", R"xml(<BaseClass>Element</BaseClass>
        <BaseClass>IAnnotated</BaseClass>
        <ECProperty propertyName="Text" typeName="string" />)xml");
    ExpectSkipped(schemaXml, "ECDb_0638");

    ECClassCP note = GetPersistedClass("Domain", "Note");
    ASSERT_TRUE(note != nullptr);
    ASSERT_EQ((size_t) 1, note->GetBaseClasses().size());
    EXPECT_STREQ("IAnnotated", note->GetBaseClasses()[0]->GetName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DoNotFailUpdateTests, RemovingBaseClassIsSkipped)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("Domain");
    ReplaceInClass(schemaXml, "Valve", "<BaseClass>Element</BaseClass>", "");
    ExpectSkipped(schemaXml, "ECDb_0637");

    ECClassCP valve = GetPersistedClass("Domain", "Valve");
    ASSERT_TRUE(valve != nullptr);
    ASSERT_EQ((size_t) 1, valve->GetBaseClasses().size());
    EXPECT_STREQ("Element", valve->GetBaseClasses()[0]->GetName().c_str());
    }

//---------------------------------------------------------------------------------------
// Disabled: the deletion is reported as skipped, but the new list of enumerators is stored.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DoNotFailUpdateTests, DISABLED_DeletingEnumeratorIsSkipped)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("Domain");
    ReplaceInSchema(schemaXml, R"xml(<ECEnumerator name="Removed" value="3" />)xml", "");
    ExpectSkipped(schemaXml, "ECDb_0650");

    ECEnumerationCP status = m_ecdb.Schemas().GetEnumeration("Domain", "Status");
    ASSERT_TRUE(status != nullptr);
    EXPECT_EQ((size_t) 3, status->GetEnumeratorCount());
    EXPECT_TRUE(status->FindEnumerator(3) != nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DoNotFailUpdateTests, DeletingEnumerationWithoutReadVersionIncrementIsSkipped)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("Domain");
    ReplaceInSchema(schemaXml, R"xml(<ECEnumeration typeName="Color" backingTypeName="string" isStrict="false"><ECEnumerator name="Red" value="red" /><ECEnumerator name="Blue" value="blue" /></ECEnumeration>)xml", "");
    ExpectSkipped(schemaXml, "ECDb_0652");

    EXPECT_TRUE(m_ecdb.Schemas().GetEnumeration("Domain", "Color") != nullptr);
    }

//***************************************************************************************
// Changes the option does not cover. They fail the import as without the option.
//***************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DoNotFailUpdateTests, ChangingClassModifierIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("Domain");
    ReplaceInClass(schemaXml, "Valve", R"xml(typeName="Valve")xml", R"xml(typeName="Valve" modifier="Abstract")xml");
    ExpectRejected(schemaXml, "ECDb_0363");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DoNotFailUpdateTests, ChangingPropertyKindIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("Domain");
    ReplaceInClass(schemaXml, "Pipe", R"xml(<ECProperty propertyName="Material" typeName="string" />)xml",
        R"xml(<ECArrayProperty propertyName="Material" typeName="string" />)xml");
    ExpectRejected(schemaXml, "ECDb_0337");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DoNotFailUpdateTests, ChangingECDbMapCustomAttributeIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("Domain");
    ReplaceInClass(schemaXml, "Element", "<MaxSharedColumnsBeforeOverflow>8</MaxSharedColumnsBeforeOverflow>",
        "<MaxSharedColumnsBeforeOverflow>16</MaxSharedColumnsBeforeOverflow>");
    ExpectRejected(schemaXml, "ECDb_0354");
    }

END_ECDBUNITTESTS_NAMESPACE
