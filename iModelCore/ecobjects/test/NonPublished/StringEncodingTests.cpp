/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/StringEncodingTests.cpp $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include "StopWatch.h"
#include "TestFixture.h"

#include <ECObjects\ECInstance.h>
#include <ECObjects\StandaloneECInstance.h>
#include <ECObjects\ECValue.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

using namespace std;

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct StringEncodingTests : ECTestFixture
    {
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
        WCharCP wc = v.GetString();
        EXPECT_EQ (0, wcscmp (wc, str));
        Utf8CP u8 = v.GetUtf8CP();
        EXPECT_TRUE (0 == Utf8String (str).compare (u8));
        Utf16CP u16 = v.GetUtf16CP();
        EXPECT_EQ (0, BeStringUtilities::CompareUtf16WChar (u16, str));
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

END_BENTLEY_ECOBJECT_NAMESPACE

