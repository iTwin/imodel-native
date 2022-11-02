/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ShxFont::~ShxFont() {
    for (auto& cacheEntry : m_glyphCache)
        DELETE_AND_CLEAR(cacheEntry.second);
}

//=======================================================================================
// @bsiclass
//=======================================================================================
struct AutoRestoreFPos {
private:
    ShxFontR m_font;
    uint32_t m_positionToRestore;

public:
    AutoRestoreFPos(ShxFontR font) : m_font(font), m_positionToRestore(font.Tell()) {}
    ~AutoRestoreFPos() { m_font.Seek(m_positionToRestore); }
};

/** Validate the header of an SHX file and return its type */
ShxFont::ShxType ShxFont::ValidateHeader(CharCP fileHeader) {
    static const CharCP UNIFONT_HEADER = "AutoCAD-86 unifont 1.0";
    static const CharCP SHAPES1_0_HEADER = "AutoCAD-86 shapes 1.0";
    static const CharCP SHAPES1_1_HEADER = "AutoCAD-86 shapes 1.1";

    if (0 == strncmp(fileHeader, UNIFONT_HEADER, strlen(UNIFONT_HEADER)))
        return ShxFont::ShxType::Unicode;

    if ((0 == strncmp(fileHeader, SHAPES1_0_HEADER, strlen(SHAPES1_0_HEADER))) || (0 == strncmp(fileHeader, SHAPES1_1_HEADER, strlen(SHAPES1_1_HEADER))))
        return ShxFont::ShxType::Locale;

    return ShxFont::ShxType::Invalid;
}

