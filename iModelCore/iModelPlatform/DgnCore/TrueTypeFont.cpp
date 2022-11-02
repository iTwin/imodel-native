/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#if defined (BENTLEY_WIN32)
    #define UNICODE
    #include <Windows.h>
#endif

#include <DgnPlatformInternal.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H
#include FT_OUTLINE_H
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_IDS_H

#if !defined (BENTLEY_WIN32)
    #define TT_POLYGON_TYPE 24
    #define TT_PRIM_LINE 1
    #define TT_PRIM_QSPLINE 2
    #define TT_PRIM_CSPLINE 3

    struct FIXED {
        uint16_t fract;
        int16_t value;
    };

    struct POINTFX {
        FIXED x;
        FIXED y;
    };

    struct TTPOLYGONHEADER {
        uint32_t cb;
        uint32_t dwType;
        POINTFX pfxStart;
    };

    struct TTPOLYCURVE {
        uint16_t wType;
        uint16_t cpfx;
        POINTFX apfx[1];
    };
#endif

typedef struct FT_FaceRec_* FT_FaceP;
typedef struct FT_LibraryRec_* FT_LibraryP;

/** Get the FreeType library object. */
static FT_LibraryP getFreeTypeLibrary() {
    static bool s_triedToLoad;
    static FT_LibraryP s_ftLib = nullptr;

    FontManager::FlagHolder lock(s_triedToLoad);
    if (!lock) {
        FT_Error loadStatus = FT_Init_FreeType(&s_ftLib);
        if (nullptr == s_ftLib || FT_Err_Ok != loadStatus) {
            BeAssert(false);
        }
    }

    return s_ftLib;
}

/**
 * Adapter for FreeType for reading font streams from a FontDb. This is used whether the font is compressed,
 * and is therefore stored in a ByteStream, or uncompressed and read directly from the BlobIO on demand.
 */
struct FreeTypeFaceStream {
    FT_FaceP m_ftFace = nullptr;
    FT_StreamRec m_stream;
    bool IsValid() const { return nullptr != m_ftFace; }
	static void CloseFunc(FT_Stream  stream) { } // no need to do anything here

    // this method is called by FreeType when it needs data
    static unsigned long ReadFunc(FT_Stream stream, unsigned long offset, unsigned char* buffer, unsigned long count) {
        if (count == 0)
            return 0;
        auto reader = (FontDbReaderP) stream->descriptor.pointer;
        BeAssert(nullptr != reader);
        reader->SetPos(offset);
        return reader->ReadBytes(buffer, count) ? count : 0;
    }

    // create a FreeTypeFaceStream from a faceReader
    FreeTypeFaceStream(ReaderForFaceR faceReader) {
        m_stream = {0};
        m_stream.size = faceReader.m_reader->GetNumBytes();
        m_stream.descriptor.pointer = faceReader.m_reader;
        m_stream.read = ReadFunc;
        m_stream.close = CloseFunc;

        FT_Open_Args args = {0};
        args.flags = FT_OPEN_STREAM;
        args.stream = &m_stream;
        if (FT_Err_Ok != FT_Open_Face(getFreeTypeLibrary(), &args, faceReader.m_face->m_subId, &m_ftFace))
            m_ftFace = nullptr;
    }

    // create a FreeTypeFaceStream for a file
    FreeTypeFaceStream(Utf8CP path, int faceIndex) {
        FT_New_Face(getFreeTypeLibrary(), path, faceIndex, &m_ftFace);
    }

    ~FreeTypeFaceStream() {
        if (nullptr != m_ftFace)
            FT_Done_Face(m_ftFace);
    }
};

/**
 * Initialize a TrueTypeFace from a ReaderForFace
 */
void TrueTypeFont::TrueTypeFace::Initialize(ReaderForFaceR reader) {
    FontManager::FlagHolder lock(m_initialized);
    if (lock)
        return;

    m_ftFaceStream = new FreeTypeFaceStream(reader);
    if (!m_ftFaceStream->IsValid())
        return;

    auto ftFace = m_ftFaceStream->m_ftFace;

#ifdef FONT_DESIGNER_SCALE
    /*
        TrueType fonts (vs. RSC and SHX) have many metrics determining glyph scaling and line spacing.
        DgnV8 has classically ignored these in favor of attempting to make TrueType act more like legacy RSC and SHX systems.
        This is a nice crash course: http://chanae.walon.org/pub/ttf/ttf_glyphs.htm
        Similar: http://www.freetype.org/freetype2/docs/glyphs/glyphs-3.html
        Important fields on FT_FaceRec:
            units_per_EM - The height of this box should be mapped to the desired physical height of the text. This means virtually all glyphs will be notably smaller than the indicated height.
            height - The intended height from one baseline to the next; a 1.0 line spacing if you will. Could also think of this as a visually padded box around the text.
            ascender - The general distance of ascenders in the font. Useful with ratios.
            descender - The general distance of descenders in the font. Useful with ratios.
            line gap (derived as height - ascender - descender) - the residual distance between the EM box and the line height.
        All of the above is easy to correlate in Word, for example. What is harder to find documentation for is how to distribute the line gap above and below the EM box.
        Empirically, this seems distributed as a ratio of ascender/units_per_EM and descender/units_per_EM.
        While utilizing all of this is well and good, it makes TrueType act unlike RSC and SHX, and makes DgnV8 import more difficult.
        We should perhaps consider allowing this for "annotation" text (vs. "physical text").
    */
    m_pixelScale = face->units_per_EM;
#else // Legacy DgnV8 scaling
    FT_Short effectiveAscender = ftFace->ascender;

    if (FT_Err_Ok == FT_Load_Char(ftFace, L'A', FT_LOAD_NO_SCALE))
        effectiveAscender = (FT_Short)abs(ftFace->glyph->metrics.horiBearingY);

    m_pixelScale = (FT_UInt)(ftFace->units_per_EM * (ftFace->units_per_EM / (double)effectiveAscender));
#endif

    if (FT_Err_Ok != FT_Set_Pixel_Sizes(ftFace, m_pixelScale, 0))
        Destroy();
}

