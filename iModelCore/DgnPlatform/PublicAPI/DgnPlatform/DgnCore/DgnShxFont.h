/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnShxFont.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include "DgnFontManager.h"

//! @addtogroup DgnFontModule
//! @{

DGNPLATFORM_TYPEDEFS (DgnShxFont)

DGNPLATFORM_REF_COUNTED_PTR (DgnShxFont)

// __PUBLISH_SECTION_END__

DGNPLATFORM_TYPEDEFS (DgnShxBigFont)
DGNPLATFORM_TYPEDEFS (DgnShxGlyph)
DGNPLATFORM_TYPEDEFS (DgnShxShapeFont)
DGNPLATFORM_TYPEDEFS (DgnShxUniFont)

// __PUBLISH_SECTION_START__

//! @}

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup DgnFontModule
//! @{

// __PUBLISH_SECTION_END__

struct IShxDataAccessor;

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     07/2012
//=======================================================================================
struct DgnShxGlyph : public DgnGlyph
    {
    DEFINE_T_SUPER (DgnGlyph)

private:
    DgnShxFontCP m_font;
    IShxDataAccessor* m_data;
    DPoint2d m_verticalCellBoxStart;
    DPoint2d m_verticalCellBoxEnd;
    GPArraySmartP m_gpa;
    bool m_isBlank;

protected:
    virtual BentleyStatus _FillGpa (GPArrayR) const override;
    virtual DgnFontType _GetType () const override;
    virtual bool _IsBlank () const override;

public:
    DgnShxGlyph (FontChar, size_t filePos, size_t dataSize, DgnShxFontCR, IShxDataAccessor&);
    double GetVerticalCellBoxBottom () const;
    double GetVerticalCellBoxLeft () const;
    double GetVerticalCellBoxTop () const;

    }; // DgnShxGlyph

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     07/2012
//=======================================================================================
struct DgnShxGlyphFPos
    {
    UInt16 m_charCode;
    UInt16 m_dataSize;
    long m_filePosition;

    }; // DgnShxGlyphFPos

// __PUBLISH_SECTION_START__

//=======================================================================================
//! Implementation of DgnFont for AutoCAD SHX fonts.
// @bsiclass                                                    Jeff.Marker     07/2012
//=======================================================================================
struct DgnShxFont : public DgnFont
    {
// __PUBLISH_SECTION_END__

    DEFINE_T_SUPER (DgnFont)

private:
    typedef bmap<FontChar, DgnShxGlyphP> T_GlyphMap;

protected:
    typedef bmap<FontChar, DgnShxGlyphFPos> T_GlyphFPosMap;

private:
    LangCodePage m_codePage;
    FontChar m_degreeCharCode;
    FontChar m_diameterCharCode;
    FontChar m_plusMinusCharCode;
    mutable bool m_isVertical;
    mutable T_GlyphMap m_glyphMap;

protected:
    IShxDataAccessor* m_data;
    mutable Byte m_ascender;
    mutable Byte m_descender;
    mutable Byte m_charHeight;
    mutable Byte m_charWidth;
    mutable T_GlyphFPosMap m_glyphFPosMap;

private:
    size_t LookupGlyphs (DgnShxGlyphCP* glyphs, UInt16 const* inChars, int nChars, bool shouldIgnorePercentEscapes, DgnShxBigFontCP) const;

protected:
    virtual bool _ContainsCharacter (WChar) const override;
    virtual DgnFontPtr _Embed (DgnProjectR) const override;
    virtual void _EnsureFontIsLoaded () const = 0;
    virtual LangCodePage _GetCodePage () const override;
    virtual FontChar _GetDegreeCharCode () const override;
    virtual double _GetDescenderRatio () const override;
    virtual FontChar _GetDiameterCharCode () const override;
    virtual FontChar _GetPlusMinusCharCode () const override;
    virtual DgnFontType _GetType () const override;
    virtual bool _IsGlyphBlank (FontChar) const override;
    virtual BentleyStatus _LayoutGlyphs (DgnGlyphLayoutContextCR, DgnGlyphLayoutResultR) const override;
    virtual bool _ShouldDrawWithLineWeight () const override;
    DgnShxFont (Utf8CP name, DgnFontConfigurationData const&, WCharCP filePath);
    DgnShxFont (Utf8CP name, DgnFontConfigurationData const&, UInt32 fontNumber, DgnProjectCR);

public:
    virtual ~DgnShxFont ();
    static void AcquireSystemFonts (T_FontCatalogMap&);
    static DgnFontPtr CreateFromEmbeddedFont (Utf8CP name, UInt32 fontNumber, DgnProjectCR);
    static DgnFontPtr CreateLastResortFont ();
    static DgnFontPtr CreateMissingFont (Utf8CP name);
    DgnShxGlyphCP FindGlyphCP (FontChar) const;
    double GetAscender () const;
    Byte GetCharHeight () const;
    Byte GetCharWidth () const;
    DGNPLATFORM_EXPORT BentleyStatus GetGlyphCodeFromGlyphName (FontChar& glyphCode, CharCP glyphName) const; // WIP_CHAR_OK - SHX format is limited to char*
    DgnShxGlyphFPos const* GetGlyphFilePos (FontChar) const;
    bool IsVertical () const;
    void SetIsVertical (bool);
    DGNPLATFORM_EXPORT static BeSQLite::PropertySpec CreateEmbeddedFontPropertySpec();

// __PUBLISH_CLASS_VIRTUAL__
// __PUBLISH_SECTION_START__

    }; // DgnShxFont