/** Read the header of an ShxFont to determine its type */
ShxFont::ShxType ShxFont::GetShxType() {
    static const size_t MAX_HEADER = 40; // an SHX file with less than 40 bytes isn't valid

    // Allow this to be used in the middle of other read operations.
    AutoRestoreFPos restoreFPos(*this);
    Seek(0);

    char fileHeader[MAX_HEADER];
    if (MAX_HEADER != Read (fileHeader,MAX_HEADER))
        return ShxFont::ShxType::Invalid;

    return ValidateHeader(fileHeader);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ShxFont::LoadNonUnicodeGlyphFPosCacheAndMetrics() {
    Seek(0);

    for (size_t iByte = 0; ((iByte < 40) && (0x1a != GetNextChar())); ++iByte) {
        // Skip header...
    }

    // Skip FirstCode and LastCode (both UInt16)...
    Skip(4);

    uint16_t numGlyphs = GetNextUInt16();
    uint32_t dataStart = Tell() + (4 * numGlyphs);
    uint32_t dataOffset = 0;

    // Read glyph GlyphFPos data.
    for (size_t iGlyph = 0; iGlyph < numGlyphs; ++iGlyph) {
        uint16_t glyphId = GetNextUInt16();
        ShxFont::GlyphFPos glyphFPos;
        glyphFPos.m_dataOffset = (dataStart + dataOffset);
        glyphFPos.m_dataSize = GetNextUInt16();

        m_glyphFPosCache[glyphId] = glyphFPos;
        dataOffset += glyphFPos.m_dataSize;
    }

    // Read metric data. Char 0 is the font specifier...
    T_GlyphFPosCache::const_iterator zeroGlyphFPos = m_glyphFPosCache.find(0);
    if (m_glyphFPosCache.end() == zeroGlyphFPos) {
        m_ascender = 1;
        m_descender = 1;
        return;
    }

    if (0 != Seek(zeroGlyphFPos->second.m_dataOffset))
        return;

    uint32_t pBufSize = zeroGlyphFPos->second.m_dataSize;
    ScopedArray<Byte> pBuf(pBufSize);

    // Read code 0 which should be the font specifier.
    Read(pBuf.GetData(), pBufSize);

    // Look for a NULL terminator.
    size_t iLen = 0;
    for (; ((iLen < pBufSize) && (0 != pBuf.GetData()[iLen])); ++iLen)
        ;

    // No Terminator found?
    if (iLen == pBufSize)
        return;

    // Prehistoric legacy...
    if ((pBufSize - iLen) != 5)
        return;

    size_t iOffset = (iLen + 1);
    m_ascender = pBuf.GetData()[iOffset++];
    m_descender = pBuf.GetData()[iOffset++];

    // This hack comes from the OpenDWG code. I don't understand it but see TR#182253 for font with this problem (gbcbig.shx).
    if (0 == m_ascender)
        m_ascender = 8;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ShxFont::LoadUnicodeGlyphFPosCacheAndMetrics() {
    Seek(0);

    // Skip header.
    for (size_t iByte = 0; ((iByte < 40) && (0x1a != GetNextChar())); ++iByte)
        ;

    size_t numGlyphs = (size_t)GetNextUInt16();

    // Not sure why we have to do this, but old code does, and most fonts indeed have less 1 glyph.
    --numGlyphs;

    // Skip 2 bytes...
    Skip(2);

    uint16_t fontInfoSize = GetNextUInt16();
    ScopedArray<Byte> fontInfo(fontInfoSize);

    Read(fontInfo.GetData(), fontInfoSize);

    size_t firstNullOffset = strlen(reinterpret_cast<char*>(fontInfo.GetData()));
    size_t fontInfoDataOffset = (firstNullOffset + 1);

    // ascender
    m_ascender = fontInfo.GetData()[fontInfoDataOffset++];

    // descender
    m_descender = fontInfo.GetData()[fontInfoDataOffset++];

    // Skip "modes".
    ++fontInfoDataOffset;

    // Special case when the "encoding" is "shape file".
    if (2 == fontInfo.GetData()[fontInfoDataOffset]) {
        m_ascender = 1;
        m_descender = 1;
    }

    uint32_t nextAddress = Tell();
    for (size_t iGlyph = 0; iGlyph < numGlyphs; ++iGlyph) {
        if (0 != Seek((int64_t)nextAddress))
            break;

        uint16_t glyphId = GetNextUInt16();

        ShxFont::GlyphFPos glyphFPos;
        glyphFPos.m_dataOffset = (nextAddress + 4);
        glyphFPos.m_dataSize = GetNextUInt16();

        nextAddress = (size_t)(glyphFPos.m_dataOffset + glyphFPos.m_dataSize);

        m_glyphFPosCache[glyphId] = glyphFPos;
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ShxFont::LoadGlyphFPosCacheAndMetrics() {
    // Because we have to load and O(n) iterate the header to even know what glyphs are in the file, do it once up-front instead of repeatedly for each glyph.
    // The glyphs will still load their glyph geometry data on-demand.
    FontManager::FlagHolder hasLoaded(m_hasLoadedGlyphFPosCacheAndMetrics);
    if (hasLoaded)
        return;

    // Allow this to be used in the middle of other read operations.
    AutoRestoreFPos restoreFPos(*this);

    switch (GetShxType()) {
    case ShxFont::ShxType::Locale:
        LoadNonUnicodeGlyphFPosCacheAndMetrics();
        return;
    case ShxFont::ShxType::Unicode:
        LoadUnicodeGlyphFPosCacheAndMetrics();
        return;
    default:
        return;
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ShxFont::GlyphFPos const* ShxFont::GetGlyphFPos(T_Id glyphId) {
    LoadGlyphFPosCacheAndMetrics();

    auto foundGlyphFPos = m_glyphFPosCache.find(glyphId);
    if (m_glyphFPosCache.end() != foundGlyphFPos)
        return &foundGlyphFPos->second;

    return nullptr;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
uint32_t ShxFont::Read(void* buffer, uint32_t requestedBytes) {
    auto& reader = *GetReader().m_reader;
    uint32_t pos = Tell();
    size_t remainingBytes = reader.GetNumBytes() - pos;
    uint32_t actuallyRead = std::min((uint32_t)remainingBytes, requestedBytes);
    reader.ReadBytes(buffer, actuallyRead);
    return actuallyRead;
}

/**
 *
 */
BentleyStatus ShxFont::Seek(uint32_t pos) {
    auto& reader = *GetReader().m_reader;
    if (pos >= reader.GetNumBytes())
        return ERROR;
    reader.SetPos(pos);
    return SUCCESS;
}

//=======================================================================================
// @bsiclass
//=======================================================================================
enum ShxShapeCode {
    SHAPECODE_ENDOFSHAPE = 0,
    SHAPECODE_PENDOWN = 1,
    SHAPECODE_PENUP = 2,
    SHAPECODE_DIVLENGTHS = 3,
    SHAPECODE_MULTLENGTHS = 4,
    SHAPECODE_PUSH = 5,
    SHAPECODE_POP = 6,
    SHAPECODE_SUBSHAPE = 7,
    SHAPECODE_XYDISP = 8,
    SHAPECODE_MULTIXYDISP = 9,
    SHAPECODE_OCTANTARC = 10,
    SHAPECODE_FRACTIONARC = 11,
    SHAPECODE_BULGEARC = 12,
    SHAPECODE_MULTIBULGEARC = 13,
    SHAPECODE_VERTICAL = 14
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static void bulgeFactorToDEllipse3d(DEllipse3dP pEllipse, DPoint2dCP pStart, DPoint2dCP pEnd, double bulgeFactor) {
    double sweep = (4.0 * atan(bulgeFactor));
    DPoint2d v;
    DPoint2d u;
    DPoint2d r;
    DPoint2d x;
    DPoint2d center;
    double theta0;
    double radius;

    // Find tan (included angle / 2)
    double tangentHalfAngle = tan(sweep * 0.5);

    // U = chord bvector
    u.DifferenceOf(*pEnd, *pStart);
    u.Scale(u, 0.5);

    // V = perpendicular to chord bvector, same length.
    v.x = -u.y;
    v.y = u.x;

    // Center = p0 + U + -V/tangentHalfAngle
    center.SumOf(*pStart, u, 1.0, v, 1.0 / tangentHalfAngle);

    // Radius
    r.DifferenceOf(*pStart, center);
    radius = r.Magnitude();

    // Start angle
    x.x = 1.0;
    x.y = 0.0;

    theta0 = x.AngleTo(r);

    // Setup for ellipse
    pEllipse->Init(center.x, center.y, 0.0, radius, 0.0, 0.0, 0.0, radius, 0.0, theta0, sweep);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static bool codeNameExists(CharCP pBytes, size_t maxNameLength) {
    if (0 == *pBytes)
        return false;

    for (size_t j = 0; j < maxNameLength; ++j) {
        if (0 == *pBytes++)
            return true;
    }

    return false;
}

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ShapeConverter {
private:
    ShxFontR m_font;
    bool m_isPenDown;
    bool m_vStart;
    bool m_vEnd;
    bool m_processVertical;
    bool m_skipCodes;
    bool m_hasData;
    double m_multiplier;
    double m_leftBearing;
    double m_rightBearing;
    DPoint2d m_vertStartPt;
    DPoint2d m_vertEndPt;
    DPoint2d m_currentPos;

    std::stack<DPoint2d> m_stack;

    void AddPoint(DPoint2dCR pt);
    void AddEllipse(DEllipse3dCR el);

    bvector<DPoint3d> m_activeLineString;
    CurveVectorPtr m_activeLoop;
    bvector<CurveVectorPtr> m_completedLoops;
    // ensure there is an m_activeLoop.
    // move the active linestring into the active loop
    void ClearActiveLineString() {
        if (!m_activeLoop.IsValid())
            m_activeLoop = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
        if (m_activeLineString.size() > 1)
            m_activeLoop->Add(ICurvePrimitive::CreateLineString(m_activeLineString));
        m_activeLineString.clear();
    }
    // Move the active loop to completed
    void EndActiveLoop() {
        if (m_activeLineString.size() > 1)
            ClearActiveLineString();
        if (m_activeLoop.IsValid()) {
            m_completedLoops.push_back(m_activeLoop);
        }
        m_activeLoop = nullptr;
    }

public:
    CurveVectorPtr GrabCurveVector() {
        CurveVectorPtr result;
        if (m_completedLoops.size() == 1)
            result = m_completedLoops[0];
        else if (m_completedLoops.size() > 1) {
            result = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);
            for (auto& c : m_completedLoops)
                result->Add(c);
        } else {
            result = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);
        }
        m_completedLoops.clear();
        return result;
    }

public:
    ShapeConverter(ShxFontR);
    DPoint2dCR GetCurrentPos() const { return m_currentPos; }
    double GetLeftBearing() const { return m_leftBearing; }
    double GetMultiplier() const { return m_multiplier; }
    double GetRightBearing() const { return m_rightBearing; }
    DPoint2dCR GetVEndPt() const { return m_vertEndPt; }
    DPoint2dCR GetVStartPt() const { return m_vertStartPt; }
    bool HasVEnd() const { return m_vEnd; }
    bool HasVStart() const { return m_vStart; }
    bool IsPenDown() const { return m_isPenDown; }
    void DecodeBulgeArc(int& currentCode, CharCP pCodes);
    void DecodeCodes(int& currentCode, CharCP pCodes);
    void DecodeDivLengths(int& currentCode, CharCP pCodes);
    void DecodeExtenedFontSubShape(int& currentCode, CharCP pCodes);
    void DecodeFractionArc(int& currentCode, CharCP pCodes);
    void DecodeMultiBulgeArc(int& currentCode, CharCP pCodes);
    void DecodeMultLengths(int& currentCode, CharCP pCodes);
    void DecodeOctantArc(int& currentCode, CharCP pCodes);
    void DecodePenDown();
    void DecodePenUp();
    void DecodePop();
    void DecodePush();
    void DecodeSubShape(int& currentCode, CharCP pCodes);
    void DecodeVector(Byte iCode, CharCP);
    void DecodeVertical(int& currentCode, CharCP pCodes);
    void DecodeXYDisp(int& currentCode, CharCP pCodes);
    void DecodeXYMultiDisp(int& currentCode, CharCP pCodes);
    void DoConversion(size_t numBytes, CharCP buff, bool doVerticals, bool hasData);
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ShapeConverter::ShapeConverter(ShxFontR font) : m_font(font), m_isPenDown(true), m_vStart(false), m_vEnd(false), m_hasData(false), m_processVertical(false), m_skipCodes(false),
                                                m_multiplier(1.0), m_leftBearing(INT_MAX), m_rightBearing(-INT_MAX) {
    m_vertStartPt.Zero();
    m_vertEndPt.Zero();
    m_currentPos.Zero();
}
static bool IsDuplicationOfFinalPoint(bvector<DPoint3d> const& data, DPoint2dCR xy) {
    if (data.size() == 0)
        return false;
    static double s_tol = 1.0e-10;
    double delta = fabs(data.back().x - xy.x) + fabs(data.back().y - xy.y);
    return delta < s_tol;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ShapeConverter::AddPoint(DPoint2dCR pt) {
    m_hasData = true;
    if (!m_processVertical && m_isPenDown) {
        if (!IsDuplicationOfFinalPoint(m_activeLineString, pt))
            m_activeLineString.push_back(DPoint3d::From(pt));
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ShapeConverter::AddEllipse(DEllipse3dCR el) {
    m_hasData = true;
    if (!m_processVertical) {
        ClearActiveLineString();
        m_activeLoop->Add(ICurvePrimitive::CreateArc(el));
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodeBulgeArc(int& currentCode, CharCP pCodes) {
    DPoint2d sDisp;
    sDisp.x = ((double)pCodes[++currentCode] * m_multiplier);
    sDisp.y = ((double)pCodes[++currentCode] * m_multiplier);

    signed char b = pCodes[++currentCode];

    if (m_skipCodes)
        return;

    DPoint2d pt = m_currentPos;
    pt.x += sDisp.x;
    pt.y += sDisp.y;
    DPoint2d sStart = m_currentPos;
    DPoint2d sEnd = pt;

    // *Note: We have empirically found that DWG will always draw arcs, regardless of the pen up/down state.
    //  It does not, however, leave the pen down after arcs; therefore, simply ignore IsPenDown checks.
    //  Special case for bulge arcs: bulge arcs with a 0 bulge, which are effectively straight lines, obey the pen state.

    if (0 != b) {
        DEllipse3d sEllipse;
        bulgeFactorToDEllipse3d(&sEllipse, &sStart, &sEnd, (double)b / 127.0);

        AddPoint(m_currentPos);

        AddEllipse(sEllipse);
    } else {
        if (IsPenDown())
            AddPoint(m_currentPos);

        if (IsPenDown())
            AddPoint(pt);
    }

    m_currentPos = pt;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodeCodes(int& currentCode, CharCP pCodes) {
    Byte iCode = pCodes[currentCode];
    if ((int)(iCode & 0xF0) > 0) {
        DecodeVector(iCode, pCodes);
        return;
    }

    switch (iCode) {
    case SHAPECODE_ENDOFSHAPE:
        return;
    case SHAPECODE_PENDOWN:
        DecodePenDown();
        break;
    case SHAPECODE_PENUP:
        DecodePenUp();
        break;
    case SHAPECODE_DIVLENGTHS:
        DecodeDivLengths(currentCode, pCodes);
        break;
    case SHAPECODE_MULTLENGTHS:
        DecodeMultLengths(currentCode, pCodes);
        break;
    case SHAPECODE_PUSH:
        DecodePush();
        break;
    case SHAPECODE_POP:
        DecodePop();
        break;
    case SHAPECODE_SUBSHAPE:
        DecodeSubShape(currentCode, pCodes);
        break;
    case SHAPECODE_XYDISP:
        DecodeXYDisp(currentCode, pCodes);
        break;
    case SHAPECODE_MULTIXYDISP:
        DecodeXYMultiDisp(currentCode, pCodes);
        break;
    case SHAPECODE_OCTANTARC:
        DecodeOctantArc(currentCode, pCodes);
        break;
    case SHAPECODE_FRACTIONARC:
        DecodeFractionArc(currentCode, pCodes);
        break;
    case SHAPECODE_BULGEARC:
        DecodeBulgeArc(currentCode, pCodes);
        break;
    case SHAPECODE_MULTIBULGEARC:
        DecodeMultiBulgeArc(currentCode, pCodes);
        break;
    case SHAPECODE_VERTICAL:
        DecodeVertical(currentCode, pCodes);
        break;

    default:
        BeAssert(false && L"Unknown/unexpected opcode.");
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodeDivLengths(int& currentCode, CharCP pCodes) {
    Byte iCode = pCodes[++currentCode];
    BeAssert(0 != iCode);

    if (m_skipCodes)
        return;

    if (0 != iCode)
        m_multiplier /= (double)iCode;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodeExtenedFontSubShape(int&, CharCP) {
    // As far as I can tell, this only applies to big fonts, which we no longer support.
    return;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodeFractionArc(int& currentCode, CharCP pCodes) {
    double radius;
    double sweep;
    double startOffset;
    double endOffset;
    double x;
    double y;
    double startOffsetDeg;
    double endOffsetDeg;
    char iStartingOctant;
    char iEndOctant;
    char iSweep;
    char iOctants;
    unsigned char iStartOffset = 0;
    unsigned char iEndOffset = 0;
    unsigned char iHighRadius = 0;
    unsigned char iRadius = 0;
    DPoint3d center;
    DPoint3d sEnd;
    DVec3d s0Vector;
    DVec3d s90Vector;
    DEllipse3d sEllipse;

    // start offset
    iStartOffset = pCodes[++currentCode];

    // end offset
    iEndOffset = pCodes[++currentCode];

    // high radius
    iHighRadius = pCodes[++currentCode];

    // radius
    iRadius = pCodes[++currentCode];

    // Octant specifier
    char osc = pCodes[++currentCode];

    if (m_skipCodes)
        return;

    iStartingOctant = ((char)(osc & 0x70) >> 4);
    iSweep = (char)(osc & 0x07);
    iOctants = iSweep;

    bool isClockwise = false;
    if (osc < 0) {
        isClockwise = true;
        iSweep = -iSweep;
    }

    if (!iOctants)
        iOctants = 8;

    iEndOctant = iStartingOctant + iSweep;

    if (isClockwise) {
        iOctants = -iOctants;
        if (iEndOctant < 0)
            iEndOctant += 8;
    } else {
        if (iEndOctant >= 8)
            iEndOctant -= 8;
    }

    // may need to floor this
    if (!isClockwise) {
        startOffsetDeg = (((iStartOffset * 45) / 256) + (iStartingOctant * 45));
        startOffset = Angle::DegreesToRadians(startOffsetDeg);
        endOffsetDeg = (((iEndOffset * 45.0) / 256.0) + (((0 == iEndOffset) ? iEndOctant : (iStartingOctant + abs(iSweep) - 1)) * 45.0));
        endOffset = Angle::DegreesToRadians(endOffsetDeg);
    } else {
        if (0 == iStartOffset)
            startOffsetDeg = iStartingOctant * 45.0;
        else
            startOffsetDeg = (iStartingOctant * 45) - iStartOffset * 45.0 / 256.0;

        startOffset = Angle::DegreesToRadians(startOffsetDeg);

        if (0 == iEndOffset)
            endOffsetDeg = (((0 == iEndOctant) ? 8 : iEndOctant) * 45);
        else
            endOffsetDeg = ((0 == iEndOctant) ? ((((iStartingOctant - abs(iSweep + 1))) * 45) - ((iEndOffset * 45) / 256)) : (((iEndOctant + 1) * 45) - ((iEndOffset * 45) / 256)));

        endOffset = Angle::DegreesToRadians(endOffsetDeg);
    }

    radius = ((double)(iRadius + (256 * iHighRadius)) * m_multiplier);

    if (!isClockwise) {
        if (endOffsetDeg <= startOffsetDeg)
            endOffsetDeg += 360;

        sweep = Angle::DegreesToRadians(endOffsetDeg - startOffsetDeg + 1);
    } else {
        // need a -ve angle
        if (startOffsetDeg <= 0)
            startOffsetDeg += 360;

        if (startOffsetDeg <= endOffsetDeg)
            sweep = Angle::DegreesToRadians(-(360 - (endOffsetDeg - startOffsetDeg)));
        else
            sweep = Angle::DegreesToRadians(endOffsetDeg - startOffsetDeg);
    }

    startOffset = Angle::DegreesToRadians(startOffsetDeg);

    x = (radius * cos(startOffset));
    y = (radius * sin(startOffset));
    center.x = (m_currentPos.x - x);
    center.y = (m_currentPos.y - y);
    center.z = 0.0;
    endOffset = Angle::DegreesToRadians(endOffsetDeg);

    x = (radius * cos(endOffset));
    y = (radius * sin(endOffset));
    sEnd = center;
    sEnd.x = (center.x + x);
    sEnd.y = (center.y + y);

    s0Vector.x = (m_currentPos.x - center.x);
    s0Vector.y = (m_currentPos.y - center.y);
    s90Vector.x = -s0Vector.y;
    s90Vector.y = s0Vector.x;
    s90Vector.z = 0.0;
    s0Vector.z = 0.0;

    sEllipse.InitFromVectors(center, s0Vector, s90Vector, 0, sweep);

    // *Note: We have empirically found that DWG will always draw arcs, regardless of the pen up/down state.
    //  It does not, however, leave the pen down after arcs; therefore, simply ignore IsPenDown checks.

    AddPoint(m_currentPos);

    AddEllipse(sEllipse);

    m_currentPos.x = sEnd.x;
    m_currentPos.y = sEnd.y;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodeMultiBulgeArc(int& currentCode, CharCP pCodes) {
    signed char x = pCodes[currentCode + 1];
    signed char y = pCodes[currentCode + 2];

    while ((0 != x) || (0 != y)) {
        DecodeBulgeArc(currentCode, pCodes);

        // Get next 2 codes
        x = pCodes[currentCode + 1];
        y = pCodes[currentCode + 2];
    }

    // Move past the last 2 codes
    currentCode += 2;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodeMultLengths(int& currentCode, CharCP pCodes) {
    Byte iCode = pCodes[++currentCode];

    if (m_skipCodes)
        return;

    BeAssert(0 != iCode);

    if (0 != iCode)
        m_multiplier *= (double)iCode;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodeOctantArc(int& currentCode, CharCP pCodes) {
    signed char b = pCodes[++currentCode];
    double radius = ((double)b * m_multiplier);
    double radiusBySin45 = (radius * sin(msGeomConst_piOver4));

    // Octant specifier
    b = pCodes[++currentCode];

    if (m_skipCodes)
        return;

    char iStartingOctant = (char)((b & 0x70) >> 4);
    char iSweep = (char)(b & 0x07);
    char iOctants = iSweep;
    bool isClockwise = false;

    if (b < 0) {
        isClockwise = true;
        iSweep = -iSweep;
    }

    if (!iOctants)
        iOctants = 8;

    char iEndOctant = (iStartingOctant + iSweep);

    if (isClockwise) {
        iOctants = -iOctants;
        if (iEndOctant < 0)
            iEndOctant += 8;
    } else {
        if (iEndOctant >= 8)
            iEndOctant -= 8;
    }

    DPoint3d center;
    center.Init(m_currentPos.x, m_currentPos.y, 0);

    switch (iStartingOctant) {
    case 0:
        center.x -= radius;
        break;

    case 1:
        center.x -= radiusBySin45;
        center.y -= radiusBySin45;
        break;

    case 2:
        center.y -= radius;
        break;

    case 3:
        center.x += radiusBySin45;
        center.y -= radiusBySin45;
        break;

    case 4:
        center.x += radius;
        break;

    case 5:
        center.x += radiusBySin45;
        center.y += radiusBySin45;
        break;

    case 6:
        center.y += radius;
        break;

    case 7:
        center.x -= radiusBySin45;
        center.y += radiusBySin45;
        break;
    }

    DPoint3d sEnd = center;
    switch (iEndOctant) {
    case 0:
    case 8:
        sEnd.x += radius;
        break;

    case 1:
        sEnd.x += radiusBySin45;
        sEnd.y += radiusBySin45;
        break;

    case 2:
        sEnd.y += radius;
        break;

    case 3:
        sEnd.x -= radiusBySin45;
        sEnd.y += radiusBySin45;
        break;

    case 4:
        sEnd.x -= radius;
        break;

    case 5:
        sEnd.x -= radiusBySin45;
        sEnd.y -= radiusBySin45;
        break;

    case 6:
        sEnd.y -= radius;
        break;

    case 7:
        sEnd.x += radiusBySin45;
        sEnd.y -= radiusBySin45;
        break;
    }

    double sweep = ((0.0 == iSweep) ? msGeomConst_2pi : Angle::DegreesToRadians(iSweep * 45.0));
    DVec3d s0Vector;
    DVec3d s90Vector;

    s0Vector.x = (m_currentPos.x - center.x);
    s0Vector.y = (m_currentPos.y - center.y);
    s90Vector.x = -s0Vector.y;
    s90Vector.y = s0Vector.x;
    s90Vector.z = 0.0;
    s0Vector.z = 0.0;

    DEllipse3d sEllipse;
    sEllipse.InitFromVectors(center, s0Vector, s90Vector, 0, sweep);

    // *Note: We have empirically found that DWG will always draw arcs, regardless of the pen up/down state.
    //  It does not, however, leave the pen down after arcs; therefore, simply ignore IsPenDown checks.

    AddPoint(m_currentPos);

    AddEllipse(sEllipse);

    m_currentPos.x = sEnd.x;
    m_currentPos.y = sEnd.y;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodePenDown() {
    if (m_skipCodes)
        return;

    m_isPenDown = true;

    if (m_currentPos.x < m_leftBearing)
        m_leftBearing = m_currentPos.x;

    if (m_currentPos.x > m_rightBearing)
        m_rightBearing = m_currentPos.x;

    EndActiveLoop();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodePenUp() {
    if (m_skipCodes)
        return;

    m_isPenDown = false;
    EndActiveLoop();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodePop() {
    if (m_skipCodes)
        return;

    if (m_stack.empty())
        return;

    m_currentPos = m_stack.top();
    m_stack.pop();

    EndActiveLoop();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodePush() {
    if (!m_skipCodes)
        m_stack.push(m_currentPos);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodeSubShape(int& currentCode, CharCP pCodes) {
    uint16_t subshapeCode = pCodes[++currentCode];
    if (ShxFont::ShxType::Unicode == m_font.GetShxType())
        subshapeCode = ((subshapeCode << 8) + pCodes[++currentCode]);

    if (0 == subshapeCode) {
        DecodeExtenedFontSubShape(currentCode, pCodes);
        return;
    }

    ShxFont::GlyphFPos const* fPos = m_font.GetGlyphFPos(subshapeCode);
    if (NULL == fPos)
        return;

    if (0 != m_font.Seek(fPos->m_dataOffset))
        return;

    CharP pBuf = reinterpret_cast<CharP>(_alloca(sizeof(char) * fPos->m_dataSize));
    m_font.Read(pBuf, sizeof(Byte) * fPos->m_dataSize);

    DoConversion(fPos->m_dataSize, pBuf, m_processVertical, m_hasData);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodeVector(Byte iCode, CharCP) {
    if (m_skipCodes)
        return;

    int iLength = ((iCode & 0xF0) >> 4);
    int iDirection = (iCode & 0x0F);
    double dLength = (iLength * m_multiplier);

    DPoint2d pt;
    switch (iDirection) {
    case 0:
        pt.x = (m_currentPos.x + dLength);
        pt.y = (m_currentPos.y);
        break;

    case 1:
        pt.x = (m_currentPos.x + dLength);
        pt.y = (m_currentPos.y + dLength / 2.0);
        break;

    case 2:
        pt.x = (m_currentPos.x + dLength);
        pt.y = (m_currentPos.y + dLength);
        break;

    case 3:
        pt.x = (m_currentPos.x + dLength / 2.0);
        pt.y = (m_currentPos.y + dLength);
        break;

    case 4:
        pt.x = (m_currentPos.x);
        pt.y = (m_currentPos.y + dLength);
        break;

    case 5:
        pt.x = (m_currentPos.x - dLength / 2.0);
        pt.y = (m_currentPos.y + dLength);
        break;

    case 6:
        pt.x = (m_currentPos.x - dLength);
        pt.y = (m_currentPos.y + dLength);
        break;

    case 7:
        pt.x = (m_currentPos.x - dLength);
        pt.y = (m_currentPos.y + dLength / 2.0);
        break;

    case 8:
        pt.x = (m_currentPos.x - dLength);
        pt.y = (m_currentPos.y);
        break;

    case 9:
        pt.x = (m_currentPos.x - dLength);
        pt.y = (m_currentPos.y - dLength / 2.0);
        break;

    case 10:
        pt.x = (m_currentPos.x - dLength);
        pt.y = (m_currentPos.y - dLength);
        break;

    case 11:
        pt.x = (m_currentPos.x - dLength / 2.0);
        pt.y = (m_currentPos.y - dLength);
        break;

    case 12:
        pt.x = (m_currentPos.x);
        pt.y = (m_currentPos.y - dLength);
        break;

    case 13:
        pt.x = (m_currentPos.x + dLength / 2.0);
        pt.y = (m_currentPos.y - dLength);
        break;

    case 14:
        pt.x = (m_currentPos.x + dLength);
        pt.y = (m_currentPos.y - dLength);
        break;

    case 15:
        pt.x = (m_currentPos.x + dLength);
        pt.y = (m_currentPos.y - dLength / 2.0);
        break;

    default:
        pt.Zero();
        BeAssert(false);
        break;
    }

    if (IsPenDown()) {
        AddPoint(m_currentPos);

        AddPoint(pt);
    }

    m_currentPos = pt;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodeVertical(int& currentCode, CharCP pCodes) {
    // While we aren't supporting vertical SHX glyphs at this time, we still need to eat the codes.
    AutoRestore<bool> skipCodes(&m_skipCodes, true);
    ++currentCode;
    DecodeCodes(currentCode, pCodes);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodeXYDisp(int& currentCode, CharCP pCodes) {
    DPoint2d sDisp;
    DPoint2d pt;
    signed char b = 0;

    memset(&sDisp, 0, sizeof(DPoint2d));
    b = pCodes[++currentCode];

    sDisp.x = ((double)b * m_multiplier);
    b = pCodes[++currentCode];

    sDisp.y = ((double)b * m_multiplier);

    if (m_skipCodes)
        return;

    pt = m_currentPos;
    pt.x += sDisp.x;
    pt.y += sDisp.y;

    AddPoint(m_currentPos);

    AddPoint(pt);

    m_currentPos = pt;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodeXYMultiDisp(int& currentCode, CharCP pCodes) {
    signed char x = pCodes[currentCode + 1];
    signed char y = pCodes[currentCode + 2];

    while ((0 != x) || (0 != y)) {
        DecodeXYDisp(currentCode, pCodes);

        // Get next 2 codes.
        x = pCodes[currentCode + 1];
        y = pCodes[currentCode + 2];
    }

    // Move past the last 2 codes.
    currentCode += 2;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ShapeConverter::DoConversion(size_t numBytes, CharCP buff, bool doVerticals, bool hasData) {
    m_isPenDown = true;
    m_hasData = hasData;
    m_processVertical = doVerticals;

    // If there is a code name for the glyph then skip it
    size_t iOffset = (codeNameExists(buff, numBytes) ? (strlen(buff) + 1) : 1);

    numBytes -= iOffset;
    buff += iOffset;

    for (int currentCode = 0; currentCode < (int)numBytes; ++currentCode)
        DecodeCodes(currentCode, buff);
}

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ShxGlyph : DbGlyph {
private:
    typedef ShxFont::T_Id T_Id;
    T_Id m_glyphId;
    ShxFontR m_font;
    uint32_t m_dataOffset;
    uint32_t m_dataSize;
    mutable bool m_areMetricsValid = false;
    mutable DRange2d m_range;
    mutable DRange2d m_exactRange;
    mutable bool m_isBlank;
    void EnsureMetrics() const;

public:
    ShxGlyph(ShxFontR font) : m_font(font), m_glyphId(0), m_dataOffset(0), m_dataSize(0), m_isBlank(true) {}
    ShxGlyph(ShxFontR font, T_Id glyphId, uint32_t dataOffset, uint32_t dataSize) : m_glyphId(glyphId), m_font(font), m_dataOffset(dataOffset), m_dataSize(dataSize) {}
    uint32_t GetId() const override { return m_glyphId; }
    DRange2d GetRange() const override {
        EnsureMetrics();
        return m_range;
    }
    DRange2d GetExactRange() const override {
        EnsureMetrics();
        return m_exactRange;
    }
    CurveVectorPtr GetCurveVector() const override;
    CurveVectorPtr GetCurveVectorWithBearings(double& rightBearing, double& leftBearing, DPoint2dR finalXY) const;
    bool IsBlank() const override {
        EnsureMetrics();
        return m_isBlank;
    }
    DoFixup _DoFixup() const override { return DoFixup::Never; }
};

static ShxGlyph* createBlankGlyph(ShxFontR font) {
    return new ShxGlyph(font);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ShxGlyph::EnsureMetrics() const {
    FontManager::FlagHolder areValid(m_areMetricsValid);
    if (areValid)
        return;

    memset(&m_range, 0, sizeof(m_range));
    memset(&m_exactRange, 0, sizeof(m_exactRange));
    m_isBlank = true;

    if (0 == m_dataSize)
        return;
    ScopedArray<Byte> glyphData(m_dataSize);
    if (SUCCESS != m_font.Seek(m_dataOffset))
        return;

    if (0 == m_font.Read(glyphData.GetData(), m_dataSize))
        return;
    DPoint2d finalXY;
    double rightBearing, leftBearing;
    auto curves = GetCurveVectorWithBearings(rightBearing, leftBearing, finalXY);
    DRange2d tRange = DRange2d::NullRange();

    if (curves.IsValid()) {
        DRange3d range3d;
        curves->GetRange(range3d);

        tRange.low.x = range3d.low.x;
        tRange.low.y = range3d.low.y;
        tRange.high.x = range3d.high.x;
        tRange.high.y = range3d.high.y;
    }

    if (!tRange.IsEmpty() && !tRange.IsNull()) {
        m_exactRange = tRange;

        if (m_exactRange.high.x < rightBearing)
            m_exactRange.high.x = rightBearing;

        if (m_exactRange.low.x > leftBearing)
            m_exactRange.low.x = leftBearing;
    }

    m_range.low.Zero();
    m_range.high.x = finalXY.x;
    m_range.high.y = m_exactRange.high.y;

    // If the pen position is to the left of zero, that means we have a right-to-left character. Reverse the start/end of the black box too.
    if (m_range.high.x < 0.0) {
        double d = m_exactRange.low.x;
        m_exactRange.low.x = m_exactRange.high.x;
        m_exactRange.high.x = d;
    }

    m_isBlank = tRange.IsNull();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
CurveVectorPtr ShxGlyph::GetCurveVectorWithBearings(double& rightBearing, double& leftBearing, DPoint2dR finalXY) const {
    rightBearing = leftBearing = 0.0;
    finalXY.Zero();
    BeMutexHolder lock(FontManager::GetMutex());

    if (0 == m_dataSize)
        return nullptr;

    ScopedArray<Byte> glyphData(m_dataSize);
    if (SUCCESS != m_font.Seek(m_dataOffset))
        return nullptr;

    if (0 == m_font.Read(glyphData.GetData(), m_dataSize))
        return nullptr;
    ShapeConverter converter(m_font);
    converter.DoConversion(m_dataSize, (CharCP)glyphData.GetDataCP(), false, false);
    auto curves = converter.GrabCurveVector();
    if (!curves.IsValid())
        return nullptr;

    double scale = (1.0 / m_font.GetAscender());
    Transform scaleTransform = Transform::FromScaleFactors(scale, scale, 1.0);
    curves->TransformInPlace(scaleTransform);
    // For SHX files created from RSC files, we sometimes have to do a penDown/penUp to set the left and right side bearings to match what we would have had in the RSC glphy (e.g. in RSC glyphs LSB is always 0). In that case, there's no geometry in the GPA to set those extremes.
    rightBearing = (converter.GetRightBearing() * scale);
    leftBearing = (converter.GetLeftBearing() * scale);
    finalXY = converter.GetCurrentPos();
    finalXY.x *= scale;
    finalXY.y *= scale;
    return curves;
}

/**
 * Get a CurveVector for this glyph
 */
CurveVectorPtr ShxGlyph::GetCurveVector() const {
    double rightBearing, leftBearing;
    DPoint2d finalXY;
    return GetCurveVectorWithBearings(rightBearing, leftBearing, finalXY);
}

/**
 * Find a glyph in this SxhFont. If it is not loaded, it is created.
 */
DbGlyphCP ShxFont::FindShxGlyph(T_Id id) {
    BeMutexHolder lock(FontManager::GetMutex());
    auto foundGlyph = m_glyphCache.find(id);
    if (m_glyphCache.end() != foundGlyph)
        return foundGlyph->second;

    auto fpos = GetGlyphFPos(id);
    if (nullptr == fpos)
        return nullptr;

    ShxGlyph* glyph = new ShxGlyph(*this, id, fpos->m_dataOffset, fpos->m_dataSize);
    m_glyphCache[id] = glyph;

    return glyph;
}

//=======================================================================================
// @bsiclass
//=======================================================================================
enum {
    UNICODE_DEGREE = 0x00b0,
    UNICODE_DIAMETER = 0x2205,
    UNICODE_PLUSMINUS = 0x00b1
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ShxFont::T_Id ShxFont::Ucs4CharToFontChar(uint32_t ucs4Char, CharCP codePageString, bvector<Byte>& localeBuffer) {
    auto const& face = GetFace();

    // Special Unicode mappings.
    T_Id specialFontChar = 0;
    switch (ucs4Char) {
    case UNICODE_DEGREE:
        specialFontChar = (T_Id)face.m_encoding.m_degreeCode;
        break;
    case UNICODE_DIAMETER:
        specialFontChar = (T_Id)face.m_encoding.m_diameterCode;
        break;
    case UNICODE_PLUSMINUS:
        specialFontChar = (T_Id)face.m_encoding.m_plusMinusCode;
        break;
    }

    if (0 != specialFontChar)
        return specialFontChar;

    // Trivial ASCII?
    if (ucs4Char <= 0x007f)
        return (T_Id)ucs4Char;

    // Otherwise normal locale multi-byte.
    if (SUCCESS != BeStringUtilities::TranscodeStringDirect(localeBuffer, codePageString, (ByteCP)&ucs4Char, sizeof(ucs4Char), "UCS-4"))
        return 0;

    if (1 == localeBuffer.size())
        return (0x00ff & (T_Id)localeBuffer[0]);

    return *(T_Id const*)&localeBuffer[0];
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bvector<ShxFont::T_Id> ShxFont::Utf8ToFontChars(Utf8StringCR str) {
    bvector<T_Id> fontChars;

    // First, use UCS-4 to isolate each Unicode character.
    bvector<Byte> ucs4CharsBuffer;
    size_t numUcs4Chars;
    uint32_t const* ucs4Chars = Utf8ToUcs4(ucs4CharsBuffer, numUcs4Chars, str);
    if (0 == numUcs4Chars)
        return fontChars;

    auto& face = GetFace();
    // Then, for-each Unicode character, convert what the font expects.
    if (LangCodePage::Unicode == face.m_encoding.m_codePage) {
        // Assume UCS-2.
        fontChars.reserve(numUcs4Chars);
        for (size_t iUcs4Char = 0; iUcs4Char < numUcs4Chars; ++iUcs4Char)
            fontChars.push_back((T_Id)ucs4Chars[iUcs4Char]);

        return fontChars;
    }

    // Otherwise need to convert to locale.
    char codePageString[16]; // enough to hold "CP" plus max uint32_t for the sprintf below; but is typically of the form CP####
    if ((int)face.m_encoding.m_codePage <= 0)
        BeStringUtilities::Strncpy(codePageString, "ASCII");
    else
        BeStringUtilities::Snprintf(codePageString, _countof(codePageString), "CP%u", (uint32_t)face.m_encoding.m_codePage);

    fontChars.reserve(numUcs4Chars);
    bvector<Byte> localeBuffer;
    for (size_t iUcs4Char = 0; iUcs4Char < numUcs4Chars; ++iUcs4Char) {
        T_Id fontChar = Ucs4CharToFontChar(ucs4Chars[iUcs4Char], codePageString, localeBuffer);
        if (0 != fontChar)
            fontChars.push_back(fontChar);
    }

    return fontChars;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ShxFont::LayoutGlyphs(GlyphLayoutResultR result, GlyphLayoutContextCR context) {
    GetReader();

    // SHX glyphs can encoded via UCS-2 or locale... in either case, need to process one character at a time.
    bvector<T_Id> fontChars = Utf8ToFontChars(context.m_string);

    // Sometimes need to reserve a special blank glyph for missing glyphs.
    unique_ptr<ShxGlyph> blankGlyph;

    // Acquire the glyphs.
    result.m_glyphs.reserve(fontChars.size());
    for (T_Id fontChar : fontChars) {
        DbGlyph const* glyph = FindShxGlyph(fontChar);

        // Per SHX rules, if the glyph doesn't exist, substitute with the 0 glyph.
        if (nullptr == glyph) {
            if (!blankGlyph)
                blankGlyph.reset(createBlankGlyph(*this));

            glyph = blankGlyph.get();
        }

        result.m_glyphs.push_back(glyph);
    }

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

        // For SHX, at this point, glyphs should not be NULL; they should have been remapped to the blank glyph.
        BeAssert(nullptr != glyph);

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

        penPosition.x += (glyphRange.XLength());
    }

    // Fail-safe in case nothing accumulated above. Callers will expect a zero'ed range more than an inverted range.
    result.ZeroNullRanges();

    // Missing glyphs are allocated temporarily by this function...
    // I don't think it's worth the overhead to ref-count every glyph just to accommodate this bad data behavior.
    // The real information is already captured in glyph origins and ranges.
    // I believe the best compromise is to leave NULL glyphs in the output glyph vector in this scenario instead of complicating memory management.
    if (blankGlyph) {
        for (DbGlyphCP& glyph : result.m_glyphs) {
            if (blankGlyph.get() == glyph)
                glyph = nullptr;
        }
    }

    return SUCCESS;
}
