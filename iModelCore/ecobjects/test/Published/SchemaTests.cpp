/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/nativeatp/Published/SchemaTests.cpp $
|
|  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include <objbase.h>
#include <comdef.h>
#include "TestFixture.h"

BEGIN_BENTLEY_EC_NAMESPACE

// NEEDSWORK Improve strategy for seed data.  Should not be maintained in source.
#define SCHEMAS_PATH  L"" 

struct SchemaDeserializationTest : ECTestFixture {};
struct SchemaSerializationTest   : ECTestFixture {};
struct SchemaReferenceTest       : ECTestFixture {};
struct SchemaCreationTest        : ECTestFixture {};
struct ClassTest                 : ECTestFixture {};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void VerifyWidgetsSchema
(
ECSchemaPtr schema
)
    {
    EXPECT_STREQ (L"Widgets", schema->Name.c_str());
    EXPECT_STREQ (L"wid", schema->NamespacePrefix.c_str());
    EXPECT_STREQ (L"Widgets Display Label", schema->DisplayLabel.c_str());
    EXPECT_TRUE (schema->IsDisplayLabelDefined);
    EXPECT_STREQ (L"Widgets Description", schema->Description.c_str());
    EXPECT_EQ (9, schema->VersionMajor);
    EXPECT_EQ (6, schema->VersionMinor);        
    
    for each (ECClassP pClass in schema->Classes)
        {
        wprintf (L"Widgets contains class: '%s' with display label '%s'\n", pClass->Name.c_str(), pClass->DisplayLabel.c_str());
        }

    ECClassP pClass = schema->GetClassP(L"ClassDoesNotExistInSchema");
    EXPECT_FALSE (pClass);

    pClass = schema->GetClassP(L"ecProject");
    ASSERT_TRUE (pClass);
    EXPECT_STREQ (L"ecProject", pClass->Name.c_str());    
    EXPECT_STREQ (L"Project", pClass->DisplayLabel.c_str());
    EXPECT_TRUE (pClass->IsDisplayLabelDefined);
    EXPECT_STREQ (L"Project Class", pClass->Description.c_str());
    EXPECT_FALSE (pClass->IsStruct);
    EXPECT_FALSE (pClass->IsCustomAttributeClass);
    EXPECT_TRUE (pClass->IsDomainClass);
    EXPECT_FALSE (pClass->HasBaseClasses());
    ECPropertyP pProperty = pClass->GetPropertyP (L"Name");
    EXPECT_TRUE (pProperty);
    EXPECT_STREQ (L"Name", pProperty->Name.c_str());
    EXPECT_TRUE (pProperty->IsPrimitive);
    EXPECT_FALSE (pProperty->IsStruct);
    EXPECT_FALSE (pProperty->IsArray);
    EXPECT_STREQ (L"string", pProperty->TypeName.c_str());
    EXPECT_TRUE (PRIMITIVETYPE_String == pProperty->GetAsPrimitiveProperty()->Type);
    EXPECT_TRUE (pProperty->IsDisplayLabelDefined);
    EXPECT_STREQ (L"Project Name", pProperty->DisplayLabel.c_str());
    EXPECT_STREQ (L"", pProperty->Description.c_str());
    EXPECT_EQ (pClass, &pProperty->Class);
    EXPECT_FALSE (pProperty->IsReadOnly);

    pProperty = pClass->GetPropertyP (L"PropertyDoesNotExistInClass");
    EXPECT_FALSE (pProperty);

    pClass = schema->GetClassP(L"AccessCustomAttributes");
    ASSERT_TRUE (pClass);
    EXPECT_STREQ (L"AccessCustomAttributes", pClass->Name.c_str());    
    EXPECT_STREQ (L"AccessCustomAttributes", pClass->DisplayLabel.c_str());
    EXPECT_FALSE (pClass->IsDisplayLabelDefined);
    EXPECT_STREQ (L"", pClass->Description.c_str());
    EXPECT_FALSE (pClass->IsStruct);
    EXPECT_TRUE (pClass->IsCustomAttributeClass);
    EXPECT_FALSE (pClass->IsDomainClass);
    EXPECT_FALSE (pClass->HasBaseClasses());

    pClass = schema->GetClassP(L"Struct1");
    ASSERT_TRUE (pClass);
    EXPECT_STREQ (L"Struct1", pClass->Name.c_str());    
    EXPECT_STREQ (L"Struct1", pClass->DisplayLabel.c_str());
    EXPECT_FALSE (pClass->IsDisplayLabelDefined);
    EXPECT_STREQ (L"", pClass->Description.c_str());
    EXPECT_TRUE (pClass->IsStruct);
    EXPECT_FALSE (pClass->IsCustomAttributeClass);
    EXPECT_FALSE (pClass->IsDomainClass);
    EXPECT_FALSE (pClass->HasBaseClasses());

    pClass = schema->GetClassP(L"Struct2");
    ASSERT_TRUE (pClass);
    EXPECT_STREQ (L"Struct2", pClass->Name.c_str());    
    EXPECT_STREQ (L"Struct2", pClass->DisplayLabel.c_str());
    EXPECT_FALSE (pClass->IsDisplayLabelDefined);
    EXPECT_STREQ (L"", pClass->Description.c_str());
    EXPECT_TRUE (pClass->IsStruct);
    EXPECT_FALSE (pClass->IsCustomAttributeClass);
    EXPECT_TRUE (pClass->IsDomainClass);
    EXPECT_FALSE (pClass->HasBaseClasses());
    pProperty = pClass->GetPropertyP (L"NestedArray");
    EXPECT_TRUE (pProperty);
    EXPECT_STREQ (L"NestedArray", pProperty->Name.c_str());
    EXPECT_FALSE (pProperty->IsPrimitive);
    EXPECT_FALSE (pProperty->IsStruct);
    EXPECT_TRUE (pProperty->IsArray);
    EXPECT_STREQ (L"Struct1", pProperty->TypeName.c_str());
    ArrayECPropertyP arrayProperty = pProperty->GetAsArrayProperty();
    EXPECT_TRUE (ARRAYKIND_Struct == arrayProperty->Kind);
    EXPECT_EQ (schema->GetClassP(L"Struct1"), arrayProperty->StructElementType);
    EXPECT_EQ (0, arrayProperty->MinOccurs);
    EXPECT_EQ (UINT_MAX, arrayProperty->MaxOccurs);    
    EXPECT_FALSE (pProperty->IsDisplayLabelDefined);
    EXPECT_STREQ (L"NestedArray", pProperty->DisplayLabel.c_str());
    EXPECT_STREQ (L"", pProperty->Description.c_str());
    EXPECT_EQ (pClass, &pProperty->Class);
    EXPECT_FALSE (pProperty->IsReadOnly);

    pClass = schema->GetClassP(L"TestClass");
    ASSERT_TRUE (pClass);
    EXPECT_TRUE (pClass->HasBaseClasses());
    pProperty = pClass->GetPropertyP (L"EmbeddedStruct");
    EXPECT_TRUE (pProperty);
    EXPECT_STREQ (L"EmbeddedStruct", pProperty->Name.c_str());
    EXPECT_FALSE (pProperty->IsPrimitive);
    EXPECT_TRUE (pProperty->IsStruct);
    EXPECT_FALSE (pProperty->IsArray);
    EXPECT_STREQ (L"Struct1", pProperty->TypeName.c_str());
    StructECPropertyP structProperty = pProperty->GetAsStructProperty();    
    EXPECT_EQ (schema->GetClassP(L"Struct1"), &(structProperty->Type));
    EXPECT_FALSE (pProperty->IsDisplayLabelDefined);
    EXPECT_STREQ (L"EmbeddedStruct", pProperty->DisplayLabel.c_str());
    EXPECT_STREQ (L"", pProperty->Description.c_str());
    EXPECT_EQ (pClass, &pProperty->Class);
    EXPECT_FALSE (pProperty->IsReadOnly);
   
    for each (ECPropertyP pProperty in pClass->Properties)
        {
        wprintf (L"TestClass contains property: %s of type %s\n", pProperty->Name.c_str(), pProperty->TypeName.c_str());
        }
    
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectErrorWhenCOMNotInitialized)
    {
    // HACK The stopwatch class causes COM to get initialized and therefore if tests using it are executed before this test, we will fail because
    // COM is initialized.  I don't know anyway to force COM to uninitialize or to check if it is currently initialized so as a hack I'm just
    // uninitializing 20 times.  In the future if any test causes CoInitialize to get invoked 20 times or more then we will start breaking here.    
    for (int i=0;i<20;i++)
        CoUninitialize();
        
    ECSchemaPtr schema;        
    
    DISABLE_ASSERTS
    ECSchemaConstructionContextPtr schemaContext = ECSchemaConstructionContext::CreateContext();
    SchemaDeserializationStatus status = ECSchema::ReadXmlFromFile (schema, L"t", *schemaContext);
        
    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_FailedToInitializeMsmxl, status);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectErrorWhenXmlFileDoesNotExist)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    ECSchemaPtr schema;
    
    ECSchemaConstructionContextPtr schemaContext = ECSchemaConstructionContext::CreateContext();
    SchemaDeserializationStatus status = ECSchema::ReadXmlFromFile (schema, ECTestFixture::GetTestDataPath(L"ThisFileIsntReal.xml").c_str(), *schemaContext);

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_FailedToParseXml, status);
    
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectErrorWhenXmlFileIsMissingNodes)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    ECSchemaPtr schema;
    
    ECSchemaConstructionContextPtr schemaContext = ECSchemaConstructionContext::CreateContext();
    SchemaDeserializationStatus status = ECSchema::ReadXmlFromFile (schema, ECTestFixture::GetTestDataPath( L"MissingNodes.01.00.ecschema.xml").c_str(), *schemaContext);  

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_FailedToParseXml, status);
    
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectErrorWhenXmlFileIsIllFormed)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    ECSchemaPtr schema;
    
    ECSchemaConstructionContextPtr schemaContext = ECSchemaConstructionContext::CreateContext();
    SchemaDeserializationStatus status = ECSchema::ReadXmlFromFile (schema, ECTestFixture::GetTestDataPath( L"IllFormedXml.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_FailedToParseXml, status);
    
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectErrorWhenXmlFileIsMissingECSchemaNode)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    ECSchemaPtr schema;
    
    ECSchemaConstructionContextPtr schemaContext = ECSchemaConstructionContext::CreateContext();
    SchemaDeserializationStatus status = ECSchema::ReadXmlFromFile (schema, ECTestFixture::GetTestDataPath( L"MissingECSchemaNode.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml, status);
    
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectErrorWhenXmlFileIsMissingNamespace)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    ECSchemaPtr schema;
    
    ECSchemaConstructionContextPtr schemaContext = ECSchemaConstructionContext::CreateContext();
    SchemaDeserializationStatus status = ECSchema::ReadXmlFromFile (schema, ECTestFixture::GetTestDataPath( L"MissingNamespace.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml, status);
    
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectErrorWhenXmlFileHasUnsupportedNamespace)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    ECSchemaPtr schema;
    
    ECSchemaConstructionContextPtr schemaContext = ECSchemaConstructionContext::CreateContext();
    SchemaDeserializationStatus status = ECSchema::ReadXmlFromFile (schema, ECTestFixture::GetTestDataPath( L"UnsupportedECXmlNamespace.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml, status);
    
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectErrorWhenXmlFileHasMissingSchemaNameAttribute)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    ECSchemaPtr schema;
    
    ECSchemaConstructionContextPtr schemaContext = ECSchemaConstructionContext::CreateContext();
    SchemaDeserializationStatus status = ECSchema::ReadXmlFromFile (schema, ECTestFixture::GetTestDataPath( L"MissingSchemaName.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml, status);
    
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectErrorWhenXmlFileHasMissingClassNameAttribute)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    ECSchemaPtr schema;
    
    ECSchemaConstructionContextPtr schemaContext = ECSchemaConstructionContext::CreateContext();
    SchemaDeserializationStatus status = ECSchema::ReadXmlFromFile (schema, ECTestFixture::GetTestDataPath( L"MissingClassName.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml, status);
    
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenXmlFileHasInvalidVersionString)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    ECSchemaPtr schema;
    
    ECSchemaConstructionContextPtr schemaContext = ECSchemaConstructionContext::CreateContext();
    SchemaDeserializationStatus status = ECSchema::ReadXmlFromFile (schema, ECTestFixture::GetTestDataPath( L"InvalidVersionString.01.00.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ (1, schema->VersionMajor);
    EXPECT_EQ (0, schema->VersionMinor);

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_Success, status);
    
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenDeserializingSchemaWithBaseClassInReferencedFile)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    ECSchemaPtr schema;

    ECSchemaConstructionContextPtr schemaContext = ECSchemaConstructionContext::CreateContext();
    SchemaDeserializationStatus status = ECSchema::ReadXmlFromFile (schema, ECTestFixture::GetTestDataPath( L"SchemaThatReferences.01.00.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_Success, status);    

    ECClassP pClass = schema->GetClassP(L"circle");    
    ASSERT_TRUE (pClass);
    }    
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenECSchemaContainsOnlyRequiredAttributes)
    {                
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    ECSchemaPtr schema;

    ECSchemaConstructionContextPtr schemaContext = ECSchemaConstructionContext::CreateContext();
    SchemaDeserializationStatus status = ECSchema::ReadXmlFromFile (schema, ECTestFixture::GetTestDataPath( L"OnlyRequiredECSchemaAttributes.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_Success, status);    
    EXPECT_STREQ (L"OnlyRequiredECSchemaAttributes", schema->Name.c_str());
    EXPECT_STREQ (L"", schema->NamespacePrefix.c_str());
    EXPECT_STREQ (L"OnlyRequiredECSchemaAttributes", schema->DisplayLabel.c_str());
    EXPECT_FALSE (schema->IsDisplayLabelDefined);
    EXPECT_STREQ (L"", schema->Description.c_str());
    EXPECT_EQ (1, schema->VersionMajor);
    EXPECT_EQ (0, schema->VersionMinor);
    
    ECClassP pClass = schema->GetClassP(L"OnlyRequiredECClassAttributes");    
    ASSERT_TRUE (pClass);
    EXPECT_STREQ (L"OnlyRequiredECClassAttributes", pClass->Name.c_str());    
    EXPECT_STREQ (L"OnlyRequiredECClassAttributes", pClass->DisplayLabel.c_str());
    EXPECT_FALSE (pClass->IsDisplayLabelDefined);
    EXPECT_STREQ (L"", pClass->Description.c_str());
    EXPECT_FALSE (pClass->IsStruct);
    EXPECT_FALSE (pClass->IsCustomAttributeClass);
    EXPECT_TRUE (pClass->IsDomainClass);

    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenDeserializingWidgetsECSchema)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    ECSchemaPtr schema;
    
    ECSchemaConstructionContextPtr schemaContext = ECSchemaConstructionContext::CreateContext();
    SchemaDeserializationStatus status = ECSchema::ReadXmlFromFile (schema, ECTestFixture::GetTestDataPath( L"Widgets.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_Success, status);  
    VerifyWidgetsSchema(schema);  
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenDeserializingECSchemaFromString)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    ECSchemaPtr schema;
    
    ECSchemaConstructionContextPtr schemaContext = ECSchemaConstructionContext::CreateContext();
    SchemaDeserializationStatus status = ECSchema::ReadXmlFromString (schema, 
        L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        L"<ECSchema schemaName=\"Widgets\" version=\"09.06\" displayLabel=\"Widgets Display Label\" description=\"Widgets Description\" nameSpacePrefix=\"wid\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" xmlns:ec=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" xmlns:ods=\"Bentley_ODS.01.02\">"
        L"<ECSchemaReference name=\"EditorCustomAttributes\" version=\"01.00\" prefix=\"beca\" />"
        L"    <ECClass typeName=\"ecProject\" description=\"Project ECClass\" displayLabel=\"Project\" isDomainClass=\"True\">"
        L"       <ECProperty propertyName=\"Name\" typeName=\"string\" displayLabel=\"Project Name\" />"
        L"    </ECClass>"
        L"</ECSchema>", *schemaContext);

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_Success, status);    
    EXPECT_STREQ (L"Widgets", schema->Name.c_str());
    EXPECT_STREQ (L"wid", schema->NamespacePrefix.c_str());
    EXPECT_STREQ (L"Widgets Display Label", schema->DisplayLabel.c_str());
    EXPECT_TRUE (schema->IsDisplayLabelDefined);
    EXPECT_STREQ (L"Widgets Description", schema->Description.c_str());
    EXPECT_EQ (9, schema->VersionMajor);
    EXPECT_EQ (6, schema->VersionMinor);        
    
    for each (ECClassP pClass in schema->Classes)
        {
        wprintf (L"Widgets contains class: '%s' with display label '%s'\n", pClass->Name.c_str(), pClass->DisplayLabel.c_str());
        }

    ECClassP pClass = schema->GetClassP(L"ClassDoesNotExistInSchema");
    EXPECT_FALSE (pClass);

    pClass = schema->GetClassP(L"ecProject");
    ASSERT_TRUE (pClass);
    EXPECT_STREQ (L"ecProject", pClass->Name.c_str());    
    EXPECT_STREQ (L"Project", pClass->DisplayLabel.c_str());
    EXPECT_TRUE (pClass->IsDisplayLabelDefined);
    EXPECT_STREQ (L"Project ECClass", pClass->Description.c_str());
    EXPECT_FALSE (pClass->IsStruct);
    EXPECT_FALSE (pClass->IsCustomAttributeClass);
    EXPECT_TRUE (pClass->IsDomainClass);
    EXPECT_FALSE (pClass->HasBaseClasses());
    ECPropertyP pProperty = pClass->GetPropertyP (L"Name");
    EXPECT_TRUE (pProperty);
    EXPECT_STREQ (L"Name", pProperty->Name.c_str());
    EXPECT_TRUE (pProperty->IsPrimitive);
    EXPECT_FALSE (pProperty->IsStruct);
    EXPECT_FALSE (pProperty->IsArray);
    EXPECT_STREQ (L"string", pProperty->TypeName.c_str());
    EXPECT_TRUE (PRIMITIVETYPE_String == pProperty->GetAsPrimitiveProperty()->Type);
    EXPECT_TRUE (pProperty->IsDisplayLabelDefined);
    EXPECT_STREQ (L"Project Name", pProperty->DisplayLabel.c_str());
    EXPECT_STREQ (L"", pProperty->Description.c_str());
    EXPECT_EQ (pClass, &pProperty->Class);
    EXPECT_FALSE (pProperty->IsReadOnly);

    pProperty = pClass->GetPropertyP (L"PropertyDoesNotExistInClass");
    EXPECT_FALSE (pProperty);
    
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenRoundtripUsingString)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    ECSchemaPtr schema;
    
    ECSchemaConstructionContextPtr schemaContext = ECSchemaConstructionContext::CreateContext();
    SchemaDeserializationStatus status = ECSchema::ReadXmlFromFile (schema, ECTestFixture::GetTestDataPath( L"Widgets.01.00.ecschema.xml").c_str(), *schemaContext);
    wprintf(L"Verifying original schema from file.\n"); 
    VerifyWidgetsSchema(schema);

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_Success, status);

    bwstring ecSchemaXmlString;
    
    SchemaSerializationStatus status2 = schema->WriteXmlToString(ecSchemaXmlString);
    EXPECT_EQ(SCHEMA_SERIALIZATION_STATUS_Success, status2);
    
    ECSchemaPtr deserializedSchema;
    schemaContext = ECSchemaConstructionContext::CreateContext();
    status = ECSchema::ReadXmlFromString(deserializedSchema, ecSchemaXmlString.c_str(), *schemaContext);
    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_Success, status); 
    wprintf(L"Verifying schema deserialized from string.\n");
    VerifyWidgetsSchema(deserializedSchema);

    CoUninitialize();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
//TEST_F(SchemaDeserializationTest, ExpectSuccessWhenSerializingToFile)
//    {
//    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
//    ECSchemaPtr schema;
//    
//    ECSchemaConstructionContextPtr schemaContext = ECSchemaConstructionContext::CreateContext();
//    SchemaDeserializationStatus status = ECSchema::ReadXmlFromFile (schema, SCHEMAS_PATH L"Widgets.01.00.ecschema.xml", *schemaContext);
//    wprintf(L"Verifying original schema from file.\n"); 
//    VerifyWidgetsSchema(schema);
//
//    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_Success, status);
//
//    SchemaSerializationStatus status2 = schema->WriteXmlToFile(L"d:\\temp\\test.xml");
//    CoUninitialize();
//    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenRoundtripUsingStream)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    ECSchemaPtr schema;
    
    ECSchemaConstructionContextPtr schemaContext = ECSchemaConstructionContext::CreateContext();
    SchemaDeserializationStatus status = ECSchema::ReadXmlFromFile (schema, ECTestFixture::GetTestDataPath( L"Widgets.01.00.ecschema.xml").c_str(), *schemaContext);
    wprintf(L"Verifying original schema from file.\n"); 
    VerifyWidgetsSchema(schema);

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_Success, status);    
    LPSTREAM stream = NULL;
    HRESULT res = ::CreateStreamOnHGlobal(NULL,TRUE,&stream);

    SchemaSerializationStatus status2 = schema->WriteXmlToStream(stream);
    EXPECT_EQ(SCHEMA_SERIALIZATION_STATUS_Success, status2);
    
    LARGE_INTEGER liPos = {0};
    stream->Seek(liPos, STREAM_SEEK_SET, NULL);

    ECSchemaPtr deserializedSchema;
    schemaContext = ECSchemaConstructionContext::CreateContext();
    status = ECSchema::ReadXmlFromStream(deserializedSchema, stream, *schemaContext);
    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_Success, status); 
    wprintf(L"Verifying schema deserialized from stream.\n");
    VerifyWidgetsSchema(deserializedSchema);

    CoUninitialize();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaSerializationTest, ExpectErrorWhenCOMNotInitialized)
    {    
    // HACK The stopwatch class causes COM to get initialized and therefore if tests using it are executed before this test, we will fail because
    // COM is initialized.  I don't know anyway to force COM to uninitialize or to check if it is currently initialized so as a hack I'm just
    // uninitializing 20 times.  In the future if any test causes CoInitialize to get invoked 20 times or more then we will start breaking here.
    for (int i=0;i<20;i++)
        CoUninitialize();    
        
    ECSchemaPtr schema;        
    ECSchema::CreateSchema(schema, L"Widget");
    
    DISABLE_ASSERTS
    bwstring ecSchemaXmlString;
    
    SchemaSerializationStatus status = schema->WriteXmlToString(ecSchemaXmlString);
        
    EXPECT_EQ (SCHEMA_SERIALIZATION_STATUS_FailedToInitializeMsmxl, status);
    };
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaSerializationTest, ExpectSuccessWithSerializingBaseClasses)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));

    ECSchemaPtr schema;
    ECSchemaPtr schema2;
    ECSchemaPtr schema3;
    
    ECSchema::CreateSchema(schema, L"Widget");
    ECSchema::CreateSchema(schema2, L"BaseSchema");
    ECSchema::CreateSchema(schema3, L"BaseSchema2");
    
    schema->NamespacePrefix = L"ecw";
    schema2->NamespacePrefix = L"base";
    schema3->NamespacePrefix = L"base";
    
    ECClassP class1;
    ECClassP baseClass;
    ECClassP anotherBase;
    ECClassP gadget;
    ECClassP bolt;
    schema->CreateClass(class1, L"TestClass");
    schema->CreateClass(gadget, L"Gadget");
    schema->CreateClass(bolt, L"Bolt");
    schema2->CreateClass(baseClass, L"BaseClass");
    schema3->CreateClass(anotherBase, L"AnotherBase");
    
    EXPECT_EQ(ECOBJECTS_STATUS_SchemaNotFound, class1->AddBaseClass(*baseClass));
    schema->AddReferencedSchema(schema2);
    schema->AddReferencedSchema(schema3);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, class1->AddBaseClass(*baseClass));
    EXPECT_EQ(ECOBJECTS_STATUS_Success, class1->AddBaseClass(*anotherBase));
    EXPECT_EQ(ECOBJECTS_STATUS_Success, gadget->AddBaseClass(*class1));
    
