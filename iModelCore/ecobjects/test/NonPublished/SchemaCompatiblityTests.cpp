/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

// The following test file is to validate that the code gracefully handles unknown 
// scenarios within the EC3 generation.

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct SchemaCompatibilityTests : ECTestFixture
{
    Utf8CP templateSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.%s">
            %s
        </ECSchema>
    )xml";

    ECSchemaPtr m_schema;
    ECSchemaReadContextPtr m_context;

    virtual void SetUp() override
        {
        m_schema = nullptr;
        m_context = ECSchemaReadContext::CreateContext();
        }

    //! The additionalSchemaXml is a snippet of the ECXml that will be placed within the 
    void CreateSchema(Utf8CP additionSchemaXml, Utf8CP failureMessage);
};

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
void SchemaCompatibilityTests::CreateSchema(Utf8CP additionSchemaXml, Utf8CP failureMessage)
    {
    uint32_t ecMajorVersion;
    uint32_t ecMinorVersion;
    ECSchema::ParseECVersion(ecMajorVersion, ecMinorVersion, ECVersion::Latest); // Always want to grab the latest the current software handles

    ASSERT_EQ(3, ecMajorVersion) << "The major version of ECObjects has changed from 3";

    // Add one to the version so that the minor version is always one more than the latest.
    ecMinorVersion += 1;

    // Creating the string for use in the error messages.
    Utf8PrintfString versionString("%d.%d", ecMajorVersion, ecMinorVersion);

    Utf8PrintfString schemaXml(templateSchemaXml, versionString.c_str(), additionSchemaXml);

    Utf8String message = "Failed to load an EC";
    message.append(versionString.c_str())
        .append(" schema ")
        .append(failureMessage);

    DeserializeSchema(m_schema, *m_context, SchemaItem(schemaXml.c_str()), SchemaReadStatus::Success, message.c_str());
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(SchemaCompatibilityTests, PrimitiveTypeAdded)
    {
    Utf8CP partialXml = R"xml(
    <ECEntityClass typeName="TestClass">
        <ECProperty propertyName="BananaTypeProperty" typeName="banana"/>
        <ECArrayProperty propertyName="BananaArrayProperty" typeName="banana"/>
    </ECEntityClass>
    )xml";

    CreateSchema(partialXml, "with an unknown primitive type.");

    ECClassCP testClass = m_schema->GetClassCP("TestClass");
    ASSERT_NE(nullptr, testClass);

    {
    ECPropertyP bananaProp = testClass->GetPropertyP("BananaTypeProperty");
    ASSERT_NE(nullptr, bananaProp) << "Expected the 'BananaTypeProperty' to be found within the 'TestClass'";
    EXPECT_TRUE(bananaProp->GetIsPrimitive());
    EXPECT_EQ(PrimitiveType::PRIMITIVETYPE_String, bananaProp->GetAsPrimitiveProperty()->GetType()) << "The property is expected to have the default Primitive Type of string since it has an unknown type.";
    }
    {
    ECPropertyP bananaArrProp = testClass->GetPropertyP("BananaArrayProperty");
    ASSERT_NE(nullptr, bananaArrProp) << "Expected the 'BananaArrayProperty' to be found within the 'TestClass'";
    EXPECT_TRUE(bananaArrProp->GetIsPrimitiveArray());
    EXPECT_EQ(PrimitiveType::PRIMITIVETYPE_String, bananaArrProp->GetAsPrimitiveArrayProperty()->GetPrimitiveElementType()) << "The property is expected to have the default Primitive Type of string since it has an unknown type.";
    }
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(SchemaCompatibilityTests, NewEnumerationBackingType)
    {
    Utf8CP partialXml = R"xml(
    <ECEnumeration typeName="TestEnumeration" backingTypeName="banana" />
    )xml";

    CreateSchema(partialXml, "with an unknown Enumeration backing type.");
    
    ASSERT_NE(nullptr, m_schema.get());

    ECEnumerationCP ecEnum = m_schema->GetEnumerationCP("TestEnumeration");
    ASSERT_NE(nullptr, ecEnum);
    EXPECT_EQ(PrimitiveType::PRIMITIVETYPE_String, ecEnum->GetType()) << "The enumeration is expected to have a default backing type of string since it has a currently unknown backing type.";
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(SchemaCompatibilityTests, NewClassModifier)
    {
    Utf8CP partialXml = R"xml(
    <ECEntityClass typeName="TestClass" modifier="banana" />
    )xml";

    CreateSchema(partialXml, "with an unknown class modifier.");
    ASSERT_NE(nullptr, m_schema.get());

    ECClassCP testClass = m_schema->GetClassCP("TestClass");
    ASSERT_NE(nullptr, testClass);

    EXPECT_EQ(ECClassModifier::None, testClass->GetClassModifier()) << "The class is expected to have a modifier of None since it currently has an unknown backing type";
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(SchemaCompatibilityTests, NewPropertyKind)
    {
    Utf8CP partialXml = R"xml(
    <ECEntityClass typeName="TestClass">
        <MyNewPropertyKind propertyName="MyNewProperty" />
    </ECEntityClass>
    )xml";

    CreateSchema(partialXml, "with a new property kind");
    ASSERT_NE(nullptr, m_schema.get());

    ECClassCP testClass = m_schema->GetClassCP("TestClass");
    ASSERT_NE(nullptr, testClass);

    EXPECT_EQ(0, testClass->GetPropertyCount()) << "The property kind that is unknown is expected to be dropped from the class.";
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(SchemaCompatibilityTests, NewStrengthType)
    {
    Utf8CP partialXml = R"xml(
    <ECEntityClass typeName="Source"/>
    <ECEntityClass typeName="Target"/>

    <ECRelationshipClass typeName="TestRelationship" modifier="None" direction="forward" strength="banana">
        <Source multiplicity="(1..1)" roleLabel="likes" polymorphic="False">
            <Class class="Source" />
        </Source>
        <Target multiplicity="(1..1)" roleLabel="is liked by" polymorphic="True">
            <Class class="Target" />
        </Target>
    </ECRelationshipClass>
    )xml";

    CreateSchema(partialXml, "with an unknown relationship strength.");
    ASSERT_NE(nullptr, m_schema.get());

    ECClassCP testClass = m_schema->GetClassCP("TestRelationship");
    ASSERT_NE(nullptr, testClass);
    EXPECT_TRUE(testClass->IsRelationshipClass());

    EXPECT_EQ(StrengthType::Referencing, testClass->GetRelationshipClassCP()->GetStrength()) << "The relationship is expected to have a strength of Referencing since it currently has an unknown strength";
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(SchemaCompatibilityTests, NewSchemaItemType)
    {
    Utf8CP partialXml = R"xml(
    <BananaClass typeName="TestBananaClass" bananaAttribute="yellow"/>
    )xml";

    CreateSchema(partialXml, "with an unknown Schema Item type.");
    ASSERT_NE(nullptr, m_schema.get());
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(SchemaCompatibilityTests, SchemaItemWithUnknownAttributes)
    {
    Utf8CP partialXml = R"xml(
    <ECEntityClass typeName="TestBananaClass" bananaAttribute="yellow"/>
    )xml";

    CreateSchema(partialXml, "with an unknown attribute on a Schema Item.");
    ASSERT_NE(nullptr, m_schema.get());

    ECClassCP ecClass = m_schema->GetClassCP("TestBananaClass");
    EXPECT_NE(nullptr, ecClass);
    }

END_BENTLEY_ECN_TEST_NAMESPACE
