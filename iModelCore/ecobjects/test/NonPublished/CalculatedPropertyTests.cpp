/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/CalculatedPropertyTests.cpp $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include "TestFixture.h"

#include <ECObjects\ECInstance.h>
#include <ECObjects\StandaloneECInstance.h>
#include <ECObjects\ECValue.h>

#define EXPECT_SUCCESS(EXPR) EXPECT_TRUE(SUCCESS == EXPR)

BEGIN_BENTLEY_EC_NAMESPACE

using namespace std;

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct CalculatedPropertyTests : ECTestFixture
    {
    enum { OPTION_UseLastValid = 1 << 0, OPTION_DefaultOnly = 1 << 1 };

    IECInstancePtr      CreateTestCase (WCharCP propName, WCharCP ecExpr, int options, WCharCP failureValue, WCharCP parserRegex = L"Unused");
    template <typename T>
    void SetValue (IECInstanceR instance, WCharCP accessor, T const& val)
        {
        ECValue v (val);
        instance.SetValue (accessor, v);
        }
    template <typename T>
    void Test (IECInstanceCR instance, WCharCP propName, T const& expectedVal)
        {
        ECValue actualVal;
        EXPECT_SUCCESS (instance.GetValue (actualVal, propName));
        EXPECT_TRUE (actualVal.Equals (ECValue (expectedVal))) << "Expect: " << expectedVal << " Actual: " << actualVal.ToString().c_str();
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr CalculatedPropertyTests::CreateTestCase (WCharCP propName, WCharCP ecExpr, int options, WCharCP failureValue, WCharCP parserRegex)
    {
    // We need to generate a new schema for each test case because we will apply different custom attributes to different properties
    static Int32 s_schemaNumber = 0;
    
    ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext();
    SchemaKey schemaKey (L"Bentley_Standard_CustomAttributes", 1, 5);
    ECSchemaPtr customAttrSchema = schemaContext->LocateSchema (schemaKey, SCHEMAMATCHTYPE_Latest);
    EXPECT_TRUE (customAttrSchema.IsValid());

    // Create the schema
    static ECSchemaPtr schema;
    WString schemaName;
    schemaName.Sprintf (L"TestSchema_%d", s_schemaNumber++);
    EXPECT_EQ (SUCCESS, ECSchema::CreateSchema (schema, schemaName, 1, 0));  
    EXPECT_EQ (ECOBJECTS_STATUS_Success, schema->AddReferencedSchema (*customAttrSchema, L"besc"));

    ECClassP ecClass;
    EXPECT_EQ (ECOBJECTS_STATUS_Success, schema->CreateClass (ecClass, L"TestClass"));
    PrimitiveECPropertyP ecProp;
    ecClass->CreatePrimitiveProperty (ecProp, L"S", PRIMITIVETYPE_String);
    ecClass->CreatePrimitiveProperty (ecProp, L"I", PRIMITIVETYPE_Integer);
    ecClass->CreatePrimitiveProperty (ecProp, L"I2", PRIMITIVETYPE_Integer);
    ecClass->CreatePrimitiveProperty (ecProp, L"D", PRIMITIVETYPE_Double);
    ecClass->CreatePrimitiveProperty (ecProp, L"D2", PRIMITIVETYPE_Double);

    // Apply the CalculatedECPropertySpecification
    ecProp = ecClass->GetPropertyP (propName)->GetAsPrimitiveProperty();

    ECClassP calcSpecClass = customAttrSchema->GetClassP (L"CalculatedECPropertySpecification");
    IECInstancePtr calcSpecAttr = calcSpecClass->GetDefaultStandaloneEnabler()->CreateInstance();

    ECValue v;
    v.SetString (ecExpr);                                   calcSpecAttr->SetValue (L"ECExpression", v);
    v.SetString (failureValue);                             calcSpecAttr->SetValue (L"FailureValue", v);
    v.SetBoolean (0 != (options & OPTION_DefaultOnly));     calcSpecAttr->SetValue (L"IsDefaultValueOnly", v);
    v.SetBoolean (0 != (options & OPTION_UseLastValid));    calcSpecAttr->SetValue (L"UseLastValidValueOnFailure", v);
    v.SetString (parserRegex);                              calcSpecAttr->SetValue (L"ParserRegularExpression", v);

    EXPECT_EQ (ECOBJECTS_STATUS_Success, ecProp->SetCustomAttribute (*calcSpecAttr));

    // Create an instance to test against
    return ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CalculatedPropertyTests, BasicExpressions)
    {
    // Literals
    IECInstancePtr instance = CreateTestCase (L"I", L"2 + 3", 0, L"-999");
    Test (*instance, L"I", 5);

    instance = CreateTestCase (L"S", L"\"Dogs \" & \"&\" & \" Cats\"", 0, L"ERROR");
    Test (*instance, L"S", L"Dogs & Cats");

    // Properties
    instance = CreateTestCase (L"S", L"this.I + this.D", 0, L"ERROR");
    // Test (*instance, L"S", L"ERROR");      // this.I and this.D are null. Test fails as expected, but also asserts, so commented out
    SetValue (*instance, L"I", 5);
    SetValue (*instance, L"D", 1.234);
    Test (*instance, L"S", L"6.234000");

#ifdef ECEXPRESSIONS_SUPPORTS_PROMOTING_STRING_TO_NUMERIC
    instance = CreateTestCase (L"I", L"this.D + this.S", 0, L"-999");
    SetValue (*instance, L"D", 2.5);
    SetValue (*instance, L"S", L"3.15");
    Test (*instance, L"I", 6);      // 2.5 + 3.15 rounds up to 6
#endif

    instance = CreateTestCase (L"I2", L"this.D + this.D2", 0, L"-999");
    SetValue (*instance, L"D", 2.5);
    SetValue (*instance, L"D2", 3.15);
    Test (*instance, L"I2", 6);     // 2.5 + 3.15 rounds up to 6

    instance = CreateTestCase (L"D", L"this.I & this.S", 0, L"-999");
    SetValue (*instance, L"I", 5);
    SetValue (*instance, L"S", L".4");
    Test (*instance, L"D", 5.4);

    // Conditionals
    instance = CreateTestCase (L"S", L"\"abs(I-D) == \" & IIf(this.I < this.D, this.D - this.I, this.I - this.D)", 0, L"ERROR");
    SetValue (*instance, L"I", 10);
    SetValue (*instance, L"D", 12.5);
    Test (*instance, L"S", L"abs(I-D) == 2.500000");
    SetValue (*instance, L"D", 5.25);
    Test (*instance, L"S", L"abs(I-D) == 4.750000");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CalculatedPropertyTests, DefaultValueOnly)
    {
    // The first time we get the value, it should match the expression. Subsequently we should be able to set it to whatever we want
    IECInstancePtr instance = CreateTestCase (L"S", L"5 * 10", OPTION_DefaultOnly, L"ERROR");
    Test (*instance, L"S", L"50");
    SetValue (*instance, L"S", L"Kangaroos");
    Test (*instance, L"S", L"Kangaroos");

    // If we set it before getting it, should never see calculated value
    instance = CreateTestCase (L"S", L"5 * 10", OPTION_DefaultOnly, L"ERROR");
    SetValue (*instance, L"S", L"Giraffes");
    Test (*instance, L"S", L"Giraffes");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CalculatedPropertyTests, FailureValue)
    {
    // If an error occurs, we should get the failure value back
    IECInstancePtr instance = CreateTestCase (L"S", L"this.NonexistentProperty", 0, L"ERROR");
    Test (*instance, L"S", L"ERROR");

    // If no last valid value and evaluation fails we should get back the failure value
    instance = CreateTestCase (L"I", L"this.NonexistentProperty", 0, L"-999");
    Test (*instance, L"I", -999);

    // If an error occurs, we should get back the last valid value
    instance = CreateTestCase (L"I", L"this.S", OPTION_UseLastValid, L"-999");
    SetValue (*instance, L"S", L"12345");
    Test (*instance, L"I", 12345);
    SetValue (*instance, L"S", L"not a number");
    Test (*instance, L"I", 12345);
    SetValue (*instance, L"S", L"821");
    Test (*instance, L"I", 821);
    }

END_BENTLEY_EC_NAMESPACE

