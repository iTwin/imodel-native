/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <Bentley/Base64Utilities.h>
#include <Bentley/ByteStream.h>

USING_NAMESPACE_BENTLEY

//Algo from http://www.adp-gmbh.ch/cpp/common/base64.html 

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static const Utf8String base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                        "abcdefghijklmnopqrstuvwxyz"
                                        "0123456789+/";

//base64 char to blob char look-up index. For a given base64 char, its ASCII code is the index into
//the look-up array. The value at the index is the ASCII code of the blob character.
//This is a faster (0(1)) than calling find on base64_char O(n)
static const Byte base64_decodeindex[] {
    //ASCII code of base64 char:    0 1 2                                                    41 42   
    (Byte) -1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    //43 44 45 46  47  48                                  57 58                64
    // +           /   0   1   2 ...                       9
      62, 0, 0, 0, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0, 0, 0, 0, 0, 0, 0,
    //65 66                                                                                     90 91             96
    //A  B  ...                                                                                 Z
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0, 0, 0, 0, 0, 0,
    //97  98                                                                                             122
    //a   b  ...                                                                                          z
      26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};

static_assert(sizeof(base64_decodeindex)/sizeof(Byte) == 123, "base64decodelookup is expected to have (int) 'z' + 1 elements");

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool is_base64(Byte c) { return (isalnum (c) || (c == '+') || (c == '/')); }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String Base64Utilities::Encode(Utf8CP byteArray, size_t byteCount, Utf8CP header)
    {
    Utf8String encodedString;
    Encode(encodedString, reinterpret_cast<Byte const*> (byteArray), byteCount, header);
    return encodedString;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void Base64Utilities::Encode(Utf8StringR encodedString, Byte const* byteArray, size_t byteCount, Utf8CP header)
    {
    if (byteArray == nullptr || byteCount == 0)
        return;

    if (!Utf8String::IsNullOrEmpty(header))
        encodedString.assign(header);

    size_t nEncodedBytes = (size_t) (4.0 * ((byteCount + 2) / 3.0)) + encodedString.size();
    encodedString.reserve(nEncodedBytes);

    Byte byte_array_3[3];
    Byte byte_array_4[4];

    int i = 0;
    int j = 0;
    while (byteCount--)
        {
        byte_array_3[i++] = *(byteArray++);
        if (i == 3)
            {
            byte_array_4[0] = (byte_array_3[0] & 0xfc) >> 2;
            byte_array_4[1] = ((byte_array_3[0] & 0x03) << 4) + ((byte_array_3[1] & 0xf0) >> 4);
            byte_array_4[2] = ((byte_array_3[1] & 0x0f) << 2) + ((byte_array_3[2] & 0xc0) >> 6);
            byte_array_4[3] = byte_array_3[2] & 0x3f;

            for (i = 0; i < 4; i++)
                encodedString += base64_chars[byte_array_4[i]];

            i = 0;
            }
        }

    if (i)
        {
        for (j = i; j < 3; j++)
            byte_array_3[j] = '\0';

        byte_array_4[0] = (byte_array_3[0] & 0xfc) >> 2;
        byte_array_4[1] = ((byte_array_3[0] & 0x03) << 4) + ((byte_array_3[1] & 0xf0) >> 4);
        byte_array_4[2] = ((byte_array_3[1] & 0x0f) << 2) + ((byte_array_3[2] & 0xc0) >> 6);
        byte_array_4[3] = byte_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            encodedString += base64_chars[byte_array_4[j]];

        while ((i++ < 3))
            encodedString += '=';
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Base64Utilities::Decode(Utf8CP encodedString, size_t encodedStringLength)
    {
    ByteStream byteStream;
    Decode(byteStream, encodedString, encodedStringLength);
    if (byteStream.empty())
        return Utf8String();

    return Utf8String((Utf8CP) byteStream.data(), byteStream.size());
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
template<typename T> static void base64_decode(T& byteArray, Utf8CP encodedString, size_t encodedStringLength)
    {
    if (Utf8String::IsNullOrEmpty(encodedString) || encodedStringLength == 0)
        return;

    int i = 0;
    int j = 0;
    int in_ = 0;

    Byte byte_array_4[4], byte_array_3[3];

    while (encodedStringLength-- && (encodedString[in_] != '=') && is_base64(encodedString[in_]))
        {
        byte_array_4[i++] = encodedString[in_];
        in_++;
        if (i != 4)
            continue;

        for (i = 0; i < 4; i++)
            {
            byte_array_4[i] = base64_decodeindex[byte_array_4[i]];
            }

        byte_array_3[0] = (byte_array_4[0] << 2) + ((byte_array_4[1] & 0x30) >> 4);
        byte_array_3[1] = ((byte_array_4[1] & 0xf) << 4) + ((byte_array_4[2] & 0x3c) >> 2);
        byte_array_3[2] = ((byte_array_4[2] & 0x3) << 6) + byte_array_4[3];

        for (i = 0; (i < 3); i++)
            byteArray.push_back(byte_array_3[i]);

        i = 0;
        }

    if (i == 0)
        return;

    for (j = i; j < 4; j++)
        byte_array_4[j] = 0;

    for (j = 0; j < 4; j++)
        {
        byte_array_4[j] = base64_decodeindex[byte_array_4[j]];
        }

    byte_array_3[0] = (byte_array_4[0] << 2) + ((byte_array_4[1] & 0x30) >> 4);
    byte_array_3[1] = ((byte_array_4[1] & 0xf) << 4) + ((byte_array_4[2] & 0x3c) >> 2);
    byte_array_3[2] = 0xFF & (((byte_array_4[2] & 0x3) << 6) + byte_array_4[3]);


    for (j = 0; (j < i - 1); j++)
        byteArray.push_back(byte_array_3[j]);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void Base64Utilities::Decode(bvector<Byte>& byteArray, Utf8CP encodedString, size_t encodedStringLength)
    {
    base64_decode(byteArray, encodedString, encodedStringLength);
    }

//=======================================================================================
// @bsistruct
//=======================================================================================
struct ByteStreamAdapter
{
    ByteStream& m_buffer;

    ByteStreamAdapter(ByteStream& buffer, size_t srcLen) : m_buffer(buffer)
        {
        uint32_t nDecodedBytes = (uint32_t) ((3.0/4.0) * srcLen);
        m_buffer.Reserve(nDecodedBytes);
        }

    void push_back(Byte b)
        {
        if (m_buffer.GetAllocSize() <= m_buffer.GetSize())
            m_buffer.Reserve(2 * m_buffer.GetAllocSize());  // we miscalculated above??

        m_buffer.Append(&b, 1);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Base64Utilities::Decode(ByteStream& dest, Utf8CP src, size_t srcLen)
    {
    if (0 == srcLen || Utf8String::IsNullOrEmpty(src))
        return;

    ByteStreamAdapter proxy(dest, srcLen);
    base64_decode(proxy, src, srcLen);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR Base64Utilities::Alphabet() { return base64_chars; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool Base64Utilities::MatchesAlphabet(Utf8CP input)
    {
    if (Utf8String::IsNullOrEmpty(input))
        return true;

    for (Utf8CP ch = input; 0 != *ch; ++ch)
        if (!is_base64(*ch) && *ch != '=')
            return false;

    return true;
    }

