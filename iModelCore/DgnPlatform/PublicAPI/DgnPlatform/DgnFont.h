/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnFont.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnPlatform.h"

typedef struct FT_FaceRec_* FT_Face; // Shield users from freetype.h because they have a bad include scheme.

BEGIN_BENTLEY_DGN_NAMESPACE

typedef bvector<DgnFontPtr> T_DgnFontPtrs;

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     03/2015
//=======================================================================================
struct DgnGlyph
{
    typedef uint16_t T_Id; // The only real use of exposing this is for QuickVision. It should in reality be a QVwchar, but replicate here because we're below quickvision.

public:
    virtual ~DgnGlyph() {}
    virtual T_Id _GetId() const = 0;
    T_Id GetId() const { return _GetId(); }
    virtual DRange2d _GetRange() const = 0;
    DRange2d GetRange() const { return _GetRange(); }
    virtual DRange2d _GetExactRange() const = 0;
    DRange2d GetExactRange() const { return _GetExactRange(); }
    virtual BentleyStatus _FillGpa(GPArrayR) const = 0;
    BentleyStatus FillGpa(GPArrayR gpa) const { return _FillGpa(gpa); }
    virtual bool _IsBlank() const = 0;
    bool IsBlank() const { return _IsBlank(); }
};

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     03/2015
//=======================================================================================
struct DgnGlyphLayoutContext
{
    Utf8String m_string;
    DPoint2d m_drawSize;
    bool m_isBold;
    bool m_isItalic;

    DgnGlyphLayoutContext() : m_isBold(false), m_isItalic(false) { m_drawSize.Zero(); }
};

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     03/2015
//=======================================================================================
struct DgnGlyphLayoutResult
{
    typedef bvector<DgnGlyphCP> T_Glyphs;
    typedef bvector<DPoint3d> T_GlyphOrigins;

    DRange2d m_range;
    DRange2d m_exactRange;
    DRange2d m_justificationRange;
    T_Glyphs m_glyphs;
    T_GlyphOrigins m_glyphOrigins;

    DgnGlyphLayoutResult() { m_range.Init(); m_exactRange.Init(); m_justificationRange.Init(); }
};

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     03/2015
//=======================================================================================
struct IDgnFontData;
struct IDgnTrueTypeFontData;
struct IDgnRscFontData;
struct IDgnShxFontData;

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     03/2015
//=======================================================================================
struct DgnFontDataSession
{
private:
    IDgnFontData* m_data;
    bool m_hasStarted;
    bool m_isValid;

public:
    DgnFontDataSession() : m_data(nullptr), m_hasStarted(false), m_isValid(false) {}
    DgnFontDataSession(IDgnFontData* data) : m_data(data), m_hasStarted(false), m_isValid(false) {}
    ~DgnFontDataSession() { Stop(); }
    DGNPLATFORM_EXPORT bool Start();
    DGNPLATFORM_EXPORT void Stop();
    void Reset(IDgnFontData* data) { m_data = data; m_hasStarted = false; m_isValid = false; }
    bool HasStarted() { return m_hasStarted; }
    bool IsValid() { return m_isValid; }
};

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     03/2015
//=======================================================================================
struct AutoDgnFontDataSession : DgnFontDataSession
{
    AutoDgnFontDataSession(IDgnFontData& data) : DgnFontDataSession(&data) { Start(); }
};

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     03/2015
//=======================================================================================
struct DgnFont : RefCountedBase
{
protected:
    friend struct DgnFontPersistence;
    DgnFontType m_type;
    Utf8String m_name;
    IDgnFontDataP m_data;
    mutable DgnFontDataSession m_dataSession;

private:
    DGNPLATFORM_EXPORT void CopyFrom(DgnFontCR);

protected:
    DgnFont(DgnFontType type, Utf8CP name, IDgnFontDataP data) : m_type(type), m_name(name), m_data(data), m_dataSession(data) {}
    DgnFont(DgnFontCR rhs) : RefCountedBase(rhs) { CopyFrom(rhs); }
    DGNPLATFORM_EXPORT virtual ~DgnFont();
    DgnFontR operator=(DgnFontCR rhs) { RefCountedBase::operator=(rhs); if (this != &rhs) CopyFrom(rhs); return *this; }
    static uint32_t const* Utf8ToUcs4(bvector<Byte>& ucs4CharsBuffer, size_t& numUcs4Chars, Utf8StringCR);
    static void ScaleAndOffsetGlyphRange(DRange2dR, DPoint2dCR, DPoint2d);

public:
    DgnFontType GetType() const { return m_type; }
    Utf8StringCR GetName() const { return m_name; }
    virtual DgnFontPtr _Clone() const = 0;
    DgnFontPtr Clone() const { return _Clone(); }
    virtual BentleyStatus _LayoutGlyphs(DgnGlyphLayoutResultR, DgnGlyphLayoutContextCR) const = 0;
    BentleyStatus LayoutGlyphs(DgnGlyphLayoutResultR result, DgnGlyphLayoutContextCR context) const { return _LayoutGlyphs(result, context); }
    virtual bool _CanDrawWithLineWeight() const = 0;
    bool CanDrawWithLineWeight() const { return _CanDrawWithLineWeight(); }
    DGNPLATFORM_EXPORT bool IsResolved() const;
    virtual DgnGlyphCP _FindGlyphCP(DgnGlyph::T_Id, DgnFontStyle) const = 0;
    DgnGlyphCP FindGlyphCP(DgnGlyph::T_Id glyphId, DgnFontStyle fontStyle) const { return _FindGlyphCP(glyphId, fontStyle); }
    DGNPLATFORM_EXPORT static DgnFontStyle FontStyleFromBoldItalic(bool isBold, bool isItalic);
    DGNPLATFORM_EXPORT static void FontStyleToBoldItalic(bool& isBold, bool& isItalic, DgnFontStyle);
    virtual double _GetDescenderRatio(DgnFontStyle) const = 0;
    double GetDescenderRatio(DgnFontStyle fontStyle) const { return _GetDescenderRatio(fontStyle); }
};

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     03/2015
//=======================================================================================
struct DgnTrueTypeFont : DgnFont
{
private:
    friend struct DgnFontPersistence;
    typedef bmap<unsigned int /*FT_Uint*/, DgnGlyphCP> T_GlyphCache;
    typedef bmap<DgnFontStyle, T_GlyphCache> T_GlyphCacheMap;
    mutable T_GlyphCacheMap m_glyphCache;

