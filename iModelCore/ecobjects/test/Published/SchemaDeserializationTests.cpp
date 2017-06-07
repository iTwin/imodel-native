/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/SchemaDeserializationTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"
#include "BeXml/BeXml.h"

#include <iostream>
#include <fstream>

using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct SchemaDeserializationTest : ECTestFixture 
    {
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Paul.Connelly   12/12
    +---------------+---------------+---------------+---------------+---------------+------*/
    static ECPropertyP GetPropertyByName (ECClassCR ecClass, Utf8CP name, bool expectExists = true)
        {
        ECPropertyP prop = ecClass.GetPropertyP (name);
        EXPECT_EQ (expectExists, NULL != prop);
        Utf8String utf8 (name);
        prop = ecClass.GetPropertyP (utf8.c_str ());
        EXPECT_EQ (expectExists, NULL != prop);
        return prop;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Carole.MacDonald                01/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    void VerifyWidgetsSchema (ECSchemaPtr const&   schema)
        {
        ASSERT_TRUE(schema.IsValid());
        EXPECT_TRUE(schema->IsECVersion(ECVersion::V3_1));

        EXPECT_STREQ ("Widgets", schema->GetName ().c_str ());
        EXPECT_STREQ ("wid", schema->GetAlias ().c_str ());
        EXPECT_STREQ ("Widgets Display Label", schema->GetDisplayLabel ().c_str ());
        EXPECT_TRUE (schema->GetIsDisplayLabelDefined ());
        EXPECT_STREQ ("Widgets Description", schema->GetDescription ().c_str ());
        EXPECT_EQ (9, schema->GetVersionRead ());
        EXPECT_EQ (6, schema->GetVersionMinor ());

    #ifdef DEBUG_PRINT
        for (ECClassP pClass: schema->GetClasses())
            {
            printf ("Widgets contains class: '%s' with display label '%s'\n", pClass->GetName().c_str(), pClass->GetDisplayLabel().c_str());
            }
    #endif

        ECClassP pClass = schema->GetClassP ("ClassDoesNotExistInSchema");
        EXPECT_FALSE (pClass);

        pClass = schema->GetClassP ("ecProject");
        ASSERT_TRUE (NULL != pClass);
        EXPECT_STREQ ("ecProject", pClass->GetName ().c_str ());
        EXPECT_STREQ ("Project", pClass->GetDisplayLabel ().c_str ());
        EXPECT_TRUE (pClass->GetIsDisplayLabelDefined ());
        EXPECT_STREQ ("Project Class", pClass->GetDescription ().c_str ());
        EXPECT_TRUE (pClass->IsEntityClass());
        EXPECT_FALSE (pClass->HasBaseClasses ());
        ECPropertyP pProperty = GetPropertyByName (*pClass, "Name");
        ASSERT_TRUE (NULL != pProperty);
        EXPECT_STREQ ("Name", pProperty->GetName ().c_str ());
        EXPECT_TRUE (pProperty->GetIsPrimitive ());
        EXPECT_FALSE (pProperty->GetIsStruct ());
        EXPECT_FALSE (pProperty->GetIsArray ());
        EXPECT_STREQ ("string", pProperty->GetTypeName ().c_str ());
        EXPECT_TRUE (PRIMITIVETYPE_String == pProperty->GetAsPrimitiveProperty ()->GetType ());
        EXPECT_TRUE (pProperty->GetIsDisplayLabelDefined ());
        EXPECT_STREQ ("Project Name", pProperty->GetDisplayLabel ().c_str ());
        EXPECT_STREQ ("", pProperty->GetDescription ().c_str ());
        EXPECT_EQ (pClass, &pProperty->GetClass ());
        EXPECT_FALSE (pProperty->GetIsReadOnly ());

        pProperty = GetPropertyByName (*pClass, "PropertyDoesNotExistInClass", false);
        EXPECT_FALSE (pProperty);

        ECClassP customAttribClass = schema->GetClassP ("AccessCustomAttributes");
        ASSERT_TRUE (NULL != customAttribClass);
        EXPECT_STREQ ("AccessCustomAttributes", customAttribClass->GetName ().c_str ());
        EXPECT_STREQ ("AccessCustomAttributes", customAttribClass->GetDisplayLabel ().c_str ());
        EXPECT_FALSE (customAttribClass->GetIsDisplayLabelDefined ());
        EXPECT_STREQ ("", customAttribClass->GetDescription ().c_str ());
        EXPECT_TRUE (customAttribClass->IsCustomAttributeClass());
        EXPECT_FALSE (customAttribClass->HasBaseClasses ());

        pClass = schema->GetClassP ("Struct1");
        ASSERT_TRUE (NULL != pClass);
        EXPECT_STREQ ("Struct1", pClass->GetName ().c_str ());
        EXPECT_STREQ ("Struct1", pClass->GetDisplayLabel ().c_str ());
        EXPECT_FALSE (pClass->GetIsDisplayLabelDefined ());
        EXPECT_STREQ ("", pClass->GetDescription ().c_str ());
        EXPECT_TRUE (pClass->IsStructClass());
        EXPECT_FALSE (pClass->HasBaseClasses ());

        pClass = schema->GetClassP ("Struct2");
        ASSERT_TRUE (NULL != pClass);
        EXPECT_STREQ ("Struct2", pClass->GetName ().c_str ());
        EXPECT_STREQ ("Struct2", pClass->GetDisplayLabel ().c_str ());
        EXPECT_FALSE (pClass->GetIsDisplayLabelDefined ());
        EXPECT_STREQ ("", pClass->GetDescription ().c_str ());
        EXPECT_TRUE (pClass->IsStructClass());
        EXPECT_FALSE (pClass->HasBaseClasses ());
        pProperty = GetPropertyByName (*pClass, "NestedArray");
        EXPECT_TRUE (NULL != pProperty);
        EXPECT_STREQ ("NestedArray", pProperty->GetName ().c_str ());
        EXPECT_FALSE (pProperty->GetIsPrimitive ());
        EXPECT_FALSE (pProperty->GetIsStruct ());
        EXPECT_TRUE (pProperty->GetIsArray ());
        EXPECT_STREQ ("Struct1", pProperty->GetTypeName ().c_str ());
        StructArrayECPropertyP structArrayProperty = pProperty->GetAsStructArrayPropertyP ();
        EXPECT_TRUE (NULL != structArrayProperty);
        EXPECT_EQ (schema->GetClassP ("Struct1"), &structArrayProperty->GetStructElementType ());
        EXPECT_EQ (0, structArrayProperty->GetMinOccurs ());
        EXPECT_EQ (UINT_MAX, structArrayProperty->GetMaxOccurs ());
        EXPECT_FALSE (pProperty->GetIsDisplayLabelDefined ());
        EXPECT_STREQ ("NestedArray", pProperty->GetDisplayLabel ().c_str ());
        EXPECT_STREQ ("", pProperty->GetDescription ().c_str ());
        EXPECT_EQ (pClass, &pProperty->GetClass ());
        EXPECT_FALSE (pProperty->GetIsReadOnly ());

        pClass = schema->GetClassP ("TestClass");
        ASSERT_TRUE (NULL != pClass);
        EXPECT_TRUE (pClass->HasBaseClasses ());
        pProperty = GetPropertyByName (*pClass, "EmbeddedStruct");
        ASSERT_TRUE (NULL != pProperty);
        EXPECT_STREQ ("EmbeddedStruct", pProperty->GetName ().c_str ());
        EXPECT_FALSE (pProperty->GetIsPrimitive ());
        EXPECT_TRUE (pProperty->GetIsStruct ());
        EXPECT_FALSE (pProperty->GetIsArray ());
        EXPECT_STREQ ("Struct1", pProperty->GetTypeName ().c_str ());
        StructECPropertyP structProperty = pProperty->GetAsStructPropertyP ();
        EXPECT_EQ (schema->GetClassP ("Struct1"), &(structProperty->GetType ()));
        EXPECT_FALSE (pProperty->GetIsDisplayLabelDefined ());
        EXPECT_STREQ ("EmbeddedStruct", pProperty->GetDisplayLabel ().c_str ());
        EXPECT_STREQ ("", pProperty->GetDescription ().c_str ());
        EXPECT_EQ (pClass, &pProperty->GetClass ());
        EXPECT_FALSE (pProperty->GetIsReadOnly ());

        IECInstancePtr instance = pClass->GetCustomAttribute (*customAttribClass);
        EXPECT_TRUE (instance.IsValid ());

        ECValue ecValue;
        EXPECT_EQ (ECObjectsStatus::Success, instance->GetValue (ecValue, "AccessLevel"));
        EXPECT_EQ (4, ecValue.GetInteger ());

        EXPECT_EQ (ECObjectsStatus::Success, instance->GetValue (ecValue, "Writeable"));
        EXPECT_FALSE (ecValue.GetBoolean ());

    #ifdef DEBUG_PRINT
        for (ECPropertyP pProperty: pClass->GetProperties())
            {
            printf ("TestClass contains property: %s of type %s\n", pProperty->GetName().c_str(), pProperty->GetTypeName().c_str());
            }
    #endif   
        }
    };




//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2015
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
// @bsimethod                                   Carole.MacDonald            12/2015
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, InvalidStructArrayPropertySpecification)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

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
    ASSERT_TRUE(nullptr != prop);
    StructArrayECPropertyCP typeReferences1 = prop->GetAsStructArrayProperty();
    ASSERT_TRUE(nullptr == typeReferences1);

    ArrayECPropertyCP typeReferences2 = prop->GetAsArrayProperty();
    ASSERT_TRUE(nullptr != typeReferences2);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, CaseSensitivity)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

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
    ASSERT_TRUE(nullptr != ent);
    ECPropertyP prop = ent->GetPropertyP("StructProp");
    ASSERT_TRUE(nullptr != prop);
    StructECPropertyP structProp = prop->GetAsStructPropertyP();
    ASSERT_TRUE(nullptr != structProp);
    EXPECT_TRUE(ent->IsDefined("CustomAttrib"));

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
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
TEST_F(SchemaDeserializationTest, ExpectUnrecognizedTypeNamesToSurviveRoundtrip)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='a' version='23.42' nameSpacePrefix='a' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECClass typeName='c'>"
        "       <ECProperty      propertyName='p' typeName='foobar'  />"
        "       <ECArrayProperty propertyName='q' typeName='barfood' minOccurs='0' maxOccurs='unbounded'/>"
        "    </ECClass>"
        "</ECSchema>";

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);

    Utf8String ecSchemaXml;
    SchemaWriteStatus writeStatus = schema->WriteToXmlString(ecSchemaXml);
    EXPECT_EQ(SchemaWriteStatus::Success, writeStatus);

    EXPECT_NE(Utf8String::npos, ecSchemaXml.find("typeName=\"foobar\""));
    EXPECT_NE(Utf8String::npos, ecSchemaXml.find("typeName=\"barfood\""));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWithInvalidTypeNameInPrimitiveProperty)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Widgets' version='09.06' displayLabel='Widgets Display Label' description='Widgets Description' nameSpacePrefix='wid' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "<ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "    <ECClass typeName='ecProject' description='Project ECClass' displayLabel='Project' isDomainClass='True'>"
        "       <ECProperty propertyName='Name' typeName='strng' displayLabel='Project Name' />"
        "    </ECClass>"
        "</ECSchema>";
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext);

    EXPECT_EQ(SchemaReadStatus::Success, status);

    ECClassP pClass = schema->GetClassP("ecProject");
    ECPropertyP pProperty = GetPropertyByName(*pClass, "Name");
    EXPECT_TRUE(PRIMITIVETYPE_String == pProperty->GetAsPrimitiveProperty()->GetType());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWithEmptyCustomAttribute)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    //schemaContext->AddSchemaPath(L"C:\\temp\\data\\ECXA\\SchemasAndDgn\');
    //SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, L"C:\\temp\\data\\ECXA\\SchemasAndDgn\\Bentley_Plant.06.00.ecschema.xml", *schemaContext);
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
    WString seedPath(ECTestFixture::GetTestDataPath(L"").c_str());
    schemaContext->AddSchemaPath(seedPath.c_str());

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"SchemaThatReferences.01.00.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);

    ECClassP pClass = schema->GetClassP("circle");
    ASSERT_TRUE(NULL != pClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenDeserializingSchemaWithEnumerationInReferencedFile)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    WString seedPath(ECTestFixture::GetTestDataPath(L"").c_str());
    schemaContext->AddSchemaPath(seedPath.c_str());

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"EnumInReferencedSchema.01.00.01.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);

    ASSERT_TRUE(schema.IsValid());

    ECClassP pClass = schema->GetClassP("Entity");
    ASSERT_TRUE(nullptr != pClass);

    ECPropertyP p = pClass->GetPropertyP("EnumeratedProperty");
    ASSERT_TRUE(p != nullptr);

    PrimitiveECPropertyCP prim = p->GetAsPrimitiveProperty();
    ASSERT_TRUE(prim != nullptr);

    ECEnumerationCP ecEnum = prim->GetEnumeration();
    ASSERT_TRUE(ecEnum != nullptr);
    EXPECT_STREQ("MyEnumeration", ecEnum->GetName().c_str());

    ECPropertyP p2 = pClass->GetPropertyP("EnumeratedArray");
    ASSERT_TRUE(p2 != nullptr);

    PrimitiveArrayECPropertyCP primArr = p2->GetAsPrimitiveArrayProperty();
    ASSERT_TRUE(primArr != nullptr);

    ECEnumerationCP arrEnum = primArr->GetEnumeration();
    ASSERT_TRUE(arrEnum != nullptr);
    EXPECT_STREQ("MyEnumeration", arrEnum->GetName().c_str());

    ASSERT_TRUE(ecEnum->GetSchema().GetVersionWrite() == 12);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenDeserializingSchemaWithKindOfQuantityInReferencedFile)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    WString seedPath(ECTestFixture::GetTestDataPath(L"").c_str());
    schemaContext->AddSchemaPath(seedPath.c_str());

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"KindOfQuantityInReferencedSchema.01.00.00.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);

    ASSERT_TRUE(schema.IsValid());

    ECClassP pClass = schema->GetClassP("Entity");
    ASSERT_TRUE(nullptr != pClass);

    ECPropertyP p = pClass->GetPropertyP("KindOfQuantityProperty");
    ASSERT_TRUE(p != nullptr);

    PrimitiveECPropertyCP prim = p->GetAsPrimitiveProperty();
    ASSERT_TRUE(prim != nullptr);

    KindOfQuantityCP kindOfQuantity = prim->GetKindOfQuantity();
    ASSERT_TRUE(kindOfQuantity != nullptr);

    EXPECT_STREQ(kindOfQuantity->GetName().c_str(), "MyKindOfQuantity");
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
    ASSERT_TRUE(NULL != pClass);
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
    ASSERT_TRUE(NULL != pClass);
    EXPECT_STREQ("ecProject", pClass->GetName().c_str());
    EXPECT_STREQ("Project", pClass->GetDisplayLabel().c_str());
    EXPECT_TRUE(pClass->GetIsDisplayLabelDefined());
    EXPECT_STREQ("Project ECClass", pClass->GetDescription().c_str());
    EXPECT_TRUE(pClass->IsEntityClass());
    EXPECT_FALSE(pClass->HasBaseClasses());
    ECPropertyP pProperty = GetPropertyByName(*pClass, "Name");
    ASSERT_TRUE(NULL != pProperty);
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
    ASSERT_TRUE(NULL != pProperty);
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
    ASSERT_TRUE(NULL != pProperty);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenRoundtripEnumerationUsingString)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);
    ASSERT_TRUE(schema.IsValid());

    //Create Enumeration
    ECEnumerationP enumeration;
    auto status = schema->CreateEnumeration(enumeration, "Enumeration", PrimitiveType::PRIMITIVETYPE_String);
    enumeration->SetDescription("de");
    enumeration->SetDisplayLabel("dl");
    enumeration->SetIsStrict(false);
    ASSERT_TRUE(enumeration != nullptr);
    ASSERT_TRUE(status == ECObjectsStatus::Success);
    EXPECT_STREQ("dl", enumeration->GetDisplayLabel().c_str());
    EXPECT_STREQ("de", enumeration->GetDescription().c_str());
    EXPECT_STREQ("string", enumeration->GetTypeName().c_str());

    ECEnumeratorP enumerator;
    EXPECT_TRUE(enumeration->CreateEnumerator(enumerator, "First") == ECObjectsStatus::Success);
    enumerator->SetDisplayLabel("First Value");
    EXPECT_TRUE(enumeration->CreateEnumerator(enumerator, "Second") == ECObjectsStatus::Success);
    enumerator->SetDisplayLabel("Second Value");
    EXPECT_TRUE(enumeration->GetEnumeratorCount() == 2);

    ECEntityClassP entityClass;
    status = schema->CreateEntityClass(entityClass, "EntityClass");
    ASSERT_TRUE(status == ECObjectsStatus::Success);
    PrimitiveECPropertyP property;
    status = entityClass->CreateEnumerationProperty(property, "EnumProperty", *enumeration);
    ASSERT_TRUE(status == ECObjectsStatus::Success);
    ASSERT_TRUE(property != nullptr);
    EXPECT_STREQ("Enumeration", property->GetTypeName().c_str());

    PrimitiveArrayECPropertyP arrProperty;
    status = entityClass->CreatePrimitiveArrayProperty(arrProperty, "EnumArrProperty");
    ASSERT_TRUE(status == ECObjectsStatus::Success);
    ASSERT_TRUE(nullptr != arrProperty);
    status = arrProperty->SetType(*enumeration);
    ASSERT_TRUE(status == ECObjectsStatus::Success);
    EXPECT_STREQ("Enumeration", arrProperty->GetTypeName().c_str());

    Utf8String ecSchemaXmlString;
    SchemaWriteStatus status2 = schema->WriteToXmlString(ecSchemaXmlString, ECVersion::V3_1);
    EXPECT_EQ(SchemaWriteStatus::Success, status2);

    ECSchemaPtr deserializedSchema;
    auto schemaContext = ECSchemaReadContext::CreateContext();
    auto status3 = ECSchema::ReadFromXmlString(deserializedSchema, ecSchemaXmlString.c_str(), *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status3);

    ECEnumerationP deserializedEnumeration;
    deserializedEnumeration = deserializedSchema->GetEnumerationP("Enumeration");
    ASSERT_TRUE(deserializedEnumeration != nullptr);
    EXPECT_STREQ("dl", deserializedEnumeration->GetDisplayLabel().c_str());
    EXPECT_STREQ("de", deserializedEnumeration->GetDescription().c_str());
    EXPECT_STREQ("string", deserializedEnumeration->GetTypeName().c_str());

    EXPECT_TRUE(deserializedEnumeration->GetEnumeratorCount() == 2);
    enumerator = deserializedEnumeration->FindEnumerator("First");
    EXPECT_TRUE(enumerator != nullptr);
    EXPECT_STREQ("First Value", enumerator->GetInvariantDisplayLabel().c_str());
    EXPECT_FALSE(deserializedEnumeration->GetIsStrict());

    ECClassCP deserializedClass = deserializedSchema->GetClassCP("EntityClass");
    ECPropertyP deserializedProperty = deserializedClass->GetPropertyP("EnumProperty");
    EXPECT_STREQ("Enumeration", deserializedProperty->GetTypeName().c_str());
    PrimitiveECPropertyCP deserializedPrimitive = deserializedProperty->GetAsPrimitiveProperty();
    ASSERT_TRUE(nullptr != deserializedPrimitive);

    ECPropertyP deserializedArrayProperty = deserializedClass->GetPropertyP("EnumArrProperty");
    EXPECT_STREQ("Enumeration", deserializedArrayProperty->GetTypeName().c_str());
    PrimitiveArrayECPropertyCP deserializedPrimitiveArray = deserializedArrayProperty->GetAsPrimitiveArrayProperty();
    ASSERT_TRUE(nullptr != deserializedPrimitiveArray);

    ECEnumerationCP propertyEnumeration = deserializedPrimitive->GetEnumeration();
    ASSERT_TRUE(nullptr != propertyEnumeration);
    EXPECT_STREQ("string", propertyEnumeration->GetTypeName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenRoundtripKindOfQuantityUsingString)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);
    ASSERT_TRUE(schema.IsValid());

    KindOfQuantityP kindOfQuantity;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateKindOfQuantity(kindOfQuantity, "MyKindOfQuantity"));
    kindOfQuantity->SetDescription("DESC");
    kindOfQuantity->SetDisplayLabel("DL");
    kindOfQuantity->SetPersistenceUnit("CM");
    kindOfQuantity->SetRelativeError(10e-3);
    kindOfQuantity->SetDefaultPresentationUnit("FT");
    auto& altPresUnits = kindOfQuantity->GetPresentationUnitListR();
    altPresUnits.push_back("IN");
    altPresUnits.push_back("MILLIINCH");

    ECEntityClassP entityClass;
    ASSERT_TRUE(schema->CreateEntityClass(entityClass, "EntityClass") == ECObjectsStatus::Success);
    PrimitiveArrayECPropertyP property;
    auto status = entityClass->CreatePrimitiveArrayProperty(property, "QuantifiedProperty", PrimitiveType::PRIMITIVETYPE_Double);
    ASSERT_TRUE(status == ECObjectsStatus::Success);
    ASSERT_TRUE(property != nullptr);
    property->SetKindOfQuantity(kindOfQuantity);

    Utf8String ecSchemaXmlString;
    SchemaWriteStatus status2 = schema->WriteToXmlString(ecSchemaXmlString, ECVersion::V3_1);
    EXPECT_EQ(SchemaWriteStatus::Success, status2);

    ECSchemaPtr deserializedSchema;
    auto schemaContext = ECSchemaReadContext::CreateContext();
    auto status3 = ECSchema::ReadFromXmlString(deserializedSchema, ecSchemaXmlString.c_str(), *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status3);

    EXPECT_EQ(1, deserializedSchema->GetKindOfQuantityCount());
    KindOfQuantityCP deserializedKindOfQuantity;
    deserializedKindOfQuantity = deserializedSchema->GetKindOfQuantityCP("MyKindOfQuantity");
    ASSERT_TRUE(deserializedKindOfQuantity != nullptr);
    EXPECT_STREQ("DL", deserializedKindOfQuantity->GetDisplayLabel().c_str());
    EXPECT_STREQ("DESC", deserializedKindOfQuantity->GetDescription().c_str());
    EXPECT_STREQ("CM", deserializedKindOfQuantity->GetPersistenceUnit().GetUnit()->GetName());
    EXPECT_EQ(10e-3, deserializedKindOfQuantity->GetRelativeError());

    EXPECT_STREQ("FT", deserializedKindOfQuantity->GetDefaultPresentationUnit().GetUnit()->GetName());
    auto& resultAltUnits = deserializedKindOfQuantity->GetPresentationUnitList();
    EXPECT_EQ(3, resultAltUnits.size()); // Default presentation unit is included in list of presentation units
    EXPECT_STREQ("IN", resultAltUnits[1].GetUnit()->GetName());
    EXPECT_STREQ("MILLIINCH", resultAltUnits[2].GetUnit()->GetName());

    ECClassCP deserializedClass = deserializedSchema->GetClassCP("EntityClass");
    ECPropertyP deserializedProperty = deserializedClass->GetPropertyP("QuantifiedProperty");
    KindOfQuantityCP propertyKoq = deserializedProperty->GetAsPrimitiveArrayProperty()->GetKindOfQuantity();
    ASSERT_TRUE(nullptr != propertyKoq);
    EXPECT_EQ(propertyKoq, deserializedKindOfQuantity);
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
// @bsimethod                                   Carole.MacDonald            12/2016
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
* @bsimethod                                                  Raimondas.Rimkus 02/2013
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
// @bsimethod                                   Carole.MacDonald            11/2015
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
* @bsimethod                                                    Paul.Connelly   11/12
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWithEnumerationInReferencedSchema)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='ReferencedSchema' version='01.00' displayLabel='Display Label' description='Description' nameSpacePrefix='ref' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECEnumeration typeName='RevisionStatus' backingTypeName='int' description='...' displayLabel='Revision Status' isStrict='False'>"
        "       <ECEnumerator value='0' displayLabel='Undefined' />"
        "       <ECEnumerator value='1' displayLabel='Planned' />"
        "       <ECEnumerator value='2' displayLabel='Not Approved' />"
        "       <ECEnumerator value='3' displayLabel='Approved' />"
        "       <ECEnumerator value='4' displayLabel='Previous Revision' />"
        "       <ECEnumerator value='5' displayLabel='Obsolete' />"
        "   </ECEnumeration>"
        "</ECSchema>";
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr refSchema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(refSchema, schemaXML, *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);

    schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Stuff' version='09.06' displayLabel='Display Label' description='Description' nameSpacePrefix='stuff' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "<ECSchemaReference name='ReferencedSchema' version='01.00' prefix='ref' />"
        "   <ECClass typeName='Document' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='Name' typeName='string' displayLabel='Title' />"
        "       <ECProperty propertyName='DateEffective' typeName='dateTime' displayLabel='Date Effective' />"
        "       <ECProperty propertyName='DateObsolete' typeName='dateTime' displayLabel='Date Obsolete' />"
        "       <ECProperty propertyName='IsTemplate' typeName='boolean' />"
        "       <ECProperty propertyName='RevisionStatus' typeName='ref:RevisionStatus' displayLabel='Revision Status' />"
        "   </ECClass>"
        "</ECSchema>";
    ECSchemaPtr schema;
    status = ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);

    ECEnumerationP enumeration;
    enumeration = refSchema->GetEnumerationP("RevisionStatus");
    ASSERT_TRUE(enumeration != nullptr);
    EXPECT_STREQ("int", enumeration->GetTypeName().c_str());
    EXPECT_FALSE(enumeration->GetIsStrict());

    ECClassCP documentClass = schema->GetClassCP("Document");
    ECPropertyP deserializedProperty = documentClass->GetPropertyP("RevisionStatus");
    EXPECT_STREQ("ref:RevisionStatus", deserializedProperty->GetTypeName().c_str());
    PrimitiveECPropertyCP deserializedPrimitive = deserializedProperty->GetAsPrimitiveProperty();
    ASSERT_TRUE(nullptr != deserializedPrimitive);

    ECEnumerationCP propertyEnumeration = deserializedPrimitive->GetEnumeration();
    ASSERT_TRUE(nullptr != propertyEnumeration);
    EXPECT_STREQ("int", propertyEnumeration->GetTypeName().c_str());
    ASSERT_TRUE(enumeration == propertyEnumeration);

    EXPECT_TRUE(enumeration->GetEnumeratorCount() == 6);
    ECEnumeratorP ecEnumerator = enumeration->FindEnumerator(3);
    EXPECT_TRUE(ecEnumerator != nullptr);
    EXPECT_TRUE(ecEnumerator->GetInteger() == 3);
    Utf8StringCR displayLabel = ecEnumerator->GetDisplayLabel();
    EXPECT_STREQ(displayLabel.c_str(), "Approved");
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   06/15
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaDeserializationTest, TestMultipleConstraintClasses)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0' schemaName='ReferencedSchema' nameSpacePrefix='ref' version='01.00' description='Description' displayLabel='Display Label' xmlns:ec='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "  <ECClass typeName = 'Class' isDomainClass = 'True'>"
        "      <ECProperty propertyName = 'Property' typeName = 'string' />"
        "  </ECClass>"
        "  <ECClass typeName = 'Class1' isDomainClass = 'True'>"
        "      <ECProperty propertyName = 'Property1' typeName = 'string' />"
        "      <ECProperty propertyName = 'Property2' typeName = 'string' />"
        "  </ECClass>"
        "  <ECClass typeName = 'Class2' isDomainClass = 'True'>"
        "      <ECProperty propertyName = 'Property3' typeName = 'string' />"
        "      <ECProperty propertyName = 'Property4' typeName = 'string' />"
        "  </ECClass>"
        "  <ECRelationshipClass typeName = 'ClassHasClass1Or2' isDomainClass = 'True' strength = 'referencing' strengthDirection = 'forward'>"
        "      <Source cardinality = '(0, 1)' polymorphic = 'True'>"
        "          <Class class = 'Class' />"
        "      </Source>"
        "      <Target cardinality = '(0, 1)' polymorphic = 'True'>"
        "          <Class class = 'Class1' />"
        "          <Class class = 'Class2' />"
        "      </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr ecSchema = nullptr;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(ecSchema, schemaXml, *schemaContext));
    ECRelationshipClassCP relClass = ecSchema->GetClassCP("ClassHasClass1Or2")->GetRelationshipClassCP();
    ASSERT_TRUE(relClass != nullptr);
    ASSERT_EQ(1, relClass->GetSource().GetConstraintClasses().size());
    ASSERT_STREQ("Class", relClass->GetSource().GetConstraintClasses()[0]->GetName().c_str());

    ASSERT_EQ(2, relClass->GetTarget().GetConstraintClasses().size());
    ASSERT_STREQ("Class1", relClass->GetTarget().GetConstraintClasses()[0]->GetName().c_str());
    ASSERT_STREQ("Class2", relClass->GetTarget().GetConstraintClasses()[1]->GetName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Basanta.Kharel   12/2015
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaDeserializationTest, KindOfQuantityTest)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "KindOfQuantitySchema", "koq", 5, 0, 6);
    ASSERT_TRUE(schema.IsValid());

    ECEntityClassP entityClass;
    schema->CreateEntityClass(entityClass, "Class");
    ASSERT_NE(entityClass, nullptr);

    PrimitiveECPropertyP prop;
    entityClass->CreatePrimitiveProperty(prop, "Property");
    ASSERT_NE(prop, nullptr);

    KindOfQuantityP koq;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateKindOfQuantity(koq, "MyKindOfQuantity"));
    koq->SetPersistenceUnit("FT(real)");
    prop->SetKindOfQuantity(koq);

    Utf8String schemaXML;
    EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(schemaXML, ECVersion::V3_1));

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr refSchema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(refSchema, schemaXML.c_str(), *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);

    ECClassCP entityClassDup = refSchema->GetClassCP("Class");
    ASSERT_NE(entityClassDup, nullptr);
    PrimitiveECPropertyCP property = entityClassDup->GetPropertyP("Property")->GetAsPrimitiveProperty();
    ASSERT_NE(property, nullptr);

    auto resultKindOfQuantity = property->GetKindOfQuantity();
    ASSERT_NE(resultKindOfQuantity, nullptr);
    EXPECT_STREQ("MyKindOfQuantity", resultKindOfQuantity->GetName().c_str());
    EXPECT_STREQ("FT(DefaultReal)", resultKindOfQuantity->GetPersistenceUnit().ToText(false).c_str());
    }


