/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnRscFont.h $
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

DGNPLATFORM_TYPEDEFS (DgnRscFont)

DGNPLATFORM_REF_COUNTED_PTR (DgnRscFont)

// __PUBLISH_SECTION_END__

DGNPLATFORM_TYPEDEFS (DgnRscGlyph)

// __PUBLISH_SECTION_START__

//! @}

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup DgnFontModule
//! @{

// __PUBLISH_SECTION_END__

struct RscGlyphHeader;
struct RscGlyphDataOffset;

typedef bmap<FontChar, std::pair<UInt8, UInt8> > T_RscFontFractionMap;

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct IRscDataAccessor
    {
    public: virtual BentleyStatus   _Embed                  (DgnProjectR, UInt32 fontNumber) = 0;
    public: virtual BentleyStatus   _ReadFontHeader         (bvector<Byte>&) = 0;
    public: virtual BentleyStatus   _ReadFractionMap        (T_RscFontFractionMap&) = 0;
    public: virtual BentleyStatus   _ReadGlyphData          (Byte*, size_t dataOffset, size_t dataSize) = 0;
    public: virtual BentleyStatus   _ReadGlyphDataOffsets   (bvector<RscGlyphDataOffset>&) = 0;
    public: virtual BentleyStatus   _ReadGlyphHeaders       (bvector<RscGlyphHeader>&) = 0;
    
    public: virtual                 ~IRscDataAccessor       () { }
    }; // IRscDataAccessor

typedef Int16           RscFontVec;
typedef UShort          RscFontDist;
typedef struct SPoint2d RscFontPoint2d;

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct RscFontVersion
    {
    byte    version;    // major version number
    byte    release;    // sub release within a version
    byte    stage;      // development, alpha, beta, release
    byte    fixes;      // fixes within stage
    
    }; // RscFontVersion

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct RscFontDate
    {
    Int32   julian; // julian date
    Int32   secs;   // seconds since midnight
    
    }; // RscFontDate

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct RscGlyphBoundingBox
    {
    RscFontVec left;
    RscFontVec bottom;
    RscFontVec right;
    RscFontVec top;
    
    }; // RscGlyphBoundingBox


//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct RscFontFraction
    {
    UInt8   numerator;
    UInt8   denominator;
    UInt16  charCode;
    
    }; // RscFontFraction

