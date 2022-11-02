/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once


#include <Bentley/Bentley.h>
#include "bvector.h"
#include "BeStringUtilities.h"
#include "BeAssert.h"
#include <algorithm>
#include <locale>
#include <cstring>
#include <cwctype>
#include <memory> // needed only for backwards compatibility

BEGIN_BENTLEY_NAMESPACE

using bastring = std::string;
using bwstring = std::wstring;

//=======================================================================================
// A string class that derives from std::string, adding a function to create from wchar_t*.
// @bsiclass
//=======================================================================================
struct AString : public bastring
{
    using bastring::bastring;
    AString() : bastring() {}
    AString(CharCP str) : bastring(str ? str : ""){}
    AString(bastring const& other) : bastring(other){}
    BENTLEYDLL_EXPORT AString(WCharCP str);

    // Computes the size, in bytes, of this string's data, including its NULL-terminator.
    size_t SizeInBytes() {return (sizeof (value_type) * (size() + 1));}
};

//=======================================================================================
// A string class that derives from std::wstring, adding functions such as conversion from
// UTF-8, UTF-16, and locale-encoded strings, case-insensitive compare, trimming, padding, etc.
//  @bsiclass
//=======================================================================================
struct WString : public bwstring
{
protected:
    wchar_t* GetBase() {return const_cast<wchar_t*>(data());}

public:
    using bwstring::bwstring;

    WString() : bwstring() {}
    WString(WCharCP str) : bwstring(str ? str : L"") { }
    WString(bwstring const& other) : bwstring(other){}

    // Computes the size, in bytes, of this string's data, including its NULL-terminator.
    size_t SizeInBytes() {return (sizeof (value_type) * (size() + 1));}

    // Construct a WString from a Utf16 encoded Unicode string
    BENTLEYDLL_EXPORT explicit WString(Utf16CP in);

#ifdef DGNV8_WSTRING_LEGACY_SUPPORT
    explicit WString(CharCP in) {AssignA (in);}
#endif

    // @deprecated
    BENTLEYDLL_EXPORT WString(CharCP in, bool isUtf8);

    // Construct a WString from a CharCP string in the specified encoding
    BENTLEYDLL_EXPORT WString(CharCP in, BentleyCharEncoding encoding);

    // Define the contents of this WString from a CharCP using the current system locale
    // @param[in] in   The ASCII string
    // @return  Reference to this string.
    BENTLEYDLL_EXPORT WStringR AssignA (CharCP in);

    // Define the contents of this WString from a Utf8CP
    // @param[in] in   The Utf8 string. May be NULL.
    // @return  Reference to this string.
    BENTLEYDLL_EXPORT WStringR AssignUtf8(Utf8CP in);

    // Define the contents of this WString from a Utf8CP
    // @param[in] in   The Utf8 string. May be NULL.
    // @return  Reference to this string.
    BENTLEYDLL_EXPORT WStringR AssignUtf16(Utf16CP in);

    WStringR AssignOrClear(WCharCP in)
        {
        if (NULL == in)
            {
            clear();
            return  *this;
            }

        assign(in);
        return  *this;
        }
    // Append a null-terminated multibyte, locale-encoded character array to this WString.
    // This will create a WString internally, so if you are appending a character constant it is more efficient to
    // use append (L"string to append").
    // @param[in] in   The multibyte string
    // @return  Reference to this string.
    WStringR AppendA (CharCP in)
        {
        append(WString(in, BentleyCharEncoding::Locale));
        return  *this;
        }

    // Append a Utf8 character array to this WString.
    // This will create a WString internally, so if you are appending a character constant it is more efficient to
    // use append (L"string to append").
    // @param[in] in   The multibyte string
    // @return  Reference to this string.
    WStringR AppendUtf8(CharCP in)
        {
        append(WString(in, BentleyCharEncoding::Utf8));
        return  *this;
        }

    // Get the number of bytes required to hold the current string. This is the value to use to allocate a buffer (e.g. via _alloca) to
    // to call #ConvertToLocaleChars.
    size_t GetMaxLocaleCharBytes() const {return (length() + 1) * sizeof (wchar_t);}

    // Get the contents of this string as a 0-terminated multibyte locale-encoded array.
    // @param out  output buffer
    // @param maxOutBytes maximum number number of bytes to copy to \a out.
    // @return the \a out pointer
    CharP ConvertToLocaleChars(CharP out, size_t maxOutBytes) const {return BeStringUtilities::WCharToCurrentLocaleChar(out, data(), (0 == maxOutBytes) ? GetMaxLocaleCharBytes() : maxOutBytes);}

