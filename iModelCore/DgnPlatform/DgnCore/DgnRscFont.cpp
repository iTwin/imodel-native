/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnRscFont.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/DgnFontData.h>
#include <DgnPlatform/DgnCore/DgnRscFontStructures.h>

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
DgnFontPtr DgnRscFont::Create(Utf8CP name, IDgnFontDataP data) { return new DgnRscFont(name, data); }
DgnFontPtr DgnRscFont::_Clone() const { return new DgnRscFont(*this); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
DgnRscFont::~DgnRscFont()
    {
    for (T_GlyphCache::reference cacheEntry : m_glyphCache)
        DELETE_AND_CLEAR(cacheEntry.second);
    
    if (m_isDefaultGlyphAllocated)
        DELETE_AND_CLEAR(m_defaultGlyph);
    }

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     03/2015
//=======================================================================================
struct DgnRscGlyph : DgnGlyph
{
private:
    T_Id m_glyphId;
    double m_ascenderScaleFactor;
    bool m_isFilled;
    RscGlyphDataOffset m_glyphDataOffset;
    IDgnRscFontData* m_data;
    DRange2d m_range;
    DRange2d m_exactRange;
    bool m_isBlank;
    
    DgnRscGlyph();

public:
    DgnRscGlyph(T_Id, double ascender, bool isFilled, RscGlyphHeader const&, RscGlyphDataOffset const&, IDgnRscFontData&);

    virtual T_Id _GetId() const override { return m_glyphId; }
    virtual DRange2d _GetRange() const override { return m_range; }
    virtual DRange2d _GetExactRange() const override { return m_exactRange; }
    virtual BentleyStatus _FillGpa(GPArrayR) const override;
    virtual bool _IsBlank() const override { return m_isBlank; }
    static DgnRscGlyph* CreateUnitSpaceGlyph();
    void SetWidthDirect(double width, double exactWidth) { m_range.high.x = width; m_exactRange.high.x = exactWidth; }
};

//---------------------------------------------------------------------------------------
// In the absence of a blank/space glyph, this "default" glyph serves as a unit cube to use in its place.
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
DgnRscGlyph* DgnRscGlyph::CreateUnitSpaceGlyph() { return new DgnRscGlyph(); }
DgnRscGlyph::DgnRscGlyph() :
    m_glyphId(0), m_ascenderScaleFactor(1.0 / 1.0), m_isFilled(false), m_glyphDataOffset({0, 0}), m_data(nullptr)
    {
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

//---------------------------------------------------------------------------------------
// We have to read the glyph headers to get glyphs code and information to begin with, so read the data up-front instead of lazier evaluation.
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
DgnRscGlyph::DgnRscGlyph(T_Id glyphId, double ascender, bool isFilled, RscGlyphHeader const& glyphHeader, RscGlyphDataOffset const& glyphDataOffset, IDgnRscFontData& data) :
m_glyphId(glyphId), m_ascenderScaleFactor(1.0 / ascender), m_isFilled(isFilled), m_glyphDataOffset(glyphDataOffset), m_data(&data)
    {
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

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct RscGlyphElementIterator
{
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
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
RscGlyphElementIterator RscGlyphElementIterator::ToNext()
    {
    if (0 == m_numRemaining)
        return RscGlyphElementIterator(NULL, 0);

    RscGlyphElement const* next = (RscGlyphElement const*)((Byte*)m_curr + sizeof(RscGlyphElement) + (sizeof(RscFontPoint2d) * (m_curr->numVerts - 1)));
    return RscGlyphElementIterator(next, m_numRemaining);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
static void addPtsToGPA(GPArrayR gpa, bvector<DPoint2d>& pts)
    {
    DPoint3d dPoint;
    dPoint.z = 0.0;

    for (bvector<DPoint2d>::const_iterator curr = pts.begin(); pts.end() != curr; ++curr)
        {
        dPoint.x = curr->x;
        dPoint.y = curr->y;

        gpa.Add(&dPoint);
        }
    }

//---------------------------------------------------------------------------------------
// Outer polys should be CW, and holes should be CCW.
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
static bool fixDirection(DPoint2dP pts, size_t nPts, RscGlyphElementType ptType)
    {
    bool cw;
    switch (ptType)
        {
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

    double area = PolygonOps::Area (pts, (int)nPts);
    if (area == 0.0)
        return true;

    if ((0 < area) == cw)
        return false;

    // Reverse direction.
    for (size_t i = 0, j = (nPts - 1); i < j; ++i, --j)
        {
        DPoint2d xy = pts[i];
        pts[i] = pts[j];
        pts[j] = xy;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnRscGlyph::_FillGpa(GPArrayR gpa) const
    {
    // Can be null when no default glyph exists and layout needs a blank unit cube glyph.
    if ((nullptr == m_data) || (0 == m_glyphDataOffset.size))
        return SUCCESS;

    bvector<Byte> glyphDataBuffer;
    glyphDataBuffer.reserve(m_glyphDataOffset.size);
    if (SUCCESS != m_data->_ReadGlyphData(glyphDataBuffer, (size_t)m_glyphDataOffset.offset, (size_t)m_glyphDataOffset.size))
        return  ERROR;

    RscGlyphData const* glyphData = (RscGlyphData const*)&glyphDataBuffer[0];
    if (0 == glyphData->numElems)
        return ERROR;

    bool hasLines = false;
    bool hasPolys = false;

    for (RscGlyphElementIterator curr((RscGlyphElement const*)(glyphData + 1), (size_t)glyphData->numElems); curr.IsValid(); curr = curr.ToNext())
        {
        size_t nPts = (size_t)curr->numVerts;
        RscFontPoint2d const* glyphPoint = curr->vertice;
        RscFontPoint2d const* glyphEnd = (glyphPoint + nPts);
        bvector<DPoint2d> tPts;

        for (; glyphPoint < glyphEnd; ++glyphPoint)
            {
            DPoint2d pt2d;
            pt2d.x = (glyphPoint->x * m_ascenderScaleFactor);
            pt2d.y = (glyphPoint->y * m_ascenderScaleFactor);

            tPts.push_back(pt2d);
            }

        if (fixDirection(&tPts[0], nPts, (RscGlyphElementType)curr->type))
            continue;

        int firstPtNum = gpa.GetCount();
        addPtsToGPA(gpa, tPts);

        gpa.MarkBreak();

        GraphicsPoint* firstPt = gpa.GetPtr(firstPtNum);
        switch (curr->type)
            {
            case RSCGLYPHELEMENT_Line:
                hasLines = true;
                firstPt->mask |= HMASK_RSC_LINE;
                break;

            case RSCGLYPHELEMENT_Poly:
            case RSCGLYPHELEMENT_PolyHoles:
                hasPolys = true;
                firstPt->mask |= HMASK_RSC_POLY;
                gpa.MarkMajorBreak();
                break;

            case RSCGLYPHELEMENT_PolyHole:
            case RSCGLYPHELEMENT_PolyHoleFinal:
                firstPt->mask |= HMASK_RSC_HOLE;
                gpa.MarkMajorBreak();
                break;
            }
        }

    // Add major break so we can transform individual glyphs.
    gpa.MarkMajorBreak();

    // Since QV won't draw any lines if fill is on, we can't turn it on if the glyph has any lines in it.
    if (!hasLines || (hasPolys && m_isFilled))
        gpa.SetArrayMask(HPOINT_ARRAYMASK_FILL);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
void DgnRscFont::LoadGlyphs() const
    {
    // Because we have to load and O(n) iterate the glyph header blob to even know what glyphs are in the file, do it once up-front instead of repeatedly for each glyph.
    // The glyphs will still load their glyph geometry data on-demand.
    if (m_hasLoadedGlyphs)
        return;

    m_hasLoadedGlyphs = true;

    IDgnRscFontData* data = (IDgnRscFontData*)m_data;
    if (nullptr == data)
        return;

    bvector<Byte> fontHeaderBuffer;
    if (SUCCESS != data->_ReadFontHeader(fontHeaderBuffer))
        return;

    bvector<Byte> glyphHeadersBuffer;
    if (SUCCESS != data->_ReadGlyphHeaders(glyphHeadersBuffer))
        return;

    bvector<Byte> glyphOffsetsBuffer;
    if (SUCCESS != data->_ReadGlyphDataOffsets(glyphOffsetsBuffer))
        return;

    RscFontHeader const& fontHeader = *(RscFontHeader const*)&fontHeaderBuffer[0];
    RscGlyphHeader const* glyphHeaders = (RscGlyphHeader const*)&glyphHeadersBuffer[0];
    RscGlyphDataOffset const* glyphOffsets = (RscGlyphDataOffset const*)&glyphOffsetsBuffer[0];
    for (size_t iGlyph = 0; iGlyph < fontHeader.totalChars; ++iGlyph)
        m_glyphCache[glyphHeaders[iGlyph].code] = new DgnRscGlyph(glyphHeaders[iGlyph].code, fontHeader.ascender, fontHeader.filledFlag, glyphHeaders[iGlyph], glyphOffsets[iGlyph], *data);
    
    m_defaultGlyph = FindGlyphCP(fontHeader.defaultChar);
    if (nullptr == m_defaultGlyph)
        {
        m_defaultGlyph = DgnRscGlyph::CreateUnitSpaceGlyph();
        m_isDefaultGlyphAllocated = true;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
DgnGlyphCP DgnRscFont::FindGlyphCP(DgnGlyph::T_Id id) const
    {
    LoadGlyphs();

    T_GlyphCache::const_iterator foundGlyph = m_glyphCache.find(id);
    if (m_glyphCache.end() != foundGlyph)
        return foundGlyph->second;
    
    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
DgnGlyphCP DgnRscFont::GetDefaultGlyphCP() const
    {
    LoadGlyphs();
    return m_defaultGlyph;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
DgnGlyph::T_Id DgnRscFont::FindFractionGlyphCode(uint8_t numerator, uint8_t denominator) const
    {
    // Read all of the fractions up-front to avoid the repeated data lookups later; should be cheap enough to be up-front vs. on-demand.
    if (!m_hasLoadedFractions)
        {
        m_hasLoadedFractions = true;

        IDgnRscFontData* data = (IDgnRscFontData*)m_data;
        if (nullptr == data)
            return 0;

        bvector<Byte> fontHeaderBuffer;
        if (SUCCESS != data->_ReadFontHeader(fontHeaderBuffer))
            return 0;

        RscFontHeader& fontHeader = *reinterpret_cast<RscFontHeader*>(&fontHeaderBuffer[0]);
        if (0 == fontHeader.fractMap)
            return 0;
        
        bvector<Byte> fractionsBuffer;
        if (SUCCESS != data->_ReadFractionMap(fractionsBuffer))
            return 0;

        RscFontFraction const* fractions = (RscFontFraction const*)&fractionsBuffer[0];
        for (size_t iFract = 0; iFract < MAX_FRACTION_MAP_COUNT; ++iFract)
            {
            if (0 == fractions[iFract].denominator)
                break;

            T_FractionMapKey key(fractions[iFract].numerator, fractions[iFract].denominator);
            DgnGlyph::T_Id value = fractions[iFract].charCode;
            m_fractions[key] = value;
            }
        }

    T_FractionMap::const_iterator foundFraction = m_fractions.find(T_FractionMapKey(numerator, denominator));
    if (m_fractions.end() != foundFraction)
        return foundFraction->second;
    
    return 0;
    }

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     05/2015
//=======================================================================================
enum
{
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

    RSC_CHAR_FirstStandardRscFraction = 0x81, // While technically customizable per-font, I have never seen this in practice, and the font utility no longer allows it.
    RSC_CHAR_LastStandardRscFraction = 0xbf, // While technically customizable per-font, I have never seen this in practice, and the font utility no longer allows it.
    RSC_CHAR_PrivateUseFirstRscFraction = 0xe100,
    RSC_CHAR_PrivateUseLastRscFraction = (RSC_CHAR_PrivateUseFirstRscFraction + (RSC_CHAR_LastStandardRscFraction - RSC_CHAR_FirstStandardRscFraction)),
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
DgnGlyph::T_Id DgnRscFont::Ucs4CharToFontChar(uint32_t ucs4Char, CharCP codePageString, bvector<Byte>& localeBuffer) const
    {
    // Special RSC fraction mappings.
    if ((ucs4Char >= RSC_CHAR_PrivateUseFirstRscFraction) && (ucs4Char <= RSC_CHAR_PrivateUseLastRscFraction))
        return (DgnGlyph::T_Id)(RSC_CHAR_FirstStandardRscFraction + (ucs4Char - RSC_CHAR_PrivateUseFirstRscFraction));

    // Special Unicode mappings.
    DgnGlyph::T_Id specialFontChar = 0;
    switch (ucs4Char)
        {
        case UNICODE_DEGREE: specialFontChar = m_metadata.m_degreeCode; break;
        case UNICODE_DIAMETER: specialFontChar = m_metadata.m_diameterCode; break;
        case UNICODE_FRACTION_1_4: specialFontChar = FindFractionGlyphCode(1, 4); break;
        case UNICODE_FRACTION_1_2: specialFontChar = FindFractionGlyphCode(1, 2); break;
        case UNICODE_FRACTION_3_4: specialFontChar = FindFractionGlyphCode(3, 4); break;
        case UNICODE_FRACTION_1_7: specialFontChar = FindFractionGlyphCode(1, 7); break;
        case UNICODE_FRACTION_1_9: specialFontChar = FindFractionGlyphCode(1, 9); break;
        case UNICODE_FRACTION_1_10: specialFontChar = FindFractionGlyphCode(1, 10); break;
        case UNICODE_FRACTION_1_3: specialFontChar = FindFractionGlyphCode(1, 3); break;
        case UNICODE_FRACTION_2_3: specialFontChar = FindFractionGlyphCode(2, 3); break;
        case UNICODE_FRACTION_1_5: specialFontChar = FindFractionGlyphCode(1, 5); break;
        case UNICODE_FRACTION_2_5: specialFontChar = FindFractionGlyphCode(2, 5); break;
        case UNICODE_FRACTION_3_5: specialFontChar = FindFractionGlyphCode(3, 5); break;
        case UNICODE_FRACTION_4_5: specialFontChar = FindFractionGlyphCode(4, 5); break;
        case UNICODE_FRACTION_1_6: specialFontChar = FindFractionGlyphCode(1, 6); break;
        case UNICODE_FRACTION_5_6: specialFontChar = FindFractionGlyphCode(5, 6); break;
        case UNICODE_FRACTION_1_8: specialFontChar = FindFractionGlyphCode(1, 8); break;
        case UNICODE_FRACTION_3_8: specialFontChar = FindFractionGlyphCode(3, 8); break;
        case UNICODE_FRACTION_5_8: specialFontChar = FindFractionGlyphCode(5, 8); break;
        case UNICODE_FRACTION_7_8: specialFontChar = FindFractionGlyphCode(7, 8); break;
        case UNICODE_PLUSMINUS: specialFontChar = m_metadata.m_plusMinusCode; break;
        }

    if (0 != specialFontChar)
        return specialFontChar;

    // Trivial ASCII?
    if (ucs4Char <= 0x007f)
        return (DgnGlyph::T_Id)ucs4Char;

    // Otherwise normal locale multi-byte.
    if (SUCCESS != BeStringUtilities::TranscodeStringDirect(localeBuffer, codePageString, (ByteCP)&ucs4Char, sizeof(ucs4Char), "UCS-4"))
        return 0;

    if (1 == localeBuffer.size())
        return (0x00ff & (DgnGlyph::T_Id)localeBuffer[0]);

    return *(DgnGlyph::T_Id const*)&localeBuffer[0];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
bvector<DgnGlyph::T_Id> DgnRscFont::Utf8ToFontChars(Utf8StringCR str) const
    {
    bvector<uint16_t> fontChars;
    
    // First, use UCS-4 to isolate each Unicode character.
    bvector<Byte> ucs4CharsBuffer;
    size_t numUcs4Chars;
    uint32_t const* ucs4Chars = DgnFont::Utf8ToUcs4(ucs4CharsBuffer, numUcs4Chars, str);
    if (0 == numUcs4Chars)
        return fontChars;

    // Then, for-each Unicode character, convert to locale / RSC glyph index.
    char codePageString[16]; // enough to hold "CP" plus max uint32_t for the sprintf below; but is typically of the form CP####
    if ((int)m_metadata.m_codePage <= 0)
        BeStringUtilities::Strncpy(codePageString, "ASCII");
    else
        BeStringUtilities::Snprintf(codePageString, _countof(codePageString), "CP%u", (uint32_t)m_metadata.m_codePage);

    fontChars.reserve(numUcs4Chars);
    bvector<Byte> localeBuffer;
    for (size_t iUcs4Char = 0; iUcs4Char < numUcs4Chars; ++iUcs4Char)
        {
        DgnGlyph::T_Id fontChar = Ucs4CharToFontChar(ucs4Chars[iUcs4Char], codePageString, localeBuffer);
        if (0 != fontChar)
            fontChars.push_back(fontChar);
        }
    
    return fontChars;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnRscFont::_LayoutGlyphs(DgnGlyphLayoutResultR result, DgnGlyphLayoutContextCR context) const
    {
    if (!IsResolved())
        return ERROR;

    // If you make any changes to this method, also consider examining DgnTrueTypeFont::_LayoutGlpyhs and DgnShxFont::_LayoutGlyphs.
    //  This method differs from the V8i variants in that it is designed to compute only the low-level information needed,
    //  and to serve both TextString and TextBlock through a single code path. This does mean that some extraneous information
    //  is potentially computed, but should be cheap compared to the overall layout operation.

    // RSC glyphs codes are encoded as 16-bit multi-byte locale. In other words, we have to take each Unicode character, convert to locale, and make a uint16_t entry for each (single byte or otherwise).
    bvector<DgnGlyph::T_Id> fontChars = Utf8ToFontChars(context.m_string);
    
    // Sometimes need to reserve a special blank glyph because if an RSC font does not provide a space glyph, it must be adjusted based on the previous character.
    unique_ptr<DgnRscGlyph> blankGlyph;
    
    // Acquire the glyphs.
    result.m_glyphs.reserve(fontChars.size());
    for (DgnGlyph::T_Id fontChar : fontChars)
        {
        DgnGlyph const* glyph = FindGlyphCP(fontChar);
        
        // Per RSC rules, if the glyph doesn't exist...
        if (nullptr == glyph)
            {
            // If no space character, assign to a local copy that will be our running variable space character.
            if (' ' == fontChar)
                {
                if (!blankGlyph)
                    blankGlyph.reset(DgnRscGlyph::CreateUnitSpaceGlyph());
                
                glyph = blankGlyph.get();
                }
            // Otherwise substitute with the default glyph.
            else
                {
                glyph = GetDefaultGlyphCP();
                }
            }
        
        result.m_glyphs.push_back(glyph);
        }

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
        
        // For RSC, at this point, glyphs should not be NULL; they should have been remapped to the default or blank glyph.
        BeAssert(nullptr != glyph);

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

        penPosition.x += (glyphRange.XLength());

        // Update variable space glyph if it's being used.
        if (blankGlyph)
            blankGlyph->SetWidthDirect(glyph->GetRange().high.x, glyph->GetExactRange().high.x);
        }

    // Fake space glyphs have variable widths and are allocated temporarily by this function...
    // I don't think it's worth the overhead to ref-count every glyph just to accommodate this weird RSC behavior.
    // The real information is already captured in glyph origins and ranges.
    // I believe the best compromise is to leave NULL glyphs in the output glyph vector in this scenario instead of complicating memory management.
    if (blankGlyph)
        {
        for (DgnGlyphCP& glyph : result.m_glyphs)
            {
            if (blankGlyph.get() == glyph)
                glyph = nullptr;
            }
        }
    
    return SUCCESS;
    }
