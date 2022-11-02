/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"
#include "BeXml/BeXml.h"

#include <iostream>
#include <fstream>
#include <regex>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct SchemaDeserializationTest : ECTestFixture 
    {
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static ECPropertyP GetPropertyByName(ECClassCR ecClass, Utf8CP name, bool expectExists = true)
        {
        ECPropertyP prop = ecClass.GetPropertyP(name);
        EXPECT_EQ(expectExists, nullptr != prop);
        Utf8String utf8(name);
        prop = ecClass.GetPropertyP(utf8.c_str());
        EXPECT_EQ(expectExists, nullptr != prop);
        return prop;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void VerifyWidgetsSchema(ECSchemaPtr const& schema)
        {
        ASSERT_TRUE(schema.IsValid());
        EXPECT_TRUE(schema->IsECVersion(ECVersion::V3_2));

        EXPECT_STREQ("Widgets", schema->GetName().c_str());
        EXPECT_STREQ("wid", schema->GetAlias().c_str());
        EXPECT_STREQ("Widgets Display Label", schema->GetDisplayLabel().c_str());
        EXPECT_TRUE(schema->GetIsDisplayLabelDefined());
        EXPECT_STREQ("Widgets Description", schema->GetDescription().c_str());
        EXPECT_EQ(9, schema->GetVersionRead());
        EXPECT_EQ(6, schema->GetVersionMinor());

    #ifdef DEBUG_PRINT
        for(ECClassP pClass: schema->GetClasses())
            {
            printf("Widgets contains class: '%s' with display label '%s'\n", pClass->GetName().c_str(), pClass->GetDisplayLabel().c_str());
            }
    #endif

        ECClassP pClass = schema->GetClassP("ClassDoesNotExistInSchema");
        EXPECT_FALSE(pClass);

        pClass = schema->GetClassP("ecProject");
        ASSERT_NE(nullptr, pClass);
        EXPECT_STREQ("ecProject", pClass->GetName().c_str());
        EXPECT_STREQ("Project", pClass->GetDisplayLabel().c_str());
        EXPECT_TRUE(pClass->GetIsDisplayLabelDefined());
        EXPECT_STREQ("Project Class", pClass->GetDescription().c_str());
        EXPECT_TRUE(pClass->IsEntityClass());
        EXPECT_FALSE(pClass->HasBaseClasses());
        ECPropertyP pProperty = GetPropertyByName(*pClass, "Name");
        ASSERT_NE(nullptr, pProperty);
        EXPECT_STREQ("Name", pProperty->GetName().c_str());
        EXPECT_TRUE(pProperty->GetIsPrimitive());
        EXPECT_FALSE(pProperty->GetIsStruct());
        EXPECT_FALSE(pProperty->GetIsArray());
        EXPECT_STREQ("string", pProperty->GetTypeName().c_str());
        EXPECT_TRUE(PRIMITIVETYPE_String == pProperty->GetAsPrimitiveProperty()->GetType());
        EXPECT_TRUE(pProperty->GetIsDisplayLabelDefined());
        EXPECT_STREQ("Project Name", pProperty->GetDisplayLabel().c_str());
        EXPECT_STREQ("", pProperty->GetDescription().c_str());
        EXPECT_EQ(pClass, &pProperty->GetClass());
        EXPECT_FALSE(pProperty->GetIsReadOnly());

        pProperty = GetPropertyByName(*pClass, "PropertyDoesNotExistInClass", false);
        EXPECT_FALSE(pProperty);

        ECClassP customAttribClass = schema->GetClassP("AccessCustomAttributes");
        ASSERT_NE(nullptr, customAttribClass);
        EXPECT_STREQ("AccessCustomAttributes", customAttribClass->GetName().c_str());
        EXPECT_STREQ("AccessCustomAttributes", customAttribClass->GetDisplayLabel().c_str());
        EXPECT_FALSE(customAttribClass->GetIsDisplayLabelDefined());
        EXPECT_STREQ("", customAttribClass->GetDescription().c_str());
        EXPECT_TRUE(customAttribClass->IsCustomAttributeClass());
        EXPECT_FALSE(customAttribClass->HasBaseClasses());

        pClass = schema->GetClassP("Struct1");
        ASSERT_NE(nullptr, pClass);
        EXPECT_STREQ("Struct1", pClass->GetName().c_str());
        EXPECT_STREQ("Struct1", pClass->GetDisplayLabel().c_str());
        EXPECT_FALSE(pClass->GetIsDisplayLabelDefined());
        EXPECT_STREQ("", pClass->GetDescription().c_str());
        EXPECT_TRUE(pClass->IsStructClass());
        EXPECT_FALSE(pClass->HasBaseClasses());

        pClass = schema->GetClassP("Struct2");
        ASSERT_NE(nullptr, pClass);
        EXPECT_STREQ("Struct2", pClass->GetName().c_str());
        EXPECT_STREQ("Struct2", pClass->GetDisplayLabel().c_str());
        EXPECT_FALSE(pClass->GetIsDisplayLabelDefined());
        EXPECT_STREQ("", pClass->GetDescription().c_str());
        EXPECT_TRUE(pClass->IsStructClass());
        EXPECT_FALSE(pClass->HasBaseClasses());
        pProperty = GetPropertyByName(*pClass, "NestedArray");
        EXPECT_NE(nullptr, pProperty);
        EXPECT_STREQ("NestedArray", pProperty->GetName().c_str());
        EXPECT_FALSE(pProperty->GetIsPrimitive());
        EXPECT_FALSE(pProperty->GetIsStruct());
        EXPECT_TRUE(pProperty->GetIsArray());
        EXPECT_STREQ("Struct1", pProperty->GetTypeName().c_str());
        StructArrayECPropertyP structArrayProperty = pProperty->GetAsStructArrayPropertyP();
        EXPECT_NE(nullptr, structArrayProperty);
        EXPECT_EQ(schema->GetClassP("Struct1"), &structArrayProperty->GetStructElementType());
        EXPECT_EQ(0, structArrayProperty->GetMinOccurs());
        EXPECT_EQ(INT_MAX, structArrayProperty->GetMaxOccurs());
        EXPECT_FALSE(pProperty->GetIsDisplayLabelDefined());
        EXPECT_STREQ("NestedArray", pProperty->GetDisplayLabel().c_str());
        EXPECT_STREQ("", pProperty->GetDescription().c_str());
        EXPECT_EQ(pClass, &pProperty->GetClass());
        EXPECT_FALSE(pProperty->GetIsReadOnly());

        pClass = schema->GetClassP("TestClass");
        ASSERT_TRUE(nullptr != pClass);
        EXPECT_TRUE(pClass->HasBaseClasses());
        pProperty = GetPropertyByName(*pClass, "EmbeddedStruct");
        ASSERT_TRUE(nullptr != pProperty);
        EXPECT_STREQ("EmbeddedStruct", pProperty->GetName().c_str());
        EXPECT_FALSE(pProperty->GetIsPrimitive());
        EXPECT_TRUE(pProperty->GetIsStruct());
        EXPECT_FALSE(pProperty->GetIsArray());
        EXPECT_STREQ("Struct1", pProperty->GetTypeName().c_str());
        StructECPropertyP structProperty = pProperty->GetAsStructPropertyP();
        EXPECT_EQ(schema->GetClassP("Struct1"), &(structProperty->GetType()));
        EXPECT_FALSE(pProperty->GetIsDisplayLabelDefined());
        EXPECT_STREQ("EmbeddedStruct", pProperty->GetDisplayLabel().c_str());
        EXPECT_STREQ("", pProperty->GetDescription().c_str());
        EXPECT_EQ(pClass, &pProperty->GetClass());
        EXPECT_FALSE(pProperty->GetIsReadOnly());

        IECInstancePtr instance = pClass->GetCustomAttribute(*customAttribClass);
        EXPECT_TRUE(instance.IsValid());

        ECValue ecValue;
        EXPECT_EQ(ECObjectsStatus::Success, instance->GetValue(ecValue, "AccessLevel"));
        EXPECT_EQ(4, ecValue.GetInteger());

        EXPECT_EQ(ECObjectsStatus::Success, instance->GetValue(ecValue, "Writeable"));
        EXPECT_FALSE(ecValue.GetBoolean());

    #ifdef DEBUG_PRINT
        for(ECPropertyP pProperty: pClass->GetProperties())
            {
            printf("TestClass contains property: %s of type %s\n", pProperty->GetName().c_str(), pProperty->GetTypeName().c_str());
            }
    #endif   
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, TestAbstractness)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Abstract' version='01.00' displayLabel='AbstractClasses' description='Schema with abstract class' nameSpacePrefix='ab' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECClass typeName='Abstract1' description='Abstract ECClass' displayLabel='Abstract' isDomainClass='false' isCustomAttributeClass='false' isStruct='false'>"
        "       <ECProperty propertyName='Name' typeName='string' displayLabel='Project Name' />"
        "    </ECClass>"
        "</ECSchema>";
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);

    ECClassCP absClass = schema->GetClassCP("Abstract1");
    ASSERT_TRUE(absClass->IsEntityClass());
    ASSERT_TRUE(ECClassModifier::Abstract == absClass->GetClassModifier());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, InvalidStructArrayPropertySpecification)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();

    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='StructArray' version='01.00' displayLabel='StructArrays' description='Schema with invalid XML for a struct array property' nameSpacePrefix='sa' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECClass typeName='Ent1' description='Entity ECClass' displayLabel='Entity' isDomainClass='true' isCustomAttributeClass='false' isStruct='false'>"
        "        <ECArrayProperty propertyName = 'TypeReferences' typeName = 'string' minOccurs = '0' maxOccurs = 'unbounded' isStruct = 'True' />"
        "    </ECClass>"
        "</ECSchema>";
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);
    ECClassP ent = schema->GetClassP("Ent1");
    ASSERT_TRUE(ent->IsEntityClass());
    ECPropertyP prop = ent->GetPropertyP("TypeReferences");
    ASSERT_NE(nullptr, prop);
    StructArrayECPropertyCP typeReferences1 = prop->GetAsStructArrayProperty();
    ASSERT_EQ(nullptr, typeReferences1);

    ArrayECPropertyCP typeReferences2 = prop->GetAsArrayProperty();
    ASSERT_NE(nullptr, typeReferences2);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, CaseSensitivity)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();

    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='CaseInsensitive' version='01.00' displayLabel='Case Insensitive' description='Testing case sensitivity with struct names and custom attributes' nameSpacePrefix='cs' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECCustomAttributeClass typeName='CustomAttrib' description='CustomAttribute' displayLabel='CA' />"
        "    <ECStructClass typeName='MyStruct'>"
        "        <ECProperty propertyName='Prop1' typeName='string' />"
        "    </ECStructClass>"
        "    <ECEntityClass typeName='Entity'>"
        "        <ECCustomAttributes>"
        "            <Customattrib xmlns='CaseInsensitive.01.00' />"
        "        </ECCustomAttributes>"
        "        <ECStructProperty propertyName='StructProp' typeName='Mystruct' />"
        "    </ECEntityClass>"
        "</ECSchema>";
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);
    ECClassP ent = schema->GetClassP("Entity");
    ASSERT_NE(nullptr, ent);
    ECPropertyP prop = ent->GetPropertyP("StructProp");
    ASSERT_NE(nullptr, prop);
    StructECPropertyP structProp = prop->GetAsStructPropertyP();
    ASSERT_NE(nullptr, structProp);
    EXPECT_TRUE(ent->IsDefined("CustomAttrib"));

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenRoundtripUsingFile)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"Widgets.01.00.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);
    VerifyWidgetsSchema(schema);

    SchemaWriteStatus status2 = schema->WriteToXmlFile(ECTestFixture::GetTempDataPath(L"widgets.xml").c_str());
    EXPECT_EQ(SchemaWriteStatus::Success, status2);

    ECSchemaPtr deserializedSchema;
    schemaContext = ECSchemaReadContext::CreateContext();
    status = ECSchema::ReadFromXmlFile(deserializedSchema, ECTestFixture::GetTempDataPath(L"widgets.xml").c_str(), *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);
    VerifyWidgetsSchema(deserializedSchema);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectErrorWhenXmlFileDoesNotExist)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling(true, false);

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"ThisFileIsntReal.xml").c_str(), *schemaContext);

    EXPECT_EQ(SchemaReadStatus::FailedToParseXml, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectErrorWhenXmlFileIsMissingNodes)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling(true, false);
    DISABLE_ASSERTS
        ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"MissingNodes.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ(SchemaReadStatus::FailedToParseXml, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectErrorWhenXmlFileIsIllFormed)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling(true, false);
    DISABLE_ASSERTS
        ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"IllFormedXml.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ(SchemaReadStatus::FailedToParseXml, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectErrorWhenXmlFileIsMissingECSchemaNode)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling(true, false);

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"MissingECSchemaNode.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ(SchemaReadStatus::InvalidECSchemaXml, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectErrorWhenXmlFileIsMissingNamespace)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling(true, false);

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"MissingNamespace.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ(SchemaReadStatus::InvalidECSchemaXml, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenCustomAttributeIsMissingNamespace)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling(true, false);

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"MissingNamespaceOnCustomAttribute.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ(SchemaReadStatus::Success, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectErrorWhenXmlFileHasUnsupportedNamespace)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling(true, false);

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"UnsupportedECXmlNamespace.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ(SchemaReadStatus::InvalidECSchemaXml, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectErrorWhenRelationshipEndpointNotFound)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling(true, false);

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"BadRelationship.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ(SchemaReadStatus::InvalidECSchemaXml, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWithDuplicateNamespacePrefixes)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling(true, false);

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"DuplicatePrefixes.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ(SchemaReadStatus::Success, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectErrorWhenXmlFileHasMissingSchemaNameAttribute)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling(true, false);

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"MissingSchemaName.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ(SchemaReadStatus::InvalidECSchemaXml, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectErrorWhenXmlFileHasMissingClassNameAttribute)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling(true, false);

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"MissingClassName.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ(SchemaReadStatus::InvalidECSchemaXml, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenXmlFileHasInvalidVersionString)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling(true, false);

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"InvalidVersionString.01.00.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ(1, schema->GetVersionRead());
    EXPECT_EQ(0, schema->GetVersionMinor());

    EXPECT_EQ(SchemaReadStatus::Success, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectFailureWhenMissingTypeNameInProperty)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling(true, false);

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Widgets' version='09.06' displayLabel='Widgets Display Label' description='Widgets Description' nameSpacePrefix='wid' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECClass typeName='ecProject' description='Project ECClass' displayLabel='Project' isDomainClass='True'>"
        "       <ECProperty propertyName='Name' typename='string' displayLabel='Project Name' />" // typename is mis-capitalized
        "    </ECClass>"
        "</ECSchema>";
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext);

    EXPECT_EQ(SchemaReadStatus::InvalidECSchemaXml, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, InvalidTypeNameInPrimitivePropertyConvertedToString)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Widgets' version='09.06' displayLabel='Widgets Display Label' description='Widgets Description' nameSpacePrefix='wid' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "<ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "    <ECClass typeName='ecProject' description='Project ECClass' displayLabel='Project' isDomainClass='True'>"
        "       <ECProperty propertyName='Name' typeName='strng' displayLabel='Project Name' />"
        "       <ECProperty propertyName='Date' typeName='banana' />"
        "    </ECClass>"
        "</ECSchema>";
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext);

    EXPECT_EQ(SchemaReadStatus::Success, status);

    ECClassP pClass = schema->GetClassP("ecProject");
    ECPropertyP pProperty = GetPropertyByName(*pClass, "Name");
    EXPECT_TRUE(PRIMITIVETYPE_String == pProperty->GetAsPrimitiveProperty()->GetType());
    pProperty = GetPropertyByName(*pClass, "Date");
    EXPECT_EQ(PRIMITIVETYPE_String, pProperty->GetAsPrimitiveProperty()->GetType());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWithEmptyCustomAttribute)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    //schemaContext->AddSchemaPath(L"C:\\temp\\data\\ECXA\\SchemasAndDgn\');
    //SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, L"C:\\temp\\data\\ECXA\\SchemasAndDgn\\Bentley_Plant.06.00.ecschema.xml", *schemaContext);
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"EmptyCustomAttribute.01.00.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);

    Utf8String ecSchemaXmlString;
    SchemaWriteStatus status2 = schema->WriteToXmlString(ecSchemaXmlString);
    EXPECT_EQ(SchemaWriteStatus::Success, status2);
    EXPECT_NE(Utf8String::npos, ecSchemaXmlString.find("<Relationship/>"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenDeserializingSchemaWithBaseClassInReferencedFile)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"SchemaThatReferences.01.00.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);

    ECClassP pClass = schema->GetClassP("circle");
    ASSERT_TRUE(nullptr != pClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenECSchemaContainsOnlyRequiredAttributes)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling(true, false);

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"OnlyRequiredECSchemaAttributes.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ(SchemaReadStatus::Success, status);
    EXPECT_STREQ("OnlyRequiredECSchemaAttributes", schema->GetName().c_str());
    EXPECT_STREQ("OnlyRequiredECSchemaAttributes", schema->GetAlias().c_str());
    EXPECT_STREQ("OnlyRequiredECSchemaAttributes", schema->GetDisplayLabel().c_str());
    EXPECT_FALSE(schema->GetIsDisplayLabelDefined());
    EXPECT_STREQ("", schema->GetDescription().c_str());
    EXPECT_EQ(1, schema->GetVersionRead());
    EXPECT_EQ(0, schema->GetVersionMinor());

    ECClassP pClass = schema->GetClassP("OnlyRequiredECClassAttributes");
    ASSERT_TRUE(nullptr != pClass);
    EXPECT_STREQ("OnlyRequiredECClassAttributes", pClass->GetName().c_str());
    EXPECT_STREQ("OnlyRequiredECClassAttributes", pClass->GetDisplayLabel().c_str());
    EXPECT_FALSE(pClass->GetIsDisplayLabelDefined());
    EXPECT_STREQ("", pClass->GetDescription().c_str());
    EXPECT_TRUE(pClass->IsEntityClass());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenDeserializingWidgetsECSchema)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"Widgets.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ(SchemaReadStatus::Success, status);
    VerifyWidgetsSchema(schema);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenDeserializingECSchemaFromString)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Widgets' version='09.06' displayLabel='Widgets Display Label' description='Widgets Description' nameSpacePrefix='wid' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECClass typeName='ecProject' description='Project ECClass' displayLabel='Project' isDomainClass='True'>"
        "       <ECProperty propertyName='Name' typeName='string' displayLabel='Project Name' />"
        "       <ECProperty propertyName='Geometry' typeName='Bentley.Geometry.Common.IGeometry' displayLabel='Geometry' />"
        "       <ECProperty propertyName='LineSegment' typeName='Bentley.Geometry.Common.ILineSegment' displayLabel='Line Segment' />"
        "    </ECClass>"
        "    <ECEnumeration typeName='Enumeration' backingTypeName='int' description='desc' displayLabel='dl'/>"
        "</ECSchema>";
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext);

    EXPECT_EQ(SchemaReadStatus::Success, status);
    EXPECT_STREQ("Widgets", schema->GetName().c_str());
    EXPECT_STREQ("wid", schema->GetAlias().c_str());
    EXPECT_STREQ("Widgets Display Label", schema->GetDisplayLabel().c_str());
    EXPECT_TRUE(schema->GetIsDisplayLabelDefined());
    EXPECT_STREQ("Widgets Description", schema->GetDescription().c_str());
    EXPECT_EQ(9, schema->GetVersionRead());
    EXPECT_EQ(6, schema->GetVersionMinor());

    ECClassP pClass = schema->GetClassP("ClassDoesNotExistInSchema");
    EXPECT_FALSE(pClass);

    pClass = schema->GetClassP("ecProject");
    ASSERT_TRUE(nullptr != pClass);
    EXPECT_STREQ("ecProject", pClass->GetName().c_str());
    EXPECT_STREQ("Project", pClass->GetDisplayLabel().c_str());
    EXPECT_TRUE(pClass->GetIsDisplayLabelDefined());
    EXPECT_STREQ("Project ECClass", pClass->GetDescription().c_str());
    EXPECT_TRUE(pClass->IsEntityClass());
    EXPECT_FALSE(pClass->HasBaseClasses());
    ECPropertyP pProperty = GetPropertyByName(*pClass, "Name");
    ASSERT_TRUE(nullptr != pProperty);
    EXPECT_STREQ("Name", pProperty->GetName().c_str());
    EXPECT_TRUE(pProperty->GetIsPrimitive());
    EXPECT_FALSE(pProperty->GetIsStruct());
    EXPECT_FALSE(pProperty->GetIsArray());
    EXPECT_STREQ("string", pProperty->GetTypeName().c_str());
    EXPECT_TRUE(PRIMITIVETYPE_String == pProperty->GetAsPrimitiveProperty()->GetType());
    EXPECT_TRUE(pProperty->GetIsDisplayLabelDefined());
    EXPECT_STREQ("Project Name", pProperty->GetDisplayLabel().c_str());
    EXPECT_STREQ("", pProperty->GetDescription().c_str());
    EXPECT_EQ(pClass, &pProperty->GetClass());
    EXPECT_FALSE(pProperty->GetIsReadOnly());

    pProperty = pClass->GetPropertyP("Geometry");
    ASSERT_TRUE(nullptr != pProperty);
    EXPECT_STREQ("Geometry", pProperty->GetName().c_str());
    EXPECT_TRUE(pProperty->GetIsPrimitive());
    EXPECT_FALSE(pProperty->GetIsStruct());
    EXPECT_FALSE(pProperty->GetIsArray());
    EXPECT_STREQ("Bentley.Geometry.Common.IGeometry", pProperty->GetTypeName().c_str());
    EXPECT_TRUE(PRIMITIVETYPE_IGeometry == pProperty->GetAsPrimitiveProperty()->GetType());
    EXPECT_TRUE(pProperty->GetIsDisplayLabelDefined());
    EXPECT_STREQ("Geometry", pProperty->GetDisplayLabel().c_str());
    EXPECT_STREQ("", pProperty->GetDescription().c_str());
    EXPECT_EQ(pClass, &pProperty->GetClass());
    EXPECT_FALSE(pProperty->GetIsReadOnly());

    pProperty = pClass->GetPropertyP("LineSegment");
    ASSERT_TRUE(nullptr != pProperty);
    EXPECT_STREQ("LineSegment", pProperty->GetName().c_str());
    EXPECT_TRUE(pProperty->GetIsPrimitive());
    EXPECT_FALSE(pProperty->GetIsStruct());
    EXPECT_FALSE(pProperty->GetIsArray());
    EXPECT_STREQ("Bentley.Geometry.Common.IGeometry", pProperty->GetTypeName().c_str());
    EXPECT_TRUE(PRIMITIVETYPE_IGeometry == pProperty->GetAsPrimitiveProperty()->GetType());
    EXPECT_TRUE(pProperty->GetIsDisplayLabelDefined());
    EXPECT_STREQ("Line Segment", pProperty->GetDisplayLabel().c_str());
    EXPECT_STREQ("", pProperty->GetDescription().c_str());
    EXPECT_EQ(pClass, &pProperty->GetClass());
    EXPECT_FALSE(pProperty->GetIsReadOnly());

    pProperty = GetPropertyByName(*pClass, "PropertyDoesNotExistInClass", false);
    EXPECT_FALSE(pProperty);

    ECEnumerationP pEnumeration = schema->GetEnumerationP("Enumeration");
    EXPECT_TRUE(pEnumeration != nullptr);
    EXPECT_STREQ("int", pEnumeration->GetTypeName().c_str());
    EXPECT_STREQ("desc", pEnumeration->GetDescription().c_str());
    EXPECT_STREQ("dl", pEnumeration->GetDisplayLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenRoundtripUsingString)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"Widgets.01.00.ecschema.xml").c_str(), *schemaContext);
    VerifyWidgetsSchema(schema);

    EXPECT_EQ(SchemaReadStatus::Success, status);

    Utf8String ecSchemaXmlString;

    SchemaWriteStatus status2 = schema->WriteToXmlString(ecSchemaXmlString);
    EXPECT_EQ(SchemaWriteStatus::Success, status2);

    ECSchemaPtr deserializedSchema;
    schemaContext = ECSchemaReadContext::CreateContext();
    status = ECSchema::ReadFromXmlString(deserializedSchema, ecSchemaXmlString.c_str(), *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);

    VerifyWidgetsSchema(deserializedSchema);
    }


