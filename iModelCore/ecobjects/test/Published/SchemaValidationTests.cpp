/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/SchemaValidationTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

//---------------------------------------------------------------------------------------//
// Tests that strict schema rules are complied with
// @bsimethod                                    Andreas.Kurka                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------//
struct SchemaValidationTests : ECTestFixture
    {
    StandaloneECEnablerP    m_customAttributeEnabler;
    ECSchemaPtr             m_schema;

    ECClassCP           CreateClass(Utf8CP className, bool hasInstanceLabelAttribute, Utf8CP instanceLabelPropertyName)
        {
        ECEntityClassP ecClass;
        m_schema->CreateEntityClass(ecClass, className);

        PrimitiveECPropertyP prop;
        ecClass->CreatePrimitiveProperty(prop, instanceLabelPropertyName, PRIMITIVETYPE_String);

        if (hasInstanceLabelAttribute)
            {
            IECInstancePtr labelAttr = m_customAttributeEnabler->CreateInstance();
            ECValue v;
            if (instanceLabelPropertyName)
                v.SetUtf8CP(instanceLabelPropertyName, false);

            labelAttr->SetValue("PropertyName", v);
            ecClass->SetCustomAttribute(*labelAttr);
            }

        return ecClass;
        }

    static Utf8String    GetTestSchemaXMLString()
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
    };

//---------------------------------------------------------------------------------------//
// Tests that names (SchemaName, ClassName, PropertyName) do not differ by case.
// @bsimethod                                    Andreas.Kurka                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(SchemaValidationTests, TestNamesMayNotDifferByCase)
    {
    ECSchema::CreateSchema(m_schema, "MySchema", "ts", 1, 0, 0);

    ECEntityClassP ecClass;
    m_schema->CreateEntityClass(ecClass, "MyClass");

    ECClassP testClass = m_schema->GetClassP("myClass");
    EXPECT_TRUE(testClass != nullptr) << "Expected 'myClass' to return 'MyClass'";

    PrimitiveECPropertyP prop;
    ecClass->CreatePrimitiveProperty(prop, "MyProperty", PRIMITIVETYPE_String);

    ECPropertyP testProp = ecClass->GetPropertyP("myProperty");
    EXPECT_TRUE(testProp != nullptr) << "Expected 'myProperty' to return 'MyProperty'";
    }

//---------------------------------------------------------------------------------------//
// Tests that names (SchemaName, ClassName, PropertyName) are valid. For instance they   //
// may not differ by case                                                                //
// @bsimethod                                    Andreas.Kurka                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(SchemaValidationTests, TestValidSchemaNames)
    {
    ECSchemaPtr schemaA;
    ECSchemaPtr schemaB;

    ECObjectsStatus statusA = ECSchema::CreateSchema(schemaA, "somethingsomethingdarkside", "ts", 2, 0, 3);
    ECObjectsStatus statusB = ECSchema::CreateSchema(schemaB, "MySchema5&!(/$§!$", "ts", 2, 0, 3);

    EXPECT_TRUE(statusA == ECObjectsStatus::Success) << "Expected to return success when a valid schema name is given";
    EXPECT_TRUE(statusB == ECObjectsStatus::InvalidName) << "Expected InvalidName because the schema name contains invalid characters";
    ECEntityClassP ecClass;
    statusB = schemaA->CreateEntityClass(ecClass, "MyClass""§$%&/()");
    EXPECT_TRUE(statusB == ECObjectsStatus::InvalidName) << "Expected InvalidName because the class name contains invalid characters";

    PrimitiveECPropertyP prop;
    statusB = ecClass->CreatePrimitiveProperty(prop, "MyProperty!§$%&/()=", PRIMITIVETYPE_String);
    EXPECT_TRUE(statusB == ECObjectsStatus::InvalidName) << "Expected InvalidName because the property name contains invalid characters";
    }

//---------------------------------------------------------------------------------------//
// Tests that Aliases are valid. For instance they may not differ by case                //
// @bsimethod                                    Andreas.Kurka                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(SchemaValidationTests, TestAliases)
    {
    ECSchemaPtr schemaA;

    ECObjectsStatus status = ECSchema::CreateSchema(schemaA, "somethingsomethingdarkside", "testalias", 3, 0, 1);
    EXPECT_TRUE(status == ECObjectsStatus::Success) << "Expected success because a valid alias was given";

    status = schemaA->SetAlias("invalid&/(!$&(/§!$");
    EXPECT_TRUE(status == ECObjectsStatus::InvalidName) << "Expected InvalidName because the alias contains invalid characters";

    //status = schemaA->SetAlias("");
    //EXPECT_TRUE(status == ECObjectsStatus::InvalidName) << "Expected InvalidName because the alias is empty";

    }

END_BENTLEY_ECN_TEST_NAMESPACE


