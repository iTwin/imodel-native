/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/Tools/varichar.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

typedef char            VariChar;                   //WIP_CHAR_OK
typedef VariChar const* VariCharCP;
typedef VariChar*       VariCharP;

#include <Bentley/Bentley.h>
#include <DgnPlatform/ExportMacros.h>
#include <Bentley/CodePages.h>

BEGIN_BENTLEY_API_NAMESPACE

//=======================================================================================
//! VariChar is a Bentley-specific way to encode string data. It should be considered deprecated, but is still used in the DGN V8 file format.
//! Any new persisted string data should use UTF8-LE. See BeStringUtilities for related conversion functions.
//! The limited functionality that remains in this class (vs. the old mdlVariChar_* functions) is only intended for reading/writing persisted data to/from Unicode.
//! All string operations should then occur on the Unicode string.
//! A "Unicode" VariChar uses UTF16-LE; the WChar-based arguments are platform-dependent Unicode.
// @bsiclass                                                    Jeff.Marker     12/2011
//=======================================================================================
struct VariCharConverter : public NonCopyableClass
    {
    //=======================================================================================
    // Result of VariCharToUnicode.
    // @bsiclass                                                    Jeff.Marker     09/2014
    //=======================================================================================
    enum struct Result
        {
        Success,
        Truncated,
        Error
        }; // Result
        
private:
    // Static class; don't allow instances.
    VariCharConverter () {}

public:
    //! Converts Unicode string into a VariChar buffer.
    //! If codePage is 0 or 1200 (LangCodePage::Unicode in DgnPlatform), the VariChar uses UTF-16LE; otherwise it is locale-encoded. The provided Unicode buffer must always be NULL terminated.
    //! @warning Do not use this when the VariChar must be in the encoding of a DgnFont. DgnFonts have special rules for locale conversion. Use DgnFont::UnicodeStringToFontChars and VariCharConverter::FontCharToVariChar.
    //! @warning If creating a Unicode VariChar, the first character cannot be character code U+0001.
    DGNPLATFORM_EXPORT static BentleyStatus UnicodeToVariChar(bvector<VariChar>&, WCharCP, LangCodePage codePage, bool shouldNullTerminate);

    //! Converts Unicode string into a VariChar buffer.
    //! If codePage is 0 or 1200 (LangCodePage::Unicode in DgnPlatform), the VariChar uses UTF-16LE; otherwise it is locale-encoded. The provided Unicode buffer must always be NULL terminated.
    //! @warning Do not use this when the VariChar must be in the encoding of a DgnFont. DgnFonts have special rules for locale conversion. Use DgnFont::UnicodeStringToFontChars and VariCharConverter::FontCharToVariChar.
    //! @warning If creating a Unicode VariChar, the first character cannot be character code U+0001.
    DGNPLATFORM_EXPORT static BentleyStatus UnicodeToVariChar(bvector<VariChar>&, Utf8CP, LangCodePage codePage, bool shouldNullTerminate);
    
    //! Fills a VariChar buffer from a FontChar buffer. No transcoding takes place. Indicating Unicode controls the internal flags of the resulting VariChar.
    //! This is only meant to be used with the output of DgnFont::UnicodeStringToFontChars.
    //! The FontChar buffer must be NULL-terminated, and numFontChars cannot be 0.
    DGNPLATFORM_EXPORT static BentleyStatus FontCharToVariChar(bvector<VariChar>&, uint16_t const* fontChars, size_t numFontChars, bool isUnicode, bool shouldNullTerminate);

    //! Converts a VariChar buffer into a Unicode string.
    //! If you know that your VariChar is Unicode, you can pass 0 or 1200 (LangCodePage::Unicode in DgnPlatform) for codePage.
    //! numVariBytes cannot be 0; if you have a NULL-terminated VariChar and do not already know its size, use ComputeNumBytes.
    //! As with all WString objects, it will be NULL-terminated. The VariChar string does not have to be.
    //! @warning Do not use this when the VariChar was in the encoding of a DgnFont. DgnFonts have special rules for locale conversion. Use VariCharConverter::VariCharToFontChar and DgnFont::FontCharsToUnicodeString.
    DGNPLATFORM_EXPORT static BentleyStatus VariCharToUnicode(WStringR, VariCharCP, size_t numVariBytes, LangCodePage codePage);
    
    //! Converts a VariChar buffer into a Unicode string.
    //! If you know that your VariChar is Unicode, you can pass 0 or 1200 (LangCodePage::Unicode in DgnPlatform) for codePage.
    //! numVariBytes cannot be 0; if you have a NULL-terminated VariChar and do not already know its size, use ComputeNumBytes.
    //! numUnicodeChars is expressed in characters (not bytes), including the NULL terminator (e.g. a value of 1 necessarily results in an empty string because the NULL terminator will take the entire buffer).
    //! @note The resulting Unicode buffer will always be NULL terminated, even if the string must then be truncated.
    //! @note The upper bound for the Unicode buffer count is (1 + (2 * numVariBytes)). If you need bytes, then scale by sizeof(WChar).
    //!         We purposefully do not allow passing 0 for numUnicodeChars and giving you a precise answer, because we must do all the actual work to compute this length anyway. It is better to over-allocate than to do all the expensive work twice.
    //! @warning Do not use this when the VariChar was in the encoding of a DgnFont. DgnFonts have special rules for locale conversion. Use VariCharConverter::VariCharToFontChar and DgnFont::FontCharsToUnicodeString.
    DGNPLATFORM_EXPORT static Result VariCharToUnicode(WCharP, size_t numUnicodeChars, VariCharCP, size_t numVariBytes, LangCodePage codePage);

    //! Converts a VariChar buffer into a Unicode string.
    //! If you know that your VariChar is Unicode, you can pass 0 or 1200 (LangCodePage::Unicode in DgnPlatform) for codePage.
    //! numVariBytes cannot be 0; if you have a NULL-terminated VariChar and do not already know its size, use ComputeNumBytes.
    //! As with all WString objects, it will be NULL-terminated. The VariChar string does not have to be.
    //! @warning Do not use this when the VariChar was in the encoding of a DgnFont. DgnFonts have special rules for locale conversion. Use VariCharConverter::VariCharToFontChar and DgnFont::FontCharsToUnicodeString.
    DGNPLATFORM_EXPORT static BentleyStatus VariCharToUnicode(Utf8StringR, VariCharCP, size_t numVariBytes, LangCodePage codePage);
    
    //! Fills a FontChar buffer from a VariChar buffer. No transcoding takes place.
    //! This is only meant to be used as input to DgnFont::FontCharsToUnicodeString.
    //! The resulting FontChar buffer is NULL-terminated.
    //! numVariBytes cannot be 0; if you have a NULL-terminated VariChar and do not already know its size, use ComputeNumBytes.
    DGNPLATFORM_EXPORT static BentleyStatus VariCharToFontChar(bvector<uint16_t>& fontChars, VariCharCP, size_t numVariBytes);

    //! Assuming that the VariChar string is NULL-terminated, computes the number of bytes it uses, including the inducers and terminating NULL.
    DGNPLATFORM_EXPORT static size_t ComputeNumBytes(VariCharCP);
    };

END_BENTLEY_API_NAMESPACE