void verifyKoqOnProperties (ECSchemaPtr schema)
    {
    auto testClass = schema->GetClassCP("TestClass");
    EXPECT_EQ(7, testClass->GetPropertyCount(true));
    for (auto const& prop : testClass->GetProperties(true))
        {
        auto koq = prop->GetKindOfQuantity();
        ASSERT_NE(nullptr, koq) << prop->GetName().c_str();
        EXPECT_STREQ("MyKoq", koq->GetName().c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, CanRoundTripKOQsOnPropertyTypes)
    {
    // NOTE: ECDb doesn't allow KOQs on Navigation properties and they have no use on them, but we have supported it for this long so I'm leaving it in.  Could safely be removed without breaking file format but might keep some existing schemas from loading.
    // ECDb supports KOQs on structs and struct arrays, there is little use for this but it was requested at some point.  There is a possibility that removing support would break some schema imported into an iModel
    // Primitive of all type, primitive array and enums of any backing primitive types are intentionally supported.
    Utf8CP schemaXML = R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECEnumeration typeName="MyEnum" backingTypeName="int" />
                    <KindOfQuantity typeName="MyKoq" relativeError="0.1" persistenceUnit="FT" />
                    <ECStructClass typeName="TestStruct" >
                        <ECProperty propertyName="Name" typeName="string"/>
                    </ECStructClass>
                    <ECEntityClass typeName="TestClass" >
                        <ECProperty propertyName="Prop1" typeName="double" kindOfQuantity="MyKoq" />
                        <ECProperty propertyName="Prop2" typeName="MyEnum" kindOfQuantity="MyKoq" />
                        <ECArrayProperty propertyName="Prop3" typeName="double" kindOfQuantity="MyKoq" />
                        <ECArrayProperty propertyName="Prop4" typeName="MyEnum" kindOfQuantity="MyKoq" />
                        <ECStructProperty propertyName="Prop5" typeName="TestStruct" kindOfQuantity="MyKoq" />
                        <ECStructArrayProperty propertyName="Prop6" typeName="TestStruct" kindOfQuantity="MyKoq" />
                        <ECNavigationProperty propertyName="NavProp" relationshipName="RelClass" direction="forward" kindOfQuantity="MyKoq" />
                    </ECEntityClass>
                    <ECRelationshipClass typeName="RelClass" strength="Referencing" modifier="None">
                        <Source multiplicity="(0..1)" roleLabel="source" polymorphic="true">
                            <Class class="TestClass"/>
                        </Source>
                        <Target multiplicity="(1..1)" roleLabel="target" polymorphic="true">
                            <Class class="TestClass"/>
                        </Target>
                    </ECRelationshipClass>
                </ECSchema>)xml";
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext);

    EXPECT_EQ(SchemaReadStatus::Success, status);

    verifyKoqOnProperties(schema);

    Utf8String ecSchemaXmlString;

    SchemaWriteStatus status2 = schema->WriteToXmlString(ecSchemaXmlString);
    EXPECT_EQ(SchemaWriteStatus::Success, status2);

    ECSchemaPtr deserializedSchema;
    schemaContext = ECSchemaReadContext::CreateContext();
    status = ECSchema::ReadFromXmlString(deserializedSchema, ecSchemaXmlString.c_str(), *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);

    verifyKoqOnProperties(deserializedSchema);

    Utf8String jsonSchema;
    EXPECT_TRUE(deserializedSchema->WriteToJsonString(jsonSchema));
    std::regex koqFinder("\"kindOfQuantity\" *: *\"TestSchema.MyKoq\"");
    auto counter = std::sregex_iterator(jsonSchema.begin(), jsonSchema.end(), koqFinder);
    EXPECT_EQ(7, std::distance(counter, std::sregex_iterator()));
    }

