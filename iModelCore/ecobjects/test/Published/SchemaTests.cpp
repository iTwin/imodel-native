/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/SchemaTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

// NEEDSWORK Improve strategy for seed data.  Should not be maintained in source.
#define SCHEMAS_PATH  L"" 

struct SchemaNameParsingTest     : ECTestFixture {};
struct SchemaDeserializationTest : ECTestFixture {};
struct SchemaSerializationTest   : ECTestFixture {};
struct SchemaReferenceTest       : ECTestFixture {};
struct SchemaCreationTest        : ECTestFixture {};
struct SchemaCopyTest            : ECTestFixture {};
struct SchemaLocateTest          : ECTestFixture {};

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct ClassTest                 : ECTestFixture
    {
    void TestPropertyCount (ECClassCR ecClass, size_t nPropertiesWithoutBaseClasses, size_t nPropertiesWithBaseClasses)
        {
        EXPECT_EQ (ecClass.GetPropertyCount (false), nPropertiesWithoutBaseClasses);
        EXPECT_EQ (ecClass.GetPropertyCount (true), nPropertiesWithBaseClasses);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyP GetPropertyByName (ECClassCR ecClass, Utf8CP name, bool expectExists = true)
    {
    ECPropertyP prop = ecClass.GetPropertyP (name);
    EXPECT_EQ (expectExists, NULL != prop);
    Utf8String utf8 (name);
    prop = ecClass.GetPropertyP (utf8.c_str());
    EXPECT_EQ (expectExists, NULL != prop);
    return prop;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void VerifyWidgetsSchema
(
ECSchemaPtr const&   schema
)
    {
    EXPECT_STREQ ("Widgets", schema->GetName().c_str());
    EXPECT_STREQ ("wid", schema->GetNamespacePrefix().c_str());
    EXPECT_STREQ ("Widgets Display Label", schema->GetDisplayLabel().c_str());
    EXPECT_TRUE (schema->GetIsDisplayLabelDefined());
    EXPECT_STREQ ("Widgets Description", schema->GetDescription().c_str());
    EXPECT_EQ (9, schema->GetVersionMajor());
    EXPECT_EQ (6, schema->GetVersionMinor());        
 
#ifdef DEBUG_PRINT
    for (ECClassP pClass: schema->GetClasses())
        {
        printf ("Widgets contains class: '%s' with display label '%s'\n", pClass->GetName().c_str(), pClass->GetDisplayLabel().c_str());
        }
#endif

    ECClassP pClass = schema->GetClassP("ClassDoesNotExistInSchema");
    EXPECT_FALSE (pClass);

    pClass = schema->GetClassP("ecProject");
    ASSERT_TRUE (NULL != pClass);
    EXPECT_STREQ ("ecProject", pClass->GetName().c_str());    
    EXPECT_STREQ ("Project", pClass->GetDisplayLabel().c_str());
    EXPECT_TRUE (pClass->GetIsDisplayLabelDefined());
    EXPECT_STREQ ("Project Class", pClass->GetDescription().c_str());
    EXPECT_FALSE (pClass->GetIsStruct());
    EXPECT_FALSE (pClass->GetIsCustomAttributeClass());
    EXPECT_TRUE (pClass->GetIsDomainClass());
    EXPECT_FALSE (pClass->HasBaseClasses());
    ECPropertyP pProperty = GetPropertyByName (*pClass, "Name");
    ASSERT_TRUE (NULL != pProperty);
    EXPECT_STREQ ("Name", pProperty->GetName().c_str());
    EXPECT_TRUE (pProperty->GetIsPrimitive());
    EXPECT_FALSE (pProperty->GetIsStruct());
    EXPECT_FALSE (pProperty->GetIsArray());
    EXPECT_STREQ ("string", pProperty->GetTypeName().c_str());
    EXPECT_TRUE (PRIMITIVETYPE_String == pProperty->GetAsPrimitiveProperty()->GetType());
    EXPECT_TRUE (pProperty->GetIsDisplayLabelDefined());
    EXPECT_STREQ ("Project Name", pProperty->GetDisplayLabel().c_str());
    EXPECT_STREQ ("", pProperty->GetDescription().c_str());
    EXPECT_EQ (pClass, &pProperty->GetClass());
    EXPECT_FALSE (pProperty->GetIsReadOnly());

    pProperty = GetPropertyByName (*pClass, "PropertyDoesNotExistInClass", false);
    EXPECT_FALSE (pProperty);

    ECClassP customAttribClass = schema->GetClassP("AccessCustomAttributes");
    ASSERT_TRUE (NULL != customAttribClass);
    EXPECT_STREQ ("AccessCustomAttributes", customAttribClass->GetName().c_str());    
    EXPECT_STREQ ("AccessCustomAttributes", customAttribClass->GetDisplayLabel().c_str());
    EXPECT_FALSE (customAttribClass->GetIsDisplayLabelDefined());
    EXPECT_STREQ ("", customAttribClass->GetDescription().c_str());
    EXPECT_FALSE (customAttribClass->GetIsStruct());
    EXPECT_TRUE (customAttribClass->GetIsCustomAttributeClass());
    EXPECT_FALSE (customAttribClass->GetIsDomainClass());
    EXPECT_FALSE (customAttribClass->HasBaseClasses());

    pClass = schema->GetClassP("Struct1");
    ASSERT_TRUE (NULL != pClass);
    EXPECT_STREQ ("Struct1", pClass->GetName().c_str());    
    EXPECT_STREQ ("Struct1", pClass->GetDisplayLabel().c_str());
    EXPECT_FALSE (pClass->GetIsDisplayLabelDefined());
    EXPECT_STREQ ("", pClass->GetDescription().c_str());
    EXPECT_TRUE (pClass->GetIsStruct());
    EXPECT_FALSE (pClass->GetIsCustomAttributeClass());
    EXPECT_FALSE (pClass->GetIsDomainClass());
    EXPECT_FALSE (pClass->HasBaseClasses());

    pClass = schema->GetClassP("Struct2");
    ASSERT_TRUE (NULL != pClass);
    EXPECT_STREQ ("Struct2", pClass->GetName().c_str());    
    EXPECT_STREQ ("Struct2", pClass->GetDisplayLabel().c_str());
    EXPECT_FALSE (pClass->GetIsDisplayLabelDefined());
    EXPECT_STREQ ("", pClass->GetDescription().c_str());
    EXPECT_TRUE (pClass->GetIsStruct());
    EXPECT_FALSE (pClass->GetIsCustomAttributeClass());
    EXPECT_TRUE (pClass->GetIsDomainClass());
    EXPECT_FALSE (pClass->HasBaseClasses());
    pProperty = GetPropertyByName (*pClass, "NestedArray");
    EXPECT_TRUE (NULL != pProperty);
    EXPECT_STREQ ("NestedArray", pProperty->GetName().c_str());
    EXPECT_FALSE (pProperty->GetIsPrimitive());
    EXPECT_FALSE (pProperty->GetIsStruct());
    EXPECT_TRUE (pProperty->GetIsArray());
    EXPECT_STREQ ("Struct1", pProperty->GetTypeName().c_str());
    ArrayECPropertyP arrayProperty = pProperty->GetAsArrayPropertyP();
    EXPECT_TRUE (ARRAYKIND_Struct == arrayProperty->GetKind());
    EXPECT_EQ (schema->GetClassP("Struct1"), arrayProperty->GetStructElementType());
    EXPECT_EQ (0, arrayProperty->GetMinOccurs());
    EXPECT_EQ (UINT_MAX, arrayProperty->GetMaxOccurs());    
    EXPECT_FALSE (pProperty->GetIsDisplayLabelDefined());
    EXPECT_STREQ ("NestedArray", pProperty->GetDisplayLabel().c_str());
    EXPECT_STREQ ("", pProperty->GetDescription().c_str());
    EXPECT_EQ (pClass, &pProperty->GetClass());
    EXPECT_FALSE (pProperty->GetIsReadOnly());

    pClass = schema->GetClassP("TestClass");
    ASSERT_TRUE (NULL != pClass);
    EXPECT_TRUE (pClass->HasBaseClasses());
    pProperty = GetPropertyByName (*pClass, "EmbeddedStruct");
    ASSERT_TRUE (NULL != pProperty);
    EXPECT_STREQ ("EmbeddedStruct", pProperty->GetName().c_str());
    EXPECT_FALSE (pProperty->GetIsPrimitive());
    EXPECT_TRUE (pProperty->GetIsStruct());
    EXPECT_FALSE (pProperty->GetIsArray());
    EXPECT_STREQ ("Struct1", pProperty->GetTypeName().c_str());
    StructECPropertyP structProperty = pProperty->GetAsStructPropertyP();    
    EXPECT_EQ (schema->GetClassP("Struct1"), &(structProperty->GetType()));
    EXPECT_FALSE (pProperty->GetIsDisplayLabelDefined());
    EXPECT_STREQ ("EmbeddedStruct", pProperty->GetDisplayLabel().c_str());
    EXPECT_STREQ ("", pProperty->GetDescription().c_str());
    EXPECT_EQ (pClass, &pProperty->GetClass());
    EXPECT_FALSE (pProperty->GetIsReadOnly());
    
    IECInstancePtr instance = pClass->GetCustomAttribute(*customAttribClass);
    EXPECT_TRUE(instance.IsValid());

    ECValue ecValue;
    EXPECT_EQ (SUCCESS, instance->GetValue (ecValue, "AccessLevel"));
    EXPECT_EQ (4, ecValue.GetInteger());

    EXPECT_EQ (SUCCESS, instance->GetValue (ecValue, "Writeable"));
    EXPECT_FALSE (ecValue.GetBoolean());
   
#ifdef DEBUG_PRINT
    for (ECPropertyP pProperty: pClass->GetProperties())
        {
        printf ("TestClass contains property: %s of type %s\n", pProperty->GetName().c_str(), pProperty->GetTypeName().c_str());
        }
#endif   
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
static  void    ValidateSchemaNameParsing (Utf8CP fullName, bool expectFailure, Utf8CP expectName, uint32_t expectMajor, uint32_t expectMinor)
    {
    Utf8String    shortName;
    Utf8String    shortNameStr;
    uint32_t   versionMajor;
    uint32_t   versionMinor;
    Utf8String    fullNameStr = Utf8String(fullName);

    ECObjectsStatus status = ECSchema::ParseSchemaFullName (shortName, versionMajor, versionMinor, fullName);
    ECObjectsStatus statusStr = ECSchema::ParseSchemaFullName (shortNameStr, versionMajor, versionMinor, fullNameStr);

    if (expectFailure)
        {
        EXPECT_TRUE (ECOBJECTS_STATUS_Success != status);
        EXPECT_TRUE (ECOBJECTS_STATUS_Success != statusStr);
        return;
        }
    
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == status);
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == statusStr);
    
    EXPECT_STREQ (shortName.c_str(), expectName);
    EXPECT_STREQ (shortNameStr.c_str(), expectName);
    EXPECT_EQ    (versionMajor,      expectMajor);
    EXPECT_EQ    (versionMinor,      expectMinor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaNameParsingTest, ParseFullSchemaName)
    {
    ValidateSchemaNameParsing ("TestName.6.8",      false, "TestName", 6, 8);
    ValidateSchemaNameParsing ("TestName.16.18",    false, "TestName", 16, 18);
    ValidateSchemaNameParsing ("TestName.126.128",  false, "TestName", 126, 128);
    ValidateSchemaNameParsing ("TestName.1267.128", true,  NULL, 0, 0);
    ValidateSchemaNameParsing ("TestName.1267",     true,  NULL, 0, 0);
    ValidateSchemaNameParsing ("TestName",          true,  NULL, 0, 0);
    ValidateSchemaNameParsing ("",                  true,  NULL, 0, 0);
    ValidateSchemaNameParsing ("12.18",             true,  NULL, 0, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectErrorWhenXmlFileDoesNotExist)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling (true, false);

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath(L"ThisFileIsntReal.xml").c_str(), *schemaContext);

    EXPECT_EQ (SCHEMA_READ_STATUS_FailedToParseXml, status);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectErrorWhenXmlFileIsMissingNodes)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling (true, false);
    DISABLE_ASSERTS
    
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath( L"MissingNodes.01.00.ecschema.xml").c_str(), *schemaContext);  

    EXPECT_EQ (SCHEMA_READ_STATUS_FailedToParseXml, status);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectErrorWhenXmlFileIsIllFormed)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling (true, false);
    DISABLE_ASSERTS

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath( L"IllFormedXml.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ (SCHEMA_READ_STATUS_FailedToParseXml, status);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectErrorWhenXmlFileIsMissingECSchemaNode)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling (true, false);

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath( L"MissingECSchemaNode.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ (SCHEMA_READ_STATUS_InvalidECSchemaXml, status);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectErrorWhenXmlFileIsMissingNamespace)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling (true, false);

    
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath( L"MissingNamespace.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ (SCHEMA_READ_STATUS_InvalidECSchemaXml, status);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenCustomAttributeIsMissingNamespace)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling (true, false);

    
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath( L"MissingNamespaceOnCustomAttribute.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ (SCHEMA_READ_STATUS_Success, status);
    };
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectErrorWhenXmlFileHasUnsupportedNamespace)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling (true, false);

    
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath( L"UnsupportedECXmlNamespace.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ (SCHEMA_READ_STATUS_InvalidECSchemaXml, status);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectErrorWhenRelationshipEndpointNotFound)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling (true, false);

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath( L"BadRelationship.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ (SCHEMA_READ_STATUS_InvalidECSchemaXml, status);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWithDuplicateNamespacePrefixes)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling (true, false);

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath( L"DuplicatePrefixes.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ (SCHEMA_READ_STATUS_Success, status);
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectErrorWhenXmlFileHasMissingSchemaNameAttribute)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling (true, false);

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath( L"MissingSchemaName.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ (SCHEMA_READ_STATUS_InvalidECSchemaXml, status);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectErrorWhenXmlFileHasMissingClassNameAttribute)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling (true, false);

    
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath( L"MissingClassName.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ (SCHEMA_READ_STATUS_InvalidECSchemaXml, status);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenXmlFileHasInvalidVersionString)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling (true, false);

    
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath( L"InvalidVersionString.01.00.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ (1, schema->GetVersionMajor());
    EXPECT_EQ (0, schema->GetVersionMinor());

    EXPECT_EQ (SCHEMA_READ_STATUS_Success, status);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectFailureWhenMissingTypeNameInProperty)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling (true, false);

    
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString (schema, 
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"Widgets\" version=\"09.06\" displayLabel=\"Widgets Display Label\" description=\"Widgets Description\" nameSpacePrefix=\"wid\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" xmlns:ec=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" xmlns:ods=\"Bentley_ODS.01.02\">"
        "    <ECClass typeName=\"ecProject\" description=\"Project ECClass\" displayLabel=\"Project\" isDomainClass=\"True\">"
        "       <ECProperty propertyName=\"Name\" typename=\"string\" displayLabel=\"Project Name\" />" // typename is mis-capitalized
        "    </ECClass>"
        "</ECSchema>", *schemaContext);

    EXPECT_EQ (SCHEMA_READ_STATUS_InvalidECSchemaXml, status);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectUnrecognizedTypeNamesToSurviveRoundtrip)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString (schema, 
        "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='a' version='23.42' nameSpacePrefix='a' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECClass typeName='c'>"
        "       <ECProperty      propertyName='p' typeName='foobar'  />"
        "       <ECArrayProperty propertyName='q' typeName='barfood' minOccurs='0' maxOccurs='unbounded'/>"
        "    </ECClass>"
        "</ECSchema>", *schemaContext);
    EXPECT_EQ (SCHEMA_READ_STATUS_Success, status);

    Utf8String ecSchemaXml;
    SchemaWriteStatus writeStatus = schema->WriteToXmlString(ecSchemaXml);
    EXPECT_EQ (SCHEMA_WRITE_STATUS_Success, writeStatus);

    EXPECT_NE (Utf8String::npos, ecSchemaXml.find("typeName=\"foobar\""));
    EXPECT_NE (Utf8String::npos, ecSchemaXml.find("typeName=\"barfood\""));
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWithInvalidTypeNameInPrimitiveProperty)
    {
    
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString (schema, 
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"Widgets\" version=\"09.06\" displayLabel=\"Widgets Display Label\" description=\"Widgets Description\" nameSpacePrefix=\"wid\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" xmlns:ec=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" xmlns:ods=\"Bentley_ODS.01.02\">"
        "<ECSchemaReference name=\"EditorCustomAttributes\" version=\"01.00\" prefix=\"beca\" />"
        "    <ECClass typeName=\"ecProject\" description=\"Project ECClass\" displayLabel=\"Project\" isDomainClass=\"True\">"
        "       <ECProperty propertyName=\"Name\" typeName=\"strng\" displayLabel=\"Project Name\" />"
        "    </ECClass>"
        "</ECSchema>", *schemaContext);

    EXPECT_EQ (SCHEMA_READ_STATUS_Success, status);

    ECClassP pClass = schema->GetClassP("ecProject");
    ECPropertyP pProperty = GetPropertyByName (*pClass, "Name");
    EXPECT_TRUE (PRIMITIVETYPE_String == pProperty->GetAsPrimitiveProperty()->GetType());
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWithEmptyCustomAttribute)
    {
    
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;

    //schemaContext->AddSchemaPath(L"C:\\temp\\data\\ECXA\\SchemasAndDgn\\");
    //SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, L"C:\\temp\\data\\ECXA\\SchemasAndDgn\\Bentley_Plant.06.00.ecschema.xml", *schemaContext);
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath( L"EmptyCustomAttribute.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ (SCHEMA_READ_STATUS_Success, status);  

    Utf8String ecSchemaXmlString;

    SchemaWriteStatus status2 = schema->WriteToXmlString(ecSchemaXmlString);
    EXPECT_EQ(SCHEMA_WRITE_STATUS_Success, status2);
    EXPECT_NE (Utf8String::npos, ecSchemaXmlString.find ("<Relationship/>"));
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
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath( L"SchemaThatReferences.01.00.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ (SCHEMA_READ_STATUS_Success, status);    

    ECClassP pClass = schema->GetClassP("circle");    
    ASSERT_TRUE (NULL != pClass);
    }; 
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenECSchemaContainsOnlyRequiredAttributes)
    {                
    // show error messages but do not assert.
    ECSchema::SetErrorHandling (true, false);

    
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath( L"OnlyRequiredECSchemaAttributes.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ (SCHEMA_READ_STATUS_Success, status);    
    EXPECT_STREQ ("OnlyRequiredECSchemaAttributes", schema->GetName().c_str());
    EXPECT_STREQ ("", schema->GetNamespacePrefix().c_str());
    EXPECT_STREQ ("OnlyRequiredECSchemaAttributes", schema->GetDisplayLabel().c_str());
    EXPECT_FALSE (schema->GetIsDisplayLabelDefined());
    EXPECT_STREQ ("", schema->GetDescription().c_str());
    EXPECT_EQ (1, schema->GetVersionMajor());
    EXPECT_EQ (0, schema->GetVersionMinor());
    
    ECClassP pClass = schema->GetClassP("OnlyRequiredECClassAttributes");    
    ASSERT_TRUE (NULL != pClass);
    EXPECT_STREQ ("OnlyRequiredECClassAttributes", pClass->GetName().c_str());    
    EXPECT_STREQ ("OnlyRequiredECClassAttributes", pClass->GetDisplayLabel().c_str());
    EXPECT_FALSE (pClass->GetIsDisplayLabelDefined());
    EXPECT_STREQ ("", pClass->GetDescription().c_str());
    EXPECT_FALSE (pClass->GetIsStruct());
    EXPECT_FALSE (pClass->GetIsCustomAttributeClass());
    EXPECT_TRUE (pClass->GetIsDomainClass());
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenDeserializingWidgetsECSchema)
    {
    
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath( L"Widgets.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ (SCHEMA_READ_STATUS_Success, status);  
    VerifyWidgetsSchema(schema);  
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenDeserializingECSchemaFromString)
    {
    
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString (schema, 
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"Widgets\" version=\"09.06\" displayLabel=\"Widgets Display Label\" description=\"Widgets Description\" nameSpacePrefix=\"wid\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" xmlns:ec=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" xmlns:ods=\"Bentley_ODS.01.02\">"
        "    <ECClass typeName=\"ecProject\" description=\"Project ECClass\" displayLabel=\"Project\" isDomainClass=\"True\">"
        "       <ECProperty propertyName=\"Name\" typeName=\"string\" displayLabel=\"Project Name\" />"
        "       <ECProperty propertyName=\"Geometry\" typeName=\"Bentley.Geometry.Common.IGeometry\" displayLabel=\"Geometry\" />"
        "       <ECProperty propertyName=\"LineSegment\" typeName=\"Bentley.Geometry.Common.ILineSegment\" displayLabel=\"Line Segment\" />"
        "    </ECClass>"
        "</ECSchema>", *schemaContext);

    EXPECT_EQ (SCHEMA_READ_STATUS_Success, status);    
    EXPECT_STREQ ("Widgets", schema->GetName().c_str());
    EXPECT_STREQ ("wid", schema->GetNamespacePrefix().c_str());
    EXPECT_STREQ ("Widgets Display Label", schema->GetDisplayLabel().c_str());
    EXPECT_TRUE (schema->GetIsDisplayLabelDefined());
    EXPECT_STREQ ("Widgets Description", schema->GetDescription().c_str());
    EXPECT_EQ (9, schema->GetVersionMajor());
    EXPECT_EQ (6, schema->GetVersionMinor());        
    
#ifdef DEBUG_PRINT
    for (ECClassP pClass: schema->GetClasses())
        {
        printf ("Widgets contains class: '%s' with display label '%s'\n", pClass->GetName().c_str(), pClass->GetDisplayLabel().c_str());
        }
#endif
    ECClassP pClass = schema->GetClassP("ClassDoesNotExistInSchema");
    EXPECT_FALSE (pClass);

    pClass = schema->GetClassP("ecProject");
    ASSERT_TRUE (NULL != pClass);
    EXPECT_STREQ ("ecProject", pClass->GetName().c_str());    
    EXPECT_STREQ ("Project", pClass->GetDisplayLabel().c_str());
    EXPECT_TRUE (pClass->GetIsDisplayLabelDefined());
    EXPECT_STREQ ("Project ECClass", pClass->GetDescription().c_str());
    EXPECT_FALSE (pClass->GetIsStruct());
    EXPECT_FALSE (pClass->GetIsCustomAttributeClass());
    EXPECT_TRUE (pClass->GetIsDomainClass());
    EXPECT_FALSE (pClass->HasBaseClasses());
    ECPropertyP pProperty = GetPropertyByName (*pClass, "Name");
    ASSERT_TRUE (NULL != pProperty);
    EXPECT_STREQ ("Name", pProperty->GetName().c_str());
    EXPECT_TRUE (pProperty->GetIsPrimitive());
    EXPECT_FALSE (pProperty->GetIsStruct());
    EXPECT_FALSE (pProperty->GetIsArray());
    EXPECT_STREQ ("string", pProperty->GetTypeName().c_str());
    EXPECT_TRUE (PRIMITIVETYPE_String == pProperty->GetAsPrimitiveProperty()->GetType());
    EXPECT_TRUE (pProperty->GetIsDisplayLabelDefined());
    EXPECT_STREQ ("Project Name", pProperty->GetDisplayLabel().c_str());
    EXPECT_STREQ ("", pProperty->GetDescription().c_str());
    EXPECT_EQ (pClass, &pProperty->GetClass());
    EXPECT_FALSE (pProperty->GetIsReadOnly());

    pProperty = pClass->GetPropertyP ("Geometry");
    ASSERT_TRUE (NULL != pProperty);
    EXPECT_STREQ ("Geometry", pProperty->GetName().c_str());
    EXPECT_TRUE (pProperty->GetIsPrimitive());
    EXPECT_FALSE (pProperty->GetIsStruct());
    EXPECT_FALSE (pProperty->GetIsArray());
    EXPECT_STREQ ("Bentley.Geometry.Common.IGeometry", pProperty->GetTypeName().c_str());
    EXPECT_TRUE (PRIMITIVETYPE_IGeometry == pProperty->GetAsPrimitiveProperty()->GetType());
    EXPECT_TRUE (pProperty->GetIsDisplayLabelDefined());
    EXPECT_STREQ ("Geometry", pProperty->GetDisplayLabel().c_str());
    EXPECT_STREQ ("", pProperty->GetDescription().c_str());
    EXPECT_EQ (pClass, &pProperty->GetClass());
    EXPECT_FALSE (pProperty->GetIsReadOnly());

    pProperty = pClass->GetPropertyP ("LineSegment");
    ASSERT_TRUE (NULL != pProperty);
    EXPECT_STREQ ("LineSegment", pProperty->GetName().c_str());
    EXPECT_TRUE (pProperty->GetIsPrimitive());
    EXPECT_FALSE (pProperty->GetIsStruct());
    EXPECT_FALSE (pProperty->GetIsArray());
    EXPECT_STREQ ("Bentley.Geometry.Common.IGeometry", pProperty->GetTypeName().c_str());
    EXPECT_TRUE (PRIMITIVETYPE_IGeometry == pProperty->GetAsPrimitiveProperty()->GetType());
    EXPECT_TRUE (pProperty->GetIsDisplayLabelDefined());
    EXPECT_STREQ ("Line Segment", pProperty->GetDisplayLabel().c_str());
    EXPECT_STREQ ("", pProperty->GetDescription().c_str());
    EXPECT_EQ (pClass, &pProperty->GetClass());
    EXPECT_FALSE (pProperty->GetIsReadOnly());

    pProperty = GetPropertyByName (*pClass, "PropertyDoesNotExistInClass", false);
    EXPECT_FALSE (pProperty);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenRoundtripUsingString)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath( L"Widgets.01.00.ecschema.xml").c_str(), *schemaContext);
#ifdef DEBUG_PRINT
    printf("Verifying original schema from file.\n"); 
#endif
    VerifyWidgetsSchema(schema);

    EXPECT_EQ (SCHEMA_READ_STATUS_Success, status);

    Utf8String ecSchemaXmlString;
    
    SchemaWriteStatus status2 = schema->WriteToXmlString(ecSchemaXmlString);
    schema->WriteToXmlFile(L"d:\\temp\\data\\schema.xml");
    EXPECT_EQ(SCHEMA_WRITE_STATUS_Success, status2);
    
    ECSchemaPtr deserializedSchema;
    schemaContext = ECSchemaReadContext::CreateContext();
    status = ECSchema::ReadFromXmlString(deserializedSchema, ecSchemaXmlString.c_str(), *schemaContext);
    EXPECT_EQ (SCHEMA_READ_STATUS_Success, status); 
#ifdef DEBUG_PRINT
    wprintf(L"Verifying schema deserialized from string.\n");
#endif
    VerifyWidgetsSchema(deserializedSchema);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectDomainClassToBeSetProperly)
    {
    
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString (schema, 
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"Widgets\" version=\"09.06\" displayLabel=\"Widgets Display Label\" description=\"Widgets Description\" nameSpacePrefix=\"wid\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" xmlns:ec=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" xmlns:ods=\"Bentley_ODS.01.02\">"
        "    <ECClass typeName=\"ecProject\" description=\"Project ECClass\" displayLabel=\"Project\" isDomainClass=\"True\" isCustomAttributeClass=\"True\">"
        "       <ECProperty propertyName=\"Name\" typeName=\"string\" displayLabel=\"Project Name\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"ecWidget\" description=\"Widget ECClass\" displayLabel=\"Widget\" isCustomAttributeClass=\"True\">"
        "       <ECProperty propertyName=\"Name\" typeName=\"string\" displayLabel=\"Widget Name\" />"
        "    </ECClass>"
        "</ECSchema>", *schemaContext);

    EXPECT_EQ (SCHEMA_READ_STATUS_Success, status);

    ECClassP pClass = schema->GetClassP("ecProject");
    EXPECT_TRUE (pClass->GetIsCustomAttributeClass());
    EXPECT_TRUE (pClass->GetIsDomainClass());

    pClass = schema->GetClassP("ecWidget");
    EXPECT_TRUE (pClass->GetIsCustomAttributeClass());
    EXPECT_FALSE (pClass->GetIsDomainClass());
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWithDuplicateClassesInXml)
    {
    
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString (schema, 
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"Widgets\" version=\"09.06\" displayLabel=\"Widgets Display Label\" description=\"Widgets Description\" nameSpacePrefix=\"wid\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" xmlns:ec=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" xmlns:ods=\"Bentley_ODS.01.02\">"
        "    <ECClass typeName=\"DifferentClass\" isDomainClass=\"True\">"
        "    </ECClass>"
        "    <ECClass typeName=\"ecProject\" description=\"Project ECClass\" displayLabel=\"Project\" isDomainClass=\"True\">"
        "       <ECProperty propertyName=\"Name\" typeName=\"string\" displayLabel=\"Project Name\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"ecProject\" isDomainClass=\"True\">"
        "    </ECClass>"
        "</ECSchema>", *schemaContext);

    EXPECT_EQ (SCHEMA_READ_STATUS_Success, status); 

    // Nothing should have been overwritten
    ECClassP projectClass = schema->GetClassP("ecProject");
    ASSERT_TRUE (NULL != projectClass);
    EXPECT_STREQ("Project ECClass", projectClass->GetDescription().c_str());
    EXPECT_STREQ("Project", projectClass->GetDisplayLabel().c_str());
    ECPropertyP pProperty = GetPropertyByName (*projectClass, "Name");
    ASSERT_TRUE (NULL != pProperty);
    EXPECT_STREQ ("Name", pProperty->GetName().c_str());
    EXPECT_TRUE (pProperty->GetIsPrimitive());
    EXPECT_FALSE (pProperty->GetIsStruct());
    EXPECT_FALSE (pProperty->GetIsArray());
    EXPECT_STREQ ("string", pProperty->GetTypeName().c_str());


    ECSchemaPtr schema2;
    ECSchemaReadContextPtr   schemaContext2 = ECSchemaReadContext::CreateContext();


    status = ECSchema::ReadFromXmlString (schema2, 
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"Widgets2\" version=\"09.06\" displayLabel=\"Widgets Display Label\" description=\"Widgets Description\" nameSpacePrefix=\"wid\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" xmlns:ec=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" xmlns:ods=\"Bentley_ODS.01.02\">"
        "    <ECClass typeName=\"ecProject\" description=\"Project ECClass\" displayLabel=\"Project\" isDomainClass=\"True\">"
        "       <ECProperty propertyName=\"Name\" typeName=\"string\" displayLabel=\"Project Name\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"ecProject\" description=\"New Project ECClass\" isDomainClass=\"True\">"
        "       <ECProperty propertyName=\"Author\" typeName=\"string\" displayLabel=\"Project Name\" />"
        "    </ECClass>"
        "</ECSchema>", *schemaContext2);

    EXPECT_EQ (SCHEMA_READ_STATUS_Success, status); 
    projectClass = schema2->GetClassP("ecProject");
    ASSERT_TRUE (NULL != projectClass);
    EXPECT_STREQ("New Project ECClass", projectClass->GetDescription().c_str());
    EXPECT_STREQ("Project", projectClass->GetDisplayLabel().c_str());
    pProperty = GetPropertyByName (*projectClass, "Name");
    ASSERT_TRUE (NULL != pProperty);
    EXPECT_STREQ ("Name", pProperty->GetName().c_str());
    EXPECT_TRUE (pProperty->GetIsPrimitive());
    EXPECT_FALSE (pProperty->GetIsStruct());
    EXPECT_FALSE (pProperty->GetIsArray());
    EXPECT_STREQ ("string", pProperty->GetTypeName().c_str());

    pProperty = GetPropertyByName (*projectClass, "Author");
    ASSERT_TRUE (NULL != pProperty);
    EXPECT_STREQ ("Author", pProperty->GetName().c_str());
    EXPECT_TRUE (pProperty->GetIsPrimitive());
    EXPECT_FALSE (pProperty->GetIsStruct());
    EXPECT_FALSE (pProperty->GetIsArray());
    EXPECT_STREQ ("string", pProperty->GetTypeName().c_str());
    }

TEST_F(SchemaDeserializationTest, EnsureSupplementalSchemaCannotHaveBaseClasses)
	{
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString (schema, 
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" schemaName=\"SupplementalSchemaWithBaseClasses_Supplemental_Mapping\" nameSpacePrefix=\"ss\" version=\"01.00\" description=\"Test Supplemental Mapping Schema\" displayLabel=\"Electrical Extended Supplemental Mapping\" xmlns:ec=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
            "<ECClass typeName=\"MAPPING\" displayLabel=\"Mapping\" isStruct=\"false\" isDomainClass=\"true\" isCustomAttributeClass=\"false\"/>"
            "<ECClass typeName=\"ELECTRICAL_PROPERTY_MAPPING\" displayLabel=\"Electrical Property Mapping\" isStruct=\"false\" isDomainClass=\"false\" isCustomAttributeClass=\"true\">"
                "<BaseClass>MAPPING</BaseClass>"
                "<ECProperty propertyName=\"APPLICATION_PROPERTY_NAME\" typeName=\"string\" displayLabel=\"Application Property Name\" readOnly=\"false\"/>"
            "</ECClass>"
            "<ECClass typeName=\"ELECTRICAL_ITEM\" displayLabel=\"Electrical Item\" isStruct=\"false\" isDomainClass=\"true\" isCustomAttributeClass=\"false\">"
                "<BaseClass>bentley:BENTLEY_BASE_OBJECT</BaseClass>"
                "<ECProperty propertyName=\"ID\" typeName=\"string\" description=\"Business ID for an electrical item.\" readOnly=\"false\">"
                    "<ECCustomAttributes>"
                        "<ELECTRICAL_PROPERTY_MAPPING xmlns=\"ElectricalExtended_Supplemental_Mapping.01.00\">"
                        "<APPLICATION_PROPERTY_NAME>DeviceID</APPLICATION_PROPERTY_NAME>"
                        "</ELECTRICAL_PROPERTY_MAPPING>"
                    "</ECCustomAttributes>"
                "</ECProperty>"
            "</ECClass>"
        "</ECSchema>", *schemaContext);

    EXPECT_EQ (SCHEMA_READ_STATUS_Success, status); 
    Utf8CP className = "ELECTRICAL_ITEM";
    ECClassCP ecClass = schema->GetClassCP (className);

    const ECBaseClassesList& baseClassList = ecClass->GetBaseClasses ();
    EXPECT_EQ (0, baseClassList.size ()) << "Class " << className << " should not have any base classes since it is in a supplemental schema.";
    
    className = "ELECTRICAL_PROPERTY_MAPPING";
    ecClass = schema->GetClassCP (className);

    const ECBaseClassesList& baseClassList2 = ecClass->GetBaseClasses ();
    EXPECT_EQ (0, baseClassList2.size ()) << "Class " << className << " should not have any base classes since it is in a supplemental schema.";
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenSerializingToFile)
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath( L"Widgets.01.00.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ(status, SCHEMA_READ_STATUS_Success);
    VerifyWidgetsSchema(schema);

    SchemaWriteStatus status2 = schema->WriteToXmlFile(ECTestFixture::GetTempDataPath( L"test.xml").c_str());
    EXPECT_EQ (SCHEMA_WRITE_STATUS_Success, status2);
    }

#if defined (NEEDSWORK_LIBXML)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenRoundtripUsingStream)
    {
    
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath( L"Widgets.01.00.ecschema.xml").c_str(), *schemaContext);
#ifdef DEBUG_PRINT
    printf("Verifying original schema from file.\n");
#endif
    VerifyWidgetsSchema(schema);

    EXPECT_EQ (SCHEMA_READ_STATUS_Success, status);    
    LPSTREAM stream = NULL;
    //HRESULT res = ::CreateStreamOnHGlobal(NULL,TRUE,&stream);
    ::CreateStreamOnHGlobal(NULL,TRUE,&stream);

    SchemaWriteStatus status2 = schema->WriteToXmlStream(stream);
    EXPECT_EQ(SCHEMA_WRITE_STATUS_Success, status2);
    
    LARGE_INTEGER liPos = {0};
    stream->Seek(liPos, STREAM_SEEK_SET, NULL);

    ECSchemaP deserializedSchema;
    schemaOwner = ECSchemaCache::Create(); // We need a new cache... we don't want to read the ECSchema into the cache that already has a copy of this ECSchema
    schemaContext = ECSchemaReadContext::CreateContext(*schemaOwner);
    status = ECSchema::ReadFromXmlStream(deserializedSchema, stream, *schemaContext);
    EXPECT_EQ (SCHEMA_READ_STATUS_Success, status); 
#ifdef DEBUG_PRINT
    printf("Verifying schema deserialized from stream.\n");
#endif
    VerifyWidgetsSchema(deserializedSchema);
    }
#endif

    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaSerializationTest, ExpectSuccessWithSerializingBaseClasses)
    {
    ECSchemaPtr schema;
    ECSchemaPtr schema2;
    ECSchemaPtr schema3;
    
    ECSchema::CreateSchema(schema, "Widget", 5, 5);
    ECSchema::CreateSchema(schema2, "BaseSchema", 5, 5);
    ECSchema::CreateSchema(schema3, "BaseSchema2", 5, 5);
    
    schema->SetNamespacePrefix("ecw");
    schema2->SetNamespacePrefix("base");
    schema3->SetNamespacePrefix("base");
    
    ECClassP class1;
    ECClassP baseClass;
    ECClassP anotherBase;
    ECClassP gadget;
    ECClassP bolt;
    schema->CreateClass(class1, "TestClass");
    schema->CreateClass(gadget, "Gadget");
    schema->CreateClass(bolt, "Bolt");
    schema2->CreateClass(baseClass, "BaseClass");
    schema3->CreateClass(anotherBase, "AnotherBase");
    
    EXPECT_EQ(ECOBJECTS_STATUS_SchemaNotFound, class1->AddBaseClass(*baseClass));
    schema->AddReferencedSchema(*schema2);
    schema->AddReferencedSchema(*schema3);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, class1->AddBaseClass(*baseClass));
    EXPECT_EQ(ECOBJECTS_STATUS_Success, class1->AddBaseClass(*anotherBase));
    EXPECT_EQ(ECOBJECTS_STATUS_Success, gadget->AddBaseClass(*class1));
    
    SchemaWriteStatus status2 = schema->WriteToXmlFile(ECTestFixture::GetTempDataPath( L"base.xml").c_str());
    EXPECT_EQ(SCHEMA_WRITE_STATUS_Success, status2);
    
    WString ecSchemaXmlString;
    SchemaWriteStatus status3 = schema->WriteToXmlString(ecSchemaXmlString);
    EXPECT_EQ(SCHEMA_WRITE_STATUS_Success, status3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaReferenceTest, AddAndRemoveReferencedSchemas)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    
    ECSchemaPtr refSchema;
    ECSchema::CreateSchema(refSchema, "RefSchema", 5, 5);
    
    EXPECT_EQ(ECOBJECTS_STATUS_Success, schema->AddReferencedSchema(*refSchema));

    EXPECT_EQ(ECOBJECTS_STATUS_NamedItemAlreadyExists, schema->AddReferencedSchema(*refSchema));
    
    ECSchemaReferenceListCR refList = schema->GetReferencedSchemas();

    ECSchemaReferenceList::const_iterator schemaIterator = refList.find (refSchema->GetSchemaKey());
        
    EXPECT_FALSE(schemaIterator == refList.end());
    EXPECT_EQ(ECOBJECTS_STATUS_Success, schema->RemoveReferencedSchema(*refSchema));
    
    schemaIterator = refList.find (refSchema->GetSchemaKey());

    EXPECT_TRUE(schemaIterator == refList.end());
    EXPECT_EQ(ECOBJECTS_STATUS_SchemaNotFound, schema->RemoveReferencedSchema(*refSchema));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaReferenceTest, InvalidReference)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling (true, false);

    
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    WString seedPath(ECTestFixture::GetTestDataPath(L"").c_str());
    schemaContext->AddSchemaPath(seedPath.c_str());

    //The "InvalidReference" schema contains a reference to BaseSchema.01.01.  This schema 
    //does not exist.  1.0 exists, but the minor version numbers are incompatible.
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath( L"InvalidReference.01.00.ecschema.xml").c_str(), *schemaContext);  

    EXPECT_TRUE (schema.IsNull());
    EXPECT_EQ (SCHEMA_READ_STATUS_ReferencedSchemaNotFound, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaReferenceTest, ExpectErrorWhenTryRemoveSchemaInUse)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    
    ECSchemaPtr refSchema;
    ECSchema::CreateSchema(refSchema, "RefSchema", 5, 5);
    
    EXPECT_EQ(ECOBJECTS_STATUS_Success, schema->AddReferencedSchema(*refSchema));
    ECClassP class1;
    ECClassP baseClass;
    ECClassP structClass;
            
    refSchema->CreateClass(baseClass, "BaseClass");
    refSchema->CreateClass(structClass, "StructClass");
    schema->CreateClass(class1, "TestClass");
    structClass->SetIsStruct(true);
    
    class1->AddBaseClass(*baseClass);
    EXPECT_EQ (ECOBJECTS_STATUS_SchemaInUse, schema->RemoveReferencedSchema(*refSchema));
    
    class1->RemoveBaseClass(*baseClass);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, schema->RemoveReferencedSchema(*refSchema));

    EXPECT_EQ(ECOBJECTS_STATUS_Success, schema->AddReferencedSchema(*refSchema));
    StructECPropertyP structProp;
    ArrayECPropertyP nestedArrayProp;

    ArrayECPropertyP primitiveArrayProp;
    
    class1->CreateStructProperty(structProp, "StructMember");
    class1->CreateArrayProperty(nestedArrayProp, "NestedArray");
    
    class1->CreateArrayProperty(primitiveArrayProp, "PrimitiveArrayProp");
    primitiveArrayProp->SetPrimitiveElementType (PRIMITIVETYPE_Long);
    primitiveArrayProp->SetMinOccurs(1);
    primitiveArrayProp->SetMaxOccurs(10);

    structProp->SetType(*structClass);
    nestedArrayProp->SetStructElementType(structClass);

    EXPECT_EQ (ECOBJECTS_STATUS_SchemaInUse, schema->RemoveReferencedSchema(*refSchema));
    class1->RemoveProperty("StructMember");
    EXPECT_EQ (ECOBJECTS_STATUS_SchemaInUse, schema->RemoveReferencedSchema(*refSchema));
    class1->RemoveProperty("NestedArray");
    EXPECT_EQ(ECOBJECTS_STATUS_Success, schema->RemoveReferencedSchema(*refSchema));
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaReferenceTest, ExpectFailureWithCircularReferences)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    WString seedPath(ECTestFixture::GetTestDataPath(L"").c_str());
    schemaContext->AddSchemaPath(seedPath.c_str());

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath( L"CircleSchema.01.00.ecschema.xml").c_str(), *schemaContext);
    EXPECT_FALSE (SCHEMA_READ_STATUS_Success == status);
    EXPECT_FALSE(schema.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaReferenceTest, ExpectSuccessWithSpecialCaseOpenPlantSchema)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    WString seedPath(ECTestFixture::GetTestDataPath(L"").c_str());
    schemaContext->AddSchemaPath(seedPath.c_str());

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath( L"OpenPlant_Supplemental_Mapping_OPPID.01.01.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ (SCHEMA_READ_STATUS_Success, status);

    ECSchemaReferenceListCR refList = schema->GetReferencedSchemas();
    EXPECT_EQ(1, refList.size());
    ECSchemaPtr refSchema = refList.begin()->second;
    EXPECT_EQ(0, refSchema->GetName().CompareTo("Bentley_Standard_CustomAttributes"));
    }