/** Load a face for a TrueTypeFont */
TrueTypeFont::TrueTypeFace& TrueTypeFont::LoadFace(FaceStyle style) {
    BeMutexHolder lock(FontManager::GetMutex());

    auto& reader = GetReader(style);
    switch (reader.m_face->m_faceStyle) {
    case FaceStyle::Bold:
        m_ftBold.Initialize(reader);
        return m_ftBold;
    case FaceStyle::BoldItalic:
        m_ftBoldItalic.Initialize(reader);
        return m_ftBoldItalic;
    case FaceStyle::Italic:
        m_ftItalic.Initialize(reader);
        return m_ftItalic;
    }
    m_ftRegular.Initialize(reader);
    return m_ftRegular;
}

/*---------------------------------------------------------------------------------**/ /**
 * @bsimethod
 +---------------+---------------+---------------+---------------+---------------+------*/
void TrueTypeFont::TrueTypeFace::Destroy() {
    BeMutexHolder lock(FontManager::GetMutex());
    DELETE_AND_CLEAR(m_ftFaceStream);
}

/**
 * Dtor for TrueTypeFace. Destroys FreeType face and deletes all cached glyphs.
 */
TrueTypeFont::TrueTypeFace::~TrueTypeFace() {
    BeMutexHolder lock(FontManager::GetMutex());
    Destroy();
    for (auto& entry : m_glyphCache)
        DELETE_AND_CLEAR(entry.second);
}

/**
 * A Glyph within a TrueTypeFace
 */