    // Get the contents of this string as a 0-terminated multibyte locale-encoded array.
    // @remarks The entire string is copied. The caller must ensure that the output buffer is large enough before calling this function. See GetMaxLocaleCharBytes.
    // @param out  output buffer
    // @return the \a out pointer
    CharP ConvertToLocaleChars(CharP out) const {return BeStringUtilities::WCharToCurrentLocaleChar(out, data(), GetMaxLocaleCharBytes());}

    // Equivalent to c_str
    WCharCP GetWCharCP() const {return c_str();}

    // Perform a case-insensitive comparison. @return 0 if the strings are equal (ignoring case), otherwise a negative or positive number representing order. @param other The other string.
    int CompareToI(WCharCP other) const {return BeStringUtilities::Wcsicmp(c_str(), other);}
    // Perform a case-insensitive comparison. @return 0 if the strings are equal (ignoring case), otherwise a negative or positive number representing order. @param other The other string.
    int CompareToI(WStringCR other) const {return CompareToI(other.c_str());}
    // Perform a (case-sensitive) comparison. @return 0 if the strings are equal, otherwise a negative or positive number representing order. @param other The other string.
    int CompareTo(WCharCP other) const {return ::wcscmp(c_str(), other);}
    // Perform a (case-sensitive) comparison. @return 0 if the strings are equal, otherwise a negative or positive number representing order. @param other The other string.
    int CompareTo(WStringCR other) const {return ::wcscmp(c_str(), other.c_str());}
    // Test for equality with another string. @return true if the strings are equal. @param other The other string.
    bool Equals(WCharCP other) const {return (0 == CompareTo(other));}
    // Test for equality with another string. @return true if the strings are equal. @param other The other string.
    bool Equals(WStringCR other) const {return (0 == CompareTo(other));}
    // Test for equality with another string, ignoring case. @return true if the strings are equal (ignoring case). @param other The other string.
    bool EqualsI (WStringCR other) const {return (0 == CompareToI(other.c_str()));}
    // Test for equality with another string, ignoring case. @return true if the strings are equal (ignoring case). @param other The other string.
    bool EqualsI (WCharCP other) const {return (0 == CompareToI(other));}

    // Removes all whitespace from the left and right sides. Whitespace includes space, line feed, carriage return, and tab (e.g. iswspace).
    BENTLEYDLL_EXPORT WStringR Trim();

    // Removes all instances of any of the given characters from the left and right sides.
    BENTLEYDLL_EXPORT WStringR Trim(WCharCP trimCharacters);

    // Pads, if necessary, to the given totalSize by adding charToPadWith to the left side.
    BENTLEYDLL_EXPORT WStringR PadLeft(size_t totalSize, value_type charToPadWith);

    // Pads, if necessary, to the given totalSize by adding charToPadWith to the right side.
    BENTLEYDLL_EXPORT WStringR PadRight(size_t totalSize, value_type charToPadWith);

    // Determines if this instance starts with the provided string.
    BENTLEYDLL_EXPORT bool StartsWith(WCharCP) const;
    // Determines if this instance starts with the provided string, ignoring case.
    BENTLEYDLL_EXPORT bool StartsWithI(WCharCP) const;

    // Determines if this instance ends with the provided string.
    BENTLEYDLL_EXPORT bool EndsWith(WCharCP) const;
    // Determines if this instance ends with the provided string, ignoring case.
    BENTLEYDLL_EXPORT bool EndsWithI(WCharCP) const;

    // Test for whether this string contains another string. @param other The other string. @return true if this string contains the other string.
    BENTLEYDLL_EXPORT bool Contains(WStringCR other) const;
    // Test for whether this string contains another string. @param other The other string. @return true if this string contains the other string.
    BENTLEYDLL_EXPORT bool Contains(WCharCP other) const;
    // Test for whether this string contains another string, ignoring case. @param other The other string. @return true if this string contains the other string.
    BENTLEYDLL_EXPORT bool ContainsI (WStringCR other) const;
    // Test for whether this string contains another string, ignoring case. @param other The other string. @return true if this string contains the other string.
    BENTLEYDLL_EXPORT bool ContainsI (WCharCP other) const;

