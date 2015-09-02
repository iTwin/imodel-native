/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/CalculatedPropertyTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

    IECInstancePtr      CreateTestCase (Utf8CP propName, Utf8CP ecExpr, int options, Utf8CP failureValue, Utf8CP parserRegex = "Unused");
    template <typename T>
    void SetValue (IECInstanceR instance, Utf8CP accessor, T const& val, uint32_t arrayIndex = -1)
        {
        ECValue v (val);
        if (-1 == arrayIndex)
            instance.SetValue (accessor, v);
        else
            instance.SetValue (accessor, v, arrayIndex);
        }

    void SetNullValue (IECInstanceR instance, Utf8CP accessor, uint32_t arrayIndex = -1)
        {
        ECValue v;
        if (-1 == arrayIndex)
            instance.SetValue (accessor, v);
        else
            instance.SetValue (accessor, v, arrayIndex);
        }

    template <typename T>
    void Test (IECInstanceCR instance, Utf8CP propName, T const& expectedVal)
        {
        ECValue actualVal;
        EXPECT_SUCCESS (instance.GetValue (actualVal, propName));
        EXPECT_TRUE (actualVal.Equals (ECValue (expectedVal))) << "Expect: " << expectedVal << " Actual: " << actualVal.ToString ().c_str ();
        }

    void TestNull (IECInstanceCR instance, Utf8CP propName)
        {
        ECValue actualVal;
        ECValue nullValue;
        EXPECT_SUCCESS (instance.GetValue (actualVal, propName));
        EXPECT_TRUE (actualVal.Equals (nullValue)) << "Expect: " << nullValue.ToString ().c_str () << " Actual: " << actualVal.ToString ().c_str ();
        }

    struct ExpectedValue
        {
        Utf8CP     name;
        ECValue     v;

        template<typename T> ExpectedValue (Utf8CP n, T const& val) : name (n), v (val) {}
        };

    struct ExpectedValueList
        {
        bvector<ExpectedValue>  values;

        ExpectedValueList () {}
        template<typename T>
        ExpectedValueList (Utf8CP n, T const& v) { values.push_back (ExpectedValue (n, v)); }
        template<typename T, typename U>
        ExpectedValueList (Utf8CP n1, T const& v1, Utf8CP n2, U const& v2)
            {
            values.push_back (ExpectedValue (n1, v1));
            values.push_back (ExpectedValue (n2, v2));
            }
        };

    void TestUpdate (IECInstanceR instance, Utf8CP newValue, ExpectedValueList const& dependentValues)
        {
        static const Utf8CP propname = "S";
        ECValue v (newValue);
        EXPECT_SUCCESS (instance.SetValue (propname, v));

        for (ExpectedValue const& exp : dependentValues.values)
            {
            ECValueAccessor accessor;
            EXPECT_SUCCESS (ECValueAccessor::PopulateValueAccessor (accessor, instance, exp.name));
            ECValue actualVal;
            EXPECT_SUCCESS (instance.GetValueUsingAccessor (actualVal, accessor));
            EXPECT_TRUE (actualVal.Equals (exp.v)) << "Expect " << exp.v.ToString ().c_str () << " Actual " << actualVal.ToString ().c_str ();
            }
        }

    static Utf8String    GetTestSchemaXMLString ()
        {
        Utf8Char fmt[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<ECSchema schemaName=\"RelationshipTesting\" nameSpacePrefix=\"test\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
            "    <ECClass typeName=\"ClassA\" displayLabel=\"Class A\" isDomainClass=\"True\">"
            "        <ECProperty propertyName=\"p\" typeName=\"int\" />"
            "    </ECClass>"
            "    <ECClass typeName=\"ClassB\" displayLabel=\"Class B\" isDomainClass=\"True\">"
            "        <ECProperty propertyName=\"p\" typeName=\"int\" />"
            "        <ECProperty propertyName=\"b\" typeName=\"int\" />"
            "    </ECClass>"
            "    <ECRelationshipClass typeName=\"ALikesB\" displayLabel=\"A likes B\" strength=\"referencing\">"
            "        <Source cardinality=\"(1,1)\" roleLabel=\"likes\" polymorphic=\"False\">"
            "            <Class class=\"ClassA\" />"
            "        </Source>"
            "        <Target cardinality=\"(1,1)\" roleLabel=\"is liked by\" polymorphic=\"True\">"
            "            <Class class=\"ClassB\" />"
            "        </Target>"
            "        <ECProperty propertyName=\"p\" typeName=\"int\" />"
            "        <ECProperty propertyName=\"Name\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"SourceOrderId\" typeName=\"int\" />"
            "        <ECProperty propertyName=\"TargetOrderId\" typeName=\"int\" />"
            "    </ECRelationshipClass>"
            "</ECSchema>";

        return fmt;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Muhammad.Zaighum                  10/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    static ECSchemaP       CreateTestSchema (ECSchemaCacheR schemaOwner)
        {
        Utf8String schemaXMLString = GetTestSchemaXMLString ();
        ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext ();

        ECSchemaPtr schema;
        EXPECT_EQ (SUCCESS, ECSchema::ReadFromXmlString (schema, schemaXMLString.c_str (), *schemaContext));

        return schema.get ();
        }

    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr CalculatedPropertyTests::CreateTestCase (Utf8CP propName, Utf8CP ecExpr, int options, Utf8CP failureValue, Utf8CP parserRegex)
    {
    // We need to generate a new schema for each test case because we will apply different custom attributes to different properties
    static int32_t s_schemaNumber = 0;

    ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext ();
    SearchPathSchemaFileLocaterPtr schemaLocater;
    bvector<WString> searchPaths;
    searchPaths.push_back (ECTestFixture::GetTestDataPath (L""));
    schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater (searchPaths);
    schemaContext->AddSchemaLocater (*schemaLocater);

    SchemaKey schemaKey ("Bentley_Standard_CustomAttributes", 1, 5);
    ECSchemaPtr customAttrSchema = schemaContext->LocateSchema (schemaKey, SCHEMAMATCHTYPE_Latest);
    EXPECT_TRUE (customAttrSchema.IsValid ());

    // Create the schema
    static ECSchemaPtr schema;
    Utf8String schemaName;
    schemaName.Sprintf ("TestSchema_%d", s_schemaNumber++);
    EXPECT_EQ (SUCCESS, ECSchema::CreateSchema (schema, schemaName, 1, 0));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, schema->AddReferencedSchema (*customAttrSchema, "besc"));

    ECClassP ecClass = NULL;
    EXPECT_EQ (ECOBJECTS_STATUS_Success, schema->CreateClass (ecClass, "TestClass"));
    PrimitiveECPropertyP ecProp = NULL;
    ArrayECPropertyP arrayProp = NULL;
    ecClass->CreatePrimitiveProperty (ecProp, "S", PRIMITIVETYPE_String);
    ecClass->CreatePrimitiveProperty (ecProp, "S1", PRIMITIVETYPE_String);
    ecClass->CreatePrimitiveProperty (ecProp, "S2", PRIMITIVETYPE_String);
    ecClass->CreatePrimitiveProperty (ecProp, "I", PRIMITIVETYPE_Integer);
    ecClass->CreatePrimitiveProperty (ecProp, "I1", PRIMITIVETYPE_Integer);
    ecClass->CreatePrimitiveProperty (ecProp, "I2", PRIMITIVETYPE_Integer);
    ecClass->CreatePrimitiveProperty (ecProp, "D", PRIMITIVETYPE_Double);
    ecClass->CreatePrimitiveProperty (ecProp, "D1", PRIMITIVETYPE_Double);
    ecClass->CreatePrimitiveProperty (ecProp, "D2", PRIMITIVETYPE_Double);
    ecClass->CreatePrimitiveProperty (ecProp, "B", PRIMITIVETYPE_Boolean);
    ecClass->CreateArrayProperty (arrayProp, "A", PRIMITIVETYPE_Integer);

    // Apply the CalculatedECPropertySpecification
    ecProp = ecClass->GetPropertyP (propName)->GetAsPrimitivePropertyP ();

    ECClassCP calcSpecClass = customAttrSchema->GetClassCP ("CalculatedECPropertySpecification");
    IECInstancePtr calcSpecAttr = calcSpecClass->GetDefaultStandaloneEnabler ()->CreateInstance ();

    ECValue v;
    v.SetUtf8CP (ecExpr);                                   calcSpecAttr->SetValue ("ECExpression", v);
    if (NULL != failureValue)
        {
        v.SetUtf8CP (failureValue);
        calcSpecAttr->SetValue ("FailureValue", v);
        }
    v.SetBoolean (0 != (options & OPTION_DefaultOnly));     calcSpecAttr->SetValue ("IsDefaultValueOnly", v);
    v.SetBoolean (0 != (options & OPTION_UseLastValid));    calcSpecAttr->SetValue ("UseLastValidValueOnFailure", v);
    v.SetUtf8CP (parserRegex);                              calcSpecAttr->SetValue ("ParserRegularExpression", v);

    EXPECT_EQ (ECOBJECTS_STATUS_Success, ecProp->SetCustomAttribute (*calcSpecAttr));

    // Create an instance to test against
    return ecClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CalculatedPropertyTests, BasicExpressions)
    {
    // Literals
    IECInstancePtr instance = CreateTestCase ("I", "2 + 3", 0, "-999");
    Test (*instance, "I", 5);

    instance = CreateTestCase ("S", "\"Dogs \" & \"&\" & \" Cats\"", 0, "ERROR");
    Test (*instance, "S", "Dogs & Cats");

    // Properties
    instance = CreateTestCase ("S", "this.I + this.D", 0, "ERROR");
        {
        DISABLE_ASSERTS
        //Test (*instance, L"S", L"ERROR");       //this.I and this.D are null. Test fails as expected, but also asserts, so commented out
        }

    SetValue (*instance, "I", 5);
    SetValue (*instance, "D", 1.234);
    Test (*instance, "S", "6.234");

#ifdef ECEXPRESSIONS_SUPPORTS_PROMOTING_STRING_TO_NUMERIC
    instance = CreateTestCase (L"I", L"this.D + this.S", 0, L"-999");
    SetValue (*instance, L"D", 2.5);
    SetValue (*instance, L"S", L"3.15");
    Test (*instance, L"I", 6);      // 2.5 + 3.15 rounds up to 6
#endif

    instance = CreateTestCase ("I2", "this.D + this.D2", 0, "-999");
    SetValue (*instance, "D", 2.5);
    SetValue (*instance, "D2", 3.15);
    Test (*instance, "I2", 6);     // 2.5 + 3.15 rounds up to 6
    // Change the constituent properties and confirm the calculated property re-evaluates
    SetValue (*instance, "D", 4.7);
    Test (*instance, "I2", 8);

    instance = CreateTestCase ("D", "this.I & this.S", 0, "-999");
    SetValue (*instance, "I", 5);
    SetValue (*instance, "S", L".4");
    Test (*instance, "D", 5.4);
    SetValue (*instance, "S", "string");
    Test (*instance, "S", "string");
    Test (*instance, "D", 5.0); // concatenates into a string of "5string" which equals 5 as a double

    instance = CreateTestCase ("S", "this.S1 & this.S2", 0, "Error calculating value");
    SetValue (*instance, "S1", "S1");
    Test (*instance, "S", "Error calculating value");
    SetValue (*instance, "S2", "S2");
    Test (*instance, "S", "S1S2");
    SetNullValue (*instance, "S2");
    Test (*instance, "S", "Error calculating value");

    // Conditionals
    instance = CreateTestCase ("S", "\"abs(I-D) == \" & IIf(this.I < this.D, this.D - this.I, this.I - this.D)", 0, "ERROR");
    SetValue (*instance, "I", 10);
    SetValue (*instance, "D", 12.5);
    Test (*instance, "S", "abs(I-D) == 2.500000");
    SetValue (*instance, "D", 5.25);
    Test (*instance, "S", "abs(I-D) == 4.750000");

    // Array properties
    instance = CreateTestCase ("S", "this.A[0] * this.A[1]", 0, "ERROR");
    instance->AddArrayElements ("A", 2);
    SetValue (*instance, "A", 5, 0);
    SetValue (*instance, "A", 6, 1);
    Test (*instance, "S", "30");

    // Null comparisons
    instance = CreateTestCase ("B", "this.S1 = Null", 0, "False");
    SetNullValue (*instance, "S1");
    Test (*instance, "B", true);
    SetValue (*instance, "S1", "no longer null");
    Test (*instance, "B", false);

    instance = CreateTestCase ("B", "this.S1 <> Null", 0, "False");
    SetNullValue (*instance, "S1");
    Test (*instance, "B", false);
    SetValue (*instance, "S1", "no longer null");
    Test (*instance, "B", true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CalculatedPropertyTests, DefaultValueOnly)
    {
    // The first time we get the value, it should match the expression. Subsequently we should be able to set it to whatever we want
    IECInstancePtr instance = CreateTestCase ("S", "5 * 10", OPTION_DefaultOnly, "ERROR");
    Test (*instance, "S", "50");
    SetValue (*instance, "S", "Kangaroos");
    Test (*instance, "S", "Kangaroos");

    // If we set it before getting it, should never see calculated value
    instance = CreateTestCase ("S", "5 * 10", OPTION_DefaultOnly, "ERROR");
    SetValue (*instance, "S", "Giraffes");
    Test (*instance, "S", "Giraffes");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CalculatedPropertyTests, FailureValue)
    {
    // If an error occurs, we should get the failure value back
    IECInstancePtr instance = CreateTestCase ("S", "this.NonexistentProperty", 0, "ERROR");
    Test (*instance, "S", "ERROR");

    instance = CreateTestCase ("I", "this.S1", 0, NULL);
    SetValue (*instance, "S", "not a number");
        {
        DISABLE_ASSERTS
        TestNull (*instance, "I");
        }

    // If no last valid value and evaluation fails we should get back the failure value
    instance = CreateTestCase ("I", "this.NonexistentProperty", OPTION_UseLastValid, "-999");
    Test (*instance, "I", -999);

    // If an error occurs, we should get back the last valid value
    instance = CreateTestCase ("I", "this.S", OPTION_UseLastValid, "-999");
    SetValue (*instance, "S", "12345");
    Test (*instance, "I", 12345);
    SetValue (*instance, "S", "not a number");
    Test (*instance, "I", 12345);
    SetValue (*instance, "S", "821");
    Test (*instance, "I", 821);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CalculatedPropertyTests, SetValue)
    {
    // Two properties whose values are always identical
    IECInstancePtr instance = CreateTestCase ("S", "this.S1", 0, "ERROR", "(?<S1>.*)");
    TestUpdate (*instance, "new value", ExpectedValueList ("S1", "new value"));

    instance = CreateTestCase ("S", "\"I == \" & this.I", 0, "ERROR", "I == (?<I>.+)");
    TestUpdate (*instance, "I == 1234", ExpectedValueList ("I", 1234));

    // Multiple properties
    instance = CreateTestCase ("S", "this.I & \", \" & this.I2", 0, "ERROR", "(?<I>-?\\d+), (?<D>-?\\d*\\.?\\d+)");
    TestUpdate (*instance, "1, 2.5", ExpectedValueList ("I", 1, "D", 2.5));
    TestUpdate (*instance, "-5, -.75", ExpectedValueList ("I", -5, "D", -0.75));
    TestUpdate (*instance, "0, 1", ExpectedValueList ("I", 0, "D", 1.0));

    // Array properties. The value is always positive
    instance = CreateTestCase ("S", "this.A[0]", 0, "ERROR", "-?(?<A[0]>\\d+)");
    instance->AddArrayElements ("A", 1);
    TestUpdate (*instance, "555", ExpectedValueList ("A[0]", 555));
    TestUpdate (*instance, "-555", ExpectedValueList ("A[0]", 555));

    // Uncaptured groups
    instance = CreateTestCase ("S", "\"prefix \" & this.I", 0, "ERROR", "(?:[^\\-\\d])+(?<I>-?\\d+)");
    TestUpdate (*instance, "uncaptured -321", ExpectedValueList ("I", -321));

    // Nested capture groups
    // We map the calculated property to properties I, I2, and D
    // such that, given an input string consisting of 2 or more digits:
    //  I = input[0]
    //  I2 = input[1..end]
    //  D = input
    // Totally contrived and it seems doubtful anyone would use nested capture groups with calculated properties, but it is an option so we test it
    Utf8CP nestedCapture = "(?<D>(?<I>\\d)(?<I2>\\d+))";
    instance = CreateTestCase ("S", "this.D", 0, "ERROR", nestedCapture);
    ExpectedValueList valueList ("D", 1234.0, "I", 1);
    valueList.values.push_back (ExpectedValue ("I2", 234));
    TestUpdate (*instance, "1234", valueList);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CalculatedPropertyTests, SerializeAndDeserializeInstanceWithCalculatedProperties)
    {
    IECInstancePtr instance = CreateTestCase ("S", "this.S1 & \", \" & this.S2", 0, "ERROR", "(?<S1>.*), (?<S2>.*)");
    SetValue (*instance, "S1", "string1");
    SetValue (*instance, "S2", "string2");
    Test (*instance, "S", "string1, string2");

    Utf8String ecInstanceXml;

    InstanceWriteStatus status2 = instance->WriteToXmlString (ecInstanceXml, true, false);
    EXPECT_EQ (INSTANCE_WRITE_STATUS_Success, status2);

    IECInstancePtr deserializedInstance;
    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext (instance->GetClass ().GetSchema ());

    InstanceReadStatus status3 = IECInstance::ReadFromXmlString (deserializedInstance, ecInstanceXml.c_str (), *instanceContext);
    EXPECT_EQ (INSTANCE_READ_STATUS_Success, status3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CalculatedPropertyTests, SerializeAndDeserializeInstanceWithFailedCalculatedProperties)
    {
    IECInstancePtr instance = CreateTestCase ("S", "this.S1 & \", \" & this.S2", 0, "ERROR", "(?<S1>.*), (?<S2>.*)");
    Test (*instance, "S", "ERROR");

    Utf8String ecInstanceXml;

    InstanceWriteStatus status2 = instance->WriteToXmlString (ecInstanceXml, true, false);
    EXPECT_EQ (INSTANCE_WRITE_STATUS_Success, status2);

    IECInstancePtr deserializedInstance;
    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext (instance->GetClass ().GetSchema ());

    InstanceReadStatus status3 = IECInstance::ReadFromXmlString (deserializedInstance, ecInstanceXml.c_str (), *instanceContext);
    EXPECT_EQ (INSTANCE_READ_STATUS_Success, status3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CalculatedPropertyTests, TestFailureValuesWithIntsAndDoubles)
    {
    IECInstancePtr instance = CreateTestCase ("I", "this.S1", 0, NULL, NULL);
        {
        DISABLE_ASSERTS
        TestNull (*instance, "I");
        }
    instance = CreateTestCase ("I", "this.S1", 0, "-1");
    Test (*instance, "I", -1);

    instance = CreateTestCase ("I", "this.S1", 0, "<Could not be calculated>");
    TestNull (*instance, "I");

    instance = CreateTestCase ("I", "this.I1 + this.I2", 0, "<Could not be calculated>");
    SetValue (*instance, "I1", 3);
        {
        DISABLE_ASSERTS
        TestNull (*instance, "I");
        }
    instance = CreateTestCase ("I", "this.I1 + this.I2", 0, NULL, NULL);
    SetValue (*instance, "I1", 3);
    TestNull (*instance, "I");

    instance = CreateTestCase ("I", "this.I1 / this.I2", 0, NULL, NULL);
    SetValue (*instance, "I1", 3);
    SetValue (*instance, "I2", 0);
    TestNull (*instance, "I");

    instance = CreateTestCase ("I", "this.I1 / this.I2", 0, "-1");
    SetValue (*instance, "I1", 3);
    SetValue (*instance, "I2", 0);
    Test (*instance, "I", -1);

    instance = CreateTestCase ("D", "this.D1 + this.D2", 0, NULL, NULL);
    SetValue (*instance, "D1", 3.7);
    TestNull (*instance, "D");

    instance = CreateTestCase ("D", "this.D1 / this.D2", 0, NULL, NULL);
    SetValue (*instance, "D1", 3.7);
    SetValue (*instance, "D2", 0);
    TestNull (*instance, "D");

    instance = CreateTestCase ("D", "this.D1 / this.D2", 0, "-1");
    SetValue (*instance, "D1", 3.7);
    SetValue (*instance, "D2", 0);
    Test (*instance, "D", -1.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad.Zaighum                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CalculatedPropertyTests, ExpectSuccessWhenCopyFailedCalculatedPropertyThatHasARegex)
    {
    IECInstancePtr instance = CreateTestCase ("S", "S1 - S2", 0, "<Could not be calculated>", "^(?<S1>.*)\\-(?<S2>.*)");
    IECInstancePtr instance2 = StandaloneECInstance::Duplicate (*instance);

    Test (*instance2, "S", "<Could not be calculated>");
    Test (*instance, "S", "<Could not be calculated>");
    TestNull (*instance, "S1");
    TestNull (*instance, "S2");
    TestNull (*instance2, "S1");
    TestNull (*instance2, "S2");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad.Zaighum                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CalculatedPropertyTests, EvaluationAgainstANewInstanceUsesItsNewValues)
    {
    IECInstancePtr instance = CreateTestCase ("S", "this.S1", 0, NULL, NULL);

    SetValue (*instance, "S1", "initial");
    Test (*instance, "S", "initial");

    SetValue (*instance, "S1", "different");
    Test (*instance, "S", "different");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad.Zaighum                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CalculatedPropertyTests, SettingACalculatedPropertyUpdatesThePropertiesFromWhichItWasCalculate)
    {
    IECInstancePtr instance = CreateTestCase ("S", "S1 - S2", 0, "<Could not be calculated>", "^(?<S1>.*)-(?<S2>.*)");
    SetValue (*instance, "S", "1-2");
    Test (*instance, "S1", "1");
    Test (*instance, "S2", "2");

    SetValue (*instance, "S", "3-4");
    Test (*instance, "S1", "3");
    Test (*instance, "S2", "4");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad.Zaighum                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CalculatedPropertyTests, CalculatedPropertiesWithNoParserRegExAreReadOnly)
    {
    IECInstancePtr instance = CreateTestCase ("S", "S1", 1, "<Could not be calculated>", NULL);
    ASSERT_EQ (true, instance->IsPropertyReadOnly ("S"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad.Zaighum                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CalculatedPropertyTests, UseLastValidValueInsteadOfFailure)
    {
    IECInstancePtr instance = CreateTestCase ("S", "this.S1 & \"-\" & this.S2", OPTION_UseLastValid, "<Could not be calculated>");
    Test (*instance, "S", "<Could not be calculated>");

    SetValue (*instance, "S1", "3");
    SetValue (*instance, "S2", "4");
    Test (*instance, "S", "3-4");

    SetNullValue (*instance, "S1");
    TestNull (*instance, "S1");

    Test (*instance, "S", "3-4");

    SetNullValue (*instance, "S2");
    TestNull (*instance, "S2");
    Test (*instance, "S", "3-4");
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
    IECInstancePtr instance = CreateTestCase ("S", "this.S1 & \" X \" & this.S2", 0, "FAILED", "^(?<S1>[ a-z\\\\d/]+[\\\"]?)[\\\\s]*X[\\\\s]*(?<S2>[ a-z\\\\d/]+[\\\"]?)|[\\\\w]*");
    SetValue (*instance, "S1", "a");
    SetValue (*instance, "S2", "b");
    Test (*instance, "S", "a X b");

    // Note that the parser regex doesn't appear to match what the author thinks it should match...he seems to be confused about escape characters.
    SetValue (*instance, "S", "xXy");
    Test (*instance, "S1", "x");
    Test (*instance, "S2", "y");
    }

TEST_F (CalculatedPropertyTests, ConvertNamedCaptureGroupsToUnnamedFromFile)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();
    ECSchemaPtr schema;

    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath (L"pidSnippet.01.02.ecschema.xml").c_str (), *schemaContext);
    EXPECT_EQ (SCHEMA_READ_STATUS_Success, status);

    ECClassP ecClass = schema->GetClassP ("BASE_REDUCER");
    IECInstancePtr instance = ecClass->GetDefaultStandaloneEnabler ()->CreateInstance ();

    SetValue (*instance, "LEFT_TEXT", "left");
    SetValue (*instance, "RIGHT_TEXT", "right");
    Test (*instance, "DISPLAY_TEXT", "left X right");

    SetValue (*instance, "DISPLAY_TEXT", "leftXright");
    Test (*instance, "LEFT_TEXT", "left");
    Test (*instance, "RIGHT_TEXT", "right");
    }

END_BENTLEY_ECN_TEST_NAMESPACE