template <typename INamedItems>
void expectNoDisplayLabels(ECSchemaCP schema, INamedItems const & (ECSchema::*getItems)() const)
    {
    for (const auto& item : (schema->*getItems)())
        EXPECT_FALSE(item->GetIsDisplayLabelDefined()) << item->GetName().c_str();
    }

void verifyNoExplicitDisplayLabels(ECSchemaCP schema)
    {
    EXPECT_FALSE(schema->GetIsDisplayLabelDefined());
    expectNoDisplayLabels(schema, &ECSchema::GetEnumerations);
    expectNoDisplayLabels(schema, &ECSchema::GetKindOfQuantities);
    expectNoDisplayLabels(schema, &ECSchema::GetUnitSystems);
    expectNoDisplayLabels(schema, &ECSchema::GetPhenomena);
    expectNoDisplayLabels(schema, &ECSchema::GetUnits);
    expectNoDisplayLabels(schema, &ECSchema::GetFormats);
    expectNoDisplayLabels(schema, &ECSchema::GetPropertyCategories);
    expectNoDisplayLabels(schema, &ECSchema::GetClasses);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, RoundTripUnsetDisplayLabels)
    {
    Utf8CP schemaXML = R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECEnumeration typeName="MyEnum" backingTypeName="int" />
                    <KindOfQuantity typeName="MyKoq" relativeError="0.1" persistenceUnit="FT" />
                    <UnitSystem typeName="MySystem" />
                    <Phenomenon typeName="MyPhen" definition="MyPhen" />
                    <Unit typeName="MyUnit" definition="MyUnit" phenomenon="MyPhen" unitSystem="MySystem" />
                    <Format typeName="MyFormat" type="decimal" precision="4" />
                    <PropertyCategory typeName="MyCat" priority="42" />
                    <ECStructClass typeName="TestStruct" >
                        <ECProperty propertyName="Name" typeName="string"/>
                    </ECStructClass>
                    <ECEntityClass typeName="TestClass" >
                        <ECCustomAttributes>
                            <TestCA />
                        </ECCustomAttributes>
                        <ECProperty propertyName="Prop1" typeName="double" kindOfQuantity="MyKoq" />
                        <ECStructProperty propertyName="Prop5" typeName="TestStruct" kindOfQuantity="MyKoq" />
                    </ECEntityClass>
                    <ECCustomAttributeClass typeName="TestCA" />
                    <ECRelationshipClass typeName="RelClass" strength="Referencing" modifier="None">
                        <Source multiplicity="(0..1)" roleLabel="source" polymorphic="true">
                            <Class class="TestClass"/>
                        </Source>
                        <Target multiplicity="(1..1)" roleLabel="target" polymorphic="true">
                            <Class class="TestClass"/>
                        </Target>
                    </ECRelationshipClass>
                </ECSchema>)xml";
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext);

    EXPECT_EQ(SchemaReadStatus::Success, status);

    verifyNoExplicitDisplayLabels(schema.get());

    Utf8String ecSchemaXmlString;

    SchemaWriteStatus status2 = schema->WriteToXmlString(ecSchemaXmlString);
    EXPECT_EQ(SchemaWriteStatus::Success, status2);

    ECSchemaPtr deserializedSchema;
    schemaContext = ECSchemaReadContext::CreateContext();
    status = ECSchema::ReadFromXmlString(deserializedSchema, ecSchemaXmlString.c_str(), *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);

    verifyNoExplicitDisplayLabels(deserializedSchema.get());

    Utf8String jsonSchema;
    EXPECT_TRUE(deserializedSchema->WriteToJsonString(jsonSchema));
    std::regex labelFinder("label");
    auto counter = std::sregex_iterator(jsonSchema.begin(), jsonSchema.end(), labelFinder);
    EXPECT_EQ(0, std::distance(counter, std::sregex_iterator()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWithDuplicateClassesInECXml2)
    {
    Utf8CP widgets_schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Widgets' version='09.06' displayLabel='Widgets Display Label' description='Widgets Description' nameSpacePrefix='wid' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECClass typeName='DifferentClass' isDomainClass='True'>"
        "    </ECClass>"
        "    <ECClass typeName='ecProject' description='Project ECClass' displayLabel='Project' isDomainClass='True'>"
        "       <ECProperty propertyName='Name' typeName='string' displayLabel='Project Name' />"
        "    </ECClass>"
        "    <ECClass typeName='ecProject' isDomainClass='True'>"
        "    </ECClass>"
        "</ECSchema>";
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, widgets_schemaXML, *schemaContext);

    EXPECT_EQ(SchemaReadStatus::Success, status);

    ECSchemaPtr schema2;
    ECSchemaReadContextPtr   schemaContext2 = ECSchemaReadContext::CreateContext();

    Utf8CP widgets2_schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Widgets2' version='09.06' displayLabel='Widgets Display Label' description='Widgets Description' nameSpacePrefix='wid' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECClass typeName='ecProject' description='Project ECClass' displayLabel='Project' isDomainClass='True'>"
        "       <ECProperty propertyName='Name' typeName='string' displayLabel='Project Name' />"
        "    </ECClass>"
        "    <ECClass typeName='ecProject' description='New Project ECClass' isDomainClass='True'>"
        "       <ECProperty propertyName='Author' typeName='string' displayLabel='Project Name' />"
        "    </ECClass>"
        "</ECSchema>";
    status = ECSchema::ReadFromXmlString(schema2, widgets2_schemaXML, *schemaContext2);

    EXPECT_EQ(SchemaReadStatus::Success, status);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, ExpectFailureWithDuplicateClassesInECXml3)
    {
    Utf8CP widgets_schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Widgets' version='09.06' displayLabel='Widgets Display Label' description='Widgets Description' nameSpacePrefix='wid' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEntityClass typeName='DifferentClass'>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='ecProject' description='Project ECClass' displayLabel='Project'>"
        "       <ECProperty propertyName='Name' typeName='string' displayLabel='Project Name' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='ecProject'>"
        "    </ECEntityClass>"
        "</ECSchema>";
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, widgets_schemaXML, *schemaContext);

    EXPECT_EQ(SchemaReadStatus::DuplicateTypeName, status);

    ECSchemaPtr schema2;
    ECSchemaReadContextPtr   schemaContext2 = ECSchemaReadContext::CreateContext();

    Utf8CP widgets2_schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Widgets2' version='09.06' displayLabel='Widgets Display Label' description='Widgets Description' nameSpacePrefix='wid' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEntityClass typeName='ecProject' description='Project ECClass' displayLabel='Project' isDomainClass='True'>"
        "       <ECProperty propertyName='Name' typeName='string' displayLabel='Project Name' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='ecProject' description='New Project ECClass' isDomainClass='True'>"
        "       <ECProperty propertyName='Author' typeName='string' displayLabel='Project Name' />"
        "    </ECEntityClass>"
        "</ECSchema>";
    status = ECSchema::ReadFromXmlString(schema2, widgets2_schemaXML, *schemaContext2);

    EXPECT_EQ(SchemaReadStatus::DuplicateTypeName, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, EnsureSupplementalSchemaCannotHaveBaseClasses)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0' schemaName='SupplementalSchemaWithBaseClasses_Supplemental_Mapping' nameSpacePrefix='ss' version='01.00' description='Test Supplemental Mapping Schema' displayLabel='Electrical Extended Supplemental Mapping' xmlns:ec='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "<ECClass typeName='MAPPING' displayLabel='Mapping' isStruct='false' isDomainClass='true' isCustomAttributeClass='false'/>"
        "<ECClass typeName='ELECTRICAL_PROPERTY_MAPPING' displayLabel='Electrical Property Mapping' isStruct='false' isDomainClass='false' isCustomAttributeClass='true'>"
        "<BaseClass>MAPPING</BaseClass>"
        "<ECProperty propertyName='APPLICATION_PROPERTY_NAME' typeName='string' displayLabel='Application Property Name' readOnly='false'/>"
        "</ECClass>"
        "<ECClass typeName='ELECTRICAL_ITEM' displayLabel='Electrical Item' isStruct='false' isDomainClass='true' isCustomAttributeClass='false'>"
        "<BaseClass>bentley:BENTLEY_BASE_OBJECT</BaseClass>"
        "<ECProperty propertyName='ID' typeName='string' description='Business ID for an electrical item.' readOnly='false'>"
        "<ECCustomAttributes>"
        "<ELECTRICAL_PROPERTY_MAPPING xmlns='ElectricalExtended_Supplemental_Mapping.01.00'>"
        "<APPLICATION_PROPERTY_NAME>DeviceID</APPLICATION_PROPERTY_NAME>"
        "</ELECTRICAL_PROPERTY_MAPPING>"
        "</ECCustomAttributes>"
        "</ECProperty>"
        "</ECClass>"
        "</ECSchema>";
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext);

    EXPECT_EQ(SchemaReadStatus::Success, status);
    Utf8CP className = "ELECTRICAL_ITEM";
    ECClassCP ecClass = schema->GetClassCP(className);

    const ECBaseClassesList& baseClassList = ecClass->GetBaseClasses();
    EXPECT_EQ(0, baseClassList.size()) << "Class " << className << " should not have any base classes since it is in a supplemental schema.";

    className = "ELECTRICAL_PROPERTY_MAPPING";
    ecClass = schema->GetClassCP(className);

    const ECBaseClassesList& baseClassList2 = ecClass->GetBaseClasses();
    EXPECT_EQ(0, baseClassList2.size()) << "Class " << className << " should not have any base classes since it is in a supplemental schema.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenSerializingToFile)
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"Widgets.01.00.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ(status, SchemaReadStatus::Success);
    VerifyWidgetsSchema(schema);

    SchemaWriteStatus status2 = schema->WriteToXmlFile(ECTestFixture::GetTempDataPath(L"test.xml").c_str());
    EXPECT_EQ(SchemaWriteStatus::Success, status2);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenReferencingNonExistingUnitAttributesSchema)
    {
    Utf8CP schemaXML = "<ECSchema schemaName=\"ReferencesUnits\" nameSpacePrefix=\"ru\" Description=\"Schema that references the unavailable U_A.1.1 schema\" "
        "version=\"1.1\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "    <ECSchemaReference name=\"Unit_Attributes\" version=\"01.01\" prefix=\"ua\" />"
        "</ECSchema>";

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectErrorWhenBaseClassNotFound)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='ReferencedSchema' version='01.00' displayLabel='Display Label' description='Description' nameSpacePrefix='ref' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECClass typeName='BaseClass' description='Project ECClass' displayLabel='Project' isDomainClass='True' />"
        "</ECSchema>";
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr refSchema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(refSchema, schemaXML, *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);

    schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Stuff' version='09.06' displayLabel='Display Label' description='Description' nameSpacePrefix='stuff' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "<ECSchemaReference name='ReferencedSchema' version='01.00' prefix='ref' />"
        "    <ECClass typeName='ecProject' description='Project ECClass' displayLabel='Project' isDomainClass='True'>"
        "       <BaseClass>BaseClassDOESNOTEXIST</BaseClass>"
        "       <BaseClass>ref:BaseClass</BaseClass>"
        "    </ECClass>"
        "</ECSchema>";
    ECSchemaPtr schema;
    status = ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext);

    EXPECT_NE(SchemaReadStatus::Success, status);
    EXPECT_TRUE(schema.IsNull());
    }



