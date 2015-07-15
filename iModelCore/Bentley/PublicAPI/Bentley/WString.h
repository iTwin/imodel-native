/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/WString.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

// __PUBLISH_SECTION_START__

#include <Bentley/Bentley.h>
#include <Bentley/stdcxx/basic_string.h>
#include "bvector.h"
#include "BeStringUtilities.h"
#include "BeAssert.h"
#include "BentleyAllocator.h"

#if defined (BENTLEY_TOOL_CONTEXT_IsLinuxGcc)
#include <cstring>
#endif

#define __W_CONST(name) L##name
#define UTF8_TO_WCONST(name) __W_CONST(name)

BEGIN_BENTLEY_NAMESPACE

typedef Bstdcxx::basic_string <char,     std::char_traits<char>,     BentleyAllocator<char> >      bastring;
typedef Bstdcxx::basic_string <wchar_t,  std::char_traits<wchar_t>,  BentleyAllocator<wchar_t> >   bwstring;

//=======================================================================================
//! A string class that has many of the same capabilities as std::string, plus
//! additional functions such as conversion from wchar_t*.
// @bsiclass                                                    Keith.Bentley   04/11
//=======================================================================================
struct AString : public bastring
    {
    AString() : bastring() {}
    AString(CharCP str) : bastring(str){}
    AString(bastring const& other) : bastring(other){}
    AString(bastring const& __str, size_t __pos, size_t __n = npos) : bastring (__str, __pos, __n){}
    AString(CharCP __s, size_t __n) : bastring (__s, __n){}
    AString(size_t __n, char __c) : bastring (__n, __c){}
    AString(iterator __beg, iterator __end) : bastring (__beg, __end){}
    AString(const_iterator __beg, const_iterator __end) : bastring (__beg, __end){}
    AString(reverse_iterator __beg, reverse_iterator __end) : bastring (__beg, __end){}
    AString(const_reverse_iterator __beg, const_reverse_iterator __end) : bastring (__beg, __end){}
    BENTLEYDLL_EXPORT AString (WCharCP str);

    //! Computes the size, in bytes, of this string's data, including its NULL-terminator.
    size_type SizeInBytes() {return (sizeof (value_type) * (size() + 1));}
    };

