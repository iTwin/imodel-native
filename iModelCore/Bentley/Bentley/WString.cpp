/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    #include <Windows.h>
    #include <objbase.h>
#elif defined (__unix__)
    // TBD
#else
    #error unknown compiler
#endif
#include <exception>

#include "BentleyInternal.h"
#include <Bentley/WString.h>
#include <Bentley/BeAssert.h>

USING_NAMESPACE_BENTLEY

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t WString::ReplaceAll (WCharCP subStringToReplace, WCharCP replacement)
    {
    if (WString::IsNullOrEmpty(subStringToReplace) || NULL == replacement)
        return 0;

    size_t  currPos = 0;
    size_t  numReplaced = 0;
    size_t  oldSubLen = wcslen (subStringToReplace);
    size_t  newSubLen = wcslen (replacement);

    while (WString::npos != (currPos = find (subStringToReplace, currPos)))
        {
        replace (currPos, oldSubLen, replacement);
        currPos += newSubLen;

        ++numReplaced;
        }

    return numReplaced;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Utf8String::ReplaceAll (Utf8CP subStringToReplace, Utf8CP replacement)
    {
    if (Utf8String::IsNullOrEmpty(subStringToReplace) || NULL == replacement)
        return 0;

    size_t  currPos = 0;
    size_t  numReplaced = 0;
    size_t  oldSubLen = strlen (subStringToReplace);
    size_t  newSubLen = strlen (replacement);

    while (Utf8String::npos != (currPos = find (subStringToReplace, currPos)))
        {
        replace (currPos, oldSubLen, replacement);
        currPos += newSubLen;

        ++numReplaced;
        }

    return numReplaced;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
WString::WString (CharCP in, bool useUtf8) : bwstring()
    {
    if (useUtf8)
        AssignUtf8 (in);
    else // encoding == BentleyCharEncoding::Locale
        AssignA (in);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
WString::WString (CharCP in, BentleyCharEncoding encoding) : bwstring()
    {
    if (encoding == BentleyCharEncoding::Utf8)
        AssignUtf8 (in);
    else // encoding == BentleyCharEncoding::Locale
        AssignA (in);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
WStringR WString::AssignA (CharCP in)
    {
    if (NULL == in)
        {
        clear();
        return *this;
        }

    BeStringUtilities::CurrentLocaleCharToWChar (*this, in);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
WStringR WString::AssignUtf8 (Utf8CP in)
    {
    if (NULL == in)
        clear();
    else
        BeStringUtilities::Utf8ToWChar (*this, in);

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
WStringR WString::AssignUtf16 (Utf16CP in)
    {
    if (NULL == in)
        clear();
    else
        BeStringUtilities::Utf16ToWChar (*this, in);

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
WStringR WString::Trim ()
    {
    if (empty ())
        return *this;

    size_t firstNonSpaceIdx = 0;
    for (; (firstNonSpaceIdx < length ()) && iswspace (at (firstNonSpaceIdx)); ++firstNonSpaceIdx)
        ;

    size_t lastNonSpaceIdx = length () - 1;
    for (; (lastNonSpaceIdx > firstNonSpaceIdx) && iswspace (at (lastNonSpaceIdx)); --lastNonSpaceIdx)
        ;

    erase ((begin () + lastNonSpaceIdx + 1), end ());
    erase (begin (), (begin () + firstNonSpaceIdx));
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
WStringR WString::Trim (WCharCP trimCharacters)
    {
    if (empty ())
        return *this;

    size_t firstNonTrimIdx = 0;
    for (; (firstNonTrimIdx < length ()) && (NULL != wcschr (trimCharacters, at (firstNonTrimIdx))); ++firstNonTrimIdx)
        ;

    size_t lastNonTrimIdx = length () - 1;
    for (; (lastNonTrimIdx > firstNonTrimIdx) && (NULL != wcschr (trimCharacters, at (lastNonTrimIdx))); --lastNonTrimIdx)
        ;

    erase ((begin() + lastNonTrimIdx + 1), end ());
    erase (begin(), (begin () + firstNonTrimIdx));
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
WStringR WString::PadLeft (size_t totalSize, value_type charToPadWith)
    {
    if (length () >= totalSize)
        return *this;

    insert (begin (), (totalSize - length ()), charToPadWith);
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
WStringR WString::PadRight (size_t totalSize, value_type charToPadWith)
    {
    if (length () >= totalSize)
        return *this;

    insert (end (), (totalSize - length ()), charToPadWith);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool WString::Contains (WStringCR other) const
    {
    return Contains (other.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool WString::Contains (WCharCP other) const
    {
    size_type position = this->find (other);
    return (WString::npos != position);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool WString::ContainsI (WStringCR other) const
    {
    return ContainsI(other.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool WString::ContainsI (WCharCP other) const
    {
    WString thisLower (this->c_str());
    thisLower.ToLower();
    WString otherLower (other);
    otherLower.ToLower();
    return thisLower.Contains (otherLower.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t WString::FindI (WCharCP other) const
    {
    WString thisLower (this->c_str());
    thisLower.ToLower();
    WString otherLower (other);
    otherLower.ToLower();
    return thisLower.find (otherLower.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool WString::ReplaceI (WCharCP findString, WCharCP replaceString)
    {
    if (NULL == findString || NULL == replaceString)
        return false;

    size_t pos = FindI (findString);

    if (std::string::npos == pos)
        return false;

    replace (pos, wcslen(findString), replaceString);
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool WString::StartsWith(WCharCP value) const
    {
    if (WString::IsNullOrEmpty(value))
        return false;

    size_t valueLen = wcslen(value);
    if (valueLen > size())
        return false;

    return (0 == memcmp(c_str(), value, sizeof(WChar) * valueLen));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool WString::StartsWithI (WCharCP value) const
    {
    if (WString::IsNullOrEmpty(value))
        return false;

    size_t valueLen = wcslen(value);
    if (valueLen > size())
        return false;

    WString temp = this->substr (0, valueLen);
    return (0 == temp.CompareToI (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool WString::EndsWith (WCharCP value) const
    {
    if (WString::IsNullOrEmpty(value))
        return false;

    size_t valueLen = wcslen(value);
    if (valueLen > size())
        return false;

    WString temp = this->substr (size()-valueLen);
    return (0 == temp.CompareTo (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool WString::EndsWithI (WCharCP value) const
    {
    if (WString::IsNullOrEmpty(value))
        return false;

    size_t valueLen = wcslen(value);
    if (valueLen > size())
        return false;

    WString temp = this->substr (size()-valueLen);
    return (0 == temp.CompareToI (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool Utf8String::IsAscii () const
    {
    for (Utf8CP p = c_str(); 0 != *p; ++p)
        {
        if (0 != (*p & 0x80))
            return false;
        }
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool Utf8String::EndsWith(Utf8StringCR ending) const
    {
    auto thisLength = length();
    auto endingLength = ending.length ();

    if (thisLength < endingLength)
        {
        return false;
        }

    return (0 == compare(thisLength - endingLength, endingLength, ending));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8StringR Utf8String::Trim ()
    {
    if (empty ())
        return *this;

    size_t firstNonSpaceIdx = 0;
    for (; (firstNonSpaceIdx < length ()) && isspace (at (firstNonSpaceIdx)); ++firstNonSpaceIdx)
        ;

    size_t lastNonSpaceIdx = length () - 1;
    for (; (lastNonSpaceIdx > firstNonSpaceIdx) && isspace (at (lastNonSpaceIdx)); --lastNonSpaceIdx)
        ;

    erase ((begin () + lastNonSpaceIdx + 1), end ());
    erase (begin (), (begin () + firstNonSpaceIdx));
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8StringR Utf8String::TrimEnd()
    {
    if (empty())
        return *this;

    size_t nonSpaceCharacterCount = length();
    for (; (nonSpaceCharacterCount > 0) && isspace(at(nonSpaceCharacterCount - 1)); --nonSpaceCharacterCount)
        ;

    erase((begin() + nonSpaceCharacterCount), end());
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8StringR Utf8String::Trim (Utf8CP trimCharacters)
    {
    if (empty ())
        return *this;

    size_t firstNonTrimIdx = 0;
    for (; (firstNonTrimIdx < length ()) && (NULL != strchr (trimCharacters, at (firstNonTrimIdx))); ++firstNonTrimIdx)
        ;

    size_t lastNonTrimIdx = length () - 1;
    for (; (lastNonTrimIdx > firstNonTrimIdx) && (NULL != strchr (trimCharacters, at (lastNonTrimIdx))); --lastNonTrimIdx)
        ;

    erase ((begin () + lastNonTrimIdx + 1), end ());
    erase (begin (), (begin () + firstNonTrimIdx));
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AString::AString (WCharCP source)
    {
    BeStringUtilities::WCharToCurrentLocaleChar (*this, source);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8StringR Utf8String::ToUpper()
    {
    if (IsAscii())
        {
        std::transform(begin(), end(), begin(), [](char const& c){return (char)toupper(c);});
        return *this;
        }

    WString wstr(c_str(), BentleyCharEncoding::Utf8);
    wstr.ToUpper();
    clear();
    BeStringUtilities::WCharToUtf8(*this, wstr.c_str());
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool Utf8String::Contains(Utf8StringCR other) const
    {
    return Contains(other.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool Utf8String::Contains(Utf8CP other) const
    {
    size_type position = this->find(other);
    return (Utf8String::npos != position);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool Utf8String::ContainsI(Utf8StringCR other) const
    {
    return ContainsI(other.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool Utf8String::ContainsI(Utf8CP other) const
    {
    Utf8String thisLower(this->c_str());
    thisLower.ToLower();
    Utf8String otherLower(other);
    otherLower.ToLower();
    return thisLower.Contains(otherLower.c_str());
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool Utf8String::StartsWith(Utf8CP value) const
    {
    if (Utf8String::IsNullOrEmpty(value))
        return false;

    size_t valueLen = strlen(value);
    if (valueLen > size())
        return false;

    Utf8String temp(this->substr (0, valueLen));
    return (0 == temp.CompareTo (value));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool Utf8String::StartsWithI (Utf8CP value) const
    {
    if (Utf8String::IsNullOrEmpty(value))
        return false;

    size_t valueLen = strlen(value);
    if (valueLen > size())
        return false;

    Utf8String temp(this->substr (0, valueLen));
    return (0 == temp.CompareToI (value));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool Utf8String::StartsWithIAscii(Utf8CP value) const
    {
    if (Utf8String::IsNullOrEmpty(value))
        return false;

    size_t valueLen = strlen(value);
    if (valueLen > size())
        return false;

    Utf8String temp(this->substr(0, valueLen));
    return (0 == temp.CompareToIAscii(value));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool Utf8String::EndsWith (Utf8CP value) const
    {
    if (Utf8String::IsNullOrEmpty(value))
        return false;

    size_t valueLen = strlen(value);
    if (valueLen > size())
        return false;

    Utf8String temp = this->substr (size()-valueLen);
    return (0 == temp.CompareTo (value));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool Utf8String::EndsWithI (Utf8CP value) const
    {
    if (Utf8String::IsNullOrEmpty(value))
        return false;

    size_t valueLen = strlen(value);
    if (valueLen > size())
        return false;

    Utf8String temp = this->substr (size()-valueLen);
    return (0 == temp.CompareToI (value));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool Utf8String::EndsWithIAscii(Utf8CP value) const
    {
    if (Utf8String::IsNullOrEmpty(value))
        return false;

    size_t valueLen = strlen(value);
    if (valueLen > size())
        return false;

    Utf8String temp = this->substr(size() - valueLen);
    return (0 == temp.CompareToIAscii(value));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String::Utf8String(Utf8PrintfString&& other) : bastring(std::move(other))
    {
    }
