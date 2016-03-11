/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Bentley/nonport/Base64Utilities.cpp $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#include <Bentley/Base64Utilities.h>

USING_NAMESPACE_BENTLEY

//Algo from http://www.adp-gmbh.ch/cpp/common/base64.html 

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static const Utf8String base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                        "abcdefghijklmnopqrstuvwxyz"
                                        "0123456789+/";

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool is_base64(unsigned char c)
    {
    return (isalnum (c) || (c == '+') || (c == '/'));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Tahir.Hayat    12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Base64Utilities::Encode (Utf8CP bytes_to_encode, size_t byteCount)
    {
    Utf8String ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (byteCount--)
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

//--------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus Base64Utilities::Encode(Utf8StringR encodedString, Byte const* bytesToEncode, size_t byteCount)
    {
    encodedString.clear();

    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    int i = 0;
    int j = 0;
    while (byteCount--)
        {
        char_array_3[i++] = *(bytesToEncode++);
        if (i == 3)
            {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; i < 4; i++)
                encodedString += base64_chars[char_array_4[i]];

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

        for (j = 0; (j < i + 1); j++)
            encodedString += base64_chars[char_array_4[j]];

        while ((i++ < 3))
            encodedString += '=';

        }

    return SUCCESS;
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

//--------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus Base64Utilities::Decode(bvector<Byte>& byteArray, Utf8StringCR encodedString)
    {
    int size = (int) encodedString.size();
    int i = 0;
    int j = 0;
    int in_ = 0;

    unsigned char char_array_4[4], char_array_3[3];

    while (size-- && (encodedString[in_] != '=') && is_base64(encodedString[in_]))
        {
        char_array_4[i++] = encodedString[in_]; in_++;
        if (i == 4)
            {
            for (i = 0; i < 4; i++)
                char_array_4[i] = (unsigned char) base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                byteArray.push_back(char_array_3[i]);
            i = 0;
            }
        }

    if (i)
        {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] = (unsigned char) base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++)
            byteArray.push_back(char_array_3[j]);
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR Base64Utilities::Alphabet ()
    {
    return base64_chars;
    }