//=======================================================================================
//! A string class that has many of the same capabilities as std::string, plus
//! additional functions such as conversion from UTF-8, UTF-16, and local-encoded strings,
//! case-insensitive compare, trimming, padding, and others.
//! @ingroup BeStringGroup
//  @bsiclass                                                   Keith.Bentley   03/11
//=======================================================================================
struct WString : public bwstring
    {
protected:
    wchar_t* GetBase() {return const_cast<wchar_t*>(data());}

public:
    WString() : bwstring() {}
    WString(WCharCP str) : bwstring(str){}
    WString(bwstring const& other) : bwstring(other){}
    WString(bwstring const& __str, size_t __pos, size_t __n = npos) : bwstring (__str, __pos, __n){}
    WString(WCharCP __s, size_t __n) : bwstring (__s, __n){}
    WString(size_t __n, wchar_t __c) : bwstring (__n, __c){}
    WString(iterator __beg, iterator __end) : bwstring (__beg, __end){}
    WString(const_iterator __beg, const_iterator __end) : bwstring (__beg, __end){}
    WString(reverse_iterator __beg, reverse_iterator __end) : bwstring (__beg, __end){}
    WString(const_reverse_iterator __beg, const_reverse_iterator __end) : bwstring (__beg, __end){}

    //! Computes the size, in bytes, of this string's data, including its NULL-terminator.
    size_type SizeInBytes() {return (sizeof (value_type) * (size() + 1));}

    //! Construct a WString from a Utf16 encoded Unicode string
    BENTLEYDLL_EXPORT explicit WString (Utf16CP in);

#ifdef DGNV8_WSTRING_LEGACY_SUPPORT
    explicit WString (CharCP in) {AssignA (in);}
#endif

    //! Construct a WString from a CharCP using either the current system locale or Utf8
    BENTLEYDLL_EXPORT WString (CharCP in, bool isUtf8);

    //! Construct a WString from a CharCP string in the specified encoding
    BENTLEYDLL_EXPORT WString (CharCP in, BentleyCharEncoding encoding);

    //! Define the contents of this WString from a CharCP using the current system locale
    //! @param[in] in   The ASCII string
    //! @return  Reference to this string.
    BENTLEYDLL_EXPORT WStringR AssignA (CharCP in);

    //! Define the contents of this WString from a Utf8CP
    //! @param[in] in   The Utf8 string. May be NULL.
    //! @return  Reference to this string.
    BENTLEYDLL_EXPORT WStringR AssignUtf8 (Utf8CP in);

    //! Define the contents of this WString from a Utf8CP
    //! @param[in] in   The Utf8 string. May be NULL.
    //! @return  Reference to this string.
    BENTLEYDLL_EXPORT WStringR AssignUtf16 (Utf16CP in);

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
    //! Append a null-terminated multibyte, locale-encoded character array to this WString.
    //! This will create a WString internally, so if you are appending a character constant it is more efficient to
    //! use append (L"string to append").
    //! @param[in] in   The multibyte string
    //! @return  Reference to this string.
    WStringR AppendA (CharCP in)
        {
        append (WString(in, BentleyCharEncoding::Locale));
        return  *this;
        }

    //! Append a Utf8 character array to this WString.
    //! This will create a WString internally, so if you are appending a character constant it is more efficient to
    //! use append (L"string to append").
    //! @param[in] in   The multibyte string
    //! @return  Reference to this string.
    WStringR AppendUtf8 (CharCP in)
        {
        append (WString(in, BentleyCharEncoding::Utf8));
        return  *this;
        }

    //! Get the number of bytes required to hold the current string. This is the value to use to allocate a buffer (e.g. via _alloca) to
    //! to call #ConvertToLocaleChars.
    size_t GetMaxLocaleCharBytes() const {return (length() + 1) * sizeof (wchar_t);}

    //! Get the contents of this string as a 0-terminated multibyte locale-encoded array.
    //! @param out  output buffer
    //! @param maxOutBytes maximum number number of bytes to copy to \a out.
    //! @return the \a out pointer
    CharP ConvertToLocaleChars (CharP out, size_t maxOutBytes) const {return BeStringUtilities::WCharToCurrentLocaleChar (out, data(), (0 == maxOutBytes) ? GetMaxLocaleCharBytes() : maxOutBytes);}

    //! Get the contents of this string as a 0-terminated multibyte locale-encoded array.
    //! @remarks The entire string is copied. The caller must ensure that the output buffer is large enough before calling this function. See GetMaxLocaleCharBytes.
    //! @param out  output buffer
    //! @return the \a out pointer
    CharP ConvertToLocaleChars (CharP out) const {return BeStringUtilities::WCharToCurrentLocaleChar (out, data(), GetMaxLocaleCharBytes());}

    //! Equivalent to c_str
    WCharCP GetWCharCP() const {return c_str();}

      /**
       *  @brief  Get a substring.
       *  @param __pos  Index of first character (default 0).
       *  @param __n  Number of characters in substring (default remainder).
       *  @return  The new string.
       *  @throw  std::out_of_range  If pos > size().
       *
       *  Construct and return a new string using the @a __n characters starting
       *  at @a __pos.  If the string is too short, use the remainder of the
       *  characters.  If @a __pos is beyond the end of the string, out_of_range
       *  is thrown.
      */
    WString substr (size_type __pos = 0, size_type __n = npos) const { return WString (*this, __pos, __n); }

    //! Perform a case-insensitive comparison. @return 0 if the strings are equal (ignoring case), otherwise a negative or positive number representing order. @param other The other string.
    int CompareToI(WCharCP other) const {return BeStringUtilities::Wcsicmp (c_str(), other);}
    //! Perform a case-insensitive comparison. @return 0 if the strings are equal (ignoring case), otherwise a negative or positive number representing order. @param other The other string.
    int CompareToI(WStringCR other) const {return CompareToI(other.c_str());}
    //! Perform a (case-sensitive) comparison. @return 0 if the strings are equal, otherwise a negative or positive number representing order. @param other The other string.
    int CompareTo (WCharCP other) const {return ::wcscmp (c_str(), other);}
    //! Perform a (case-sensitive) comparison. @return 0 if the strings are equal, otherwise a negative or positive number representing order. @param other The other string.
    int CompareTo (WStringCR other) const {return ::wcscmp (c_str(), other.c_str());}
    //! Test for equality with another string. @return true if the strings are equal. @param other The other string.
    bool Equals (WCharCP other) const {return (0 == CompareTo(other));}
    //! Test for equality with another string. @return true if the strings are equal. @param other The other string.
    bool Equals (WStringCR other) const {return (0 == CompareTo(other));}
    //! Test for equality with another string, ignoring case. @return true if the strings are equal (ignoring case). @param other The other string.
    bool EqualsI (WStringCR other) const {return (0 == CompareToI(other.c_str()));}
    //! Test for equality with another string, ignoring case. @return true if the strings are equal (ignoring case). @param other The other string.
    bool EqualsI (WCharCP other) const {return (0 == CompareToI(other));}

    //! Removes all whitespace from the left and right sides. Whitespace includes space, line feed, carriage return, and tab (e.g. iswspace).
    BENTLEYDLL_EXPORT WStringR Trim ();

    //! Removes all instances of any of the given characters from the left and right sides.
    BENTLEYDLL_EXPORT WStringR Trim (WCharCP trimCharacters);

    //! Pads, if necessary, to the given totalSize by adding charToPadWith to the left side.
    BENTLEYDLL_EXPORT WStringR PadLeft (size_t totalSize, value_type charToPadWith);

    //! Pads, if necessary, to the given totalSize by adding charToPadWith to the right side.
    BENTLEYDLL_EXPORT WStringR PadRight (size_t totalSize, value_type charToPadWith);

    //! Determines if this instance starts with the provided string.
    BENTLEYDLL_EXPORT bool StartsWith(WCharCP) const;
    //! Determines if this instance starts with the provided string, ignoring case.
    BENTLEYDLL_EXPORT bool StartsWithI(WCharCP) const;

    //! Determines if this instance ends with the provided string.
    BENTLEYDLL_EXPORT bool EndsWith(WCharCP) const;
    //! Determines if this instance ends with the provided string, ignoring case.
    BENTLEYDLL_EXPORT bool EndsWithI(WCharCP) const;

    //! Test for whether this string contains another string. @param other The other string. @return true if this string contains the other string. 
    BENTLEYDLL_EXPORT bool Contains (WStringCR other) const;
    //! Test for whether this string contains another string. @param other The other string. @return true if this string contains the other string. 
    BENTLEYDLL_EXPORT bool Contains (WCharCP other) const;
    //! Test for whether this string contains another string, ignoring case. @param other The other string. @return true if this string contains the other string. 
    BENTLEYDLL_EXPORT bool ContainsI (WStringCR other) const;
    //! Test for whether this string contains another string, ignoring case. @param other The other string. @return true if this string contains the other string. 
    BENTLEYDLL_EXPORT bool ContainsI (WCharCP other) const;

    //! Converts this string, in-place, to all upper case.
// __PUBLISH_SECTION_END__
    //! @remarks A WString can share the string buffer with another WString object. Do not try to modify its internal buffer without first triggering the copy-on-write mechanism to make sure that this object has its own copy.
// __PUBLISH_SECTION_START__
    WStringR ToUpper ()
        {
        std::transform (begin(), end(), begin(), ::towupper);
        return *this;
        }

    //! Converts this string, in-place, to all lower case.
// __PUBLISH_SECTION_END__
    //! @remarks A WString can share the string buffer with another WString object. Do not try to modify its internal buffer without first triggering the copy-on-write mechanism to make sure that this object has its own copy.
// __PUBLISH_SECTION_START__
    WStringR ToLower () 
        {
        std::transform (begin(), end(), begin(), ::towlower);
        return *this;
        }

    //! True if the provided string is NULL or contains no character data.
    static bool IsNullOrEmpty (WCharCP str) { return (NULL == str) || (0 == str[0]); }

    //! Put quotes around a string.
    WStringR AddQuotes ()
        {
        insert (begin(), L'\"');
        insert (end(),   L'\"');
        return *this;
        }

    //! Remove quotes from around a string.
    WStringR DropQuotes ()
        {
        if (2 > length())
            return *this;

        iterator first = begin();
        iterator last  = end() - 1;

        if ('\"' != *first || '\"' != *last)
            return *this;

        erase (last);
        erase (first);
        return *this;
        }

    //! Replace all instances of a sub string. Returns the number of replacements made.
    BENTLEYDLL_EXPORT size_t ReplaceAll (WCharCP subStringToReplace, WCharCP replacement);

    //! Replace the contents of this string with a formatted result. 
    //! @param format The sprintf-like format string. 
    //! @param argptr The variable-length argument list
    //! @note This function will fail to generate the complete formatted string <em>on some platforms</em> if the full string is longer than a fixed maximum. In that case, a truncated version is saved and BSIERROR is returned.
    //! @return non-zero error status if the if the format string and the args are not valid or if the formatted string was truncated
    BENTLEYDLL_EXPORT BentleyStatus VSprintf (WCharCP format, va_list argptr);
    //! Replace the contents of this string with a formatted result. 
    //! @param format The sprintf-like format string.
    //! @note This function will always succeed in generating the complete formatted string if the format string and the args are valid
    //! @return non-zero error status if the if the format string and the args are not valid
    BENTLEYDLL_EXPORT BentleyStatus Sprintf (WCharCP format, ...);
    };

