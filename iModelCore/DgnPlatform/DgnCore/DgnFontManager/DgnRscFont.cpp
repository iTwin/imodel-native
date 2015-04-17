/*----------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnFontManager/DgnRscFont.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/

#if defined (BENTLEY_WIN32) // WIP_NONPORT
    #include <Windows.h>
    #include <objbase.h>
    #include <usp10.h>
#endif

#include <DgnPlatformInternal.h>
#include <DgnPlatformInternal/DgnCore/PlatformTextServices.h>

USING_NAMESPACE_BENTLEY_SQLITE

IDgnFontFinder* DgnRscFont::s_fontFinder;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      06/2013
//---------------------------------------------------------------------------------------
void DgnRscFont::RegisterIDgnFontFinder (IDgnFontFinder& finder)
    {
    BeAssert (s_fontFinder == NULL); // MT: This global should be set only by ForeignFormat (publisher)
    s_fontFinder= &finder;
    }

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct RscGlyphElementIterator
    {
    private:    RscGlyphElement const*  m_curr;
    private:    size_t                  m_numRemaining;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: RscGlyphElementIterator (RscGlyphElement const* curr, size_t numRemaining) :
        m_curr          (curr),
        m_numRemaining  (numRemaining - 1)
        {
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: bool                    IsValid     () const    { return (NULL != m_curr); }
    public: RscGlyphElement const*  operator->  () const    { return m_curr; }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: RscGlyphElementIterator ToNext ()
        {
        if (0 == m_numRemaining)
            return RscGlyphElementIterator (NULL, 0);
        
        RscGlyphElement const* next = reinterpret_cast<RscGlyphElement const*>((Byte*)m_curr + sizeof (RscGlyphElement) + (sizeof (RscFontPoint2d) * (m_curr->numVerts - 1)));
        return RscGlyphElementIterator (next, m_numRemaining);
        }
    
    }; // RscGlyphElementIterator

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
static void addPtsToGPA (GPArrayR gpa, bvector<DPoint2d>& pts)
    {
    DPoint3d dPoint;
    dPoint.z = 0.0;

    for (bvector<DPoint2d>::const_iterator curr = pts.begin (); pts.end() != curr; ++curr)
        {
        dPoint.x    = curr->x;
        dPoint.y    = curr->y;
        
        gpa.Add (&dPoint);
        }
    }

//---------------------------------------------------------------------------------------
// Outer polys should be CW, and holes should be CCW.
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
static bool fixDirection (DPoint2dP pts, size_t nPts, RscGlyphElementType ptType)
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

    double area = bsiDPoint2d_getPolygonArea (pts, (int)nPts);
    if (area == 0.0)
        return true;

    if ((0 < area) == cw)
        return false;

    // Reverse direction.
    for (size_t i = 0, j = (nPts - 1); i < j; ++i, --j)
        {
        DPoint2d xy = pts[i];
        pts[i]      = pts[j];
        pts[j]      = xy;
        }
    
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
double      DgnRscGlyph::GetVerticalCellBoxBottom   () const                                    { return m_verticalCellBoxStart.y; }
double      DgnRscGlyph::GetVerticalCellBoxLeft     () const                                    { return m_verticalCellBoxStart.x; }
double      DgnRscGlyph::GetVerticalCellBoxRight    () const                                    { return m_verticalCellBoxEnd.x; }
double      DgnRscGlyph::GetVerticalCellBoxTop      () const                                    { return m_verticalCellBoxEnd.y; }
DgnFontType DgnRscGlyph::_GetType                   () const                                    { return DgnFontType::Rsc; }
bool        DgnRscGlyph::_IsBlank                   () const                                    { return (0 == GetCellBoxWidth ()); }
void        DgnRscGlyph::SetWidth                   (double blackBoxWidth, double cellBoxWidth) { m_blackBoxEnd.x = blackBoxWidth; m_cellBoxEnd.x = cellBoxWidth; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnRscGlyph::DgnRscGlyph (IRscDataAccessor& data, double ascender, bool isFontFilled, RscGlyphHeader const& glyphHeader, RscGlyphDataOffset const& glyphOffset) :
    T_Super (glyphHeader.code),
    m_data                  (&data),
    m_ascenderScaleFactor   (1.0 / ascender),
    m_isFontFilled          (isFontFilled),
    m_glyphDataOffset       (glyphOffset.offset),
    m_glyphDataSize         (glyphOffset.size)
    {
    m_blackBoxStart.x           = (glyphHeader.leftEdge * m_ascenderScaleFactor);
    m_blackBoxStart.y           = 0.0;
    m_blackBoxEnd.x             = (glyphHeader.rightEdge * m_ascenderScaleFactor);
    m_blackBoxEnd.y             = 1.0;
    m_cellBoxStart.x            = 0.0;
    m_cellBoxStart.y            = 0.0;
    m_cellBoxEnd.x              = (glyphHeader.width * m_ascenderScaleFactor);
    m_cellBoxEnd.y              = 1.0;
    m_verticalCellBoxStart.x    = (m_blackBoxEnd.x / 2.0);
    m_verticalCellBoxEnd.x      = (m_blackBoxEnd.x / 2.0);
    m_verticalCellBoxStart.y    = 1.0;
    m_verticalCellBoxEnd.y      = 0.0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnRscGlyph::DgnRscGlyph (FontChar charCode) :
    T_Super (charCode),
    m_data                  (NULL),
    m_ascenderScaleFactor   (0.0),
    m_isFontFilled          (false),
    m_glyphDataOffset       (0),
    m_glyphDataSize         (0)
    {
    m_blackBoxStart.x           = 0.0;
    m_blackBoxStart.y           = 1.0;
    m_blackBoxEnd.x             = 1.0;
    m_blackBoxEnd.y             = 0.0;
    m_cellBoxStart.x            = 0.0;
    m_cellBoxStart.y            = 0.0;
    m_cellBoxEnd.x              = 1.0;
    m_cellBoxEnd.y              = 1.0;
    m_verticalCellBoxStart.x    = 0.5;
    m_verticalCellBoxEnd.x      = 0.5;
    m_verticalCellBoxStart.y    = 1.0;
    m_verticalCellBoxEnd.y      = 0.0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
BentleyStatus DgnRscGlyph::_FillGpa (GPArrayR gpa) const
    {
    if (0 == m_glyphDataSize)
        return SUCCESS;

    ScopedArray<Byte>   glyphDataBuffer (m_glyphDataSize);
    RscGlyphData*       glyphData       = reinterpret_cast<RscGlyphData*>(glyphDataBuffer.GetData ());
    
    if (SUCCESS != m_data->_ReadGlyphData (glyphDataBuffer.GetData (), m_glyphDataOffset, m_glyphDataSize))
        return  ERROR;

    if (0 == glyphData->numElems)
        return ERROR;

    bool    hasLines    = false;
    bool    hasPolys    = false;

    for (RscGlyphElementIterator curr (reinterpret_cast<RscGlyphElement const*>(glyphData + 1), (size_t)glyphData->numElems); curr.IsValid (); curr = curr.ToNext ())
        {
        size_t              nPts        = (size_t)curr->numVerts;
        RscFontPoint2d const*     glyphPoint = curr->vertice;
        RscFontPoint2d const*     glyphEnd   = (glyphPoint + nPts);
        bvector<DPoint2d>   tPts;
        
        for (; glyphPoint < glyphEnd; ++glyphPoint)
            {
            DPoint2d pt2d;
            pt2d.x = (glyphPoint->x * m_ascenderScaleFactor);
            pt2d.y = (glyphPoint->y * m_ascenderScaleFactor);
            
            tPts.push_back(pt2d);
            }

        if (fixDirection (&tPts[0], nPts, (RscGlyphElementType)curr->type))
            continue;

        int firstPtNum = gpa.GetCount ();
        addPtsToGPA (gpa, tPts);

        gpa.MarkBreak();

        GraphicsPoint* firstPt = gpa.GetPtr (firstPtNum);
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
                gpa.MarkMajorBreak ();
                break;

            case RSCGLYPHELEMENT_PolyHole:
            case RSCGLYPHELEMENT_PolyHoleFinal:
                firstPt->mask |= HMASK_RSC_HOLE;
                gpa.MarkMajorBreak ();
                break;
            }
        }

    // Add major break so we can transform individual glyphs.
    gpa.MarkMajorBreak ();

    // Since QV won't draw any lines if fill is on, we can't turn it on if the glyph has any lines in it.
    if (!hasLines || (hasPolys && m_isFontFilled))
        gpa.SetArrayMask (HPOINT_ARRAYMASK_FILL);

    return  SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
PropertySpec    DgnRscFont::CreateEmbeddedFontHeaderPropertySpec        ()  { return PropertySpec ("fnthdr",    PROPERTY_APPNAME_DgnFont); }
PropertySpec    DgnRscFont::CreateEmbeddedFractionsPropertySpec         ()  { return PropertySpec ("fracs",     PROPERTY_APPNAME_DgnFont); }
PropertySpec    DgnRscFont::CreateEmbeddedGlyphDataPropertySpec         ()  { return PropertySpec ("vecs",      PROPERTY_APPNAME_DgnFont); }
PropertySpec    DgnRscFont::CreateEmbeddedGlyphDataOffsetsPropertySpec  ()  { return PropertySpec ("vecoffs",   PROPERTY_APPNAME_DgnFont); }
PropertySpec    DgnRscFont::CreateEmbeddedGlyphHeadersPropertySpec      ()  { return PropertySpec ("chrhdrs",   PROPERTY_APPNAME_DgnFont); }


//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct DbRscDataReader : public IRscDataAccessor
    {
    private:    uint32_t        m_fontNumber;
    private:    DgnDbCR    m_dgndb;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: DbRscDataReader (uint32_t fontNumber, DgnDbCR project) :
        m_fontNumber    (fontNumber),
        m_dgndb       (project)
        {
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _Embed (DgnDbR project, uint32_t fontNumber) override
        {
        bvector<Byte> buffer;

        //.........................................................................................
        PropertySpec    fontHeaderPropSpec  = DgnRscFont::CreateEmbeddedFontHeaderPropertySpec ();
        uint32_t            fontHeaderPropSize;
        if ((BE_SQLITE_ROW != m_dgndb.QueryPropertySize (fontHeaderPropSize, fontHeaderPropSpec, m_fontNumber)) || (0 == fontHeaderPropSize))
            return ERROR;
    
        buffer.resize (fontHeaderPropSize);
        if (BE_SQLITE_ROW != m_dgndb.QueryProperty (&buffer[0], (uint32_t)buffer.size (), fontHeaderPropSpec, m_fontNumber))
            return ERROR;
        
        if (BE_SQLITE_OK != project.SaveProperty (fontHeaderPropSpec, &buffer[0], (uint32_t)buffer.size (), fontNumber))
            return ERROR;
            
        //.........................................................................................
        PropertySpec    charHeadersPropSpec     = DgnRscFont::CreateEmbeddedGlyphHeadersPropertySpec ();
        uint32_t            charHeadersPropSize;
        if ((BE_SQLITE_ROW != m_dgndb.QueryPropertySize (charHeadersPropSize, charHeadersPropSpec, m_fontNumber)) || (0 == charHeadersPropSize))
            return ERROR;
    
        buffer.resize (charHeadersPropSize);
        if (BE_SQLITE_ROW != m_dgndb.QueryProperty (&buffer[0], (uint32_t)buffer.size (), charHeadersPropSpec, m_fontNumber))
            return ERROR;
        
        if (BE_SQLITE_OK != project.SaveProperty (charHeadersPropSpec, &buffer[0], (uint32_t)buffer.size (), fontNumber))
            return ERROR;
        
        //.........................................................................................
        PropertySpec    glyphDataOffsetsPropSpec    = DgnRscFont::CreateEmbeddedGlyphDataOffsetsPropertySpec ();
        uint32_t            glyphDataOffsetsPropSize;
        if ((BE_SQLITE_ROW != m_dgndb.QueryPropertySize (glyphDataOffsetsPropSize, glyphDataOffsetsPropSpec, m_fontNumber)) || (0 == glyphDataOffsetsPropSize))
            return ERROR;
    
        buffer.resize (glyphDataOffsetsPropSize);
        if (BE_SQLITE_ROW != m_dgndb.QueryProperty (&buffer[0], (uint32_t)buffer.size (), glyphDataOffsetsPropSpec, m_fontNumber))
            return ERROR;

        if (BE_SQLITE_OK != project.SaveProperty (glyphDataOffsetsPropSpec, &buffer[0], (uint32_t)buffer.size (), fontNumber))
            return ERROR;
        
        //.........................................................................................
        PropertySpec    glyphDataPropSpec   = DgnRscFont::CreateEmbeddedGlyphDataPropertySpec ();
        uint32_t            glyphDataPropSize;
        if ((BE_SQLITE_ROW != m_dgndb.QueryPropertySize (glyphDataPropSize, glyphDataPropSpec, m_fontNumber)) || (0 == glyphDataPropSize))
            return ERROR;
    
        buffer.resize (glyphDataPropSize);
        if (BE_SQLITE_ROW != m_dgndb.QueryProperty (&buffer[0], (uint32_t)buffer.size (), glyphDataPropSpec, m_fontNumber))
            return ERROR;

        if (BE_SQLITE_OK != project.SaveProperty (glyphDataPropSpec, &buffer[0], (uint32_t)buffer.size (), fontNumber))
            return ERROR;
        
        //.........................................................................................
        PropertySpec fractionsPropSpec = DgnRscFont::CreateEmbeddedFractionsPropertySpec ();
    
        uint32_t fractionsPropSize;
        if ((BE_SQLITE_ROW == m_dgndb.QueryPropertySize (fractionsPropSize, fractionsPropSpec, m_fontNumber)) && (fractionsPropSize > 0))
            {
            buffer.resize (fractionsPropSize);
            if (BE_SQLITE_ROW != m_dgndb.QueryProperty (&buffer[0], (uint32_t)buffer.size (), fractionsPropSpec, m_fontNumber))
                return ERROR;

            if (BE_SQLITE_OK != project.SaveProperty (fractionsPropSpec, &buffer[0], (uint32_t)buffer.size (), fontNumber))
                return ERROR;
            }
        
        return SUCCESS;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _ReadFontHeader (bvector<Byte>& fontHeaderBuffer) override
        {
        PropertySpec fontHeaderPropSpec = DgnRscFont::CreateEmbeddedFontHeaderPropertySpec ();
        
        {
        //  This has no effect unless there is a range tree query occurring during update dynamics.  See comments
        //  on HighPriorityOperationBlock for more information.
        BeSQLite::HighPriorityOperationBlock highPriorityOperationBlock;

        uint32_t fontHeaderPropSize;
        if ((BE_SQLITE_ROW != m_dgndb.QueryPropertySize (fontHeaderPropSize, fontHeaderPropSpec, m_fontNumber)) || (0 == fontHeaderPropSize))
            return ERROR;
    
        fontHeaderBuffer.resize (fontHeaderPropSize);
        
        if (BE_SQLITE_ROW != m_dgndb.QueryProperty (&fontHeaderBuffer[0], fontHeaderPropSize, fontHeaderPropSpec, m_fontNumber))
            return ERROR;
        }

        return SUCCESS;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _ReadFractionMap (DgnRscFont::T_FractionMap& fractMap) override
        {
        PropertySpec fractionsPropSpec = DgnRscFont::CreateEmbeddedFractionsPropertySpec ();
    
        RscFontFraction rawFractMap[MAX_FRACTION_MAP_COUNT];
        {
        //  This has no effect unless there is a range tree query occurring during update dynamics.  See comments
        //  on HighPriorityOperationBlock for more information.
        BeSQLite::HighPriorityOperationBlock highPriorityOperationBlock;

        uint32_t fractionsPropSize;
        if ((BE_SQLITE_ROW != m_dgndb.QueryPropertySize (fractionsPropSize, fractionsPropSpec, m_fontNumber)) || (0 == fractionsPropSize))
            return ERROR;
        
        if (fractionsPropSize > sizeof (rawFractMap))
            return ERROR;
        
        if (BE_SQLITE_ROW != m_dgndb.QueryProperty (rawFractMap, fractionsPropSize, fractionsPropSpec, m_fontNumber))
            return ERROR;
        }

        for (size_t iFract = 0; iFract < MAX_FRACTION_MAP_COUNT; ++iFract)
            {
            if (0 == rawFractMap[iFract].denominator)
                break;
            
            fractMap[rawFractMap[iFract].charCode] = DgnRscFont::T_FractionMap::mapped_type (rawFractMap[iFract].numerator, rawFractMap[iFract].denominator);
            }
        
        return SUCCESS;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _ReadGlyphData (Byte* glyphData, size_t dataOffset, size_t dataSize) override
        {
        PropertySpec glyphDataPropSpec = DgnRscFont::CreateEmbeddedGlyphDataPropertySpec ();
    
        {
        //  This has no effect unless there is a range tree query occurring during update dynamics.  See comments
        //  on HighPriorityOperationBlock for more information.
        BeSQLite::HighPriorityOperationBlock highPriorityOperationBlock;

        uint32_t glyphDataPropSize;
        if ((BE_SQLITE_ROW != m_dgndb.QueryPropertySize (glyphDataPropSize, glyphDataPropSpec, m_fontNumber)) || (0 == glyphDataPropSize))
            return ERROR;
    
        ScopedArray<Byte> glyphDataPropValue (glyphDataPropSize);
        if (BE_SQLITE_ROW != m_dgndb.QueryProperty (glyphDataPropValue.GetData (), glyphDataPropSize, glyphDataPropSpec, m_fontNumber))
            return ERROR;

        memcpy (glyphData, glyphDataPropValue.GetData () + dataOffset, dataSize);
        }

        return SUCCESS;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _ReadGlyphDataOffsets (bvector<RscGlyphDataOffset>& glyphOffsets) override
        {
        PropertySpec glyphDataOffsetsPropSpec = DgnRscFont::CreateEmbeddedGlyphDataOffsetsPropertySpec ();
    
        {
        //  This has no effect unless there is a range tree query occurring during update dynamics.  See comments
        //  on HighPriorityOperationBlock for more information.
        BeSQLite::HighPriorityOperationBlock highPriorityOperationBlock;

        uint32_t glyphDataOffsetsPropSize;
        if ((BE_SQLITE_ROW != m_dgndb.QueryPropertySize (glyphDataOffsetsPropSize, glyphDataOffsetsPropSpec, m_fontNumber)) || (0 == glyphDataOffsetsPropSize))
            return ERROR;
    
        glyphOffsets.resize (glyphDataOffsetsPropSize / sizeof (RscGlyphDataOffset));
        
        if (BE_SQLITE_ROW != m_dgndb.QueryProperty (&glyphOffsets[0], glyphDataOffsetsPropSize, glyphDataOffsetsPropSpec, m_fontNumber))
            return ERROR;
        }

        return SUCCESS;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _ReadGlyphHeaders (bvector<RscGlyphHeader>& glyphHeaders) override
        {
        PropertySpec charHeadersPropSpec = DgnRscFont::CreateEmbeddedGlyphHeadersPropertySpec ();
    
        {
        //  This has no effect unless there is a range tree query occurring during update dynamics.  See comments
        //  on HighPriorityOperationBlock for more information.
        BeSQLite::HighPriorityOperationBlock highPriorityOperationBlock;

        uint32_t charHeadersPropSize;
        if ((BE_SQLITE_ROW != m_dgndb.QueryPropertySize (charHeadersPropSize, charHeadersPropSpec, m_fontNumber)) || (0 == charHeadersPropSize))
            return ERROR;
    
        glyphHeaders.resize (charHeadersPropSize / sizeof (RscGlyphHeader));
        
        if (BE_SQLITE_ROW != m_dgndb.QueryProperty (&glyphHeaders[0], charHeadersPropSize, charHeadersPropSpec, m_fontNumber))
            return ERROR;
        }

        return SUCCESS;
        }
    
    }; // DbRscDataReader

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     06/2010
//=======================================================================================
struct IRscGlyphShaper
    {
    public: virtual bool            _IsValidLogicalGlyphCode    (uint16_t glyphCode) = 0;
    public: virtual BentleyStatus   _ShapeGlyphs                (uint16_t* glyphCodes, size_t numGlyphCodes) = 0;
    
    }; // IRscGlyphShaper

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     05/2010
//=======================================================================================
struct ArabicRscGlyphShaper : public IRscGlyphShaper
    {
    //=======================================================================================
    // @bsiclass                                                    Jeff.Marker     05/2010
    //=======================================================================================
    private: enum GlyphType
        {
        GLYPH_TYPE_None,
        GLYPH_TYPE_Isolated,
        GLYPH_TYPE_Initial,
        GLYPH_TYPE_Medial,
        GLYPH_TYPE_Final,
        GLYPH_TYPE_Diacritical
    
        }; // GlyphType

    //=======================================================================================
    // @bsiclass
    //=======================================================================================
    private: struct LigaturePair
        {
        public: unsigned char m_first;
        public: unsigned char m_second;
        public: uint16_t m_ligature;
    
        }; // LigaturePair

    private: static const LigaturePair  s_ligaturePairs[];
    private: static const size_t        s_numLigaturePairs;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Graeme.Coutts   09/95
    //---------------------------------------------------------------------------------------
    private: static bool IsGlyphOfType (uint16_t glyphCode, GlyphType glyphType)
        {
        //=======================================================================================
        // @bsiclass
        //=======================================================================================
        struct GlyphTypeData
            {
            public: uint8_t m_isIsolated    :1;
            public: uint8_t m_isFinal       :1;
            public: uint8_t m_isMedial      :1;
            public: uint8_t m_isInitial     :1;
            public: uint8_t m_isDiacritical :1;
            public: uint8_t unused          :3;
        
            }; // GlyphTypeData

        static const GlyphTypeData s_glyphTypes[] =
            {
            // 0x80 - 0x8f
            {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },  {   true,   false,  false,  false,  false   },
            {   true,   false,  false,  false,  false   },  {   true,   false,  false,  false,  false   },  {   true,   false,  false,  false,  false   },  {   true,   false,  false,  false,  false   },
            {   true,   false,  false,  false,  false   },  {   true,   false,  false,  false,  false   },  {   true,   false,  false,  false,  false   },  {   true,   true,   true,   true,   true    },
            {   true,   true,   true,   true,   true    },  {   true,   true,   true,   true,   true    },  {   true,   true,   true,   true,   true    },  {   true,   true,   true,   true,   true    },
        
            // 0x90 - 0x9f
            {   true,   true,   true,   true,   true    },  {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },
            {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },
            {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },
            {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },
        
            // 0xa0 - 0xaf
            {   false,  false,  false,  false,  false   },  {   true,   false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },
            {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },
            {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },
            {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },

            // 0xb0 - 0xbf        
            {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },
            {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },
            {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },  {   true,   false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },
            {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },  {   true,   false,  false,  false,  false   },
        
            // 0xc0 - 0xcf
            {   false,  false,  false,  false,  false   },  {   true,   false,  false,  false,  false   },  {   true,   true,   false,  false,  true    },  {   true,   true,   false,  false,  true    },
            {   true,   true,   false,  false,  true    },  {   true,   true,   false,  false,  true    },  {   true,   true,   true,   true,   true    },  {   true,   true,   false,  false,  true    },
            {   true,   true,   true,   true,   true    },  {   true,   true,   false,  false,  true    },  {   true,   true,   true,   true,   true    },  {   true,   true,   true,   true,   true    },
            {   true,   true,   true,   true,   true    },  {   true,   true,   true,   true,   true    },  {   true,   true,   true,   true,   true    },  {   true,   true,   false,  false,  true    },
        
            // 0xd0 - 0xdf
            {   true,   true,   false,  false,  true    },  {   true,   true,   false,  false,  true    },  {   true,   true,   false,  false,  true    },  {   true,   true,   true,   true,   true    },
            {   true,   true,   true,   true,   true    },  {   true,   true,   true,   true,   true    },  {   true,   true,   true,   true,   true    },  {   false,  false,  false,  false,  false   },
            {   true,   true,   true,   true,   true    },  {   true,   true,   true,   true,   true    },  {   true,   true,   true,   true,   true    },  {   true,   true,   true,   true,   true    },
            {   true,   true,   true,   true,   true    },  {   true,   true,   true,   true,   true    },  {   true,   true,   true,   true,   true    },  {   true,   true,   true,   true,   true    },
        
            // 0xe0 - 0xef
            {   false,  false,  false,  false,  false   },  {   true,   true,   true,   true,   true    },  {   false,  false,  false,  false,  false   },  {   true,   true,   true,   true,   true    },
            {   true,   true,   true,   true,   true    },  {   true,   true,   true,   true,   true    },  {   true,   true,   false,  false,  true    },  {   false,  false,  false,  false,  false   },
            {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },
            {   true,   true,   false,  false,  true    },  {   true,   true,   true,   true,   true    },  {   false,  false,  false,  false,  false   },  {   false,  false,  false,  false,  false   },
        
            // 0xf0 - 0xff
            {   true,   true,   true,   true,   true    },  {   true,   true,   true,   true,   true    },  {   true,   true,   true,   true,   true    },  {   true,   true,   true,   true,   true    },
            {   false,  false,  false,  false,  false   },  {   true,   true,   true,   true,   true    },  {   true,   true,   true,   true,   true    },  {   false,  false,  false,  false,  false   },
            {   true,   true,   true,   true,   true    },  {   false,  false,  false,  false,  false   },  {   true,   true,   true,   true,   true    },  {   false,  false,  false,  false,  false   },
            {   true,   true,   false,  false,  true    },  {   true,   true,   false,  false,  true    },  {   true,   true,   false,  false,  true    },  {   true,   true,   false,  false,  true    }
            };

        if (glyphCode < 0x80)
            return false;

        switch (glyphType)
            {
            case GLYPH_TYPE_Isolated:       return s_glyphTypes[glyphCode - 0x80].m_isIsolated;
            case GLYPH_TYPE_Final:          return s_glyphTypes[glyphCode - 0x80].m_isFinal;
            case GLYPH_TYPE_Medial:         return s_glyphTypes[glyphCode - 0x80].m_isMedial;
            case GLYPH_TYPE_Initial:        return s_glyphTypes[glyphCode - 0x80].m_isInitial;
            case GLYPH_TYPE_Diacritical:    return s_glyphTypes[glyphCode - 0x80].m_isDiacritical;
        
            default:
                BeAssert (false);
                return false;
            }
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Graeme.Coutts   04/96
    //---------------------------------------------------------------------------------------
    private: static bool IsGlyphAnArabicDiacritic (uint16_t glyphCode)
        {
        uint16_t highByte = (glyphCode >> 8);

        if (0 == highByte)
            return ((0xf0 <= glyphCode && glyphCode <= 0xf3)
                        || (0xf5 <= glyphCode && glyphCode <= 0xf6)
                        || (0xf8 == glyphCode)
                        || (0xfa == glyphCode)
                        || (0x8b <= glyphCode && glyphCode <= 0x90));
    
        return (0x83 <= highByte && highByte <= 0x90);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Graeme.Coutts   05/96
    //---------------------------------------------------------------------------------------
    private: static bool IsGlyphAnEnglishPhonetic (uint16_t glyphCode)
        {
        return ((0xaf == glyphCode) || (0xb4 == glyphCode) || (0xb8 == glyphCode));
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                  Hiroo.Jumonji   01/96
    //---------------------------------------------------------------------------------------
    private: static bool IsGlyphAnArabicDigit (uint16_t glyphCode)
        {
        return ((0x96 <= glyphCode) && (glyphCode <= 0x9f));
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Graeme.Coutts   04/96
    //---------------------------------------------------------------------------------------
    private: static uint16_t ComputeDiacriticGlyph (uint16_t baseGlyph, GlyphType baseGlyphType, uint16_t diacritic)
        {
        uint16_t highByte;
        uint16_t lowByte;

        // Value of diacritic becomes new high byte value.
        if (0xf0 == diacritic)
           highByte = 0x8300;
        else if (0xf1 == diacritic)
           highByte = 0x8400;
        else if (0xf2 == diacritic)
           highByte = 0x8500;
        else if (0xf3 == diacritic)
           highByte = 0x8600;
        else if (0xf5 == diacritic)
           highByte = 0x8700;
        else if (0xf6 == diacritic)
           highByte = 0x8800;
        else if (0xf8 == diacritic)
           highByte = 0x8900;
        else if (0xfa == diacritic)
           highByte = 0x8a00;
        else
           highByte = (diacritic << 8);

        // Low byte is adjusted depending on the base char type.
        if (GLYPH_TYPE_Isolated == baseGlyphType)
            lowByte = baseGlyph;
        else if (GLYPH_TYPE_Initial == baseGlyphType)
            lowByte = (baseGlyph - 0xa0);
        else if (GLYPH_TYPE_Medial == baseGlyphType)
            lowByte = (baseGlyph - 0x70);
        else if (GLYPH_TYPE_Final == baseGlyphType)
            lowByte = (baseGlyph - 0x40);
        else
           return baseGlyph;

        return (highByte + lowByte);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Graeme.Coutts   04/96
    //---------------------------------------------------------------------------------------
    private: static BentleyStatus ComputeLigature (uint16_t firstGlyph, uint16_t secondGlyph, uint16_t& ligatureGlyph)
        {
        for (size_t iLigaturePair = 0; iLigaturePair < s_numLigaturePairs; ++iLigaturePair)
            {
            if (firstGlyph == s_ligaturePairs[iLigaturePair].m_first && secondGlyph == s_ligaturePairs[iLigaturePair].m_second)
                {
                ligatureGlyph = s_ligaturePairs[iLigaturePair].m_ligature;
                return SUCCESS;
                }
            }
    
        return ERROR;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     06/2010
    //---------------------------------------------------------------------------------------
    public: virtual bool _IsValidLogicalGlyphCode (uint16_t glyphCode) override
        {
        // Ligatures do not belong in the logical character stream. Additionally, our delivered Arabic RSC font re-purposes certain glyph codes to
        //  mean ligatures (e.g. the RLM mark), so we cannot let them pass through; otherwise non-printable control characters show up as ligatures.
        
        for (size_t iLigaturePair = 0; iLigaturePair < s_numLigaturePairs; ++iLigaturePair)
            {
            if (s_ligaturePairs[iLigaturePair].m_ligature == glyphCode)
                return false;
            }
    
        return true;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Graeme.Coutts   09/95
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _ShapeGlyphs (uint16_t* glyphCodes, size_t numGlyphCodes) override
        {
        // Do not process 0- or 1-character strings.
        if (numGlyphCodes < 2)
            return ERROR;

        GlyphType leftGlyphType = GLYPH_TYPE_None;

        // Analyze each character, comparing with the left and right characters.
        for (uint16_t *inP = glyphCodes, *outP = glyphCodes; (size_t)(inP - glyphCodes) < numGlyphCodes; )
            {
            *inP &= 0xff;

            // Is the active character an arabic character?
            if (ArabicRscGlyphShaper::IsGlyphOfType (*inP, GLYPH_TYPE_Isolated))
                {
                int     charSkip    = 0;
                uint16_t rightGlyph;
            
                if (inP < (glyphCodes + numGlyphCodes - 1))
                    rightGlyph = *(inP + 1);
                else
                    rightGlyph = 0;
            
                rightGlyph &= 0xff;

                // Check this char against next char for ligature.
                if (0 != rightGlyph)
                    {
                    uint16_t ligatureChar;
                    if (SUCCESS == ArabicRscGlyphShaper::ComputeLigature (*inP, *(inP + 1), ligatureChar))
                        {
                        *inP = ligatureChar;
                    
                        if (inP < (glyphCodes + numGlyphCodes - 2))
                            rightGlyph = *(inP + 2);
                        else
                            rightGlyph = 0;
                    
                        charSkip = 1;
                        }
                    }

                // Do not use a diacritic as the right character for analysis.
                if (0 != rightGlyph && ArabicRscGlyphShaper::IsGlyphAnArabicDiacritic (rightGlyph))
                    {
                    uint16_t* tmpP = (inP + charSkip + 1);
                    for (; ArabicRscGlyphShaper::IsGlyphAnArabicDiacritic (*tmpP) && tmpP <= (glyphCodes + numGlyphCodes); ++tmpP)
                       ;;
                
                    rightGlyph = *tmpP;
                    }

                // Diacritic processing
                if (ArabicRscGlyphShaper::IsGlyphAnArabicDiacritic (*inP))
                    {
                    if ((GLYPH_TYPE_None != leftGlyphType) && !ArabicRscGlyphShaper::IsGlyphAnArabicDiacritic (*(outP - 1)))
                        *(outP - 1) = ArabicRscGlyphShaper::ComputeDiacriticGlyph ((*(outP - 1) & 0xff), leftGlyphType, *inP);
                    else
                        *outP++ = *inP;
                
                    inP += (charSkip + 1);
                
                    continue;
                    }

                uint16_t newHighByte = 0;
            
                if ((GLYPH_TYPE_None == leftGlyphType) || (GLYPH_TYPE_Isolated == leftGlyphType) || (GLYPH_TYPE_Final == leftGlyphType))
                    {
                    if (ArabicRscGlyphShaper::IsGlyphOfType (*inP, GLYPH_TYPE_Initial) && ArabicRscGlyphShaper::IsGlyphOfType (rightGlyph, GLYPH_TYPE_Final))
                        {
                        newHighByte     = 0x8000;
                        leftGlyphType   = GLYPH_TYPE_Initial;
                        }
                    else
                        {
                        newHighByte     = 0;
                        leftGlyphType   = GLYPH_TYPE_Isolated;
                        }
                    }
                else if ((GLYPH_TYPE_Initial == leftGlyphType) || (GLYPH_TYPE_Medial == leftGlyphType))
                    {
                    if (ArabicRscGlyphShaper::IsGlyphOfType (*inP, GLYPH_TYPE_Medial) && ArabicRscGlyphShaper::IsGlyphOfType (rightGlyph, GLYPH_TYPE_Final))
                        {
                        newHighByte     = 0x8100;
                        leftGlyphType   = GLYPH_TYPE_Medial;
                        }
                    else
                        {
                        newHighByte     = 0x8200;
                        leftGlyphType   = GLYPH_TYPE_Final;
                        }
                    }

                *outP++ = (newHighByte + *inP);
                inP     += (charSkip + 1);
                }
            else if (ArabicRscGlyphShaper::IsGlyphAnEnglishPhonetic (*(inP + 1)))
                {
                uint16_t ligatureChar;
                if (SUCCESS == ArabicRscGlyphShaper::ComputeLigature (*inP, *(inP + 1), ligatureChar))
                    {
                    *outP++ = ligatureChar;
                    inP     += 2;
                    }
                else
                    {
                    *outP++ = *inP++;
                    }
            
                leftGlyphType = GLYPH_TYPE_None;
                }
            /*
            This is the wrong place to do things like this; we should find the places that don't let you specify a number format and fix them instead.
            
            else if (('.' == *inP) && (inP > glyphCodes) && ArabicRscGlyphShaper::IsGlyphAnArabicDigit (*(inP - 1)) && ArabicRscGlyphShaper::IsGlyphAnArabicDigit(*(inP + 1)))
                {
                inP++;
                *outP++         = ',';
                leftGlyphType   = GLYPH_TYPE_None;
                }
            */
            else
                {
                *outP++         = *inP++;
                leftGlyphType   = GLYPH_TYPE_None;
                }
            }
        
        return SUCCESS;
        }

    }; // ArabicRscGlyphShaper

