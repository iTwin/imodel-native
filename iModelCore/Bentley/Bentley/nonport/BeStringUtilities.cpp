/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)

    #define NOMINMAX
    #include <Windows.h>
    #include <WinNls.h>
    #include <objbase.h>

    #if defined (BENTLEY_WIN32)
        #include <Wininet.h>
    #endif

#elif defined (__unix__)

    #include <wchar.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <errno.h>
    #include <stdlib.h>

    #if !defined (MAXHOSTNAMELEN)
        #define MAXHOSTNAMELEN 256
    #endif

    static const size_t FORMAT_RESULT_BUFFER_GUESS = 4096;

#else

    #error unknown compiler

#endif

#include <icu4c/unicode/putil.h>
#include <icu4c/unicode/ucnv.h>
#include <icu4c/unicode/ustring.h>

// Conflicts with ICU.
#define NO_UCHAR_TYPEDEF

#include <stdlib.h>

#include "../BentleyInternal.h"

#include <Bentley/BeStringUtilities.h>
#include <Bentley/BeFileName.h>
#include <Bentley/WString.h>
#include <Bentley/ScopedArray.h>
#include <Bentley/BeAssert.h>
#include <Bentley/BeThread.h>
#include <Bentley/BeId.h>
#include "strfunc.h"

#include "utf8.h"

#ifndef CP_UTF8
#define CP_UTF8 (uint32_t)LangCodePage::ISCII_UNICODE_UTF_8
#endif