static const size_t MAX_FRACTION_MAP_COUNT = 76;

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct RscGlyphHeader
    {
    UInt16      code;           // character/glyph code
    RscFontDist width;          // distance to next character
    RscFontVec  leftEdge;       // distance to left edge of glyph from origin
    RscFontVec  rightEdge;      // distance to right edge of glyph from origin
    UInt32      unused;
    Int32       kernOffset;     // not used
    Int32       symbJust;       // if symbol font, justification for this char
    
    }; // RscGlyphHeader

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct RscGlyphDataOffset
    {
    Int32   offset;
    Int32   size;
    
    }; // RscGlyphDataOffset

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct RscGlyphData
    {
    UInt32  unused;
    Int32   length;     // total length of font elems (specified in bytes)
    Int32   numElems;   // number of font elements in glyph The elements will follow this header
    
    }; // RscGlyphData

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct RscGlyphElement
    {
    UChar           type;       // element type
    UChar           color;      // 0=no color
    UShort          numVerts;   // number of points
    RscFontPoint2d  vertice[1]; // points
    
    }; // RscGlyphElement

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
enum RscGlyphElementType
    {
    RSCGLYPHELEMENT_Line             = 1,
    RSCGLYPHELEMENT_Poly             = 2,
    RSCGLYPHELEMENT_PolyHoles        = 3,
    RSCGLYPHELEMENT_PolyHole         = 4,
    RSCGLYPHELEMENT_PolyHoleFinal    = 5,
    RSCGLYPHELEMENT_Spline           = 6
    
    }; // RscGlyphElementType

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct RscFontHeader
    {
    RscFontVersion      version;        // format version
    RscFontDate         creation;       // creation date
    UShort              machine;
    UShort              type;           // type of font
    UInt32              flags;          // various flags
    UInt32              totalKern;      // total number of kerning pairs
    UInt32              totalLig;       // total number of ligature values
    UInt32              number;         // font number (backpointer to self)
    UInt16              totalChars;     // total number of characters in font
    UInt16              firstChar;      // character id of first character
    UInt16              lastChar;       // character id of last character
    UInt16              defaultChar;    // char to be used if a character is missing
    RscFontPoint2d      origin;         // should always be zero (future support)
    RscGlyphBoundingBox maxbrr;         // maximized bounding ranges
    RscFontVec          ascender;       // distance above baseline
    RscFontVec          capHeight;      // distance to capline
    RscFontVec          xHeight;        // distance to top of lowercase 'x'
    RscFontVec          descender;      // distance below baseline
    RscFontDist         N_width;        // width of lower-case 'n'
    RscFontVec          kernTracks[4];  // kerning tracks
    RscFontVec          resv0;          // reserved padding
    UInt32              fractMap;       // resource ID of fractions map
    UInt32              unicodeMap;     // resource ID of unicode map
    UInt32              m_filledFlag:1; // if true, all glyphs in this font that have polygons should be treated as filled.
    UInt32              m_unused:31;
    Int16               languageId;     // language id
    UShort              nFamilyName;    // offset into 'name' of the family's name
    UShort              nFontName;      // offset into 'name' of the font's name
    UShort              nFastName;      // offset into 'name' for the fast font's name
    UShort              nDescript;      // offset into 'name' for font's description
    UShort              nName;          // number of characters used in 'name' (incl. nulls)
    char                name[1];        // font name data (null terminated)
    
    }; // RscFontHeader

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     07/2012
//=======================================================================================
struct DgnRscGlyph : public DgnGlyph
    {
    DEFINE_T_SUPER (DgnGlyph)

private:
    DPoint2d m_verticalCellBoxStart;
    DPoint2d m_verticalCellBoxEnd;
    IRscDataAccessor* m_data;
    double m_ascenderScaleFactor;
    bool m_isFontFilled;
    size_t m_glyphDataOffset;
    size_t m_glyphDataSize;

protected:
    virtual DgnFontType _GetType () const override;
    virtual BentleyStatus _FillGpa (GPArrayR) const override;
    virtual bool _IsBlank () const override;

public:
    DgnRscGlyph (IRscDataAccessor&, double ascender, bool isFontFilled, RscGlyphHeader const&, RscGlyphDataOffset const&);
    DgnRscGlyph (FontChar);
    double GetVerticalCellBoxBottom () const;
    double GetVerticalCellBoxLeft () const;
    double GetVerticalCellBoxRight () const;
    double GetVerticalCellBoxTop () const;
    void SetWidth (double blackBoxWidth, double cellBoxWidth);

    }; // DgnRscGlyph

//=======================================================================================
// @bsiclass
//=======================================================================================
enum RscGPAMasks
    {
    HMASK_RSC_LINE = 0x40000000,   //!< RSC lines
    HMASK_RSC_POLY = 0x80000000,   //!< RSC solid poly
    HMASK_RSC_HOLE = 0xC0000000    //!< RSC hole poly

    }; // RscGPAMasks

// __PUBLISH_SECTION_START__