void ValidateElementOrder(bvector<Utf8String> expectedTypeNames, BeXmlNodeP root)
    {
    BeXmlNodeP currentNode = root->GetFirstChild();
    for(auto expectedTypeName : expectedTypeNames)
        {
        if(currentNode == nullptr)
            {
            FAIL() << "Reached end of document, Node '" << expectedTypeName << "' expected.";
            }

        Utf8String nodeTypeName;
        EXPECT_EQ(BeXmlStatus::BEXML_Success, currentNode->GetAttributeStringValue(nodeTypeName, "typeName"));
        EXPECT_EQ(expectedTypeName, nodeTypeName);

        currentNode = currentNode->GetNextSibling();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, TestPreservingElementOrder)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    schemaContext->SetPreserveElementOrder(true);

    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='TestSchema' version='01.00' nameSpacePrefix='ab' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEntityClass typeName=\"GHI\" description=\"Project ECClass\" displayLabel=\"Class GHI\"></ECEntityClass>"
        "    <ECEntityClass typeName=\"ABC\" description=\"Project ECClass\" displayLabel=\"Class ABC\"></ECEntityClass>"
        "    <ECEnumeration typeName=\"DEF\" displayLabel=\"Enumeration DEF\" backingTypeName=\"int\" />"
        "</ECSchema>";
    ECSchemaPtr schema;
    SchemaReadStatus schemaReadingStatus = ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, schemaReadingStatus);

    Utf8String ecSchemaXmlString;
    SchemaWriteStatus schemaWritingStatus = schema->WriteToXmlString(ecSchemaXmlString, ECVersion::V3_0);
    EXPECT_EQ(SchemaWriteStatus::Success, schemaWritingStatus);

    size_t stringByteCount = ecSchemaXmlString.length() * sizeof(Utf8Char);
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromString(xmlStatus, ecSchemaXmlString.c_str(), stringByteCount);
    EXPECT_EQ(BEXML_Success, xmlStatus);

    bvector<Utf8String> typeNames = {"GHI", "ABC", "DEF"};
    ValidateElementOrder(typeNames, xmlDom.get()->GetRootElement());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, TestDefaultElementOrder)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='TestSchema' version='01.00' nameSpacePrefix='ab' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEntityClass typeName=\"GHI\" description=\"Project ECClass\" displayLabel=\"Class GHI\"></ECEntityClass>"
        "    <ECEntityClass typeName=\"ABC\" description=\"Project ECClass\" displayLabel=\"Class ABC\"></ECEntityClass>"
        "    <ECEnumeration typeName=\"DEF\" displayLabel=\"Enumeration DEF\" backingTypeName=\"int\" />"
        "</ECSchema>";
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext));

    WString ecSchemaXmlString;
    ASSERT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(ecSchemaXmlString, ECVersion::V3_0));

    size_t stringByteCount = ecSchemaXmlString.length() * sizeof(Utf8Char);
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromString(xmlStatus, ecSchemaXmlString.c_str(), stringByteCount);
    ASSERT_EQ(BEXML_Success, xmlStatus);

    // Enumerations(DEF) are serialized first, then classes(ABC, GHI)
    bvector<Utf8String> typeNames = {"DEF", "ABC", "GHI"};
    ValidateElementOrder(typeNames, xmlDom.get()->GetRootElement());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, TestPreserveElementOrderWithBaseClassAndRelationships)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    schemaContext->SetPreserveElementOrder(true);

    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='TestSchema' version='01.00' nameSpacePrefix='ab' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEntityClass typeName=\"GHI\" description=\"Project ECClass\" displayLabel=\"Class GHI\"></ECEntityClass>"
        "    <ECEntityClass typeName=\"ABC\" description=\"Project ECClass\" displayLabel=\"Class ABC\">"
        "      <BaseClass>MNO</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName=\"PQR\" isDomainClass=\"True\" strength=\"referencing\" strengthDirection=\"forward\">"
        "      <Source cardinality = \"(0, 1)\" polymorphic = \"True\">"
        "          <Class class = \"MNO\" />"
        "      </Source>"
        "      <Target cardinality = '(0, 1)' polymorphic = 'True'>"
        "          <Class class = 'JKL'>"
        "              <Key>"
        "                  <Property name = 'Property1' />"
        "                  <Property name = 'Property2' />"
        "              </Key>"
        "          </Class>"
        "      </Target>"
        "    </ECRelationshipClass>"
        "    <ECEntityClass typeName = \"MNO\" description=\"Project ECClass\" displayLabel=\"Class MNO\"></ECEntityClass>"
        "    <ECEntityClass typeName = \"JKL\" description=\"Project ECClass\" displayLabel=\"Class JKL\">"
        "      <ECProperty propertyName=\"Property1\" typeName=\"string\" />"
        "      <ECProperty propertyName=\"Property2\" typeName=\"string\" />"
        "    </ECEntityClass>"
        "    <ECEnumeration typeName=\"DEF\" displayLabel=\"Enumeration PQR\" backingTypeName=\"int\" />"
        "</ECSchema>";
    ECSchemaPtr schema;
    EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext));

    WString ecSchemaXmlString;
    EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(ecSchemaXmlString, ECVersion::V3_0));

    size_t stringByteCount = ecSchemaXmlString.length() * sizeof(Utf8Char);
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromString(xmlStatus, ecSchemaXmlString.c_str(), stringByteCount);
    EXPECT_EQ(BEXML_Success, xmlStatus);

    // Expecting the same order as specified in the SchemaXML Document.
    bvector<Utf8String> typeNames = {"GHI","ABC","PQR", "MNO", "JKL", "DEF",};
    ValidateElementOrder(typeNames, xmlDom.get()->GetRootElement());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, TestDefaultElementOrderWithBaseClassAndRelationships)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='TestSchema' version='01.00' nameSpacePrefix='ab' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEntityClass typeName=\"GHI\" description=\"Project ECClass\" displayLabel=\"Class GHI\"></ECEntityClass>"
        "    <ECEntityClass typeName=\"ABC\" description=\"Project ECClass\" displayLabel=\"Class ABC\">"
        "      <BaseClass>MNO</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName=\"DEF\" isDomainClass=\"True\" strength=\"referencing\" strengthDirection=\"forward\">"
        "      <Source cardinality = \"(0, 1)\" polymorphic = \"True\">"
        "          <Class class = \"MNO\" />"
        "      </Source>"
        "      <Target cardinality = '(0, 1)' polymorphic = 'True'>"
        "          <Class class = 'JKL'>"
        "              <Key>"
        "                  <Property name = 'Property1' />"
        "                  <Property name = 'Property2' />"
        "              </Key>"
        "          </Class>"
        "      </Target>"
        "    </ECRelationshipClass>"
        "    <ECEntityClass typeName = \"MNO\" description=\"Project ECClass\" displayLabel=\"Class MNO\"></ECEntityClass>"
        "    <ECEntityClass typeName = \"JKL\" description=\"Project ECClass\" displayLabel=\"Class JKL\">"
        "      <ECProperty propertyName=\"Property1\" typeName=\"string\" />"
        "      <ECProperty propertyName=\"Property2\" typeName=\"string\" />"
        "    </ECEntityClass>"
        "    <ECEnumeration typeName=\"PQR\" displayLabel=\"Enumeration PQR\" backingTypeName=\"int\" />"
        "</ECSchema>";
    ECSchemaPtr schema;
    EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext));

    WString ecSchemaXmlString;
    EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(ecSchemaXmlString, ECVersion::V3_0));

    size_t stringByteCount = ecSchemaXmlString.length() * sizeof(Utf8Char);
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromString(xmlStatus, ecSchemaXmlString.c_str(), stringByteCount);
    EXPECT_EQ(BEXML_Success, xmlStatus);

    // First Enumeration(PQR), then classes alphabetically(ABC, DEF, GHI). As MNO is the base class of ABC and
    // JKL has a constraint in DEF, those two classes are written before the class they depend in.
    bvector<Utf8String> typeNames = {"PQR", "MNO", "ABC", "JKL", "DEF", "GHI"};
    ValidateElementOrder(typeNames, xmlDom.get()->GetRootElement());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, ExpectInvalidSchemaAndDuplicateStatusWhenLoadingSchemaFromXmlASecondTime)
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    WString schemaFilePath = ECTestFixture::GetTestDataPath(L"Widgets.01.00.ecschema.xml");
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, schemaFilePath.c_str(), *schemaContext);
    EXPECT_EQ(status, SchemaReadStatus::Success);
    VerifyWidgetsSchema(schema);

    ECSchemaPtr schema2;
    status = ECSchema::ReadFromXmlFile(schema2, schemaFilePath.c_str(), *schemaContext);
    EXPECT_EQ(SchemaReadStatus::DuplicateSchema, status) << "Getting the schema a second time did not return a status of DuplicateSchema";
    EXPECT_TRUE(schema2.IsNull()) << "Expected schema to be nullptr on second call to ECSchema::ReadFromXmlFile";

    SchemaKey key(schema->GetSchemaKey());
    ECSchemaPtr schema3 = schemaContext->LocateSchema(key, SchemaMatchType::Exact);
    EXPECT_TRUE(schema3.IsValid()) << "Expected a valid schema when calling ECSchemaReadContext::LocateSchema after schema was loaded using ECSchema::ReadFromXmlFile";
    VerifyWidgetsSchema(schema3);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, ExpectFailureWhenDerivedPropertyDifferByCaseECXml3)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A' modifier='abstract'>"
        "       <ECProperty propertyName='Prop' typeName='string'></ECProperty>"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='B' modifier='sealed'>"
        "       <BaseClass>A</BaseClass>"
        "       <ECProperty propertyName='PROP' typeName='string'></ECProperty>"
        "   </ECEntityClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenDerivedPropertyDifferByCaseECXml2)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema2' version='01.00' nameSpacePrefix='ts2' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECClass typeName='A' isDomainClass='true'>"
        "       <ECProperty propertyName='Prop' typeName='string'></ECProperty>"
        "   </ECClass>"
        "   <ECClass typeName='B' isDomainClass='true'>"
        "       <BaseClass>A</BaseClass>"
        "       <ECProperty propertyName='PROP' typeName='string'></ECProperty>"
        "   </ECClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, ExpectFailureWhenAliasNotFoundOrEmpty)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();

    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "</ECSchema>";

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "Schema with no alias attribute was supposed to fail to deserialize.";

    Utf8CP schemaXml2 = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "</ECSchema>";

    ECSchemaPtr schema2;
    status = ECSchema::ReadFromXmlString(schema2, schemaXml2, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "Schema with empty alias attribute was supposed to fail to deserialize.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, CanLoadCaInstanceWhichAppearsBeforeCaDefinition)
    {
    Utf8CP schemaXml = R"xml(<?xml version='1.0' encoding='UTF-8'?>
        <ECSchema schemaName='testSchema' version='01.00.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>

            <ECEntityClass typeName='MyEntity'>
                <ECCustomAttributes>
                    <SillyCA xmlns='testSchema.01.00'>
                        <SillyDouble>42</SillyDouble>
                        <SillyStruct>
                            <SillyStructDouble>35</SillyStructDouble>
                        </SillyStruct>
                    </SillyCA>
                </ECCustomAttributes>
            </ECEntityClass>

            <ECCustomAttributeClass typeName='SillyCA'>
                <ECProperty propertyName='SillyDouble' typeName='double'>
                    <ECCustomAttributes>
                        <DoubleSillyCA>
                            <DoubleSillyDouble>49</DoubleSillyDouble>
                        </DoubleSillyCA>
                    </ECCustomAttributes>
                </ECProperty>
                <ECStructProperty propertyName='SillyStruct' typeName='SillyStruct' />
            </ECCustomAttributeClass>

            <ECStructClass typeName='SillyStruct'>
                <BaseClass>SillyBaseStruct</BaseClass>
                <ECProperty propertyName='SillyStructDouble' typeName='double' />
            </ECStructClass>

            <ECCustomAttributeClass typeName='DoubleSillyCA'>
                <ECProperty propertyName='DoubleSillyDouble' typeName='double' />
            </ECCustomAttributeClass>

            <ECStructClass typeName='SillyBaseStruct'>
                <ECProperty propertyName='SillyBaseStructDouble' typeName='double' />
            </ECStructClass>
        </ECSchema>)xml";

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status) << "Failed to load schema";

    ECClassCP myEntity = schema->GetClassCP("MyEntity");
    ASSERT_NE(nullptr, myEntity);

    IECInstancePtr sillyCA = myEntity->GetCustomAttribute("testSchema", "SillyCA");
    ASSERT_TRUE(sillyCA.IsValid());
    ECValue sillyDouble;
    EXPECT_EQ(ECObjectsStatus::Success, sillyCA->GetValue(sillyDouble, "SillyDouble"));
    EXPECT_FALSE(sillyDouble.IsNull());
    EXPECT_EQ(42, sillyDouble.GetDouble());

    ECValue sillyStructDouble;
    EXPECT_EQ(ECObjectsStatus::Success, sillyCA->GetValue(sillyStructDouble, "SillyStruct.SillyStructDouble"));
    EXPECT_FALSE(sillyStructDouble.IsNull());
    EXPECT_EQ(35, sillyStructDouble.GetDouble());

    ECClassCP sillyCAClass = schema->GetClassCP("SillyCA");
    ASSERT_NE(nullptr, sillyCAClass);
    ECPropertyCP sillyDoubleProp = sillyCAClass->GetPropertyP("SillyDouble");
    ASSERT_NE(nullptr, sillyDoubleProp);

    IECInstancePtr doubleSillyCA = sillyDoubleProp->GetCustomAttribute("testSchema", "DoubleSillyCA");
    ASSERT_TRUE(doubleSillyCA.IsValid());
    ECValue doubleSillyDouble;
    EXPECT_EQ(ECObjectsStatus::Success, doubleSillyCA->GetValue(doubleSillyDouble, "DoubleSillyDouble"));
    EXPECT_EQ(49, doubleSillyDouble.GetDouble());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//This test ensures we support any unknown element or attribute put into existing ECSchema XML. Important for backwards compatibility of future EC versions.
