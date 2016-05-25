/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Bentley/nonport/Base64Utilities.cpp $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#include <stdlib.h>
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
bool is_base64(Byte c) { return (isalnum (c) || (c == '+') || (c == '/')); }

//--------------------------------------------------------------------------------------
// @bsimethod                                                    Tahir.Hayat    12/2012
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String Base64Utilities::Encode(Utf8CP byteArray, size_t byteCount)
    {
    Utf8String encodedString;
    if (SUCCESS != Encode(encodedString, reinterpret_cast<Byte const*> (byteArray), byteCount))
        return Utf8String();

    return encodedString;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus Base64Utilities::Encode(Utf8StringR encodedString, Byte const* byteArray, size_t byteCount)
    {
    if (byteArray == nullptr || byteCount == 0)
        return SUCCESS;

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

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Tahir.Hayat    12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Base64Utilities::Decode(Utf8CP encodedString, size_t encodedStringLength)
    {
    bvector<Byte> byteArray;
    if (SUCCESS != Decode(byteArray, encodedString, encodedStringLength) || byteArray.empty())
        return Utf8String();

    return Utf8String((Utf8CP) byteArray.data(), byteArray.size());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    03/2016
//+---------------+---------------+---------------+---------------+---------------+------
template<typename T> static BentleyStatus base64_decode(T& byteArray, Utf8CP encodedString, size_t encodedStringLength)
    {
    if (Utf8String::IsNullOrEmpty(encodedString) || encodedStringLength == 0)
        return SUCCESS;

    int i = 0;
    int j = 0;
    int in_ = 0;

    Byte byte_array_4[4], byte_array_3[3];

    while (encodedStringLength-- && (encodedString[in_] != '=') && is_base64(encodedString[in_]))
        {
        byte_array_4[i++] = encodedString[in_]; in_++;
        if (i == 4)
            {
            for (i = 0; i < 4; i++)
                byte_array_4[i] = (Byte) base64_chars.find(byte_array_4[i]);

            byte_array_3[0] = (byte_array_4[0] << 2) + ((byte_array_4[1] & 0x30) >> 4);
            byte_array_3[1] = ((byte_array_4[1] & 0xf) << 4) + ((byte_array_4[2] & 0x3c) >> 2);
            byte_array_3[2] = ((byte_array_4[2] & 0x3) << 6) + byte_array_4[3];

            for (i = 0; (i < 3); i++)
                byteArray.push_back(byte_array_3[i]);
            i = 0;
            }
        }

    if (i)
        {
        for (j = i; j < 4; j++)
            byte_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            byte_array_4[j] = (Byte) base64_chars.find(byte_array_4[j]);

        byte_array_3[0] = (byte_array_4[0] << 2) + ((byte_array_4[1] & 0x30) >> 4);
        byte_array_3[1] = ((byte_array_4[1] & 0xf) << 4) + ((byte_array_4[2] & 0x3c) >> 2);
        byte_array_3[2] = 0xFF & (((byte_array_4[2] & 0x3) << 6) + byte_array_4[3]);


        for (j = 0; (j < i - 1); j++)
            byteArray.push_back(byte_array_3[j]);
        }

    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus Base64Utilities::Decode(bvector<Byte>& byteArray, Utf8CP encodedString, size_t encodedStringLength)
    {
    return base64_decode(byteArray, encodedString, encodedStringLength);
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   05/16
//=======================================================================================
struct ByteStreamAdapter
{
    ByteStream& m_buffer;

    ByteStreamAdapter(ByteStream& buffer, size_t srcLen) : m_buffer(buffer)
        {
        uint32_t nDecodedBytes = static_cast<uint32_t>((3.0/4.0) * srcLen);
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
* @bsimethod                                                    Paul.Connelly   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Base64Utilities::Decode(ByteStream& dest, Utf8CP src, size_t srcLen)
    {
    if (0 == srcLen || Utf8String::IsNullOrEmpty(src))
        return SUCCESS;

    ByteStreamAdapter proxy(dest, srcLen);
    return base64_decode(proxy, src, srcLen);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR Base64Utilities::Alphabet() { return base64_chars; }