const ArabicRscGlyphShaper::LigaturePair ArabicRscGlyphShaper::s_ligaturePairs[] =
    {
    {   0xe1,   0xc7,   0xfc    },  {   0xe1,   0xc3,   0xfd    },  {   0xe1,   0xc2,   0xfe    },  {   0xe1,   0xc5,   0xff    },
    
    // The following are the diacritic combinations - not really ligatures, but can be processed as such.
    {   0xf8,   0xf0,   0x8b    },  {   0xf8,   0xf1,   0x8c    },  {   0xf8,   0xf2,   0x8d    },  {   0xf8,   0xf3,   0x8e    },
    {   0xf8,   0xf5,   0x8f    },  {   0xf8,   0xf6,   0x90    },

    // The following are the english phonetic combinations.
    {   0x41,   0xaf,   0x8300  },  {   0x41,   0xb4,   0x8301  },  {   0x61,   0xaf,   0x8302  },  {   0x61,   0xb4,   0x8303  },
    {   0x44,   0xb8,   0x8304  },  {   0x64,   0xb8,   0x8305  },  {   0x48,   0xb8,   0x8306  },  {   0x68,   0xb8,   0x8307  },
    {   0x54,   0xb8,   0x8308  },  {   0x74,   0xb8,   0x8309  },  {   0x55,   0xaf,   0x830a  },  {   0x75,   0xaf,   0x830b  },
    {   0x5a,   0xb8,   0x830c  },  {   0x7a,   0xb8,   0x830d  },  {   0x49,   0xaf,   0x830e  },  {   0x69,   0xaf,   0x830f  },
    {   0x53,   0xb8,   0x8310  },  {   0x73,   0xb8,   0x8311  }
    };

