/*--------------------------------------------------------------------------------------+
 |
 |  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/WString.h>
#include <Bentley/bvector.h>

BEGIN_BENTLEY_NAMESPACE

struct ByteStream;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct Base64Utilities
    {
private:
    Base64Utilities();
    ~Base64Utilities();

public:
    static Utf8String Encode(Utf8StringCR stringToEncode, Utf8CP header = nullptr) { return Encode(stringToEncode.c_str(), stringToEncode.size(), header); }
    BENTLEYDLL_EXPORT static Utf8String Encode (Utf8CP bytesToEncode, size_t byteCount, Utf8CP header = nullptr);
    BENTLEYDLL_EXPORT static void Encode(Utf8StringR encodedString, Byte const* bytesToEncode, size_t byteCount, Utf8CP header = nullptr);

    static void Decode(bvector<Byte>& byteArray, Utf8StringCR encodedString) { return Decode(byteArray, encodedString.c_str(), encodedString.size()); }
    BENTLEYDLL_EXPORT static void Decode(bvector<Byte>& byteArray, Utf8CP encodedString, size_t encodedStringLength);

    static Utf8String Decode(Utf8StringCR encodedString) { return Decode(encodedString.c_str(), encodedString.size()); }
    BENTLEYDLL_EXPORT static Utf8String Decode(Utf8CP encodedString, size_t encodedStringLength);

    static void Decode(ByteStream& dest, Utf8StringCR src) { return Decode(dest, src.c_str(), src.size()); }
    BENTLEYDLL_EXPORT static void Decode(ByteStream& dest, Utf8CP src, size_t srcLen);

    BENTLEYDLL_EXPORT static Utf8StringCR Alphabet();

    BENTLEYDLL_EXPORT static bool MatchesAlphabet(Utf8CP str); 
    };

END_BENTLEY_NAMESPACE
