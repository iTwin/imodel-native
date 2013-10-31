/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublishedScenario/TestCopySchema.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

using namespace Bentley::ECN;

#define EXPECT_SUCCESS(EXPR) EXPECT_TRUE(SUCCESS == EXPR)
BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct TestCopySchema : ECTestFixture {
	ECSchemaPtr m_testSchema;
	ECSchemaReadContextPtr   m_schemaContext;
	SearchPathSchemaFileLocaterPtr m_schemaLocater;
public:
	void SetUp()
	{
		bvector<WString> searchPaths;
		searchPaths.push_back (ECTestFixture::GetTestDataPath(L""));
		m_schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(searchPaths);
		m_schemaContext = ECSchemaReadContext::CreateContext();
		m_schemaContext->AddSchemaLocater (*m_schemaLocater);
		
	}
	void TearDown()
	{
		m_schemaContext->RemoveSchemaLocater(*m_schemaLocater);
	}
};

TEST_F (TestCopySchema, CopySimpleSchemaAndCreateInstance)
{
	SchemaKey key(L"BaseSchema", 01, 00);
	m_testSchema = m_schemaContext->LocateSchema(key, SCHEMAMATCHTYPE_Latest);//ECSchema::LocateSchema(key, *schemaContext);
	EXPECT_TRUE(m_testSchema.IsValid());
	ECClassCP ellipseClass = m_testSchema->GetClassCP (L"ellipse");
    IECInstancePtr ellipseClassInstance = ellipseClass->GetDefaultStandaloneEnabler()->CreateInstance();
	ECValue v;
    v.SetString (L"test");
	ellipseClassInstance->SetValue (L"Name", v);
	ECValue out;
	EXPECT_SUCCESS(ellipseClassInstance->GetValue(out,L"Name"));
	EXPECT_TRUE(out.Equals (ECValue (L"test"))) << "Expect: " << L"test" << " Actual: " << out.ToString().c_str();

	ECSchemaPtr CopiedSchema=NULL;
	m_testSchema->CopySchema (CopiedSchema);
	EXPECT_TRUE(CopiedSchema.IsValid());
	ellipseClass = CopiedSchema->GetClassCP (L"ellipse");
    ellipseClassInstance = ellipseClass->GetDefaultStandaloneEnabler()->CreateInstance();
	v;
    v.SetString (L"test");
	ellipseClassInstance->SetValue (L"Name", v);
	out;
	EXPECT_SUCCESS(ellipseClassInstance->GetValue(out,L"Name"));
	EXPECT_TRUE(out.Equals (ECValue (L"test"))) << "Expect: " << L"test" << " Actual: " << out.ToString().c_str();
}

TEST_F (TestCopySchema, CopySchemaWithDuplicatePrefixesAndCreateInstance)
{
	SchemaKey key(L"DuplicatePrefixes", 01, 00);
	m_testSchema = m_schemaContext->LocateSchema(key, SCHEMAMATCHTYPE_Latest);//ECSchema::LocateSchema(key, *schemaContext);
	EXPECT_TRUE(m_testSchema.IsValid());
	ECClassCP ellipseClass = m_testSchema->GetClassCP (L"Circle");
	EXPECT_TRUE(ellipseClass!=NULL)<<"Cannot Load Ellipse Class";
    IECInstancePtr ellipseClassInstance = ellipseClass->GetDefaultStandaloneEnabler()->CreateInstance();
	ECValue v;
    v.SetString (L"test");
	ellipseClassInstance->SetValue (L"Name", v);
	ECValue out;
	EXPECT_SUCCESS(ellipseClassInstance->GetValue(out,L"Name"));
	EXPECT_TRUE(out.Equals (ECValue (L"test"))) << "Expect: " << L"test" << " Actual: " << out.ToString().c_str();

	ECSchemaPtr CopiedSchema=NULL;
	m_testSchema->CopySchema (CopiedSchema);
	EXPECT_TRUE(CopiedSchema.IsValid());
	ellipseClass = CopiedSchema->GetClassCP (L"Circle");
    ellipseClassInstance = ellipseClass->GetDefaultStandaloneEnabler()->CreateInstance();
	v;
    v.SetString (L"test");
	ellipseClassInstance->SetValue (L"Name", v);
	out;
	EXPECT_SUCCESS(ellipseClassInstance->GetValue(out,L"Name"));
	EXPECT_TRUE(out.Equals (ECValue (L"test"))) << "Expect: " << L"test" << " Actual: " << out.ToString().c_str();
}


TEST_F (TestCopySchema, CopySchemaWithInvalidReferenceAndCreateInstance)
{
	//create Context with legacy support
	m_schemaContext->RemoveSchemaLocater(*m_schemaLocater);
	m_schemaContext = ECSchemaReadContext::CreateContext(true);
	m_schemaContext->AddSchemaLocater (*m_schemaLocater);
	SchemaKey key(L"InvalidReference", 01, 00);
	m_testSchema = m_schemaContext->LocateSchema(key, SCHEMAMATCHTYPE_Latest);//ECSchema::LocateSchema(key, *schemaContext);
	EXPECT_TRUE(m_testSchema.IsValid());
	ECClassCP ellipseClass = m_testSchema->GetClassCP (L"circle");
    IECInstancePtr ellipseClassInstance = ellipseClass->GetDefaultStandaloneEnabler()->CreateInstance();
	ECValue v;
    v.SetString (L"test");
	ellipseClassInstance->SetValue (L"Name", v);
	ECValue out;
	EXPECT_SUCCESS(ellipseClassInstance->GetValue(out,L"Name"));
	EXPECT_TRUE(out.Equals (ECValue (L"test"))) << "Expect: " << L"test" << " Actual: " << out.ToString().c_str();

	ECSchemaPtr CopiedSchema=NULL;
	m_testSchema->CopySchema (CopiedSchema);
	EXPECT_TRUE(CopiedSchema.IsValid());
	ellipseClass = CopiedSchema->GetClassCP (L"circle");
    ellipseClassInstance = ellipseClass->GetDefaultStandaloneEnabler()->CreateInstance();
	v;
    v.SetString (L"test");
	ellipseClassInstance->SetValue (L"Name", v);
	out;
	EXPECT_SUCCESS(ellipseClassInstance->GetValue(out,L"Name"));
	EXPECT_TRUE(out.Equals (ECValue (L"test"))) << "Expect: " << L"test" << " Actual: " << out.ToString().c_str();
}

END_BENTLEY_ECN_TEST_NAMESPACE