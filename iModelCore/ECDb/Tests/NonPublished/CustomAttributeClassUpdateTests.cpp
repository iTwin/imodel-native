/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "SchemaUpdateTestFixture.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// Changes to custom attribute classes whose instances are stored in the file.
// ECDb stores custom attribute instances as XML and reads them with the current definition of their class.
// An update of a schema with custom attribute classes does not rewrite the instances that other schemas hold,
// like an update of a custom attribute schema that the schemas using it do not take part in.
//
// Two schemas:
// - Attributes: custom attribute classes only
// - TestSchema: uses them on a class and a property
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct CustomAttributeClassUpdateTests : SchemaUpdateTestFixture
    {
    static SchemaUpdateBaseline const& GetBaseline()
        {
        static SchemaUpdateBaseline s_baseline("CustomAttributeClasses", bvector<Utf8String>{R"xml(<?xml version="1.0" encoding="utf-8"?>
<ECSchema schemaName="Attributes" alias="attr" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
    <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA" />
    <ECCustomAttributeClass typeName="Tag" appliesTo="EntityClass, PrimitiveProperty">
        <ECProperty propertyName="Code" typeName="string" />
        <ECProperty propertyName="Level" typeName="int" />
        <ECProperty propertyName="Remark" typeName="string" />
    </ECCustomAttributeClass>
    <ECCustomAttributeClass typeName="Marker" appliesTo="EntityClass" />
    <ECCustomAttributeClass typeName="Spare" appliesTo="Any" />
</ECSchema>)xml",
R"xml(<?xml version="1.0" encoding="utf-8"?>
<ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
    <ECSchemaReference name="Attributes" version="01.00.00" alias="attr" />
    <ECEntityClass typeName="Pipe">
        <ECCustomAttributes>
            <Tag xmlns="Attributes.01.00.00">
                <Code>P-1</Code>
                <Level>2</Level>
                <Remark>steel</Remark>
            </Tag>
            <Marker xmlns="Attributes.01.00.00" />
        </ECCustomAttributes>
        <ECProperty propertyName="Diameter" typeName="double">
            <ECCustomAttributes>
                <Tag xmlns="Attributes.01.00.00">
                    <Code>D-1</Code>
                    <Level>3</Level>
                </Tag>
            </ECCustomAttributes>
        </ECProperty>
        <ECProperty propertyName="Material" typeName="string" />
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

    void ExpectAccepted(Utf8StringCR schemaXml, SchemaVersionChange versionChange = SchemaVersionChange::Minor,
        SchemaManager::SchemaImportOptions options = SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade)
        {
        EXPECT_EQ(SUCCESS, UpdateSchema(schemaXml, versionChange, options));
        VerifyData();
        }

    void ExpectRejected(Utf8StringCR schemaXml, Utf8CP issueId, SchemaVersionChange versionChange = SchemaVersionChange::Minor)
        {
        EXPECT_EQ(ERROR, UpdateSchema(schemaXml, versionChange));
        ExpectRejectionReason(issueId);
        VerifyData();
        }

    ECClassCP GetPipe() const { return m_ecdb.Schemas().GetClass("TestSchema", "Pipe"); }

    //! The Tag on Pipe, or on its property if a name is given. Null if Pipe or the custom attribute cannot be loaded.
    IECInstancePtr GetTag(Utf8CP propertyName = nullptr) const
        {
        ECClassCP pipe = GetPipe();
        if (pipe == nullptr)
            return nullptr;

        if (propertyName == nullptr)
            return pipe->GetCustomAttribute("Attributes", "Tag");

        ECPropertyCP prop = pipe->GetPropertyP(propertyName);
        return prop == nullptr ? nullptr : prop->GetCustomAttribute("Attributes", "Tag");
        }

    //! The value as a string, "<null>" for NULL, "<missing>" if the custom attribute has no such property.
    static Utf8String GetValueAsString(IECInstanceCR instance, Utf8CP accessString)
        {
        ECValue value;
        if (ECObjectsStatus::Success != instance.GetValue(value, accessString))
            return "<missing>";

        return value.IsNull() ? "<null>" : value.ToString();
        }

    //! Expects the Tag on Pipe and on Diameter to read the given values.
    void ExpectTags(Utf8CP pipeCode, Utf8CP pipeLevel, Utf8CP pipeRemark, Utf8CP diameterCode, Utf8CP diameterLevel) const
        {
        IECInstancePtr pipeTag = GetTag();
        ASSERT_TRUE(pipeTag.IsValid()) << "Tag on Pipe cannot be loaded.";
        EXPECT_STREQ(pipeCode, GetValueAsString(*pipeTag, "Code").c_str());
        EXPECT_STREQ(pipeLevel, GetValueAsString(*pipeTag, "Level").c_str());
        EXPECT_STREQ(pipeRemark, GetValueAsString(*pipeTag, "Remark").c_str());

        IECInstancePtr diameterTag = GetTag("Diameter");
        ASSERT_TRUE(diameterTag.IsValid()) << "Tag on Pipe.Diameter cannot be loaded.";
        EXPECT_STREQ(diameterCode, GetValueAsString(*diameterTag, "Code").c_str());
        EXPECT_STREQ(diameterLevel, GetValueAsString(*diameterTag, "Level").c_str());
        }
    };

//***************************************************************************************
// Properties of custom attribute classes
//***************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CustomAttributeClassUpdateTests, AddProperty)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("Attributes");
    ReplaceInClass(schemaXml, "Tag", R"xml(<ECProperty propertyName="Remark" typeName="string" />)xml", R"xml(<ECProperty propertyName="Remark" typeName="string" />
        <ECProperty propertyName="Owner" typeName="string" />)xml");
    ExpectAccepted(schemaXml);
    ExpectTags("P-1", "2", "steel", "D-1", "3");

    IECInstancePtr pipeTag = GetTag();
    ASSERT_TRUE(pipeTag.IsValid());
    EXPECT_STREQ("<null>", GetValueAsString(*pipeTag, "Owner").c_str());
    }

//---------------------------------------------------------------------------------------
// The stored Tag on Pipe still has a value for Remark.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CustomAttributeClassUpdateTests, DeleteProperty)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("Attributes");
    ReplaceInClass(schemaXml, "Tag", R"xml(<ECProperty propertyName="Remark" typeName="string" />)xml", "");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major);
    ExpectTags("P-1", "2", "<missing>", "D-1", "3");
    }

