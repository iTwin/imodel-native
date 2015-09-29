/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECSqlBuilder_Tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
void AssertEquals (ECSqlBuilderCR lhs, ECSqlBuilderCR rhs, bool expectedIsEqual) 
{
Utf8String assertMessageStr;
assertMessageStr.Sprintf ("ECSqlBuilder comparison failed for LHS '%s' and RHS '%s'.", lhs.ToString ().c_str (), rhs.ToString ().c_str ());
Utf8CP assertMessage = assertMessageStr.c_str ();

if (expectedIsEqual)
    {
    EXPECT_TRUE (lhs == rhs) << assertMessage;
    EXPECT_TRUE (rhs == lhs) << assertMessage;
    EXPECT_FALSE (lhs != rhs) << assertMessage;
    EXPECT_FALSE (rhs != lhs) << assertMessage;
    }
else
    {
    EXPECT_FALSE (lhs == rhs) << assertMessage;
    EXPECT_FALSE (rhs == lhs) << assertMessage;
    EXPECT_TRUE (lhs != rhs) << assertMessage;
    EXPECT_TRUE (rhs != lhs) << assertMessage;
    }
}


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECSqlBuilderTests, ECSqlSelectBuilder_CopySemantics)
    {
    ECDbTestFixture::Initialize ();
    auto testSchema = ECDbTestUtility::ReadECSchemaFromDisk (L"StartupCompany.02.00.ecschema.xml");
    auto testClass = testSchema->GetClassCP ("AAA");

    ECSqlSelectBuilder b1;
    b1.Select ("bla").From (*testClass, "myalias").Where ("i = 123").OrderBy ("d").Limit ("40");
    ECSqlSelectBuilder b2 (b1);

    AssertEquals (b1, b2, true);

    ECSqlSelectBuilder b3;
    b3 = b1;
    AssertEquals (b1, b3, true);


    ECSqlBuilder* b4 = &b1;
    AssertEquals (b1, *dynamic_cast<ECSqlSelectBuilder*> (b4), true);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECSqlBuilderTests, ECSqlSelectBuilder_Comparisons)
    {
    ECDbTestFixture::Initialize ();

    auto testSchema = ECDbTestUtility::ReadECSchemaFromDisk (L"StartupCompany.02.00.ecschema.xml");
    auto testClass = testSchema->GetClassCP ("AAA");

        {
        //empty builders
        ECSqlSelectBuilder lhs;
        ECSqlSelectBuilder rhs;
        AssertEquals (lhs, rhs, true);

        //*** select clause ***
        lhs.SelectAll ();
        AssertEquals (lhs, rhs, false);

        rhs.Select ("t, l");
        AssertEquals (lhs, rhs, false);

        //different select clause -> not equal
        lhs = ECSqlSelectBuilder ();
        rhs = ECSqlSelectBuilder ();
        lhs.Select ("t, l");
        rhs.Select ("t, l, bo");
        AssertEquals (lhs, rhs, false);

        //upper case -> not equal
        lhs = ECSqlSelectBuilder ();
        rhs = ECSqlSelectBuilder ();
        lhs.Select ("t, l");
        rhs.Select ("T, L");
        AssertEquals (lhs, rhs, false);

        //mixed case -> not equal
        lhs = ECSqlSelectBuilder ();
        rhs = ECSqlSelectBuilder ();
        lhs.Select ("t, l");
        rhs.Select ("t, L");
        AssertEquals (lhs, rhs, false);

        lhs = ECSqlSelectBuilder ();
        rhs = ECSqlSelectBuilder ();
        lhs.Select ("t, l");
        rhs.Select ("[t], [l]");
        AssertEquals (lhs, rhs, false);

        //no blank after comma -> not equal
        lhs = ECSqlSelectBuilder ();
        rhs = ECSqlSelectBuilder ();
        lhs.Select ("t, l");
        rhs.Select ("t,l");
        AssertEquals (lhs, rhs, false);

        //*** from clause ***
        lhs = ECSqlSelectBuilder ();
        rhs = ECSqlSelectBuilder ();
        lhs.From (*testClass);
        rhs.From (*testClass);
        AssertEquals (lhs, rhs, true);

        //different IsPolymorphic
        lhs = ECSqlSelectBuilder ();
        rhs = ECSqlSelectBuilder ();
        lhs.From (*testClass, true);
        rhs.From (*testClass, false);
        AssertEquals (lhs, rhs, false);

        lhs = ECSqlSelectBuilder ();
        rhs = ECSqlSelectBuilder ();
        lhs.From (*testClass);
        auto anotherClass = testSchema->GetClassCP ("AAFoo");
        rhs.From (*anotherClass);
        AssertEquals (lhs, rhs, false);

        //*** where clause ***
        lhs = ECSqlSelectBuilder ();
        rhs = ECSqlSelectBuilder ();
        lhs.Where ("ECInstanceId = 1234");
        rhs.Where ("ECInstanceId = 4541234");
        AssertEquals (lhs, rhs, false);

        lhs = ECSqlSelectBuilder ();
        rhs = ECSqlSelectBuilder ();
        lhs.Where ("l = 1000");
        rhs.Where ("l = 1000");
        AssertEquals (lhs, rhs, true);

        lhs = ECSqlSelectBuilder ();
        rhs = ECSqlSelectBuilder ();
        lhs.Where ("l > i");
        rhs.Where ("l > i");
        AssertEquals (lhs, rhs, true);

        lhs = ECSqlSelectBuilder ();
        rhs = ECSqlSelectBuilder ();
        lhs.Where ("l > i and s = 'hello, willy' or i = 1");
        rhs.Where ("l > i and s = 'hello, willy' or i = 1");
        AssertEquals (lhs, rhs, true);

        //different case -> not equal
        lhs = ECSqlSelectBuilder ();
        rhs = ECSqlSelectBuilder ();
        lhs.Where ("l > i and s = 'hello, willy' or i = 1");
        rhs.Where ("l > i AND s = 'hello, willy' OR i = 1");
        AssertEquals (lhs, rhs, false);

        //different case -> not equal
        lhs = ECSqlSelectBuilder ();
        rhs = ECSqlSelectBuilder ();
        lhs.Where ("l > i and s = 'hello, willy' or i = 1");
        rhs.Where ("L > i and s = 'hello, willy' or i = 1");
        AssertEquals (lhs, rhs, false);

        //different case -> not equal
        lhs = ECSqlSelectBuilder ();
        rhs = ECSqlSelectBuilder ();
        lhs.Where ("l > i and s = 'hello, willy' or i = 1");
        rhs.Where ("l>i and s = 'hello, willy' or i = 1");
        AssertEquals (lhs, rhs, false);
        }

        //**** different builder types *****
        {
        ECSqlSelectBuilder lhs;
        ECSqlInsertBuilder rhs;
        AssertEquals (lhs, rhs, false);
        }

        {
        ECSqlSelectBuilder lhs;
        ECSqlUpdateBuilder rhs;
        AssertEquals (lhs, rhs, false);
        }

        {
        ECSqlSelectBuilder lhs;
        ECSqlDeleteBuilder rhs;
        AssertEquals (lhs, rhs, false);
        }

        {
        ECSqlInsertBuilder lhs;
        ECSqlUpdateBuilder rhs;
        AssertEquals (lhs, rhs, false);
        }

        {
        ECSqlInsertBuilder lhs;
        ECSqlDeleteBuilder rhs;
        AssertEquals (lhs, rhs, false);
        }

        {
        ECSqlUpdateBuilder lhs;
        ECSqlDeleteBuilder rhs;
        AssertEquals (lhs, rhs, false);
        }

    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECSqlBuilderTests, ECSqlSelectBuilder_IncompleteBuilder)
    {
    ECDbTestFixture::Initialize ();
    auto testSchema = ECDbTestUtility::ReadECSchemaFromDisk (L"StartupCompany.02.00.ecschema.xml");
    auto testClass = testSchema->GetClassCP ("AAA");

    ECSqlSelectBuilder ecsql;

    auto sqlStr = ecsql.ToString ();
    EXPECT_TRUE (sqlStr.empty ()) << "ECSqlSelectBuilder::ToString () is expected to return empty string on empty builder.";

    ecsql.Select ("");
    sqlStr = ecsql.ToString ();
    EXPECT_STREQ ("SELECT ", sqlStr.c_str ()) << "Unexpected result for ECSqlSelectBuilder::ToString ()";

    ecsql = ECSqlSelectBuilder ();
    ecsql.Select ("").From (*testClass, false);
    sqlStr = ecsql.ToString ();
    EXPECT_STREQ ("SELECT  FROM ONLY [StartupCompany].[AAA]", sqlStr.c_str ()) << "Unexpected result for ECSqlSelectBuilder::ToString ()";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  08/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECSqlBuilderTests, ECSqlSelectBuilder_ToString)
    {
    ECDbTestFixture::Initialize ();
    auto testSchema = ECDbTestUtility::ReadECSchemaFromDisk (L"ECSqlTest.01.00.ecschema.xml");
    auto pClass = testSchema->GetClassCP ("P");
    auto psaClass = testSchema->GetClassCP ("PSA");
    auto nonDomainStructWithPrims = testSchema->GetClassCP ("PStruct");
    auto psaHasPsaRelClass = testSchema->GetClassCP ("PSAHasPSA")->GetRelationshipClassCP ();
    auto psaHasPRelClass = testSchema->GetClassCP ("PSAHasP")->GetRelationshipClassCP ();

    ECSqlSelectBuilder builder;
    EXPECT_STREQ ("", builder.ToString ().c_str ());

    //invalid ECSQL - to assert that no validation is done in ToString
    builder = ECSqlSelectBuilder ();
    builder.From (*psaClass, false);
    EXPECT_STREQ (" FROM ONLY [ECSqlTest].[PSA]", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("").From (*psaClass, false);
    EXPECT_STREQ ("SELECT  FROM ONLY [ECSqlTest].[PSA]", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("I, D");
    EXPECT_STREQ ("SELECT I, D", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("I, D").Where ("D > 3.14");
    EXPECT_STREQ ("SELECT I, D WHERE D > 3.14", builder.ToString ().c_str ());


    builder = ECSqlSelectBuilder ();
    builder.Select ("I, D").From (*psaClass, false);
    EXPECT_STREQ ("SELECT I, D FROM ONLY [ECSqlTest].[PSA]", builder.ToString ().c_str ());

    //ECSQL entities are case sensitive
    builder = ECSqlSelectBuilder ();
    builder.Select ("I, D").From (*psaClass, false);
    EXPECT_STRNE ("SELECT i, D FROM ONLY [ECSqlTest].[PSa]", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("I, D").From (*psaClass, false);
    EXPECT_STRNE ("SELECT I, D FROM ONLY [eCSqlTest].[PSA]", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("I, D").From (*psaClass, false);
    EXPECT_STRNE ("SELECT I, D FROM ONLY [ECSqlTest].[PsA]", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("I, D").From (*psaClass, true);
    EXPECT_STREQ ("SELECT I, D FROM [ECSqlTest].[PSA]", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("I, D").From (*psaClass, "a", true);
    EXPECT_STREQ ("SELECT I, D FROM [ECSqlTest].[PSA] a", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("I, D").From (*psaClass, "a", false);
    EXPECT_STREQ ("SELECT I, D FROM ONLY [ECSqlTest].[PSA] a", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("a.I, a.D").From (*psaClass, "a", false);
    EXPECT_STREQ ("SELECT a.I, a.D FROM ONLY [ECSqlTest].[PSA] a", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.SelectAll ().From (*psaClass, false);
    EXPECT_STREQ ("SELECT * FROM ONLY [ECSqlTest].[PSA]", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.SelectAll ().From (*pClass, "a", false);
    EXPECT_STREQ ("SELECT * FROM ONLY [ECSqlTest].[P] a", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.SelectAll ().From (*nonDomainStructWithPrims, false);
    EXPECT_STREQ ("SELECT * FROM ONLY [ECSqlTest].[PStruct]", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("B, ECInstanceId, S").From (*psaClass, false);
    EXPECT_STREQ ("SELECT B, ECInstanceId, S FROM ONLY [ECSqlTest].[PSA]", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("Dt_Array, P3D, PStruct_Array, P2D, PStructProp").From (*psaClass, false);
    EXPECT_STREQ ("SELECT Dt_Array, P3D, PStruct_Array, P2D, PStructProp FROM ONLY [ECSqlTest].[PSA]", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("PStructProp.i, PStructProp.dtUtc").From (*psaClass, false);
    EXPECT_STREQ ("SELECT PStructProp.i, PStructProp.dtUtc FROM ONLY [ECSqlTest].[PSA]", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("3.14").From (*psaClass, false);
    EXPECT_STREQ ("SELECT 3.14 FROM ONLY [ECSqlTest].[PSA]", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("3.14 AS D, 3.14 AS BlaBla").From (*psaClass, true);
    EXPECT_STREQ ("SELECT 3.14 AS D, 3.14 AS BlaBla FROM [ECSqlTest].[PSA]", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("I, Dt, S").From (*psaClass, "a", false).Where ("a.D < 3.14");
    EXPECT_STREQ ("SELECT I, Dt, S FROM ONLY [ECSqlTest].[PSA] a WHERE a.D < 3.14", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("I, Dt, S").From (*psaClass, false).Where ("a.D < 3.14");
    EXPECT_STREQ ("SELECT I, Dt, S FROM ONLY [ECSqlTest].[PSA] WHERE a.D < 3.14", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("I, Dt, S").From (*psaClass, false).Where ("ECInstanceId > 123");
    EXPECT_STREQ ("SELECT I, Dt, S FROM ONLY [ECSqlTest].[PSA] WHERE ECInstanceId > 123", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("I, Dt, S").From (*psaClass, false).Where ("Dt = ?");
    EXPECT_STREQ ("SELECT I, Dt, S FROM ONLY [ECSqlTest].[PSA] WHERE Dt = ?", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("I, Dt, S").From (*psaClass, false).Where ("Dt = :dt AND L > :lmin AND L <= :lmax");
    EXPECT_STREQ ("SELECT I, Dt, S FROM ONLY [ECSqlTest].[PSA] WHERE Dt = :dt AND L > :lmin AND L <= :lmax", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("I, Dt, S").From (*psaClass, false).Where ("B = True OR B = False");
    EXPECT_STREQ ("SELECT I, Dt, S FROM ONLY [ECSqlTest].[PSA] WHERE B = True OR B = False", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("I, S").From (*psaClass, false).Where ("Dt = DATE '2013-08-05'");
    EXPECT_STREQ ("SELECT I, S FROM ONLY [ECSqlTest].[PSA] WHERE Dt = DATE '2013-08-05'", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("I, S").From (*psaClass, false).Where ("P3D = POINT3D (1.1, 2.2, 3.3)");
    EXPECT_STREQ ("SELECT I, S FROM ONLY [ECSqlTest].[PSA] WHERE P3D = POINT3D (1.1, 2.2, 3.3)", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("I, S").From (*psaClass, false).OrderBy ("a.ECInstanceId");
    EXPECT_STREQ ("SELECT I, S FROM ONLY [ECSqlTest].[PSA] ORDER BY a.ECInstanceId", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("I, S").From (*psaClass, false).OrderBy ("a.ECInstanceId ASC, a.I DESC");
    EXPECT_STREQ ("SELECT I, S FROM ONLY [ECSqlTest].[PSA] ORDER BY a.ECInstanceId ASC, a.I DESC", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("I, S").From (*psaClass, false).Limit ("10");
    EXPECT_STREQ ("SELECT I, S FROM ONLY [ECSqlTest].[PSA] LIMIT 10", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("I, S").From (*psaClass, false).Limit ("10", "5");
    EXPECT_STREQ ("SELECT I, S FROM ONLY [ECSqlTest].[PSA] LIMIT 10 OFFSET 5", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("p.I, p.S").From (*psaClass, "p", false).Join (*pClass, "c", false).Using (*psaHasPRelClass);
    EXPECT_STREQ ("SELECT p.I, p.S FROM ONLY [ECSqlTest].[PSA] p JOIN ONLY [ECSqlTest].[P] c USING [ECSqlTest].[PSAHasP]", builder.ToString ().c_str ());

    //This is invalid ECSQL, but the builder doesn't do validation, so we can still test the string conversion
    builder = ECSqlSelectBuilder ();
    builder.Select ("p.I, p.S").From (*psaClass, "p", false).Join (*psaClass, "c", false).Using (*psaHasPsaRelClass);
    EXPECT_STREQ ("SELECT p.I, p.S FROM ONLY [ECSqlTest].[PSA] p JOIN ONLY [ECSqlTest].[PSA] c USING [ECSqlTest].[PSAHasPSA]", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("p.I, p.S").From (*psaClass, "p", false).Join (*psaClass, "c", false).Using (*psaHasPsaRelClass, JoinDirection::Forward);
    EXPECT_STREQ ("SELECT p.I, p.S FROM ONLY [ECSqlTest].[PSA] p JOIN ONLY [ECSqlTest].[PSA] c USING [ECSqlTest].[PSAHasPSA] FORWARD", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("p.I, p.S").From (*psaClass, "p", false).Join (*psaClass, "c", false).Using (*psaHasPsaRelClass, JoinDirection::Reverse);
    EXPECT_STREQ ("SELECT p.I, p.S FROM ONLY [ECSqlTest].[PSA] p JOIN ONLY [ECSqlTest].[PSA] c USING [ECSqlTest].[PSAHasPSA] REVERSE", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("p.I, p.S").From (*psaClass, "p", false).Join (*psaClass, "c", false).Using (*psaHasPRelClass).Where ("c.ECInstanceId = ? AND p.S LIKE 'ma%'");
    EXPECT_STREQ ("SELECT p.I, p.S FROM ONLY [ECSqlTest].[PSA] p JOIN ONLY [ECSqlTest].[PSA] c USING [ECSqlTest].[PSAHasP] WHERE c.ECInstanceId = ? AND p.S LIKE 'ma%'", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("p.I, p.S").From (*psaClass, "p", false).Join (*psaClass, "c", false).Using (*psaHasPsaRelClass).WhereSourceEndIs ("p", "?");
    EXPECT_STREQ ("SELECT p.I, p.S FROM ONLY [ECSqlTest].[PSA] p JOIN ONLY [ECSqlTest].[PSA] c USING [ECSqlTest].[PSAHasPSA] WHERE SourceECInstanceId = ?", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("p.I, p.S").From (*psaClass, "p", false).Join (*psaClass, "c", false).Using (*psaHasPsaRelClass).WhereTargetEndIs ("c", "?");
    EXPECT_STREQ ("SELECT p.I, p.S FROM ONLY [ECSqlTest].[PSA] p JOIN ONLY [ECSqlTest].[PSA] c USING [ECSqlTest].[PSAHasPSA] WHERE TargetECInstanceId = ?", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Select ("p.I, p.S").From (*psaClass, "p", false).Join (*psaClass, "c", false).Using (*psaHasPsaRelClass).WhereRelationshipEndsAre ("p", "123", "c", "444");
    EXPECT_STREQ ("SELECT p.I, p.S FROM ONLY [ECSqlTest].[PSA] p JOIN ONLY [ECSqlTest].[PSA] c USING [ECSqlTest].[PSAHasPSA] WHERE SourceECInstanceId = 123 AND TargetECInstanceId = 444", builder.ToString ().c_str ());

    //use classes which don't have a relationship to prove that no validation is done in ToString
    builder = ECSqlSelectBuilder ();
    builder.Select ("p.I, p.S").From (*psaClass, "p", false).Join (*pClass, "c", false).Using (*psaHasPsaRelClass).Where ("c.ECInstanceId = ? AND p.S LIKE 'ma%'");
    EXPECT_STREQ ("SELECT p.I, p.S FROM ONLY [ECSqlTest].[PSA] p JOIN ONLY [ECSqlTest].[P] c USING [ECSqlTest].[PSAHasPSA] WHERE c.ECInstanceId = ? AND p.S LIKE 'ma%'", builder.ToString ().c_str ());

    //shuffle order in builder which should not affect generated ECSQL string
    builder = ECSqlSelectBuilder ();
    builder.Select ("p.I, p.S").From (*psaClass, "p", false).Join (*psaClass, "c", false).Using (*psaHasPsaRelClass).Where ("p.L = 10").OrderBy ("c.ECInstanceId").Limit ("10", "5");
    EXPECT_STREQ ("SELECT p.I, p.S FROM ONLY [ECSqlTest].[PSA] p JOIN ONLY [ECSqlTest].[PSA] c USING [ECSqlTest].[PSAHasPSA] WHERE p.L = 10 ORDER BY c.ECInstanceId LIMIT 10 OFFSET 5", builder.ToString ().c_str ());

    builder = ECSqlSelectBuilder ();
    builder.Limit ("10", "5").From (*psaClass, "p", false).OrderBy ("c.ECInstanceId").Where ("p.L = 10").Select ("p.I, p.S").Join (*psaClass, "c", false).Using (*psaHasPsaRelClass);
    EXPECT_STREQ ("SELECT p.I, p.S FROM ONLY [ECSqlTest].[PSA] p JOIN ONLY [ECSqlTest].[PSA] c USING [ECSqlTest].[PSAHasPSA] WHERE p.L = 10 ORDER BY c.ECInstanceId LIMIT 10 OFFSET 5", builder.ToString ().c_str ());

    //non-sense ECSQL. To assert that ToString doesn't do any validation
    builder = ECSqlSelectBuilder ();
    builder.Select ("Bli? Bla!").From (*psaClass, false).Where ("Blo knirsch 3.14");
    EXPECT_STREQ ("SELECT Bli? Bla! FROM ONLY [ECSqlTest].[PSA] WHERE Blo knirsch 3.14", builder.ToString ().c_str ());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECSqlBuilderTests, ECSqlInsertBuilder_CopySemantics)
    {
    ECDbTestFixture::Initialize ();
    auto testSchema = ECDbTestUtility::ReadECSchemaFromDisk (L"StartupCompany.02.00.ecschema.xml");
    auto testClass = testSchema->GetClassCP ("AAA");

    ECSqlInsertBuilder b1;
    b1.InsertInto (*testClass).AddValue ("i", "123").AddValue ("d", "3.14");
    ECSqlInsertBuilder b2 (b1);

    AssertEquals (b1, b2, true);

    ECSqlInsertBuilder b3;
    b3 = b1;
    AssertEquals (b1, b3, true);

    ECSqlInsertBuilder* b10 = new ECSqlInsertBuilder ();
    b10->InsertInto (*testClass).AddValue ("i", "123");

    delete b10;
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECSqlBuilderTests, ECSqlInsertBuilder_Comparisons)
    {
    ECDbTestFixture::Initialize ();
    auto testSchema = ECDbTestUtility::ReadECSchemaFromDisk (L"StartupCompany.02.00.ecschema.xml");
    auto testClass = testSchema->GetClassCP ("AAA");

        {
        //empty builders
        ECSqlInsertBuilder lhs;
        ECSqlInsertBuilder rhs;
        AssertEquals (lhs, rhs, true);

        //*** target clause ***
        lhs = ECSqlInsertBuilder ();
        rhs = ECSqlInsertBuilder ();
        lhs.InsertInto (*testClass);
        rhs.InsertInto (*testClass);
        AssertEquals (lhs, rhs, true);

        lhs = ECSqlInsertBuilder ();
        rhs = ECSqlInsertBuilder ();
        lhs.InsertInto (*testClass);
        ECClassCP anotherClass = testSchema->GetClassCP ("AAFoo");
        rhs.InsertInto (*anotherClass);
        AssertEquals (lhs, rhs, false);

        //*** Target property / values ***
        lhs = ECSqlInsertBuilder ();
        rhs = ECSqlInsertBuilder ();

        lhs.AddValue ("l", "100000");
        AssertEquals (lhs, rhs, false);

        rhs.AddValue ("l", "100000");
        AssertEquals (lhs, rhs, true);

        lhs = ECSqlInsertBuilder ();
        rhs = ECSqlInsertBuilder ();
        lhs.AddValue ("l", "100000");
        rhs.AddValue ("L", "100000");
        AssertEquals (lhs, rhs, false);

        lhs = ECSqlInsertBuilder ();
        rhs = ECSqlInsertBuilder ();
        lhs.AddValue ("l", "100000");
        rhs.AddValue ("[l]", "100000");
        AssertEquals (lhs, rhs, false);

        lhs = ECSqlInsertBuilder ();
        rhs = ECSqlInsertBuilder ();
        lhs.AddValue ("l", "100000");
        rhs.AddValue ("l", "100000").AddValue ("s", "?");
        AssertEquals (lhs, rhs, false);
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECSqlBuilderTests, ECSqlInsertBuilder_IncompleteBuilder)
    {
    ECSqlInsertBuilder ecsql;
    auto sqlStr = ecsql.ToString ();
    EXPECT_TRUE (sqlStr.empty ()) << "ECSqlInsertBuilder::ToString () is expected to return empty string on empty builder.";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  08/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECSqlBuilderTests, ECSqlInsertBuilder_ToString)
    {
    ECDbTestFixture::Initialize ();
    auto testSchema = ECDbTestUtility::ReadECSchemaFromDisk (L"ECSqlTest.01.00.ecschema.xml");
    auto psaClass = testSchema->GetClassCP ("PSA");
    auto nonDomainStructWithPrims = testSchema->GetClassCP ("PStruct");

    ECSqlInsertBuilder builder;
    EXPECT_STREQ ("", builder.ToString ().c_str ());

    builder = ECSqlInsertBuilder ();
    builder.InsertInto (*psaClass);
    EXPECT_STREQ ("INSERT INTO [ECSqlTest].[PSA]", builder.ToString ().c_str ());

    //invalid ECSQL - to assert that no validation is done in ToString
    builder = ECSqlInsertBuilder ();
    builder.AddValue ("I", "12");
    EXPECT_STREQ (" (I) VALUES (12)", builder.ToString ().c_str ());

    builder = ECSqlInsertBuilder ();
    builder.InsertInto (*nonDomainStructWithPrims).AddValue ("i", "12");
    EXPECT_STREQ ("INSERT INTO [ECSqlTest].[PStruct] (i) VALUES (12)", builder.ToString ().c_str ());

    builder = ECSqlInsertBuilder ();
    builder.InsertInto (*psaClass).AddValue ("I", "?").AddValue ("L", "1023122");
    EXPECT_STREQ ("INSERT INTO [ECSqlTest].[PSA] (I, L) VALUES (?, 1023122)", builder.ToString ().c_str ());

    builder = ECSqlInsertBuilder ();
    builder.InsertInto (*psaClass).AddValue ("I", "?").AddValue ("L", "1023122").AddValue ("D", "?");
    EXPECT_STREQ ("INSERT INTO [ECSqlTest].[PSA] (I, L, D) VALUES (?, 1023122, ?)", builder.ToString ().c_str ());

    //non-sense ECSQL to assert that no validation is done in ToString
    builder = ECSqlInsertBuilder ();
    builder.InsertInto (*psaClass).AddValue ("knirsch ? no", "he! ").AddValue ("bli !", "");
    EXPECT_STREQ ("INSERT INTO [ECSqlTest].[PSA] (knirsch ? no, bli !) VALUES (he! , )", builder.ToString ().c_str ());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECSqlBuilderTests, ECSqlUpdateBuilder_CopySemantics)
    {
    ECDbTestFixture::Initialize ();
    auto testSchema = ECDbTestUtility::ReadECSchemaFromDisk (L"StartupCompany.02.00.ecschema.xml");
    auto testClass = testSchema->GetClassCP ("AAA");

    ECSqlUpdateBuilder b1;
    b1.Update (*testClass).AddSet ("i", "123").AddSet ("d", "3.14").Where ("l = 10000");
    ECSqlUpdateBuilder b2 (b1);

    AssertEquals (b1, b2, true);

    ECSqlUpdateBuilder b3;
    b3 = b1;
    AssertEquals (b1, b3, true);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECSqlBuilderTests, ECSqlUpdateBuilder_Comparisons)
    {
    ECDbTestFixture::Initialize ();
    auto testSchema = ECDbTestUtility::ReadECSchemaFromDisk (L"StartupCompany.02.00.ecschema.xml");
    auto testClass = testSchema->GetClassCP ("AAA");

    //empty builders
    ECSqlUpdateBuilder lhs;
    ECSqlUpdateBuilder rhs;
    AssertEquals (lhs, rhs, true);

    //*** target clause ***
    lhs = ECSqlUpdateBuilder ();
    rhs = ECSqlUpdateBuilder ();
    lhs.Update (*testClass);
    rhs.Update (*testClass);
    AssertEquals (lhs, rhs, true);

    //IsPolymorphic differs
    lhs = ECSqlUpdateBuilder ();
    rhs = ECSqlUpdateBuilder ();
    lhs.Update (*testClass, true);
    rhs.Update (*testClass, false);
    AssertEquals (lhs, rhs, false);

    lhs = ECSqlUpdateBuilder ();
    rhs = ECSqlUpdateBuilder ();
    lhs.Update (*testClass);
    auto anotherClass = testSchema->GetClassCP ("AAFoo");
    rhs.Update (*anotherClass);
    AssertEquals (lhs, rhs, false);

    //*** Set clause ***
    lhs = ECSqlUpdateBuilder ();
    rhs = ECSqlUpdateBuilder ();

    lhs.AddSet ("l", "100000");
    AssertEquals (lhs, rhs, false);

    rhs.AddSet ("l", "100000");
    AssertEquals (lhs, rhs, true);

    lhs = ECSqlUpdateBuilder ();
    rhs = ECSqlUpdateBuilder ();
    lhs.AddSet ("l", "100000");
    rhs.AddSet ("L", "100000");
    AssertEquals (lhs, rhs, false);

    lhs = ECSqlUpdateBuilder ();
    rhs = ECSqlUpdateBuilder ();
    lhs.AddSet ("l", "100000");
    rhs.AddSet ("[l]", "100000");
    AssertEquals (lhs, rhs, false);

    lhs = ECSqlUpdateBuilder ();
    rhs = ECSqlUpdateBuilder ();
    lhs.AddSet ("l", "100000");
    rhs.AddSet ("l", "100000").AddSet ("s", "?");
    AssertEquals (lhs, rhs, false);

    //*** where clause ***
    lhs = ECSqlUpdateBuilder ();
    rhs = ECSqlUpdateBuilder ();
    lhs.Where ("ECInstanceId = 1234");
    rhs.Where ("ECInstanceId = 4541234");
    AssertEquals (lhs, rhs, false);

    lhs = ECSqlUpdateBuilder ();
    rhs = ECSqlUpdateBuilder ();
    lhs.Where ("l = 1000");
    rhs.Where ("l = 1000");
    AssertEquals (lhs, rhs, true);

    lhs = ECSqlUpdateBuilder ();
    rhs = ECSqlUpdateBuilder ();
    lhs.Where ("l > i");
    rhs.Where ("l > i");
    AssertEquals (lhs, rhs, true);

    lhs = ECSqlUpdateBuilder ();
    rhs = ECSqlUpdateBuilder ();
    lhs.Where ("l > i and s = 'hello, willy' or i = 1");
    rhs.Where ("l > i and s = 'hello, willy' or i = 1");
    AssertEquals (lhs, rhs, true);

    //different case -> not equal
    lhs = ECSqlUpdateBuilder ();
    rhs = ECSqlUpdateBuilder ();
    lhs.Where ("l > i and s = 'hello, willy' or i = 1");
    rhs.Where ("l > i AND s = 'hello, willy' OR i = 1");
    AssertEquals (lhs, rhs, false);

    //different case -> not equal
    lhs = ECSqlUpdateBuilder ();
    rhs = ECSqlUpdateBuilder ();
    lhs.Where ("l > i and s = 'hello, willy' or i = 1");
    rhs.Where ("L > i and s = 'hello, willy' or i = 1");
    AssertEquals (lhs, rhs, false);

    //different case -> not equal
    lhs = ECSqlUpdateBuilder ();
    rhs = ECSqlUpdateBuilder ();
    lhs.Where ("l > i and s = 'hello, willy' or i = 1");
    rhs.Where ("l>i and s = 'hello, willy' or i = 1");
    AssertEquals (lhs, rhs, false);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECSqlBuilderTests, ECSqlUpdateBuilder_IncompleteBuilder)
    {
    ECSqlUpdateBuilder ecsql;
    auto sqlStr = ecsql.ToString ();
    EXPECT_TRUE (sqlStr.empty ()) << "ECSqlUpdateBuilder::ToString () is expected to return empty string on empty builder.";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  08/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECSqlBuilderTests, ECSqlUpdateBuilder_ToString)
    {
    ECDbTestFixture::Initialize ();
    auto testSchema = ECDbTestUtility::ReadECSchemaFromDisk (L"ECSqlTest.01.00.ecschema.xml");
    auto psaClass = testSchema->GetClassCP ("PSA");
    auto nonDomainStructWithPrims = testSchema->GetClassCP ("PStruct");

    ECSqlUpdateBuilder builder;
    EXPECT_STREQ ("", builder.ToString ().c_str ());

    builder = ECSqlUpdateBuilder ();
    builder.Update (*psaClass, false);
    EXPECT_STREQ ("UPDATE ONLY [ECSqlTest].[PSA]", builder.ToString ().c_str ());

    builder = ECSqlUpdateBuilder ();
    builder.Update (*psaClass, false).AddSet ("I", "123").Where ("ECInstanceId > ?");
    EXPECT_STREQ ("UPDATE ONLY [ECSqlTest].[PSA] SET I = 123 WHERE ECInstanceId > ?", builder.ToString ().c_str ());

    builder = ECSqlUpdateBuilder ();
    builder.Update (*nonDomainStructWithPrims, false).AddSet ("i", "123").Where ("l IN (?, ?)");
    EXPECT_STREQ ("UPDATE ONLY [ECSqlTest].[PStruct] SET i = 123 WHERE l IN (?, ?)", builder.ToString ().c_str ());

    builder = ECSqlUpdateBuilder ();
    builder.Update (*psaClass, false).AddSet ("I", "123").AddSet ("L", "?").Where ("ECInstanceId > ?");
    EXPECT_STREQ ("UPDATE ONLY [ECSqlTest].[PSA] SET I = 123, L = ? WHERE ECInstanceId > ?", builder.ToString ().c_str ());

    builder = ECSqlUpdateBuilder ();
    builder.Update (*psaClass, true).AddSet ("I", "123").AddSet ("L", "?").Where ("ECInstanceId > ?");
    EXPECT_STREQ ("UPDATE [ECSqlTest].[PSA] SET I = 123, L = ? WHERE ECInstanceId > ?", builder.ToString ().c_str ());

    //shuffle tokens
    builder = ECSqlUpdateBuilder ();
    builder.Where ("ECInstanceId > ?").AddSet ("I", "123").Update (*psaClass, true).AddSet ("L", "?");
    EXPECT_STREQ ("UPDATE [ECSqlTest].[PSA] SET I = 123, L = ? WHERE ECInstanceId > ?", builder.ToString ().c_str ());

    //non-sense ECSQL to assert that no validation is done in ToString
    builder = ECSqlUpdateBuilder ();
    builder.Update (*psaClass, false).AddSet ("knirsch ? no", "he! ").AddSet ("bli !", "$");
    EXPECT_STREQ ("UPDATE ONLY [ECSqlTest].[PSA] SET knirsch ? no = he! , bli ! = $", builder.ToString ().c_str ());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECSqlBuilderTests, ECSqlDeleteBuilder_CopySemantics)
    {
    ECDbTestFixture::Initialize ();
    auto testSchema = ECDbTestUtility::ReadECSchemaFromDisk (L"StartupCompany.02.00.ecschema.xml");
    auto testClass = testSchema->GetClassCP ("AAA");

    ECSqlDeleteBuilder b1;
    b1.DeleteFrom (*testClass).Where ("l = 10000");
    ECSqlDeleteBuilder b2 (b1);

    AssertEquals (b1, b2, true);

    ECSqlDeleteBuilder b3;
    b3 = b1;
    AssertEquals (b1, b3, true);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECSqlBuilderTest, ECSqlDeleteBuilder_Comparisons)
    {
    ECDbTestFixture::Initialize ();
    auto testSchema = ECDbTestUtility::ReadECSchemaFromDisk (L"StartupCompany.02.00.ecschema.xml");
    auto testClass = testSchema->GetClassCP ("AAA");

    //empty builders
    ECSqlDeleteBuilder lhs;
    ECSqlDeleteBuilder rhs;
    AssertEquals (lhs, rhs, true);

    //*** target clause ***
    lhs = ECSqlDeleteBuilder ();
    rhs = ECSqlDeleteBuilder ();
    lhs.DeleteFrom (*testClass);
    rhs.DeleteFrom (*testClass);
    AssertEquals (lhs, rhs, true);

    //IsPolymorphic differs
    lhs = ECSqlDeleteBuilder ();
    rhs = ECSqlDeleteBuilder ();
    lhs.DeleteFrom (*testClass, true);
    rhs.DeleteFrom (*testClass, false);
    AssertEquals (lhs, rhs, false);

    lhs = ECSqlDeleteBuilder ();
    rhs = ECSqlDeleteBuilder ();
    lhs.DeleteFrom (*testClass);
    auto anotherClass = testSchema->GetClassCP ("AAFoo");
    rhs.DeleteFrom (*anotherClass);
    AssertEquals (lhs, rhs, false);

    //*** where clause ***
    lhs = ECSqlDeleteBuilder ();
    rhs = ECSqlDeleteBuilder ();
    lhs.Where ("ECInstanceIdd = 1234");
    rhs.Where ("ECInstanceId = 4541234");
    AssertEquals (lhs, rhs, false);

    lhs = ECSqlDeleteBuilder ();
    rhs = ECSqlDeleteBuilder ();
    lhs.Where ("l = 1000");
    rhs.Where ("l = 1000");
    AssertEquals (lhs, rhs, true);

    lhs = ECSqlDeleteBuilder ();
    rhs = ECSqlDeleteBuilder ();
    lhs.Where ("l > i");
    rhs.Where ("l > i");
    AssertEquals (lhs, rhs, true);

    lhs = ECSqlDeleteBuilder ();
    rhs = ECSqlDeleteBuilder ();
    lhs.Where ("l > i and s = 'hello, willy' or i = 1");
    rhs.Where ("l > i and s = 'hello, willy' or i = 1");
    AssertEquals (lhs, rhs, true);

    //different case -> not equal
    lhs = ECSqlDeleteBuilder ();
    rhs = ECSqlDeleteBuilder ();
    lhs.Where ("l > i and s = 'hello, willy' or i = 1");
    rhs.Where ("l > i AND s = 'hello, willy' OR i = 1");
    AssertEquals (lhs, rhs, false);

    //different case -> not equal
    lhs = ECSqlDeleteBuilder ();
    rhs = ECSqlDeleteBuilder ();
    lhs.Where ("l > i and s = 'hello, willy' or i = 1");
    rhs.Where ("L > i and s = 'hello, willy' or i = 1");
    AssertEquals (lhs, rhs, false);

    //different case -> not equal
    lhs = ECSqlDeleteBuilder ();
    rhs = ECSqlDeleteBuilder ();
    lhs.Where ("l > i and s = 'hello, willy' or i = 1");
    rhs.Where ("l>i and s = 'hello, willy' or i = 1");
    AssertEquals (lhs, rhs, false);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECSqlBuilderTests, ECSqlDeleteBuilder_IncompleteBuilder)
    {
    ECSqlDeleteBuilder ecsql;
    auto sqlStr = ecsql.ToString ();
    EXPECT_TRUE (sqlStr.empty ()) << "ECSqlDeleteBuilder::ToString () is expected to return empty string on empty builder.";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  08/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECSqlBuilderTests, ECSqlDeleteBuilder_ToString)
    {
    ECDbTestFixture::Initialize ();
    auto testSchema = ECDbTestUtility::ReadECSchemaFromDisk (L"ECSqlTest.01.00.ecschema.xml");
    auto psaClass = testSchema->GetClassCP ("PSA");
    auto nonDomainStructWithPrims = testSchema->GetClassCP ("PStruct");

    ECSqlDeleteBuilder builder;
    EXPECT_STREQ ("", builder.ToString ().c_str ());

    builder = ECSqlDeleteBuilder ();
    builder.DeleteFrom (*psaClass, false);
    EXPECT_STREQ ("DELETE FROM ONLY [ECSqlTest].[PSA]", builder.ToString ().c_str ());

    builder = ECSqlDeleteBuilder ();
    builder.DeleteFrom (*psaClass, true);
    EXPECT_STREQ ("DELETE FROM [ECSqlTest].[PSA]", builder.ToString ().c_str ());

    builder = ECSqlDeleteBuilder ();
    builder.DeleteFrom (*psaClass, true).Where ("I = ? OR S LIKE 'he%'");
    EXPECT_STREQ ("DELETE FROM [ECSqlTest].[PSA] WHERE I = ? OR S LIKE 'he%'", builder.ToString ().c_str ());

    builder = ECSqlDeleteBuilder ();
    builder.DeleteFrom (*psaClass, false).Where ("I = ? OR S LIKE 'he%'");
    EXPECT_STREQ ("DELETE FROM ONLY [ECSqlTest].[PSA] WHERE I = ? OR S LIKE 'he%'", builder.ToString ().c_str ());

    builder = ECSqlDeleteBuilder ();
    builder.DeleteFrom (*nonDomainStructWithPrims, false).Where ("i = ? OR s LIKE 'he%'");
    EXPECT_STREQ ("DELETE FROM ONLY [ECSqlTest].[PStruct] WHERE i = ? OR s LIKE 'he%'", builder.ToString ().c_str ());

    builder = ECSqlDeleteBuilder ();
    builder.Where ("I = ? OR S LIKE 'he%'").DeleteFrom (*psaClass, false);
    EXPECT_STREQ ("DELETE FROM ONLY [ECSqlTest].[PSA] WHERE I = ? OR S LIKE 'he%'", builder.ToString ().c_str ());

    //non-sense ECSQL, to assert that no validation is done in ToString
    builder = ECSqlDeleteBuilder ();
    builder.Where ("Blo knirsch 3.14").DeleteFrom (*psaClass, false);
    EXPECT_STREQ ("DELETE FROM ONLY [ECSqlTest].[PSA] WHERE Blo knirsch 3.14", builder.ToString ().c_str ());
    }

END_ECDBUNITTESTS_NAMESPACE