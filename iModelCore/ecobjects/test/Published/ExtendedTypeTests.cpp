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
            "        <ECProperty propertyName = \"unknownExtendedType\" extendedTypeName=\"postalcode\" typeName=\"inst\" />"
            "        <ECArrayProperty propertyName=\"arrayExtendedType\" extendedTypeName=\"banana\" typeName=\"int\" />"
            "        <ECProperty propertyName=\"withoutExtendedType\" typeName=\"int\" />"
            "        <ECProperty propertyName=\"arrayWithoutExtendedType\" typeName=\"int\" />"
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

    /*---------------------------------------------------------------------------------**//**
     * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void VerifyExtendedTypeOnProperty
        (
        Utf8String expectedExtendedTypeName,
        Utf8String propertyName,
        ECClassP ecClass
        )
        {
        ECPropertyP ecProperty = ecClass->GetPropertyP(propertyName);
        EXPECT_FALSE(ecProperty == nullptr) << "Property " + propertyName + " not found on " + ecClass->GetName() + ".";

        Utf8String extendedTypeName;
        if (ecProperty->GetIsPrimitive())
            {
            PrimitiveECPropertyP primitiveProperty = ecProperty->GetAsPrimitivePropertyP();
            EXPECT_FALSE(primitiveProperty == nullptr) << "Property " + extendedTypeName + " is not an PrimitiveECProperty.";

            extendedTypeName = primitiveProperty->GetExtendedTypeName();
            }
        else if (ecProperty->GetIsPrimitiveArray())
            {
            ArrayECPropertyP arrayProperty = ecProperty->GetAsArrayPropertyP();
            EXPECT_FALSE(arrayProperty == nullptr) << "Property " + extendedTypeName + " is not an ArrayECProperty.";

            extendedTypeName = arrayProperty->GetExtendedTypeName();
            }
        EXPECT_EQ(expectedExtendedTypeName, extendedTypeName) << "Unexpected extended type name " + extendedTypeName +
            " expected: '" + expectedExtendedTypeName + "'";
        }

    template<typename PROPERTY_TYPE>
    static void VerifyExtendedTypeSet
        (
        PROPERTY_TYPE &ecProperty,
        Utf8CP extendedTypeName,
        bool useReset
        )
        {        
        Utf8String propertyName = ecProperty->GetName();
        if (ECObjectsStatus::Success != ecProperty->SetExtendedTypeName(extendedTypeName))
            {
            FAIL() << "Couldn't set the ExtendedTypeName on property " + propertyName;
            }

        EXPECT_TRUE(ecProperty->HasExtendedType()) << "Property " + propertyName + " was expected to return true on HasExtendedType.";

        if (!useReset)
            {
            if (ECObjectsStatus::Success != ecProperty->SetExtendedTypeName(nullptr))
                {
                FAIL() << "Couldn't set the ExtendedTypeName to nullptr on property " + propertyName;
                }
            }
        else
            {
            if (!ecProperty->RemoveExtendedTypeName())
                {
                FAIL() << "Couldn't remove the ExtendedTypeName on property " + propertyName;
                }
            }
        EXPECT_FALSE(ecProperty->HasExtendedType()) << "Property " + propertyName + " was expected to return false on HasExtendedType.";
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

    ECPropertyP property3 = classPtr->GetPropertyP("arrayExtendedType");
    EXPECT_FALSE(property3 == nullptr) << "Property arrayExtendedType not found on ClassA.";

    ECPropertyP property4 = classPtr->GetPropertyP("withoutExtendedType");
    EXPECT_FALSE(property4 == nullptr) << "Property withoutExtendedType not found on ClassA.";

    ECPropertyP property5 = classPtr->GetPropertyP("arrayWithoutExtendedType");
    EXPECT_FALSE(property5 == nullptr) << "Property arrayWithoutExtendedType not found on ClassA.";

    // Checks if the ECProperties are flagged correctly as extended type properties
    EXPECT_TRUE(property1->HasExtendedType())  << "Property knownExtendedType was expected to return true on HasExtendedType.";
    EXPECT_TRUE(property2->HasExtendedType())  << "Property arrayExtendedType was expected to return true on HasExtendedType.";
    EXPECT_TRUE(property3->HasExtendedType())  << "Property arrayExtendedType was expected to return true on HasExtendedType.";
    EXPECT_FALSE(property4->HasExtendedType()) << "Property withoutExtendedType was expected to return false on HasExtendedType.";
    EXPECT_FALSE(property4->HasExtendedType()) << "Property arrayWithoutExtendedType was expected to return false on HasExtendedType.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ExtendedTypeTests, WriteSchemaWithExtendedTypes)
    {
    ECSchemaPtr schemaPtr;
    ECSchema::CreateSchema(schemaPtr, "TestSchema", 1, 0);

    // Writing
    ECEntityClassP classPtr;
    schemaPtr.get()->CreateEntityClass(classPtr, "ClassA");
    PrimitiveECPropertyP propertyPtr;
    classPtr->CreatePrimitiveProperty(propertyPtr, "knownExtendedType");
    if (ECObjectsStatus::Success != propertyPtr->SetExtendedTypeName("color"))
        {
        FAIL() << "Couldn't set the ExtendedTypeName on property knownExtendedType";
        }

    ArrayECPropertyP arrayPropertyPtr;
    classPtr->CreateArrayProperty(arrayPropertyPtr, "arrayExtendedType", PrimitiveType::PRIMITIVETYPE_Integer);
    if (ECObjectsStatus::Success != arrayPropertyPtr->SetExtendedTypeName("banana"))
        {
        FAIL() << "Couldn't set the ExtendedTypeName on property arrayExtendedType";
        }

    WString resultString;
    schemaPtr->WriteToXmlString(resultString);

    // Verification (Reading)
    ECSchemaPtr resultSchema;
    ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(resultSchema, resultString.c_str(), *schemaContext);

    ECClassP resultClassPtr = schemaPtr.get()->GetClassP("ClassA");
    VerifyExtendedTypeOnProperty("color",  "knownExtendedType", resultClassPtr);
    VerifyExtendedTypeOnProperty("banana", "arrayExtendedType", resultClassPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ExtendedTypeTests, GetKnownExtendedType)
    {
    ECSchemaPtr schemaPtr = CreateTestSchema();
    ECClassP classPtr = schemaPtr->GetClassP("ClassA");

    VerifyExtendedTypeOnProperty("color", "knownExtendedType", classPtr);
    VerifyExtendedTypeOnProperty("banana", "arrayExtendedType", classPtr);
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

    PrimitiveECPropertyP colorPropertyPtr;
    classPtr->CreatePrimitiveProperty(colorPropertyPtr, "colorProperty", PrimitiveType::PRIMITIVETYPE_Integer);
    PrimitiveECPropertyP resetPropertyPtr;
    classPtr->CreatePrimitiveProperty(resetPropertyPtr, "resetProperty", PrimitiveType::PRIMITIVETYPE_Integer);
    ArrayECPropertyP arrayPropertyPtr;
    classPtr->CreateArrayProperty(arrayPropertyPtr, "arrayProperty", PrimitiveType::PRIMITIVETYPE_Integer);

    VerifyExtendedTypeSet(colorPropertyPtr, "color", false);
    VerifyExtendedTypeSet(resetPropertyPtr, "reset", true);
    VerifyExtendedTypeSet(arrayPropertyPtr, "array", false);
    }
END_BENTLEY_ECN_TEST_NAMESPACE
