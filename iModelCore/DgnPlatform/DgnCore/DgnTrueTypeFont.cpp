/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnTrueTypeFont.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnFontData.h>
#include <regex>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
FT_Library DgnPlatformLib::Host::FontAdmin::_GetFreeTypeLibrary()
    {
    if (!m_triedToLoadFTLibrary)
        {
        m_triedToLoadFTLibrary = true;

        FT_Error loadStatus = FT_Init_FreeType(&m_ftLibrary);
        if ((nullptr == m_ftLibrary) || (FT_Err_Ok != loadStatus))
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
    FT_Face m_face;
    FT_UInt m_glyphIndex;
    mutable bool m_isRangeValid;
    mutable DRange2d m_range;
    mutable bool m_isExactRangeValid;
    mutable DRange2d m_exactRange;
    mutable enum { IS_BLANK_Untested, IS_BLANK_Yes, IS_BLANK_No } m_isBlank;
    
public:
    DgnTrueTypeGlyph(FT_Face face, FT_UInt glyphIndex) : m_face(face), m_glyphIndex(glyphIndex), m_isRangeValid(false), m_isExactRangeValid(false), m_isBlank(IS_BLANK_Untested) {}
    virtual T_Id _GetId() const override { return (T_Id)m_glyphIndex; }
    virtual DRange2d _GetRange() const override;
    virtual DRange2d _GetExactRange() const override;
    virtual BentleyStatus _FillGpa(GPArrayR) const override;
    virtual bool _IsBlank() const override;
    FT_Face GetFace() const { return m_face; }
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
DRange2d DgnTrueTypeGlyph::_GetRange() const
    {
    if (!m_isRangeValid)
        {
        m_isRangeValid = true;

        if (FT_Err_Ok != FT_Load_Glyph(m_face, m_glyphIndex, FT_LOAD_DEFAULT))
            {
            m_range.low.Zero();
            m_range.high.Zero();
            }
        else
            {
            m_range.low.x = 0.0;
            m_range.low.y = 0.0;
            m_range.high.x = (m_face->glyph->metrics.horiAdvance / (64.0 * (double)m_face->units_per_EM));
            m_range.high.y = 1.0;
            }
        }
    
    return m_range;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
DRange2d DgnTrueTypeGlyph::_GetExactRange() const
    {
    if (!m_isExactRangeValid)
        {
        m_isExactRangeValid = true;

        if (FT_Err_Ok != FT_Load_Glyph(m_face, m_glyphIndex, FT_LOAD_DEFAULT))
            {
            m_exactRange.low.Zero();
            m_exactRange.high.Zero();
            }
        else
            {
            m_exactRange.low.x = (m_face->glyph->metrics.horiBearingX / (64.0 * (double)m_face->units_per_EM));
            m_exactRange.high.y = (m_face->glyph->metrics.horiBearingY / (64.0 * (double)m_face->units_per_EM));
            m_exactRange.low.y = (m_exactRange.high.y - (m_face->glyph->metrics.height / (64.0 * (double)m_face->units_per_EM)));
            m_exactRange.high.x = (m_exactRange.low.x + (m_face->glyph->metrics.width / (64.0 * (double)m_face->units_per_EM)));
            }
        }

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
        parseState->m_bufferSize += (3 * sizeof(POINTFX));

        return 0;
        }

    TTPOLYCURVE* currCurve = reinterpret_cast<TTPOLYCURVE*>(parseState->m_buffer + parseState->m_bufferOffset);
    currCurve->wType = TT_PRIM_QSPLINE;
    currCurve->cpfx = 3;

    parseState->m_bufferOffset += (sizeof(currCurve->wType) + sizeof(currCurve->cpfx));

    POINTFX* currPoint = reinterpret_cast<POINTFX*>(parseState->m_buffer + parseState->m_bufferOffset);
    currPoint->x = parseState->m_lastContourPoint.x;
    currPoint->y = parseState->m_lastContourPoint.y;

    ++currPoint;
    currPoint->x = ftPosToFIXED(ftVecControl->x);
    currPoint->y = ftPosToFIXED(ftVecControl->y);

    ++currPoint;
    currPoint->x = ftPosToFIXED(ftVecTo->x);
    currPoint->y = ftPosToFIXED(ftVecTo->y);

    parseState->m_bufferOffset += (currCurve->cpfx * sizeof(POINTFX));
    parseState->m_currentContour->cb += ((2 * sizeof(uint16_t/*WORD*/)) + (3 * sizeof(POINTFX)));
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
        parseState->m_bufferSize += (4 * sizeof(POINTFX));

        return 0;
        }

    TTPOLYCURVE* currCurve = reinterpret_cast<TTPOLYCURVE*>(parseState->m_buffer + parseState->m_bufferOffset);
    currCurve->wType = TT_PRIM_CSPLINE;
    currCurve->cpfx = 4;

    parseState->m_bufferOffset += (sizeof(currCurve->wType) + sizeof(currCurve->cpfx));

    POINTFX* currPoint = reinterpret_cast<POINTFX*>(parseState->m_buffer + parseState->m_bufferOffset);
    currPoint->x = parseState->m_lastContourPoint.x;
    currPoint->y = parseState->m_lastContourPoint.y;

    ++currPoint;
    currPoint->x = ftPosToFIXED(ftVecControl1->x);
    currPoint->y = ftPosToFIXED(ftVecControl1->y);

    ++currPoint;
    currPoint->x = ftPosToFIXED(ftVecControl2->x);
    currPoint->y = ftPosToFIXED(ftVecControl2->y);

    ++currPoint;
    currPoint->x = ftPosToFIXED(ftVecTo->x);
    currPoint->y = ftPosToFIXED(ftVecTo->y);

    parseState->m_bufferOffset += currCurve->cpfx * sizeof(POINTFX);
    parseState->m_currentContour->cb += ((2 * sizeof(uint16_t/*WORD*/)) + (4 * sizeof(POINTFX)));
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
static void toDPoint4d(DPoint4d* pt, POINTFX* pPoint)
    {
    pt->x = ((double)(pPoint->x.value + (double)pPoint->x.fract / 65536.0));
    pt->y = ((double)(pPoint->y.value + (double)pPoint->y.fract / 65536.0));
    pt->z = 0.0;
    pt->w = 1.0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
static void convertNativeGlyphToGraphicsPoints(GraphicsPointArrayR gpa, TTPOLYGONHEADER* lpHeader, size_t size)
    {
    TTPOLYGONHEADER* lpStart = lpHeader;
    TTPOLYCURVE* lpCurve;
    int i = 0;
    while ((uintptr_t)lpHeader < (uintptr_t)(((char*)lpStart) + size))
        {
        if (TT_POLYGON_TYPE == lpHeader->dwType)
            {
            DPoint4d ptHead;
            DPoint4d ptLast;

            // Get to first curve.
            lpCurve = (TTPOLYCURVE*)(lpHeader + 1);

            while ((uintptr_t)lpCurve < (uintptr_t)(((char*)lpHeader) + lpHeader->cb))
                {
                // Format assumption:
                //  The bytes immediately preceding a POLYCURVE structure contain a valid POINTFX.
                //  If this is first curve, this points to the pfxStart of the POLYGONHEADER. Otherwise, this points to the last point of the previous POLYCURVE.
                //  In either case, this is representative of the previous curve's last point.

                if (TT_PRIM_LINE == lpCurve->wType)
                    {
                    DPoint4d pt;
                    toDPoint4d(&pt, (POINTFX*)((char*)lpCurve - sizeof(POINTFX)));

                    gpa.Add(GraphicsPoint(pt));

                    for (i = 0; i < lpCurve->cpfx; ++i)
                        {
                        toDPoint4d(&pt, &lpCurve->apfx[i]);
                        gpa.Add(GraphicsPoint(pt));
                        }
                    }
                else if (lpCurve->wType == TT_PRIM_QSPLINE)
                    {
                    DPoint4d pts[100];
                    DPoint4d ptsp[50];
                    DPoint4d* pt = pts;
                    DPoint4d* pt1 = ptsp;
                    DPoint4d* pMem = NULL;
                    DPoint4d* pMem1 = NULL;
                    DPoint4d* pStart = pts;
                    POINTFX* pFixed;
                    int iSegments = 1;
                    int numPoles = (lpCurve->cpfx + 1);
                    int order = 3;

                    if ((lpCurve->cpfx + 1) > 100)
                        {
                        pMem = (DPoint4d*)malloc((lpCurve->cpfx + 1) * sizeof(DPoint4d));
                        if (NULL == pMem)
                            return;

                        pStart = pMem;
                        pt = pMem;
                        }

                    if ((numPoles > 3) && (numPoles + (numPoles - order) * (order - 1)) > 50)
                        {
                        pMem1 = (DPoint4d*)malloc((numPoles + (numPoles - order) * (order - 1)) * sizeof(DPoint4d));
                        if (NULL == pMem1)
                            return;

                        pt1 = pMem1;
                        }

                    toDPoint4d(pt, (POINTFX*)((char*)lpCurve - sizeof(POINTFX)));
                    ++pt;

                    for (i = 0, pFixed = lpCurve->apfx; i < lpCurve->cpfx; ++i, ++pt, ++pFixed)
                        toDPoint4d(pt, pFixed);

                    if (numPoles > 3)
                        {
                        iSegments = bsiBezierDPoint4d_convertOpenUniformBsplineToBeziers(pt1, pStart, numPoles, order, false);
                        pStart = pt1;
                        }

                    for (int i = 0; i < iSegments; ++i)
                        gpa.AddBezier((pStart + (i * order)), order);

                    if (pMem)
                        free(pMem);

                    if (pMem1)
                        free(pMem1);
                    }

                // Move on to next curve.
                lpCurve = (TTPOLYCURVE*)&(lpCurve->apfx[i]);
                }

            // Add points to close curve. Depending on the specific font and glyph being used, these may not always be needed, but it never hurts. Our last point could have been a curve so we need to add that point again.
            toDPoint4d(&ptLast, (POINTFX*)((char*)lpCurve - sizeof(POINTFX)));
            gpa.Add(GraphicsPoint(ptLast));

            toDPoint4d(&ptHead, &lpHeader->pfxStart);
            gpa.Add(GraphicsPoint(ptHead));

            // Move on to next polygon.
            lpHeader = (TTPOLYGONHEADER*)(((char*)lpHeader) + lpHeader->cb);
            }
        else
            {
            break;
            }

        gpa.MarkMajorBreak();
        }

    // Add user break so we can transform individual glyphs.
    gpa.MarkMajorBreak();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnTrueTypeGlyph::_FillGpa(GPArrayR gpa) const
    {
    if (FT_Err_Ok != FT_Load_Glyph(m_face, m_glyphIndex, FT_LOAD_DEFAULT))
        return ERROR;

    size_t dataSize = decomposeOutline(m_face->glyph->outline, nullptr, 0);
    if (0 == dataSize)
        return SUCCESS;

    bvector<Byte> data;
    data.resize(dataSize);
    decomposeOutline(m_face->glyph->outline, &data[0], dataSize);
    
    convertNativeGlyphToGraphicsPoints(gpa, reinterpret_cast<TTPOLYGONHEADER*>(&data[0]), data.size());
    gpa.SetArrayMask(HPOINT_ARRAYMASK_FILL);
    
    return SUCCESS;
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
static FT_Face determineFace(DgnFontStyle& style, bool isBold, bool isItalic, IDgnTrueTypeFontData& data)
    {
    style = DgnFont::FontStyleFromBoldItalic(isBold, isItalic);
    FT_Face face = data._GetFaceP(style);
    if (nullptr == face)
        face = data._GetFaceP(DgnFontStyle::Regular);

    return face;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
DgnGlyphCP DgnTrueTypeFont::FindGlyphCP(FT_Face face, FT_UInt id, DgnFontStyle style) const
    {
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
    FT_Face effectiveFace = determineFace(fontStyle, isBold, isItalic, (IDgnTrueTypeFontData&)*m_data);

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
    
    if (FT_Err_Ok != FT_Load_Glyph(ttGlyph->GetFace(), ttGlyph->GetId(), FT_LOAD_DEFAULT))
        return ERROR;

    size_t dataSize = decomposeOutline(ttGlyph->GetFace()->glyph->outline, nullptr, 0);
    if (0 == dataSize)
        return SUCCESS;

    data.resize(dataSize);
    decomposeOutline(ttGlyph->GetFace()->glyph->outline, &data[0], dataSize);

    // ftPosToFIXED, used by our decomposition functions, already takes out the 64.0 to increase precision at that level.
    scaleFactor = (double)ttGlyph->GetFace()->units_per_EM;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnTrueTypeFont::ComputeAdvanceWidths(T_DoubleVectorR advanceWidths, FT_Face face, uint32_t const* ucs4Chars, size_t numChars) const
    {
    // Currently only supports unidirectional, left-to-right text. Need to consider libraries like harfbuzz, pango, and cairo for complex and bidirectional scripts.
    for (size_t iChar = 0; iChar < numChars; ++iChar)
        {
        FT_UInt glyphIndex = FT_Get_Char_Index(face, ucs4Chars[iChar]);
        if (0 == glyphIndex)
            {
            advanceWidths.push_back(0.0);
            continue;
            }

        if (FT_Err_Ok != FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT))
            {
            advanceWidths.push_back(0.0);
            continue;
            }

        FT_Vector kerning;
        FT_UInt nextGlyphIndex = (((numChars - 1) == iChar) ? 0 : FT_Get_Char_Index(face, ucs4Chars[iChar + 1]));

        if ((0 == nextGlyphIndex) || (FT_Err_Ok != FT_Get_Kerning(face, glyphIndex, nextGlyphIndex, FT_KERNING_DEFAULT, &kerning)))
            memset(&kerning, 0, sizeof(kerning));

        // Freetype normally exposes lengths as "26.6 fractional pixel" units. This means they're integers made of a 26-bit integer mantissa, and a 6-bit fractional part. In other words, all coordinates are multiplied by 64.
        // This allows an integer to represent sub-pixel positions. The goal of this function is to operate in a 0..1 nominal space, so divide.
        advanceWidths.push_back((face->glyph->advance.x / (64.0 * (double)face->units_per_EM)) + (kerning.x / (64.0 * (double)face->units_per_EM)));
        }

    return SUCCESS;
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
    FT_Face face = determineFace(style, context.m_isBold, context.m_isItalic, (IDgnTrueTypeFontData&)*m_data);
    
    // UTF-8 is mulit-byte; need to figure out each UCS "character" so we can look up the glyph.
    bvector<Byte> ucs4CharsBuffer;
    size_t numUcs4Chars;
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
    for (size_t iGlyph = 0; iGlyph < numUcs4Chars; ++iGlyph)
        result.m_glyphs.push_back(FindGlyphCP(face, FT_Get_Char_Index(face, ucs4Chars[iGlyph]), style));

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

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
double DgnTrueTypeFont::_GetDescenderRatio(DgnFontStyle fontStyle) const
    {
    bool isBold, isItalic;
    DgnFont::FontStyleToBoldItalic(isBold, isItalic, fontStyle);
    FT_Face effectiveFace = determineFace(fontStyle, isBold, isItalic, (IDgnTrueTypeFontData&)*m_data);

    return fabs ((double)effectiveFace->descender / (double)effectiveFace->ascender);
    }
