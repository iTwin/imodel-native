/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/Bentley/Base64Utilities.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/WString.h>

BEGIN_BENTLEY_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct Base64Utilities
    {
    public:
        BENTLEYDLL_EXPORT static Utf8String Encode (Utf8StringCR stringToEncode);
        BENTLEYDLL_EXPORT static Utf8String Encode (Utf8CP bytesToEncode, size_t length);

        BENTLEYDLL_EXPORT static Utf8String Decode (Utf8StringCR encodedString);
        BENTLEYDLL_EXPORT static Utf8String Decode (Utf8CP encodedBytes, size_t length);

        BENTLEYDLL_EXPORT static Utf8StringCR Alphabet ();
    };

END_BENTLEY_NAMESPACE
