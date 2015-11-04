/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/SchemaTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

using namespace BentleyApi::ECN;
#define EXPECT_SUCCESS(EXPR) EXPECT_TRUE(ECObjectsStatus::Success == EXPR)
BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct SchemaTest : ECTestFixture {};

struct TestCopySchema : ECTestFixture
    {
    ECSchemaPtr m_testSchema;
    ECSchemaReadContextPtr   m_schemaContext;
    SearchPathSchemaFileLocaterPtr m_schemaLocater;
    public:
        void SetUp ()
            {
            bvector<WString> searchPaths;
            searchPaths.push_back (ECTestFixture::GetTestDataPath (L""));
            m_schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater (searchPaths);
            m_schemaContext = ECSchemaReadContext::CreateContext ();
            m_schemaContext->AddSchemaLocater (*m_schemaLocater);

            }
        void TearDown ()
            {
            m_schemaContext->RemoveSchemaLocater (*m_schemaLocater);
            }
    };

//---------------------------------------------------------------------------------**//**
// @bsimethod                                   Raimondas.Rimkus                   02/13
// +---------------+---------------+---------------+---------------+---------------+-----
TEST_F (SchemaTest, ShouldBeAbleToIterateOverECClassContainer)
    {
    ECSchemaPtr schema;
    ECEntityClassP foo;
    ECEntityClassP bar;

    ECSchema::CreateSchema (schema, "TestSchema", 5, 5);
    schema->CreateEntityClass (foo, "foo");
    schema->CreateEntityClass (bar, "bar");

    ClassMap classMap;
    classMap.insert (bpair<Utf8CP, ECClassP> (foo->GetName ().c_str (), foo));
    classMap.insert (bpair<Utf8CP, ECClassP> (bar->GetName ().c_str (), bar));

    int count = 0;
    ECClassContainer container (classMap);
    for (ECClassContainer::const_iterator cit = container.begin (); cit != container.end (); ++cit)
        {
        ECClassCP ecClass = *cit;
        Utf8String name = ecClass->GetName ();
        count++;
        }
    ASSERT_EQ (2, count);

    for (ECClassCP ecClass : container)
        {
        Utf8String name = ecClass->GetName ();
        count++;
        }
    ASSERT_EQ (4, count);
    }

//---------------------------------------------------------------------------------**//**
// @bsimethod                                   Raimondas.Rimkus                   02/13
// +---------------+---------------+---------------+---------------+---------------+-----
TEST_F (SchemaTest, TestGetClassCount)
    {
    ECSchemaPtr schema;
    ECEntityClassP foo;
    ECEntityClassP bar;

    ECSchema::CreateSchema (schema, "TestSchema", 5, 5);
    schema->CreateEntityClass (foo, "foo");
    schema->CreateEntityClass (bar, "bar");

    ClassMap classMap;
    classMap.insert (bpair<Utf8CP, ECClassP> (foo->GetName ().c_str (), foo));
    classMap.insert (bpair<Utf8CP, ECClassP> (bar->GetName ().c_str (), bar));

    int count = 0;
    ECClassContainer container (classMap);
    for (ECClassContainer::const_iterator cit = container.begin (); cit != container.end (); ++cit)
        {
        ECClassCP ecClass = *cit;
        Utf8String name = ecClass->GetName ();
        //printf("ECClass=0x%lx, name=%s\n", (uintptr_t)ecClass, name.c_str());
        count++;
        }
    ASSERT_EQ (2, count);

    for (ECClassCP ecClass : container)
        {
        Utf8String name = ecClass->GetName ();
        //printf("ECClass=0x%lx, name=%s\n", (uintptr_t)ecClass, name.c_str());
        count++;
        }
    ASSERT_EQ (4, count);
    ASSERT_EQ (2, schema->GetClassCount ());
    }

//---------------------------------------------------------------------------------**//**
// @bsimethod                                   Raimondas.Rimkus                   02/13
// +---------------+---------------+---------------+---------------+---------------+-----
TEST_F (SchemaTest, TestCircularReference)
    {
    ECSchemaPtr testSchema;
    ECSchemaReadContextPtr   schemaContext;
    SearchPathSchemaFileLocaterPtr schemaLocater;
    bvector<WString> searchPaths;
    searchPaths.push_back (ECTestFixture::GetTestDataPath (L""));
    schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater (searchPaths);
    schemaContext = ECSchemaReadContext::CreateContext ();
    schemaContext->AddSchemaLocater (*schemaLocater);
    SchemaKey key ("CircleSchema", 01, 00);
    testSchema = schemaContext->LocateSchema (key, SCHEMAMATCHTYPE_Latest);
    EXPECT_FALSE (testSchema.IsValid ());
    }

