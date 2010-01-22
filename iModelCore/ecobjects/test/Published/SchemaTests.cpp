
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

BEGIN_BENTLEY_EC_NAMESPACE

// NEEDSWORK Improve strategy for seed data.  Should not be maintained in source.
#define SCHEMAS_PATH  L"" 

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
TEST(SchemaDeserializationTest, ExpectErrorWhenCOMNotInitialized)
    {
    ECSchemaPtr schema;        
    
    DISABLE_ASSERTS
    SchemaDeserializationStatus status = ECSchema::ReadXmlFromFile (schema, L"t");
        
    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_FailedToInitializeMsmxl, status);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SchemaDeserializationTest, ExpectErrorWhenXmlFileDoesNotExist)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    ECSchemaPtr schema;
    
    SchemaDeserializationStatus status = ECSchema::ReadXmlFromFile (schema, SCHEMAS_PATH L"ThisFileIsntReal.xml");   

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_FailedToParseXml, status);
    
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SchemaDeserializationTest, ExpectErrorWhenXmlFileIsMissingNodes)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    ECSchemaPtr schema;
    
    SchemaDeserializationStatus status = ECSchema::ReadXmlFromFile (schema, SCHEMAS_PATH L"MissingNodes.01.00.ecschema.xml");   

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_FailedToParseXml, status);
    
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SchemaDeserializationTest, ExpectErrorWhenXmlFileIsIllFormed)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    ECSchemaPtr schema;
    
    SchemaDeserializationStatus status = ECSchema::ReadXmlFromFile (schema, SCHEMAS_PATH L"IllFormedXml.01.00.ecschema.xml");   

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_FailedToParseXml, status);
    
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SchemaDeserializationTest, ExpectErrorWhenXmlFileIsMissingECSchemaNode)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    ECSchemaPtr schema;
    
    SchemaDeserializationStatus status = ECSchema::ReadXmlFromFile (schema, SCHEMAS_PATH L"MissingECSchemaNode.01.00.ecschema.xml");   

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml, status);
    
    CoUninitialize();
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SchemaDeserializationTest, ExpectErrorWhenXmlFileIsMissingNamespace)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    ECSchemaPtr schema;
    
    SchemaDeserializationStatus status = ECSchema::ReadXmlFromFile (schema, SCHEMAS_PATH L"MissingNamespace.01.00.ecschema.xml");   

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml, status);
    
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SchemaDeserializationTest, ExpectErrorWhenXmlFileHasUnsupportedNamespace)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    ECSchemaPtr schema;
    
    SchemaDeserializationStatus status = ECSchema::ReadXmlFromFile (schema, SCHEMAS_PATH L"UnsupportedECXmlNamespace.01.00.ecschema.xml");   

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml, status);
    
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SchemaDeserializationTest, ExpectErrorWhenXmlFileHasMissingSchemaNameAttribute)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    ECSchemaPtr schema;
    
    SchemaDeserializationStatus status = ECSchema::ReadXmlFromFile (schema, SCHEMAS_PATH L"MissingSchemaName.01.00.ecschema.xml");   

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml, status);
    
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SchemaDeserializationTest, ExpectErrorWhenXmlFileHasMissingClassNameAttribute)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    ECSchemaPtr schema;
    
    SchemaDeserializationStatus status = ECSchema::ReadXmlFromFile (schema, SCHEMAS_PATH L"MissingClassName.01.00.ecschema.xml");   

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml, status);
    
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SchemaDeserializationTest, ExpectSuccessWhenXmlFileHasInvalidVersionString)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    ECSchemaPtr schema;
    
    SchemaDeserializationStatus status = ECSchema::ReadXmlFromFile (schema, SCHEMAS_PATH L"InvalidVersionString.01.00.ecschema.xml");   
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
    ECSchemaPtr schema;

    SchemaDeserializationStatus status = ECSchema::ReadXmlFromFile (schema, SCHEMAS_PATH L"OnlyRequiredECSchemaAttributes.01.00.ecschema.xml");   

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
TEST(SchemaDeserializationTest, ExpectSuccessWhenDeserializingWidgetsECSchema)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    ECSchemaPtr schema;
    
    SchemaDeserializationStatus status = ECSchema::ReadXmlFromFile (schema, SCHEMAS_PATH L"Widgets.01.00.ecschema.xml");   

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_Success, status);  
    VerifyWidgetsSchema(schema);  
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SchemaDeserializationTest, ExpectSuccessWhenDeserializingECSchemaFromString)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    ECSchemaPtr schema;
    
    SchemaDeserializationStatus status = ECSchema::ReadXmlFromString (schema, 
        L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        L"<ECSchema schemaName=\"Widgets\" version=\"09.06\" displayLabel=\"Widgets Display Label\" description=\"Widgets Description\" nameSpacePrefix=\"wid\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" xmlns:ec=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" xmlns:ods=\"Bentley_ODS.01.02\">"
        L"<ECSchemaReference name=\"EditorCustomAttributes\" version=\"01.00\" prefix=\"beca\" />"
        L"    <ECClass typeName=\"ecProject\" description=\"Project ECClass\" displayLabel=\"Project\" isDomainClass=\"True\">"
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
TEST(SchemaDeserializationTest, ExpectSuccessWhenRoundtripUsingString)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    ECSchemaPtr schema;
    
    SchemaDeserializationStatus status = ECSchema::ReadXmlFromFile (schema, SCHEMAS_PATH L"Widgets.01.00.ecschema.xml");
    wprintf(L"Verifying original schema from file.\n"); 
    VerifyWidgetsSchema(schema);

    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_Success, status);

    const wchar_t *ecSchemaXmlString;
    
    SchemaSerializationStatus status2 = schema->WriteXmlToString(ecSchemaXmlString);
    EXPECT_EQ(SCHEMA_SERIALIZATION_STATUS_Success, status2);
    
    ECSchemaPtr deserializedSchema;
    status = ECSchema::ReadXmlFromString(deserializedSchema, ecSchemaXmlString);
    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_Success, status); 
    wprintf(L"Verifying schema deserialized from string.\n");
    VerifyWidgetsSchema(deserializedSchema);

    CoUninitialize();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
