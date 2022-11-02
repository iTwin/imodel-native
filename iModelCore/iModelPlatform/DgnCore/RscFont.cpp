/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnRscFontStructures.h>
#include <DgnPlatform/DgnFont.fb.h>

using namespace flatbuffers;

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
RscFont::~RscFont() {
    for (auto& cacheEntry : m_glyphCache)
        DELETE_AND_CLEAR(cacheEntry.second);

    if (m_isDefaultGlyphAllocated)
        DELETE_AND_CLEAR(m_defaultGlyph);
}

//=======================================================================================
// @bsiclass
//=======================================================================================
struct RscGlyph : DbGlyph {
private:
    typedef uint16_t T_Id;

    RscFontR m_font;
    T_Id m_glyphId;
    double m_ascenderScaleFactor;
    RscGlyphDataOffset m_glyphDataOffset;
    DRange2d m_range;
    DRange2d m_exactRange;
    bool m_isBlank;

public:
    RscGlyph(RscFontR font) : m_font(font), m_glyphId(0), m_ascenderScaleFactor(1.0 / 1.0), m_glyphDataOffset({0, 0}) {
        m_range.low.x = 0.0;
        m_range.low.y = 0.0;
        m_range.high.x = 1.0;
        m_range.high.y = 1.0;
        m_exactRange.low.x = 0.0;
        m_exactRange.low.y = 0.0;
        m_exactRange.high.x = 1.0;
        m_exactRange.high.y = 1.0;
        m_isBlank = false;
    }

    RscGlyph(RscFontR font, T_Id glyphId, double ascender, RscGlyphHeader const& glyphHeader, RscGlyphDataOffset const& glyphDataOffset) : m_font(font),
                                                                                                                                           m_glyphId(glyphId), m_ascenderScaleFactor(1.0 / ascender), m_glyphDataOffset(glyphDataOffset) {
        m_range.low.x = 0.0;
        m_range.low.y = 0.0;
        m_range.high.x = (glyphHeader.width * m_ascenderScaleFactor);
        m_range.high.y = 1.0;

        m_exactRange.low.x = (glyphHeader.leftEdge * m_ascenderScaleFactor);
        m_exactRange.low.y = 0.0;
        m_exactRange.high.x = (glyphHeader.rightEdge * m_ascenderScaleFactor);
        m_exactRange.high.y = 1.0;

        m_isBlank = (m_range.XLength() <= 0.0);
    }

    uint32_t GetId() const override { return m_glyphId; }
    DRange2d GetRange() const override { return m_range; }
    DRange2d GetExactRange() const override { return m_exactRange; }
    CurveVectorPtr GetCurveVector() const override;
    bool IsBlank() const override { return m_isBlank; }
    void SetWidthDirect(double width, double exactWidth) {
        m_range.high.x = width;
        m_exactRange.high.x = exactWidth;
    }
    DoFixup _DoFixup() const override { return DoFixup::IfHoles; }
};

//---------------------------------------------------------------------------------------
// In the absence of a blank/space glyph, this "default" glyph serves as a unit cube to use in its place.
// @bsimethod
//---------------------------------------------------------------------------------------
static RscGlyph* createUnitSpaceGlyph(RscFontR font) {
    return new RscGlyph(font);
}