    // Converts this string, in-place, to all upper case.
    WStringR ToUpper()
        {
        std::transform(begin(), end(), begin(), ::towupper);
        return *this;
        }

    // Converts this string, in-place, to all lower case.
    WStringR ToLower()
        {
        std::transform(begin(), end(), begin(), std::towlower);
        return *this;
        }

    // True if the provided string is NULL or contains no character data.
    static bool IsNullOrEmpty(WCharCP str) {return (NULL == str) || (0 == str[0]);}

    // Put quotes around a string.
    WStringR AddQuotes()
        {
        insert(begin(), L'\"');
        insert(end(),   L'\"');
        return *this;
        }

    // Remove quotes from around a string.
    WStringR DropQuotes()
        {
        if (2 > length())
            return *this;

        iterator first = begin();
        iterator last  = end() - 1;

        if ('\"' != *first || '\"' != *last)
            return *this;

        erase(last);
        erase(first);
        return *this;
        }

    // Replace all instances of a sub string. Returns the number of replacements made.
    BENTLEYDLL_EXPORT size_t ReplaceAll(WCharCP subStringToReplace, WCharCP replacement);

    // Find first occurrence of findString, ignoring case.
    // @param findString The substring to find.
    // @return location of substring or std::string::npos if not found.
    BENTLEYDLL_EXPORT size_t FindI(WCharCP findString) const;

    // Replace first occurrence of findString with replaceString, ignoring case.
    // @param findString The substring to find.
    // @param replaceString The replacement string. @return true is a replacement is made.
    BENTLEYDLL_EXPORT bool ReplaceI(WCharCP findString, WCharCP replaceString);

    // Replace the contents of this string with a formatted result.
    // @param format The sprintf-like format string.
    // @param argptr The variable-length argument list
    // @note This function will fail to generate the complete formatted string <em>on some platforms</em> if the full string is longer than a fixed maximum. In that case, a truncated version is saved and BSIERROR is returned.
    // @return non-zero error status if the if the format string and the args are not valid or if the formatted string was truncated
    BENTLEYDLL_EXPORT BentleyStatus VSprintf(WCharCP format, va_list argptr);

    // Replace the contents of this string with a formatted result.
    // @param format The sprintf-like format string.
    // @note This function will always succeed in generating the complete formatted string if the format string and the args are valid
    // @return non-zero error status if the if the format string and the args are not valid
    BENTLEYDLL_EXPORT BentleyStatus Sprintf(WCharCP format, ...);

    // A "safe" version of swscanf. Actually, all this does is make sure you don't use "%s" in your format string - that's not safe
    // due to buffer overrun and should be avoided.
    template<typename... Args> static int Swscanf_safe(const wchar_t* buffer, const wchar_t* format, Args&&... args) {
        // The lambda is because of a compiler error with just the straight expression. It does not seem to like the temporary variable in the template.
        BeAssert([](const wchar_t* format) {return (std::wstring::npos == std::wstring(format).find(L"%s") && "%s is unsafe, do not use sscanf for that purpose");}(format));
PUSH_DISABLE_DEPRECATION_WARNINGS
        return swscanf(buffer, format, std::forward<Args>(args)...);
POP_DISABLE_DEPRECATION_WARNINGS
    }

};

//=======================================================================================
// @ingroup GROUP_String
//  @bsiclass
//=======================================================================================
struct WPrintfString : WString
{
    // Construct a WString from a format string and optional arguments
    BENTLEYDLL_EXPORT WPrintfString(WCharCP format, ...);

    // Construct a WString from a format string and a va_list
    BENTLEYDLL_EXPORT WPrintfString(WCharCP format, va_list);
};

//=======================================================================================
// Contains a UTF-8 encoded string. This class derives from std::string,
//  but it is intended to hold only UTF-8 encoded strings.
// This class also defines utility functions for constructing and manipulating the string.
//  @bsiclass
//=======================================================================================
struct Utf8String : public bastring
{
public:
    using bastring::bastring;
    Utf8String() : bastring() {}
    Utf8String(Utf8CP str) : bastring(str ? str : "") { }
    Utf8String(bastring const& other) : bastring(other){}

    BENTLEYDLL_EXPORT Utf8String(struct Utf8PrintfString&&);