//TEST_F(SchemaReferenceTest, ExpectSchemaGraphInCorrectOrder)
//    {
//    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
//    WString seedPath(ECTestFixture::GetTestDataPath(L"").c_str());
//    schemaContext->AddSchemaPath(seedPath.c_str());
//
//    ECSchemaPtr schema;
//    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath( L"Bentley_Plant.06.00.ecschema.xml").c_str(), *schemaContext);
//    ASSERT_EQ (SCHEMA_READ_STATUS_Success, status);
//    ASSERT_TRUE( schema.IsValid() );
//    bvector<ECSchemaP> schemasToImport;
//    schema->FindAllSchemasInGraph (schemasToImport, true);
//    bvector<WCharCP> expectedPrefixes;
//    expectedPrefixes.push_back(L"iipmdbca");
//    expectedPrefixes.push_back(L"rdlca");
//    expectedPrefixes.push_back(L"beca");
//    expectedPrefixes.push_back(L"bsca");
//    expectedPrefixes.push_back(L"bsm");
//    expectedPrefixes.push_back(L"bjca");
//    expectedPrefixes.push_back(L"jclass");
//    expectedPrefixes.push_back(L"dmd");
//    expectedPrefixes.push_back(L"stepmd");
//    expectedPrefixes.push_back(L"bjeca");
//    expectedPrefixes.push_back(L"bpca");
//    expectedPrefixes.push_back(L"bp");
//
//    EXPECT_EQ(schemasToImport.size(), expectedPrefixes.size());
//    int counter = 0;
//    for (ECSchemaP testSchema: schemasToImport)
//        {
//        WString prefix = testSchema->GetNamespacePrefix();
//        EXPECT_EQ(0, wcscmp(prefix.c_str(), expectedPrefixes[counter]));
//        counter++;
//        }
//    }

