/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnFontManager.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "../DgnPlatform.h"
#include "TextParam.h"
#include <Bentley/CodePages.h>

//! @defgroup DgnFontModule DGN Platform Font System
//! The role of the font system is to provide access to font objects, which contain properties and metadata for RSC, SHX, and TrueType fonts. Font objects are useful when dealing with text-related APIs to control the presentation of the characters. The most important classes are DgnFontManager and DgnFont. Each instance of a DgnPlatformLib::Host has a single font manager; see DgnPlatformLib::Host::GetDgnFontManager. Font information is accessed via instances of DgnFont, which are typically obtained from the font manager. Fonts can be located on the system (e.g. managed by the operating system), in a workspace (e.g. through search paths), or embedded in a project. You can perform the same operations regardless of the font's source, and most operations can be performed on any font type; some specialized operations exist on DgnRscFont, DgnShxFont, or DgnTrueTypeFont. See DgnPlatformLib::Host::FontAdmin for ways to control the font manager's configuration and search paths.
//!
//! <b>Enumerating Fonts</b>
//!
//! DgnFontManager and DgnProject can be used to enumerate or find fonts. Project objects only know about their embedded fonts; the font manager knows about system and workspace fonts, and can optionally query a project (taken as a CP to many methods).
//!
//! <b>Font Numbers</b>
//!
//! While the vast majority of APIs will work with DgnFont objects, you may still encounter font numbers. These are granted on a per-project basis. The first time a font is used by an element, it is granted a number. There is no way to directly correlate any information about a font from its number (you must always query a sepcific instance of a project to resolve the number into a font object), and this number has no meaning outside of the given project. You can use DgnProject::FindFont to resolve a font number to a DgnFont object and vice versa (DgnProject::FindFontNumber and DgnProject::AcquireFontNumber).
//!
//! <b>Font Types</b>
//!
//! DGN platform supports three font types (DgnFontType): MicroStation resource (RSC), AutoCAD SHX, and TrueType. A font is uniquely identified by its name and type. If there are duplicates between a project and the environment (e.g. embedded vs. workspace and system), the embedded version gets priority. Within the environment, the first copy found gets priority.
//!
//! While "type" is used at a higher level to essentially separate the format of the font, the SHX format has three sub-types; "variant" (DgnFontVariant) is used to distinguish at this level. Because certain sub-types are only valid in certain scenarios, you sometimes need to distinguish the variant, not just the type. For example, it is invalid to specify an SHX big font for the base font of TextStringProperties. If you are requesting a font and specify DGNFONTVARIANT_DontCare, the precedence is as follows: RSC, SHX (other than DGNFONTVARIANT_ShxBig), then TrueType; note the absence of DGNFONTVARIANT_ShxBig in the precedence order.
//!
//! @note Future development and enhancements will be focused on TrueType fonts. RSC and SHX fonts will only continue to be supported in the interest of backwards compatibility and interoperability.
//!
//! <b>Notable Methods</b>
//!
//! <ul>
//! <li>Find a font by-name: DgnFontManager::FindFont</li>
//! <li>Get a list of available fonts: DgnFontManager::GetFonts</li>
//! <li>Get a list of embedded fonts: DgnProject::EmbeddedFonts</li>
//! <li>Get a font's name and type: DgnFont::GetName and DgnFont::GetType</li>
//! </ul>

//! @addtogroup DgnFontModule
//! @{

DGNPLATFORM_TYPEDEFS(DgnFont)
DGNPLATFORM_TYPEDEFS(DgnFontKey)
DGNPLATFORM_TYPEDEFS(DgnGlyph)

DGNPLATFORM_REF_COUNTED_PTR(DgnFont)

// __PUBLISH_SECTION_END__

DGNPLATFORM_TYPEDEFS(DgnGlyphLayoutContext)
DGNPLATFORM_TYPEDEFS(DgnGlyphLayoutResult)

// __PUBLISH_SECTION_START__

//! @}

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup DgnFontModule
//! @{

