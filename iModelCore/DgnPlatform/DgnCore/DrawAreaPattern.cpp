/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DrawAreaPattern.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#define     VCClipStencil           ViewContext::ClipStencil

#define     MAX_GPA_STROKES         1000
#define     MAX_GPA_HATCH_LINES     10000
#define     MAX_LINE_DASHES         100000  // NOTE: This used to be the limit for all the dashes, not just the dashes per line so I don't expect to hit it...
#define     MAX_HATCH_ITERATIONS    50000
#define     MAX_AREA_PATTERN_TILES  1E6     // pre-Athens...which can really slow things down; create warning uses 10000...  Bmped back up to 1E6 with introduction of geometry map patterning - RayB 7/2013.

#define     TOLERANCE_ChoordAngle   .4
#define     TOLERANCE_ChoordLen     1000

static      MSElementDescrCP        s_patternOverrideCell;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::pattern_setOverrideCell (MSElementDescrCP cellEdP)
    {
    /* NOTE: The sole purpose of this is to support the published mdlPattern_area api
             and still be able to use DropContext to create a non-associative pattern. */
    s_patternOverrideCell = cellEdP;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
MSElementDescrCP BentleyApi::pattern_getOverrideCell ()
    {
    return s_patternOverrideCell;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PatternBoundaryCollector : IElementGraphicsProcessor
{
private:

ElementHandleCR     m_eh;
IStrokeForCache&    m_stroker;
CurveVectorPtr      m_boundary;
ViewContextP        m_context;
Transform           m_currentTransform;

protected:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/13
+---------------+---------------+---------------+---------------+---------------+------*/
explicit PatternBoundaryCollector (ElementHandleCR eh, IStrokeForCache& stroker) : m_eh (eh), m_stroker (stroker) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _ProcessAsFacets (bool isPolyface) const override {return false;}
virtual bool _ProcessAsBody (bool isCurved) const override {return false;}
virtual void _AnnounceContext (ViewContextR context) override {m_context = &context;}
virtual void _AnnounceTransform (TransformCP trans) override {if (trans) m_currentTransform = *trans; else m_currentTransform.InitIdentity ();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _ProcessCurveVector (CurveVectorCR curves, bool isFilled) override
    {
    if (!curves.IsAnyRegionType () || m_boundary.IsValid ())
        {
        BeAssert (false); // A valid boundary must be a closed, parity, or union region...

        return SUCCESS;
        }

    m_boundary = curves.Clone ();

    if (NULL != m_context->GetCurrLocalToFrustumTransformCP ())
        m_boundary->TransformInPlace (*m_context->GetCurrLocalToFrustumTransformCP ());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _OutputGraphics (ViewContextR context) override
    {
    CachedDrawHandle drawHandle (&m_eh);

    m_stroker._StrokeForCache (drawHandle, context);
    }

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/13
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr GetBoundary () {return m_boundary;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/13
+---------------+---------------+---------------+---------------+---------------+------*/
static CurveVectorPtr Process (ElementHandleCR eh, IStrokeForCache& stroker)
    {
    PatternBoundaryCollector  processor (eh, stroker);

    ElementGraphicsOutput::Process (processor, *eh.GetDgnProject ());

    return processor.GetBoundary ();
    }

}; // PatternBoundaryCollector

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
ViewContext::ClipStencil::ClipStencil (IStrokeForCache& stroker, int qvIndex, bool saveQvElem, QvElem* qvElem) : m_stroker (stroker)

    {
    m_qvIndex    = qvIndex;
    m_saveQvElem = saveQvElem;
    m_qvElem     = qvElem;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
QvElem* ViewContext::ClipStencil::GetQvElem (ElementHandleCR thisElm, ViewContextR context, bool& deleteQvElem)
    {
    if (m_qvElem)
        {
        deleteQvElem = false;

        return m_qvElem;
        }

    return context.GetCachedGeometry (CachedDrawHandle (&thisElm), m_stroker, m_qvIndex, deleteQvElem, m_saveQvElem);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr ViewContext::ClipStencil::GetCurveVector (ElementHandleCR thisElm)
    {
    if (m_curveVector.IsNull ())
        m_curveVector = PatternBoundaryCollector::Process (thisElm, m_stroker);

    return m_curveVector;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
IStrokeForCache& ViewContext::ClipStencil::GetStroker ()
    {
    return m_stroker;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
ViewContext::PatternParamSource::PatternParamSource (int patternIndex)
    {
    m_patternIndex  = patternIndex;
    m_paramsP       = NULL;
    m_hatchLinesP   = NULL;
    m_freeData      = false;
    m_origin.zero ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
ViewContext::PatternParamSource::PatternParamSource (PatternParamsP params, DwgHatchDefLineP hatchLines, int patternIndex)
    {
    m_patternIndex  = patternIndex;
    m_paramsP       = params;
    m_hatchLinesP   = params->dwgHatchDef.nDefLines ? hatchLines : NULL;
    m_freeData      = false;
    m_origin        = params->origin;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
ViewContext::PatternParamSource::~PatternParamSource ()
    {
    if (!m_freeData)
        return;

    FREE_AND_CLEAR (m_paramsP);
    FREE_AND_CLEAR (m_hatchLinesP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
PatternParamsP  ViewContext::PatternParamSource::GetParams (ElementHandleCR thisElm, DPoint3dP origin, DwgHatchDefLineP* hatchLines, int* index, ViewContextP context)
    {
    if (!m_paramsP)
        {
        if (NULL == context || NULL == context->GetOverrideMatSymb () || NULL == (m_paramsP = const_cast <PatternParamsP> (context->GetOverrideMatSymb ()->GetPatternParams ())))
            {
            if (!mdlElement_attributePresent (thisElm.GetElementCP (), PATTERN_ID, NULL))
                return NULL;

            m_paramsP     = (PatternParamsP) malloc (sizeof (*m_paramsP));
            m_hatchLinesP = (DwgHatchDefLineP) malloc (MAX_DWG_EXPANDEDHATCH_LINES * sizeof (*m_hatchLinesP));

#if defined (NEEDS_WORK_DGNITEM)
            if (SUCCESS != PatternLinkageUtil::ExtractFromElement (NULL, *m_paramsP, m_hatchLinesP, MAX_DWG_EXPANDEDHATCH_LINES, &m_origin, *thisElm.GetElementCP (), thisElm.GetDgnModelP (), m_patternIndex))
                {
                FREE_AND_CLEAR (m_paramsP);
                FREE_AND_CLEAR (m_hatchLinesP);

                return NULL;
                }
#endif

            m_freeData = true;
            }
        }

    if (origin)
        *origin = m_origin;

    if (index)
        *index = m_patternIndex;

    if (hatchLines)
        *hatchLines = m_hatchLinesP;

    return m_paramsP;
    }

/*=================================================================================**//**
* @bsiclass                                                     BrienBastings   11/07
+===============+===============+===============+===============+===============+======*/
struct PatternSymbol : IDisplaySymbol
{
private:

mutable EditElementHandle   m_eeh;
mutable ElementRefP         m_elementRef;
mutable DRange3d            m_range;

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandleCR GetElemHandle () const {return m_eeh;}
ElementRefP     GetElementRef () const {return m_elementRef;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _Draw (ViewContextR context) override
    {
    context.VisitElemHandle (m_eeh, false, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _GetRange (DRange3dR range) const override
    {
    range = m_range;
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
PatternSymbol (ElementId cellId, DgnProjectR project)
    {
    MSElementDescrPtr edP;

    if (!cellId.IsValid() && s_patternOverrideCell)
        {
        edP = s_patternOverrideCell->Duplicate();
        m_elementRef = NULL;
        }
    else
        {
        // If pattern cell doesn't already exist in project...it should fail...
        m_elementRef = project.Models ().FindElementById (cellId);
        if (NULL == m_elementRef)
            return;

        edP = m_elementRef->GetElementDescr();
        if (!edP.IsValid())
            return;
        }

    BeAssert (edP.IsValid());
    m_eeh.SetElementDescr(edP.get(), true);

#if defined (NEEDSWORK_REVISIT_PATTERN_SYMBOLS_SCDEF)
    // Match dimension of modelRef; otherwise pattern in 3D reference to 2D ends up at z=0, which may be outside of the range of the reference file.
    if (modelRef->Is3d () && !DisplayHandler::Is3dElem (m_eeh.GetElementCP ()))
        {
        m_eeh.GetHandler ().ConvertTo3d (m_eeh, 0.0);
        }
    else if (!modelRef->Is3d () && DisplayHandler::Is3dElem (m_eeh.GetElementCP ()))
        {
        DVec3d      flattenDir;
        Transform   flattenTrans;

        flattenDir.init (0.0, 0.0, 1.0);
        flattenTrans.initIdentity ();              
        flattenTrans.form3d[2][2] = 0.0;

        m_eeh.GetHandler ().ConvertTo2d (m_eeh, flattenTrans, flattenDir);
        }
#endif

    DisplayHandlerP dHandler = m_eeh.GetDisplayHandler ();

    if (NULL == dHandler || SUCCESS != dHandler->CalcElementRange (m_eeh, m_range, NULL))
        {
        BeAssert (false);
        m_range = DRange3d::NullRange ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsPointCellSymbol ()
    {
    return !m_eeh.GetElementCP ()->IsSnappable(); // Always SCDef
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool Is3dCellSymbol ()
    {
    return m_eeh.GetElementCP ()->Is3d();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
void ApplyElemDisplayParams (ElemDisplayParamsCR elParams)
    {
    ElementPropertiesSetter remapper;

    remapper.SetLevel (elParams.GetSubLevelId().GetLevel ());
    remapper.SetElementClass (elParams.GetElementClass ());
    remapper.SetTransparency (elParams.GetTransparency ());

    if (IsPointCellSymbol ())
        {
        UInt32  color = elParams.GetLineColor ();

        if (INVALID_COLOR == color) // Use an existing extended color if it exists (Don't expect this case actually occurs)...
            color = m_eeh.GetDgnProject()->Colors().FindElementColor (IntColorDef (elParams.GetLineColorTBGR ()));

        if (INVALID_COLOR != color)
            remapper.SetColor (color);

        remapper.SetLinestyle (elParams.GetLineStyle (), elParams.GetLineStyleParams ());
        remapper.SetWeight (elParams.GetWeight ());
        }

    remapper.Apply (m_eeh);
    }

}; // PatternSymbol

/*=================================================================================**//**
* @bsiclass                                                     BrienBastings   11/07
+===============+===============+===============+===============+===============+======*/
struct PatternHelper
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void CookPatternSymbology (PatternParamsCR params, ViewContextR context)
    {
    ElemDisplayParamsP  elParams = context.GetCurrentDisplayParams ();

    elParams->SetElementClass (DgnElementClass::PatternComponent);

    if (PatternParamsModifierFlags::None != ((PatternParamsModifierFlags::Color | PatternParamsModifierFlags::Weight | PatternParamsModifierFlags::Style) & params.modifiers))
        {
        if (PatternParamsModifierFlags::None != (params.modifiers & PatternParamsModifierFlags::Color))
            elParams->SetLineColor (params.color);

        if (PatternParamsModifierFlags::None != (params.modifiers & PatternParamsModifierFlags::Weight))
            elParams->SetWeight (params.weight);

        if (PatternParamsModifierFlags::None != (params.modifiers & PatternParamsModifierFlags::Style))
            elParams->SetLineStyle (params.style, elParams->GetLineStyleParams ());
        }

    context.CookDisplayParams ();

    // NOTE: Re-cook overrides for stuff like weight used by hilite, etc.
    context.CookDisplayParamsOverrides ();
    context.ActivateOverrideMatSymb ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static bool PushBoundaryClipStencil (ElementHandleCR thisElm, ViewContextR context, VCClipStencil& boundary, QvElem*& qvElem, bool& deleteQvElem)
    {
    deleteQvElem = false;

    if (NULL == (qvElem = boundary.GetQvElem (thisElm, context, deleteQvElem)))
        return false;

    context.GetIViewDraw ().PushClipStencil (qvElem);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void PopBoundaryClipStencil (ViewContextR context, QvElem* qvElem, bool deleteQvElem)
    {
    if (NULL == qvElem)
        return;

    context.GetIViewDraw ().PopClipStencil ();

    if (deleteQvElem)
        T_HOST.GetGraphicsAdmin()._DeleteQvElem (qvElem);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static bool GetCellTileInfo
(
ElementHandleCR thisElm,
VCClipStencil&  boundary,
PatternSymbol&  symbCell,
DRange3dR       cellRange,
DPoint2dR       cellOrg,
DPoint2dR       spacing,
DPoint2dR       low,
DPoint2dR       high,
bool&           isPlanar,
DPoint3dCR      origin,
RotMatrixCR     rMatrix,
double&         scale,
double          rowSpacing,
double          columnSpacing
)
    {
    if (SUCCESS != symbCell._GetRange (cellRange))
        return false;

    if (columnSpacing < 0.0)
        spacing.x = -columnSpacing;
    else
        spacing.x = columnSpacing + scale * (cellRange.high.x - cellRange.low.x);

    if (rowSpacing < 0.0)
        spacing.y = -rowSpacing;
    else
        spacing.y = rowSpacing + scale * (cellRange.high.y - cellRange.low.y);

    if (spacing.x < 0.5 || spacing.y < 0.5)
        return false;

    RotMatrix   invMatrix;
    Transform   transform;

    invMatrix.inverseOf (&rMatrix);
    transform.initFrom (&invMatrix);
    transform.translateInLocalCoordinates (&transform, -origin.x, -origin.y, -origin.z);

    CurveVectorPtr  boundaryCurve = boundary.GetCurveVector (thisElm);

    if (!boundaryCurve.IsValid ())
        return false;

    DRange3d  localRange;

    boundaryCurve->GetRange (localRange, transform);

    low.x = localRange.low.x;
    low.y = localRange.low.y;

    high.x = localRange.high.x;
    high.y = localRange.high.y;

    if (low.x < 0.0)
        low.x -= spacing.x + fmod (low.x, spacing.x);
    else
        low.x -= fmod (low.x, spacing.x);

    if (low.y < 0.0)
        low.y -= spacing.y + fmod (low.y, spacing.y);
    else
        low.y -= fmod (low.y, spacing.y);

    cellOrg.x = cellRange.low.x * scale;
    cellOrg.y = cellRange.low.y * scale;

    low.x -= cellOrg.x;
    low.y -= cellOrg.y;

    high.x -= cellOrg.x;
    high.y -= cellOrg.y;

    double  numTiles = ((high.x - low.x) / spacing.x) * ((high.y - low.y) / spacing.y);

    if (numTiles > MAX_AREA_PATTERN_TILES)
        {
        double  factor = (numTiles / MAX_AREA_PATTERN_TILES) * 0.5;

        // NOTE: Increase pattern scale and display *something* instead of useless message center alert...
        spacing.scale (&spacing, factor);
        scale *= factor;
        }

    // Can't use a stencil for a non-planar pattern cell....
    isPlanar = !symbCell.Is3dCellSymbol () || ((cellRange.high.z - cellRange.low.z) < 2.0);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void GetCellOrientationAndScale
(
RotMatrixR      rMatrix,
double&         scale,
RotMatrixR      patternRMatrix,
double          patternAngle,
double          patternScale
)
    {
    RotMatrix   angleRot;
    
    angleRot.initFromAxisAndRotationAngle (2, patternAngle);
    rMatrix.productOf (&patternRMatrix, &angleRot);
    scale = patternScale ? patternScale : 1.0;
    }

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      05/2007
+===============+===============+===============+===============+===============+======*/
struct GeometryMapPatternAppData : ElementRefAppData 
{
MaterialPtr     m_material;

GeometryMapPatternAppData (MaterialPtr material) : m_material (material) {}
MaterialCP GetMaterial () {return m_material.get ();}

virtual void _OnCleanup (ElementRefP host, bool unloadingModel, HeapZoneR zone) {m_material = NULL;}
};
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
static MaterialPtr CreateGeometryMapMaterial (ViewContextR context, PatternSymbol& symbCell, PatternParamsP params, DPoint2dCR spacing)
    {
    MaterialPtr         pMaterial = Material::Create (symbCell.GetElemHandle ().GetDgnModelP ()->GetDgnProject());
    MaterialSettingsR   settings = pMaterial->GetSettingsR ();
    MaterialMapP        map = settings.GetMapsR().AddMap (MaterialMap::MAPTYPE_Geometry);
    MaterialMapLayerR   layer = map->GetLayersR().GetTopLayerR();

    map->SetValue (1.0);
    layer.SetMode (MapMode::Parametric);
    layer.SetScale (1.0, 1.0, 1.0);
    layer.SetIsBackgroundTransparent (true);

    // NOTE: Need to setup pattern symbology on cell element and hide 0 length lines used as pattern cell extent markers, etc.
    PatternHelper::CookPatternSymbology (*params, context);
    symbCell.ApplyElemDisplayParams (*context.GetCurrentDisplayParams ());

    context.GetIViewDraw ().DefineQVGeometryMap (*pMaterial, symbCell.GetElemHandle (), &spacing, !symbCell.IsPointCellSymbol (), context, true);

    return pMaterial;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
static void AddPatternParametersToPolyface (PolyfaceHeaderPtr& polyface, RotMatrixCR rMatrix, DPoint3dCR origin, DPoint2dCR spacing)
    {
    Transform   cellToWorld = Transform::From (rMatrix, origin), worldToCell;

    cellToWorld.ScaleMatrixColumns (spacing.x, spacing.y, 1.0);
    worldToCell.InverseOf (cellToWorld);

    BlockedVectorDPoint2dR  params = polyface->Param();

    params.SetActive (true);

    for (DPoint3dCR point: polyface->Point())
        {
        DPoint3d    paramPoint;

        worldToCell.Multiply (paramPoint, point);
        params.push_back (DPoint2d::From (paramPoint.x, paramPoint.y));
        }
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t GetVertexCount (CurveVectorCR curveVector)
    {
    size_t  loopCount = 0;

    for (ICurvePrimitivePtr curve: curveVector)
        {
        if (!curve.IsValid ())
            continue;

        switch (curve->GetCurvePrimitiveType ())
            {
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                loopCount += curve->GetLineStringCP ()->size ();
                break;

            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector:
                loopCount += GetVertexCount (*curve->GetChildCurveVectorCP ());
                break;

            default:
                loopCount += 2;
                break;
            }
        }

    return loopCount;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
static bool ProcessAreaPatternAsGeometryMap  
(
ElementHandleCR thisElm,
ViewContextR    context,
VCClipStencil&  boundary,
PatternSymbol&  symbCell,
PatternParamsP  params,
RotMatrixCR     rMatrix,
DPoint3dCR      origin, 
DPoint2dCR      spacing,
double          scale
)
    {
    static      ElementRefAppData::Key s_appDataKey;
#ifdef WIP_CFGVAR
    static      int s_doGeometryMapsChecked;

    if (ConfigurationManager::CheckVariableIsDefined (s_doGeometryMapsChecked, L"MS_NO_PATTERN_GEOMETRY_MAPS"))
        return false;

    if (DrawPurpose::Plot == context.GetDrawPurpose())      // Opt for slower, higher quality when plotting.
        return false;
#endif

#if !defined(BENTLEYCONFIG_GRAPHICS_OPENGLES)  //  We always want to use geometry map with OpenGL ES because the our OpenGL implementation of PushClipStencil does not work
    double          pixelSize = context.GetPixelSizeAtPoint (NULL);
    double          tilePixels = MAX (spacing.x, spacing.y) / pixelSize;
    static double   s_maxGeometryMapTile = 32.0;

    if (tilePixels > s_maxGeometryMapTile)
        return false;                                       // The pattern is too big... Draw it.
#endif

    ElementRefP     cellElementRef;

    if (NULL == (cellElementRef = symbCell.GetElementRef ()))
        return false;                                       // No place to persist map...

    GeometryMapPatternAppData*  appData;
    DPoint2d                    cellSpacing = spacing;

    cellSpacing.Scale (1.0 / scale);

    if (NULL == (appData = reinterpret_cast <GeometryMapPatternAppData*> (cellElementRef->FindAppData (s_appDataKey))))
        {
        MaterialPtr pMaterial = CreateGeometryMapMaterial (context, symbCell, params, cellSpacing);

        if (!pMaterial.IsValid ())
            return false;

        cellElementRef->AddAppData (s_appDataKey, appData = new GeometryMapPatternAppData (pMaterial), cellElementRef->GetHeapZone ());
        }

    // NOTE: Colors aren't stored in geometry map for point cells, setup active matsymb color from pattern if different than element color...
    if (symbCell.IsPointCellSymbol () && PatternParamsModifierFlags::None != (params->modifiers & PatternParamsModifierFlags::Color) && context.GetCurrentDisplayParams ()->GetLineColor () != params->color)
        {
        context.GetCurrentDisplayParams ()->SetLineColor (params->color);
        context.CookDisplayParams ();
        context.CookDisplayParamsOverrides ();
        }

    OvrMatSymbP  ovrMatSymb = context.GetOverrideMatSymb ();

    ovrMatSymb->SetTransparentFillColor (0xff);
    ovrMatSymb->SetMaterial (appData->GetMaterial ());
    context.ActivateOverrideMatSymb ();

    CurveVectorPtr  boundaryCurve = boundary.GetCurveVector (thisElm);

    // NOTE: Use stencils for curved or complex boundaries as these can take significant time to create polyface...
#if !defined(BENTLEYCONFIG_GRAPHICS_OPENGLES)  //  Don't allow this with OpenGL ES since PushBoundaryClipStencil does not work for geometry map.
    static size_t   s_facetBoundaryMax = 1000;
    if (boundaryCurve->ContainsNonLinearPrimitive () || GetVertexCount (*boundaryCurve) > s_facetBoundaryMax)
        {
        bool        deleteQvElem;
        QvElem*     qvElem = NULL;

        if (!PatternHelper::PushBoundaryClipStencil (thisElm, context, boundary, qvElem, deleteQvElem))
            return false;

        PolyfaceHeaderPtr       polyface = PolyfaceHeader::CreateVariableSizeIndexed ();
        BlockedVectorDPoint3dR  points = polyface->Point ();
        BlockedVectorIntR       pointIndices = polyface->PointIndex ();

        points.SetActive (true);
        pointIndices.SetActive (true);
        points.resize (4);
        
        DPoint3d    shapePts[5];

        GetBoundaryShapePts (shapePts, *boundaryCurve, rMatrix, origin);

        for (int i=0; i<4; i++)
            {
            pointIndices.push_back (i+1);
            points[i] = shapePts[i];
            }
        
        pointIndices.push_back (0);
        AddPatternParametersToPolyface (polyface, rMatrix, origin, spacing);
        context.GetIDrawGeom ().DrawPolyface (*polyface, true);

        PatternHelper::PopBoundaryClipStencil (context, qvElem, deleteQvElem);
        }
    else
#endif
        {
        IFacetOptionsPtr  options = IFacetOptions::CreateForCurves ();

        options->SetConvexFacetsRequired (true);
        options->SetMaxPerFace (MAX_VERTICES);

        IPolyfaceConstructionPtr  builder = IPolyfaceConstruction::New (*options);

        builder->AddRegion (*boundaryCurve);

        PolyfaceHeaderPtr  polyface = builder->GetClientMeshPtr ();

        if (!polyface.IsValid ())
            {
            BeAssert (false);
            return false;
            }

        AddPatternParametersToPolyface (polyface, rMatrix, origin, spacing);
        context.GetIDrawGeom ().DrawPolyface (*polyface, true);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static bool DrawCellTiles (ViewContextR context, PatternSymbol& symbCell, DPoint2dCR low, DPoint2dCR high, DPoint2dCR spacing, double scale, TransformCR orgTrans, DPoint3dCP cellCorners, bool drawFiltered, CurveVectorCP boundaryToPush = NULL)
    {
    if (NULL != boundaryToPush)
        {
        ClipVectorPtr   clip = ClipVector::CreateFromCurveVector (*boundaryToPush, 0.0, TOLERANCE_ChoordAngle);

        if (!clip.IsValid ())
            return false;

        clip->ParseClipPlanes ();
        context.PushClip (*clip); // NOTE: Pop handled by context mark...
        }

    bool        wasAborted = false;
    DPoint2d    patOrg;
    DPoint3d    tileCorners[8];
    Transform   cellTrans;

    for (patOrg.x = low.x; patOrg.x < high.x && !wasAborted; patOrg.x += spacing.x)
        {
        for (patOrg.y = low.y; patOrg.y < high.y && !wasAborted; patOrg.y += spacing.y)
            {
            if (context.CheckStop ())
                {
                wasAborted = true;
                break;
                }

            cellTrans.TranslateInLocalCoordinates (orgTrans, patOrg.x/scale, patOrg.y/scale, context.GetDisplayPriority ());
            cellTrans.Multiply (tileCorners, cellCorners, 8);

            if (ClipPlaneContainment_StronglyOutside == context.GetTransformClipStack ().ClassifyPoints (tileCorners, 8))
                continue;

            if (drawFiltered)
                {
                DPoint3d    tmpPt;

                cellTrans.GetTranslation (tmpPt);
                context.GetIDrawGeom ().DrawPointString3d (1, &tmpPt, NULL);
                }
            else
                {
                context.DrawSymbol (&symbCell, &cellTrans, NULL, false, false);
                }

            wasAborted = context.WasAborted ();
            }
        }

    return wasAborted;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void ProcessAreaPattern
(
ElementHandleCR thisElm,
ViewContextR    context,
VCClipStencil&  boundary,
PatternParamsP  params,
DPoint3dR       origin,
double          contextScale
)
    {
    PatternSymbol symbCell (ElementId (params->cellId), context.GetDgnProject ());

    if (!symbCell.GetElemHandle ().IsValid ())
        return;

    double      scale;
    RotMatrix   rMatrix;

    PatternHelper::GetCellOrientationAndScale (rMatrix, scale, params->rMatrix, params->angle1, params->scale);

    bool        isPlanar;
    DPoint2d    cellOrg, spacing, low, high;
    DRange3d    cellRange;

    // The contextScale value allows PlotContext to adjust the pattern scale to reduce
    // the size of the QV display list and resulting plot output file.
    scale *= contextScale;

    if (!PatternHelper::GetCellTileInfo (thisElm, boundary, symbCell, cellRange, cellOrg, spacing, low, high, isPlanar, origin, rMatrix, scale, params->space1, params->space2))
        return;

    bool        isQVOutput = context.GetIViewDraw().IsOutputQuickVision ();
    bool        useStencil = isQVOutput && isPlanar && !context.CheckICachedDraw (); // Can't use stencil if creating QvElem...dimension terminators want patterns!
    bool        deleteQvElem = false;
    QvElem*     qvElem = NULL;

    if (useStencil && PatternHelper::ProcessAreaPatternAsGeometryMap (thisElm, context, boundary, symbCell, params, rMatrix, origin, spacing, scale))
        return;

    if (useStencil && !PatternHelper::PushBoundaryClipStencil (thisElm, context, boundary, qvElem, deleteQvElem))
        return;

    // NOTE: Setup symbology AFTER visit to compute stencil/clip since that may change current display params!
    PatternHelper::CookPatternSymbology (*params, context);
    symbCell.ApplyElemDisplayParams (*context.GetCurrentDisplayParams ());

    bool        drawFiltered = false;
    Transform   orgTrans;
    DPoint3d    cellCorners[8];

    // Setup initial pattern instance transform
    LegacyMath::TMatrix::ComposeOrientationOriginScaleXYShear (&orgTrans, NULL, &rMatrix, &origin, scale, scale, 0.0);
    cellRange.get8Corners (cellCorners);

#if defined (NEEDS_WORK_DGNITEM)
    if (isQVOutput)
        {
        int     cellCmpns = symbCell.GetElemHandle ().GetElementCP ()->GetComplexComponentCount ();
        double  numTiles = ((high.x - low.x) / spacing.x) * ((high.y - low.y) / spacing.y);

        if (numTiles * cellCmpns > 5000)
            {
            DPoint3d    viewPts[8];
            DRange2d    viewRange;

            orgTrans.multiply (viewPts, cellCorners, 8);
            context.LocalToView (viewPts, viewPts, 8);
            viewRange.initFrom (viewPts, 8);

            if (symbCell.IsPointCellSymbol ())
                drawFiltered = (viewRange.extentSquared () < context.GetMinLOD ());
            }
        }
#endif

    if (useStencil)
        {
        DrawCellTiles (context, symbCell, low, high, spacing, scale, orgTrans, cellCorners, drawFiltered);
        }
    else
        {
        CurveVectorPtr  boundaryCurve = boundary.GetCurveVector (thisElm);

        // NOTE: Union regions aren't valid clip boundaries, need to push separate clip for each solid area...
        if (boundaryCurve->IsUnionRegion ())
            {
            for (ICurvePrimitivePtr curve: *boundaryCurve)
                {
                if (curve.IsNull () || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType ())
                    continue;

                ViewContext::ContextMark mark (&context);

                // NOTE: Cell tile exclusion check makes sending all tiles for each solid area less offensive (union regions also fairly rare)...
                if (DrawCellTiles (context, symbCell, low, high, spacing, scale, orgTrans, cellCorners, drawFiltered, curve->GetChildCurveVectorCP ()))
                    break; // Was aborted...
                }
            }
        else
            {
            DrawCellTiles (context, symbCell, low, high, spacing, scale, orgTrans, cellCorners, drawFiltered, boundaryCurve.get ());
            }
        }

    PatternHelper::PopBoundaryClipStencil (context, qvElem, deleteQvElem);

    context.DeleteSymbol (&symbCell); // Only needed if DrawSymbol has been called...i.e. early returns are ok!
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static int DrawHatchGPA
(
GPArrayP        pGPA,
ViewContextR    context
)
    {
    size_t        nGot, sourceCount = pGPA->GetGraphicsPointCount ();
    DPoint3d      localPoints[MAX_GPA_STROKES];
    bool          is3d = context.Is3dView ();
    double        priority = context.GetDisplayPriority ();
    GraphicsPoint gp;

    for (size_t i=0; i < sourceCount;)
        {
        if (0 == (i % 100) && context.CheckStop ())
            return ERROR;

        nGot = 0;
        
        while (pGPA->GetGraphicsPoint (i, gp))
            {
            gp.point.GetProjectedXYZ (localPoints[nGot++]);

            if (gp.IsCurveBreak ())
                {
                i++;
                break;
                }
            else if (nGot >= MAX_GPA_STROKES)
                {
                // Leave i where it is to resume from non-break !!!
                break;
                }
            else
                {
                i++;
                }
            }

        if (nGot <= 1)
            continue;

        if (is3d)
            {
            context.DrawStyledLineString3d ((int) nGot, localPoints, NULL, false);
            }
        else
            {
            // To ensure display priority is properly honored in non-rasterized plots, it is necessary to call QuickVision 2D draw methods. TR 180390.
            std::valarray<DPoint2d> localPoints2dBuf (nGot);

            for (size_t iPoint = 0; iPoint < nGot; iPoint++)
                {
                localPoints2dBuf[iPoint].x = localPoints[iPoint].x;
                localPoints2dBuf[iPoint].y = localPoints[iPoint].y;
                }

            context.DrawStyledLineString2d ((int) nGot, &localPoints2dBuf[0], priority, NULL, false);
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/13
+---------------+---------------+---------------+---------------+---------------+------*/
static void GetBoundaryShapePts
(
DPoint3dP       pts,
CurveVectorCR   boundaryCurve,
RotMatrixCR     rMatrix,
DPoint3dCR      origin
)
    {
    RotMatrix   invMatrix;
    Transform   transform;

    invMatrix.InverseOf (rMatrix);
    transform.InitFrom (invMatrix);

    DRange3d    localRange;

    boundaryCurve.GetRange (localRange, transform);

    pts[0].x = pts[3].x = localRange.low.x;
    pts[1].x = pts[2].x = localRange.high.x;

    pts[0].y = pts[1].y = localRange.low.y;
    pts[2].y = pts[3].y = localRange.high.y;

    DPoint3d    stencilOrigin;

    transform.Multiply (stencilOrigin, origin);

    pts[0].z = pts[1].z = pts[2].z = pts[3].z = stencilOrigin.z;
    pts[4] = pts[0];

    transform.MultiplyTranspose (pts, pts, 5);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static GPArrayP GetBoundaryGPA
(
ElementHandleCR thisElm,
VCClipStencil&  boundary,
RotMatrixCR     rMatrix,
DPoint3dCR      origin,
bool            useElmRange
)
    {
    CurveVectorPtr boundaryCurve = boundary.GetCurveVector (thisElm);

    if (!boundaryCurve.IsValid ())
        return NULL;

    GPArrayP    gpa = GPArray::Grab ();

    if (!useElmRange)
        {
        gpa->Add (*boundaryCurve);

        return gpa;
        }

    DPoint3d    shapePts[5];

    GetBoundaryShapePts (shapePts, *boundaryCurve, rMatrix, origin);

    gpa->Add (shapePts, 5);
    gpa->MarkBreak ();
    gpa->MarkMajorBreak ();

    return gpa;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void GetHatchLineLimitTransform
(
TransformR      scaledTransform,
TransformCR     hatchTransform,
GPArrayP        boundGpa
)
    {
    double      zScale;

    // modify the transform to limit the number of hatch lines.
    if ((zScale = jmdlGPA_hatchDensityScale (&hatchTransform, boundGpa, NULL, MAX_GPA_HATCH_LINES)) > 1.0)
        {
        RotMatrix   scaleMatrix;

        bsiRotMatrix_initFromScaleFactors (&scaleMatrix, 1.0, 1.0, zScale);
        bsiTransform_multiplyTransformRotMatrix (&scaledTransform, &hatchTransform, &scaleMatrix);
        }
    else
        {
        scaledTransform = hatchTransform;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void AddHatchLinesToGPA
(
GPArrayP        hatchGpa,
GPArrayP        boundGpa,
TransformR      scaledTransform
)
    {
    jmdlGraphicsPointArray_addTransformedCrossHatchClipped (hatchGpa, boundGpa, &scaledTransform, NULL, NULL, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void ProcessHatchBoundary
(
GPArrayP        boundGpa,
ViewContextR    context,
TransformR      baseTransform,
TransformR      hatchTransform,
double          angle,
double          space
)
    {
    DVec3d      xVec,yVec, zVec;
    DPoint3d    origin;

    hatchTransform.GetOriginAndVectors (origin, xVec, yVec, zVec);
    xVec.Init( cos (angle), sin (angle), 0.0);
    zVec.CrossProduct (xVec, yVec);
    zVec.Scale (space);
    hatchTransform.InitFromOriginAndVectors (origin, xVec, yVec, zVec);

    Transform       scaledTransform;
    GPArraySmartP   hatchGpa;

    GetHatchLineLimitTransform (scaledTransform, hatchTransform, boundGpa);
    AddHatchLinesToGPA (hatchGpa, boundGpa, scaledTransform);

    hatchGpa->Transform (&baseTransform);

    PatternHelper::DrawHatchGPA (hatchGpa, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void ProcessHatchPattern
(
ElementHandleCR thisElm,
ViewContextR    context,
VCClipStencil&  boundary,
PatternParams*  params,
DPoint3dR       origin
)
    {
#if !defined(BENTLEYCONFIG_GRAPHICS_OPENGLES)  //  We always want to use geometry map with OpenGL ES because the our OpenGL implementation of PushClipStencil does not work
    bool            useStencil = context.GetIViewDraw().IsOutputQuickVision () && !context.CheckICachedDraw (); // Can't use stencil if creating QvElem...dimension terminators want patterns!
#else
    bool            useStencil = false;
#endif
    GPArraySmartP   boundGpa (PatternHelper::GetBoundaryGPA (thisElm, boundary, params->rMatrix, origin, useStencil));

    if (NULL == boundGpa || 0 == boundGpa->GetCount ())
        return;

    bool            deleteQvElem=false;
    QvElem*         qvElem = NULL;

    if (useStencil && !PatternHelper::PushBoundaryClipStencil (thisElm, context, boundary, qvElem, deleteQvElem))
        return;

    // NOTE: Setup symbology AFTER visit to compute stencil/clip since that may change current display params!
    PatternHelper::CookPatternSymbology (*params, context);

    Transform       baseTransform, invBaseTransform;
    Transform       hatchTransform;

    hatchTransform.initFromRowValues (1,0,0, 0, 0,0,0, 0, 0,-1,0, 0);
    baseTransform.initFrom (&params->rMatrix, &origin);
    invBaseTransform.inverseOf (&baseTransform);

    boundGpa->Transform (&invBaseTransform);

    PatternHelper::ProcessHatchBoundary (boundGpa, context, baseTransform, hatchTransform, params->angle1, params->space1);

    if (PatternParamsModifierFlags::None != (params->modifiers & PatternParamsModifierFlags::Space2))
        PatternHelper::ProcessHatchBoundary (boundGpa, context, baseTransform, hatchTransform, params->angle2, params->space2);

    PatternHelper::PopBoundaryClipStencil (context, qvElem, deleteQvElem);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/01
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsValidPatternDefLine
(
DwgHatchDefLineP    lineP,
double              rangeDiagonal
)
    {
    double      offsetMagnitude = lineP->offset.magnitude ();

    if (0.0 == offsetMagnitude || rangeDiagonal / offsetMagnitude > MAX_HATCH_ITERATIONS)
        return false;

    if (0 == lineP->nDashes)
        return true;

    double      totalDashLength = 0.0;

    for (int i=0; i < lineP->nDashes; i++)
        totalDashLength += fabs (lineP->dashes[i]);

    return (!(0.0 == totalDashLength || rangeDiagonal / totalDashLength > MAX_HATCH_ITERATIONS));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void ProcessDWGHatchBoundary
(
GPArrayP            boundGpa,
ViewContextR        context,
TransformR          baseTransform,
TransformR          hatchTransform,
double              annotationScale,
DwgHatchDefLineP    hatchLines,
int                 nDefLines,
bool                is3d
)
    {
    DRange3d    boundRange;

    bsiDRange3d_init (&boundRange);
    jmdlDRange3d_extendByGraphicsPointArray (&boundRange, boundGpa);

    if (!is3d)
        boundRange.low.z = boundRange.high.z = 0.0;

    StatusInt       status = SUCCESS;
    double          rangeDiagonal = boundRange.low.distance (&boundRange.high);
    GPArraySmartP   hatchGpa, dashGpa;

    // NOTE: In ACAD hatch definitions both the base angle and scale have already been applied to the definitions and MUST not be applied again!
    for (DwgHatchDefLineP lineP = hatchLines; SUCCESS == status && lineP < &hatchLines[nDefLines]; lineP++)
        {
        if (!IsValidPatternDefLine (lineP, rangeDiagonal))
            continue;

        DVec3d      xVec, yVec, zVec;
        DPoint3d    origin;

        // NOTE: Annotation scale isn't handled by CookPatternParams since we ignore scale for draw, need to apply to hatch lines directly...
        hatchTransform.GetOriginAndVectors (origin, xVec, yVec, zVec);
        bsiDPoint3d_setXYZ (&xVec, cos (lineP->angle), sin (lineP->angle), 0.0);
        bsiDPoint3d_setXYZ (&zVec, lineP->offset.x * annotationScale, lineP->offset.y * annotationScale, 0.0);
        bsiDPoint3d_setXYZ (&origin, lineP->through.x * annotationScale, lineP->through.y * annotationScale, 0.0);
        hatchTransform.InitFromOriginAndVectors (origin, xVec, yVec, zVec);

        Transform    scaledTransform;

        GetHatchLineLimitTransform (scaledTransform, hatchTransform, boundGpa);
        AddHatchLinesToGPA (hatchGpa, boundGpa, scaledTransform);

        if (0 != lineP->nDashes)
            {
            double*         dashesOut = lineP->dashes;
            bvector<double> localDashes;

            if (1.0 != annotationScale)
                {
                localDashes.insert (localDashes.begin (), dashesOut, dashesOut + lineP->nDashes);

                for (double& dashLen: localDashes)
                    dashLen *= annotationScale;

                dashesOut = &localDashes.front ();
                }

            // NOTE: Copy of jmdlGraphicsPointArray_expandDashPattern to avoid s_maxCollectorPoint limit...
            int         i1;
            DPoint4d    point0, point1;
            int         curveType;
            double      dashPeriod = jmdlGPA_computeDashPeriod (dashesOut, lineP->nDashes);

            for (int i0 = 0; SUCCESS == status && jmdlGraphicsPointArray_parseFragment (hatchGpa, &i1, &point0, &point1, &curveType, i0); i0 = i1 + 1)
                {
                if (0 == curveType)
                    {
                    GraphicsPoint   gp0, gp1;

                    jmdlGraphicsPointArray_getGraphicsPoint (hatchGpa, &gp0, i0);

                    for (int i = i0 + 1; SUCCESS == status && i <= i1; i++, gp0 = gp1)
                        {
                        jmdlGraphicsPointArray_getGraphicsPoint (hatchGpa, &gp1, i);
                        jmdlGPA_expandSingleLineDashPattern (dashGpa, &gp0, &gp1, dashesOut, lineP->nDashes, dashPeriod, MAX_LINE_DASHES);

                        dashGpa->Transform (&baseTransform);

                        status = PatternHelper::DrawHatchGPA (dashGpa, context);

                        dashGpa->Empty ();
                        }
                    }
                else
                    {
                    jmdlGraphicsPointArray_appendFragment (dashGpa, hatchGpa, i0, i1, 0);
                    jmdlGraphicsPointArray_markBreak (dashGpa);

                    dashGpa->Transform (&baseTransform);

                    status = PatternHelper::DrawHatchGPA (dashGpa, context);

                    dashGpa->Empty ();
                    }
                }
            }
        else
            {
            hatchGpa->Transform (&baseTransform);

            status = PatternHelper::DrawHatchGPA (hatchGpa, context);
            }

        hatchGpa->Empty ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void ProcessDWGHatchPattern
(
ElementHandleCR     thisElm,
ViewContextR        context,
VCClipStencil&      boundary,
PatternParamsP      params,
DwgHatchDefLineP    hatchLines,
DPoint3dR           origin
)
    {
    // NOTE: Never use stencil for DWG patterns, boundary can be open elements...
    GPArraySmartP   boundGpa (PatternHelper::GetBoundaryGPA (thisElm, boundary, params->rMatrix, origin, false));

    if (NULL == boundGpa || 0 == boundGpa->GetCount ())
        return;

    Transform   baseTransform, invBaseTransform;
    Transform   hatchTransform;

    // NOTE: Setup symbology AFTER visit to compute stencil/clip since that may change current display params!
    PatternHelper::CookPatternSymbology (*params, context);

    hatchTransform.initFromRowValues (1,0,0, 0, 0,0,0, 0, 0,-1,0, 0);

    DPoint3d    hatchOrigin = origin;

    if (PatternParamsModifierFlags::None == (params->modifiers & PatternParamsModifierFlags::DwgHatchOrigin))
        {
        // Old style DWG Hatch Definitions are implicitly about (0,0)
        params->rMatrix.multiplyTranspose (&hatchOrigin);
        hatchOrigin.x = hatchOrigin.y = 0.0;
        params->rMatrix.multiply (&hatchOrigin);
        }

    baseTransform.initFrom (&params->rMatrix, &hatchOrigin);
    invBaseTransform.inverseOf (&baseTransform);

    boundGpa->Transform (&invBaseTransform);

    double  annotationScale = 1.0;//PatternHelper::GetAnnotationScale (*params, context); removed in graphite

    PatternHelper::ProcessDWGHatchBoundary (boundGpa, context, baseTransform, hatchTransform, annotationScale, hatchLines, params->dwgHatchDef.nDefLines, thisElm.GetElementCP ()->Is3d());
    }

//static double GetAnnotationScale (PatternParamsCR params, ViewContextR context) removed in graphite
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sunand.Sandurkar 04/13
+---------------+---------------+---------------+---------------+---------------+------*/
static PatternParamsPtr CookPatternParams (PatternParamsCR params, ViewContextR context)
    {
    PatternParamsPtr  cookedParams = PatternParams::CreateFromExisting (params);
    //double            annotationScale = PatternHelper::GetAnnotationScale (*cookedParams, context); removed in graphite

    //cookedParams->scale  *= annotationScale;
    //cookedParams->space1 *= annotationScale;
    //cookedParams->space2 *= annotationScale;

    return cookedParams;
    }

}; // PatternHelper

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::_WantAreaPatterns ()
    {
    ViewFlagsCP viewFlags = GetViewFlags();

    if (!viewFlags || !viewFlags->patterns || (!viewFlags->patternDynamics && DrawPurpose::Dynamics == GetDrawPurpose()))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void correctPatternOffsetAndRotation (ElementHandleCR thisElm, PatternParamsR params, DPoint3dR origin, ViewContext::ClipStencil& boundary)
    {
    if (PatternParamsModifierFlags::None != (params.modifiers & PatternParamsModifierFlags::Origin) ||
        PatternParamsModifierFlags::None != (params.modifiers & PatternParamsModifierFlags::DwgHatchOrigin))
        return; // Explicit origin supplied, assume PatternParam information is good (ex. cut section). Normal patterns specify an offset, not an origin...

    CurveVectorPtr  curves = boundary.GetCurveVector (thisElm);

    if (!curves.IsValid ())
        return;

    // TR#261083 - We don't store good pattern offsets for complex shapes, grouped holes, and assoc regions. Offset is from the
    //             lower left corner of range and may not lie in the plane of the geometry. The offset should be correct for 
    //             SHAPE_ELM, ELLIPSE_ELM, and MULTILINE_ELM...but we'll verify it anyway since it doesn't hurt to get the 
    //             CurveVector now (it's cached) and this PatternParams could be from an OvrMatSymb where the offset/rotation
    //             aren't set per-element.
    DVec3d      planeNormal;
    DPoint3d    planePt;

    if (!curves->GetStartPoint (planePt))
        return;

    if (PatternParamsModifierFlags::None == (params.modifiers & PatternParamsModifierFlags::RotMatrix))
        {
        Transform   localToWorld;

        if (curves->GetAnyFrenetFrame (localToWorld))
            {
            localToWorld.GetMatrix (params.rMatrix);
            params.rMatrix.SquareAndNormalizeColumns (params.rMatrix, 2, 0);
            }
        }

    double      t;
    DVec3d      diff, rtmp;

    params.rMatrix.GetColumn (planeNormal, 2);
    diff.DifferenceOf (planePt, origin);
    t = diff.DotProduct (planeNormal);
    rtmp.Scale (planeNormal, t);
    origin.SumOf (rtmp, origin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawAreaPattern (ElementHandleCR thisElm, ClipStencil& boundary, PatternParamSource& source)
    {
    if (_CheckStop ())
        return;

    if (!_WantAreaPatterns ())
        return;

    int              patternIndex;
    DPoint3d         origin;
    PatternParamsP   params;
    DwgHatchDefLineP hatchLines;

    if (NULL == (params = source.GetParams (thisElm, &origin, &hatchLines, &patternIndex, this)))
        return;

    // Can greatly speed up fit/range calculation by just drawing boundary...
    if (DrawPurpose::RangeCalculation == GetDrawPurpose () || DrawPurpose::FitView == GetDrawPurpose ())
        {
        CachedDrawHandle drawHandle(&thisElm);
        boundary.GetStroker ()._StrokeForCache (drawHandle, *this);

        return;
        }

    IPickGeom*  pickGeom = GetIPickGeom();
    GeomDetailP detail = pickGeom ? &pickGeom->GetGeomDetail () : NULL;
    bool        wasSnappable = true;

    if (NULL != detail)
        {
        wasSnappable = detail->IsSnappable ();
        detail->SetPatternIndex (patternIndex);
        detail->SetNonSnappable (PatternParamsModifierFlags::None == (params->modifiers & PatternParamsModifierFlags::Snap));
        }

    PatternParamsPtr cookedParams = PatternHelper::CookPatternParams (*params, *this);

    if (Is3dView ())
        correctPatternOffsetAndRotation (thisElm, *cookedParams, origin, boundary);

    // Use mark to keep track of number of CoordSys to pop...
    ViewContext::ContextMark mark (this);

    /* NOTE: If parity isn't used then pattern won't match fill display. This means we can't use
             a stencil. QV allows for parity or 0 winding rule, neither of which matches old pattern
             display, which in gpa case made use of booleans to subtract holes.
             useParity = (0 != (cookedParams->modifiers & PatternParamsModifierFlags::HoleStyle) && PatternParamsHoleStyleType::Parity == cookedParams->holeStyle) */

    if (PatternParamsModifierFlags::None != (cookedParams->modifiers & PatternParamsModifierFlags::Cell))
        PatternHelper::ProcessAreaPattern (thisElm, *this, boundary, cookedParams.get (), origin, m_patternScale);
    else if (PatternParamsModifierFlags::None != (cookedParams->modifiers & PatternParamsModifierFlags::DwgHatchDef))
        PatternHelper::ProcessDWGHatchPattern (thisElm, *this, boundary, cookedParams.get (), hatchLines, origin);
    else
        PatternHelper::ProcessHatchPattern (thisElm, *this, boundary, cookedParams.get (), origin);

    if (NULL != detail)
        {
        detail->SetPatternIndex (0);
        detail->SetNonSnappable (!wasSnappable);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::WantAreaPatterns () {return _WantAreaPatterns ();}
void ViewContext::DrawAreaPattern (ElementHandleCR thisElm, ClipStencil& boundary, PatternParamSource& pattern) {_DrawAreaPattern (thisElm, boundary, pattern);}
