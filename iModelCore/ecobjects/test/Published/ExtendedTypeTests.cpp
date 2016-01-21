/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/ExtendedTypeTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct ExtendedTypeTests : ECTestFixture
    {
    
		static Utf8String    GetTestSchemaXMLString ()
        {
        Utf8Char schemaFragment[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<ECSchema schemaName=\"ExtendedTypeTesting\" nameSpacePrefix=\"test\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
            "    <ECClass typeName=\"ClassA\" displayLabel=\"Class B\" isDomainClass=\"True\">"
            "        <ECProperty propertyName=\"knownExtendedType\" extendedTypeName=\"color\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"unknownExtendedType\" extendedTypeName=\"banana\" typeName=\"int\" />"
            "        <ECProperty propertyName=\"withoutExtendedType\" typeName=\"int\" />"
            "    </ECClass>"
            "</ECSchema>";

        return schemaFragment;
        }

    static ECSchemaPtr       CreateTestSchema ()
        {
        Utf8String schemaXMLString = GetTestSchemaXMLString ();
        ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext ();

        ECSchemaPtr schema;
        EXPECT_EQ (SchemaReadStatus::Success, ECSchema::ReadFromXmlString (schema, schemaXMLString.c_str (), *schemaContext));

        return schema;
        }
    };
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ExtendedTypeTests, ReadSchemaWithExtendedTypes)
    {
    ECSchemaPtr schemaPtr = CreateTestSchema();
    ECClassP classPtr = schemaPtr->GetClassP("ClassA");
    
    ECPropertyP property1 = classPtr->GetPropertyP("knownExtendedType");
    EXPECT_FALSE(property1 == nullptr) << "Property knownExtendedType not found on ClassA.";

    ECPropertyP property2 = classPtr->GetPropertyP("unknownExtendedType");
    EXPECT_FALSE(property2 == nullptr) << "Property unknownExtendedType not found on ClassA.";

    ECPropertyP property3 = classPtr->GetPropertyP("withoutExtendedType");
    EXPECT_FALSE(property3 == nullptr) << "Property withoutExtendedType not found on ClassA.";

    // Checks if the ECProperties are flagged correctly as extended type properties
    EXPECT_TRUE(property1->HasExtendedType())  << "Property knownExtendedType was expected to return true on HasExtendedType.";
    EXPECT_TRUE(property2->HasExtendedType())  << "Property unknownExtendedType was expected to return true on HasExtendedType.";
    EXPECT_FALSE(property3->HasExtendedType()) << "Property withoutExtendedType was expected to return false on HasExtendedType.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ExtendedTypeTests, WriteSchemaWithExtendedTypes)
    {
    ECSchemaPtr schemaPtr;
    ECSchema::CreateSchema(schemaPtr, "TestSchema", 1, 0);

    ECEntityClassP classPtr;
    schemaPtr.get()->CreateEntityClass(classPtr, "ClassA");
    PrimitiveECPropertyP propertyPtr;
    classPtr->CreatePrimitiveProperty(propertyPtr, "knownExtendedType");
    if (ECObjectsStatus::Success != propertyPtr->SetExtendedTypeName("color"))
        {
        FAIL() << "Couldn't set the ExtendedTypeName on property knownExtendedType";
        }

    WString resultString;
    schemaPtr->WriteToXmlString(resultString);

    ECSchemaPtr resultSchema;
    ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(resultSchema, resultString.c_str(), *schemaContext);

    ECClassP resultClassPtr = schemaPtr.get()->GetClassP("ClassA");

    ECPropertyP property1 = resultClassPtr->GetPropertyP("knownExtendedType");
    EXPECT_FALSE(property1 == nullptr) << "Property knownExtendedType not found on ClassA.";

    PrimitiveECPropertyP primitiveProperty = property1->GetAsPrimitivePropertyP();
    EXPECT_FALSE(primitiveProperty == nullptr) << "Property knownExtendedType is not an PrimitiveECProperty.";

    Utf8String extendedTypeName = primitiveProperty->GetExtendedTypeName();
    EXPECT_EQ("color", extendedTypeName) << "Unexpected extended type name " << extendedTypeName << "expected: 'color'";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ExtendedTypeTests, GetKnownExtendedType)
    {
    ECSchemaPtr schemaPtr = CreateTestSchema();
    ECClassP classPtr = schemaPtr->GetClassP("ClassA");

    ECPropertyP property1 = classPtr->GetPropertyP("knownExtendedType");
    EXPECT_FALSE(property1 == nullptr) << "Property knownExtendedType not found on ClassA.";

    PrimitiveECPropertyP primitiveProperty = property1->GetAsPrimitivePropertyP();
    EXPECT_FALSE(primitiveProperty == nullptr) << "Property knownExtendedType is not an PrimitiveECProperty.";

    Utf8String extendedTypeName = primitiveProperty->GetExtendedTypeName();
    EXPECT_EQ("color", extendedTypeName) << "Unexpected extended type name " << extendedTypeName << "expected: 'color'";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ExtendedTypeTests, SetExtendedTypeToNull)
    {
    ECSchemaPtr schemaPtr;
    ECSchema::CreateSchema(schemaPtr, "TestSchema", 1, 0);

    ECEntityClassP classPtr;
    schemaPtr.get()->CreateEntityClass(classPtr, "ClassA");

    PrimitiveECPropertyP colorPropertyPtr, resetPropertyPtr;
    classPtr->CreatePrimitiveProperty(colorPropertyPtr, "colorProperty");
    classPtr->CreatePrimitiveProperty(resetPropertyPtr, "resetProperty");

    if (ECObjectsStatus::Success != colorPropertyPtr->SetExtendedTypeName("color"))
        {
        FAIL() << "Couldn't set the ExtendedTypeName on property colorProperty";
        }

    if (ECObjectsStatus::Success != resetPropertyPtr->SetExtendedTypeName("reset"))
        {
        FAIL() << "Couldn't set the ExtendedTypeName on property resetProperty";
        }

    EXPECT_TRUE(colorPropertyPtr->HasExtendedType()) << "Property colorProperty was expected to return true on HasExtendedType.";
    EXPECT_TRUE(resetPropertyPtr->HasExtendedType()) << "Property resetProperty was expected to return true on HasExtendedType.";
   
    
    if (ECObjectsStatus::Success != colorPropertyPtr->SetExtendedTypeName(nullptr))
        {
        FAIL() << "Couldn't set the ExtendedTypeName to nullptr on property colorProperty";
        }
    if (!resetPropertyPtr->RemoveExtendedTypeName())
        {
        FAIL() << "Couldn't set the ExtendedTypeName to nullptr on property resetProperty";
        }
    EXPECT_FALSE(colorPropertyPtr->HasExtendedType()) << "Property colorProperty was expected to return true on HasExtendedType.";
    EXPECT_FALSE(resetPropertyPtr->HasExtendedType()) << "Property resetProperty was expected to return true on HasExtendedType.";
    
    }
    
END_BENTLEY_ECN_TEST_NAMESPACE