//=======================================================================================
//! The types of fonts supported by DGN Platform. All fonts are unique by name and type.
// @bsiclass                                                      Jeff.Marker     11/2010
//=======================================================================================
enum class DgnFontType
{
    None       = 0,
    Rsc        = 1,
    TrueType   = 2,
    Shx        = 3,
};

//=======================================================================================
//! A more descriptive form of DgnFontType, useful in some specific scenarios. This primarily deals with the different kinds of SHX fonts.
// @bsiclass                                                      Jeff.Marker     07/2012
//=======================================================================================
enum DgnFontVariant
    {
    DGNFONTVARIANT_Invalid = 0, //!< An invalid font variant; used in error conditions.
    DGNFONTVARIANT_DontCare = 0, //!< When asking for a font, use a system-defined precedence, ignoring SHX big fonts.
    DGNFONTVARIANT_Rsc = (1 << 0), //!< MicroStation RSC font
    DGNFONTVARIANT_TrueType = (1 << 1), //!< TrueType font
    DGNFONTVARIANT_ShxShape = (1 << 2), //!< AutoCAD SHX shape font
    DGNFONTVARIANT_ShxUni = (1 << 3), //!< AutoCAD SHX unicode font
    DGNFONTVARIANT_ShxBig = (1 << 4) //!< AutoCAD SHX big font
    };

//=======================================================================================
//! Code points for special characters.
// @bsiclass                                                      Jeff.Marker     07/2012
//=======================================================================================
enum SpecialCharValues
    {
    SPECIALCHAR_UnicodeDegree = 0x00b0,
    SPECIALCHAR_UnicodePlusMinus = 0x00b1,
    
    // This is all horrible, but is required for backwards compatibility.
    // We need these because multiple Unicode code points look like a diameter, but not all fonts map all of the code points to that glyph.
    // u2205 (empty set) is typically seen in DWG files, and is what %%C should map to.
    // u2300 (diameter) is the real diameter symbol, and what every single instance of this character should have been using all along...
    // u00d8 (capital O with stroke) is another character that looks like a diameter. We have recieved customer files where they have selected this character, seemingly because it's the first character in a symbol selector that looks like a dimaeter symbol...
    // u00f8 (lower O with stroke) is another character that looks like a diameter. I'm less sure about the background for this (probably similar to u00d8, however).
    SPECIALCHAR_UnicodeDiameter = 0x2205,
    SPECIALCHAR_UnicodeRealDiameter = 0x2300,
    SPECIALCHAR_UnicodeCapitalOWithStroke = 0x00d8,
    SPECIALCHAR_UnicodeSmallOWithStroke = 0x00f8,
    
    SPECIALCHAR_UnicodeFraction_1_4 = 0x00bc,
    SPECIALCHAR_UnicodeFraction_1_2 = 0x00bd,
    SPECIALCHAR_UnicodeFraction_3_4 = 0x00be,
    SPECIALCHAR_UnicodeFraction_1_7 = 0x2150,
    SPECIALCHAR_UnicodeFraction_1_9 = 0x2151,
    SPECIALCHAR_UnicodeFraction_1_10 = 0x2152,
    SPECIALCHAR_UnicodeFraction_1_3 = 0x2153,
    SPECIALCHAR_UnicodeFraction_2_3 = 0x2154,
    SPECIALCHAR_UnicodeFraction_1_5 = 0x2155,
    SPECIALCHAR_UnicodeFraction_2_5 = 0x2156,
    SPECIALCHAR_UnicodeFraction_3_5 = 0x2157,
    SPECIALCHAR_UnicodeFraction_4_5 = 0x2158,
    SPECIALCHAR_UnicodeFraction_1_6 = 0x2159,
    SPECIALCHAR_UnicodeFraction_5_6 = 0x215a,
    SPECIALCHAR_UnicodeFraction_1_8 = 0x215b,
    SPECIALCHAR_UnicodeFraction_3_8 = 0x215c,
    SPECIALCHAR_UnicodeFraction_5_8 = 0x215d,
    SPECIALCHAR_UnicodeFraction_7_8 = 0x215e,

    SPECIALCHAR_FirstStandardRscFraction = 0x81, // While technically customizable per-font, I have never seen this in practice, and the font utility no longer allows it.
    SPECIALCHAR_LastStandardRscFraction = 0xbf, // While technically customizable per-font, I have never seen this in practice, and the font utility no longer allows it.

// __PUBLISH_SECTION_END__
    // Private use allows us to round-trip characters with custom meaning. It is from 0xe000 through 0xf8ff.
    SPECIALCHAR_PrivateUse_Degree_Unicode = 0xe000,
    SPECIALCHAR_PrivateUse_PlusMinus_Unicode = 0xe001,
    SPECIALCHAR_PrivateUse_Diameter_Unicode = 0xe002,
    SPECIALCHAR_PrivateUse_FirstRscFraction = 0xe100,
    SPECIALCHAR_PrivateUse_LastRscFraction = (SPECIALCHAR_PrivateUse_FirstRscFraction + (SPECIALCHAR_LastStandardRscFraction - SPECIALCHAR_FirstStandardRscFraction))
// __PUBLISH_SECTION_START__
    }; // SpecialCharValues