    DgnGlyphCP FindGlyphCP(FT_Face, unsigned int /*FT_UInt*/ glyphIndex, DgnFontStyle) const;
    BentleyStatus ComputeAdvanceWidths(T_DoubleVectorR, FT_Face, uint32_t const* ucs4Chars, size_t numChars) const;

public:
    DgnTrueTypeFont(Utf8CP name, IDgnFontDataP data) : DgnFont(DgnFontType::TrueType, name, data) {}
    DgnTrueTypeFont(DgnTrueTypeFontCR rhs) : DgnFont(rhs) {}
    DGNPLATFORM_EXPORT virtual ~DgnTrueTypeFont();
    DgnTrueTypeFontR operator=(DgnTrueTypeFontCR rhs) { DgnFont::operator=(rhs); return *this; }
    DGNPLATFORM_EXPORT DgnFontPtr Create(Utf8CP name, IDgnFontDataP data);
    DGNPLATFORM_EXPORT virtual DgnFontPtr _Clone() const override;
    DGNPLATFORM_EXPORT virtual BentleyStatus _LayoutGlyphs(DgnGlyphLayoutResultR, DgnGlyphLayoutContextCR) const override;
    virtual bool _CanDrawWithLineWeight() const override { return false; }
    DGNPLATFORM_EXPORT static BentleyStatus GetTrueTypeGlyphDataDirect(bvector<Byte>&, double& scaleFactor, DgnGlyphCR);
    DGNPLATFORM_EXPORT virtual DgnGlyphCP _FindGlyphCP(DgnGlyph::T_Id, DgnFontStyle) const override;
    DGNPLATFORM_EXPORT virtual double _GetDescenderRatio(DgnFontStyle) const override;
};

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     03/2015
//=======================================================================================
struct DgnRscFont : DgnFont
{
    //=======================================================================================
    // @bsiclass                                                    Jeff.Marker     03/2015
    //=======================================================================================
    struct Metadata
    {
        LangCodePage m_codePage;
        DgnGlyph::T_Id m_degreeCode;
        DgnGlyph::T_Id m_diameterCode;
        DgnGlyph::T_Id m_plusMinusCode;
        
        Metadata() : m_codePage(LangCodePage::Unknown), m_degreeCode(94), m_diameterCode(216), m_plusMinusCode(200) {}
    };

private:
    friend struct DgnFontPersistence;
    Metadata m_metadata;
    typedef bmap<DgnGlyph::T_Id, DgnGlyphCP> T_GlyphCache;
    mutable T_GlyphCache m_glyphCache;
    mutable DgnGlyphCP m_defaultGlyph;
    mutable bool m_isDefaultGlyphAllocated;
    mutable double m_descenderRatio;
    mutable bool m_hasLoadedGlyphs;
    typedef bpair<uint8_t, uint8_t> T_FractionMapKey;
    typedef bmap<T_FractionMapKey, DgnGlyph::T_Id> T_FractionMap;
    mutable T_FractionMap m_fractions;
    mutable bool m_hasLoadedFractions;