    // Construct a Utf8String by converting from a wchar_t string.
    explicit Utf8String(WCharCP str) : bastring(){BeStringUtilities::WCharToUtf8(*this, str);}
    // Construct a Utf8String by converting from a wchar_t string.
    explicit Utf8String(WString str) : bastring(){BeStringUtilities::WCharToUtf8(*this, str.c_str(), str.size());}
    // Construct a Utf8String by converting from a wchar_t string.
    Utf8StringR Assign(WCharCP str){BeStringUtilities::WCharToUtf8(*this, str); return *this;}

    // Computes the size, in bytes, of this string's data, including its NULL-terminator.
    size_t SizeInBytes() const {return (sizeof (value_type) * (size() + 1));}

    // Test if this string contains only characters less than or equal to 127.
    BENTLEYDLL_EXPORT bool IsAscii() const;
    // Replace the contents of this string with a formatted result.
    // @param format The sprintf-like format string.
    // @param argptr The variable-length argument list
    // @note This function will fail to generate the complete formatted string <em>on some platforms</em> if the full string is longer than a fixed maximum. In that case, a truncated version is saved and BSIERROR is returned.
    // @return non-zero error status if the if the format string and the args are not valid or if the formatted string was truncated
    BENTLEYDLL_EXPORT BentleyStatus VSprintf(Utf8CP format, va_list argptr);
    // Replace the contents of this string with a formatted result.
    // @param format The sprintf-like format string.
    // @note This function will always succeed in generating the complete formatted string if the format string and the args are valid
    // @return non-zero error status if the if the format string and the args are not valid
    BENTLEYDLL_EXPORT BentleyStatus Sprintf(Utf8CP format, ...);
    // Utility function to test if \a value represents the empty string. This function interprets NULL to be the empty string.
    static bool IsNullOrEmpty(Utf8CP value) {return ((NULL == value) || (0 == *value));}

    // Determine whether the supplied character is a whitespace character in the ascii (below 128) code page. This is necessary since
    // the c "isspace" function is locale specific and sometimes returns true for the non-breaking-space character (0xA0), which is not a valid
    // test for a Utf8 string. Note this does not test for VT, or FF as they are considered obsolete.
    static bool IsAsciiWhiteSpace(char val) {return val==' ' || val=='\t' || val == '\n' || val == '\r';}

    // Perform a case-insensitive comparison.
    // @param other The other string.
    // @return 0 if the strings are equal (ignoring case), otherwise a negative or positive number representing order.
    int CompareToI(Utf8CP other) const {return BeStringUtilities::Stricmp(c_str(), other);}
    // Perform a case-insensitive comparison. @return 0 if the strings are equal (ignoring case), otherwise a negative or positive number representing order. @param other The other string.
    int CompareToI(Utf8StringCR other) const {return CompareToI(other.c_str());}
    // Perform a (case-sensitive) comparison. @return 0 if the strings are equal, otherwise a negative or positive number representing order. @param other The other string.
    // @return 0 if the strings are equal (ignoring case), otherwise a negative or positive number representing order.
    // @note Use only if you know that both strings contain only ASCII characters.
    int CompareToIAscii(Utf8CP other) const {return BeStringUtilities::StricmpAscii(c_str(), other);}
    // Perform a case-insensitive comparison.
    // @return 0 if the strings are equal (ignoring case), otherwise a negative or positive number representing order. @param other The other string.
    // @note Use only if you know that both strings contain only ASCII characters.
    int CompareToIAscii(Utf8StringCR other) const {return CompareToIAscii(other.c_str());}
    // Perform a (case-sensitive) comparison. @return 0 if the strings are equal, otherwise a negative or positive number representing order. @param other The other string.
    int CompareTo(Utf8CP other) const {return std::strcmp(c_str(), other);}
    // Perform a (case-sensitive) comparison. @return 0 if the strings are equal, otherwise a negative or positive number representing order. @param other The other string.
    int CompareTo(Utf8StringCR other) const {return std::strcmp(c_str(), other.c_str());}
    // Test for equality with another string. @return true if the strings are equal. @param other The other string.
    bool Equals(Utf8CP other) const {return (0 == CompareTo(other));}
    // Test for equality with another string. @return true if the strings are equal. @param other The other string.
    bool Equals(Utf8StringCR other) const {return (0 == CompareTo(other));}
    // Test for equality with another string, ignoring case. @return true if the strings are equal (ignoring case). @param other The other string.
    bool EqualsI(Utf8StringCR other) const {return (0 == CompareToI(other.c_str()));}
    // Test for equality with another string, ignoring case. @return true if the strings are equal (ignoring case). @param other The other string.
    bool EqualsI(Utf8CP other) const {return (0 == CompareToI(other));}
    // Test for equality with another string, ignoring case. @return true if the strings are equal (ignoring case). @param other The other string.
    // @note Use only if you know that both strings contain only ASCII characters.
    bool EqualsIAscii(Utf8StringCR other) const {return (0 == CompareToIAscii(other.c_str()));}
    // Test for equality with another string, ignoring case. @return true if the strings are equal (ignoring case). @param other The other string.
    // @note Use only if you know that both strings contain only ASCII characters.
    bool EqualsIAscii(Utf8CP other) const {return (0 == CompareToIAscii(other));}
    // Checks if string ends with other string (case sensitive)
    BENTLEYDLL_EXPORT bool EndsWith(Utf8StringCR ending) const;
    // Removes all whitespace from the left and right sides. Whitespace includes space, line feed, carriage return, and tab (e.g. iswspace).
    BENTLEYDLL_EXPORT Utf8StringR Trim();
    // Removes all whitespace from the end. Whitespace includes space, line feed, carriage return, and tab (e.g. iswspace).
    BENTLEYDLL_EXPORT Utf8StringR TrimEnd();
    // Removes all instances of any of the given characters from the left and right sides.
    BENTLEYDLL_EXPORT Utf8StringR Trim(Utf8CP trimCharacters);