// __PUBLISH_SECTION_END__

struct IDgnGlyphLayoutListener;

// FontChar is most aptly described as a glyph index. It is only intended for quick lookup for glyphs in a font, and should remain an internal concept.
typedef UInt16 FontChar;
typedef UInt16* FontCharP;
typedef UInt16 const* FontCharCP;
typedef bvector<FontChar> T_FontChars;

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     11/2010
//=======================================================================================
enum DgnGlyphRunLayoutFlags
    {
    GLYPH_RUN_LAYOUT_FLAG_None = 0,
    GLYPH_RUN_LAYOUT_FLAG_Vertical =(1 << 0),
    GLYPH_RUN_LAYOUT_FLAG_Backwards =(1 << 1),
    GLYPH_RUN_LAYOUT_FLAG_UpsideDown =(1 << 2),
    GLYPH_RUN_LAYOUT_FLAG_IsForEditing =(1 << 3)
    };

//=======================================================================================
//! This is the input container for DgnFont::LayoutGlyphs.
// @bsiclass                                                      Jeff.Marker     12/2009
//=======================================================================================
struct DgnGlyphLayoutContext
    {
    typedef T_FontChars const& T_FontCharsCR;
    typedef bvector<bool> T_EdfMask;
    typedef T_EdfMask const& T_EdfMaskCR;

private:
    CharacterSpacingType m_characterSpacingType;
    double m_characterSpacingValue;
    DPoint3d m_displayOffset;
    DPoint2d m_displaySize;
    DgnFontCP m_font;
    DgnFontCP m_shxBigFont;
    bool m_isBold;
    bool m_shouldUseItalicTypeface;

    DgnGlyphRunLayoutFlags m_runLayoutFlags;
    T_FontChars m_fontChars;
    T_EdfMask m_edfMask;
    IDgnGlyphLayoutListener* m_glyphLayoutListener;
    bool m_shouldIgnoreDisplayShifts;
    bool m_shouldIgnorePercentEscapes;
    bool m_shouldIgnoreLsb;

public:
    CharacterSpacingType GetCharacterSpacingType() const;
    void SetCharacterSpacingType(CharacterSpacingType);
    double GetCharacterSpacingValue() const;
    void SetCharacterSpacingValue(double);
    DPoint3d GetDisplayOffset() const;
    void SetDisplayOffset(DPoint3d);
    DPoint2d GetDisplaySize() const;
    void SetDisplaySize(DPoint2d);
    DgnFontCR GetFont() const;
    DgnFontCP GetShxBigFont() const;
    bool IsBold() const;
    void SetIsBold(bool);
    bool ShouldUseItalicTypeface() const;
    void SetShouldUseItalicTypeface(bool);
    DgnGlyphRunLayoutFlags GetRunLayoutFlags() const;
    bool IsVertical() const;
    bool IsBackwards() const;
    bool IsUpsideDown() const;
    void SetRunLayoutFlags(DgnGlyphRunLayoutFlags);
    T_FontCharsCR GetFontChars() const;
    void SetFontChars(T_FontChars::value_type const*, T_EdfMask::value_type const*, size_t count);
    void SetFontChars(WCharCP, T_EdfMask::value_type const*, size_t count);
    void SetFontChars(Utf8CP, size_t count);
    T_EdfMaskCR GetEDFMask() const;
    IDgnGlyphLayoutListener* GetIDgnGlyphLayoutListenerP() const;
    void SetIDgnGlyphLayoutListenerP(IDgnGlyphLayoutListener*);
    bool ShouldIgnoreDisplayShifts() const;
    void SetShouldIgnoreDisplayShifts(bool);
    bool ShouldIgnorePercentEscapes() const;
    void SetShouldIgnorePercentEscapes(bool);
    bool ShouldIgnoreLsb() const;
    void SetShouldIgnoreLsb(bool);

    DgnGlyphLayoutContext(DgnFontCR font, DgnFontCP shxBixFont);

    void SetPropertiesFromRunPropertiesBase(RunPropertiesBaseCR);

    }; // DgnGlyphLayoutContext