#if defined (__APPLE__) || defined (ANDROID)
    static char*    strlwr_ascii_only(char* s);
    static wchar_t* wcslwr_portable(wchar_t* s);
    static char*    strupr_ascii_only(char* s);
    static wchar_t* wcsupr_portable(wchar_t* s);
    static int      wcsicmp_portable(WCharCP lhs, WCharCP rhs, size_t n);
    static wchar_t* wcsdup_portable(wchar_t const* s);

    static BentleyStatus unixUtf16ToWChar(WCharP wbuf, size_t wbufSizeInChars, Utf16CP inStr, size_t _count);
    static BentleyStatus unixWCharToUtf16(Utf16P ubuf, size_t ubufSizeInChars, WCharCP inStr, size_t count);
    static wchar_t* unixWcsncpy(wchar_t *strDest, size_t destLen, const wchar_t *strSource, size_t count);
    static char* unixStrncpy(char *strDest, size_t destLen, const char *strSource, size_t count);
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isAscii(char const* p)
    {
    for ( ; 0 != *p; ++p)
        {
        if (0 != (*p & 0x80))
            return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t utf16len(Utf16CP utf16, size_t bufSizeIncludingTerminator=BeStringUtilities::NPOS)
    {
    size_t count = 0;
    while (count < bufSizeIncludingTerminator && 0 != utf16[count])
        ++count;
    return count;
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static BentleyStatus directUtf16ToUtf8(bvector<Byte>& outStringBuff, Byte const* inString, size_t inStringNumBytes)
    {
    // UTF-16 is 2 or 4 bytes per-character; UTF-8 is 1-6... without getting clever, I believe this is a fair compromise between memory and re-allocation costs.
    outStringBuff.reserve(inStringNumBytes);

    try { utf8::utf16to8((char16_t const*)inString, (char16_t const*)(inString + inStringNumBytes), std::back_inserter(outStringBuff)); }
    catch(...) { return ERROR; }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static BentleyStatus directUtf32ToUtf8(bvector<Byte>& outStringBuff, Byte const* inString, size_t inStringNumBytes)
    {
    // UTF-32 is 4 bytes per-character; UTF-8 is 1-6 bytes... without getting clever, I believe this is a fair compromise between memory and re-allocation costs.
    outStringBuff.reserve(inStringNumBytes / 2);

    try { utf8::utf32to8((char32_t const*)inString, (char32_t const*)(inString + inStringNumBytes), std::back_inserter(outStringBuff)); }
    catch(...) { return ERROR; }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static BentleyStatus directUtf8ToUtf16(bvector<Byte>& outStringBuff, Byte const* inString, size_t inStringNumBytes)
    {
    // UTF-8 is 1-6 bytes per-character; UTF-16 is 2 or 4... without getting clever, I believe this is a fair compromise between memory and re-allocation costs.
    bvector<char16_t> utf16;
    utf16.reserve(inStringNumBytes / 2);

    try { utf8::utf8to16(inString, (inString + inStringNumBytes), std::back_inserter(utf16)); }
    catch(...) { return ERROR; }

    size_t outStringBuffNumBytes = (utf16.size() * sizeof(char16_t));
    outStringBuff.resize(outStringBuffNumBytes);
    memcpy(&outStringBuff[0], &utf16[0], outStringBuffNumBytes);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static BentleyStatus directUtf8ToUtf32(bvector<Byte>& outStringBuff, Byte const* inString, size_t inStringNumBytes)
    {
    // UTF-8 is 1-6 bytes per-character; UTF-32 is 4... without getting clever, I believe this is a fair compromise between memory and re-allocation costs.
    bvector<char32_t> utf32;
    utf32.reserve(inStringNumBytes / 2);

    try { utf8::utf8to32(inString, (inString + inStringNumBytes), std::back_inserter(utf32)); }
    catch(...) { return ERROR; }

    size_t outStringBuffNumBytes = (utf32.size() * sizeof(char32_t));
    outStringBuff.resize(outStringBuffNumBytes);
    memcpy(&outStringBuff[0], &utf32[0], outStringBuffNumBytes);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static bool isEncodingUtf8(CharCP encoding)
    {
    return ((0 == strcmp("UTF-8", encoding)) || (0 == strcmp("CP65001", encoding)));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static bool handleDirectUtfConversion(BentleyStatus& status, bvector<Byte>& outStringBuff, CharCP outEncoding, Byte const* inString, size_t inStringNumBytes, CharCP inEncoding)
    {
    // We're on non-Win32, so I feel we're most likely to get UTF-32-based requests, so strcmp those first.

    if ((0 == strcmp("UTF-32LE", inEncoding)) && isEncodingUtf8(outEncoding))
        {
        status = directUtf32ToUtf8(outStringBuff, inString, inStringNumBytes);
        return true;
        }

    if (isEncodingUtf8(inEncoding) && (0 == strcmp("UTF-32LE", outEncoding)))
        {
        status = directUtf8ToUtf32(outStringBuff, inString, inStringNumBytes);
        return true;
        }

    if ((0 == strcmp("UTF-32LE", inEncoding)) && (0 == strcmp("UTF-16LE", outEncoding)))
        {
        bvector<Byte> utf8;
        if (SUCCESS != directUtf32ToUtf8(utf8, inString, inStringNumBytes))
            status = ERROR;
        else
            status = directUtf8ToUtf16(outStringBuff, &utf8[0], utf8.size());

        return true;
        }

    if ((0 == strcmp("UTF-16LE", inEncoding)) && (0 == strcmp("UTF-32LE", outEncoding)))
        {
        bvector<Byte> utf8;
        if (SUCCESS != directUtf16ToUtf8(utf8, inString, inStringNumBytes))
            status = ERROR;
        else
            status = directUtf8ToUtf32(outStringBuff, &utf8[0], utf8.size());

        return true;
        }

    if ((0 == strcmp("UTF-16LE", inEncoding)) && isEncodingUtf8(outEncoding))
        {
        status = directUtf16ToUtf8(outStringBuff, inString, inStringNumBytes);
        return true;
        }

    if (isEncodingUtf8(inEncoding) && (0 == strcmp("UTF-16", outEncoding)))
        {
        status = directUtf8ToUtf16(outStringBuff, inString, inStringNumBytes);
        return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// outStringBuff will effectively be NULL-terminated only if inString (up to inStringNumBytes) is also NULL-terminated; otherwise it will not.
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus BeStringUtilities::TranscodeStringDirect(bvector<Byte>& outStringBuff, CharCP outEncoding, Byte const* inString, size_t inStringNumBytes, CharCP inEncoding)
    {
    // All callers (local to this file) allocate a new bvector<char> to pass to us; don't bother calling clear().
    // We cannot do an empty check on inString here because we do not know its original encoding (or rather, we don't want to have rules to handle it; let the caller do it).
    // Basic workflow is to convert to Unicode (ICU UChar, UTF-16), then to the target encoding.

    // Fun fact: We have to be able to do UTF conversions in order to initialize ICU (e.g. file path conversions on Android). Therefore, we cannot rely on ICU to do UTF conversions...
    BentleyStatus utfStatus;
    if (handleDirectUtfConversion(utfStatus, outStringBuff, outEncoding, inString, inStringNumBytes, inEncoding))
        return utfStatus;

    UErrorCode icuError = U_ZERO_ERROR;

    // Try and make the converters (early fail).
    icu::LocalUConverterPointer toUnicodeConverter(ucnv_open(inEncoding, &icuError));
    if (toUnicodeConverter.isNull() || U_FAILURE(icuError))
        { BeAssert(false); return ERROR; }

    icuError = U_ZERO_ERROR;

    icu::LocalUConverterPointer fromUnicodeConverter(ucnv_open(outEncoding, &icuError));
    if (fromUnicodeConverter.isNull() || U_FAILURE(icuError))
        { BeAssert(false); return ERROR; }

    icuError = U_ZERO_ERROR;

    // Convert to Unicode.
    bvector<UChar> unicodeBuffer;

    // Seems like a good initial guess. UChar is UTF-16, so I feel on average one locale char will go to 1 UChar.
    // At worst, this will need to double once (e.g. each input char goes to a UTF-16 surrogate pair).
    unicodeBuffer.resize(inStringNumBytes);

    int32_t unicodeBufferLength = 0;

    for (;;)
        {
        unicodeBufferLength = ucnv_toUChars(toUnicodeConverter.getAlias(), &unicodeBuffer[0], (int32_t)unicodeBuffer.size(), (CharCP)inString, (int32_t)inStringNumBytes, &icuError);
        if (U_BUFFER_OVERFLOW_ERROR == icuError)
            {
            unicodeBuffer.resize(unicodeBuffer.size() * 2);
            icuError = U_ZERO_ERROR;
            continue;
            }

        break;
        }

    if (U_FAILURE(icuError))
        { BeAssert(false); return ERROR; }

    icuError = U_ZERO_ERROR;

    // Seems like a good initial guess. The average locale is optimized to hold its characters in one byte, so one UChar will go to one char.
    // Normal worst case, this will need to double once (e.g. each input UChar goes to a multibyte pair).
    // Worst case, this will need to double twice (e.g. each input UChar goes to a 4-byte UCS-4 character).
    outStringBuff.resize(unicodeBufferLength);

    for (;;)
        {
        ucnv_fromUChars(fromUnicodeConverter.getAlias(), (CharP)&outStringBuff[0], (int32_t)outStringBuff.size(), &unicodeBuffer[0], (int32_t)unicodeBufferLength, &icuError);
        if (U_BUFFER_OVERFLOW_ERROR == icuError)
            {
            outStringBuff.resize(outStringBuff.size() * 2);
            icuError = U_ZERO_ERROR;
            continue;
            }

        break;
        }

    if (U_FAILURE(icuError))
        { BeAssert(false); return ERROR; }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t BeStringUtilities::Utf16Len(Utf16CP str)
    {
    return utf16len(str);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
WString::WString(Utf16CP in) : bwstring((wchar_t*)in) {}
#elif defined (__unix__)
WString::WString(Utf16CP in) : bwstring()
    {
    BeStringUtilities::Utf16ToWChar(*this, in);
    }
#else
#error unknown runtime
#endif

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus BeStringUtilities::Utf16ToWChar(WStringR outStr, Utf16CP inStr, size_t _count)
    {
    if ((NULL == inStr) || (0 == _count) || (0 == inStr[0]))
        {
        outStr.clear();
        return SUCCESS;
        }

    // 'count' is NOT NULL-terminated
    size_t inStrLen = utf16len(inStr, _count);
    int inStrCount = (int)( (_count == NPOS)? inStrLen: std::min(_count, inStrLen) );

#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)

    outStr.assign((WCharCP)inStr, inStrCount);

#elif defined (__unix__)

    // "LE" (little endian) is implied on all intended platforms, but being specific anyway.

    bvector<Byte> resultBytes;
    if (SUCCESS != TranscodeStringDirect(resultBytes, "UTF-32LE", (Byte const*)inStr, (sizeof (uint16_t) * inStrCount), "UTF-16LE"))
        {
        BeAssert(false && L"Unicode to Unicode should never fail.");
        return ERROR;
        }

    // basic_string objects maintain a NULL-terminated buffer internally; the value given to resize is the number of real characters.
    outStr.resize(resultBytes.size() / sizeof (uint32_t));
    memcpy((CharP)const_cast<WCharP>(outStr.data()), &resultBytes[0], resultBytes.size());

#else
#error unknown runtime
#endif

    return SUCCESS;
    }

#if defined (__unix__)
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static BentleyStatus unixUtf16ToWChar(WCharP wbuf, size_t wbufSizeInChars, Utf16CP inStr, size_t _count)
    {
    // "LE" (little endian) is implied on all intended platforms, but being specific anyway.

    bvector<Byte> resultBytes;
    if ((NULL != inStr) && (0 != _count))
        {
        // Note: 'inStrCount' does NOT include the NULL-terminator
        size_t inStrLen = utf16len(inStr, _count);
        int inStrCount = (int)( (_count == BeStringUtilities::NPOS)? inStrLen: std::min(_count, inStrLen) );

        if (SUCCESS != BeStringUtilities::TranscodeStringDirect(resultBytes, "UTF-32LE", (Byte const*)inStr, (sizeof (Utf16Char) * inStrCount), "UTF-16LE"))
            {
            BeAssert(false && L"Unicode to Unicode should never fail.");
            return ERROR;
            }
        }

    size_t nChars = std::min(wbufSizeInChars-1, resultBytes.size()/sizeof(WChar));
    memcpy(wbuf, (WCharP)resultBytes.data(), nChars*sizeof(WChar));
    wbuf[nChars] = 0;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static BentleyStatus unixWCharToUtf16(Utf16P ubuf, size_t ubufSizeInChars, WCharCP inStr, size_t _count)
    {
    // "LE" (little endian) is implied on all intended platforms, but being specific anyway.

    bvector<Byte> resultBytes;
    if ((NULL != inStr) && (0 != _count))
        {
        // Note: 'inStrCount' does NOT include the NULL-terminator
        size_t inStrLen = wcslen(inStr);
        int inStrCount = (int)( (_count == BeStringUtilities::NPOS)? inStrLen: std::min(_count, inStrLen) );

        if (SUCCESS != BeStringUtilities::TranscodeStringDirect(resultBytes, "UTF-16LE", (Byte const*)inStr, (sizeof (WChar) * inStrCount), "UTF-32LE"))
            {
            BeAssert(false && L"Unicode to Unicode should never fail.");
            return ERROR;
            }
        }

    size_t nChars = std::min(ubufSizeInChars-1, resultBytes.size()/sizeof(Utf16Char));
    memcpy(ubuf, (Utf16P)resultBytes.data(), nChars*sizeof(Utf16Char));
    ubuf[nChars] = 0;

    return SUCCESS;
    }

#endif

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus BeStringUtilities::WCharToUtf16(Utf16BufferR outStr, WCharCP inStr, size_t _count)
    {
    if ((NULL == inStr) || (0 == _count) || (0 == inStr[0]))
        {
        // Ensure that outStr is NULL-terminated
        outStr.resize(1);
        outStr[0] = 0;
        return SUCCESS;
        }

    // Note: 'inStrCount' does NOT include the NULL-terminator
    size_t inStrLen = wcslen(inStr);
    int inStrCount = (int)( (_count == NPOS)? inStrLen: std::min(_count, inStrLen) );

#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)

    outStr.resize(inStrCount+1);                                    // (allocates space for 1+inStrCount to leave room for the trailing \0)
    outStr.assign((uint16_t*)inStr, (uint16_t*)(inStr+inStrCount+1));   // copy all wchars and the trailing \0

#elif defined (__unix__)

    // "LE" (little endian) is implied on all intended platforms, but being specific anyway.

    bvector<Byte> resultBytes;
    if (SUCCESS != TranscodeStringDirect(resultBytes, "UTF-16LE", (Byte const*)inStr, (sizeof (uint32_t) * inStrCount), "UTF-32LE"))
        {
        BeAssert(false && L"Unicode to Unicode should never fail.");

        // Ensure that outStr is NULL-terminated
        outStr.resize(1);
        outStr[0] = 0;
        return ERROR;
        }

    outStr.resize((resultBytes.size() / sizeof (uint16_t)) + 1);
    memcpy((CharP)outStr.data(), &resultBytes[0], resultBytes.size());
    outStr.back() = 0;

#else
#error unknown runtime
#endif

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus BeStringUtilities::WCharToUtf8(Utf8StringR outStr, WCharCP inStr, size_t _count)
    {
    return WCharToLocaleChar((AStringR) outStr, LangCodePage::ISCII_UNICODE_UTF_8, inStr, _count);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus BeStringUtilities::Utf8ToWChar(WStringR outStr, Utf8CP inStr, size_t _count)
    {
    return LocaleCharToWChar(outStr, reinterpret_cast<CharCP>(inStr), LangCodePage::ISCII_UNICODE_UTF_8, _count);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
WCharP        BeStringUtilities::Utf8ToWChar(WCharP outWChar, Utf8CP inStr, size_t outMaxChars)
    {
    return LocaleCharToWChar(outWChar, reinterpret_cast<CharCP>(inStr), LangCodePage::ISCII_UNICODE_UTF_8, outMaxChars);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus BeStringUtilities::Utf8ToUtf16(Utf16BufferR outStr, Utf8CP inStr, size_t _count)
    {
    if ((NULL == inStr) || (0 == inStr[0]) || (0 == _count))
        {
        outStr.clear();
        return SUCCESS;
        }

    size_t inStrLen = strlen(inStr);
    int inStrCount = (int)( (_count == NPOS)? inStrLen: std::min(_count, inStrLen) );

#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)

    int outStrCount = ::MultiByteToWideChar(CP_UTF8, 0, inStr, inStrCount, NULL, 0);

    outStr.resize(outStrCount + 1);
    ::MultiByteToWideChar(CP_UTF8, 0, inStr, inStrCount, (LPWSTR)&outStr[0], outStrCount);

    outStr[outStrCount] = '\0';

#elif defined (__unix__)

    // "LE" (little endian) is implied on all intended platforms, but being specific anyway.

    bvector<Byte> resultBytes;
    if (SUCCESS != TranscodeStringDirect(resultBytes, "UTF-16LE", (Byte const*)inStr, (sizeof (char) * inStrCount), "UTF-8"))
        return ERROR;

    size_t outStrCount = (resultBytes.size() / sizeof (Utf16Char));
    outStr.resize(outStrCount + 1);
    memcpy(outStr.data(), &resultBytes[0], resultBytes.size());
    outStr[outStrCount] = '\0';

#else
#error unknown runtime
#endif

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus BeStringUtilities::Utf16ToUtf8(Utf8StringR utf8Buffer, Utf16CP utf16Buffer, size_t count)
    {
    if ((NULL == utf16Buffer) || (0 == utf16Buffer[0]) || (0 == count))
        {
        utf8Buffer.clear();
        return SUCCESS;
        }

    size_t  inStrLen    = utf16len(utf16Buffer, count);
    int     inStrCount  = (int)((NPOS == count) ? inStrLen : std::min(count, inStrLen));

#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)

    int outStrCount = ::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)utf16Buffer, inStrCount, NULL, 0, NULL, NULL);

    utf8Buffer.resize(outStrCount);
    ::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)utf16Buffer, inStrCount, const_cast<LPSTR>(utf8Buffer.data()), outStrCount, NULL, NULL);

#elif defined (__unix__)

    bvector<Byte> resultBytes;
    if (SUCCESS != TranscodeStringDirect(resultBytes, "UTF-8", (Byte const*)utf16Buffer, (sizeof (*utf16Buffer) * inStrCount), "UTF-16LE"))
        return ERROR;

    size_t outStrCount = (resultBytes.size() / sizeof (Utf8Char));

    utf8Buffer.resize(outStrCount);
    memcpy(const_cast<CharP>(utf8Buffer.data()), &resultBytes[0], resultBytes.size());
    utf8Buffer[outStrCount] = 0;

#else
#error unknown runtime
#endif

    return SUCCESS;
    }

//#define DEBUG_UNICODE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus BeStringUtilities::WCharToLocaleChar(AStringR outStr, LangCodePage codePageIn, WCharCP inStr, size_t _count)
    {
    if ((NULL == inStr) || (0 == inStr[0]) || (0 == _count))
        {
        outStr = "";
        return SUCCESS;
        }

    uint32_t codePage = (uint32_t)codePageIn;

    // Note: 'inStrCount' does NOT include the NULL-terminator
    size_t inStrLen = wcslen(inStr);
    int inStrCount = (int)( (_count == NPOS)? inStrLen: std::min(_count, inStrLen) );

#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)

    int outStrCount = ::WideCharToMultiByte(codePage, 0, inStr, inStrCount, NULL, 0, NULL, NULL);
    if (outStrCount <= 0)
        {
        outStr = "";
        return ERROR;
        }

    outStr.resize((size_t)outStrCount); // (allocates space for outStrCount + 1 chars)

    int conversionStatus = ::WideCharToMultiByte(codePage, 0, inStr, inStrCount, &outStr[0], outStrCount, NULL, NULL);
    if (0 == conversionStatus)
        {
        outStr = "";
        return ERROR;
        }

    #if !defined (NDEBUG) && defined (DEBUG_UNICODE)
        printf("Warning: BeStringUtilities::WCharToLocaleChar: '%ls'\n", inStr);
    #endif

#elif defined (__unix__)

    if (CP_UTF8 == codePage || 0 == codePage)
        {
        // optimize for common case
        outStr.resize(inStrCount);
        bool anyNotAscii = false;
        for (size_t i=0; i<inStrCount; ++i)
            {
            if ((anyNotAscii = !isascii(inStr[i])))
                break;
            outStr[i] = (char)inStr[i];
            }

        if (!anyNotAscii)
            return SUCCESS;
        }
    char cpStrBuf[16];
    char const* cpStr;
    if (CP_UTF8 == codePage)
        cpStr = "UTF-8";
    else if (0 == codePage)
        cpStr = "ASCII";
    else
        {
        BeStringUtilities::Snprintf(cpStrBuf, _countof(cpStrBuf), "CP%u", codePage);
        cpStr = cpStrBuf;
        }

    // "LE" (little endian) is implied on all intended platforms, but being specific anyway.

    bvector<Byte> resultBytes;
    if (SUCCESS != TranscodeStringDirect(resultBytes, cpStr, (Byte const*)inStr, (sizeof (WChar) * inStrCount), "UTF-32LE"))
        return ERROR;

    // basic_string objects maintain a NULL-terminated buffer internally; the value given to resize is the number of real characters.
    outStr.resize(resultBytes.size() / sizeof (char));
    memcpy(const_cast<CharP>(outStr.data()), &resultBytes[0], resultBytes.size());

#else
#error unknown runtime
#endif

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// This version of the function has the same signature as the old MSWCharToChar, which is very widely used.
// @bsimethod
//---------------------------------------------------------------------------------------
char* BeStringUtilities::WCharToCurrentLocaleChar(char* outChar, wchar_t const* inWChar, size_t outMaxBytes)
    {
    if ((NULL == outChar) || (0 == outMaxBytes))
        {
        BeDataAssert(false);
        return outChar;
        }

    LangCodePage codePage;
    BeStringUtilities::GetCurrentCodePage(codePage);

    AString astr;
    if (WCharToLocaleChar(astr, codePage, inWChar) != SUCCESS)
        {
        outChar[0] = 0;
        return outChar;
        }

    size_t outCount = std::min(astr.length(), outMaxBytes-1);     // max chars to copy INCLUDING \0
PUSH_DISABLE_DEPRECATION_WARNINGS
    strncpy(outChar, astr.c_str(), outCount);
POP_DISABLE_DEPRECATION_WARNINGS
    outChar[outCount] = 0;

    return outChar;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus BeStringUtilities::WCharToCurrentLocaleChar(AStringR localeStr, WCharCP inWChar)
    {
    LangCodePage codePage;
    BeStringUtilities::GetCurrentCodePage(codePage);

    return BeStringUtilities::WCharToLocaleChar(localeStr, codePage, inWChar);
    }

#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
void utf8ToWChar(wchar_t* outStr, size_t outStrCount, char const* inStr, size_t inStrCount)
    {
    BeAssert(outStrCount != 0);
    int actualCount = ::MultiByteToWideChar(CP_UTF8, 0, inStr, (int)inStrCount, &outStr[0], (int)outStrCount);
    actualCount = std::min<int> (actualCount, (int)outStrCount-1);
    outStr[actualCount] = 0;
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus BeStringUtilities::LocaleCharToWChar(WStringR outStr, CharCP inStr, LangCodePage codePageIn, size_t _count)
    {
    if ((NULL == inStr) || (0 == inStr[0]) || (0 == _count))
        {
        outStr = L"";
        return SUCCESS;
        }

    uint32_t codePage = (uint32_t)codePageIn;

    int inStrCount;
    if (_count == NPOS)
        inStrCount = (int)strlen(inStr);               // strlen is necessary, because MultiByteToWideChar count or does not count the NULL
    else
        inStrCount = (int)strnlen(inStr, _count);      // Need to stop at _count or less, if a NULL is not present

#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)

    int outStrCount = ::MultiByteToWideChar(codePage, 0, inStr, inStrCount, NULL, 0);

    outStr.resize(outStrCount); // (allocates space for outStrCount + 1 wchar_t's)
    ::MultiByteToWideChar(codePage, 0, inStr, inStrCount, &outStr[0], outStrCount);

    outStr[outStrCount] = '\0';

#elif defined (__unix__)

    if (CP_UTF8 == codePage || 0 == codePage)
        {
        // optimize for common case
        outStr.resize(inStrCount);
        bool anyNotAscii = false;
        for (size_t i=0; i<inStrCount; ++i)
            {
            if ((anyNotAscii = !isascii(inStr[i])))
                break;
            outStr[i] = (wchar_t)inStr[i];
            }

        if (!anyNotAscii)
            return SUCCESS;
        }

    char cpStrBuf[16];
    char const* cpStr;
    if (CP_UTF8 == codePage)
        cpStr = "UTF-8";
    else if (0 == codePage)
        cpStr = "ASCII";
    else
        {
        BeStringUtilities::Snprintf(cpStrBuf, _countof(cpStrBuf), "cp%u", codePage);
        cpStr = cpStrBuf;
        }

    // "LE" (little endian) is implied on all intended platforms, but being specific anyway.

    bvector<Byte> resultBytes;
    if (SUCCESS != TranscodeStringDirect(resultBytes, "UTF-32LE", (Byte const*)inStr, (sizeof (char) * inStrCount), cpStr))
        return ERROR;

    // basic_string objects maintain a NULL-terminated buffer internally; the value given to resize is the number of real characters.
    outStr.resize(resultBytes.size() / sizeof (WChar));
    memcpy(const_cast<WCharP>(outStr.data()), &resultBytes[0], resultBytes.size());

#else
#error unknown runtime
#endif

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
WCharP  BeStringUtilities::LocaleCharToWChar(WCharP outWChar, CharCP inChar, LangCodePage codePageIn, size_t outMaxChars)
    {
    if ((NULL == outWChar) || (0 == outMaxChars))
        {
        BeDataAssert(false);
        return outWChar;
        }

    uint32_t codePage = (uint32_t)codePageIn;

    // no input?
    if ((NULL == inChar) || (0 == inChar[0]))
        {
        *outWChar = 0;
        return outWChar;
        }

    // get length of input string.
    size_t inStrLen = strlen(inChar);

#if defined (_WIN32)

    size_t  outChars = ::MultiByteToWideChar(codePage, 0, inChar, (int)inStrLen, outWChar, (int)(outMaxChars-1)    );
    outWChar[outChars] = 0;

#elif defined (__unix__)

    if (CP_UTF8 == codePage || 0 == codePage)
        {
        // optimize for common case
        size_t outChars = std::min(inStrLen, outMaxChars-1);
        bool anyNotAscii = false;
        for (size_t i=0; i<outChars; ++i)
            {
            if ((anyNotAscii = !isascii(inChar[i])))
                break;
            outWChar[i] = (wchar_t)inChar[i];
            }
        outWChar[outChars] = 0;

        if (!anyNotAscii)
            return outWChar;
        }

    char cpStrBuf[16];
    char const* cpStr;
    if (CP_UTF8 == codePage)
        cpStr = "UTF-8";
    else if (0 == codePage)
        cpStr = "ASCII";
    else
        {
        BeStringUtilities::Snprintf(cpStrBuf, _countof(cpStrBuf), "CP%u", codePage);
        cpStr = cpStrBuf;
        }

    // "LE" (little endian) is implied on all intended platforms, but being specific anyway.
    bvector<Byte> resultBytes;
    if (SUCCESS != TranscodeStringDirect(resultBytes, "UTF-32LE", (Byte const*)inChar, (sizeof (char) * inStrLen), cpStr))
        return outWChar;

    // basic_string objects maintain a NULL-terminated buffer internally; the value given to resize is the number of real characters.
    size_t  copySize = std::min(resultBytes.size(), ((outMaxChars-1) * sizeof (WChar)));
    memcpy(outWChar, &resultBytes[0], copySize);
    outWChar[copySize/sizeof(WChar)] = 0;

#endif

    return outWChar;
    }

//---------------------------------------------------------------------------------------
// This version of the function has the same signature as the old CharToMSWChar, which is very widely used.
// @bsimethod
//---------------------------------------------------------------------------------------
wchar_t* BeStringUtilities::CurrentLocaleCharToWChar(wchar_t* outWChar, char const* inStr, size_t outMaxChars)
    {
    LangCodePage codePage;
    BeStringUtilities::GetCurrentCodePage(codePage);

    return LocaleCharToWChar(outWChar, inStr, codePage, outMaxChars);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus BeStringUtilities::CurrentLocaleCharToWChar(WStringR outStr, char const* inStr)
    {
    LangCodePage codePage;
    BeStringUtilities::GetCurrentCodePage(codePage);

    return LocaleCharToWChar(outStr, inStr, codePage);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WString::VSprintf(WCharCP format, va_list argptr)
    {
    auto result = BeWStringSprintf(*this, format, argptr);
    return (result < 0 || result > (int)size())? BSIERROR: BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Utf8String::VSprintf(Utf8CP format, va_list argptr)
    {
    auto result = BeUtf8StringSprintf(*this, format, argptr);
    return (result < 0 || result > (int)size())? BSIERROR: BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Utf8String::Sprintf(Utf8CP format, ...)
    {
    va_list args;
    va_start(args, format);
    auto result = BeUtf8StringSprintf(*this, format, args);
    va_end(args);

    if (result < 0)
        return BSIERROR;

    if (result > (int)size()) // on *nix, the initial attempt may fail, because it can only guess at the length of the formatted string.
        {                // Note that we have to re-create 'args' in order make a second attempt.
        va_start(args, format);
        result = BeUtf8StringSprintf(*this, format, args, result);
        va_end(args);

        if (result < 0)
            return BSIERROR;
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8PrintfString::Utf8PrintfString(Utf8CP format, ...) : Utf8String()
    {
    va_list args;
    va_start(args, format);
    auto result = BeUtf8StringSprintf(*this, format, args);
    va_end(args);

    if (result < 0)
        return;

    if (result > (int)size()) // on *nix, the initial attempt may fail, because it can only guess at the length of the formatted string.
        {                // Note that we have to re-create 'args' in order make a second attempt.
        va_start(args, format);
        result = BeUtf8StringSprintf(*this, format, args, result);
        va_end(args);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8PrintfString Utf8PrintfString::CreateFromVaList(Utf8CP format, va_list args)
    {
    Utf8PrintfString str;

    // must make this copy before starting to use `args` and its state is modified
    va_list argsCopy;
    va_copy(argsCopy, args);

    auto result = BeUtf8StringSprintf(str, format, args);
    if (result < 0)
        {
        va_end(argsCopy);
        return str;
        }

    if (result > (int)str.size()) // on *nix, the initial attempt may fail, because it can only guess at the length of the formatted string.
        {
        // note that 'argsCopy' is used to make the second attempt
        result = BeUtf8StringSprintf(str, format, argsCopy, result);
        }

    va_end(argsCopy);
    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WString::Sprintf(WCharCP format, ...)
    {
#if defined (_WIN32)
    va_list args;
    va_start(args, format);
    auto result = BeWStringSprintf(*this, format, args);
    va_end(args);

    if (result < 0)
        return BSIERROR;

    if (result > (int)size()) // on *nix, the initial attempt may fail, because it can only guess at the length of the formatted string.
        {                // Note that we have to re-create 'args' in order make a second attempt.
        va_start(args, format);
        result = BeWStringSprintf(*this, format, args, result);
        va_end(args);

        if (result < 0)
            return BSIERROR;
        }

    return BSISUCCESS;
#elif defined (__unix__)
    va_list args;
    size_t resultBufferGuess = 0;
    int formatResult = -1;

    while (formatResult < 0)
        {
        resultBufferGuess += FORMAT_RESULT_BUFFER_GUESS;
        va_start(args, format);
        formatResult = BeWStringSprintf(*this, format, args, resultBufferGuess);
        va_end(args);
        }

    return BSISUCCESS;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
WPrintfString::WPrintfString(WCharCP format, ...) : WString()
    {
    va_list args;
    va_start(args, format);
    auto result = BeWStringSprintf(*this, format, args);
    va_end(args);

    if (result < 0)
        return;

    if (result > (int)size()) // on *nix, the initial attempt may fail, because it can only guess at the length of the formatted string.
        {                // Note that we have to re-create 'args' in order make a second attempt.
        va_start(args, format);
        result = BeWStringSprintf(*this, format, args, result);
        va_end(args);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
WPrintfString::WPrintfString(WCharCP format, va_list args) : WString()
    {
    auto result = BeWStringSprintf(*this, format, args);

    if (result < 0)
        return;

    if (result > (int)size()) // on *nix, the initial attempt may fail, because it can only guess at the length of the formatted string.
        BeWStringSprintf(*this, format, args, result);
    }

#if defined (__unix__)
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static int wcsicmp_portable(WCharCP lhs, WCharCP rhs, size_t n)
    {
    // I have yet to find an equivalent. Some systems have wcscasecmp, but this is POSIX 2008, and not on Android, for example.

    if (n==0)
        return 0;

    WChar lhsChar;
    WChar rhsChar;

    do  {
        lhsChar = towlower(*lhs);
        ++lhs;

        rhsChar = towlower(*rhs);
        ++rhs;
        }
    while ((0 != lhsChar) && (lhsChar == rhsChar) && (0 != --n));

    return (int)(lhsChar - rhsChar);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BeStringUtilities::GetCurrentCodePage(LangCodePage& cp)
    {
#if defined (BENTLEY_WIN32)
    cp = static_cast<LangCodePage>(::GetACP());
    return SUCCESS;

#elif defined (BENTLEY_WINRT)

    CPINFOEXW info;
    if (!GetCPInfoExW (CP_ACP, 0, &info))
        return ERROR;
    cp = static_cast<LangCodePage>(info.CodePage);
    return SUCCESS;

#elif defined (__unix__)
    cp = LangCodePage::None; // ASCII        WIP_NONPORT - check an envvar?
    return ERROR;

#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeStringUtilities::IsValidCodePage(LangCodePage codePage)
    {
#if defined (_WIN32)
    return 0 != ::IsValidCodePage(static_cast<uint32_t>(codePage));
#elif defined (__unix__)
    // WIP_NONPORT
    return (uint32_t)-1 != static_cast<uint32_t>(codePage);
#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BeStringUtilities::GetDecimalSeparator(WStringR sep)
    {
#if defined (BENTLEY_WIN32)||defined (BENTLEY_WINRT)

    wchar_t decimalBuffer[4] = {0};
    BentleyStatus status = 0 != GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_SDECIMAL, decimalBuffer, 4) ? BSISUCCESS : BSIERROR;
    if (status)
        decimalBuffer[0] = '.';

    sep.assign(decimalBuffer);
    return status;

#elif defined (__unix__)

    // WIP_NONPORT
    sep.assign(L".");
    return SUCCESS;

#else
#error unknown runtime
#endif
    }

// Characters that are safe to put into a URI
static const char s_safeForUri[256] =
    {
    /*      0 1 2 3  4 5 6 7  8 9 A B  C D E F */
    /* 0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* 1 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* 2 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* 3 */ 1,1,1,1, 1,1,1,1, 1,1,0,0, 0,0,0,0,

    /* 4 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
    /* 5 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,
    /* 6 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
    /* 7 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,

    /* 8 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* 9 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* A */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* B */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,

    /* C */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* D */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* E */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* F */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BeStringUtilities::UriEncode(Utf8CP charsToEncode)
    {
    if (NULL == charsToEncode)
        return Utf8String();

    const char DEC2HEX[16 + 1] = "0123456789ABCDEF";

    const int      length = (const int) strlen(charsToEncode);
    unsigned char* start  = new unsigned char[length * 3];
    unsigned char* end    = start;
    Utf8CP         srcEnd = charsToEncode + length;

    for ( ; charsToEncode < srcEnd; charsToEncode++)
        {
        if (s_safeForUri[(uint8_t)*charsToEncode])
            {
            *end++ = *charsToEncode;
            }
        else
            {
            *end++ = '%';
            *end++ = DEC2HEX[(unsigned char) *charsToEncode >> 4];
            *end++ = DEC2HEX[(unsigned char) *charsToEncode & 0x0F];
            }
        }

    Utf8String result((Utf8CP) start, (Utf8CP) end);
    delete [] start;
    return result;
    }

// Adapted from http://www.geekhideout.com/urlcode.shtml

/* Converts a hex character to its integer value */
char from_hex(char ch) {
  return isdigit(ch) ? ch - '0' : static_cast <char> (tolower(ch)) - 'a' + 10;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String BeStringUtilities::UriDecode(Utf8CP start, Utf8CP end)
    {
    Utf8String decoded;
    decoded.reserve(end-start); // that will be at least long enough

    for (Utf8CP pstr = start; pstr < end; ++pstr)
        {
        if (*pstr == '%')
            {
            if (pstr[1] && pstr[2])
                {
                decoded.append(1, static_cast <Utf8Char> ((from_hex(pstr[1]) << 4 | from_hex(pstr[2]))));
                pstr += 2;
                }
            }
        else if (*pstr == '+') // Some encoders turn space into '+'
            {
            decoded.append(1, ' ');
            }
        else
            {
            decoded.append(1, *pstr);
            }
        }
    return decoded;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String BeStringUtilities::UriDecode(Utf8CP str)
    {
    return UriDecode(str, str+strlen(str));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void BeStringUtilities::Split(WCharCP str, WCharCP delimiters, WCharCP escapeChars, bvector<WString>& tokens)
    {
    size_t lengthOfStr = wcslen(str);
    ScopedArray<WChar> copyOfStrBuffer(lengthOfStr + 1);
    WCharP copyOfStr = copyOfStrBuffer.GetData();

    Wcsncpy(copyOfStr, (lengthOfStr + 1), str);

    WCharP tokenContext;
    WCharCP token = Wcstok(copyOfStr, delimiters, &tokenContext);
    bool hasEscapeChars = !WString::IsNullOrEmpty(escapeChars);

    while (NULL != token)
        {
        if (hasEscapeChars
            && !tokens.empty() // has a previous token
            && NULL != wcschr(escapeChars, *tokens.back().rbegin()) // previous token ends in an escape char
            && (tokens.back().size() <= 1 || NULL == wcschr(escapeChars, *(tokens.back().rbegin() + 1))) // previous token is a single character or second-to-last is not an escape char (to escape the escape char)
            )
            tokens.back() += token;
        else
            tokens.push_back(token);

        tokens.back().Trim();

        token = Wcstok(NULL, delimiters, &tokenContext);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void BeStringUtilities::Split(Utf8CP str, Utf8CP delimiters, Utf8CP escapeChars, bvector<Utf8String>& tokens)
    {
    size_t lengthOfStr = strlen(str);
    ScopedArray<char> copyOfStrBuffer(lengthOfStr + 1);
    Utf8P copyOfStr = copyOfStrBuffer.GetData();

    BeStringUtilities::Strncpy(copyOfStr, (lengthOfStr + 1), str);

    Utf8P tokenContext;
    Utf8CP token = BeStringUtilities::Strtok(copyOfStr, delimiters, &tokenContext);
    bool hasEscapeChars = !Utf8String::IsNullOrEmpty(escapeChars);

    while (NULL != token)
        {
        if (hasEscapeChars
            && !tokens.empty() // has a previous token
            && NULL != strchr(escapeChars, *tokens.back().rbegin()) // previous token ends in an escape char
            && (tokens.back().size() <= 1 || NULL == strchr(escapeChars, *(tokens.back().rbegin() + 1))) // previous token is a single character or second-to-last is not an escape char (to escape the escape char)
            )
            tokens.back() += token;
        else
            tokens.push_back(token);

        tokens.back().Trim();

        token = BeStringUtilities::Strtok(NULL, delimiters, &tokenContext);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static Utf8String joinStrings(bvector<T> const& strings, Utf8CP delim)
    {
    Utf8String out;
    if (0 < strings.size())
        {
        out.append (strings[0]);

        for (size_t i = 1; i < strings.size(); i++)
            {
            if (NULL != delim)
                out.append (delim);

            out.append (strings[i]);
            }
        }

    return out;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BeStringUtilities::Join (bvector<Utf8String> const& strings, Utf8CP delim)
    {
    return joinStrings(strings, delim);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BeStringUtilities::Join(bvector<Utf8CP> const& strings, Utf8CP delim)
    {
    return joinStrings(strings, delim);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool charIsInString(WCharCP str, WChar ch) { return nullptr != wcschr(str, ch); }
static bool charIsInString(Utf8CP str, Utf8Char ch) { return nullptr != strchr(str, ch); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static void     parseIntoArgcArgv
(
T const*       inString,        //  => pointer to string to be parsed
T            **argv,            // <=> where to build argv array; NULL means don't build array
T*             args,            // <=> where to place argument text; NULL means don't store text
uint32_t      *numargs,         // <=  returns number of argv entries created
uint32_t      *numchars,        // <=  number of characters used in args buffer
T const*       allDelimiters    // =>  the characters that can be used as separators/delimiters
)
    {
    T const*     p;
    int          inquote;        // 1 = inside quotes
    int          copychar;       /* 1 = copy char to *args */
    unsigned int numslash;       /* num of backslashes seen */

    *numchars = *numargs = 0;

    p = inString;

    if (argv)
        *argv = args;

    if (args)
        *args = *p;

    ++*numchars;

    inquote = 0;

    /* loop on each argument */
    for (;;)
        {
        if (*p)
            {
            while (' ' == *p  || '\t' == *p)
                ++p;
            }

        if ( ! *p)
            break;              /* end of args */

        /* scan an argument */
        if (argv)
            *argv++ = args;     /* store ptr to arg */
        ++*numargs;

        /* loop through scanning one argument */
        for (;;)
            {
            copychar = 1;
            /* Rules: 2N backslashes + " ==> N backslashes and begin/end quote
               2N+1 backslashes + " ==> N backslashes + literal "
               N backslashes ==> N backslashes */
            numslash = 0;
            while (*p == '\\')
                {
                /* count number of backslashes for use below */
                ++p;
                ++numslash;
                }

            if ('"' == *p)
                {
                /* If 2N backslashes before, start/end quote, otherwise copy literally */
                if ( ! (numslash % 2) )
                    {
                    if (inquote)
                        {
                        if ('"' == p[1])
                            p++;    /* Double quote inside quoted string */
                        else        /* skip first quote char and copy second */
                            copychar = 0;
                        }
                    else
                        {
                        copychar = 0;       /* don't copy quote */
                        }

                    inquote = !inquote;
                    }
                numslash /= 2;          /* divide numslash by two */
                }

            /* copy slashes */
            while (numslash--)
                {
                if (args)
                    *args++ = '\\';
                ++*numchars;
                }

            /* if at end of arg, break loop */
            if (0 == *p)
                break;

            if (nullptr != allDelimiters)
                {
                if (!inquote && charIsInString(allDelimiters, *p))
                    {
                    ++p;
                    break;
                    }
                }
            else if (!inquote && (' ' == *p || '\t' == *p))
                {
                ++p;
                break;
                }

            /* copy character into argument */
            if (copychar)
                {
                if (args)
                    *args++ = *p;
                ++*numchars;
                }
            ++p;

            }

        /* null-terminate the argument */
        if (args)
            *args++ = '\0';             /* terminate string */
        ++*numchars;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename C, typename S> static void parseArguments(bvector<S>& subStrings, C const* inString, C const* auxDelimiters)
    {
    uint32_t argc, numchars;

    // if there are auxDelimiters, add them to the usual space and tab.
    C const* allDelimiters = nullptr;
    S allDelimiterString;
    if (nullptr != auxDelimiters)
        {
        allDelimiterString.append(1, ' ').append(1, '\t');
        allDelimiterString.append(auxDelimiters);
        allDelimiters = allDelimiterString.c_str();
        }

    // Determine the space needed for argc/argv
    parseIntoArgcArgv<C>(inString, nullptr, nullptr, &argc, &numchars, allDelimiters);

    IndexedScopedArray<C*, 32*sizeof(C*)> argv(argc+1);
    IndexedScopedArray<C> argStrings(numchars+1);

    // Actually split up the arguments
    parseIntoArgcArgv<C>(inString, argv.GetData(), argStrings.GetData(), &argc, &numchars, allDelimiters);

    for (uint32_t iArg = 0; iArg < argc; iArg++)
        subStrings.push_back(argv[iArg]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            BeStringUtilities::ParseArguments(bvector<WString>& subStrings, WCharCP inString, WCharCP auxDelimiters)
    {
    parseArguments(subStrings, inString, auxDelimiters);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            BeStringUtilities::ParseArguments(bvector<Utf8String>& subStrings, Utf8CP inString, Utf8CP auxDelimiters)
    {
    parseArguments(subStrings, inString, auxDelimiters);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t        BeStringUtilities::ParseArguments(WCharCP inString, uint32_t numSubStrings, ...)
    {
    uint32_t numParsed = 0;
    uint32_t argc, numchars;

    // Determine the space needed for argc/argv
    parseIntoArgcArgv<WChar>(inString, nullptr, nullptr, &argc, &numchars, nullptr);

    IndexedScopedArray<WChar*, 32*sizeof(WChar*)> argv(argc+1);
    IndexedScopedArray<WChar> argStringsArray(numchars+1);

    // Actually split up the arguments
    parseIntoArgcArgv<WChar>(inString, argv.GetData(), argStringsArray.GetData(), &argc, &numchars, nullptr);

    /* localization block for processing variable arguments */
        {
        WStringP outStr;
        uint32_t iArg;

        va_list args;
        va_start(args, numSubStrings /*last fixed arg*/);

        for (iArg = 0; iArg < numSubStrings && iArg < argc; iArg++)
            {
            outStr = va_arg(args, WStringP);

            if (NULL != outStr)
                outStr->assign(argv[iArg]);
            }

        numParsed = iArg;

        va_end(args);
        }

    return numParsed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            BeStringUtilities::ParseDelimitedString (bvector<WString>& subStrings, WCharCP inString, WCharCP delimiters)
    {
    uint32_t argc, numchars;

    // Determine the space needed for argc/argv
    parseIntoArgcArgv<WChar>(inString, nullptr, nullptr, &argc, &numchars, delimiters);

    IndexedScopedArray<WChar*, 32*sizeof(WChar*)> argv(argc+1);
    IndexedScopedArray<WChar> argStringsArray(numchars+1);

    // Actually split up the arguments
    parseIntoArgcArgv<WChar>(inString, argv.GetData(), argStringsArray.GetData(), &argc, &numchars, delimiters);

    for (uint32_t iArg = 0; iArg < argc; iArg++)
        subStrings.push_back(argv[iArg]);
    }

#if defined (__unix__)
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static WCharP wcsdup_portable(wchar_t const* s)
    {
    size_t len = wcslen(s);
    WCharP cc = (WCharP) malloc((len+1)*sizeof(wchar_t));
    wcscpy(cc, s);
    return cc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static char* unixStrncpy(char *strDest, size_t destLen, const char *strSource, size_t count)
    {
    if (count >= destLen)
        count = destLen-1;
    strncpy(strDest, strSource, count);
    strDest[count] = 0;
    return strDest;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static wchar_t* unixWcsncpy(wchar_t *strDest, size_t destLen, const wchar_t *strSource, size_t count)
    {
    if (count >= destLen)
        count = destLen-1;
    wcsncpy(strDest, strSource, count);
    strDest[count] = 0;
    return strDest;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static char* strlwr_ascii_only(char* s)
    {
    for (char* p = s; *p; ++p)
        *p = (char)tolower(*p);
    return s;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
char* strupr_ascii_only(char* s)
    {
    for (char* p = s; *p; ++p)
        *p = (char)toupper(*p);
    return s;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
wchar_t* wcslwr_portable(wchar_t* s)
    {
    for (wchar_t* p = s; *p; ++p)
        *p = towlower(*p);
    return s;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
wchar_t* wcsupr_portable(wchar_t* s)
    {
    for (wchar_t* p = s; *p; ++p)
        *p = towupper(*p);
    return s;
    }

#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BeStringUtilities::Utf16ToWChar(WCharP wbuf, size_t wbufSizeInChars, Utf16CP inStr, size_t count)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    Wcsncpy(wbuf, wbufSizeInChars, (WCharCP)inStr, count);
    return SUCCESS;
#elif defined (__unix__)
    return unixUtf16ToWChar(wbuf, wbufSizeInChars, inStr, count);
#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BeStringUtilities::WCharToUtf16(Utf16P ubuf, size_t ubufSizeInChars, WCharCP inStr, size_t count)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    Wcsncpy((WCharP)ubuf, ubufSizeInChars, inStr, count);
    return SUCCESS;
#elif defined (__unix__)
    return unixWCharToUtf16(ubuf, ubufSizeInChars, inStr, count);
#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int BeStringUtilities::CompareUtf16(Utf16CP s1, Utf16CP s2)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    return wcscmp((wchar_t*)s1, (wchar_t*)s2);
#elif defined (__unix__)
    // *** WIP_NONPORT
    return WString(s1).CompareTo(WString(s2));
#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int BeStringUtilities::CompareUtf16WChar(Utf16CP s1, WCharCP s2)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    return wcscmp((wchar_t*)s1, s2);
#elif defined (__unix__)
    // *** WIP_NONPORT
    return WString(s1).CompareTo(s2);
#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeStringUtilities::CopyUtf16(Utf16P outStr, size_t outStrCapacity, Utf16CP inStr)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    Wcsncpy((wchar_t*)outStr, outStrCapacity, (wchar_t*)inStr);
#elif defined (__unix__)
    if (0 == outStrCapacity)
        return;
    Utf16CP x = ((BeStringUtilities::AsManyAsPossible == outStrCapacity) ? ((Utf16CP)-1) : (outStr + outStrCapacity - 1));
    while (*inStr && outStr < x)
        *outStr++ = *inStr++;
    *outStr = 0;
#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int BeStringUtilities::Snprintf(CharP buffer, size_t numCharsInBuffer, CharCP format, ...)
    {
    va_list args;
    va_start(args, format);
    int count = Bevsnprintf(buffer, numCharsInBuffer, format, args);
    va_end(args);
    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int BeStringUtilities::Vsnprintf(CharP buffer, size_t numCharsInBuffer, CharCP format, va_list args)
    {
    return Bevsnprintf(buffer, numCharsInBuffer, format, args);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int BeStringUtilities::Snwprintf(WCharP buffer, size_t numCharsInBuffer, WCharCP format, ...)
    {
    va_list args;
    va_start(args, format);
    int count = Bevsnwprintf(buffer, numCharsInBuffer, format, args);
    va_end(args);
    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int BeStringUtilities::Vsnwprintf(WCharP buffer, size_t numCharsInBuffer, WCharCP format, va_list args)
    {
    return Bevsnwprintf(buffer, numCharsInBuffer, format, args);
    }

#ifdef _MSC_VER
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static _locale_t getEnUsLocale()
    {
    // Microsoft does not support UTF-8 for the format string.
    // While you'd be correct in thinking the specifier characters it cares about are ASCII, it also validates that you have a valid locale string, meaning it doesn't end in an invalid locale multi-byte sequence.
    // Thus, a valid UTF-8 string can trigger the invalid parameter handler (i.e. crash by default) if its last byte happens to be a locale multi-byte lead byte (thus leaving a dangling invalid locale multi-byte sequence).
    // This applies to both the format string, and any %s parameters.
    // Note that you cannot pick apart or re-create va_list objects on-the-fly, so by the time we're in this function, we don't have the ability to remap to wprintf, which in theory is what Microsoft wants.
    // The best workaround I've come up with is to force the en_US locale. I believe apps do NOT need actual custom formatting per locale via printf, and en_US is a single-byte locale, thus side-stepping the multi-byte check.
    // In a future branch, we should investigate third-party libraries to better correct this scenario.

    static bool s_isCreated = false;
    static _locale_t s_enUsLocale;

    if (!s_isCreated)
        {
        s_enUsLocale = _create_locale(LC_CTYPE, "en-US");
        s_isCreated = true;
        }

    return s_enUsLocale;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int BentleyApi::BeUtf8StringGuessLength(CharCP fmt, va_list ap)
    {
#ifdef _MSC_VER
    return _vscprintf_p_l(fmt, getEnUsLocale(), ap);
#else
    return FORMAT_RESULT_BUFFER_GUESS;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int BentleyApi::BeWStringGuessLength(WCharCP fmt, va_list ap)
    {
#ifdef _MSC_VER
    return _vscwprintf_p(fmt, ap);
#else
    return FORMAT_RESULT_BUFFER_GUESS;
#endif
    }

#ifndef BENTLEY_CPP_MISSING_WCHAR_SUPPORT
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int BentleyApi::BeUtf8StringSprintf(Utf8String& outStr, CharCP fmt, va_list ap, int initialLengthGuess)
    {
    // See Notes in BeWStringSprintf

    if (initialLengthGuess == -1)
        initialLengthGuess = BeUtf8StringGuessLength(fmt, ap);

    // Format is invalid
    if (initialLengthGuess < 0)
        return -1;

    ScopedArray<Utf8Char> buffer (initialLengthGuess + 1);

#ifdef _MSC_VER
        auto result = _vsprintf_p_l(buffer.GetData(), initialLengthGuess + 1, fmt, getEnUsLocale(), ap);
#else
        auto result = vsnprintf(buffer.GetData(), initialLengthGuess + 1, fmt, ap);
#endif

    if (result < 0)
        return result;

    outStr.assign(buffer.GetData());

    BeAssert(result >= (int)outStr.size());

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int BentleyApi::BeWStringSprintf(WString& outStr, WCharCP fmt, va_list ap, int initialLengthGuess)
    {
    // *** NB: DO NOT MAKE TWO PASSES, calling vsnprintf once to get the length and again to do the formatting.
    // ***     vsnprintf modifies ap. The second time you call it, ap is garbage.

    // Note on return value. On Windows (MSVC), we'll never truncate, because BeWStringGuessLength will return an accurate
    //  measurement. On *nix, the return value will always be the length of the full formatted string, even if it was truncated
    // and not all of it was written to the output buffer.

    if (initialLengthGuess == -1)
        initialLengthGuess = BeWStringGuessLength(fmt, ap);

    // Format is invalid
    if (initialLengthGuess < 0)
        return -1;

    ScopedArray<wchar_t> buffer (initialLengthGuess + 1);    // output buffer must have space for trailing \0

#ifdef _MSC_VER
    auto result = _vswprintf_p(buffer.GetData(), initialLengthGuess + 1, fmt, ap); // output buffer must have space for trailing \0
#else
    auto result = vswprintf(buffer.GetData(), initialLengthGuess + 1, fmt, ap); // output buffer must have space for trailing \0
#endif

    if (result < 0)   // Note: on Windows (MSVC) result will never by < 0. That's because BeWStringGuessLength (which calls _vscwprintf) will always give an accurate size for buffer, and so the formatted string will always fit.
        return result;

    outStr.assign(buffer.GetData());

    BeAssert(result <= (int)outStr.size());

    return result;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int BentleyApi::Bevsnprintf(CharP buffer, size_t numCharsInBuffer, CharCP fmt, va_list ap)
    {
    //                                         ^^^^^^^^^^^^^^^^ Note that numCharsInBuffer includes trailing \0

    //  Format the whole string, making it as long as necessary
    Utf8String s;
    int res = BeUtf8StringSprintf(s, fmt, ap); // We must let the first guess call Be...GuessLength, which on Windows will be the correct length. That's how we avoid a return value of -1
    if (EOF == res)
        {
        if (NULL != buffer && 0 != numCharsInBuffer)
            *buffer = 0;
        return EOF;
        }

PUSH_DISABLE_DEPRECATION_WARNINGS
    //  Truncate the formatted string as necessary to fit into the output buffer
    if (NULL != buffer)
        {
        if (s.size() < numCharsInBuffer) // enough space for string AND trailing \0?
            strcpy(buffer, s.c_str());
        else if (numCharsInBuffer > 0)
            {                       // truncate
            strncpy(buffer, s.c_str(), numCharsInBuffer);
            buffer[numCharsInBuffer-1] = 0;
            }
        }
POP_DISABLE_DEPRECATION_WARNINGS

    if ((numCharsInBuffer==0) && (!s.empty()))// For strings that must be truncated, this function mimics the Windows behavior of copying as much as will fit and then returning -1
        return EOF;

    if (s.size() > numCharsInBuffer-1)   // For strings that must be truncated, this function mimics the Windows behavior of copying as much as will fit and then returning -1
        return EOF;

    return (int)s.size(); // the return value is the number of characters (EXCLUDING the terminating null byte)
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int BentleyApi::Bevsnwprintf(WCharP buffer, size_t numCharsInBuffer, WCharCP fmt, va_list ap)
    {
    //                                            ^^^^^^^^^^^^^^^^ Note that numCharsInBuffer includes trailing \0

    //  Format the whole string, making it as long as necessary
    WString s;
    int res = BeWStringSprintf(s, fmt, ap); // We must let the first guess call Be...GuessLength, which on Windows will be the correct length. That's how we avoid a return value of -1
    if (res < 0)
        {
        if (NULL != buffer && 0 != numCharsInBuffer)
            *buffer = 0;
        return EOF;
        }

PUSH_DISABLE_DEPRECATION_WARNINGS
    //  Truncate the formatted string as necessary to fit into the output buffer
    if (NULL != buffer)
        {
        if (s.size() < numCharsInBuffer) // enough space for string AND trailing \0?
            wcscpy(buffer, s.c_str());
        else if (numCharsInBuffer > 0)
            {                       // truncate
            wcsncpy(buffer, s.c_str(), numCharsInBuffer);
            buffer[numCharsInBuffer-1] = 0;
            }
        }
POP_DISABLE_DEPRECATION_WARNINGS

    if ((numCharsInBuffer==0) && (!s.empty()))// For strings that must be truncated, this function mimics the Windows behavior of copying as much as will fit and then returning -1
        return EOF;

    if (s.size() > numCharsInBuffer-1)   // For strings that must be truncated, this function mimics the Windows behavior of copying as much as will fit and then returning -1
        return EOF;

    return (int)s.size(); // the return value is the number of characters (EXCLUDING the terminating null byte)
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
char* BeStringUtilities::Strlwr(char* s)
    {
    if (!isAscii(s))
        {
        WString w(s, true);
        w.ToLower();
        Utf8String a(w);
        size_t alen = strlen(a.c_str()); // don't use a.size(). That's often larger than the actual length. The transcode logic resizes the buffer without being exact.
        if (alen > strlen(s))
            {
            BeAssert(false);
            return s;
            }
PUSH_DISABLE_DEPRECATION_WARNINGS
        strcpy(s, a.c_str());
POP_DISABLE_DEPRECATION_WARNINGS
        return s;
        }

#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
PUSH_DISABLE_DEPRECATION_WARNINGS
    return _strlwr(s);
POP_MSVC_IGNORE
#elif defined (__unix__)
    return strlwr_ascii_only(s);

#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
wchar_t* BeStringUtilities::Wcslwr(wchar_t* s)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
PUSH_DISABLE_DEPRECATION_WARNINGS
    return _wcslwr(s);
POP_DISABLE_DEPRECATION_WARNINGS
#elif defined (__unix__)
    return wcslwr_portable(s);
#else
#error unknown runtime
#endif
    }

PUSH_DISABLE_DEPRECATION_WARNINGS
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
char* BeStringUtilities::Strupr(char* s)
    {
    if (!isAscii(s))
        {
        WString w(s, true);
        w.ToUpper();
        Utf8String a(w);
        if (a.size() > strlen(s))
            {
            BeAssert(false);
            return s;
            }
        strcpy(s, a.c_str());
        return s;
        }

#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    return _strupr(s);
#elif defined (__unix__)
    return strupr_ascii_only(s);
#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
wchar_t* BeStringUtilities::Wcsupr(wchar_t* s)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    return _wcsupr(s);
#elif defined (__unix__)
    return wcsupr_portable(s);
#else
#error unknown runtime
#endif
    }
POP_DISABLE_DEPRECATION_WARNINGS

static char inline asciiTolower(char c) { return ((c >= 'A') && (c <= 'Z')) ? (c - 'A' + 'a') : c; }
/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int BeStringUtilities::StricmpAscii(char const* s1, char const* s2) {
    // Note: using the locale-based functions is very slow under windows since it has to look up the locale from the environment.
    for (;;) {
        char c1 = asciiTolower(*s1++);
        char c2 = asciiTolower(*s2++);
        if (c1 == 0 || (c1 != c2))
            return c1 - c2;
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int BeStringUtilities::Stricmp(const char* s1, const char* s2)
    {
    if (!isAscii(s1) || !isAscii(s2))
        return WString(s1,BentleyCharEncoding::Utf8).CompareToI(WString(s2,BentleyCharEncoding::Utf8));

    return StricmpAscii(s1, s2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int BeStringUtilities::Wcsicmp(WCharCP lhs, WCharCP rhs)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    return _wcsicmp(lhs, rhs);
#elif defined (__unix__)
    return wcsicmp_portable(lhs, rhs, AsManyAsPossible);
#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int BeStringUtilities::Strnicmp(const char* s1, const char* s2, size_t n)
    {
    if (!isAscii(s1) || !isAscii(s2))
        return Wcsnicmp(WString(s1,BentleyCharEncoding::Utf8).c_str(), WString(s2,BentleyCharEncoding::Utf8).c_str(), n);

#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    return _strnicmp(s1, s2, n);
#elif defined (__unix__)
    return strncasecmp(s1, s2, n);
#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int BeStringUtilities::Wcsnicmp(const wchar_t* s1, const wchar_t* s2, size_t n)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    return _wcsnicmp(s1, s2, n);
#elif defined (__unix__)
    return wcsicmp_portable(s1, s2, n);
#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
char* BeStringUtilities::Strdup(char const* s)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    return _strdup(s);
#elif defined (__unix__)
    return strdup(s);
#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
wchar_t* BeStringUtilities::Wcsdup(wchar_t const* s)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    return _wcsdup(s);
#elif defined (__unix__)
    return wcsdup_portable(s);
#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
char* BeStringUtilities::Strtok(char *strToken, const char *strDelimit, char **context)
    {
    BeAssert(isAscii(strDelimit));

#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    return strtok_s(strToken, strDelimit, context);
#elif defined (__unix__)
    return strtok_r(strToken, strDelimit, context);
#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
wchar_t* BeStringUtilities::Wcstok(wchar_t *wcsToken, const wchar_t *wcsDelimit, wchar_t **context)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    return wcstok_s(wcsToken, wcsDelimit, context);
#elif defined (__unix__)
    return wcstok(wcsToken, wcsDelimit, context);
#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
char* BeStringUtilities::Strrev(char *s)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    return _strrev(s);
#elif defined (__unix__)
     for (size_t i=0, j=strlen(s)-1;  i < j; ++i, --j)
        {
        char temp = s[i];
        s[i] = s[j];
        s[j] = temp;
        }
    return s;
#else
#error unknown runtime
#endif
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
wchar_t* BeStringUtilities::Wcsrev(wchar_t *s)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    return _wcsrev(s);
#elif defined (__unix__)
     for (size_t i=0, j=wcslen(s)-1;  i < j; ++i, --j)
        {
        wchar_t temp = s[i];
        s[i] = s[j];
        s[j] = temp;
        }
    return s;
#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
wchar_t* BeStringUtilities::Wcsncpy(wchar_t *strDest, size_t destLen, const wchar_t *strSource, size_t count)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    wcsncpy_s(strDest, destLen, strSource, count);
#elif defined (__unix__)
    unixWcsncpy(strDest, destLen, strSource, count);
#else
#error unknown runtime
#endif
    return strDest;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
char* BeStringUtilities::Strncpy(char *strDest, size_t destLen, const char *strSource, size_t count)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    strncpy_s(strDest, destLen, strSource, count);
#elif defined (__unix__)
    unixStrncpy(strDest, destLen, strSource, count);
#else
#error unknown runtime
#endif
    return strDest;
    }

#if defined (__unix__)
    /** Adapted from:
     * C++ version 0.4 char* style "itoa":
     * Written by Lukas Chmela
     * Released under GPLv3.
     */
    static wchar_t* bentleyItow(int value, wchar_t* result, size_t sizeInCharacters, int base) {
        if (0==sizeInCharacters)
            return NULL;

        // check that the base if valid
        if (base < 2 || base > 36) { *result = '\0'; return result; }

        wchar_t* ptr = result, *ptr1 = result, tmp_char;
        int tmp_value;

        do {
            if (0 == --sizeInCharacters)
                return NULL;
            tmp_value = value;
            value /= base;
            *ptr++ = L"zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
        } while ( value );

        // Apply negative sign
        if (tmp_value < 0) *ptr++ = '-';
        *ptr-- = '\0';
        while (ptr1 < ptr) {
            tmp_char = *ptr;
            *ptr--= *ptr1;
            *ptr1++ = tmp_char;
        }
        return result;
    }

#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void*   BeStringUtilities::WCharToPointer (WCharCP inWChar)
    {
    if ( (nullptr == inWChar) || (0 == *inWChar) )
        return NULL;

    uintptr_t v;
    WString::Swscanf_safe(inWChar, L"%" SCNxPTR, &v);
    return (void*)v;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void*   BeStringUtilities::Utf8ToPointer (Utf8CP inChar)
    {
    if ( (nullptr == inChar) || (0 == *inChar) )
        return NULL;

    uintptr_t v;
    Utf8String::Sscanf_safe(inChar, "%" SCNxPTR, &v);
    return (void*)v;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BeStringUtilities::Itow(wchar_t *buffer, int value, size_t sizeInCharacters, int radix)
    {
#if defined (_WIN32)
    return _itow_s(value, buffer, sizeInCharacters, radix) == 0? SUCCESS: ERROR;
#else
    return bentleyItow(value, buffer, sizeInCharacters, radix) != NULL? SUCCESS: ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int BeStringUtilities::Wtoi(wchar_t const* s)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    return _wtoi(s);
#elif defined (__unix__)
    int i = 0;
    BE_STRING_UTILITIES_SWSCANF (s, L"%d", &i);
    return i;
#else
#error unknown runtime
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void BeStringUtilities::FormatUInt64(WCharP string, uint64_t number, uint64_t base)
    {
    WCharCP digits = nullptr;
    switch (base)
        {
        case 2ULL:
            digits = L"01";
            break;

        case 10ULL:
            digits = L"0123456789";
            break;

        case 16ULL:
            digits = L"0123456789abcdef";
            break;
        default:
            BeAssert(false && "BeStringUtilities::FormatUInt64: Unsupported base");
            return;
        }

    WCharP bufpt = string;
    // Convert to ascii
    do
        {
        *bufpt = digits[number%base];
        bufpt++;
        number /= base;
        }
    while (number > 0ULL);

    *bufpt = '\0';
    Wcsrev(string);
    }

void BeStringUtilities::FormatUInt64(Utf8P string, uint64_t number, uint64_t base)
    {
    Utf8CP digits = nullptr;
    switch (base)
        {
        case 2ULL:
            digits = "01";
            break;

        case 10ULL:
            digits = "0123456789";
            break;

        case 16ULL:
            digits = "0123456789abcdef";
            break;
        default:
            BeAssert(false && "BeStringUtilities::FormatUInt64: Unsupported base");
            return;
        }

    Utf8P bufpt = string;
    // Convert to ascii
    do
        {
        *bufpt = digits[number%base];
        bufpt++;
        number /= base;
        }
        while (number > 0ULL);

        *bufpt = '\0';
        Strrev(string);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
uint64_t BeStringUtilities::ParseUInt64(Utf8CP string, BentleyStatus* outStat)
    {
    BentleyStatus ALLOW_NULL_OUTPUT(status,outStat);

    status = ERROR;
    if (Utf8String::IsNullOrEmpty(string))
        return 0;

    if (HasHexPrefix(string))
        return ParseHex(string, &status);

    uint64_t value = 0;
    for ( ; *string; ++string)
        {
        char c = *string;
        if (!isdigit(c))
            return 0;

        uint64_t digit = c - '0';
        value *= 10;
        value += digit;
        }

    status = SUCCESS;
    return value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t BeStringUtilities::ParseHex(Utf8CP hs, BentleyStatus* outStat)
    {
    BentleyStatus ALLOW_NULL_OUTPUT(status,outStat);
    status = ERROR;

    if (HasHexPrefix(hs))
        hs += 2;

    if (!*hs)
        return 0;

    uint64_t value = 0;
    for ( ; *hs; ++hs)
        {
        uint64_t hexit;
        char c = *hs;
        if (isdigit(c))
            hexit = c - '0';
        else
            {
            if (c >= 'A' && c <= 'F')
                hexit = 10 + (c - 'A');
            else if (c >= 'a' && c <= 'f')
                hexit = 10 + (c - 'a');
            else
                return 0;
            }
        value <<= 4;
        value += hexit;
        }

    status = SUCCESS;
    return value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
unsigned long  BeStringUtilities::Wcstoul(wchar_t const* s, wchar_t** end, int base)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    return wcstoul(s, end, base);
#elif defined (__unix__)
    Utf8String a(s);
    char const* abegin = a.c_str();
    char* aend = NULL;
    unsigned long value = strtoul(abegin, &aend, base);
    if (end)
        *end = const_cast<wchar_t*>(s) + (aend - abegin);  // *** TRICKY: the # of utf-8 chars in a run of digits and a-z chars will be the same in all encodings
    return value;
#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
long  BeStringUtilities::Wcstol(wchar_t const* s, wchar_t** end, int base)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    return wcstol(s, end, base);
#elif defined (__unix__)
    Utf8String a(s);
    char const* abegin = a.c_str();
    char* aend = NULL;
    long value = strtol(abegin, &aend, base);
    if (end)
        *end = const_cast<wchar_t*>(s) + (aend - abegin);  // *** TRICKY: the # of utf-8 chars in a run of digits and a-z chars will be the same in all encodings
    return value;
#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double BeStringUtilities::Wcstod(wchar_t const* s, wchar_t** end)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    return wcstod(s, end);
#elif defined (__unix__)
    Utf8String a(s);
    char const* abegin = a.c_str();
    char* aend = NULL;
    double value = strtod(abegin, &aend);
    if (end)
        *end = const_cast<wchar_t*>(s) + (aend - abegin);  // *** TRICKY: the # of utf-8 chars in a run of digits and a-z chars will be the same in all encodings
    return value;
#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double BeStringUtilities::Wtof(wchar_t const* s)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    return _wtof(s);
#elif defined (__unix__)
    double v = 0;
    BE_STRING_UTILITIES_SWSCANF (s, L"%lf", &v);
    return v;
#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BeStringUtilities::Memmove(void *dest, size_t numberOfElements, const void *src, size_t count)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    return memmove_s(dest, numberOfElements, src, count) == 0? SUCCESS: ERROR;
#elif defined (__unix__)
    if (NULL == dest || NULL == src || numberOfElements < count)
        return ERROR;
    memmove(dest, src, count);
    return SUCCESS;
#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BeStringUtilities::Wmemmove(wchar_t *dest, size_t numberOfElements, const wchar_t *src, size_t count)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    return wmemmove_s(dest, numberOfElements, src, count) == 0? SUCCESS: ERROR;
#elif defined (__unix__)
    if (NULL == dest || NULL == src || numberOfElements < count)
        return ERROR;
    wmemmove(dest, src, count);
    return SUCCESS;
#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BeStringUtilities::Memcpy(void *dest, size_t numberOfElements, const void *src, size_t count)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    return memcpy_s(dest, numberOfElements, src, count) == 0? SUCCESS: ERROR;
#elif defined (__unix__)
    if (NULL == dest || NULL == src || numberOfElements < count)
        return ERROR;
    memcpy(dest, src, count);
    return SUCCESS;
#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BeStringUtilities::Wmemcpy(wchar_t *dest, size_t numberOfElements, const wchar_t *src, size_t count)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    return wmemcpy_s(dest, numberOfElements, src, count) == 0? SUCCESS: ERROR;
#elif defined (__unix__)
    if (NULL == dest || NULL == src || numberOfElements < count)
        return ERROR;
    wmemcpy(dest, src, count);
    return SUCCESS;
#else
#error unknown runtime
#endif
    }

template <typename value_type> void MyStrCpy(value_type *pDest, size_t count, value_type* src){}
template<> void MyStrCpy<char>(char *pDest, size_t count, char *src)
    {    BeStringUtilities::Strncpy(pDest, count, src);    }

template<> void MyStrCpy<wchar_t>(wchar_t* pDest, size_t count, wchar_t* src)
    {    BeStringUtilities::Wcsncpy (pDest, count, src);    }

/*---------------------------------------------------------------------------------**//**
* Note: This method was motivated by a significant performance degradation in
* BeStringUtilities::Snwprintf(). DgnECManager::FormatInstanceId() and PersistentElementPath::
* ToWString() were making frequent use of Snwprintf() to format hexadecimal values.
* Snwprintf() must convert the incoming format string and any wchar_t arguments from wchar_t
* to Utf8, and convert the result from Utf8 to wchar_t. Switching to a dedicated formatting
* function reduced time spent formatting integers by two orders of magnitude.
*
* If similar bottlenecks appear in other code making  heavy use of Snwprintf(), it is suggested
* that dedicated formatting methods similar to this one be implemented.
* Note that when formatting integers and floats, the produced output cannot exceed the ASCII
* range; therefore even a dedicated method which simply builds a utf8 format string and
* hands it off to Snwprintf() would eliminate half of the wchar_t<->Utf8 conversion overhead.
*
* Note this method is tested against Snwprintf() so any anomalous behavior is duplicated
* (in particular, an input value of 0 produces different output than might be expected with
* some combinations of options).
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename value_type> int _FormatUInt64(value_type *dest, size_t numCharsInBuffer, uint64_t inputVal, HexFormatOptions opts, uint8_t minWidth, uint8_t minPrecisionIn)
    {
    if (NULL == dest || 0 >= numCharsInBuffer)
        { BeAssert(false); return 0; }

    static const value_type s_offsets[2][2] = { { '0', 'a'-0xA }, { '0', 'A'-0xA } };
    static const size_t bufSize = 0x12      // max minWidth (== max minPrecision + max prefix length)
                                + 1;        // null terminator

    // 1. Options
    bool wantPrefix     = (0 != (static_cast<int>(HexFormatOptions::IncludePrefix) & static_cast<int>(opts))),
         uppercase      = (0 != (static_cast<int>(HexFormatOptions::Uppercase) & static_cast<int>(opts))),
         leadingZeros   = (0 != (static_cast<int>(HexFormatOptions::LeadingZeros) & static_cast<int>(opts))),
         leftJustify    = (0 != (static_cast<int>(HexFormatOptions::LeftJustify) & static_cast<int>(opts))),
         usePrecision   = (0 != (static_cast<int>(HexFormatOptions::UsePrecision) & static_cast<int>(opts)));

    wantPrefix &= (0 != inputVal);                          // Snwprintf() does not produce a prefix for a value of 0
    leadingZeros &= (usePrecision || !leftJustify);         // Don't pad out the width with leading zeros if we're left-justifying

    // 2. Adjust arguments to bounds
    if (0x10 < minPrecisionIn)
        minPrecisionIn = 0x10;
    if (0x12 < minWidth)
        minWidth = 0x12;

    // 3. Determine precision
    uint8_t minPrecision = usePrecision ? minPrecisionIn : 1;
    if (!usePrecision && leadingZeros)
        {
        if (wantPrefix)
            minPrecision = minWidth >= 2 ? minWidth - 2 : 0;
        else
            minPrecision = minWidth > 0 ? minWidth : 1;
        }

    // 4. Produce hexits
    value_type buf[bufSize];
    value_type const* offsets = uppercase ? s_offsets[1] : s_offsets[0];
    value_type* pStart = buf + bufSize - 1,
           * pCur   = pStart;

    *pCur = '\0';
    uint64_t val = inputVal;
    while (val > 0)
        {
        value_type i = (value_type)(val & 0xF);
        i += offsets[(int)(i > 9)];
        *--pCur = i;
        val >>= 4;
        }

    size_t nDigits = pStart - pCur;                         // this can be 0 if val==0; this is correct if minPrecision is also 0, otherwise we will add zeros below

    // 5. Meet minimum precision
    while (nDigits < minPrecision)
        {
        *--pCur = '0';
        nDigits++;
        }

    size_t width = wantPrefix ? nDigits + 2 : nDigits;

    // 6. Prepend prefix
    if (wantPrefix)
        {
        *--pCur = uppercase ? 'X' : 'x';
        *--pCur = '0';
        }

    value_type * pDest = dest,
            * pEnd  = dest + numCharsInBuffer - 1;

    // 7. Right-justify in output buffer
    if (!leftJustify)
        {
        while (width < minWidth && pDest < pEnd)
            {
            *pDest++ = ' ';
            width++;
            }
        }

    // 8. Copy to output buffer
    size_t nCharsWritten = pDest - dest;
    size_t nCharsRemaining = numCharsInBuffer - nCharsWritten;
    MyStrCpy(pDest, nCharsRemaining, pCur);

    // 9. Left-justify
    if (leftJustify && width < (size_t)minWidth)
        {
        size_t nPadding = (size_t)minWidth - width;
        pDest += width;
        while (0 < nPadding && pDest < pEnd)
            {
            *pDest++ = ' ';
            width++;
            nPadding--;
            }
        }

    // 10. Null-terminate
    BeAssert(width < numCharsInBuffer);
    dest[width] = '\0';

    return (int)width;
    }

int BeStringUtilities::FormatUInt64 (wchar_t *dest, size_t numCharsInBuffer, uint64_t inputVal, HexFormatOptions opts, uint8_t minWidth, uint8_t minPrecisionIn)
    {
    return _FormatUInt64(dest, numCharsInBuffer, inputVal, opts, minWidth, minPrecisionIn);
    }

int BeStringUtilities::FormatUInt64 (Utf8Char *dest, size_t numCharsInBuffer, uint64_t inputVal, HexFormatOptions opts, uint8_t minWidth, uint8_t minPrecisionIn)
    {
    return _FormatUInt64(dest, numCharsInBuffer, inputVal, opts, minWidth, minPrecisionIn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void countDigitChars(uint32_t& digitCount, uint32_t& leadingZeroCount, WCharCP inString)
    {
    WCharCP     currChar = inString;
    bool        foundNonZeroDigit = false;

    digitCount  = leadingZeroCount = 0;

    for (; iswdigit(*currChar); ++currChar)
        {
        digitCount++;

        if (foundNonZeroDigit)
            continue;

        if ('0' == *currChar)
            leadingZeroCount++;
        else
            foundNonZeroDigit = true;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int BeStringUtilities::LexicographicCompare(WCharCP value0, WCharCP value1)
    {
    int     zeroComparisonHint = 0;
    while (*value0 && *value1)
        {
        if (iswdigit(*value0) && iswdigit(*value1))
            {
            // Previously, this used atoi to compare any digits found in two strings.  If either number was too large for atoi,
            // this would overflow and sort incorrectly/cause collisions.  The sort here now preserves the behavior of
            // atoi (comparison by number value, ignoring leading zeroes) without the risk of overflow.
            uint32_t    digitCount0, leadingZeroCount0;
            uint32_t    digitCount1, leadingZeroCount1;

            countDigitChars(digitCount0, leadingZeroCount0, value0);
            countDigitChars(digitCount1, leadingZeroCount1, value1);

            if (0 == zeroComparisonHint)
                {
                // If both sub-strings are just zeros, the one with fewer zeros is considered smaller (everything else being equal)
                if (leadingZeroCount0 == digitCount0 && leadingZeroCount1 == digitCount1)
                    zeroComparisonHint = digitCount0 - digitCount1;
                }

            // If only one string is just zeros, it is smaller
            if (leadingZeroCount0 == digitCount0 && leadingZeroCount1 != digitCount1)
                return -1;
            if (leadingZeroCount0 != digitCount0 && leadingZeroCount1 == digitCount1)
                return 1;

            // Excluding leading zeros, if one string has fewer digits it is smaller.
            if (digitCount0 - leadingZeroCount0 < digitCount1 - leadingZeroCount1)
                return -1;
            if (digitCount0 - leadingZeroCount0 > digitCount1 - leadingZeroCount1)
                return 1;

            // Otherwise, skip the leading zeros and do a digit-wise comparison.
            value0 += leadingZeroCount0;
            value1 += leadingZeroCount1;

            while (iswdigit(*value0) && iswdigit(*value1))
                {
                int compare = towlower(*value0++) - towlower(*value1++);
                if (compare)
                    return compare;
                }
            }
        else
            {
            int compare = towlower(*value0++) - towlower(*value1++);
            if (compare)
                return compare;
            }
        }

    if (*value0 == *value1)
        return zeroComparisonHint;

    return *value0 - *value1;
    }

#if defined (BENTLEY_WIN32)
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
WString  BeStringUtilities::ParseFileURI (WCharCP uri, WCharCP basePath)
    {
    if (NULL == uri || L'\0' == *uri)
        return WString();

    int                 length       = static_cast<int>(wcslen(uri));
    int                 maxLength    = length * 2;
    WCharP              outComponent = static_cast<WCharP> (_alloca(maxLength * sizeof(WChar)));
    outComponent[0] = 0;

    URL_COMPONENTSW     components;
    memset(&components, 0, sizeof(components));

    components.dwStructSize     = sizeof(components);
    components.lpszUrlPath      = outComponent;
    components.dwUrlPathLength  = maxLength;

    if (TRUE != InternetCrackUrlW (uri, length, ICU_DECODE | ICU_ESCAPE, &components))
        return WString();

    if (INTERNET_SCHEME_FILE != components.nScheme)
        return WString();

    return WString(outComponent);
    }

#endif

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void BeStringUtilities::Initialize(BeFileNameCR assetPathW)
    {
    Utf8String assetPath(assetPathW.c_str());
    u_setDataDirectory(assetPath.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
size_t BeStringUtilities::ComputeNumLogicalChars(WCharCP str, size_t numUnits)
    {
    // We are doing this ourselves because of the mandate to avoid using third-party libraries such as ICU at lower layers of the API.
    WCharCP end = (str + numUnits);

    PUSH_STATIC_ANALYSIS_WARNING(6326) // NEEDS WORK STATIC ANALYSIS - this should be converted into an #ifdef - the sizeof(WChar) does not vary at run time
    switch (sizeof(WChar))
        {
        case 2:
            {
            // UTF-16 can be 1 or 2 units per-character.
            size_t numLogChars = 0;
            while (str < end)
                {
                // Surrogate? Walk an additional unit.
                if (*str >= 0xD800 && *str < 0xE000)
                    {
                    ++str;

                    if (str >= end)
                        {
                        BeAssert(false); // Invalid UTF-16 lead byte.
                        return numLogChars;
                        }
                    }

                ++numLogChars;
                ++str;
                }

            return numLogChars;
            }
        case 4:
            {
            // UTF-32 is 1 unit per-character.
            return numUnits;
            }
        default:
            {
            BeAssert(false); // Unanticipated WChar size.
            return numUnits;
            }
        }
    POP_STATIC_ANALYSIS_WARNING
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
size_t BeStringUtilities::ComputeByteOffsetOfLogicalChar(Utf8CP str, size_t numLogicalChars)
    {
    // We are doing this ourselves because of the mandate to avoid using third-party libraries such as ICU at lower layers of the API.
    size_t bytesInLogChar = 0;
    size_t byteOffset = 0;

    for (size_t iLogChar = 0; iLogChar < numLogicalChars; ++iLogChar)
        {
        bytesInLogChar = 1;

        if ((*str & 0xE0) == 0xC0)
            bytesInLogChar = 2;
        else if ((*str & 0xF0) == 0xE0)
            bytesInLogChar = 3;
        else if ((*str & 0xF8) == 0xF0)
            bytesInLogChar = 4;
        else if (0 != (*str & 0x80))
            BeAssert(false); // Invalid UTF-8 lead byte.

        str += bytesInLogChar;
        byteOffset += bytesInLogChar;
        }

    return byteOffset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// We sometimes use memcpy to copy unaligned data to an aligned buffer so we can access the
// data without an alignment exception.  When clang sees that both the source and destination
// are doubles it assumes they are aligned doubles and generates an inlined-memcpy that requires alignment.
// To work around this problem I've added this function in a source file that doesn't use it
// so the optimizer cannot figure out the type of data being copied.
//---------------------------------------------------------------------------------------
void BentleyApi::UnalignedMemcpy(void*dest, void const*source, size_t num) { memcpy(dest, source, num); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool BeStringUtilities::IsInvalidUtf8Sequence(Utf8CP str)
    {
    UErrorCode icuError = U_ZERO_ERROR;
    u_strFromUTF8(NULL, 0, NULL, str, (int32_t)strlen(str), &icuError);
    return (U_INVALID_CHAR_FOUND == icuError);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeInt64Id::IsWellFormedString(Utf8StringCR id)
    {
    auto len = id.length();
    if (0 == len || 18 < len || '0' != id[0])
        return false;

    // Normalized invalid Id: "0"
    if (1 == len)
        return true;

    // Valid Ids begin with "0x" followed by at least one lower-case hexadecimal digit.
    if (2 == len || 'x' != id[1])
        return false;

    auto isValidHexString = [](Utf8StringCR str, size_t startIndex, size_t len)
        {
        auto isHexDigit = [](Utf8StringCR str, size_t index, bool allowZero)
            {
            auto ch = str[index];
            if (ch >= 'a' && ch <= 'f')
                return true;

            auto minDigit = allowZero ? '0' : '1';
            return ch >= minDigit && ch <= '9';
            };

        if (0 == len)
            return false;

        // No leading zeroes...
        if (!isHexDigit(str, startIndex, false))
            return false;

        // ...followed by len-1 lowercase hexadecimal digits.
        for (size_t i = 1; i < len; i++)
            if (!isHexDigit(str, startIndex + i, true))
                return false;

        return true;
        };

    // If briefcase Id is present, it occupies at least one digit, followed by 10 digits for local Id
    size_t localIdStart = 2;
    if (len > 12)
        {
        localIdStart = len - 10;

        // Verify briefcase Id
        if (!isValidHexString(id, 2, localIdStart - 2))
            return false;

        // Skip leading zeroes in local Id
        for (size_t i = localIdStart; i < len; i++)
            {
            if ('0' != id[i])
                break;
            else
                ++localIdStart;
            }

        if (localIdStart >= len)
            return false;
        }

    return isValidHexString(id, localIdStart, len - localIdStart);
    }
