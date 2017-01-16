/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/base64utilities_performance.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"
#include <Bentley/Base64Utilities.h>

#define OPCOUNT 100000

USING_NAMESPACE_BENTLEY

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    01/17
//---------------------------------------------------------------------------------------
TEST(Performance_Base64Utilities, Decode)
    {
    int64_t blob[10];
    for (size_t i = 0; i < 10; i++)
        {
        blob[i] = INT64_C(1234567890) * i;
        }

    const size_t blobSize = sizeof(blob);

    Utf8String base64Str;
    Base64Utilities::Encode(base64Str, reinterpret_cast<Byte const*> (&blob), blobSize);

    StopWatch timer(true);
    for (int i = 0; i < OPCOUNT; i++)
        {
        ByteStream actualBlob;
        Base64Utilities::Decode(actualBlob, base64Str);
        ASSERT_EQ(0, memcmp(&blob, actualBlob.data(), blobSize));
        }

    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "Base64Utilities::Decode with ByteStream", OPCOUNT);

    timer.Start();
    ByteStream actualBlob2;
    for (int i = 0; i < OPCOUNT; i++)
        {
        Base64Utilities::Decode(actualBlob2, base64Str);
        ASSERT_EQ(0, memcmp(&blob, actualBlob2.data(), blobSize));
        actualBlob2.Resize(0);
        }

    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "Base64Utilities::Decode with preallocated and reused ByteStream", OPCOUNT);

    timer.Start();
    for (int i = 0; i < OPCOUNT; i++)
        {
        bvector<Byte> actualBlob;
        Base64Utilities::Decode(actualBlob, base64Str);
        ASSERT_EQ(0, memcmp(&blob, actualBlob.data(), blobSize));
        }

    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "Base64Utilities::Decode with bvector<Byte>", OPCOUNT);

    timer.Start();
    bvector<Byte> actualBlob;
    actualBlob.reserve((size_t) ((3.0 / 4.0) * base64Str.size()));
    for (int i = 0; i < OPCOUNT; i++)
        {
        Base64Utilities::Decode(actualBlob, base64Str);
        ASSERT_EQ(0, memcmp(&blob, actualBlob.data(), blobSize));
        actualBlob.resize(0);
        }

    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "Base64Utilities::Decode with preallocated and reused bvector<Byte>", OPCOUNT);

    }
