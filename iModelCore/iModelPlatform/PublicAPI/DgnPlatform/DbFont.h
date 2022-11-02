/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "FontManager.h"

typedef struct FreeTypeFaceStream* FtFaceStreamP;

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/**
 * A Glyph from a DbFont
 */
struct DbGlyph : NonCopyableClass {
    enum class RasterStatus {
        Success = 0,
        CannotLoadGlyph = 1,
        CannotRenderGlyph = 2,
        NoGetRasterFunc = 3,
    };

    virtual ~DbGlyph() {}
    virtual uint32_t GetId() const = 0;
    virtual DRange2d GetRange() const = 0;
    virtual DRange2d GetExactRange() const = 0;
    virtual CurveVectorPtr GetCurveVector() const { return nullptr; }
    virtual bool IsBlank() const = 0;
    RasterStatus GetRaster(Dgn::Render::ImageR raster) const { return _GetRaster(raster); }

protected:
    enum class DoFixup {
        Always,
        IfHoles,
        Never,
    };
    virtual DoFixup _DoFixup() const = 0;
    virtual RasterStatus _GetRaster(Dgn::Render::ImageR) const { return RasterStatus::NoGetRasterFunc; }
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct GlyphLayoutContext {
    Utf8String m_string;
    DPoint2d m_drawSize;
    bool m_isBold = false;
    bool m_isItalic = false;
    GlyphLayoutContext() { m_drawSize.Zero(); }
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct GlyphLayoutResult {
    typedef bvector<DbGlyphCP> T_Glyphs;
    typedef bvector<DPoint3d> T_GlyphOrigins;

    DRange2d m_range;
    DRange2d m_exactRange;
    DRange2d m_justificationRange;
    T_Glyphs m_glyphs;
    T_GlyphOrigins m_glyphOrigins;

    GlyphLayoutResult() { m_range.Init(); m_exactRange.Init(); m_justificationRange.Init(); }
    void ZeroNullRanges();
};

/**
 * A font to be used by text-related apis.
 * Fonts may either be embedded within an iModel (if it is meant to be readonly), or loaded from WorkspaceDbs.
 * DbFonts are created by `FontDb.Load`, and are only and always deleted when the FontDb is closed.
 * There are 3 subclasses of DbFont: RscFont, ShxFont, and TrueTypeFont. Every instance of a DbFont has a unique (per type) "familyName", and
 * potentially separate "faces" for regular, bold, italics, and boldItalic - though multiple faces are only supported by TrueType.
 * They load and cache their DbGlyphs on demand, by reading their data from their FontDb via the appropriate `ReaderForFace`.
 *
 * @note for TrueType fonts only:
 * It is important to understand that multiple `ReaderForFace`s may share the same entry in the embedded font table, since "ttc" (truetype collection)
 * files may have more than one family/face in the same file. Also, the `ReaderForFace`s for a single TrueTypeFont object may use different
 * entries in the embedded font table, since some fonts have a separate file for each face.
 */
struct DbFont : NonCopyableClass {
protected:
    /** the FontDb from which this DbFont was loaded. */
    FontDbCR m_db;
    /** The type of font */
    FontType m_type;
    /** the family name for this font. Note that font name lookups are always case insensitive */
    Utf8String m_familyname;
    /** the reader for the Regular face. */
    ReaderForFace m_regular;
    /** the reader for the bold face. If not present, fall back to regular */
    ReaderForFace m_bold;
    /** the reader for the italic face. If not present, fall back to regular */
    ReaderForFace m_italic;
    /** the reader for the italic face. If not present, fall back to bold then italic */
    ReaderForFace m_boldItalic;

    DbFont(FontType type, Utf8CP name, FontDbCR db) : m_type(type), m_familyname(name), m_db(db) {}
    /** Get the reader for a FaceStyle */
    ReaderForFaceR FindReader(FaceStyle style);

public:
    static uint32_t const* Utf8ToUcs4(bvector<Byte>& ucs4CharsBuffer, size_t& numUcs4Chars, Utf8StringCR);
    static void ScaleAndOffsetGlyphRange(DRange2dR, DPoint2dCR, DPoint2d);
    virtual ~DbFont() {}

    void SetReader(FaceStyle fontStyle, FontFaceR face, FontDbReaderR reader);
    ReaderForFaceR GetReader(FaceStyle fontStyle = FaceStyle::Regular);

    FontType GetType() const { return m_type; }
    /** get the family name for this font. Note that font name lookups are always case insensitive */
    Utf8StringCR GetName() const { return m_familyname; }
    /** Determine whether this DbFont came from a WorkspaceDb (or is embedded in an iModel) */
    bool IsWorkspaceFont() const { return m_db.m_isWorkspace; }
    virtual BentleyStatus LayoutGlyphs(GlyphLayoutResultR result, GlyphLayoutContextCR context) = 0;
    virtual bool CanDrawWithLineWeight() = 0;
    virtual DbGlyphCP FindGlyphCP(uint32_t glyphId, FaceStyle) = 0;
    virtual double GetDescenderRatio(FaceStyle fontStyle) = 0;
    bool HasFace(FaceStyle style = FaceStyle::Regular) { return FindReader(style).m_face->m_faceStyle == style; }
    DGNPLATFORM_EXPORT bool IsFallback();
};

/** A TrueType DbFont */
struct TrueTypeFont : DbFont {
    typedef uint32_t T_Id;
    /** A single face within a TrueTypeFont. This holds the glyph cache for the face as well as the FreeType face object */
    struct TrueTypeFace {
        typedef bmap<T_Id, DbGlyphCP> T_GlyphCache;

        bool m_initialized = false;
        FtFaceStreamP m_ftFaceStream = nullptr;
        unsigned int m_pixelScale = 0;
        FaceStyle m_style;
        T_GlyphCache m_glyphCache;

        FaceStyle GetFaceStyle();
        Utf8String GetFamilyName();
        bool HasEmbeddingRights();
        void Destroy();
        ~TrueTypeFace();

        bool IsValid() const { return nullptr != m_ftFaceStream; }
        unsigned int GetPixelScale() const { return m_pixelScale; }

        /** Given some callable object accepting an FT_Face as its argument, execute it in a thread-safe manner. */
        template <typename F>
        auto Execute(F func) const -> decltype(func(m_ftFaceStream)) {
            BeMutexHolder lock(FontManager::GetMutex());
            BeAssert(IsValid());
            return func(m_ftFaceStream);
        }
        // Initialize TrueTypeFace. Check IsValid() to determine if the operation succeeded.
        void Initialize(ReaderForFaceR);

        // Initialize from a file
        void Initialize(Utf8CP path, int faceIndex = 0);
        BentleyStatus ComputeAdvanceWidths(T_DoubleVectorR, uint32_t const* ucs4Chars, size_t numChars);
    };

private:
    TrueTypeFace m_ftRegular;
    TrueTypeFace m_ftBold;
    TrueTypeFace m_ftItalic;
    TrueTypeFace m_ftBoldItalic;

public:
    TrueTypeFont(Utf8CP name, FontDbCR db) : DbFont(FontType::TrueType, name, db) {}
    virtual ~TrueTypeFont() {}
    BentleyStatus LayoutGlyphs(GlyphLayoutResultR, GlyphLayoutContextCR)  override;
    bool CanDrawWithLineWeight() override { return false; }
    DbGlyphCP FindGlyphCP(T_Id glyphId, FaceStyle) override;
    double GetDescenderRatio(FaceStyle) override;
    TrueTypeFace& LoadFace(FaceStyle);

    //! @private -- only intended for use in the DgnV8Converter.
    DGNPLATFORM_EXPORT bvector<unsigned int> ComputeGlyphIndices(Utf8CP, bool isBold, bool isItalic);
};

/** An RSC DbFont */
struct RscFont : DbFont {
private:
    typedef uint16_t T_Id;
    typedef bmap<T_Id, DbGlyphCP> T_GlyphCache;
    typedef bpair<uint8_t, uint8_t> T_FractionMapKey;
    typedef bmap<T_FractionMapKey, T_Id> T_FractionMap;

    T_GlyphCache m_glyphCache;
    DbGlyphCP m_defaultGlyph = nullptr;
    bool m_isDefaultGlyphAllocated = false;
    bool m_hasLoadedGlyphs = false;
    bool m_hasLoadedFractions = false;
    double m_descenderRatio = 1.0;
    T_FractionMap m_fractions;

    void LoadGlyphs();
    DbGlyphCP FindRscGlyph(T_Id);
    DbGlyphCP GetDefaultGlyphCP() {
        LoadGlyphs();
        return m_defaultGlyph;
    }
    T_Id FindFractionGlyphCode(uint8_t numerator, uint8_t denominator);
    T_Id Ucs4CharToFontChar(uint32_t, CharCP codePageString, bvector<Byte>& localeBuffer);
    bvector<T_Id> Utf8ToFontChars(Utf8StringCR);

public:
    FontFaceCR GetFace() { return *GetReader().m_face; }
    BentleyStatus ReadFontHeader(bvector<Byte>&);
    BentleyStatus ReadFractionMap(bvector<Byte>&);
    BentleyStatus ReadGlyphData(bvector<Byte>&, size_t offset, size_t size);
    BentleyStatus ReadGlyphDataOffsets(bvector<Byte>&);
    BentleyStatus ReadGlyphHeaders(bvector<Byte>&);

    RscFont(Utf8CP name, FontDbCR db) : DbFont(FontType::Rsc, name, db) {}
    virtual ~RscFont();
    BentleyStatus LayoutGlyphs(GlyphLayoutResultR, GlyphLayoutContextCR)  override;
    bool CanDrawWithLineWeight() override { return true; }
    DbGlyphCP FindGlyphCP(uint32_t glyphId, FaceStyle)  override { return FindRscGlyph((T_Id) glyphId); }
    double GetDescenderRatio(FaceStyle) override {
        LoadGlyphs();
        return m_descenderRatio;
    }
};

/** An SHX DbFont. */
struct ShxFont : DbFont {
    enum class ShxType { Invalid, Unicode, Locale };
    struct GlyphFPos {
        uint32_t m_dataOffset;
        uint32_t m_dataSize;
    };

    typedef uint16_t T_Id;
private:
    typedef bmap<T_Id, DbGlyphCP> T_GlyphCache;
    typedef bmap<uint32_t, ShxFont::GlyphFPos> T_GlyphFPosCache;
    T_GlyphFPosCache m_glyphFPosCache;
    T_GlyphCache m_glyphCache;
    bool m_hasLoadedGlyphFPosCacheAndMetrics = false;
    Byte m_ascender = 0;
    Byte m_descender = 0;
public:
    uint32_t Tell() { return GetReader().m_reader->GetPos(); }
    BentleyStatus Seek(uint32_t pos);
    void Skip(uint32_t nBytes) { GetReader().m_reader->Skip(nBytes); }

    FontFaceCR GetFace() { return *GetReader().m_face; }
    void LoadGlyphFPosCacheAndMetrics();
    void LoadNonUnicodeGlyphFPosCacheAndMetrics();
    void LoadUnicodeGlyphFPosCacheAndMetrics();

    ShxType GetShxType();
    GlyphFPos const* GetGlyphFPos(T_Id);
    Byte GetAscender() { LoadGlyphFPosCacheAndMetrics(); return m_ascender; }
    Byte GetDescender() { LoadGlyphFPosCacheAndMetrics(); return m_descender; }
    uint32_t Read(void* buffer, uint32_t size);
    char GetNextChar()  { char next; Read(&next, sizeof(next)); return next; }
    uint16_t GetNextUInt16()  { uint16_t next; Read(&next, sizeof(next)); return next; }

    DbGlyphCP FindShxGlyph(T_Id);
    T_Id Ucs4CharToFontChar(uint32_t, CharCP codePageString, bvector<Byte>& localeBuffer);
    bvector<T_Id> Utf8ToFontChars(Utf8StringCR);

    ShxFont(Utf8CP name, FontDbCR db) : DbFont(FontType::Shx, name, db) {}
    virtual ~ShxFont();
    BentleyStatus LayoutGlyphs(GlyphLayoutResultR, GlyphLayoutContextCR)  override;
    bool CanDrawWithLineWeight() override { return true; }
    DGNPLATFORM_EXPORT static ShxType ValidateHeader(CharCP);
    ShxType GetShxType() const;
    DbGlyphCP FindGlyphCP(uint32_t glyphId, FaceStyle)  override { return FindShxGlyph((T_Id) glyphId); }
    double GetDescenderRatio(FaceStyle) override { return ((double)GetAscender() / (double)GetDescender()); }
};

/**
 * An external TrueType file, *used only for embedding*.
 * A single truetype file may hold multiple faces (i.e. a familyName and a FaceStyle), potentially for multiple familyNames.
 * In fact, a face within a truetype file may have multiple familyName aliases, so the same face (and all its
 * glyphs, etc.) may be loaded by different names and will be shared.
 * @note When a truetype file is embedded in a WorkspaceDb, we record all the applicable faces. Attempting to embed a a truetype file with *any*
 * face that is already serviced by another truetype file will cause the entire file to be rejected.
 */
struct TrueTypeFile {
    Utf8String m_fileName;
    bool m_compress;
    TrueTypeFile(Utf8CP fileName, bool compress) : m_fileName(fileName), m_compress(compress) {}
    DGNPLATFORM_EXPORT bool HasFamily(Utf8CP familyName);
    DGNPLATFORM_EXPORT bool Embed(FontDbR);
};

/**
 * Used only to find and embed a font family from the "system" fonts.
 * @note Currently only implemented for Windows and does nothing on other platforms.
 */
struct SystemTrueTypeFont {
    Utf8String m_familyName;
    bool m_compress;
    SystemTrueTypeFont(Utf8CP familyName, bool compress) : m_familyName(familyName), m_compress(compress) {}
    bool EmbedFace(FontDbR, FaceStyle style);
    DGNPLATFORM_EXPORT bool Embed(FontDbR);
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
