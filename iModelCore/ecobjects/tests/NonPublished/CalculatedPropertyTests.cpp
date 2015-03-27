/*--------------------------------------------------------------------------------------+
|
|     $Source: tests/NonPublished/CalculatedPropertyTests.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

#include <ECObjects/ECInstance.h>
#include <ECObjects/StandaloneECInstance.h>
#include <ECObjects/ECValue.h>

#define EXPECT_SUCCESS(EXPR) EXPECT_TRUE(SUCCESS == EXPR)
using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

using namespace std;

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct CalculatedPropertyTests : ECTestFixture
    {
    enum { OPTION_UseLastValid = 1 << 0, OPTION_DefaultOnly = 1 << 1 };

    IECInstancePtr      CreateTestCase (WCharCP propName, WCharCP ecExpr, int options, WCharCP failureValue, WCharCP parserRegex = L"Unused");
    template <typename T>
    void SetValue (IECInstanceR instance, WCharCP accessor, T const& val, uint32_t arrayIndex = -1)
        {
        ECValue v (val);
        if (-1 == arrayIndex)
            instance.SetValue (accessor, v);
        else
            instance.SetValue (accessor, v, arrayIndex);
        }
    void SetNullValue (IECInstanceR instance, WCharCP accessor, uint32_t arrayIndex = -1)
        {
        ECValue v;
        if (-1 == arrayIndex)
            instance.SetValue (accessor, v);
        else
            instance.SetValue (accessor, v, arrayIndex);
        }
    template <typename T>
    void Test (IECInstanceCR instance, WCharCP propName, T const& expectedVal)
        {
        ECValue actualVal;
        EXPECT_SUCCESS (instance.GetValue (actualVal, propName));
        EXPECT_TRUE (actualVal.Equals (ECValue (expectedVal))) << "Expect: " << expectedVal << " Actual: " << actualVal.ToString().c_str();
        }

    void TestNull (IECInstanceCR instance, WCharCP propName)
        {
        ECValue actualVal;
        ECValue nullValue;
        EXPECT_SUCCESS (instance.GetValue (actualVal, propName));
        EXPECT_TRUE (actualVal.Equals (nullValue)) << "Expect: " << nullValue.ToString().c_str() << " Actual: " << actualVal.ToString().c_str();
        }

    struct ExpectedValue
        {
        WCharCP     name;
        ECValue     v;

        template<typename T> ExpectedValue (WCharCP n, T const& val) : name(n), v(val) { }
        };

    struct ExpectedValueList
        {
        bvector<ExpectedValue>  values;

        ExpectedValueList() { }
        template<typename T>
        ExpectedValueList (WCharCP n, T const& v) { values.push_back (ExpectedValue (n, v)); }
        template<typename T, typename U>
        ExpectedValueList (WCharCP n1, T const& v1, WCharCP n2, U const& v2)
            {
            values.push_back (ExpectedValue (n1, v1));
            values.push_back (ExpectedValue (n2, v2));
            }
        };

    void TestUpdate (IECInstanceR instance, WCharCP newValue, ExpectedValueList const& dependentValues)
        {
        static const WCharCP propname = L"S";
        ECValue v (newValue);
        EXPECT_SUCCESS (instance.SetValue (propname, v));

        for (ExpectedValue const& exp: dependentValues.values)
            {
            ECValueAccessor accessor;
            EXPECT_SUCCESS (ECValueAccessor::PopulateValueAccessor (accessor, instance, exp.name));
            ECValue actualVal;
            EXPECT_SUCCESS (instance.GetValueUsingAccessor (actualVal, accessor));
            EXPECT_TRUE (actualVal.Equals (exp.v)) << "Expect " << exp.v.ToString().c_str() << " Actual " << actualVal.ToString().c_str();
            }
        }

	static WString    GetTestSchemaXMLString ()
    {
    wchar_t fmt[] = L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                    L"<ECSchema schemaName=\"RelationshipTesting\" nameSpacePrefix=\"test\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                    L"    <ECClass typeName=\"ClassA\" displayLabel=\"Class A\" isDomainClass=\"True\">"
                    L"        <ECProperty propertyName=\"p\" typeName=\"int\" />"
                    L"    </ECClass>"
                    L"    <ECClass typeName=\"ClassB\" displayLabel=\"Class B\" isDomainClass=\"True\">"
                    L"        <ECProperty propertyName=\"p\" typeName=\"int\" />"
                    L"        <ECProperty propertyName=\"b\" typeName=\"int\" />"
                    L"    </ECClass>"
                    L"    <ECRelationshipClass typeName=\"ALikesB\" displayLabel=\"A likes B\" strength=\"referencing\">"
                    L"        <Source cardinality=\"(1,1)\" roleLabel=\"likes\" polymorphic=\"False\">"
                    L"            <Class class=\"ClassA\" />"
                    L"        </Source>"
                    L"        <Target cardinality=\"(1,1)\" roleLabel=\"is liked by\" polymorphic=\"True\">"
                    L"            <Class class=\"ClassB\" />"
                    L"        </Target>"
                    L"        <ECProperty propertyName=\"p\" typeName=\"int\" />"
                    L"        <ECProperty propertyName=\"Name\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"SourceOrderId\" typeName=\"int\" />"
                    L"        <ECProperty propertyName=\"TargetOrderId\" typeName=\"int\" />"
                    L"    </ECRelationshipClass>"
                    L"</ECSchema>";

    return fmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad.Zaighum                  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static ECSchemaP       CreateTestSchema (ECSchemaCacheR schemaOwner)
    {
    WString schemaXMLString = GetTestSchemaXMLString ();

   // EXPECT_EQ (S_OK, CoInitialize(NULL));  

    ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;        
    EXPECT_EQ (SUCCESS, ECSchema::ReadFromXmlString (schema, schemaXMLString.c_str(), *schemaContext));  

  //  CoUninitialize();
    return schema.get();
    }

    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr CalculatedPropertyTests::CreateTestCase (WCharCP propName, WCharCP ecExpr, int options, WCharCP failureValue, WCharCP parserRegex)
    {
    // We need to generate a new schema for each test case because we will apply different custom attributes to different properties
    static int32_t s_schemaNumber = 0;
    
    ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext();
    SearchPathSchemaFileLocaterPtr schemaLocater;
    bvector<WString> searchPaths;
    searchPaths.push_back (ECTestFixture::GetTestDataPath(L""));
    schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(searchPaths);
    schemaContext->AddSchemaLocater (*schemaLocater);

    SchemaKey schemaKey (L"Bentley_Standard_CustomAttributes", 1, 5);
    ECSchemaPtr customAttrSchema = schemaContext->LocateSchema (schemaKey, SCHEMAMATCHTYPE_Latest);
    EXPECT_TRUE (customAttrSchema.IsValid());

    // Create the schema
    static ECSchemaPtr schema;
    WString schemaName;
    schemaName.Sprintf (L"TestSchema_%d", s_schemaNumber++);
    EXPECT_EQ (SUCCESS, ECSchema::CreateSchema (schema, schemaName, 1, 0));  
    EXPECT_EQ (ECOBJECTS_STATUS_Success, schema->AddReferencedSchema (*customAttrSchema, L"besc"));

    ECClassP ecClass = NULL;
    EXPECT_EQ (ECOBJECTS_STATUS_Success, schema->CreateClass (ecClass, L"TestClass"));
    PrimitiveECPropertyP ecProp = NULL;
    ArrayECPropertyP arrayProp = NULL;
    ecClass->CreatePrimitiveProperty (ecProp, L"S", PRIMITIVETYPE_String);
    ecClass->CreatePrimitiveProperty (ecProp, L"S1", PRIMITIVETYPE_String);
    ecClass->CreatePrimitiveProperty (ecProp, L"S2", PRIMITIVETYPE_String);
    ecClass->CreatePrimitiveProperty (ecProp, L"I", PRIMITIVETYPE_Integer);
    ecClass->CreatePrimitiveProperty (ecProp, L"I1", PRIMITIVETYPE_Integer);
    ecClass->CreatePrimitiveProperty (ecProp, L"I2", PRIMITIVETYPE_Integer);
    ecClass->CreatePrimitiveProperty (ecProp, L"D", PRIMITIVETYPE_Double);
    ecClass->CreatePrimitiveProperty (ecProp, L"D1", PRIMITIVETYPE_Double);
    ecClass->CreatePrimitiveProperty (ecProp, L"D2", PRIMITIVETYPE_Double);
    ecClass->CreatePrimitiveProperty (ecProp, L"B", PRIMITIVETYPE_Boolean);
    ecClass->CreateArrayProperty (arrayProp, L"A", PRIMITIVETYPE_Integer);

    // Apply the CalculatedECPropertySpecification
    ecProp = ecClass->GetPropertyP (propName)->GetAsPrimitivePropertyP();

    ECClassCP calcSpecClass = customAttrSchema->GetClassCP (L"CalculatedECPropertySpecification");
    IECInstancePtr calcSpecAttr = calcSpecClass->GetDefaultStandaloneEnabler()->CreateInstance();

    ECValue v;
    v.SetString (ecExpr);                                   calcSpecAttr->SetValue (L"ECExpression", v);
    if (NULL != failureValue)
        {
        v.SetString (failureValue);
        calcSpecAttr->SetValue (L"FailureValue", v);
        }
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
    {
        DISABLE_ASSERTS
    // Test (*instance, L"S", L"ERROR");      // this.I and this.D are null. Test fails as expected, but also asserts, so commented out
    }

    SetValue (*instance, L"I", 5);
    SetValue (*instance, L"D", 1.234);
    Test (*instance, L"S", L"6.234");

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
    // Change the constituent properties and confirm the calculated property re-evaluates
    SetValue (*instance, L"D", 4.7);
    Test (*instance, L"I2", 8);

    instance = CreateTestCase (L"D", L"this.I & this.S", 0, L"-999");
    SetValue (*instance, L"I", 5);
    SetValue (*instance, L"S", L".4");
    Test (*instance, L"D", 5.4);
    SetValue (*instance, L"S", L"string");
    Test (*instance, L"S", L"string");
    Test (*instance, L"D", 5.0); // concatenates into a string of "5string" which equals 5 as a double

    instance = CreateTestCase (L"S", L"this.S1 & this.S2", 0, L"Error calculating value");
    SetValue (*instance, L"S1", L"S1");
    Test (*instance, L"S", L"Error calculating value");
    SetValue (*instance, L"S2", L"S2");
    Test (*instance, L"S", L"S1S2");
    SetNullValue (*instance, L"S2");
    Test (*instance, L"S", L"Error calculating value");

    // Conditionals
    instance = CreateTestCase (L"S", L"\"abs(I-D) == \" & IIf(this.I < this.D, this.D - this.I, this.I - this.D)", 0, L"ERROR");
    SetValue (*instance, L"I", 10);
    SetValue (*instance, L"D", 12.5);
    Test (*instance, L"S", L"abs(I-D) == 2.500000");
    SetValue (*instance, L"D", 5.25);
    Test (*instance, L"S", L"abs(I-D) == 4.750000");
    
    // Array properties
    instance = CreateTestCase (L"S", L"this.A[0] * this.A[1]", 0, L"ERROR");
    instance->AddArrayElements (L"A", 2);
    SetValue (*instance, L"A", 5, 0);
    SetValue (*instance, L"A", 6, 1);
    Test (*instance, L"S", L"30");

    // Null comparisons
    instance = CreateTestCase (L"B", L"this.S1 = Null", 0, L"False");
    SetNullValue(*instance, L"S1");
    Test (*instance, L"B", true);
    SetValue (*instance, L"S1", L"no longer null");
    Test (*instance, L"B", false);

    instance = CreateTestCase (L"B", L"this.S1 <> Null", 0, L"False");
    SetNullValue (*instance, L"S1");
    Test (*instance, L"B", false);
    SetValue (*instance, L"S1", L"no longer null");
    Test (*instance, L"B", true);
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

    instance = CreateTestCase (L"I", L"this.S1", 0, NULL);
    SetValue (*instance, L"S", L"not a number");
    {
        DISABLE_ASSERTS
        TestNull (*instance, L"I");
    }

    // If no last valid value and evaluation fails we should get back the failure value
    instance = CreateTestCase (L"I", L"this.NonexistentProperty", OPTION_UseLastValid, L"-999");
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CalculatedPropertyTests, SetValue)
    {
    // Two properties whose values are always identical
    IECInstancePtr instance = CreateTestCase (L"S", L"this.S1", 0, L"ERROR", L"(?<S1>.*)");
    TestUpdate (*instance, L"new value", ExpectedValueList (L"S1", L"new value"));

    instance = CreateTestCase (L"S", L"\"I == \" & this.I", 0, L"ERROR", L"I == (?<I>.+)");
    TestUpdate (*instance, L"I == 1234", ExpectedValueList (L"I", 1234));

    // Multiple properties
    instance = CreateTestCase (L"S", L"this.I & \", \" & this.I2", 0, L"ERROR", L"(?<I>-?\\d+), (?<D>-?\\d*\\.?\\d+)");
    TestUpdate (*instance, L"1, 2.5", ExpectedValueList (L"I", 1, L"D", 2.5));
    TestUpdate (*instance, L"-5, -.75", ExpectedValueList (L"I", -5, L"D", -0.75));
    TestUpdate (*instance, L"0, 1", ExpectedValueList (L"I", 0, L"D", 1.0));

    // Array properties. The value is always positive
    instance = CreateTestCase (L"S", L"this.A[0]", 0, L"ERROR", L"-?(?<A[0]>\\d+)");
    instance->AddArrayElements (L"A", 1);
    TestUpdate (*instance, L"555", ExpectedValueList (L"A[0]", 555));
    TestUpdate (*instance, L"-555", ExpectedValueList (L"A[0]", 555));

    // Uncaptured groups
    instance = CreateTestCase (L"S", L"\"prefix \" & this.I", 0, L"ERROR", L"(?:[^\\-\\d])+(?<I>-?\\d+)");
    TestUpdate (*instance, L"uncaptured -321", ExpectedValueList (L"I", -321));

    // Nested capture groups
    // We map the calculated property to properties I, I2, and D
    // such that, given an input string consisting of 2 or more digits:
    //  I = input[0]
    //  I2 = input[1..end]
    //  D = input
    // Totally contrived and it seems doubtful anyone would use nested capture groups with calculated properties, but it is an option so we test it
    WCharCP nestedCapture = L"(?<D>(?<I>\\d)(?<I2>\\d+))";
    instance = CreateTestCase (L"S", L"this.D", 0, L"ERROR", nestedCapture);
    ExpectedValueList valueList (L"D", 1234.0, L"I", 1);
    valueList.values.push_back (ExpectedValue (L"I2", 234));
    TestUpdate (*instance, L"1234", valueList);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CalculatedPropertyTests, SerializeAndDeserializeInstanceWithCalculatedProperties)
    {
    IECInstancePtr instance = CreateTestCase(L"S", L"this.S1 & \", \" & this.S2", 0, L"ERROR", L"(?<S1>.*), (?<S2>.*)");
    SetValue(*instance, L"S1", L"string1");
    SetValue(*instance, L"S2", L"string2");
    Test(*instance, L"S", L"string1, string2"); 

    WString ecInstanceXml;

    InstanceWriteStatus status2 = instance->WriteToXmlString(ecInstanceXml, true, false);
    EXPECT_EQ(INSTANCE_WRITE_STATUS_Success, status2);

    IECInstancePtr deserializedInstance;
    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext (instance->GetClass().GetSchema());

    InstanceReadStatus status3 = IECInstance::ReadFromXmlString(deserializedInstance, ecInstanceXml.c_str(), *instanceContext);
    EXPECT_EQ (INSTANCE_READ_STATUS_Success, status3); 


    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CalculatedPropertyTests, SerializeAndDeserializeInstanceWithFailedCalculatedProperties)
    {
    IECInstancePtr instance = CreateTestCase(L"S", L"this.S1 & \", \" & this.S2", 0, L"ERROR", L"(?<S1>.*), (?<S2>.*)");
    Test(*instance, L"S", L"ERROR"); 

    WString ecInstanceXml;

    InstanceWriteStatus status2 = instance->WriteToXmlString(ecInstanceXml, true, false);
    EXPECT_EQ(INSTANCE_WRITE_STATUS_Success, status2);

    IECInstancePtr deserializedInstance;
    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext (instance->GetClass().GetSchema());

    InstanceReadStatus status3 = IECInstance::ReadFromXmlString(deserializedInstance, ecInstanceXml.c_str(), *instanceContext);
    EXPECT_EQ (INSTANCE_READ_STATUS_Success, status3); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CalculatedPropertyTests, TestFailureValuesWithIntsAndDoubles)
    {
    IECInstancePtr instance = CreateTestCase(L"I", L"this.S1", 0, NULL, NULL);
        {
        DISABLE_ASSERTS
        TestNull (*instance, L"I");
        }
    instance = CreateTestCase(L"I", L"this.S1", 0, L"-1");
    Test (*instance, L"I", -1);

    instance = CreateTestCase(L"I", L"this.S1", 0, L"<Could not be calculated>");
    TestNull (*instance, L"I");

    instance = CreateTestCase(L"I", L"this.I1 + this.I2", 0, L"<Could not be calculated>");
    SetValue(*instance, L"I1", 3);
        {
        DISABLE_ASSERTS
        TestNull (*instance, L"I");
        }
    instance = CreateTestCase(L"I", L"this.I1 + this.I2", 0, NULL, NULL);
    SetValue(*instance, L"I1", 3);
    TestNull (*instance, L"I");

    instance = CreateTestCase(L"I", L"this.I1 / this.I2", 0, NULL, NULL);
    SetValue(*instance, L"I1", 3);
    SetValue(*instance, L"I2", 0);
    TestNull (*instance, L"I");

    instance = CreateTestCase(L"I", L"this.I1 / this.I2", 0, L"-1");
    SetValue(*instance, L"I1", 3);
    SetValue(*instance, L"I2", 0);
    Test (*instance, L"I", -1);

    instance = CreateTestCase(L"D", L"this.D1 + this.D2", 0, NULL, NULL);
    SetValue(*instance, L"D1", 3.7);
    TestNull (*instance, L"D");

    instance = CreateTestCase(L"D", L"this.D1 / this.D2", 0, NULL, NULL);
    SetValue(*instance, L"D1", 3.7);
    SetValue(*instance, L"D2", 0);
    TestNull (*instance, L"D");

    instance = CreateTestCase(L"D", L"this.D1 / this.D2", 0, L"-1");
    SetValue(*instance, L"D1", 3.7);
    SetValue(*instance, L"D2", 0);
    Test (*instance, L"D", -1.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad.Zaighum                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CalculatedPropertyTests, ExpectSuccessWhenCopyFailedCalculatedPropertyThatHasARegex)
    {
		IECInstancePtr instance = CreateTestCase(L"S",L"S1 - S2",0, L"<Could not be calculated>", L"^(?<S1>.*)\\-(?<S2>.*)");
		IECInstancePtr instance2=StandaloneECInstance::Duplicate(*instance);

		Test(*instance2,L"S",L"<Could not be calculated>");
		Test(*instance,L"S",L"<Could not be calculated>");
		TestNull (*instance, L"S1");
		TestNull (*instance, L"S2");
		TestNull (*instance2, L"S1");
		TestNull (*instance2, L"S2");
	}



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad.Zaighum                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CalculatedPropertyTests, EvaluationAgainstANewInstanceUsesItsNewValues)
{
	IECInstancePtr instance = CreateTestCase(L"S",L"this.S1",0, NULL, NULL);

	SetValue(*instance, L"S1", L"initial");
	Test (*instance, L"S", L"initial");

	SetValue(*instance, L"S1", L"different");
	Test (*instance, L"S", L"different");
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad.Zaighum                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CalculatedPropertyTests, SettingACalculatedPropertyUpdatesThePropertiesFromWhichItWasCalculate)
{
	IECInstancePtr instance = CreateTestCase(L"S",L"S1 - S2",0, L"<Could not be calculated>",L"^(?<S1>.*)-(?<S2>.*)");
	SetValue(*instance, L"S", L"1-2");
	Test(*instance, L"S1", L"1");
	Test(*instance, L"S2", L"2");


	SetValue(*instance, L"S", L"3-4");
	Test(*instance, L"S1", L"3");
	Test(*instance, L"S2", L"4");
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad.Zaighum                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CalculatedPropertyTests, CalculatedPropertiesWithNoParserRegExAreReadOnly)
{
	IECInstancePtr instance = CreateTestCase(L"S",L"S1",1, L"<Could not be calculated>",NULL);
	ASSERT_EQ (true,instance->IsPropertyReadOnly(L"S"));
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad.Zaighum                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CalculatedPropertyTests, UseLastValidValueInsteadOfFailure)
{	
	IECInstancePtr instance = CreateTestCase(L"S",L"this.S1 & \"-\" & this.S2",OPTION_UseLastValid , L"<Could not be calculated>");
	Test(*instance, L"S", L"<Could not be calculated>");
	
	SetValue(*instance, L"S1", L"3");
	SetValue(*instance, L"S2", L"4");
	Test(*instance, L"S", L"3-4");

	SetNullValue(*instance, L"S1");
	TestNull(*instance, L"S1");

	Test(*instance, L"S", L"3-4");

	SetNullValue(*instance, L"S2");
	TestNull(*instance, L"S2");
	Test(*instance, L"S", L"3-4");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CalculatedPropertyTests, ConvertNamedCaptureGroupsToUnnamed)
    {
    // According to email from graphite team, an imodel containing specification with following parser regex:
    //  "^(?<S1>[ a-z\\d/]+[\"]?)[\\s]*X[\\s]*(?<S2>[ a-z\\d/]+[\"]?)|[\\w]*"
    // causes exception in construction of std::wregex from ParserRegex::Create()
    // We're mainly testing to make sure we don't get an exception in constructing the regex for the CalculatedPropertySpecfication
    IECInstancePtr instance = CreateTestCase (L"S", L"this.S1 & \" X \" & this.S2", 0, L"FAILED", L"^(?<S1>[ a-z\\\\d/]+[\\\"]?)[\\\\s]*X[\\\\s]*(?<S2>[ a-z\\\\d/]+[\\\"]?)|[\\\\w]*");
    SetValue (*instance, L"S1", L"a");
    SetValue (*instance, L"S2", L"b");
    Test (*instance, L"S", L"a X b");


    // Note that the parser regex doesn't appear to match what the author thinks it should match...he seems to be confused about escape characters.
    SetValue (*instance, L"S", L"xXy");
    Test (*instance, L"S1", L"x");
    Test (*instance, L"S2", L"y");
    }

TEST_F(CalculatedPropertyTests, ConvertNamedCaptureGroupsToUnnamedFromFile)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath( L"pidSnippet.01.02.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ (SCHEMA_READ_STATUS_Success, status);

    ECClassP ecClass = schema->GetClassP(L"BASE_REDUCER");
    IECInstancePtr instance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();

    SetValue(*instance, L"LEFT_TEXT", L"left");
    SetValue(*instance, L"RIGHT_TEXT", L"right");
    Test(*instance, L"DISPLAY_TEXT", L"left X right");

    SetValue(*instance, L"DISPLAY_TEXT", L"leftXright");
    Test(*instance, L"LEFT_TEXT", L"left");
    Test(*instance, L"RIGHT_TEXT", L"right");
    }

END_BENTLEY_ECN_TEST_NAMESPACE