//---------------------------------------------------------------------------------**//**
// @bsimethod                                   Raimondas.Rimkus                   02/13
// +---------------+---------------+---------------+---------------+---------------+-----
TEST_F (SchemaTest, TestsLatestCompatible)
    {
    ECSchemaPtr testSchema;
    ECSchemaReadContextPtr   schemaContext;
    SearchPathSchemaFileLocaterPtr schemaLocater;
    bvector<WString> searchPaths;
    searchPaths.push_back (ECTestFixture::GetTestDataPath (L""));
    schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater (searchPaths);
    schemaContext = ECSchemaReadContext::CreateContext ();
    schemaContext->AddSchemaLocater (*schemaLocater);
    SchemaKey key ("Widgets", 01, 00);
    testSchema = schemaContext->LocateSchema (key, SCHEMAMATCHTYPE_LatestCompatible);
    EXPECT_TRUE (testSchema.IsValid ());
    EXPECT_TRUE (testSchema->GetVersionMajor () == 9);
    EXPECT_TRUE (testSchema->GetVersionMinor () == 6);
    }

//---------------------------------------------------------------------------------**//**
// @bsimethod                                   Raimondas.Rimkus                   02/13
// +---------------+---------------+---------------+---------------+---------------+-----
TEST_F (SchemaTest, TestsLatest)
    {
    ECSchemaPtr testSchema;
    ECSchemaReadContextPtr   schemaContext;
    SearchPathSchemaFileLocaterPtr schemaLocater;
    bvector<WString> searchPaths;
    searchPaths.push_back (ECTestFixture::GetTestDataPath (L""));
    schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater (searchPaths);
    schemaContext = ECSchemaReadContext::CreateContext ();
    schemaContext->AddSchemaLocater (*schemaLocater);
    SchemaKey key ("Widgets", 9, 7);
    testSchema = schemaContext->LocateSchema (key, SCHEMAMATCHTYPE_Latest);
    EXPECT_TRUE (testSchema.IsValid ());
    EXPECT_TRUE (testSchema->GetVersionMajor () == 9);
    EXPECT_TRUE (testSchema->GetVersionMinor () == 6);
    }

//---------------------------------------------------------------------------------**//**
// @bsimethod                                   Raimondas.Rimkus                   02/13
// +---------------+---------------+---------------+---------------+---------------+-----
TEST_F (SchemaTest, GetBaseClassPropertyWhenSchemaHaveDuplicatePrefixes)
    {
    ECSchemaPtr testSchema;
    ECSchemaReadContextPtr   schemaContext;
    SearchPathSchemaFileLocaterPtr schemaLocater;
    bvector<WString> searchPaths;
    searchPaths.push_back (ECTestFixture::GetTestDataPath (L""));
    schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater (searchPaths);
    schemaContext = ECSchemaReadContext::CreateContext ();
    schemaContext->AddSchemaLocater (*schemaLocater);
    SchemaKey key ("DuplicatePrefixes", 01, 00);
    testSchema = schemaContext->LocateSchema (key, SCHEMAMATCHTYPE_Latest);
    EXPECT_TRUE (testSchema.IsValid ());
    ECClassCP CircleClass = testSchema->GetClassCP ("Circle");
    EXPECT_TRUE (CircleClass != NULL) << "Cannot Load Ellipse Class";

    IECInstancePtr CircleClassInstance = CircleClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ECValue v;
    v.SetUtf8CP ("test");
    CircleClassInstance->SetValue ("Name", v);
    }

