/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

#include <ECObjects/ECInstance.h>
#include <ECObjects/StandaloneECInstance.h>
#include <ECObjects/ECValue.h>
#include <ECObjects/ECExpressions.h>
#include <ECObjects/ECExpressionNode.h>

#define EXPECT_SUCCESS(EXPR) EXPECT_TRUE(ExpressionStatus::Success == (EXPR))
#define EXPECT_ERROR(EXPR) EXPECT_FALSE(ExpressionStatus::Success == (EXPR))
#define EXPECT_NOT_NULL(EXPR) EXPECT_FALSE(NULL == (EXPR))
#define EXPECT_NULL(EXPR) EXPECT_TRUE(NULL == (EXPR))

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct ExpressionTests : ECTestFixture
    {
    virtual void        PublishSymbols (SymbolExpressionContextR context) { }

    ExpressionStatus    EvaluateExpression (EvaluationResult& result, Utf8CP expr, IECInstanceR instance)
        {
        InstanceExpressionContextPtr context = InstanceExpressionContext::Create (NULL);
        context->SetInstance (instance);
        return EvaluateExpression (result, expr, *context);
        }
    ExpressionStatus    EvaluateExpression (EvaluationResult& result, Utf8CP expr, ECInstanceListCR instances)
        {
        InstanceListExpressionContextPtr context = InstanceListExpressionContext::Create (instances, NULL);
        return EvaluateExpression (result, expr, *context);
        }
    ExpressionStatus    EvaluateExpression (EvaluationResult& result, Utf8CP expr, InstanceListExpressionContextR context)
        {
        SymbolExpressionContextPtr symbolContext = SymbolExpressionContext::Create (NULL);
        ContextSymbolPtr instanceSymbol = ContextSymbol::CreateContextSymbol ("this", context);
        symbolContext->AddSymbol (*instanceSymbol);

        PublishSymbols (*symbolContext);

        NodePtr tree = ECEvaluator::ParseValueExpressionAndCreateTree (expr);
        EXPECT_NOT_NULL (tree.get());

        return tree->GetValue (result, *symbolContext);
        }
    ExpressionStatus    EvaluateExpression (EvaluationResult& result, Utf8CP expr, bvector<Utf8String>& requiredSymbolSets, IECInstanceP instance)
        {
        SymbolExpressionContextPtr contextWithThis = SymbolExpressionContext::CreateWithThis (requiredSymbolSets, instance);

        return  ECEvaluator::EvaluateExpression (result, expr, *contextWithThis);
        }

        ExpressionStatus    EvaluateExpression (EvaluationResult& result, Utf8CP expr)
        {
        // when a symbolset is passed in - published symbol providers will add their symbols to the context.
        SymbolExpressionContextPtr symbolContext = SymbolExpressionContext::Create (NULL);

        PublishSymbols (*symbolContext);

        NodePtr tree = ECEvaluator::ParseValueExpressionAndCreateTree (expr);
        EXPECT_NOT_NULL (tree.get());

        return tree->GetValue (result, *symbolContext);
        }


    void                TestExpressionEquals (IECInstanceR instance, Utf8CP expr, ECValueCR expectVal)
        {
        EvaluationResult result;
        ExpressionStatus status = EvaluateExpression (result, expr, instance);
        EXPECT_SUCCESS (status);
        if (ExpressionStatus::Success == status)
            {
            EXPECT_TRUE (result.IsECValue());
            if (result.IsECValue ())
                {
                if (expectVal.IsString())
                    EXPECT_TRUE (expectVal.ToString().Equals (result.GetECValue()->ToString())) << "Expected: " << expectVal.ToString().c_str() << " Actual: " << result.GetECValue()->ToString().c_str() << " Expr: " << expr;
                else
                    EXPECT_TRUE (expectVal.Equals (*result.GetECValue())) << "Expected: " << expectVal.ToString().c_str() << " Actual: " << result.GetECValue()->ToString().c_str() << " Expr: " << expr;
                }
            }
        }

    void                TestExpressionEquals (bvector<Utf8String>& requiredSymbolSets, Utf8CP expr, ECValueCR expectVal, IECInstanceP instance=nullptr)
        {
        EvaluationResult result;
        ExpressionStatus status = EvaluateExpression (result, expr, requiredSymbolSets, instance);
        EXPECT_SUCCESS (status);
        if (ExpressionStatus::Success == status)
            {
            EXPECT_TRUE (result.IsECValue());
            if (result.IsECValue())
                {
                if (expectVal.IsString())
                    EXPECT_TRUE (expectVal.ToString().Equals (result.GetECValue()->ToString().c_str())) << "Expected: " << expectVal.ToString().c_str() << " Actual: " << result.GetECValue()->ToString().c_str() << " Expr: " << expr;
                else
                    EXPECT_TRUE (expectVal.Equals (*result.GetECValue())) << "Expected: " << expectVal.ToString().c_str() << " Actual: " << result.GetECValue()->ToString().c_str() << " Expr: " << expr;
                }
            }
        }

        void                TestExpressionEquals (Utf8CP expr, ECValueCR expectVal)
        {
        EvaluationResult result;
        ExpressionStatus status = EvaluateExpression (result, expr);
        EXPECT_SUCCESS (status);
        if (ExpressionStatus::Success == status)
            {
            EXPECT_TRUE (result.IsECValue());
            if (result.IsECValue())
                {
                if (expectVal.IsString())
                    EXPECT_TRUE (expectVal.ToString().Equals (result.GetECValue()->ToString().c_str())) << "Expected: " << expectVal.ToString().c_str() << " Actual: " << result.GetECValue()->ToString().c_str() << " Expr: " << expr;
                else
                    EXPECT_TRUE (expectVal.Equals (*result.GetECValue())) << "Expected: " << expectVal.ToString().c_str() << " Actual: " << result.GetECValue()->ToString().c_str() << " Expr: " << expr;
                }
            }
        }

    template<typename T> void TestExpressionEquals (IECInstanceR instance, Utf8CP expr, T const& v)
        {
        TestExpressionEquals (instance, expr, ECValue (v));
        }

    void                TestExpressionNullity (IECInstanceR instance, Utf8CP expr, bool expectNull)
        {
        EvaluationResult result;
        ExpressionStatus status = EvaluateExpression (result, expr, instance);
        EXPECT_SUCCESS (status);
        if (ExpressionStatus::Success == status)
            {
            if (result.IsECValue())
                EXPECT_EQ (expectNull, result.GetECValue()->IsNull());
            else
                EXPECT_TRUE (!expectNull && result.IsInstanceList());
            }
        }
    void                TestExpressionNull (IECInstanceR instance, Utf8CP expr) { TestExpressionNullity (instance, expr, true); }
    void                TestExpressionNotNull (IECInstanceR instance, Utf8CP expr) { TestExpressionNullity (instance, expr, false); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct LambdaInputsAggregator : LambdaValue::IProcessor
    {
    bvector<EvaluationResult> m_evaluatedValues;
    bool ProcessResult(ExpressionStatus, EvaluationResultCR, EvaluationResultCR result) override
        {
        m_evaluatedValues.push_back(result);
        return true;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ExpressionTests, LambdaValue_EvaluatesValueListOfContexts)
    {
    ExpressionContextPtr outerContext = SymbolExpressionContext::Create(nullptr);
    LambdaNodePtr node = LambdaNode::Create("x", *ECEvaluator::ParseValueExpressionAndCreateTree("x.property"));
    LambdaValuePtr value = LambdaValue::Create(*node, *outerContext);

    bvector<EvaluationResult> contexts;

    SymbolExpressionContextPtr context1 = SymbolExpressionContext::Create(nullptr);
    context1->AddSymbol(*ValueSymbol::Create("property", ECValue(123)));
    EvaluationResult contextResult1;
    contextResult1.SetContext(*context1);
    contexts.push_back(contextResult1);

    SymbolExpressionContextPtr context2 = SymbolExpressionContext::Create(nullptr);
    context2->AddSymbol(*ValueSymbol::Create("property", ECValue(456)));
    EvaluationResult contextResult2;
    contextResult2.SetContext(*context2);
    contexts.push_back(contextResult2);

    IValueListResultPtr valueList = IValueListResult::Create(contexts);

    LambdaInputsAggregator agg;
    EXPECT_EQ(ExpressionStatus::Success, value->Evaluate(*valueList, agg));
    EXPECT_EQ(2, agg.m_evaluatedValues.size());
    EXPECT_EQ(ECValue(123), *agg.m_evaluatedValues[0].GetECValue());
    EXPECT_EQ(ECValue(456), *agg.m_evaluatedValues[1].GetECValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
struct RoundtripExpressionTests : ExpressionTests
    {
    void        Roundtrip (Utf8CP inExpr, Utf8CP expectedExpr)
        {
        NodePtr tree = ECEvaluator::ParseValueExpressionAndCreateTree (inExpr);
        EXPECT_TRUE (tree.IsValid());

        Utf8String roundtrippedExpr = tree->ToExpressionString();
        EXPECT_TRUE (roundtrippedExpr.Equals (expectedExpr))
            << "Input:    " << inExpr << "\n"
            << "Expected: " << expectedExpr << "\n"
            << "Actual:   " << roundtrippedExpr.c_str();
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RoundtripExpressionTests, Roundtrip)
    {
    Roundtrip ("1 + 2 * 3 / 4 + 5 - 6 ^ 7 + -8 / -9 + +10", "1+2*3/4+5-6^7+-8/-9++10");
    Roundtrip ("this.Property", "this.Property");
    Roundtrip ("this.Property *  this[\"Property\"]", "this.Property*this[\"Property\"]");
    Roundtrip ("Something.Method ( )", "Something.Method()");
    Roundtrip ("Something.Method (0,1.5, 2.000,  \t\"string\", this.Property   )", "Something.Method(0,1.5,2.0,\"string\",this.Property)");
    Roundtrip ("IIf (True,  Null, \t2 ^3  -3* 4)", "IIf(True,Null,2^3-3*4)");
    Roundtrip ("X = \"Thing\" OrElse X = \"Stuff\"", "X=\"Thing\"OrElse X=\"Stuff\"");

    // Make sure we're capturing parens...
    Roundtrip ("(1 +2) * 3", "(1+2)*3");
    Roundtrip ("(1 + (2 - 3)) * 4", "(1+(2-3))*4");
    Roundtrip ("IIf (True, 1, (IIf (False, 0, -1)))", "IIf(True,1,(IIf(False,0,-1)))");

    // Should throw away redundant parens...but keep important ones...
    Roundtrip ("(1)", "(1)");
    Roundtrip ("((1))", "(1)");
    Roundtrip ("(((((1)))))", "(1)");
    Roundtrip ("((1 + 2) * 3 / (4 + (5 - 6))) ^ (((7 + ((-8) -(((9)))) + +(10))))", "((1+2)*3/(4+(5-6)))^(7+((-8)-(9))++(10))");
    Roundtrip ("0.00390625", "0.00390625");
    Roundtrip ("method (method (True, method (method (1.5), False)))", "method(method(True,method(method(1.5),False)))");

    Roundtrip ("X => X < 5.0 AndAlso X > 1.5", "X=>X<5.0 AndAlso X>1.5");
    Roundtrip ("this.Array.Any (X => X.Name = \"Chuck\" OrElse X.Name = \"Bob\")", "this.Array.Any(X=>X.Name=\"Chuck\"OrElse X.Name=\"Bob\")");
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceExpressionTests : ExpressionTests
    {
protected:
    ECSchemaPtr         m_schema;
    uint64_t m_instanceIdCounter;
public:
    InstanceExpressionTests() : m_instanceIdCounter(0) {}
    virtual Utf8String     GetTestSchemaXMLString () = 0;

    ECSchemaR           GetSchema()
        {
        if (m_schema.IsNull())
            {
            Utf8String schemaXMLString = GetTestSchemaXMLString ();
            ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext();
            EXPECT_EQ (SchemaReadStatus::Success, ECSchema::ReadFromXmlString (m_schema, schemaXMLString.c_str(), *schemaContext));
            }

        return *m_schema;
        }
    IECInstancePtr      CreateInstance (Utf8CP classname)
        {
        IECInstancePtr instance = CreateInstance (classname, GetSchema());
        if (instance.IsValid())
            instance->SetInstanceId(BeInt64Id(++m_instanceIdCounter).ToString().c_str());
        return instance;
        }
    static IECInstancePtr CreateInstance (Utf8CP classname, ECSchemaCR schema)
        {
        ECClassCP ecClass = schema.GetClassCP (classname);
        return NULL != ecClass ? ecClass->GetDefaultStandaloneEnabler()->CreateInstance() : NULL;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct LiteralExpressionTests : InstanceExpressionTests
    {
public:
    virtual Utf8String GetTestSchemaXMLString() override
        {
        return
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<ECSchema schemaName=\"TestSchema\" alias=\"test\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">"
            "    <ECEntityClass typeName=\"ClassA\">"
            "        <ECProperty propertyName=\"d\" typeName=\"double\" />"
            "        <ECProperty propertyName=\"s\" typeName=\"string\" />"
            "    </ECEntityClass>"
            "    <ECEntityClass typeName=\"ClassB\">"
            "        <ECNavigationProperty propertyName=\"n\" relationshipName=\"ClassBPointsToClassA\" direction=\"Backward\" />"
            "    </ECEntityClass>"
            "    <ECRelationshipClass typeName=\"ClassBPointsToClassA\" strength=\"referencing\" strengthDirection=\"forward\" modifier=\"None\">"
            "        <Source multiplicity=\"(0..1)\" polymorphic=\"False\" roleLabel=\"Class A\">"
            "            <Class class=\"ClassA\" />"
            "        </Source>"
            "        <Target multiplicity=\"(0..*)\" polymorphic=\"False\" roleLabel=\"Class B\">"
            "            <Class class=\"ClassB\" />"
            "        </Target>"
            "    </ECRelationshipClass>"
            "</ECSchema>";
        }

    IECInstancePtr CreateInstanceA(double d)
        {
        auto instance = InstanceExpressionTests::CreateInstance ("ClassA");
        instance->SetValue ("d", ECValue (d));
        return instance;
        }

    IECInstancePtr CreateInstanceA(Utf8String& s)
        {
        auto instance = InstanceExpressionTests::CreateInstance ("ClassA");
        instance->SetValue ("s", ECValue (s.c_str()));
        return instance;
        }

    IECInstancePtr CreateInstanceB(IECInstanceP instanceA = nullptr)
        {
        auto instance = InstanceExpressionTests::CreateInstance("ClassB");
        if (instanceA)
            {
            ECRelationshipClassCP rel = GetSchema().GetClassCP("ClassBPointsToClassA")->GetRelationshipClassCP();
            instance->SetValue("n", ECValue(BeInt64Id::FromString(instanceA->GetInstanceId().c_str()), rel));
            }
        return instance;
        }
    };

/*---------------------------------------------------------------------------------**//**
* John was comparing floating point values for exact equality.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LiteralExpressionTests, FloatComparisons)
    {
    auto instance = CreateInstanceA(12.000000000001);
    TestExpressionEquals (*instance, "this.d = 12", ECValue (true));
    TestExpressionEquals (*instance, "this.d >= 12", ECValue (true));
    TestExpressionEquals (*instance, "this.d <= 12", ECValue (true));
    TestExpressionEquals (*instance, "this.d <> 12", ECValue (false));
    TestExpressionEquals (*instance, "this.d > 12", ECValue (false));
    TestExpressionEquals (*instance, "this.d < 12", ECValue (false));

    instance = CreateInstanceA(12.1);
    TestExpressionEquals (*instance, "this.d = 12", ECValue (false));
    TestExpressionEquals (*instance, "this.d >= 12", ECValue (true));
    TestExpressionEquals (*instance, "this.d <= 12", ECValue (false));
    TestExpressionEquals (*instance, "this.d <> 12", ECValue (true));
    TestExpressionEquals (*instance, "this.d > 12", ECValue (true));
    TestExpressionEquals (*instance, "this.d < 12", ECValue (false));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LiteralExpressionTests, IdProperties)
    {
    auto a = CreateInstanceA(0);
    EvaluationResult result;
    TestExpressionEquals(*a, "this.ECInstanceId", ECValue(BeInt64Id::FromString(a->GetInstanceId().c_str()).ToHexStr().c_str()));
    TestExpressionEquals(*a, "this.ECClassId", ECValue("0")); // ECClassId is 0 because the class is not persisted
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LiteralExpressionTests, NavigationProperties)
    {
    auto a = CreateInstanceA(0);
    auto b1 = CreateInstanceB();
    auto b2 = CreateInstanceB(a.get());

    EvaluationResult result;
    EXPECT_EQ(ExpressionStatus::UnknownError, EvaluateExpression(result, "this.n", *b1));
    EXPECT_EQ(ExpressionStatus::UnknownMember, EvaluateExpression(result, "this.n.test", *b1));
    EXPECT_EQ(ExpressionStatus::UnknownMember, EvaluateExpression(result, "this.n.id.test", *b1));
    TestExpressionEquals(*b1, "this.n.Id", ECValue());
    TestExpressionEquals(*b2, "this.n.Id", ECValue(BeInt64Id::FromString(a->GetInstanceId().c_str()).GetValue()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LiteralExpressionTests, GetBoolOperation)
    {
    NodePtr node = ECEvaluator::ParseValueExpressionAndCreateTree ("false");
    EXPECT_EQ(TOKEN_False, node->GetOperation());

    node = ECEvaluator::ParseValueExpressionAndCreateTree ("true");
    EXPECT_EQ(TOKEN_True, node->GetOperation());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LiteralExpressionTests, Utf8Expressions)
    {
    ECValue expectVal(true);

    // when a symbolset is passed in - published symbol providers will add their symbols to the context.
    SymbolExpressionContextPtr symbolContext = SymbolExpressionContext::Create(NULL);

    Utf8String expr("12.1 > 12");

    EvaluationResult result;
    ExpressionStatus status = ECEvaluator::EvaluateExpression(result, expr.c_str(), *symbolContext);
    EXPECT_SUCCESS(status);

    status = ECEvaluator::EvaluateExpression(result, "12.0 = 12.0", *symbolContext);
    EXPECT_SUCCESS(status);
    EXPECT_TRUE(result.IsECValue());
    EXPECT_TRUE(expectVal.Equals(*result.GetECValue()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LiteralExpressionTests, EmptySymbolSet)
    {
    TestExpressionEquals ("12.1 = 12", ECValue (false));
    TestExpressionEquals ("12.1 >= 12", ECValue (true));
    TestExpressionEquals ("12.1 <= 12", ECValue (false));
    TestExpressionEquals ("12.1 <> 12", ECValue (true));
    TestExpressionEquals ("12.1 > 12", ECValue (true));
    TestExpressionEquals ("12.1 < 12", ECValue (false));
    }

 /*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LiteralExpressionTests, MathSymbols)
    {
    bvector<Utf8String> requiredSymbolSets;

    // native ECExpression processing ignores the list of requiredSymbolSets and publishes all symbols from all symbol providers.
    TestExpressionEquals (requiredSymbolSets, "System.Math.AlmostEqual(System.Math.E, 2.71828182846)", ECValue(true));
    TestExpressionEquals (requiredSymbolSets, "System.Math.Acos(0.5) * 180.0 / System.Math.PI", ECValue(60.0));
    TestExpressionEquals (requiredSymbolSets, "System.Math.Asin(0.5) * 180.0 / System.Math.PI", ECValue(30.0));
    TestExpressionEquals (requiredSymbolSets, "System.Math.Atan(1) * 180 / System.Math.PI", ECValue(45.0));
    TestExpressionEquals (requiredSymbolSets, "System.Math.Atan2(10, -10) * 180/System.Math.PI", ECValue(135.0));
    TestExpressionEquals (requiredSymbolSets, "System.Math.AlmostEqual(System.Math.BigMul(2000000000, 2000000000), 4000000000000000000)", ECValue(true));
    TestExpressionEquals (requiredSymbolSets, "System.Math.Cos(60.0*System.Math.PI/180)", ECValue(0.5));
    TestExpressionEquals (requiredSymbolSets, "System.Math.Cosh(System.Math.Log(2.0))", ECValue(1.25));
    TestExpressionEquals (requiredSymbolSets, "System.Math.AlmostEqual(System.Math.Exp(5.0), 148.4131591025766)", ECValue(true));
    TestExpressionEquals (requiredSymbolSets, "System.Math.Abs(-2.5)", ECValue(2.5));
    TestExpressionEquals (requiredSymbolSets, "System.Math.Floor(-3.1)", ECValue(-4.0));
    TestExpressionEquals (requiredSymbolSets, "System.Math.Ceiling(-3.1)", ECValue(-3.0));
    TestExpressionEquals (requiredSymbolSets, "System.Math.AlmostEqual(System.Math.Log(5.5), 1.7047480922384253)", ECValue(true));
    TestExpressionEquals (requiredSymbolSets, "System.Math.Log10(1000)", ECValue(3.0));
    TestExpressionEquals (requiredSymbolSets, "System.Math.Max (-5.5, 5.0)", ECValue(5.0));
    TestExpressionEquals (requiredSymbolSets, "System.Math.Min (-5, 5)", ECValue(-5.0));
    TestExpressionEquals (requiredSymbolSets, "System.Math.Round (1.4)", ECValue(1.0));
    TestExpressionEquals (requiredSymbolSets, "System.Math.Round (1.6)", ECValue(2.0));
    TestExpressionEquals (requiredSymbolSets, "System.Math.Round (1.5)", ECValue(2.0));
    TestExpressionEquals (requiredSymbolSets, "System.Math.Round (2.5)", ECValue(2.0));
    TestExpressionEquals (requiredSymbolSets, "System.Math.Round (-1.4)", ECValue(-1.0));
    TestExpressionEquals (requiredSymbolSets, "System.Math.Round (-1.6)", ECValue(-2.0));
    TestExpressionEquals (requiredSymbolSets, "System.Math.Round (-1.5)", ECValue(-2.0));
    TestExpressionEquals (requiredSymbolSets, "System.Math.Round (-2.5)", ECValue(-2.0));
    TestExpressionEquals (requiredSymbolSets, "System.Math.Pow(7,3)", ECValue(343.0));
    TestExpressionEquals (requiredSymbolSets, "System.Math.AlmostEqual(System.Math.Pow(32.01,1.54), 208.03669140538651)", ECValue(true));
    TestExpressionEquals (requiredSymbolSets, "System.Math.AlmostEqual(System.Math.Sin(30*System.Math.PI/180.0),0.50)", ECValue(true));
    TestExpressionEquals (requiredSymbolSets, "System.Math.Sinh(System.Math.Log(2.0))", ECValue(0.75));
    TestExpressionEquals (requiredSymbolSets, "System.Math.Sqrt (1024.0)", ECValue(32.0));
    TestExpressionEquals (requiredSymbolSets, "System.Math.Tan(45.0*System.Math.PI/180.0)", ECValue(1.0));
    TestExpressionEquals (requiredSymbolSets, "System.Math.Tanh(System.Math.Log(2.0))", ECValue(0.6));
    TestExpressionEquals (requiredSymbolSets, "System.Math.IEEERemainder(3,2)", ECValue(-1.0));
    TestExpressionEquals (requiredSymbolSets, "System.Math.IEEERemainder(10, 3)", ECValue(1.0));
    TestExpressionEquals (requiredSymbolSets, "System.Math.IEEERemainder(17.8,4)", ECValue(1.8));
    TestExpressionEquals (requiredSymbolSets, "System.Math.IEEERemainder(17.8,4.1)", ECValue(1.4));
    TestExpressionEquals (requiredSymbolSets, "System.Math.IEEERemainder(17.8,-4.1)", ECValue(1.4));
    TestExpressionEquals (requiredSymbolSets, "System.Math.IEEERemainder(-17.8,-4.1)", ECValue(-1.4));
    }

 /*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LiteralExpressionTests, StringSymbols)
    {
    bvector<Utf8String> requiredSymbolSets;

    // native ECExpression processing ignores the list of requiredSymbolSets and publishes all symbols from all symbol providers.
    TestExpressionEquals (requiredSymbolSets, "System.String.ToUpper(\"loweR\")", ECValue("LOWER"));
    TestExpressionEquals (requiredSymbolSets, "System.String.ToLower(\"LOwEr\")", ECValue("lower"));
    TestExpressionEquals (requiredSymbolSets, "System.String.IndexOf(\"squid SQUID SQUID squid\", \"QUID\")", ECValue(7));
    TestExpressionEquals (requiredSymbolSets, "System.String.LastIndexOf(\"squid SQUID SQUID squid\", \"QUID\")", ECValue(13));
    TestExpressionEquals (requiredSymbolSets, "System.String.Length(\"12345678\")", ECValue(8));
    TestExpressionEquals (requiredSymbolSets, "System.String.SubString(\"dogCATdog\", 3, 3)", ECValue("CAT"));
    TestExpressionEquals (requiredSymbolSets, "System.String.Trim(\"  is \t trimmed\t\t\n\")", ECValue("is \t trimmed"));
    TestExpressionEquals (requiredSymbolSets, "IIf(System.String.Contains(\"thing\", \"in\"), \"true\", \"false\")", ECValue("true"));
    TestExpressionEquals (requiredSymbolSets, "IIf(System.String.Contains(\"thing\", \"In\"), \"true\", \"false\")", ECValue("false"));

    TestExpressionEquals (requiredSymbolSets, "IIf(System.String.ContainsI(\"thing\",\"In\"),\"true\",\"false\")",   ECValue("true"));
    TestExpressionEquals (requiredSymbolSets, "IIf(System.String.Compare(\"thing\",\"thing\"),\"true\",\"false\")",  ECValue("true"));
    TestExpressionEquals (requiredSymbolSets, "IIf(System.String.Compare(\"thing\",\"THING\"),\"true\",\"false\")",  ECValue("false"));
    TestExpressionEquals (requiredSymbolSets, "IIf(System.String.CompareI(\"thing\",\"THING\"),\"true\",\"false\")", ECValue("true"));
    }

 /*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LiteralExpressionTests, MiscSymbols)
    {
    bvector<Utf8String> requiredSymbolSets;  // to have expression processing use published symbols requiredSymbolSets must be passed in, even if empty

    // datetime
    EvaluationResult result;

    ExpressionStatus status = EvaluateExpression (result, "System.DateTime.Now()", requiredSymbolSets, nullptr);
    EXPECT_SUCCESS (status);
    if (result.IsECValue ())
        {
        DateTime now = DateTime::GetCurrentTimeUtc();
        EXPECT_EQ (now.GetYear (), result.GetECValue ()->GetDateTime ().GetYear ());
        }

#if defined (BENTLEYCONFIG_OS_WINDOWS) // Windows && WinRT
    #define TEST_PATH_SEP "\\"
    // path
    TestExpressionEquals(requiredSymbolSets, "System.Path.GetDirectoryName(\"c:\\dir\\subdir\\filename.ext\")", ECValue("c:" TEST_PATH_SEP "dir" TEST_PATH_SEP "subdir"));
    TestExpressionEquals(requiredSymbolSets, "System.Path.GetExtension(\"c:\\dir\\subdir\\filename.ext\")", ECValue(".ext"));
    TestExpressionEquals(requiredSymbolSets, "System.Path.GetFileNameWithoutExtension(\"c:\\dir\\subdir\\filename.ext\")", ECValue("filename"));
    TestExpressionEquals(requiredSymbolSets, "System.Path.GetFileName(\"c:\\dir\\subdir\\filename.ext\")", ECValue("filename.ext"));
    TestExpressionEquals(requiredSymbolSets, "System.Path.Combine (\"c:\\dir\")", ECValue("c:" TEST_PATH_SEP "dir"));
    TestExpressionEquals(requiredSymbolSets, "System.Path.Combine (\"c:\\dir\", \"subdir\")", ECValue("c:" TEST_PATH_SEP "dir" TEST_PATH_SEP "subdir"));
    TestExpressionEquals(requiredSymbolSets, "System.Path.Combine (\"c:\\dir\", \"subdir\\\", \"filename.ext\")", ECValue("c:" TEST_PATH_SEP "dir" TEST_PATH_SEP "subdir" TEST_PATH_SEP "filename.ext"));

    Utf8String fileName(L"c:\\dir\\subdir\\filename.ext");
    auto instance = CreateInstanceA(fileName);       // set "s" property
    TestExpressionEquals(requiredSymbolSets, "System.Path.GetDirectoryName(this.s)", ECValue("c:" TEST_PATH_SEP "dir" TEST_PATH_SEP "subdir"), instance.get());
    TestExpressionEquals(requiredSymbolSets, "System.Path.GetExtension(this.s)", ECValue(".ext"), instance.get());
    TestExpressionEquals(requiredSymbolSets, "System.Path.GetFileNameWithoutExtension(this.s)", ECValue("filename"), instance.get());
    TestExpressionEquals(requiredSymbolSets, "System.Path.GetFileName(this.s)", ECValue("filename.ext"), instance.get());

#else
    #define TEST_PATH_SEP L"/"
    // path
    TestExpressionEquals (requiredSymbolSets, "System.Path.GetDirectoryName(\"\\dir\\subdir\\filename.ext\")",             ECValue(TEST_PATH_SEP L"dir" TEST_PATH_SEP L"subdir"));
    TestExpressionEquals (requiredSymbolSets, "System.Path.GetExtension(\"\\dir\\subdir\\filename.ext\")",                 ECValue(L".ext"));
    TestExpressionEquals (requiredSymbolSets, "System.Path.GetFileNameWithoutExtension(\"\\dir\\subdir\\filename.ext\")",  ECValue(L"filename"));
    TestExpressionEquals (requiredSymbolSets, "System.Path.GetFileName(\"\\dir\\subdir\\filename.ext\")",                  ECValue(L"filename.ext"));
    TestExpressionEquals (requiredSymbolSets, "System.Path.Combine (\"\\dir\")",                                           ECValue(TEST_PATH_SEP L"dir"));
    TestExpressionEquals (requiredSymbolSets, "System.Path.Combine (\"\\dir\", \"subdir\")",                               ECValue(TEST_PATH_SEP L"dir" TEST_PATH_SEP L"subdir"));
    TestExpressionEquals (requiredSymbolSets, "System.Path.Combine (\"\\dir\", \"subdir\\\", \"filename.ext\")",           ECValue(TEST_PATH_SEP L"dir" TEST_PATH_SEP L"subdir" TEST_PATH_SEP L"filename.ext"));

    Utf8String fileName (L"\\dir\\subdir\\filename.ext");
    auto instance = CreateInstanceA(fileName);       // set "s" property
    TestExpressionEquals (requiredSymbolSets, "System.Path.GetDirectoryName(this.s)",             ECValue(TEST_PATH_SEP L"dir" TEST_PATH_SEP L"subdir"), instance.get());
    TestExpressionEquals (requiredSymbolSets, "System.Path.GetExtension(this.s)",                 ECValue(L".ext"),            instance.get());
    TestExpressionEquals (requiredSymbolSets, "System.Path.GetFileNameWithoutExtension(this.s)",  ECValue(L"filename"),        instance.get());
    TestExpressionEquals (requiredSymbolSets, "System.Path.GetFileName(this.s)",                  ECValue(L"filename.ext"),    instance.get());
#endif

#undef TEST_PATH_SEP
    }


/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct FullyQualifiedExpressionTests : InstanceExpressionTests
    {
    virtual Utf8String GetTestSchemaXMLString() override
        {
        Utf8Char fmt[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                        "    <ECClass typeName=\"ClassA\" displayLabel=\"Class A\" isDomainClass=\"True\">"
                        "        <ECProperty propertyName=\"p\" typeName=\"int\" />"
                        "    </ECClass>"
                        "    <ECClass typeName=\"ClassB\" displayLabel=\"Class B\" isDomainClass=\"True\">"
                        "        <ECProperty propertyName=\"p\" typeName=\"int\" />"
                        "        <ECProperty propertyName=\"b\" typeName=\"int\" />"
                        "    </ECClass>"
                        "    <ECClass typeName=\"DerivesFromA\" displayLabel=\"Derives From A\" isDomainClass=\"True\">"
                        "        <BaseClass>ClassA</BaseClass>"
                        "        <ECProperty propertyName=\"p2\" typeName=\"int\" />"
                        "    </ECClass>"
                        "</ECSchema>";

        return fmt;
        }
    };

/*---------------------------------------------------------------------------------**//**
* In Vancouver, in native ECExpressions, we introduced support for fully-qualified
* property accessors using syntax like "this.SchemaName::ClassName::AccessString".
* It should work polymorphically and should not evaluate for an instance not of the
* specified class, even if that instance has a property matching AccessString.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (FullyQualifiedExpressionTests, FullyQualifiedAccessors)
    {
    IECInstancePtr A = CreateInstance ("ClassA"),
                   B = CreateInstance ("ClassB"),
                   ADerived = CreateInstance ("DerivesFromA");

    EvaluationResult result;

    // ClassA contains 'p'
    EXPECT_SUCCESS (EvaluateExpression (result, "this.TestSchema::ClassA::p", *A));
    // ClassB contains 'p' but we have specified ClassA
    EXPECT_ERROR (EvaluateExpression (result, "this.TestSchema::ClassA::p", *B));
    // ClassB contains 'p' and we have specified ClassB
    EXPECT_SUCCESS (EvaluateExpression (result, "this.TestSchema::ClassB::p", *B));
    // ClassA contains 'p' and DerivesFromA is a subclass - we can find it whether we specify base or derived class
    EXPECT_SUCCESS (EvaluateExpression (result, "this.TestSchema::ClassA::p", *ADerived));
    EXPECT_SUCCESS (EvaluateExpression (result, "this.TestSchema::DerivesFromA::p", *ADerived));
    // ClassA contains 'p' but we have specified a subclass - should not find it
    EXPECT_ERROR (EvaluateExpression (result, "this.TestSchema::DerivesFromA::p", *A));
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceListExpressionTests : InstanceExpressionTests
    {
    virtual Utf8String GetTestSchemaXMLString() override
        {
        return      "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                    "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                    "    <ECClass typeName=\"Struct1\" isStruct=\"True\">"
                    "        <ECArrayProperty propertyName=\"Ints\" typeName=\"int\" />"
                    "        <ECProperty propertyName=\"Int\" typeName=\"int\" />"
                    "    </ECClass>"
                    "    <ECClass typeName=\"Struct2\" isStruct=\"True\">"
                    "        <ECArrayProperty propertyName=\"Structs\" typeName=\"Struct1\" isStruct=\"True\" />"
                    "        <ECStructProperty propertyName=\"Struct\" typeName=\"Struct1\" />"
                    "    </ECClass>"
                    "    <ECClass typeName=\"ClassA\" isDomainClass=\"True\">"
                    "        <ECStructProperty propertyName=\"Struct\" typeName=\"Struct2\" />"
                    "        <ECArrayProperty propertyName=\"Structs\" typeName=\"Struct2\" isStruct=\"True\" />"
                    "        <ECProperty propertyName=\"Int\" typeName=\"int\" />"
                    "        <ECArrayProperty propertyName=\"Ints\" typeName=\"int\" />"
                    "        <ECProperty propertyName=\"String\" typeName=\"string\" />"
                    "    </ECClass>"
                    "    <ECClass typeName=\"DerivedA\" isDomainClass=\"True\">"
                    "        <BaseClass>ClassA</BaseClass>"
                    "        <ECProperty propertyName=\"DerivedInt\" typeName=\"int\" />"
                    "    </ECClass>"
                    "</ECSchema>";
        }

    void    AddArrayElement (IECInstanceR instance, Utf8CP accessString, ECValueCR entryVal)
        {
        ECValue arrayVal;
        EXPECT_TRUE (ECObjectsStatus::Success == instance.GetValue (arrayVal, accessString));
        uint32_t index = arrayVal.GetArrayInfo().GetCount();
        EXPECT_TRUE (ECObjectsStatus::Success == instance.AddArrayElements (accessString, 1));
        EXPECT_TRUE (ECObjectsStatus::Success == instance.SetValue (accessString, entryVal, index));
        }
    };

/*---------------------------------------------------------------------------------**//**
* Test that the logic for evaluating property values in context of instance list behaves
* as expected for complex expressions involving embedded structs and nested struct arrays.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (InstanceListExpressionTests, ComplexExpressions)
    {
    IECInstancePtr s1 = CreateInstance ("Struct1");
    AddArrayElement (*s1, "Ints", ECValue (1));
    s1->SetValue ("Int", ECValue (2));

    ECValue v;
    IECInstancePtr s2 = CreateInstance ("Struct2");
    s2->SetValue ("Struct.Int", ECValue (3));
    AddArrayElement (*s2, "Struct.Ints", ECValue (4));

    v.SetStruct (s1.get());
    AddArrayElement (*s2, "Structs", v);

    IECInstancePtr a = CreateInstance ("ClassA");
    a->SetValue ("Int", ECValue (5));
    AddArrayElement (*a, "Ints", ECValue (6));

    v.SetStruct (s2.get());
    AddArrayElement (*a, "Structs", v);

    s1 = CreateInstance ("Struct1");
    AddArrayElement (*s1, "Ints", ECValue (7));
    s1->SetValue ("Int", ECValue (8));

    v.SetStruct (s1.get());
    AddArrayElement (*a, "Struct.Structs", v);

    a->SetValue ("Struct.Struct.Int", ECValue (9));
    AddArrayElement (*a, "Struct.Struct.Ints", ECValue (10));

    static Utf8CP  s_expressions[10] =
        {
        "this.Structs[0].Structs[0].Ints[0]",
        "this.Structs[0].Structs[0].Int",
        "this.Structs[0].Struct.Int",
        "this.Structs[0].Struct.Ints[0]",
        "this.Int",
        "this.Ints[0]",
        "this.Struct.Structs[0].Ints[0]",
        "this.Struct.Structs[0].Int",
        "this.Struct.Struct.Int",
        "this.Struct.Struct.Ints[0]"
        };

    for (size_t i = 0; i < _countof(s_expressions); i++)
        {
        Utf8CP expr = s_expressions[i];
        EvaluationResult result;
        ExpressionStatus status = EvaluateExpression (result, expr, *a);
        EXPECT_SUCCESS (status);
        if (ExpressionStatus::Success != status)
            continue;

        EXPECT_TRUE (result.IsECValue());
        EXPECT_EQ (result.GetECValue()->GetInteger(), (int32_t)i+1);
        }

#ifdef NEEDSWORK_COMPLEX_ACCESS_STRINGS
    // Doesn't work for expressions like 'this["Struct.Member"]'
    static Utf8CP s_bracketExpressions[10] =
        {
        "this[\"Structs\"][0][\"Structs\"][0][\"Ints\"][0]",
        "this[\"Structs\"][0][\"Structs\"][0][\"Int\"]",
        "this[\"Structs\"][0][\"Struct.Int\"]",
        "this[\"Structs\"][0][\"Struct.Ints\"][0]",
        "this[\"Int\"]",
        "this[\"Ints\"][0]",
        "this[\"Struct.Structs\"][0][\"Ints\"][0]",
        "this[\"Struct.Structs\"][0][\"Int\"]",
        "this[\"Struct.Struct.Int\"]",
        "this[\"Struct.Struct.Ints\"][0]"
        };

    for (size_t i = 0; i < _countof(s_bracketExpressions); i++)
        {
        Utf8CP expr = s_bracketExpressions[i];
        EvaluationResult result;
        ExpressionStatus status = EvaluateExpression (result, expr, *a);
        EXPECT_SUCCESS (status);
        if (ExpressionStatus::Success != status)
            continue;

        EXPECT_TRUE (result.IsECValue());
        EXPECT_EQ (result.GetECValue()->GetInteger(), (int32_t)i+1);
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct ArrayExpressionTests : InstanceListExpressionTests
    {
    IValueListResultPtr     GetIValueList (IECInstanceR instance, Utf8CP expr)
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

    static ExpressionStatus     SumArrayMembers (EvaluationResultR result, void* context, IValueListResultCR valueList, EvaluationResultVector& args)
        {
        uint32_t count = valueList.GetCount();
        int32_t sum = 0;
        ExpressionStatus status = ExpressionStatus::Success;

        for (uint32_t i = 0; i < count; i++)
            {
            EvaluationResult member;
            status = valueList.GetValueAt (member, i);
            if (ExpressionStatus::Success == status && member.IsECValue() && member.GetECValue()->IsInteger())
                sum += member.GetECValue()->GetInteger();
            }

        if (ExpressionStatus::Success == status)
            result.InitECValue().SetInteger (sum);

        return status;
        }

    virtual void PublishSymbols (SymbolExpressionContextR context) override
        {
        context.AddSymbol (*MethodSymbol::Create ("SumArray", &SumArrayMembers));
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArrayExpressionTests, ArrayProperties)
    {
    IECInstancePtr s1 = CreateInstance ("Struct1");
    AddArrayElement (*s1, "Ints", ECValue (1));
    AddArrayElement (*s1, "Ints", ECValue (2));

    IECInstancePtr s2 = CreateInstance ("Struct2");
    ECValue v;
    v.SetStruct (s1.get());
    AddArrayElement (*s2, "Structs", v);

    IECInstancePtr a = CreateInstance ("ClassA");
    for (uint32_t i = 0; i < 5; i++)
        {
        v.SetInteger ((int32_t)i);
        AddArrayElement (*a, "Ints", v);
        }

    v.SetStruct (s2.get());
    AddArrayElement (*a, "Structs", v);

    // -- Test querying the value lists directly --

    IValueListResultPtr list = GetIValueList (*a, "this.Ints");
    TestIValueList (list.get(), 5);

    list = GetIValueList (*a, "this.Structs");
    TestIValueList (list.get(), 1);

    list = GetIValueList (*a, "this.Struct.Structs");
    TestIValueList (list.get(), 0);

    list = GetIValueList (*a, "this.Structs[0].Structs");
    TestIValueList (list.get(), 1);

    list = GetIValueList (*a, "this.Structs[0].Structs[0].Ints");
    TestIValueList (list.get(), 2);

    // -- Test Count, First, Last properties --

    TestExpressionEquals (*a, "this.Ints.Count", 5);
    TestExpressionEquals (*a, "this.Structs.Count", 1);
    TestExpressionEquals (*a, "this.Struct.Structs.Count", 0);
    TestExpressionEquals (*a, "this.Structs[0].Structs.Count", 1);
    TestExpressionEquals (*a, "this.Structs[0].Structs[0].Ints.Count", 2);

    TestExpressionEquals (*a, "this.Ints.First", 0);
    TestExpressionEquals (*a, "this.Ints.Last", 4);

    TestExpressionEquals (*a, "this.Structs[0].Structs[0].Ints.First", 1);
    TestExpressionEquals (*a, "this.Structs[0].Structs[0].Ints.Last", 2);

    // First and Last return null for an empty array (but not an error!)
    TestExpressionNull (*a, "this.Struct.Structs.First");
    TestExpressionNull (*a, "this.Struct.Structs.Last");

    // Can't really test struct array values for equality, but can test not null
    TestExpressionNotNull (*a, "this.Structs.First");
    TestExpressionNotNull (*a, "this.Structs.Last");

    // -- ###TODO Test Any() and All() methods --

    //EvaluationResult result;
    //EXPECT_SUCCESS (EvaluateExpression (result, L"this.Structs.Any(\"args...\")", *a));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArrayExpressionTests, ArrayMethods)
    {
    IECInstancePtr a = CreateInstance ("ClassA");
    int32_t expectedSum = 0;
    for (int32_t i = 0; i < 4; i++)
        {
        expectedSum += i;
        AddArrayElement (*a, "Ints", ECValue (i));
        }

    TestExpressionEquals (*a, "this.Ints.SumArray()", expectedSum);
    TestExpressionEquals (*a, "this.Ints.SumArray() * this.Ints.SumArray()", expectedSum*expectedSum);
    }

/*---------------------------------------------------------------------------------**//**
* In a case where two instances in the list can satisfy a property accessor, the value is
* obtained from the first matching instance found.
* Nobody should write expressions that rely on this behavior but we do want to test that
* it behaves as expected.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (InstanceListExpressionTests, FirstMatchWins)
    {
    IECInstancePtr a1 = CreateInstance("ClassA");
    a1->SetValue ("Int", ECValue (1));
    IECInstancePtr a2 = CreateInstance ("ClassA");
    a2->SetValue ("Int", ECValue (2));

    ECInstanceList instances;
    instances.push_back (a1);
    instances.push_back (a2);

    EvaluationResult result;
    EXPECT_SUCCESS (EvaluateExpression (result, "this.Int", instances));
    EXPECT_TRUE (result.IsECValue());
    EXPECT_EQ (result.GetECValue()->GetInteger(), 1);

    instances.clear();
    instances.push_back (a2);
    instances.push_back (a1);

    EXPECT_SUCCESS (EvaluateExpression (result, "this.Int", instances));
    EXPECT_TRUE (result.IsECValue());
    EXPECT_EQ (result.GetECValue()->GetInteger(), 2);

    // using fully-qualified property names can help resolve the correct instance
    IECInstancePtr s1 = CreateInstance ("Struct1");
    s1->SetValue ("Int", ECValue (3));

    instances.clear();
    instances.push_back (s1);
    instances.push_back (a1);

    EXPECT_SUCCESS (EvaluateExpression (result, "this.TestSchema::ClassA::Int", instances));
    EXPECT_EQ (result.GetECValue()->GetInteger(), 1);
    EXPECT_SUCCESS (EvaluateExpression (result, "this.TestSchema::Struct1::Int", instances));
    EXPECT_EQ (result.GetECValue()->GetInteger(), 3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct MethodsReturningInstancesTests : InstanceListExpressionTests
    {
    static ECSchemaP        s_schema;

    virtual void PublishSymbols (SymbolExpressionContextR context)
        {
        // NEEDSWORK? Cannot invoke methods on instance lists returned by static methods, only by instance methods...doesn't seem right...
        context.AddSymbol (*MethodSymbol::Create ("CreateInstanceA", NULL, &CreateInstanceA));
        context.AddSymbol (*MethodSymbol::Create ("CreateInstancesA", NULL, &CreateInstancesA));
        }

    static ExpressionStatus     CreateInstanceA (EvaluationResultR result, void* context, ECInstanceListCR, EvaluationResultVector& args)
        {
        IECInstancePtr instance = CreateInstance ("ClassA", *s_schema);
        instance->SetValue ("String", ECValue ("A"));
        result.SetInstance (*instance);
        return ExpressionStatus::Success;
        }
    static ExpressionStatus     CreateInstancesA (EvaluationResultR result, void* context, ECInstanceListCR, EvaluationResultVector& args)
        {
        ECInstanceList instances;
        IECInstancePtr a = CreateInstance ("ClassA", *s_schema);
        a->SetValue ("String", ECValue ("A1"));
        instances.push_back (a);

        a = CreateInstance ("DerivedA", *s_schema);
        a->SetValue ("String", ECValue ("A2"));
        a->SetValue ("DerivedInt", ECValue (2));
        instances.push_back (a);

        result.SetInstanceList (instances, true);
        return ExpressionStatus::Success;
        }
    };

ECSchemaP MethodsReturningInstancesTests::s_schema = NULL;

/*---------------------------------------------------------------------------------**//**
* An ECExpression method can return 1 or multiple instances. Test both.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (MethodsReturningInstancesTests, InstanceMethods)
    {
    s_schema = &GetSchema();
    IECInstancePtr dummy = CreateInstance ("ClassA");

    EvaluationResult result;
    EXPECT_SUCCESS (EvaluateExpression (result, "this.CreateInstanceA()", *dummy));
    EXPECT_TRUE (result.IsInstanceList());
    EXPECT_EQ (1, result.GetInstanceList()->size());

    EXPECT_SUCCESS (EvaluateExpression (result, "this.CreateInstancesA()", *dummy));
    EXPECT_TRUE (result.IsInstanceList());
    EXPECT_EQ (2, result.GetInstanceList()->size());

    EXPECT_SUCCESS (EvaluateExpression (result, "this.CreateInstanceA().String", *dummy));
    EXPECT_TRUE (result.IsECValue());
    EXPECT_EQ (0, strcmp (result.GetECValue()->GetUtf8CP(), "A"));

    EXPECT_SUCCESS (EvaluateExpression (result, "this.CreateInstancesA()[\"String\"]", *dummy));
    EXPECT_TRUE (result.IsECValue());
    EXPECT_EQ (0, strcmp (result.GetECValue()->GetUtf8CP(), "A1"));

    EXPECT_SUCCESS (EvaluateExpression (result, "this.CreateInstancesA().DerivedInt", *dummy));
    EXPECT_TRUE (result.IsECValue());
    EXPECT_EQ (result.GetECValue()->GetInteger(), 2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct ExpressionRemappingTest : ECTestFixture, IECSchemaRemapper
    {
private:
    ECSchemaPtr     m_preSchema;
    ECSchemaPtr     m_postSchema;

    ECSchemaPtr     CreateSchema (bool rename);

    Utf8String         GetName (Utf8CP name, bool rename) const
        {
        Utf8String newName (name);
        if (rename)
            newName.ToUpper();

        return newName;
        }

    Utf8String         GetOldName (Utf8CP name) const
        {
        Utf8String oldName (name);
        oldName.ToLower();
        return oldName;
        }

    virtual bool    _ResolveClassName (Utf8StringR className, ECSchemaCR schema) const override
        {
        if (&schema != m_postSchema.get() || nullptr == m_preSchema->GetClassCP (GetOldName (className.c_str()).c_str()))
            return false;

        className.ToUpper();
        return true;
        }
    virtual bool    _ResolvePropertyName (Utf8StringR propName, ECClassCR ecClass) const override
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

    void            Expect (Utf8CP input, Utf8CP output)
        {
        NodePtr expr = ECEvaluator::ParseValueExpressionAndCreateTree (input);
        EXPECT_NOT_NULL (expr.get());

        Utf8String str = expr->ToExpressionString();
        EXPECT_TRUE (str.Equals (input)) << "Input: " << input << " Round-tripped: " << str.c_str();

        bool anyRemapped = expr->Remap (*m_preSchema, *m_postSchema, *this);
        EXPECT_EQ (anyRemapped, 0 != strcmp (input, output));

        str = expr->ToExpressionString();
        EXPECT_TRUE (str.Equals (output)) << "Expected: " << output << " Actual: " << str.c_str() << " Input: " << input;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr ExpressionRemappingTest::CreateSchema (bool rename)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema (schema, GetName ("schema", rename), "testAlias", 1, 0, 0);

    ECStructClassP structClass;
    schema->CreateStructClass (structClass, GetName ("struct", rename));

    ECStructClassP subStructClass;
    schema->CreateStructClass(subStructClass, GetName ("substruct", rename));

    PrimitiveECPropertyP primProp;
    subStructClass->CreatePrimitiveProperty (primProp, GetName ("bool", rename), PRIMITIVETYPE_Boolean);

    StructECPropertyP structProp;
    structClass->CreateStructProperty (structProp, GetName ("substruct", rename), *subStructClass);
    structClass->CreatePrimitiveProperty (primProp, GetName ("int", rename), PRIMITIVETYPE_Integer);

    ECEntityClassP ecClass;
    schema->CreateEntityClass (ecClass, GetName ("class", rename));

    ecClass->CreatePrimitiveProperty (primProp, GetName ("string", rename), PRIMITIVETYPE_String);

    PrimitiveArrayECPropertyP arrayProp;
    ecClass->CreatePrimitiveArrayProperty (arrayProp, GetName ("doubles", rename), PRIMITIVETYPE_Double);

    StructArrayECPropertyP structArrayProp;
    ecClass->CreateStructArrayProperty (structArrayProp, GetName ("structs", rename), *structClass);
    ecClass->CreateStructProperty (structProp, GetName ("struct", rename), *structClass);

    return schema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ExpressionRemappingTest, Remap)
    {
    static Utf8CP s_expressions[][2] =
        {
            { "this.schema::class::string", "this.SCHEMA::CLASS::STRING" },
            { "this.schema::class::doubles[0]", "this.SCHEMA::CLASS::DOUBLES[0]" },
            { "this.schema::class::struct.int", "this.SCHEMA::CLASS::STRUCT.INT" },
            { "this.schema::class::struct.substruct.bool", "this.SCHEMA::CLASS::STRUCT.SUBSTRUCT.BOOL" },
            { "this.schema::class::structs[0].int", "this.SCHEMA::CLASS::STRUCTS[0].INT" },
            { "this.schema::class::structs[0].substruct.bool", "this.SCHEMA::CLASS::STRUCTS[0].SUBSTRUCT.BOOL" },
            { "System.String.CompareI(this.DgnCustomItemTypes_Schema::Class::Struct.Prop,\"abc\")", "System.String.CompareI(this.DgnCustomItemTypes_Schema::Class::Struct.Prop,\"abc\")" },
        };

    for (auto const& pair : s_expressions)
        Expect (pair[0], pair[1]);
    }

END_BENTLEY_ECN_TEST_NAMESPACE
