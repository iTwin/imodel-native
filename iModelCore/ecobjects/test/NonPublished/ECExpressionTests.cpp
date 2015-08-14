/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/ECExpressionTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include "TestFixture.h"

#include <ECObjects\ECInstance.h>
#include <ECObjects\StandaloneECInstance.h>
#include <ECObjects\ECValue.h>
#include <ECObjects\ECExpressions.h>

#define EXPECT_SUCCESS(EXPR) EXPECT_TRUE(SUCCESS == (EXPR))
#define EXPECT_ERROR(EXPR) EXPECT_FALSE(SUCCESS == (EXPR))
#define EXPECT_NOT_NULL(EXPR) EXPECT_FALSE(NULL == (EXPR))
#define EXPECT_NULL(EXPR) EXPECT_TRUE(NULL == (EXPR))

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

using namespace std;

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct ExpressionTests : ECTestFixture
    {
    virtual void        PublishSymbols (SymbolExpressionContextR context) { }

    ExpressionStatus    EvaluateExpression (EvaluationResult& result, WCharCP expr, IECInstanceR instance)
        {
        InstanceExpressionContextPtr context = InstanceExpressionContext::Create (NULL);
        context->SetInstance (instance);
        return EvaluateExpression (result, expr, *context);
        }
    ExpressionStatus    EvaluateExpression (EvaluationResult& result, WCharCP expr, ECInstanceListCR instances)
        {
        InstanceListExpressionContextPtr context = InstanceListExpressionContext::Create (instances, NULL);
        return EvaluateExpression (result, expr, *context);
        }
    ExpressionStatus    EvaluateExpression (EvaluationResult& result, WCharCP expr, InstanceListExpressionContextR context)
        {
        SymbolExpressionContextPtr symbolContext = SymbolExpressionContext::Create (NULL);
        ContextSymbolPtr instanceSymbol = ContextSymbol::CreateContextSymbol (L"this", context);
        symbolContext->AddSymbol (*instanceSymbol);

        PublishSymbols (*symbolContext);

        NodePtr tree = ECEvaluator::ParseValueExpressionAndCreateTree (expr);
        EXPECT_NOT_NULL (tree.get());

        return tree->GetValue (result, *symbolContext);
        }

    void                TestExpressionEquals (IECInstanceR instance, WCharCP expr, ECValueCR expectVal)
        {
        EvaluationResult result;
        ExpressionStatus status = EvaluateExpression (result, expr, instance);
        EXPECT_SUCCESS (status);
        if (SUCCESS == status)
            {
            EXPECT_TRUE (result.IsECValue());
            if (result.IsECValue())
                EXPECT_TRUE (expectVal.Equals (*result.GetECValue())) << L"Expected: " << expectVal.ToString().c_str() << L" Actual: " << result.GetECValue()->ToString().c_str() << " Expr: " << expr;
            }
        }

    template<typename T> void TestExpressionEquals (IECInstanceR instance, WCharCP expr, T const& v)
        {
        TestExpressionEquals (instance, expr, ECValue (v));
        }

    void                TestExpressionNullity (IECInstanceR instance, WCharCP expr, bool expectNull)
        {
        EvaluationResult result;
        ExpressionStatus status = EvaluateExpression (result, expr, instance);
        EXPECT_SUCCESS (status);
        if (SUCCESS == status)
            {
            if (result.IsECValue())
                EXPECT_EQ (expectNull, result.GetECValue()->IsNull());
            else
                EXPECT_TRUE (!expectNull && result.IsInstanceList());
            }
        }
    void                TestExpressionNull (IECInstanceR instance, WCharCP expr) { TestExpressionNullity (instance, expr, true); }
    void                TestExpressionNotNull (IECInstanceR instance, WCharCP expr) { TestExpressionNullity (instance, expr, false); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct RoundtripExpressionTests : ExpressionTests
    {
    void        Roundtrip (WCharCP inExpr, WCharCP expectedExpr)
        {
        NodePtr tree = ECEvaluator::ParseValueExpressionAndCreateTree (inExpr);
        EXPECT_TRUE (tree.IsValid());

        WString roundtrippedExpr = tree->ToExpressionString();
        EXPECT_TRUE (roundtrippedExpr.Equals (expectedExpr))
            << "Input:    " << inExpr << "\n"
            << "Expected: " << expectedExpr << "\n"
            << "Actual:   " << roundtrippedExpr.c_str();
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RoundtripExpressionTests, Roundtrip)
    {
    Roundtrip (L"1 + 2 * 3 / 4 + 5 - 6 ^ 7 + -8 / -9 + +10", L"1+2*3/4+5-6^7+-8/-9++10");
    Roundtrip (L"this.Property", L"this.Property");
    Roundtrip (L"this.Property *  this[\"Property\"]", L"this.Property*this[\"Property\"]");
    Roundtrip (L"Something.Method ( )", L"Something.Method()");
    Roundtrip (L"Something.Method (0,1.5, 2.000,  \t\"string\", this.Property   )", L"Something.Method(0,1.5,2.0,\"string\",this.Property)");
    Roundtrip (L"IIf (True,  Null, \t2 ^3  -3* 4)", L"IIf(True,Null,2^3-3*4)");
    Roundtrip (L"X = \"Thing\" OrElse X = \"Stuff\"", L"X=\"Thing\"OrElse X=\"Stuff\"");

    // Make sure we're capturing parens...
    Roundtrip (L"(1 +2) * 3", L"(1+2)*3");
    Roundtrip (L"(1 + (2 - 3)) * 4", L"(1+(2-3))*4");
    Roundtrip (L"IIf (True, 1, (IIf (False, 0, -1)))", L"IIf(True,1,(IIf(False,0,-1)))");

    // Should throw away redundant parens...but keep important ones...
    Roundtrip (L"(1)", L"(1)");
    Roundtrip (L"((1))", L"(1)");
    Roundtrip (L"(((((1)))))", L"(1)");
    Roundtrip (L"((1 + 2) * 3 / (4 + (5 - 6))) ^ (((7 + ((-8) -(((9)))) + +(10))))", L"((1+2)*3/(4+(5-6)))^(7+((-8)-(9))++(10))");
    Roundtrip (L"0.00390625", L"0.00390625");
    Roundtrip (L"method (method (True, method (method (1.5), False)))", L"method(method(True,method(method(1.5),False)))");

    Roundtrip (L"X => X < 5.0 AndAlso X > 1.5", L"X=>X<5.0 AndAlso X>1.5");
    Roundtrip (L"this.Array.Any (X => X.Name = \"Chuck\" OrElse X.Name = \"Bob\")", L"this.Array.Any(X=>X.Name=\"Chuck\"OrElse X.Name=\"Bob\")");
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceExpressionTests : ExpressionTests
    {
protected:
    ECSchemaPtr         m_schema;
public:
    virtual WString     GetTestSchemaXMLString () = 0;

    ECSchemaR           GetSchema()
        {
        if (m_schema.IsNull())
            {
            WString schemaXMLString = GetTestSchemaXMLString ();
            ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext();
            EXPECT_EQ (SUCCESS, ECSchema::ReadFromXmlString (m_schema, schemaXMLString.c_str(), *schemaContext));  
            }

        return *m_schema;
        }
    IECInstancePtr      CreateInstance (WCharCP classname)
        {
        return CreateInstance (classname, GetSchema());
        }
    static IECInstancePtr CreateInstance (WCharCP classname, ECSchemaCR schema)
        {
        ECClassCP ecClass = schema.GetClassCP (classname);
        return NULL != ecClass ? ecClass->GetDefaultStandaloneEnabler()->CreateInstance() : NULL;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
struct LiteralExpressionTests : InstanceExpressionTests
    {
public:
    virtual WString GetTestSchemaXMLString() override
        {
        return
            L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            L"<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
            L"    <ECClass typeName=\"ClassA\" displayLabel=\"Class A\" isDomainClass=\"True\">"
            L"        <ECProperty propertyName=\"d\" typeName=\"double\" />"
            L"    </ECClass>"
            L"</ECSchema>";
        }

    IECInstancePtr  CreateInstance (double d)
        {
        auto instance = InstanceExpressionTests::CreateInstance (L"ClassA");
        instance->SetValue (L"d", ECValue (d));
        return instance;
        }
    };

/*---------------------------------------------------------------------------------**//**
* John was comparing floating point values for exact equality.
* @bsimethod                                                    Paul.Connelly   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LiteralExpressionTests, FloatComparisons)
    {
    auto instance = CreateInstance (12.000000000001);
    TestExpressionEquals (*instance, L"this.d = 12", ECValue (true));
    TestExpressionEquals (*instance, L"this.d >= 12", ECValue (true));
    TestExpressionEquals (*instance, L"this.d <= 12", ECValue (true));
    TestExpressionEquals (*instance, L"this.d <> 12", ECValue (false));
    TestExpressionEquals (*instance, L"this.d > 12", ECValue (false));
    TestExpressionEquals (*instance, L"this.d < 12", ECValue (false));

    instance = CreateInstance (12.1);
    TestExpressionEquals (*instance, L"this.d = 12", ECValue (false));
    TestExpressionEquals (*instance, L"this.d >= 12", ECValue (true));
    TestExpressionEquals (*instance, L"this.d <= 12", ECValue (false));
    TestExpressionEquals (*instance, L"this.d <> 12", ECValue (true));
    TestExpressionEquals (*instance, L"this.d > 12", ECValue (true));
    TestExpressionEquals (*instance, L"this.d < 12", ECValue (false));
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct FullyQualifiedExpressionTests : InstanceExpressionTests
    {
    virtual WString GetTestSchemaXMLString() override
        {
        wchar_t fmt[] = L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                        L"<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                        L"    <ECClass typeName=\"ClassA\" displayLabel=\"Class A\" isDomainClass=\"True\">"
                        L"        <ECProperty propertyName=\"p\" typeName=\"int\" />"
                        L"    </ECClass>"
                        L"    <ECClass typeName=\"ClassB\" displayLabel=\"Class B\" isDomainClass=\"True\">"
                        L"        <ECProperty propertyName=\"p\" typeName=\"int\" />"
                        L"        <ECProperty propertyName=\"b\" typeName=\"int\" />"
                        L"    </ECClass>"
                        L"    <ECClass typeName=\"DerivesFromA\" displayLabel=\"Derives From A\" isDomainClass=\"True\">"
                        L"        <BaseClass>ClassA</BaseClass>"
                        L"        <ECProperty propertyName=\"p2\" typeName=\"int\" />"
                        L"    </ECClass>"
                        L"</ECSchema>";

        return fmt;
        }
    };

/*---------------------------------------------------------------------------------**//**
* In Vancouver, in native ECExpressions, we introduced support for fully-qualified
* property accessors using syntax like "this.SchemaName::ClassName::AccessString".
* It should work polymorphically and should not evaluate for an instance not of the
* specified class, even if that instance has a property matching AccessString.
* @bsimethod                                                    Paul.Connelly   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (FullyQualifiedExpressionTests, FullyQualifiedAccessors)
    {
    IECInstancePtr A = CreateInstance (L"ClassA"),
                   B = CreateInstance (L"ClassB"),
                   ADerived = CreateInstance (L"DerivesFromA");

    EvaluationResult result;

    // ClassA contains 'p'
    EXPECT_SUCCESS (EvaluateExpression (result, L"this.TestSchema::ClassA::p", *A));
    // ClassB contains 'p' but we have specified ClassA
    EXPECT_ERROR (EvaluateExpression (result, L"this.TestSchema::ClassA::p", *B));
    // ClassB contains 'p' and we have specified ClassB
    EXPECT_SUCCESS (EvaluateExpression (result, L"this.TestSchema::ClassB::p", *B));
    // ClassA contains 'p' and DerivesFromA is a subclass - we can find it whether we specify base or derived class
    EXPECT_SUCCESS (EvaluateExpression (result, L"this.TestSchema::ClassA::p", *ADerived));
    EXPECT_SUCCESS (EvaluateExpression (result, L"this.TestSchema::DerivesFromA::p", *ADerived));
    // ClassA contains 'p' but we have specified a subclass - should not find it
    EXPECT_ERROR (EvaluateExpression (result, L"this.TestSchema::DerivesFromA::p", *A));
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceListExpressionTests : InstanceExpressionTests
    {
    virtual WString GetTestSchemaXMLString() override
        {
        return      L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                    L"<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                    L"    <ECClass typeName=\"Struct1\" isStruct=\"True\">"
                    L"        <ECArrayProperty propertyName=\"Ints\" typeName=\"int\" />"
                    L"        <ECProperty propertyName=\"Int\" typeName=\"int\" />"
                    L"    </ECClass>"
                    L"    <ECClass typeName=\"Struct2\" isStruct=\"True\">"
                    L"        <ECArrayProperty propertyName=\"Structs\" typeName=\"Struct1\" isStruct=\"True\" />"
                    L"        <ECStructProperty propertyName=\"Struct\" typeName=\"Struct1\" />"
                    L"    </ECClass>"
                    L"    <ECClass typeName=\"ClassA\" isDomainClass=\"True\">"
                    L"        <ECStructProperty propertyName=\"Struct\" typeName=\"Struct2\" />"
                    L"        <ECArrayProperty propertyName=\"Structs\" typeName=\"Struct2\" isStruct=\"True\" />"
                    L"        <ECProperty propertyName=\"Int\" typeName=\"int\" />"
                    L"        <ECArrayProperty propertyName=\"Ints\" typeName=\"int\" />"
                    L"        <ECProperty propertyName=\"String\" typeName=\"string\" />"
                    L"    </ECClass>"
                    L"    <ECClass typeName=\"DerivedA\" isDomainClass=\"True\">"
                    L"        <BaseClass>ClassA</BaseClass>"
                    L"        <ECProperty propertyName=\"DerivedInt\" typeName=\"int\" />"
                    L"    </ECClass>"
                    L"</ECSchema>";
        }

    void    AddArrayElement (IECInstanceR instance, WCharCP accessString, ECValueCR entryVal)
        {
        ECValue arrayVal;
        EXPECT_SUCCESS (instance.GetValue (arrayVal, accessString));
        uint32_t index = arrayVal.GetArrayInfo().GetCount();
        EXPECT_SUCCESS (instance.AddArrayElements (accessString, 1));
        EXPECT_SUCCESS (instance.SetValue (accessString, entryVal, index));
        }
    };

/*---------------------------------------------------------------------------------**//**
* Test that the logic for evaluating property values in context of instance list behaves
* as expected for complex expressions involving embedded structs and nested struct arrays.
* @bsimethod                                                    Paul.Connelly   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (InstanceListExpressionTests, ComplexExpressions)
    {
    IECInstancePtr s1 = CreateInstance (L"Struct1");
    AddArrayElement (*s1, L"Ints", ECValue (1));
    s1->SetValue (L"Int", ECValue (2));

    ECValue v;
    IECInstancePtr s2 = CreateInstance (L"Struct2");
    s2->SetValue (L"Struct.Int", ECValue (3));
    AddArrayElement (*s2, L"Struct.Ints", ECValue (4));

    v.SetStruct (s1.get());
    AddArrayElement (*s2, L"Structs", v);

    IECInstancePtr a = CreateInstance (L"ClassA");
    a->SetValue (L"Int", ECValue (5));
    AddArrayElement (*a, L"Ints", ECValue (6));
    
    v.SetStruct (s2.get());
    AddArrayElement (*a, L"Structs", v);

    s1 = CreateInstance (L"Struct1");
    AddArrayElement (*s1, L"Ints", ECValue (7));
    s1->SetValue (L"Int", ECValue (8));

    v.SetStruct (s1.get());
    AddArrayElement (*a, L"Struct.Structs", v);

    a->SetValue (L"Struct.Struct.Int", ECValue (9));
    AddArrayElement (*a, L"Struct.Struct.Ints", ECValue (10));

    static WCharCP  s_expressions[10] =
        {
        L"this.Structs[0].Structs[0].Ints[0]",
        L"this.Structs[0].Structs[0].Int",
        L"this.Structs[0].Struct.Int",
        L"this.Structs[0].Struct.Ints[0]",
        L"this.Int",
        L"this.Ints[0]",
        L"this.Struct.Structs[0].Ints[0]",
        L"this.Struct.Structs[0].Int",
        L"this.Struct.Struct.Int",
        L"this.Struct.Struct.Ints[0]"
        };

    for (size_t i = 0; i < _countof(s_expressions); i++)
        {
        WCharCP expr = s_expressions[i];
        EvaluationResult result;
        ExpressionStatus status = EvaluateExpression (result, expr, *a);
        EXPECT_SUCCESS (status);
        if (ExprStatus_Success != status)
            continue;

        EXPECT_TRUE (result.IsECValue());
        EXPECT_EQ (result.GetECValue()->GetInteger(), (int32_t)i+1);
        }

#ifdef NEEDSWORK_COMPLEX_ACCESS_STRINGS
    // Doesn't work for expressions like 'this["Struct.Member"]'
    static WCharCP s_bracketExpressions[10] =
        {
        L"this[\"Structs\"][0][\"Structs\"][0][\"Ints\"][0]",
        L"this[\"Structs\"][0][\"Structs\"][0][\"Int\"]",
        L"this[\"Structs\"][0][\"Struct.Int\"]",
        L"this[\"Structs\"][0][\"Struct.Ints\"][0]",
        L"this[\"Int\"]",
        L"this[\"Ints\"][0]",
        L"this[\"Struct.Structs\"][0][\"Ints\"][0]",
        L"this[\"Struct.Structs\"][0][\"Int\"]",
        L"this[\"Struct.Struct.Int\"]",
        L"this[\"Struct.Struct.Ints\"][0]"
        };

    for (size_t i = 0; i < _countof(s_bracketExpressions); i++)
        {
        WCharCP expr = s_bracketExpressions[i];
        EvaluationResult result;
        ExpressionStatus status = EvaluateExpression (result, expr, *a);
        EXPECT_SUCCESS (status);
        if (ExprStatus_Success != status)
            continue;

        EXPECT_TRUE (result.IsECValue());
        EXPECT_EQ (result.GetECValue()->GetInteger(), (int32_t)i+1);
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct ArrayExpressionTests : InstanceListExpressionTests
    {
    IValueListResultPtr     GetIValueList (IECInstanceR instance, WCharCP expr)
        {
        EvaluationResult result;
        ExpressionStatus status = EvaluateExpression (result, expr, instance);
        EXPECT_SUCCESS (status);
        EXPECT_NOT_NULL (result.GetValueList());
        return const_cast<IValueListResultP>(result.GetValueList());
        }

    void                    TestIValueList (IValueListResultP list, uint32_t expectedCount)
        {
        EXPECT_NOT_NULL (list);
        if (NULL != list)
            {
            EXPECT_EQ (list->GetCount(), expectedCount);

            for (uint32_t i = 0; i < expectedCount; i++)
                {
                EvaluationResult elem;
                EXPECT_SUCCESS (list->GetValueAt (elem, i));
                }
            }
        }

    static ExpressionStatus     SumArrayMembers (EvaluationResultR result, IValueListResultCR valueList, EvaluationResultVector& args)
        {
        uint32_t count = valueList.GetCount();
        int32_t sum = 0;
        ExpressionStatus status = ExprStatus_Success;

        for (uint32_t i = 0; i < count; i++)
            {
            EvaluationResult member;
            status = valueList.GetValueAt (member, i);
            if (ExprStatus_Success == status && member.IsECValue() && member.GetECValue()->IsInteger())
                sum += member.GetECValue()->GetInteger();
            }

        if (ExprStatus_Success == status)
            result.InitECValue().SetInteger (sum);

        return status;
        }

    virtual void PublishSymbols (SymbolExpressionContextR context) override
        {
        context.AddSymbol (*MethodSymbol::Create (L"SumArray", &SumArrayMembers));
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArrayExpressionTests, ArrayProperties)
    {
    IECInstancePtr s1 = CreateInstance (L"Struct1");
    AddArrayElement (*s1, L"Ints", ECValue (1));
    AddArrayElement (*s1, L"Ints", ECValue (2));

    IECInstancePtr s2 = CreateInstance (L"Struct2");
    ECValue v;
    v.SetStruct (s1.get());
    AddArrayElement (*s2, L"Structs", v);

    IECInstancePtr a = CreateInstance (L"ClassA");
    for (uint32_t i = 0; i < 5; i++)
        {
        v.SetInteger ((int32_t)i);
        AddArrayElement (*a, L"Ints", v);
        }

    v.SetStruct (s2.get());
    AddArrayElement (*a, L"Structs", v);

    // -- Test querying the value lists directly --

    IValueListResultPtr list = GetIValueList (*a, L"this.Ints");
    TestIValueList (list.get(), 5);

    list = GetIValueList (*a, L"this.Structs");
    TestIValueList (list.get(), 1);

    list = GetIValueList (*a, L"this.Struct.Structs");
    TestIValueList (list.get(), 0);

    list = GetIValueList (*a, L"this.Structs[0].Structs");
    TestIValueList (list.get(), 1);

    list = GetIValueList (*a, L"this.Structs[0].Structs[0].Ints");
    TestIValueList (list.get(), 2);

    // -- Test Count, First, Last properties --

    TestExpressionEquals (*a, L"this.Ints.Count", 5);
    TestExpressionEquals (*a, L"this.Structs.Count", 1);
    TestExpressionEquals (*a, L"this.Struct.Structs.Count", 0);
    TestExpressionEquals (*a, L"this.Structs[0].Structs.Count", 1);
    TestExpressionEquals (*a, L"this.Structs[0].Structs[0].Ints.Count", 2);

    TestExpressionEquals (*a, L"this.Ints.First", 0);
    TestExpressionEquals (*a, L"this.Ints.Last", 4);

    TestExpressionEquals (*a, L"this.Structs[0].Structs[0].Ints.First", 1);
    TestExpressionEquals (*a, L"this.Structs[0].Structs[0].Ints.Last", 2);

    // First and Last return null for an empty array (but not an error!)
    TestExpressionNull (*a, L"this.Struct.Structs.First");
    TestExpressionNull (*a, L"this.Struct.Structs.Last");

    // Can't really test struct array values for equality, but can test not null
    TestExpressionNotNull (*a, L"this.Structs.First");
    TestExpressionNotNull (*a, L"this.Structs.Last");

    // -- ###TODO Test Any() and All() methods -- 

    //EvaluationResult result;
    //EXPECT_SUCCESS (EvaluateExpression (result, L"this.Structs.Any(\"args...\")", *a));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArrayExpressionTests, ArrayMethods)
    {
    IECInstancePtr a = CreateInstance (L"ClassA");
    int32_t expectedSum = 0;
    for (int32_t i = 0; i < 4; i++)
        {
        expectedSum += i;
        AddArrayElement (*a, L"Ints", ECValue (i));
        }

    TestExpressionEquals (*a, L"this.Ints.SumArray()", expectedSum);
    TestExpressionEquals (*a, L"this.Ints.SumArray() * this.Ints.SumArray()", expectedSum*expectedSum);
    }

/*---------------------------------------------------------------------------------**//**
* In a case where two instances in the list can satisfy a property accessor, the value is
* obtained from the first matching instance found.
* Nobody should write expressions that rely on this behavior but we do want to test that
* it behaves as expected.
* @bsimethod                                                    Paul.Connelly   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (InstanceListExpressionTests, FirstMatchWins)
    {
    IECInstancePtr a1 = CreateInstance(L"ClassA");
    a1->SetValue (L"Int", ECValue (1));
    IECInstancePtr a2 = CreateInstance (L"ClassA");
    a2->SetValue (L"Int", ECValue (2));

    ECInstanceList instances;
    instances.push_back (a1);
    instances.push_back (a2);

    EvaluationResult result;
    EXPECT_SUCCESS (EvaluateExpression (result, L"this.Int", instances));
    EXPECT_TRUE (result.IsECValue());
    EXPECT_EQ (result.GetECValue()->GetInteger(), 1);

    instances.clear();
    instances.push_back (a2);
    instances.push_back (a1);

    EXPECT_SUCCESS (EvaluateExpression (result, L"this.Int", instances));
    EXPECT_TRUE (result.IsECValue());
    EXPECT_EQ (result.GetECValue()->GetInteger(), 2);

    // using fully-qualified property names can help resolve the correct instance
    IECInstancePtr s1 = CreateInstance (L"Struct1");
    s1->SetValue (L"Int", ECValue (3));

    instances.clear();
    instances.push_back (s1);
    instances.push_back (a1);

    EXPECT_SUCCESS (EvaluateExpression (result, L"this.TestSchema::ClassA::Int", instances));
    EXPECT_EQ (result.GetECValue()->GetInteger(), 1);
    EXPECT_SUCCESS (EvaluateExpression (result, L"this.TestSchema::Struct1::Int", instances));
    EXPECT_EQ (result.GetECValue()->GetInteger(), 3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct MethodsReturningInstancesTests : InstanceListExpressionTests
    {
    static ECSchemaP        s_schema;

    virtual void PublishSymbols (SymbolExpressionContextR context)
        {
        // NEEDSWORK? Cannot invoke methods on instance lists returned by static methods, only by instance methods...doesn't seem right...
        context.AddSymbol (*MethodSymbol::Create (L"CreateInstanceA", NULL, &CreateInstanceA));
        context.AddSymbol (*MethodSymbol::Create (L"CreateInstancesA", NULL, &CreateInstancesA));
        }

    static ExpressionStatus     CreateInstanceA (EvaluationResultR result, ECInstanceListCR, EvaluationResultVector& args)
        {
        IECInstancePtr instance = CreateInstance (L"ClassA", *s_schema);
        instance->SetValue (L"String", ECValue (L"A"));
        result.SetInstance (*instance);
        return ExprStatus_Success;
        }
    static ExpressionStatus     CreateInstancesA (EvaluationResultR result, ECInstanceListCR, EvaluationResultVector& args)
        {
        ECInstanceList instances;
        IECInstancePtr a = CreateInstance (L"ClassA", *s_schema);
        a->SetValue (L"String", ECValue (L"A1"));
        instances.push_back (a);

        a = CreateInstance (L"DerivedA", *s_schema);
        a->SetValue (L"String", ECValue (L"A2"));
        a->SetValue (L"DerivedInt", ECValue (2));
        instances.push_back (a);

        result.SetInstanceList (instances, true);
        return ExprStatus_Success;
        }
    };

ECSchemaP MethodsReturningInstancesTests::s_schema = NULL;

/*---------------------------------------------------------------------------------**//**
* An ECExpression method can return 1 or multiple instances. Test both.
* @bsimethod                                                    Paul.Connelly   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (MethodsReturningInstancesTests, InstanceMethods)
    {
    s_schema = &GetSchema();
    IECInstancePtr dummy = CreateInstance (L"ClassA");

    EvaluationResult result;
    EXPECT_SUCCESS (EvaluateExpression (result, L"this.CreateInstanceA()", *dummy));
    EXPECT_TRUE (result.IsInstanceList());
    EXPECT_EQ (1, result.GetInstanceList()->size());

    EXPECT_SUCCESS (EvaluateExpression (result, L"this.CreateInstancesA()", *dummy));
    EXPECT_TRUE (result.IsInstanceList());
    EXPECT_EQ (2, result.GetInstanceList()->size());

    EXPECT_SUCCESS (EvaluateExpression (result, L"this.CreateInstanceA().String", *dummy));
    EXPECT_TRUE (result.IsECValue());
    EXPECT_EQ (0, wcscmp (result.GetECValue()->GetString(), L"A"));

    EXPECT_SUCCESS (EvaluateExpression (result, L"this.CreateInstancesA()[\"String\"]", *dummy));
    EXPECT_TRUE (result.IsECValue());
    EXPECT_EQ (0, wcscmp (result.GetECValue()->GetString(), L"A1"));

    EXPECT_SUCCESS (EvaluateExpression (result, L"this.CreateInstancesA().DerivedInt", *dummy));
    EXPECT_TRUE (result.IsECValue());
    EXPECT_EQ (result.GetECValue()->GetInteger(), 2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct ExpressionRemappingTest : ECTestFixture, IECSchemaRemapper
    {
private:
    ECSchemaPtr     m_preSchema;
    ECSchemaPtr     m_postSchema;

    ECSchemaPtr     CreateSchema (bool rename);

    WString         GetName (WCharCP name, bool rename) const
        {
        WString newName (name);
        if (rename)
            newName.ToUpper();

        return newName;
        }

    WString         GetOldName (WCharCP name) const
        {
        WString oldName (name);
        oldName.ToLower();
        return oldName;
        }

    virtual bool    _ResolveClassName (WStringR className, ECSchemaCR schema) const override
        {
        if (&schema != m_postSchema.get() || nullptr == m_preSchema->GetClassCP (GetOldName (className.c_str()).c_str()))
            return false;

        className.ToUpper();
        return true;
        }
    virtual bool    _ResolvePropertyName (WStringR propName, ECClassCR ecClass) const override
        {
        if (&ecClass.GetSchema() == m_postSchema.get())
            {
            ECClassCP oldClass = m_preSchema->GetClassCP (GetOldName (ecClass.GetName().c_str()).c_str());
            if (nullptr != oldClass && nullptr != oldClass->GetPropertyP (propName.c_str()))
                {
                propName.ToUpper();
                return true;
                }
            }

        return false;
        }

public:
    ExpressionRemappingTest() : m_preSchema (CreateSchema (false)), m_postSchema (CreateSchema (true)) { }

    void            Expect (WCharCP input, WCharCP output)
        {
        NodePtr expr = ECEvaluator::ParseValueExpressionAndCreateTree (input);
        EXPECT_NOT_NULL (expr.get());

        WString str = expr->ToExpressionString();
        EXPECT_TRUE (str.Equals (input)) << L"Input: " << input << " Round-tripped: " << str.c_str();

        bool anyRemapped = expr->Remap (*m_preSchema, *m_postSchema, *this);
        EXPECT_EQ (anyRemapped, 0 != wcscmp (input, output));

        str = expr->ToExpressionString();
        EXPECT_TRUE (str.Equals (output)) << L"Expected: " << output << " Actual: " << str.c_str() << " Input: " << input;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr ExpressionRemappingTest::CreateSchema (bool rename)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema (schema, GetName (L"schema", rename), 1, 0);

    ECClassP structClass;
    schema->CreateClass (structClass, GetName (L"struct", rename));
    structClass->SetIsStruct (true);

    ECClassP subStructClass;
    schema->CreateClass (subStructClass, GetName (L"substruct", rename));
    subStructClass->SetIsStruct (true);

    PrimitiveECPropertyP primProp;
    subStructClass->CreatePrimitiveProperty (primProp, GetName (L"bool", rename), PRIMITIVETYPE_Boolean);

    StructECPropertyP structProp;
    structClass->CreateStructProperty (structProp, GetName (L"substruct", rename), *subStructClass);
    structClass->CreatePrimitiveProperty (primProp, GetName (L"int", rename), PRIMITIVETYPE_Integer);

    ECClassP ecClass;
    schema->CreateClass (ecClass, GetName (L"class", rename));

    ecClass->CreatePrimitiveProperty (primProp, GetName (L"string", rename), PRIMITIVETYPE_String);

    ArrayECPropertyP arrayProp;
    ecClass->CreateArrayProperty (arrayProp, GetName (L"doubles", rename), PRIMITIVETYPE_Double);
    ecClass->CreateArrayProperty (arrayProp, GetName (L"structs", rename), structClass);
    ecClass->CreateStructProperty (structProp, GetName (L"struct", rename), *structClass);

    return schema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ExpressionRemappingTest, Remap)
    {
    static WCharCP s_expressions[][2] =
        {
            { L"this.schema::class::string", L"this.SCHEMA::CLASS::STRING" },
            { L"this.schema::class::doubles[0]", L"this.SCHEMA::CLASS::DOUBLES[0]" },
            { L"this.schema::class::struct.int", L"this.SCHEMA::CLASS::STRUCT.INT" },
            { L"this.schema::class::struct.substruct.bool", L"this.SCHEMA::CLASS::STRUCT.SUBSTRUCT.BOOL" },
            { L"this.schema::class::structs[0].int", L"this.SCHEMA::CLASS::STRUCTS[0].INT" },
            { L"this.schema::class::structs[0].substruct.bool", L"this.SCHEMA::CLASS::STRUCTS[0].SUBSTRUCT.BOOL" },
            { L"System.String.CompareI(this.DgnCustomItemTypes_Schema::Class::Struct.Prop,\"abc\")", L"System.String.CompareI(this.DgnCustomItemTypes_Schema::Class::Struct.Prop,\"abc\")" },
        };

    for (auto const& pair : s_expressions)
        Expect (pair[0], pair[1]);
    }

END_BENTLEY_ECOBJECT_NAMESPACE