//---------------------------------------------------------------------------------**//**
// @bsimethod                                   Raimondas.Rimkus                   02/13
// +---------------+---------------+---------------+---------------+---------------+-----
TEST_F (SchemaTest, GetBaseClassProperty)
    {
    ECSchemaPtr testSchema;
    ECSchemaReadContextPtr   schemaContext;
    SearchPathSchemaFileLocaterPtr schemaLocater;
    bvector<WString> searchPaths;
    searchPaths.push_back (ECTestFixture::GetTestDataPath (L""));
    schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater (searchPaths);
    schemaContext = ECSchemaReadContext::CreateContext ();
    schemaContext->AddSchemaLocater (*schemaLocater);
    SchemaKey key ("testschema", 01, 00);
    testSchema = schemaContext->LocateSchema (key, SCHEMAMATCHTYPE_Latest);
    EXPECT_TRUE (testSchema.IsValid ());
    ECClassCP WheelsChildClass = testSchema->GetClassCP ("WheelsChild");
    EXPECT_TRUE (WheelsChildClass != NULL) << "Cannot Load WheelsChild Class";

    IECInstancePtr WheelsChildInstance = WheelsChildClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ECValue v;
    v.SetUtf8CP ("test");
    WheelsChildInstance->SetValue ("Name", v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TestCopySchema, CopySimpleSchemaAndCreateInstance)
    {
    SchemaKey key ("BaseSchema", 01, 00);
    m_testSchema = m_schemaContext->LocateSchema (key, SCHEMAMATCHTYPE_Latest);//ECSchema::LocateSchema(key, *schemaContext);
    EXPECT_TRUE (m_testSchema.IsValid ());
    ECClassCP ellipseClass = m_testSchema->GetClassCP ("ellipse");
    IECInstancePtr ellipseClassInstance = ellipseClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ECValue v;
    v.SetUtf8CP ("test");
    ellipseClassInstance->SetValue ("Name", v);
    ECValue out;
    EXPECT_SUCCESS (ellipseClassInstance->GetValue (out, "Name"));
    EXPECT_TRUE (out.Equals (ECValue ("test"))) << "Expect: " << "test" << " Actual: " << out.ToString ().c_str ();

    ECSchemaPtr CopiedSchema = NULL;
    m_testSchema->CopySchema (CopiedSchema);
    EXPECT_TRUE (CopiedSchema.IsValid ());
    ellipseClass = CopiedSchema->GetClassCP ("ellipse");
    ellipseClassInstance = ellipseClass->GetDefaultStandaloneEnabler ()->CreateInstance ();

    v.SetUtf8CP ("test");
    ellipseClassInstance->SetValue ("Name", v);
    EXPECT_SUCCESS (ellipseClassInstance->GetValue (out, "Name"));
    EXPECT_TRUE (out.Equals (ECValue ("test"))) << "Expect: " << "test" << " Actual: " << out.ToString ().c_str ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TestCopySchema, CopySchemaWithDuplicatePrefixesAndCreateInstance)
    {
    SchemaKey key ("DuplicatePrefixes", 01, 00);
    m_testSchema = m_schemaContext->LocateSchema (key, SCHEMAMATCHTYPE_Latest);//ECSchema::LocateSchema(key, *schemaContext);
    EXPECT_TRUE (m_testSchema.IsValid ());
    ECClassCP ellipseClass = m_testSchema->GetClassCP ("Circle");
    EXPECT_TRUE (ellipseClass != NULL) << "Cannot Load Ellipse Class";
    IECInstancePtr ellipseClassInstance = ellipseClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ECValue v;
    v.SetUtf8CP ("test");
    ellipseClassInstance->SetValue ("Name", v);
    ECValue out;
    EXPECT_SUCCESS (ellipseClassInstance->GetValue (out, "Name"));
    EXPECT_TRUE (out.Equals (ECValue ("test"))) << "Expect: " << "test" << " Actual: " << out.ToString ().c_str ();

    ECSchemaPtr CopiedSchema = NULL;
    m_testSchema->CopySchema (CopiedSchema);
    EXPECT_TRUE (CopiedSchema.IsValid ());
    ellipseClass = CopiedSchema->GetClassCP ("Circle");
    ellipseClassInstance = ellipseClass->GetDefaultStandaloneEnabler ()->CreateInstance ();

    v.SetUtf8CP ("test");
    ellipseClassInstance->SetValue ("Name", v);
    EXPECT_SUCCESS (ellipseClassInstance->GetValue (out, "Name"));
    EXPECT_TRUE (out.Equals (ECValue ("test"))) << "Expect: " << "test" << " Actual: " << out.ToString ().c_str ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TestCopySchema, CopySchemaWithInvalidReferenceAndCreateInstance)
    {
    //create Context with legacy support
    m_schemaContext->RemoveSchemaLocater (*m_schemaLocater);
    m_schemaContext = ECSchemaReadContext::CreateContext (true);
    m_schemaContext->AddSchemaLocater (*m_schemaLocater);
    SchemaKey key ("InvalidReference", 01, 00);
    m_testSchema = m_schemaContext->LocateSchema (key, SCHEMAMATCHTYPE_Latest);
    EXPECT_TRUE (m_testSchema.IsValid ());
    ECClassCP ellipseClass = m_testSchema->GetClassCP ("circle");
    IECInstancePtr ellipseClassInstance = ellipseClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ECValue v;
    v.SetUtf8CP ("test");
    ellipseClassInstance->SetValue ("Name", v);
    ECValue out;
    EXPECT_SUCCESS (ellipseClassInstance->GetValue (out, "Name"));
    EXPECT_TRUE (out.Equals (ECValue ("test"))) << "Expect: " << "test" << " Actual: " << out.ToString ().c_str ();

    ECSchemaPtr CopiedSchema = NULL;
    m_testSchema->CopySchema (CopiedSchema);
    EXPECT_TRUE (CopiedSchema.IsValid ());
    ellipseClass = CopiedSchema->GetClassCP ("circle");
    ellipseClassInstance = ellipseClass->GetDefaultStandaloneEnabler ()->CreateInstance ();

    v.SetUtf8CP ("test");
    ellipseClassInstance->SetValue ("Name", v);
    EXPECT_SUCCESS (ellipseClassInstance->GetValue (out, "Name"));
    EXPECT_TRUE (out.Equals (ECValue ("test"))) << "Expect: " << "test" << " Actual: " << out.ToString ().c_str ();
    }

END_BENTLEY_ECN_TEST_NAMESPACE