//=======================================================================================
//! @ingroup BeStringGroup
//  @bsiclass                                                   Keith.Bentley   11/11
//=======================================================================================
struct WPrintfString : WString
{
    //! Construct a WString from a format string and optional arguments
    BENTLEYDLL_EXPORT WPrintfString(WCharCP format, ...);

    //! Construct a WString from a format string and a va_list 
    BENTLEYDLL_EXPORT WPrintfString(WCharCP format, va_list);

    operator WCharCP(){return c_str();}
};

//=======================================================================================
//! Contains a UTF-8 encoded string. This class has many of the capabilities of std::string,
//! except that it is intended to hold only UTF-8 encoded strings.
//! This class also defines utility functions for constructing and manipulating the string.
//! @ingroup BeStringGroup
//  @bsiclass                                                   Keith.Bentley   04/11
//=======================================================================================
struct Utf8String : public bastring
    {
    Utf8String() : bastring() {}
    Utf8String(Utf8CP str) : bastring(str){}
    Utf8String(bastring const& other) : bastring(other){}
    Utf8String(bastring const& __str, size_t __pos, size_t __n = npos) : bastring (__str, __pos, __n){}
    Utf8String(CharCP __s, size_t __n) : bastring (__s, __n){}
    Utf8String(size_t __n, char __c) : bastring (__n, __c){}
    //Utf8String(CharCP __beg, CharCP __end) : bastring (__beg, __end){}
    Utf8String(iterator __beg, iterator __end) : bastring (__beg, __end){}
    Utf8String(const_iterator __beg, const_iterator __end) : bastring (__beg, __end){}
    Utf8String(reverse_iterator __beg, reverse_iterator __end) : bastring (__beg, __end){}
    Utf8String(const_reverse_iterator __beg, const_reverse_iterator __end) : bastring (__beg, __end){}

    //! Construct a Utf8String by converting from a wchar_t string.
    explicit Utf8String(WCharCP str) : bastring(){BeStringUtilities::WCharToUtf8 (*this, str);}
    //! Construct a Utf8String by converting from a wchar_t string.
    explicit Utf8String(WString str) : bastring(){BeStringUtilities::WCharToUtf8 (*this, str.c_str(), str.size());}
    //! Construct a Utf8String by converting from a wchar_t string.
    Utf8StringR Assign (WCharCP str){BeStringUtilities::WCharToUtf8 (*this, str); return *this;}

    //! Computes the size, in bytes, of this string's data, including its NULL-terminator.
    size_type SizeInBytes() {return (sizeof (value_type) * (size() + 1));}

    //! Test if this string contains only characters less than or equal to 127.
    BENTLEYDLL_EXPORT bool IsAscii ();
    //! Replace the contents of this string with a formatted result. 
    //! @param format The sprintf-like format string. 
    //! @param argptr The variable-length argument list
    //! @note This function will fail to generate the complete formatted string <em>on some platforms</em> if the full string is longer than a fixed maximum. In that case, a truncated version is saved and BSIERROR is returned.
    //! @return non-zero error status if the if the format string and the args are not valid or if the formatted string was truncated
    BENTLEYDLL_EXPORT BentleyStatus VSprintf (Utf8CP format, va_list argptr);
    //! Replace the contents of this string with a formatted result. 
    //! @param format The sprintf-like format string.
    //! @note This function will always succeed in generating the complete formatted string if the format string and the args are valid
    //! @return non-zero error status if the if the format string and the args are not valid
    BENTLEYDLL_EXPORT BentleyStatus Sprintf (Utf8CP format, ...);
    //! Utility function to test if \a value represents the empty string. This function interprets NULL to be the empty string.
    static bool IsNullOrEmpty(Utf8CP value) { return ((NULL == value) || (0 == *value)); }

    //! Determine whether the supplied character is a whitespace character in the ascii (below 128) code page. This is necessary since
    //! the c "isspace" function is locale specific and sometimes returns true for the non-breaking-space character (0xA0), which is not a valid
    //! test for a Utf8 string. Note this does not test for VT, or FF as they are considered obsolete.
    static bool IsAsciiWhiteSpace(char val) {return val==' ' || val=='\t' || val == '\n' || val == '\r';}

    //! Perform a case-insensitive comparison. 
    //! @param other The other string.
    //! @return 0 if the strings are equal (ignoring case), otherwise a negative or positive number representing order. 
    int CompareToI(Utf8CP other) const {return BeStringUtilities::Stricmp (c_str(), other);}
    //! Perform a case-insensitive comparison. @return 0 if the strings are equal (ignoring case), otherwise a negative or positive number representing order. @param other The other string.
    int CompareToI(Utf8StringCR other) const {return CompareToI(other.c_str());}
    //! Perform a (case-sensitive) comparison. @return 0 if the strings are equal, otherwise a negative or positive number representing order. @param other The other string.
    int CompareTo (Utf8CP other) const {return ::strcmp (c_str(), other);}
    //! Perform a (case-sensitive) comparison. @return 0 if the strings are equal, otherwise a negative or positive number representing order. @param other The other string.
    int CompareTo (Utf8StringCR other) const {return ::strcmp (c_str(), other.c_str());}
    //! Test for equality with another string. @return true if the strings are equal. @param other The other string.
    bool Equals (Utf8CP other) const {return (0 == CompareTo(other));}
    //! Test for equality with another string. @return true if the strings are equal. @param other The other string.
    bool Equals (Utf8StringCR other) const {return (0 == CompareTo(other));}
    //! Test for equality with another string, ignoring case. @return true if the strings are equal (ignoring case). @param other The other string.
    bool EqualsI (Utf8StringCR other) const {return (0 == CompareToI(other.c_str()));}
    //! Test for equality with another string, ignoring case. @return true if the strings are equal (ignoring case). @param other The other string.
    bool EqualsI (Utf8CP other) const {return (0 == CompareToI(other));}
    //! Removes all whitespace from the left and right sides. Whitespace includes space, line feed, carriage return, and tab (e.g. iswspace).
    BENTLEYDLL_EXPORT Utf8StringR Trim ();
    //! Removes all whitespace from the end. Whitespace includes space, line feed, carriage return, and tab (e.g. iswspace).
    BENTLEYDLL_EXPORT Utf8StringR TrimEnd ();
    //! Removes all instances of any of the given characters from the left and right sides.
    BENTLEYDLL_EXPORT Utf8StringR Trim (Utf8CP trimCharacters);

    //! Test for whether this string contains another string. @param other The other string. @return true if this string contains the other string. 
    BENTLEYDLL_EXPORT bool Contains(Utf8StringCR other) const;
    //! Test for whether this string contains another string. @param other The other string. @return true if this string contains the other string. 
    BENTLEYDLL_EXPORT bool Contains(Utf8CP other) const;
    //! Test for whether this string contains another string, ignoring case. @param other The other string. @return true if this string contains the other string. 
    BENTLEYDLL_EXPORT bool ContainsI(Utf8StringCR other) const;
    //! Test for whether this string contains another string, ignoring case. @param other The other string. @return true if this string contains the other string. 
    BENTLEYDLL_EXPORT bool ContainsI(Utf8CP other) const;

    //! Update this string to be equal to \a in. If \a in is NULL, clear this string.
    Utf8StringR AssignOrClear(Utf8CP in)
        {
        if (nullptr == in)
            clear();
        else
            assign(in);

        return *this;
        }

    //! Replace all instances of a sub string. Returns the number of replacements made.
    BENTLEYDLL_EXPORT size_t ReplaceAll(Utf8CP subStringToReplace, Utf8CP replacement);

    //! Equivalent to tolower
    //! @note This function is dangerous to use on non-ASCII strings.
    static char ToLowerChar(char c) {return (char)tolower(c);}

    //! Converts this string, in-place, to all lower case.
    //! @remarks This function can be very slow if the string contains non-ascii characters.
    Utf8StringR ToLower ()
        {
        if (IsAscii())
            std::transform (begin(), end(), begin(), ToLowerChar);
        else
            {
            WString wstr (c_str(), BentleyCharEncoding::Utf8);
            wstr.ToLower();
            clear ();
            BeStringUtilities::WCharToUtf8 (*this, wstr.c_str());
            }
        return *this;
        }
    
    //! Converts this string, in-place, to all upper case.
    //! @remarks This function can be very slow if the string contains non-ascii characters.
    BENTLEYDLL_EXPORT Utf8StringR ToUpper ();

    //! Reads the next token delimited by any character in \a delims or \0.
    //! @param[out] next    set to next token, if found, or cleared if not
    //! @param[in]  delims  the characters that could delimit the tokens
    //! @param[in]  offset  where to start search
    //! @return 1 beyond the end of the current token or npos if token not found
    //! Example
    //! @code
    //!     // Read lines from a string, where each line is delimited by \n. The last line need not have a trailing \n.
    //!     size_t offset = 0;
    //!     Utf8String m;
    //!     while ((offset = m_virtuals.GetNextToken (m, "\n", offset)) != Utf8String::npos)
    //!         {
    //!         printf ("%s\n", m.c_str());
    //!         }
    //! @endcode
    //! @note If this string ends with a delimiter, then the last token returned is the one before the trailing delimiter.
    //! If this string does not end with a delimiter, then the last token is everything following the last delimiter (if any) and the end of this string.
    //! If this string has no delimiters at all, then the first and only token returned is the string itself.
    //! So, for example, if this string were "abc\n" then GetNextToken (next, "\n", 0) would set next to "abc" and return 4, and GetNextToken (next, "\n", 4) would return npos.
    //! If this string were "abc", the two calls would return the same results.
    size_t GetNextToken (Utf8StringR next, CharCP delims, size_t offset) const
        {
        size_t idelim = find_first_of (delims, offset);
        if (idelim == npos)
            {
            if (offset >= length())
                {
                next.clear();
                return npos;
                }
            idelim = length();  // there's no trailing delimiter, so the last token is just the rest of the string.
            }
        next = substr (offset, idelim-offset);
        return idelim+1;
        }
    };