TEST_F(SchemaDeserializationTest, DeserializeComprehensiveSchemaWithUnknowns)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"ComprehensiveSchemaWithUnknowns.01.00.ecschema.xml").c_str(), *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status) << "Failed to load ComprehensiveSchemaWithUnknowns for test";

    EXPECT_EQ(6, schema->GetClassCount());
    EXPECT_EQ(1, schema->GetEnumerationCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, SelfReferencingStructArray)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="testSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECClass typeName="Foo" description="Used as data type" isStruct="True" isDomainClass="False">
                    <ECProperty propertyName="Name" typeName="string" />
                    <ECProperty propertyName="AreaType" typeName="string" />
                    <ECProperty propertyName="Area" typeName="double" description="in [m2]" />
                    <ECProperty propertyName="LuminaireCategory" typeName="int" />
                    <ECArrayProperty propertyName="Luminaires" typeName="Foo" minOccurs="0" maxOccurs="unbounded" isStruct="True" />
                </ECClass>
            </ECSchema>
        )xml";

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status) << "Failed to load schema";

    ECClassCP foo = schema->GetClassCP("Foo");
    ASSERT_NE(nullptr, foo);

    ASSERT_EQ(4, foo->GetPropertyCount());
    ECPropertyP bad = foo->GetPropertyP("Luminaires");
    ASSERT_EQ(nullptr, bad);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(SchemaDeserializationTest, ChecksumIsCalculatedFromContext)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        </ECSchema>
        )xml";

    {
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->SetCalculateChecksum(true);
    ASSERT_TRUE(context->GetCalculateChecksum()) << "The calculate checksum flag should be turned on.";

    ECSchemaPtr testSchema;
    DeserializeSchema(testSchema, *context, SchemaItem(schemaXml));

    ASSERT_TRUE(testSchema.IsValid());
    EXPECT_FALSE(testSchema->GetSchemaKey().m_checksum.empty()) << "Expect the checksum of the schema to be valid when the calculate checksum flag is set to true on the ECSchemaReadContext.";
    }
    {
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_FALSE(context->GetCalculateChecksum()) << "By default the calculate checksum flag should be off";
    ECSchemaPtr testSchema;
    DeserializeSchema(testSchema, *context, SchemaItem(schemaXml));
    ASSERT_TRUE(testSchema.IsValid());
    EXPECT_TRUE(testSchema->GetSchemaKey().m_checksum.empty()) << "Expect the checksum of the schema to be empty when the calculate checksum flag is false on the ECSchemaReadContext.";
    }
    {
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->SetSkipValidation(true);
    context->SetCalculateChecksum(true);

    ECSchemaPtr testSchema;
    DeserializeSchema(testSchema, *context, SchemaItem(schemaXml));

    ASSERT_TRUE(testSchema.IsValid());
    EXPECT_FALSE(testSchema->GetSchemaKey().m_checksum.empty()) << "Expect the checksum of the schema to be valid when the calculate checksum and skip validation flags are set to true on the ECSchemaReadContext.";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, CaseConflictInMultipleBaseClasses)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Test" nameSpacePrefix="ts" version="01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECClass typeName="ClassA" isDomainClass="True">
                    <ECProperty propertyName="foo" typeName="string" />
                </ECClass>
                <ECClass typeName="ClassB" isDomainClass="True">
                    <ECProperty propertyName="Foo" typeName="string" />
                </ECClass>
                <ECClass typeName="ClassC" isDomainClass="True">
                    <ECProperty propertyName="Foo" typeName="string" />
                </ECClass>
                <ECClass typeName="ClassD" isDomainClass="True">
                    <BaseClass>ClassB</BaseClass>
                    <BaseClass>ClassC</BaseClass>
                    <BaseClass>ClassA</BaseClass>
                </ECClass>
            </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->SetResolveConflicts(true);
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ECPropertyP propA = schema->GetClassCP("ClassA")->GetPropertyP("foo", false);
    ECPropertyP propB = schema->GetClassCP("ClassB")->GetPropertyP("foo", false);
    ECPropertyP propC = schema->GetClassCP("ClassC")->GetPropertyP("foo", false);

    // Ensure all properties are now the same case
    ASSERT_TRUE(propA->GetName().Equals(propB->GetName()));
    ASSERT_TRUE(propA->GetName().Equals(propC->GetName()));
    ASSERT_TRUE(propC->GetName().Equals(propB->GetName()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, PruneSchemas)
    {
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->GetSchemasToPrune() = bvector<Utf8String>{"BaseElementSchema"};
    context->SetResolveConflicts(true);
    ECSchemaPtr schema, refSchema;

    Utf8CP refSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="ValidRefSchema" nameSpacePrefix="rs" version="01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECClass typeName="TestCustomAttr3" isDomainClass="False" isCustomAttributeClass="True">
                    <ECProperty propertyName="A" typeName="boolean" />
                </ECClass>
                <ECClass typeName="TestCustomAttr6" isDomainClass="False" isCustomAttributeClass="True">
                    <ECProperty propertyName="B" typeName="boolean" />
                </ECClass>
            </ECSchema>)xml";

    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(refSchema, refSchemaXml, *context));
    ASSERT_TRUE(refSchema.IsValid());

    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Test" nameSpacePrefix="ts" version="01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECSchemaReference name="BaseElementSchema" version="01.00" prefix="baseElem" />
                <ECSchemaReference name="ValidRefSchema" version="01.00" prefix="rs" />
                <ECCustomAttributes>
                    <TestCustomAttr1 xmlns="BaseElementSchema.01.00" />
                    <TestCustomAttr2 xmlns="NonExistentRefSchema.01.00" />
                    <TestCustomAttr3 xmlns="ValidRefSchema.01.00" />
                </ECCustomAttributes>
                <ECClass typeName="Foo" isDomainClass="True">
                    <ECProperty propertyName="Bar" typeName="string" />
                    <ECProperty propertyName="New" typeName="string" />
                    <ECStructProperty propertyName="Params" typeName="baseElem:MaterialProjectionClass" displayLabel="parameters" />
                    <ECArrayProperty propertyName="TestStructArray" typeName="baseElem:MaterialProjectionClass" isStruct="True" />
                    <ECArrayProperty propertyName="TestPrimitiveArray" typeName="baseElem:EnumExample" />
                    <ECProperty propertyName="ToPruneRef" typeName="baseElem:EnumExample" />
                    <ECProperty propertyName="NonExistentRef" typeName="nonExistentRef:EnumExample2" />
                    <ECCustomAttributes>
                        <TestCustomAttr4 xmlns="BaseElementSchema.01.00" />
                        <TestCustomAttr5 xmlns="NonExistentRefSchema.01.00" />
                        <TestCustomAttr6 xmlns="ValidRefSchema.01.00" />
                    </ECCustomAttributes>
                </ECClass>
                <ECRelationshipClass typeName="RelationshipWithBadConstraint" isDomainClass="True" strength="referencing" strengthDirection="forward">
                    <Source cardinality="(0,N)" polymorphic="true">
                        <Class class="foo" />
                    </Source>
                    <Target cardinality="(0,1)" polymorphic="true">
                        <Class class="baseElem:MstnGraphHeader" />
                    </Target>
                </ECRelationshipClass>
            </ECSchema>)xml";

    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());

    auto const& refedSchemas = schema->GetReferencedSchemas();
    EXPECT_TRUE(refedSchemas.end() == std::find_if(refedSchemas.begin(), refedSchemas.end(), [&](const auto& s) { return s.second->GetName().EqualsIAscii("BaseElementSchema");}))
        << "invalid schema should no longer be referenced";
    ASSERT_TRUE(nullptr != schema->GetClassCP("RelationshipWithBadConstraint"))
        << "baseElem-referencing constraint should have been pruned";
    EXPECT_EQ(nullptr, schema->GetClassCP("Foo")->GetPropertyP("Params"))
        << "baseElem-referencing struct property should have been pruned";
    EXPECT_EQ(nullptr, schema->GetClassCP("Foo")->GetPropertyP("TestPrimitiveArray"))
        << "baseElem-referencing primitive array property should have been pruned";
    EXPECT_EQ(nullptr, schema->GetClassCP("Foo")->GetPropertyP("TestStructArray"))
        << "baseElem-referencing struct array property should have been pruned";
    EXPECT_EQ(nullptr, schema->GetClassCP("Foo")->GetPropertyP("ToPruneRef"))
        << "baseElem-referencing primitive property should have been pruned";
    EXPECT_NE(nullptr, schema->GetClassCP("Foo")->GetPropertyP("NonExistentRef"))
        << "invalid reference property should remain as string";

    EXPECT_FALSE(schema->GetCustomAttributeContainer().GetCustomAttribute("ValidRefSchema", "TestCustomAttr1").IsValid())
        << "custom attribute referencing baseElem should be pruned";
    EXPECT_FALSE(schema->GetCustomAttributeContainer().GetCustomAttribute("ValidRefSchema", "TestCustomAttr2").IsValid())
        << "custom attribute referencing non existent schema should be pruned";
    EXPECT_TRUE(schema->GetCustomAttributeContainer().GetCustomAttribute("ValidRefSchema", "TestCustomAttr3").IsValid())
        << "custom attribute referencing valid schema should be fine";

    EXPECT_FALSE(schema->GetClassCP("Foo")->IsDefined("ValidRefSchema", "TestCustomAttr4)"))
        << "custom attribute referencing baseElem should be pruned";
    EXPECT_FALSE(schema->GetClassCP("Foo")->IsDefined("ValidRefSchema", "TestCustomAttr5"))
        << "custom attribute referencing non existent schema should be pruned";
    EXPECT_TRUE(schema->GetClassCP("Foo")->IsDefined("ValidRefSchema", "TestCustomAttr6"))
        << "custom attribute referencing valid schema should be fine";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, AliasesForPruneSchemasAreResetForEachSchema)
    {
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->GetSchemasToPrune() = bvector<Utf8String>{"BadRef"};
    context->SetResolveConflicts(true);

    Utf8CP refSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Ref" nameSpacePrefix="rs" version="01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECClass typeName="TestStruct" isDomainClass="False" isStruct="True">
                    <ECProperty propertyName="A" typeName="boolean" />
                </ECClass>
            </ECSchema>)xml";

    Utf8CP otherSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="OtherSchema" nameSpacePrefix="os" version="01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECSchemaReference name="Ref" version="01.00" prefix="rs" />
                <ECClass typeName="SecondStruct" isDomainClass="False" isStruct="True">
                    <ECStructProperty propertyName="A" typeName="rs:TestStruct" />
                </ECClass>
            </ECSchema>)xml";

    Utf8CP badSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="BadRef" nameSpacePrefix="rs" version="01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECClass typeName="BadStruct" isDomainClass="False" isStruct="True">
                    <ECProperty propertyName="B" typeName="boolean" />
                </ECClass>
            </ECSchema>)xml";

    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Test" nameSpacePrefix="ts" version="01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECSchemaReference name="BadRef" version="01.00" prefix="rs" />
                <ECSchemaReference name="OtherSchema" version="01.00" prefix="os" />
                <ECClass typeName="Banana" isDomainClass="True">
                    <ECStructProperty propertyName="BadStruct" typeName="rs:BadStruct" displayLabel="bad struct" />
                    <ECStructProperty propertyName="AStruct" typeName="os:SecondStruct" displayLabel="a struct" />
                </ECClass>
            </ECSchema>)xml";

    StringSchemaLocater locater;
    locater.AddSchemaString("Ref", refSchemaXml);
    locater.AddSchemaString("OtherSchema", otherSchemaXml);
    locater.AddSchemaString("BadRef", badSchemaXml);
    locater.AddSchemaString("Test", schemaXml);
    context->AddSchemaLocater(locater);

    SchemaKey testKey ("Test", 1, 0);
    ECSchemaPtr schema = context->LocateSchema(testKey, SchemaMatchType::Latest);
    ASSERT_TRUE(schema.IsValid());

    auto banana = schema->GetClassCP("Banana");
    ASSERT_NE(nullptr, banana);
    auto badStructProp = banana->GetPropertyP("BadStruct");
    ASSERT_EQ(nullptr, badStructProp);
    auto aStructProp = banana->GetPropertyP("AStruct");
    ASSERT_NE(nullptr, aStructProp);

    auto it = schema->GetReferencedSchemas().Find(SchemaKey("BadRef", 1, 0), SchemaMatchType::Latest);
    ASSERT_TRUE (it == schema->GetReferencedSchemas().end()) << "Should not find BadRef Schema ... it was pruned";

    it = schema->GetReferencedSchemas().Find(SchemaKey("OtherSchema", 1, 0), SchemaMatchType::Latest);
    ASSERT_FALSE(it == schema->GetReferencedSchemas().end()) << "Should find OtherSchema Schema ... it was not pruned";
    ECSchemaPtr otherSchema = it->second;
    ASSERT_TRUE(otherSchema.IsValid());

    auto testStruct = otherSchema->GetClassCP("SecondStruct");
    ASSERT_NE(nullptr, testStruct);
    auto aProp = testStruct->GetPropertyP("A");
    ASSERT_NE(nullptr, aProp);

    it = otherSchema->GetReferencedSchemas().Find(SchemaKey("Ref", 1, 0), SchemaMatchType::Latest);
    ASSERT_FALSE(it == otherSchema->GetReferencedSchemas().end()) << "Should find RefSchema Schema ... it was not pruned";
    ECSchemaPtr refSchema = it->second;
    ASSERT_TRUE(refSchema.IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, PropertyConflictRenamedWhenDerivedClassBeforeBaseClass)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Test" nameSpacePrefix="ts" version="01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECClass typeName="Apple" isDomainClass="True">
                    <BaseClass>Fruit</BaseClass>
                    <ECProperty propertyName="Color" typeName="string" />
                </ECClass>
                <ECClass typeName="Fruit" isDomainClass="True">
                    <ECProperty propertyName="Color" typeName="int" />
                </ECClass>
            </ECSchema>)xml";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->SetResolveConflicts(true);
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->Validate());
    ASSERT_EQ(1, schema->GetClassCP("Fruit")->GetPropertyCount());
    ASSERT_EQ(2, schema->GetClassCP("Apple")->GetPropertyCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, PropertyConflictRenamedWhenDerivedClassAfterBaseClass)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Test" nameSpacePrefix="ts" version="01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECClass typeName="Fruit" isDomainClass="True">
                    <ECProperty propertyName="Color" typeName="int" />
                </ECClass>
                <ECClass typeName="Apple" isDomainClass="True">
                    <BaseClass>Fruit</BaseClass>
                    <ECProperty propertyName="Color" typeName="string" />
                </ECClass>
            </ECSchema>)xml";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->SetResolveConflicts(true);
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->Validate());
    ASSERT_EQ(1, schema->GetClassCP("Fruit")->GetPropertyCount());
    ASSERT_EQ(2, schema->GetClassCP("Apple")->GetPropertyCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, PropertyConflictFoundWhenDerivedClassBeforeBaseClass)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Test" alias="ts" version="01.01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEntityClass typeName="Apple" >
                    <BaseClass>Fruit</BaseClass>
                    <ECProperty propertyName="Color" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Fruit" isDomainClass="True">
                    <ECProperty propertyName="Color" typeName="int" />
                </ECEntityClass>
            </ECSchema>)xml";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, PropertyConflictFoundWhenDerivedClassAfterBaseClass)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Test" alias="ts" version="01.01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEntityClass typeName="Fruit" isDomainClass="True">
                    <ECProperty propertyName="Color" typeName="int" />
                </ECEntityClass>
                <ECEntityClass typeName="Apple" >
                    <BaseClass>Fruit</BaseClass>
                    <ECProperty propertyName="Color" typeName="string" />
                </ECEntityClass>
            </ECSchema>)xml";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, PropertyRenameHandlesBasePropertyWithCaseDifference)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Test" alias="ts" version="01.01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEntityClass typeName="Fruit" isDomainClass="True">
                    <ECProperty propertyName="Color" typeName="int" />
                    <ECProperty propertyName="ts_color_" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Apple" >
                    <BaseClass>Fruit</BaseClass>
                    <ECProperty propertyName="Color" typeName="string" />
                </ECEntityClass>
            </ECSchema>)xml";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->SetResolveConflicts(true);
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());
    auto prop = schema->GetClassCP("Apple")->GetPropertyP("ts_color_", false);
    ASSERT_NE(nullptr, prop);
    ASSERT_STREQ("ts_color_", prop->GetName().c_str()) << "Expected renamed property in derived class to match base class name casing";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, PropertyRenameHandlesBasePropertyWithTargetNameButIncompatibleType)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Test" alias="ts" version="01.01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEntityClass typeName="Fruit" isDomainClass="True">
                    <ECProperty propertyName="Color" typeName="int" />
                    <ECProperty propertyName="ts_Color_" typeName="double" />
                </ECEntityClass>
                <ECEntityClass typeName="Apple" >
                    <BaseClass>Fruit</BaseClass>
                    <ECProperty propertyName="Color" typeName="string" />
                </ECEntityClass>
            </ECSchema>)xml";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->SetResolveConflicts(true);
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());
    auto prop = schema->GetClassCP("Apple")->GetPropertyP("ts_Color__", false);
    Utf8String schemaString;
    schema->WriteToXmlString(schemaString);
    ASSERT_NE(nullptr, prop) << schemaString.c_str();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// NOTE: Exact rename order is arbitrary but should probably remain consistent
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, PropertyRenameHandlesDerivedPropertyWithTargetNameButIncompatibleType)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Test" alias="ts" version="01.01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEntityClass typeName="Fruit" isDomainClass="True">
                    <ECProperty propertyName="Color" typeName="int" />
                </ECEntityClass>
                <ECEntityClass typeName="Apple" >
                    <BaseClass>Fruit</BaseClass>
                    <ECProperty propertyName="Color" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Gala" >
                    <BaseClass>Apple</BaseClass>
                    <ECProperty propertyName="ts_Color_" typeName="double" />
                </ECEntityClass>
            </ECSchema>)xml";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->SetResolveConflicts(true);
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());
    Utf8String schemaString;
    schema->WriteToXmlString(schemaString);
    EXPECT_NE(nullptr, schema->GetClassCP("Fruit")->GetPropertyP("Color", false)) << schemaString.c_str();
    EXPECT_NE(nullptr, schema->GetClassCP("Apple")->GetPropertyP("ts_Color_", false)) << schemaString.c_str();
    EXPECT_NE(nullptr, schema->GetClassCP("Gala")->GetPropertyP("ts_ts_Color__", false)) << schemaString.c_str();
    }