TEST_F(SchemaLocateTest, ExpectSuccessWhenLocatingStandardSchema)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    bmap<Utf8String, Utf8CP> standardSchemaNames;
    standardSchemaNames.insert(bpair<Utf8String, Utf8CP>("Bentley_Standard_CustomAttributes", "01.04"));
    standardSchemaNames.insert(bpair<Utf8String, Utf8CP>("Bentley_Standard_Classes", "01.00"));
    standardSchemaNames.insert(bpair<Utf8String, Utf8CP>("Bentley_ECSchemaMap", "01.00"));
    standardSchemaNames.insert(bpair<Utf8String, Utf8CP>("EditorCustomAttributes", "01.00"));
    standardSchemaNames.insert(bpair<Utf8String, Utf8CP>("Bentley_Common_Classes", "01.00"));
    standardSchemaNames.insert(bpair<Utf8String, Utf8CP>("Dimension_Schema", "01.00"));
    standardSchemaNames.insert(bpair<Utf8String, Utf8CP>("iip_mdb_customAttributes", "01.00"));
    standardSchemaNames.insert(bpair<Utf8String, Utf8CP>("KindOfQuantity_Schema", "01.01"));
    standardSchemaNames.insert(bpair<Utf8String, Utf8CP>("rdl_customAttributes", "01.00"));
    standardSchemaNames.insert(bpair<Utf8String, Utf8CP>("SIUnitSystemDefaults", "01.00"));
    standardSchemaNames.insert(bpair<Utf8String, Utf8CP>("Unit_Attributes", "01.00"));
    standardSchemaNames.insert(bpair<Utf8String, Utf8CP>("Units_Schema", "01.00"));
    standardSchemaNames.insert(bpair<Utf8String, Utf8CP>("USCustomaryUnitSystemDefaults", "01.00"));

    ECSchemaPtr schema;

    for (bmap<Utf8String, Utf8CP>::const_iterator it = standardSchemaNames.begin(); it != standardSchemaNames.end(); ++it)
        {
        bpair<Utf8String, Utf8CP>const& entry = *it;
        
        SchemaKey key (entry.first.c_str(), 1, 0);
        EXPECT_TRUE(ECSchema::ParseVersionString(key.m_versionMajor, key.m_versionMinor, entry.second) == ECOBJECTS_STATUS_Success);
        EXPECT_EQ(key.m_versionMajor, atoi(entry.second));
        EXPECT_EQ(key.m_versionMinor, atoi(strchr(entry.second, '.') + 1));
        schema = ECSchema::LocateSchema(key, *schemaContext);
        EXPECT_TRUE(schema.IsValid());
        EXPECT_TRUE(schema->IsStandardSchema());
        EXPECT_STREQ(entry.second, ECSchema::FormatSchemaVersion(key.m_versionMajor, key.m_versionMinor).c_str());
        }
    }
  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaLocateTest, ExpectFailureWithNonStandardSchema)
    {
    ECSchemaPtr testSchema;
    ECSchema::CreateSchema(testSchema, "TestSchema", 1, 2);
    EXPECT_FALSE(testSchema->IsStandardSchema());
    }
    
