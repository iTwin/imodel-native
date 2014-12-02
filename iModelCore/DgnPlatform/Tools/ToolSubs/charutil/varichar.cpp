/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/charutil/varichar.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/ScopedArray.h>
#include <Bentley/BeAssert.h>
#include <Bentley/WString.h>
#include <DgnPlatform/Tools/varichar.h>

//  VariChar can be any of the following. '-' denotes character data.
//
//  -----------------------------------------------------------------------------------------------------------------
//  | char size | encoding  | first word        | second word       | flag(s)                                       |
//  -----------------------------------------------------------------------------------------------------------------
//  | 1 byte    | locale    | -                 | -                 | VARI_CHAR_FLAG_None                           |
//  | 2 bytes   | locale    | WIDECHAR_INDUCER  | -                 | VARI_CHAR_FLAG_Wide                           |
//  | 1 byte    | Unicode   | UNICODE_INDUCER   | UNICODE_NARROW    | VARI_CHAR_FLAG_Unicode                        |
//  | 2 bytes   | Unicode   | UNICODE_INDUCER   | -                 | VARI_CHAR_FLAG_Unicode | VARI_CHAR_FLAG_Wide  |
//  -----------------------------------------------------------------------------------------------------------------

USING_NAMESPACE_BENTLEY

//=======================================================================================
//! Inducers and flags in the underlying VariChar string that identify it.
// @bsiclass                                                    Jeff.Marker     12/2011
//=======================================================================================
enum
    {
    WIDECHAR_INDUCER        = 0xfdffu,
    UNICODE_INDUCER         = 0xfeffu,
    UNICODE_NARROW          = 0x0001u,

    WIDECHAR_INDUCER_SWAP   = 0xfffdu,
    UNICODE_INDUCER_SWAP    = 0xfffeu,
    UNICODE_NARROW_SWAP     = 0x0100u

    };