//=======================================================================================
// @bsiclass
//=======================================================================================
struct RscGlyphElementIterator {
private:
    RscGlyphElement const* m_curr;
    size_t m_numRemaining;

public:
    RscGlyphElementIterator(RscGlyphElement const* curr, size_t numRemaining) : m_curr(curr), m_numRemaining(numRemaining - 1) {}
    bool IsValid() const { return (NULL != m_curr); }
    RscGlyphElement const* operator->() const { return m_curr; }
    RscGlyphElementIterator ToNext();
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
RscGlyphElementIterator RscGlyphElementIterator::ToNext() {
    if (0 == m_numRemaining)
        return RscGlyphElementIterator(NULL, 0);

    RscGlyphElement const* next = (RscGlyphElement const*)((Byte const*)m_curr + sizeof(RscGlyphElement) + (sizeof(RscFontPoint2d) * (m_curr->numVerts - 1)));
    return RscGlyphElementIterator(next, m_numRemaining);
}

//---------------------------------------------------------------------------------------
// Outer polys should be CW, and holes should be CCW.
// @bsimethod
//---------------------------------------------------------------------------------------
static bool fixDirection(DPoint2dP pts, size_t nPts, RscGlyphElementType ptType) {
    bool cw;
    switch (ptType) {
    case RSCGLYPHELEMENT_Poly:
    case RSCGLYPHELEMENT_PolyHoles:
        cw = true;
        break;

    case RSCGLYPHELEMENT_PolyHole:
    case RSCGLYPHELEMENT_PolyHoleFinal:
        cw = false;

    default:
        // Neither a poly nor a hole.
        return false;
    }

    double area = PolygonOps::Area(pts, (int)nPts);
    if (area == 0.0)
        return true;

    if ((0 < area) == cw)
        return false;

    // Reverse direction.
    for (size_t i = 0, j = (nPts - 1); i < j; ++i, --j) {
        DPoint2d xy = pts[i];
        pts[i] = pts[j];
        pts[j] = xy;
    }

    return false;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static void CaptureAndReset(bvector<CurveVectorPtr>& completedPaths, CurveVectorPtr& currentPath) {
    if (currentPath.IsValid() && currentPath->size() > 0)
        completedPaths.push_back(currentPath);
    currentPath = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
CurveVectorPtr RscGlyph::GetCurveVector() const {
    bvector<CurveVectorPtr> completedPaths;
    CurveVectorPtr currentPath = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);

    // Can be null when no default glyph exists and layout needs a blank unit cube glyph.
    if (0 == m_glyphDataOffset.size)
        return currentPath; // EDL -- really?  How is this going to flow through?

    bvector<Byte> glyphDataBuffer;
    glyphDataBuffer.reserve(m_glyphDataOffset.size);
    if (SUCCESS != m_font.ReadGlyphData(glyphDataBuffer, (size_t)m_glyphDataOffset.offset, (size_t)m_glyphDataOffset.size))
        return nullptr;

    RscGlyphData const* glyphData = (RscGlyphData const*)&glyphDataBuffer[0];
    if (0 == glyphData->numElems)
        return nullptr;

    bool hasPolys = false;

    bvector<DPoint2d> tPts;
    for (RscGlyphElementIterator curr((RscGlyphElement const*)(glyphData + 1), (size_t)glyphData->numElems); curr.IsValid(); curr = curr.ToNext()) {
        size_t nPts = (size_t)curr->numVerts;
        RscFontPoint2d const* glyphPoint = curr->vertice;
        RscFontPoint2d const* glyphEnd = (glyphPoint + nPts);

        tPts.clear();
        for (; glyphPoint < glyphEnd; ++glyphPoint)
            tPts.push_back(DPoint2d::From(glyphPoint->x * m_ascenderScaleFactor, glyphPoint->y * m_ascenderScaleFactor));

        if (fixDirection(&tPts[0], nPts, (RscGlyphElementType)curr->type))
            continue;
        currentPath->Add(ICurvePrimitive::CreateLineString(tPts, 0.0));

        switch (curr->type) {
        case RSCGLYPHELEMENT_Line:
            break;

        case RSCGLYPHELEMENT_Poly:
        case RSCGLYPHELEMENT_PolyHoles:
            hasPolys = true;
            CaptureAndReset(completedPaths, currentPath);
            break;

        case RSCGLYPHELEMENT_PolyHole:
        case RSCGLYPHELEMENT_PolyHoleFinal:
            CaptureAndReset(completedPaths, currentPath);
            break;
        }
    }

    CaptureAndReset(completedPaths, currentPath);

    if (completedPaths.size() == 0)
        return nullptr;

    // Is it possible to have a mix of poly and path?
    if (hasPolys) {
        for (auto& loop : completedPaths)
            loop->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Outer);
    }

    if (completedPaths.size() == 1)
        return completedPaths[0];

    auto result = CurveVector::Create(hasPolys ? CurveVector::BOUNDARY_TYPE_ParityRegion : CurveVector::BOUNDARY_TYPE_None);
    for (auto& child : completedPaths)
        result->Add(child);
    return result;
}

/** Load all the glyph headers and offsets for this RscFont */
void RscFont::LoadGlyphs() {
    FontManager::FlagHolder loadedGlyphs(m_hasLoadedGlyphs);
    if (loadedGlyphs)
        return;

    GetReader();

    bvector<Byte> fontHeaderBuffer;
    if (SUCCESS != ReadFontHeader(fontHeaderBuffer))
        return;

    bvector<Byte> glyphHeadersBuffer;
    if (SUCCESS != ReadGlyphHeaders(glyphHeadersBuffer))
        return;

    bvector<Byte> glyphOffsetsBuffer;
    if (SUCCESS != ReadGlyphDataOffsets(glyphOffsetsBuffer))
        return;

    RscFontHeader const& fontHeader = *(RscFontHeader const*)&fontHeaderBuffer[0];
    RscGlyphHeader const* glyphHeaders = (RscGlyphHeader const*)&glyphHeadersBuffer[0];
    RscGlyphDataOffset const* glyphOffsets = (RscGlyphDataOffset const*)&glyphOffsetsBuffer[0];

    RscFontVec ascender = fontHeader.ascender;
    RscFontVec descender = fontHeader.descender;
    if (0 == descender)
        descender = fontHeader.maxbrr.bottom;

    m_descenderRatio = fabs(descender / (double)ascender);

    for (size_t iGlyph = 0; iGlyph < fontHeader.totalChars; ++iGlyph)
        m_glyphCache[glyphHeaders[iGlyph].code] = new RscGlyph(*this, glyphHeaders[iGlyph].code, fontHeader.ascender, glyphHeaders[iGlyph], glyphOffsets[iGlyph]);

    auto foundDefaultGlyph = m_glyphCache.find(fontHeader.defaultChar);
    m_defaultGlyph = foundDefaultGlyph != m_glyphCache.end() ? foundDefaultGlyph->second : nullptr;
    if (nullptr == m_defaultGlyph) {
        m_defaultGlyph = createUnitSpaceGlyph(*this);
        m_isDefaultGlyphAllocated = true;
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbGlyphCP RscFont::FindRscGlyph(T_Id id) {
    LoadGlyphs();

    T_GlyphCache::const_iterator foundGlyph = m_glyphCache.find(id);
    if (m_glyphCache.end() != foundGlyph)
        return foundGlyph->second;

    return nullptr;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
RscFont::T_Id RscFont::FindFractionGlyphCode(uint8_t numerator, uint8_t denominator) {
    // Read all of the fractions up-front to avoid the repeated data lookups later; should be cheap enough to be up-front vs. on-demand.
    FontManager::FlagHolder loadedFractions(m_hasLoadedFractions);
    if (!loadedFractions) {
        bvector<Byte> fontHeaderBuffer;
        if (SUCCESS != ReadFontHeader(fontHeaderBuffer))
            return 0;

        RscFontHeader& fontHeader = *reinterpret_cast<RscFontHeader*>(&fontHeaderBuffer[0]);
        if (0 == fontHeader.fractMap)
            return 0;

        bvector<Byte> fractionsBuffer;
        if (SUCCESS != ReadFractionMap(fractionsBuffer))
            return 0;

        RscFontFraction const* fractions = (RscFontFraction const*)&fractionsBuffer[0];
        for (size_t iFract = 0; iFract < MAX_FRACTION_MAP_COUNT; ++iFract) {
            if (0 == fractions[iFract].denominator)
                break;

            T_FractionMapKey key(fractions[iFract].numerator, fractions[iFract].denominator);
            T_Id value = fractions[iFract].charCode;
            m_fractions[key] = value;
        }
    }

    T_FractionMap::const_iterator foundFraction = m_fractions.find(T_FractionMapKey(numerator, denominator));
    if (m_fractions.end() != foundFraction)
        return foundFraction->second;

    return 0;
}

//=======================================================================================
// @bsiclass
//=======================================================================================
enum {
    UNICODE_DEGREE = 0x00b0,
    UNICODE_DIAMETER = 0x2205,
    UNICODE_FRACTION_1_4 = 0x00bc,
    UNICODE_FRACTION_1_2 = 0x00bd,
    UNICODE_FRACTION_3_4 = 0x00be,
    UNICODE_FRACTION_1_7 = 0x2150,
    UNICODE_FRACTION_1_9 = 0x2151,
    UNICODE_FRACTION_1_10 = 0x2152,
    UNICODE_FRACTION_1_3 = 0x2153,
    UNICODE_FRACTION_2_3 = 0x2154,
    UNICODE_FRACTION_1_5 = 0x2155,
    UNICODE_FRACTION_2_5 = 0x2156,
    UNICODE_FRACTION_3_5 = 0x2157,
    UNICODE_FRACTION_4_5 = 0x2158,
    UNICODE_FRACTION_1_6 = 0x2159,
    UNICODE_FRACTION_5_6 = 0x215a,
    UNICODE_FRACTION_1_8 = 0x215b,
    UNICODE_FRACTION_3_8 = 0x215c,
    UNICODE_FRACTION_5_8 = 0x215d,
    UNICODE_FRACTION_7_8 = 0x215e,
    UNICODE_PLUSMINUS = 0x00b1,

    RSC_CHAR_FirstStandardRscFraction = 0x81,
    RSC_CHAR_LastStandardRscFraction = 0xbf,
    RSC_CHAR_PrivateUseFirstRscFraction = 0xe100,
    RSC_CHAR_PrivateUseLastRscFraction = (RSC_CHAR_PrivateUseFirstRscFraction + (RSC_CHAR_LastStandardRscFraction - RSC_CHAR_FirstStandardRscFraction)),
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
RscFont::T_Id RscFont::Ucs4CharToFontChar(uint32_t ucs4Char, CharCP codePageString, bvector<Byte>& localeBuffer) {
    // Special RSC fraction mappings.
    if ((ucs4Char >= RSC_CHAR_PrivateUseFirstRscFraction) && (ucs4Char <= RSC_CHAR_PrivateUseLastRscFraction))
        return (T_Id)(RSC_CHAR_FirstStandardRscFraction + (ucs4Char - RSC_CHAR_PrivateUseFirstRscFraction));

    auto& face = GetFace();
    // Special Unicode mappings.
    T_Id specialFontChar = 0;
    switch (ucs4Char) {
    case UNICODE_DEGREE:
        specialFontChar = (T_Id)face.m_encoding.m_degreeCode;
        break;
    case UNICODE_DIAMETER:
        specialFontChar = (T_Id)face.m_encoding.m_diameterCode;
        break;
    case UNICODE_FRACTION_1_4:
        specialFontChar = FindFractionGlyphCode(1, 4);
        break;
    case UNICODE_FRACTION_1_2:
        specialFontChar = FindFractionGlyphCode(1, 2);
        break;
    case UNICODE_FRACTION_3_4:
        specialFontChar = FindFractionGlyphCode(3, 4);
        break;
    case UNICODE_FRACTION_1_7:
        specialFontChar = FindFractionGlyphCode(1, 7);
        break;
    case UNICODE_FRACTION_1_9:
        specialFontChar = FindFractionGlyphCode(1, 9);
        break;
    case UNICODE_FRACTION_1_10:
        specialFontChar = FindFractionGlyphCode(1, 10);
        break;
    case UNICODE_FRACTION_1_3:
        specialFontChar = FindFractionGlyphCode(1, 3);
        break;
    case UNICODE_FRACTION_2_3:
        specialFontChar = FindFractionGlyphCode(2, 3);
        break;
    case UNICODE_FRACTION_1_5:
        specialFontChar = FindFractionGlyphCode(1, 5);
        break;
    case UNICODE_FRACTION_2_5:
        specialFontChar = FindFractionGlyphCode(2, 5);
        break;
    case UNICODE_FRACTION_3_5:
        specialFontChar = FindFractionGlyphCode(3, 5);
        break;
    case UNICODE_FRACTION_4_5:
        specialFontChar = FindFractionGlyphCode(4, 5);
        break;
    case UNICODE_FRACTION_1_6:
        specialFontChar = FindFractionGlyphCode(1, 6);
        break;
    case UNICODE_FRACTION_5_6:
        specialFontChar = FindFractionGlyphCode(5, 6);
        break;
    case UNICODE_FRACTION_1_8:
        specialFontChar = FindFractionGlyphCode(1, 8);
        break;
    case UNICODE_FRACTION_3_8:
        specialFontChar = FindFractionGlyphCode(3, 8);
        break;
    case UNICODE_FRACTION_5_8:
        specialFontChar = FindFractionGlyphCode(5, 8);
        break;
    case UNICODE_FRACTION_7_8:
        specialFontChar = FindFractionGlyphCode(7, 8);
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
    if (SUCCESS != BeStringUtilities::TranscodeStringDirect(localeBuffer, codePageString, (Byte const*)&ucs4Char, sizeof(ucs4Char), "UCS-4"))
        return 0;

    if (1 == localeBuffer.size())
        return (0x00ff & (T_Id)localeBuffer[0]);

    return *(T_Id const*)&localeBuffer[0];
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bvector<RscFont::T_Id> RscFont::Utf8ToFontChars(Utf8StringCR str) {
    bvector<uint16_t> fontChars;

    // First, use UCS-4 to isolate each Unicode character.
    bvector<Byte> ucs4CharsBuffer;
    size_t numUcs4Chars;
    uint32_t const* ucs4Chars = Utf8ToUcs4(ucs4CharsBuffer, numUcs4Chars, str);
    if (0 == numUcs4Chars || nullptr == ucs4Chars)
        return fontChars;

    // Then, for-each Unicode character, convert to locale / RSC glyph index.
    char codePageString[16]; // enough to hold "CP" plus max uint32_t for the sprintf below; but is typically of the form CP####
    int codePage = (int)GetFace().m_encoding.m_codePage;
    if (codePage <= 0)
        BeStringUtilities::Strncpy(codePageString, "ASCII");
    else
        BeStringUtilities::Snprintf(codePageString, _countof(codePageString), "CP%u", codePage);

    fontChars.reserve(numUcs4Chars);
    bvector<Byte> localeBuffer;
    for (size_t iUcs4Char = 0; iUcs4Char < numUcs4Chars; ++iUcs4Char) {
        auto fontChar = Ucs4CharToFontChar(ucs4Chars[iUcs4Char], codePageString, localeBuffer);
        if (0 != fontChar)
            fontChars.push_back(fontChar);
    }

    return fontChars;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus RscFont::LayoutGlyphs(GlyphLayoutResultR result, GlyphLayoutContextCR context) {
    GetReader();

    // If you make any changes to this method, also consider examining TrueTypeFont::_LayoutGlpyhs and ShxFont::_LayoutGlyphs.
    // This method differs from the V8i variants in that it is designed to compute only the low-level information needed,
    // and to serve both TextString and TextBlock through a single code path. This does mean that some extraneous information
    // is potentially computed, but should be cheap compared to the overall layout operation.

    // RSC glyphs codes are encoded as 16-bit multi-byte locale. In other words, we have to take each Unicode character, convert to locale, and make a uint16_t entry for each (single byte or otherwise).
    bvector<T_Id> fontChars = Utf8ToFontChars(context.m_string);

    // Sometimes need to reserve a special blank glyph because if an RSC font does not provide a space glyph, it must be adjusted based on the previous character.
    unique_ptr<RscGlyph> blankGlyph;

    // Acquire the glyphs.
    result.m_glyphs.reserve(fontChars.size());
    for (auto fontChar : fontChars) {
        DbGlyphCP glyph = FindRscGlyph(fontChar);

        // Per RSC rules, if the glyph doesn't exist...
        if (nullptr == glyph) {
            // If no space character, assign to a local copy that will be our running variable space character.
            if (' ' == fontChar) {
                if (!blankGlyph)
                    blankGlyph.reset(createUnitSpaceGlyph(*this));

                glyph = blankGlyph.get();
            }
            // Otherwise substitute with the default glyph.
            else {
                glyph = GetDefaultGlyphCP();
            }
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

        // For RSC, at this point, glyphs should not be NULL; they should have been remapped to the default or blank glyph.
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

        // Update variable space glyph if it's being used.
        if (blankGlyph)
            blankGlyph->SetWidthDirect(glyph->GetRange().high.x, glyph->GetExactRange().high.x);
    }

    // Fail-safe in case nothing accumulated above. Callers will expect a zero'ed range more than an inverted range.
    result.ZeroNullRanges();

    // Fake space glyphs have variable widths and are allocated temporarily by this function...
    // I don't think it's worth the overhead to ref-count every glyph just to accommodate this weird RSC behavior.
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

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus RscFont::ReadFontHeader(bvector<Byte>& buffer) {
    auto const* rscFontData = GetRoot<FB::DgnRscFont>(GetReader().m_reader->FillAndGetDataP());
    if (nullptr == rscFontData)
        return ERROR;

    buffer.clear();
    ByteCP start = rscFontData->header()->Data();
    ByteCP end = start + rscFontData->header()->size();
    std::copy(start, end, std::back_inserter(buffer));
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus RscFont::ReadFractionMap(bvector<Byte>& buffer) {
    auto const* rscFontData = GetRoot<FB::DgnRscFont>(GetReader().m_reader->FillAndGetDataP());
    if (nullptr == rscFontData)
        return ERROR;

    buffer.clear();
    ByteCP start = rscFontData->fractionMap()->Data();
    ByteCP end = start + rscFontData->fractionMap()->size();
    std::copy(start, end, std::back_inserter(buffer));
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus RscFont::ReadGlyphData(bvector<Byte>& buffer, size_t offset, size_t size) {
    auto const* rscFontData = GetRoot<FB::DgnRscFont>(GetReader().m_reader->FillAndGetDataP());
    if (nullptr == rscFontData)
        return ERROR;

    buffer.clear();
    ByteCP start = rscFontData->glyphData()->Data() + offset;
    ByteCP end = start + size;
    std::copy(start, end, std::back_inserter(buffer));
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus RscFont::ReadGlyphDataOffsets(bvector<Byte>& buffer) {
    auto const* rscFontData = GetRoot<FB::DgnRscFont>(GetReader().m_reader->FillAndGetDataP());
    if (nullptr == rscFontData)
        return ERROR;

    buffer.clear();
    ByteCP start = rscFontData->glyphDataOffsets()->Data();
    ByteCP end = start + rscFontData->glyphDataOffsets()->size();
    std::copy(start, end, std::back_inserter(buffer));
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus RscFont::ReadGlyphHeaders(bvector<Byte>& buffer) {
    auto const* rscFontData = GetRoot<FB::DgnRscFont>(GetReader().m_reader->FillAndGetDataP());
    if (nullptr == rscFontData)
        return ERROR;

    buffer.clear();
    ByteCP start = rscFontData->glyphHeaders()->Data();
    ByteCP end = start + rscFontData->glyphHeaders()->size();
    std::copy(start, end, std::back_inserter(buffer));
    return SUCCESS;
}
