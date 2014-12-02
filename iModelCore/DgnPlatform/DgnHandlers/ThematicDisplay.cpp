/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/ThematicDisplay.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#include    <algorithm>

#define COMPARE_VALUES(val0, val1, tol) if (val0 < val1 - tol) return true; if (val0 > val1 + tol) return false;
#define ONE_INDEX(i)                    (abs (i) - 1)

DGNPLATFORM_TYPEDEFS (ThematicDisplayStyleHandlerKey)
DGNPLATFORM_TYPEDEFS (ThematicMeshValueDisplayStyleHandler)

       
typedef bvector <bool>                          BoolArray;
typedef bvector <Int32>                         IndexArray;

static double       s_margin = .25, s_colorRange = .5;


#ifdef NEEDS_WORK_LEGEND
/*=================================================================================**//**
* @bsiclass                                                     RayBentley      11/2010
+===============+===============+===============+===============+===============+======*/
struct ThematicLegendElementHandler : DisplayHandler
{
    DEFINE_T_SUPER(DisplayHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS (ThematicLegendElementHandler, DGNPLATFORM_EXPORT)


    virtual void                              _GetTypeName (WStringR string, UInt32 desiredLength) override;
    virtual void                              _Draw (ElementHandleCR el, ViewContextR context) override;
    virtual StatusInt                         _OnTransform (EditElementHandleR, TransformInfoCR) override;
    virtual void                              _GetTransformOrigin (ElementHandleCR eh, DPoint3dR origin) override { origin = GetData (eh)->m_origin; }
    virtual void                              _GetOrientation (ElementHandleCR eh, RotMatrixR rotation) override;
    virtual bool                              _IsPlanar (ElementHandleCR el, DVec3dP normal, DPoint3dP point, DVec3dCP inputDefaultNormal) override;

    static ThematicLegendElementData const*   GetData (ElementHandleCR eh);
    static StatusInt                          SetData (EditElementHandleR eeh, ThematicLegendElementData const& data);

};  // ThematicLegendElementHandler


ELEMENTHANDLER_DEFINE_MEMBERS(ThematicLegendElementHandler)

HandlerR ThematicLegend::GetElementHandler() { return ELEMENTHANDLER_INSTANCE (ThematicLegendElementHandler); }
#endif

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2011
+===============+===============+===============+===============+===============+======*/
struct  TempFileList : DgnModelAppData
{
    bvector <WString>       m_tempFileNames;