//=======================================================================================
//! This is the output container for DgnFont::LayoutGlyphs.
// @bsiclass                                                      Jeff.Marker     12/2009
//=======================================================================================
struct DgnGlyphLayoutResult
    {
    typedef bvector<FontChar> T_GlyphCodes;
    typedef T_GlyphCodes& T_GlyphCodesR;
    typedef bvector<DPoint3d> T_GlyphOrigins;
    typedef T_GlyphOrigins& T_GlyphOriginsR;

private:
    T_GlyphCodes m_glyphCodes;
    T_GlyphOrigins m_glyphOrigins;
    T_DoubleVector m_leadingCaretOffsets;
    T_DoubleVector m_trailingCaretOffsets;
    DRange2d m_cellBoxRange;
    DRange2d m_blackBoxRange;
    DRange2d m_justificationCellBoxRange;
    DRange2d m_justificationBlackBoxRange;
    double m_trailingInterCharacterSpacing;
    double m_maxHorizontalCellIncrement;
    bool m_isLastGlyphBlank;
    double m_leftSideBearingToIgnore;

public:
    //! The glyph codes resolved from the input string. This potentially includes Windows API- or plug-in-based remapping.
    DGNPLATFORM_EXPORT T_GlyphCodesR GetGlyphCodesR();

    //! Origin for each glyph.
    DGNPLATFORM_EXPORT T_GlyphOriginsR GetGlyphOriginsR();

    //! Leading caret offsets for each logical character in the input(can differ from the glyph codes recorded in this structure).
    DGNPLATFORM_EXPORT T_DoubleVectorR GetLeadingCaretOffsetsR();

    //! Trailing caret offsets for each logical character in the input(can differ from the glyph codes recorded in this structure).
    DGNPLATFORM_EXPORT T_DoubleVectorR GetTrailingCaretOffsetsR();

    //! The union of all glyph cell boxes. The cell box height is defined by the run properties, and its width is defined by the font. This includes all bearings.
    DGNPLATFORM_EXPORT DRange2dR GetCellBoxRangeR();

    //! The union of all glyph black boxes. The black box is defined by the font, and includes no bearings.
    DGNPLATFORM_EXPORT DRange2dR GetBlackBoxRangeR();

    //! The union of glyph cell boxes, excluding trailing blank glyphs. The cell box height is defined by the run properties, and its width is defined by the font. This includes all bearings.
    DGNPLATFORM_EXPORT DRange2dR GetJustificationCellBoxRangeR();
    
    //! The union of glyph black boxes, excluding trailing blank glyphs (except if they're part of an EDF). The black box is defined by the font, and includes no bearings.
    DGNPLATFORM_EXPORT DRange2dR GetJustificationBlackBoxRangeR();

    //! The distance from the last glyph's cell box to the theoretical next glyph's origin(axis determined by orientation provided to DgnGlyphLayoutContext).
    DGNPLATFORM_EXPORT double& GetTrailingInterCharacterSpacingR();

    //! The maximum horizontal cell increment among all of the glyphs. Maximum horizontal cell increment is an evil derived value, computed by and for legacy reasons.
    DGNPLATFORM_EXPORT double& GetMaxHorizontalCellIncrementR();

    //! True if the very last glyph is blank; used for computing element range(another evil legacy behavior).
    DGNPLATFORM_EXPORT bool& IsLastGlyphBlankR();

    //! The font- and orientation-dependent left-side bearing to ignore if the run properties say to do so.
    DGNPLATFORM_EXPORT double& LeftSideBearingToIgnoreR();

    //! Creates a new instance with zero'ed values.
    DGNPLATFORM_EXPORT DgnGlyphLayoutResult();

    //! Based on evil legacy rules and the internal computed values, computes element range as if the original string were for a text element.
    DGNPLATFORM_EXPORT DRange2d ComputeElementRange(DgnGlyphLayoutContextCR) const;

    }; // DgnGlyphLayoutResult