// __PUBLISH_SECTION_END__

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct DgnShxShapeFont : public DgnShxFont
    {
    friend struct DgnShxFont;

    DEFINE_T_SUPER (DgnShxFont)

private:
    mutable bool m_wasLoaded;

protected:
    virtual void _EnsureFontIsLoaded () const override;
    virtual DgnFontVariant _GetVariant () const override;
    DgnShxShapeFont (Utf8CP name, DgnFontConfigurationData const&, WCharCP filePath);
    DgnShxShapeFont (Utf8CP name, DgnFontConfigurationData const&, UInt32 fontNumber, DgnProjectCR);

    }; // DgnShxShapeFont

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct DgnShxBigFont : public DgnShxFont
    {
    friend struct DgnShxFont;

    DEFINE_T_SUPER (DgnShxFont)

private:
    typedef std::pair<FontChar, FontChar> T_InducerRange;
    typedef bvector<T_InducerRange> T_InducerRanges;

    mutable bool m_wasLoaded;
    mutable T_InducerRanges m_inducerRanges;

protected:
    virtual void _EnsureFontIsLoaded () const override;
    virtual DgnFontVariant _GetVariant () const override;
    DgnShxBigFont (Utf8CP name, DgnFontConfigurationData const&, WCharCP filePath);
    DgnShxBigFont (Utf8CP name, DgnFontConfigurationData const&, UInt32 fontNumber, DgnProjectCR);

public:
    bool IsMultiCharInducer (FontChar) const;

    }; // DgnShxBigFont

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct DgnShxUniFont : public DgnShxFont
    {
    friend struct DgnShxFont;

    DEFINE_T_SUPER (DgnShxFont)

private:
    mutable bool m_wasLoaded;

protected:
    virtual void _EnsureFontIsLoaded () const override;
    virtual DgnFontVariant _GetVariant () const override;
    DgnShxUniFont (Utf8CP name, DgnFontConfigurationData const&, WCharCP filePath);
    DgnShxUniFont (Utf8CP name, DgnFontConfigurationData const&, UInt32 fontNumber, DgnProjectCR);

    }; // DgnShxUniFont

// __PUBLISH_SECTION_START__

/** @} */

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
