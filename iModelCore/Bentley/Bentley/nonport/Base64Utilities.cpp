/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Bentley/nonport/Base64Utilities.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#include <Bentley/Base64Utilities.h>

USING_NAMESPACE_BENTLEY

static const Utf8String base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

static inline bool is_base64 (unsigned char c)
    {
    return (isalnum (c) || (c == '+') || (c == '/'));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Base64Utilities::Encode (Utf8StringCR string_to_encode)
    {
    return Encode (string_to_encode.c_str (), string_to_encode.size ());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Tahir.Hayat    12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Base64Utilities::Encode (Utf8CP bytes_to_encode, size_t in_len)
    {
    Utf8String ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--)
        {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3)
            {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; i < 4; i++)
                ret += base64_chars[char_array_4[i]];

            i = 0;
            }
        }

    if (i)
        {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; j < i + 1; j++)
            ret += base64_chars[char_array_4[j]];

        while (i++ < 3)
            ret += '=';
        }

    return ret;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Base64Utilities::Decode (Utf8StringCR encoded_string)
    {
    return Decode (encoded_string.c_str (), encoded_string.size ());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Tahir.Hayat    12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Base64Utilities::Decode (Utf8CP encoded_bytes, size_t in_len)
    {
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    Utf8String ret;

    while (in_len-- && (encoded_bytes[in_] != '=') && is_base64 (encoded_bytes[in_]))
        {
        char_array_4[i++] = encoded_bytes[in_]; in_++;
        if (i == 4)
            {
            for (i = 0; i < 4; i++)
                char_array_4[i] = (unsigned char)base64_chars.find (char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
            }
        }

    if (i)
        {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] = (unsigned char)base64_chars.find (char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = 0xFF & (((char_array_4[2] & 0x3) << 6) + char_array_4[3]);

        for (j = 0; j < i - 1; j++)
            ret += char_array_3[j];
        }

    return ret;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR Base64Utilities::Alphabet ()
    {
    return base64_chars;
    }