// __PUBLISH_SECTION_START__

//=======================================================================================
//! Represents a glyph in a DgnFont.
// @bsiclass                                                    Jeff.Marker     07/2012
//=======================================================================================
struct DgnGlyph
    {
// __PUBLISH_SECTION_END__

protected:
    FontChar m_charCode;
    DPoint2d m_blackBoxStart;
    DPoint2d m_blackBoxEnd;
    DPoint2d m_cellBoxStart;
    DPoint2d m_cellBoxEnd;

    virtual BentleyStatus _FillGpa(GPArrayR) const = 0;
    virtual DgnFontType _GetType() const = 0;
    virtual bool _IsBlank() const = 0;
    DgnGlyph(FontChar);

public:
    virtual ~DgnGlyph(){}
    double GetBlackBoxBottom() const;
    double GetBlackBoxHeight() const;
    DGNPLATFORM_EXPORT double GetBlackBoxLeft() const;
    DGNPLATFORM_EXPORT double GetBlackBoxRight() const;
    double GetBlackBoxWidth() const;
    double GetCellBoxHeight() const;
    double GetCellBoxRight() const;
    double GetCellBoxWidth() const;
    DGNPLATFORM_EXPORT FontChar GetCharCode() const;
    DGNPLATFORM_EXPORT DgnFontType GetType() const;
    DGNPLATFORM_EXPORT bool IsBlank() const;

// __PUBLISH_CLASS_VIRTUAL__
// __PUBLISH_SECTION_START__

public:

    //! Fills a GPA with the geometry of this glyph.
    DGNPLATFORM_EXPORT BentleyStatus FillGpa(GPArrayR) const;
    };

// __PUBLISH_SECTION_END__

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     07/2012
//=======================================================================================
struct DgnFontConfigurationData
    {
    LangCodePage m_codePage;
    FontChar m_degreeCharCode;
    FontChar m_diameterCharCode;
    FontChar m_plusMinusCharCode;
    bool m_shouldCreateShxUnifont;

    DgnFontConfigurationData();
    };

// __PUBLISH_SECTION_START__

