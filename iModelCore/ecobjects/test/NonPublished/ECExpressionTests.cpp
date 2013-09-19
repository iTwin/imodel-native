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

#define EXPECT_SUCCESS(EXPR) EXPECT_TRUE(SUCCESS == EXPR)

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

END_BENTLEY_ECOBJECT_NAMESPACE