void ValidateElementOrder(bvector<Utf8String> expectedTypeNames, BeXmlNodeP root)
    {
    BeXmlNodeP currentNode = root->GetFirstChild();
    for (auto expectedTypeName : expectedTypeNames)
        {
        if (currentNode == nullptr)
            {
            FAIL() << "Expected end of document, Node '" << expectedTypeName << "' expected.";
            }

        Utf8String nodeTypeName;
        EXPECT_EQ(BeXmlStatus::BEXML_Success, currentNode->GetAttributeStringValue(nodeTypeName, "typeName"));
        EXPECT_EQ(expectedTypeName, nodeTypeName);

        currentNode = currentNode->GetNextSibling();
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Stefan.Apfel    02/2016
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
// @bsimethod                                                    Stefan.Apfel    02/2016
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

    // Enumerations (DEF) are serialized first, then classes (ABC, GHI)
    bvector<Utf8String> typeNames = {"DEF", "ABC", "GHI"};
    ValidateElementOrder(typeNames, xmlDom.get()->GetRootElement());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Stefan.Apfel    02/2016
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
// @bsimethod                                                    Stefan.Apfel    02/2016
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

    // First Enumeration (PQR), then classes alphabetically (ABC, DEF, GHI). As MNO is the base class of ABC and
    // JKL has a constraint in DEF, those two classes are written before the class they depend in.
    bvector<Utf8String> typeNames = {"PQR", "MNO", "ABC", "JKL", "DEF", "GHI"};
    ValidateElementOrder(typeNames, xmlDom.get()->GetRootElement());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Colin.Kerr    04/2016
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
    EXPECT_TRUE(schema2.IsNull()) << "Expected schema to be null on second call to ECSchema::ReadFromXmlFile";

    SchemaKey key(schema->GetSchemaKey());
    ECSchemaPtr schema3 = schemaContext->LocateSchema(key, SchemaMatchType::Exact);
    EXPECT_TRUE(schema3.IsValid()) << "Expected a valid schema when calling ECSchemaReadContext::LocateSchema after schema was loaded using ECSchema::ReadFromXmlFile";
    VerifyWidgetsSchema(schema3);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    08/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenDerivedClassComesBeforeBaseClass)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='C' modifier='sealed'>"
        "       <BaseClass>B</BaseClass>"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='A' modifier='abstract'></ECEntityClass>"
        "   <ECEntityClass typeName='B' modifier='abstract'></ECEntityClass>"
        "   <ECRelationshipClass typeName='ARelC' modifier='sealed'>"
        "       <BaseClass>ARelB</BaseClass>"
        "       <Source multiplicity='(1..1)' polymorphic='True' roleLabel='testSource' >"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(1..*)' polymorphic='True' roleLabel='testTarget' >"
        "           <Class class='C' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "   <ECRelationshipClass typeName='ARelB' modifier='abstract'>"
        "       <Source multiplicity='(1..1)' polymorphic='True' roleLabel='testSource' >"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..*)' polymorphic='True' roleLabel='testTarget'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ASSERT_TRUE(schema.IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    08/2016
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
// @bsimethod                                                    Caleb.Shafer    12/2016
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
// @bsimethod                                                    Caleb.Shafer    09/2016
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
// @bsimethod                                                    Caleb.Shafer    09/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, ExpectFailureWhenMultiplicityNotFoundOrEmpty)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();

    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'/>"
        "   <ECRelationshipClass typeName='ARelB' modifier='abstract'>"
        "       <Source polymorphic='True' roleLabel='test'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target polymorphic='True' roleLabel='test'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "ECRelationshipConstraint without a mutliplicity attribute is supposed to fail to deserialize.";

    Utf8CP schemaXml2 = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'/>"
        "   <ECRelationshipClass typeName='ARelB' modifier='abstract'>"
        "       <Source multiplicity='' polymorphic='True' roleLabel='test'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='' polymorphic='True' roleLabel='test'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema2;
    status = ECSchema::ReadFromXmlString(schema2, schemaXml2, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "ECRelationshipConstraint with an empty mutliplicity attribute is supposed to fail to deserialize.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    09/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, ExpectFailureWhenPolymorphicNotFoundOrEmpty)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();

    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'/>"
        "   <ECRelationshipClass typeName='ARelB' modifier='abstract'>"
        "       <Source multiplicity='(0..1)' roleLabel='test'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..1)' roleLabel='test'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "ECRelationshipConstraint without a polymorphic attribute is supposed to fail to deserialize.";

    Utf8CP schemaXml2 = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'/>"
        "   <ECRelationshipClass typeName='ARelB' modifier='abstract'>"
        "       <Source multiplicity='(0..1)' polymorphic='' roleLabel='test'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..1)' polymorphic='' roleLabel='test'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema2;
    status = ECSchema::ReadFromXmlString(schema2, schemaXml2, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "ECRelationshipConstraint with an empty polymorphic attribute is supposed to fail to deserialize.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    09/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, ExpectFailureWhenRoleLabelNotFoundOrEmpty)
    {
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'/>"
        "   <ECRelationshipClass typeName='ARelB' modifier='abstract'>"
        "       <Source multiplicity='(0..1)' polymorphic='True' >"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..1)' polymorphic='True' >"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "ECRelationshipConstraint without a roleLabel attribute is supposed to fail to deserialize.";
    }

    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'/>"
        "   <ECRelationshipClass typeName='ARelB' modifier='abstract'>"
        "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='' >"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..1)' polymorphic='True' roleLabel='' >"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "ECRelationshipConstraint with an empty roleLabel attribute is supposed to fail to deserialize.";
    }
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'/>"
        "   <ECEntityClass typeName='C' modifier='abstract'>"
        "       <BaseClass>B</BaseClass>"
        "   </ECEntityClass>"
        "   <ECRelationshipClass typeName='ARelB' modifier='abstract'>"
        "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='' >"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..1)' polymorphic='True' roleLabel='' >"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "   <ECRelationshipClass typeName='ARelC' modifier='abstract'>"
        "       <BaseClass>ARelC</BaseClass>"
        "       <Source multiplicity='(0..1)' polymorphic='True' >"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..1)' polymorphic='True' >"
        "           <Class class='C' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "ECRelationshipConstraint with an empty roleLabel attribute is supposed to fail to deserialize.";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    09/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenKindOfQuantityIsAppliedToStructAndStructArrayPropertes)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'>"
        "       <ECStructProperty propertyName='StructWithKOQ' typeName='S' kindOfQuantity='MyKindOfQuantity' />"
        "       <ECStructArrayProperty propertyName='StructArrayWithKOQ' typeName='S' kindOfQuantity='MyKindOfQuantity' />"
        "   </ECEntityClass>"
        "   <ECStructClass typeName='S'>"
        "       <ECProperty propertyName='S_P' typeName='string' />"
        "   </ECStructClass>"
        "   <KindOfQuantity typeName='MyKindOfQuantity' description='Kind of a Description here' "
        "       displayLabel='best quantity of all times' persistenceUnit='CM' relativeError='10e-3' "
        "       presentationUnits='FT;IN;MILLIINCH'/>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status) << "ECSchema failed to deserialize.";
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->IsECVersion(ECVersion::V3_1));

    ECClassCP aClass = schema->GetClassCP("A");
    ASSERT_NE(nullptr, aClass) << "Could not find 'A' class";
    ECPropertyP structProp = aClass->GetPropertyP("StructWithKOQ");
    ASSERT_NE(nullptr, structProp) << "Can't find 'StructWithKOQ' property";
    KindOfQuantityCP koq = structProp->GetKindOfQuantity();
    ASSERT_NE(nullptr, koq) << "'StructWithKOQ' property does not have a KOQ as expected";
    ECPropertyP structArrayProp = aClass->GetPropertyP("StructArrayWithKOQ");
    ASSERT_NE(nullptr, structArrayProp) << "Can't find 'StructArrayWithKOQ' property";
    koq = structArrayProp->GetKindOfQuantity();
    ASSERT_NE(nullptr, koq) << "'StructArrayWithKOQ' property does not have a KOQ as expected";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    09/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenKindOfQuantityInherited)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'>"
        "       <ECProperty propertyName='KindOfQuantityProperty' typeName='double' kindOfQuantity='MyKindOfQuantity' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='B' modifer='abstract'>"
        "       <BaseClass>A</BaseClass>"
        "       <ECProperty propertyName='KindOfQuantityProperty' typeName='double' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='C' modifer='sealed'>"
        "       <BaseClass>B</BaseClass>"
        "       <ECProperty propertyName='KindOfQuantityProperty' typeName='double' />"
        "   </ECEntityClass>"
        "   <KindOfQuantity typeName='MyKindOfQuantity' description='Kind of a Description here' "
        "       displayLabel='best quantity of all times' persistenceUnit='CM' relativeError='10e-3' "
        "       presentationUnits='FT;IN;MILLIINCH'/>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status) << "ECSchema failed to deserialize.";
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->IsECVersion(ECVersion::V3_1));

    for (auto const& pClass : schema->GetClasses())
        {
        ECPropertyP p = pClass->GetPropertyP("KindOfQuantityProperty");
        EXPECT_TRUE(p != nullptr);

        PrimitiveECPropertyCP prim = p->GetAsPrimitiveProperty();
        EXPECT_TRUE(prim != nullptr);

        KindOfQuantityCP kindOfQuantity = prim->GetKindOfQuantity();
        EXPECT_TRUE(kindOfQuantity != nullptr);

        if (pClass->GetName() != "A")
            EXPECT_FALSE(prim->IsKindOfQuantityDefinedLocally()) << "The Kind of Quantity is defined in a base class, so it should not be defined locally.";
        else
            EXPECT_TRUE(prim->IsKindOfQuantityDefinedLocally()) << "The Kind of Quantity is defined, so it should be defined locally.";

        EXPECT_STREQ(kindOfQuantity->GetName().c_str(), "MyKindOfQuantity");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    09/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenAbstractConstraintValid)
    {
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'>"
        "       <BaseClass>A</BaseClass>"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='C' modifier='abstract'>"
        "       <BaseClass>A</BaseClass>"
        "   </ECEntityClass>"
        "   <ECRelationshipClass typeName='ARelB' modifier='abstract'>"
        "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='testSource' >"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..1)' polymorphic='True' roleLabel='testTarget' abstractConstraint='A'>"
        "           <Class class='B' />"
        "           <Class class='C' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ASSERT_TRUE(schema.IsValid());
    
    EXPECT_STREQ("A", schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetSource().GetAbstractConstraint()->GetName().c_str()) << "The Source constraint's abstract constraint is implicitly defined to be A.";
    EXPECT_STREQ("A", schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetTarget().GetAbstractConstraint()->GetName().c_str()) << "The Target constraint's abstract constraint is explicitly defined to be B.";
    }
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'/>"
        "   <ECRelationshipClass typeName='ARelB' modifier='abstract'>"
        "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='testSource' >"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..1)' polymorphic='True' roleLabel='testTarget' >"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ASSERT_TRUE(schema.IsValid());
    
    EXPECT_STREQ("A", schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetSource().GetAbstractConstraint()->GetName().c_str()) << "The Source constraint's abstract constraint is implicitly defined to be A.";
    EXPECT_STREQ("B", schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetTarget().GetAbstractConstraint()->GetName().c_str()) << "The Target constraint's abstract constraint is implicitly defined to be B.";
    }
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'/>"
        "   <ECEntityClass typeName='C' modifier='abstract'>"
        "       <BaseClass>B</BaseClass>"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='CB' modifier='abstract'>"
        "       <BaseClass>B</BaseClass>"
        "       <BaseClass>C</BaseClass>"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='AB' modifier='abstract'>"
        "       <BaseClass>A</BaseClass>"
        "       <BaseClass>B</BaseClass>"
        "   </ECEntityClass>"
        "   <ECRelationshipClass typeName='ARelB' modifier='abstract'>"
        "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='testSource' >"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..1)' polymorphic='True' roleLabel='testTarget' abstractConstraint='B'>"
        "           <Class class='B' />"
        "           <Class class='C' />"
        "           <Class class='CB' />"
        "           <Class class='AB' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ASSERT_TRUE(schema.IsValid());

    EXPECT_STREQ("A", schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetSource().GetAbstractConstraint()->GetName().c_str()) << "The Source constraint's abstract constraint is implicitly defined to be A.";
    EXPECT_STREQ("B", schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetTarget().GetAbstractConstraint()->GetName().c_str()) << "The Target constraint's abstract constraint should be explicitly defined to B.";
    }
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'/>"
        "   <ECEntityClass typeName='C' modifier='abstract'>"
        "       <BaseClass>B</BaseClass>"
        "   </ECEntityClass>"
        "   <ECRelationshipClass typeName='ARelB' modifier='abstract'>"
        "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='testSource' >"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..1)' polymorphic='True' roleLabel='testTarget' abstractConstraint='B'>"
        "           <Class class='C' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ASSERT_TRUE(schema.IsValid());

    EXPECT_STREQ("A", schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetSource().GetAbstractConstraint()->GetName().c_str()) << "The Source constraint's abstract constraint is implicitly defined to be A.";
    EXPECT_STREQ("B", schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetTarget().GetAbstractConstraint()->GetName().c_str()) << "The Target constraint's abstract constraint is explicitly defined to B.";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    09/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, ExpectFailureWhenAbstractConstraintNotFoundOrEmpty)
    {
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'/>"
        "   <ECEntityClass typeName='C' modifier='abstract'>"
        "       <BaseClass>B</BaseClass>"
        "   </ECEntityClass>"
        "   <ECRelationshipClass typeName='ARelB' modifier='abstract'>"
        "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='testSource' >"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..1)' polymorphic='True' roleLabel='testTarget' >"
        "           <Class class='B' />"
        "           <Class class='C' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "The schema should fail to deserialize when an abstractConstraint attribute is not found and multiple constraint classes are defined.";
    }
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'/>"
        "   <ECRelationshipClass typeName='ARelB' modifier='abstract'>"
        "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='testSource' abstractConstraint='' >"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..1)' polymorphic='True' roleLabel='testTarget' abstractConstraint='' >"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "The schema should fail to deserialize because the abstractConstraint is empty.";
    }
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'/>"
        "   <ECEntityClass typeName='C' modifier='abstract'>"
        "       <BaseClass>B</BaseClass>"
        "   </ECEntityClass>"
        "   <ECRelationshipClass typeName='ARelB' modifier='abstract'>"
        "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='testSource' abstractConstraint='D' >"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..1)' polymorphic='True' roleLabel='testTarget' abstractConstraint='E' >"
        "           <Class class='B' />"
        "           <Class class='C' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "The schema should fail to deserialize because the abstractConstraint class do not exist.";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    09/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, ExpectFailureWhenAbstractConstraintViolatesNarrowing)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'/>"
        "   <ECEntityClass typeName='C' modifier='abstract'>"
        "       <BaseClass>B</BaseClass>"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='D' modifier='sealed'>"
        "       <BaseClass>C</BaseClass>"
        "   </ECEntityClass>"
        "   <ECRelationshipClass typeName='DerivedARelD' modifier='abstract'>"
        "       <BaseClass>ARelD</BaseClass>"
        "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='testSource' >"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..1)' polymorphic='True' roleLabel='testTarget' abstractConstraint='B' >"
        "           <Class class='D' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "   <ECRelationshipClass typeName='ARelD' modifier='abstract'>"
        "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='testSource' abstractConstraint='A'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..1)' polymorphic='True' roleLabel='testTarget' abstractConstraint='C' >"
        "           <Class class='D' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_NE(SchemaReadStatus::Success, status) << "The schema should fail because DerivedARelD has a more base class as the target abstract constraint then it's base class ARelD.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    11/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, ExpectFailureWhenRelationshipConstraintsEmpty)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'/>"
        "   <ECRelationshipClass typeName='ARelD' modifier='abstract'>"
        "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='testSource' abstractConstraint='A'>"
        "       </Source>"
        "       <Target multiplicity='(0..1)' polymorphic='True' roleLabel='testTarget' abstractConstraint='B' >"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_NE(SchemaReadStatus::Success, status) << "The schema should fail because DerivedARelD has a more base class as the target abstract constraint then it's base class ARelD.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    12/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, TestMultiplicityValidation)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'/>"
        "   <ECEntityClass typeName='C' modifier='abstract'>"
        "       <BaseClass>B</BaseClass>"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='D' modifier='abstract'>"
        "       <BaseClass>C</BaseClass>"
        "   </ECEntityClass>"
        "   <ECRelationshipClass typeName='BRelD' modifier='sealed'>"
        "       <BaseClass>ARelB</BaseClass>"
        "       <Source multiplicity='(1..1)' polymorphic='True' roleLabel='testSource' abstractConstraint='A'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..*)' polymorphic='True' roleLabel='testTarget' abstractConstraint='B' >"
        "           <Class class='D' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "   <ECRelationshipClass typeName='ARelB' modifier='abstract'>"
        "       <Source multiplicity='(1..1)' polymorphic='True' roleLabel='testSource' abstractConstraint='A'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..*)' polymorphic='True' roleLabel='testTarget' abstractConstraint='B' >"
        "           <Class class='C' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *context);
    ASSERT_EQ(SchemaReadStatus::Success, status);

    ECClassCP ecClass = schema->GetClassCP("ARelB");
    ASSERT_TRUE(nullptr != ecClass);
    ECRelationshipClassCP relClass = ecClass->GetRelationshipClassCP();
    ASSERT_TRUE(nullptr != relClass);

    ECClassCP derivedClass = schema->GetClassCP("BRelD");
    ASSERT_TRUE(nullptr != derivedClass);
    ECRelationshipClassCP derivedRelClass = ecClass->GetRelationshipClassCP();
    ASSERT_TRUE(nullptr != derivedRelClass);

    EXPECT_EQ(0, RelationshipMultiplicity::Compare(relClass->GetSource().GetMultiplicity(), RelationshipMultiplicity::OneOne()));
    EXPECT_EQ(0, RelationshipMultiplicity::Compare(relClass->GetTarget().GetMultiplicity(), RelationshipMultiplicity::ZeroMany()));
    EXPECT_EQ(0, RelationshipMultiplicity::Compare(derivedRelClass->GetSource().GetMultiplicity(), RelationshipMultiplicity::OneOne()));
    EXPECT_EQ(0, RelationshipMultiplicity::Compare(derivedRelClass->GetTarget().GetMultiplicity(), RelationshipMultiplicity::ZeroMany()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    12/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, TestFailureWhenRelationshipClassModifierNotFound)
    {
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A'/>"
        "   <ECEntityClass typeName='B'/>"
        "   <ECRelationshipClass typeName='ARelB' >"
        "       <Source multiplicity='(1..1)' polymorphic='True' roleLabel='testSource'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..*)' polymorphic='True' roleLabel='testTarget'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *context);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "The schema did not fail to deserialize even though the relationship class is missing the modifier attribute.";
    }
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A'/>"
        "   <ECEntityClass typeName='B'/>"
        "   <ECRelationshipClass typeName='ARelB' modifier=''>"
        "       <Source multiplicity='(1..1)' polymorphic='True' roleLabel='testSource'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..*)' polymorphic='True' roleLabel='testTarget'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *context);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "The schema did not fail to deserialize even though the relationship class modifier attribute is empty.";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    12/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, TestFailureWithRelationshipKeys)
    {
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A'/>"
        "   <ECEntityClass typeName='B'/>"
        "   <ECRelationshipClass typeName='ARelB' modifier='Sealed'>"
        "       <Source multiplicity='(1..1)' polymorphic='True' roleLabel='testSource'>"
        "           <Class class='A'>"
        "               <Key>"
        "                   <Property name='Property' />"
        "               </Key>"
        "           </Class>"
        "       </Source>"
        "       <Target multiplicity='(0..*)' polymorphic='True' roleLabel='testTarget'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *context);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "The schema did not fail to deserialize even though the source constraint of the relationship class has a key property.";
    }
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A'/>"
        "   <ECEntityClass typeName='B'/>"
        "   <ECRelationshipClass typeName='ARelB' modifier='Sealed'>"
        "       <Source multiplicity='(1..1)' polymorphic='True' roleLabel='testSource'>"
        "           <Class class='A'/>"
        "       </Source>"
        "       <Target multiplicity='(0..*)' polymorphic='True' roleLabel='testTarget'>"
        "           <Class class='B' >"
        "               <Key>"
        "                   <Property name='Property' />"
        "               </Key>"
        "           </Class>"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *context);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "The schema did not fail to deserialize even though the target constraint of the relationship class has a key property.";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    12/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, TestSuccessWithRelationshipKeysFromEC2)
    {
    // This test is only to make sure de-serialization does not fail when 2.0/3.0 schemas have key properties.
    // Even though they are allowed to have them they are dropped and there is no way to access
    // them via the API
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' nameSpacePrefix='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A'/>"
        "   <ECEntityClass typeName='B'/>"
        "   <ECRelationshipClass typeName='ARelB' >"
        "       <Source multiplicity='(1..1)' polymorphic='True' roleLabel='testSource'>"
        "           <Class class='A'>"
        "               <Key>"
        "                   <Property name='Property' />"
        "               </Key>"
        "           </Class>"
        "       </Source>"
        "       <Target multiplicity='(0..*)' polymorphic='True' roleLabel='testTarget'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *context);
    ASSERT_EQ(SchemaReadStatus::Success, status) << "The 3.0 schema failed even though relationship classes can have key properties.";
    }
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' nameSpacePrefix='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECClass typeName='A' isDomainClass='True' />"
        "   <ECClass typeName='B' isDomainClass='True' />"
        "   <ECRelationshipClass typeName='ARelB' isDomainClass='True' >"
        "       <Source multiplicity='(1..1)' polymorphic='True' roleLabel='testSource'>"
        "           <Class class='A'>"
        "               <Key>"
        "                   <Property name='Property' />"
        "               </Key>"
        "           </Class>"
        "       </Source>"
        "       <Target multiplicity='(0..*)' polymorphic='True' roleLabel='testTarget'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *context);
    ASSERT_EQ(SchemaReadStatus::Success, status) << "The 2.0 schema failed even though relationship classes can have key properties.";
    }
    }

END_BENTLEY_ECN_TEST_NAMESPACE
