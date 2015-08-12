/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/BeStringUtilities.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "Bentley.h"
#include "bvector.h"
#include "NonCopyableClass.h"
#include "CodePages.h"
#include <stdarg.h>
#include <stdio.h>

BEGIN_BENTLEY_NAMESPACE

typedef bvector<Utf16Char>  Utf16Buffer;
typedef Utf16Buffer const*  Utf16BufferCP;
typedef Utf16Buffer const&  Utf16BufferCR;
typedef Utf16Buffer*        Utf16BufferP;
typedef Utf16Buffer&        Utf16BufferR;

// Options for formatting unsigned integer values in hexadecimal
enum class HexFormatOptions
    {
    None              = 0,
    LeftJustify       = 1 << 0,       // Append spaces to meet minimum width. Analogous to '-' printf option.
    IncludePrefix     = 1 << 1,       // Prefix with "0x" or "0X" depending on the HexFormatOptions::Uppercase flag. Analogous to '#' printf option.
    Uppercase         = 1 << 2,       // Use uppercase hexadecimal digits; also affects case of prefix. Analogous to "%X" printf specifier.
    LeadingZeros      = 1 << 3,       // Pad with leading zeros to meet minimum precision and/or width. Analogous to '0' printf option.
    UsePrecision      = 1 << 4,       // Without this option, default precision of 1 is used and leading zeros are applied to fulfill minimum width based on HexFormatOptions::LeadingZeros option.
                                      // With this option, leading zeros are not used to satisfy minimum width regardless of HexFormatOptions::LeadingZeros, but are used to satisfy minimum precision, in keeping with Snwprintf()
    };

/**
* @addtogroup BeStringGroup
* Cross-platform utilities for working with strings.
*/

