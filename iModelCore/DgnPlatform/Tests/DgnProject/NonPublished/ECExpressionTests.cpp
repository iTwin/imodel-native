/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/DgnProject/NonPublished/ECExpressionTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnPlatformApi.h>
#include "DgnHandlersTests.h"
#include <Bentley/BeTimeUtilities.h>
#include <Logging/bentleylogging.h>

#include <ECObjects/ECInstance.h>
#include <ECObjects/StandaloneECInstance.h>
#include <ECObjects/ECValue.h>
#include <ECObjects/ECExpressions.h>

#include <algorithm>

#define EXPECT_SUCCESS(EXPR) EXPECT_TRUE(SUCCESS == (EXPR))
#define EXPECT_ERROR(EXPR) EXPECT_FALSE(SUCCESS == (EXPR))
#define EXPECT_NOT_NULL(EXPR) EXPECT_FALSE(NULL == (EXPR))
#define EXPECT_NULL(EXPR) EXPECT_TRUE(NULL == (EXPR))

USING_NAMESPACE_EC


using namespace std;

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgpExpressionTests : public ::testing::Test
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
        if (SUCCESS == status)
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
        if (SUCCESS == status)
            {
            EXPECT_TRUE (result.IsECValue());
            if (result.IsECValue())
                {
                if (expectVal.IsString())
                    EXPECT_TRUE (expectVal.ToString().Equals (result.GetECValue()->ToString())) << "Expected: " << expectVal.ToString().c_str() << " Actual: " << result.GetECValue()->ToString().c_str() << " Expr: " << expr;
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
        if (SUCCESS == status)
            {
            EXPECT_TRUE (result.IsECValue());
            if (result.IsECValue())
                {
                if (expectVal.IsString())
                    EXPECT_TRUE (expectVal.ToString().Equals (result.GetECValue()->ToString())) << "Expected: " << expectVal.ToString().c_str() << " Actual: " << result.GetECValue()->ToString().c_str() << " Expr: " << expr;
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
        if (SUCCESS == status)
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
* @bsimethod                                                    Paul.Connelly   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgpRoundtripExpressionTests : DgpExpressionTests
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
* @bsimethod                                                    Paul.Connelly   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgpRoundtripExpressionTests, Roundtrip)
    {
    Roundtrip ("1 + 2 * 3 / 4 + 5 - 6 ^ 7 + -8 / -9 + +10", "1+2*3/4+5-6^7+-8/-9++10");
    Roundtrip ("this.Property", "this.Property");
    Roundtrip ("this.Property *  this[\"Property\"]", "this.Property*this[\"Property\"]");
    Roundtrip ("Something.Method ( )", "Something.Method()");
    Roundtrip ("Something.Method (0,1.5, 2.000,  \t\"string\", this.Property   )", "Something.Method(0,1.5,2,\"string\",this.Property)");
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

    Roundtrip ("X => X < 5.0 AndAlso X > 1.5", "X=>X<5 AndAlso X>1.5");
    Roundtrip ("this.Array.Any (X => X.Name = \"Chuck\" OrElse X.Name = \"Bob\")", "this.Array.Any(X=>X.Name=\"Chuck\"OrElse X.Name=\"Bob\")");
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgpInstanceExpressionTests : DgpExpressionTests
    {
protected:
    ECSchemaPtr         m_schema;
public:
    virtual Utf8String GetTestSchemaXMLString() = 0;

    ECSchemaR           GetSchema()
        {
        if (m_schema.IsNull())
            {
            Utf8String schemaXMLString = GetTestSchemaXMLString();
            ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext();
            EXPECT_EQ (SUCCESS, ECSchema::ReadFromXmlString (m_schema, schemaXMLString.c_str(), *schemaContext));  
            }

        return *m_schema;
        }
    IECInstancePtr      CreateInstance (Utf8CP classname)
        {
        return CreateInstance (classname, GetSchema());
        }
    static IECInstancePtr CreateInstance (Utf8CP classname, ECSchemaCR schema)
        {
        ECClassCP ecClass = schema.GetClassCP (classname);
        return NULL != ecClass ? ecClass->GetDefaultStandaloneEnabler()->CreateInstance() : NULL;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgpLiteralExpressionTests : DgpInstanceExpressionTests
    {
public:
    virtual Utf8String GetTestSchemaXMLString() override
        {
        return
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
            "    <ECClass typeName=\"ClassA\" displayLabel=\"Class A\" isDomainClass=\"True\">"
            "        <ECProperty propertyName=\"d\" typeName=\"double\" />"
            "        <ECProperty propertyName=\"s\" typeName=\"string\" />"
            "    </ECClass>"
            "</ECSchema>";
        }

    IECInstancePtr  CreateInstance (double d)
        {
        auto instance = DgpInstanceExpressionTests::CreateInstance ("ClassA");
        instance->SetValue ("d", ECValue (d));
        return instance;
        }

    IECInstancePtr  CreateInstance (Utf8String& s)
        {
        auto instance = DgpInstanceExpressionTests::CreateInstance ("ClassA");
        instance->SetValue ("s", ECValue (s.c_str()));
        return instance;
        }

    };

/*---------------------------------------------------------------------------------**//**
* John was comparing floating point values for exact equality.
* @bsimethod                                                    Paul.Connelly   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgpLiteralExpressionTests, DgnFloatComparisons)
    {
    auto instance = CreateInstance (12.000000000001);
    TestExpressionEquals (*instance, "this.d = 12", ECValue (true));
    TestExpressionEquals (*instance, "this.d >= 12", ECValue (true));
    TestExpressionEquals (*instance, "this.d <= 12", ECValue (true));
    TestExpressionEquals (*instance, "this.d <> 12", ECValue (false));
    TestExpressionEquals (*instance, "this.d > 12", ECValue (false));
    TestExpressionEquals (*instance, "this.d < 12", ECValue (false));

    instance = CreateInstance (12.1);
    TestExpressionEquals (*instance, "this.d = 12", ECValue (false));
    TestExpressionEquals (*instance, "this.d >= 12", ECValue (true));
    TestExpressionEquals (*instance, "this.d <= 12", ECValue (false));
    TestExpressionEquals (*instance, "this.d <> 12", ECValue (true));
    TestExpressionEquals (*instance, "this.d > 12", ECValue (true));
    TestExpressionEquals (*instance, "this.d < 12", ECValue (false));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgpLiteralExpressionTests, DgnEmptySymbolSet)
    {
    TestExpressionEquals ("12.1 = 12", ECValue (false));
    TestExpressionEquals ("12.1 >= 12", ECValue (true));
    TestExpressionEquals ("12.1 <= 12", ECValue (false));
    TestExpressionEquals ("12.1 <> 12", ECValue (true));
    TestExpressionEquals ("12.1 > 12", ECValue (true));
    TestExpressionEquals ("12.1 < 12", ECValue (false));
    }

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgpLiteralExpressionTests, DgnMathSymbols)
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
* @bsimethod                                    Bill.Steinbock                  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgpLiteralExpressionTests, DgnStringSymbols)
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
* @bsimethod                                    Bill.Steinbock                  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgpLiteralExpressionTests, DgnMiscSymbols)
    {
    bvector<Utf8String> requiredSymbolSets;  // to have expression processing use published symbols requiredSymbolSets must be passed in, even if empty
    
    // datetime
    EvaluationResult result;

    ExpressionStatus status = EvaluateExpression (result, "System.DateTime.Now()", requiredSymbolSets, nullptr);
    EXPECT_SUCCESS (status);
    if (result.IsECValue ())
        {
        DateTime now = DateTime::GetCurrentTime ();
        EXPECT_EQ (now.GetYear (), result.GetECValue ()->GetDateTime ().GetYear ());
        }

#if defined (BENTLEYCONFIG_OS_WINDOWS) // Windows && WinRT
    #define DIRSEP            "\\"
#elif defined (BENTLEYCONFIG_OS_UNIX)
    #define DIRSEP            "/"
#else
    #error unknown platform
#endif

    // path
    TestExpressionEquals (requiredSymbolSets, "System.Path.GetDirectoryName(\"c:\\dir\\subdir\\filename.ext\")",             ECValue("c:" DIRSEP "dir" DIRSEP "subdir"));
    TestExpressionEquals (requiredSymbolSets, "System.Path.GetExtension(\"c:\\dir\\subdir\\filename.ext\")",                 ECValue(".ext"));
    TestExpressionEquals (requiredSymbolSets, "System.Path.GetFileNameWithoutExtension(\"c:\\dir\\subdir\\filename.ext\")",  ECValue("filename"));
    TestExpressionEquals (requiredSymbolSets, "System.Path.GetFileName(\"c:\\dir\\subdir\\filename.ext\")",                  ECValue("filename.ext"));
    TestExpressionEquals (requiredSymbolSets, "System.Path.Combine (\"c:\\dir\")",                                           ECValue("c:" DIRSEP "dir"));
    TestExpressionEquals (requiredSymbolSets, "System.Path.Combine (\"c:\\dir\", \"subdir\")",                               ECValue("c:" DIRSEP "dir" DIRSEP "subdir"));
    TestExpressionEquals (requiredSymbolSets, "System.Path.Combine (\"c:\\dir\", \"subdir\\\", \"filename.ext\")",           ECValue("c:" DIRSEP "dir" DIRSEP "subdir" DIRSEP "filename.ext"));

    Utf8String fileName ("c:\\dir\\subdir\\filename.ext");
    auto instance = CreateInstance (fileName);       // set "s" property
    TestExpressionEquals (requiredSymbolSets, "System.Path.GetDirectoryName(this.s)",             ECValue("c:" DIRSEP "dir" DIRSEP "subdir"), instance.get());
    TestExpressionEquals (requiredSymbolSets, "System.Path.GetExtension(this.s)",                 ECValue(".ext"),            instance.get());
    TestExpressionEquals (requiredSymbolSets, "System.Path.GetFileNameWithoutExtension(this.s)",  ECValue("filename"),        instance.get());
    TestExpressionEquals (requiredSymbolSets, "System.Path.GetFileName(this.s)",                  ECValue("filename.ext"),    instance.get());
    }


/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgpFullyQualifiedExpressionTests : DgpInstanceExpressionTests
    {
    virtual Utf8String GetTestSchemaXMLString() override
        {
        return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
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
        }
    };

/*---------------------------------------------------------------------------------**//**
* In Vancouver, in native ECExpressions, we introduced support for fully-qualified
* property accessors using syntax like "this.SchemaName::ClassName::AccessString".
* It should work polymorphically and should not evaluate for an instance not of the
* specified class, even if that instance has a property matching AccessString.
* @bsimethod                                                    Paul.Connelly   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgpFullyQualifiedExpressionTests, DgnFullyQualifiedAccessors)
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
* @bsistruct                                                    Paul.Connelly   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgpInstanceListExpressionTests : DgpInstanceExpressionTests
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
TEST_F (DgpInstanceListExpressionTests, DgnComplexExpressions)
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
        if (ExprStatus_Success != status)
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
        if (ExprStatus_Success != status)
            continue;

        EXPECT_TRUE (result.IsECValue());
        EXPECT_EQ (result.GetECValue()->GetInteger(), (int32_t)i+1);
        }
#endif
    }




