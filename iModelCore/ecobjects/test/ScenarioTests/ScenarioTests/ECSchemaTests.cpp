
#include "ECObjectsTestPCH.h"
#include <objbase.h>
#include <comdef.h>
using namespace Bentley::EC;

#include <tchar.h>
using namespace std;

#define SCHEMAS_PATH  L"" 

using namespace TestHelpers;

class ECSchemaTests : public ECSchemaTestFixture
    {
    public:
        ECClassP                    m_classP;
        ECClassP                    m_classA;
        PrimitiveECPropertyP        m_property;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Farrukh Latif     06/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus serializeSchema(ECSchemaP m_schema)
    {
    std::wstring xmlFile;
    xmlFile = L"Widgets.09.06.ecschema.xml";
    const wchar_t * writeFile = xmlFile.c_str();
    ECSchemaVerifier ecSchVer;
    SchemaSerializationStatus status = ecSchVer.WriteXmlToFile(writeFile, m_schema);
    EXPECT_EQ(SCHEMA_SERIALIZATION_STATUS_Success, status);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Farrukh Latif     06/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus serializeSchema_New(ECSchemaP m_schema)
    {
    std::wstring xmlFile;
    xmlFile = L"NewSchema.01.00.ecschema.xml";
    const wchar_t * writeFile = xmlFile.c_str();
    ECSchemaVerifier ecSchVer;
    SchemaSerializationStatus status = ecSchVer.WriteXmlToFile(writeFile, m_schema);
    EXPECT_EQ(SCHEMA_SERIALIZATION_STATUS_Success, status);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Farrukh Latif     06/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus serializeSchema_WithStructProperty(ECSchemaP m_schema)
    {
    std::wstring xmlFile;
    xmlFile = L"WidgetsWithStructProperty.09.06.ecschema.xml";
    const wchar_t * writeFile = xmlFile.c_str();
    ECSchemaVerifier ecSchVer;
    SchemaSerializationStatus status = ecSchVer.WriteXmlToFile(writeFile, m_schema);
    EXPECT_EQ(SCHEMA_SERIALIZATION_STATUS_Success, status);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Farrukh Latif     06/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus serializeSchema_WithClassModification(ECSchemaP m_schema)
    {
    std::wstring xmlFile;
    xmlFile = L"WidgetsWithClassModification.09.06.ecschema.xml";
    const wchar_t * writeFile = xmlFile.c_str();
    ECSchemaVerifier ecSchVer;
    SchemaSerializationStatus status = ecSchVer.WriteXmlToFile(writeFile, m_schema);
    EXPECT_EQ(SCHEMA_SERIALIZATION_STATUS_Success, status);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Farrukh Latif     07/10
+---------------+---------------+---------------+---------------+---------------+------*/
//BentleyStatus serializeSchema_WithNonEnglishCharacters(ECSchemaP m_schema, bwstring fileName)
//    {
    //std::wstring dirPath, xmlFile;
    //dirPath = GetOutRoot().c_str();
    //xmlFile = L".09.06.ecschema.xml";
    //fileName += xmlFile;
    //dirPath += fileName;
    //const wchar_t * writeFile = dirPath.c_str();
    //wcout<<"Serialization path: "<<writeFile<<endl;
    //ECSchemaVerifier ecSchVer;
    //SchemaSerializationStatus status = ecSchVer.WriteXmlToFile(writeFile, m_schema);
    //EXPECT_EQ(SCHEMA_SERIALIZATION_STATUS_Success, status);
    //return SUCCESS;
    //}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Farrukh Latif     06/10
+---------------+---------------+---------------+---------------+---------------+------*/
//BentleyStatus VerifySchema (bwstring findECObject)
//    {
        //TestDataManager tdm(L"Widgets.09.06.ecschema.xml", __FILE__, OPENMODE_READWRITE);
        //ECSchemaVerifier ecSchVer;
        //SchemaDeserializationStatus status = ecSchVer.ReadXmlFromFile (m_schema, tdm.GetPath().c_str(), NULL, NULL);
        //EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_Success, status)<< tdm.GetPath().c_str();
        //if (NULL != (m_schema->GetClassP(findECObject)))
        //{
        //    m_classP = m_schema->GetClassP(findECObject);
        //    wcout<<"Added Class is: "<<m_classP->Name.c_str()<<endl;
        //    EXPECT_EQ (findECObject.c_str(), m_classP->Name.c_str());
        //    return SUCCESS;
        //}
    //    return ERROR;
    //}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSchemaTests, circularBaseClass)
{
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));

    ECClassP m_ClassA;
    ECClassP m_ClassB;
    ECClassP m_ClassC;
    m_ClassA = m_schema->GetClassP(L"ecProject");
    m_ClassB = m_schema->GetClassP(L"AccessCustomAttributes");
    m_ClassC = m_schema->GetClassP(L"ecWidget");
    EXPECT_FALSE(m_ClassA->Is(m_ClassB));

    ECClassVerifier ecSchVer(m_schema);
    ecSchVer.AddBaseClass_Success(m_ClassB, m_ClassA);
    EXPECT_TRUE(m_ClassA->Is(m_ClassB));
    ecSchVer.AddBaseClass_Success(m_ClassC, m_ClassB);
    EXPECT_TRUE(m_ClassB->Is(m_ClassC));

    //making circle of base classes
    ecSchVer.AddBaseClass_Failure(m_ClassA, m_ClassC);
    EXPECT_FALSE(m_ClassC->Is(m_ClassA));
    CoUninitialize();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSchemaTests, AddAndRemoveRelationshipBaseClass)
    {
    ECClassP class1;
    ECRelationshipClassP relBaseClass;
    ECClassVerifier ecClsVer(m_schema);
    ecClsVer.CreateClass_Success(class1, L"TestClass2");

    ECSchemaVerifier ecSchVer(m_schema);
    ecSchVer.CreateRelationshipClass_Success(relBaseClass, L"RelBaseClass");

    ecClsVer.AddBaseClass_Success(relBaseClass, class1);
    ECBaseClassesList classList = class1->GetBaseClasses();
    ECBaseClassesList::const_iterator baseClassIterator;
    for (baseClassIterator = classList.begin(); baseClassIterator != classList.end(); baseClassIterator++)
        {
        if (*baseClassIterator == relBaseClass)
            break;
        }
    EXPECT_FALSE(baseClassIterator == classList.end());
    
    // Remove relationship base class
    EXPECT_EQ(ECOBJECTS_STATUS_Success, class1->RemoveBaseClass(*relBaseClass));
    classList = class1->GetBaseClasses();
    for (baseClassIterator = classList.begin(); baseClassIterator != classList.end(); baseClassIterator++)
        {
        if (*baseClassIterator == relBaseClass)
            break;
        }
    EXPECT_TRUE(baseClassIterator == classList.end());
    EXPECT_EQ(ECOBJECTS_STATUS_ClassNotFound, class1->RemoveBaseClass(*relBaseClass));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSchemaTests, modifyECClass)
{
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    m_classP = m_schema->GetClassP(L"ecWidget");
    ECClassVerifier ecClsVer(m_schema);
    ecClsVer.ModifyECClass(m_classP, L"TestWidget Label", 0); // 0 indicates DisplayLabel
    ecClsVer.ModifyECClass(m_classP, L"TestWidget Desc", 1); // 1 indicates Description
    ECClassP m_myClass;
    ecClsVer.CreateClass_Success(m_myClass, L"Verifier");
    serializeSchema_WithClassModification(m_schema);
    CoUninitialize();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSchemaTests, multipleBaseClasses)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    //ECSchemaP     schema;
    ECClassP        class1;
    ECClassP        baseClass1;
    ECClassP        baseClass2;
    
    //ECSchemaVerifier ecSchVer;
    //ecSchVer.CreateSchema_Success(schema, L"TestSchema");

    ECClassVerifier ecClsVer(m_schema);
    ecClsVer.CreateClass_Success(class1, L"ChildClass");
    ecClsVer.CreateClass_Success(baseClass1, L"BaseClass1");
    ecClsVer.CreateClass_Success(baseClass2, L"BaseClass2");
    
    EXPECT_FALSE(class1->Is(baseClass1));
    ecClsVer.AddBaseClass_Success(baseClass1, class1);
    EXPECT_TRUE(class1->Is(baseClass1));

    ecClsVer.AddBaseClass_Success(baseClass2, class1);
    EXPECT_TRUE(class1->Is(baseClass2));
    serializeSchema(m_schema);
    CoUninitialize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSchemaTests, InvalidSchemaName)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    ECSchemaP ecSchPtr;
    ECSchemaVerifier ecSchVer;
    ECSchemaOwnerPtr schemaOwner = ECSchemaOwner::CreateOwner();
    
    ecSchVer.CreateSchema_Success (ecSchPtr, L"test", 1, 0, *schemaOwner);
    serializeSchema_New(ecSchPtr);
    
    ECSchemaP ecSchPtr2;
    ecSchVer.CreateSchema_Failure (ecSchPtr2, L"test.01.00", 1, 0, *schemaOwner);

    ECSchemaP ecSchPtr3;
    ecSchVer.CreateSchema_Failure (ecSchPtr3, L"test&01", 1, 0, *schemaOwner);
    CoUninitialize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSchemaTests, InvalidClassName)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));

    ECClassP testClass2;
    ECClassVerifier ecClsVer(m_schema);

    ASSERT_TRUE(m_schema != NULL);

    //////////////////////////////// D-66522 ///////////////////////////////
    ecClsVer.CreateClass_Failure(ECOBJECTS_STATUS_InvalidName, L"Test.Class");
    ecClsVer.CreateClass_Failure(ECOBJECTS_STATUS_InvalidName, L"2TestClass");
    ecClsVer.CreateClass_Failure(ECOBJECTS_STATUS_InvalidName, L"");
    ecClsVer.CreateClass_Failure(ECOBJECTS_STATUS_InvalidName, L" ");
    ecClsVer.CreateClass_Failure(ECOBJECTS_STATUS_InvalidName, L"$Test");
    ecClsVer.CreateClass_Failure(ECOBJECTS_STATUS_InvalidName, L"Tes%t");
    ecClsVer.CreateClass_Failure(ECOBJECTS_STATUS_InvalidName, L"Test^");
    ecClsVer.CreateClass_Failure(ECOBJECTS_STATUS_InvalidName, L"Te)st");
    //ecClsVer.CreateClass_Success(testClass, L"TestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassTestClassdfsfsfsdsdTestClassTestClassTestClassTestClassTestClassTestClassdfsfsfsdsdsjsksf"); //No. of characters 8,189
    //to do --- incoporate all special characters
    ecClsVer.CreateClass_Success(testClass2, L"MyTestClass2");
    serializeSchema(m_schema);
    CoUninitialize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSchemaTests, InvalidRelationshipName)
    {
    ECSchemaP schema;
    ECRelationshipClassP relBaseClass;
    ECSchemaOwnerPtr schemaOwner = ECSchemaOwner::CreateOwner();
    
    ECSchema::CreateSchema(schema, L"TestSchema", 1, 0, *schemaOwner);
    EXPECT_EQ (ECOBJECTS_STATUS_InvalidName, schema->CreateRelationshipClass(relBaseClass, L"Rel.BaseClass"));
    EXPECT_EQ (ECOBJECTS_STATUS_InvalidName, schema->CreateRelationshipClass(relBaseClass, L"1RelBaseClass"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSchemaTests, ExpectSuccessWhenDeserializingWidgetsECSchema)
    {
    ECClassP pClass;
    ECClassP pClass1;
    ECClassP pClass2;

    pClass = m_schema->GetClassP(L"ecProject");
    EXPECT_STREQ (L"ecProject", pClass->Name.c_str());
    ECPropertyP pProperty = pClass->GetPropertyP (L"Name");
    EXPECT_STREQ (L"Name", pProperty->Name.c_str());
    pClass1 = m_schema->GetClassP(L"RelatedClass");
    ECPropertyP pProperty1 = pClass1->GetPropertyP (L"StringMember");
    wprintf (L"Class Name: '%s' with display label '%s'\n", pClass1->Name.c_str(), pClass1->DisplayLabel.c_str());
    wprintf (L"Property Name: '%s' with display label: '%s' with Class '%s'\n", 
            pProperty1->Name.c_str(), pProperty1->DisplayLabel.c_str(), pProperty1->Class.Name.c_str() );

    pClass2 = m_schema->GetClassP(L"TestClass");
    ECPropertyP pProperty2 = pClass2->GetPropertyP (L"StringMember");
    wprintf (L"Class Name: '%s' with display label '%s'\n", pClass2->Name.c_str(), pClass2->DisplayLabel.c_str());
    wprintf (L"Property Name: '%s' with display label: '%s' with Class '%s'\n", 
    pProperty2->Name.c_str(), pProperty2->DisplayLabel.c_str(), pProperty2->Class.Name.c_str() );
    ASSERT_TRUE (pProperty->IsPrimitive);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSchemaTests, PropertiesValidation)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    m_classP = m_schema->GetClassP(L"ecWidget");
    ECPropertyP pProperty = m_classP->GetPropertyP(L"eclongattr");
    EXPECT_STREQ (L"eclongattr", pProperty->Name.c_str());
    wprintf (L"Property Name: '%s'\n", pProperty->TypeName.c_str());
    EXPECT_EQ(false, pProperty->IsReadOnly);
    m_schema->CreateClass(m_classP, L"HelloClass");
    ECClassVerifier ecClsVer(m_schema);
    ecClsVer.CreateClass_Success (m_classP, L"HelloClass2");
    serializeSchema(m_schema);
    CoUninitialize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSchemaTests, createStructProperty)
{
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    m_classP = m_schema->GetClassP(L"ecProject");
    m_classP->SetIsStruct(L"True");
    StructECPropertyP m_StructProperty;
    ECPropertyVerifier ecPropVer(m_schema);
    m_classA = m_schema->GetClassP(L"HelloClass");
    ecPropVer.CreateStructProperty(m_StructProperty, L"VerifierStructProp", *m_classP, *m_classA);
    serializeSchema_WithStructProperty(m_schema);
    CoUninitialize();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSchemaTests, StructTypePropertiesValidation)
    {
    m_classP = m_schema->GetClassP(L"Struct2");
    ECPropertyP pProperty = m_classP->GetPropertyP(L"NestedArray");
    EXPECT_STREQ (L"NestedArray", pProperty->Name.c_str());
    wprintf (L"Property Name: '%s' with display label: '%s' with Class '%s' Type Name: '%s'\n", 
    pProperty->Name.c_str(), pProperty->DisplayLabel.c_str(), pProperty->Class.Name.c_str(), pProperty->TypeName.c_str() );
    EXPECT_EQ(false, pProperty->IsReadOnly);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSchemaTests, AddProperty)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    m_classP = m_schema->GetClassP(L"ecProject");
    PrimitiveECPropertyP pProperty;
    ECPropertyVerifier ecPropVer;

    ecPropVer.CreatePrimitiveProperty(pProperty, L"NewProp", PRIMITIVETYPE_Double, m_classP);
    serializeSchema(m_schema);
    CoUninitialize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSchemaTests, AddClass)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    int count = 0;
    ECClassContainerCR classes = m_schema->Classes;
    for(ECClassContainer::const_iterator iter=classes.begin(); iter != classes.end(); ++iter)
        {
        wprintf ( L"Class Name: '%s'\n" , (*iter)->Name);
        count++;    
        }
    wprintf ( L"Count :" , count );
    ECClassP class1;
    m_schema->CreateClass(class1, L"TestClassB");
    serializeSchema(m_schema);
    CoUninitialize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
//TEST_F(ECSchemaTests, maximumClassesInASchema)
//    {
//    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
//    ECClassP class1;
//    ECClassVerifier ecClsVer(m_schema);
//    for(int i =1; i <= 8192; i++)
//        {
//        WString className(L"C");
//        char charInt[10];
//        _itoa(i, charInt, 10);
//        className.Append(charInt);
//        ecClsVer.CreateClass_Success(class1, className);
//        }
//    serializeSchema(m_schema);
//    CoUninitialize();
//    }