const size_t ArabicRscGlyphShaper::s_numLigaturePairs = _countof (ArabicRscGlyphShaper::s_ligaturePairs);

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     06/2010
//=======================================================================================
struct RscGlyphShaperManager
    {
    private: typedef map<LangCodePage, IRscGlyphShaper*> T_IRscGlyphShaperMap;
    
    private:    static  RscGlyphShaperManager*  s_instance;
    private:            T_IRscGlyphShaperMap    m_shapers;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     06/2010
    //---------------------------------------------------------------------------------------
    public: static RscGlyphShaperManager* GetInstance ()
        {
        if (NULL == s_instance)
            {
            s_instance = new RscGlyphShaperManager ();
            
            s_instance->m_shapers[LangCodePage::Arabic] = new ArabicRscGlyphShaper ();
            }

        return s_instance;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     06/2010
    //---------------------------------------------------------------------------------------
    public: IRscGlyphShaper* GetShaperForCodePage (LangCodePage codePage) const
        {
        T_IRscGlyphShaperMap::const_iterator shaperIter = m_shapers.find (codePage);
        if (m_shapers.end () == shaperIter)
            return NULL;

        return shaperIter->second;
        }
    
    }; // RscGlyphShaperManager

RscGlyphShaperManager* RscGlyphShaperManager::s_instance = NULL;

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
static uint8_t computeGreatestCommonDenominator (uint8_t lhs, uint8_t rhs)
    {
    if (0 == rhs)
        return lhs;

    return computeGreatestCommonDenominator (rhs, lhs % rhs);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
static bool detectFraction (uint32_t& numerator, uint32_t& denominator, size_t& numCharsInFraction, WCharCP wholeString, WCharCP subString)
    {
    // BeStringUtilities::Wtoi will eat whitespace looking for numbers, so ignore whitespace separately.
    if (iswspace (*subString))
        return false;
    
    // Don't attempt to process if preceded by a slash (e.g. a 3-part date).
    if ((wholeString < subString) && (L'/' == *(subString - 1)))
        return false;
    
    // Need to differentiate literal zero vs. failure to parse a number... Also don't allow sign characters...
    if (!iswdigit (*subString))
        return false;
    
    WCharCP originalStr = subString;
    int     numeratorI  = BeStringUtilities::Wtoi (subString);

    if (numeratorI <= 0)
        return false;

    subString += (size_t)(floor (log10 ((double)numeratorI)) + 1.0);

    // We're dealing with old-style RSC fractions... only search for '/' (and not '#' or '^' since we can't do anything with that data).
    if (L'/' != *subString)
        return false;
    
    ++subString;

    if (!iswdigit (*subString))
        return false;
    
    int denominatorI = BeStringUtilities::Wtoi (subString);
    
    if (denominatorI <= 0)
        return false;

    subString += (size_t)(floor (log10 ((double)denominatorI)) + 1.0);

    // Don't stack on a trailing '/' (e.g. a 3-part date).
    if (L'/' == *subString)
        return false;

    numerator           = (uint32_t)numeratorI;
    denominator         = (uint32_t)denominatorI;
    numCharsInFraction  = (subString - originalStr);
    
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnFontConfigurationData DgnRscFont::GetEffectiveFontConfig (DgnFontKey const& fontKey)
    {
    DgnFontConfigurationData const* customConfig = DgnFontManager::FindCustomFontConfiguration (fontKey);
    if (NULL != customConfig)
        return *customConfig;
    
    DgnFontConfigurationData defaultConfig;
    
    LangCodePage acp;
    BeStringUtilities::GetCurrentCodePage (acp);
                
    defaultConfig.m_codePage                = acp;
    defaultConfig.m_degreeCharCode          = 94;
    defaultConfig.m_diameterCharCode        = 216;
    defaultConfig.m_plusMinusCharCode       = 200;
    defaultConfig.m_shouldCreateShxUnifont  = true;

    return defaultConfig;
    }

#if defined (BENTLEY_WIN32) // WIP_NONPORT DgnRscFont layout

//---------------------------------------------------------------------------------------
// Official Msdn docs: http://msdn.microsoft.com/en-us/library/dd374091(VS.85).aspx
// Additional documentation and examples from a Google Chrome developer (Brett Wilson): http://maxradi.us/documents/uniscribe/.
// Pre-Vancouver, we had a separate arabic.ma app that registered as a codeset handler to get into _LayoutGlyphs and re-order and shape the RSC glyphs. I think it was a travesty that users had to run a separate app to get Arabic and Hebrew support, so attempting to roll that into core. Instead of wholesale bringing the app into core, I'd like to use as much Uniscribe as possible; it's updated and fixed over the years by Microsoft, and will act the same as TrueType layout. Unfortunately, Uniscribe requires a font to do shaping (e.g. ligatures and diacritics) because it simply uses the CMap table on the OpenType font. RSC fonts do not have CMaps, nor can we use an arbitrary one (a typical CMap will have many more mappings than basic ligatures), so let Uniscribe itemize our string and re-order the logical runs into visual order, but use our old ligature table for shaping.
// @bsimethod                                                   Jeff.Marker     05/2010
//---------------------------------------------------------------------------------------
static BentleyStatus orderAndShapeGlyphs
(
uint16_t const *                      localeCharacters,
WCharCP                             unicodeCharacters,
size_t                              totalNumChars,
IRscGlyphShaper*                    shaper,
DgnGlyphLayoutResult::T_GlyphCodesR outGlyphCodes,
bvector<size_t>&                    visualToLogicalCharMapping,
bvector<bool>&                      logicalRtlMask
)
    {
    //...............................................................................................................................................
    // We only need to do any work if it's a complex string (unlike TrueType where we'd still need to compute advance widths), so bail out if possible.
    if (S_OK != ::ScriptIsComplex (unicodeCharacters, (int)totalNumChars, SIC_COMPLEX))
        {
        for (size_t iFontChar = 0; iFontChar < totalNumChars; ++iFontChar)
            {
            outGlyphCodes.push_back (localeCharacters[iFontChar]);
            visualToLogicalCharMapping.push_back (iFontChar);
            logicalRtlMask.push_back (false);
            }

        return SUCCESS;
        }
    
    UniscribeServices::T_ScriptItemVector   scriptItems     (UniscribeServices::GetRecommendedScripItemVectorSize ());
    UniscribeServices::T_OpenTypeTagVector  scriptTags      (UniscribeServices::GetRecommendedScripItemVectorSize());
    size_t                                  numScriptItems  = 0;
    
    if (SUCCESS != UniscribeServices::ItemizeString(unicodeCharacters, totalNumChars, scriptItems, scriptTags, numScriptItems))
        return ERROR;

    //...............................................................................................................................................
    // Determine the visual ordering of items so that we display the entire run correctly.
    
    ScopedArray<BYTE> bidiLevels (numScriptItems);
    for (size_t iScriptItem = 0; iScriptItem < numScriptItems; ++iScriptItem)
        {
        bidiLevels.GetData ()[iScriptItem] = scriptItems[iScriptItem].a.s.uBidiLevel;

        for (size_t iChar = 0; iChar < (size_t)(scriptItems[iScriptItem + 1].iCharPos - scriptItems[iScriptItem].iCharPos); ++iChar)
            logicalRtlMask.push_back (scriptItems[iScriptItem].a.fRTL);
        }
    
    ScopedArray<int> visualToLogicalMapping (numScriptItems);
    ScopedArray<int> logicalToVisualMapping (numScriptItems);

    if (0 != ::ScriptLayout ((int)numScriptItems, bidiLevels.GetData (), visualToLogicalMapping.GetData (), logicalToVisualMapping.GetData ()))
        { BeAssert (false); return ERROR; }
    
    for (size_t iVisualItem = 0; iVisualItem < numScriptItems; ++iVisualItem)
        {
        //...............................................................................................................................................
        // For-each item, perform manual shaping and insert glyph codes.

        int             iScriptItem                 = visualToLogicalMapping.GetData ()[iVisualItem];
        size_t          currScriptItemGlyphStart    = outGlyphCodes.size ();
        bvector<size_t> itemCharMapping;
        
        // ::ScriptItemize will always put a trailing fake SCRIPT_ITEM into the array, pointing one past the last real item,
        //  allowing you to always look one past numScriptItems to compute number of characters.
        size_t numCharsInScriptItem = (scriptItems[iScriptItem + 1].iCharPos - scriptItems[iScriptItem].iCharPos);
        
        for (size_t iScriptItemCharOffset = 0; iScriptItemCharOffset < numCharsInScriptItem; ++iScriptItemCharOffset)
            {
            size_t  iCharIndex  = (scriptItems[iScriptItem].iCharPos + iScriptItemCharOffset);
            uint16_t glyphCode   = localeCharacters[iCharIndex];
            
            if ((NULL != shaper) && !shaper->_IsValidLogicalGlyphCode (glyphCode))
                continue;
            
            itemCharMapping.push_back (outGlyphCodes.size ());
            outGlyphCodes.push_back (glyphCode);
            }
        
        if (NULL != shaper)
            shaper->_ShapeGlyphs (&outGlyphCodes[currScriptItemGlyphStart], numCharsInScriptItem);

        if (scriptItems[iScriptItem].a.fRTL)
            {
            reverse (outGlyphCodes.begin () + currScriptItemGlyphStart, outGlyphCodes.end ());
            reverse (itemCharMapping.begin (), itemCharMapping.end ());
            }

        visualToLogicalCharMapping.insert (visualToLogicalCharMapping.end (), itemCharMapping.begin (), itemCharMapping.end ());
        }

    return SUCCESS;
    }

#endif // defined (BENTLEY_WIN32)

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
LangCodePage    DgnRscFont::_GetCodePage                () const    { return m_codePage; }
FontChar        DgnRscFont::GetDefaultCharCode          () const    { if (m_isMissing) { return 0; } EnsureFontIsLoaded (); return m_defaultCharCode; }
FontChar        DgnRscFont::_GetDegreeCharCode          () const    { return m_degreeCharCode; }
double          DgnRscFont::_GetDescenderRatio          () const    { if (m_isMissing) { return 0.0; } EnsureFontIsLoaded (); return m_descenderRatio; }
FontChar        DgnRscFont::_GetDiameterCharCode        () const    { return m_diameterCharCode; }
FontChar        DgnRscFont::_GetPlusMinusCharCode       () const    { return m_plusMinusCharCode; }
DgnFontType     DgnRscFont::_GetType                    () const    { return DgnFontType::Rsc; }
uint32_t        DgnRscFont::GetV8FontNumber             () const    { return m_v8FontNumber; }
DgnFontVariant  DgnRscFont::_GetVariant                 () const    { return DGNFONTVARIANT_Rsc; }
bool            DgnRscFont::_CanDrawWithLineWeight   () const    { return true; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
DgnRscFont::DgnRscFont (Utf8CP name, DgnFontConfigurationData const& config, IRscDataAccessor* accessor, uint32_t v8FontNumber, bool isLastResort) :
    T_Super (name, config),
    m_data                      (accessor),
    m_codePage                  (config.m_codePage),
    m_v8FontNumber              (v8FontNumber),
    m_degreeCharCode            (config.m_degreeCharCode),
    m_diameterCharCode          (config.m_diameterCharCode),
    m_plusMinusCharCode         (config.m_plusMinusCharCode),
    m_wasLoaded                 (false),
    m_defaultCharCode           (0),
    m_descenderRatio            (1.0)
    {
    m_isLastResort = isLastResort;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
DgnRscFont::DgnRscFont (Utf8CP name, DgnFontConfigurationData const& config, uint32_t fontNumber, DgnDbCR project) :
    T_Super (name, config),
    m_data                      (new DbRscDataReader (fontNumber, project)),
    m_codePage                  (config.m_codePage),
    m_v8FontNumber              ((uint32_t)-1),
    m_degreeCharCode            (config.m_degreeCharCode),
    m_diameterCharCode          (config.m_diameterCharCode),
    m_plusMinusCharCode         (config.m_plusMinusCharCode),
    m_wasLoaded                 (false),
    m_defaultCharCode           (0),
    m_descenderRatio            (1.0)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnRscFont::~DgnRscFont ()
    {
    for (T_GlyphMap::const_iterator glyphIter = m_glyphMap.begin (); m_glyphMap.end () != glyphIter; ++glyphIter)
        delete glyphIter->second;
    
    delete m_data;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
void DgnRscFont::AcquireSystemFonts (T_FontCatalogMap& systemFonts)
    {
    if (s_fontFinder != NULL)
        s_fontFinder->AcquireSystemFonts (systemFonts);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
void DgnRscFont::CompressFractions (WStringR outStr, WCharCP inStr) const
    {
    if (WString::IsNullOrEmpty (inStr))
        {
        outStr.clear ();
        return;
        }

    EnsureFontIsLoaded ();
    if (m_fractionMap.empty ())
        {
        outStr = inStr;
        return;
        }
    
    size_t              lenOfcompressedStr  = wcslen (inStr);
    ScopedArray<WChar>  compressedStrBuff   (lenOfcompressedStr + 1);
    WCharP              compressedStr       = compressedStrBuff.GetData ();
    uint32_t            numerator;
    uint32_t            denominator;
    size_t              numCharsInFraction;
    
    BeStringUtilities::Wcsncpy (compressedStr, (lenOfcompressedStr + 1), inStr);

    for (WCharP currCharP = compressedStr; ((size_t)(currCharP - compressedStr) < lenOfcompressedStr); ++currCharP)
        {
        if (!detectFraction (numerator, denominator, numCharsInFraction, compressedStr, currCharP))
            continue;
        
        // Parsing supports Int32, whereas RSC fractions are limited to Byte.
        if ((numerator > 0xff) || (denominator > 0xff))
            continue;

        FontChar fractionCharCode = FractionToFontChar (static_cast <uint8_t> (numerator), static_cast <uint8_t> (denominator));
        if (0 == fractionCharCode)
            continue;
        
        *currCharP = RemapFontCharToUnicodeChar (fractionCharCode);
        
        BeStringUtilities::Wmemmove (
            (currCharP + 1),
            (lenOfcompressedStr - (size_t)((currCharP + 1) - compressedStr) + 1),
            (currCharP + numCharsInFraction),
            (((compressedStr + lenOfcompressedStr) - (currCharP + numCharsInFraction)) + 1));

        lenOfcompressedStr -= (numCharsInFraction - 1);
        }
    
    outStr.assign (compressedStr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnFontPtr DgnRscFont::CreateFromEmbeddedFont (Utf8CP name, uint32_t fontNumber, DgnDbCR project)
    {
    return new DgnRscFont(name, GetEffectiveFontConfig (DgnFontKey (DgnFontType::Rsc, name)), fontNumber, project);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnFontPtr DgnRscFont::CreateLastResortFont ()
    {
    if (s_fontFinder != NULL)
        return s_fontFinder->CreateLastResortFont ();
    
    // Still waiting to discuss with Sam why this is here, and why s_fontFinder can be NULL.
    // The system is not designed to not have a real last resort font.
    static Utf8CP FAKE_LAST_RESORT = "Fake Last Resort";
    
    DgnRscFontP font = DgnRscFont::Create(FAKE_LAST_RESORT, DgnRscFont::GetEffectiveFontConfig(DgnFontKey(DgnFontType::Rsc, FAKE_LAST_RESORT)), NULL, (uint32_t)-1, true);
    font->m_isMissing = true;

    return font;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnFontPtr DgnRscFont::CreateMissingFont(Utf8CP name, uint32_t v8FontNumber)
    {
    DgnRscFontP font = new DgnRscFont(name, DgnRscFont::GetEffectiveFontConfig(DgnFontKey(DgnFontType::Rsc, Utf8String(name))), NULL, v8FontNumber, false);
    font->m_isMissing = true;
    
    return font;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
bool DgnRscFont::_ContainsCharacter (WChar uniChar) const
    {
    return (m_glyphMap.end () != m_glyphMap.find (RemapUnicodeCharToFontChar (uniChar)));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnFontPtr DgnRscFont::_Embed (DgnDbR project) const
    {
    // Can't embed missing fonts; don't bother embedding last resort fonts.
    if (IsLastResort() || IsMissing())
        return NULL;
    
    T_FontCatalogMap::const_iterator foundFont = project.Fonts().EmbeddedFonts().find (DgnFontKey (DgnFontType::Rsc, m_name));
    if (project.Fonts().EmbeddedFonts ().end () != foundFont)
        return foundFont->second;
    
    uint32_t fontNumber;
    if (SUCCESS != project.Fonts().AcquireFontNumber (fontNumber, *this))
        { BeAssert (false); return NULL; }
    
    if (SUCCESS != m_data->_Embed (project, fontNumber))
        { BeAssert (false); return NULL; }

    return DgnRscFont::CreateFromEmbeddedFont (m_name.c_str (), fontNumber, project);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
void DgnRscFont::EnsureFontIsLoaded () const
    {
    if (m_wasLoaded)
        return;

    m_wasLoaded = true;
    
    if (m_isMissing)
        { BeAssert (false && L"Don't attempt to load a missing font!"); return; }

    bvector<Byte> fontHeaderBuffer;
    if (SUCCESS != m_data->_ReadFontHeader (fontHeaderBuffer))
        {
        BeAssert (false);
        m_isMissing = true;
        return;
        }

    RscFontHeader& fontHeader = *reinterpret_cast<RscFontHeader*>(&fontHeaderBuffer[0]);
    
    m_defaultCharCode   = fontHeader.defaultChar;
    
    RscFontVec  ascender    = fontHeader.ascender;
    RscFontVec  descender   = fontHeader.descender;
    
    if (0 == descender)
        descender = fontHeader.maxbrr.bottom;
    
    m_descenderRatio = fabs (descender / (double)ascender);

    if ((0 != fontHeader.fractMap) && (SUCCESS != m_data->_ReadFractionMap (m_fractionMap)))
        BeAssert (false);
    
    bvector<RscGlyphHeader> glyphHeaders;
    if (SUCCESS != m_data->_ReadGlyphHeaders (glyphHeaders))
        {
        BeAssert (false);
        m_isMissing = true;
        return;
        }
    
    bvector<RscGlyphDataOffset> glyphOffsets;
    if (SUCCESS != m_data->_ReadGlyphDataOffsets (glyphOffsets))
        {
        BeAssert (false);
        m_isMissing = true;
        return;
        }
    
    for (size_t iGlyph = 0; iGlyph < fontHeader.totalChars; ++iGlyph)
        m_glyphMap[glyphHeaders[iGlyph].code] = new DgnRscGlyph (*m_data, (double)fontHeader.ascender, fontHeader.m_filledFlag, glyphHeaders[iGlyph], glyphOffsets[iGlyph]);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
void DgnRscFont::ExpandFractions (WStringR outStr, WCharCP inStr) const
    {
    if (WString::IsNullOrEmpty (inStr))
        {
        outStr.clear ();
        return;
        }

    EnsureFontIsLoaded ();
    if (m_fractionMap.empty ())
        {
        outStr = inStr;
        return;
        }
    
    uint8_t numerator;
    uint8_t denominator;

    outStr.clear ();

    for (WCharCP currCharP = inStr; (0 != *currCharP); ++currCharP)
        {
        if (SUCCESS != FontCharToFraction (numerator, denominator, RemapUnicodeCharToFontChar (*currCharP)))
            {
            outStr.push_back (*currCharP);
            }
        else
            {
            WChar buf[128];
            BeStringUtilities::Snwprintf (buf, L"%d/%d", numerator, denominator);
            outStr.append (buf);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
BentleyStatus DgnRscFont::FontCharToFraction (uint8_t& numerator, uint8_t& denominator, FontChar charCode) const
    {
    numerator   = 0;
    denominator = 0;

    if (m_isMissing)
        return ERROR;
    
    if (0 == charCode)
        return ERROR;

    EnsureFontIsLoaded ();
    if (m_fractionMap.empty ())
        return ERROR;

    for (T_FractionMap::const_iterator fractionIter = m_fractionMap.begin (); m_fractionMap.end () != fractionIter; ++fractionIter)
        {
        if (fractionIter->first == charCode)
            {
            numerator   = fractionIter->second.first;
            denominator = fractionIter->second.second;

            return SUCCESS;
            }
        }

    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
FontChar DgnRscFont::FractionToFontChar (uint8_t numerator, uint8_t denominator, bool reduce) const
    {
    if (m_isMissing)
        return 0;
    
    if ((0 == numerator) || (0 == denominator))
        return 0;

    EnsureFontIsLoaded ();
    if (m_fractionMap.empty ())
        return ERROR;

    if (reduce)
        {
        uint8_t gcd   = computeGreatestCommonDenominator (numerator, denominator);
        numerator   /= gcd;
        denominator /= gcd;
        }

    for (T_FractionMap::const_iterator fractionIter = m_fractionMap.begin (); m_fractionMap.end () != fractionIter; ++fractionIter)
        {
        if ((fractionIter->second.first == numerator) && (fractionIter->second.second == denominator))
            return fractionIter->first;
        }

    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnRscGlyphCP DgnRscFont::FindGlyphCP (FontChar charCode) const
    {
    T_GlyphMap::const_iterator foundGlyph = m_glyphMap.find (charCode);
    if (m_glyphMap.end () == foundGlyph)
        return NULL;
    
    return foundGlyph->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
bool DgnRscFont::_IsGlyphBlank (FontChar charCode) const
    {
    T_GlyphMap::const_iterator foundGlyph = m_glyphMap.find (charCode);
    if (m_glyphMap.end () == foundGlyph)
        return true;
    
    return foundGlyph->second->IsBlank ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2009
//---------------------------------------------------------------------------------------
BentleyStatus DgnRscFont::_LayoutGlyphs (DgnGlyphLayoutContextCR layoutContext, DgnGlyphLayoutResultR layoutResult) const
    {
    if (m_isMissing)
        return ERROR;

    // If you make any changes to this method, also consider examining DgnTrueTypeFont::_LayoutGlpyhs and DgnShxFont::_LayoutGlyphs.
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

    EnsureFontIsLoaded();

    // Expand any escape sequences to get the display string.
    ScopedArray<FontChar> processedCharsBuff(fontChars.size() + 1);
    FontCharP processedChars = processedCharsBuff.GetData();
    size_t numProcessedChars = 0;

    for (CharIter iter(*this, &fontChars.front(), fontChars.size(), layoutContext.ShouldIgnorePercentEscapes()); iter.IsValid(); iter.ToNext(), ++numProcessedChars)
        processedChars[numProcessedChars] = iter.CurrCharCode();

    if (0 == numProcessedChars)
        return SUCCESS;

    processedChars[numProcessedChars] = 0;

    bvector<size_t> visualToLogicalCharMapping;
    bvector<bool> logicalRtlMask;

    // Guess how many... helps reduce malloc/free overhead.
    outGlyphCodes.reserve(numProcessedChars);
    visualToLogicalCharMapping.reserve(numProcessedChars);
    logicalRtlMask.reserve(numProcessedChars);

#if defined (BENTLEY_WIN32) // WIP_NONPORT DgnRscFont layout

    if (DgnFontManager::IsGlyphShapingDisabled())
        {
        for (size_t iFontChar = 0; iFontChar < numProcessedChars; ++iFontChar)
            {
            outGlyphCodes.push_back(processedChars[iFontChar]);
            visualToLogicalCharMapping.push_back(iFontChar);
            logicalRtlMask.push_back(false);
            }
        }
    else
        {
        // Get the glyphs in left-to-right display order (e.g. reverse and re-order for RTL), and shape for ligatures etc.
        if (SUCCESS != orderAndShapeGlyphs(processedChars, FontCharsToUnicodeString(processedChars).c_str(), numProcessedChars, RscGlyphShaperManager::GetInstance()->GetShaperForCodePage(m_codePage), outGlyphCodes, visualToLogicalCharMapping, logicalRtlMask))
            return ERROR;
        }

#else

    // This will only suffice for basic English text.
    for (size_t iFontChar = 0; iFontChar < numProcessedChars; ++iFontChar)
        outGlyphCodes.push_back(processedChars[iFontChar]);

#endif    

    if (0 == outGlyphCodes.size())
        return SUCCESS;

    DgnRscGlyph blankGlyph(' ');
    DgnGlyphCP defaultGlyph = FindGlyphCP(GetDefaultCharCode());

    if (NULL == defaultGlyph)
        defaultGlyph = &blankGlyph;

    // We know how many; might as well reserve.
    outGlyphOrigins.reserve(outGlyphCodes.size());

    // We need the glyphs for two loops below, so cache them up first.
    outGlyphs.reserve(outGlyphCodes.size());
    for (size_t iGlyphCode = 0; iGlyphCode < outGlyphCodes.size(); ++iGlyphCode)
        {
        DgnGlyphCP glyph = FindGlyphCP(outGlyphCodes[iGlyphCode]);
        if (NULL == glyph)
            {
            glyph = ((' ' == outGlyphCodes[iGlyphCode]) ? &blankGlyph : defaultGlyph);
            outGlyphCodes[iGlyphCode] = (GlyphCode)glyph->GetCharCode();
            }

        outGlyphs.push_back(glyph);
        }

    // Right-justified text needs to ignore trailing blanks (justification black box).
    size_t numNonBlankGlyphs = outGlyphCodes.size ();
    for (; numNonBlankGlyphs > 0; --numNonBlankGlyphs)
        {
        DgnGlyphCR glyph = *outGlyphs[numNonBlankGlyphs - 1];
        if (!glyph.IsBlank ())
            break;
        }
    
    // Compute origins, ranges, etc...
    DPoint3d penPosition = DPoint3d::FromZero();
    DPoint2d drawSize = layoutContext.GetSize();

    for (size_t iGlyph = 0; iGlyph < outGlyphs.size(); ++iGlyph)
        {
        DgnRscGlyph const& glyph = static_cast<DgnRscGlyph const&>(*outGlyphs[iGlyph]);

        outGlyphOrigins.push_back(penPosition);

        DRange2d glyphRange;
        glyphRange.low.x = outGlyphOrigins[iGlyph].x;
        glyphRange.low.y = outGlyphOrigins[iGlyph].y;
        glyphRange.high.x = glyphRange.low.x + drawSize.x * glyph.GetCellBoxWidth();
        glyphRange.high.y = glyphRange.low.y + drawSize.y;

        // It is important to use the array overload of DRange2d::Extend; the ranges can be inverted for backwards/upside-down text, and the DRange2d overload enforces low/high semantics.
        outRange.Extend(&glyphRange.low, 2);

        if (iGlyph < numNonBlankGlyphs)
            outJustificationRange.Extend(&glyphRange.low, 2);
        
        penPosition.x += (glyph.GetCellBoxRight() * drawSize.x);

        // For Rsc, the width of the space character is equal to the width of the previous character.
        blankGlyph.SetWidth(glyph.GetBlackBoxRight(), glyph.GetCellBoxRight());
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
void DgnRscFont::OnHostTermination ()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
WChar DgnRscFont::_RemapFontCharToUnicodeChar (FontChar charCode) const
    {
    // The only reason we need to load a font for character conversion is dealing with custom RSC fraction glyphs.
    // If a font is missing, it is an error to try and load it.
    // In this case, we can't look up the custom fractions anyway, so don't try to load, and let the fraction lookup below fail and forward to the parent class.
    if (!IsMissing())
        EnsureFontIsLoaded ();
    
    // Put RSC fractions in the private use area (U+E000..U+F8FF), so that they don't interfere with normal characters, such as U+00A0, a non-breaking space, a.k.a. 1/64.
    //  Unicode has limited support for fractions... so might as well map those when possible.
    //  Further, while it is possible that an RSC font can define its own fraction ranges, I have never seen one that does this, and it would greatly complicate code to have to deal with custom (potentially disjoint) ranges.
    if (!m_fractionMap.empty ())
        {
        uint8_t numerator;
        uint8_t denominator;
        
        if (SUCCESS == FontCharToFraction (numerator, denominator, charCode))
            {
            // These are the fractions available in Unicode.
            if ((1 == numerator) && (4 == denominator))     return SPECIALCHAR_UnicodeFraction_1_4;
            if ((1 == numerator) && (2 == denominator))     return SPECIALCHAR_UnicodeFraction_1_2;
            if ((3 == numerator) && (4 == denominator))     return SPECIALCHAR_UnicodeFraction_3_4;
            if ((1 == numerator) && (7 == denominator))     return SPECIALCHAR_UnicodeFraction_1_7;
            if ((1 == numerator) && (9 == denominator))     return SPECIALCHAR_UnicodeFraction_1_9;
            if ((1 == numerator) && (10 == denominator))    return SPECIALCHAR_UnicodeFraction_1_10;
            if ((1 == numerator) && (3 == denominator))     return SPECIALCHAR_UnicodeFraction_1_3;
            if ((2 == numerator) && (3 == denominator))     return SPECIALCHAR_UnicodeFraction_2_3;
            if ((1 == numerator) && (5 == denominator))     return SPECIALCHAR_UnicodeFraction_1_5;
            if ((2 == numerator) && (5 == denominator))     return SPECIALCHAR_UnicodeFraction_2_5;
            if ((3 == numerator) && (5 == denominator))     return SPECIALCHAR_UnicodeFraction_3_5;
            if ((4 == numerator) && (5 == denominator))     return SPECIALCHAR_UnicodeFraction_4_5;
            if ((1 == numerator) && (6 == denominator))     return SPECIALCHAR_UnicodeFraction_1_6;
            if ((5 == numerator) && (6 == denominator))     return SPECIALCHAR_UnicodeFraction_5_6;
            if ((1 == numerator) && (8 == denominator))     return SPECIALCHAR_UnicodeFraction_1_8;
            if ((3 == numerator) && (8 == denominator))     return SPECIALCHAR_UnicodeFraction_3_8;
            if ((5 == numerator) && (8 == denominator))     return SPECIALCHAR_UnicodeFraction_5_8;
            if ((7 == numerator) && (8 == denominator))     return SPECIALCHAR_UnicodeFraction_7_8;

            // Shift all others into the private use area.
            return (SPECIALCHAR_PrivateUse_FirstRscFraction + (charCode - SPECIALCHAR_FirstStandardRscFraction));
            }
        }

    return T_Super::_RemapFontCharToUnicodeChar (charCode);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
FontChar DgnRscFont::_RemapUnicodeCharToFontChar (WChar uniChar) const
    {
    // The only reason we need to load a font for character conversion is dealing with custom RSC fraction glyphs.
    // If a font is missing, it is an error to try and load it.
    // In this case, we can't look up the custom fractions anyway, so don't try to load, and let the fraction lookup below fail and forward to the parent class.
    if (!IsMissing())
        EnsureFontIsLoaded ();
    
    // See comments in _RemapFontCharToUnicodeChar about remapping RSC fractions.
    if (!m_fractionMap.empty ())
        {
        if ((uniChar >= SPECIALCHAR_PrivateUse_FirstRscFraction) && (uniChar <= SPECIALCHAR_PrivateUse_LastRscFraction))
            return (SPECIALCHAR_FirstStandardRscFraction + (uniChar - SPECIALCHAR_PrivateUse_FirstRscFraction));

        FontChar fontChar = 0;

        if ((SPECIALCHAR_UnicodeFraction_1_4 == uniChar)    && (0 != (fontChar = FractionToFontChar (1, 4, false))))  return fontChar;
        if ((SPECIALCHAR_UnicodeFraction_1_2 == uniChar)    && (0 != (fontChar = FractionToFontChar (1, 2, false))))  return fontChar;
        if ((SPECIALCHAR_UnicodeFraction_3_4 == uniChar)    && (0 != (fontChar = FractionToFontChar (3, 4, false))))  return fontChar;
        if ((SPECIALCHAR_UnicodeFraction_1_7 == uniChar)    && (0 != (fontChar = FractionToFontChar (1, 7, false))))  return fontChar;
        if ((SPECIALCHAR_UnicodeFraction_1_9 == uniChar)    && (0 != (fontChar = FractionToFontChar (1, 9, false))))  return fontChar;
        if ((SPECIALCHAR_UnicodeFraction_1_10 == uniChar)   && (0 != (fontChar = FractionToFontChar (1, 10, false)))) return fontChar;
        if ((SPECIALCHAR_UnicodeFraction_1_3 == uniChar)    && (0 != (fontChar = FractionToFontChar (1, 3, false))))  return fontChar;
        if ((SPECIALCHAR_UnicodeFraction_2_3 == uniChar)    && (0 != (fontChar = FractionToFontChar (2, 3, false))))  return fontChar;
        if ((SPECIALCHAR_UnicodeFraction_1_5 == uniChar)    && (0 != (fontChar = FractionToFontChar (1, 5, false))))  return fontChar;
        if ((SPECIALCHAR_UnicodeFraction_2_5 == uniChar)    && (0 != (fontChar = FractionToFontChar (2, 5, false))))  return fontChar;
        if ((SPECIALCHAR_UnicodeFraction_3_5 == uniChar)    && (0 != (fontChar = FractionToFontChar (3, 5, false))))  return fontChar;
        if ((SPECIALCHAR_UnicodeFraction_4_5 == uniChar)    && (0 != (fontChar = FractionToFontChar (4, 5, false))))  return fontChar;
        if ((SPECIALCHAR_UnicodeFraction_1_6 == uniChar)    && (0 != (fontChar = FractionToFontChar (1, 6, false))))  return fontChar;
        if ((SPECIALCHAR_UnicodeFraction_5_6 == uniChar)    && (0 != (fontChar = FractionToFontChar (5, 6, false))))  return fontChar;
        if ((SPECIALCHAR_UnicodeFraction_1_8 == uniChar)    && (0 != (fontChar = FractionToFontChar (1, 8, false))))  return fontChar;
        if ((SPECIALCHAR_UnicodeFraction_3_8 == uniChar)    && (0 != (fontChar = FractionToFontChar (3, 8, false))))  return fontChar;
        if ((SPECIALCHAR_UnicodeFraction_5_8 == uniChar)    && (0 != (fontChar = FractionToFontChar (5, 8, false))))  return fontChar;
        if ((SPECIALCHAR_UnicodeFraction_7_8 == uniChar)    && (0 != (fontChar = FractionToFontChar (7, 8, false))))  return fontChar;
        }
    
    return T_Super::_RemapUnicodeCharToFontChar (uniChar);
    }
    
