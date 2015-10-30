/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnShxFont.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/DgnFontData.h>

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
DgnFontPtr DgnShxFont::Create(Utf8CP name, IDgnFontDataP data) { return new DgnShxFont(name, data); }
DgnFontPtr DgnShxFont::_Clone() const { return new DgnShxFont(*this); }
DgnShxFont::ShxType DgnShxFont::GetShxType() const { return ((IDgnShxFontData*)m_data)->GetShxType(); }

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
enum ShxShapCode
{
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
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
static void bulgeFactorToDEllipse3d(DEllipse3dP pEllipse, DPoint2dCP pStart, DPoint2dCP pEnd, double bulgeFactor)
    {
    double sweep = (4.0 * atan(bulgeFactor));
    DPoint2d v;
    DPoint2d u;
    DPoint2d r;
    DPoint2d    x;
    DPoint2d center;
    double theta0;
    double radius;

    // Find tan (included angle / 2)
    double tangentHalfAngle = tan(sweep * 0.5);

    // U = chord bvector
    u.DifferenceOf (*pEnd, *pStart);
    u.Scale (u, 0.5);

    // V = perpendicular to chord bvector, same length.
    v.x = -u.y;
    v.y = u.x;

    // Center = p0 + U + -V/tangentHalfAngle
    center.SumOf(*pStart, u, 1.0, v, 1.0 / tangentHalfAngle);

    // Radius
    r.DifferenceOf (*pStart, center);
    radius = r.Magnitude ();

    // Start angle
    x.x = 1.0;
    x.y = 0.0;

    theta0 = x.AngleTo (r);

    // Setup for ellipse
    bsiDEllipse3d_init(pEllipse, center.x, center.y, 0.0, radius, 0.0, 0.0, 0.0, radius, 0.0, theta0, sweep);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
static bool codeNameExists(ByteCP pBytes, size_t maxNameLength)
    {
    if (0 == *pBytes)
        return false;

    for (size_t j = 0; j < maxNameLength; ++j)
        {
        if (0 == *pBytes++)
            return true;
        }

    return false;
    }

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct ShapeConverter
{
private:
    IDgnShxFontData* m_data;
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
    GPArrayP m_gpa;
    std::stack<DPoint2d> m_stack;

    void AddPoint(DPoint2dCR pt);
    void AddEllipse(DEllipse3dCR el);
    
public:
    ShapeConverter(GPArrayR, IDgnShxFontData&);
    DPoint2dCR GetCurrentPos() const { return m_currentPos; }
    double GetLeftBearing() const { return m_leftBearing; }
    double GetMultiplier() const { return m_multiplier; }
    int GetNumPoints() const { return m_gpa->GetCount(); }
    double GetRightBearing() const { return m_rightBearing; }
    DPoint2dCR GetVEndPt() const { return m_vertEndPt; }
    DPoint2dCR GetVStartPt() const { return m_vertStartPt; }
    bool HasVEnd() const { return m_vEnd; }
    bool HasVStart() const { return m_vStart; }
    bool IsPenDown() const { return m_isPenDown; }
    void DecodeBulgeArc(int& currentCode, ByteCP pCodes);
    void DecodeCodes(int& currentCode, ByteCP pCodes);
    void DecodeDivLengths(int& currentCode, ByteCP pCodes);
    void DecodeExtenedFontSubShape(int& currentCode, ByteCP pCodes);
    void DecodeFractionArc(int& currentCode, ByteCP pCodes);
    void DecodeMultiBulgeArc(int& currentCode, ByteCP pCodes);
    void DecodeMultLengths(int& currentCode, ByteCP pCodes);
    void DecodeOctantArc(int& currentCode, ByteCP pCodes);
    void DecodePenDown();
    void DecodePenUp();
    void DecodePop();
    void DecodePush();
    void DecodeSubShape(int& currentCode, ByteCP pCodes);
    void DecodeVector(Byte iCode, ByteCP);
    void DecodeVertical(int& currentCode, ByteCP pCodes);
    void DecodeXYDisp(int& currentCode, ByteCP pCodes);
    void DecodeXYMultiDisp(int& currentCode, ByteCP pCodes);
    void DoConversion(size_t numBytes, ByteCP buff, bool doVerticals, bool hasData);
    bool IsPointInGPA(DPoint2dCR point) const;
    bool NeedPointInGPA(DPoint2dCR point) const;
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
ShapeConverter::ShapeConverter(GPArrayR gpa, IDgnShxFontData& data) :
    m_data(&data), m_isPenDown(true), m_vStart(false), m_vEnd(false), m_hasData(false), m_processVertical(false), m_skipCodes(false),
    m_multiplier(1.0), m_leftBearing(INT_MAX), m_rightBearing(-INT_MAX), m_gpa(&gpa)
    {
    m_vertStartPt.Zero();
    m_vertEndPt.Zero();
    m_currentPos.Zero();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
void ShapeConverter::AddPoint(DPoint2dCR pt)
    {
    m_hasData = true;
    if (!m_processVertical)
        m_gpa->Add(&pt, 1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
void ShapeConverter::AddEllipse(DEllipse3dCR el)
    {
    m_hasData = true;
    if (!m_processVertical)
        m_gpa->Add(el);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodeBulgeArc(int& currentCode, ByteCP pCodes)
    {
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

    // *Note: We have empirically found that AutoCAD will always draw arcs, regardless of the pen up/down state.
    //  It does not, however, leave the pen down after arcs; therefore, simply ignore IsPenDown checks.
    //  Special case for bulge arcs: bulge arcs with a 0 bulge, which are effectively straight lines, obey the pen state.

    if (0 != b)
        {
        DEllipse3d sEllipse;
        bulgeFactorToDEllipse3d(&sEllipse, &sStart, &sEnd, (double) b / 127.0);

        if (NeedPointInGPA(m_currentPos))
            AddPoint(m_currentPos);

        AddEllipse(sEllipse);
        }
    else
        {
        if (IsPenDown() && !IsPointInGPA(m_currentPos))
            AddPoint(m_currentPos);

        if (IsPenDown())
            AddPoint(pt);
        }

    m_currentPos = pt;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodeCodes(int& currentCode, ByteCP pCodes)
    {
    Byte iCode = pCodes[currentCode];
    if ((int)(iCode & 0xF0) > 0)
        {
        DecodeVector(iCode, pCodes);
        return;
        }

    switch (iCode)
        {
        case SHAPECODE_ENDOFSHAPE: return;
        case SHAPECODE_PENDOWN:DecodePenDown(); break;
        case SHAPECODE_PENUP: DecodePenUp(); break;
        case SHAPECODE_DIVLENGTHS: DecodeDivLengths (currentCode, pCodes); break;
        case SHAPECODE_MULTLENGTHS: DecodeMultLengths (currentCode, pCodes); break;
        case SHAPECODE_PUSH: DecodePush (); break;
        case SHAPECODE_POP: DecodePop (); break;
        case SHAPECODE_SUBSHAPE: DecodeSubShape (currentCode, pCodes); break;
        case SHAPECODE_XYDISP: DecodeXYDisp (currentCode, pCodes); break;
        case SHAPECODE_MULTIXYDISP: DecodeXYMultiDisp (currentCode, pCodes); break;
        case SHAPECODE_OCTANTARC: DecodeOctantArc (currentCode, pCodes); break;
        case SHAPECODE_FRACTIONARC: DecodeFractionArc (currentCode, pCodes); break;
        case SHAPECODE_BULGEARC: DecodeBulgeArc (currentCode, pCodes); break;
        case SHAPECODE_MULTIBULGEARC: DecodeMultiBulgeArc (currentCode, pCodes); break;
        case SHAPECODE_VERTICAL: DecodeVertical (currentCode, pCodes); break;

        default:
            BeAssert(false && L"Unknown/unexpected opcode.");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodeDivLengths(int& currentCode, ByteCP pCodes)
    {
    Byte iCode = pCodes[++currentCode];
    BeAssert(0 != iCode);

    if (m_skipCodes)
        return;

    if (0 != iCode)
        m_multiplier /= (double)iCode;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodeExtenedFontSubShape(int&, ByteCP)
    {
    // As far as I can tell, this only applies to big fonts, which we no longer support.
    return;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodeFractionArc(int& currentCode, ByteCP pCodes)
    {
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
    iHighRadius  = pCodes[++currentCode];

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
    if (osc < 0)
        {
        isClockwise = true;
        iSweep = -iSweep;
        }

    if (!iOctants)
        iOctants = 8;

    iEndOctant = iStartingOctant + iSweep;

    if (isClockwise)
        {
        iOctants = -iOctants;
        if (iEndOctant < 0)
            iEndOctant += 8;
        }
    else
        {
        if (iEndOctant >= 8)
            iEndOctant -= 8;
        }

    // may need to floor this
    if (!isClockwise)
        {
        startOffsetDeg = (((iStartOffset * 45) / 256) + (iStartingOctant * 45));
        startOffset = Angle::DegreesToRadians(startOffsetDeg);
        endOffsetDeg = (((iEndOffset * 45.0) / 256.0) + ((0 == iEndOffset) ? iEndOctant : ((iStartingOctant + abs(iSweep) - 1)) * 45.0));
        endOffset = Angle::DegreesToRadians(endOffsetDeg);
        }
    else
        {
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

    if (!isClockwise)
        {
        if (endOffsetDeg <= startOffsetDeg)
            endOffsetDeg += 360;

        sweep = Angle::DegreesToRadians(endOffsetDeg - startOffsetDeg + 1);
        }
    else
        {
        // need a -ve angle
        if (startOffsetDeg <= 0)
            startOffsetDeg += 360;

        if (startOffsetDeg <= endOffsetDeg)
            sweep = Angle::DegreesToRadians(-(360 - (endOffsetDeg - startOffsetDeg)));
        else
            sweep = Angle::DegreesToRadians(endOffsetDeg - startOffsetDeg);
        }

    startOffset = Angle::DegreesToRadians(startOffsetDeg);

    x = (radius * cos (startOffset));
    y = (radius * sin (startOffset));
    center.x = (m_currentPos.x - x);
    center.y = (m_currentPos.y - y);
    center.z = 0.0;
    endOffset = Angle::DegreesToRadians (endOffsetDeg);
    
    x = (radius * cos (endOffset));
    y = (radius * sin (endOffset));
    sEnd = center;
    sEnd.x = (center.x + x);
    sEnd.y = (center.y + y);

    s0Vector.x = (m_currentPos.x - center.x);
    s0Vector.y = (m_currentPos.y - center.y);
    s90Vector.x = -s0Vector.y;
    s90Vector.y = s0Vector.x;
    s90Vector.z = 0.0;
    s0Vector.z = 0.0;

    bsiDEllipse3d_initFrom3dVectors(&sEllipse, &center, &s0Vector, &s90Vector, 0, sweep);

    // *Note: We have empirically found that AutoCAD will always draw arcs, regardless of the pen up/down state.
    //  It does not, however, leave the pen down after arcs; therefore, simply ignore IsPenDown checks.

    if (NeedPointInGPA(m_currentPos))
        AddPoint(m_currentPos);

    AddEllipse(sEllipse);

    m_currentPos.x = sEnd.x;
    m_currentPos.y = sEnd.y;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodeMultiBulgeArc(int& currentCode, ByteCP pCodes)
    {
    signed char x = pCodes[currentCode + 1];
    signed char y = pCodes[currentCode + 2];

    while ((0 != x) || (0 != y))
        {
        DecodeBulgeArc(currentCode, pCodes);

        // Get next 2 codes
        x = pCodes[currentCode + 1];
        y = pCodes[currentCode + 2];
        }
    
    // Move past the last 2 codes
    currentCode += 2;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodeMultLengths(int& currentCode, ByteCP pCodes)
    {
    Byte iCode = pCodes[++currentCode];

    if (m_skipCodes)
        return;

    BeAssert(0 != iCode);
    
    if (0 != iCode)
        m_multiplier *= (double)iCode;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodeOctantArc(int& currentCode, ByteCP pCodes)
    {
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

    if (b < 0)
        {
        isClockwise = true;
        iSweep = -iSweep;
        }

    if (!iOctants)
        iOctants = 8;

    char iEndOctant = (iStartingOctant + iSweep);

    if (isClockwise)
        {
        iOctants = -iOctants;
        if (iEndOctant < 0)
            iEndOctant += 8;
        }
    else
        {
        if (iEndOctant >= 8)
            iEndOctant -= 8;
        }

    DPoint3d center;
    center.Init(m_currentPos.x, m_currentPos.y, 0);

    switch (iStartingOctant)
        {
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
    switch (iEndOctant)
        {
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
    bsiDEllipse3d_initFrom3dVectors(&sEllipse, &center, &s0Vector, &s90Vector, 0, sweep);

    // *Note: We have empirically found that AutoCAD will always draw arcs, regardless of the pen up/down state.
    //  It does not, however, leave the pen down after arcs; therefore, simply ignore IsPenDown checks.

    if (NeedPointInGPA(m_currentPos))
        AddPoint(m_currentPos);

    AddEllipse(sEllipse);

    m_currentPos.x = sEnd.x;
    m_currentPos.y = sEnd.y;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodePenDown()
    {
    if (m_skipCodes)
        return;

    m_isPenDown = true;

    if (m_currentPos.x < m_leftBearing)
        m_leftBearing = m_currentPos.x;

    if (m_currentPos.x >  m_rightBearing)
        m_rightBearing = m_currentPos.x;

    m_gpa->MarkMajorBreak();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodePenUp()
    {
    if (m_skipCodes)
        return;

    m_isPenDown = false;
    m_gpa->MarkMajorBreak();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodePop()
    {
    if (m_skipCodes)
        return;

    if (m_stack.empty())
        return;

    m_currentPos = m_stack.top();
    m_stack.pop();

    // Mark break in GPA added due to BOLD.shx R character
    m_gpa->MarkMajorBreak();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodePush()
    {
    if (!m_skipCodes)
        m_stack.push(m_currentPos);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodeSubShape(int& currentCode, ByteCP pCodes)
    {
    uint16_t subshapeCode = pCodes[++currentCode];
    if (DgnShxFont::ShxType::Unicode == m_data->GetShxType())
        subshapeCode = ((subshapeCode << 8) + pCodes[++currentCode]);

   if (0 == subshapeCode)
        {
        DecodeExtenedFontSubShape(currentCode, pCodes);
        return;
        }

    DgnShxFont::GlyphFPos const* fPos = m_data->GetGlyphFPos((DgnGlyph::T_Id)subshapeCode);
    if (NULL == fPos)
        return;

    if (0 != m_data->_Seek(fPos->m_dataOffset, BeFileSeekOrigin::Begin))
        return;

    Byte* pBuf = reinterpret_cast<Byte*>(_alloca(sizeof (Byte) * fPos->m_dataSize));
    m_data->_Read(pBuf, sizeof (Byte) * fPos->m_dataSize, 1);

    DoConversion(fPos->m_dataSize, pBuf, m_processVertical, m_hasData);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodeVector(Byte iCode, ByteCP)
    {
    if (m_skipCodes)
        return;

    int iLength = ((iCode & 0xF0) >> 4);
    int iDirection = (iCode & 0x0F);
    double dLength = (iLength * m_multiplier);

    DPoint2d pt;
    switch (iDirection)
        {
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

    if (IsPenDown())
        {
        if (!IsPointInGPA(m_currentPos))
            AddPoint(m_currentPos);

        AddPoint(pt);
        }

    m_currentPos = pt;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodeVertical(int& currentCode, ByteCP pCodes)
    {
    // While we aren't supporting vertical SHX glyphs at this time, we still need to eat the codes.
    AutoRestore<bool> skipCodes(&m_skipCodes, true);
    ++currentCode;
    DecodeCodes(currentCode, pCodes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodeXYDisp(int& currentCode, ByteCP pCodes)
    {
    DPoint2d sDisp;
    DPoint2d pt;
    signed char b = 0;

    memset(&sDisp, 0, sizeof (DPoint2d));
    b = pCodes[++currentCode];

    sDisp.x = ((double)b * m_multiplier);
    b = pCodes[++currentCode];

    sDisp.y = ((double)b * m_multiplier);
    
    if (m_skipCodes)
        return;

    pt = m_currentPos;
    pt.x += sDisp.x;
    pt.y += sDisp.y;

    if (IsPenDown() && !IsPointInGPA(m_currentPos))
        AddPoint(m_currentPos);

    if (IsPenDown())
        AddPoint(pt);

    m_currentPos = pt;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
void ShapeConverter::DecodeXYMultiDisp(int& currentCode, ByteCP pCodes)
    {
    signed char x = pCodes[currentCode + 1];
    signed char y = pCodes[currentCode + 2];

    while ((0 != x) || (0 != y))
        {
        DecodeXYDisp(currentCode, pCodes);

        // Get next 2 codes.
        x = pCodes[currentCode + 1];
        y = pCodes[currentCode + 2];
        }

    // Move past the last 2 codes.
    currentCode += 2;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
void ShapeConverter::DoConversion(size_t numBytes, ByteCP buff, bool doVerticals, bool hasData)
    {
    m_isPenDown = true;
    m_hasData = hasData;
    m_processVertical = doVerticals;

    // If there is a code name for the glyph then skip it
    size_t iOffset = (codeNameExists ((unsigned char*)buff, numBytes) ? (strlen (reinterpret_cast<CharCP>(buff)) + 1) : 1);

    numBytes -= iOffset;
    buff += iOffset;

    for (int currentCode = 0; currentCode < (int)numBytes; ++currentCode)
        DecodeCodes(currentCode, buff);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
bool ShapeConverter::IsPointInGPA(DPoint2dCR point) const
    {
    int count = GetNumPoints();
    if (0 == count)
        return false;

    GraphicsPoint gp;
    m_gpa->GetGraphicsPoint(count - 1, gp);
    
    DPoint4d pt = gp.point;
    int mask = gp.mask;

    // If this point is a break then the point will need to be added again.
    if (0 != mask)
        return false;

    return ((pt.x == point.x) && (pt.y == point.y));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
bool ShapeConverter::NeedPointInGPA(DPoint2dCR point) const
    {
    int count = GetNumPoints();
    if (0 == count)
        return false;

    GraphicsPoint gp;
    m_gpa->GetGraphicsPoint(count - 1, gp);
    
    DPoint4d pt = gp.point;
    int mask = gp.mask;

    // If this point is anything other than a bland linestrignpoint, say no...
    if (0 != mask)
        return false;

    return ((pt.x != point.x) || (pt.y != point.y));
    }

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     05/2015
//=======================================================================================
struct DgnShxGlyph : DgnGlyph
{
private:
    T_Id m_glyphId;
    IDgnShxFontData* m_data;
    int64_t m_dataOffset;
    size_t m_dataSize;
    mutable bool m_areMetricsValid;
    mutable DRange2d m_range;
    mutable DRange2d m_exactRange;
    mutable bool m_isBlank;

    DgnShxGlyph() : m_glyphId(0), m_data(nullptr), m_dataOffset(0), m_dataSize(0), m_areMetricsValid(false), m_isBlank(true) {}
    void EnsureMetrics() const;

public:
    DgnShxGlyph(T_Id glyphId, IDgnShxFontData& data, int64_t dataOffset, size_t dataSize) : m_glyphId(glyphId), m_data(&data), m_dataOffset(dataOffset), m_dataSize(dataSize), m_areMetricsValid(false) {}
    static DgnShxGlyph* CreateBlankGlyph() { return new DgnShxGlyph(); }
    virtual T_Id _GetId() const override { return m_glyphId; }
    virtual DRange2d _GetRange() const override { EnsureMetrics(); return m_range; }
    virtual DRange2d _GetExactRange() const override { EnsureMetrics(); return m_exactRange; }
    virtual BentleyStatus _FillGpa(GPArrayR) const override;
    virtual bool _IsBlank() const override { EnsureMetrics(); return m_isBlank; }
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
void DgnShxGlyph::EnsureMetrics() const
    {
    if (m_areMetricsValid)
        return;

    m_areMetricsValid = true;

    memset(&m_range, 0, sizeof(m_range));
    memset(&m_exactRange, 0, sizeof(m_exactRange));
    m_isBlank = true;

    if ((nullptr == m_data) || (0 == m_dataSize))
        return;
    
    ScopedArray<Byte> glyphData(m_dataSize);
    if (SUCCESS != m_data->_Seek(m_dataOffset, BeFileSeekOrigin::Begin))
        return;

    if (0 == m_data->_Read(glyphData.GetData(), m_dataSize, 1))
        return;
    
    GPArraySmartP gpa;
    ShapeConverter converter(gpa, *m_data);
    converter.DoConversion(m_dataSize, glyphData.GetDataCP(), false, false);

    double scale = (1.0 / m_data->GetAscender());
    DRange2d tRange = DRange2d::NullRange();

    if (!gpa->IsEmpty())
        {
        // Scale the gpa into a 0-1 range
        Transform sMatrix;
        sMatrix.InitIdentity();

        bsiTransform_scaleMatrixRows(&sMatrix, &sMatrix, scale, scale, 1.0);
        gpa->Multiply(sMatrix);
        
        DRange3d range3d;
        gpa->GetRange(range3d);
        
        tRange.low.x = range3d.low.x;
        tRange.low.y = range3d.low.y;
        tRange.high.x = range3d.high.x;
        tRange.high.y = range3d.high.y;
        }

    if (!tRange.IsEmpty() && !tRange.IsNull())
        {
        m_exactRange = tRange;

        // For SHX files created from RSC files, we sometimes have to do a penDown/penUp to set the left and right side bearings to match what we would have had in the RSC glphy (e.g. in RSC glyphs LSB is always 0). In that case, there's no geometry in the GPA to set those extremes.
        double rsb = (converter.GetRightBearing() * scale);
        double lsb = (converter.GetLeftBearing() * scale);

        if (m_exactRange.high.x < rsb)
            m_exactRange.high.x = rsb;

        if (m_exactRange.low.x > lsb)
            m_exactRange.low.x = lsb;
        }

    m_range.low.Zero();
    m_range.high.x = (converter.GetCurrentPos().x * scale);
    m_range.high.y = m_exactRange.high.y;

    // If the pen position is to the left of zero, that means we have a right-to-left character. Reverse the start/end of the black box too.
    if (m_range.high.x < 0.0)
        {
        double d = m_exactRange.low.x;
        m_exactRange.low.x = m_exactRange.high.x;
        m_exactRange.high.x = d;
        }

    m_isBlank = tRange.IsNull();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnShxGlyph::_FillGpa(GPArrayR gpa) const
    {
    if ((nullptr == m_data) || (0 == m_dataSize))
        return ERROR;

    ScopedArray<Byte> glyphData(m_dataSize);
    if (SUCCESS != m_data->_Seek(m_dataOffset, BeFileSeekOrigin::Begin))
        return ERROR;

    if (0 == m_data->_Read(glyphData.GetData(), m_dataSize, 1))
        return ERROR;

    ShapeConverter converter(gpa, *m_data);
    converter.DoConversion(m_dataSize, glyphData.GetDataCP(), false, false);

    if (gpa.IsEmpty())
        return SUCCESS;
    
    double scale = (1.0 / m_data->GetAscender());
    Transform sMatrix;
    sMatrix.InitIdentity();
    bsiTransform_scaleMatrixRows(&sMatrix, &sMatrix, scale, scale, 1.0);
    
    gpa.Multiply(sMatrix);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
DgnGlyphCP DgnShxFont::FindGlyphCP(DgnGlyph::T_Id id) const
    {
    T_GlyphCache::const_iterator foundGlyph = m_glyphCache.find(id);
    if (m_glyphCache.end() != foundGlyph)
        return foundGlyph->second;

    GlyphFPos const* fpos = ((IDgnShxFontData*)m_data)->GetGlyphFPos(id);
    if (nullptr == fpos)
        return nullptr;
    
    DgnShxGlyph* glyph = new DgnShxGlyph(id, *(IDgnShxFontData*)m_data, fpos->m_dataOffset, fpos->m_dataSize);
    m_glyphCache[id] = glyph;
    
    return glyph;
    }

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     05/2015
//=======================================================================================
enum
{
    UNICODE_DEGREE = 0x00b0,
    UNICODE_DIAMETER = 0x2205,
    UNICODE_PLUSMINUS = 0x00b1
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
DgnGlyph::T_Id DgnShxFont::Ucs4CharToFontChar(uint32_t ucs4Char, CharCP codePageString, bvector<Byte>& localeBuffer) const
    {
    // Special Unicode mappings.
    DgnGlyph::T_Id specialFontChar = 0;
    switch (ucs4Char)
        {
        case UNICODE_DEGREE: specialFontChar = m_metadata.m_degreeCode; break;
        case UNICODE_DIAMETER: specialFontChar = m_metadata.m_diameterCode; break;
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
bvector<DgnGlyph::T_Id> DgnShxFont::Utf8ToFontChars(Utf8StringCR str) const
    {
    bvector<uint16_t> fontChars;

    // First, use UCS-4 to isolate each Unicode character.
    bvector<Byte> ucs4CharsBuffer;
    size_t numUcs4Chars;
    uint32_t const* ucs4Chars = DgnFont::Utf8ToUcs4(ucs4CharsBuffer, numUcs4Chars, str);
    if (0 == numUcs4Chars)
        return fontChars;

    // Then, for-each Unicode character, convert what the font expects.
    if (LangCodePage::Unicode == m_metadata.m_codePage)
        {
        // Assume UCS-2.
        fontChars.reserve(numUcs4Chars);
        for (size_t iUcs4Char = 0; iUcs4Char < numUcs4Chars; ++iUcs4Char)
            fontChars.push_back((DgnGlyph::T_Id)ucs4Chars[iUcs4Char]);
        
        return fontChars;
        }

    // Otherwise need to convert to locale.
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
BentleyStatus DgnShxFont::_LayoutGlyphs(DgnGlyphLayoutResultR result, DgnGlyphLayoutContextCR context) const
    {
    if (!IsResolved())
        return ERROR;

    // If you make any changes to this method, also consider examining DgnTrueTypeFont::_LayoutGlyphs and DgnRscFont::_LayoutGlpyhs.
    //  This method differs from the V8i variants in that it is designed to compute only the low-level information needed,
    //  and to serve both TextString and TextBlock through a single code path. This does mean that some extraneous information
    //  is potentially computed, but should be cheap compared to the overall layout operation.

    // SHX glyphs can encoded via UCS-2 or locale... in either case, need to process one character at a time.
    bvector<DgnGlyph::T_Id> fontChars = Utf8ToFontChars(context.m_string);

    // Sometimes need to reserve a special blank glyph for missing glyphs.
    unique_ptr<DgnShxGlyph> blankGlyph;

    // Acquire the glyphs.
    result.m_glyphs.reserve(fontChars.size());
    for (DgnGlyph::T_Id fontChar : fontChars)
        {
        DgnGlyph const* glyph = FindGlyphCP(fontChar);

        // Per SHX rules, if the glyph doesn't exist, substitute with the 0 glyph.
        if (nullptr == glyph)
            {
            if (!blankGlyph)
                blankGlyph.reset(DgnShxGlyph::CreateBlankGlyph());

            glyph = blankGlyph.get();
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

        // For SHX, at this point, glyphs should not be NULL; they should have been remapped to the blank glyph.
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
        }

    // Missing glyphs are allocated temporarily by this function...
    // I don't think it's worth the overhead to ref-count every glyph just to accommodate this bad data behavior.
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
double DgnShxFont::_GetDescenderRatio(DgnFontStyle) const
    {
    IDgnShxFontData* shxData = ((IDgnShxFontData*)m_data);
    return ((double)shxData->GetAscender() / (double)shxData->GetDescender());
    }