struct TrueTypeGlyph : DbGlyph {
private:
    TrueTypeFont::TrueTypeFace& m_face;
    uint32_t m_glyphIndex;
    mutable bool m_isRangeValid = false;
    mutable bool m_isExactRangeValid = false;
    mutable enum { IS_BLANK_Untested,
                   IS_BLANK_Yes,
                   IS_BLANK_No } m_isBlank = IS_BLANK_Untested;
    mutable DRange2d m_range;
    mutable DRange2d m_exactRange;

public:
    TrueTypeGlyph(TrueTypeFont::TrueTypeFace& face, uint32_t glyphIndex) : m_face(face), m_glyphIndex(glyphIndex) {}
    uint32_t GetId() const override { return m_glyphIndex; }
    DRange2d GetRange() const override;
    DRange2d GetExactRange() const override;
    CurveVectorPtr GetCurveVector() const override;
    bool IsBlank() const override;
    TrueTypeFont::TrueTypeFace& GetFace() const { return m_face; }
    DoFixup _DoFixup() const override { return DoFixup::Always; }
    RasterStatus _GetRaster(Render::ImageR) const override;
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbGlyph::RasterStatus TrueTypeGlyph::_GetRaster(Render::ImageR img) const {
    return m_face.Execute([&](FtFaceStreamP ftStream) {
        auto ftFace = ftStream->m_ftFace;
        static const int s_pixelSize = 48;

        if (FT_Err_Ok != FT_Set_Pixel_Sizes(ftFace, s_pixelSize, 0))
            return RasterStatus::CannotRenderGlyph;

        if (FT_Err_Ok != FT_Load_Glyph(ftFace, m_glyphIndex, FT_LOAD_DEFAULT))
            return RasterStatus::CannotLoadGlyph;

        if (FT_Err_Ok != FT_Render_Glyph(ftFace->glyph, FT_RENDER_MODE_NORMAL /*256 gray levels with AA*/))
            return RasterStatus::CannotRenderGlyph;

        // Produce an image with white pixels and alpha
        FT_Bitmap* ftBmp = &ftFace->glyph->bitmap;
        uint32_t outputWidth = ftBmp->width;
        uint32_t outputHeight = ftBmp->rows;
        uint32_t outputSize = outputWidth * 4 * outputHeight;
        ByteStream bytes(outputSize > 0 ? outputSize : 4 * 4); // width is number of pixels in a bitmap row
        memset(bytes.GetDataP(), 0, bytes.GetSize());
        for (uint32_t by = 0; by < ftBmp->rows; by++) {
            for (uint32_t bx = 0; bx < ftBmp->width; bx++) {
                size_t ftIndex = by * ftBmp->width + bx;
                size_t bsIndex = (outputHeight - 1 - by) * outputWidth * 4 + bx * 4; // flip Y for GLES output (could do later)
                bytes[bsIndex] = bytes[bsIndex + 1] = bytes[bsIndex + 2] = 0xff;
                bytes[bsIndex + 3] = ftBmp->buffer[ftIndex];
            }
        }

        if (FT_Err_Ok != FT_Set_Pixel_Sizes(ftFace, m_face.GetPixelScale(), 0))
            BeAssert(false);

        img = Render::Image(outputWidth, outputHeight, std::move(bytes), Render::Image::Format::Rgba);

        // possibly downsize the image if it exceeds the maximum expected size in either dimension
        if (outputWidth > s_pixelSize || outputHeight > s_pixelSize)
            {
            if (outputWidth > outputHeight) // xPrimary
                {
                double reduceScale = static_cast<double>(s_pixelSize) / static_cast<double>(outputWidth);
                outputWidth = static_cast<uint32_t>(s_pixelSize);
                outputHeight = static_cast<uint32_t>(std::floor(outputHeight * reduceScale));
                }
            else // yPrimary
                {
                double reduceScale = static_cast<double>(s_pixelSize) / static_cast<double>(outputHeight);
                outputWidth = static_cast<uint32_t>(std::floor(outputWidth * reduceScale));
                outputHeight = static_cast<uint32_t>(s_pixelSize);
                }
            img = Render::Image::Scale(img, outputWidth, outputHeight);
            }

        return RasterStatus::Success;
    });
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DRange2d TrueTypeGlyph::GetRange() const {
    FontManager::FlagHolder flagLock(m_isRangeValid);
    if (!flagLock) {
        m_face.Execute([&](FtFaceStreamP ftStream) {
            auto ftFace = ftStream->m_ftFace;
            if (FT_Err_Ok != FT_Load_Glyph(ftFace, m_glyphIndex, FT_LOAD_DEFAULT)) {
                m_range.low.Zero();
                m_range.high.Zero();
            } else {
                m_range.low.x = 0.0;
                m_range.low.y = 0.0;
                m_range.high.x = (ftFace->glyph->metrics.horiAdvance / (64.0 * (double)ftFace->units_per_EM));
                m_range.high.y = 1.0;
            }
        });
    };

    return m_range;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DRange2d TrueTypeGlyph::GetExactRange() const {
    FontManager::FlagHolder flagLock(m_isExactRangeValid);
    if (!flagLock) {
        m_face.Execute([&](FtFaceStreamP ftStream) {
            auto ftFace = ftStream->m_ftFace;
            if (FT_Err_Ok != FT_Load_Glyph(ftFace, m_glyphIndex, FT_LOAD_DEFAULT)) {
                m_exactRange.low.Zero();
                m_exactRange.high.Zero();
            } else {
                m_exactRange.low.x = (ftFace->glyph->metrics.horiBearingX / (64.0 * (double)ftFace->units_per_EM));
                m_exactRange.high.y = (ftFace->glyph->metrics.horiBearingY / (64.0 * (double)ftFace->units_per_EM));
                m_exactRange.low.y = (m_exactRange.high.y - (ftFace->glyph->metrics.height / (64.0 * (double)ftFace->units_per_EM)));
                m_exactRange.high.x = (m_exactRange.low.x + (ftFace->glyph->metrics.width / (64.0 * (double)ftFace->units_per_EM)));
            }
        });
    };

    return m_exactRange;
}


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static FIXED ftPosToFIXED(FT_Pos ftPos) {
    double d = (ftPos / 64.0);
    long l = (long)(d * 65536L);
    return *(FIXED*)&l;
}

//=======================================================================================
// @bsiclass
//=======================================================================================
struct FTOutlineParseState {
    Byte* m_buffer;
    size_t m_bufferSize;
    intptr_t m_bufferOffset;
    TTPOLYGONHEADER* m_currentContour;
    POINTFX m_lastContourPoint;
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static int decomposeConicTo(FT_Vector const* ftVecControl, FT_Vector const* ftVecTo, void* args) {
    FTOutlineParseState* parseState = static_cast<FTOutlineParseState*>(args);
    if (NULL == parseState->m_buffer) {
        parseState->m_bufferSize += (2 * sizeof(uint16_t /*WORD*/));
        parseState->m_bufferSize += (2 * sizeof(POINTFX));
        return 0;
    }

    TTPOLYCURVE* currCurve = reinterpret_cast<TTPOLYCURVE*>(parseState->m_buffer + parseState->m_bufferOffset);
    currCurve->wType = TT_PRIM_QSPLINE;
    currCurve->cpfx = 2;

    parseState->m_bufferOffset += (sizeof(currCurve->wType) + sizeof(currCurve->cpfx));

    POINTFX* currPoint = reinterpret_cast<POINTFX*>(parseState->m_buffer + parseState->m_bufferOffset);
    currPoint->x = ftPosToFIXED(ftVecControl->x);
    currPoint->y = ftPosToFIXED(ftVecControl->y);

    ++currPoint;
    currPoint->x = ftPosToFIXED(ftVecTo->x);
    currPoint->y = ftPosToFIXED(ftVecTo->y);

    parseState->m_bufferOffset += (currCurve->cpfx * sizeof(POINTFX));
    parseState->m_currentContour->cb += ((2 * sizeof(uint16_t /*WORD*/)) + (2 * sizeof(POINTFX)));
    parseState->m_lastContourPoint.x = currPoint->x;
    parseState->m_lastContourPoint.y = currPoint->y;
    return 0;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static int decomposeCubicTo(FT_Vector const* ftVecControl1, FT_Vector const* ftVecControl2, FT_Vector const* ftVecTo, void* args) {
    FTOutlineParseState* parseState = static_cast<FTOutlineParseState*>(args);
    if (NULL == parseState->m_buffer) {
        parseState->m_bufferSize += (2 * sizeof(uint16_t /*WORD*/));
        parseState->m_bufferSize += (3 * sizeof(POINTFX));

        return 0;
    }

    TTPOLYCURVE* currCurve = reinterpret_cast<TTPOLYCURVE*>(parseState->m_buffer + parseState->m_bufferOffset);
    currCurve->wType = TT_PRIM_CSPLINE;
    currCurve->cpfx = 3;

    parseState->m_bufferOffset += (sizeof(currCurve->wType) + sizeof(currCurve->cpfx));

    POINTFX* currPoint = reinterpret_cast<POINTFX*>(parseState->m_buffer + parseState->m_bufferOffset);
    currPoint->x = ftPosToFIXED(ftVecControl1->x);
    currPoint->y = ftPosToFIXED(ftVecControl1->y);

    ++currPoint;
    currPoint->x = ftPosToFIXED(ftVecControl2->x);
    currPoint->y = ftPosToFIXED(ftVecControl2->y);

    ++currPoint;
    currPoint->x = ftPosToFIXED(ftVecTo->x);
    currPoint->y = ftPosToFIXED(ftVecTo->y);

    parseState->m_bufferOffset += currCurve->cpfx * sizeof(POINTFX);
    parseState->m_currentContour->cb += ((2 * sizeof(uint16_t /*WORD*/)) + (3 * sizeof(POINTFX)));
    parseState->m_lastContourPoint.x = currPoint->x;
    parseState->m_lastContourPoint.y = currPoint->y;

    return 0;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static int decomposeLineTo(FT_Vector const* ftVecTo, void* args) {
    FTOutlineParseState* parseState = static_cast<FTOutlineParseState*>(args);
    if (NULL == parseState->m_buffer) {
        parseState->m_bufferSize += (2 * sizeof(uint16_t /*WORD*/));
        parseState->m_bufferSize += (2 * sizeof(POINTFX));
        return 0;
    }

    TTPOLYCURVE* currCurve = reinterpret_cast<TTPOLYCURVE*>(parseState->m_buffer + parseState->m_bufferOffset);
    currCurve->wType = TT_PRIM_LINE;
    currCurve->cpfx = 2;

    parseState->m_bufferOffset += (sizeof(currCurve->wType) + sizeof(currCurve->cpfx));

    POINTFX* currPoint = reinterpret_cast<POINTFX*>(parseState->m_buffer + parseState->m_bufferOffset);
    currPoint->x = parseState->m_lastContourPoint.x;
    currPoint->y = parseState->m_lastContourPoint.y;

    ++currPoint;
    currPoint->x = ftPosToFIXED(ftVecTo->x);
    currPoint->y = ftPosToFIXED(ftVecTo->y);

    parseState->m_bufferOffset += currCurve->cpfx * sizeof(POINTFX);
    parseState->m_currentContour->cb += ((2 * sizeof(uint16_t /*WORD*/)) + (2 * sizeof(POINTFX)));
    parseState->m_lastContourPoint.x = currPoint->x;
    parseState->m_lastContourPoint.y = currPoint->y;
    return 0;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int decomposeeMoveTo(FT_Vector const* ftVecTo, void* args) {
    FTOutlineParseState* parseState = static_cast<FTOutlineParseState*>(args);
    if (NULL == parseState->m_buffer) {
        parseState->m_bufferSize += sizeof(TTPOLYGONHEADER);
        return 0;
    }

    TTPOLYGONHEADER* currCountour = reinterpret_cast<TTPOLYGONHEADER*>(parseState->m_buffer + parseState->m_bufferOffset);
    currCountour->cb = sizeof(TTPOLYGONHEADER);
    currCountour->dwType = TT_POLYGON_TYPE;
    currCountour->pfxStart.x = ftPosToFIXED(ftVecTo->x);
    currCountour->pfxStart.y = ftPosToFIXED(ftVecTo->y);

    parseState->m_bufferOffset += sizeof(TTPOLYGONHEADER);
    parseState->m_currentContour = currCountour;
    parseState->m_lastContourPoint.x = currCountour->pfxStart.x;
    parseState->m_lastContourPoint.y = currCountour->pfxStart.y;
    return 0;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static size_t decomposeOutline(FT_Outline& outline, Byte* buffer, size_t bufferSize) {
    FT_Outline_Funcs outlineFuncs;
    memset(&outlineFuncs, 0, sizeof(outlineFuncs));
    outlineFuncs.conic_to = (FT_Outline_ConicToFunc)&decomposeConicTo;
    outlineFuncs.line_to = (FT_Outline_LineToFunc)&decomposeLineTo;
    outlineFuncs.cubic_to = (FT_Outline_CubicToFunc)&decomposeCubicTo;
    outlineFuncs.move_to = (FT_Outline_MoveToFunc)&decomposeeMoveTo;

    FTOutlineParseState parseState;
    memset(&parseState, 0, sizeof(parseState));

    parseState.m_buffer = buffer;
    parseState.m_bufferSize = bufferSize;

    if (FT_Err_Ok != FT_Outline_Decompose(&outline, &outlineFuncs, &parseState))
        return 0;

    return parseState.m_bufferSize;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static void appendPoint(bvector<DPoint3d>& points, POINTFX* pPoint, double npcScale) {
    DPoint3d pt;
    pt.x = (((double)(pPoint->x.value + (double)pPoint->x.fract / 65536.0)) / npcScale);
    pt.y = (((double)(pPoint->y.value + (double)pPoint->y.fract / 65536.0)) / npcScale);
    pt.z = 0.0;
    points.push_back(pt);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static CurveVectorPtr convertNativeGlyphToCurveVector(TTPOLYGONHEADER* lpHeader, size_t size, double npcScale) {
    TTPOLYGONHEADER* lpStart = lpHeader;
    TTPOLYCURVE* lpCurve;
    int i = 0;
    bvector<DPoint3d> points;
    bvector<CurveVectorPtr> loops;
    while ((uintptr_t)lpHeader < (uintptr_t)(((char*)lpStart) + size)) {
        if (TT_POLYGON_TYPE == lpHeader->dwType) {
            // Get to first curve.
            lpCurve = (TTPOLYCURVE*)(lpHeader + 1);
            auto loop = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer);

            while ((uintptr_t)lpCurve < (uintptr_t)(((char*)lpHeader) + lpHeader->cb)) {
                // Format assumption:
                //  The bytes immediately preceding a POLYCURVE structure contain a valid POINTFX.
                //  If this is first curve, this points to the pfxStart of the POLYGONHEADER. Otherwise, this points to the last point of the previous POLYCURVE.
                //  In either case, this is representative of the previous curve's last point.
                if (TT_PRIM_LINE == lpCurve->wType) {
                    points.clear();
                    appendPoint(points, (POINTFX*)((char*)lpCurve - sizeof(POINTFX)), npcScale);
                    for (i = 0; i < lpCurve->cpfx; ++i) {
                        appendPoint(points, &lpCurve->apfx[i], npcScale);
                    }
                    loop->Add(ICurvePrimitive::CreateLineString(points));
                } else if (lpCurve->wType == TT_PRIM_QSPLINE) {
                    int order = 3; // always quadratic !!
                    points.clear();
                    appendPoint(points, (POINTFX*)((char*)lpCurve - sizeof(POINTFX)), npcScale);
                    for (i = 0; i < lpCurve->cpfx; ++i)
                        appendPoint(points, &lpCurve->apfx[i], npcScale);
                    if (points.size() >= order)
                        loop->Add(ICurvePrimitive::CreateBsplineCurve(
                            MSBsplineCurve::CreateFromPolesAndOrder(
                                points, nullptr, nullptr, order, false)));
                }

                // Move on to next curve.
                lpCurve = (TTPOLYCURVE*)&(lpCurve->apfx[i]);
            }
            loop->ConsolidateAdjacentPrimitives();
            loops.push_back(loop);
            // Move on to next polygon.
            lpHeader = (TTPOLYGONHEADER*)(((char*)lpHeader) + lpHeader->cb);
        } else {
            break;
        }
    }
    if (loops.size() == 0)
        return CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);
    if (loops.size() == 1)
        return loops[0];
    auto parityRegion = CurveVector::Create(CurveVector::BOUNDARY_TYPE_ParityRegion);
    for (auto& loop : loops)
        parityRegion->Add(loop);

    parityRegion->FixupXYOuterInner();

    return parityRegion;
}

/**
 *
 */
CurveVectorPtr TrueTypeGlyph::GetCurveVector() const {
    return m_face.Execute([&](FtFaceStreamP ftStream) {
        auto ftFace = ftStream->m_ftFace;
        if (FT_Err_Ok != FT_Load_Glyph(ftFace, m_glyphIndex, FT_LOAD_DEFAULT))
            return CurveVectorPtr();
        size_t dataSize = decomposeOutline(ftFace->glyph->outline, nullptr, 0);
        if (0 == dataSize)
            return CurveVector::Create(CurveVector::BOUNDARY_TYPE_None); // empty !

        bvector<Byte> data;
        data.resize(dataSize);
        decomposeOutline(ftFace->glyph->outline, &data[0], dataSize);

        return convertNativeGlyphToCurveVector(reinterpret_cast<TTPOLYGONHEADER*>(&data[0]), data.size(), ftFace->units_per_EM);
    });
}

/**
 * Determine whether this is a blank glyph
 */
bool TrueTypeGlyph::IsBlank() const {
    if (IS_BLANK_Untested == m_isBlank)
        m_isBlank = ((GetExactRange().XLength() <= 0.0) ? IS_BLANK_Yes : IS_BLANK_No);

    return (IS_BLANK_Yes == m_isBlank);
}

/**
 * Find a glyph within a TrueTypeFace
 */
DbGlyphCP TrueTypeFont::FindGlyphCP(T_Id glyphId, FaceStyle style) {
    BeMutexHolder lock(FontManager::GetMutex());

    auto& face = LoadFace(style);
    auto foundGlyph = face.m_glyphCache.find(glyphId);
    if (face.m_glyphCache.end() != foundGlyph)
        return foundGlyph->second;

    DbGlyphP glyph = new TrueTypeGlyph(face, glyphId);
    face.m_glyphCache.Insert(glyphId, glyph);
    return glyph;
}

/**
 *
 */
BentleyStatus TrueTypeFont::TrueTypeFace::ComputeAdvanceWidths(T_DoubleVectorR advanceWidths, uint32_t const* ucs4Chars, size_t numChars) {
    // Currently only supports unidirectional, left-to-right text. Need to consider libraries like harfbuzz, pango, and cairo for complex and bidirectional scripts.
    return Execute([&](FtFaceStreamP ftStream) {
        auto ftFace = ftStream->m_ftFace;
        for (size_t iChar = 0; iChar < numChars; ++iChar) {
            FT_UInt glyphIndex = FT_Get_Char_Index(ftFace, ucs4Chars[iChar]);
            if (0 == glyphIndex) {
                advanceWidths.push_back(0.0);
                continue;
            }

            if (FT_Err_Ok != FT_Load_Glyph(ftFace, glyphIndex, FT_LOAD_DEFAULT)) {
                advanceWidths.push_back(0.0);
                continue;
            }

            FT_Vector kerning;
            FT_UInt nextGlyphIndex = (((numChars - 1) == iChar) ? 0 : FT_Get_Char_Index(ftFace, ucs4Chars[iChar + 1]));

            if ((0 == nextGlyphIndex) || (FT_Err_Ok != FT_Get_Kerning(ftFace, glyphIndex, nextGlyphIndex, FT_KERNING_DEFAULT, &kerning)))
                memset(&kerning, 0, sizeof(kerning));

            // Freetype normally exposes lengths as "26.6 fractional pixel" units. This means they're integers made of a 26-bit integer mantissa, and a 6-bit fractional part. In other words, all coordinates are multiplied by 64.
            // This allows an integer to represent sub-pixel positions. The goal of this function is to operate in a 0..1 nominal space, so divide.
            advanceWidths.push_back((ftFace->glyph->advance.x / (64.0 * (double)ftFace->units_per_EM)) + (kerning.x / (64.0 * (double)ftFace->units_per_EM)));
        }

        return SUCCESS;
    });
}

/**
 *
 */
BentleyStatus TrueTypeFont::LayoutGlyphs(GlyphLayoutResultR result, GlyphLayoutContextCR context) {
    // Determine the best face data to use.
    auto style = FontManager::FaceStyleFromBoldItalic(context.m_isBold, context.m_isItalic);
    auto& face = LoadFace(style);

    // UTF-8 is multi-byte; need to figure out each UCS "character" so we can look up the glyph.
    bvector<Byte> ucs4CharsBuffer;
    size_t numUcs4Chars = 0;
    uint32_t const* ucs4Chars = DbFont::Utf8ToUcs4(ucs4CharsBuffer, numUcs4Chars, context.m_string);
    if (0 == numUcs4Chars)
        return SUCCESS;

    // Compute the advance widths.
    T_DoubleVector widths;
    widths.reserve(numUcs4Chars);
    if (SUCCESS != face.ComputeAdvanceWidths(widths, ucs4Chars, numUcs4Chars))
        return ERROR;

    // Acquire the glyphs.
    // We need a 1:1 correlation between widths and glyphs, so this means we can insert null glyphs.
    result.m_glyphs.reserve(numUcs4Chars);
    face.Execute([&](FtFaceStreamP ftStream) {
        auto ftFace = ftStream->m_ftFace;
        for (size_t iGlyph = 0; iGlyph < numUcs4Chars; ++iGlyph)
            result.m_glyphs.push_back(FindGlyphCP(FT_Get_Char_Index(ftFace, ucs4Chars[iGlyph]), style));
    });

    // Right-justified text needs to ignore trailing blanks.
    size_t numNonBlankGlyphs = result.m_glyphs.size();
    for (; numNonBlankGlyphs > 0; --numNonBlankGlyphs) {
        DbGlyphCR glyph = *result.m_glyphs[numNonBlankGlyphs - 1];
        if (!glyph.IsBlank())
            break;
    }

    // Compute origins, ranges, etc...
    DPoint2d penPosition = DPoint2d::FromZero();
    result.m_glyphOrigins.reserve(result.m_glyphs.size());
    for (size_t iGlyph = 0; iGlyph < result.m_glyphs.size(); ++iGlyph) {
        DbGlyphCP glyph = result.m_glyphs[iGlyph];
        if (nullptr == glyph)
            continue;

        result.m_glyphOrigins.push_back(DPoint3d::From(penPosition));

        DRange2d glyphRange = glyph->GetRange();
        DbFont::ScaleAndOffsetGlyphRange(glyphRange, context.m_drawSize, penPosition);

        DRange2d glyphExactRange = glyph->GetExactRange();
        DbFont::ScaleAndOffsetGlyphRange(glyphExactRange, context.m_drawSize, penPosition);

        // It is important to use the array overload of DRange2d::Extend; the ranges can be inverted for backwards/upside-down text, and the DRange2d overload enforces low/high semantics.
        result.m_range.Extend(&glyphRange.low, 2);
        result.m_exactRange.Extend(&glyphExactRange.low, 2);

        if (iGlyph < numNonBlankGlyphs)
            result.m_justificationRange.Extend(&glyphRange.low, 2);

        penPosition.x += (widths[iGlyph] * context.m_drawSize.x);
    }

    result.ZeroNullRanges();
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
double TrueTypeFont::GetDescenderRatio(FaceStyle style) {
    return LoadFace(style).Execute([&](FtFaceStreamP ftStream) {
        auto ftFace = ftStream->m_ftFace;
        return fabs((double)ftFace->descender / (double)ftFace->ascender);
    });
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bvector<uint32_t> TrueTypeFont::ComputeGlyphIndices(Utf8CP str, bool isBold, bool isItalic) {
    bvector<unsigned int /*FT_Uint*/> glyphIndices;

    // Determine the best face data to use.
    FaceStyle style = FontManager::FaceStyleFromBoldItalic(isBold, isItalic);
    auto& face = LoadFace(style);

    // UTF-8 is multi-byte; need to figure out each UCS "character" so we can look up the glyph.
    bvector<Byte> ucs4CharsBuffer;
    size_t numUcs4Chars = 0;
    uint32_t const* ucs4Chars = DbFont::Utf8ToUcs4(ucs4CharsBuffer, numUcs4Chars, str);
    if (0 == numUcs4Chars)
        return glyphIndices;

    // Convert to glyph indices.
    glyphIndices.reserve(numUcs4Chars);
    face.Execute([&](FtFaceStreamP ftStream) {
        auto face = ftStream->m_ftFace;
        for (size_t iGlyph = 0; iGlyph < numUcs4Chars; ++iGlyph)
            glyphIndices.push_back(FT_Get_Char_Index(face, ucs4Chars[iGlyph]));
    });

    return glyphIndices;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static BentleyStatus convertTTNameString(Utf8StringR value, FT_SfntName const& name) {
    // There are over 50 ways that strings can be encoded within a TrueType file, many of which are hard to decode nowadays.
    // Of the 1000+ TrueType files on my system, supporting the various Unicodes plus macroman achieves full coverage, so I am focusing on that.

    Utf8String srcEncoding;
    switch (name.platform_id) {
    case TT_PLATFORM_APPLE_UNICODE: {
        switch (name.encoding_id) {
        // Some fonts use "Unicode 1.* semantics". I cannot find an easy way to go back to this old Unicode, so I'm hoping for our purposes it is the same as UTF-16BE.
        case TT_APPLE_ID_DEFAULT:     // fall through
        case TT_APPLE_ID_UNICODE_1_1: // fall through
        case TT_APPLE_ID_ISO_10646:   // fall through
        case TT_APPLE_ID_UNICODE_2_0:
            srcEncoding = "UTF-16BE";
            break;
        case TT_APPLE_ID_UNICODE_32:
            srcEncoding = "UTF-32BE";
            break;
        default:
            return ERROR;
        }

        break;
    }

    case TT_PLATFORM_MACINTOSH: {
        switch (name.encoding_id) {
        case TT_MAC_ID_ROMAN:
            srcEncoding = "macroman";
            break;
        default:
            return ERROR;
        }

        break;
    }

    case TT_PLATFORM_MICROSOFT: {
        switch (name.encoding_id) {
        case TT_MS_ID_UNICODE_CS:
            srcEncoding = "UTF-16BE";
            break;
        default:
            return ERROR;
        }

        break;
    }

    default:
        return ERROR;
    }

    bvector<Byte> outBuff;
    if (SUCCESS != BeStringUtilities::TranscodeStringDirect(outBuff, "UTF-8", (ByteCP)name.string, name.string_len, srcEncoding.c_str()))
        return ERROR;

    value.assign((CharCP)&outBuff[0], outBuff.size());
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static bool isNameUnicode(FT_SfntName const& name) {
    return (TT_PLATFORM_APPLE_UNICODE == name.platform_id || (TT_PLATFORM_MICROSOFT == name.platform_id && TT_MS_ID_UNICODE_CS == name.encoding_id));
};

/** A single FreeType face, loaded from a TrueType file */
void TrueTypeFont::TrueTypeFace::Initialize(Utf8CP path, int faceIndex) {
    m_initialized = true;
    m_ftFaceStream = new FreeTypeFaceStream(path, faceIndex);
}

/** Determine whether this font face may be embedded. */
static bool hasEmbeddingRights(FT_Face face) {
    TT_OS2 const* os2Table = static_cast<TT_OS2 const*>(FT_Get_Sfnt_Table(face, ft_sfnt_os2));
    if (NULL == os2Table)
        return false;

    return (0 == (0x1 & os2Table->fsType));
}

/** Determine whether this font face may be embedded. */
bool TrueTypeFont::TrueTypeFace::HasEmbeddingRights() {
    return hasEmbeddingRights(m_ftFaceStream->m_ftFace);
}

/** Convert FreeType style flags to FaceStyle */
FaceStyle TrueTypeFont::TrueTypeFace::GetFaceStyle() {
    FaceStyle style = FaceStyle::Regular;
    switch (m_ftFaceStream->m_ftFace->style_flags) {
    case FT_STYLE_FLAG_BOLD:
        style = FaceStyle::Bold;
        break;
    case FT_STYLE_FLAG_ITALIC:
        style = FaceStyle::Italic;
        break;
    case (FT_STYLE_FLAG_BOLD | FT_STYLE_FLAG_ITALIC):
        style = FaceStyle::BoldItalic;
        break;
    }

    return style;
}

// To determine what the base/regular face of a font is, we need more information than FT_Face::family_name/style_flags/style_name.
// For example:
//
//  File        family_name             style_name
//  arial.ttf   'Arial'                 'Regular'
//  ariali.ttf  'Arial'                 'Italic'
//  arialn.ttf  'Arial'                 'Narrow'
//  arialni.ttf 'Arial'                 'Narrow Italic'
//  vera.ttf    'Bitstream Vera Sans'   'Roman'
//
// Since every font in a family reports the same broad family_name, and style_name is not standardized, there's no way to know across all families what the base font is.
// We must therefore dig into the name table for its actual family and style name. For example:
//
//  File        Font Family name(1)     Font Subfamily name(2)
//  arial.ttf   'Arial'                 'Regular'
//  ariali.ttf  'Arial'                 'Italic'
//  arialn.ttf  'Arial Narrow'          'Regular'
//  arialni.ttf 'Arial Narrow'          'Italic'
//  vera.ttf    'Bitstream Vera Sans'   'Roman'
//
// Thus, we can use name ID 1 for family to properly group fonts together.
Utf8String TrueTypeFont::TrueTypeFace::GetFamilyName() {
    if (!m_ftFaceStream->IsValid())
        return "";

    bool hasFamilyName = false;
    FT_SfntName familyName = {0};
    FT_UInt numStrings = FT_Get_Sfnt_Name_Count(m_ftFaceStream->m_ftFace);
    for (FT_UInt iString = 0; iString < numStrings; ++iString) {
        FT_SfntName thisName;
        FT_Get_Sfnt_Name(m_ftFaceStream->m_ftFace, iString, &thisName);

        // The values in the name table can have multiple definition with various encodings, targeting various languages.
        // We do not intend to support all 50+ possible ways to encode strings. I have sampled over 1000 fonts, and all can be decoded with merely a handful of schemes.
        // However, this also means we want to use a precedence so that we can use the most generous encoding possible.
        // The precedence rules below are seemingly arbitrary because I cannot find a part of the standard on how to prefer one name over another.
        // The name entries also have a language identifier that is also platform specific...
        // I would like to be language-agnostic, so arbitrarily always taking the first one encountered... I don't know how to otherwise determine precedence for language.

        if (1 != thisName.name_id)
            continue;

        if (!hasFamilyName) {
            hasFamilyName = true;
            familyName = thisName;
            continue;
        }

        bool isThisNameUnicode = isNameUnicode(thisName);
        bool isTargetNameUnicode = isNameUnicode(familyName);

        // Unicode is always better.
        if (isThisNameUnicode && !isTargetNameUnicode) {
            familyName = thisName;
            continue;
        }

        // One locale is never better than another.
        if (!isThisNameUnicode)
            continue;

        if (TT_PLATFORM_APPLE_UNICODE == thisName.platform_id && TT_PLATFORM_APPLE_UNICODE == familyName.platform_id && thisName.encoding_id > familyName.encoding_id) {
            familyName = thisName;
            continue;
        }

        if (TT_PLATFORM_MICROSOFT == thisName.platform_id && TT_MS_ID_UNICODE_CS == thisName.encoding_id && TT_PLATFORM_APPLE_UNICODE == familyName.platform_id && familyName.encoding_id < TT_APPLE_ID_UNICODE_2_0) {
            familyName = thisName;
            continue;
        }
    }

    Utf8String familyNameStr;
    if (!hasFamilyName || SUCCESS != convertTTNameString(familyNameStr, familyName))
        return m_ftFaceStream->m_ftFace->family_name;

    return familyNameStr;
}

/** return true if this file has any faces for a familyName. */
bool TrueTypeFile::HasFamily(Utf8CP familyName) {
    int numFaces = 1;
    for (int iFace = 0; iFace < numFaces; ++iFace) {
        TrueTypeFont::TrueTypeFace face;
        face.Initialize(m_fileName.c_str(), iFace);
        if (!face.m_ftFaceStream->m_ftFace)
            continue;

        if (face.GetFamilyName().EqualsI(familyName))
            return true;

        if (0 == iFace)  // use the first face to get the number of faces, yuk.
            numFaces = face.m_ftFaceStream->m_ftFace->num_faces;
    }
    return false;
}

/**
 * Embed a TrueType file in a FontDb.
 * @note TrueType files can have multiple faces and multiple families. The file is embedded, and the list of faces is stored.
 * @return true if successful
 */
bool TrueTypeFile::Embed(FontDbR fontDb) {
    BeFile ttFile;
    if (BeFileStatus::Success != ttFile.Open(m_fileName.c_str(), BeFileAccess::Read))
        return false;

    ByteStream data;
    ttFile.ReadEntireFile(data);
    ttFile.Close();

    bvector<FontFace> faces;
    int numFaces = 1;
    for (int iFace = 0; iFace < numFaces; ++iFace) {
        TrueTypeFont::TrueTypeFace face;
        face.Initialize(m_fileName.c_str(), iFace);
        if (!face.m_ftFaceStream->m_ftFace)
            continue;

        if (0 == iFace) {
            numFaces = face.m_ftFaceStream->m_ftFace->num_faces; // use the first face to get the number of faces, yuk.
            if (!fontDb.m_isWorkspace && !face.HasEmbeddingRights())
                return false; // this not a workspace, and the file says not to embed.
        }

        Utf8String familyName = face.GetFamilyName();
        if (!familyName.empty())
            faces.emplace_back(FontFace(FontType::TrueType, familyName.c_str(), face.GetFaceStyle(), nullptr, iFace));
        }

    if (faces.size() <= 0)
        return false; // probably wasn't a valid truetype file

    return SUCCESS == fontDb.EmbedFont(faces, data, m_compress);
}

/**
 *
 */
struct SystemFontReader {
    Utf8String m_familyName;
    FaceStyle m_style;
    bvector<FontFace> m_faces;
    ByteStream m_data;
    int m_faceIndex = 0;
    SystemFontReader(Utf8CP familyName, FaceStyle style) : m_familyName(familyName), m_style(style) {}
    void Load();
};

/**
 *
 */
void SystemFontReader::Load() {
#if defined (BENTLEY_WIN32)
    struct TrueTypeWinDc {
        HDC m_dc;
        TrueTypeWinDc() { m_dc = ::CreateDCW(L"DISPLAY", NULL, NULL, NULL); }
        ~TrueTypeWinDc() {
            if (nullptr != m_dc) {
                ::DeleteDC(m_dc);
            }
        }
    };

    TrueTypeWinDc fontDC;
    if (nullptr == fontDC.m_dc)
        return;

    // Select the font into the global context.
    bool isBold = (FaceStyle::Bold == m_style || FaceStyle::BoldItalic == m_style);
    bool isItalic = (FaceStyle::Italic == m_style || FaceStyle::BoldItalic == m_style);
    WString fontName(m_familyName.c_str(), BentleyCharEncoding::Utf8);

    int FONT_LOGICAL_UNITS = 2048;
    HFONT newFont = ::CreateFontW(FONT_LOGICAL_UNITS, 0, 0, 0, isBold ? FW_BOLD : FW_LIGHT, isItalic, false, false,
                                  DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH, fontName.c_str());

    if (nullptr == newFont)
        return;

    HFONT previousFont = (HFONT)::SelectObject(fontDC.m_dc, newFont);
    if (NULL != previousFont)
        ::DeleteObject(previousFont);

    // Does the system actually have the requested font? CreateFontW tries too hard to return something/anything.
    WString selectedFontName;
    int selectedFontNameCount = ::GetTextFaceW(fontDC.m_dc, 0, nullptr);
    if (selectedFontNameCount <= 0) {
        BeDataAssert(false); // failed to find face information for selected font?!
        return;
    }

    // basic_string objects maintain a NULL-terminated buffer internally; the value given to resize is the number of real characters.
    // The result of GetTextFaceW includes the terminator.
    selectedFontName.resize(selectedFontNameCount - 1);
    int res = ::GetTextFaceW(fontDC.m_dc, selectedFontNameCount, const_cast<WCharP>(selectedFontName.data()));
    if (res <= 0) {
        BeDataAssert(false); // failed to find face information for selected font?!
        return;
    }

    if (!selectedFontName.EqualsI(fontName))
        return;

    // There is no reliable way to go from HFONT to file, but we can use GetFontData to read the raw
    // bytes from the font. The dwTable parameter to GetFontData allows as follows: If this
    // parameter is zero, the information is retrieved starting at the beginning of the file for
    // TrueType font files or from the beginning of the data for the currently selected font for
    // TrueType Collection files. To retrieve the data from the beginning of the file for TrueType
    // Collection files specify 'ttcf' (0x66637474).

    // Freetype does not understand the font when data starts "from the beginning of the data for
    // the currently selected font" in a TTC file (because offsets in the font data are from the
    // beginning of the file, not the beginning of the font data, so offsets cannot be correctly
    // computed when starting in the middle of the stream). And non-TTC files do NOT have a ttcf
    // table. Therefore, we try 'ttcf' first, and then fall back to zero.

    // Care should also be taken that freetype deals in faces, not fonts... so in the TTC case, we
    // need to know which face index that Windows actually selected from the collection, so that can
    // be passed to FT_New_Memory_Face to select the proper font from the collection.

    DWORD dwTable = 'fctt'; // big endian 'ttcf'
    DWORD fontDataSize = ::GetFontData(fontDC.m_dc, dwTable, 0, nullptr, 0);
    if ((GDI_ERROR == fontDataSize) || (0 == fontDataSize)) {
        dwTable = 0;
        fontDataSize = ::GetFontData(fontDC.m_dc, dwTable, 0, nullptr, 0);
        if ((GDI_ERROR == fontDataSize) || (0 == fontDataSize))
            return;
    }

    m_data.resize((size_t)fontDataSize);
    DWORD bytesRetrieved = ::GetFontData(fontDC.m_dc, dwTable, 0, m_data.GetDataP(), fontDataSize);
    if ((GDI_ERROR == bytesRetrieved) || (0 == bytesRetrieved))
        return;

    // Compute face index if we're a TTC file.
    if (0 == dwTable) {
        m_faceIndex = 0;
    } else {
        PUSH_MSVC_IGNORE(4200) // tableDirectoryOffsets is a variable-length array that we're re-interpretting.
        struct TTCHeader {
            uint32_t ttcTag;
            uint16_t majorVersion;
            uint16_t minorVersion;
            uint32_t numFonts;
            uint32_t tableDirectoryOffsets[];
        };
        POP_MSVC_IGNORE

        auto thisFontDataSize = ::GetFontData(fontDC.m_dc, 0, 0, nullptr, 0);
        auto fontOffset = fontDataSize - thisFontDataSize;

        // !!! TT data is big-endian !!!
        auto bigEndianTTCHeader = reinterpret_cast<TTCHeader const*>(m_data.GetDataP());
        auto numFonts = _byteswap_ulong(bigEndianTTCHeader->numFonts);
        m_faceIndex = -1;
        for (size_t iOffset = 0; iOffset < numFonts; ++iOffset) {
            auto thisOffset = _byteswap_ulong(bigEndianTTCHeader->tableDirectoryOffsets[iOffset]);
            if (thisOffset == fontOffset) {
                m_faceIndex = (FT_Long)iOffset;
                break;
            }
        }

        if (m_faceIndex < 0) {
            BeAssert(false); // failed to infer face index from offset table.
            return;
        }
    }
#endif
}

/** Embed just one face of a font family */
bool SystemTrueTypeFont::EmbedFace(FontDbR fontDb, FaceStyle style) {
    SystemFontReader reader(m_familyName.c_str(), style);
    reader.Load();

    if (reader.m_data.size() <= 0)
        return false;

    FT_Face face = nullptr;
    FT_Error faceStatus = FT_New_Memory_Face(getFreeTypeLibrary(), reader.m_data.data(), (FT_Long)reader.m_data.size(), reader.m_faceIndex, &face);
    bool canEmbed = (FT_Err_Ok == faceStatus) && (nullptr != face) && hasEmbeddingRights(face);
    FT_Done_Face(face);
    if (!canEmbed)
        return false;

    reader.m_faces.emplace_back(FontFace(FontType::TrueType, m_familyName.c_str(), style, nullptr, reader.m_faceIndex));
    return SUCCESS == fontDb.EmbedFont(reader.m_faces, reader.m_data, m_compress);
}

/** attempt to embed all faces from system fonts for a font family */
bool SystemTrueTypeFont::Embed(FontDbR fontDb) {
    if (m_familyName.empty())
        return false;

    // Try to embed all; return true if any succeeded.
    return (int)EmbedFace(fontDb, FaceStyle::Regular) |
           (int)EmbedFace(fontDb, FaceStyle::Bold) |
           (int)EmbedFace(fontDb, FaceStyle::Italic) |
           (int)EmbedFace(fontDb, FaceStyle::BoldItalic);
}