//=======================================================================================
//! Implementation of DgnFont for MicroStation RSC fonts.
// @bsiclass                                                    Jeff.Marker     07/2012
//=======================================================================================
struct DgnRscFont : public DgnFont
    {
// __PUBLISH_SECTION_END__

    DEFINE_T_SUPER (DgnFont)

private:
    typedef bmap<FontChar, DgnRscGlyphP> T_GlyphMap;

public:
    typedef bvector<DgnFontPtr> T_DgnFontPtrs;
    typedef T_RscFontFractionMap T_FractionMap;

    enum {RSCFONT_FallbackRscFont = 1};

private:
    IRscDataAccessor* m_data;
    LangCodePage m_codePage;
    UInt32 m_v8FontNumber;
    FontChar m_degreeCharCode;
    FontChar m_diameterCharCode;
    FontChar m_plusMinusCharCode;
    mutable bool m_wasLoaded;
    mutable T_FractionMap m_fractionMap;
    mutable FontChar m_defaultCharCode;
    mutable T_GlyphMap m_glyphMap;
    mutable double m_descenderRatio;
    static IDgnFontFinder* s_fontFinder;

protected:
    virtual bool _ContainsCharacter (WChar) const override;
    virtual DgnFontPtr _Embed (DgnProjectR) const override;
    virtual LangCodePage _GetCodePage () const override;
    virtual FontChar _GetDegreeCharCode () const override;
    virtual double _GetDescenderRatio () const override;
    virtual FontChar _GetDiameterCharCode () const override;
    virtual FontChar _GetPlusMinusCharCode () const override;
    virtual DgnFontType _GetType () const override;
    virtual bool _IsGlyphBlank (FontChar) const override;
    virtual DgnFontVariant _GetVariant () const override;
    virtual BentleyStatus _LayoutGlyphs (DgnGlyphLayoutContextCR, DgnGlyphLayoutResultR) const override;
    virtual WChar _RemapFontCharToUnicodeChar (FontChar) const override;
    virtual FontChar _RemapUnicodeCharToFontChar (WChar) const override;
    virtual bool _ShouldDrawWithLineWeight () const override;
    DgnRscFont (Utf8CP name, DgnFontConfigurationData const&, UInt32 fontNumber, DgnProjectCR);
    DgnRscFont (Utf8CP name, DgnFontConfigurationData const& config, IRscDataAccessor* accessor, UInt32 v8FontNumber, bool isLastResort);
    void EnsureFontIsLoaded () const;
    FontChar GetDefaultCharCode () const;

public:
    virtual ~DgnRscFont ();
    static void       AcquireSystemFonts (T_FontCatalogMap&);
    static DgnFontPtr CreateFromEmbeddedFont (Utf8CP name, UInt32 fontNumber, DgnProjectCR);
    static DgnFontPtr CreateLastResortFont ();
    static DgnFontPtr CreateMissingFont(Utf8CP name, UInt32 v8FontNumber);
    DGNPLATFORM_EXPORT DgnRscGlyphCP FindGlyphCP (FontChar) const;
    DGNPLATFORM_EXPORT BentleyStatus FontCharToFraction (UInt8& numerator, UInt8& denominator, FontChar) const;
    DGNPLATFORM_EXPORT FontChar FractionToFontChar (UInt8 numerator, UInt8 denominator, bool reduce = true) const;
    DGNPLATFORM_EXPORT UInt32 GetV8FontNumber () const;
    static void OnHostTermination ();

    //  The following functions are used by the FontManager when foreignformat calls InitializeRscFontFinder
    static void RegisterIDgnFontFinder (IDgnFontFinder&);

    //  The following functions are used by the RscFontImporter in ForeignFormat/DgnV8
    DGNPLATFORM_EXPORT static DgnRscFont* Create (Utf8CP name, DgnFontConfigurationData const& config, IRscDataAccessor* accessor, UInt32 v8FontNumber, bool isLastResort) {return new DgnRscFont(name,config,accessor,v8FontNumber,isLastResort);}

    DGNPLATFORM_EXPORT static  BeSQLite::PropertySpec    CreateEmbeddedFontHeaderPropertySpec        ();
    DGNPLATFORM_EXPORT static  BeSQLite::PropertySpec    CreateEmbeddedFractionsPropertySpec         ();
    DGNPLATFORM_EXPORT static  BeSQLite::PropertySpec    CreateEmbeddedGlyphDataPropertySpec         ();
    DGNPLATFORM_EXPORT static  BeSQLite::PropertySpec    CreateEmbeddedGlyphDataOffsetsPropertySpec  ();
    DGNPLATFORM_EXPORT static  BeSQLite::PropertySpec    CreateEmbeddedGlyphHeadersPropertySpec      ();

    DGNPLATFORM_EXPORT static DgnFontConfigurationData GetEffectiveFontConfig (DgnFontKey const& fontKey);

// __PUBLISH_CLASS_VIRTUAL__
// __PUBLISH_SECTION_START__

public:

    //! Scans the input string for fractions (##/##), and if an RSC fraction glyph exists for any, they are replaced with the glyph.
    DGNPLATFORM_EXPORT void CompressFractions (WStringR outStr, WCharCP inStr) const;

    //! Scans the input string for special RSC fraction glyphs, and expands them to their full value (e.g. of the form ##/##).
    DGNPLATFORM_EXPORT void ExpandFractions (WStringR outStr, WCharCP inStr) const;

    }; // DgnRscFont

/** @} */

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
