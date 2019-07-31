/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"
#include <Bentley/Base64Utilities.h>
#include <vector>

#define OPCOUNT 100000

USING_NAMESPACE_BENTLEY

struct TestBlob
    {
    private:
        std::vector<int64_t> m_data;

    public:
        TestBlob(size_t blobSize, int64_t seed)
            {
            for (size_t i = 0; i < blobSize; i++)
                {
                m_data.push_back(seed * (i + 1));
                }
            }

        Byte const* GetBlob() const { return (Byte const*) m_data.data(); }
        size_t GetBlobSize() const { return m_data.size() * sizeof(int64_t); }
    };

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    01/17
//---------------------------------------------------------------------------------------
TEST(Performance_Base64Utilities, Encode)
    {
    std::vector<TestBlob> testBlobs {TestBlob(10, INT64_C(1234567890)), TestBlob(13, INT64_C(3412523412)),
        TestBlob(4, INT64_C(-5132443)), TestBlob(23, INT64_C(-4412313124)) };

    const size_t testBlobCount = testBlobs.size();
    StopWatch timer(true);
    for (int i = 0; i < OPCOUNT; i++)
        {
        TestBlob const& testBlob = testBlobs[i % testBlobCount];
        Utf8String base64;
        Base64Utilities::Encode(base64, testBlob.GetBlob(), testBlob.GetBlobSize());
        }

    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), OPCOUNT, "Base64Utilities::Encode without reusing resulting Utf8String");

    timer.Start();
    Utf8String base64;
    for (int i = 0; i < OPCOUNT; i++)
        {
        TestBlob const& testBlob = testBlobs[i % testBlobCount];
        Base64Utilities::Encode(base64, testBlob.GetBlob(), testBlob.GetBlobSize());
        base64.resize(0);
        }

    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), OPCOUNT, "Base64Utilities::Encode with reusing resulting Utf8String");
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    01/17
//---------------------------------------------------------------------------------------
TEST(Performance_Base64Utilities, Decode)
    {
    std::vector<TestBlob> testBlobs {TestBlob(10, INT64_C(1234567890)), TestBlob(13, INT64_C(3412523412)),
        TestBlob(4, INT64_C(-5132443)), TestBlob(23, INT64_C(-4412313124))};

    std::vector<Utf8String> testBase64Strings;
    for (TestBlob const& testBlob : testBlobs)
        {
        Utf8String base64Str;
        Base64Utilities::Encode(base64Str, testBlob.GetBlob(), testBlob.GetBlobSize());
        testBase64Strings.push_back(base64Str);
        }

    const size_t testStringCount = testBase64Strings.size();

    StopWatch timer(true);
    for (int i = 0; i < OPCOUNT; i++)
        {
        Utf8StringCR testBase64String = testBase64Strings[i % testStringCount];
        TestBlob const& expectedBlob = testBlobs[i % testStringCount];

        ByteStream actualBlob;
        Base64Utilities::Decode(actualBlob, testBase64String);
        ASSERT_EQ(0, memcmp(expectedBlob.GetBlob(), actualBlob.data(), expectedBlob.GetBlobSize()));
        }

    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), OPCOUNT, "Base64Utilities::Decode with ByteStream");

    timer.Start();
    ByteStream actualBlob2;
    for (int i = 0; i < OPCOUNT; i++)
        {
        Utf8StringCR testBase64String = testBase64Strings[i % testStringCount];
        TestBlob const& expectedBlob = testBlobs[i % testStringCount];

        Base64Utilities::Decode(actualBlob2, testBase64String);
        ASSERT_EQ(0, memcmp(expectedBlob.GetBlob(), actualBlob2.data(), expectedBlob.GetBlobSize()));
        actualBlob2.Resize(0);
        }

    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), OPCOUNT, "Base64Utilities::Decode with preallocated and reused ByteStream");

    timer.Start();
    for (int i = 0; i < OPCOUNT; i++)
        {
        Utf8StringCR testBase64String = testBase64Strings[i % testStringCount];
        TestBlob const& expectedBlob = testBlobs[i % testStringCount];

        bvector<Byte> actualBlob;
        Base64Utilities::Decode(actualBlob, testBase64String);
        ASSERT_EQ(0, memcmp(expectedBlob.GetBlob(), actualBlob.data(), expectedBlob.GetBlobSize()));
        }

    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), OPCOUNT, "Base64Utilities::Decode with bvector<Byte>");

    timer.Start();
    bvector<Byte> actualBlob;
    for (int i = 0; i < OPCOUNT; i++)
        {
        Utf8StringCR testBase64String = testBase64Strings[i % testStringCount];
        TestBlob const& expectedBlob = testBlobs[i % testStringCount];

        Base64Utilities::Decode(actualBlob, testBase64String);
        ASSERT_EQ(0, memcmp(expectedBlob.GetBlob(), actualBlob.data(), expectedBlob.GetBlobSize()));
        actualBlob.resize(0);
        }

    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), OPCOUNT, "Base64Utilities::Decode with preallocated and reused bvector<Byte>");

    }