//    const wchar_t *ecSchemaXmlString;
    
    //SchemaSerializationStatus status2 = schema->WriteXmlToFile(L"d:\\temp\\base.xml");
    //
    //ECSchemaPtr schema4;
    //ECSchemaConstructionContextPtr schemaContext = ECSchemaConstructionContext::CreateContext();
    //SchemaDeserializationStatus status3 = ECSchema::ReadXmlFromFile (schema4, L"d:\\temp\\base.xml", *schemaContext);
    
    bwstring ecSchemaXmlString;
    
    SchemaSerializationStatus status2 = schema->WriteXmlToString(ecSchemaXmlString);
    EXPECT_EQ(SCHEMA_SERIALIZATION_STATUS_Success, status2);
    
    CoUninitialize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaReferenceTest, AddAndRemoveReferencedSchemas)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, L"TestSchema");
    
    ECSchemaPtr refSchema;
    ECSchema::CreateSchema(refSchema, L"RefSchema");
    
    EXPECT_EQ(ECOBJECTS_STATUS_Success, schema->AddReferencedSchema(refSchema));
    EXPECT_EQ(ECOBJECTS_STATUS_NamedItemAlreadyExists, schema->AddReferencedSchema(refSchema));
    
    ECSchemaReferenceList refList = schema->GetReferencedSchemas();
    ECSchemaReferenceList::const_iterator schemaIterator;
    for (schemaIterator = refList.begin(); schemaIterator != refList.end(); schemaIterator++)
        {
        if (*schemaIterator == refSchema)
            break;
        }
        
    EXPECT_FALSE(schemaIterator == refList.end());
    EXPECT_EQ(ECOBJECTS_STATUS_Success, schema->RemoveReferencedSchema(refSchema));
    
    refList = schema->GetReferencedSchemas();
    for (schemaIterator = refList.begin(); schemaIterator != refList.end(); schemaIterator++)
        {
        if (*schemaIterator == refSchema)
            break;
        }
        
    EXPECT_TRUE(schemaIterator == refList.end());
    EXPECT_EQ(ECOBJECTS_STATUS_SchemaNotFound, schema->RemoveReferencedSchema(refSchema));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaReferenceTest, ExpectErrorWhenTryRemoveSchemaInUse)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, L"TestSchema");
    
    ECSchemaPtr refSchema;
    ECSchema::CreateSchema(refSchema, L"RefSchema");
    
    EXPECT_EQ(ECOBJECTS_STATUS_Success, schema->AddReferencedSchema(refSchema));
    ECClassP class1;
    ECClassP baseClass;
    ECClassP structClass;
            
    refSchema->CreateClass(baseClass, L"BaseClass");
    refSchema->CreateClass(structClass, L"StructClass");
    schema->CreateClass(class1, L"TestClass");
    structClass->IsStruct = true;
    
    class1->AddBaseClass(*baseClass);
    EXPECT_EQ (ECOBJECTS_STATUS_SchemaInUse, schema->RemoveReferencedSchema(refSchema));
    
    class1->RemoveBaseClass(*baseClass);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, schema->RemoveReferencedSchema(refSchema));

    EXPECT_EQ(ECOBJECTS_STATUS_Success, schema->AddReferencedSchema(refSchema));
    StructECPropertyP structProp;
    ArrayECPropertyP nestedArrayProp;
    
    class1->CreateStructProperty(structProp, L"StructMember");
    class1->CreateArrayProperty(nestedArrayProp, L"NestedArray");
    
    structProp->Type = *structClass;
    nestedArrayProp->StructElementType = structClass;

    EXPECT_EQ (ECOBJECTS_STATUS_SchemaInUse, schema->RemoveReferencedSchema(refSchema));
    class1->RemoveProperty(L"StructMember");
    EXPECT_EQ (ECOBJECTS_STATUS_SchemaInUse, schema->RemoveReferencedSchema(refSchema));
    class1->RemoveProperty(L"NestedArray");
    EXPECT_EQ(ECOBJECTS_STATUS_Success, schema->RemoveReferencedSchema(refSchema));
    
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaReferenceTest, ExpectSuccessWithCircularReferences)
    {
#if defined (WIP_FUSION)
// Temporarily disabled this test.  It fails because it leaks schemas
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    ECSchemaPtr schema;
    
    ECSchemaConstructionContextPtr schemaContext = ECSchemaConstructionContext::CreateContext();
    SchemaDeserializationStatus status = ECSchema::ReadXmlFromFile (schema, ECTestFixture::GetTestDataPath( L"CircleSchema.01.00.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_Success, status);

    CoUninitialize();
#endif
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaCreationTest, CanFullyCreateASchema)
    {
    ECSchemaPtr testSchema;
    ECSchema::CreateSchema(testSchema, L"TestSchema");
    testSchema->NamespacePrefix = L"ts";
    testSchema->Description = L"Schema for testing programmatic construction";
    testSchema->DisplayLabel = L"Test Schema";
    testSchema->VersionMajor = 1;
    testSchema->VersionMinor = 2;
    
    EXPECT_TRUE(testSchema->IsDisplayLabelDefined);
    EXPECT_EQ(1, testSchema->VersionMajor);
    EXPECT_EQ(2, testSchema->VersionMinor);
    EXPECT_EQ(0, wcscmp(testSchema->Name.c_str(), L"TestSchema"));
    EXPECT_EQ(0, wcscmp(testSchema->NamespacePrefix.c_str(), L"ts"));
    EXPECT_EQ(0, wcscmp(testSchema->Description.c_str(), L"Schema for testing programmatic construction"));
    EXPECT_EQ(0, wcscmp(testSchema->DisplayLabel.c_str(), L"Test Schema"));
    
    ECSchemaPtr schema2;
    ECSchema::CreateSchema(schema2, L"BaseSchema");
    
    testSchema->AddReferencedSchema(schema2);
    
    ECClassP class1;
    ECClassP baseClass;
    ECClassP structClass;
    ECClassP relatedClass;
    ECRelationshipClassP relationshipClass;

    testSchema->CreateClass(class1, L"TestClass");
    testSchema->CreateClass(structClass, L"StructClass");
    schema2->CreateClass(baseClass, L"BaseClass");
    testSchema->CreateClass(relatedClass, L"RelatedClass");
    
    class1->Description = L"Class for testing purposes";
    class1->DisplayLabel = L"Test Class";
    
    EXPECT_EQ(0, wcscmp(class1->Description.c_str(), L"Class for testing purposes"));
    EXPECT_EQ(0, wcscmp(class1->DisplayLabel.c_str(), L"Test Class"));
    EXPECT_FALSE(class1->IsStruct);
    EXPECT_FALSE(class1->IsCustomAttributeClass);
    EXPECT_TRUE(class1->IsDomainClass);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, class1->AddBaseClass(*baseClass));
    EXPECT_TRUE(class1->HasBaseClasses());
    
    structClass->IsStruct = true;
    EXPECT_TRUE(structClass->IsStruct);
    EXPECT_TRUE(structClass->IsDomainClass);
    
    PrimitiveECPropertyP stringProp;
    StructECPropertyP structProp;
    ArrayECPropertyP nestedArrayProp;
    ArrayECPropertyP primitiveArrayProp;
    
    class1->CreatePrimitiveProperty(stringProp, L"StringMember");
    class1->CreateStructProperty(structProp, L"StructMember");
    class1->CreateArrayProperty(nestedArrayProp, L"NestedArray");
    class1->CreateArrayProperty(primitiveArrayProp, L"PrimitiveArray");
    
    structProp->Type = *structClass;
    nestedArrayProp->StructElementType = structClass;
    primitiveArrayProp->PrimitiveElementType = PRIMITIVETYPE_Long;
    primitiveArrayProp->MinOccurs = 1;
    primitiveArrayProp->MaxOccurs = 10;
    
    EXPECT_TRUE(ARRAYKIND_Struct == nestedArrayProp->Kind);
    EXPECT_TRUE(ARRAYKIND_Primitive == primitiveArrayProp->Kind);
    EXPECT_EQ(0, nestedArrayProp->MinOccurs);
    EXPECT_EQ(UINT_MAX, nestedArrayProp->MaxOccurs);
    EXPECT_EQ(1, primitiveArrayProp->MinOccurs);
    EXPECT_EQ(10, primitiveArrayProp->MaxOccurs);
    
    EXPECT_TRUE(stringProp->IsPrimitive);
    EXPECT_FALSE(stringProp->IsStruct);
    EXPECT_FALSE(stringProp->IsArray);
    
    EXPECT_FALSE(structProp->IsPrimitive);
    EXPECT_TRUE(structProp->IsStruct);
    EXPECT_FALSE(structProp->IsArray);
    
    EXPECT_FALSE(primitiveArrayProp->IsPrimitive);
    EXPECT_FALSE(primitiveArrayProp->IsStruct);
    EXPECT_TRUE(primitiveArrayProp->IsArray);
    
    EXPECT_FALSE(stringProp->IsReadOnly);
    
    EXPECT_EQ(0, wcscmp(stringProp->TypeName.c_str(), L"string"));
    EXPECT_TRUE(PRIMITIVETYPE_String == stringProp->Type);
    EXPECT_EQ(0, wcscmp(structProp->Type.Name.c_str(), L"StructClass"));
    
    PrimitiveECPropertyP binaryProperty;
    PrimitiveECPropertyP booleanProperty;
    PrimitiveECPropertyP dateTimeProperty;
    PrimitiveECPropertyP doubleProperty;
    PrimitiveECPropertyP integerProperty;
    PrimitiveECPropertyP longProperty;
    PrimitiveECPropertyP point2DProperty;
    PrimitiveECPropertyP point3DProperty;
    
    class1->CreatePrimitiveProperty(binaryProperty, L"BinaryProp");
    class1->CreatePrimitiveProperty(booleanProperty, L"BooleanProp");
    class1->CreatePrimitiveProperty(dateTimeProperty, L"DateTimeProp");
    class1->CreatePrimitiveProperty(doubleProperty, L"DoubleProp");
    class1->CreatePrimitiveProperty(integerProperty, L"IntProp");
    class1->CreatePrimitiveProperty(longProperty, L"LongProp");
    class1->CreatePrimitiveProperty(point2DProperty, L"Point2DProp");
    class1->CreatePrimitiveProperty(point3DProperty, L"Point3DProp");
    
    EXPECT_EQ(ECOBJECTS_STATUS_ParseError, binaryProperty->TypeName = L"fake");
    
    binaryProperty->TypeName = L"binary";
    booleanProperty->TypeName = L"boolean";
    dateTimeProperty->TypeName = L"dateTime";
    doubleProperty->TypeName = L"double";
    integerProperty->TypeName = L"int";
    longProperty->TypeName = L"long";
    point2DProperty->TypeName = L"point2d";
    point3DProperty->TypeName = L"point3d";
    
    EXPECT_TRUE(PRIMITIVETYPE_Binary == binaryProperty->Type);
    EXPECT_TRUE(PRIMITIVETYPE_Boolean == booleanProperty->Type);
    EXPECT_TRUE(PRIMITIVETYPE_DateTime == dateTimeProperty->Type);
    EXPECT_TRUE(PRIMITIVETYPE_Double == doubleProperty->Type);
    EXPECT_TRUE(PRIMITIVETYPE_Integer == integerProperty->Type);
    EXPECT_TRUE(PRIMITIVETYPE_Long == longProperty->Type);
    EXPECT_TRUE(PRIMITIVETYPE_Point2D == point2DProperty->Type);
    EXPECT_TRUE(PRIMITIVETYPE_Point3D == point3DProperty->Type);

    class1->CreatePrimitiveProperty(binaryProperty, L"BinaryProp2", PRIMITIVETYPE_Binary);
    class1->CreatePrimitiveProperty(booleanProperty, L"BooleanProp2", PRIMITIVETYPE_Boolean);
    class1->CreatePrimitiveProperty(dateTimeProperty, L"DateTimeProp2", PRIMITIVETYPE_DateTime);
    class1->CreatePrimitiveProperty(doubleProperty, L"DoubleProp2", PRIMITIVETYPE_Double);
    class1->CreatePrimitiveProperty(integerProperty, L"IntProp2", PRIMITIVETYPE_Integer);
    class1->CreatePrimitiveProperty(longProperty, L"LongProp2", PRIMITIVETYPE_Long);
    class1->CreatePrimitiveProperty(point2DProperty, L"Point2DProp2", PRIMITIVETYPE_Point2D);
    class1->CreatePrimitiveProperty(point3DProperty, L"Point3DProp2", PRIMITIVETYPE_Point3D);

    EXPECT_TRUE(PRIMITIVETYPE_Binary == binaryProperty->Type);
    EXPECT_TRUE(PRIMITIVETYPE_Boolean == booleanProperty->Type);
    EXPECT_TRUE(PRIMITIVETYPE_DateTime == dateTimeProperty->Type);
    EXPECT_TRUE(PRIMITIVETYPE_Double == doubleProperty->Type);
    EXPECT_TRUE(PRIMITIVETYPE_Integer == integerProperty->Type);
    EXPECT_TRUE(PRIMITIVETYPE_Long == longProperty->Type);
    EXPECT_TRUE(PRIMITIVETYPE_Point2D == point2DProperty->Type);
    EXPECT_TRUE(PRIMITIVETYPE_Point3D == point3DProperty->Type);

    class1->CreateStructProperty(structProp, L"StructMember2", *structClass);
    class1->CreateArrayProperty(nestedArrayProp, L"NestedArray2", structClass);
    class1->CreateArrayProperty(primitiveArrayProp, L"PrimitiveArray2", PRIMITIVETYPE_Integer);
    EXPECT_TRUE(ARRAYKIND_Struct == nestedArrayProp->Kind);
    EXPECT_TRUE(ARRAYKIND_Primitive == primitiveArrayProp->Kind);
    EXPECT_EQ(0, wcscmp(structProp->Type.Name.c_str(), L"StructClass"));
    EXPECT_EQ(0, wcscmp(nestedArrayProp->TypeName.c_str(), L"StructClass"));
    EXPECT_EQ(0, wcscmp(primitiveArrayProp->TypeName.c_str(), L"int"));

    testSchema->CreateRelationshipClass(relationshipClass, L"RelationshipClass");
    EXPECT_TRUE(STRENGTHTYPE_Referencing == relationshipClass->Strength);
    relationshipClass->Strength = STRENGTHTYPE_Embedding;
    EXPECT_TRUE(STRENGTHTYPE_Embedding == relationshipClass->Strength);
    
    EXPECT_TRUE(STRENGTHDIRECTION_Forward == relationshipClass->StrengthDirection);
    relationshipClass->StrengthDirection = STRENGTHDIRECTION_Backward;
    EXPECT_TRUE(STRENGTHDIRECTION_Backward == relationshipClass->StrengthDirection);
    
    EXPECT_FALSE(relationshipClass->Source.IsMultiple);
    EXPECT_TRUE(relationshipClass->Target.IsMultiple);
    
    EXPECT_TRUE(relationshipClass->Target.IsPolymorphic);
    EXPECT_TRUE(relationshipClass->Source.IsPolymorphic);
    relationshipClass->Source.IsPolymorphic = false;
    EXPECT_FALSE(relationshipClass->Source.IsPolymorphic);
    
    relationshipClass->Description = L"Relates the test class to the related class";
    relationshipClass->DisplayLabel = L"TestRelationshipClass";
    
    EXPECT_EQ(0, relationshipClass->Source.Classes.size());
    EXPECT_EQ(0, relationshipClass->Target.Classes.size());
    
    relationshipClass->Source.AddClass(*structClass);
    EXPECT_EQ(1, relationshipClass->Source.Classes.size());
    relationshipClass->Source.AddClass(*class1);
    EXPECT_EQ(1, relationshipClass->Source.Classes.size());
    
    relationshipClass->Target.AddClass(*relatedClass);
    EXPECT_EQ(1, relationshipClass->Target.Classes.size());
    relationshipClass->Target.AddClass(*relatedClass);
    EXPECT_EQ(1, relationshipClass->Target.Classes.size());
    relationshipClass->Target.AddClass(*structClass);
    EXPECT_EQ(2, relationshipClass->Target.Classes.size());
    
    EXPECT_EQ(0, relationshipClass->Source.Cardinality.LowerLimit);
    EXPECT_EQ(0, relationshipClass->Target.Cardinality.LowerLimit);
    EXPECT_EQ(1, relationshipClass->Source.Cardinality.UpperLimit);
    EXPECT_EQ(1, relationshipClass->Target.Cardinality.UpperLimit);
    
    relationshipClass->Source.Cardinality = RelationshipCardinality::OneMany();
    EXPECT_EQ(1, relationshipClass->Source.Cardinality.LowerLimit);
    EXPECT_TRUE(relationshipClass->Source.Cardinality.IsUpperLimitUnbounded());
    
    RelationshipCardinality *card = new RelationshipCardinality(2, 5);
    relationshipClass->Target.Cardinality = *card;
    EXPECT_EQ(2, relationshipClass->Target.Cardinality.LowerLimit);
    EXPECT_EQ(5, relationshipClass->Target.Cardinality.UpperLimit);
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
    
    ECSchema::CreateSchema(schema, L"TestSchema");
    schema->CreateClass(class1, L"TestClass");
    schema->CreateClass(baseClass1, L"BaseClass1");
    schema->CreateClass(baseClass2, L"BaseClass2");
    
    EXPECT_EQ(ECOBJECTS_STATUS_Success, class1->AddBaseClass(*baseClass1));
    EXPECT_EQ(ECOBJECTS_STATUS_Success, baseClass1->AddBaseClass(*baseClass2));
    EXPECT_EQ(ECOBJECTS_STATUS_BaseClassUnacceptable, baseClass2->AddBaseClass(*class1));
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, AddAndRemoveBaseClass)
    {
    ECSchemaPtr schema;
    ECClassP class1;
    ECClassP baseClass1;
    
    ECSchema::CreateSchema(schema, L"TestSchema");
    schema->CreateClass(class1, L"TestClass");
    schema->CreateClass(baseClass1, L"BaseClass");
    EXPECT_EQ(ECOBJECTS_STATUS_Success, class1->AddBaseClass(*baseClass1));

    ECBaseClassesList classList = class1->GetBaseClasses();
    ECBaseClassesList::const_iterator baseClassIterator;
    for (baseClassIterator = classList.begin(); baseClassIterator != classList.end(); baseClassIterator++)
        {
        if (*baseClassIterator == baseClass1)
            break;
        }
        
    EXPECT_FALSE(baseClassIterator == classList.end());
    
    EXPECT_EQ(ECOBJECTS_STATUS_Success, class1->RemoveBaseClass(*baseClass1));
    classList = class1->GetBaseClasses();
    for (baseClassIterator = classList.begin(); baseClassIterator != classList.end(); baseClassIterator++)
        {
        if (*baseClassIterator == baseClass1)
            break;
        }
        
    EXPECT_TRUE(baseClassIterator == classList.end());
    EXPECT_EQ(ECOBJECTS_STATUS_ClassNotFound, class1->RemoveBaseClass(*baseClass1));
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
    
    ECSchema::CreateSchema(schema, L"TestSchema");
    schema->CreateClass(class1, L"TestClass");
    schema->CreateClass(baseClass1, L"BaseClass1");
    schema->CreateClass(baseClass2, L"BaseClass2");
    
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
    
    ECSchema::CreateSchema(schema, L"TestSchema");
    schema->CreateClass(class1, L"TestClass");
    schema->CreateClass(baseClass1, L"BaseClass1");
    schema->CreateClass(structClass, L"ClassForStructs");
    structClass->IsStruct = true;
    schema->CreateClass(structClass2, L"ClassForStructs2");
    structClass2->IsStruct = true;
    class1->AddBaseClass(*baseClass1);
    
    PrimitiveECPropertyP baseStringProp;
    PrimitiveECPropertyP baseIntProp;
    PrimitiveECPropertyP baseDoubleProp;
    StructECPropertyP baseStructProp;
    ArrayECPropertyP baseStringArrayProperty;
    ArrayECPropertyP baseStructProperty;
    
    baseClass1->CreatePrimitiveProperty(baseStringProp, L"StringProperty", PRIMITIVETYPE_String);
    baseClass1->CreatePrimitiveProperty(baseIntProp, L"IntegerProperty", PRIMITIVETYPE_Integer);
    baseClass1->CreatePrimitiveProperty(baseDoubleProp, L"DoubleProperty", PRIMITIVETYPE_Double);
    baseClass1->CreateStructProperty(baseStructProp, L"StructProperty", *structClass);
    baseClass1->CreateArrayProperty(baseStringArrayProperty, L"StringArrayProperty", PRIMITIVETYPE_String);
    baseClass1->CreateArrayProperty(baseStructProperty, L"StructArrayProperty", structClass);
    
    PrimitiveECPropertyP longProperty;
    PrimitiveECPropertyP stringProperty;
    
    // Primitives overriding primitives
    EXPECT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, class1->CreatePrimitiveProperty(longProperty, L"StringProperty", PRIMITIVETYPE_Long));
    EXPECT_EQ(NULL, longProperty);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, class1->CreatePrimitiveProperty(stringProperty, L"StringProperty", PRIMITIVETYPE_String));
    EXPECT_EQ(baseStringProp, stringProperty->BaseProperty);
    class1->RemoveProperty(L"StringProperty");
    
    // Primitives overriding structs
    EXPECT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, class1->CreatePrimitiveProperty(longProperty, L"StructProperty", PRIMITIVETYPE_Long));

    // Primitives overriding arrays
    EXPECT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, class1->CreatePrimitiveProperty(longProperty, L"StringArrayProperty", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECOBJECTS_STATUS_Success, class1->CreatePrimitiveProperty(stringProperty, L"StringArrayProperty", PRIMITIVETYPE_String));
    class1->RemoveProperty(L"StringArrayProperty");

    StructECPropertyP structProperty;
    // Structs overriding primitives
    EXPECT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, class1->CreateStructProperty(structProperty, L"IntegerProperty"));

    // Structs overriding structs
    // If we don't specify a struct type for the new property, then it should succeed
    EXPECT_EQ(ECOBJECTS_STATUS_Success, class1->CreateStructProperty(structProperty, L"StructProperty"));
    class1->RemoveProperty(L"StructProperty");
    EXPECT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, class1->CreateStructProperty(structProperty, L"StructProperty", *structClass2));

    // Structs overriding arrays
    EXPECT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, class1->CreateStructProperty(structProperty, L"StringArrayProperty"));
    EXPECT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, class1->CreateStructProperty(structProperty, L"StringArrayProperty", *structClass));
    EXPECT_EQ(ECOBJECTS_STATUS_Success, class1->CreateStructProperty(structProperty, L"StructArrayProperty"));
    class1->RemoveProperty(L"StructArrayProperty");
    EXPECT_EQ(ECOBJECTS_STATUS_Success, class1->CreateStructProperty(structProperty, L"StructArrayProperty", *structClass));
    class1->RemoveProperty(L"StructArrayProperty");
    EXPECT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, class1->CreateStructProperty(structProperty, L"StructArrayProperty", *structClass2));

    ArrayECPropertyP stringArrayProperty;
    ArrayECPropertyP stringArrayProperty2;
    ArrayECPropertyP structArrayProperty;
    ArrayECPropertyP structArrayProperty2;
    // Arrays overriding primitives
    EXPECT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, class1->CreateArrayProperty(stringArrayProperty, L"IntegerProperty", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECOBJECTS_STATUS_Success, class1->CreateArrayProperty(stringArrayProperty, L"StringProperty", PRIMITIVETYPE_String));
    class1->RemoveProperty(L"StringProperty");
    EXPECT_EQ(ECOBJECTS_STATUS_Success, class1->CreateArrayProperty(stringArrayProperty2, L"StringProperty"));

    // Arrays overriding structs
    EXPECT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, class1->CreateArrayProperty(structArrayProperty, L"StructProperty", structClass2));
    EXPECT_EQ(ECOBJECTS_STATUS_Success, class1->CreateArrayProperty(structArrayProperty, L"StructProperty", structClass));
    class1->RemoveProperty(L"StructProperty");

    // the default array type is string if none is passed in
    EXPECT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, class1->CreateArrayProperty(structArrayProperty2, L"StructProperty"));
    
    ArrayECPropertyP intArrayProperty;
    // Arrays overriding arrays
    EXPECT_EQ(ECOBJECTS_STATUS_DataTypeMismatch, class1->CreateArrayProperty(intArrayProperty, L"StringArrayProperty", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECOBJECTS_STATUS_Success, class1->CreateArrayProperty(stringArrayProperty, L"StringArrayProperty", PRIMITIVETYPE_String));
    class1->RemoveProperty(L"StringArrayProperty");

    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, ExpectPropertiesInOrder)
    {
    std::vector<const wchar_t *> propertyNames;
    propertyNames.push_back(L"beta");
    propertyNames.push_back(L"gamma");
    propertyNames.push_back(L"delta");
    propertyNames.push_back(L"alpha");
    
    ECSchemaPtr schema;
    ECClassP class1;
    PrimitiveECPropertyP property1;
    PrimitiveECPropertyP property2;
    PrimitiveECPropertyP property3;
    PrimitiveECPropertyP property4;
    
    ECSchema::CreateSchema(schema, L"TestSchema");
    schema->CreateClass(class1, L"TestClass");
    class1->CreatePrimitiveProperty(property1, L"beta");
    class1->CreatePrimitiveProperty(property2, L"gamma");
    class1->CreatePrimitiveProperty(property3, L"delta");
    class1->CreatePrimitiveProperty(property4, L"alpha");
    
    int i = 0;
    for each (ECPropertyP prop in class1->GetProperties(false))
        {
        EXPECT_EQ(0, prop->Name.compare(propertyNames[i]));
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
    
    ECSchema::CreateSchema(schema, L"TestSchema");
    schema->CreateClass(ab, L"ab");
    schema->CreateClass(cd, L"cd");
    schema->CreateClass(ef, L"ef");

    ab->CreatePrimitiveProperty(a, L"a");
    ab->CreatePrimitiveProperty(b, L"b");

    cd->CreatePrimitiveProperty(c, L"c");
    cd->CreatePrimitiveProperty(d, L"d");
    
    ef->CreatePrimitiveProperty(e, L"e");
    ef->CreatePrimitiveProperty(f, L"f");
    
    cd->AddBaseClass(*ab);
    ef->AddBaseClass(*cd);

    EXPECT_TRUE(NULL != ef->GetPropertyP(L"e"));    
    EXPECT_TRUE(NULL != ef->GetPropertyP(L"c"));    
    EXPECT_TRUE(NULL != ef->GetPropertyP(L"a"));    
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
    
    ECSchema::CreateSchema(schema, L"TestSchema");
    schema->CreateClass(ab, L"ab");
    schema->CreateClass(cd, L"cd");
    schema->CreateClass(ef, L"ef");
    schema->CreateClass(gh, L"gh");
    schema->CreateClass(ij, L"ij");
    schema->CreateClass(kl, L"kl");
    schema->CreateClass(mn, L"mn");

    ab->CreatePrimitiveProperty(a, L"a");
    ab->CreatePrimitiveProperty(b, L"b");

    cd->CreatePrimitiveProperty(c, L"c");
    cd->CreatePrimitiveProperty(d, L"d");
    
    ef->CreatePrimitiveProperty(e, L"e");
    ef->CreatePrimitiveProperty(f, L"f");
    
    gh->CreatePrimitiveProperty(g, L"g");
    gh->CreatePrimitiveProperty(h, L"h");
    
    ij->CreatePrimitiveProperty(i, L"i");
    ij->CreatePrimitiveProperty(j, L"j");
    
    kl->CreatePrimitiveProperty(k, L"k");
    kl->CreatePrimitiveProperty(l, L"l");
    
    mn->CreatePrimitiveProperty(m, L"m");
    mn->CreatePrimitiveProperty(n, L"n");
    
    ef->AddBaseClass(*ab);
    ef->AddBaseClass(*cd);
    
    kl->AddBaseClass(*gh);
    kl->AddBaseClass(*ij);
    
    mn->AddBaseClass(*ef);
    mn->AddBaseClass(*kl);
    
    std::vector<ECPropertyP> testVector;
    for each (ECPropertyP prop in mn->GetProperties(true))
        testVector.push_back(prop);
        
    EXPECT_EQ(14, testVector.size());
    EXPECT_EQ(0, testVector[0]->Name.compare(L"i"));
    EXPECT_EQ(0, testVector[1]->Name.compare(L"j"));
    EXPECT_EQ(0, testVector[2]->Name.compare(L"g"));
    EXPECT_EQ(0, testVector[3]->Name.compare(L"h"));
    EXPECT_EQ(0, testVector[4]->Name.compare(L"k"));
    EXPECT_EQ(0, testVector[5]->Name.compare(L"l"));
    EXPECT_EQ(0, testVector[6]->Name.compare(L"c"));
    EXPECT_EQ(0, testVector[7]->Name.compare(L"d"));
    EXPECT_EQ(0, testVector[8]->Name.compare(L"a"));
    EXPECT_EQ(0, testVector[9]->Name.compare(L"b"));
    EXPECT_EQ(0, testVector[10]->Name.compare(L"e"));
    EXPECT_EQ(0, testVector[11]->Name.compare(L"f"));
    EXPECT_EQ(0, testVector[12]->Name.compare(L"m"));
    EXPECT_EQ(0, testVector[13]->Name.compare(L"n"));
    
    // now we add some duplicate properties to mn which will "override" those from the base classes
    PrimitiveECPropertyP b2;
    PrimitiveECPropertyP d2;
    PrimitiveECPropertyP f2;
    PrimitiveECPropertyP h2;
    PrimitiveECPropertyP j2;
    PrimitiveECPropertyP k2;
    
    mn->CreatePrimitiveProperty(b2, L"b");
    mn->CreatePrimitiveProperty(d2, L"d");
    mn->CreatePrimitiveProperty(f2, L"f");
    mn->CreatePrimitiveProperty(h2, L"h");
    mn->CreatePrimitiveProperty(j2, L"j");
    mn->CreatePrimitiveProperty(k2, L"k");

    testVector.clear();
    for each (ECPropertyP prop in mn->GetProperties(true))
        testVector.push_back(prop);
        
    EXPECT_EQ(14, testVector.size());
    EXPECT_EQ(0, testVector[0]->Name.compare(L"i"));
    EXPECT_EQ(0, testVector[1]->Name.compare(L"g"));
    EXPECT_EQ(0, testVector[2]->Name.compare(L"l"));
    EXPECT_EQ(0, testVector[3]->Name.compare(L"c"));
    EXPECT_EQ(0, testVector[4]->Name.compare(L"a"));
    EXPECT_EQ(0, testVector[5]->Name.compare(L"e"));
    EXPECT_EQ(0, testVector[6]->Name.compare(L"m"));
    EXPECT_EQ(0, testVector[7]->Name.compare(L"n"));
    EXPECT_EQ(0, testVector[8]->Name.compare(L"b"));
    EXPECT_EQ(0, testVector[9]->Name.compare(L"d"));
    EXPECT_EQ(0, testVector[10]->Name.compare(L"f"));
    EXPECT_EQ(0, testVector[11]->Name.compare(L"h"));
    EXPECT_EQ(0, testVector[12]->Name.compare(L"j"));
    EXPECT_EQ(0, testVector[13]->Name.compare(L"k"));

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

    kl->CreatePrimitiveProperty(e2, L"e");
    kl->CreatePrimitiveProperty(a2, L"a");
    kl->CreatePrimitiveProperty(c2, L"c");
    kl->CreatePrimitiveProperty(g2, L"g");
    
    ef->CreatePrimitiveProperty(l2, L"l");
    gh->CreatePrimitiveProperty(i2, L"i");
    ij->CreatePrimitiveProperty(g3, L"g");
    
    gh->CreatePrimitiveProperty(a3, L"a");
    gh->CreatePrimitiveProperty(b3, L"b");
    ab->CreatePrimitiveProperty(g4, L"g");
    ab->CreatePrimitiveProperty(h3, L"h");

    testVector.clear();
    for each (ECPropertyP prop in mn->GetProperties(true))
        testVector.push_back(prop);
        
    EXPECT_EQ(14, testVector.size());
    EXPECT_EQ(0, testVector[0]->Name.compare(L"i"));
    EXPECT_EQ(0, testVector[1]->Name.compare(L"c"));
    EXPECT_EQ(0, testVector[2]->Name.compare(L"a"));
    EXPECT_EQ(0, testVector[3]->Name.compare(L"g"));
    EXPECT_EQ(0, testVector[4]->Name.compare(L"e"));
    EXPECT_EQ(0, testVector[5]->Name.compare(L"l"));
    EXPECT_EQ(0, testVector[6]->Name.compare(L"m"));
    EXPECT_EQ(0, testVector[7]->Name.compare(L"n"));
    EXPECT_EQ(0, testVector[8]->Name.compare(L"b"));
    EXPECT_EQ(0, testVector[9]->Name.compare(L"d"));
    EXPECT_EQ(0, testVector[10]->Name.compare(L"f"));
    EXPECT_EQ(0, testVector[11]->Name.compare(L"h"));
    EXPECT_EQ(0, testVector[12]->Name.compare(L"j"));
    EXPECT_EQ(0, testVector[13]->Name.compare(L"k"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, AddAndRemoveConstraintClasses)
    {
    ECSchemaPtr schema;
    ECSchemaPtr refSchema;
    
    ECSchema::CreateSchema(schema, L"TestSchema");
    ECSchema::CreateSchema(refSchema, L"RefSchema");
    
    ECRelationshipClassP relClass;
    ECClassP targetClass;
    ECClassP sourceClass;
    
    schema->CreateRelationshipClass(relClass, L"RElationshipClass");
    schema->CreateClass(targetClass, L"Target");
    refSchema->CreateClass(sourceClass, L"Source");
    
    EXPECT_EQ(ECOBJECTS_STATUS_Success, relClass->Target.AddClass(*targetClass));
    EXPECT_EQ(ECOBJECTS_STATUS_SchemaNotFound, relClass->Source.AddClass(*sourceClass));
    
    schema->AddReferencedSchema(refSchema);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, relClass->Source.AddClass(*sourceClass));
    
    EXPECT_EQ(ECOBJECTS_STATUS_Success, relClass->Target.RemoveClass(*targetClass));
    EXPECT_EQ(ECOBJECTS_STATUS_ClassNotFound, relClass->Target.RemoveClass(*targetClass));
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, ExpectErrorWithBadClassName)
    {
    ECSchemaPtr schema;
    ECClassP class1;
    
    // . is an invalid character
    EXPECT_EQ(ECOBJECTS_STATUS_InvalidName, ECSchema::CreateSchema(schema, L"TestSchema.1.0"));

    ECSchema::CreateSchema(schema, L"TestSchema");
    
    EXPECT_EQ(ECOBJECTS_STATUS_InvalidName, schema->CreateClass(class1, L""));
    
    // name cannot be an empty string
    EXPECT_EQ(ECOBJECTS_STATUS_InvalidName, schema->CreateClass(class1, L"    "));
    
    // name cannot contain special characters
    EXPECT_EQ(ECOBJECTS_STATUS_InvalidName, schema->CreateClass(class1, L"&&&&"));
    
    // name cannot start with a digit
    EXPECT_EQ(ECOBJECTS_STATUS_InvalidName, schema->CreateClass(class1, L"0InvalidName"));
    
    // name may include underscores
    EXPECT_EQ(ECOBJECTS_STATUS_Success, schema->CreateClass(class1, L"_____"));
    
    // % is an invalid character
    EXPECT_EQ(ECOBJECTS_STATUS_InvalidName, schema->CreateClass(class1, L"%"));
    
    // a is a valid character
    EXPECT_EQ(ECOBJECTS_STATUS_Success, schema->CreateClass(class1, L"a"));
    
    // Names can only include characters from the intersection of 7bit ascii and alphanumeric
    EXPECT_EQ(ECOBJECTS_STATUS_InvalidName, schema->CreateClass(class1, L"abc123!@#"));
    EXPECT_EQ(ECOBJECTS_STATUS_Success, schema->CreateClass(class1, L"abc123"));

    }
          
END_BENTLEY_EC_NAMESPACE