//=======================================================================================
//! Flags for the result of VariCharConverter::Classify.
// @bsiclass                                                    Jeff.Marker     12/2011
//=======================================================================================
enum VariCharAnalysisFlags
    {
    VARI_CHAR_FLAG_None     = 0,
    VARI_CHAR_FLAG_Unicode  = 1 << 0,
    VARI_CHAR_FLAG_Wide     = 1 << 1,
    VARI_CHAR_FLAG_Byteswap = 1 << 2

    }; // VariCharAnalysisFlags

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2011
//---------------------------------------------------------------------------------------
static  Byte    extractLowByte  (UInt16 word)                   { return (Byte)(word & 0xff); }
static  Byte    extractHighByte (UInt16 word)                   { return (Byte)((word >> 8) & 0xff); }
static  bool    isUnicode       (VariCharAnalysisFlags flags)   { return (VARI_CHAR_FLAG_Unicode == (flags & VARI_CHAR_FLAG_Unicode)); }
static  bool    isWide          (VariCharAnalysisFlags flags)   { return (VARI_CHAR_FLAG_Wide == (flags & VARI_CHAR_FLAG_Wide)); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2011
//---------------------------------------------------------------------------------------
static VariCharAnalysisFlags classifyString(VariCharCP variBuffer)
    {
    if ((NULL == variBuffer) || (0 == *variBuffer))
        return VARI_CHAR_FLAG_None;

    UInt16 code1 = *((UInt16*)variBuffer);

    switch(code1)
        {
        case UNICODE_INDUCER:
            {
            UInt16 code2 = *((UInt16*)(variBuffer + 2));
            
            if (code2 == UNICODE_NARROW)
                return VARI_CHAR_FLAG_Unicode;

            return (VariCharAnalysisFlags)(VARI_CHAR_FLAG_Unicode | VARI_CHAR_FLAG_Wide);
            }
        
        case UNICODE_INDUCER_SWAP:
            {
            VariCharAnalysisFlags   result  =(VariCharAnalysisFlags)(VARI_CHAR_FLAG_Unicode | VARI_CHAR_FLAG_Byteswap);
            UInt16                  code2   = *((UInt16*)(variBuffer + 2));
            
            if (code2 != UNICODE_NARROW_SWAP)
                result =(VariCharAnalysisFlags)(result | VARI_CHAR_FLAG_Wide);
            
            return result;
            }

        case WIDECHAR_INDUCER:
            {
            return VARI_CHAR_FLAG_Wide;
            }

        case WIDECHAR_INDUCER_SWAP:
            {
            return (VariCharAnalysisFlags)(VARI_CHAR_FLAG_Wide | VARI_CHAR_FLAG_Byteswap);
            }
        }

    return VARI_CHAR_FLAG_None;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2011
//---------------------------------------------------------------------------------------
static size_t computeNumInducerBytes(VariCharAnalysisFlags flags)
    {
    if (isUnicode(flags))
        {
        if (isWide(flags))
            return 2;

        return 4;
        }

    if (isWide(flags))
        return 2;
    
    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2011
//---------------------------------------------------------------------------------------
static bool canBeStoredAsNarrow(UInt16 const* pWide)
    {
    for (; *pWide; ++pWide)
        {
        if (0 != extractHighByte(*pWide))
            return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2011
//---------------------------------------------------------------------------------------
BentleyStatus VariCharConverter::UnicodeToVariChar(bvector<VariChar>& variBuffer, WCharCP uniString, LangCodePage codePage, bool shouldNullTerminate)
    {
    variBuffer.clear();
    if (shouldNullTerminate)
        variBuffer.push_back(0);

    // Nothing to convert?
    if (WString::IsNullOrEmpty(uniString))
        return SUCCESS;
    
    // Do they want a Unicode VariChar?
    if ((LangCodePage::None == codePage) || (LangCodePage::Unicode == codePage))
        {
        // Note that in terms of data written to the file (the only legitimate use of VariChar), "Unicode" means UTF-16LE.
        Utf16Buffer utf16String;
        BeStringUtilities::WCharToUtf16(utf16String, uniString);
        
        // If all high bytes are zero, we can write a "narrow" UTF-16LE string by omitting them and saving space.
        bool isNarrow = canBeStoredAsNarrow((UInt16 const*)&utf16String[0]);

        variBuffer.clear();

        // All flags are words.
        variBuffer.push_back(extractLowByte(UNICODE_INDUCER));
        variBuffer.push_back(extractHighByte(UNICODE_INDUCER));
        
        size_t numUtf16CharsToProcess = utf16String.size();
        if (!shouldNullTerminate)
            --numUtf16CharsToProcess;
        
        if (isNarrow)
            {
            variBuffer.push_back(extractLowByte(UNICODE_NARROW));
            variBuffer.push_back(extractHighByte(UNICODE_NARROW));

            // Write only the low bytes out.
            for (size_t iChar = 0; iChar < numUtf16CharsToProcess; ++iChar)
                variBuffer.push_back(extractLowByte(utf16String[iChar]));
            
            return SUCCESS;
            }
    
        // We're wide, so write all bytes out (and don't flag).
        for (size_t iChar = 0; iChar < numUtf16CharsToProcess; ++iChar)
            {
            variBuffer.push_back(extractLowByte(utf16String[iChar]));
            variBuffer.push_back(extractHighByte(utf16String[iChar]));
            }
            
        return SUCCESS;
        }
    
    if (!BeStringUtilities::IsValidCodePage(codePage))
        return ERROR;
    
    // Locale VariChar it is then...
    // "Wide" dedicates two bytes to every character. In a locale scenario, each Unicode character goes to locale char* separately, and the conversion has the high byte padded if necessary.
    bvector<UInt16> localeString;
    WCharCP         uniEnd          =(uniString + wcslen(uniString) + 1);
    
    // Don't attempt to convert the NULL terminator, and always add it at the end.
    for (WCharCP uniIter = uniString; uniIter <(uniEnd - 1); ++uniIter)
        {
        WChar uniBuffer[2];
        uniBuffer[0] = *uniIter;
        uniBuffer[1] = 0;
        
        AString tempLocaleString;
            BeStringUtilities::WCharToLocaleChar(tempLocaleString, codePage, uniBuffer, _countof(uniBuffer));

        if (1 == tempLocaleString.size())
            {
            localeString.push_back((unsigned char)tempLocaleString[0]);
            continue;
            }
        
        localeString.push_back(((unsigned char)tempLocaleString[0] << 8) | (unsigned char)tempLocaleString[1]);
        }
    
    localeString.push_back(0);

    // If all high bytes are zero, we can write a "narrow" string by omitting them and saving space.
    bool isNarrow = canBeStoredAsNarrow(&localeString[0]);

    size_t numLocaleCharsToProcess = localeString.size();
    if (!shouldNullTerminate)
        --numLocaleCharsToProcess;
        
    variBuffer.clear();

    if (isNarrow)
        {
        // Locale VariChar is narrow by default; don't flag.
        // Write only the low bytes out.
        for (size_t iChar = 0; iChar < numLocaleCharsToProcess; ++iChar)
            variBuffer.push_back(extractLowByte(localeString[iChar]));
            
        return SUCCESS;
        }
    
    // We're wide, so write the inducer, and all bytes out.
    // All flags are words.
    variBuffer.push_back(extractLowByte(WIDECHAR_INDUCER));
    variBuffer.push_back(extractHighByte(WIDECHAR_INDUCER));

    for (size_t iChar = 0; iChar < numLocaleCharsToProcess; ++iChar)
        {
        variBuffer.push_back(extractLowByte(localeString[iChar]));
        variBuffer.push_back(extractHighByte(localeString[iChar]));
        }
            
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
BentleyStatus VariCharConverter::UnicodeToVariChar(bvector<VariChar>& variBuffer, Utf8CP uniString, LangCodePage codePage, bool shouldNullTerminate)
    {
    // Hopefully efficient enough vs. re-factoring the WString overload to support multiple Unicode encodings in its loop.
    WString uniStringW(uniString, BentleyCharEncoding::Utf8);

    return UnicodeToVariChar(variBuffer, uniStringW.c_str(), codePage, shouldNullTerminate);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2013
//---------------------------------------------------------------------------------------
BentleyStatus VariCharConverter::FontCharToVariChar(bvector<VariChar>& variBuffer, UInt16 const* fontChars, size_t numFontChars, bool isUnicode, bool shouldNullTerminate)
    {
    variBuffer.clear();
    if (shouldNullTerminate)
        variBuffer.push_back(0);

    // Nothing to convert?
    if ((NULL == fontChars) || (0 == *fontChars) || (0 == numFontChars))
        return SUCCESS;
    
    // If all high bytes are zero, we can write a "narrow" string by omitting them and saving space.
    bool isNarrow = canBeStoredAsNarrow(&fontChars[0]);

    // Prefer to reserve a little too much instead of being meticulous about counting a couple of inducer bytes.
    if (isNarrow)
        variBuffer.reserve(numFontChars + 5);
    else
        variBuffer.reserve((numFontChars * 2) + 6);
    
    // All flags are words.
    if (isUnicode)
        {
        variBuffer.push_back(extractLowByte(UNICODE_INDUCER));
        variBuffer.push_back(extractHighByte(UNICODE_INDUCER));
        }
        
    if (isNarrow)
        {
        if (isUnicode)
            {
            variBuffer.push_back(extractLowByte(UNICODE_NARROW));
            variBuffer.push_back(extractHighByte(UNICODE_NARROW));
            }

        // Write only the low bytes out.
        for (size_t iChar = 0; iChar < numFontChars; ++iChar)
            variBuffer.push_back(extractLowByte(fontChars[iChar]));
        
        if (!shouldNullTerminate && (0 == variBuffer.back()))
            variBuffer.resize(variBuffer.size() - 1);
        else if (shouldNullTerminate && (0 != variBuffer.back()))
            variBuffer.push_back(0);
        }
    else
        {
        for (size_t iChar = 0; iChar < numFontChars; ++iChar)
            {
            variBuffer.push_back(extractLowByte(fontChars[iChar]));
            variBuffer.push_back(extractHighByte(fontChars[iChar]));
            }
        
        if (!shouldNullTerminate && (0 == variBuffer[variBuffer.size() - 1]) && (0 == variBuffer[variBuffer.size() - 2]))
            {
            variBuffer.resize(variBuffer.size() - 2);
            }
        else if (shouldNullTerminate && ((0 != variBuffer[variBuffer.size() - 1]) || (0 == variBuffer[variBuffer.size() - 2])))
            {
            variBuffer.push_back(0);
            variBuffer.push_back(0);
            }
        }

    return SUCCESS;    
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2011
//---------------------------------------------------------------------------------------
BentleyStatus VariCharConverter::VariCharToUnicode(WStringR uniString, VariCharCP variBuffer, size_t numVariBytes, LangCodePage codePage)
    {
    uniString.clear();
    
    // Detects elements that have stored empty strings: nothing to convert. Purposefully not allowing 0 to mean "figure it out for me" because of this.
    if (0 == numVariBytes)
        return SUCCESS;
    
    // Bad input?
    if (NULL == variBuffer)
        return ERROR;
    
    // Nothing to convert?
    if (0 == *variBuffer)
        return SUCCESS;
    
    VariCharAnalysisFlags   classification  = classifyString(variBuffer);
    size_t                  numInducerBytes = computeNumInducerBytes(classification);

    if (VARI_CHAR_FLAG_Byteswap ==(classification & VARI_CHAR_FLAG_Byteswap))
        {
        BeAssert(false && L"I was hoping we no longer had to support this.");
        return ERROR;
        }

    numVariBytes -= numInducerBytes;
    
    // Valid but empty string: nothing to convert.
    // It is not trivial to also check for empty(only NULL terminator left), since we don't yet know if we're looking for one zero, or two.
    if (0 == numVariBytes)
        return SUCCESS;
    
    // Advance to the actual string data.
    variBuffer += numInducerBytes;
    
    // One past the end, like standard end iterators.
    VariCharCP variEnd = variBuffer + numVariBytes;
    
    if (isUnicode(classification) || (LangCodePage::Unicode == codePage))
        {
        // Note that in terms of data written to the file(the only legitimate use of VariChar), "Unicode" means UTF-16LE.
        // BeStringUtilities::Utf16ToWChar requires a NULL-terminated string; VariChar data is not required to be this way.
        // Attempt to optimize(e.g. don't make intermediate buffer) in the NULL-terminated wide UTF-16LE case.
        if ((isWide(classification)) && (0 == *(variEnd - 1)) && (0 == *(variEnd - 2)))
            return BeStringUtilities::Utf16ToWChar(uniString,(Utf16Char const*)variBuffer, numVariBytes / sizeof(Utf16Char));

        Utf16Buffer utf16String;

        // We know we're UTF-16LE, but the high zeros may have been omitted.
        if (!isWide(classification))
            for (VariCharCP variIter = variBuffer; variIter < variEnd; variIter += sizeof(Byte))
                utf16String.push_back(*(Byte const*)variIter);
        else
            for (VariCharCP variIter = variBuffer; variIter < variEnd; variIter += sizeof(Utf16Char))
                utf16String.push_back(*(Utf16Char const*)variIter);

        // Ensure NULL-termination; we got here either because lack of NULL, or simply because we're narrow(and had to make a temporary buffer anyway).
        if (0 != utf16String.back())
            utf16String.push_back(0);

        return BeStringUtilities::Utf16ToWChar(uniString, &utf16String[0], utf16String.size());
        }
    
    if (!BeStringUtilities::IsValidCodePage(codePage))
        return ERROR;
    
    // Otherwise we're locale. IsWide here implies 2 bytes per character(padded as necessary).
    // "Wide" dedicates two bytes to every character. In a locale scenario, each Unicode character goes to locale char* separately, and the conversion has the high byte padded if necessary.
    if (isWide(classification))
        {
        for (VariCharCP variIter = variBuffer; variIter < variEnd; variIter += sizeof(UInt16))
            {
            char localeBuffer[3];

            if (0 == *(variIter + 1))
                {
                localeBuffer[0] = *variIter;
                localeBuffer[1] = 0;
                }
            else
                {
                localeBuffer[0] = *(variIter + 1);
                localeBuffer[1] = *variIter;
                localeBuffer[2] = 0;
                }
            
            WString tempUniString;
            BeStringUtilities::LocaleCharToWChar(tempUniString, localeBuffer, codePage, _countof(localeBuffer));

            uniString += tempUniString;
            }
        
        return SUCCESS;
        }

    // We're left with locale-encoded char*.
    // BeStringUtilities::LocaleCharToWChar requires a NULL-terminated string; VariChar data is not required to be this way.
    // Attempt to optimize in the NULL-terminated case.
    if (0 == *(variEnd - 1))
            return BeStringUtilities::LocaleCharToWChar(uniString,(CharCP)variBuffer, codePage, numVariBytes);
    
    ScopedArray<char> terminatedVariBytes(numVariBytes + 1);
    memcpy(terminatedVariBytes.GetData(),(CharCP)variBuffer, numVariBytes);
    terminatedVariBytes.GetData()[numVariBytes] = 0;
    
        return BeStringUtilities::LocaleCharToWChar(uniString, terminatedVariBytes.GetData(), codePage, numVariBytes + 1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2014
//---------------------------------------------------------------------------------------
static VariCharConverter::Result ensureNullTerminated(WCharP currChar, WCharP endChar, bool truncated)
    {
    if (0 == *(currChar - 1))
        return (truncated ? VariCharConverter::Result::Truncated : VariCharConverter::Result::Success);

    if (currChar < (endChar - 1))
        {
        *currChar = 0;
        return (truncated ? VariCharConverter::Result::Truncated : VariCharConverter::Result::Success);
        }

    *(currChar - 1) = 0;
    return VariCharConverter::Result::Truncated;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2014
//---------------------------------------------------------------------------------------
VariCharConverter::Result VariCharConverter::VariCharToUnicode(WCharP unicodeBuffer, size_t numUnicodeChars, VariCharCP variBuffer, size_t numVariBytes, LangCodePage codePage)
    {
    PRECONDITION(NULL != variBuffer, Result::Error);
    PRECONDITION(NULL != unicodeBuffer, Result::Error);
    PRECONDITION(numUnicodeChars > 0, Result::Error);
    
    unicodeBuffer[0] = 0;

    // Purposefully not allowing 0 numVariBytes to mean "figure it out for me" because it is valid for elements to store empty strings.
    if ((0 == numVariBytes) || (0 == *variBuffer))
        return Result::Success;

    VariCharAnalysisFlags classification = classifyString(variBuffer);
    size_t numInducerBytes = computeNumInducerBytes(classification);

    // I was hoping we no longer had to support this.
    if (UNEXPECTED_CONDITION(VARI_CHAR_FLAG_Byteswap == (classification & VARI_CHAR_FLAG_Byteswap)))
        return Result::Error;

    numVariBytes -= numInducerBytes;

    // Valid but empty string: nothing to convert.
    // It is not trivial to also check for empty (only NULL terminator left), since we don't yet know if we're looking for one zero, or two.
    if (0 == numVariBytes)
        return Result::Success;

    // Advance to the actual string data.
    variBuffer += numInducerBytes;

    // "Unicode" VariChar means UTF-16; since we only support Windows, we can just copy the data.
    if (isUnicode(classification) || (LangCodePage::Unicode == codePage))
        {
        // If wide, use memcpy for performance.
        if (isWide(classification))
            {
            size_t numCharsToCopy = std::min(numUnicodeChars, numVariBytes / sizeof(WChar));
            size_t numBytesToCopy = (numCharsToCopy * sizeof(WChar));
            memcpy(unicodeBuffer, variBuffer, numBytesToCopy);
            
            return ensureNullTerminated(&unicodeBuffer[numCharsToCopy], &unicodeBuffer[numUnicodeChars], (numUnicodeChars < numCharsToCopy));
            }

        // Otherwise need to allow the interspersed zeros.
        VariCharCP variIter = variBuffer;
        VariCharCP variEnd = (variBuffer + numVariBytes);
        WCharP unicodeChar = unicodeBuffer;
        WCharP unicodeEnd = (unicodeBuffer + numUnicodeChars);
        
        for (; variIter < variEnd && unicodeChar < unicodeEnd; variIter += sizeof(Byte), ++unicodeChar)
            *unicodeChar = *(Byte const*)variIter;
        
        return ensureNullTerminated(unicodeChar, unicodeEnd, (variIter < variEnd));
        }

    if (!BeStringUtilities::IsValidCodePage(codePage))
        return Result::Error;

    // Otherwise we're locale. IsWide here implies 2 bytes per character (padded as necessary).
    // "Wide" dedicates two bytes to every character. In a locale scenario, each Unicode character goes to locale char* separately, and the conversion has the high byte padded if necessary.
    if (isWide(classification))
        {
        VariCharCP variIter = variBuffer;
        VariCharCP variEnd = (variBuffer + numVariBytes);
        WCharP unicodeChar = unicodeBuffer;
        WCharP unicodeEnd = (unicodeBuffer + numUnicodeChars);
        
        for (; variIter < variEnd && unicodeChar < unicodeEnd; variIter += sizeof(UInt16))
            {
            char localeBuffer[3];
            if (0 == *(variIter + 1))
                {
                localeBuffer[0] = *variIter;
                localeBuffer[1] = 0;
                }
            else
                {
                localeBuffer[0] = *(variIter + 1);
                localeBuffer[1] = *variIter;
                localeBuffer[2] = 0;
                }

            BeStringUtilities::LocaleCharToWChar(unicodeChar, localeBuffer, codePage, (unicodeEnd - unicodeChar));
            size_t numConvertedChars = wcslen(unicodeChar);

            unicodeChar += numConvertedChars;
            }

        return ensureNullTerminated(unicodeChar, unicodeEnd, (variIter < variEnd));
        }

    // We're left with locale-encoded char*.
    WCharP unicodeEnd = (unicodeBuffer + numUnicodeChars);
    BeStringUtilities::LocaleCharToWChar(unicodeBuffer, variBuffer, codePage, numUnicodeChars);
    size_t numConvertedChars = wcslen(unicodeBuffer);
    
    return ensureNullTerminated(unicodeBuffer + numConvertedChars, unicodeEnd, (numConvertedChars < (int) numVariBytes));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2013
//---------------------------------------------------------------------------------------
BentleyStatus VariCharConverter::VariCharToUnicode(Utf8StringR uniString, VariCharCP variBuffer, size_t numVariBytes, LangCodePage codePage)
    {
    uniString.clear();
    
    // Hopefully efficient enough vs. re-factoring the WString overload to support multiple Unicode encodings in its loop.
    WString uniStringW;
    BentleyStatus status = VariCharToUnicode(uniStringW, variBuffer, numVariBytes, codePage);
    if (SUCCESS != status)
        return status;
    
    uniString.Assign(uniStringW.c_str());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2013
//---------------------------------------------------------------------------------------
BentleyStatus VariCharConverter::VariCharToFontChar(bvector<UInt16>& fontChars, VariCharCP variBuffer, size_t numVariBytes)
    {
    fontChars.clear();
    fontChars.push_back(0);
    
    // Detects elements that have stored empty strings: nothing to convert. Purposefully not allowing 0 to mean "figure it out for me" because of this.
    if (0 == numVariBytes)
        return SUCCESS;
    
    // Bad input?
    if (NULL == variBuffer)
        return ERROR;
    
    // Nothing to convert?
    if (0 == *variBuffer)
        return SUCCESS;
    
    VariCharAnalysisFlags   classification  = classifyString(variBuffer);
    size_t                  numInducerBytes = computeNumInducerBytes(classification);

    if (VARI_CHAR_FLAG_Byteswap == (classification & VARI_CHAR_FLAG_Byteswap))
        {
        BeAssert(false && L"I was hoping we no longer had to support this.");
        return ERROR;
        }

    numVariBytes -= numInducerBytes;
    
    // Valid but empty string: nothing to convert.
    // It is not trivial to also check for empty (only NULL terminator left), since we don't yet know if we're looking for one zero, or two.
    if (0 == numVariBytes)
        return SUCCESS;
    
    // Advance to the actual string data.
    variBuffer += numInducerBytes;
    
    // We don't have to do any transcoding; DgnFont will do that when converting between FontChar and Unicode.
    // IsWide here implies 2 bytes per character (padded as necessary).
    // "Wide" dedicates two bytes to every character. In a locale scenario, each Unicode character goes to locale char* separately, and the conversion has the high byte padded if necessary.
    if (isWide(classification))
        {
        fontChars.resize(numVariBytes / sizeof (FontChar));
        
        memcpy(&fontChars.front(), variBuffer, numVariBytes);
        }
    else
        {
        fontChars.clear();
        fontChars.reserve (numVariBytes);

        // One past the end, like standard end iterators.
        VariCharCP variEnd = variBuffer + numVariBytes;
    
        for (VariCharCP variIter = variBuffer; variIter < variEnd; ++variIter)
            fontChars.push_back((FontChar)(unsigned char)*variIter);
        }
    
    if (0 != fontChars.back())
        fontChars.push_back(0);
        
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2011
//---------------------------------------------------------------------------------------
size_t VariCharConverter::ComputeNumBytes(VariCharCP variBuffer)
    {
    if (NULL == variBuffer)
        return 0;
    
    VariCharAnalysisFlags   classification  = classifyString(variBuffer);
    size_t                  numInducerBytes = computeNumInducerBytes(classification);

    // Go past inducers, and either look for first 0 byte or word(depending on wide).
    
    if (isWide(classification))
        {
        VariCharCP variIter = variBuffer + numInducerBytes;
        for (; 0 != *(Utf16CP)variIter; variIter += sizeof(UInt16))
            ;
        
        return ((variIter - variBuffer) + sizeof(UInt16));
        }
    
    VariCharCP variIter = variBuffer + numInducerBytes;
    for (; 0 != *(UInt8 const*)variIter; variIter += sizeof(UInt8))
        ;
        
    return ((variIter - variBuffer) + sizeof(UInt8));
    }
