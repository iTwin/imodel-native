/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnTrueTypeFont.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnFontData.h>
#include <DgnPlatform/Render.h>
#include <regex>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
FT_Library DgnPlatformLib::Host::FontAdmin::_GetFreeTypeLibrary()
    {
    DgnFonts::FlagHolder lock(m_triedToLoadFTLibrary);
    if (!lock.IsSet())
        {
        FT_Error loadStatus = FT_Init_FreeType(&m_ftLibrary);
        if (nullptr == m_ftLibrary || FT_Err_Ok != loadStatus)
            { BeAssert(false); }
        }

    return m_ftLibrary;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
DgnFontPtr DgnTrueTypeFont::Create(Utf8CP name, IDgnFontDataP data) { return new DgnTrueTypeFont(name, data); }
DgnFontPtr DgnTrueTypeFont::_Clone() const { return new DgnTrueTypeFont(*this); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
DgnTrueTypeFont::~DgnTrueTypeFont()
    {
    for (T_GlyphCacheMap::reference cacheMapEntry : m_glyphCache)
        {
        for (T_GlyphCache::reference cacheEntry : cacheMapEntry.second)
            DELETE_AND_CLEAR(cacheEntry.second);
        }
    }

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     03/2015
//=======================================================================================
struct DgnTrueTypeGlyph : DgnGlyph
{
private:
    FreeTypeFace m_face;
    FT_UInt m_glyphIndex;
    mutable bool m_isRangeValid;
    mutable DRange2d m_range;
    mutable bool m_isExactRangeValid;
    mutable DRange2d m_exactRange;
    mutable enum { IS_BLANK_Untested, IS_BLANK_Yes, IS_BLANK_No } m_isBlank;
    
public:
    DgnTrueTypeGlyph(FreeTypeFace face, FT_UInt glyphIndex) : m_face(face), m_glyphIndex(glyphIndex), m_isRangeValid(false), m_isExactRangeValid(false), m_isBlank(IS_BLANK_Untested) {}
    T_Id _GetId() const override { return (T_Id)m_glyphIndex; }
    DRange2d _GetRange() const override;
    DRange2d _GetExactRange() const override;
    CurveVectorPtr _GetCurveVector () const override;
    bool _IsBlank() const override;
    FreeTypeFace GetFace() const { return m_face; }
    DoFixup _DoFixup () const override { return DoFixup::Always; }
    RasterStatus _GetRaster(Render::ImageR) const override;
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Mark.Schlosser  01/2018
//---------------------------------------------------------------------------------------
DgnGlyph::RasterStatus DgnTrueTypeGlyph::_GetRaster(Render::ImageR img) const
    {
    return m_face.Execute([&](FT_Face ftFace)
        {
        static const int s_pixelSize = 64;

        if (FT_Err_Ok != FT_Set_Pixel_Sizes(ftFace, s_pixelSize, 0))
            return RasterStatus::CannotRenderGlyph;

        if (FT_Err_Ok != FT_Load_Glyph(ftFace, m_glyphIndex, FT_LOAD_DEFAULT))
            return RasterStatus::CannotLoadGlyph;

        if (FT_Err_Ok != FT_Render_Glyph(ftFace->glyph, FT_RENDER_MODE_NORMAL/*256 gray levels with AA*/))
            return RasterStatus::CannotRenderGlyph;

        // Produce an image with white pixels and alpha
        FT_Bitmap* ftBmp = &ftFace->glyph->bitmap;
        uint32_t outputWidth = ftBmp->width + 2; // 1 pixel empty border around image
        uint32_t outputHeight = ftBmp->rows + 2;
        ByteStream bytes(outputWidth * outputHeight * 4); // width is number of pixels in a bitmap row
        memset(bytes.GetDataP(), 0, bytes.GetSize());
        for (uint32_t by = 0; by < ftBmp->rows; by++)
            {
            for (uint32_t bx = 0; bx < ftBmp->width; bx++)
                {
                size_t ftIndex = by * ftBmp->width + bx;
                size_t bsIndex = (outputHeight - 1 - (by + 1)) * outputWidth * 4 + (bx + 1) * 4; // flip Y for GLES output (could do later)
                bytes[bsIndex] = bytes[bsIndex+1] = bytes[bsIndex+2] = 0xff;
                bytes[bsIndex+3] = ftBmp->buffer[ftIndex];
                }
            }

        if (FT_Err_Ok != FT_Set_Pixel_Sizes(ftFace, m_face.GetPixelScale(), 0))
            BeAssert(false);

        img = Render::Image::FromResizedImage(64, 64, Render::Image(outputWidth, outputHeight, std::move(bytes), Render::Image::Format::Rgba));
        return RasterStatus::Success;
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
DRange2d DgnTrueTypeGlyph::_GetRange() const
    {
    DgnFonts::FlagHolder flagLock(m_isRangeValid);
    if (!flagLock.IsSet())
        {
        m_face.Execute([&](FT_Face ftFace)
            {
            if (FT_Err_Ok != FT_Load_Glyph(ftFace, m_glyphIndex, FT_LOAD_DEFAULT))
                {
                m_range.low.Zero();
                m_range.high.Zero();
                }
            else
                {
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
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
DRange2d DgnTrueTypeGlyph::_GetExactRange() const
    {
    DgnFonts::FlagHolder flagLock(m_isExactRangeValid);
    if (!flagLock.IsSet())
        {
        m_face.Execute([&](FT_Face ftFace)
            {
            if (FT_Err_Ok != FT_Load_Glyph(ftFace, m_glyphIndex, FT_LOAD_DEFAULT))
                {
                m_exactRange.low.Zero();
                m_exactRange.high.Zero();
                }
            else
                {
                m_exactRange.low.x = (ftFace->glyph->metrics.horiBearingX / (64.0 * (double)ftFace->units_per_EM));
                m_exactRange.high.y = (ftFace->glyph->metrics.horiBearingY / (64.0 * (double)ftFace->units_per_EM));
                m_exactRange.low.y = (m_exactRange.high.y - (ftFace->glyph->metrics.height / (64.0 * (double)ftFace->units_per_EM)));
                m_exactRange.high.x = (m_exactRange.low.x + (ftFace->glyph->metrics.width / (64.0 * (double)ftFace->units_per_EM)));
                }
            });
        };

    return m_exactRange;
    }

// WinGDI.h
#define TT_POLYGON_TYPE 24
#define TT_PRIM_LINE 1
#define TT_PRIM_QSPLINE 2
#define TT_PRIM_CSPLINE 3

// WinGDI.h
struct FIXED
{
    uint16_t fract;
    int16_t value;
};
    
// WinGDI.h
struct POINTFX
{
    FIXED x;
    FIXED y;
};

// WinGDI.h
struct TTPOLYGONHEADER
{
    uint32_t cb;
    uint32_t dwType;
    POINTFX pfxStart;
};

// WinGDI.h
struct TTPOLYCURVE
{
    uint16_t wType;
    uint16_t cpfx;
    POINTFX apfx[1];
};
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
static FIXED ftPosToFIXED(FT_Pos ftPos)
    {
    double d = (ftPos / 64.0);
    long l = (long)(d * 65536L);
    return *(FIXED*)&l;
    }

//=======================================================================================
// Adapted for DGNPlatform from Mark Schlosser's prototype.
// @bsiclass                                                    Jeff.Marker     06/2012
//=======================================================================================
struct FTOutlineParseState
{
    Byte* m_buffer;
    size_t m_bufferSize;
    intptr_t m_bufferOffset;
    TTPOLYGONHEADER* m_currentContour;
    POINTFX m_lastContourPoint;
};

//---------------------------------------------------------------------------------------
// Adapted for DGNPlatform from Mark Schlosser's prototype.
// @bsimethod                                                   Jeff.Marker     06/2012
//---------------------------------------------------------------------------------------
static int decomposeConicTo(FT_Vector const* ftVecControl, FT_Vector const* ftVecTo, void* args)
    {
    FTOutlineParseState* parseState = static_cast<FTOutlineParseState*>(args);
    if (NULL == parseState->m_buffer)
        {
        parseState->m_bufferSize += (2 * sizeof(uint16_t/*WORD*/));
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
    parseState->m_currentContour->cb += ((2 * sizeof(uint16_t/*WORD*/)) + (2 * sizeof(POINTFX)));
    parseState->m_lastContourPoint.x = currPoint->x;
    parseState->m_lastContourPoint.y = currPoint->y;

    return 0;
    }

//---------------------------------------------------------------------------------------
// Adapted for DGNPlatform from Mark Schlosser's prototype.
// @bsimethod                                                   Jeff.Marker     06/2012
//---------------------------------------------------------------------------------------
static int decomposeCubicTo(FT_Vector const* ftVecControl1, FT_Vector const* ftVecControl2, FT_Vector const* ftVecTo, void* args)
    {
    FTOutlineParseState* parseState = static_cast<FTOutlineParseState*>(args);
    if (NULL == parseState->m_buffer)
        {
        parseState->m_bufferSize += (2 * sizeof(uint16_t/*WORD*/));
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
    parseState->m_currentContour->cb += ((2 * sizeof(uint16_t/*WORD*/)) + (3 * sizeof(POINTFX)));
    parseState->m_lastContourPoint.x = currPoint->x;
    parseState->m_lastContourPoint.y = currPoint->y;

    return 0;
    }

//---------------------------------------------------------------------------------------
// Adapted for DGNPlatform from Mark Schlosser's prototype.
// @bsimethod                                                   Jeff.Marker     06/2012
//---------------------------------------------------------------------------------------
static int decomposeLineTo(FT_Vector const* ftVecTo, void* args)
    {
    FTOutlineParseState* parseState = static_cast<FTOutlineParseState*>(args);
    if (NULL == parseState->m_buffer)
        {
        parseState->m_bufferSize += (2 * sizeof(uint16_t/*WORD*/));
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
    parseState->m_currentContour->cb += ((2 * sizeof(uint16_t/*WORD*/)) + (2 * sizeof(POINTFX)));
    parseState->m_lastContourPoint.x = currPoint->x;
    parseState->m_lastContourPoint.y = currPoint->y;

    return 0;
    }

//---------------------------------------------------------------------------------------
// Adapted for DGNPlatform from Mark Schlosser's prototype.
// @bsimethod                                                   Jeff.Marker     06/2012
//---------------------------------------------------------------------------------------
int decomposeeMoveTo(FT_Vector const* ftVecTo, void* args)
    {
    FTOutlineParseState* parseState = static_cast<FTOutlineParseState*>(args);
    if (NULL == parseState->m_buffer)
        {
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
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
static size_t decomposeOutline(FT_Outline& outline, Byte* buffer, size_t bufferSize)
    {
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
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
static void appendPoint (bvector<DPoint3d> &points, POINTFX* pPoint, double npcScale)
    {
    DPoint3d pt;
    pt.x = (((double)(pPoint->x.value + (double)pPoint->x.fract / 65536.0)) / npcScale);
    pt.y = (((double)(pPoint->y.value + (double)pPoint->y.fract / 65536.0)) / npcScale);
    pt.z = 0.0;
    points.push_back (pt);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
static CurveVectorPtr convertNativeGlyphToCurveVector (TTPOLYGONHEADER* lpHeader, size_t size, double npcScale)
    {
    TTPOLYGONHEADER* lpStart = lpHeader;
    TTPOLYCURVE* lpCurve;
    int i = 0;
    bvector<DPoint3d> points;
    bvector<CurveVectorPtr> loops;
    while ((uintptr_t)lpHeader < (uintptr_t)(((char*)lpStart) + size))
        {
        if (TT_POLYGON_TYPE == lpHeader->dwType)
            {
            // Get to first curve.
            lpCurve = (TTPOLYCURVE*)(lpHeader + 1);
            auto loop = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer);

            while ((uintptr_t)lpCurve < (uintptr_t)(((char*)lpHeader) + lpHeader->cb))
                {
                // Format assumption:
                //  The bytes immediately preceding a POLYCURVE structure contain a valid POINTFX.
                //  If this is first curve, this points to the pfxStart of the POLYGONHEADER. Otherwise, this points to the last point of the previous POLYCURVE.
                //  In either case, this is representative of the previous curve's last point.
                if (TT_PRIM_LINE == lpCurve->wType)
                    {
                    points.clear ();
                    appendPoint (points, (POINTFX*)((char*)lpCurve - sizeof (POINTFX)), npcScale);
                    for (i = 0; i < lpCurve->cpfx; ++i)
                        {
                        appendPoint (points, &lpCurve->apfx[i], npcScale);
                        }
                    loop->Add (ICurvePrimitive::CreateLineString (points));
                    }
                else if (lpCurve->wType == TT_PRIM_QSPLINE)
                    {
                    int order = 3;  // always quadratic !!
                    points.clear ();
                    appendPoint (points, (POINTFX*)((char*)lpCurve - sizeof (POINTFX)), npcScale);
                    for (i = 0; i < lpCurve->cpfx; ++i)
                        appendPoint (points, &lpCurve->apfx[i], npcScale);
                    if (points.size () >= order)
                        loop->Add (ICurvePrimitive::CreateBsplineCurve  (
                                MSBsplineCurve::CreateFromPolesAndOrder(
                                    points, nullptr, nullptr, order, false)));
                    }

                // Move on to next curve.
                lpCurve = (TTPOLYCURVE*)&(lpCurve->apfx[i]);
                }
            loop->ConsolidateAdjacentPrimitives ();
            loops.push_back (loop);
            // Move on to next polygon.
            lpHeader = (TTPOLYGONHEADER*)(((char*)lpHeader) + lpHeader->cb);
            }
        else
            {
            break;
            }
        }
    if (loops.size () == 0)
        return CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    if (loops.size () == 1)
        return loops[0];
    auto parityRegion = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
    for (auto &loop : loops)
        parityRegion->Add (loop);

    parityRegion->FixupXYOuterInner ();

    return parityRegion;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Earlin.Lutz     05/2018
//---------------------------------------------------------------------------------------
CurveVectorPtr DgnTrueTypeGlyph::_GetCurveVector () const
    {
    static bool s_nocv = false;
    if (s_nocv)
        return nullptr;
    return m_face.Execute ([&](FT_Face ftFace)
        {
        if (FT_Err_Ok != FT_Load_Glyph (ftFace, m_glyphIndex, FT_LOAD_DEFAULT))
            return CurveVectorPtr ();
        size_t dataSize = decomposeOutline (ftFace->glyph->outline, nullptr, 0);
        if (0 == dataSize)
            return CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);   // empty !

        bvector<Byte> data;
        data.resize (dataSize);
        decomposeOutline (ftFace->glyph->outline, &data[0], dataSize);

        return convertNativeGlyphToCurveVector(reinterpret_cast<TTPOLYGONHEADER*>(&data[0]), data.size (), ftFace->units_per_EM);
        });

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
bool DgnTrueTypeGlyph::_IsBlank() const
    {
    if (IS_BLANK_Untested == m_isBlank)
        m_isBlank = ((_GetExactRange().XLength() <= 0.0) ? IS_BLANK_Yes : IS_BLANK_No);

    return (IS_BLANK_Yes == m_isBlank);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
static FreeTypeFace determineFace(DgnFontStyle& style, bool isBold, bool isItalic, IDgnTrueTypeFontData& data)
    {
    style = DgnFont::FontStyleFromBoldItalic(isBold, isItalic);
    FreeTypeFace face = data._GetFaceP(style);
    if (!face.IsValid())
        face = data._GetFaceP(DgnFontStyle::Regular);

    return face;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
DgnGlyphCP DgnTrueTypeFont::FindGlyphCP(FreeTypeFace face, FT_UInt id, DgnFontStyle style) const
    {
    BeMutexHolder lock(DgnFonts::GetMutex());

    T_GlyphCacheMap::iterator foundCache = m_glyphCache.find(style);
    T_GlyphCache* glyphCache = nullptr;
    if (m_glyphCache.end() != foundCache)
        {
        glyphCache = &foundCache->second;
        }
    else
        {
        m_glyphCache[style] = T_GlyphCache();
        glyphCache = &m_glyphCache[style];
        }
    
    T_GlyphCache::const_iterator foundGlyph = glyphCache->find(id);
    if (glyphCache->end() != foundGlyph)
        return foundGlyph->second;
    
    DgnGlyphP glyph = new DgnTrueTypeGlyph(face, id);
    glyphCache->Insert(id, glyph);
    return glyph;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
DgnGlyphCP DgnTrueTypeFont::_FindGlyphCP(DgnGlyph::T_Id glyphId, DgnFontStyle fontStyle) const
    {
    if (!IsResolved())
        return nullptr;

    bool isBold, isItalic;
    DgnFont::FontStyleToBoldItalic(isBold, isItalic, fontStyle);
    FreeTypeFace effectiveFace = determineFace(fontStyle, isBold, isItalic, (IDgnTrueTypeFontData&)*m_data);

    return FindGlyphCP(effectiveFace, glyphId, fontStyle);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnTrueTypeFont::GetTrueTypeGlyphDataDirect(bvector<Byte>& data, double& scaleFactor, DgnGlyphCR glyph)
    {
    DgnTrueTypeGlyph const* ttGlyph = dynamic_cast<DgnTrueTypeGlyph const*>(&glyph);
    if (nullptr == ttGlyph)
        return ERROR;
    
    return ttGlyph->GetFace().Execute([&](FT_Face ftFace)
        {
        if (FT_Err_Ok != FT_Load_Glyph(ftFace, ttGlyph->GetId(), FT_LOAD_DEFAULT))
            return ERROR;

        size_t dataSize = decomposeOutline(ftFace->glyph->outline, nullptr, 0);
        if (0 == dataSize)
            return SUCCESS;

        data.resize(dataSize);
        decomposeOutline(ftFace->glyph->outline, &data[0], dataSize);

        // ftPosToFIXED, used by our decomposition functions, already takes out the 64.0 to increase precision at that level.
        scaleFactor = (double)ftFace->units_per_EM;

        return SUCCESS;
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnTrueTypeFont::ComputeAdvanceWidths(T_DoubleVectorR advanceWidths, FreeTypeFace face, uint32_t const* ucs4Chars, size_t numChars) const
    {
    // Currently only supports unidirectional, left-to-right text. Need to consider libraries like harfbuzz, pango, and cairo for complex and bidirectional scripts.
    return face.Execute([&](FT_Face ftFace)
        {
        for (size_t iChar = 0; iChar < numChars; ++iChar)
            {
            FT_UInt glyphIndex = FT_Get_Char_Index(ftFace, ucs4Chars[iChar]);
            if (0 == glyphIndex)
                {
                advanceWidths.push_back(0.0);
                continue;
                }

            if (FT_Err_Ok != FT_Load_Glyph(ftFace, glyphIndex, FT_LOAD_DEFAULT))
                {
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnTrueTypeFont::_LayoutGlyphs(DgnGlyphLayoutResultR result, DgnGlyphLayoutContextCR context) const
    {
    // If you make any changes to this method, also consider examining DgnRscFont::_LayoutGlpyhs and DgnShxFont::_LayoutGlyphs.
    //  This method differs from the V8i variants in that it is designed to compute only the low-level information needed,
    //  and to serve both TextString and TextBlock through a single code path. This does mean that some extraneous information
    //  is potentially computed, but should be cheap compared to the overall layout operation.

    if (!IsResolved())
        return ERROR;

    // Determine the best face data to use.
    DgnFontStyle style;
    FreeTypeFace face = determineFace(style, context.m_isBold, context.m_isItalic, (IDgnTrueTypeFontData&)*m_data);
    
    // UTF-8 is mulit-byte; need to figure out each UCS "character" so we can look up the glyph.
    bvector<Byte> ucs4CharsBuffer;
    size_t numUcs4Chars=0;
    uint32_t const* ucs4Chars = DgnFont::Utf8ToUcs4(ucs4CharsBuffer, numUcs4Chars, context.m_string);
    if (0 == numUcs4Chars)
        return SUCCESS;
    
    // Compute the advance widths.
    T_DoubleVector widths;
    widths.reserve(numUcs4Chars);
    if (SUCCESS != ComputeAdvanceWidths(widths, face, ucs4Chars, numUcs4Chars))
        return ERROR;

    // Acquire the glyphs.
    // We need a 1:1 correlation between widths and glyphs, so this means we can insert null glyphs.
    result.m_glyphs.reserve(numUcs4Chars);
    face.Execute([&](FT_Face ftFace)
        {
        for (size_t iGlyph = 0; iGlyph < numUcs4Chars; ++iGlyph)
            result.m_glyphs.push_back(FindGlyphCP(face, FT_Get_Char_Index(ftFace, ucs4Chars[iGlyph]), style));
        });

    // Right-justified text needs to ignore trailing blanks.
    size_t numNonBlankGlyphs = result.m_glyphs.size();
    for (; numNonBlankGlyphs > 0; --numNonBlankGlyphs)
        {
        DgnGlyphCR glyph = *result.m_glyphs[numNonBlankGlyphs - 1];
        if (!glyph._IsBlank())
            break;
        }

    // Compute origins, ranges, etc...
    DPoint2d penPosition = DPoint2d::FromZero();
    result.m_glyphOrigins.reserve(result.m_glyphs.size());
    for (size_t iGlyph = 0; iGlyph < result.m_glyphs.size(); ++iGlyph)
        {
        DgnGlyphCP glyph = result.m_glyphs[iGlyph];
        if (nullptr == glyph)
            continue;
        
        result.m_glyphOrigins.push_back(DPoint3d::From(penPosition));

        DRange2d glyphRange = glyph->GetRange();
        DgnFont::ScaleAndOffsetGlyphRange(glyphRange, context.m_drawSize, penPosition);
        
        DRange2d glyphExactRange = glyph->GetExactRange();
        DgnFont::ScaleAndOffsetGlyphRange(glyphExactRange, context.m_drawSize, penPosition);
        
        // It is important to use the array overload of DRange2d::Extend; the ranges can be inverted for backwards/upside-down text, and the DRange2d overload enforces low/high semantics.
        result.m_range.Extend(&glyphRange.low, 2);
        result.m_exactRange.Extend(&glyphExactRange.low, 2);

        if (iGlyph < numNonBlankGlyphs)
            result.m_justificationRange.Extend(&glyphRange.low, 2);

        penPosition.x += (widths[iGlyph] * context.m_drawSize.x);
        }

    // Fail-safe in case nothing accumulated above. Callers will expect a zero'ed range more than an inverted range.
    result.ZeroNullRanges();
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
double DgnTrueTypeFont::_GetDescenderRatio(DgnFontStyle fontStyle) const
    {
    if (!IsResolved())
        return 0.0;
    
    bool isBold, isItalic;
    DgnFont::FontStyleToBoldItalic(isBold, isItalic, fontStyle);
    FreeTypeFace effectiveFace = determineFace(fontStyle, isBold, isItalic, (IDgnTrueTypeFontData&)*m_data);

    return effectiveFace.Execute([&](FT_Face ftFace)
        {
        return fabs ((double)ftFace->descender / (double)ftFace->ascender);
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
bvector<unsigned int /*FT_Uint*/> DgnTrueTypeFont::ComputeGlyphIndices(Utf8CP str, bool isBold, bool isItalic) const
    {
    bvector<unsigned int /*FT_Uint*/> glyphIndices;

    // Determine the best face data to use.
    DgnFontStyle style;
    FreeTypeFace face = determineFace(style, isBold, isItalic, (IDgnTrueTypeFontData&)*m_data);
    
    // UTF-8 is mulit-byte; need to figure out each UCS "character" so we can look up the glyph.
    bvector<Byte> ucs4CharsBuffer;
    size_t numUcs4Chars = 0;
    uint32_t const* ucs4Chars = DgnFont::Utf8ToUcs4(ucs4CharsBuffer, numUcs4Chars, str);
    if (0 == numUcs4Chars)
        return glyphIndices;
    
    // Convert to glyph indices.
    glyphIndices.reserve(numUcs4Chars);
    face.Execute([&](FT_Face face)
        {
        for (size_t iGlyph = 0; iGlyph < numUcs4Chars; ++iGlyph)
            glyphIndices.push_back(FT_Get_Char_Index(face, ucs4Chars[iGlyph]));
        });

    return glyphIndices;
    }