    void    AddTempFile (WString& fileName)             { m_tempFileNames.push_back (fileName); }
    virtual void _OnCleanup (DgnModelR host) override   { delete this; }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
~TempFileList ()
    {
    for (size_t i=0; i<m_tempFileNames.size(); i++)
        BeFileName::BeDeleteFile (m_tempFileNames[i].c_str());
    }

};  // TempFileList

#ifdef UNUSED_FUNCTION
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static void    cleanUpTempFiles (DgnModelP modelRef, int userDataKey, void *userData)
    {
    delete ((TempFileList*) userData);
    }
#endif


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void    addDgnModelTempFile (DgnModelR modelRef, WString fileName)
    {
    TempFileList*           tempFileList;
    DgnModelAppData::Key    s_tempFileKey;

    if (NULL == (tempFileList = dynamic_cast <TempFileList*> (modelRef.FindAppData (s_tempFileKey))))
        modelRef.AddAppData (s_tempFileKey,  tempFileList = new TempFileList());

    tempFileList->AddTempFile (fileName);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Int32          maxIndex (Int32 const* indices, size_t nIndices)
    {
    Int32       max = 0;
    for (size_t i=0; i<nIndices; i++)
        {
        Int32   absIndex = abs (indices[i]);

        if (absIndex > max)
            max  = absIndex;
        }
        
    return max;
    }
         
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool    doThematicForDrawPurpose (DrawPurpose drawPurpose, ThematicDisplaySettingsCR settings)
    {
    switch (drawPurpose)
        {
        default:
            return true;

        case DrawPurpose::FitView:
        case DrawPurpose::ExportVisibleEdges:
            return false;

        case DrawPurpose::Pick:
            return ThematicSteppedDisplay_Isolines == settings.GetFlags().m_steppedDisplay;
        }

    // Unreachable code
    // return true;
    }



#if WIP_HITINFO
/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2010
+===============+===============+===============+===============+===============+======*/
struct ThematicDisplayHitInfo : Bentley::Ustn::DisplayStyleHitInfo
{
    ThematicDisplayStyleHandlerCR       m_handler;
    ThematicDisplaySettingsCR           m_settings;
    DPoint3d                            m_hitPoint;

                                        ThematicDisplayHitInfo (ThematicDisplayStyleHandlerCR handler, ThematicDisplaySettingsCR settings, DPoint3dCR hitPoint) : m_handler (handler), m_settings (settings), m_hitPoint (hitPoint) { }
    virtual IViewHandlerHitInfo*        Clone() const { return new ThematicDisplayHitInfo (*this); }
    MSCORE_EXPORT virtual void          OnGetInfoString (WStringR pathDescr, DisplayPath const&, MSWCharCP delimiter) const;

};  // ThematicDisplayHitInfo 

#endif


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ThematicColorSchemeProvider::ThematicColorSchemeProvider (ThematicColorScheme icse)   { m_icse = icse;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void    ThematicColorSchemeProvider::KeysToArrays (size_t nKeys, const ThematicGradientKey* keys, double* values, RgbFactor* colors)
    {
    for(size_t i=0; i<nKeys;i++)
        {
        values[i]       = keys[i].value;
        colors[i].red   = static_cast <double> (keys[i].red)   / 255.0;
        colors[i].green = static_cast <double> (keys[i].green) / 255.0;
        colors[i].blue  = static_cast <double> (keys[i].blue)  / 255.0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void    ThematicColorSchemeProvider::ArraysToKeys (size_t nKeys, const double* values, const RgbFactor* colors, ThematicGradientKey* keys)
    {
    for(size_t i=0; i<nKeys;i++)
        {
        keys[i].value  = values[i];
        keys[i].red    = static_cast <UInt8> (colors[i].red   * 255.0 + 0.5);
        keys[i].green  = static_cast <UInt8> (colors[i].green * 255.0 + 0.5);
        keys[i].blue   = static_cast <UInt8> (colors[i].blue  * 255.0 + 0.5);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   ThematicColorSchemeProvider::GradientKeys (size_t maxKeys, size_t* fetchedKeys, ThematicGradientKey* outKeys)
    {
    static const size_t                 s_nKeys[]   = { 5, 5, 2, 3, 6 };
    //Note: These colors are in RBG format to match the key structure (not RGB like most colors)
    static const ThematicGradientKey    s_keys[ThematicColorScheme_Max][MAX_GRADIENT_KEYS] = 
        {  
           { {0.0, 0,   255, 0}, {0.25, 0,   255, 255}, {0.5, 0, 0, 255}, {0.75, 255, 0,   255}, {1.0, 255, 0,   0}},
           { {0.0, 255, 0,   0}, {0.25, 255, 0,   255}, {0.5, 0, 0, 255}, {0.75, 0,   255, 255}, {1.0, 0,   255, 0}},
           { {0.0, 0,   0,   0}, {1.0,  255, 255, 255}},

           //Based off of the topographic gradients in Point Clouds
           { {0.0, 152, 148, 188}, {0.5, 204, 160, 204}, {1.0, 152, 72, 128}},

           //Based off of the sea-mountain gradient in Point Clouds
           { {0.0, 0, 255, 0}, {0.2, 72, 96, 160}, {0.4, 152, 96, 160}, {0.6, 128, 32, 104}, {0.7, 148, 180, 128}, {1.0, 240, 240, 240}}};
       
    //If we're in custom mode, we should use whatever's in the settings. In practice we generally don't have to do anything. 
    //Return an error and let the caller decide what to do.
    if (m_icse >= ThematicColorScheme_Max)
        return ERROR;

    *fetchedKeys = MIN(s_nKeys[m_icse], MIN (MAX_GRADIENT_KEYS, maxKeys));
    memcpy (outKeys, s_keys[m_icse], sizeof(s_keys[0][0]) * *fetchedKeys);       
        
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   ThematicColorSchemeProvider::GradientArrays (size_t maxKeys, size_t* fetchedKeys, double* outValues, RgbFactor* outColors)
    {
    ThematicGradientKey keys[MAX_GRADIENT_KEYS]; 

    if (SUCCESS != GradientKeys (maxKeys, fetchedKeys, keys))
        return ERROR;

    KeysToArrays (*fetchedKeys, keys, outValues, outColors);
        
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ThematicMeshColorMap::Init (ThematicDisplaySettingsCR settings, size_t size)
    {
    bvector <RgbColorDef>       falseColors (size);
    RgbColorDef                 keyColors[MAX_GRADIENT_KEYS];
    size_t                      fetchedKeys;
    ThematicGradientKey         keys[MAX_GRADIENT_KEYS];
    IndexArray                  intKeyValues (MAX_GRADIENT_KEYS);

    m_colors.resize (size);
    settings.GetGradient (keys, MAX_GRADIENT_KEYS, &fetchedKeys);
        
    for(size_t i=0; i < fetchedKeys; i++)
        {
        Byte red = static_cast<Byte>(keys[i].red);
        Byte green = static_cast<Byte>(keys[i].green);
        Byte blue = static_cast<Byte>(keys[i].blue);
        RgbColorDef color = { red, green, blue };

        intKeyValues[i] = static_cast<Int32> ((size-1) * keys[i].value);
        keyColors[i] = color;
        falseColors [intKeyValues [i]] = color;
        }

    for(size_t i=0; i+1 < fetchedKeys; i++)
        {
        RgbColorDefP  segmentStart = &falseColors [intKeyValues [i]];
        size_t        nColors      = (size_t) 1 + intKeyValues[i+1] - intKeyValues[i];

        ColorUtil::InterpolateColorsRGB (segmentStart, nColors, keyColors[i], keyColors[i+1]);
        }

    for (size_t i = 0; i < size; i++)
        m_colors[i] = falseColors[i].blue << 16 | falseColors[i].green << 8 | falseColors[i].red;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32      ThematicMeshColorMap::Get (double value, double min, double max) const
    {
    if (value < min)
        value = min;
    else if (value > max)
        value = max;

    if (max == min) // Used when there's only one band
        return m_colors[0];

    if (max < min)
        return 0;


    return m_colors[(size_t) ((double) (m_colors.size()-1) * (value - min) / (max - min))];
    }

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      02/2008
+===============+===============+===============+===============+===============+======*/
struct CompareParams    
    {
bool operator () (const DPoint2d& value0, const DPoint2d& value1) const
    {
    static double       s_compareTol = 1.0E-3;

    return value0.y < value1.y - s_compareTol;
    }
};  // CompareParams



typedef std::map <DPoint2d, Int32, CompareParams>  ParamMap;
typedef bvector <DPoint2d>  ScalarMeshParamArray;



/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2011                                                                              
+===============+===============+===============+===============+===============+======*/
struct  IsoTriangle
{
    Int32           m_pointIndices[3];
    Int32           m_valueIndices[3];
    double          m_minimum;
    double          m_maximum;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IsoTriangle (size_t index0, size_t index1, size_t index2, Int32 const* pointIndices, Int32 const* valueIndices, double const* values, double& meshMin, double& meshMax, bool hideFirstEdge, bool hideLastEdge)
    {
    m_pointIndices[0] = pointIndices[index0];
    m_pointIndices[1] = pointIndices[index1];
    m_pointIndices[2] = pointIndices[index2];

    if (hideFirstEdge && m_pointIndices[0] > 0)
        m_pointIndices[0] = -m_pointIndices[0];

    if (hideLastEdge && m_pointIndices[2] > 0)
        m_pointIndices[2] = -m_pointIndices[2];

    m_valueIndices[0] = valueIndices[index0];
    m_valueIndices[1] = valueIndices[index1];
    m_valueIndices[2] = valueIndices[index2];

    m_minimum = m_maximum = values[ONE_INDEX (m_valueIndices[0])];

    for (int i=1; i<3; i++)
        {
        double      value = values[ONE_INDEX (m_valueIndices[i])];
        if (value < m_minimum)
            m_minimum = value;

        if (value > m_maximum)
            m_maximum = value;
        }
    if (m_minimum < meshMin)
        meshMin = m_minimum;

    if (m_maximum > meshMax)
        meshMax = m_maximum;
    }

};  // IsoTriangle

typedef bvector <IsoTriangle>     T_IsoTriangles;

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2010
+===============+===============+===============+===============+===============+======*/
struct ThematicMeshDrawGeom : SimplifyViewDrawGeom, IThematicMeshStroker 
{
    ThematicMeshPointArray              m_points;
    ThematicMeshIndexArray              m_indices;
    ThematicMeshDoubleArray             m_values;
    ThematicMeshIndexArray              m_valueIndices;
    ThematicMeshDisplayStyleHandlerCR   m_handler;
    ThematicDisplaySettingsCR           m_settings;
    ViewContextR                        m_viewContext;
    IViewDrawP                          m_output;
    IFacetOptionsPtr                    m_thematicFacetOptions;
    IThematicMeshStroker*               m_stroker;
    mutable bool                        m_curvedGeometryStroked;
    bool                                m_isOutputQuickVision;

    virtual bool _ProcessAsFacets (bool isPolyface) const override {return true;}
    virtual bool _ProcessAsBody (bool isCurved) const override {m_curvedGeometryStroked |= isCurved; return false;}

    void                        SetStroker (IThematicMeshStroker& stroker) { m_stroker = &stroker; }
    IThematicMeshStroker&       GetStroker ()                              { return NULL == m_stroker ? *this : *m_stroker; }
    bool                        CurvedGeometryStroked ()                   { return m_curvedGeometryStroked; }
    bool                        GetCurrHiliteState ();

    bool   _IsOutputQuickVision () const override                                                           { return m_isOutputQuickVision; }

    virtual void   _PushTransClip (TransformCP trans, ClipPlaneSetCP clip) override                         { if (NULL != m_output) ViewContext::DirectPushTransClipOutput (*m_output, trans, clip); }
    virtual void   _PopTransClip () override                                                                { if (NULL != m_output) ViewContext::DirectPopTransClipOutput (*m_output); }

    virtual void   _DrawTextString (TextStringCR text, double* zDepth) override                             { if (NULL != m_output) m_output->DrawTextString (text, zDepth); }
    virtual void   _DrawLineString3d (int numPoints, DPoint3dCP points, DPoint3dCP range) override          { if (NULL != m_output) m_output->DrawLineString3d (numPoints, points, range); }
    virtual void   _DrawPolyface (PolyfaceQueryCR facets, bool filled) override                             { _ProcessFacetSet (facets, filled); }      // Optimization...

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ThematicMeshDrawGeom (ThematicMeshDisplayStyleHandlerCR handler, ThematicDisplaySettingsCR settings, ViewContextR viewContext, IViewDrawP output) : 
                    m_handler (handler), m_settings (settings), m_viewContext (viewContext), m_output (output), m_stroker (NULL), m_curvedGeometryStroked (false)
    {
    static      double s_toleranceRatio = 1.0 / 1500.0;

    SetViewContext (&viewContext);

    if (NULL != output)
        m_viewFlags = *output->GetDrawViewFlags();
    else if (NULL != viewContext.GetViewport())
        m_viewFlags = *viewContext.GetViewport()->GetViewFlagsP();
    else
        { BeAssert(false); }

    ActivateMatSymb (viewContext.GetElemMatSymb());
    ActivateOverrideMatSymb (viewContext.GetOverrideMatSymb());

    IFacetOptionsP      facetOptions = GetFacetOptions();

    facetOptions->SetMaxPerFace (3);
    facetOptions->SetNormalsRequired (true);
    facetOptions->SetParamsRequired (true); // Are these really needed??

    if (NULL != viewContext.GetViewport())
        facetOptions->SetChordTolerance (viewContext.GetViewport()->GetViewController().GetDelta().magnitudeXY() * s_toleranceRatio);

    m_isOutputQuickVision = (NULL != output ? output->IsOutputQuickVision () : false);
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt  _ProcessFacetSet (PolyfaceQueryCR facets, bool filled) override   
    { 
    if (facets.GetPointIndexCount() <= 2)
        return SUCCESS;

    m_handler.ProcessMesh (GetStroker (), m_viewContext, facets.GetNumPerFace(), facets.GetPointIndexCount(), facets.GetPointIndexCP(), facets.GetNormalIndexCP(), facets.GetPointCount(), facets.GetPointCP(), facets.GetNormalCP(), facets.GetTwoSided(), m_settings);

    return SUCCESS;
    }

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      03/2011                                                                              
+===============+===============+===============+===============+===============+======*/
struct ThematicPointCloudDrawParams : IPointCloudDrawParams
{
    IPointCloudDrawParams&      m_sourceParams;
    bvector<RgbColorDef>        m_thematicColors;

    virtual UInt32              AddRef ()   { return 1; }
    virtual UInt32              Release ()  { return 1; }

                                ThematicPointCloudDrawParams (IPointCloudDrawParams& params) : m_sourceParams (params), m_thematicColors (params.GetNumPoints())  {}
    virtual                     ~ThematicPointCloudDrawParams () { }

    virtual bool                IsThreadBound ()                { return true; }
    virtual bool                GetRange (DPoint3d*range)       { return m_sourceParams.GetRange (range); }

    //  Added to points returned by GetPoints or GetFPoints
    virtual bool                GetOrigin (DPoint3dP origin)     { return m_sourceParams.GetOrigin(origin); }

    virtual RgbColorDef const * GetRgbColors ()                 { return &m_thematicColors[0]; }

    virtual UInt32              GetNumPoints ()                 { return m_sourceParams.GetNumPoints(); }
    virtual DPoint3dCP          GetDPoints ()                   { return m_sourceParams.GetDPoints(); }
    virtual FPoint3dCP          GetFPoints ()                   { return m_sourceParams.GetFPoints(); }
  };  // PointCloudDrawParams
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void   _DrawPointCloud (IPointCloudDrawParams* params) override
    {
    if (NULL == m_output || NULL == params)
        return;

    if (m_handler.SupportsPointColors ())
        {
        DPoint3dCP      points;

        if (NULL != (points = params->GetDPoints()))
            {
            ThematicPointCloudDrawParams thematicParams (*params);
            
            if (SUCCESS == m_handler.GetPointColors (&thematicParams.m_thematicColors[0], points, params->GetNumPoints(), m_settings, m_viewContext))
                {
                m_output->DrawPointCloud (&thematicParams);
                return;
                }
            }
        }

    m_output->DrawPointCloud (params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void  _DrawShape3d (int numPoints, DPoint3dCP points, bool filled, DPoint3dCP range) override
    {
    if (!bsiGeom_testPolygonConvex (points, numPoints))
        {
        SimplifyViewDrawGeom::_DrawShape3d (numPoints, points, filled, range);
        }
    else
        {
        if (points[0].isEqual (&points[numPoints-1]))
            numPoints--;

        if (numPoints >= 3)
            {
            bvector <Int32>      vertIndices (numPoints+1);
            for (int i=0; i<numPoints;i++)
                vertIndices[i] = i+1;
                 
            vertIndices[numPoints] = 0;

            m_handler.ProcessMesh (GetStroker(), m_viewContext, 0, numPoints+1, &vertIndices[0], NULL, numPoints, points, NULL, true, m_settings);
            }
        }
    }
 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void    CompressParams (Int32* indices, size_t nIndices, ScalarMeshParamArray& params)
    {
    ThematicMeshIndexArray    remapped (params.size());
    Int32                   nextIndex = 0;
    CompareParams           comparer;
    ParamMap                map (comparer);

    for (size_t i=0, count = params.size(); i< count; i++)
        {
        ParamMap::iterator  found = map.find (params[i]);

        if (found == map.end())
            {
            map.insert (std::pair <DPoint2d, Int32>  (params[i], remapped[i] = nextIndex++));
            }
        else
            {
            remapped[i] = found->second;
            }
        }
    for (size_t i=0; i<nIndices; i++)
        if (0 != indices[i])
            indices[i] = 1 + remapped[abs(indices[i])-1];

    for (ParamMap::iterator curr = map.begin(), end = map.end(); curr != end; curr++)
        params[curr->second] = curr->first;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void    GetIsoTriangles (T_IsoTriangles& triangles, double& min, double& max, int polySize, size_t nIndices, Int32 const* indices, Int32 const* valueIndices, double const* values)
    {
    size_t      start = 0, nPoints = 0, maxPolySize = (0 == polySize ?  0xffff : polySize);

    for (size_t i=0; i < nIndices; i++)
        {
        if (nPoints >= maxPolySize || 0 == indices[i])
            {
            int         nTriangles = (int) nPoints - 2;

            for (int  j=0; j<nTriangles; j++)
                triangles.push_back (IsoTriangle (start, start + j + 1, start + j + 2, indices, valueIndices, values, min, max, j > 0, j !=  nTriangles-1));

            if (0 == indices[start = i])
                {
                start++;
                nPoints = 0;
                }
            else
                {
                nPoints = 1;
                }
            }
        else
            {
            nPoints++;
            }
        }
    if (nPoints > 0)
        {
        int         nTriangles = (int) nPoints - 2;

        for (int  j=0; j<nTriangles; j++)
            triangles.push_back (IsoTriangle (start, start + j + 1, start + j + 2, indices, valueIndices, values, min, max, j > 0, j !=  nTriangles-1));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer      9/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3dR LerpByInsolation(DPoint3dR result, DPoint3dCP p1, DPoint3dCP p2, double i1, double i2, const double c)
    {
    //The idea is to calculate the linear interpolation of i1 and i2 that would produce c, then apply 
    //that interpolation to p1 and p2. Given the interpolation formula c = i1 + x(i2-i1), we can solve
    //for x = (c-i1)/(i2-i1)
    double x = (i2 == i1) ? 0.0 :(c-i1)/(i2-i1);

    result.sumOf (NULL, p1, (1.0-x), p2, x);

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer      9/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipMax(ThematicMeshPointArray& polyPoints, ThematicMeshDoubleArray& polyIntensities, double max, BoolArray& edgesHidden)
    {
    bool                        thisInside, nextInside = polyIntensities[0] <= max, thisHidden, nextHidden = edgesHidden[0];
    ThematicMeshPointArray      outPoints;
    ThematicMeshDoubleArray     outIntensities;
    BoolArray                   outHidden;

    for(size_t i=0; i<polyPoints.size(); i++)
        {
        size_t iNext = (i == polyPoints.size() - 1) ? 0 : i+1;

        thisInside = nextInside;
        nextInside = polyIntensities[iNext] <= max;
        thisHidden = nextHidden;
        nextHidden = edgesHidden[iNext];

        if(thisInside != nextInside)       
            {                   
            DPoint3d    result;

            outPoints.push_back (LerpByInsolation(result, &polyPoints[i], &polyPoints[iNext], polyIntensities[i], polyIntensities[iNext], max));
            outIntensities.push_back(max);
            outHidden.push_back(nextInside ? thisHidden : false);
           }

        if(nextInside)
            {
            outPoints.push_back(polyPoints[iNext]);
            outIntensities.push_back(polyIntensities[iNext]);
            outHidden.push_back(nextHidden);
            }
        }

    polyPoints = ThematicMeshPointArray(outPoints); 
    polyIntensities = ThematicMeshDoubleArray(outIntensities); 
    edgesHidden = BoolArray(outHidden);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer      9/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipMin(ThematicMeshPointArray& polyPoints, ThematicMeshDoubleArray& polyIntensities, double min, BoolArray& edgesHidden)
    {
    bool                    thisInside, nextInside = polyIntensities[0] > min, thisHidden, nextHidden = edgesHidden[0];
    ThematicMeshPointArray    outPoints;
    ThematicMeshDoubleArray   outIntensities;
    BoolArray               outHidden;

    for(size_t i=0; i<polyPoints.size(); i++)
        {
        size_t iNext = (i == polyPoints.size() - 1) ? 0 : i+1;

        thisInside = nextInside;
        nextInside = polyIntensities[iNext] > min;
        thisHidden = nextHidden;
        nextHidden = edgesHidden[iNext];

        if(thisInside != nextInside)
            {
            DPoint3d        result;

            outPoints.push_back(LerpByInsolation (result, &polyPoints[i], &polyPoints[iNext], polyIntensities[i], polyIntensities[iNext], min));
            outIntensities.push_back(min);
            outHidden.push_back(nextInside ? thisHidden : false);
            }

        if(nextInside)
            {
            outPoints.push_back(polyPoints[iNext]);
            outIntensities.push_back(polyIntensities[iNext]);
            outHidden.push_back(nextHidden);
            }
        }

    polyPoints = ThematicMeshPointArray(outPoints); 
    polyIntensities = ThematicMeshDoubleArray(outIntensities); 
    edgesHidden = BoolArray(outHidden);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void    ComputeTextureParam (DPoint2dR param, double value)
    {
    static double       s_paramEpsilon = .002; 
    static double       s_minMargin = s_margin, s_maxMargin = 1.0 - s_margin;
    static double       s_minLimit = -s_margin + s_paramEpsilon, s_maxLimit = 1.0 + s_minMargin - s_paramEpsilon;
    
    double              textureValue = s_margin + s_colorRange * value; 
            
    if (textureValue < s_minLimit)
        textureValue = s_minLimit;
    else if (textureValue > s_maxLimit)
        textureValue = s_maxLimit;
    else if (fabs (textureValue - s_minMargin) < s_paramEpsilon)
        textureValue += s_paramEpsilon;
    else if (fabs (textureValue - s_maxMargin) < s_paramEpsilon)
        textureValue -= s_paramEpsilon;

    param.x = .5;
    param.y = 1.0 - textureValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void StrokeIsoContourMesh (UInt32 polySize, size_t nIsoIndices, Int32 const* isoIndices, size_t nIsoPoints, DPoint3dCP isoPoints, double value)
    {
    bvector<Int32>              singleValueIndices;

    for (size_t i=0; i<nIsoIndices; i++)
        singleValueIndices.push_back (0 == isoIndices[i] ? 0 : 1);

    if (DrawPurpose::XGraphicsCreate == m_viewContext.GetDrawPurpose())
        {
        // The handling for elemColorByRGB changes during element draw was not present in XGraphics for 8.11.7 so write out color indices for compatibility.
        ThematicMeshIndexArray      isoColorIndices (nIsoIndices, 1);
        UInt32                      color      = (value < 0.0 || value > 1.0) ? m_settings.GetMarginColor() : m_settings.GetColor (value);
        FloatRgb                    colorFloat = {(float) (color & 0xff) / 255.0f,
                                                  (float) ((color >> 8) & 0xff) / 255.0f,
                                                  (float) ((color >> 16) & 0xff) / 255.0f};

        PolyfaceQueryCarrier        polyface (polySize, false, nIsoIndices,             // NumPerFace, twosided, nIndices,
                                              nIsoPoints, isoPoints, isoIndices,        // Points.
                                              0, NULL, NULL,                            // Normals.
                                              0, NULL, NULL,                            // Params.
                                              1,  &singleValueIndices[0], &colorFloat); // Colors

        m_output->DrawPolyface (polyface, true);
        }
    else
        {   
        DPoint2d                param;
        ComputeTextureParam (param, value);

        PolyfaceQueryCarrier        polyface (polySize, false, nIsoIndices,             // NumPerFace, twosided.    
                                              nIsoPoints, isoPoints, isoIndices,        // Points.
                                              0, NULL, NULL,                           // Normals.
                                              1, &param, &singleValueIndices[0]);       // Params.

        m_output->DrawPolyface (polyface, true);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer      9/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClipIsoContourMesh
(
T_IsoTriangles&         triangles,
size_t                  nPoints,
DPoint3dCP              points,
int                     polySize,
size_t                  nIndices,
Int32 const*            indices, 
double                  meshMin, 
double                  meshMax,
double                  contourMin, 
double                  contourMax, 
double const*           values,
Int32 const*            valueIndices,
IPolyfaceConstructionR  builder,
double                  value
)
    {
    static double       s_epsilon = 1.0E-6;
    double              contourMinMinusEpsilon = contourMin - s_epsilon, contourMaxPlusEpsilon = contourMax + s_epsilon;

    if (meshMax < contourMinMinusEpsilon || meshMin > contourMaxPlusEpsilon)                 // Totally outside contour.
        return false;

    if (meshMin > contourMinMinusEpsilon && meshMax <= contourMaxPlusEpsilon)                 // Totally within contour.
        {
        StrokeIsoContourMesh (polySize, nIndices, indices, nPoints, points, value);
        return true;
        }

    BoolArray                           edgesHidden(3);
    ThematicMeshPointArray              polyPoints(3);
    ThematicMeshDoubleArray             polyValues(3);

    for (T_IsoTriangles::iterator curr = triangles.begin(), end = triangles.end(); curr != end; curr++)
        {
        //Check whether the polygon is touched by the contour. If not, skip it.
        if (curr->m_minimum > contourMax || curr->m_maximum < contourMin )
            continue;

       polyPoints.resize(3);

        for(size_t j=0; j < 3; j++)
            {
            polyPoints[j]   = points[abs(curr->m_pointIndices[j])-1];
            polyValues[j]   = values[abs(curr->m_valueIndices[j])-1];
            edgesHidden[j]  = curr->m_pointIndices[j] < 0;   
            }

        //Remove the area above or below the contour. What's left is the section of the contour on our triangle.
        if (curr->m_maximum > contourMaxPlusEpsilon)
            ClipMax (polyPoints, polyValues, contourMax, edgesHidden);

        if (curr->m_minimum < contourMinMinusEpsilon)
            ClipMin (polyPoints, polyValues, contourMin, edgesHidden);

        size_t          polySize;
        if (0 != (polySize = polyPoints.size()))
            {
            for(size_t j=0; j < polySize; j++)
                builder.AddPointIndex (builder.FindOrAddPoint(polyPoints[j]), edgesHidden[j]);

            builder.AddPointIndexTerminator ();
            }
        }

    PolyfaceHeaderR     polyface = builder.GetClientMeshR();
    size_t              nIsoIndices = polyface.GetPointIndexCount();
    bool                hasGraphics = (0 != nIsoIndices);

    if (hasGraphics)
        StrokeIsoContourMesh (0, nIsoIndices, polyface.GetPointIndexCP(), polyface.GetPointCount(), polyface.GetPointCP(), value);

    builder.Clear ();
    return hasGraphics;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   07/2011
* Ugly kludge. If accurate stepped display and transparent margins are on, we used to draw 
* nothing for elements that are completely in the margin, and the ViewContext had nothing
* to cache. Later the PickContext drew non-thematically (because we tell it to) and put those
* graphics in the cache. The next draw tried to use those pick graphics with the thematic
* texture applied, which was obviously not intended. Draw something that produces no 
* graphics so the ViewContext has something to cache.
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawEmptyGraphics ()
    {
    // NEEDS_WORK
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void StrokeIsoContourMeshes (int polySize, size_t nPoints, DPoint3dCP points, Int32 const* indices, size_t nIndices, double const* values, Int32 const* valueIndices)
    {
    ThematicLegend      legend      = m_settings.GetLegend ();
    size_t              nContours   = legend.GetNEntries ();
    double              legendMin   = legend.GetMinValue (0, false);
    double              legendMax   = legend.GetMaxValue (nContours - 1, false);
    static double       s_epsilon   = 1.0E-6;
    T_IsoTriangles      isoTriangles;
    double              meshMin = 1.0E10, meshMax = -1.0E10;
    bool                didDraw = false;
    
    GetIsoTriangles (isoTriangles, meshMin, meshMax, polySize, nIndices, indices, valueIndices, values);

    IFacetOptionsPtr            facetOptions = IFacetOptions::New ();
    IPolyfaceConstructionPtr    builder = IPolyfaceConstruction::New (*facetOptions);

    if (!m_settings.GetFlags ().m_outOfRangeTransparent)
        {
        didDraw |= ClipIsoContourMesh (isoTriangles, nPoints, points, polySize, nIndices, indices, meshMin, meshMax, -DBL_MAX, -s_epsilon, values, &valueIndices[0], *builder, -.5);   // upper margin
        didDraw |= ClipIsoContourMesh (isoTriangles, nPoints, points, polySize, nIndices, indices, meshMin, meshMax, 1.0 + s_epsilon, DBL_MAX,  values, &valueIndices[0], *builder, 1.5);   // lower margin
        }

    for(size_t i = 0; i < nContours; i++)
        {
        double          min   = (legend.GetMinValue (i, false) - legendMin) / (legendMax - legendMin);
        double          max   = (legend.GetMaxValue (i, false) - legendMin) / (legendMax - legendMin);
        double          value = (max + min) / 2.0;

        if (legend.GetVisible ((Int32) i, false))
            didDraw |= ClipIsoContourMesh (isoTriangles, nPoints, points, polySize, nIndices, indices, meshMin, meshMax, min, max, values, &valueIndices[0], *builder, value);
        }

    if (!didDraw)
        DrawEmptyGraphics ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void StrokeIsoContourLine (T_IsoTriangles& triangles, double meshMin, double meshMax, DPoint3dCP points, int polySize, size_t nIndices, Int32 const* indices, double const* values, Int32 const* valueIndices, double value, size_t iContour, ViewContextR context)
    {
    if (value < meshMin || value > meshMax) 
        return;

    ElemMatSymbP        matSymb = context.GetElemMatSymb();
    
    if (!m_settings.GetLegend ().GetVisible (iContour, false))
        return;

    matSymb->SetLineColorTBGR (m_settings.GetLegend ().GetIntColor (iContour, false));
    m_output->ActivateMatSymb (matSymb);

    for (T_IsoTriangles::iterator curr = triangles.begin(), end = triangles.end(); curr != end; curr++)
        {
        if (value > curr->m_minimum && value < curr->m_maximum)
            {
            size_t      nOutput = 0;
            DPoint3d    outPoints[3];

            for (size_t j=0; j < 3; j++)
                {
                size_t      jNext = (j + 1) % 3;
                double      thisValue = values[abs(curr->m_valueIndices[j])-1], 
                            nextValue = values[abs(curr->m_valueIndices[jNext])-1];
                double      delta;
                
                if (0.0 != (delta = nextValue - thisValue))
                    {
                    double  t = (value - thisValue) / delta;

                    if (t > 0.0 && t < 1.0)
                        outPoints[nOutput++].sumOf (NULL, &points[abs(curr->m_pointIndices[j])-1], 1.0 - t, &points[abs(curr->m_pointIndices[jNext])-1], t);
                    }
                }
            if (nOutput> 0)
                m_output->DrawLineString3d ((int) nOutput, outPoints,  NULL);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void StrokeIsoContourLines (int polySize, DPoint3dCP points, Int32 const* indices, size_t nIndices, double const* values, Int32 const* valueIndices, ViewContextR context)
    {
    ThematicLegend      legend    = m_settings.GetLegend ();
    size_t              nContours = legend.GetNEntries ();
    double              legendMin = legend.GetMinValue (0, false);
    double              legendMax = legend.GetMaxValue (nContours - 1, false);

    T_IsoTriangles      isoTriangles;
    double              meshMin, meshMax;
    
    GetIsoTriangles (isoTriangles, meshMin, meshMax, polySize, nIndices, indices, valueIndices, values);

    for (size_t i=0; i<nContours; i++)
        {
        double fraction = (legend.GetMinValue (i, false) - legendMin) / (legendMax - legendMin);

        StrokeIsoContourLine (isoTriangles, meshMin, meshMax, points, polySize, nIndices, indices, values, &valueIndices[0], fraction, i, context);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void    StorePeriodicTriangle (IPolyfaceConstructionR builder, ThematicMeshPointArray& polyPoints, ThematicMeshDoubleArray& polyValues, BoolArray& edgesHidden)
    {
    size_t          polySize;
    if (0 != (polySize = polyPoints.size()))
        {
        for(size_t j=0; j < polySize; j++)
            {
            DPoint2d                param = {0.0, polyValues[j]};

            builder.AddPointIndex (builder.FindOrAddPoint (polyPoints[j]), !edgesHidden[j]);
            builder.AddParamIndex (builder.FindOrAddParam (param));
            }
        builder.AddPointIndexTerminator ();
        builder.AddParamIndexTerminator ();
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void    ClipPeriodicTriangleLow (IPolyfaceConstructionR builder, ThematicMeshPointArray& polyPoints, ThematicMeshDoubleArray& polyValues, BoolArray& edgesHidden, double min, double max)
    {
    ThematicMeshDoubleArray     clipValues = polyValues;
    ThematicMeshPointArray      clipPoints = polyPoints;
    BoolArray                   clipEdgesHidden = edgesHidden;

    // Decrease the high value(s) by one period
    for (size_t i=0; i<polyValues.size(); i++)
        if (clipValues[i] > .5)
            clipValues[i] -= (max-min);

    ClipMin (clipPoints, clipValues, min, clipEdgesHidden);

    StorePeriodicTriangle (builder, clipPoints, clipValues, clipEdgesHidden);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void    ClipPeriodicTriangleHigh  (IPolyfaceConstructionR builder, ThematicMeshPointArray& polyPoints, ThematicMeshDoubleArray& polyValues, BoolArray& edgesHidden, double min, double max)
    {
    ThematicMeshDoubleArray     clipValues = polyValues;
    ThematicMeshPointArray      clipPoints = polyPoints;
    BoolArray                   clipEdgesHidden = edgesHidden;

    // Increase the low value(s) by one period
    for (size_t i=0; i<polyValues.size(); i++)
        if (clipValues[i] < .5)
            clipValues[i] += (max-min);

    ClipMax (clipPoints, clipValues, max, clipEdgesHidden);

    StorePeriodicTriangle (builder, clipPoints, clipValues, clipEdgesHidden);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void StrokeIPolyfaceConstruction (IPolyfaceConstructionR builder, ViewContextR context)
    {
    PolyfaceHeaderR                 polyface = builder.GetClientMeshR();
    size_t                          paramCount = polyface.GetParamCount();
    DPoint2dCP                      params = polyface.GetParamCP();
    ThematicMeshDoubleArray         cacheValues (paramCount);

    for (UInt32 i=0; i<paramCount; i++)
        cacheValues[i] = params[i].y;

    _StrokeThematicMesh (0, polyface.GetPointIndexCount(), polyface.GetPointIndexCP(), const_cast <Int32*> (polyface.GetParamIndexCP()), polyface.GetPointCount(), polyface.GetPointCP(), &cacheValues[0], false, 0.0, 0.0, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SplitPeriodic (int polySize, size_t nPoints, DPoint3dCP points, Int32 const* indices, size_t nIndices, double const* values, Int32* valueIndices, double periodicMin, double periodicMax, ViewContextR context)
    {
    T_IsoTriangles      isoTriangles;
    double              meshMin = 1.0E10, meshMax = -1.0E10;

    GetIsoTriangles (isoTriangles, meshMin, meshMax, polySize, nIndices, indices, valueIndices, values);

    if (meshMax - meshMin < 0.5 * (periodicMax - periodicMin))
        {
        _StrokeThematicMesh (polySize, nIndices, indices, valueIndices, nPoints, points, values, false, 0.0, 0.0, context);
        return;
        }

    BoolArray                           edgesHidden(3);
    ThematicMeshPointArray              polyPoints(3);
    ThematicMeshDoubleArray             polyValues(3);
    static double                       s_splitMax = .6;
    IFacetOptionsPtr                    facetOptions = IFacetOptions::New();

    facetOptions->SetParamsRequired (true);
    IPolyfaceConstructionPtr            builder = IPolyfaceConstruction::New (*facetOptions);

    for (T_IsoTriangles::iterator curr = isoTriangles.begin(), end = isoTriangles.end(); curr != end; curr++)
        {
        polyPoints.resize(3);

        for(size_t j=0; j < 3; j++)
            {
            polyPoints[j]   = points[abs(curr->m_pointIndices[j])-1];
            polyValues[j]   = values[abs(curr->m_valueIndices[j])-1];
            edgesHidden[j]  = curr->m_pointIndices[j] < 0;
            }

        if (curr->m_maximum - curr->m_minimum > s_splitMax * (periodicMax - periodicMin))
            {
            ClipPeriodicTriangleLow (*builder, polyPoints, polyValues, edgesHidden, periodicMin, periodicMax);    // Section near bottom of range
            ClipPeriodicTriangleHigh (*builder, polyPoints, polyValues, edgesHidden, periodicMin, periodicMax);   // Section near top of range
            }
        else
            {
            StorePeriodicTriangle (*builder, polyPoints, polyValues, edgesHidden);
            }
        }

    StrokeIPolyfaceConstruction (*builder, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void        _StrokeThematicMesh (int polySize, size_t nIndices, Int32 const* pointIndices, Int32* valueIndices, size_t nPoints, DPoint3dCP points, double const* values, bool periodic, double periodicMin, double periodicMax, ViewContextR viewContext) override
    {
    if (NULL == m_output)
        {
        BeAssert (false);
        return;
        }

    if (periodic)
        {
        SplitPeriodic (polySize, nPoints, points, pointIndices, nIndices, values, valueIndices, periodicMin, periodicMax, viewContext);
        return;
        }

    ThematicDisplayViewFlagsCR  thematicDisplayViewFlags = m_settings.GetFlags();
    bool                        doThematicDisplay = doThematicForDrawPurpose(viewContext.GetDrawPurpose(), m_settings); 

    if (doThematicDisplay && ThematicSteppedDisplay_Isolines == thematicDisplayViewFlags.m_steppedDisplay)
        {
        StrokeIsoContourLines (polySize, points, pointIndices, nIndices, values, valueIndices, viewContext);
        return;
        }

    ThematicMeshIndexArray      visibleIndices;
    
    // Edge override
    if (doThematicDisplay && ThematicEdgeOverrideDisplay_None != thematicDisplayViewFlags.m_edgeDisplayOverride)
        {
        visibleIndices.resize (nIndices);

        if (ThematicEdgeOverrideDisplay_AllOn == thematicDisplayViewFlags.m_edgeDisplayOverride)
            {
            for (size_t i=0; i<nIndices; i++)
                visibleIndices[i] = abs (pointIndices[i]);
            }
        else
            {
            for (size_t i=0; i<nIndices; i++)
                visibleIndices[i] = -abs (pointIndices[i]);
            }
        
        pointIndices = &visibleIndices[0];
        }


    if (doThematicDisplay && ThematicSteppedDisplay_Accurate == thematicDisplayViewFlags.m_steppedDisplay)
        {
        StrokeIsoContourMeshes (polySize, nPoints, points, pointIndices, nIndices, values, valueIndices);
        }
    else
        {
        size_t                      nValues = maxIndex (valueIndices, nIndices);
        ScalarMeshParamArray        params (nValues);
 
        for (size_t iValue=0; iValue < nValues; iValue++)
            ComputeTextureParam (params[iValue], values[iValue]);

        for (size_t i=0; i<nIndices; i++)
            valueIndices[i] = abs (valueIndices[i]);

        CompressParams (valueIndices, nIndices, params);

        PolyfaceQueryCarrier        polyface (polySize, false, nIndices,                // NumPerFace, twosided.    
                                              nPoints, points, pointIndices,            // Points.
                                              0, NULL, NULL,                            // Normals.
                                              params.size(), &params[0], valueIndices); // Params.

        m_output->DrawPolyface (polyface, true);
       }
    }
};  // ThematicMeshDrawGeom

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    BrandonBohrer   02/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//static void    computeRange (DRange3d* range, DPoint3dCR origin, double width, double height, RotMatrixCR rotation)
//    {
//    Transform   transform;
//    DPoint3d    points [] =    {{ 0, 0, 0 }, // Top-right
//                                { -width, 0, 0 }, // Top-left
//                                { -width, -height, 0 }, // Bottom-left
//                                { 0, -height, 0 }}; // Bottom-right
//
//    transform.initFrom (&rotation, &origin);
//
//    range->low = range->high = origin;
//
//    for (size_t i=0; i<_countof (points); i++)
//        {
//        transform.multiply (&points[i]);
//
//        range->low.x = MIN (range->low.x, points [i].x);
//        range->low.y = MIN (range->low.y, points [i].y);
//        range->low.z = MIN (range->low.z, points [i].z);
//        
//        range->high.x = MAX (range->high.x, points [i].x);
//        range->high.y = MAX (range->high.y, points [i].y);
//        range->high.z = MAX (range->high.z, points [i].z);
//        }
//    }
//
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      10/2010
//+---------------+---------------+---------------+---------------+---------------+------*/
//StatusInt ThematicLegend::CreateElement (EditElementHandleR eeh, DPoint3dCR origin, double width, double height, RotMatrixCR rotation, DgnModelP modelRef)
//    { not needed  in graphite
//    DRange3d range;
//
//    computeRange (&range, origin, width, height, rotation);
//
//    ExtendedElementHandler::InitializeElement (eeh, NULL, modelRef, modelRef->Is3d(), false);
//
//    DataConvert::DRange3dToScanRange (eeh.GetElementP()->hdr.dhdr.range, range);
//
//    ElementHandlerManager::AddHandlerToElement (eeh, ElementHandlerXAttribute (ElementHandlerId (XATTRIBUTEID_DisplayStyleHandler, DisplayStyleHandlerSubID_ThematicLegendElementHandler), MISSING_HANDLER_PERMISSION_All_));
//    ThematicLegendElementData   data (origin, width, height, rotation);
//
//    eeh.ScheduleWriteXAttribute (XAttributeHandlerId (XATTRIBUTEID_DisplayStyleHandler, DisplayStyleHandlerSubID_ThematicLegendElementData),  0, sizeof (data), &data);
//
//    return SUCCESS;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   ThematicDisplayStyleHandler::_GetHitInfoStringFromRawValue (WStringR string, double value, ThematicDisplaySettingsCR settings, ViewportR viewport, ElementHandleR el, WCharCP delimiter) const 
    {
    string = GetName () + WString (L": ") + GetStringFromRawValue (value, settings, *viewport.GetViewController ().GetTargetModel());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void    ThematicDisplayStyleHandler::_CookRange (ThematicDisplaySettingsR settings, ViewportR viewport, DgnModelR modelRef) const 
    {
    settings.m_cookedRange.Set (settings.m_data.m_range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void    ThematicDisplayStyleHandler::_OnFrustumChange (DisplayStyleHandlerSettingsR settings, ViewContextR viewContext, DgnModelR modelRef) const
    {
    ThematicDisplaySettingsP        scalarMeshSettings;

    if (NULL != (scalarMeshSettings  = dynamic_cast <ThematicDisplaySettingsP> (&settings)) &&
        NULL != viewContext.GetViewport())
        _CookRange (*scalarMeshSettings, *viewContext.GetViewport(), *(scalarMeshSettings->m_frustumModel = &modelRef));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void    drawRectangle (DPoint3dCR   lowerLeft, DPoint3dCR upperRight, double z, UInt32 color, UInt8 transparency, ViewContextR context)
    {
    DPoint3d                outlinePoints[5];

    outlinePoints[0] = outlinePoints[4] = lowerLeft;
    outlinePoints[1].init (upperRight.x, lowerLeft.y, z);
    outlinePoints[2] = upperRight;
    outlinePoints[3].init (lowerLeft.x, upperRight.y, z);

    for (size_t i=0; i<5; i++)
        outlinePoints[i].z = z;

    ElemMatSymbP     matSymb = context.GetElemMatSymb ();

    matSymb->SetFillColorTBGR (color + (transparency << 24));
    matSymb->SetWidth (1);
    matSymb->SetMaterial (NULL);

    context.GetIDrawGeom().ActivateMatSymb(matSymb);
    context.GetIDrawGeom().DrawShape3d (5, outlinePoints, true, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void drawText (DPoint3dCR viewOrigin, WCharCP text, double viewTextSize, DgnModelP modelRef, ViewContextR context)
    {
#ifdef DGNV10FORMAT_CHANGES_WIP
    TextParamWide       textParams;
    
    memset (&textParams, 0, sizeof (textParams));
    textParams.just = TextElementJustification::CenterBaseline;
    textParams.exFlags.color = TRUE;

    DPoint2d            scale = { 1, 1 };
    TextString          textString (text, NULL, NULL, true, scale, textParams, *modelRef->GetDgnModelP ());
    textString.SetOriginFromUserOrigin (viewOrigin);

    ElemMatSymbP     matSymb = context.GetElemMatSymb ();

    matSymb->SetLineColorTBGR (0);
    matSymb->SetFillColorTBGR (0);
    context.GetIDrawGeom().ActivateMatSymb(matSymb);
    context.GetIDrawGeom().DrawTextString (textString);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void    initContextForDrawLegend (ViewContextR context)
    {
    ViewportCP      viewport    = NULL;
    ViewFlagsCP     pViewFlags  = NULL;

    if (NULL == (viewport = context.GetViewport ()) 
        || NULL == (pViewFlags = viewport->GetViewFlags ()))
        return;

    ViewFlags       viewFlags = *pViewFlags;

    viewFlags.SetRenderMode (MSRenderMode::SmoothShade);
    viewFlags.renderDisplayEdges = true;
    viewFlags.renderDisplayHidden = false;

    CookedDisplayStyle          cookedStyle (viewFlags, NULL);

    cookedStyle.m_flags.m_smoothIgnoreLights = TRUE;

    context.GetIViewDraw().PushRenderOverrides (viewFlags, &cookedStyle);
    }

// Formatting constants
static const double s_boxHeight = 2.0;
static const double s_gap = 0.5;
static const double s_colorWidth = 5.0;
#ifdef DGNV10FORMAT_CHANGES_WIP
static const double s_npcMargin = 0.01;
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static double  computeLabelWidth (WString const& label, DgnModelR modelRef)
    {
    TextStringProperties    props;
    DPoint2d                scale = { 1, 1 };
    
    props.SetFontSize (scale);
    
    TextString textString (label.GetWCharCP (), NULL, NULL, props);

    textString.LoadGlyphs (NULL);
    return textString.GetWidth ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static double computeValueLabelWidth (WString const& label, DgnModelR modelRef)
    {
    return computeLabelWidth (label, modelRef) + 4.0 * s_gap + s_colorWidth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   02/2011
* Always uses dialog font for size calculation
+---------------+---------------+---------------+---------------+---------------+------*/
void    ThematicDisplayStyleHandler::ComputeLabelsAndSize
(
ThematicDisplaySettingsCR       settings,
double                          maxHeight, 
DgnModelR                    modelRef,
bvector <WString>*              outLabels,
double*                         outWidth,
size_t*                         outNValues,
bool*                           outMaxHeightExceeded
) const
    {
    double                  maxTextHeight = maxHeight - (s_boxHeight + 2 * s_gap + 1.0), min, max;
    size_t                  nValues =  MIN (settings.GetLegend ().GetNEntries ()+ 1, 255);
    double                  width = computeLabelWidth (GetName(), modelRef) + 3.0 * s_gap;
    bvector <WString>   labels;
    bool                    maxHeightExceeded = (s_boxHeight * (nValues - 1) > maxTextHeight);

    if (maxTextHeight <= 0)
        {
        if (NULL != outNValues)
            *outNValues = 0;

        if (0 != width)
            *outWidth = 0.0;

        return;
        }

    settings.GetRange (min, max);

    if (maxHeightExceeded)
        {
        nValues = (size_t) ((double) maxTextHeight / (double) s_boxHeight) + 1;

        // Note: We only need to worry about division by zero when nValues is 1, in which case delta isn't used. So the MAX is strictly for crash prevention.
        double                  delta = (max - min) / (double) MAX (nValues - 1, 1); 

        for (size_t i=0; i < nValues; i++)
            {
            double              value = settings.GetFlags ().m_invertLegend ? (max - delta * i) : min + delta * i;
            WString             label = GetStringFromRawValue (value, settings, modelRef, true);

            width = MAX (width, computeValueLabelWidth (label, modelRef));
            labels.push_back (label);
            }
        }
    else
        {
        bvector <double>    values = settings.GetLegend ().GetAllValues (true);

        // Get labels first as max label size is used to determine width.
        for (size_t i=0; i < values.size (); i++)
            {
            WString             label = GetStringFromRawValue (values[i], settings, modelRef, true);

            width = MAX (width, computeValueLabelWidth (label, modelRef));
            labels.push_back (label);
            }
        }

    if (NULL != outNValues)
        *outNValues = nValues;

    if (NULL != outWidth)
        *outWidth = width;

    if (NULL != outLabels)
        *outLabels = labels;

    if (NULL != outMaxHeightExceeded)
        *outMaxHeightExceeded = maxHeightExceeded;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static double  heightEms (size_t nValues) { return ((double) nValues + 1) * s_boxHeight; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void    ThematicDisplayStyleHandler::DrawLegend 
(
ViewContextR    context,        // =>
double          maxHeight,      // => Maximum height or exact height of the legend, depending on exactHeight
bool            exactHeight,    // => If true, the legend must be drawn at exactly maxHeight, if false, it may be drawn smaller.
UInt8*          transparency    // => Optional, overrides legend transparency
) const 
    {
    // All drawing here is done in ems. Everything is based on text size so no other unit really makes sense. Make sure to set up
    // the transform to draw in ems from your location with your orientation before calling this. +Y is up, +X is right,
    // and the origin is the top-right, to make it easier to draw in that corner of the view.

    ViewportP                   viewport;
    ThematicDisplaySettingsP    settings;

    if (NULL == (viewport = context.GetViewport()) ||
        NULL == (settings = dynamic_cast <ThematicDisplaySettingsP> (context.GetDisplayStyleHandlerSettings())))
        return;

    double                  min, max;
    DgnModelP               modelRef = context.GetViewport()->GetViewController().GetTargetModel ();
    UInt8                   fillTransparency = static_cast<UInt8>((NULL == transparency) ? 255 - settings->GetLegendOpacity () : *transparency);
    
    CookRange (*settings, *viewport, *modelRef);
    settings->GetRange (min, max);

    size_t                  nValues;
    bool                    drawSmooth = false;
    bvector<WString>        labels;
    double                  width = 0;
    
    ComputeLabelsAndSize (*settings, maxHeight, *modelRef, &labels, &width, &nValues, &drawSmooth);
    drawSmooth |= (ThematicSteppedDisplay_None == settings->GetFlags().m_steppedDisplay);

    static const DPoint3d   s_origin = {};
    static const double     s_epsilonZ = 2E-2;

    double                  height = exactHeight ? maxHeight : MIN (heightEms (nValues), maxHeight);
    double                  boxWidth  = width - 2.0 * s_gap;
    DPoint3d                lowerLeft  = { -width, -height, 0};
    double                  boxZ = - (2 * s_epsilonZ), textZ = -s_epsilonZ;
    double                  centerX = lowerLeft.x + width / 2.0;
    DPoint3d                labelOrigin = {centerX, - (s_gap + 1.0), textZ };
    
    drawRectangle (lowerLeft, s_origin, boxZ, 0x00ffffff, fillTransparency, context);
    drawText (labelOrigin, GetName().c_str(), 1.0, modelRef, context);     

    if (drawSmooth)
        {
        if (nValues > 1)
            {
            // Draw a continuous bar showing the entire color scheme,
            double                      topParam        = 0.748, bottomParam = 1.0 - topParam;
       
            if (settings->GetFlags ().m_invertLegend) 
                std::swap (topParam, bottomParam);

            DPoint3d const              boxUpperRight   =   { -s_gap - boxWidth + s_colorWidth, - (s_boxHeight + s_gap + 0.5), textZ },
                                        boxLowerLeft    =   { boxUpperRight.x - s_colorWidth, - (nValues * s_boxHeight + 2.0 * s_gap), textZ };

            DPoint3d                    polyPoints[]    = { { boxLowerLeft.x, boxLowerLeft.y, textZ },   { boxUpperRight.x, boxLowerLeft.y, textZ },
                                                            { boxUpperRight.x, boxUpperRight.y, textZ }, { boxLowerLeft.x, boxUpperRight.y, textZ } };
            Int32    const              indices[]       =   { 1, 2, 3, 4, 0 };

            DPoint2d const              uvCoords[]      = { { 0.5, bottomParam}, {0.5, topParam} };
            Int32    const              uvIndices[]     =   { 1, 1, 2, 2, 0 };
            PolyfaceQueryCarrier        polyface (0, true, 5,
                                                  4, polyPoints, indices,
                                                  0, NULL, NULL,
                                                  2, uvCoords, uvIndices);
                                                  

            ElemMatSymbP                matSymb = context.GetElemMatSymb();

            matSymb->SetMaterial (settings->GetMaterial (const_cast <ThematicDisplayStyleHandlerR> (*this), context), &context);
            context.GetIDrawGeom().ActivateMatSymb (matSymb);
            context.GetIDrawGeom().DrawPolyface (polyface, true);
            }
        }
    else
        {
        //Draw boxes showing the color at each contour
        for (size_t i=0; i+1 < nValues; i++)
            {
            DPoint3d            boxUpperRight = { -s_gap - boxWidth + s_colorWidth, -((i + 1) * s_boxHeight + s_gap + 0.5), textZ },
                                boxLowerLeft  = { boxUpperRight.x - s_colorWidth, boxUpperRight.y - s_boxHeight, textZ };
 
            drawRectangle (boxLowerLeft, boxUpperRight, textZ, settings->GetLegend ().GetIntColor (i, true), fillTransparency, context);
            }
        }

    //Draw a list of labels showing the values at different points. In isocontour mode these are the start/end points for contours.
    double boxY = s_boxHeight + s_gap + 1;
    
    for (size_t i=0; i < nValues && boxY < height; i++, boxY += s_boxHeight)
        {
        DPoint3d            origin         = { centerX + (s_colorWidth/2), -boxY, textZ };

        drawText (origin,  labels[i].c_str(), 1.0, modelRef, context);
        }
    }

#ifdef DGNV10FORMAT_CHANGES_WIP
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static double computeEmSize (double sizeEms, ViewContextR context)
    {
    DPoint3d    plusYNpc  = { 0, (1 - (2 * s_npcMargin)) / sizeEms, 1 }, plusYFrustum, 
                originNpc = { 0, 0, 1}, originFrustum, diff;

    context.NpcToFrustum (&plusYFrustum, &plusYNpc, 1);
    context.NpcToFrustum (&originFrustum, &originNpc, 1);

    diff.differenceOf (&plusYFrustum, &originFrustum);

    return diff.magnitude ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t computeMaxBarCount (ViewContextR context)
    {
    static size_t       s_plotMaxBarCount = 50;                 // If plotting assume they can at least see 50 rows of text
    static double       s_minTextScreenPixels = 20.0;           // Else don't let text size go below 20 pixels.

    if (DrawPurpose::Plot == context.GetDrawPurpose())          
        return s_plotMaxBarCount;
    
    DPoint3d                npc[2], view[2];                    

    npc[0].init (0.0, 0.0, 1.0);
    npc[1].init (0.0, 1.0, 1.0);

    context.NpcToView (view, npc, 2);

    return (size_t) (fabs (view[1].y - view[0].y) / s_minTextScreenPixels);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ThematicDisplayStyleHandler::_DrawLegendForView (ViewContextR context, ViewportR vp, ThematicDisplaySettingsR settings) const 
    {
#ifdef DGNV10FORMAT_CHANGES_WIP
    DPoint3d                    npcUpperRightFront = { 1 - s_npcMargin, 1 - s_npcMargin, 1.0} , localUpperRightFront;
    RotMatrix                   fullMatrix;
    Transform                   viewTransform;
    ViewContext::DgnModelMark   mark (context);
    size_t                      barCount = computeMaxBarCount(context);
    double                      sizeEms = heightEms (barCount + 1);
    double                      scale = computeEmSize (sizeEms, context);

    if (SUCCESS != context.SetToDgnModel (vp.GetRootModel ()) ||
        NULL == context.GetCurrentModel())
        return;

    CookRange (settings, vp, *context.GetCurrentModel ());
    initContextForDrawLegend (context);

    context.NpcToFrustum (&localUpperRightFront, &npcUpperRightFront, 1);
    fullMatrix.inverseOf (context.GetViewport ()->GetRotMatrix ());
    fullMatrix.scaleRows (&fullMatrix, scale, scale, scale);
    viewTransform.initFrom (&fullMatrix, &localUpperRightFront);

    context.PushTransform (viewTransform);
    DrawLegend (context, sizeEms, false, NULL);
    context.PopTransformClip ();

    context.GetIViewDraw().PopRenderOverrides ();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void  ThematicDisplayStyleHandler::_DrawLegend (ViewContextR context) const 
    {
    switch (context.GetDrawPurpose())
        {
        case DrawPurpose::Update:                                                                                                                                      
        case DrawPurpose::UpdateHealing:
        case DrawPurpose::UpdateDynamic:
        case DrawPurpose::Plot:
        case DrawPurpose::GenerateThumbnail:
            break;

        default:
            return;
        }

    ViewportP                   vp = NULL;
    ThematicDisplaySettingsP    settings = NULL;

    if (NULL != (vp = context.GetViewport ()) &&
        NULL != (settings = dynamic_cast <ThematicDisplaySettingsP> (context.GetDisplayStyleHandlerSettings ())) &&
        !settings->GetFlags().m_noLegend)
        _DrawLegendForView (context, *vp, *settings);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ThematicDisplayStyleHandler::_IsValidForDisplayStyle (DisplayStyleCR style) const
    {
    return (0 == style.GetOverrides ().m_flags.m_elementColor)
        && (0 == style.GetFlags ().m_ignoreImageMaps)
        && (MSRenderMode::SmoothShade == style.GetFlags().m_displayMode);
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EmbeddedScalarMeshStroker : IStrokeForCache
{
    ThematicDisplaySettingsR                    m_settings;
    ThematicMeshValueDisplayStyleHandlerCR      m_handler;

    EmbeddedScalarMeshStroker (ThematicDisplaySettingsR settings, ThematicMeshValueDisplayStyleHandlerCR handler) : m_settings (settings), m_handler (handler)  { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void            _StrokeForCache (CachedDrawHandleCR dh, ViewContextR context, double pixelSize = 0.0) override
    {
    int                         polySize = 0;
    ThematicMeshPointArray      points;
    ThematicMeshDoubleArray     values;
    ThematicMeshIndexArray      pointIndices, valueIndices;
    
   BeAssert(false && "in EmbeddedScalarMeshStroker"); //  WIP_NEW_CACHE

    ElementHandleCR thisElm = *dh.GetElementHandleCP();
    if (m_handler.GetMesh (thisElm, polySize, pointIndices, valueIndices, points, values, m_settings))
        ThematicMeshDrawGeom (m_handler, m_settings, context, &context.GetIViewDraw ())._StrokeThematicMesh (polySize, pointIndices.size(), &pointIndices[0], &valueIndices[0], points.size(), &points[0], &values[0], false, 0.0, 0.0, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _WantLocateByStroker () override {return false;} // Don't call _StrokeForCache, locate interior by QvElem...

};  // EmbeddedScalarMeshStroker

static bool    doHilite (ViewContextR context)  { return  HILITED_None != context.GetCurrHiliteState(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool        ThematicMeshValueDisplayStyleHandler::_DrawElement (ElementHandleCR el, ViewContextR context) const
    {
    if (!_EmbeddedDataPresent (el))
        return false;          

    ViewportP                   viewport;
    ThematicDisplaySettingsP    settings = NULL;
    double                      min, max;

    if (NULL == (settings = dynamic_cast <ThematicDisplaySettingsP> (context.GetDisplayStyleHandlerSettings())))
        return false;

    settings->GetRawRange (min, max);
    if (max <= min)
        return false;       // TR# 329131 - Avoid crash when insolation model info is not present (after merge).


    if (NULL != (viewport = context.GetViewport()) &&
        settings->MaterialOverrideRequired() &&
        !doHilite (context))
        {
        OvrMatSymbP  overrideMatSymb = context.GetOverrideMatSymb();

        overrideMatSymb->SetMaterial (settings->GetMaterial (*this, context), &context);
        context.ActivateOverrideMatSymb ();
        }

    ThematicDisplaySettings defaultSettings;
    EmbeddedScalarMeshStroker       stroker (NULL == settings ? defaultSettings : *settings, *this);

    context.DrawCached (el, stroker, 1);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   02/2011
*   Kludge to keep the thematic legend elements from being displayed thematically 
*   themselves. Note that it would be more robust to put this with DisplayStyleHandler,
*   But we put it here to avoid making DisplayStyleHandler dependent on thematic display.
+---------------+---------------+---------------+---------------+---------------+------*/
bool        ThematicDisplayStyleHandler::NeverApplyThematic (DisplayHandlerCP handler) 
    { 
#ifdef NEEDS_WORK_LEGEND
    return NULL != dynamic_cast <ThematicLegendElementHandler const*> (handler); 
#else
    return false;
#endif
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static bool doThematicDisplay (ThematicDisplaySettingsP& settings, ViewportP& viewport, ElementHandleCR el, ViewContextR context)
    {
    return NULL != (settings = dynamic_cast <ThematicDisplaySettingsP> (context.GetDisplayStyleHandlerSettings())) &&
           doThematicForDrawPurpose(context.GetDrawPurpose(), *settings) &&
           NULL != (viewport = context.GetViewport()) &&
           !ThematicDisplayStyleHandler::NeverApplyThematic (el.GetDisplayHandler ());
    }
    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool        ThematicElementPropertyDisplayHandler::_DrawElement (ElementHandleCR el, ViewContextR context) const 
    {
    ThematicDisplaySettingsP    settings;
    ViewportP                   viewport;
    double                      value;
                                                    
    if (!doThematicDisplay (settings, viewport, el, context) ||
        !_GetElementValue (value, el, *viewport))
        return false;

    UInt32                      color = settings->GetLegendColor (value);
    OvrMatSymbP                 overrideMatSymb = context.GetOverrideMatSymb();

    overrideMatSymb->SetLineColorTBGR ((overrideMatSymb->GetLineColorTBGR() & 0xff000000) | color);         // Preserve existing transparency.
    overrideMatSymb->SetFillColorTBGR ((overrideMatSymb->GetFillColorTBGR() & 0xff000000) | color);
    overrideMatSymb->SetMaterial (NULL);
    context.ActivateOverrideMatSymb ();

    el.GetDisplayHandler()->Draw (el, context);

    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ThematicElementPropertyDisplayHandler::_GetHitInfoString (WStringR string, HitPathCR hitPath, DPoint3dCR hitPoint, ThematicDisplaySettingsCR settings, ViewportR viewport, ElementHandleR el, WCharCP delimiter) const
    {
    double          value;

    return  _GetElementValue (value, el, viewport) ? _GetHitInfoStringFromRawValue (string, value, settings, viewport, el, delimiter) : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void     ThematicElementPropertyDisplayHandler::_CookRange (ThematicDisplaySettingsR settings, ViewportR viewport, DgnModelR modelRef) const
    {
    double          min, max;

    if ((!settings.IsMinFixed() || !settings.IsMaxFixed()) && _GetModelRange (min, max, modelRef))
        {
        if (!settings.IsMinFixed())
            settings.SetRangeMin (min);

        if (!settings.IsMaxFixed())
            settings.SetRangeMax (max);
        }

    settings.m_cookedRange.Set (settings.GetRange());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool     ThematicElementPropertyDisplayHandler::_IsValidForViewport (ViewportCR viewport) const 
    {
    double          min, max;

    return _GetModelRange (min, max, *viewport.GetViewController ().GetTargetModel());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool        GeometricThematicMeshDisplayStyleHandler::_DrawElement (ElementHandleCR el, ViewContextR context) const
    {
    if (NULL != dynamic_cast <ThematicMeshDrawGeom*> (&context.GetIDrawGeom()))
        return false;

    ThematicDisplaySettingsP    settings;
    ViewportP                   viewport;

    if (!doThematicDisplay (settings, viewport, el, context))
        return false;

    if (settings->MaterialOverrideRequired() && !doHilite (context))
        {
        OvrMatSymbP overrideMatSymb = context.GetOverrideMatSymb();

        overrideMatSymb->SetMaterial (settings->GetMaterial (*this, context), &context);
        context.GetIViewDraw().ActivateOverrideMatSymb (overrideMatSymb);
        }

    ThematicMeshDrawGeom  thematicMeshDrawGeom (*this, *settings, context, &context.GetIViewDraw ());
    IDrawGeomR            saveDrawGeom = context.GetIDrawGeom (); // Save both m_IViewDraw and m_IDrawGeom in case they aren't the same...
    IViewDrawR            saveViewDraw = context.GetIViewDraw ();

    (const_cast <GeometricThematicMeshDisplayStyleHandler*> (this))->m_stroked = false;
    (const_cast <GeometricThematicMeshDisplayStyleHandler*> (this))->m_curvedGeometryStroked = false;

    context.SetIViewDraw (thematicMeshDrawGeom); // This sets both m_IViewDraw and m_IDrawGeom...
    el.GetDisplayHandler()->Draw (el, context);
    context.SetIViewDraw (saveViewDraw); // Restore previous m_IViewDraw...
    context.SetIDrawGeom (saveDrawGeom); // Restore previous m_IDrawGeom...

#ifdef NEEDS_WORK_SIZE_DEPENDENT
    if (m_stroked && !m_curvedGeometryStroked)
        ViewContext::MakeQvCacheElemSizeIndependent (el.GetElementRef());
#endif

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool        GeometricThematicMeshDisplayStyleHandler::_StrokeForCache (CachedDrawHandleCR dh, ViewContextR context, IStrokeForCache& stroker, double pixelSize) const
    {
    if (NULL != dynamic_cast <ThematicMeshDrawGeom*> (&context.GetIDrawGeom()))
        return false;

    ThematicDisplaySettingsP    settings;
    ViewportP                   viewport;

    BeAssert(false && "in GeometricThematicMeshDisplayStyleHandler"); //  WIP_NEW_CACHE
    ElementHandleCR el = *dh.GetElementHandleCP();
    if (!doThematicDisplay (settings, viewport, el, context))
        return false;

    ThematicMeshDrawGeom  thematicMeshDrawGeom (*this, *settings, context, &context.GetIViewDraw ());
    IDrawGeomR            saveDrawGeom = context.GetIDrawGeom (); // Save both m_IViewDraw and m_IDrawGeom in case they aren't the same...
    IViewDrawR            saveViewDraw = context.GetIViewDraw ();

    (const_cast <GeometricThematicMeshDisplayStyleHandler*> (this))->m_stroked = true;

    context.SetIViewDraw (thematicMeshDrawGeom); // This sets both m_IViewDraw and m_IDrawGeom...
    stroker._StrokeForCache (dh, context, pixelSize);
    context.SetIViewDraw (saveViewDraw); // Restore previous m_IViewDraw...
    context.SetIDrawGeom (saveDrawGeom); // Restore previous m_IDrawGeom...

    (const_cast <GeometricThematicMeshDisplayStyleHandler*> (this))->m_curvedGeometryStroked = thematicMeshDrawGeom.CurvedGeometryStroked();
    
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt  PointThematicMeshDisplayStyleHandler::_ProcessMesh (IThematicMeshStroker& stroker, ViewContextR context, int polySize, size_t nIndices, Int32 const* pointIndices, Int32 const* normalIndices, size_t nPoints, DPoint3dCP points, DVec3dCP normals, bool twoSided, ThematicDisplaySettingsCR settings) const 
    {
    ThematicMeshDoubleArray       values (nPoints);
    ThematicMeshIndexArray        valueIndices (nIndices);

    for (UInt32 i=0; i<nPoints; i++)
        {
        DPoint3d        localPoint;
        
        context.LocalToFrustum (&localPoint, &points[i], 1);
        
        values[i] = settings.GetNormalizedValueFromRaw (_GetPointValue (localPoint, settings));
        }

    memcpy (&valueIndices[0], pointIndices, nIndices * sizeof (Int32));
    stroker._StrokeThematicMesh (polySize, nIndices, pointIndices, &valueIndices[0], nPoints, points, &values[0], false, 0.0, 0.0, context);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PointThematicMeshDisplayStyleHandler::_GetPointColors (RgbColorDef* colors, DPoint3dCP points, int nPoints, ThematicDisplaySettingsCR settings, ViewContextR context) const
    {           
    Transform       localToFrustum;

    if (SUCCESS != context.GetCurrLocalToFrustumTrans (localToFrustum) ||
        localToFrustum.isIdentity())
        {
        for (int i=0; i<nPoints; i++)
            settings.GetColor (colors[i], _GetPointValue (points[i], settings)); 
        }
    else
        {
        for (int i=0; i<nPoints; i++)
            {
            DPoint3d        frustumPoint;
        
            localToFrustum.multiply (&frustumPoint, &points[i], 1);
            settings.GetColor (colors[i], _GetPointValue (frustumPoint, settings)); 
            }
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       calculateSmoothMeshNormals (ThematicMeshPointArray& normals, ThematicMeshIndexArray& normalIndices, int polySize, size_t nIndices, Int32 const*  pointIndices, DPoint3dCP points)
    {
    size_t                  start = 0, nPoints = 0, maxPolySize = (0 == polySize ?  0xffff : polySize), pointSize = maxIndex (pointIndices, nIndices);
    const size_t            minPolySize = 3; // Since anything less is degenerate
    bvector<size_t>     normalCounts (pointSize, 0);

    normals.resize (pointSize);
    
    for (size_t i=0; i <= nIndices; i++)
        {
        bool        isLast = (i == nIndices), isSeparator = (isLast || 0 == pointIndices[i]);

        // Found end of non-degenerate poly
        if (nPoints >= maxPolySize || (isSeparator && nPoints >= minPolySize))
            {
            DVec3d      normal;

            normal.crossProductToPoints (&points[ONE_INDEX (pointIndices[start])], &points[ONE_INDEX (pointIndices[start+1])], &points[ONE_INDEX (pointIndices[start+2])]);
            normal.normalize ();

            for (size_t j=0; j<nPoints; j++)
                {
                size_t      index = ONE_INDEX (pointIndices[start+j]);

                if (0 == normalCounts[index])
                    normals[index] = normal;
                else 
                    normals[index].add (&normal);

                normalCounts[index]++;
                }
            }
        
        // On the last iteration running the tests below would cause an out-of-bounds read.
        if (isLast)
            break;

        // Found separator. This part is required even for degenerate polys.
        if (0 == pointIndices[i])
            {
            start = i+1;
            nPoints = 0;
            }
        // Found end of fixed-sized poly
        else if (nPoints >= maxPolySize)
            {
            start = i;
            nPoints = 1;
            }
        // In middle of poly
        else
            {
            nPoints++;
            }
        }

    for (size_t i=0; i<pointSize; i++)
        if (0 != normalCounts[i])
            normals[i].scale (1.0 / (double) normalCounts[i]);

    normalIndices.resize (nIndices);
    memcpy (&normalIndices[0], &pointIndices[0], nIndices * sizeof (Int32));

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
* Note: If this function encounters degenerate polys it will need the same fix as TR# 330804. 
*  However it appears we always clean up the geometry by this point and I don't want to
*  break anything with the fix.
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ThematicMeshDisplayStyleHandler::CalculateMeshNormals (ThematicMeshPointArray& normals, ThematicMeshDoubleArray* areas, ThematicMeshIndexArray& normalIndices, int polySize, size_t nIndices, Int32 const*  pointIndices, DPoint3dCP points)
    {
    size_t          start = 0, nPoints = 0, maxPolySize = (0 == polySize ?  0xffff : polySize);

    for (size_t i=0; i <= nIndices; i++)
        {
        bool        isLast = (i == nIndices), isSeparator = (isLast || 0 == pointIndices[i]);

        if (nPoints >= maxPolySize || (isSeparator && 0 != nPoints))
            {
            DVec3d      normal;
            double      area = 0.0;

            if (nPoints < 3)
                {
                normal.zero();
                }
            else
                {
                normal.crossProductToPoints (&points[ONE_INDEX (pointIndices[start])], &points[ONE_INDEX (pointIndices[start+1])], &points[ONE_INDEX (pointIndices[start+2])]);
                area = normal.normalize ();
                }
            normals.push_back (normal);
            if (NULL != areas)
                areas->push_back (area);

            for (size_t j=0; j<nPoints; j++)
                normalIndices.push_back ((Int32) normals.size());

            if (isSeparator)
                normalIndices.push_back (0);

            if (isLast)
                break;

            if (0 == pointIndices[start = i])
                {
                start++;
                nPoints = 0;
                }
            else
                {
                nPoints = 1;
                }
            }
        else
            {
            nPoints++;
            }
        }

    BeAssert (nIndices == normalIndices.size());
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt  NormalThematicMeshDisplayStyleHandler::_ProcessMesh (IThematicMeshStroker& stroker, ViewContextR context, int polySize, size_t nIndices, Int32 const* pointIndices, Int32 const* normalIndices, size_t nPoints, DPoint3dCP points, DVec3dCP normals, bool twoSided, ThematicDisplaySettingsCR settings) const 
    {
    ThematicMeshPointArray          calculatedNormals;
    ThematicMeshIndexArray          calculatedNormalIndices;
    StatusInt                       status;

    if (NULL == normals)
        {
        if (SUCCESS != (status = settings.GetFlags().m_noSmoothing ? CalculateMeshNormals (calculatedNormals, NULL, calculatedNormalIndices, polySize, nIndices, pointIndices, points) :
                                                                     calculateSmoothMeshNormals (calculatedNormals, calculatedNormalIndices, polySize, nIndices, pointIndices, points)))
            return status;

        normals         = (DVec3dCP)  &calculatedNormals[0];
        normalIndices   = &calculatedNormalIndices[0];
        }
    else if (NULL == normalIndices)
        {
        // If normals are present , but no normal indices, Use pointIndices is implied.
        normalIndices = pointIndices;
        }
                        
    size_t          nNormals = maxIndex (normalIndices, nIndices);
    Transform       localTransform;
    bool            applyLocal;

    applyLocal = (SUCCESS == context.GetCurrLocalToFrustumTrans (localTransform));

    ThematicMeshDoubleArray       values (nNormals);
    ThematicMeshIndexArray        valueIndices (nIndices);

    for (UInt32 i=0; i<nNormals; i++)
        {
        DVec3d      normal = normals[i];

        if (applyLocal)
            localTransform.multiplyMatrixOnly (&normal);
            
        values[i] = settings.GetNormalizedValueFromRaw (_GetNormalValue (normal, twoSided, settings));
        }

    double normalizedMin = settings.GetNormalizedValueFromRaw (_GetMinValue (settings.GetDisplayMode())),
           normalizedMax = settings.GetNormalizedValueFromRaw (_GetMaxValue (settings.GetDisplayMode()));

    memcpy (&valueIndices[0], normalIndices, nIndices * sizeof (Int32));
    stroker._StrokeThematicMesh (polySize, nIndices, pointIndices, &valueIndices[0], nPoints, points, &values[0], IsPeriodic(), normalizedMin, normalizedMax, context);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool        insidePolygon (double& value, ThematicMeshPointArray& points, ThematicMeshDoubleArray& values, DPoint3dCR point)
    {
    size_t                      nPoints = points.size();
    ThematicMeshDoubleArray     barycentrics (nPoints);

    value = 0.0;
    if (bsiDPoint3d_barycentricFromDPoint3dConvexPolygon (&barycentrics[0], &point, &points[0], (int) nPoints))
        {
        for (size_t i=0; i<nPoints; i++)
            {
            double& barycentric  = barycentrics[i];

            if (barycentric < 0.0 || barycentric > 1.0)
                return false;
            else
                value += barycentric * values[i];
            }
        }

    return true;
    }



void        ThematicDisplayStyleHandler::InitDefaultSettings (ThematicDisplaySettingsR settings) const                     { _InitDefaultSettings (settings); }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString     ThematicDisplayStyleHandler::GetStringFromNormalizedValue (double value, ThematicDisplaySettingsCR settings, DgnModelR modelRef, bool includeUnits) const                                                                
    {
    double      min, max;

    settings.GetRange (min, max);
    return GetStringFromRawValue (min + value * (max - min), settings, modelRef, includeUnits);
    }


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2011                                                                              
+===============+===============+===============+===============+===============+======*/
struct      GetHitValueStroker : IThematicMeshStroker
    {
    bool            m_hitFound;
    double          m_value;
    DPoint3d        m_hitPoint;
    size_t          m_hitFacetIndex;
    size_t          m_hitFacetNumber;
    ViewportR       m_viewport;

    GetHitValueStroker (DPoint3dCR hitPoint, ViewportR viewport) : m_hitPoint (hitPoint), m_viewport (viewport), m_hitFound (false) { }
    bool            GetHitValue (double& value, size_t& index, size_t& number)   { value = m_value; index = m_hitFacetIndex, number = m_hitFacetNumber; return m_hitFound; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void        _StrokeThematicMesh (int polySize, size_t nIndices, Int32 const* pointIndices, Int32* valueIndices, size_t nPoints, DPoint3dCP points, double const* values, bool isPeriodic, double periodicMin, double periodicMax, ViewContextR context) override
    {
    size_t                      maxPolySize = (0 == polySize) ? 0xffff : polySize;
    DRange3d                    range;
    double                      containTolerance = m_viewport.GetViewController().GetDelta().magnitudeXY() / 1000.0;
    DPoint3d                    localHitPoint;
    ThematicMeshPointArray      facetPoints;
    ThematicMeshDoubleArray     facetValues;

    range.init ();
    m_hitFacetIndex = m_hitFacetNumber = 0;
    context.FrustumToLocal (&localHitPoint, &m_hitPoint, 1);

    for (size_t i=0; i <= nIndices; i++)
        {
        Int32       index = pointIndices[i];

        if (facetPoints.size() == maxPolySize || 0 == index)
            {
            range.extend (containTolerance);

            if (range.isContained (&localHitPoint) && insidePolygon (m_value, facetPoints, facetValues, localHitPoint))
                {
                m_hitFound = true;
                return;
                }

            m_hitFacetNumber++;
            m_hitFacetIndex = i;
            range.init ();
            facetPoints.clear();
            facetValues.clear();
            }

        if (0 != index && i < nIndices)
            {
            DPoint3dCR      point = points[abs(index) - 1];

            range.extend (&point);
            facetPoints.push_back (point);
            facetValues.push_back (values[abs(valueIndices[i])-1]);
            }
        }
    }
};   // GetHitValueStroker



/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2010
+===============+===============+===============+===============+===============+======*/
struct StrokeHitContext : ViewContext
{

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StrokeHitContext (ViewportR viewport)
    {
    Attach (&viewport, DrawPurpose::CaptureGeometry);
    m_parentRangeResult = RangeResult::Inside;
    SetViewFlags (viewport.GetViewFlags ());
    }

    void      _SetupOutputs () override { }
    QvElem*   _DrawCached (CachedDrawHandleCR dh, IStrokeForCache& stroker, Int32) override { stroker._StrokeForCache (dh, *this); return NULL; }
    

 };  // StrokeHitContext

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void pushPath (ViewContextR vContext, HitPathCR hitPath)
    {
    vContext.SetDgnProject (hitPath.GetRoot()->GetDgnProject ());

    for (int i=0; i<hitPath.GetCursorIndex() ; i++)
        {
        ElementHandle eh (hitPath.GetPathElem (i));
        IDisplayHandlerPathEntryExtension* extension = IDisplayHandlerPathEntryExtension::Cast (eh.GetHandler ());

        if (extension)
            extension->_PushDisplayEffects (eh, vContext);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt  ThematicMeshDisplayStyleHandler::_GetHitInfoStringFromFacet (WStringR string, double normalizedValue, size_t facetIndex, size_t facetNumber, ThematicDisplaySettingsCR settings, ViewportR viewport, ElementHandleR el, WCharCP delimiter) const
    {                           
    return _GetHitInfoStringFromRawValue (string, settings.GetRawValueFromNormalized (normalizedValue), settings, viewport, el, delimiter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt  ThematicMeshDisplayStyleHandler::_GetHitInfoString (WStringR string, HitPathCR hitPath, DPoint3dCR hitPoint, ThematicDisplaySettingsCR settings, ViewportR viewport, ElementHandleR el, WCharCP delimiter)  const
    {
    double                      value;
    size_t                      hitIndex, hitNumber;
    GetHitValueStroker          hitValueStroker (hitPoint, viewport);

    StrokeHitContext            strokeHitContext (viewport);
    ThematicMeshDrawGeom        thematicMeshDrawGeom (*this, settings, strokeHitContext, NULL);

    strokeHitContext.SetIViewDraw (thematicMeshDrawGeom);
    pushPath (strokeHitContext, hitPath);
    thematicMeshDrawGeom.SetStroker (hitValueStroker);
    strokeHitContext.CookElemDisplayParams (el);
    el.GetDisplayHandler()->Draw (el, strokeHitContext);
    strokeHitContext.Detach ();

    return hitValueStroker.GetHitValue (value, hitIndex, hitNumber) ? _GetHitInfoStringFromFacet (string, value, hitIndex, hitNumber, settings, viewport, el, delimiter) : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt  ThematicMeshValueDisplayStyleHandler::_GetHitInfoString (WStringR string, HitPathCR hitPath, DPoint3dCR hitPoint, ThematicDisplaySettingsCR settings, ViewportR viewport, ElementHandleR el, WCharCP delimiter) const
    {
    int                     polySize = 0;
    ThematicMeshPointArray  points;
    ThematicMeshDoubleArray values;
    ThematicMeshIndexArray  pointIndices, valueIndices;

    if (!_GetMesh (el, polySize, pointIndices, valueIndices, points, values, settings))
        return ERROR;

    double              value;
    size_t              facetIndex, facetNumber;
    GetHitValueStroker  hitValueStroker (hitPoint, viewport);
    NullOutput          output;
    NullContext         context (&output);

    context.Attach (&viewport, DrawPurpose::NotSpecified);
    pushPath (context, hitPath);

    hitValueStroker._StrokeThematicMesh (polySize, pointIndices.size(), &pointIndices[0], &valueIndices[0], points.size(), &points[0], &values[0], false, 0.0, 0.0, context);
    context.Detach ();

    return hitValueStroker.GetHitValue (value, facetIndex, facetNumber) ? _GetHitInfoStringFromFacet (string, value, facetIndex, facetNumber, settings, viewport, el, delimiter) : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ThematicDisplaySettings::ThematicDisplaySettings () : m_pMaterial (NULL), DisplayStyleHandlerSettings ()
    {
    m_marginColor = s_defaultMarginColor;
    m_legendTransparency = 0;
    m_legendValueStep = 0.0; // Special value meaning "Generate a reasonable default"
    m_legend = ThematicLegend (this);
    memset (&m_data, 0, sizeof (m_data)); 
    m_colorMap.Init (*this, 512);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void    ThematicDisplaySettings::SetColorScheme (ThematicColorScheme scheme)  
    { 
    m_data.m_colorScheme = scheme; 
    m_colorMap.Init (*this, 512);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 ThematicDisplaySettings::GetColor (double value, double min, double max) const 
    {
    return m_colorMap.Get (value, min, max); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void  ThematicDisplaySettings::GetColor (RgbColorDef& color, double rawValue) const
    {
    double      normalizedValue = GetNormalizedValueFromRaw (rawValue);

    if (normalizedValue < 0.0 || normalizedValue > 1.0)
        {
        memcpy (&color, &m_marginColor, sizeof (color));
        }
    else
        {
        double      textureValue =  s_margin + s_colorRange * normalizedValue;
        static      double      s_dTextureSize = (double) THEMATIC_TEXTURE_SIZE;

        memcpy (&color, &m_texturePixels[(int) (s_dTextureSize * textureValue)], sizeof (color));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 ThematicDisplaySettings::GetLegendColor (double value) const 
    {
   ThematicLegendCR         legend      = GetLegend ();
   size_t                   legendSize  = legend.GetNEntries ();

    
    for (size_t i=0; i < legendSize; i++)
        if (value >= legend.GetMinValue (i, false) && value <= legend.GetMaxValue(i, false))
            return legend.GetIntColor (i, false);

    return GetMarginColor();
    }


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2010
+===============+===============+===============+===============+===============+======*/
 enum   ThematicDisplaySettingsSubType
 {
    ThematicDisplaySettingsSubType_Unused               = 1,
    ThematicDisplaySettingsSubType_GradientKey          = 2,
    ThematicDisplaySettingsSubType_LegendKey            = 3,
    ThematicDisplaySettingsSubType_LegendValue          = 4,   
    ThematicDisplaySettingsSubType_LegendColor          = 5,   
    ThematicDisplaySettingsSubType_MarginColor          = 6,
    ThematicDisplaySettingsSubType_LegendTransparency   = 7,
    ThematicDisplaySettingsSubType_LegendValueStep      = 8,
 };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ThematicDisplaySettings::_Read (ElementRefP elementRef, int styleIndex) 
    {
    StatusInt               status;
    bvector<byte>       buffer;

    if (SUCCESS != (status = DisplayStyleHandlerSettings::_Read (elementRef, styleIndex)) ||
        SUCCESS != (status = ReadData (buffer, elementRef, DisplayStyleHandler_SettingsXAttributeSubId_Thematic, styleIndex)))
        return status;

    byte*                   data = &buffer[0];
    size_t                  dataSize = buffer.size();

    memset (&m_data, 0, sizeof (m_data));
    memcpy (&m_data, data, MIN (sizeof (m_data), dataSize));

    // ..Fix data from old versions.
    if (m_data.m_flags.m_steppedDisplay > ThematicSteppedDisplay_FastWithIsolines)
        m_data.m_flags.m_steppedDisplay = ThematicSteppedDisplay_Fast;

    if (m_data.m_colorScheme >= ThematicColorScheme_Max && m_data.m_colorScheme  != ThematicColorScheme_Custom)
        m_data.m_colorScheme  =ThematicColorScheme_BlueRed;

    // Read SubTypes/
    byte*                               pFragment = (byte*) data + sizeof (m_data);
    byte*                               pEnd      = (byte*) data + dataSize;
    bvector <ThematicLegendKey>     legendKeys;

    m_marginColor           = s_defaultMarginColor;
    m_legendValueStep       = 0.0;
    m_legendTransparency    = 0;

    while (pFragment < pEnd)
        {
        UInt16         fragmentSize, subType;

        memcpy (&fragmentSize, pFragment, sizeof (fragmentSize));
        if (pFragment + fragmentSize > pEnd)
            break;

        memcpy (&subType, pFragment + 2, sizeof (subType));

        size_t              fragmentDataSize = fragmentSize - 4;
        byte*               fragmentData     = pFragment + 4;
        switch (subType)
            {
            case ThematicDisplaySettingsSubType_GradientKey:
                if (fragmentDataSize == sizeof (ThematicGradientKey))
                    m_gradientKeys.push_back (*((ThematicGradientKey*) fragmentData)); 

                break;
                
            case ThematicDisplaySettingsSubType_LegendKey:
                if (fragmentDataSize == sizeof (ThematicLegendKey))
                    legendKeys.push_back (*((ThematicLegendKey*) fragmentData));
                
                break;

            case ThematicDisplaySettingsSubType_MarginColor:
                if (fragmentDataSize == sizeof (UInt32))
                    m_marginColor = *((UInt32*) fragmentData);

                break;

            case ThematicDisplaySettingsSubType_LegendTransparency:
                if (fragmentDataSize == sizeof (UInt32))
                    m_legendTransparency = *((UInt32*) fragmentData);

                break;

            case ThematicDisplaySettingsSubType_LegendValueStep:
                if (fragmentDataSize == sizeof (double))
                    m_legendValueStep = *((double*) fragmentData);

                break;
            }
        
        pFragment += fragmentSize;
        }

    m_legend = ThematicLegend (legendKeys, this);
    m_cookedRange.Set (m_data.m_range);
    m_colorMap.Init (*this, 512);
    
    return SUCCESS;
    }
     
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ThematicDisplaySettings::~ThematicDisplaySettings ()       {  ClearMaterial (); }

#ifdef NEEDS_WORK_MOVE_TO_DGNDISPLAY

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void            ThematicDisplaySettings::ClearMaterial ()
    {
    if (NULL != m_pMaterial)
        {
        qv_deleteRenderMaterial ((QvRendMatID) m_pMaterial);
        qv_deleteTexture ((QvTextureID) mdlMaterial_getMap (m_pMaterial->GetProperties(), MAPTYPE_Pattern, 0));
        DELETE_AND_CLEAR (m_pMaterial);
        }
    }

#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   createTempImageFile (BeFileNameR fileSpec, byte* texturePixels, Point2dR imageSize, WCharCP baseName, DgnModelP modelRef)
    {
    WString         tempDev, tempDir, name, extension = L"tif";
    static UInt32   s_dupeCount = 0;          // Never repeat during a session (to avoid reusing and confusing the texture cache).


    T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectoryBaseName().ParseName (&tempDev, &tempDir, NULL, NULL);

    do
        {
        name.Sprintf (L"%ls_tmpTxtr_%03d", baseName, s_dupeCount++);

        fileSpec.BuildName (tempDev.c_str(), tempDir.c_str(), name.c_str(), extension.c_str());
        }  while (BeFileName::DoesPathExist (fileSpec.GetName()));

    StatusInt       status;

    if (SUCCESS == (status = DgnPlatformLib::GetHost().GetMaterialAdmin()._CreateImageFileFromRGB (fileSpec.GetName(), texturePixels, imageSize, true)))
        addDgnModelTempFile (*modelRef, fileSpec.GetName());

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialCP      ThematicDisplaySettings::GetMaterial (ThematicDisplayStyleHandlerCR handler, ViewContextR context)
    {
    static const size_t     s_marginSize = 256, s_colorSize = 512;
    static double           s_defaultDiffuse = .6, s_defaultAmbient = 0.1, /*s_defaultSbmapecularExponent = .9,*/ s_defaultSpecular = .05, s_defaultFinish = 0.1;

    if (m_pMaterial.IsNull())
        {
        m_pMaterial = Material::Create (m_frustumModel->GetDgnProject());

        ThematicLegendCR        legend      = GetLegend ();
        size_t                  legendSize  = legend.GetNEntries ();
        double                  legendMin   = legend.GetMinValue (0, false), legendMax = legend.GetMaxValue (legendSize - 1, false);
    
        for (size_t i=0; i<s_marginSize; i++)
            m_texturePixels[i] = m_texturePixels[THEMATIC_TEXTURE_SIZE - i - 1] = m_data.m_flags.m_outOfRangeTransparent ? 0 : (0xff000000 | GetMarginColor());

        // Note: In Isolines mode we need the "stepped" material to display the "compressed" legend (when the full legend doesn't fit in the view),
        // Accurate, Fast, and Fast + Isolines need it for obvious reasons, which leaves only None to use the smooth texture.
        if (ThematicSteppedDisplay_None != GetFlags().m_steppedDisplay && 0 != legendSize)
            {
            double          pixelsPerValue = s_colorSize / (legendMax - legendMin);

            size_t          currentPixel = s_marginSize;

            for (size_t i=0; i < legendSize; i++)
                {
                size_t minPix = static_cast <size_t> (pixelsPerValue * (legend.GetMinValue (i, false) - legendMin));
                size_t maxPix = static_cast <size_t> (pixelsPerValue * (legend.GetMaxValue (i, false) - legendMin));

                for (size_t j=minPix; j < maxPix; j++, currentPixel++)
                    m_texturePixels[currentPixel] = (legend.GetVisible (i, false) ? 0xff000000 | legend.GetIntColor (i, false) : 0);
                }

            if (ThematicSteppedDisplay_FastWithIsolines == GetFlags().m_steppedDisplay)
                {
                for (size_t i=0; i<legendSize; i++)
                    {
                    if (legend.GetVisible (i, false))
                        {
                        double  fractionMin = (legend.GetMinValue (i, false) - legendMin) / (legendMax - legendMin);
                        double  fractionMax = (legend.GetMaxValue (i, false) - legendMin) / (legendMax - legendMin);

                        m_texturePixels[s_marginSize + (int)(fractionMin * s_colorSize)] = 0xff000000;
                        m_texturePixels[s_marginSize + (int)(fractionMax * s_colorSize)] = 0xff000000;
                        }
                    }
                }
            }
        else
            {
            for (int i=0; i<s_colorSize; i++)
                m_texturePixels[s_marginSize + i] = 0xff000000 | GetColor ((double) i, 0.0, (double) (s_colorSize - 1));
            }

        MaterialSettingsR       settings = m_pMaterial->GetSettingsR();

        settings.SetDiffuseIntensity (s_defaultDiffuse);
        settings.SetAmbientIntensity (s_defaultAmbient);
        settings.SetSpecularIntensity (s_defaultSpecular);
        settings.SetFinishScale (s_defaultFinish);

        MaterialMapP            map = settings.GetMapsR().AddMap (MaterialMap::MAPTYPE_Pattern);
        BeFileName              fileName;
        Point2d                 imageSize = { 1, THEMATIC_TEXTURE_SIZE};
        DgnModelP               modelRef = context.GetViewport()->GetViewController().GetTargetModel ();

        if (SUCCESS == createTempImageFile (fileName, (byte*) m_texturePixels, imageSize, L"ThematicDisplay", modelRef))
            {
            MaterialMapLayerR  layer = map->GetLayersR().GetTopLayerR();

            map->SetValue (1.0);
            layer.SetMode (MapMode::Parametric);
            layer.SetScale (1.0, 1.0, 1.0);
            layer.SetIsBackgroundTransparent (true);
            layer.SetFileName (fileName.GetName());
            }

#ifdef WIP_VANCOUVER_MERGE // material
        m_pMaterial->SetName (handler.GetName().c_str());
#endif

        DgnPlatformLib::GetHost().GetGraphicsAdmin()._SendMaterialToQV (*m_pMaterial, 0, context.GetViewport());
        }
    return m_pMaterial.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   ThematicDisplaySettings::_Save (ElementRefP elementRef, int styleIndex)
    {
    DisplayStyleHandlerSettings::_Save (elementRef, styleIndex);

    bvector<byte>                   buffer;
    ThematicLegend                      legend       = GetLegend ();
    bvector<ThematicLegendKey>      legendKeys   = legend.GetInternalKeys (); //Will not store anything if the key array is not initialized.

    buffer.resize (sizeof (m_data));
    memcpy (&buffer[0], &m_data, sizeof (m_data));                                 

    for (size_t i=0; i<m_gradientKeys.size(); i++)
        AddSettingsSubType (buffer, ThematicDisplaySettingsSubType_GradientKey, sizeof (ThematicGradientKey), &m_gradientKeys[i]); 

    for (size_t i=0; i<legendKeys.size (); i++)
        AddSettingsSubType (buffer, ThematicDisplaySettingsSubType_LegendKey, sizeof (ThematicLegendKey), &legendKeys[i]);

    AddSettingsSubType (buffer, ThematicDisplaySettingsSubType_MarginColor, sizeof (UInt32), &m_marginColor);
    AddSettingsSubType (buffer, ThematicDisplaySettingsSubType_LegendTransparency, sizeof (UInt32), &m_legendTransparency);
    AddSettingsSubType (buffer, ThematicDisplaySettingsSubType_LegendValueStep, sizeof (double), &m_legendValueStep);

    return SaveData (&buffer[0], buffer.size(), elementRef, DisplayStyleHandler_SettingsXAttributeSubId_Thematic, styleIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   11/2010
+---------------+--------------+---------------+---------------+---------------+------*/
void        ThematicDisplaySettings::GetGradient (ThematicGradientKey* gradientKeys, size_t maxKeys, size_t* outNKeys) const
    {
    if(m_data.m_colorScheme == ThematicColorScheme_Custom)
        {
        *outNKeys = MIN(maxKeys, m_gradientKeys.size());
        memcpy (gradientKeys, &m_gradientKeys[0], *outNKeys * sizeof(ThematicGradientKey));
        }
    else
        ThematicColorSchemeProvider (m_data.m_colorScheme).GradientKeys (maxKeys, outNKeys, gradientKeys);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void        ThematicDisplaySettings::SetGradient (ThematicGradientKey* gradientKeys, size_t maxKeys)
    {
    m_gradientKeys.resize (maxKeys);
    memcpy (&m_gradientKeys[0], gradientKeys, maxKeys * sizeof(ThematicGradientKey));
    m_colorMap.Init (*this, 512);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void       ThematicDisplaySettings::RefreshLegend ()
    {    
    double                              min   = 0, max = 0;
    
    GetRawRange (min, max); //Use the user-supplied range and throw out the min / max values if we have them.
    
    size_t                          nKeys = GetFlags ().m_valuesByStep ? MAX (1, static_cast <size_t> (ceil ((max-min) / GetLegendValueStep ()))) : GetSeedNLegendEntries ();
    bvector <ThematicLegendKey>     keys;
        
    for (size_t i=0; i <= nKeys; i++)
        {
        double              value;
        
        if (GetFlags ().m_valuesByStep)
            {
            //Note: If the legend step is greater than the display range, this will select the minimum on iteration 0 and the maximum on iteration 1, giving us
            // a key that matches the display range.
            if (i == nKeys)
                value = max;
            else
                value = min + i*GetLegendValueStep ();
            }
        else
            value = min + ((double)i / (double) nKeys) * (max-min);

        UInt32              intColor = GetColor ((double) i, 0.0, (double) (nKeys) - 1.0);    // Invalid for final key
        RgbColorDef         rgbColor = { (UInt8)(intColor & 0xFF), (UInt8)((intColor >> 8) & 0xFF), (UInt8)((intColor >> 16) & 0xFF) };
        ThematicLegendKey   key      = { value, rgbColor };

        keys.push_back (key);
        }

    m_legend = ThematicLegend (keys, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void      ThematicDisplaySettings::GetRange (double& min, double& max) const
    {
    ThematicLegendCR     legend = GetLegend ();

    if (legend.IsStatic ())
        {
        min = legend.GetMinValue (0, false);
        max = legend.GetMaxValue (legend.GetNEntries () - 1, false);
        }
    else
        m_data.m_range.Get (min, max);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ThematicRange const&    ThematicDisplaySettings::GetRange () const
    { 
    static ThematicRange    s_range;
    ThematicLegendCR        legend = GetLegend ();

    s_range = m_data.m_range;

    if (legend.IsStatic ())
        {
        s_range.SetMin (legend.GetMinValue (0, false));
        s_range.SetMax (legend.GetMaxValue (legend.GetNEntries () - 1, false));
        }

    return s_range; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayStyleHandlerSettingsPtr  ThematicDisplayStyleHandler::_GetSettings () const
    {
    return  ThematicDisplaySettings::Create();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayStyleHandlerKeyPtr  ThematicDisplayStyleHandler::_GetCacheKey (DgnModelR modelRef, DisplayStyleHandlerSettingsCP settings) const
    {
    ThematicDisplaySettingsCP thematicSettings = dynamic_cast <ThematicDisplaySettingsCP> (settings);
    
    if (NULL == thematicSettings)
        return NULL;

    return ThematicDisplayStyleHandlerKey::Create (*thematicSettings, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2010
+--------------+---------------+---------------+---------------+---------------+------*/
StatusInt  ThematicDisplayStyleHandler::_GetRawValueFromString (double& value, ThematicDisplaySettingsCR settings, WStringCR string, DgnModelR modelRef) const
    { 
    return 1 == BE_STRING_UTILITIES_SWSCANF (string.c_str(), L"%lf", &value) ? SUCCESS : ERROR;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2010
+--------------+---------------+---------------+---------------+---------------+------*/
WString  ThematicDisplayStyleHandler::_GetStringFromRawValue (double value, ThematicDisplaySettingsCR, DgnModelR modelRef, bool includeUnits) const
    {
    WString     string;

    string.Sprintf (L"%.3f", value);
    return string;
    }

#ifdef NEEDS_WORK_HITINFO

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IViewHandlerHitInfo*  ThematicDisplayStyleHandler:: _GetViewHandlerHitInfo (DisplayStyleHandlerSettingsCP settings, DPoint3dCR hitPoint) const 
    { 
    ThematicDisplaySettingsCP       scalarMeshSettings;

    if (NULL != (scalarMeshSettings =  dynamic_cast <ThematicDisplaySettingsCP> (settings)))
        return new ThematicDisplayHitInfo (*this, *scalarMeshSettings, hitPoint);

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void    ThematicDisplayHitInfo::OnGetInfoString (WStringR pathDescr, DisplayPath const& path, WCharCP delimiter) const
    {
    WString         string;
    HitPathCP       hitPath;
    ViewportP       viewport;

    if (NULL != (hitPath = dynamic_cast <HitPathCP> (&path)) &&
        NULL != (viewport = hitPath->GetViewport()))
        {
        ElementHandle  cursorElement (hitPath->GetCursorElem(), hitPath->GetRoot());

        if (NULL != cursorElement.GetDisplayHandler() &&
            SUCCESS == m_handler.GetHitInfoString (string, *hitPath, m_hitPoint, m_settings, *viewport, cursorElement, delimiter))
            {
            pathDescr.insert (0, delimiter);
            pathDescr.insert (0, string);
            }
        }
    }
#endif



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   ThematicMeshDisplayStyleHandler::GetUorsFromString (double& uors, WStringCR string, DgnModelR modelRef)
    {
    DistanceParserPtr   distanceParser = DistanceParser::Create(modelRef);
    
    return distanceParser->ToValue (uors, string.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString     ThematicMeshDisplayStyleHandler::GetStringFromUors (double uors, DgnModelR modelRef, bool includeUnits)
    {
    return DistanceFormatter::Create(modelRef)->ToString (uors);        // includeUnits ?? 
    }


#ifdef NEEDS_WORK_LEGEND

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void        ThematicLegendElementHandler::_GetTypeName (WStringR string, UInt32 desiredLength)                        
    {  
#ifdef NEEDS_WORK_STRINGS
    char msg [1024];

    mdlResource_getUstationString (msg, sizeof (msg), (MessageListNumber) RSCID_ThematicHandlerMessages, THEMATIC_HANDLER_LegendTypeName);
    string.Assign (msg);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   ThematicLegendElementHandler::_OnTransform (EditElementHandleR ElementHandle, TransformInfoCR trans)
    {
    StatusInt   status;

    if (SUCCESS != (status = DisplayHandler::_OnTransform (ElementHandle, trans)))
        return status;

    Transform                   newTransform = *trans.GetTransform (), oldTransform;
    ThematicLegendElementData   data;

    data = *GetData (ElementHandle);

    oldTransform.initFrom (&data.m_rotation, &data.m_origin);
    newTransform.productOf (&newTransform, &oldTransform);
    newTransform.getMatrix (&data.m_rotation);
    newTransform.getTranslation (&data.m_origin);

    SetData (ElementHandle, data);

    return SUCCESS; 
    }

#ifdef NEEDS_WORK_MOVE_TO_MSTNPLATFORM

PlaceThematicLegendTool::PlaceThematicLegendTool () : MstnPrimitiveTool (CMD_PLACE_LEGEND, MS_CMDNAME_PlaceThematicDisplayLegend, 141),  m_pointNumber (0)  { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PlaceThematicLegendTool::InstallToolImplementation ()
    {
    StatusInt status = __super::InstallToolImplementation ();
    mdlAccuDraw_setEnabledState (true);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void    PlaceThematicLegendTool::OnRestartCommand ()
    {
    (new PlaceThematicLegendTool ())->InstallTool ();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   createLegendElement (EditElementHandleR eeh, DPoint3d points[2], int viewNumber)
    {
    RotMatrix           rMatrix, inverse;
    DPoint3d            viewPoints[2], origin;
    DRange3d            viewRange;

    mdlRMatrix_fromView (&rMatrix, viewNumber, TRUE);
    inverse.inverseOf (&rMatrix);
    rMatrix.multiply (viewPoints, points, 2);
    viewRange.initFrom (viewPoints, 2);
    inverse.multiply (&origin, &viewRange.high);

    return  ThematicLegendElementHandler::CreateElement (eeh, origin, viewRange.high.x - viewRange.low.x, viewRange.high.y - viewRange.low.y, inverse, ACTIVEMODEL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool PlaceThematicLegendTool::OnDataButton(MstnButtonEventCP ev)
    {
    m_points[m_pointNumber++] = *ev->GetPoint();

    switch (m_pointNumber)
        {
        case 1:
            BeginComplexDynamics ();
            mdlOutput_promptNum (26);       // Enter opposite corner.
            break;

        case 2:
            {
            EditElementHandle      eeh;

            if (SUCCESS == createLegendElement (eeh, m_points, ev->GetViewNum()))
                eeh.AddToModel (ACTIVEMODEL);

            EndComplexDynamics ();
            mdlOutput_promptNum (141);
            m_pointNumber = 0;
            break;
            }
         }
    

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void    PlaceThematicLegendTool::OnComplexDynamics (MstnButtonEventCP ev) 
    {
    if (1 == m_pointNumber)
        {
        m_points[1] = *ev->GetPoint();

        EditElementHandle      eeh;

        if (SUCCESS == createLegendElement (eeh, m_points, ev->GetViewNum()))
            RedrawElems(ev->GetViewport(), DgnDrawMode::TempDraw, DrawPurpose::Dynamics).DoRedraw (eeh);

        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool    PlaceThematicLegendTool::OnResetButton (MstnButtonEventCP ev)
    {
    ExitTool();
    return true;
    }
#endif

void    ThematicLegendElementHandler::_GetOrientation (ElementHandleCR eh, RotMatrixR rotation) { rotation = GetData (eh)->m_rotation;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static void    drawOutline (ViewContextR context, ThematicLegendElementData& data, UInt8 transparency)
    {
    DPoint3d    lowerLeft  = { -data.m_width, -data.m_height, 0}, 
                upperRight = { 0, 0, 0};
            
    drawRectangle (lowerLeft, upperRight, 0, 0xffffff, transparency, context);
    }

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2011
+===============+===============+===============+===============+===============+======*/
struct  ThematicLegendStroker : IStrokeForCache
{
private:
    ThematicLegendElementHandler&   m_ElementHandler;

public:

ThematicLegendStroker (ThematicLegendElementHandler& ElementHandler) : m_ElementHandler (ElementHandler) {}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _StrokeForCache (CachedDrawHandleCR dh, ViewContextR context, double pixelSize) override
    {
    if (NULL == context.GetViewport())
        return;

    CookedDisplayStyleCP                displayStyle = context.GetCurrentCookedDisplayStyle();
    ThematicMeshDisplayStyleHandlerCP   thematicHandler;
    ThematicDisplaySettingsCP           settings;
    static double                       s_epsilon = 1E-6;
    XAttributeHandlerId                 handlerId (XATTRIBUTEID_DisplayStyleHandler, DisplayStyleHandlerSubID_ThematicLegendElementData);
    ElementHandle::XAttributeIter       xAttr (el, handlerId, 0);
    ThematicLegendElementData           data;

    if (xAttr.IsValid () && xAttr.GetSize() >= sizeof (data))
        {
        memcpy (&data, xAttr.PeekData(), sizeof(data));
        
        RotMatrix       rotation;
        UInt8           transparency = 0; // NEEDS_WORK_TRANSPARENCY ...static_cast <UInt8> (255.0 * mdlElement_getNetTransparency (el.GetElementCP (), el.GetDgnModel ())); 

        m_ElementHandler._GetOrientation(el, rotation);

        // Draw the outline if we don't have enough information to draw anything else, or we're drawing for pick (often both)
        if (NULL == displayStyle ||
            NULL == (thematicHandler = dynamic_cast <ThematicMeshDisplayStyleHandlerCP> (displayStyle->m_displayHandler)) ||
            NULL == (settings = dynamic_cast <ThematicDisplaySettingsP> (context.GetDisplayStyleHandlerSettings())))
            {                                                                                                              
            drawOutline (context, data, transparency);
            }
        else
            {
            double          widthEms = 0;
            DgnModelP    modelRef = context.GetViewport ()->GetRootModel ();
            
            // NEEDSWORK / BUG ALERT: We don't know what size to use here (since it's expressed in ems), and although it probably doesn't happen 
            // much the maximum label length can be different than the "real" one, giving us a legend with incorrectly sized text.
            thematicHandler->ComputeLabelsAndSize (*settings, DBL_MAX, *modelRef, NULL, &widthEms, NULL, NULL);

            if (data.m_width < s_epsilon || data.m_height < s_epsilon) 
                return;
            
            double      scale = data.m_width / widthEms;
            double      maxHeight = data.m_height / scale;
            Transform   elementTransform;

            rotation.scaleRows (&rotation, scale, scale, scale);
            elementTransform.initFrom (&rotation, &data.m_origin);

            thematicHandler->DrawLegend (context, maxHeight, true, &transparency);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _WantLocateByStroker () override {return false;} // Don't call _StrokeForCache, locate interior by QvElem...

}; // ThematicLegendStroker

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void    ThematicLegendElementHandler::_Draw (ElementHandleCR el, ViewContextR context) 
    {
    CookedDisplayStyleCP                    displayStyle = NULL;
    ThematicMeshDisplayStyleHandlerCP       thematicHandler = NULL;
    ThematicDisplaySettingsCP               settings = NULL;
    DrawPurpose                             drawPurpose = context.GetDrawPurpose ();

    if (NULL == context.GetViewport())
        return;

    if (DrawPurpose::ChangedPre == drawPurpose)
        {
        // NOTE: XAttributes are already updated. - just force heal.
        context.DrawElementRange (el.GetElementCP ());
        return;
        }

    ThematicLegendElementData const*    pData = GetData (el);
    Transform                           elementTransform;
    
    if (NULL != pData)
        {    
        ThematicLegendElementData data = *pData;

        if (NULL == (displayStyle = context.GetCurrentCookedDisplayStyle()) ||
            NULL == (thematicHandler = dynamic_cast <ThematicMeshDisplayStyleHandlerCP> (displayStyle->m_displayHandler)) ||
            NULL == (settings = dynamic_cast <ThematicDisplaySettingsP> (context.GetDisplayStyleHandlerSettings()))) 
            {
            elementTransform.initFrom (&data.m_rotation, &data.m_origin);
            }
        else
            {
            RotMatrix       rotation;
            double          widthEms = 0;
            DgnModelP    modelRef = context.GetViewport ()->GetRootModel ();

            thematicHandler->ComputeLabelsAndSize(*settings, DBL_MAX, *modelRef, NULL, &widthEms, NULL, NULL);

            double      scale = data.m_width / widthEms;

            _GetOrientation(el, rotation);
            rotation.scaleRows (&rotation, scale, scale, scale);
            elementTransform.initFrom (&rotation, &data.m_origin);
            }

        IViewDrawR  viewDraw = context.GetIViewDraw();

        initContextForDrawLegend (context);
        context.PushTransform (elementTransform);

        if (DrawPurpose::Pick == drawPurpose)
            {
            context.DrawCached (el, ThematicLegendStroker (*this), 1);
            }
        else
            {
            // TR# 317886 - dont cache legend - it can get out of date with display style changes and there won't be more than one so performance shouldn't be an issue.
            if (!doHilite (context))
                viewDraw.ActivateOverrideMatSymb (NULL);

            CachedDrawHandle dh(el);
            ThematicLegendStroker (*this)._StrokeForCache (dh, context, 0.0);
            }

        context.PopTransformClip ();
        viewDraw.PopRenderOverrides ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ThematicLegendElementData const* ThematicLegendElementHandler::GetData (ElementHandleCR eh)
    {
    ElementHandle::XAttributeIter  dataIterator   (eh, XAttributeHandlerId (XATTRIBUTEID_DisplayStyleHandler, DisplayStyleHandlerSubID_ThematicLegendElementData), 0);

    return (ThematicLegendElementData  const*) dataIterator.PeekData ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   ThematicLegendElementHandler::SetData (EditElementHandleR eeh, ThematicLegendElementData const& data)
    {
    return eeh.ScheduleWriteXAttribute (XAttributeHandlerId (XATTRIBUTEID_DisplayStyleHandler, DisplayStyleHandlerSubID_ThematicLegendElementData), 0, sizeof (data), &data);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool        ThematicLegendElementHandler::_IsPlanar (ElementHandleCR eh, DVec3dP normal, DPoint3dP point, DVec3dCP inputDefaultNormal)
    {
    ThematicLegendElementData data = *GetData (eh);

    data.m_rotation.getColumn (normal, 2);
    *point = data.m_origin;

    return true;
    }


#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
size_t           ThematicLegend::_OrderIndex (size_t i) const
    {
    return m_parent->GetFlags ().m_invertLegend ? (GetNEntries () - (i + 1)) : i; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ThematicLegend::IsStatic () const 
    { 
    return 0 != m_keys.size () && m_parent->GetFlags ().m_customLegend;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
RgbColorDef const&  ThematicLegend::GetRgbColor (size_t i, bool ordered) const                        
    { 
    if (ordered)
        i = _OrderIndex (i);

    if (IsStatic ())
        return m_keys[i].m_color;
    
    double          min      = 0, max = 0;
    
    m_parent->GetRawRange (min, max);
    size_t                  size       = m_parent->GetFlags ().m_valuesByStep ? static_cast <size_t> (ceil ((max-min)/m_parent->GetLegendValueStep ())) : GetNEntries ();
    UInt32                  intColor   = m_parent->GetColor ((double) i, 0.0, (double) size - 1.0);
    RgbColorDef             rgbColor   = { (UInt8)(intColor & 0xFF), (UInt8)((intColor >> 8) & 0xFF), (UInt8)((intColor >> 16) & 0xFF) };
    static  RgbColorDef     s_rgbColor;

    return (s_rgbColor = rgbColor);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
double              ThematicLegend::GetMinValue (size_t i, bool ordered) const
    { 
    if (ordered)
        i = _OrderIndex (i);

    if (IsStatic ())
        return m_keys[i].m_value;
        
    double          min      = 0, max = 0;

    m_parent->GetRawRange (min, max);

    if (m_parent->GetFlags ().m_valuesByStep)
        {
        return MIN (max, min + i * m_parent->GetLegendValueStep ());
        }
    else
        {
        double          fraction = (double) i / (double)GetNEntries ();
        
        return min + fraction * (max - min);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
double              ThematicLegend::GetMaxValue (size_t i, bool ordered) const
    { 
    if (ordered)
        i = _OrderIndex (i);

    return GetMinValue (i+1, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bvector <double> const& ThematicLegend::GetAllValues (bool ordered) const      
    {
    static bvector <double>    s_values;
    
    s_values.clear ();

    if (ordered && m_parent->GetFlags ().m_invertLegend)
        {
        for (Int32 i= (Int32) GetNEntries () - 1; i >= 0; i--)
            s_values.push_back (GetMaxValue (i, false));

        s_values.push_back (GetMinValue (0, false));
        }
    else
        {
        for (size_t i=0; i<GetNEntries (); i++)
            s_values.push_back (GetMinValue (i, false));

        s_values.push_back (GetMaxValue (GetNEntries ()-1, false));
        }

    return s_values;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bvector <RgbColorDef> const& ThematicLegend::GetAllColors (bool ordered) const
    {
    static bvector <RgbColorDef>    s_colors;
    
    s_colors.clear ();

    for (size_t i=0; i<GetNEntries (); i++)
        s_colors.push_back (GetRgbColor (i, ordered));

    return s_colors;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bvector <bool> const& ThematicLegend::GetAllVisible (bool ordered) const
    {
    static bvector <bool>   s_visible;

    s_visible.clear ();

    for (size_t i=0; i<GetNEntries (); i++)                                                                                                                                     
        s_visible.push_back (GetVisible (i, ordered));

    return s_visible;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void    ThematicLegend::SetRgbColor (size_t i, RgbColorDef const& color, bool ordered)
    { 
    if (ordered)
        i = _OrderIndex (i);

    if (0 == m_keys.size ())
        m_parent->RefreshLegend ();

    m_keys[i].m_color = color;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void    ThematicLegend::SetMinValue (size_t i, double value, bool ordered)
    { 
    if (ordered)
        i = _OrderIndex (i);

    if (0 == m_keys.size ())
        m_parent->RefreshLegend ();

    m_keys[i].m_value = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void    ThematicLegend::SetMaxValue (size_t i, double value, bool ordered)
    { 
    if (ordered)
        i = _OrderIndex (i);

    if (0 == m_keys.size ())
        m_parent->RefreshLegend ();

    m_keys[i+1].m_value = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ThematicLegend::GetVisible (size_t i, bool ordered) const
    {
    if (ordered)
        i = _OrderIndex (i);

    if (IsStatic ())
        return !m_keys[i].m_flags.m_hidden;
    
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void    ThematicLegend::SetVisible (size_t i, bool visible, bool ordered)
    {
    if (ordered)
        i = _OrderIndex (i);

    if (0 == m_keys.size ())
        m_parent->RefreshLegend ();

    m_keys[i].m_flags.m_hidden = !visible;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
size_t    ThematicLegend::GetNEntries () const
    {
    if (m_parent->GetFlags ().m_valuesByStep && !m_parent->GetFlags ().m_customLegend)
        {
        double          min      = 0, max = 0, step = m_parent->GetLegendValueStep ();

        m_parent->GetRawRange (min, max);

        return static_cast <size_t> (ceil ((max-min)/step));
        }
    else
        return (m_keys.size () == 0) ? 10 : m_keys.size () - 1; 
    }


// static WCharCP    XML_ELEMENT_DEMFilter                    = L"DEMFilter";
// static WCharCP    XML_ELEMENT_style                        = L"style";
// static WCharCP    XML_ELEMENT_hillShade                    = L"hillShade";
// static WCharCP    XML_ELEMENT_verticalExaggeration         = L"verticalExaggeration";
// static WCharCP    XML_ELEMENT_clipToEndValues              = L"clipToEndValues";
// static WCharCP    XML_ELEMENT_defaultColor                 = L"defaultColor";
// static WCharCP    XML_ELEMENT_red                          = L"red";
// static WCharCP    XML_ELEMENT_green                        = L"green";
// static WCharCP    XML_ELEMENT_blue                         = L"blue";
// static WCharCP    XML_ELEMENT_upperRanges                  = L"upperRanges";
// static WCharCP    XML_ELEMENT_upperRange                   = L"upperRange";
// static WCharCP    XML_ELEMENT_value                        = L"value";
// static WCharCP    XML_ELEMENT_state                        = L"state";
// static WCharCP    XML_ELEMENT_color                        = L"color";

typedef bvector <RgbColorDef> ThematicMeshColorArray;
typedef bvector <bool>        ThematicMeshBoolArray;

#ifdef NOTYET_DESCARTES_XML

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static WCharCP   getStyleVal (DisplayStyleHandlerCP handler)
    {
    switch (handler->GetHandlerId().GetMinorId())
        {
        case DisplayStyleHandlerSubID_SlopeAngle:
        case DisplayStyleHandlerSubID_SlopePercent:
            return L"SlopePercent";

        case DisplayStyleHandlerSubID_Aspect:
            return L"Aspect";

        case DisplayStyleHandlerSubID_Height:
            return L"Elevation";

        default:
            return NULL;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static void    parseColor (RgbColorDef& rgb, BeXmlNodeP node)
    {
    BeXmlNodeP          childNode = NULL;
    BeXmlNodeListRef*   childList = NULL;
    WChar               childName[1024];

    while (SUCCESS == mdlXMLDomNodeList_getNextChildElement (&childNode, childName, &childList, node))
        {
        WChar     nodeVal[1024];
        Int32       valBufLen = _countof (nodeVal);
        
        mdlXMLDomElement_getValue (nodeVal, &valBufLen, childNode, XMLDATATYPE_WIDESTRING);

        if (0 == wcscmp (childName, XML_ELEMENT_red))
            rgb.red = static_cast <UInt8> (BeStringUtilities::Wcstol (nodeVal, NULL, 10));

        else if (0 == wcscmp (childName, XML_ELEMENT_green))
            rgb.green = static_cast <UInt8> (BeStringUtilities::Wcstol (nodeVal, NULL, 10));

        else if (0 == wcscmp (childName, XML_ELEMENT_blue))
            rgb.blue = static_cast <UInt8> (BeStringUtilities::Wcstol (nodeVal, NULL, 10));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static void    parseUpperRange (BeXmlNodeR upperRangeNode, ThematicMeshColorArray& colors, ThematicMeshDoubleArray& values, ThematicMeshBoolArray& visible)
    {
    BeXmlNodeR      childNode = NULL;
    BeXmlNodeListR  childList = NULL;
    WChar         childName[1024];

    while (SUCCESS == mdlXMLDomNodeList_getNextChildElement (&childNode, childName, &childList, upperRangeNode))
        {
        WChar     nodeVal[1024];
        Int32       valBufLen = _countof (nodeVal);
        
        mdlXMLDomElement_getValue (nodeVal, &valBufLen, childNode, XMLDATATYPE_WIDESTRING);

        if (0 == wcscmp (childName, XML_ELEMENT_value))
            values.push_back (wcstod (nodeVal, NULL));

        else if (0 == wcscmp (childName, XML_ELEMENT_state))
            visible.push_back (0 == wcscmp (nodeVal, L"true"));

        else if (0 == wcscmp (childName, XML_ELEMENT_color))
            {
            RgbColorDef color;

            parseColor (color, childNode);
            colors.push_back (color);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static void    parseDescartes 
(
XmlDomRef&                  DOM,
BeXmlNodeR&                 rootNode,
bool&                       clipToEndValues,
RgbColorDef&                marginColor,
bvector <RgbColorDef>&  colors,
bvector <double>&       values,
bvector <bool>&         visible,
WCharP                    styleVal
)
    {
    BeXmlNodeR                  rootChildNode       = NULL;
    BeXmlNodeListR              rootChildNodeList   = NULL;
    WChar                     rootChildNodeName[1024];

    while (SUCCESS == mdlXMLDomNodeList_getNextChildElement (&rootChildNode, rootChildNodeName, &rootChildNodeList, rootNode))
        {
        WChar     nodeVal[1024];
        Int32       valBufLen = _countof (nodeVal);
        
        mdlXMLDomElement_getValue (nodeVal, &valBufLen, rootChildNode, XMLDATATYPE_WIDESTRING);
        
        if (0 == wcscmp (rootChildNodeName, XML_ELEMENT_style))
            wcscpy (styleVal, nodeVal);

        else if (0 == wcscmp (rootChildNodeName, XML_ELEMENT_clipToEndValues))
            clipToEndValues = (0 == wcscmp (nodeVal, L"true"));

        else if (0 == wcscmp (rootChildNodeName, XML_ELEMENT_defaultColor))
            parseColor (marginColor, rootChildNode);

        else if (0 == wcscmp (rootChildNodeName, XML_ELEMENT_upperRanges))
            {
            BeXmlNodeR                  upperRangeNode     = NULL;
            BeXmlNodeListR              upperRangeNodeList = NULL;
            WChar                     upperRangeNodeName[1024];
            
            while (SUCCESS == mdlXMLDomNodeList_getNextChildElement (&upperRangeNode, upperRangeNodeName, &upperRangeNodeList, rootChildNode))
                parseUpperRange (upperRangeNode, colors, values, visible);
            }
        }   
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool            isAscending (ThematicMeshDoubleArray const& values)
    {
    for (size_t i=0; i+1 < values.size (); i++)
        if (values[i] > values[i+1])
            return false;

    return true;
    }

#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ThematicError   ThematicDisplaySettings::ReadDescartes (WCharCP filename, DisplayStyleHandlerCP handler)
    {
#ifdef NOTYET_DESCARTES_XML
    XmlDomRef       DOM = NULL;
    BeXmlNodeR      rootNode;
    
    mdlXMLDom_create (&DOM);
    
    if (SUCCESS != mdlXMLDom_load (DOM, FILESPEC_LOCAL, filename, NULL, NULL))
        return ThematicError_ImportOpenFailed;

    mdlXMLDom_getRootElement (&rootNode, DOM);
    
    //Values to extract
    bool                        clipToEndValues     = false;
    RgbColorDef                 marginColor;
    ThematicMeshColorArray      colors;
    ThematicMeshDoubleArray     values;
    ThematicMeshBoolArray       visible;
    WChar                     styleVal[1024];

    parseDescartes (DOM, rootNode, clipToEndValues, marginColor, colors, values, visible, styleVal);

    WCharCP handlerDescartesStyle = getStyleVal (handler);

    BeAssert (NULL != handlerDescartesStyle && "This thematic style has no Descartes equivalent. The option to export should not be available.");

    // Trying to import settings from a different thematic type than the one in the handler.
    if (0 != wcscmp (styleVal, handlerDescartesStyle))
        return ThematicError_ImportHandlerMismatch;

    mdlXMLDom_free (DOM);

    //It appears that Descartes uses descending order by default, and we want ascending. Make the data ascending
    //regardless of original order.
    if (!isAscending (values))
        {
        std::reverse (values.begin (), values.end ());
        std::reverse (colors.begin (), colors.end ());
        std::reverse (visible.begin (), visible.end ());
        }

    // Neither the original nor reversed array is ascending. Thus we can't make a valid legend out of it.
    if (!isAscending (values))
        return ThematicError_ImportInconsistentValues;

    double min=0, max=0, firstVal=values.front (), lastVal=values.back ();

    GetRawRange (min, max);

    // The bottom two values represent one key. Combine them. If the minimum key is a different color
    // than the one above it, that information is lost. Since the bottom color is only used for pixels exactly
    // at its value (in Descartes), it's not very useful in Microstation where values are continuous.
    if (clipToEndValues)
        {
        // Our exporter should not trip this assert, but if Descartes does, we'll need extra handling when there's only key.
        BeAssert (colors.size () > 1);
        colors = ThematicMeshColorArray (colors.begin () + 1, colors.end ());
        visible = ThematicMeshBoolArray (visible.begin () + 1, visible.end ());
        }
    // When "Clip to End Values" is off, use the full range (add a value for the minimum).
    // TODO: Consider whether we should remove the bottom key when it would have 0 width. This would be more
    // consistent with clipToEndValues=true case, but might delete a key that the user wants if they're still editing the model.
    else
        {
        double newMin = MIN (min, values[0]), newMax = MAX (max, values[values.size () - 1]);
        values.insert (values.begin (), newMin);
        values[values.size () - 1] = newMax;
        }

    bvector <ThematicLegendKey> keys;

    for (size_t i=0; i<values.size (); i++)
        {
        ThematicLegendKey key;

        key.m_value = values[i];
        key.m_flags.m_hidden = !visible[i];
        
        if (i < colors.size ())
            key.m_color = colors[i];

        keys.push_back (key);
        }
    
    m_data.m_flags.m_fixedMinimum = m_data.m_flags.m_fixedMaximum = clipToEndValues;
    m_data.m_flags.m_customLegend = TRUE;
    m_data.m_range.Set (firstVal, lastVal);
    m_legend = ThematicLegend (keys, this);
    SetMarginColorRGB (marginColor);
#endif

    return (ThematicError)SUCCESS;
    }

#ifdef NOTYET_DESCARTES_XML
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static void    makeColorNode (XmlDomRef& DOM, BeXmlNodeR& defaultColorNode, UInt32 defaultColor)
    {
    BeXmlNodeR  redNode = NULL, greenNode = NULL, blueNode = NULL;
    WChar     redStr [4], greenStr [4], blueStr [4];

    mdlXMLDom_createElement (&redNode,   DOM, XML_ELEMENT_red);
    mdlXMLDom_createElement (&greenNode, DOM, XML_ELEMENT_green);
    mdlXMLDom_createElement (&blueNode,  DOM, XML_ELEMENT_blue);

    _itow_s (defaultColor & 0xFF,        redStr,   _countof (redStr), 10);
    _itow_s ((defaultColor >> 8) & 0xFF, greenStr, _countof (greenStr), 10);
    _itow_s ((defaultColor >> 16)& 0xFF, blueStr,  _countof (blueStr), 10);

    mdlXMLDomElement_setValue (redNode, redStr);
    mdlXMLDomElement_setValue (greenNode, greenStr);
    mdlXMLDomElement_setValue (blueNode, blueStr);

    mdlXMLDomElement_appendChild (defaultColorNode, redNode);
    mdlXMLDomElement_appendChild (defaultColorNode, greenNode);
    mdlXMLDomElement_appendChild (defaultColorNode, blueNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static void makeRangeNode (XmlDomRef& DOM, BeXmlNodeR& rootNode, double value, bool visible, RgbColorDef const& color)
    {
    BeXmlNodeR  rangeNode = NULL, stateNode = NULL, valueNode = NULL, colorNode = NULL;
    WChar     *stateStr = visible ? L"true" : L"false",
                valueStr[128];
    
    swprintf_s (valueStr, _countof (valueStr), L"%f", value);

    mdlXMLDom_createElement (&rangeNode, DOM, XML_ELEMENT_upperRange);
    mdlXMLDom_createElement (&stateNode, DOM, XML_ELEMENT_state);
    mdlXMLDom_createElement (&valueNode, DOM, XML_ELEMENT_value);
    mdlXMLDom_createElement (&colorNode, DOM, XML_ELEMENT_color);

    mdlXMLDomElement_setValue (stateNode, stateStr);
    mdlXMLDomElement_setValue (valueNode, valueStr);

    UInt32  intColor = color.red + (color.green << 8) + (color.blue << 16);

    makeColorNode (DOM,  colorNode, intColor);

    mdlXMLDomElement_appendChild (rangeNode, valueNode);
    mdlXMLDomElement_appendChild (rangeNode, stateNode);
    mdlXMLDomElement_appendChild (rangeNode, colorNode);
    mdlXMLDomElement_appendChild (rootNode, rangeNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static void    makeRangesNode (XmlDomRef& DOM, BeXmlNodeR& rootNode, ThematicDisplaySettings const& settings, bool clipToEndValues)
    {
    ThematicMeshDoubleArray     values = settings.GetLegend ().GetAllValues (false);
    ThematicMeshDoubleArray     maxValues (values.begin () + 1, values.end ());
    ThematicMeshColorArray      colors = settings.GetLegend ().GetAllColors (false);
    ThematicMeshBoolArray       visible = settings.GetLegend ().GetAllVisible (false);

    for (Int32 i = maxValues.size () - 1; i >= 0; i--)
        makeRangeNode (DOM, rootNode, maxValues[i], visible[i], colors[i]);

    // Extra key to indicate minimum value. Combined with clipToEndValues, in Descartes
    // this means values at and above our minimum will be displayed thematically. In our importer,
    // it helps us recover the original minimum.
    if (clipToEndValues)
        makeRangeNode (DOM, rootNode, values[0], visible[0], colors[0]);
    }
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void    ThematicDisplaySettings::WriteDescartes (WCharCP  filename, DisplayStyleHandlerCP handler)
    {
#ifdef NOTYET_DESCARTES_XML
    XmlDomRef       DOM = NULL;
    BeXmlNodeR      rootNode = NULL, styleNode = NULL, hillShadeNode = NULL, verticalExaggerationNode = NULL, clipToEndValuesNode = NULL, defaultColorNode = NULL, upperRangesNode = NULL;
    
    mdlXMLDom_create (&DOM);
    mdlXMLDom_createElement (&rootNode,                 DOM, XML_ELEMENT_DEMFilter);
    mdlXMLDom_createElement (&styleNode,                DOM, XML_ELEMENT_style);
    mdlXMLDom_createElement (&hillShadeNode,            DOM, XML_ELEMENT_hillShade);
    mdlXMLDom_createElement (&verticalExaggerationNode, DOM, XML_ELEMENT_verticalExaggeration);
    mdlXMLDom_createElement (&clipToEndValuesNode,      DOM, XML_ELEMENT_clipToEndValues);
    mdlXMLDom_createElement (&defaultColorNode,         DOM, XML_ELEMENT_defaultColor);
    mdlXMLDom_createElement (&upperRangesNode,          DOM, XML_ELEMENT_upperRanges);

    mdlXMLDom_setRootElement (DOM, rootNode);

    bool            clipToEndValues         = IsMinFixed () || IsMaxFixed ();
    WCharCP       styleVal                = getStyleVal (handler),
                    hillShadeVal            = L"true",
                    verticalExaggerationVal = L"1",
                    clipToEndValuesVal      = clipToEndValues ? L"true" : L"false";
    UInt32          defaultColor            = GetMarginColor ();

    BeAssert (NULL != styleVal && "This thematic style has no Descartes equivalent. The option to export should not be available.");

    mdlXMLDomElement_setValue (styleNode, styleVal);
    mdlXMLDomElement_setValue (hillShadeNode, hillShadeVal);
    mdlXMLDomElement_setValue (verticalExaggerationNode, verticalExaggerationVal);
    mdlXMLDomElement_setValue (clipToEndValuesNode, clipToEndValuesVal);
    
    makeColorNode (DOM, defaultColorNode, defaultColor);
    makeRangesNode (DOM, upperRangesNode, *this, clipToEndValues);

    mdlXMLDomElement_appendChild (rootNode, styleNode);
    mdlXMLDomElement_appendChild (rootNode, hillShadeNode);
    mdlXMLDomElement_appendChild (rootNode, verticalExaggerationNode);
    mdlXMLDomElement_appendChild (rootNode, clipToEndValuesNode);
    mdlXMLDomElement_appendChild (rootNode, defaultColorNode);
    mdlXMLDomElement_appendChild (rootNode, upperRangesNode);

    mdlXMLDom_save (DOM, FILESPEC_LOCAL, filename, NULL, NULL, L"utf-8", FALSE, TRUE, TRUE);    
    mdlXMLDom_free (DOM);
#endif
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ThematicDisplaySettings::_Equals (DisplayStyleHandlerSettingsCP rhs) const
    {
    ThematicDisplaySettingsCP   thematic = NULL;

    return  DisplayStyleHandlerSettings::_Equals (rhs) &&
            NULL != (thematic = dynamic_cast <ThematicDisplaySettingsCP> (rhs)) && 
            (0 == memcmp (&m_data, &thematic->m_data, sizeof (m_data))) &&
             m_gradientKeys.size () == thematic->m_gradientKeys.size () &&
             0 == memcmp (&m_gradientKeys[0], &thematic->m_gradientKeys[0], sizeof (m_gradientKeys[0]) * m_gradientKeys.size ()) &&
             m_legend.Equals (thematic->m_legend) &&
             m_marginColor          == thematic->m_marginColor &&
             m_legendTransparency   == thematic->m_legendTransparency &&
             m_legendValueStep      == thematic->m_legendValueStep;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ThematicDisplaySettings::ThematicDisplaySettings (ThematicDisplaySettingsCR other) 
    : DisplayStyleHandlerSettings (other)
    {
    m_data                  = other.m_data;
    m_gradientKeys          = other.m_gradientKeys;
    m_legend                = other.m_legend;
    m_marginColor           = other.m_marginColor;
    m_legendTransparency    = other.m_legendTransparency;
    m_legendValueStep       = other.m_legendValueStep;
    m_pMaterial             = NULL; // If we copy the pointer we get ownership problems and I'm not sure how to clone this. For now let the material be regenerated when needed.
    m_cookedRange           = other.m_cookedRange;
    m_colorMap              = other.m_colorMap;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ThematicDisplayStyleHandlerKey::ThematicDisplayStyleHandlerKey (ThematicDisplaySettingsCR settings, ThematicDisplayStyleHandlerCR handler) : DisplayStyleHandlerKey (handler)
    {
    m_accurateSteppedDisplay    = (ThematicSteppedDisplay_Accurate == settings.GetFlags().m_steppedDisplay);
    m_isolinesSteppedDisplay    = (ThematicSteppedDisplay_Isolines == settings.GetFlags().m_steppedDisplay);
    m_fixedMinimum              = settings.GetFlags().m_fixedMinimum;
    m_fixedMaximum              = settings.GetFlags().m_fixedMaximum;
    m_displayStyleIndex         = settings.GetStyleIndex ();
    m_edgeDisplayOverride       = settings.GetFlags ().m_edgeDisplayOverride;
    m_flatShading               = settings.GetFlags ().m_noSmoothing && (ThematicSteppedDisplay_None == settings.GetFlags ().m_steppedDisplay);

    // These settings are only relevant depending on other settings.
    m_transparentMarginContribution = m_accurateSteppedDisplay ? settings.GetFlags ().m_outOfRangeTransparent : 0;
    m_displayModeContribution       = handler.CacheOnDisplayModeChange () ? settings.GetDisplayMode (): 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ThematicDisplayStyleHandlerKey::Matches (DisplayStyleHandlerKey const& other) const
    {
    if (GetHandlerId () != other.GetHandlerId () ||
        !DisplayStyleHandlerKey::Matches (other))
        return false;

    ThematicDisplayStyleHandlerKeyCP    otherKey = static_cast <ThematicDisplayStyleHandlerKeyCP> (&other);

    return  m_accurateSteppedDisplay        == otherKey->m_accurateSteppedDisplay &&
            m_edgeDisplayOverride           == otherKey->m_edgeDisplayOverride &&
            m_fixedMinimum                  == otherKey->m_fixedMinimum &&
            m_fixedMaximum                  == otherKey->m_fixedMaximum &&
            m_isolinesSteppedDisplay        == otherKey->m_isolinesSteppedDisplay &&
            m_flatShading                   == otherKey->m_flatShading &&
            m_displayStyleIndex             == otherKey->m_displayStyleIndex &&
            m_displayModeContribution       == otherKey->m_displayModeContribution &&
            m_transparentMarginContribution == otherKey->m_transparentMarginContribution;
    }