    // Determines if this instance starts with the provided string.
    BENTLEYDLL_EXPORT bool StartsWith(Utf8CP) const;
    // Determines if this instance starts with the provided string, ignoring case.
    BENTLEYDLL_EXPORT bool StartsWithI(Utf8CP) const;
    // Determines if this instance starts with the provided string, ignoring case.
    // @note Use only if you know that both strings contain only ASCII characters.
    BENTLEYDLL_EXPORT bool StartsWithIAscii(Utf8CP) const;

    // Determines if this instance ends with the provided string.
    BENTLEYDLL_EXPORT bool EndsWith(Utf8CP) const;
    // Determines if this instance ends with the provided string, ignoring case.
    BENTLEYDLL_EXPORT bool EndsWithI(Utf8CP) const;
    // Determines if this instance ends with the provided string, ignoring case.
    // @note Use only if you know that both strings contain only ASCII characters.
    BENTLEYDLL_EXPORT bool EndsWithIAscii(Utf8CP) const;

    // Test for whether this string contains another string. @param other The other string. @return true if this string contains the other string.
    BENTLEYDLL_EXPORT bool Contains(Utf8StringCR other) const;
    // Test for whether this string contains another string. @param other The other string. @return true if this string contains the other string.
    BENTLEYDLL_EXPORT bool Contains(Utf8CP other) const;
    // Test for whether this string contains another string, ignoring case. @param other The other string. @return true if this string contains the other string.
    BENTLEYDLL_EXPORT bool ContainsI(Utf8StringCR other) const;
    // Test for whether this string contains another string, ignoring case. @param other The other string. @return true if this string contains the other string.
    BENTLEYDLL_EXPORT bool ContainsI(Utf8CP other) const;

    // Update this string to be equal to \a in. If \a in is NULL, clear this string.
    Utf8StringR AssignOrClear(Utf8CP in)
        {
        if (nullptr == in)
            clear();
        else
            assign(in);

        return *this;
        }

    // Replace all instances of a sub string. Returns the number of replacements made.
    BENTLEYDLL_EXPORT size_t ReplaceAll(Utf8CP subStringToReplace, Utf8CP replacement);

    // Equivalent to tolower
    // @note This function is dangerous to use on non-ASCII strings.
    static char ToLowerChar(char c) {return (char)tolower(c);}

    // Converts this string, in-place, to all lower case.
    // @remarks This function can be very slow if the string contains non-ascii characters.
    Utf8StringR ToLower()
        {
        if (IsAscii())
            std::transform(begin(), end(), begin(), ToLowerChar);
        else
            {
            WString wstr(c_str(), BentleyCharEncoding::Utf8);
            wstr.ToLower();
            clear();
            BeStringUtilities::WCharToUtf8(*this, wstr.c_str());
            }
        return *this;
        }

    static Utf8String FromLower(Utf8CP val) {
        return Utf8String(val).ToLower();
    }

    // Converts this string, in-place, to all upper case.
    // @remarks This function can be very slow if the string contains non-ascii characters.
    BENTLEYDLL_EXPORT Utf8StringR ToUpper();