//TEST(SchemaDeserializationTest, ExpectSuccessWhenSerializingToFile)
//    {
//    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
//    ECSchemaPtr schema;
//    
//    SchemaDeserializationStatus status = ECSchema::ReadXmlFromFile (schema, SCHEMAS_PATH L"Widgets.01.00.ecschema.xml");
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
//TEST(SchemaDeserializationTest, ExpectSuccessWhenRoundtripUsingStream)
//    {
//    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
//    ECSchemaPtr schema;
//    
//    SchemaDeserializationStatus status = ECSchema::ReadXmlFromFile (schema, SCHEMAS_PATH L"Widgets.01.00.ecschema.xml");
//    wprintf(L"Verifying original schema from file.\n"); 
//    VerifyWidgetsSchema(schema);
//
//    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_Success, status);    
//    LPSTREAM stream = NULL;
//    HRESULT res = ::CreateStreamOnHGlobal(NULL,TRUE,&stream);
//
//    SchemaSerializationStatus status2 = schema->WriteXmlToStream(stream);
//    EXPECT_EQ(SCHEMA_SERIALIZATION_STATUS_Success, status2);
//    
//    LARGE_INTEGER liPos = {0};
//    stream->Seek(liPos, STREAM_SEEK_SET, NULL);
//
//    ECSchemaPtr deserializedSchema;
//    status = ECSchema::ReadXmlFromStream(deserializedSchema, stream);
//    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_Success, status); 
//    wprintf(L"Verifying schema deserialized from stream.\n");
//    VerifyWidgetsSchema(deserializedSchema);
//
//    CoUninitialize();
//    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SchemaSerializationTest, ExpectErrorWhenCOMNotInitialized)
    {
    ECSchemaPtr schema;        
    ECSchema::CreateSchema(schema, L"Widget");
    
    DISABLE_ASSERTS
    const wchar_t *ecSchemaXmlString;
    
    SchemaSerializationStatus status = schema->WriteXmlToString(ecSchemaXmlString);
        
    EXPECT_EQ (SCHEMA_SERIALIZATION_STATUS_FailedToInitializeMsmxl, status);
    };
    
TEST(SchemaSerializationTest, ExpectSuccessWithSerializingBaseClasses)
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
    schema->AddReferencedSchema(*schema2);
    schema->AddReferencedSchema(*schema3);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, class1->AddBaseClass(*baseClass));
    EXPECT_EQ(ECOBJECTS_STATUS_Success, class1->AddBaseClass(*anotherBase));
    EXPECT_EQ(ECOBJECTS_STATUS_Success, gadget->AddBaseClass(*class1));
    
//    const wchar_t *ecSchemaXmlString;
    
    //SchemaSerializationStatus status2 = schema->WriteXmlToFile(L"d:\\temp\\base.xml");
    //
    //ECSchemaPtr schema4;
    //SchemaDeserializationStatus status3 = ECSchema::ReadXmlFromFile (schema4, L"d:\\temp\\base.xml");
    
    const wchar_t *ecSchemaXmlString;
    
    SchemaSerializationStatus status2 = schema->WriteXmlToString(ecSchemaXmlString);
    EXPECT_EQ(SCHEMA_SERIALIZATION_STATUS_Success, status2);
    
    CoUninitialize();
    }

TEST(SchemaReferenceTest, AddAndRemoveReferencedSchemas)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, L"TestSchema");
    
    ECSchemaPtr refSchema;
    ECSchema::CreateSchema(refSchema, L"RefSchema");
    
    EXPECT_EQ(ECOBJECTS_STATUS_Success, schema->AddReferencedSchema(*refSchema));
    EXPECT_EQ(ECOBJECTS_STATUS_NamedItemAlreadyExists, schema->AddReferencedSchema(*refSchema));
    
    ECSchemaReferenceList refList = schema->GetReferencedSchemas();
    ECSchemaReferenceList::const_iterator schemaIterator;
    for (schemaIterator = refList.begin(); schemaIterator != refList.end(); schemaIterator++)
        {
        if (*schemaIterator == refSchema.get())
            break;
        }
        
    EXPECT_FALSE(schemaIterator == refList.end());
    EXPECT_EQ(ECOBJECTS_STATUS_Success, schema->RemoveReferencedSchema(*refSchema));
    
    refList = schema->GetReferencedSchemas();
    for (schemaIterator = refList.begin(); schemaIterator != refList.end(); schemaIterator++)
        {
        if (*schemaIterator == refSchema.get())
            break;
        }
        
    EXPECT_TRUE(schemaIterator == refList.end());
    EXPECT_EQ(ECOBJECTS_STATUS_SchemaNotFound, schema->RemoveReferencedSchema(*refSchema));
    }
       
END_BENTLEY_EC_NAMESPACE