TEST_F(SchemaLocateTest, DetermineWhetherSchemaCanBeImported)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
     
    SchemaKey key("Bentley_Standard_CustomAttributes", 1, 4);
    
    ECSchemaPtr schema = ECSchema::LocateSchema(key, *schemaContext);
    EXPECT_TRUE(schema.IsValid());
    EXPECT_FALSE(schema->ShouldNotBeStored());

    ECSchema::CreateSchema(schema, "Units_Schema", 1, 4);
    EXPECT_TRUE(schema->ShouldNotBeStored());
    }
      
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaCreationTest, CanFullyCreateASchema)
    {
    ECSchemaPtr testSchema;
    ECSchema::CreateSchema(testSchema, "TestSchema", 1, 2);
    testSchema->SetNamespacePrefix("ts");
    testSchema->SetDescription("Schema for testing programmatic construction");
    testSchema->SetDisplayLabel("Test Schema");
    
    EXPECT_TRUE(testSchema->GetIsDisplayLabelDefined());
    EXPECT_EQ(1, testSchema->GetVersionMajor());
    EXPECT_EQ(2, testSchema->GetVersionMinor());
    EXPECT_EQ(0, strcmp(testSchema->GetName().c_str(), "TestSchema"));
    EXPECT_EQ(0, strcmp(testSchema->GetNamespacePrefix().c_str(), "ts"));
    EXPECT_EQ(0, strcmp(testSchema->GetDescription().c_str(), "Schema for testing programmatic construction"));
    EXPECT_EQ(0, strcmp(testSchema->GetDisplayLabel().c_str(), "Test Schema"));
    
    ECSchemaPtr schema2;
    ECSchema::CreateSchema(schema2, "BaseSchema", 5, 5);
    
    testSchema->AddReferencedSchema(*schema2);
    
    ECClassP class1;
    ECClassP baseClass;
    ECClassP structClass;
    ECClassP relatedClass;
    ECRelationshipClassP relationshipClass;

    testSchema->CreateClass(class1, "TestClass");
    testSchema->CreateClass(structClass, "StructClass");
    schema2->CreateClass(baseClass, "BaseClass");
    testSchema->CreateClass(relatedClass, "RelatedClass");
    
    class1->SetDescription("Class for testing purposes");
    class1->SetDisplayLabel("Test Class");
    
    EXPECT_EQ(0, strcmp(class1->GetDescription().c_str(), "Class for testing purposes"));
    EXPECT_EQ(0, strcmp(class1->GetDisplayLabel().c_str(), "Test Class"));
    EXPECT_FALSE(class1->GetIsStruct());
    EXPECT_FALSE(class1->GetIsCustomAttributeClass());
    EXPECT_TRUE(class1->GetIsDomainClass());
    EXPECT_EQ(ECOBJECTS_STATUS_Success, class1->AddBaseClass(*baseClass));
    EXPECT_TRUE(class1->HasBaseClasses());
    
    structClass->SetIsStruct(true);
    EXPECT_TRUE(structClass->GetIsStruct());
    EXPECT_TRUE(structClass->GetIsDomainClass());
    
    PrimitiveECPropertyP stringProp;
    StructECPropertyP structProp;
    ArrayECPropertyP nestedArrayProp;
    ArrayECPropertyP primitiveArrayProp;
    
    class1->CreatePrimitiveProperty(stringProp, "StringMember");
    class1->CreateStructProperty(structProp, "StructMember");
    class1->CreateArrayProperty(nestedArrayProp, "NestedArray");
    class1->CreateArrayProperty(primitiveArrayProp, "PrimitiveArray");
    
    structProp->SetType(*structClass);
    nestedArrayProp->SetStructElementType(structClass);
    primitiveArrayProp->SetPrimitiveElementType (PRIMITIVETYPE_Long);
    primitiveArrayProp->SetMinOccurs(1);
    primitiveArrayProp->SetMaxOccurs(10);
    
    EXPECT_TRUE(ARRAYKIND_Struct == nestedArrayProp->GetKind());
    EXPECT_TRUE(ARRAYKIND_Primitive == primitiveArrayProp->GetKind());
    EXPECT_EQ(0, nestedArrayProp->GetMinOccurs());
    EXPECT_EQ(UINT_MAX, nestedArrayProp->GetMaxOccurs());
    EXPECT_EQ(1, primitiveArrayProp->GetMinOccurs());
#if FIXED_COUNT_ARRAYS_ARE_SUPPORTED
    EXPECT_EQ(10, primitiveArrayProp->GetMaxOccurs());
#else
    EXPECT_EQ(UINT_MAX, primitiveArrayProp->GetMaxOccurs());
#endif
    EXPECT_TRUE(stringProp->GetIsPrimitive());
    EXPECT_FALSE(stringProp->GetIsStruct());
    EXPECT_FALSE(stringProp->GetIsArray());
    
    EXPECT_FALSE(structProp->GetIsPrimitive());
    EXPECT_TRUE(structProp->GetIsStruct());
    EXPECT_FALSE(structProp->GetIsArray());
    
    EXPECT_FALSE(primitiveArrayProp->GetIsPrimitive());
    EXPECT_FALSE(primitiveArrayProp->GetIsStruct());
    EXPECT_TRUE(primitiveArrayProp->GetIsArray());
    
    EXPECT_FALSE(stringProp->GetIsReadOnly());
    
    EXPECT_EQ(0, strcmp(stringProp->GetTypeName().c_str(), "string"));
    EXPECT_TRUE(PRIMITIVETYPE_String == stringProp->GetType());
    EXPECT_EQ(0, strcmp(structProp->GetType().GetName().c_str(), "StructClass"));
    
    PrimitiveECPropertyP binaryProperty;
    PrimitiveECPropertyP booleanProperty;
    PrimitiveECPropertyP dateTimeProperty;
    PrimitiveECPropertyP doubleProperty;
    PrimitiveECPropertyP integerProperty;
    PrimitiveECPropertyP longProperty;
    PrimitiveECPropertyP point2DProperty;
    PrimitiveECPropertyP point3DProperty;
    
    class1->CreatePrimitiveProperty(binaryProperty, "BinaryProp");
    class1->CreatePrimitiveProperty(booleanProperty, "BooleanProp");
    class1->CreatePrimitiveProperty(dateTimeProperty, "DateTimeProp");
    class1->CreatePrimitiveProperty(doubleProperty, "DoubleProp");
    class1->CreatePrimitiveProperty(integerProperty, "IntProp");
    class1->CreatePrimitiveProperty(longProperty, "LongProp");
    class1->CreatePrimitiveProperty(point2DProperty, "Point2DProp");
    class1->CreatePrimitiveProperty(point3DProperty, "Point3DProp");
    
    EXPECT_EQ(ECOBJECTS_STATUS_ParseError, binaryProperty->SetTypeName ("fake"));
    
    binaryProperty->SetTypeName ("binary");
    booleanProperty->SetTypeName ("boolean");
    dateTimeProperty->SetTypeName ("dateTime");
    doubleProperty->SetTypeName ("double");
    integerProperty->SetTypeName ("int");
    longProperty->SetTypeName ("long");
    point2DProperty->SetTypeName ("point2d");
    point3DProperty->SetTypeName ("point3d");
    
    EXPECT_TRUE(PRIMITIVETYPE_Binary == binaryProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_Boolean == booleanProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_DateTime == dateTimeProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_Double == doubleProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_Integer == integerProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_Long == longProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_Point2D == point2DProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_Point3D == point3DProperty->GetType());

    class1->CreatePrimitiveProperty(binaryProperty, "BinaryProp2", PRIMITIVETYPE_Binary);
    class1->CreatePrimitiveProperty(booleanProperty, "BooleanProp2", PRIMITIVETYPE_Boolean);
    class1->CreatePrimitiveProperty(dateTimeProperty, "DateTimeProp2", PRIMITIVETYPE_DateTime);
    class1->CreatePrimitiveProperty(doubleProperty, "DoubleProp2", PRIMITIVETYPE_Double);
    class1->CreatePrimitiveProperty(integerProperty, "IntProp2", PRIMITIVETYPE_Integer);
    class1->CreatePrimitiveProperty(longProperty, "LongProp2", PRIMITIVETYPE_Long);
    class1->CreatePrimitiveProperty(point2DProperty, "Point2DProp2", PRIMITIVETYPE_Point2D);
    class1->CreatePrimitiveProperty(point3DProperty, "Point3DProp2", PRIMITIVETYPE_Point3D);

    EXPECT_TRUE(PRIMITIVETYPE_Binary == binaryProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_Boolean == booleanProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_DateTime == dateTimeProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_Double == doubleProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_Integer == integerProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_Long == longProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_Point2D == point2DProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_Point3D == point3DProperty->GetType());

    class1->CreateStructProperty(structProp, "StructMember2", *structClass);
    class1->CreateArrayProperty(nestedArrayProp, "NestedArray2", structClass);
    class1->CreateArrayProperty(primitiveArrayProp, "PrimitiveArray2", PRIMITIVETYPE_Integer);
    EXPECT_TRUE(ARRAYKIND_Struct == nestedArrayProp->GetKind());
    EXPECT_TRUE(ARRAYKIND_Primitive == primitiveArrayProp->GetKind());
    EXPECT_EQ(0, strcmp(structProp->GetType().GetName().c_str(), "StructClass"));
    EXPECT_EQ(0, strcmp(nestedArrayProp->GetTypeName().c_str(), "StructClass"));
    EXPECT_EQ(0, strcmp(primitiveArrayProp->GetTypeName().c_str(), "int"));

    testSchema->CreateRelationshipClass(relationshipClass, "RelationshipClass");
    EXPECT_TRUE(STRENGTHTYPE_Referencing == relationshipClass->GetStrength());
    relationshipClass->SetStrength(STRENGTHTYPE_Embedding);
    EXPECT_TRUE(STRENGTHTYPE_Embedding == relationshipClass->GetStrength());
    
    EXPECT_TRUE(ECRelatedInstanceDirection::Forward == relationshipClass->GetStrengthDirection());
    relationshipClass->SetStrengthDirection(ECRelatedInstanceDirection::Backward);
    EXPECT_TRUE(ECRelatedInstanceDirection::Backward == relationshipClass->GetStrengthDirection());
    
    EXPECT_TRUE(relationshipClass->GetTarget().GetIsPolymorphic());
    EXPECT_TRUE(relationshipClass->GetSource().GetIsPolymorphic());
    relationshipClass->GetSource().SetIsPolymorphic(false);
    EXPECT_FALSE(relationshipClass->GetSource().GetIsPolymorphic());
    
    relationshipClass->SetDescription("Relates the test class to the related class");
    relationshipClass->SetDisplayLabel("TestRelationshipClass");
    
    EXPECT_EQ(0, relationshipClass->GetSource().GetClasses().size());
    EXPECT_EQ(0, relationshipClass->GetTarget().GetClasses().size());
    
    relationshipClass->GetSource().AddClass(*structClass);
    EXPECT_EQ(1, relationshipClass->GetSource().GetClasses().size());
    relationshipClass->GetSource().AddClass(*class1);
    EXPECT_EQ(2, relationshipClass->GetSource().GetClasses().size());
    
    relationshipClass->GetTarget().AddClass(*relatedClass);
    EXPECT_EQ(1, relationshipClass->GetTarget().GetClasses().size());
    relationshipClass->GetTarget().AddClass(*relatedClass);
    EXPECT_EQ(1, relationshipClass->GetTarget().GetClasses().size());
    relationshipClass->GetTarget().AddClass(*structClass);
    EXPECT_EQ(2, relationshipClass->GetTarget().GetClasses().size());
    
    EXPECT_EQ(0, relationshipClass->GetSource().GetCardinality().GetLowerLimit());
    EXPECT_EQ(0, relationshipClass->GetTarget().GetCardinality().GetLowerLimit());
    EXPECT_EQ(1, relationshipClass->GetSource().GetCardinality().GetUpperLimit());
    EXPECT_EQ(1, relationshipClass->GetTarget().GetCardinality().GetUpperLimit());
    
    relationshipClass->GetSource().SetCardinality(RelationshipCardinality::OneMany());
    EXPECT_EQ(1, relationshipClass->GetSource().GetCardinality().GetLowerLimit());
    EXPECT_TRUE(relationshipClass->GetSource().GetCardinality().IsUpperLimitUnbounded());
    
    RelationshipCardinality *card = new RelationshipCardinality(2, 5);
    relationshipClass->GetTarget().SetCardinality(*card);
    EXPECT_EQ(2, relationshipClass->GetTarget().GetCardinality().GetLowerLimit());
    EXPECT_EQ(5, relationshipClass->GetTarget().GetCardinality().GetUpperLimit());
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaCopyTest, ExpectSuccessWhenCopyingStructs)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath( L"Widgets.01.00.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ (SCHEMA_READ_STATUS_Success, status);

    ECSchemaPtr copiedSchema;
    ECObjectsStatus status2 = schema->CopySchema(copiedSchema);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, status2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, ExpectErrorWithCircularBaseClasses)
    {
    ECSchemaPtr schema;
    ECClassP class1;
    ECClassP baseClass1;
    ECClassP baseClass2;
    
    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    schema->CreateClass(class1, "TestClass");
    schema->CreateClass(baseClass1, "BaseClass1");
    schema->CreateClass(baseClass2, "BaseClass2");
    
    EXPECT_EQ(ECOBJECTS_STATUS_Success, class1->AddBaseClass(*baseClass1));
    EXPECT_EQ(ECOBJECTS_STATUS_Success, baseClass1->AddBaseClass(*baseClass2));
    EXPECT_EQ(ECOBJECTS_STATUS_BaseClassUnacceptable, baseClass2->AddBaseClass(*class1));
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, GetPropertyCount)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema (schema, "TestSchema", 1, 0);

    ECClassP baseClass1, baseClass2, derivedClass, structClass;

    PrimitiveECPropertyP primProp;
    StructECPropertyP structProp;

    // Struct class with 2 properties
    schema->CreateClass (structClass, "StructClass");
    structClass->SetIsStruct (true);
    structClass->CreatePrimitiveProperty (primProp, "StructProp1");
    structClass->CreatePrimitiveProperty (primProp, "StructProp2");

    // 1 base class with 3 primitive properties
    schema->CreateClass (baseClass1, "BaseClass1");
    baseClass1->CreatePrimitiveProperty (primProp, "Base1Prop1");
    baseClass1->CreatePrimitiveProperty (primProp, "Base1Prop2");
    baseClass1->CreatePrimitiveProperty (primProp, "Base1Prop3");

    // 1 base class with 1 primitive and 2 struct properties (each struct has 2 properties
    schema->CreateClass (baseClass2, "BaseClass2");
    baseClass2->CreatePrimitiveProperty (primProp, "Base2Prop1");
    baseClass2->CreateStructProperty (structProp, "Base2Prop2", *structClass);
    baseClass2->CreateStructProperty (structProp, "Base2Prop3", *structClass);

    // Derived class with 1 extra primitive property, 1 extra struct property, derived from 2 base classes
    schema->CreateClass (derivedClass, "DerivedClass");
    derivedClass->CreateStructProperty (structProp, "DerivedProp1", *structClass);
    derivedClass->CreatePrimitiveProperty (primProp, "DerivedProp2");
    derivedClass->AddBaseClass (*baseClass1);
    derivedClass->AddBaseClass (*baseClass2);

    TestPropertyCount (*structClass, 2, 2);
    TestPropertyCount (*baseClass1, 3, 3);
    TestPropertyCount (*baseClass2, 3, 3);
    TestPropertyCount (*derivedClass, 2, 8);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IsClassInList (bvector<ECClassP> const& classList, ECClassR searchClass)
    {
    bvector<ECClassP>::const_iterator classIterator;

    for (classIterator = classList.begin(); classIterator != classList.end(); classIterator++)
        {
        if (*classIterator == &searchClass)
            return true;
        }
        
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, AddAndRemoveBaseClass)
    {
    ECSchemaPtr schema;
    ECClassP class1;
    ECClassP baseClass1;
    
    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    schema->CreateClass(class1, "TestClass");
    schema->CreateClass(baseClass1, "BaseClass");

    EXPECT_EQ(ECOBJECTS_STATUS_Success, class1->AddBaseClass(*baseClass1));

    EXPECT_TRUE (IsClassInList (class1->GetBaseClasses(), *baseClass1));
    EXPECT_TRUE (IsClassInList (baseClass1->GetDerivedClasses(), *class1));

    EXPECT_EQ(ECOBJECTS_STATUS_Success, class1->RemoveBaseClass(*baseClass1));

    EXPECT_FALSE (IsClassInList (class1->GetBaseClasses(), *baseClass1));
    EXPECT_FALSE (IsClassInList (baseClass1->GetDerivedClasses(), *class1));

    EXPECT_EQ(ECOBJECTS_STATUS_ClassNotFound, class1->RemoveBaseClass(*baseClass1));
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, AddBaseClassWithProperties)
    {
    ECSchemaPtr schema;
    ECClassP class1;
    ECClassP baseClass1;
    ECClassP baseClass2;

    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    schema->CreateClass(class1, "TestClass");
    schema->CreateClass(baseClass1, "BaseClass");
    schema->CreateClass(baseClass2, "BaseClass2");

    PrimitiveECPropertyP stringProp;
    PrimitiveECPropertyP baseStringProp;
    PrimitiveECPropertyP intProp;
    PrimitiveECPropertyP base2NonIntProp;

    class1->CreatePrimitiveProperty(stringProp, "StringProperty", PRIMITIVETYPE_String);
    baseClass1->CreatePrimitiveProperty(baseStringProp, "StringProperty", PRIMITIVETYPE_String);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, class1->AddBaseClass(*baseClass1));

    class1->CreatePrimitiveProperty(intProp, "IntProperty", PRIMITIVETYPE_Integer);
    baseClass2->CreatePrimitiveProperty(base2NonIntProp, "IntProperty", PRIMITIVETYPE_String);
    EXPECT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, class1->AddBaseClass(*baseClass2));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, BaseClassOrder)
    {
    ECSchemaPtr schema = nullptr;
    ECClassP class1 = nullptr;
    ECClassP baseClass1 = nullptr;
    ECClassP baseClass2 = nullptr;
    ECClassP baseClass3 = nullptr;

    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    schema->CreateClass(class1, "TestClass");
    schema->CreateClass(baseClass1, "BaseClass");
    schema->CreateClass(baseClass2, "BaseClass2");
    schema->CreateClass(baseClass3, "BaseClass3");

    PrimitiveECPropertyP prop = nullptr;
    class1->CreatePrimitiveProperty(prop, "StringProperty", PRIMITIVETYPE_String);
    baseClass1->CreatePrimitiveProperty(prop, "StringProperty", PRIMITIVETYPE_String);
    baseClass2->CreatePrimitiveProperty(prop, "SstringProperty", PRIMITIVETYPE_String);
    baseClass3->CreatePrimitiveProperty(prop, "StringProperty", PRIMITIVETYPE_String);
    
    ASSERT_EQ(ECOBJECTS_STATUS_Success, class1->AddBaseClass(*baseClass1));
    ASSERT_EQ(ECOBJECTS_STATUS_Success, class1->AddBaseClass(*baseClass2));
    
    ASSERT_EQ(2, class1->GetBaseClasses().size());
    ASSERT_TRUE (baseClass1 == class1->GetBaseClasses()[0]);
    ASSERT_TRUE (baseClass2 == class1->GetBaseClasses()[1]);
    
    ASSERT_EQ(ECOBJECTS_STATUS_Success, class1->AddBaseClass(*baseClass3, true));
    ASSERT_EQ(3, class1->GetBaseClasses().size());
    ASSERT_TRUE (baseClass3 == class1->GetBaseClasses()[0]);
    ASSERT_TRUE (baseClass1 == class1->GetBaseClasses()[1]);
    ASSERT_TRUE (baseClass2 == class1->GetBaseClasses()[2]);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, IsTests)
    {
    ECSchemaPtr schema;
    ECClassP class1;
    ECClassP baseClass1;
    ECClassP baseClass2;
    
    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    schema->CreateClass(class1, "TestClass");
    schema->CreateClass(baseClass1, "BaseClass1");
    schema->CreateClass(baseClass2, "BaseClass2");
    
    EXPECT_FALSE(class1->Is(baseClass1));
    class1->AddBaseClass(*baseClass1);
    EXPECT_TRUE(class1->Is(baseClass1));
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, CanOverrideBaseProperties)
    {
    ECSchemaPtr schema;
    ECClassP class1;
    ECClassP baseClass1;
    ECClassP structClass;
    ECClassP structClass2;
    
    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    schema->CreateClass(class1, "TestClass");
    schema->CreateClass(baseClass1, "BaseClass1");
    schema->CreateClass(structClass, "ClassForStructs");
    structClass->SetIsStruct(true);
    schema->CreateClass(structClass2, "ClassForStructs2");
    structClass2->SetIsStruct(true);
    class1->AddBaseClass(*baseClass1);
    
    PrimitiveECPropertyP baseStringProp;
    PrimitiveECPropertyP baseIntProp;
    PrimitiveECPropertyP baseDoubleProp;
    StructECPropertyP baseStructProp;
    ArrayECPropertyP baseStringArrayProperty;
    ArrayECPropertyP baseStructProperty;
    
    baseClass1->CreatePrimitiveProperty(baseStringProp, "StringProperty", PRIMITIVETYPE_String);
    baseClass1->CreatePrimitiveProperty(baseIntProp, "IntegerProperty", PRIMITIVETYPE_Integer);
    baseClass1->CreatePrimitiveProperty(baseDoubleProp, "DoubleProperty", PRIMITIVETYPE_Double);
    baseClass1->CreateStructProperty(baseStructProp, "StructProperty", *structClass);
    baseClass1->CreateArrayProperty(baseStringArrayProperty, "StringArrayProperty", PRIMITIVETYPE_String);
    baseClass1->CreateArrayProperty(baseStructProperty, "StructArrayProperty", structClass);
    
    PrimitiveECPropertyP longProperty = NULL;
    PrimitiveECPropertyP stringProperty = NULL;
    
    DISABLE_ASSERTS;
    // Primitives overriding primitives
    EXPECT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, class1->CreatePrimitiveProperty(longProperty, "StringProperty", PRIMITIVETYPE_Long));
    EXPECT_EQ(NULL, longProperty);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, class1->CreatePrimitiveProperty(stringProperty, "StringProperty", PRIMITIVETYPE_String));
    EXPECT_EQ(baseStringProp, stringProperty->GetBaseProperty());
    class1->RemoveProperty("StringProperty");
    
    {
    // Primitives overriding structs
    DISABLE_ASSERTS
    EXPECT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, class1->CreatePrimitiveProperty(longProperty, "StructProperty", PRIMITIVETYPE_Long));
    }

    // Primitives overriding arrays
    EXPECT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, class1->CreatePrimitiveProperty(longProperty, "StringArrayProperty", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, class1->CreatePrimitiveProperty(stringProperty, "StringArrayProperty", PRIMITIVETYPE_String));
    class1->RemoveProperty("StringArrayProperty");

    StructECPropertyP structProperty;

    {
    // Structs overriding primitives
    DISABLE_ASSERTS
    EXPECT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, class1->CreateStructProperty(structProperty, "IntegerProperty"));
    }

    // Structs overriding structs
    // If we don't specify a struct type for the new property, then it should succeed
    EXPECT_EQ(ECOBJECTS_STATUS_Success, class1->CreateStructProperty(structProperty, "StructProperty"));
    class1->RemoveProperty("StructProperty");
    EXPECT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, class1->CreateStructProperty(structProperty, "StructProperty", *structClass2));

    // Structs overriding arrays
    EXPECT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, class1->CreateStructProperty(structProperty, "StringArrayProperty"));
    EXPECT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, class1->CreateStructProperty(structProperty, "StringArrayProperty", *structClass));
    EXPECT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, class1->CreateStructProperty(structProperty, "StructArrayProperty"));
    EXPECT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, class1->CreateStructProperty(structProperty, "StructArrayProperty", *structClass));
    EXPECT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, class1->CreateStructProperty(structProperty, "StructArrayProperty", *structClass2));

    ArrayECPropertyP stringArrayProperty;
    ArrayECPropertyP stringArrayProperty2;
    ArrayECPropertyP structArrayProperty;
    ArrayECPropertyP structArrayProperty2;
    // Arrays overriding primitives
    EXPECT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, class1->CreateArrayProperty(stringArrayProperty, "IntegerProperty", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, class1->CreateArrayProperty(stringArrayProperty, "StringProperty", PRIMITIVETYPE_String));
    EXPECT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, class1->CreateArrayProperty(stringArrayProperty2, "StringProperty"));

    // Arrays overriding structs
    EXPECT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, class1->CreateArrayProperty(structArrayProperty, "StructProperty", structClass2));
    EXPECT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, class1->CreateArrayProperty(structArrayProperty, "StructProperty", structClass));

    // the default array type is string if none is passed in
    EXPECT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, class1->CreateArrayProperty(structArrayProperty2, "StructProperty"));
    
    ArrayECPropertyP intArrayProperty;
    // Arrays overriding arrays
    EXPECT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, class1->CreateArrayProperty(intArrayProperty, "StringArrayProperty", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECOBJECTS_STATUS_Success, class1->CreateArrayProperty(stringArrayProperty, "StringArrayProperty", PRIMITIVETYPE_String));
    class1->RemoveProperty("StringArrayProperty");

    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, ExpectFailureWhenStructTypeIsNotReferenced)
    {
    ECSchemaPtr schema;
    ECSchemaPtr schema2;
    ECClassP class1;
    ECClassP structClass;
    ECClassP structClass2;

    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    ECSchema::CreateSchema(schema2, "TestSchema2", 5, 5);
    schema->CreateClass(class1, "TestClass");
    schema2->CreateClass(structClass, "ClassForStructs");
    structClass->SetIsStruct(true);
    schema->CreateClass(structClass2, "ClassForStructs2");
    structClass2->SetIsStruct(true);

    StructECPropertyP baseStructProp;
    ArrayECPropertyP structArrayProperty;
    StructECPropertyP baseStructProp2;
    ArrayECPropertyP structArrayProperty2;

    EXPECT_EQ(ECOBJECTS_STATUS_SchemaNotFound, class1->CreateStructProperty(baseStructProp, "StructProperty", *structClass));
    EXPECT_EQ(ECOBJECTS_STATUS_SchemaNotFound, class1->CreateArrayProperty(structArrayProperty, "StructArrayProperty", structClass));
    EXPECT_EQ(ECOBJECTS_STATUS_Success, class1->CreateStructProperty(baseStructProp2, "StructProperty2", *structClass2));
    EXPECT_EQ(ECOBJECTS_STATUS_Success, class1->CreateArrayProperty(structArrayProperty2, "StructArrayProperty2", structClass2));
    schema->AddReferencedSchema(*schema2);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, class1->CreateStructProperty(baseStructProp, "StructProperty", *structClass));
    EXPECT_EQ(ECOBJECTS_STATUS_Success, class1->CreateArrayProperty(structArrayProperty, "StructArrayProperty", structClass));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, ExpectPropertiesInOrder)
    {
    std::vector<Utf8CP> propertyNames;
    propertyNames.push_back("beta");
    propertyNames.push_back("gamma");
    propertyNames.push_back("delta");
    propertyNames.push_back("alpha");
    
    ECSchemaPtr schema;
    ECClassP class1;
    PrimitiveECPropertyP property1;
    PrimitiveECPropertyP property2;
    PrimitiveECPropertyP property3;
    PrimitiveECPropertyP property4;
    
    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    schema->CreateClass(class1, "TestClass");
    class1->CreatePrimitiveProperty(property1, "beta");
    class1->CreatePrimitiveProperty(property2, "gamma");
    class1->CreatePrimitiveProperty(property3, "delta");
    class1->CreatePrimitiveProperty(property4, "alpha");
    
    int i = 0;
    ECPropertyIterable  iterable = class1->GetProperties (false);
    for (ECPropertyP prop: iterable)
        {
        EXPECT_EQ(0, prop->GetName().compare(propertyNames[i]));
        i++;
        }
    }
   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, ExpectProperties)
    {
    ECSchemaPtr schema;
    ECClassP ab;
    ECClassP cd;
    ECClassP ef;
    
    PrimitiveECPropertyP a;
    PrimitiveECPropertyP b;
    PrimitiveECPropertyP c;
    PrimitiveECPropertyP d;
    PrimitiveECPropertyP e;
    PrimitiveECPropertyP f;
    
    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    schema->CreateClass(ab, "ab");
    schema->CreateClass(cd, "cd");
    schema->CreateClass(ef, "ef");

    ab->CreatePrimitiveProperty(a, "a");
    ab->CreatePrimitiveProperty(b, "b");

    cd->CreatePrimitiveProperty(c, "c");
    cd->CreatePrimitiveProperty(d, "d");
    
    ef->CreatePrimitiveProperty(e, "e");
    ef->CreatePrimitiveProperty(f, "f");
    
    cd->AddBaseClass(*ab);
    ef->AddBaseClass(*cd);

    EXPECT_TRUE(NULL != GetPropertyByName (*ef, "e"));    
    EXPECT_TRUE(NULL != GetPropertyByName (*ef, "c"));    
    EXPECT_TRUE(NULL != GetPropertyByName (*ef, "a"));    
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, ExpectPropertiesFromBaseClass)
    {
    ECSchemaPtr schema;
    ECClassP ab;
    ECClassP cd;
    ECClassP ef;
    ECClassP gh;
    ECClassP ij;
    ECClassP kl;
    ECClassP mn;
    
    PrimitiveECPropertyP a;
    PrimitiveECPropertyP b;
    PrimitiveECPropertyP c;
    PrimitiveECPropertyP d;
    PrimitiveECPropertyP e;
    PrimitiveECPropertyP f;
    PrimitiveECPropertyP g;
    PrimitiveECPropertyP h;
    PrimitiveECPropertyP i;
    PrimitiveECPropertyP j;
    PrimitiveECPropertyP k;
    PrimitiveECPropertyP l;
    PrimitiveECPropertyP m;
    PrimitiveECPropertyP n;
    
    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    schema->CreateClass(ab, "ab");
    schema->CreateClass(cd, "cd");
    schema->CreateClass(ef, "ef");
    schema->CreateClass(gh, "gh");
    schema->CreateClass(ij, "ij");
    schema->CreateClass(kl, "kl");
    schema->CreateClass(mn, "mn");

    ab->CreatePrimitiveProperty(a, "a");
    ab->CreatePrimitiveProperty(b, "b");

    cd->CreatePrimitiveProperty(c, "c");
    cd->CreatePrimitiveProperty(d, "d");
    
    ef->CreatePrimitiveProperty(e, "e");
    ef->CreatePrimitiveProperty(f, "f");
    
    gh->CreatePrimitiveProperty(g, "g");
    gh->CreatePrimitiveProperty(h, "h");
    
    ij->CreatePrimitiveProperty(i, "i");
    ij->CreatePrimitiveProperty(j, "j");
    
    kl->CreatePrimitiveProperty(k, "k");
    kl->CreatePrimitiveProperty(l, "l");
    
    mn->CreatePrimitiveProperty(m, "m");
    mn->CreatePrimitiveProperty(n, "n");
    
    ef->AddBaseClass(*ab);
    ef->AddBaseClass(*cd);
    
    kl->AddBaseClass(*gh);
    kl->AddBaseClass(*ij);
    
    mn->AddBaseClass(*ef);
    mn->AddBaseClass(*kl);
    
    ECPropertyIterable  iterable1 = mn->GetProperties (true);
    std::vector<ECPropertyP> testVector;
    for (ECPropertyP prop: iterable1)
        testVector.push_back(prop);
        
    EXPECT_EQ(14, testVector.size());
    EXPECT_EQ(0, testVector[0]->GetName().compare("i"));
    EXPECT_EQ(0, testVector[1]->GetName().compare("j"));
    EXPECT_EQ(0, testVector[2]->GetName().compare("g"));
    EXPECT_EQ(0, testVector[3]->GetName().compare("h"));
    EXPECT_EQ(0, testVector[4]->GetName().compare("k"));
    EXPECT_EQ(0, testVector[5]->GetName().compare("l"));
    EXPECT_EQ(0, testVector[6]->GetName().compare("c"));
    EXPECT_EQ(0, testVector[7]->GetName().compare("d"));
    EXPECT_EQ(0, testVector[8]->GetName().compare("a"));
    EXPECT_EQ(0, testVector[9]->GetName().compare("b"));
    EXPECT_EQ(0, testVector[10]->GetName().compare("e"));
    EXPECT_EQ(0, testVector[11]->GetName().compare("f"));
    EXPECT_EQ(0, testVector[12]->GetName().compare("m"));
    EXPECT_EQ(0, testVector[13]->GetName().compare("n"));
    
    // now we add some duplicate properties to mn which will "override" those from the base classes
    PrimitiveECPropertyP b2;
    PrimitiveECPropertyP d2;
    PrimitiveECPropertyP f2;
    PrimitiveECPropertyP h2;
    PrimitiveECPropertyP j2;
    PrimitiveECPropertyP k2;
    
    mn->CreatePrimitiveProperty(b2, "b");
    mn->CreatePrimitiveProperty(d2, "d");
    mn->CreatePrimitiveProperty(f2, "f");
    mn->CreatePrimitiveProperty(h2, "h");
    mn->CreatePrimitiveProperty(j2, "j");
    mn->CreatePrimitiveProperty(k2, "k");

    ECPropertyIterable  iterable2 = mn->GetProperties (true);
    testVector.clear();
    for (ECPropertyP prop: iterable2)
        testVector.push_back(prop);
        
    EXPECT_EQ(14, testVector.size());
    EXPECT_EQ(0, testVector[0]->GetName().compare("i"));
    EXPECT_EQ(0, testVector[1]->GetName().compare("g"));
    EXPECT_EQ(0, testVector[2]->GetName().compare("l"));
    EXPECT_EQ(0, testVector[3]->GetName().compare("c"));
    EXPECT_EQ(0, testVector[4]->GetName().compare("a"));
    EXPECT_EQ(0, testVector[5]->GetName().compare("e"));
    EXPECT_EQ(0, testVector[6]->GetName().compare("m"));
    EXPECT_EQ(0, testVector[7]->GetName().compare("n"));
    EXPECT_EQ(0, testVector[8]->GetName().compare("b"));
    EXPECT_EQ(0, testVector[9]->GetName().compare("d"));
    EXPECT_EQ(0, testVector[10]->GetName().compare("f"));
    EXPECT_EQ(0, testVector[11]->GetName().compare("h"));
    EXPECT_EQ(0, testVector[12]->GetName().compare("j"));
    EXPECT_EQ(0, testVector[13]->GetName().compare("k"));

    PrimitiveECPropertyP e2;
    PrimitiveECPropertyP a2;
    PrimitiveECPropertyP c2;
    PrimitiveECPropertyP g2;

    PrimitiveECPropertyP l2;
    PrimitiveECPropertyP i2;
    PrimitiveECPropertyP g3;

    PrimitiveECPropertyP a3;
    PrimitiveECPropertyP b3;
    PrimitiveECPropertyP g4;
    PrimitiveECPropertyP h3;

    kl->CreatePrimitiveProperty(e2, "e");
    kl->CreatePrimitiveProperty(a2, "a");
    kl->CreatePrimitiveProperty(c2, "c");
    kl->CreatePrimitiveProperty(g2, "g");
    
    ef->CreatePrimitiveProperty(l2, "l");
    gh->CreatePrimitiveProperty(i2, "i");
    ij->CreatePrimitiveProperty(g3, "g");
    
    gh->CreatePrimitiveProperty(a3, "a");
    gh->CreatePrimitiveProperty(b3, "b");
    ab->CreatePrimitiveProperty(g4, "g");
    ab->CreatePrimitiveProperty(h3, "h");

    ECPropertyIterable  iterable3 = mn->GetProperties (true);
    testVector.clear();
    for (ECPropertyP prop: iterable3)
        testVector.push_back(prop);
        
    EXPECT_EQ(14, testVector.size());
    EXPECT_EQ(0, testVector[0]->GetName().compare("i"));
    EXPECT_EQ(0, testVector[1]->GetName().compare("c"));
    EXPECT_EQ(0, testVector[2]->GetName().compare("a"));
    EXPECT_EQ(0, testVector[3]->GetName().compare("g"));
    EXPECT_EQ(0, testVector[4]->GetName().compare("e"));
    EXPECT_EQ(0, testVector[5]->GetName().compare("l"));
    EXPECT_EQ(0, testVector[6]->GetName().compare("m"));
    EXPECT_EQ(0, testVector[7]->GetName().compare("n"));
    EXPECT_EQ(0, testVector[8]->GetName().compare("b"));
    EXPECT_EQ(0, testVector[9]->GetName().compare("d"));
    EXPECT_EQ(0, testVector[10]->GetName().compare("f"));
    EXPECT_EQ(0, testVector[11]->GetName().compare("h"));
    EXPECT_EQ(0, testVector[12]->GetName().compare("j"));
    EXPECT_EQ(0, testVector[13]->GetName().compare("k"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, AddAndRemoveConstraintClasses)
    {
    

    ECSchemaPtr schema;
    ECSchemaPtr refSchema;
    
    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    ECSchema::CreateSchema(refSchema, "RefSchema", 5, 5);
    
    ECRelationshipClassP relClass;
    ECClassP targetClass;
    ECClassP sourceClass;
    
    schema->CreateRelationshipClass(relClass, "RElationshipClass");
    schema->CreateClass(targetClass, "Target");
    refSchema->CreateClass(sourceClass, "Source");
    
    EXPECT_EQ(ECOBJECTS_STATUS_Success, relClass->GetTarget().AddClass(*targetClass));
    EXPECT_EQ(ECOBJECTS_STATUS_SchemaNotFound, relClass->GetSource().AddClass(*sourceClass));
    
    schema->AddReferencedSchema(*refSchema);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, relClass->GetSource().AddClass(*sourceClass));
    
    EXPECT_EQ(ECOBJECTS_STATUS_Success, relClass->GetTarget().RemoveClass(*targetClass));
    EXPECT_EQ(ECOBJECTS_STATUS_ClassNotFound, relClass->GetTarget().RemoveClass(*targetClass));
    }
    
TEST_F(ClassTest, ExpectReadOnlyFromBaseClass)
    {
    ECSchemaPtr schema;
    ECClassP child;
    ECClassP base;
    
    PrimitiveECPropertyP readOnlyProp;
    
    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    schema->CreateClass(base, "BaseClass");
    schema->CreateClass(child, "ChildClass");

    base->CreatePrimitiveProperty(readOnlyProp, "readOnlyProp");
    readOnlyProp->SetIsReadOnly(true);

    ASSERT_EQ(ECOBJECTS_STATUS_Success, child->AddBaseClass(*base));

    ECPropertyP ecProp = GetPropertyByName (*child, "readOnlyProp");
    ASSERT_EQ(true, ecProp->GetIsReadOnly());

    }

void TestOverriding
(
Utf8CP schemaName,
int majorVersion,
bool allowOverriding
)
    {
    ECSchemaPtr schema;
    ECClassP base;
    ECClassP child;

    ECSchema::CreateSchema(schema, schemaName, majorVersion, 5);
    schema->CreateClass(base, "base");
    schema->CreateClass(child, "child");

    PrimitiveECPropertyP baseIntProp;
    ArrayECPropertyP baseIntArrayProperty;
    ArrayECPropertyP baseStringArrayProperty;
    ArrayECPropertyP baseBoolArrayProperty;

    base->CreatePrimitiveProperty(baseIntProp, "IntegerProperty", PRIMITIVETYPE_Integer);
    base->CreateArrayProperty(baseIntArrayProperty, "IntArrayProperty", PRIMITIVETYPE_Integer);
    base->CreateArrayProperty(baseStringArrayProperty, "StringArrayProperty", PRIMITIVETYPE_String);
    base->CreateArrayProperty(baseBoolArrayProperty, "BoolArrayProperty", PRIMITIVETYPE_Boolean);

    PrimitiveECPropertyP childIntProperty;
    ArrayECPropertyP childIntArrayProperty;
    ArrayECPropertyP childStringArrayProperty;
    ArrayECPropertyP childBoolArrayProperty;

    child->AddBaseClass(*base);
    // Override an integer property with an array of ints
    ECObjectsStatus status = child->CreateArrayProperty(childIntArrayProperty, "IntegerProperty", PRIMITIVETYPE_Integer);
    if (allowOverriding)
        {
        ASSERT_EQ(ECOBJECTS_STATUS_Success, status);
        child->RemoveProperty("IntegerProperty");
        }
    else
        ASSERT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, status);

    // Override an integer property with an array of strings
    status = child->CreateArrayProperty(childStringArrayProperty, "IntegerProperty", PRIMITIVETYPE_String);
    ASSERT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, status);

    // Override an integer array with an integer
    status = child->CreatePrimitiveProperty(childIntProperty, "IntArrayProperty", PRIMITIVETYPE_Integer);
    if (allowOverriding)
        {
        ASSERT_EQ(ECOBJECTS_STATUS_Success, status);
        child->RemoveProperty("IntArrayProperty");
        }
    else
        ASSERT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, status);

    // Override an array of strings with an integer
    status = child->CreatePrimitiveProperty(childIntProperty, "StringArrayProperty", PRIMITIVETYPE_Integer);
    ASSERT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, status);

    // Override an array of boolean with an array of integers
    status = child->CreateArrayProperty(childIntArrayProperty, "BoolArrayProperty", PRIMITIVETYPE_Integer);
    ASSERT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, status);

    // Override an array of integers with an array of boolean
    status = child->CreateArrayProperty(childBoolArrayProperty, "IntArrayProperty", PRIMITIVETYPE_Boolean);
    ASSERT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, status);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, TestOverridingArrayPropertyWithNonArray)
    {
    TestOverriding("TestSchema", 5, false);
    TestOverriding("jclass", 1, true);
    TestOverriding("jclass", 2, true);
    TestOverriding("ECXA_ams", 1, true);
    TestOverriding("ECXA_ams_user", 1, true);
    TestOverriding("ams", 1, true);
    TestOverriding("ams_user", 1, true);
    TestOverriding("Bentley_JSpace_CustomAttributes", 2, true);
    TestOverriding("Bentley_Plant", 6, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECNameValidationTest : ECTestFixture
    {
    struct ITester
        {
        virtual void        Preprocess (ECSchemaR schema) const = 0;
        virtual void        Postprocess (ECSchemaR schema) const = 0;
        };

    void Roundtrip (ITester const& tester)
        {
        ECSchemaPtr schema;
        ECSchema::CreateSchema (schema, "MySchema", 1, 1);
        tester.Preprocess (*schema);

        Utf8String schemaXml;
        EXPECT_EQ (SCHEMA_WRITE_STATUS_Success, schema->WriteToXmlString (schemaXml));

        schema = NULL;
        ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
        EXPECT_EQ (SCHEMA_READ_STATUS_Success, ECSchema::ReadFromXmlString (schema, schemaXml.c_str(), *context));

        tester.Postprocess (*schema);
        }
    };
    
/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct DisplayLabelTester : ECNameValidationTest::ITester
    {
    Utf8String         m_name;
    Utf8String         m_encodedName;

    DisplayLabelTester (Utf8CP name, Utf8CP encodedName) : m_name(name), m_encodedName(encodedName) { }

    template<typename T> void Compare (T const& target) const
        {
        EXPECT_FALSE (target.GetIsDisplayLabelDefined());
        EXPECT_TRUE (target.GetName().Equals (m_encodedName)) << "Name: Expected " << m_encodedName.c_str() << " Actual " << target.GetName().c_str();
        EXPECT_TRUE (target.GetDisplayLabel().Equals (m_name)) << "Label: Expected " << m_name.c_str() << " Actual " << target.GetDisplayLabel().c_str();
        }

    template<typename T> void CompareOverriddenLabel(T const& target, Utf8CP label) const
        {
        EXPECT_TRUE (target.GetIsDisplayLabelDefined());
        EXPECT_TRUE (target.GetDisplayLabel().Equals (label));
        }

    virtual void Preprocess (ECSchemaR schema) const override
        {
        schema.SetName (m_name);
        Compare (schema);

        ECClassP ecclass;
        schema.CreateClass (ecclass, m_name);
        Compare (*ecclass);

        PrimitiveECPropertyP ecprop;
        ecclass->CreatePrimitiveProperty (ecprop, m_name, PRIMITIVETYPE_String);
        Compare (*ecprop);
        }

    virtual void Postprocess (ECSchemaR schema) const override
        {
        ECClassP ecclass = schema.GetClassP (m_encodedName.c_str());
        ECPropertyP ecprop = GetPropertyByName (*ecclass, m_encodedName.c_str());

        Compare (schema);
        Compare (*ecclass);
        Compare (*ecprop);

        // Test explicitly setting display labels
        schema.SetDisplayLabel ("NewDisplayLabel");
        CompareOverriddenLabel (schema, "NewDisplayLabel");
        ecclass->SetDisplayLabel ("1!@$");
        CompareOverriddenLabel (*ecclass, "1!@$");                // will not be encoded
        ecprop->SetDisplayLabel ("__x003E__");
        CompareOverriddenLabel (*ecprop, "__x003E__");            // will not be decoded

        // Test explicitly un-setting display labels
        ecclass->SetDisplayLabel ("");
        Compare (*ecclass);
        ecprop->SetDisplayLabel ("");
        Compare (*ecprop);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECNameValidationTest, DisplayLabels)
    {
    static const Utf8CP s_testValues[] =
        {
            "NothingSpecial", "NothingSpecial",
            "Nothing1Special2", "Nothing1Special2",
            "1_LeadingDigitsDisallowed", "__x0031___LeadingDigitsDisallowed",
            "Special!", "Special__x0021__",
            "thing@mail.com", "thing__x0040__mail__x002E__com",
            "*", "__x002A__",
            "9&:", "__x0039____x0026____x003A__",
            "__xNotAChar__", "__xNotAChar__",
            "__xTTTT__", "__xTTTT__",
            "__x####__", "__x__x0023____x0023____x0023____x0023____",
            NULL, NULL
        };

    for (Utf8CP const* cur = s_testValues; *cur; cur += 2)
        {
        Utf8CP name = *cur, encoded = *(cur+1);
        Roundtrip (DisplayLabelTester (name, encoded));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECNameValidationTest, Validate)
    {
#define EXPECT_VALIDATION_RESULT(RESULTTOEXPECT, NAMETOTEST) EXPECT_EQ (ECNameValidation::RESULT_ ## RESULTTOEXPECT, ECNameValidation::Validate (NAMETOTEST))
    EXPECT_VALIDATION_RESULT(Valid, "ThisIsAValidName");
    EXPECT_VALIDATION_RESULT(Valid, "_123");
    EXPECT_VALIDATION_RESULT(Valid, "___");
    EXPECT_VALIDATION_RESULT(Valid, "A123");

    EXPECT_VALIDATION_RESULT(NullOrEmpty, "");
    EXPECT_VALIDATION_RESULT(NullOrEmpty, NULL);

    EXPECT_VALIDATION_RESULT(BeginsWithDigit, "1_C");

    EXPECT_VALIDATION_RESULT(IncludesInvalidCharacters, "!ABC");
    EXPECT_VALIDATION_RESULT(IncludesInvalidCharacters, "ABC@");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaDeserializationTest, ExpectErrorWhenBaseClassNotFound)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr refSchema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString (refSchema, 
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"ReferencedSchema\" version=\"01.00\" displayLabel=\"Display Label\" description=\"Description\" nameSpacePrefix=\"ref\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" xmlns:ec=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" xmlns:ods=\"Bentley_ODS.01.02\">"
        "    <ECClass typeName=\"BaseClass\" description=\"Project ECClass\" displayLabel=\"Project\" isDomainClass=\"True\" />"
        "</ECSchema>", *schemaContext);

    EXPECT_EQ (SCHEMA_READ_STATUS_Success, status);

    ECSchemaPtr schema;
    status = ECSchema::ReadFromXmlString (schema, 
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"Stuff\" version=\"09.06\" displayLabel=\"Display Label\" description=\"Description\" nameSpacePrefix=\"stuff\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" xmlns:ec=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" xmlns:ods=\"Bentley_ODS.01.02\">"
        "<ECSchemaReference name=\"ReferencedSchema\" version=\"01.00\" prefix=\"ref\" />"
        "    <ECClass typeName=\"ecProject\" description=\"Project ECClass\" displayLabel=\"Project\" isDomainClass=\"True\">"
        "       <BaseClass>BaseClassDOESNOTEXIST</BaseClass>"
        "       <BaseClass>ref:BaseClass</BaseClass>"
        "    </ECClass>"
        "</ECSchema>", *schemaContext);

    EXPECT_NE (SCHEMA_READ_STATUS_Success, status);    
    EXPECT_TRUE (schema.IsNull());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   06/15
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaDeserializationTest, TestMultipleConstraintClasses)
    {
    Utf8CP schemaXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                       "<ECSchema xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" schemaName=\"ReferencedSchema\" nameSpacePrefix=\"ref\" version=\"01.00\" description=\"Description\" displayLabel=\"Display Label\" xmlns:ec=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                       "  <ECClass typeName = \"Class\" isDomainClass = \"True\">"
                       "      <ECProperty propertyName = \"Property\" typeName = \"string\" />"
                       "  </ECClass>"
                       "  <ECClass typeName = \"Class1\" isDomainClass = \"True\">"
                       "      <ECProperty propertyName = \"Property1\" typeName = \"string\" />"
                       "      <ECProperty propertyName = \"Property2\" typeName = \"string\" />"
                       "  </ECClass>"
                       "  <ECClass typeName = \"Class2\" isDomainClass = \"True\">"
                       "      <ECProperty propertyName = \"Property3\" typeName = \"string\" />"
                       "      <ECProperty propertyName = \"Property4\" typeName = \"string\" />"
                       "  </ECClass>"
                       "  <ECRelationshipClass typeName = \"ClassHasClass1Or2\" isDomainClass = \"True\" strength = \"referencing\" strengthDirection = \"forward\">"
                       "      <Source cardinality = \"(0, 1)\" polymorphic = \"True\">"
                       "          <Class class = \"Class\" />"
                       "      </Source>"
                       "      <Target cardinality = \"(0, 1)\" polymorphic = \"True\">"
                       "          <Class class = \"Class1\" />"
                       "          <Class class = \"Class2\" />"
                       "      </Target>"
                       "  </ECRelationshipClass>"
                       "</ECSchema>";

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr ecSchema = nullptr;
    ASSERT_EQ(SCHEMA_READ_STATUS_Success, ECSchema::ReadFromXmlString(ecSchema, schemaXml, *schemaContext));
    ECRelationshipClassCP relClass = ecSchema->GetClassCP("ClassHasClass1Or2")->GetRelationshipClassCP();
    ASSERT_TRUE(relClass != nullptr);
    ASSERT_EQ(1, relClass->GetSource().GetConstraintClasses().size());
    ASSERT_STREQ("Class", relClass->GetSource().GetClasses()[0]->GetName().c_str());

    ASSERT_EQ(2, relClass->GetTarget().GetConstraintClasses().size());
    ASSERT_STREQ("Class1", relClass->GetTarget().GetClasses()[0]->GetName().c_str());
    ASSERT_STREQ("Class2", relClass->GetTarget().GetClasses()[1]->GetName().c_str());
    }

//---------------------------------------------------------------------------------
// @bsimethod                                                    Paul.Connelly   11/12
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaDeserializationTest, TestRelationshipKeys)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr ecSchema;
    Utf8String schemaString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" schemaName=\"ReferencedSchema\" nameSpacePrefix=\"ref\" version=\"01.00\" description=\"Description\" displayLabel=\"Display Label\" xmlns:ec=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECClass typeName = \"Class\" isDomainClass = \"True\">"
        "      <ECProperty propertyName = \"Property\" typeName = \"string\" />"
        "  </ECClass>"
        "  <ECClass typeName = \"Class1\" isDomainClass = \"True\">"
        "      <ECProperty propertyName = \"Property1\" typeName = \"string\" />"
        "      <ECProperty propertyName = \"Property2\" typeName = \"string\" />"
        "  </ECClass>"
        "  <ECRelationshipClass typeName = \"RelationshipClass\" isDomainClass = \"True\" strength = \"referencing\" strengthDirection = \"forward\">"
        "      <Source cardinality = \"(0, 1)\" polymorphic = \"True\">"
        "          <Class class = \"Class\">"
        "              <Key>"
        "                  <Property name = \"Property\" />"
        "              </Key>"
        "          </Class>"
        "      </Source>"
        "      <Target cardinality = \"(0, 1)\" polymorphic = \"True\">"
        "          <Class class = \"Class1\">"
        "              <Key>"
        "                  <Property name = \"Property1\" />"
        "                  <Property name = \"Property2\" />"
        "              </Key>"
        "          </Class>"
        "      </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>");

    ASSERT_EQ(SCHEMA_READ_STATUS_Success, ECSchema::ReadFromXmlString(ecSchema, schemaString.c_str(), *schemaContext));
    ECRelationshipClassP relClass= ecSchema->GetClassP("RelationshipClass")->GetRelationshipClassP();
    for (auto constrainClass : relClass->GetSource().GetConstraintClasses())
        {
        Utf8String key = constrainClass->GetKeys().at(0);
        ASSERT_TRUE(key.Equals("Property"));
        }
   
    for (auto constrainClass : relClass->GetTarget().GetConstraintClasses())
        {
        auto keys = constrainClass->GetKeys();
        ASSERT_EQ(2, keys.size());
        ASSERT_TRUE(((Utf8String)keys[0]).Equals("Property1"));
        ASSERT_TRUE(((Utf8String)keys[1]).Equals("Property2"));
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   06/15
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaDeserializationTest, TestMultipleConstraintClassesWithKeyProperties)
    {
    Utf8CP schemaXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" schemaName=\"ReferencedSchema\" nameSpacePrefix=\"ref\" version=\"01.00\" description=\"Description\" displayLabel=\"Display Label\" xmlns:ec=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECClass typeName = \"Class\" isDomainClass = \"True\">"
        "      <ECProperty propertyName = \"Property\" typeName = \"string\" />"
        "  </ECClass>"
        "  <ECClass typeName = \"Class1\" isDomainClass = \"True\">"
        "      <ECProperty propertyName = \"Property1\" typeName = \"string\" />"
        "      <ECProperty propertyName = \"Property2\" typeName = \"string\" />"
        "  </ECClass>"
        "  <ECClass typeName = \"Class2\" isDomainClass = \"True\">"
        "      <ECProperty propertyName = \"Property3\" typeName = \"string\" />"
        "      <ECProperty propertyName = \"Property4\" typeName = \"string\" />"
        "  </ECClass>"
        "  <ECRelationshipClass typeName = \"ClassHasClass1Or2\" isDomainClass = \"True\" strength = \"referencing\" strengthDirection = \"forward\">"
        "      <Source cardinality = \"(0, 1)\" polymorphic = \"True\">"
        "          <Class class = \"Class\" />"
        "      </Source>"
        "      <Target cardinality = \"(0, 1)\" polymorphic = \"True\">"
        "          <Class class = \"Class1\">"
        "              <Key>"
        "                  <Property name = \"Property1\" />"
        "                  <Property name = \"Property2\" />"
        "              </Key>"
        "          </Class>"
        "          <Class class = \"Class2\" />"
        "      </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr ecSchema = nullptr;
    ASSERT_EQ (SCHEMA_READ_STATUS_Success, ECSchema::ReadFromXmlString(ecSchema, schemaXml, *schemaContext));
    ECRelationshipClassCP relClass = ecSchema->GetClassCP("ClassHasClass1Or2")->GetRelationshipClassCP();
    ASSERT_TRUE(relClass != nullptr);
    ASSERT_EQ(1, relClass->GetSource().GetConstraintClasses().size());
    ASSERT_STREQ("Class", relClass->GetSource().GetClasses()[0]->GetName().c_str());

    ASSERT_EQ(2, relClass->GetTarget().GetConstraintClasses().size());
    ECRelationshipConstraintClassCP constraintClass1 = relClass->GetTarget().GetConstraintClasses()[0];
    ASSERT_STREQ("Class1", constraintClass1->GetClass().GetName().c_str());

    ASSERT_EQ(2, constraintClass1->GetKeys().size());
    ASSERT_STREQ("Property1", constraintClass1->GetKeys()[0].c_str());
    ASSERT_STREQ("Property2", constraintClass1->GetKeys()[1].c_str());

    ECRelationshipConstraintClassCP constraintClass2 = relClass->GetTarget().GetConstraintClasses()[1];
    ASSERT_STREQ("Class2", constraintClass2->GetClass().GetName().c_str());
    ASSERT_EQ(0, constraintClass2->GetKeys().size());
    }
END_BENTLEY_ECN_TEST_NAMESPACE