    // Reads the next token delimited by any character in \a delims or \0.
    // @param[out] next    set to next token, if found, or cleared if not
    // @param[in]  delims  the characters that could delimit the tokens
    // @param[in]  offset  where to start search
    // @return 1 beyond the end of the current token or npos if token not found
    // Example
    // @code
    //     // Read lines from a string, where each line is delimited by \n. The last line need not have a trailing \n.
    //     size_t offset = 0;
    //     Utf8String m;
    //     while ((offset = m_virtuals.GetNextToken (m, "\n", offset)) != Utf8String::npos)
    //         {
    //         printf ("%s\n", m.c_str());
    //         }
    // @endcode
    // @note If this string ends with a delimiter, then the last token returned is the one before the trailing delimiter.
    // If this string does not end with a delimiter, then the last token is everything following the last delimiter (if any) and the end of this string.
    // If this string has no delimiters at all, then the first and only token returned is the string itself.
    // So, for example, if this string were "abc\n" then GetNextToken (next, "\n", 0) would set next to "abc" and return 4, and GetNextToken (next, "\n", 4) would return npos.
    // If this string were "abc", the two calls would return the same results.
    size_t GetNextToken(Utf8StringR next, CharCP delims, size_t offset) const
        {
        size_t idelim = find_first_of(delims, offset);
        if (idelim == npos)
            {
            if (offset >= length())
                {
                next.clear();
                return npos;
                }
            idelim = length();  // there's no trailing delimiter, so the last token is just the rest of the string.
            }
        next = substr(offset, idelim-offset);
        return idelim+1;
        }

    // Put quotes around a string.
    Utf8StringR AddQuotes()
        {
        insert(begin(), '\"');
        insert(end(),   '\"');
        return *this;
        }

    // Remove quotes from around a string.
    Utf8StringR DropQuotes()
        {
        if (2 > length())
            return *this;

        iterator first = begin();
        iterator last  = end() - 1;

        if ('\"' != *first || '\"' != *last)
            return *this;

        erase(last);
        erase(first);
        return *this;
        }

    // A "safe" version of sscanf. Actually, all this does is make sure you don't use "%s" in your format string - that's not safe
    // due to buffer overrun and should be avoided.
    template<typename... Args> static int Sscanf_safe(const char* const buffer, const char* const format, Args&&... args) {
        // NOTE: When we use C++17 this can be string_view and become a static_assert
        // The lambda is because of a compiler error with just the straight expression. It does not seem to like the temporary variable in the template.
        BeAssert([](const char* const format) {return (std::string::npos == std::string(format).find("%s") && "%s is unsafe, do not use sscanf for that purpose");}(format));
PUSH_DISABLE_DEPRECATION_WARNINGS // this is safe, because we're sure the format string doesn't use
        return sscanf(buffer, format, std::forward<Args>(args)...);
POP_DISABLE_DEPRECATION_WARNINGS
    }
};

//=======================================================================================
// Construct a Utf8String from a printf format string and arguments.
//  @bsiclass
//=======================================================================================
struct Utf8PrintfString : Utf8String
{
private:
    Utf8PrintfString() : Utf8String() {}

public:
    BENTLEYDLL_EXPORT explicit Utf8PrintfString(Utf8CP format, ...);

    // N.B. Do NOT make this an overload of the constructor. If you pass a
    // single CharCP argument after 'format' (e.g.
    // Utf8PrintfString("%s","foo")), MSVC 14 will interpret that as a
    // va_list, erroneously choosing the below parameter list, which leads to a crash.
    BENTLEYDLL_EXPORT static Utf8PrintfString CreateFromVaList(Utf8CP format, va_list);
};

// A bvector of WString objects.
typedef bvector<WString> T_WStringVector;

typedef T_WStringVector*        T_WStringVectorP, &T_WStringVectorR;
typedef T_WStringVector const * T_WStringVectorCP;
typedef T_WStringVector const & T_WStringVectorCR;

// A bvector of Utf8String objects.
typedef bvector<Utf8String> T_Utf8StringVector;

typedef T_Utf8StringVector*         T_Utf8StringVectorP, &T_Utf8StringVectorR;
typedef T_Utf8StringVector const*   T_Utf8StringVectorCP;
typedef T_Utf8StringVector const&   T_Utf8StringVectorCR;

END_BENTLEY_NAMESPACE