//---------------------------------------------------------------------------------------
// @bsimethod
// NOTE: Exact rename order is arbitrary but should probably remain consistent
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, PropertyRenameHandleMultipleBaseClasses)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Test" alias="ts" version="01.01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEntityClass typeName="Fruit" isDomainClass="True">
                    <ECProperty propertyName="Color" typeName="int" />
                </ECEntityClass>
                <ECEntityClass typeName="Food" >
                    <ECProperty propertyName="Color" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Apple" >
                    <BaseClass>Fruit</BaseClass>
                    <BaseClass>Food</BaseClass>
                    <ECProperty propertyName="Color" typeName="double" />
                </ECEntityClass>
            </ECSchema>)xml";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->SetResolveConflicts(true);
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());

    Utf8String schemaString;
    schema->WriteToXmlString(schemaString);
    EXPECT_NE(nullptr, schema->GetClassCP("Fruit")->GetPropertyP("ts_Color_", false)) << schemaString.c_str();
    EXPECT_NE(nullptr, schema->GetClassCP("Food")->GetPropertyP("Color", false)) << schemaString.c_str();
    EXPECT_NE(nullptr, schema->GetClassCP("Apple")->GetPropertyP("ts_Color__", false)) << schemaString.c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaDeserializationTest, EmptyNamedItemsValidity)
    {
    auto wrapInSchemaAndRead = [](Utf8CP innerXml)
        {
        Utf8String schemaXml = Utf8String(
            R"xml(<?xml version="1.0" encoding="UTF-8"?>
                <ECSchema schemaName="test" displayLabel="TestSchema" alias="s1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            )xml")
            + innerXml
            + "</ECSchema>";

        ECSchemaPtr schema;
        ECSchemaReadContextPtr readCtx = ECSchemaReadContext::CreateContext();
        return ECSchema::ReadFromXmlString(schema, schemaXml.c_str(), *readCtx);
        };

    auto wrapInClassAndRead = [&](Utf8CP innerXml)
        {
        Utf8String schemaXml = Utf8String(
            R"xml(
                <ECEntityClass typeName="TestClass">
            )xml")
            + innerXml
            + "</ECEntityClass>";
        return wrapInSchemaAndRead(schemaXml.c_str());
        };

    EXPECT_EQ(SchemaReadStatus::InvalidECSchemaXml, wrapInSchemaAndRead(R"xml(<PropertyCategory typeName="" priority="0" />)xml"))
        << "it should be invalid to deserialize a property category with an empty name";
    EXPECT_EQ(SchemaReadStatus::Success, wrapInSchemaAndRead(R"xml(<PropertyCategory typeName="ValidName" priority="0" />)xml"))
        << "it should be valid to deserialize a property category with a non-empty name";

    EXPECT_EQ(SchemaReadStatus::InvalidECSchemaXml, wrapInSchemaAndRead(R"xml(<ECEntityClass typeName="" />)xml"))
        << "it should be invalid to deserialize an entity class with an empty name";
    EXPECT_EQ(SchemaReadStatus::Success, wrapInSchemaAndRead(R"xml(<ECEntityClass typeName="ValidName" />)xml"))
        << "it should be valid to deserialize an entity class with a non-empty name";

    EXPECT_EQ(SchemaReadStatus::InvalidECSchemaXml, wrapInSchemaAndRead(R"xml(<ECStructClass typeName="" />)xml"))
        << "it should be invalid to deserialize a struct class with an empty name";
    EXPECT_EQ(SchemaReadStatus::Success, wrapInSchemaAndRead(R"xml(<ECStructClass typeName="ValidName" />)xml"))
        << "it should be valid to deserialize a struct class with a non-empty name";

    EXPECT_EQ(SchemaReadStatus::InvalidECSchemaXml, wrapInClassAndRead(R"xml(<ECProperty propertyName="" typeName="int" />)xml"))
        << "it should be invalid to deserialize a property with an empty name";
    EXPECT_EQ(SchemaReadStatus::Success, wrapInClassAndRead(R"xml(<ECProperty propertyName="ValidName" typeName="int" />)xml"))
        << "it should be valid to deserialize a property with a non-empty name";

    EXPECT_EQ(SchemaReadStatus::InvalidECSchemaXml, wrapInSchemaAndRead(R"xml(
            <ECStructClass typeName="Loc" />
            <ECEntityClass typeName="TestEntity">
                <ECStructProperty propertyName="" typeName="Loc" />
            </ECEntityClass>
        )xml")
    ) << "it should be invalid to deserialize a struct property with an empty name";
    EXPECT_EQ(SchemaReadStatus::Success, wrapInSchemaAndRead(R"xml(
            <ECStructClass typeName="Loc" />
            <ECEntityClass typeName="TestEntity">
                <ECStructProperty propertyName="ValidName" typeName="Loc" />
            </ECEntityClass>
        )xml")
    ) << "it should be valid to deserialize a struct property with a non-empty name";

    EXPECT_EQ(SchemaReadStatus::InvalidECSchemaXml, wrapInSchemaAndRead(R"xml(
            <ECEntityClass typeName="A" />
            <ECEntityClass typeName="B">
                <ECNavigationProperty propertyName="" direction="backward" relationshipName="Relate" />
            </ECEntityClass>
            <ECRelationshipClass typeName="Relate" modifier="None" >
                <Source multiplicity="(0..1)" roleLabel="relates" polymorphic="true">
                    <Class class="A"/>
                </Source>
                <Target multiplicity="(0..1)" roleLabel="is related by" polymorphic="true">
                    <Class class="B"/>
                </Target>
            </ECRelationshipClass>
        )xml")
    ) << "it should be invalid to deserialize a navigation property with an empty name";
    EXPECT_EQ(SchemaReadStatus::Success, wrapInSchemaAndRead(R"xml(
            <ECEntityClass typeName="A" />
            <ECEntityClass typeName="B">
                <ECNavigationProperty propertyName="ValidName" direction="backward" relationshipName="Relate" />
            </ECEntityClass>
            <ECRelationshipClass typeName="Relate" modifier="None" >
                <Source multiplicity="(0..1)" roleLabel="relates" polymorphic="true">
                    <Class class="A"/>
                </Source>
                <Target multiplicity="(0..1)" roleLabel="is related by" polymorphic="true">
                    <Class class="B"/>
                </Target>
            </ECRelationshipClass>
        )xml")
    ) << "it should be valid to deserialize a navigation property with a non-empty name";

    EXPECT_EQ(SchemaReadStatus::InvalidECSchemaXml, wrapInSchemaAndRead(R"xml(
            <ECEnumeration typeName="" backingTypeName="int">
                <ECEnumerator name="E1" value="0" displayLabel="CODE1" />
            </ECEnumeration>
        )xml")
    ) << "it should be invalid to deserialize an enumeration with an empty name";
    EXPECT_EQ(SchemaReadStatus::Success, wrapInSchemaAndRead(R"xml(
            <ECEnumeration typeName="ValidName" backingTypeName="int">
                <ECEnumerator name="E1" value="0" displayLabel="CODE1" />
            </ECEnumeration>
        )xml")
    ) << "it should be valid to deserialize an enumeration with a non-empty name";

    EXPECT_EQ(SchemaReadStatus::InvalidECSchemaXml, wrapInSchemaAndRead(R"xml(
            <ECEnumeration typeName="E" backingTypeName="int">
                <ECEnumerator name="" value="0" displayLabel="CODE1" />
            </ECEnumeration>
        )xml")
    ) << "it should be invalid to deserialize an enumerator with an empty name";
    EXPECT_EQ(SchemaReadStatus::Success, wrapInSchemaAndRead(R"xml(
            <ECEnumeration typeName="E" backingTypeName="int">
                <ECEnumerator name="ValidName" value="0" displayLabel="CODE1" />
            </ECEnumeration>
        )xml")
    ) << "it should be valid to deserialize an enumerator with a non-empty name";

    EXPECT_EQ(SchemaReadStatus::InvalidECSchemaXml, wrapInSchemaAndRead(R"xml(
        <ECSchemaReference name="Units" version="01.00.00" alias="u" />
        <ECSchemaReference name="Formats" version="01.00.00" alias="f" />
        <KindOfQuantity typeName=""        displayLabel="test" persistenceUnit="u:RAD" presentationUnits="f:DefaultRealU(2)[u:ARC_DEG]" relativeError="0.001" />
    )xml")) << "it should be invalid to deserialize a Kind of Quantity with an empty name";
    EXPECT_EQ(SchemaReadStatus::Success,            wrapInSchemaAndRead(R"xml(
        <ECSchemaReference name="Units" version="01.00.00" alias="u" />
        <ECSchemaReference name="Formats" version="01.00.00" alias="f" />
        <KindOfQuantity typeName="TestKoQ" displayLabel="test" persistenceUnit="u:RAD" presentationUnits="f:DefaultRealU(2)[u:ARC_DEG]" relativeError="0.001" />
    )xml")) << "it should be valid to deserialize a Kind of Quantity with a non-empty name";

    EXPECT_EQ(SchemaReadStatus::InvalidECSchemaXml, wrapInSchemaAndRead(R"xml(
        <ECSchemaReference name="Units" version="01.00.00" alias="u" />
        <Unit typeName=""     phenomenon="u:VOLUME" unitSystem="u:USCUSTOM" definition="u:IN(3)" numerator="50.0" displayLabel="ttt" />
    )xml")) << "it should be invalid to deserialize a Unit with an empty name";
    EXPECT_EQ(SchemaReadStatus::Success,            wrapInSchemaAndRead(R"xml(
        <ECSchemaReference name="Units" version="01.00.00" alias="u" />
        <Unit typeName="TEST" phenomenon="u:VOLUME" unitSystem="u:USCUSTOM" definition="u:IN(3)" numerator="50.0" displayLabel="ttt" />
    )xml")) << "it should be valid to deserialize a Unit with a non-empty name";

    EXPECT_EQ(SchemaReadStatus::InvalidECSchemaXml, wrapInSchemaAndRead(R"xml(
        <Phenomenon typeName=""         definition="WORK*TEMPERATURE_CHANGE(-2)" displayLabel="Ttt" />
    )xml")) << "it should be invalid to deserialize a phenomenon with an empty name";
    EXPECT_EQ(SchemaReadStatus::Success,            wrapInSchemaAndRead(R"xml(
        <Phenomenon typeName="TESTROPY" definition="WORK*TEMPERATURE_CHANGE(-2)" displayLabel="Ttt" />
    )xml")) << "it should be valid to deserialize a phenomenon with a non-empty name";

    EXPECT_EQ(SchemaReadStatus::InvalidECSchemaXml, wrapInSchemaAndRead(R"xml(
        <ECSchemaReference name="Units" version="01.00.00" alias="u" />
        <Constant typeName=""     phenomenon="u:LENGTH_RATIO" definition="u:ONE" numerator="2.234234234234153142657" displayLabel="Ttt"/>
    )xml")) << "it should be invalid to deserialize a constant with an empty name";
    EXPECT_EQ(SchemaReadStatus::Success,            wrapInSchemaAndRead(R"xml(
        <ECSchemaReference name="Units" version="01.00.00" alias="u" />
        <Constant typeName="TEST" phenomenon="u:LENGTH_RATIO" definition="u:ONE" numerator="2.234234234234153142657" displayLabel="Ttt"/>
    )xml")) << "it should be valid to deserialize a constant with a non-empty name";

    EXPECT_EQ(SchemaReadStatus::InvalidECSchemaXml, wrapInSchemaAndRead(R"xml(
        <ECSchemaReference name="Units" version="01.00.00" alias="u" />
        <InvertedUnit typeName=""           invertsUnit="u:MM_PER_SEC" unitSystem="u:INTERNATIONAL" />
    )xml")) << "it should be invalid to deserialize an inverted unit with an empty name";
    EXPECT_EQ(SchemaReadStatus::Success,            wrapInSchemaAndRead(R"xml(
        <ECSchemaReference name="Units" version="01.00.00" alias="u" />
        <InvertedUnit typeName="SEC_PER_MM" invertsUnit="u:MM_PER_SEC" unitSystem="u:INTERNATIONAL" />
    )xml")) << "it should be valid to deserialize an inverted unit with a non-empty name";

    EXPECT_EQ(SchemaReadStatus::InvalidECSchemaXml, wrapInSchemaAndRead(R"xml(
        <UnitSystem typeName=""              />
    )xml")) << "it should be invalid to deserialize a unit system with an empty name";
    EXPECT_EQ(SchemaReadStatus::Success,            wrapInSchemaAndRead(R"xml(
        <UnitSystem typeName="TEST_UNIT_SYS" />
    )xml")) << "it should be valid to deserialize a unit system with a non-empty name";

    EXPECT_EQ(SchemaReadStatus::InvalidECSchemaXml, wrapInSchemaAndRead(R"xml(
        <ECSchemaReference name="Units" version="01.00.00" alias="u" />
        <Format typeName=""           displayLabel="Inches" type="fractional" precision="8" uomSeparator="" formatTraits="keepSingleZero|showUnitLabel">
            <Composite spacer="">
                <Unit label="&quot;">u:IN</Unit>
            </Composite>
        </Format>
    )xml")) << "it should be invalid to deserialize a format with an empty name";
    EXPECT_EQ(SchemaReadStatus::Success,            wrapInSchemaAndRead(R"xml(
        <ECSchemaReference name="Units" version="01.00.00" alias="u" />
        <Format typeName="TestFormat" displayLabel="Inches" type="fractional" precision="8" uomSeparator="" formatTraits="keepSingleZero|showUnitLabel">
            <Composite spacer="">
                <Unit label="&quot;">u:IN</Unit>
            </Composite>
        </Format>
    )xml")) << "it should be valid to deserialize a format with a non-empty name";
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaDeserializationTest, SchemaUnitWithAndWithoutPropertiesExplicitlySet)
    {

    ECSchemaPtr writeSchema;
    ECSchema::CreateSchema(writeSchema, "TestSchema", "ts", 1, 0, 7);

    ECUnitP unit;
    PhenomenonP phenomenon;
    UnitSystemP system;
    writeSchema->CreatePhenomenon(phenomenon, "LENGTH", "LENGTH");
    writeSchema->CreateUnitSystem(system, "TestUnitSystem");

    // Numerator, denominator and offset are set
    ECObjectsStatus ecObjectsStatus = writeSchema->CreateUnit(unit, "TestUnit1", "TestUnit1", *phenomenon, *system, 2.0, 3.0, 4.0, "TestLabel1", "Test unit 1.");
    ASSERT_EQ(ECObjectsStatus::Success, ecObjectsStatus);

    // Numerator, denominator and offset are not set
    ecObjectsStatus = writeSchema->CreateUnit(unit, "TestUnit2", "TestUnit2", *phenomenon, *system, "TestLabel2", "Test unit 2.");
    ASSERT_EQ(ECObjectsStatus::Success, ecObjectsStatus);

    SchemaWriteStatus writeStatus = writeSchema->WriteToXmlFile(ECTestFixture::GetTempDataPath(L"TestSchema.ecschema.xml").c_str());
    ASSERT_EQ(SchemaWriteStatus::Success, writeStatus);

    ECSchemaPtr readSchema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus readStatus = ECSchema::ReadFromXmlFile(readSchema, ECTestFixture::GetTempDataPath(L"TestSchema.ecschema.xml").c_str(), *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, readStatus);
    ASSERT_TRUE(readSchema.IsValid());

    EXPECT_STREQ("TestSchema", readSchema->GetName().c_str());
    EXPECT_STREQ("01.00.07", readSchema->GetSchemaKey().GetVersionString().c_str());

    ECUnitCP unitCP = readSchema->GetUnitCP("TestUnit1");
    EXPECT_TRUE(unitCP->HasOffset());
    EXPECT_TRUE(unitCP->HasNumerator());
    EXPECT_TRUE(unitCP->HasDenominator());

    unitCP = readSchema->GetUnitCP("TestUnit2");
    EXPECT_FALSE(unitCP->HasOffset());
    EXPECT_FALSE(unitCP->HasNumerator());
    EXPECT_FALSE(unitCP->HasDenominator());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaDeserializationTest, SchemaFailsWithPresenceOfControlCharacters)
    {

    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Test" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEntityClass typeName="Fruit" isDomainClass="True" description="A generic description">
                    <ECProperty propertyName="Color" typeName="int" />
                </ECEntityClass>
                <ECEntityClass typeName="Apple">
                    <BaseClass>Fruit</BaseClass>
                    <ECProperty propertyName="Color" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Gala" >
                    <BaseClass>Apple</BaseClass>
                    <ECProperty propertyName="ts_Color_" typeName="double" />
                </ECEntityClass>
            </ECSchema> )xml";

    //Showing current schema is valid
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->SetResolveConflicts(true);
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());

    //adding control characters
    Utf8String controlString = "\x1f This description now has control characters in it\x03\u0006";
    schema->GetClassP("Apple")->SetDescription(controlString);
    ASSERT_EQ(controlString, schema->GetClassP("Apple")->GetDescription());

    //Showing schema is now invalid
    ECSchemaReadContextPtr context_hasCtrlChars = ECSchemaReadContext::CreateContext();  
    SchemaKey testKey ("Test", 1, 0);
    ECSchemaPtr schema_hasCtrlChars = context_hasCtrlChars->LocateSchema(testKey, SchemaMatchType::Latest);
    ASSERT_FALSE(schema_hasCtrlChars.IsValid());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                  
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaDeserializationTest, ControlCharacterHandling)
    {
    
    //Creating Schema
    ECSchemaPtr schema; 
    ECSchema::CreateSchema(schema, "Test", "ts", 1, 0, 0);
    schema->SetDescription("\x09Has cont\x0Brol characters in it\x0A");
    schema->SetDisplayLabel("\x09Has cont\x0Brol characters in it\x0A");

    ECEntityClassP fruit;
    schema->CreateEntityClass(fruit, "Fruit");

    ECEntityClassP apples;
    schema->CreateEntityClass(apples, "Apples");
    apples->AddBaseClass(*fruit);

    ECEntityClassP veg;
    schema->CreateEntityClass(veg, "Vegetables");

    ECEntityClassP carrots;
    schema->CreateEntityClass(carrots, "Carrots");
    carrots->AddBaseClass(*veg);
    ASSERT_TRUE(schema.IsValid());

    //Setting Descriptions/display labels to contain control characters
    schema->GetClassP("Fruit")->SetDescription("\06Has control characters in it\07\x08");
    schema->GetClassP("Fruit")->SetDisplayLabel("\x09Has cont\x0Brol characters in it\x0A");
    schema->GetClassP("Apples")->SetDescription("\x0CHas control characters in it\x0E");
    schema->GetClassP("Apples")->SetDisplayLabel("\x0FHas control cha\x0Dracters in it\x10\11");
    schema->GetClassP("Vegetables")->SetDescription("Has contro\x12l charac\13ters in it\14");
    schema->GetClassP("Vegetables")->SetDisplayLabel("\x15Has control characters in it\16\17");
    schema->GetClassP("Carrots")->SetDescription("\x18Has control cha\x1Fracters in it\x19\x1A");
    schema->GetClassP("Carrots")->SetDisplayLabel("\x1BHas control\x1E characters in it\x1C\x1D");

    //Creating relationship class and setting its role labels
    ECRelationshipClassP relClass;
    schema->CreateRelationshipClass(relClass, "RelationshipClass");
    relClass->GetSource().AddClass(*fruit);
    relClass->GetTarget().AddClass(*veg);
    relClass->GetSource().SetRoleLabel("Has control characters in it\u0001\02");
    relClass->GetTarget().SetRoleLabel("\x03Has control characters in it\04\05");

    //creating properties and setting descriptions/display labels to contain control characters
    PrimitiveECPropertyP prop1, prop2, prop3, prop4;
    schema->GetClassP("Fruit")->CreatePrimitiveProperty(prop1, "prop1", PRIMITIVETYPE_String);
    schema->GetClassP("Apples")->CreatePrimitiveProperty(prop2, "prop2", PRIMITIVETYPE_String);
    schema->GetClassP("Vegetables")->CreatePrimitiveProperty(prop3, "prop3", PRIMITIVETYPE_String);
    schema->GetClassP("Carrots")->CreatePrimitiveProperty(prop4, "prop4", PRIMITIVETYPE_String);

    prop1->SetDescription("Has contro\x12l charac\13ters in it\14");
    prop1->SetDisplayLabel("\06Has control characters in it\07\x08");
    prop2->SetDescription("Has contro\x12l charac\13ters in it\14");
    prop2->SetDisplayLabel("\x15Has control characters in it\16\17");
    prop3->SetDescription("\x03Has control characters in it\04\05");
    prop3->SetDisplayLabel("Has control characters in it\u0001\02");
    prop4->SetDescription("\x1BHas control\x1E characters in it\x1C\x1D");
    prop4->SetDisplayLabel("\06Has control characters in it\07\x08");

    //Showing that schema is still seen as valid despite presence of control characters
    ASSERT_TRUE(schema.IsValid());

    //Removing control characters from schema
    ECSchema::RemoveInvalidDisplayCharacters(*schema);

    //Showing all Descriptions, display labels, and role labels have had their control characters removed, and schema is valid
    Utf8String noControlString ="Has control characters in it";

    ASSERT_EQ(noControlString, schema->GetDescription());
    ASSERT_EQ(noControlString, schema->GetDisplayLabel());

    ASSERT_EQ(noControlString, schema->GetClassP("Fruit")->GetDescription());
    ASSERT_EQ(noControlString, schema->GetClassP("Fruit")->GetDisplayLabel());
    ASSERT_EQ(noControlString, schema->GetClassP("Apples")->GetDescription());
    ASSERT_EQ(noControlString, schema->GetClassP("Apples")->GetDisplayLabel());
    ASSERT_EQ(noControlString, schema->GetClassP("Vegetables")->GetDescription());
    ASSERT_EQ(noControlString, schema->GetClassP("Vegetables")->GetDisplayLabel());
    ASSERT_EQ(noControlString, schema->GetClassP("Carrots")->GetDescription());
    ASSERT_EQ(noControlString, schema->GetClassP("Carrots")->GetDisplayLabel());

    ASSERT_EQ(noControlString, relClass->GetSource().GetRoleLabel());
    ASSERT_EQ(noControlString, relClass->GetTarget().GetRoleLabel());

    ASSERT_EQ(noControlString, schema->GetClassP("Fruit")->GetPropertyP("prop1")->GetDescription());
    ASSERT_EQ(noControlString, schema->GetClassP("Fruit")->GetPropertyP("prop1")->GetDisplayLabel());
    ASSERT_EQ(noControlString, schema->GetClassP("Apples")->GetPropertyP("prop2")->GetDescription());
    ASSERT_EQ(noControlString, schema->GetClassP("Apples")->GetPropertyP("prop2")->GetDisplayLabel());
    ASSERT_EQ(noControlString, schema->GetClassP("Vegetables")->GetPropertyP("prop3")->GetDescription());
    ASSERT_EQ(noControlString, schema->GetClassP("Vegetables")->GetPropertyP("prop3")->GetDisplayLabel());
    ASSERT_EQ(noControlString, schema->GetClassP("Carrots")->GetPropertyP("prop4")->GetDescription());
    ASSERT_EQ(noControlString, schema->GetClassP("Carrots")->GetPropertyP("prop4")->GetDisplayLabel());

   ASSERT_TRUE(schema.IsValid());
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (SchemaDeserializationTest, SchemaNameValidationControlCharacterRemoval)
    {

    //Shows that a schema cannot be created with a name containing control characters  
    //Cannot do something like badSchema->GetName() since it is an invalid schema  
    ECSchemaPtr badSchema;
    ECSchema::CreateSchema(badSchema, "bad\x06Name\x1f", "bn", 1, 0, 0);
    ASSERT_FALSE(badSchema.IsValid());

    //creating valid schema
    //Shows that when you SetName() with control characters, if there is already a valid name, it does not go through
    //The name must be valid to successfully reset
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "schemaName", "Ts", 1, 0, 0);
    schema->SetName("Te\x03st\x1a");
    ASSERT_NE("Te\x03st\x1a", schema->GetName());
    ASSERT_EQ("schemaName", schema->GetName());

    //shows that SetName() is working as expected without control characters
    schema->SetName("SomethingValid");
    ASSERT_EQ("SomethingValid", schema->GetName());

    //shows ECNameValidation::EncodeToValidName() encodes control characters in a way which makes the schema name valid
    Utf8String encodedName, controlCharName = "\x1fTe\x03st\x1a";
    ECNameValidation::EncodeToValidName(encodedName, controlCharName);
    ASSERT_EQ("__x001F__Te__x0003__st__x001A__", encodedName);
    schema->SetName(encodedName);
    ASSERT_EQ("__x001F__Te__x0003__st__x001A__", schema->GetName());
    ASSERT_TRUE(schema.IsValid());

    //shows ECNameValidation::DecodeFromValidName() will remove not control characters which made it into an encoded name
    //Note: Removing control characters through DecodeFromValidName() created an issue where superscript 4 was also removed in linux, so the functionality was removed 
    Utf8String decodedName, badEncodedName = "Te__x0006____x001B__s__x0015__t__x001F____x0021__";
    ECNameValidation::DecodeFromValidName(decodedName, badEncodedName);
    ASSERT_EQ("Te\x06\x1Bs\x15t\x1F!", decodedName);

    //Shows handling of non control character invalid characters
    Utf8String charName = "Te^st-";
    ECNameValidation::EncodeToValidName(encodedName, charName);
    ASSERT_EQ("Te__x005E__st__x002D__", encodedName);
    ECNameValidation::DecodeFromValidName(decodedName, encodedName);
    ASSERT_EQ(charName, decodedName);
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (SchemaDeserializationTest, ECNameValidation_NonASCIICharsAreNotRemoved)
    {
    auto removeControlChars = [](Utf8String testString)
        {
        Utf8String charsRemoved;
        EXPECT_FALSE(ECNameValidation::RemoveControlCharacters(charsRemoved, testString)) << "Expected no chars removed " << charsRemoved.c_str();
        EXPECT_STREQ(charsRemoved.c_str(), testString.c_str());        
        };
    removeControlChars("inH2O@39.2°F");
    removeControlChars("W/(m·Δ°C)");
    removeControlChars("m²·Δ°C/W");
    removeControlChars("ft³ (US Survey)");
    removeControlChars("ft⁴");
    removeControlChars("🌈💨🍩🎪⚓️📀💚👊🏽");
    removeControlChars("ÂḚƜẳ⨚∲⊭⋙⋷⨂⎝»☾☄︎🃇‱⸘⁇❣❡➩⇒↤⇶◉•∙☒☑︎№©ℹ︎ℳ℃℉ℬµΩ");
    removeControlChars("नमस्कार");
    removeControlChars("ہیلو");
    removeControlChars("你好");
    removeControlChars("こんにちは");
    removeControlChars("Здравствуйте");
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (SchemaDeserializationTest, ECNameValidation_NonASCIICharsAreProperlyEnCodedDecoded)
    {
    auto encodeDecodeString = [](Utf8String testString)
        {
        Utf8String encodedString;
        Utf8String decodedString;
        EXPECT_TRUE(ECNameValidation::EncodeToValidName(encodedString, testString)) << "Expected to encode string: " << testString.c_str();
        EXPECT_TRUE(ECNameValidation::DecodeFromValidName(decodedString, encodedString)) << "Expected to decode string: " << encodedString.c_str();
        EXPECT_STREQ(testString.c_str(), decodedString.c_str()) << "Encoded string: " << encodedString.c_str();
        };
    encodeDecodeString ("inH2O@39.2°F");
    encodeDecodeString ("W/(m·Δ°C)");
    encodeDecodeString ("m²·Δ°C/W");
    encodeDecodeString ("ft³ (US Survey)");
    encodeDecodeString ("ft⁴");
    //encodeDecodeString ("🌈💨🍩🎪📀💚👊🏽"); // Emoji do not round trip
    encodeDecodeString ("ÂḚƜẳ⨚∲⊭⋙⋷⨂⎝»☾☄︎‱⸘⁇❣❡➩⇒↤⇶◉•∙☒☑︎№©ℹ︎ℳ℃℉ℬµΩ");
    //encodeDecodeString ("🃇");  // Does not round trip
    encodeDecodeString ("नमस्कार");
    encodeDecodeString ("ہیلو");
    encodeDecodeString ("你好");
    encodeDecodeString ("こんにちは");
    encodeDecodeString ("Здравствуйте");
    }

END_BENTLEY_ECN_TEST_NAMESPACE