//=======================================================================================
//! Construct a Utf8String by creating a formatted string.
//! @ingroup BeStringGroup
//  @bsiclass                                                   Keith.Bentley   11/11
//=======================================================================================
struct Utf8PrintfString : Utf8String
{
    BENTLEYDLL_EXPORT explicit Utf8PrintfString (Utf8CP format, ...);
    BENTLEYDLL_EXPORT Utf8PrintfString(Utf8CP format, va_list);
    operator Utf8CP(){return c_str();}
};

//! A bvector of WString objects, with allocations handled by the Bentley allocator (so it can be passed across DLLs targeting different C runtimes).
typedef bvector<WString> T_WStringVector;

typedef T_WStringVector*        T_WStringVectorP, &T_WStringVectorR;
typedef T_WStringVector const * T_WStringVectorCP;
typedef T_WStringVector const & T_WStringVectorCR;

//! A bvector of WString objects, with allocations handled by the Bentley allocator (so it can be passed across DLLs targeting different C runtimes).
typedef bvector<Utf8String> T_Utf8StringVector;

typedef T_Utf8StringVector*         T_Utf8StringVectorP, &T_Utf8StringVectorR;
typedef T_Utf8StringVector const*   T_Utf8StringVectorCP;
typedef T_Utf8StringVector const&   T_Utf8StringVectorCR;

END_BENTLEY_NAMESPACE
