/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/StringEncodingTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

#include <ECObjects/ECInstance.h>
#include <ECObjects/StandaloneECInstance.h>
#include <ECObjects/ECValue.h>
using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

using namespace std;

Utf8Char s_schemaXml[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"01.01\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                "    <ECSchemaReference name=\"Bentley_Standard_CustomAttributes\" version=\"01.06\" prefix=\"besc\" />"
                "    <ECClass typeName=\"Manufacturer\" isStruct=\"True\" isDomainClass=\"True\">"
                "        <ECProperty propertyName=\"Name\" typeName=\"string\" />"
                "    </ECClass>"
                "    <ECClass typeName=\"TestClass\" isDomainClass=\"True\">"
                "        <ECArrayProperty propertyName=\"StringArray\" typeName=\"string\" />"
                "        <ECProperty propertyName=\"String\" typeName=\"string\" />"
                "        <ECStructProperty propertyName=\"Struct\" typeName=\"Manufacturer\" />"
                "        <ECArrayProperty propertyName=\"StructArray\" typeName=\"Manufacturer\" isStruct=\"True\" />"
                "    </ECClass>"
                "    <ECClass typeName=\"TestUtf8Class\" isDomainClass=\"True\">"
                "        <ECCustomAttributes>"
                "            <PersistStringsAsUtf8 xmlns=\"Bentley_Standard_CustomAttributes.01.00\" />"
                "        </ECCustomAttributes>"
                "        <ECProperty propertyName=\"String\" typeName=\"string\" />"
                "    </ECClass>"
                "</ECSchema>";

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct StringEncodingTests : ECTestFixture
    {
    ECSchemaPtr         m_schema;

    StringEncodingTests() : ECTestFixture()
        {
        ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext();
        ECSchemaPtr schema;
        EXPECT_EQ (SchemaReadStatus::Success, ECSchema::ReadFromXmlString (m_schema, s_schemaXml, *schemaContext));  
        }

    struct          Utf16String
        {
    private:
        Utf16Buffer         m_utf16;
    public:
        Utf16String (WCharCP wc)
            {
            BeStringUtilities::WCharToUtf16 (m_utf16, wc);
            }

        Utf16CP c_str() const   { return &m_utf16[0]; }
        };

    void            Compare (ECValueCR v1, ECValueCR v2, bool expectMatch)
        {
        EXPECT_EQ (expectMatch, v1.Equals (v2)) << v1.ToString().c_str() << "\n" << v2.ToString().c_str();
        EXPECT_EQ (expectMatch, v2.Equals (v1)) << v1.ToString().c_str() << "\n" << v2.ToString().c_str();
        }

    template<bool expectMatch> void Compare (WCharCP wc, Utf8CP u8, Utf16CP u16)
        {
        ECValue vw (wc), v8 (u8), v16 (u16);
        Compare (vw, v8, expectMatch);
        Compare (vw, v16, expectMatch);
        Compare (v8, v16, expectMatch);
        }

    void                            Convert (ECValueCR v, WCharCP str)
        {
        WCharCP wc = v.GetWCharCP();
        EXPECT_EQ (0, wcscmp (wc, str));
        Utf8CP u8 = v.GetUtf8CP();
        EXPECT_TRUE (0 == Utf8String (str).compare (u8));
        Utf16CP u16 = v.GetUtf16CP();
        EXPECT_EQ (0, BeStringUtilities::CompareUtf16WChar (u16, str));
        }

    StandaloneECInstancePtr CreateInstance (Utf8CP classname, ECDBuffer::StringEncoding encoding)
        {
        // Note setting the global default string encoding isn't a typical workflow.
        // We do it here so we can test instances with different encodings.
        ECDBuffer::SetDefaultStringEncoding (encoding);
        ECClassP ecClass = m_schema->GetClassP (classname);
        return ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
        }
    
    StandaloneECInstancePtr CreatePrimaryInstance (ECDBuffer::StringEncoding encoding, Utf8CP strVal)
        {
        StandaloneECInstancePtr instance = CreateInstance ("TestClass", encoding);
        ECValue v (strVal);
        EXPECT_EQ (ECObjectsStatus::Success, instance->SetValue ("String", v));
        EXPECT_EQ (ECObjectsStatus::Success, instance->SetValue ("Struct.Name", v));
        EXPECT_EQ (ECObjectsStatus::Success, instance->AddArrayElements ("StringArray", 5));
        for (uint32_t i = 0; i < 3; i++)
            EXPECT_EQ (ECObjectsStatus::Success, instance->SetValue ("StringArray", v, i));

        EXPECT_EQ (encoding, instance->GetStringEncoding());
        return instance;
        }

    StandaloneECInstancePtr CreateStructInstance (ECDBuffer::StringEncoding encoding, Utf8CP name, IECInstanceR parent)
        {
        ECValue v;
        EXPECT_EQ (ECObjectsStatus::Success, parent.GetValue (v, "StructArray"));
        EXPECT_EQ (ECObjectsStatus::Success, parent.AddArrayElements ("StructArray", 1));

        StandaloneECInstancePtr instance = CreateInstance ("Manufacturer", encoding);
        EXPECT_EQ (ECObjectsStatus::Success, instance->SetValue ("Name", ECValue (name)));

        ECValue structV;
        structV.SetStruct (instance.get());
        EXPECT_EQ (ECObjectsStatus::Success, parent.SetValue ("StructArray", structV, v.GetArrayInfo().GetCount()));
        EXPECT_EQ (encoding, instance->GetStringEncoding());
        return instance;
        }

    bool                    CompareInstances (ECValuesCollectionCR aVals, StandaloneECInstancePtr b, bool outputDifferences = false)
        {
        for (ECPropertyValueCR aVal: aVals)
            {
            ECValueCR aV = aVal.GetValue();
            ECValue bV;
            EXPECT_EQ (ECObjectsStatus::Success, b->GetValueUsingAccessor (bV, aVal.GetValueAccessor()));
            if (aVal.HasChildValues())
                {
                if (!CompareInstances (*aVal.GetChildValues(), b))
                    return false;
                }
            else if (!bV.Equals (aV))
                {
                if (outputDifferences)
                    printf ("%s differs: %s vs. %s\n", aVal.GetValueAccessor().GetAccessString(), aV.ToString().c_str(), bV.ToString().c_str());

                return false;
                }
            }
            
        return true;
        }

    template <bool expectMatch>
    void                    CompareInstances (StandaloneECInstancePtr a, StandaloneECInstancePtr b)
        {
        ECValuesCollectionPtr aVals = ECValuesCollection::Create (*a);
        EXPECT_EQ (expectMatch, CompareInstances (*aVals, b, expectMatch));
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (StringEncodingTests, TestComparisons)
    {
    WCharCP wc = L"Testing";
    Compare<true> (wc, Utf8String(wc).c_str(), Utf16String(wc).c_str());
    Compare<false> (wc, Utf8String ("abcdefg").c_str(), Utf16String(L"blarg").c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (StringEncodingTests, TestConversions)
    {
    WCharCP wc = L"Testing";
    Convert (ECValue (wc), wc);

    Utf8String u8 (wc);
    ECValue v8 (u8.c_str());
    Convert (v8, wc);

    Utf16String u16 (wc);
    ECValue v16 (u16.c_str());
    Convert (v16, wc);

    Convert (ECValue (wc, false), wc);
    Convert (ECValue (u8.c_str(), false), wc);
    Convert (ECValue (u16.c_str(), false), wc);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (StringEncodingTests, CompareBuffersWithDifferentEncodings)
    {
    StandaloneECInstancePtr a = CreatePrimaryInstance (ECDBuffer::StringEncoding_Utf8, "testing"),
                            b = CreatePrimaryInstance (ECDBuffer::StringEncoding_Utf16, "no match");
    CompareInstances<false> (a, b);

    b = CreatePrimaryInstance (ECDBuffer::StringEncoding_Utf16, "testing");
    CompareInstances<true> (a, b);

    // create a struct array instance with a different encoding than it's parent instance. Useful? not really. But nothing prohibits it.
    StandaloneECInstancePtr structA0 = CreateStructInstance (ECDBuffer::StringEncoding_Utf16, "child", *a),
                            structA1 = CreateStructInstance (ECDBuffer::StringEncoding_Utf8, "child", *a),
                            structB0 = CreateStructInstance (ECDBuffer::StringEncoding_Utf16, "child", *b),
                            structB1 = CreateStructInstance (ECDBuffer::StringEncoding_Utf8, "child", *b);

    CompareInstances<true> (structA0, structB0);
    CompareInstances<true> (structA0, structA1);
    CompareInstances<true> (structA1, structB0);
    CompareInstances<true> (a, b);

    structB1->SetValue ("Name", ECValue ("grandkid"));
    ECValue structV;
    structV.SetStruct (structB1.get());
    b->SetValue ("StructArray", structV, 1);
    CompareInstances<false> (structB1, structB0);
    CompareInstances<false> (structB1, structA1);
    CompareInstances<false> (a, b);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (StringEncodingTests, CopyBuffersWithDifferentEncodings)
    {
    // a and b use different encodings
    StandaloneECInstancePtr a = CreatePrimaryInstance (ECDBuffer::StringEncoding_Utf8, "slartibartfast");
    ECDBuffer::SetDefaultStringEncoding (ECDBuffer::StringEncoding_Utf16);
    StandaloneECInstancePtr b = m_schema->GetClassP ("TestClass")->GetDefaultStandaloneEnabler()->CreateInstance();

    EXPECT_EQ (ECDBuffer::StringEncoding_Utf16, b->GetStringEncoding());

    // Copying instances copies the entire buffer - including the encoding flag and the strings in their original encodings.
    EXPECT_EQ (ECObjectsStatus::Success, b->CopyValues (*a));

    // a and b should now have the same encoding
    EXPECT_EQ (ECDBuffer::StringEncoding_Utf8, b->GetStringEncoding());

    CompareInstances<true> (a, b);

    b->SetValue ("Struct.Name", ECValue ("finnegan"));
    CompareInstances<false> (a, b);
    }

/*---------------------------------------------------------------------------------**//**
* If the ECClass has the custom attribute "PersistStringsAsUtf8", instances will always
* use Utf-8 encoding.
* @bsimethod                                                    Paul.Connelly   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (StringEncodingTests, ForceUtf8)
    {
    ECDBuffer::SetDefaultStringEncoding (ECDBuffer::StringEncoding_Utf16);
    StandaloneECInstancePtr a = m_schema->GetClassP ("TestUtf8Class")->GetDefaultStandaloneEnabler()->CreateInstance();
    EXPECT_EQ (ECDBuffer::StringEncoding_Utf8, a->GetStringEncoding());

    ECDBuffer::SetDefaultStringEncoding (ECDBuffer::StringEncoding_Utf8);
    StandaloneECInstancePtr b = m_schema->GetClassP ("TestUtf8Class")->GetDefaultStandaloneEnabler()->CreateInstance();
    EXPECT_EQ (ECDBuffer::StringEncoding_Utf8, b->GetStringEncoding());

    ECDBuffer::SetDefaultStringEncoding (ECDBuffer::StringEncoding_Utf16);
    }

END_BENTLEY_ECN_TEST_NAMESPACE

