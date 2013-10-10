/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/ECExpressionTests.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
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

    ExpressionStatus    EvaluateExpression (EvaluationResult& result, WCharCP expr, IECInstanceR instance)
        {
        InstanceExpressionContextPtr context = InstanceExpressionContext::Create (NULL);
        context->SetInstance (instance);
        SymbolExpressionContextPtr symbolContext = SymbolExpressionContext::Create (NULL);
        ContextSymbolPtr instanceSymbol = ContextSymbol::CreateContextSymbol (L"this", *context);
        symbolContext->AddSymbol (*instanceSymbol);

        NodePtr tree = ECEvaluator::ParseValueExpressionAndCreateTree (expr);
        EXPECT_NOT_NULL (tree.get());

        return tree->GetValue (result, *symbolContext, true, true);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ExpressionTests, Roundtrip)
    {
    Roundtrip (L"1 + 2 * 3 / 4 + 5 - 6 ^ 7 + -8 / -9 + +10", L"1+2*3/4+5-6^7+-8/-9++10");
    Roundtrip (L"this.Property", L"this.Property");
    Roundtrip (L"this.Property *  this[\"Property\"]", L"this.Property*this[\"Property\"]");
    Roundtrip (L"Something.Method ( )", L"Something.Method()");
    Roundtrip (L"Something.Method (0,1.5, 2.000,  \t\"string\", this.Property   )", L"Something.Method(0,1.5,2,\"string\",this.Property)");
    Roundtrip (L"IIf (True,  Null, \t2 ^3  -3* 4)", L"IIf(True,Null,2^3-3*4)");

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
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct ExpressionSchemaTests : ExpressionTests
    {
protected:
    ECSchemaPtr         m_schema;
public:
    static WString      GetTestSchemaXMLString ()
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
        ECClassCP ecClass = GetSchema().GetClassCP (classname);
        return NULL != ecClass ? ecClass->GetDefaultStandaloneEnabler()->CreateInstance() : NULL;
        }
    };

/*---------------------------------------------------------------------------------**//**
* In Vancouver, in native ECExpressions, we introduced support for fully-qualified
* property accessors using syntax like "this.SchemaName::ClassName::AccessString".
* It should work polymorphically and should not evaluate for an instance not of the
* specified class, even if that instance has a property matching AccessString.
* @bsimethod                                                    Paul.Connelly   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ExpressionSchemaTests, FullyQualifiedAccessors)
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

END_BENTLEY_ECOBJECT_NAMESPACE