    DGNPLATFORM_EXPORT void LoadGlyphs() const;
    DGNPLATFORM_EXPORT DgnGlyphCP FindGlyphCP(DgnGlyph::T_Id) const;
    DgnGlyphCP GetDefaultGlyphCP() const { LoadGlyphs(); return m_defaultGlyph; }
    DgnGlyph::T_Id FindFractionGlyphCode(uint8_t numerator, uint8_t denominator) const;
    DgnGlyph::T_Id Ucs4CharToFontChar(uint32_t, CharCP codePageString, bvector<Byte>& localeBuffer) const;
    bvector<DgnGlyph::T_Id> Utf8ToFontChars(Utf8StringCR) const;

public:
    DgnRscFont(Utf8CP name, IDgnFontDataP data) : DgnFont(DgnFontType::Rsc, name, data), m_defaultGlyph(nullptr), m_isDefaultGlyphAllocated(false), m_descenderRatio(1.0), m_hasLoadedGlyphs(false), m_hasLoadedFractions(false) {}
    DgnRscFont(DgnRscFontCR rhs) : DgnFont(rhs) {}
    DGNPLATFORM_EXPORT virtual ~DgnRscFont();
    DgnRscFontR operator=(DgnRscFontCR rhs) { DgnFont::operator=(rhs); return *this; }
    DGNPLATFORM_EXPORT DgnFontPtr Create(Utf8CP name, IDgnFontDataP data);
    DGNPLATFORM_EXPORT virtual DgnFontPtr _Clone() const override;
    DGNPLATFORM_EXPORT virtual BentleyStatus _LayoutGlyphs(DgnGlyphLayoutResultR, DgnGlyphLayoutContextCR) const override;
    virtual bool _CanDrawWithLineWeight() const override { return true; }
    Metadata const& GetMetadata() const { return m_metadata; }
    Metadata& GetMetadataR() { return m_metadata; }
    virtual DgnGlyphCP _FindGlyphCP(DgnGlyph::T_Id glyphId, DgnFontStyle) const override { return FindGlyphCP(glyphId); }
    virtual double _GetDescenderRatio(DgnFontStyle) const override { LoadGlyphs(); return m_descenderRatio; }
};

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     03/2015
//=======================================================================================
struct DgnShxFont : DgnFont
{
    enum class ShxType { Invalid, Unicode, Locale };

    //=======================================================================================
    // @bsiclass                                                    Jeff.Marker     03/2015
    //=======================================================================================
    struct Metadata
    {
        LangCodePage m_codePage;
        DgnGlyph::T_Id m_degreeCode;
        DgnGlyph::T_Id m_diameterCode;
        DgnGlyph::T_Id m_plusMinusCode;

        Metadata() : m_codePage(LangCodePage::Unknown), m_degreeCode(0), m_diameterCode(0), m_plusMinusCode(0) {}
    };

    //=======================================================================================
    // @bsiclass                                                    Jeff.Marker     07/2012
    //=======================================================================================
    struct GlyphFPos
    {
        int64_t m_dataOffset;
        size_t m_dataSize;
    };

private:
    friend struct DgnFontPersistence;
    Metadata m_metadata;
    typedef bmap<DgnGlyph::T_Id, DgnGlyphCP> T_GlyphCache;
    mutable T_GlyphCache m_glyphCache;

    DGNPLATFORM_EXPORT DgnGlyphCP FindGlyphCP(DgnGlyph::T_Id) const;
    DgnGlyph::T_Id Ucs4CharToFontChar(uint32_t, CharCP codePageString, bvector<Byte>& localeBuffer) const;
    bvector<DgnGlyph::T_Id> Utf8ToFontChars(Utf8StringCR) const;

public:
    DgnShxFont(Utf8CP name, IDgnFontDataP data) : DgnFont(DgnFontType::Shx, name, data) {}
    DgnShxFont(DgnShxFontCR rhs) : DgnFont(rhs) {}
    DgnShxFontR operator=(DgnShxFontCR rhs) { DgnFont::operator=(rhs); return *this; }
    DGNPLATFORM_EXPORT DgnFontPtr Create(Utf8CP name, IDgnFontDataP data);
    DGNPLATFORM_EXPORT virtual DgnFontPtr _Clone() const override;
    DGNPLATFORM_EXPORT virtual BentleyStatus _LayoutGlyphs(DgnGlyphLayoutResultR, DgnGlyphLayoutContextCR) const override;
    virtual bool _CanDrawWithLineWeight() const override { return true; }
    DGNPLATFORM_EXPORT ShxType GetShxType() const;
    Metadata const& GetMetadata() const { return m_metadata; }
    Metadata& GetMetadataR() { return m_metadata; }
    virtual DgnGlyphCP _FindGlyphCP(DgnGlyph::T_Id glyphId, DgnFontStyle) const override { return FindGlyphCP(glyphId); }
    DGNPLATFORM_EXPORT virtual double _GetDescenderRatio(DgnFontStyle) const override;
};

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     03/2015
//=======================================================================================
struct DgnFontManager : NonCopyableClass
{
    DGNPLATFORM_EXPORT static DgnFontCR GetLastResortTrueTypeFont();
    DGNPLATFORM_EXPORT static DgnFontCR GetLastResortRscFont();
    DGNPLATFORM_EXPORT static DgnFontCR GetLastResortShxFont();
    DGNPLATFORM_EXPORT static DgnFontCR GetAnyLastResortFont();
    DGNPLATFORM_EXPORT static DgnFontCR GetDecoratorFont();
    DGNPLATFORM_EXPORT static DgnFontCR ResolveFont(DgnFontCP);
};

END_BENTLEY_DGN_NAMESPACE