//=======================================================================================
//!  Static utility methods for working with strings.
//!  The C++ standard library and STL provide most of the functions you need to work with 
//!  strings in a portable way. The BeStringUtilities provides wrapper functions for 
//!  standard library functions that are missing on some platforms and for the more common 
//!  string operations that are not part of the portable standard library. Note that 
//!  BeStringUtilities is not a complete set of string functions. It wraps only those 
//!  functions for which there is no portable standard library function on every platform. 
//!  If a standard library function has no wrapper in BeStringUtiliites, then you should 
//!  assume that you can use it on all platforms.
//!
//!  There are a number of conversion methods for switching between UTF-16, UTF-8, WChars, 
//!    and locale-encoded chars.
//!  The second group of functions are cross-platform compatible versions of C-runtime 
//!    specific functions.  They are all buffer-overrun safe.
//!  There are also methods for parsing arguments that may be quoted, and for extracting
//!    common username and computername information.
//!
//! Here is a general guide to the Bentley portability API:
//!     * memcpy                    - BeStringUtilities::Memcpy
//!     * Printf/scanf              - BeStringUtilities, WString/Utf8String
//!     * Integer-string conversion - BeStringUtilities
//!     * string-manipulation       - BeStringUtilities.
//!     * character conversion      - BeStringUtilities.
//!     * Time functions            - Bentley::DateTime, BeTimeUtilities
//!     * file IO                   - BeFile, BeFileName, BeFileListIterator, BeDirectoryIterator
//!     * character file IO           Not available.
//!     * tmpfile/tmpname           - DgnPlatformLib::QueryHost()->GetIKnownLocationsAdmin
//!     * get/setenv                - Not portable and must not be used at all. Must provide DgnPlatformLib::Host-based admins for specific needs.
//!     * malloc                    - Should rarely be used. Instead, use bvector, ScopedArray, and C++ new
//! @ingroup BeStringGroup
//=======================================================================================
struct BeStringUtilities : public NonCopyableClass
    {
private:
    // Static class; don't allow new instances.
    BeStringUtilities () { }

//__PUBLISH_SECTION_END__
    static void FormatUInt64 (WCharP string, uint64_t number, uint64_t base);
    static void FormatUInt64 (Utf8P string, uint64_t number, uint64_t base);

#if defined (BENTLEY_WIN32)
public:
    BENTLEYDLL_EXPORT static WString ParseFileURI (WCharCP uri, WCharCP basePath);
#endif

//__PUBLISH_SECTION_START__

public:
    static const size_t NPOS = (size_t)-1;                  //!< A maximum value used to indicate that the buffer is NULL-terminated.
    static const size_t AsManyAsPossible = ((size_t)-1);    //!< A large value indicating to use as many characters as possible. 

    //! This method must be called once per process before any conversion methods are called. This is effectively used to initialize ICU, the conversion engine on some platforms.
    BENTLEYDLL_EXPORT static void Initialize(BeFileNameCR assetPathW);

    //! Converts a UTF-16 buffer to a WChar string.
    //! @note Up to \a count characters will be converted; less may be converted if a NULL is encountered earlier. If count is BeStringUtilities::NPOS, the input is assumed to be NULL-terminated.
    //! @note The result is always cleared first, even if the input is NULL or empty.
    //! @note The result will always be NULL-terminated, even if the input (limited by count) is not.
    //! @remarks This is only intended for persistence operations, where specific UTF encoding is vital.
    //! @remarks For portable code, remember that wchar_t (also WString) does <b>not</b> always mean UTF-16.
    //! @remarks While no error should occur during this operation for valid Unicode strings, corrupt data can still be encountered.
    BENTLEYDLL_EXPORT static BentleyStatus Utf16ToWChar (WStringR, Utf16CP, size_t count = AsManyAsPossible);

    //! Converts a UTF-16 buffer to a WChar string.
    //! @note Up to \a count characters will be converted; less may be converted if a NULL is encountered earlier. If count is BeStringUtilities::AsManyAsPossible, the input is assumed to be NULL-terminated.
    //! @note The result will always be NULL-terminated, even if the input (limited by count) is not.
    //! @param[out] wbuf    The convert string is copied into \a wbuf.
    //! @param[in]  wbufSizeInChars The maximum number of characters (not bytes) that should be copied into \a ubuf, \em including the trailing \\0.
    //!             If \a wbufSizeInChars is less than or equal to count or the length of \a inStr, then the input string will be truncated to wbufSizeInChars-1 characters. That is, this function will do this:
    //!             wbuf[wbufSizeInChars-1] = 0;
    //! @param[in]  inStr   The input string to be converted.
    //! @param[in]  count   The number of characters to copy from input string. Note that \a count is the number of characters to convert, not the number of bytes.
    BENTLEYDLL_EXPORT static BentleyStatus Utf16ToWChar (WCharP wbuf, size_t wbufSizeInChars, Utf16CP inStr, size_t count = AsManyAsPossible);

    //! Converts a UTF-16 buffer to a WChar array.
    //! @note Up to \a count characters will be converted; less may be converted if a NULL is encountered earlier. If count is BeStringUtilities::AsManyAsPossible, the input is assumed to be NULL-terminated.
    //! @note The result will always be NULL-terminated, even if the input (limited by count) is not.
    //! @param[out] wbuf    The convert string is copied into \a wbuf.
    //! @remarks If \a wbufSizeInChars is less than or equal to count or the length of \a inStr, then the input string will be truncated to wbufSizeInChars-1 characters. That is, this function will do this:
    //!             wbuf[wbufSizeInChars-1] = 0;
    //! @param[in]  inStr   The input string to be converted.
    //! @param[in]  count   The number of characters to copy from input string. Note that \a count is the number of characters to convert, not the number of bytes.
    template <size_t wbufSizeInChars>
    static BentleyStatus Utf16ToWChar (WChar (&wbuf)[wbufSizeInChars], Utf16CP inStr, size_t count = AsManyAsPossible) {return Utf16ToWChar (wbuf, wbufSizeInChars, inStr, count);}

    //! Converts a WChar buffer to a UTF-16 string.
    //! @note Up to \a count characters will be converted; less may be converted if a NULL is encountered earlier. If \a count is BeStringUtilities::AsManyAsPossible, the input is assumed to be NULL-terminated.
    //! @note The result is always cleared first, even if the input is NULL or empty.
    //! @note The result will always be NULL-terminated, even if the input (limited by count) is not.
    //! @remarks This is only intended for persistence operations, where specific UTF encoding is vital.
    //! @remarks For portable code, remember that wchar_t (also WString) does <b>not</b> always mean UTF-16.
    //! @remarks While no error should occur during this operation for valid Unicode strings, corrupt data can still be encountered.
    //! @param[in]  count   the number of characters to copy from input string. Note that count is the number of characters to convert, not the number of bytes.
    BENTLEYDLL_EXPORT static BentleyStatus WCharToUtf16 (Utf16BufferR, WCharCP, size_t count = AsManyAsPossible);

    //! Converts a WChar buffer to a UTF-16 array.
    //! @note Up to \a count characters will be converted; less may be converted if a NULL is encountered earlier. If \a count is BeStringUtilities::AsManyAsPossible, the input is assumed to be NULL-terminated.
    //! @note The result will always be NULL-terminated, even if the input (limited by count) is not.
    //! @param[out] ubuf    The converted string is copied into \a ubuf.
    //! @param[in]  ubufSizeInChars  The maximum number of characters (not bytes) that should be copied into \a ubuf, \em including the trailing \\0.
    //!             If \a ubufSizeInChars is less than or equal to count or the length of \a inStr, then the input string will be truncated to ubufSizeInChars-1 characters. That is, this function will do this:
    //!             ubuf[ubufSizeInChars-1] = 0;
    //! @param[in]  inStr   The input string to be converted.
    //! @param[in]  count   The number of characters to copy from input string. Note that \a count is the number of characters to convert, not the number of bytes.
    BENTLEYDLL_EXPORT static BentleyStatus WCharToUtf16 (Utf16P ubuf, size_t ubufSizeInChars, WCharCP inStr, size_t count = AsManyAsPossible);

    //! Converts a WChar buffer to a UTF-16 string.
    //! @note Up to \a count characters will be converted; less may be converted if a NULL is encountered earlier. If \a count is BeStringUtilities::AsManyAsPossible, the input is assumed to be NULL-terminated.
    //! @note The result will always be NULL-terminated, even if the input (limited by count) is not.
    //! @param[out] ubuf    The converted string is copied into \a ubuf. If \a ubufSizeInChars is less than or equal to count or the length of \a inStr, then the input string will be truncated to ubufSizeInChars-1 characters. That is, this function will do this: ubuf[ubufSizeInChars-1] = 0;
    //! @param[in]  inStr   The input string to be converted.
    //! @param[in]  count   The number of characters to copy from input string. Note that \a count is the number of characters to convert, not the number of bytes.
    template <size_t ubufSizeInChars>
    static BentleyStatus WCharToUtf16 (Utf16Char (&ubuf)[ubufSizeInChars], WCharCP inStr, size_t count) {return WCharToUtf16 (ubuf, ubufSizeInChars, inStr, count);}

    //! Compare two 0-terminated Unicode strings that are encoded as UTF-16
    BENTLEYDLL_EXPORT static int CompareUtf16 (Utf16CP s1, Utf16CP s2);

    //! Compare a 0-terminated Unicode string encoded as UTF-16 with a wchar_t Unicode string
    BENTLEYDLL_EXPORT static int CompareUtf16WChar (Utf16CP s1, WCharCP s2);

    //! Get length of a 0-terminated Unicode string encoded as UTF-16
    BENTLEYDLL_EXPORT static size_t Utf16Len (Utf16CP);

    //! Copy a 0-terminated Unicode string encoded as UTF-16 into a Utf16 buffer
    BENTLEYDLL_EXPORT static void CopyUtf16 (Utf16P outStr, size_t outStrCapacity, Utf16CP inStr);

    //! Copy a 0-terminated Unicode string encoded as UTF-16 into a Utf16 buffer
    template <size_t outStrCapacity>
    static void CopyUtf16 (Utf16Char (&outStr)[outStrCapacity], Utf16CP inStr) {return CopyUtf16 (outStr, outStrCapacity, inStr);}

    //! Converts a WChar string to a UTF-8 string.
    //! @note Up to count characters will be converted; less may be converted if a NULL is encountered earlier. If count is BeStringUtilities::AsManyAsPossible, the input is assumed to be NULL-terminated.
    //! @note The result is always cleared first, even if the input is NULL or empty.
    //! @note The result will always be NULL-terminated, even if the input (limited by count) is not.
    //! @remarks While no error should occur during this operation for valid Unicode strings, corrupt data can still be encountered.
    BENTLEYDLL_EXPORT static BentleyStatus WCharToUtf8 (Utf8StringR, WCharCP, size_t count = AsManyAsPossible);

    //! Converts a UTF-8 string to a WChar string.
    //! @note Up to count characters will be converted; less may be converted if a NULL is encountered earlier. If count is BeStringUtilities::AsManyAsPossible, the input is assumed to be NULL-terminated.
    //! @note The result is always cleared first, even if the input is NULL or empty.
    //! @note The result will always be NULL-terminated, even if the input (limited by count) is not.
    //! @remarks While no error should occur during this operation for valid Unicode strings, corrupt data can still be encountered.
    BENTLEYDLL_EXPORT static BentleyStatus Utf8ToWChar (WStringR, Utf8CP, size_t count = AsManyAsPossible);

    //! Converts a UTF-8 string to a WChar buffer.
    //! @param[out]  outWChar       Output buffer; this will always be NULL-terminated, even if the buffer is too small
    //! @param[in]   inChar         Input string; this must be NULL-terminated
    //! @param[in]   outMaxChars    The number of characters (number of WChar) in outMSWChar; this is expected to be greater than 0, and includes the NULL terminator.
    //! @remarks While outMaxChars is declared as a size_t, the current implementation will cast it to a signed Int32.
    //! @return Returns the value of the outWChar parameter so this method can be used in expressions.
    //! @remarks While no error should occur during this operation for valid Unicode strings, corrupt data can still be encountered.
    BENTLEYDLL_EXPORT static WCharP Utf8ToWChar (WCharP outWChar, Utf8CP inChar, size_t outMaxChars);

    //! Converts a UTF-8 string to a UTF-16 string.
    //! @note Up to count characters will be converted; less may be converted if a NULL is encountered earlier. If count is BeStringUtilities::AsManyAsPossible, the input is assumed to be NULL-terminated.
    //! @note The result is always cleared first, even if the input is NULL or empty.
    //! @note The result will always be NULL-terminated, even if the input (limited by count) is not.
    //! @remarks While no error should occur during this operation for valid Unicode strings, corrupt data can still be encountered.
    BENTLEYDLL_EXPORT static BentleyStatus Utf8ToUtf16 (Utf16BufferR, Utf8CP, size_t count = AsManyAsPossible);

    //! Converts a UTF-16 string to a UTF-8 string.
    //! @note Up to count characters will be converted; less may be converted if a NULL is encountered earlier. If count is BeStringUtilities::AsManyAsPossible, the input is assumed to be NULL-terminated.
    //! @note The result is always cleared first, even if the input is NULL or empty.
    //! @note The result will always be NULL-terminated, even if the input (limited by count) is not.
    //! @remarks While no error should occur during this operation for valid Unicode strings, corrupt data can still be encountered.
    BENTLEYDLL_EXPORT static BentleyStatus Utf16ToUtf8 (Utf8StringR, Utf16CP, size_t count = AsManyAsPossible);

    //! Converts a WChar string to a locale-encoded string.
    //! @note Up to count characters will be converted; less may be converted if a NULL is encountered earlier. If count is BeStringUtilities::AsManyAsPossible, the input is assumed to be NULL-terminated.
    //! @note The result is always cleared first, even if the input is NULL or empty. Further, if an error is encountered, the result will not contain partial results.
    //! @note The result will always be NULL-terminated, even if the input (limited by count) is not.
    BENTLEYDLL_EXPORT static BentleyStatus WCharToLocaleChar (AStringR, LangCodePage codePage, WCharCP, size_t count = AsManyAsPossible);

    //! Converts a WChar string to a multibyte string using the current system locale.
    //! @param[out] outChar     Output buffer; this will always be NULL-terminated, even if the buffer is too small
    //! @param[in]  inWChar     Input string; this must be NULL-terminated
    //! @param[in]  outMaxBytes The number of bytes in outChar. This is expected to be greater than 0, and includes the NULL terminator.
    //! @remarks While outMaxBytes is declared as a size_t, the current implementation will cast it to a signed Int32.
    //! @return Returns the value of outChar so that this method can be used in expressions.
    BENTLEYDLL_EXPORT static CharP WCharToCurrentLocaleChar (CharP outChar, WCharCP inWChar, size_t outMaxBytes);

    //! Converts a WChar string to a multibyte string using the current system locale.
    //! @param[out] localeStr   Output buffer
    //! @param[in]  inWChar     Input string; this must be NULL-terminated
    BENTLEYDLL_EXPORT static BentleyStatus WCharToCurrentLocaleChar (AStringR localeStr, WCharCP inWChar);

    //! Converts a locale-encoded string to a WChar string.
    //! @note Up to count characters will be converted; less may be converted if a NULL is encountered earlier. If count is BeStringUtilities::AsManyAsPossible, the input is assumed to be NULL-terminated.
    //! @note The result is always cleared first, even if the input is NULL or empty. Further, if an error is encountered, the result will not contain partial results.
    //! @note The result will always be NULL-terminated, even if the input (limited by count) is not.
    BENTLEYDLL_EXPORT static BentleyStatus LocaleCharToWChar (WStringR, CharCP, LangCodePage codePage, size_t count = AsManyAsPossible);

    //! Converts a locale-encoded string to a WChar buffer.
    //! @param[out]  outWChar       Output buffer; this will always be NULL-terminated, even if the buffer is too small
    //! @param[in]   inChar         Input string; this must be NULL-terminated
    //! @param[in]   codePage       Code page of the input locale-encoded string
    //! @param[in]   outMaxChars    The number of characters (number of WChar) in outMSWChar; this is expected to be greater than 0, and includes the NULL terminator.
    //! @return Returns the value of outWChar parameter so that this method can be used in expressions.
    BENTLEYDLL_EXPORT static WCharP LocaleCharToWChar (WCharP outWChar, CharCP inChar, LangCodePage codePage, size_t outMaxChars);

    //! Converts a multibyte string to a WChar string using the current system locale.
    //! @param[out]  outWChar       Output buffer; this will always be NULL-terminated, even if the buffer is too small
    //! @param[in]   inChar         Input string; this must be NULL-terminated
    //! @param[in]   outMaxChars    The number of characters (number of WChar) in outWChar; this is expected to be greater than 0, and includes the NULL terminator.
    //! @remarks While outMaxChars is declared as a size_t, it must not exceed UINT32_MAX
    //! @return Returns the value of the outWChar output buffer so this method can be used in expressions.
    BENTLEYDLL_EXPORT static WCharP CurrentLocaleCharToWChar (WCharP outWChar, CharCP inChar, size_t outMaxChars);

    //! Converts a multibyte string to a WChar string using the current system locale.
    //! @param[out]  outStr     Output string
    //! @param[in]   inChar     Input string; this must be NULL-terminated
    BENTLEYDLL_EXPORT static BentleyStatus CurrentLocaleCharToWChar (WStringR outStr, CharCP inChar);

    //! Gets the current code page
    //! @param[out] codePage    Set to the current code page
    //! @return non-zero error status if the current code page cannot be determined.
    //! @private
    BENTLEYDLL_EXPORT static BentleyStatus GetCurrentCodePage (LangCodePage& codePage);

    //! Tests if the specified code page is valid
    //! @param[in] codePage    Set to the current code page
    //! @return true if \a codePage is a valid code page identifier
    //! @private
    BENTLEYDLL_EXPORT static bool IsValidCodePage (LangCodePage codePage);

//__PUBLISH_SECTION_END__
    //! This is meant to allow advanced usage of ICU to transcode odd encodings that are not worth writing high-level wrappers for (e.g. reasing strings from TrueType font metadata).
    //! @note outStringBuff will effectively be NULL-terminated only if inString (up to inStringNumBytes) is also NULL-terminated; otherwise it will not.
    BENTLEYDLL_EXPORT static BentleyStatus TranscodeStringDirect(bvector<Byte>& outStringBuff, CharCP outEncoding, Byte const* inString, size_t inStringNumBytes, CharCP inEncoding);
    
    //! Computes the number of logical characters in the span. Remember that on some platforms, WChar can be a multi-byte sequence.
    BENTLEYDLL_EXPORT static size_t ComputeNumLogicalChars(WCharCP, size_t numUnits);

    //! Computes the number of logical characters in the span. Remember that on some platforms, WChar can be a multi-byte sequence.
    BENTLEYDLL_EXPORT static size_t ComputeByteOffsetOfLogicalChar(Utf8CP, size_t numLogicalChars);
//__PUBLISH_SECTION_START__

    //! Get the decimal separator for the current locale
    //! @param[out] sep The decimal separator string
    BENTLEYDLL_EXPORT static BentleyStatus GetDecimalSeparator (WStringR sep);

    //! Encode a character string using URL encoding.
    //! @param[in] charsToEncode    Input string.
    //! @return encoded string.
    BENTLEYDLL_EXPORT static Utf8String UriEncode (Utf8CP charsToEncode);

    //! Decode a string that was encoded by UriEncode.
    //! @param[in] encodedString    The encoded input string.
    //! @return decoded string.
    BENTLEYDLL_EXPORT static Utf8String UriDecode (Utf8CP encodedString);

    //! Decode a string that was encoded by UriEncode.
    //! @param[in] start    The start of the string to be decoded.
    //! @param[in] end      One past the end of the string to be decoded.
    //! @return decoded string.
    BENTLEYDLL_EXPORT static Utf8String UriDecode (Utf8CP start, Utf8CP end);

    //! Check that all characters in \a str are safe for a URI
    BENTLEYDLL_EXPORT static bool IsUriEncoded (Utf8CP str);

    //! Format a string following the rules of sprintf.
    //! @param buffer           Where to write the formatted string
    //! @param numCharsInBuffer The maximum number of characters that may be written to \a buffer, including the trailing 0-terminator.
    //! @param format           The formatting to apply
    //! @return the number of characters, excluding the final 0-terminator, that were written to \a buffer if less than numCharsInBuffers; or, -1 if the output was truncated.
    //! @remarks If the length of the formatted string exceeds \a numCharsInBuffer, the string is truncated (i.e., buffer[numCharsInBuffer-1] = 0;)
    BENTLEYDLL_EXPORT static int Snprintf (CharP buffer, size_t numCharsInBuffer, CharCP format, ...);

    //! Format a string following the rules of sprintf.
    //! @param destArray        Where to write the formatted string
    //! @param format           The formatting to apply
    //! @return the number of characters, excluding the final 0-terminator, that were written to \a buffer if less than destArraySize; or, -1 if the output was truncated.
    //! @remarks If the length of the formatted string exceeds \a numCharsInBuffer, the string is truncated (i.e., destArray[destArraySize-1] = 0;)
    template <size_t destArraySize>
    static int Snprintf (char (&destArray)[destArraySize], CharCP format, ...)
        {
        va_list args;
        va_start (args, format);
        return Vsnprintf (destArray, destArraySize, format, args);
        }

    //! Format a string following the rules of sprintf.
    //! @param buffer           Where to write the formatted string
    //! @param numCharsInBuffer The maximum number of characters that may be written to \a buffer, including the trailing 0-terminator.
    //! @param format           The formatting to apply
    //! @param args             The values to substitute into the formatted string
    //! @return the number of characters, excluding the final 0-terminator, that were written to \a buffer if less than numCharsInBuffers; or, -1 if the output was truncated.
    //! @remarks If the length of the formatted string exceeds \a numCharsInBuffer, the string is truncated (i.e., buffer[numCharsInBuffer-1] = 0;)
    BENTLEYDLL_EXPORT static int Vsnprintf (CharP buffer, size_t numCharsInBuffer, CharCP format, va_list args);

    //! Format a string following the rules of sprintf.
    //! @param buffer           Where to write the formatted string
    //! @param numCharsInBuffer The maximum number of characters that may be written to \a buffer, including the trailing 0-terminator.
    //! @param format           The formatting to apply
    //! @return the number of characters, excluding the final 0-terminator, that were written to \a buffer if less than numCharsInBuffers; or, -1 if the output was truncated.
    //! @remarks If the length of the formatted string exceeds \a numCharsInBuffer, the string is truncated (i.e., buffer[numCharsInBuffer-1] = 0;)
    BENTLEYDLL_EXPORT static int Snwprintf (WCharP buffer, size_t numCharsInBuffer, WCharCP format, ...);

    //! Format a string following the rules of sprintf.
    //! @param destArray        Where to write the formatted string
    //! @param format           The formatting to apply
    //! @return the number of characters, excluding the final 0-terminator, that were written to \a buffer if less than destArraySize; or, -1 if the output was truncated.
    //! @remarks If the length of the formatted string exceeds \a numCharsInBuffer, the string is truncated (i.e., buffer[numCharsInBuffer-1] = 0;)
    template <size_t destArraySize>
    static int Snwprintf (wchar_t (&destArray)[destArraySize], WCharCP format, ...)
        {
        va_list args;
        va_start (args, format);
        return Vsnwprintf (destArray, destArraySize, format, args);
        }

    //! Format a string following the rules of sprintf.
    //! @param buffer           Where to write the formatted string
    //! @param numCharsInBuffer The maximum number of characters that may be written to \a buffer, including the trailing 0-terminator.
    //! @param format           The formatting to apply
    //! @param args             The values to substitute into the formatted string
    //! @return the number of characters, excluding the final 0-terminator, that were written to \a buffer if less than numCharsInBuffers; or, -1 if the output was truncated.
    //! @remarks If the length of the formatted string exceeds \a numCharsInBuffer, the string is truncated (i.e., buffer[numCharsInBuffer-1] = 0;)
    BENTLEYDLL_EXPORT static int Vsnwprintf (WCharP buffer, size_t numCharsInBuffer, WCharCP format, va_list args);

#if !defined (BENTLEY_CPP_MISSING_WCHAR_SUPPORT)
    #define BE_STRING_UTILITIES_SWSCANF(SRC,FMT,...)     swscanf(SRC,FMT,__VA_ARGS__)
    #define BE_STRING_UTILITIES_UTF8_SSCANF(SRC,FMT,...) sscanf(SRC,FMT,__VA_ARGS__)
#else
    //! Extract values from a formatted string
    //! @param stringSource the formatted string to read
    //! @param fmt the format of the string
    BENTLEYDLL_EXPORT static int Sscanf (CharCP stringSource, CharCP fmt, ...);
    #define BE_STRING_UTILITIES_UTF8_SSCANF(SRC,FMT,...) BeStringUtilities::Sscanf(SRC,FMT,__VA_ARGS__)

    //! Extract values from a formatted string
    //! @param stringSource the formatted string to read
    //! @param fmt the format of the string
    BENTLEYDLL_EXPORT static int Swscanf (WCharCP stringSource, WCharCP fmt, ...);
    #define BE_STRING_UTILITIES_SWSCANF(SRC,FMT,...)     BeStringUtilities::Swscanf(SRC,FMT,__VA_ARGS__)
#endif

    //! Convert all characters to lowercase.
    BENTLEYDLL_EXPORT static char* Strlwr (char* s);

    //! Convert all characters to lowercase.
    BENTLEYDLL_EXPORT static wchar_t* Wcslwr (wchar_t* s);

    //! Convert all characters to uppercase.
    BENTLEYDLL_EXPORT static char* Strupr (char* s);

    //! Convert all characters to uppercase.
    BENTLEYDLL_EXPORT static wchar_t* Wcsupr (wchar_t* s);

    //! Compare two strings in a case-insensitive way. Equivalent to MSVC _wcsicmp and a character-by-character comparision in GCC. @return 0 if the strings are equal (ignoring case), otherwise a negative or positive number representing order.
    BENTLEYDLL_EXPORT static int Stricmp (const char* s1, const char* s2);

    //! Compare two strings in a case-insensitive way. Equivalent to MSVC _wcsicmp and a character-by-character comparision in GCC. @return 0 if the strings are equal (ignoring case), otherwise a negative or positive number representing order.
    BENTLEYDLL_EXPORT static int Wcsicmp (WCharCP lhs, WCharCP rhs);

    //! Compare up to n characters of two strings in a case-insensitive way. @return 0 if the strings are equal (ignoring case), otherwise a negative or positive number representing order.
    BENTLEYDLL_EXPORT static int Strnicmp (const char* s1, const char* s2, size_t n);

    //! Compare up to n characters of two strings in a case-insensitive way. @return 0 if the strings are equal (ignoring case), otherwise a negative or positive number representing order.
    BENTLEYDLL_EXPORT static int Wcsnicmp (const wchar_t* s1, const wchar_t* s2, size_t n);

    //! Make a copy of a string into a new buffer.  Note that you must call free on the returned pointer or it will leak.
    BENTLEYDLL_EXPORT static char* Strdup (char const* s);

    //! Make a copy of a string into a new buffer.  Note that you must call free on the returned pointer or it will leak.
    BENTLEYDLL_EXPORT static wchar_t* Wcsdup (wchar_t const* s);

    //! This is the equivelent of strtok_s in MSVC and strtok_r in GCC.  
    BENTLEYDLL_EXPORT static char *Strtok (char *strToken, const char *strDelimit, char **context);

    //! This is the equivelent of wcstok_s in MSVC and wcstok in GCC.  
    BENTLEYDLL_EXPORT static wchar_t *Wcstok (wchar_t *wcsToken, const wchar_t *wcsDelimit, wchar_t **context);

    //! reverse the letters in str
    BENTLEYDLL_EXPORT static char *Strrev (char* wstr);

    //! reverse the letters in wstr
    BENTLEYDLL_EXPORT static wchar_t *Wcsrev (wchar_t *wstr);

    //! Equivalent to wcsncpy_s in MSVC
    BENTLEYDLL_EXPORT static wchar_t* Wcsncpy (wchar_t *strDest, size_t destLen, const wchar_t *strSource, size_t count = AsManyAsPossible);

    //! Equivalent to wcsncpy_s in MSVC, for arrays
    template <size_t destArraySize>
    static wchar_t* Wcsncpy (wchar_t (&destArray)[destArraySize], const wchar_t *strSource, size_t count = AsManyAsPossible) {return Wcsncpy (destArray, destArraySize, strSource, count);}

    //! Equivalent to strncpy_s in MSVC
    BENTLEYDLL_EXPORT static char* Strncpy (char *strDest, size_t destLen, const char *strSource, size_t count = AsManyAsPossible);

    //! Equivalent to strncpy_s in MSVC, for arrays
    template <size_t destArraySize>
    static char* Strncpy (char (&destArray)[destArraySize], const char *strSource, size_t count = AsManyAsPossible) {return Strncpy (destArray, destArraySize, strSource, count);}

    //! Parses a string containing an integer number in decimal format
    //! @param[out] value The resulting integer number
    //! @param[in] string A string representation of a decimal value.
    //! @remarks @p string must not contain any extra characters or whitespace. This function does not skip leading whitespace or stop at trailing
    //!      characters, as a function like strtoul would do. That kind of parsing is for the higher level caller to do.
    BENTLEYDLL_EXPORT static BentleyStatus ParseUInt64 (uint64_t& value, WCharCP string);

    //! Parses a UTF-8 encoded string containing an integer number in decimal format
    //! @param[out] value The resulting integer number
    //! @param[in] string A string representation of a decimal value.
    //! @remarks @p string must not contain any extra characters or whitespace. This function does not skip leading whitespace or stop at trailing
    //!      characters, as a function like strtoul would do. That kind of parsing is for the higher level caller to do.
    BENTLEYDLL_EXPORT static BentleyStatus ParseUInt64 (uint64_t& value, Utf8CP string);

    //! Converts an integer number to a string using the decimal format, e.g. 1234ULL becomes "1234" 
    //! @param[out] buf  The output buffer for the formatted string.
    //! @param[in] value the integer value to be formatted.
    BENTLEYDLL_EXPORT static void FormatUInt64 (WCharP buf, uint64_t value);
    BENTLEYDLL_EXPORT static void FormatUInt64 (Utf8P buf, uint64_t value);

    //! Parses a hexadecimal number
    //! @param[out]     value   Assigned the parsed value
    //! @param[in]      string  A string representation of a hex value. May begin with 0x or 0X.
    //! @remarks @p string must not contain any extra characters or whitespace. This function does not skip leading whitespace or stop at trailing
    //!      characters, as a function like strtoul would do. That kind of parsing is for the higher level caller to do.
    BENTLEYDLL_EXPORT static BentleyStatus ParseHexUInt64 (uint64_t& value, WCharCP string);

    //! Format an integer as hexadecimal
    //! @param[out] buf  The output buffer for the formatted string. Must be at least 17 characters in size.
    //! @param[in] value the integer value to be formatted.
    BENTLEYDLL_EXPORT static void          FormatHexUInt64 (WCharP buf, uint64_t value);


    //! Equivalent to _wtoi
    BENTLEYDLL_EXPORT static int Wtoi (wchar_t const* s);

    //! Equivalent to wcstoul
    BENTLEYDLL_EXPORT static unsigned long Wcstoul (wchar_t const* nptr, wchar_t** endptr, int base);

    //! Equivalent to wcstol
    BENTLEYDLL_EXPORT static long          Wcstol (wchar_t const* nptr, wchar_t** endptr, int base);

    //! Equivalent to wcstod
    BENTLEYDLL_EXPORT static double        Wcstod (wchar_t const* s, wchar_t** end);

    BENTLEYDLL_EXPORT static double Wtof (wchar_t const* s);

    //! Equivalent to _itow_s
    //! @remarks Note that output argument is first!
    BENTLEYDLL_EXPORT static BentleyStatus Itow (wchar_t* buffer, int value, size_t sizeInCharacters, int radix);

    //! Move a block of memory measured in wchar_t's. The dest and src buffers can overlap.
    //! @param[in] dest     The output buffer
    //! @param[in] numberOfElements The number of bytes that \a dest can hold.
    //! @param[in] src      The input buffer
    //! @param[in] count    The number of bytes to copy 
    //! @return non-zero error status if (NULL == dest || NULL == src || numberOfElements < count)
    BENTLEYDLL_EXPORT static BentleyStatus Memmove (void *dest, size_t numberOfElements, const void *src, size_t count);

    //! Move a block of memory measured in wchar_t's. The dest and src buffers can overlap.
    //! @param[in] dest     The output buffer
    //! @param[in] numberOfElements The number of chars that \a dest can hold.
    //! @param[in] src      The input buffer
    //! @param[in] count    The number of chars to copy 
    //! @return non-zero error status if (NULL == dest || NULL == src || numberOfElements < count)
    BENTLEYDLL_EXPORT static BentleyStatus Wmemmove (wchar_t *dest, size_t numberOfElements, const wchar_t *src, size_t count);

    //! Copy a block of memory. The dest and src buffers must not overlap.
    //! @param[in] dest     The output buffer
    //! @param[in] numberOfElements The number of chars that \a dest can hold.
    //! @param[in] src      The input buffer
    //! @param[in] count    The number of chars to copy 
    //! @return non-zero error status if (NULL == dest || NULL == src || numberOfElements < count)
    BENTLEYDLL_EXPORT static BentleyStatus Memcpy (void *dest, size_t numberOfElements, const void *src, size_t count);

    //! Copy a block of memory measured in wchar_t's. The dest and src buffers must not overlap.
    //! @param[in] dest     The output buffer
    //! @param[in] numberOfElements The number of wchar_t's that \a dest can hold.
    //! @param[in] src      The input buffer
    //! @param[in] count    The number of wchar_t's to copy 
    //! @return non-zero error status if (NULL == dest || NULL == src || numberOfElements < count)
    BENTLEYDLL_EXPORT static BentleyStatus Wmemcpy (wchar_t *dest, size_t numberOfElements, const wchar_t *src, size_t count);

    //! Tokenizes a string based on the provided delimiters, and adds a WString for each token into the provided collection. This essentially wraps wcstok for more convenient access.
    //! @param[in]      str         The string to tokenize; cannot be NULL or empty
    //! @param[in]      delimiters  One or more delimiters; cannot be NULL or empty
    //! @param[in,out]  tokens      One or more tokens
    static void Split(WCharCP str, WCharCP delimiters, bvector<WString>& tokens) { return Split(str, delimiters, NULL, tokens); }

    //! Tokenizes a string based on the provided delimiters, and adds a WString for each token into the provided collection. This essentially wraps wcstok for more convenient access.
    //! @param[in]      str         The string to tokenize; cannot be NULL or empty
    //! @param[in]      delimiters  One or more delimiters; cannot be NULL or empty
    //! @param[in]      escapeChars One or more characters that can escape a delimieter (e.g. don't split on it); can be NULL or empty.
    //! @param[in,out]  tokens      One or more tokens
    BENTLEYDLL_EXPORT static void Split(WCharCP str, WCharCP delimiters, WCharCP escapeChars, bvector<WString>& tokens);

    //! Tokenizes a string based on the provided delimiters, and adds a Utf8String for each token into the provided collection. This essentially wraps wcstok for more convenient access.
    //! @param[in]      str         The string to tokenize; cannot be NULL or empty
    //! @param[in]      delimiters  One or more delimiters; cannot be NULL or empty
    //! @param[in,out]  tokens      One or more tokens
    static void Split(Utf8CP str, Utf8CP delimiters, bvector<Utf8String>& tokens) { return Split(str, delimiters, NULL, tokens); }

    //! Tokenizes a string based on the provided delimiters, and adds a Utf8String for each token into the provided collection. This essentially wraps wcstok for more convenient access.
    //! @param[in]      str         The string to tokenize; cannot be NULL or empty
    //! @param[in]      delimiters  One or more delimiters; cannot be NULL or empty
    //! @param[in]      escapeChars One or more characters that can escape a delimieter (e.g. don't split on it); can be NULL or empty.
    //! @param[in,out]  tokens      One or more tokens
    BENTLEYDLL_EXPORT static void Split(Utf8CP str, Utf8CP delimiters, Utf8CP escapeChars, bvector<Utf8String>& tokens);

    //! Default logic for parsing a user supplied argument list.  Tokenizes based on whitespace and does not tokenize within double-quoted substrings.
    //! @param[out]     subStrings   The resulting sub strings will be added to this collection
    //! @param[in]      inString     The string to tokenize; cannot be NULL or empty
    //! @param[in]      auxDelimiters (optional) Each character in the string Will be used as a delimiter in addition to whitespace.
    BENTLEYDLL_EXPORT static void ParseArguments (bvector<WString>& subStrings, WCharCP inString, WCharCP auxDelimiters = NULL);

    //! Default logic for parsing a user supplied argument list.  Tokenizes based on whitespace and does not tokenize within double-quoted substrings.
    //! @param[in]      inString        The string to tokenize; cannot be NULL or empty
    //! @param[in]      numSubStrings   Number of substrings provided in subsequent args
    //! @param[out]     ...             A variable list of WString* used to hold the resulting sub strings
    //! @remarks This overload is convenient when the caller wants to discover a fixed maximum number of arguments.
    //! @return Returns the number of output arguments filled.
    BENTLEYDLL_EXPORT static uint32_t ParseArguments (WCharCP inString, uint32_t numSubStrings, ...);

    //! Formats an unsigned 64-bit integer using the same options supported by printf's "%x" and "%X" specifiers.
    //! @param[out]     dest                The buffer which will hold the string representation of the integer. Must not be NULL.
    //! @param[in]      numCharsInBuffer    The maximum number of characters which can be written to \a dest, including the null terminator. Must be at least 1.
    //! @param[in]      val                 The value to format
    //! @param[in]      opts                Flags indicating how formatting should be applied
    //! @param[in]      width               The minimum number of characters in the formatted string. Additional characters will be added according to \a opts. Width > 18 will be truncated to 18.
    //! @param[in]      precision           The minimum number of digits in the formatted string. Leading zeros will be prepended to match the precision. Precision > 16 will be truncated to 16.
    //! @return Returns the number of characters written to \a dest, not including the null terminator
    //! @remarks If the length of the formatted string exceeds \a numCharsInBuffer, the string is truncated (i.e., dest[numCharsInBuffer-1] = 0;)
    //! @remarks This method is provided to produce the same hexadecimal formatting as Snwprintf(), but much more efficiently.
    //! @remarks Typical usage: FormatUInt64(dest, count, val, HexFormatOptions::None) to produce the minimal representation or FormatUInt64(dest, count, val, HexFormatOptions::LeadingZeros, n) to produce fixed width (e.g. n=8 or 16) with leading zeros.
    BENTLEYDLL_EXPORT static int FormatUInt64 (wchar_t *dest, size_t numCharsInBuffer, uint64_t val, HexFormatOptions opts, uint8_t width = 0, uint8_t precision = 1);
    BENTLEYDLL_EXPORT static int FormatUInt64 (Utf8Char *dest, size_t numCharsInBuffer, uint64_t val, HexFormatOptions opts, uint8_t width = 0, uint8_t precision = 1);

    //! Do a lexicographic comparison of specified strings.  This is an alphabetical sort that also takes numbers into account,
    //!   such that "file9" will come before "file11" in the sort order (even though alphabetically 1 is before 9).  This method
    //!   always ignores case.
    //! param[in]    pszName1                IN first string
    //! param[in]    pszName2                IN second string
    //! return   0 if strings are equal, < 0 if pszName1 is less than pszName2, else > 0
    BENTLEYDLL_EXPORT static int LexicographicCompare (WCharCP value0, WCharCP value1);


    }; // BeStringUtilities

END_BENTLEY_NAMESPACE
