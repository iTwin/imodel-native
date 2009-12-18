
/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/nativeatp/Published/SchemaTests.cpp $
|
|  $Copyright: (c) 2009 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include <objbase.h>
#include <comdef.h>

BEGIN_BENTLEY_EC_NAMESPACE

// NEEDSWORK Improve strategy for seed data.  Should not be maintained in source.
#define SCHEMAS_PATH  L"" 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SchemaDeserializationTest, ExpectErrorWhenCOMNotInitialized)
    {
    SchemaPtr schema;        
    
    DISABLE_ASSERTS
    SchemaDeserializationStatus status = Schema::ReadXMLFromFile (schema, L"t");
        
    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_FailedToInitializeMsmxl, status);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SchemaDeserializationTest, ExpectErrorWhenXMLFileDoesNotExist)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    SchemaPtr schema;
    
    SchemaDeserializationStatus status = Schema::ReadXMLFromFile (schema, SCHEMAS_PATH L"ThisFileIsntReal.xml");   

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_FailedToParseXml, status);
    
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SchemaDeserializationTest, ExpectErrorWhenXMLFileIsMissingNodes)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    SchemaPtr schema;
    
    SchemaDeserializationStatus status = Schema::ReadXMLFromFile (schema, SCHEMAS_PATH L"MissingNodes.01.00.ecschema.xml");   

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_FailedToParseXml, status);
    
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SchemaDeserializationTest, ExpectErrorWhenXMLFileIsIllFormed)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    SchemaPtr schema;
    
    SchemaDeserializationStatus status = Schema::ReadXMLFromFile (schema, SCHEMAS_PATH L"IllFormedXml.01.00.ecschema.xml");   

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_FailedToParseXml, status);
    
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SchemaDeserializationTest, ExpectErrorWhenXMLFileIsMissingECSchemaNode)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    SchemaPtr schema;
    
    SchemaDeserializationStatus status = Schema::ReadXMLFromFile (schema, SCHEMAS_PATH L"MissingECSchemaNode.01.00.ecschema.xml");   

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXML, status);
    
    CoUninitialize();
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SchemaDeserializationTest, ExpectErrorWhenXMLFileIsMissingNamespace)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    SchemaPtr schema;
    
    SchemaDeserializationStatus status = Schema::ReadXMLFromFile (schema, SCHEMAS_PATH L"MissingNamespace.01.00.ecschema.xml");   

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXML, status);
    
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SchemaDeserializationTest, ExpectErrorWhenXMLFileHasUnsupportedNamespace)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    SchemaPtr schema;
    
    SchemaDeserializationStatus status = Schema::ReadXMLFromFile (schema, SCHEMAS_PATH L"UnsupportedECXmlNamespace.01.00.ecschema.xml");   

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXML, status);
    
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SchemaDeserializationTest, ExpectErrorWhenXMLFileHasMissingSchemaNameAttribute)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    SchemaPtr schema;
    
    SchemaDeserializationStatus status = Schema::ReadXMLFromFile (schema, SCHEMAS_PATH L"MissingSchemaName.01.00.ecschema.xml");   

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXML, status);
    
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SchemaDeserializationTest, ExpectErrorWhenXMLFileHasMissingClassNameAttribute)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    SchemaPtr schema;
    
    SchemaDeserializationStatus status = Schema::ReadXMLFromFile (schema, SCHEMAS_PATH L"MissingClassName.01.00.ecschema.xml");   

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXML, status);
    
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SchemaDeserializationTest, ExpectSuccessWhenXMLFileHasInvalidVersionString)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    SchemaPtr schema;
    
    SchemaDeserializationStatus status = Schema::ReadXMLFromFile (schema, SCHEMAS_PATH L"InvalidVersionString.01.00.ecschema.xml");   
    EXPECT_EQ (1, schema->VersionMajor);
    EXPECT_EQ (0, schema->VersionMinor);

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_Success, status);
    
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SchemaDeserializationTest, ExpectSuccessWhenECSchemaContainsOnlyRequiredAttributes)
    {                
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    SchemaPtr schema;

    SchemaDeserializationStatus status = Schema::ReadXMLFromFile (schema, SCHEMAS_PATH L"OnlyRequiredECSchemaAttributes.01.00.ecschema.xml");   

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_Success, status);    
    EXPECT_STREQ (L"OnlyRequiredECSchemaAttributes", schema->Name.c_str());
    EXPECT_STREQ (L"", schema->NamespacePrefix.c_str());
    EXPECT_STREQ (L"OnlyRequiredECSchemaAttributes", schema->DisplayLabel.c_str());
    EXPECT_FALSE (schema->IsDisplayLabelDefined);
    EXPECT_STREQ (L"", schema->Description.c_str());
    EXPECT_EQ (1, schema->VersionMajor);
    EXPECT_EQ (0, schema->VersionMinor);
    
    ClassP pClass = schema->GetClassP(L"OnlyRequiredECClassAttributes");    
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
TEST(SchemaDeserializationTest, ExpectSuccessWhenDeserializingWidgetsECSchema)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    SchemaPtr schema;
    
    SchemaDeserializationStatus status = Schema::ReadXMLFromFile (schema, SCHEMAS_PATH L"Widgets.01.00.ecschema.xml");   

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_Success, status);    
    EXPECT_STREQ (L"Widgets", schema->Name.c_str());
    EXPECT_STREQ (L"wid", schema->NamespacePrefix.c_str());
    EXPECT_STREQ (L"Widgets Display Label", schema->DisplayLabel.c_str());
    EXPECT_TRUE (schema->IsDisplayLabelDefined);
    EXPECT_STREQ (L"Widgets Description", schema->Description.c_str());
    EXPECT_EQ (9, schema->VersionMajor);
    EXPECT_EQ (6, schema->VersionMinor);        
    
    for each (ClassP pClass in schema->Classes)
        {
        wprintf (L"Widgets contains class: '%s' with display label '%s'\n", pClass->Name.c_str(), pClass->DisplayLabel.c_str());
        }

    ClassP pClass = schema->GetClassP(L"ClassDoesNotExistInSchema");
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
    PropertyP pProperty = pClass->GetPropertyP (L"Name");
    EXPECT_TRUE (pProperty);
    EXPECT_STREQ (L"Name", pProperty->Name.c_str());
    EXPECT_TRUE (pProperty->IsPrimitive);
    EXPECT_FALSE (pProperty->IsStruct);
    EXPECT_FALSE (pProperty->IsArray);
    EXPECT_STREQ (L"string", pProperty->TypeName.c_str());
    EXPECT_EQ (PRIMITIVETYPE_String, pProperty->GetAsPrimitiveProperty()->Type);
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
    ArrayPropertyP arrayProperty = pProperty->GetAsArrayProperty();
    EXPECT_EQ (ELEMENTCLASSIFICATION_Struct, arrayProperty->ElementClassification);
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
    StructPropertyP structProperty = pProperty->GetAsStructProperty();    
    EXPECT_EQ (schema->GetClassP(L"Struct1"), &(structProperty->Type));
    EXPECT_FALSE (pProperty->IsDisplayLabelDefined);
    EXPECT_STREQ (L"EmbeddedStruct", pProperty->DisplayLabel.c_str());
    EXPECT_STREQ (L"", pProperty->Description.c_str());
    EXPECT_EQ (pClass, &pProperty->Class);
    EXPECT_FALSE (pProperty->IsReadOnly);
   
    for each (PropertyP pProperty in pClass->Properties)
        {
        wprintf (L"TestClass contains property: %s of type %s\n", pProperty->Name.c_str(), pProperty->TypeName.c_str());
        }
    
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SchemaDeserializationTest, ExpectSuccessWhenDeserializingECSchemaFromString)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    SchemaPtr schema;
    
    SchemaDeserializationStatus status = Schema::ReadXMLFromString (schema, 
        L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        L"<ECSchema schemaName=\"Widgets\" version=\"09.06\" displayLabel=\"Widgets Display Label\" description=\"Widgets Description\" nameSpacePrefix=\"wid\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" xmlns:ec=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" xmlns:ods=\"Bentley_ODS.01.02\">"
        L"<ECSchemaReference name=\"EditorCustomAttributes\" version=\"01.00\" prefix=\"beca\" />"
        L"    <ECClass typeName=\"ecProject\" description=\"Project Class\" displayLabel=\"Project\" isDomainClass=\"True\">"
        L"       <ECProperty propertyName=\"Name\" typeName=\"string\" displayLabel=\"Project Name\" />"
        L"    </ECClass>"
        L"</ECSchema>");   

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_Success, status);    
    EXPECT_STREQ (L"Widgets", schema->Name.c_str());
    EXPECT_STREQ (L"wid", schema->NamespacePrefix.c_str());
    EXPECT_STREQ (L"Widgets Display Label", schema->DisplayLabel.c_str());
    EXPECT_TRUE (schema->IsDisplayLabelDefined);
    EXPECT_STREQ (L"Widgets Description", schema->Description.c_str());
    EXPECT_EQ (9, schema->VersionMajor);
    EXPECT_EQ (6, schema->VersionMinor);        
    
    for each (ClassP pClass in schema->Classes)
        {
        wprintf (L"Widgets contains class: '%s' with display label '%s'\n", pClass->Name.c_str(), pClass->DisplayLabel.c_str());
        }

    ClassP pClass = schema->GetClassP(L"ClassDoesNotExistInSchema");
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
    PropertyP pProperty = pClass->GetPropertyP (L"Name");
    EXPECT_TRUE (pProperty);
    EXPECT_STREQ (L"Name", pProperty->Name.c_str());
    EXPECT_TRUE (pProperty->IsPrimitive);
    EXPECT_FALSE (pProperty->IsStruct);
    EXPECT_FALSE (pProperty->IsArray);
    EXPECT_STREQ (L"string", pProperty->TypeName.c_str());
    EXPECT_EQ (PRIMITIVETYPE_String, pProperty->GetAsPrimitiveProperty()->Type);
    EXPECT_TRUE (pProperty->IsDisplayLabelDefined);
    EXPECT_STREQ (L"Project Name", pProperty->DisplayLabel.c_str());
    EXPECT_STREQ (L"", pProperty->Description.c_str());
    EXPECT_EQ (pClass, &pProperty->Class);
    EXPECT_FALSE (pProperty->IsReadOnly);

    pProperty = pClass->GetPropertyP (L"PropertyDoesNotExistInClass");
    EXPECT_FALSE (pProperty);
    
    CoUninitialize();
    };

END_BENTLEY_EC_NAMESPACE
