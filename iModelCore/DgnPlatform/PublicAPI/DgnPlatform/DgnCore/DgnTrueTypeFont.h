/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnTrueTypeFont.h $
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

DGNPLATFORM_TYPEDEFS (DgnTrueTypeFont)

DGNPLATFORM_REF_COUNTED_PTR (DgnTrueTypeFont)

// __PUBLISH_SECTION_END__

DGNPLATFORM_TYPEDEFS (DgnTrueTypeGlyph)

// __PUBLISH_SECTION_START__

//! @}

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup DgnFontModule
//! @{

// __PUBLISH_SECTION_END__

struct ITTPImpl;
struct ITrueTypeDataAccessor;

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     07/2012
//=======================================================================================
struct DgnTrueTypeGlyph : public DgnGlyph
    {
    DEFINE_T_SUPER (DgnGlyph)

private:
    ITrueTypeDataAccessor* m_data;
    double m_fontScaleFactor;
    bool m_isGlyphIndex;
    bool m_isBlank;
    bool m_isBold;
    bool m_isItalic;

protected:
    virtual BentleyStatus _FillGpa (GPArrayR) const override;
    virtual DgnFontType _GetType () const override;
    virtual bool _IsBlank () const override;

public:
    DgnTrueTypeGlyph (FontChar, bool isGlyphIndex, ITrueTypeDataAccessor&, double fontScaleFactor, bool isBold, bool isItalic);
    DGNPLATFORM_EXPORT bvector<Byte> GetTrueTypeData () const;

    }; // DgnTrueTypeGlyph

// __PUBLISH_SECTION_START__

//=======================================================================================
//! Implementation of DgnFont for TrueType fonts.
// @bsiclass                                                    Jeff.Marker     07/2012
//=======================================================================================
struct DgnTrueTypeFont : public DgnFont
    {
// __PUBLISH_SECTION_END__

    DEFINE_T_SUPER (DgnFont)

private:
// WIP_NONPORT - TrueType
#if defined (BENTLEY_WIN32)
    friend struct Win32TTPImpl;
#else
    friend struct FreeTypeTTPImpl;
#endif

    typedef bmap<FontChar, DgnTrueTypeGlyph*> T_GlyphMap;
    typedef bmap<UInt16, DgnTrueTypeGlyph*> T_GlyphIndexMap;

// WIP_NONPORT - TrueType
#if defined (BENTLEY_WIN32)

    //=======================================================================================
    // @bsiclass                                                    Jeff.Marker     08/2012
    //=======================================================================================
    enum ScriptCacheIndex
        {
        SCRIPT_CACHE_INDEX_Normal,
        SCRIPT_CACHE_INDEX_Bold,
        SCRIPT_CACHE_INDEX_Italic,
        SCRIPT_CACHE_INDEX_BoldItalic,
        SCRIPT_CACHE_INDEX_Max

        }; // ScriptCacheIndex

#endif

    ITTPImpl* m_ttPImpl;
    ITrueTypeDataAccessor* m_data;
    mutable bool m_wasLoaded;
    mutable double m_scaleFactor;
    mutable double m_descenderRatio;

// WIP_NONPORT - TrueType
#if defined (BENTLEY_WIN32)
    mutable void* m_uniscribeScriptCaches[SCRIPT_CACHE_INDEX_Max];
#endif

    BentleyStatus ComputeAdvanceWidths(WCharCP chars, size_t totalNumChars, T_DoubleVectorR outAdvanceWidths, bvector<DPoint2d>& outGlyphOffsets, bvector<bool>& glyphIndexMask, DgnGlyphLayoutResult::T_GlyphCodesR outGlyphCodes, T_DoubleVectorR leadingCaretOffsets, T_DoubleVectorR trailingCaretOffsets, DgnGlyphLayoutContextCR, double existingUnitCaretOffset, bool shouldDisableGlyphShaping, bool shouldAllowMissingGlyphs) const;
    void EnsureFontIsLoaded () const;
    WChar FixDiameterCharCode (WChar) const;
    BentleyStatus LayoutGlyphsInternal (DgnGlyphLayoutContextCR, DgnGlyphLayoutResultR, WCharCP processedChars, size_t numChars) const;

protected:
    virtual bool _ContainsCharacter (WChar) const override;
    virtual DgnFontPtr _Embed (DgnProjectR) const override;
    virtual LangCodePage _GetCodePage () const override;
    virtual FontChar _GetDegreeCharCode () const override;
    virtual double _GetDescenderRatio () const override;
    virtual FontChar _GetDiameterCharCode () const override;
    virtual FontChar _GetPlusMinusCharCode () const override;
    virtual DgnFontType _GetType () const override;
    virtual DgnFontVariant _GetVariant () const override;
    virtual bool _IsGlyphBlank (FontChar) const override;
    virtual BentleyStatus _LayoutGlyphs (DgnGlyphLayoutContextCR, DgnGlyphLayoutResultR) const override;
    virtual bool _ShouldDrawWithLineWeight () const override;

// WIP_NONPORT - TrueType
#if defined (BENTLEY_WIN32)
    DgnTrueTypeFont (Utf8CP name, DgnFontConfigurationData const&);
#else
    DgnTrueTypeFont (Utf8CP name, WCharCP filePath, DgnFontConfigurationData const&);
#endif
    DgnTrueTypeFont (Utf8CP name, DgnFontConfigurationData const&, UInt32 fontNumber, DgnProjectCR);

public:
    virtual ~DgnTrueTypeFont ();
    static void AcquireSystemFonts (T_FontCatalogMap&);
    static DgnFontPtr CreateFromEmbeddedFont (Utf8CP name, UInt32 fontNumber, DgnProjectCR);
    static DgnFontPtr CreateLastResortFont ();
    static DgnFontPtr CreateMissingFont (Utf8CP name);
    DGNPLATFORM_EXPORT double GetScaleFactor () const;
    static void OnHostTermination ();
    DGNPLATFORM_EXPORT static BeSQLite::PropertySpec CreateEmbeddedFontPropertySpec();

// WIP_NONPORT - TrueType
#if defined (BENTLEY_WIN32)
    static DgnFontPtr CreateDecoratorLogicalFont ();
    DGNPLATFORM_EXPORT static BentleyStatus GetFontFaceNameFromFileName (WStringR faceName, WCharCP fileName);
    DGNPLATFORM_EXPORT BentleyStatus GetPitchAndCharSet (byte& pitch, byte& charSet) const;
#endif

// __PUBLISH_CLASS_VIRTUAL__
// __PUBLISH_SECTION_START__

    }; // DgnTrueTypeFont

/** @} */

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