//=======================================================================================
//! The font base class represents fonts in text-related APIs, and gives access to properties and metadata common to all font types. Font types include TrueType, RSC, and SHX, which can come from the system, workspace, or can be embedded in a project. You cannot create instances of this type directly; see DgnFontManager and DgnProject to access instances of this type, as well as the documentation for DgnFontModule for more background.
// @bsiclass                                                    Jeff.Marker     07/2012
//=======================================================================================
struct DgnFont : public RefCountedBase, public NonCopyableClass
    {
// __PUBLISH_SECTION_END__

protected:
    //=======================================================================================
    // @bsiclass                                                      Jeff.Marker     08/2012
    //=======================================================================================
    struct CharIter
    {
        //=======================================================================================
        // @bsiclass                                                      Jeff.Marker     08/2012
        //=======================================================================================
        enum CharType
            {
            CHARTYPE_Normal = 0,
            CHARTYPE_Special = 1,    //!< degree, diameter, plusminus
            CHARTYPE_Compose = 2,    //!< %%nnn

            }; // CharType

    private:
        FontChar m_charCode;
        CharType m_charType;
        FontCharCP m_currChar;
        FontCharCP m_startOfString;
        FontCharCP m_lastCharacter;
        DgnFontCR m_font;
        bool m_shouldIgnorePercentEscapes;

        void SetChar(FontChar charCode, CharType charType);

    public:
        DGNPLATFORM_EXPORT CharIter(DgnFontCR, FontCharCP, size_t nChars);
        DGNPLATFORM_EXPORT CharIter(DgnFontCR, FontCharCP, size_t nChars, bool shouldIgnorePercentEscapes);
        FontChar CurrCharCode();
        CharType CurrCharType();
        bool IsValid();
        DGNPLATFORM_EXPORT void ToNext();
    };

    static const double VERTICAL_TEXT_WIDTH_FACTOR;

    Utf8String m_name;
    mutable bool m_isMissing;
    bool m_isLastResort;

    virtual bool _ContainsCharacter(WChar) const = 0;
    virtual DgnFontPtr _Embed(DgnProjectR) const = 0;
    virtual LangCodePage _GetCodePage() const = 0;
    virtual FontChar _GetDegreeCharCode() const = 0;
    virtual double _GetDescenderRatio() const = 0;
    virtual FontChar _GetDiameterCharCode() const = 0;
    virtual FontChar _GetPlusMinusCharCode() const = 0;
    virtual DgnFontType _GetType() const = 0;
    virtual DgnFontVariant _GetVariant() const = 0;
    virtual bool _IsGlyphBlank(FontChar) const = 0;
    virtual BentleyStatus _LayoutGlyphs(DgnGlyphLayoutContextCR, DgnGlyphLayoutResultR) const = 0;
    virtual WChar _RemapFontCharToUnicodeChar(FontChar) const;
    virtual FontChar _RemapUnicodeCharToFontChar(WChar) const;
    virtual bool _ShouldDrawWithLineWeight() const = 0;
    DgnFont(Utf8CP name, DgnFontConfigurationData const&);

public:
    DGNPLATFORM_EXPORT bool ContainsCharacter(WChar) const;
    bool DoGlyphsHaveBlankGeometry(FontCharCP, size_t) const;
    DgnFontPtr Embed(DgnProjectR) const;
    DGNPLATFORM_EXPORT bool Equals(DgnFontCR) const;
    DGNPLATFORM_EXPORT WString FontCharsToUnicodeString(FontCharCP) const;
    DGNPLATFORM_EXPORT FontChar GetDegreeCharCode() const;
    DGNPLATFORM_EXPORT double GetDescenderRatio() const;
    DGNPLATFORM_EXPORT FontChar GetDiameterCharCode() const;
    DGNPLATFORM_EXPORT FontChar GetPlusMinusCharCode() const;
    DGNPLATFORM_EXPORT bool IsAccessibleByProject(DgnProjectCR) const;
    DGNPLATFORM_EXPORT BentleyStatus LayoutGlyphs(DgnGlyphLayoutContextCR, DgnGlyphLayoutResultR) const;
    DGNPLATFORM_EXPORT WChar RemapFontCharToUnicodeChar(FontChar) const;
    DGNPLATFORM_EXPORT FontChar RemapUnicodeCharToFontChar(WChar) const;
    DGNPLATFORM_EXPORT DgnFontCP ResolveToRenderFont() const;
    bool ShouldDrawWithLineWeight() const;
    DGNPLATFORM_EXPORT T_FontChars UnicodeStringToFontChars(WCharCP) const;

// __PUBLISH_CLASS_VIRTUAL__
// __PUBLISH_SECTION_START__
public:
    //! Compresses escape sequences(converts from an escape sequence to a character). Escape sequences are base-10 numeric character codes(in the locale of the font) preceded by a backslash. There are sometimes used in user input to allow the user to enter special characters.
    DGNPLATFORM_EXPORT void CompressEscapeSequences(WStringR outStr, WCharCP inStr) const;

    //! Expands escape sequences(prevents anything in this string from being interpreted as an escape sequence). This maintains parity with CompressEscapeSequences by doubling every backslash.
    DGNPLATFORM_EXPORT void ExpandEscapeSequences(WStringR outStr, WCharCP inStr) const;

    //! Gets the code page / locale of this font. Aside from giving you an idea of the characters in this font, this locale is sometimes used(vs. Unicode) when persisting text that uses this font to the file.
    DGNPLATFORM_EXPORT LangCodePage GetCodePage() const;

    //! Gets the name of this font.
    DGNPLATFORM_EXPORT Utf8StringCR GetName() const;

    //! Gets the type of this font. See DgnFontType and the documentation for DgnFontModule for more background.
    DGNPLATFORM_EXPORT DgnFontType GetType() const;

    //! Gets the variant of this font. See DgnFontVariant and the documentation for DgnFontModule for more background.
    DGNPLATFORM_EXPORT DgnFontVariant GetVariant() const;

    //! Tells whether this font is missing. A missing font has an entry in the font table, but cannot be found in the current session. Fallback fonts will be used to display anything that requires this font, and you should not create new elements that use missing fonts.
    DGNPLATFORM_EXPORT bool IsMissing() const;

    //! Tells whether this font is a last resort font. This means that the original nor the default font of a type could not be resolved, and this font is being provided to prevent the application from crashing.
    DGNPLATFORM_EXPORT bool IsLastResort() const;
    }; // DgnFont

