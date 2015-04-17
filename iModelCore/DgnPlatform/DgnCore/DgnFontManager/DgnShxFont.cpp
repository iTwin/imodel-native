/*----------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnFontManager/DgnShxFont.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY_SQLITE


BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     07/2012
//=======================================================================================
struct IShxDataAccessor
    {
    public: virtual ~IShxDataAccessor () { }
    
    public: virtual void _Close () = 0;
    public: virtual BentleyStatus _Embed (DgnDbR, uint32_t fontNumber) = 0;
    public: virtual int _GetNextCharacter () = 0;
    public: virtual BentleyStatus _Open () = 0;
    public: virtual size_t _Read (void* buffer, size_t size, size_t count) = 0;
    public: virtual int _Seek (long offset, long origin) = 0;
    public: virtual size_t _Tell () = 0;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: uint16_t GetNextUInt16 ()
        {
        uint16_t nextUInt16;
        _Read (&nextUInt16, sizeof (nextUInt16), 1);
        return nextUInt16;
        }

    }; // IShxDataAccessor

END_BENTLEY_DGNPLATFORM_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
PropertySpec DgnShxFont::CreateEmbeddedFontPropertySpec ()
    {
    return PropertySpec ("shxdata", PROPERTY_APPNAME_DgnFont);
    }

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct FileShxDataAccessor : public IShxDataAccessor
    {
    private:    WString m_fileName;
    private:    FILE*   m_file;
    private:    size_t  m_openCount;
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: FileShxDataAccessor (WCharCP fileName) :
        m_file      (NULL),
        m_openCount (0)
        {
        if (!WString::IsNullOrEmpty(fileName))
            m_fileName = fileName;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual ~FileShxDataAccessor ()
        {
        if (m_openCount > 0)
            {
            BeAssert (false);
            m_openCount = 1;
            _Close ();
            }
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual void _Close () override
        {
        if (0 == m_openCount)
            { BeAssert (false); return; }
        
        --m_openCount;
        if (m_openCount > 0)
            return;

        if (NULL == m_file)
            return;
    
        fclose (m_file);
        m_file = NULL;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _Embed (DgnDbR project, uint32_t fontNumber) override
        {
        if (m_fileName.empty())
            return ERROR;
        
        BeFile fontFile;
        if (BeFileStatus::Success != fontFile.Open (m_fileName.c_str (), BeFileAccess::Read/*WIP_BeFile, BeFileSharing::Read*/))
            { BeAssert (false); return ERROR; }
    
        uint64_t fileSize = 0;
        if ((BeFileStatus::Success != fontFile.GetSize (fileSize)) || (0 == fileSize))
            { BeAssert (false); return ERROR; }
        
        ScopedArray<Byte> fileData ((size_t)fileSize);
        if (BeFileStatus::Success != fontFile.Read (fileData.GetData (), NULL, (uint32_t)fileSize))
            { BeAssert (false); return ERROR; }

        BeSQLite::DbResult res = project.SaveProperty(DgnShxFont::CreateEmbeddedFontPropertySpec(), fileData.GetData(), (uint32_t)fileSize, fontNumber);
    
        return ((BE_SQLITE_OK == res) ? SUCCESS : ERROR);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual int _GetNextCharacter () override
        {
        return fgetc (m_file);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _Open () override
        {
        if (m_fileName.empty())
            return ERROR;
        
        if (0 == m_openCount)
            {
            //WIP_BeFile m_file = BeFile::Fsopen (m_fileName.c_str (), L"rb", BeFileSharing::Read);
            m_file = fopen (Utf8String(m_fileName).c_str(), "rb");
            if (NULL == m_file)
                { BeAssert (false); return ERROR; }
            }
    
        ++m_openCount;

        return SUCCESS;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual size_t _Read (void* buffer, size_t size, size_t count) override
        {
        return fread (buffer, size, count, m_file);
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual int _Seek (long offset, long origin) override
        {
        return fseek (m_file, offset, origin);
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual size_t _Tell () override
        {
        return (size_t)ftell (m_file);
        }
    
    }; // FileShxDataAccessor

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct DbShxDataAccessor : public IShxDataAccessor
    {
    private:    uint32_t        m_fontNumber;
    private:    DgnDbCR    m_dgndb;
    private:    bvector<Byte>   m_buffer;
    private:    size_t          m_bufferCursor;
    private:    size_t          m_openCount;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: DbShxDataAccessor (uint32_t fontNumber, DgnDbCR project) :
        m_fontNumber    (fontNumber),
        m_dgndb       (project),
        m_bufferCursor  (0),
        m_openCount     (0)
        {
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual void _Close () override
        {
        if (0 == m_openCount)
            { BeAssert (false); return; }
        
        --m_openCount;
        if (m_openCount > 0)
            return;

        m_bufferCursor = 0;
        m_buffer.clear ();
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _Embed (DgnDbR project, uint32_t fontNumber) override
        {
        PropertySpec    shxDataPropSpec = DgnShxFont::CreateEmbeddedFontPropertySpec();
        uint32_t            shxDataPropSize;
        if (BE_SQLITE_ROW != m_dgndb.QueryPropertySize (shxDataPropSize, shxDataPropSpec, m_fontNumber))
            { BeAssert (false); return ERROR; }
    
        ScopedArray<Byte> shxDataPropValue (shxDataPropSize);
        if (BE_SQLITE_ROW != m_dgndb.QueryProperty (shxDataPropValue.GetData (), shxDataPropSize, shxDataPropSpec, m_fontNumber))
            { BeAssert (false); return ERROR; }
        
        if (BE_SQLITE_OK != project.SaveProperty (shxDataPropSpec, shxDataPropValue.GetData (), shxDataPropSize, fontNumber))
            { BeAssert (false); return ERROR; }
        
        return SUCCESS;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual int _GetNextCharacter () override
        {
        if (m_bufferCursor >= m_buffer.size ())
            return EOF;
        
        return (int)m_buffer[m_bufferCursor++];
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _Open () override
        {
        if (0 == m_openCount)
            {
            // Font data is loaded on-demand, which could be during an update when drawing an element that uses a font for the first time.
            // See TFS#78396, scheduled for Graphite06.
            HighPriorityOperationBlock highPriorityOperationBlock;
            
            PropertySpec    shxDataPropSpec = DgnShxFont::CreateEmbeddedFontPropertySpec();
            uint32_t            shxDataPropSize;
            if (BE_SQLITE_ROW != m_dgndb.QueryPropertySize (shxDataPropSize, shxDataPropSpec, m_fontNumber))
                { BeAssert (false); return ERROR; }
    
            m_buffer.resize (shxDataPropSize);
            if (BE_SQLITE_ROW != m_dgndb.QueryProperty (&m_buffer[0], shxDataPropSize, shxDataPropSpec, m_fontNumber))
                { BeAssert (false); return ERROR; }
        
            m_bufferCursor = 0;
            }

        ++m_openCount;

        return SUCCESS;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual size_t _Read (void* buffer, size_t size, size_t count) override
        {
        size_t  remainingBytes  = (m_buffer.size () - m_bufferCursor);
        size_t  requestedBytes  = (size * count);
        size_t  actuallyRead    = std::min (remainingBytes, requestedBytes);
        
        memcpy (buffer, &m_buffer[m_bufferCursor], actuallyRead);
        m_bufferCursor += actuallyRead;

        return actuallyRead;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual int _Seek (long offset, long origin) override
        {
        long newCursor;

        switch (origin)
            {
            case SEEK_CUR:  newCursor = ((long)(m_bufferCursor + offset));      break;
            case SEEK_END:  newCursor = ((long)(m_buffer.size () + offset));    break;
            case SEEK_SET:  newCursor = offset;                                 break;
            
            default:
                BeAssert (false && L"Unknown/unexpected SEEK.");
                return 1;
            }
        
        if ((newCursor < 0) || ((size_t)newCursor >= m_buffer.size ()))
            return 1;

        m_bufferCursor = (size_t)newCursor;
        
        return 0;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual size_t _Tell () override
        {
        return m_bufferCursor;
        }
    
    }; // DbShxDataAccessor

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
enum ShxShapCode
    {
    SHAPECODE_ENDOFSHAPE    = 0,
    SHAPECODE_PENDOWN       = 1,
    SHAPECODE_PENUP         = 2,
    SHAPECODE_DIVLENGTHS    = 3,
    SHAPECODE_MULTLENGTHS   = 4,
    SHAPECODE_PUSH          = 5,
    SHAPECODE_POP           = 6,
    SHAPECODE_SUBSHAPE      = 7,
    SHAPECODE_XYDISP        = 8,
    SHAPECODE_MULTIXYDISP   = 9,
    SHAPECODE_OCTANTARC     = 10,
    SHAPECODE_FRACTIONARC   = 11,
    SHAPECODE_BULGEARC      = 12,
    SHAPECODE_MULTIBULGEARC = 13,
    SHAPECODE_VERTICAL      = 14
    
    }; // ShxShapeCode

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
static void bulgeFactorToDEllipse3d (DEllipse3dP pEllipse, DPoint2dCP pStart, DPoint2dCP pEnd, double bulgeFactor)
    {
    double      sweep   = (4.0 * atan (bulgeFactor));
    DPoint2d    v;
    DPoint2d    u;
    DPoint2d    r;
    DPoint2d    x;
    DPoint2d    center;
    double      theta0;
    double      radius;

    // Find tan (included angle / 2)
    double tangentHalfAngle = tan (sweep * 0.5);

    // U = chord bvector
    bsiDPoint2d_subtractDPoint2dDPoint2d (&u, pEnd, pStart);
    bsiDPoint2d_scale (&u, &u, 0.5);

    // V = perpendicular to chord bvector, same length.
    v.x = -u.y;
    v.y = u.x;

    // Center = p0 + U + -V/tangentHalfAngle
    bsiDPoint2d_add2ScaledDPoint2d(&center, pStart, &u, 1.0, &v, 1.0/tangentHalfAngle);

    // Radius
    bsiDPoint2d_subtractDPoint2dDPoint2d (&r, pStart, &center);
    radius = bsiDPoint2d_magnitude (&r);

    // Start angle
    x.x = 1.0;
    x.y = 0.0;

    theta0 = bsiDPoint2d_angleBetweenVectors (&x, &r);

    // Setup for ellipse
    bsiDEllipse3d_init (pEllipse,   center.x,   center.y,   0.0,
                                    radius,     0.0,        0.0,
                                    0.0,        radius,     0.0,
                                    theta0,     sweep);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
static bool codeNameExists (ByteCP pBytes, size_t maxNameLength)
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
    private:    IShxDataAccessor*       m_data;
    private:    bool                    m_isPenDown;
    private:    bool                    m_vStart;
    private:    bool                    m_vEnd;
    private:    bool                    m_processVertical;
    private:    bool                    m_skipCodes;
    private:    bool                    m_hasData;
    private:    double                  m_multiplier;
    private:    double                  m_leftBearing;
    private:    double                  m_rightBearing;
    private:    DPoint2d                m_vertStartPt;
    private:    DPoint2d                m_vertEndPt;
    private:    DPoint2d                m_currentPos;
    private:    GPArrayP                m_gpa;
    private:    std::stack<DPoint2d>    m_stack;
    private:    DgnShxFontCP            m_font;
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public:     DPoint2dCR          GetCurrentPos   () const    { return m_currentPos; }
    private:    GraphicsPointArrayP GetGPA          ()          { return m_gpa; }
    public:     double              GetLeftBearing  () const    { return m_leftBearing; }
    public:     double              GetMultiplier   () const    { return m_multiplier; }
    public:     int                 GetNumPoints    () const    { return m_gpa->GetCount (); }
    public:     double              GetRightBearing () const    { return m_rightBearing; }
    public:     DPoint2dCR          GetVEndPt       () const    { return m_vertEndPt; }
    public:     DPoint2dCR          GetVStartPt     () const    { return m_vertStartPt; }
    public:     bool                HasVEnd         () const    { return m_vEnd; }
    public:     bool                HasVStart       () const    { return m_vStart; }
    public:     bool                IsPenDown       () const    { return m_isPenDown; }
        
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: ShapeConverter (DgnShxFontCR font, GPArrayR gpa, IShxDataAccessor& data) :
        m_data              (&data),
        m_isPenDown         (true),
        m_vStart            (false),
        m_vEnd              (false),
        m_hasData           (false),
        m_processVertical   (false),
        m_skipCodes         (false),
        m_multiplier        (1.0),
        m_leftBearing       (INT_MAX),
        m_rightBearing      (-INT_MAX),
        m_gpa               (&gpa),
        m_font              (&font)
        {
        m_vertStartPt.Zero ();
        m_vertEndPt.Zero ();
        m_currentPos.Zero ();
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    private: void AddPoint (DPoint2dCR pt)
        {
        m_hasData = true;
        if (!m_processVertical)
            m_gpa->Add (&pt, 1);
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    private: void AddEllipse (DEllipse3dCR el)
        {
        m_hasData = true;
        if (!m_processVertical)
            m_gpa->Add (el);
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: void DecodeBulgeArc (int& currentCode, ByteCP pCodes)
        {
        DPoint2d sDisp;
        sDisp.x = ((double)pCodes[++currentCode] * m_multiplier);
        sDisp.y = ((double)pCodes[++currentCode] * m_multiplier);
        
        signed char b = pCodes[++currentCode];

        if (m_skipCodes)
            return;

        DPoint2d pt     = m_currentPos;
        pt.x            += sDisp.x;
        pt.y            += sDisp.y;
        DPoint2d sStart = m_currentPos;
        DPoint2d sEnd   = pt;

        // *Note: We have empirically found that AutoCAD will always draw arcs, regardless of the pen up/down state.
        //  It does not, however, leave the pen down after arcs; therefore, simply ignore IsPenDown checks.
        //  Special case for bulge arcs: bulge arcs with a 0 bulge, which are effectively straight lines, obey the pen state.

        if (0 != b)
            {
            DEllipse3d sEllipse;
            bulgeFactorToDEllipse3d (&sEllipse, &sStart, &sEnd, (double) b / 127.0);

            if (NeedPointInGPA (m_currentPos))
                AddPoint (m_currentPos);

            AddEllipse (sEllipse);
            }
        else
            {
            if (IsPenDown () && !IsPointInGPA (m_currentPos))
                AddPoint (m_currentPos);

            if (IsPenDown ())
                AddPoint (pt);
            }

        m_currentPos = pt;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: void DecodeCodes (int& currentCode, ByteCP pCodes)
        {
        Byte iCode = pCodes[currentCode];
        if ((int)(iCode & 0xF0) > 0)
            {
            DecodeVector (iCode, pCodes);
            return;
            }

        switch (iCode)
            {
            case SHAPECODE_ENDOFSHAPE:      return;
            case SHAPECODE_PENDOWN:         DecodePenDown();                              break;
            case SHAPECODE_PENUP:           DecodePenUp();                                break;
            case SHAPECODE_DIVLENGTHS:      DecodeDivLengths (currentCode, pCodes);       break;
            case SHAPECODE_MULTLENGTHS:     DecodeMultLengths (currentCode, pCodes);      break;
            case SHAPECODE_PUSH:            DecodePush ();                                break;
            case SHAPECODE_POP:             DecodePop ();                                 break;
            case SHAPECODE_SUBSHAPE:        DecodeSubShape (currentCode, pCodes);         break;
            case SHAPECODE_XYDISP:          DecodeXYDisp (currentCode, pCodes);           break;
            case SHAPECODE_MULTIXYDISP:     DecodeXYMultiDisp (currentCode, pCodes);      break;
            case SHAPECODE_OCTANTARC:       DecodeOctantArc (currentCode, pCodes);        break;
            case SHAPECODE_FRACTIONARC:     DecodeFractionArc (currentCode, pCodes);      break;
            case SHAPECODE_BULGEARC:        DecodeBulgeArc (currentCode, pCodes);         break;
            case SHAPECODE_MULTIBULGEARC:   DecodeMultiBulgeArc (currentCode, pCodes);    break;
            case SHAPECODE_VERTICAL:        DecodeVertical (currentCode, pCodes);         break;

            default:
                BeAssert (false && L"Unknown/unexecpted opcode.");
            }
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: void DecodeDivLengths (int& currentCode, ByteCP pCodes)
        {
        Byte iCode = pCodes[++currentCode];
        BeAssert (0 != iCode);

        if (m_skipCodes)
            return;

        if (0 != iCode)
            m_multiplier /= (double)iCode;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: void DecodeExtenedFontSubShape (int& currentCode, ByteCP pCodes)
        {
        Byte msb     = pCodes[++currentCode];
        Byte lsb     = pCodes[++currentCode];
        uint16_t iCode   = ((msb << 8) + lsb);

        DgnShxGlyphFPos const* fPos = m_font->GetGlyphFilePos (iCode);
        if (NULL == fPos)
            return;

        Byte xOrigin = pCodes[++currentCode];
        Byte yOrigin = pCodes[++currentCode];
        Byte iWidth  = pCodes[++currentCode];
        Byte iHeight = pCodes[++currentCode];

        if (0 != m_data->_Seek (fPos->m_filePosition, 0))
            return;

        Byte* pBuf = reinterpret_cast<Byte*>(_alloca (sizeof (Byte) * fPos->m_dataSize));
        m_data->_Read (pBuf, sizeof (Byte) * fPos->m_dataSize, 1);

        GPArraySmartP   sGpa;
        ShapeConverter  sState (*m_font, sGpa, *m_data);

        sState.DoConversion (fPos->m_dataSize, pBuf, m_processVertical, false);

        double  width   = ((double)iWidth / m_font->GetCharWidth ());
        double  height  = ((double)iHeight / m_font->GetCharHeight ());

        Transform trans;
        bsiTransform_initFromRowValues (    &trans,
                                            width,  0,      0,  ((double)xOrigin + m_currentPos.x),
                                            0,      height, 0,  ((double)yOrigin + m_currentPos.y),
                                            0,      0,      1,  0);

        sState.m_gpa->Multiply (trans);
        m_gpa->AppendFrom (*sState.m_gpa);
    
        if (sState.m_hasData)
            m_hasData = true;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: void DecodeFractionArc (int& currentCode, ByteCP pCodes)
        {
        double          radius;
        double          sweep;
        double          startOffset;
        double          endOffset;
        double          x;
        double          y;
        double          startOffsetDeg;
        double          endOffsetDeg;
        char            iStartingOctant;
        char            iEndOctant;
        char            iSweep;
        char            iOctants;
        unsigned char   iStartOffset    = 0;
        unsigned char   iEndOffset      = 0;
        unsigned char   iHighRadius     = 0;
        unsigned char   iRadius         = 0;
        DPoint3d        center;
        DPoint3d        sEnd;
        DVec3d          s0Vector;
        DVec3d          s90Vector;
        DEllipse3d      sEllipse;

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
        iSweep          = (char)(osc & 0x07);
        iOctants        = iSweep;

        bool isClockwise = false;
        if (osc < 0)
            {
            isClockwise = true;
            iSweep      = -iSweep;
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
            startOffsetDeg  = (((iStartOffset * 45) / 256) + (iStartingOctant * 45));
            startOffset     = Angle::DegreesToRadians (startOffsetDeg);
            endOffsetDeg    = (((iEndOffset * 45.0) / 256.0) + ((0 == iEndOffset) ? iEndOctant : ((iStartingOctant + abs (iSweep) - 1)) * 45.0));
            endOffset       = Angle::DegreesToRadians (endOffsetDeg);
            }
        else
            {
            if (0 == iStartOffset)
                startOffsetDeg = iStartingOctant * 45.0;
            else
                startOffsetDeg = (iStartingOctant * 45) - iStartOffset * 45.0 / 256.0;

            startOffset = Angle::DegreesToRadians (startOffsetDeg);

            if (0 == iEndOffset)
                endOffsetDeg = (((0 == iEndOctant) ? 8 : iEndOctant) * 45);
            else
                endOffsetDeg = ((0 == iEndOctant) ? ((((iStartingOctant - abs (iSweep + 1))) * 45) - ((iEndOffset * 45) / 256)) : (((iEndOctant + 1) * 45) - ((iEndOffset * 45) / 256)));

            endOffset = Angle::DegreesToRadians (endOffsetDeg);
            }

        radius = ((double)(iRadius + (256 * iHighRadius)) * m_multiplier);

        if (!isClockwise)
            {
            if (endOffsetDeg <= startOffsetDeg)
                endOffsetDeg += 360;

            sweep = Angle::DegreesToRadians (endOffsetDeg - startOffsetDeg + 1);
            }
        else
            {
            // need a -ve angle
            if (startOffsetDeg <= 0)
                startOffsetDeg += 360;

            if (startOffsetDeg <= endOffsetDeg)
                sweep = Angle::DegreesToRadians (-(360 - (endOffsetDeg - startOffsetDeg)));
            else
                sweep = Angle::DegreesToRadians (endOffsetDeg - startOffsetDeg);
            }

        startOffset = Angle::DegreesToRadians (startOffsetDeg);

        x           = (radius * cos (startOffset));
        y           = (radius * sin (startOffset));
        center.x    = (m_currentPos.x - x);
        center.y    = (m_currentPos.y - y);
        center.z    = 0.0;
        endOffset   = Angle::DegreesToRadians (endOffsetDeg);
        
        x           = (radius * cos (endOffset));
        y           = (radius * sin (endOffset));
        sEnd        = center;
        sEnd.x      = (center.x + x);
        sEnd.y      = (center.y + y);

        s0Vector.x  = (m_currentPos.x - center.x);
        s0Vector.y  = (m_currentPos.y - center.y);
        s90Vector.x = -s0Vector.y;
        s90Vector.y = s0Vector.x;
        s90Vector.z = 0.0;
        s0Vector.z  = 0.0;

        bsiDEllipse3d_initFrom3dVectors (&sEllipse, &center, &s0Vector, &s90Vector, 0, sweep);

        // *Note: We have empirically found that AutoCAD will always draw arcs, regardless of the pen up/down state.
        //  It does not, however, leave the pen down after arcs; therefore, simply ignore IsPenDown checks.

        if (NeedPointInGPA (m_currentPos))
            AddPoint (m_currentPos);

        AddEllipse (sEllipse);

        m_currentPos.x  = sEnd.x;
        m_currentPos.y  = sEnd.y;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: void DecodeMultiBulgeArc (int& currentCode, ByteCP pCodes)
        {
        signed char    x   = pCodes[currentCode + 1];
        signed char    y   = pCodes[currentCode + 2];

        while ((0 != x) || (0 != y))
            {
            DecodeBulgeArc (currentCode, pCodes);

            // Get next 2 codes
            x   = pCodes[currentCode + 1];
            y   = pCodes[currentCode + 2];
            }
        
        // Move past the last 2 codes
        currentCode += 2;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: void DecodeMultLengths (int& currentCode, ByteCP pCodes)
        {
        Byte iCode = pCodes[++currentCode];

        if (m_skipCodes)
            return;

        BeAssert (0 != iCode);
        
        if (0 != iCode)
            m_multiplier *= (double)iCode;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: void DecodeOctantArc (int& currentCode, ByteCP pCodes)
        {
        signed char    b        = pCodes[++currentCode];
        double  radius          = ((double)b * m_multiplier);
        double  radiusBySin45   = (radius * sin (msGeomConst_piOver4));

        // Octant specifier
        b = pCodes[++currentCode];

        if (m_skipCodes)
            return;

        char    iStartingOctant = (char)((b & 0x70) >> 4);
        char    iSweep          = (char)(b & 0x07);
        char    iOctants        = iSweep;
        bool    isClockwise     = false;

        if (b < 0)
            {
            isClockwise = true;
            iSweep      = -iSweep;
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
        center.Init (m_currentPos.x, m_currentPos.y, 0);

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

        double  sweep       = ((0.0 == iSweep) ? msGeomConst_2pi : Angle::DegreesToRadians (iSweep * 45.0));
        DVec3d  s0Vector;
        DVec3d  s90Vector;
        
        s0Vector.x  = (m_currentPos.x - center.x);
        s0Vector.y  = (m_currentPos.y - center.y);
        s90Vector.x = -s0Vector.y;
        s90Vector.y = s0Vector.x;
        s90Vector.z = 0.0;
        s0Vector.z  = 0.0;

        DEllipse3d sEllipse;
        bsiDEllipse3d_initFrom3dVectors (&sEllipse, &center, &s0Vector, &s90Vector, 0, sweep);

        // *Note: We have empirically found that AutoCAD will always draw arcs, regardless of the pen up/down state.
        //  It does not, however, leave the pen down after arcs; therefore, simply ignore IsPenDown checks.

        if (NeedPointInGPA (m_currentPos))
            AddPoint (m_currentPos);

        AddEllipse (sEllipse);

        m_currentPos.x  = sEnd.x;
        m_currentPos.y  = sEnd.y;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: void DecodePenDown ()
        {
        if (m_skipCodes)
            return;

        m_isPenDown = true;

        if (m_currentPos.x < m_leftBearing)
            m_leftBearing = m_currentPos.x;

        if (m_currentPos.x >  m_rightBearing)
            m_rightBearing = m_currentPos.x;

        m_gpa->MarkMajorBreak ();
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: void DecodePenUp ()
        {
        if (m_skipCodes)
            return;

        m_isPenDown = false;
        m_gpa->MarkMajorBreak ();
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: void DecodePop ()
        {
        if (m_skipCodes)
            return;

        if (m_stack.empty ())
            return;

        m_currentPos = m_stack.top ();
        m_stack.pop ();

        // Mark break in GPA added due to BOLD.shx R character
        m_gpa->MarkMajorBreak ();
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: void DecodePush ()
        {
        if (!m_skipCodes)
            m_stack.push (m_currentPos);
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: void DecodeSubShape (int& currentCode, ByteCP pCodes)
        {
        uint16_t subshapeCode = pCodes[++currentCode];
        if (LangCodePage::Unicode == m_font->GetCodePage ())
            subshapeCode = ((subshapeCode << 8) + pCodes[++currentCode]);

       if (0 == subshapeCode)
            {
            DecodeExtenedFontSubShape (currentCode, pCodes);
            return;
            }

        DgnShxGlyphFPos const* fPos = m_font->GetGlyphFilePos (subshapeCode);
        if (NULL == fPos)
            return;

        if (0 != m_data->_Seek (fPos->m_filePosition, 0))
            return;

        Byte* pBuf = reinterpret_cast<Byte*>(_alloca (sizeof (Byte) * fPos->m_dataSize));
        m_data->_Read (pBuf, sizeof (Byte) * fPos->m_dataSize, 1);

        DoConversion (fPos->m_dataSize, pBuf, m_processVertical, m_hasData);
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: void DecodeVector (Byte iCode, ByteCP)
        {
        if (m_skipCodes)
            return;

        int     iLength     = ((iCode & 0xF0) >> 4);
        int     iDirection  = (iCode & 0x0F);
        double  dLength     = (iLength * m_multiplier);

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
                BeAssert (false);
                break;
            }

        if (IsPenDown ())
            {
            if (!IsPointInGPA (m_currentPos))
                AddPoint (m_currentPos);

            AddPoint (pt);
            }

        m_currentPos = pt;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: void DecodeVertical (int& currentCode, ByteCP pCodes)
        {
        // Move on to the next code
        ++currentCode;

        if (!m_processVertical)
            m_skipCodes = true;

        DPoint2d savePt = m_currentPos;

        // Decode the code after the this verical statement
        DecodeCodes (currentCode, pCodes);

        m_skipCodes = false;

        if (!m_processVertical)
            return;

        // We only pay attention to the delta movement caused by the vertical codes.
        DPoint2d delta;
        delta.DifferenceOf (m_currentPos, savePt);

        if (m_hasData)
            {
            m_vEnd = true;
            m_vertEndPt.Add (delta);
            }
        else
            {
            m_vStart = true;
            m_vertStartPt.Add (delta);
            }

        (const_cast<DgnShxFontP>(m_font))->SetIsVertical (true);
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: void DecodeXYDisp (int& currentCode, ByteCP pCodes)
        {
        DPoint2d    sDisp;
        DPoint2d    pt;
        signed char        b       = 0;

        memset (&sDisp, 0, sizeof (DPoint2d));
        b = pCodes[++currentCode];

        sDisp.x = ((double)b * m_multiplier);
        b = pCodes[++currentCode];

        sDisp.y = ((double)b * m_multiplier);
        
        if (m_skipCodes)
            return;

        pt      = m_currentPos;
        pt.x    += sDisp.x;
        pt.y    += sDisp.y;

        if (IsPenDown () && !IsPointInGPA (m_currentPos))
            AddPoint (m_currentPos);

        if (IsPenDown ())
            AddPoint (pt);

        m_currentPos = pt;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: void DecodeXYMultiDisp (int& currentCode, ByteCP pCodes)
        {
        signed char    x   = pCodes[currentCode + 1];
        signed char    y   = pCodes[currentCode + 2];

        while ((0 != x) || (0 != y))
            {
            DecodeXYDisp (currentCode, pCodes);

            // Get next 2 codes.
            x   = pCodes[currentCode + 1];
            y   = pCodes[currentCode + 2];
            }

        // Move past the last 2 codes.
        currentCode += 2;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: void DoConversion (size_t numBytes, ByteCP buff, bool doVerticals, bool hasData)
        {
        m_isPenDown         = true;
        m_hasData           = hasData;
        m_processVertical   = doVerticals;

        // If there is a code name for the glyph then skip it
        size_t iOffset = (codeNameExists ((unsigned char*)buff, numBytes) ? (strlen (reinterpret_cast<CharCP>(buff)) + 1) : 1);

        numBytes    -= iOffset;
        buff        += iOffset;

        for (int currentCode = 0; currentCode < (int)numBytes; ++currentCode)
            DecodeCodes (currentCode, buff);
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: bool IsPointInGPA (DPoint2dCR point) const
        {
        int count = GetNumPoints ();
        if (0 == count)
            return false;

        GraphicsPoint gp;
        m_gpa->GetGraphicsPoint (count - 1, gp);
        
        DPoint4d    pt      = gp.point;
        int         mask    = gp.mask;

        // If this point is a break then the point will need to be added again.
        if (0 != mask)
            return false;

        return ((pt.x == point.x) && (pt.y == point.y));
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: bool NeedPointInGPA (DPoint2dCR point) const
        {
        int count = GetNumPoints ();
        if (0 == count)
            return false;

        GraphicsPoint gp;
        m_gpa->GetGraphicsPoint (count - 1, gp);
        
        DPoint4d    pt      = gp.point;
        int         mask    = gp.mask;

        // If this point is anything other than a bland linestrignpoint, say no...
        if (0 != mask)
            return false;

        return ((pt.x != point.x) || (pt.y != point.y));
        }

    }; // ShapeConverter

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
BentleyStatus   DgnShxGlyph::_FillGpa                   (GPArrayR gpa) const    { if (!m_isBlank && m_gpa->IsEmpty ()) { return ERROR; } gpa.CopyContentsOf (m_gpa); return SUCCESS; }
DgnFontType     DgnShxGlyph::_GetType                   () const                { return DgnFontType::Shx; }
double          DgnShxGlyph::GetVerticalCellBoxBottom   () const                { return m_verticalCellBoxStart.y; }
double          DgnShxGlyph::GetVerticalCellBoxLeft     () const                { return m_verticalCellBoxStart.x; }
double          DgnShxGlyph::GetVerticalCellBoxTop      () const                { return m_verticalCellBoxEnd.y; }
bool            DgnShxGlyph::_IsBlank                   () const                { return m_isBlank; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnShxGlyph::DgnShxGlyph (FontChar charCode, size_t filePos, size_t dataSize, DgnShxFontCR font, IShxDataAccessor& data) :
    T_Super (charCode),
    m_data      (&data),
    m_isBlank   (true)
    {
    m_verticalCellBoxStart.Zero ();
    m_verticalCellBoxEnd.Zero ();
    
    if (0 == dataSize)
        return;

    Byte* pBuf = reinterpret_cast<Byte*>(_alloca (dataSize));
    
    m_data->_Seek ((long)filePos, SEEK_SET);
    m_data->_Read (pBuf, sizeof (Byte) * dataSize, 1);

    double scale = (1.0 / font.GetAscender ());

    // Convert skipping vertical movements
    ShapeConverter hConverter (font, m_gpa, data);
    hConverter.DoConversion (dataSize, pBuf, false, false);

    // Convert w vertical movements; no GPA changes
    ShapeConverter vConverter (font, m_gpa, data);
    vConverter.DoConversion (dataSize, pBuf, true, false);

    DRange3d tRange;
    tRange.Init ();

    if (!m_gpa->IsEmpty ())
        {
        // Scale the gpa into a 0-1 range
        Transform sMatrix;
        sMatrix.InitIdentity ();

        bsiTransform_scaleMatrixRows (&sMatrix, &sMatrix, scale, scale, 1.0);
        m_gpa->Multiply (sMatrix);
        m_gpa->GetRange (tRange);
        }

    if (!tRange.IsEmpty () && !tRange.IsNull ())
        {
        m_blackBoxStart.x   = tRange.low.x;
        m_blackBoxStart.y   = tRange.low.y;
        m_blackBoxEnd.x     = tRange.high.x;
        m_blackBoxEnd.y     = tRange.high.y;

        // For shx files created from RSC files, we sometimes have to do a penDown/penUp to set the left and right side bearings to match what we would have had in the RSC glphy (e.g. in RSC glyphs LSB is always 0). In that case, there's no geometry in the GPA to set those extremes.
        double  rsb = (hConverter.GetRightBearing () * scale);
        double  lsb = (hConverter.GetLeftBearing () * scale);
        
        if (m_blackBoxEnd.x < rsb)
            m_blackBoxEnd.x = rsb;

        if (m_blackBoxStart.x > lsb)
            m_blackBoxStart.x = lsb;
        }

    m_cellBoxStart.x    = 0.0;
    m_cellBoxStart.y    = 0.0;
    m_cellBoxEnd.x      = (hConverter.GetCurrentPos ().x * scale);
    m_cellBoxEnd.y      = m_blackBoxEnd.y;

    if (vConverter.HasVStart ())
        m_verticalCellBoxStart.Scale (vConverter.GetVStartPt (), scale);

    if (vConverter.HasVEnd ())
        m_verticalCellBoxEnd.Scale (vConverter.GetVEndPt (), scale);
    else
        m_verticalCellBoxEnd = m_cellBoxEnd;

    // If the pen position is to the left of zero, that means we have a right-to-left character. Reverse the start/end of the black box too.
    if (m_cellBoxEnd.x < 0.0)
        {
        double d            = m_blackBoxStart.x;
        m_blackBoxStart.x   = m_blackBoxEnd.x;
        m_blackBoxEnd.x     = d;
        }

    m_isBlank = tRange.IsNull ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
enum ShxFileType
    {
    SHXFILETYPE_Invalid = -1,
    SHXFILETYPE_Unifont = 0,
    SHXFILETYPE_Bigfont = 1,
    SHXFILETYPE_Shape10 = 2,
    SHXFILETYPE_Shape11 = 3,
    SHXFILETYPE_Symbol  = 4,
    
    }; // ShxFileType

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct ShxFileMetadata
    {
    public: ShxFileType m_shxFileType;
    public: uint64_t    m_timeStamp;

    }; // ShxFileMetadata

typedef bmap<WString, ShxFileMetadata>    T_ShxFileMetadatas;
typedef bvector<WString>                  T_WStrings;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
static T_ShxFileMetadatas acquireCachedShxFileMetaData (WCharCP cacheFilePath)
    {
    T_ShxFileMetadatas cachedMetadata;

// *** WIP_ForeignFormat - We should move this code into foreignformat as part of the implementation of an IDgnFontFinder. See DgnFontFinder for an example.
    
    FILE* cacheFile = fopen (Utf8String(cacheFilePath).c_str(), "r"); // *** WIP_BeFile  CSS_UTF8 - make this file Windows-specific and use _fsopen if CSS_UTF8 support is required
    if (NULL == cacheFile)
        return cachedMetadata;

    static const uint32_t MAX_SHXFILEDATARECORDSIZE   = (MAX_PATH * 2);
    static const WChar  ENDOFNAME                   = L'"';
    
    WChar buffer[MAX_SHXFILEDATARECORDSIZE];
    while (fgetws (buffer, _countof (buffer), cacheFile))
        {
        ShxFileMetadata cacheEntry;
        WString         filePath;
        
        WCharCP src = (buffer + 1);
        for (; (NULL != src) && (*src != ENDOFNAME); ++src)
            filePath += *src;

        if (NULL == src)
            { BeDataAssert (false); continue; }
        
        int isValid;
        
        if ((3 != BE_STRING_UTILITIES_SWSCANF ((src + 2), L"%d,%lld,%d", &cacheEntry.m_shxFileType, &cacheEntry.m_timeStamp, &isValid)) || !isValid)
            continue;
        
        cachedMetadata[filePath] = cacheEntry;
        }

    fclose (cacheFile);

    return cachedMetadata;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
static ShxFileType determineRawShxFileType (IShxDataAccessor& data)
    {
    char    buf[64];
    int     index       = 0;
    int     nextInt     = 0;

    if (SUCCESS != data._Open ())
        { BeAssert (false); return SHXFILETYPE_Invalid; }

    do
        {
        nextInt     = data._GetNextCharacter ();
        buf[index]  = (char)nextInt;
        
        } while ((0x1a != nextInt) && (index++ < 40));

    data._Close ();

    if (NULL != strstr (buf, "AutoCAD-86 unifont 1.0")) return SHXFILETYPE_Unifont;
    if (NULL != strstr (buf, "AutoCAD-86 bigfont 1.0")) return SHXFILETYPE_Bigfont;
    if (NULL != strstr (buf, "AutoCAD-86 shapes 1.0"))  return SHXFILETYPE_Shape10;
    if (NULL != strstr (buf, "AutoCAD-86 shapes 1.1"))  return SHXFILETYPE_Shape11;

    return SHXFILETYPE_Invalid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
static ShxFileType determineShxFileType (WCharCP fileName)
    {
    FileShxDataAccessor reader (fileName);
    if (SUCCESS != reader._Open ())
        return SHXFILETYPE_Invalid;

    ShxFileType fileType = determineRawShxFileType (reader);
    if ((SHXFILETYPE_Shape10 == fileType) || (SHXFILETYPE_Shape11 == fileType))
        {
        // For font files, the first glyph is 0; otherwise it is a symbol file.
        uint16_t firstGlyph;
        reader._Read (&firstGlyph, sizeof (firstGlyph), 1);
        
        if (0 != firstGlyph)
            fileType = SHXFILETYPE_Symbol;
        }

    reader._Close ();
    
    return fileType;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
static bool doesGlyphHaveName (ByteCP pBuf, size_t dataSize)
    {
    if (0 == *pBuf)
        return false;

    for (size_t iByte = 0; iByte < dataSize; ++iByte)
        {
        if (0 == *pBuf++)
            return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
static WString getCachedShxFileMetaDataFilePath ()
    {
    WString fileDirW;
#if defined (WIP_CFGVAR) // MS_DWGDATA
    if (SUCCESS != ConfigurationManager::GetVariable (fileDirW, L"MS_DWGDATA"))
#endif
        {
        BeFileName fileDir;
        if (SUCCESS != T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectory (fileDir, L"dwgdata"))
            return L"";
        
        fileDirW = fileDir.GetName ();
        }

    BeFileName::CreateNewDirectory (fileDirW.c_str ());
    
    WString dev, dir;
    BeFileName::ParseName (&dev, &dir, NULL, NULL, fileDirW.c_str ());
    
    WString cacheFilePath;
    BeFileName::BuildName (cacheFilePath, dev.c_str (), dir.c_str (), L"ShxFontList-UTF8", L"csv");

    return cacheFilePath;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2013
//---------------------------------------------------------------------------------------
static LangCodePage getDefaultCodePage(ShxFileType fileType)
    {
    switch (fileType)
        {
        case SHXFILETYPE_Unifont:
            return LangCodePage::Unicode;
        
        default:
            BeAssert (false && L"Unknown/unexpected ShxFileType.");
            // Fall through
        
        case SHXFILETYPE_Shape11:
        case SHXFILETYPE_Shape10:
        case SHXFILETYPE_Symbol:
            {
            LangCodePage acp;
            BeStringUtilities::GetCurrentCodePage (acp);
            
            return acp;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2013
//---------------------------------------------------------------------------------------
static FontChar getDefaultDegreeCharCode(ShxFileType fileType)
    {
    switch (fileType)
        {
        case SHXFILETYPE_Shape11:
            return 256;
        
        case SHXFILETYPE_Unifont:
            return SPECIALCHAR_UnicodeDegree;
        
        default:
            BeAssert (false && L"Unknown/unexpected ShxFileType.");
            // Fall through

        case SHXFILETYPE_Shape10:
        case SHXFILETYPE_Symbol:
            return 127;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2013
//---------------------------------------------------------------------------------------
static FontChar getDefaultDiameterCharCode(ShxFileType fileType)
    {
    switch (fileType)
        {
        case SHXFILETYPE_Shape11:
            return 258;
        
        case SHXFILETYPE_Unifont:
            return SPECIALCHAR_UnicodeDiameter;
        
        default:
            BeAssert (false && L"Unknown/unexpected ShxFileType.");
            // Fall through

        case SHXFILETYPE_Shape10:
        case SHXFILETYPE_Symbol:
            return 129;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2013
//---------------------------------------------------------------------------------------
static FontChar getDefaultPlusMinusCharCode(ShxFileType fileType)
    {
    switch (fileType)
        {
        case SHXFILETYPE_Shape11:
            return 257;
        
        case SHXFILETYPE_Unifont:
            return SPECIALCHAR_UnicodePlusMinus;
        
        default:
            BeAssert (false && L"Unknown/unexpected ShxFileType.");
            // Fall through

        case SHXFILETYPE_Shape10:
        case SHXFILETYPE_Symbol:
            return 128;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
static DgnFontConfigurationData getEffectiveFontConfig (DgnFontKey const& fontKey, ShxFileType fileType)
    {
    DgnFontConfigurationData const* customConfig = DgnFontManager::FindCustomFontConfiguration (fontKey);
    if (NULL != customConfig)
        {
        DgnFontConfigurationData effectiveCustomConfig = *customConfig;
        
        // We do not know when reading the configuration what type of SHX font it is, so setting defaults has to get done here.
        if (LangCodePage::None == effectiveCustomConfig.m_codePage)
            effectiveCustomConfig.m_codePage = getDefaultCodePage(fileType);

        if (0 == effectiveCustomConfig.m_degreeCharCode)
            effectiveCustomConfig.m_degreeCharCode = getDefaultDegreeCharCode(fileType);

        if (0 == effectiveCustomConfig.m_diameterCharCode)
            effectiveCustomConfig.m_diameterCharCode = getDefaultDiameterCharCode(fileType);
        
        if (0 == effectiveCustomConfig.m_plusMinusCharCode)
            effectiveCustomConfig.m_plusMinusCharCode = getDefaultPlusMinusCharCode(fileType);

        return effectiveCustomConfig;
        }
    
    DgnFontConfigurationData defaultConfig;
    defaultConfig.m_codePage                = getDefaultCodePage(fileType);
    defaultConfig.m_shouldCreateShxUnifont  = true;
    defaultConfig.m_degreeCharCode          = getDefaultDegreeCharCode(fileType);
    defaultConfig.m_diameterCharCode        = getDefaultDiameterCharCode(fileType);
    defaultConfig.m_plusMinusCharCode       = getDefaultPlusMinusCharCode(fileType);
    
    return defaultConfig;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
static T_WStrings getSystemShxFilePathList (WString searchPaths)
    {
    T_WStrings filePaths;
    
    for (size_t pathSubStrStart = 0, pathSubStrEnd = 0; WString::npos != pathSubStrEnd; pathSubStrStart = pathSubStrEnd)
        {
        pathSubStrStart = searchPaths.find_first_not_of (PATH_SEPARATOR_CHAR, pathSubStrStart);
        pathSubStrEnd   = searchPaths.find (PATH_SEPARATOR_CHAR, pathSubStrStart);

        if (pathSubStrStart == pathSubStrEnd)
            break;

        // BeFileName::IsDirectory does not detect a directory if the name ends in a trailing slash.
        WString currPathW = searchPaths.substr (pathSubStrStart, (pathSubStrEnd - pathSubStrStart));
        if (WCSDIR_SEPARATOR_CHAR == currPathW[currPathW.size () - 1])
            currPathW = currPathW.substr (0, currPathW.size () - 1);
        
        BeFileName  currPath        (currPathW.c_str ());
        WString     fileName;
        WString     fileExtension;
        
        currPath.ParseName (NULL, NULL, &fileName, &fileExtension);
        
        if (BeFileName::IsDirectory (currPath.GetName ()))
            currPath.BuildName (NULL, currPath.GetName (), L"*", L"shx");

        BeFileListIterator  iterator (currPath.GetName (), false);
        BeFileName          currFile;
        
        while (SUCCESS == iterator.GetNextFileName (currFile))
            {
            if (BeFileName::IsDirectory (currFile.GetName ()))
                continue;
            
            filePaths.push_back (currFile.GetName ());
            }
        }
    
    return filePaths;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
static void readNonShxBigFontInfo (IShxDataAccessor& data, DgnShxGlyphFPos const* zeroGlyphFPos, Byte& ascender, Byte& descender, Byte& charHeight, Byte& charWidth)
    {
    // Char 0 is the font specifier.
    if (NULL == zeroGlyphFPos)
        {
        ascender    = 1;
        descender   = 1;
        return;
        }

    if (0 != data._Seek (zeroGlyphFPos->m_filePosition, 0))
        return;

    size_t              pBufSize    = (size_t)zeroGlyphFPos->m_dataSize;
    ScopedArray<Byte>   pBuf        (pBufSize);

    // Read code 0 which should be the font specifier.
    data._Read (pBuf.GetData (), pBufSize, 1);

    // Look for a NULL terminator.
    size_t iLen = 0;
    for (; ((iLen < pBufSize) && (0 != pBuf.GetData ()[iLen])); ++iLen)
        ;
    
    // No Terminator found?
    if (iLen == pBufSize)
        return;

    // Pre-historic legacy...
    if (((pBufSize - iLen) != 5))
        return;

    size_t  iOffset = (iLen + 1);
    Byte    iAbove  = pBuf.GetData ()[iOffset++];
    Byte    iBelow  = pBuf.GetData ()[iOffset++];
    
    // Skip "modes"
    ++iOffset;
    
    ascender    = iAbove;
    descender   = iBelow;

    // This hack comes from the OpenDWG code. I don't understand it but see TR#182253 for font with this problem (gbcbig.shx).
    if (0 == ascender)
        ascender = 8;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
static void writeCachedShxFileMetaData (WCharCP cacheFilePath, T_ShxFileMetadatas const& cachedMetadata)
    {
    FILE* cacheFile = fopen (Utf8String(cacheFilePath).c_str(), "w+"); // *** WIP_BeFile CSS_UTF8 - make this file Windows-specific and use _fsopen if CSS_UTF8 support is required
    if (NULL == cacheFile)
        {
        BeAssert (false);
        NativeLogging::LoggingManager::GetLogger (L"DgnPlatform")->errorv (L"%ls - Could not open the SHX font info cache file for write.", cacheFilePath);
        return; 
        }

    for (T_ShxFileMetadatas::const_iterator cacheEntryIter = cachedMetadata.begin (); cachedMetadata.end () != cacheEntryIter; ++cacheEntryIter)
        fwprintf (cacheFile, L"\"%ls\",%d,%lld,%d\n", cacheEntryIter->first.c_str (), cacheEntryIter->second.m_shxFileType, cacheEntryIter->second.m_timeStamp, 1);

    fclose (cacheFile);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
bool            DgnShxFont::_ContainsCharacter          (WChar uniChar) const   { return (m_glyphFPosMap.end () != m_glyphFPosMap.find (RemapUnicodeCharToFontChar (uniChar))); }
double          DgnShxFont::GetAscender                 () const                { if (m_isMissing) { return 0.0; } _EnsureFontIsLoaded (); return m_ascender; }
Byte            DgnShxFont::GetCharHeight               () const                { if (m_isMissing) { return 0; } _EnsureFontIsLoaded (); return m_charHeight; }
Byte            DgnShxFont::GetCharWidth                () const                { if (m_isMissing) { return 0; } _EnsureFontIsLoaded (); return m_charWidth; }
LangCodePage    DgnShxFont::_GetCodePage                () const                { return m_codePage; }
FontChar        DgnShxFont::_GetDegreeCharCode          () const                { return m_degreeCharCode; }
double          DgnShxFont::_GetDescenderRatio          () const                { if (m_isMissing) { return 0.0; } _EnsureFontIsLoaded (); return (fabs ((double)m_descender) / m_ascender); }
FontChar        DgnShxFont::_GetDiameterCharCode        () const                { return m_diameterCharCode; }
FontChar        DgnShxFont::_GetPlusMinusCharCode       () const                { return m_plusMinusCharCode; }
DgnFontType     DgnShxFont::_GetType                    () const                { return DgnFontType::Shx; }
bool            DgnShxFont::IsVertical                  () const                { if (m_isMissing) { return false; } _EnsureFontIsLoaded (); return m_isVertical; }
void            DgnShxFont::SetIsVertical               (bool value)            { m_isVertical = value; }
bool            DgnShxFont::_CanDrawWithLineWeight   () const                { return true; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnShxFont::DgnShxFont (Utf8CP name, DgnFontConfigurationData const& config, WCharCP filePath) :
    T_Super (name, config),
    m_data              (new FileShxDataAccessor (filePath)),
    m_codePage          (config.m_codePage),
    m_degreeCharCode    (config.m_degreeCharCode),
    m_diameterCharCode  (config.m_diameterCharCode),
    m_plusMinusCharCode (config.m_plusMinusCharCode),
    m_ascender          (0),
    m_descender         (0),
    m_charHeight        (0),
    m_charWidth         (0),
    m_isVertical        (false)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnShxFont::DgnShxFont (Utf8CP name, DgnFontConfigurationData const& config, uint32_t fontNumber, DgnDbCR project) :
    T_Super (name, config),
    m_data              (new DbShxDataAccessor (fontNumber, project)),
    m_codePage          (config.m_codePage),
    m_ascender          (0),
    m_descender         (0),
    m_charHeight        (0),
    m_charWidth         (0),
    m_isVertical        (false),
    m_degreeCharCode    (config.m_degreeCharCode),
    m_diameterCharCode  (config.m_diameterCharCode),
    m_plusMinusCharCode (config.m_plusMinusCharCode)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnShxFont::~DgnShxFont ()
    {
    for (T_GlyphMap::const_iterator glyphIter = m_glyphMap.begin (); m_glyphMap.end () != glyphIter; ++glyphIter)
        delete glyphIter->second;
    
    delete m_data;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
void DgnShxFont::AcquireSystemFonts (T_FontCatalogMap& systemFonts)
    {
    WString shxFontPaths = T_HOST.GetFontAdmin ()._GetShxFontPaths ();
    if (shxFontPaths.empty ())
        return;

    T_ShxFileMetadatas  cachedMetadata;
    WString             metaDataCacheFilePath   = getCachedShxFileMetaDataFilePath ();
    
    if (!metaDataCacheFilePath.empty ())
        cachedMetadata = acquireCachedShxFileMetaData (metaDataCacheFilePath.c_str ());
    
    size_t      numOriginalCachedMetadataEntries    = cachedMetadata.size ();
    T_WStrings  systemShxFilePaths                  = getSystemShxFilePathList (shxFontPaths);
    
    for (T_WStrings::const_iterator filePathIter = systemShxFilePaths.begin (); systemShxFilePaths.end () != filePathIter; ++filePathIter)
        {
        WString fontNameW;
        BeFileName::ParseName (NULL, NULL, &fontNameW, NULL, filePathIter->c_str ());
        
        Utf8String fontName (fontNameW.c_str ());
        DgnFontKey fontKey (DgnFontType::Shx, fontName);

        // First one in by-name wins.
        if (systemFonts.end () != systemFonts.find (fontKey))
            continue;

        time_t fileLastWriteTime;
        BeFileName::GetFileTime (NULL, NULL, &fileLastWriteTime, filePathIter->c_str ());

        T_ShxFileMetadatas::const_iterator cachedEntry = cachedMetadata.find (*filePathIter);
        if ((cachedMetadata.end () == cachedEntry) || (cachedEntry->second.m_timeStamp != fileLastWriteTime))
            {
            ShxFileMetadata metadata;
            metadata.m_shxFileType  = determineShxFileType (filePathIter->c_str ());
            metadata.m_timeStamp    = fileLastWriteTime;
            
            cachedMetadata[*filePathIter]   = metadata;
            cachedEntry                     = cachedMetadata.find (*filePathIter);
            }
        
        switch (cachedEntry->second.m_shxFileType)
            {
            case SHXFILETYPE_Shape10:
            case SHXFILETYPE_Shape11:
            case SHXFILETYPE_Symbol:    systemFonts[fontKey] = new DgnShxShapeFont (fontName.c_str (), getEffectiveFontConfig (fontKey, cachedEntry->second.m_shxFileType), filePathIter->c_str ());    break;
            case SHXFILETYPE_Bigfont:   break;
            case SHXFILETYPE_Unifont:   systemFonts[fontKey] = new DgnShxUniFont (fontName.c_str (), getEffectiveFontConfig (fontKey, cachedEntry->second.m_shxFileType), filePathIter->c_str ());      break;
            
            default:
                BeAssert (false && L"Unkknown/unexpected ShxFileType.");
                continue;
            }
        }

    if ((cachedMetadata.size () != numOriginalCachedMetadataEntries) && !metaDataCacheFilePath.empty ())
        writeCachedShxFileMetaData (metaDataCacheFilePath.c_str (), cachedMetadata);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnFontPtr DgnShxFont::CreateFromEmbeddedFont (Utf8CP name, uint32_t fontNumber, DgnDbCR project)
    {
    DbShxDataAccessor   dbData      (fontNumber, project);
    ShxFileType         shxFileType = determineRawShxFileType (dbData);

    switch (shxFileType)
        {
        case SHXFILETYPE_Shape10:
        case SHXFILETYPE_Shape11:
        case SHXFILETYPE_Symbol:    return new DgnShxShapeFont (name, getEffectiveFontConfig (DgnFontKey (DgnFontType::Shx, Utf8String (name)), shxFileType), fontNumber, project);
        case SHXFILETYPE_Bigfont:   return NULL;
        case SHXFILETYPE_Unifont:   return new DgnShxUniFont (name, getEffectiveFontConfig (DgnFontKey (DgnFontType::Shx, Utf8String (name)), shxFileType), fontNumber, project);
            
        default:
            BeAssert (false && L"Unkknown/unexpected ShxFileType.");
            return NULL;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnFontPtr DgnShxFont::CreateLastResortFont ()
    {
    BeFileName lastResortFilePath = T_HOST.GetFontAdmin ()._GetLastResortShxFontFilePath ();
    if (lastResortFilePath.IsEmpty () || !BeFileName::DoesPathExist (lastResortFilePath.GetName ()))
        { BeAssert (false); return NULL; }
    
    static CharCP LAST_RESORT_FONT_NAME = "LastResortShxFont";
    
    DgnShxFontP font = new DgnShxUniFont (LAST_RESORT_FONT_NAME, getEffectiveFontConfig (DgnFontKey (DgnFontType::Shx, Utf8String (LAST_RESORT_FONT_NAME)), SHXFILETYPE_Unifont), lastResortFilePath.GetName ());
    font->m_isLastResort = true;

    return font;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnFontPtr DgnShxFont::CreateMissingFont (Utf8CP name)
    {
    DgnShxFontP font = new DgnShxUniFont (name, getEffectiveFontConfig (DgnFontKey (DgnFontType::Shx, Utf8String (name)), SHXFILETYPE_Unifont), NULL);
    font->m_isMissing = true;
    
    return font;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnFontPtr DgnShxFont::_Embed (DgnDbR project) const
    {
    // Can't embed missing fonts; don't bother embedding last resort fonts.
    if (IsLastResort() || IsMissing())
        return NULL;
    
    T_FontCatalogMap::const_iterator foundFont = project.Fonts().EmbeddedFonts().find (DgnFontKey (DgnFontType::Shx, m_name));
    if (project.Fonts().EmbeddedFonts ().end () != foundFont)
        return foundFont->second;
    
    uint32_t fontNumber;
    if (SUCCESS != project.Fonts().AcquireFontNumber (fontNumber, *this))
        { BeAssert (false); return NULL; }
    
    if (SUCCESS != m_data->_Embed (project, fontNumber))
        { BeAssert (false); return NULL; }

    return DgnShxFont::CreateFromEmbeddedFont (m_name.c_str (), fontNumber, project);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnShxGlyphCP DgnShxFont::FindGlyphCP (FontChar charCode) const
    {
    if (m_isMissing)
        return NULL;
    
    _EnsureFontIsLoaded ();

    T_GlyphMap::const_iterator foundGlyph = m_glyphMap.find (charCode);
    if (m_glyphMap.end () != foundGlyph)
        return foundGlyph->second;

    DgnShxGlyphFPos const* glyphFPos = GetGlyphFilePos (charCode);
    if (NULL == glyphFPos)
        return NULL;
    
    DgnShxGlyphP glyph = new DgnShxGlyph (charCode, glyphFPos->m_filePosition, glyphFPos->m_dataSize, *this, *m_data);
    m_glyphMap[charCode] = glyph;

    return glyph;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
BentleyStatus DgnShxFont::GetGlyphCodeFromGlyphName (FontChar& glyphCode, CharCP glyphName) const
    {
    if (m_isMissing)
        return ERROR;
    
    _EnsureFontIsLoaded ();

    for (T_GlyphFPosMap::const_iterator glyphFPosIter = m_glyphFPosMap.begin (); m_glyphFPosMap.end () != glyphFPosIter; ++glyphFPosIter)
        {
        DgnShxGlyphFPos const& fpos = glyphFPosIter->second;
        
        if (0 != m_data->_Seek (fpos.m_filePosition, 0))
            continue;

        ScopedArray<Byte> pBuf (fpos.m_dataSize);
        m_data->_Read (pBuf.GetData (), sizeof (Byte), (size_t)fpos.m_dataSize);

        if (!doesGlyphHaveName (pBuf.GetData (), fpos.m_dataSize) || (0 != BeStringUtilities::Stricmp (glyphName, reinterpret_cast<CharP>(pBuf.GetData ()))))
            continue;
        
        glyphCode = glyphFPosIter->first;
        return SUCCESS;
        }

    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnShxGlyphFPos const* DgnShxFont::GetGlyphFilePos (FontChar charCode) const
    {
    T_GlyphFPosMap::const_iterator foundGlyphFPos = m_glyphFPosMap.find (charCode);
    if (m_glyphFPosMap.end () == foundGlyphFPos)
        return NULL;
    
    return &foundGlyphFPos->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
bool DgnShxFont::_IsGlyphBlank (FontChar charCode) const
    {
    DgnShxGlyphCP glyph = FindGlyphCP (charCode);
    if (NULL != glyph)
        return glyph->IsBlank ();

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
BentleyStatus DgnShxFont::_LayoutGlyphs (DgnGlyphLayoutContextCR layoutContext, DgnGlyphLayoutResultR layoutResult) const
    {
    if (m_isMissing)
        return ERROR;

    // If you make any changes to this method, also consider examining DgnTrueTypeFont::_LayoutGlyphs and DgnRscFont::_LayoutGlpyhs.
    //  This method differs from the V8i variants in that it is designed to compute only the low-level information needed,
    //  and to serve both TextString and TextBlock through a single code path. This does mean that some extraneous information
    //  is potentially computed, but should be cheap compared to the overall layout operation.

    // Local accessors for some commonly used input.
    DgnGlyphLayoutContext::T_FontCharsCR fontChars = layoutContext.GetFontChars();

    // Local accessors for what we need to generate.
    DRange2dR outRange = layoutResult.GetRangeR();
    DRange2dR outJustificationRange = layoutResult.GetJustificationRangeR();
    DgnGlyphLayoutResult::T_GlyphsR outGlyphs = layoutResult.GetGlyphsR();
    DgnGlyphLayoutResult::T_GlyphCodesR outGlyphCodes = layoutResult.GetGlyphCodesR();
    DgnGlyphLayoutResult::T_GlyphOriginsR outGlyphOrigins = layoutResult.GetGlyphOriginsR();

    _EnsureFontIsLoaded();

    outGlyphs.reserve(fontChars.size());

    // This does the work of digging up glyphs from the rights fonts, as well as expanding any escape sequences to get the display string.
    size_t numGlyphs = LookupGlyphs(outGlyphs, &fontChars.front(), (int)fontChars.size(), layoutContext.ShouldIgnorePercentEscapes());
    if (0 == numGlyphs)
        return SUCCESS;

    static const DgnShxGlyph blankGlyph(0, 0, 0, *this, *m_data);
    for (size_t iGlyph = 0; iGlyph < numGlyphs; ++iGlyph)
        {
        if (NULL != outGlyphs[iGlyph])
            continue;

        outGlyphs[iGlyph] = &blankGlyph;
        }

    // We know how many; might as well reserve.
    outGlyphCodes.reserve(numGlyphs);
    outGlyphOrigins.reserve(numGlyphs);

    // Right-justified text needs to ignore trailing blanks.
    size_t numNonBlankGlyphs = numGlyphs;
    for (; numNonBlankGlyphs > 0; --numNonBlankGlyphs)
        {
        DgnShxGlyphCR glyph = static_cast<DgnShxGlyphCR>(*outGlyphs[numNonBlankGlyphs - 1]);
        
        if (!glyph.IsBlank())
            break;
        }
    
    // Compute origins, ranges, etc...
    DPoint3d penPosition = DPoint3d::FromZero();
    DPoint2d drawSize = layoutContext.GetSize();

    for (size_t i = 0; i < numGlyphs; ++i)
        {
        DgnShxGlyphCR glyph = static_cast<DgnShxGlyphCR>(*outGlyphs[i]);

        outGlyphCodes.push_back(glyph.GetCharCode());
        outGlyphOrigins.push_back(penPosition);

        DRange2d glyphRange;
        glyphRange.low.x = outGlyphOrigins[i].x;
        glyphRange.low.y = outGlyphOrigins[i].y;
        glyphRange.high.x = (glyphRange.low.x + drawSize.x * glyph.GetCellBoxWidth());
        glyphRange.high.y = (glyphRange.low.y + drawSize.y);

        // It is important to use the array overload of DRange2d::Extend; the ranges can be inverted for backwards/upside-down text, and the DRange2d overload enforces low/high semantics.
        outRange.Extend(&glyphRange.low, 2);
        
        if (i < numNonBlankGlyphs)
            outJustificationRange.Extend(&glyphRange.low, 2);

        penPosition.x += (glyph.GetCellBoxRight() * drawSize.x);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
size_t DgnShxFont::LookupGlyphs (DgnGlyphLayoutResult::T_GlyphsR glyphs, uint16_t const* inChars, int nChars, bool shouldIgnorePercentEscapes) const
    {
    CharIter iter(*this, inChars, nChars, shouldIgnorePercentEscapes);
    size_t numGlyphs = 0;
    
    for (; iter.IsValid(); iter.ToNext(), ++numGlyphs)
        {
        FontChar currChar = iter.CurrCharCode();
        DgnShxGlyphCP glyph = FindGlyphCP(currChar);

        glyphs.push_back(glyph);
        }

    return numGlyphs;
    }

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnFontVariant DgnShxShapeFont::_GetVariant () const { return DGNFONTVARIANT_ShxShape; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnShxShapeFont::DgnShxShapeFont (Utf8CP name, DgnFontConfigurationData const& config, WCharCP filePath) :
    T_Super (name, config, filePath),
    m_wasLoaded (false)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnShxShapeFont::DgnShxShapeFont (Utf8CP name, DgnFontConfigurationData const& config, uint32_t fontNumber, DgnDbCR project) :
    T_Super (name, config, fontNumber, project),
    m_wasLoaded (false)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
void DgnShxShapeFont::_EnsureFontIsLoaded () const
    {
    if (m_wasLoaded)
        return;

    m_wasLoaded = true;
    
    if (m_isMissing)
        { BeAssert (false && L"Don't attempt to load a missing font!"); return; }

    if (SUCCESS != m_data->_Open ())
        {
        BeAssert (false);
        m_isMissing = true;
        return;
        }
    
    // Skip header...
    for (size_t iByte = 0; ((iByte < 40) && (0x1a != m_data->_GetNextCharacter ())); ++iByte)
        ;

    // Skip FirstCode and LastCode (both UInt16)...
    m_data->_Seek (4, SEEK_CUR);
    
    size_t  numGlyphs   = (size_t)m_data->GetNextUInt16 ();
    size_t  dataStart   = (m_data->_Tell () + (4 * numGlyphs));
    size_t  dataOffset  = 0;
    
    for (size_t iGlyph = 0; iGlyph < numGlyphs; ++iGlyph)
        {
        DgnShxGlyphFPos glyphFPos;
        glyphFPos.m_filePosition    = (uint16_t)(dataStart + dataOffset);
        glyphFPos.m_charCode        = m_data->GetNextUInt16 ();
        glyphFPos.m_dataSize        = m_data->GetNextUInt16 ();

        m_glyphFPosMap.insert (T_GlyphFPosMap::value_type ((FontChar)glyphFPos.m_charCode, glyphFPos));
        dataOffset += glyphFPos.m_dataSize;
        }
    
    readNonShxBigFontInfo (*m_data, GetGlyphFilePos (0), m_ascender, m_descender, m_charHeight, m_charWidth);
    
    // We have to read a glyph to know if this font is vertical; we consider 0x3f the default glyph, so use that or assume not vertical.
    FindGlyphCP (0x3f);
    }

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnFontVariant DgnShxUniFont::_GetVariant () const { return DGNFONTVARIANT_ShxUni; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnShxUniFont::DgnShxUniFont (Utf8CP name, DgnFontConfigurationData const& config, WCharCP filePath) :
    T_Super (name, config, filePath),
    m_wasLoaded (false)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnShxUniFont::DgnShxUniFont (Utf8CP name, DgnFontConfigurationData const& config, uint32_t fontNumber, DgnDbCR project) :
    T_Super (name, config, fontNumber, project),
    m_wasLoaded (false)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
void DgnShxUniFont::_EnsureFontIsLoaded () const
    {
    if (m_wasLoaded)
        return;

    m_wasLoaded = true;
    
    if (m_isMissing)
        { BeAssert (false && L"Don't attempt to load a missing font!"); return; }
    
    if (SUCCESS != m_data->_Open ())
        {
        BeAssert (false);
        m_isMissing = true;
        return;
        }
    
    // Skip header.
    for (size_t iByte = 0; ((iByte < 40) && (0x1a != m_data->_GetNextCharacter ())); ++iByte)
        ;

    size_t numGlyphs = (size_t)m_data->GetNextUInt16 ();
    
    // Not sure why we have to do this, but old code does, and most fonts indeed have less 1 glyph.
    --numGlyphs;
    
    // Not sure why we have to do this, but old code does, and most fonts indeed have less 1 glyph.
    --numGlyphs;
    
    // Skip 2 bytes...
    m_data->_Seek (2, SEEK_CUR);

    size_t              fontInfoSize    = (size_t)m_data->GetNextUInt16 ();
    ScopedArray<Byte>   fontInfo        (fontInfoSize);
    
    m_data->_Read (fontInfo.GetData (), fontInfoSize, 1);

    size_t  firstNullOffset     = strlen (reinterpret_cast<char*>(fontInfo.GetData ()));
    size_t  fontInfoDataOffset  = (firstNullOffset + 1);

    m_ascender  = fontInfo.GetData ()[fontInfoDataOffset++];
    m_descender = fontInfo.GetData ()[fontInfoDataOffset++];
    
    // Skip "modes".
    ++fontInfoDataOffset;

    // Special case when the "encoding" is "shape file".
    if (2 == fontInfo.GetData ()[fontInfoDataOffset])
        {
        m_ascender  = 1;
        m_descender = 1;
        }

    size_t nextAddress = m_data->_Tell ();

    for (size_t iGlyph = 0; iGlyph < numGlyphs; ++iGlyph)
        {
        if (0 != m_data->_Seek ((long)nextAddress, 0))
            {
            BeDataAssert (false);
            break;
            }

        DgnShxGlyphFPos glyphFPos;
        glyphFPos.m_filePosition    = (uint16_t)(nextAddress + 4);
        glyphFPos.m_charCode        = m_data->GetNextUInt16 ();
        glyphFPos.m_dataSize        = m_data->GetNextUInt16 ();

        nextAddress = (size_t)(glyphFPos.m_filePosition + glyphFPos.m_dataSize);
        
        m_glyphFPosMap[(FontChar)glyphFPos.m_charCode] = glyphFPos;
        }
    
    // We have to read a glyph to know if this font is vertical; we consider 0x3f the default glyph, so use that or assume not vertical.
    FindGlyphCP (0x3f);
    }