//---------------------------------------------------------------------------------------
// The stored Tag on Pipe is not rewritten by the deletion. The re-added Remark is a new property and
// should not show the value of the deleted one.
// Disabled: the stored XML keeps the deleted value, and the re-added Remark reads "steel" again.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CustomAttributeClassUpdateTests, DISABLED_DeletePropertyThenAddItAgain)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("Attributes");
    ReplaceInClass(schemaXml, "Tag", R"xml(<ECProperty propertyName="Remark" typeName="string" />)xml", "");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major);
    ExpectAccepted(GetBaselineSchemaXml("Attributes"));
    ExpectTags("P-1", "2", "<null>", "D-1", "3");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CustomAttributeClassUpdateTests, DeletingPropertyWithoutMajorVersionIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("Attributes");
    ReplaceInClass(schemaXml, "Tag", R"xml(<ECProperty propertyName="Remark" typeName="string" />)xml", "");
    ExpectRejected(schemaXml, "ECDb_0384");
    ExpectTags("P-1", "2", "steel", "D-1", "3");
    }

//---------------------------------------------------------------------------------------
// The stored values are numbers, which read as text.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CustomAttributeClassUpdateTests, ChangePropertyTypeIntToString)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("Attributes");
    ReplaceInClass(schemaXml, "Tag", R"xml(<ECProperty propertyName="Level" typeName="int" />)xml", R"xml(<ECProperty propertyName="Level" typeName="string" />)xml");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major);
    ExpectTags("P-1", "2", "steel", "D-1", "3");
    }

//---------------------------------------------------------------------------------------
// Records current behavior: the stored Remark on Pipe is text that is no number, and reads as NULL
// without notice. Like type changes of entity properties, nothing converts or checks the stored values.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CustomAttributeClassUpdateTests, ChangePropertyTypeStringToInt_TextReadsAsNull)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("Attributes");
    ReplaceInClass(schemaXml, "Tag", R"xml(<ECProperty propertyName="Remark" typeName="string" />)xml", R"xml(<ECProperty propertyName="Remark" typeName="int" />)xml");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major);
    ExpectTags("P-1", "2", "<null>", "D-1", "3");
    }

//***************************************************************************************
// Custom attribute classes
//***************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CustomAttributeClassUpdateTests, DeleteUnusedClass)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("Attributes");
    RemoveClass(schemaXml, "Spare");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major);
    EXPECT_TRUE(m_ecdb.Schemas().GetClass("Attributes", "Spare") == nullptr);
    }

//---------------------------------------------------------------------------------------
// A dynamic schema with the iModel options takes deletions without a read version increment.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CustomAttributeClassUpdateTests, DeleteUnusedClassOfDynamicSchemaWithIModelOptions)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("Attributes");
    ReplaceInSchema(schemaXml, R"xml(<ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA" />)xml", R"xml(<ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA" />
    <ECCustomAttributes>
        <DynamicSchema xmlns="CoreCustomAttributes.01.00.03" />
    </ECCustomAttributes>)xml");
    ExpectAccepted(schemaXml);

    RemoveClass(schemaXml, "Spare");
    ExpectAccepted(schemaXml, SchemaVersionChange::Minor, GetIModelImportOptions());
    EXPECT_TRUE(m_ecdb.Schemas().GetClass("Attributes", "Spare") == nullptr);
    }

//---------------------------------------------------------------------------------------
// TestSchema is not part of the update, yet Pipe loses its Marker.
// Undecided whether a class still in use in another schema should be deletable.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CustomAttributeClassUpdateTests, DeleteClassUsedByAnotherSchema)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    ASSERT_TRUE(GetPipe()->GetCustomAttribute("Attributes", "Marker").IsValid());

    Utf8String schemaXml = GetBaselineSchemaXml("Attributes");
    RemoveClass(schemaXml, "Marker");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major);
    ASSERT_TRUE(GetPipe() != nullptr);
    EXPECT_FALSE(GetPipe()->GetCustomAttribute("Attributes", "Marker").IsValid());
    ExpectTags("P-1", "2", "steel", "D-1", "3");
    }

//---------------------------------------------------------------------------------------
// Disabled: SchemaComparer does not compare the container type, so the update is accepted and the old
// AppliesTo stays stored. The persisted schema check misses it too. For the same reason narrowing AppliesTo
// below existing uses, like the Tag on Pipe.Diameter, is accepted today and needs a rejection once it is stored.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CustomAttributeClassUpdateTests, DISABLED_WidenAppliesTo)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml("Attributes");
    ReplaceInClass(schemaXml, "Tag", R"xml(appliesTo="EntityClass, PrimitiveProperty")xml", R"xml(appliesTo="Any")xml");
    ExpectAccepted(schemaXml);

    ECClassCP tag = m_ecdb.Schemas().GetClass("Attributes", "Tag");
    ASSERT_TRUE(tag != nullptr && tag->IsCustomAttributeClass());
    EXPECT_EQ(CustomAttributeContainerType::Any, tag->GetCustomAttributeClassCP()->GetContainerType());
    }

END_ECDBUNITTESTS_NAMESPACE