//=======================================================================================
//! A font is uniquely identified by the font manager by its type and name. This structure wraps that concept for ease of use in collections.
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct DgnFontKey
    {
    DgnFontType m_type;
    Utf8String  m_name;

    DgnFontKey() {}
    //! Constructs a new instance with the given type and name.
    DgnFontKey(DgnFontType type, Utf8CP name) : m_type(type), m_name(name) {}
    DgnFontKey(DgnFontType type, Utf8String name) : m_type(type), m_name(name) {}
    bool operator< (DgnFontKey const& other) const {return (m_type != other.m_type) ? (m_type < other.m_type) : (m_name.CompareToI (other.m_name) < 0);}
    };

//! Collections of DgnFontCP.
typedef bvector<DgnFontCP> T_DgnFontCPs;

//! Mapping between DgnFontKey and DgnFontPtr.
typedef bmap<DgnFontKey, DgnFontPtr> T_FontCatalogMap;

// __PUBLISH_SECTION_END__
//! An agent that acquires fonts from an external source
struct IDgnFontFinder
    {
    virtual void       AcquireSystemFonts (T_FontCatalogMap&) = 0;
    virtual DgnFontPtr CreateLastResortFont () = 0;
    };
// __PUBLISH_SECTION_START__

//=======================================================================================
//! The font manager is the primary entry point into the font system.
//! It provides access to the system and workspace fonts, as well as default fonts(for when fonts cannot be found or you simply need a generic font).
// @bsiclass                                                    Jeff.Marker     07/2012
//=======================================================================================
struct DgnFontManager : public DgnHost::IHostObject
    {
// __PUBLISH_SECTION_END__

private:
    typedef bmap<DgnFontKey, DgnFontConfigurationData> T_PerFontConfigurationMap;

    T_PerFontConfigurationMap m_perFontConfigurations;
    DgnFontCP m_defaultRscFont;
    DgnFontCP m_defaultShxFont;
    DgnFontCP m_defaultShxBigFont;
    DgnFontCP m_defaultTrueTypeFont;
    DgnFontPtr m_decoratorFont;
    bool m_isUsingAnRtlLocale;
    bool m_isFontLinkingEnabled;
    bool m_isGlyphShapingDisabled;
    T_FontCatalogMap m_systemFonts;
    mutable DgnFontPtr m_lastResortRscFont;
    mutable DgnFontPtr m_lastResortShxFont;
    mutable DgnFontPtr m_lastResortTrueTypeFont;

    DgnFontCR GetLastResortRscFont() const;
    DgnFontCR GetLastResortShxFont() const;
    DgnFontCR GetLastResortTrueTypeFont() const;
    void ProcessLocaleConfiguration(BeXmlDomR, WCharCP effectiveLocaleName);
    void ReadPerFontConfigurations(BeXmlDomR);

public:
    virtual void _OnHostTermination(bool) override;
    static DgnFontConfigurationData const* FindCustomFontConfiguration(DgnFontKey const&);
    DGNPLATFORM_EXPORT static DgnFontCR GetFontForCodePage(UInt32 baseFontID, UInt32 shxBigFontID, DgnProjectCR);
    DGNPLATFORM_EXPORT void Initialize();
    DGNPLATFORM_EXPORT void InitializeRscFontFinder (IDgnFontFinder&);
    bool static IsUsingAnRtlLocale();
    bool static IsFontLinkingEnabled();
    bool static IsGlyphShapingDisabled();

// __PUBLISH_CLASS_VIRTUAL__
// __PUBLISH_SECTION_START__

public:

    //! Utility to reduce a font variant to a type(since a type is less descriptive).
    DGNPLATFORM_EXPORT static DgnFontType ConvertFontVariantToType(DgnFontVariant);

    //! Attempts to find a font by-type and -name in the system and workspace, and optionally embedded in a project.
    DGNPLATFORM_EXPORT static DgnFontCP FindFont(Utf8CP name, DgnFontType, DgnProjectCP);

    //! Gets the default decorator font. The decorator font is most useful for displaying text in a view window(e.g. when drawing with DGN objects, not necessarily for your higher level user interface).
    DGNPLATFORM_EXPORT static DgnFontCR GetDecoratorFont();

    //! Gets the default MicroStation RSC font. This is used when an element uses an RSC font that cannot be resolved in the current session.
    DGNPLATFORM_EXPORT static DgnFontCR GetDefaultRscFont();

    //! Gets the default AutoCAD SHX(non-big) font. This is used when an element uses an SHX font that cannot be resolved in the current session.
    DGNPLATFORM_EXPORT static DgnFontCR GetDefaultShxFont();

    //! Gets the default AutoCAD SHX big font. This is used when an element uses an SHX big font that cannot be resolved in the current session.
    DGNPLATFORM_EXPORT static DgnFontCP GetDefaultShxBigFont();

    //! Gets the default TrueType font. This is used when an element uses an SHX font that cannot be resolved in the current session.
    DGNPLATFORM_EXPORT static DgnFontCR GetDefaultTrueTypeFont();

    //! Gets a collection of known fonts of the requested variant, optionally searching the embedded fonts of a project.
    DGNPLATFORM_EXPORT static T_DgnFontCPs GetFonts(DgnFontVariant, DgnProjectCP);

    //! Attempts to resolve a font number to a font object. This method will always return non-NULL for DGNFONTVARIANT_DontCare, DGNFONTVARIANT_Rsc, DGNFONTVARIANT_TrueType, DGNFONTVARIANT_ShxShape, andDGNFONTVARIANT_ShxUni. Note that requesting DGNFONTVARIANT_ShxBig may return NULL.
    DGNPLATFORM_EXPORT static DgnFontCP ResolveFont(UInt32 fontID, DgnProjectCR, DgnFontVariant preferredFallbackType);

    }; // DgnFontManager

/** @} */

END_BENTLEY_DGNPLATFORM_NAMESPACE
