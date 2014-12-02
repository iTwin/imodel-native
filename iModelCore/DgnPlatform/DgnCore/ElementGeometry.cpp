/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ElementGeometry.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

static const double TOLERANCE_ChainMiterCosLimit = .707;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/10
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr ICurvePathQuery::ElementToCurveVector (ElementHandleCR eh)
    {
    if (!eh.IsValid ())
        return NULL;

    ICurvePathQuery* curveQuery;

    if (NULL == (curveQuery = dynamic_cast <ICurvePathQuery*> (&eh.GetHandler ())))
        return NULL;

    CurveVectorPtr curves;

    if (SUCCESS != curveQuery->GetCurveVector (eh, curves))
        return NULL;

    return curves;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/12
+---------------+---------------+---------------+---------------+---------------+------*/
ISolidPrimitivePtr ISolidPrimitiveQuery::ElementToSolidPrimitive (ElementHandleCR eh, bool simplify)
    {
    if (!eh.IsValid ())
        return NULL;

    ISolidPrimitiveQuery* solidQuery;

    if (NULL == (solidQuery = dynamic_cast <ISolidPrimitiveQuery*> (&eh.GetHandler ())))
        return NULL;

    ISolidPrimitivePtr  primitive;

    if (SUCCESS != solidQuery->GetSolidPrimitive (eh, primitive))
        return NULL;

    if (simplify)
        ISolidPrimitive::Simplify (primitive);

    return primitive;
    }

#ifdef WIP_CFGVAR
DEFINE_CFGVAR_CHECKER (MS_NO_STROKE_NONPLANAR_POLYGONS)
DEFINE_CFGVAR_CHECKER (MS_STROKE_NONPLANAR_POLYGONS_TOP_VIEW)
#endif

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  02/12
+===============+===============+===============+===============+===============+======*/
struct CurveVectorStroker : IStrokeForCache
{
CurveVectorPtr&     m_curves;
ICurvePathQuery&    m_query;

CurveVectorStroker (CurveVectorPtr& curves, ICurvePathQuery& query) : m_curves (curves), m_query (query) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool HaveValidCurves (ElementHandleCR eh)
    {
    if (m_curves.IsValid ())
        return true;

    m_query.GetCurveVector (eh, m_curves);

    return m_curves.IsValid ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawCurveVector (ElementHandleCR eh, ViewContextR context, bool isFilled)
    {
    if (!HaveValidCurves (eh))
        return;

    // NOTE: Always send outline to QVis as open profiles, avoids expensive QV topological analysis and problems with non-planar geometry...
    if (!isFilled && m_curves->IsAnyRegionType () && context.GetIViewDraw ().IsOutputQuickVision ())
        {
        if (DisplayHandler::Is3dElem (eh.GetElementCP ()))
            WireframeGeomUtil::DrawOutline (*m_curves, context.GetIDrawGeom ());
        else
            WireframeGeomUtil::DrawOutline2d (*m_curves, context.GetIDrawGeom (), context.GetDisplayPriority ());
        return;
        }

    if (DisplayHandler::Is3dElem (eh.GetElementCP ()))
        context.GetIDrawGeom ().DrawCurveVector (*m_curves, isFilled);
    else
        context.GetIDrawGeom ().DrawCurveVector2d (*m_curves, isFilled, context.GetDisplayPriority ());
    }

}; // CurveVectorStroker

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  02/12
+===============+===============+===============+===============+===============+======*/
struct CurveVectorFillStroker : CurveVectorStroker
{
CurveVectorFillStroker (CurveVectorPtr& curves, ICurvePathQuery& query) : CurveVectorStroker (curves, query) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool CheckLegacyNonPlanarShapes (ElementHandleCR eh)
    {
    return false;
#if defined (NEEDS_WORK_DGNITEM)
    if (!m_curves->IsAnyRegionType ())
        return false;

    // NOTE: Type check may become problematic...it's just for performance to avoid testing every curve vector.
    switch (eh.GetLegacyType())
        {
        case SHAPE_ELM:
        case CMPLX_SHAPE_ELM:
        case CELL_HEADER_ELM:
            {
            DgnElementCP  elm = eh.GetElementCP ();

            if (!DisplayHandler::Is3dElem (elm))
                return false;

            // NEEDSWORK_QV_CHANGE: Skip non-planar check if patterned until QVis supports mesh stencils...
            if (mdlElement_attributePresent (elm, PATTERN_ID, NULL))
                return false;

            if (CELL_HEADER_ELM == elm->GetLegacyType() && mdlElement_attributePresent (elm, LINKAGEID_AssocRegion, NULL))
                return false; // Assoc regions default behavior created flattened boundaries...

#ifdef WIP_CFGVAR // MS_NO_STROKE_NONPLANAR_POLYGONS
            if (CHECK_CFGVAR_IS_DEFINED(MS_NO_STROKE_NONPLANAR_POLYGONS))
                return false;
#endif

            break;
            }

        default:
            return false;
        }

    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString == m_curves->HasSingleCurvePrimitive () && m_curves->front ()->GetLineStringCP ()->size () < 5)
        return false; // Quick check to exclude triangles...

    if (m_curves->ContainsNonLinearPrimitive ())
        return false; // Ignore regions with curves...the badly non-planar areas we need to fix are always just linear segments... 

    Transform   localToWorld, worldToLocal;

    if (!m_curves->GetAnyFrenetFrame (localToWorld) || !worldToLocal.InverseOf (localToWorld))
        return false;

    DRange3d    range;

    if (!m_curves->GetRange (range, worldToLocal))
        return false;

    static double s_toleranceRatio = 1.0e-4;
    double planarTolerance = (range.low.DistanceXY (range.high)) * s_toleranceRatio;

    return (fabs (range.high.z - range.low.z) > planarTolerance);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool StrokeIfNonPlanar (ElementHandleCR eh, ViewContextR context)
    {
    // NOTE: Only need non-planar check when creating QvElem for fill/surface.
    if (!context.GetIViewDraw ().IsOutputQuickVision ())
        return false;

    if (!CheckLegacyNonPlanarShapes (eh))
        return false;

    IFacetOptionsPtr  options = IFacetOptions::CreateForCurves ();

    options->SetParamsRequired (true);

    IPolyfaceConstructionPtr  builder = IPolyfaceConstruction::New (*options);

    bvector<DPoint3d> points;
    size_t            nLoops;

    builder->Stroke (*m_curves, points, nLoops);

#ifdef WIP_CFGVAR // MS_NO_STROKE_NONPLANAR_POLYGONS
    if (CHECK_CFGVAR_IS_DEFINED(MS_STROKE_NONPLANAR_POLYGONS_TOP_VIEW))
        {
        // Make fixed Z points to get transforms rotated around z to first edge.
        for (DPoint3dR xyz: points)
            {
            if (!xyz.IsDisconnect ())
                xyz.z = points[0].z;
            }
        }
#endif

    if (!builder->AddTriangulation (points))
        return false;

    context.GetIDrawGeom ().DrawPolyface (builder->GetClientMeshR (), true);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _StrokeForCache (CachedDrawHandleCR dh, ViewContextR context, double pixelSize) override
    {
    ElementHandleCR eh = *dh.GetElementHandleCP();
    if (!HaveValidCurves (eh))
        return;

    if (StrokeIfNonPlanar (eh, context))
        return;

    DrawCurveVector (eh, context, true);
    }

}; // CurveVectorFillStroker

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  02/12
+===============+===============+===============+===============+===============+======*/
struct CurveVectorOutlineStroker : CurveVectorStroker
{
/*----------------------------------------------------------------------------------*//**
* @bsiclass                                                     Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct HasStyleChanges : IQueryProperties
    {
    bool              m_hasChanges;
    bool              m_colorValid;
    bool              m_weightValid;
    bool              m_styleValid;
    bool              m_levelValid;
    bool              m_eClassValid;

    UInt32            m_color;
    UInt32            m_weight;
    Int32             m_style;
    LineStyleParams   m_styleParams;
    LevelId           m_level;
    DgnElementClass   m_eClass;

    HasStyleChanges ()
        {
        m_hasChanges    = false;

        m_colorValid    = false;
        m_weightValid   = false;
        m_styleValid    = false;
        m_levelValid    = false;
        m_eClassValid   = false;

        m_color         = 0;
        m_weight        = 0;
        m_style         = 0;
        m_eClass        = DgnElementClass::Primary;

        m_styleParams.Init ();
        }

    virtual ElementProperties _GetQueryPropertiesMask () override {return (ElementProperties) (ELEMENT_PROPERTY_Color | ELEMENT_PROPERTY_Linestyle | ELEMENT_PROPERTY_Weight | ELEMENT_PROPERTY_Level | ELEMENT_PROPERTY_ElementClass);}

    bool CheckValue (EachPropertyBaseArg& arg)
        {
        if (m_hasChanges || 0 == (arg.GetPropertyFlags () & (PROPSCALLBACK_FLAGS_IsBaseID)))
            return false;

        return (0 == (arg.GetPropertyFlags () & (PROPSCALLBACK_FLAGS_ElementIgnoresID | PROPSCALLBACK_FLAGS_UndisplayedID)));
        }

    virtual void _EachColorCallback (EachColorArg& arg) override
        {
        if (!CheckValue (arg))
            return;

        if (m_colorValid)
            {
            m_hasChanges = (m_color != arg.GetStoredValue ());
            return;
            }

        m_color = arg.GetStoredValue ();
        m_colorValid = true;
        }

    virtual void _EachWeightCallback (EachWeightArg& arg) override
        {
        if (!CheckValue (arg))
            return;

        if (m_weightValid)
            {
            m_hasChanges = (m_weight != arg.GetStoredValue ());
            return;
            }

        m_weight = arg.GetStoredValue ();
        m_weightValid = true;
        }

    virtual void _EachLineStyleCallback (EachLineStyleArg& arg) override
        {
        if (!CheckValue (arg))
            return;

        if (m_styleValid)
            {
            m_hasChanges = (m_style != arg.GetStoredValue ());

            // Only care about start/end width changes...ignore differences like segment mode...
            if (m_hasChanges || (0 == m_styleParams.modifiers && 0 == arg.GetParams ()->modifiers))
                return;

            double  start0, start1, end0, end1;

            start0 = (0 != (STYLEMOD_SWIDTH & m_styleParams.modifiers) ? m_styleParams.startWidth : 0.0);
            end0   = (0 != (STYLEMOD_EWIDTH & m_styleParams.modifiers) ? m_styleParams.endWidth   : 0.0);

            start1 = (0 != (STYLEMOD_SWIDTH & arg.GetParams ()->modifiers) ? arg.GetParams ()->startWidth : 0.0);
            end1   = (0 != (STYLEMOD_EWIDTH & arg.GetParams ()->modifiers) ? arg.GetParams ()->endWidth   : 0.0);

            m_hasChanges = (!DoubleOps::AlmostEqual (start0, start1) || !DoubleOps::AlmostEqual (end0, end1));
            return;
            }

        m_style = arg.GetStoredValue ();
        m_styleParams = *arg.GetParams ();
        m_styleValid = true;
        }

    virtual void _EachLevelCallback (EachLevelArg& arg) override
        {
        if (!CheckValue (arg))
            return;

        if (m_levelValid)
            {
            m_hasChanges = (m_level != arg.GetStoredValue ());
            return;
            }

        m_level = arg.GetStoredValue ();
        m_levelValid = true;
        }

    virtual void _EachElementClassCallback (EachElementClassArg& arg) override
        {
        if (!CheckValue (arg))
            return;

        if (m_eClassValid)
            {
            m_hasChanges = (m_eClass != arg.GetStoredValue ());
            return;
            }

        m_eClass = arg.GetStoredValue ();
        m_eClassValid = true;
        }

    }; // HasStyleChanges

/*----------------------------------------------------------------------------------*//**
* @bsiclass                                                     Keith.Bentley   04/03
+---------------+---------------+---------------+---------------+---------------+------*/
struct ChainTangentInfo
    {
    private:

    DPoint3d    m_point;
    DVec3d      m_tangent;
    bool        m_isValid;

    public:

    ChainTangentInfo () {m_isValid = false;}

    bool        IsValid () {return m_isValid;}
    DPoint3dCR  GetPoint () {return m_point;}
    DVec3dCR    GetTangent () {return m_tangent;}
    void        SetValid (bool yesNo) {m_isValid = yesNo;}
    void        Init (DPoint3dCR point, DVec3dCR tangent) {m_point = point; m_tangent = tangent; m_isValid = true;}

    }; // ChainTangentInfo

CurveVectorOutlineStroker (CurveVectorPtr& curves, ICurvePathQuery& query) : CurveVectorStroker (curves, query) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _StrokeForCache (CachedDrawHandleCR dh, ViewContextR context, double pixelSize) override
    {
    ElementHandleCR eh = *dh.GetElementHandleCP();

    DrawCurveVector (eh, context, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/05
+---------------+---------------+---------------+---------------+---------------+------*/
static bool HasLegacyStyleChanges (ElementHandleCR eh, QvElem* qvElem)
    {
    // If we already have qv elem assume it's ok to use again...
    if (NULL != qvElem)
        return false;

    HasStyleChanges   remapper;

    PropertyContext::QueryElementProperties (eh, &remapper);

    return (remapper.m_hasChanges);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   04/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void GetChainTangents (ChainTangentInfo* startInfo, ChainTangentInfo* endInfo, ICurvePrimitiveCR curvePrimitive)
    {
    bool        isPoint = (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line == curvePrimitive.GetCurvePrimitiveType () && 0.0 == curvePrimitive.GetLineCP ()->Length ());
    DVec3d      tangents[2]; 
    DPoint3d    points[2];

    if (isPoint || !curvePrimitive.GetStartEnd (points[0], points[1], tangents[0], tangents[1]))
        {
        if (startInfo)
            startInfo->SetValid (false);

        if (endInfo)
            endInfo->SetValid (false);

        return;
        }

    tangents[0].Negate ();

    if (startInfo)
        startInfo->Init (points[0], tangents[0]);

    if (endInfo)
        endInfo->Init (points[1], tangents[1]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void DrawStyled (ElementHandleCP templateEh, ViewContextR context, CurveVectorCR curves, bool is3d, double zDepth, bool setSymbology)
    {
    if (1 > curves.size ())
        return;

#if defined (NEEDS_WORK_DGNITEM)
    if (curves.IsUnionRegion () || curves.IsParityRegion ())
        {
        ChildElemIter childTemplateEh = (IsCmplxPathHdr (templateEh) ? ChildElemIter (*templateEh, ExposeChildrenReason::Count) : ChildElemIter ());

        for (ICurvePrimitivePtr curve: curves)
            {
            if (curve.IsNull ())
                continue;

            if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType ())
                {
                BeAssert (false && "Unexpected entry in union/parity region.");

                return; // Each loop must be a child curve bvector (a closed loop or parity region for a union region)...
                }

            if (childTemplateEh.IsValid ()) // Support style changes/associations for legacy complex types...
                {
                if (setSymbology && !IsCmplxPathHdr (&childTemplateEh)) // Set symbology for simple loops...
                    {
                    context.CookElemDisplayParams (childTemplateEh);
                    context.ActivateOverrideMatSymb ();
                    }

                DrawStyled (&childTemplateEh, context, *curve->GetChildCurveVectorCP (), is3d, zDepth, setSymbology);
            
                childTemplateEh = childTemplateEh.ToNext ();
                }
            else
                {
                DrawStyled (&childTemplateEh, context, *curve->GetChildCurveVectorCP (), is3d, zDepth, setSymbology);
                }
            }
        }
    else
#endif
        {
        bool              isClosed  = curves.IsClosedPath ();
        bool              isComplex = (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Invalid == curves.HasSingleCurvePrimitive ());
        ChainTangentInfo  currEnd, prevEnd, nextEnd, currStart, nextStart, chainStart;

#if defined (NEEDS_WORK_DGNITEM)
        ChildElemIter     childTemplateEh = (IsCmplxPathHdr (templateEh) ? ChildElemIter (*templateEh, ExposeChildrenReason::Count) : ChildElemIter ());

        if (isComplex) // Support start/end tangents for linestyle w/thickness...
            {
            GetChainTangents (&currStart, &currEnd, *curves.front ());

            if (isClosed)
                GetChainTangents (NULL, &prevEnd, *curves.back ());

            chainStart = currStart;
            }
#endif

        DPoint3d    startTangent, endTangent;
        DPoint3d    *pStartTangent, *pEndTangent;
        size_t      nCmpns = curves.size ();

        for (size_t iCmpn = 0; iCmpn < nCmpns; ++iCmpn)
            {
            ICurvePrimitivePtr curve = curves.at (iCmpn);

            if (!curve.IsValid ())
                continue;

#if defined (NEEDS_WORK_DGNITEM)
            if (isComplex) // Support start/end tangents for linestyle w/thickness...
                {
                ICurvePrimitivePtr nextCurve = iCmpn < nCmpns-1 ? curves.at (iCmpn+1) : NULL;

                if (!nextCurve.IsValid ())
                    {
                    if (isClosed)
                        {
                        nextStart = chainStart;
                        }
                    else
                        {
                        nextStart.SetValid (false);
                        nextEnd.SetValid (false);
                        }
                    }
                else
                    {
                    GetChainTangents (&nextStart, &nextEnd, *nextCurve);
                    }
                }
#endif

            pStartTangent = pEndTangent = NULL;

            if (prevEnd.IsValid () && currStart.IsValid ())
                {
                if (currStart.GetTangent ().DotProduct (prevEnd.GetTangent ()) < TOLERANCE_ChainMiterCosLimit)
                    {
                    startTangent.DifferenceOf (currStart.GetTangent (), prevEnd.GetTangent ());

                    if (0.0 != startTangent.Normalize ())
                        pStartTangent = &startTangent;
                    }
                }

            if (currEnd.IsValid () && nextStart.IsValid ())
                {
                if (currEnd.GetTangent ().DotProduct (nextStart.GetTangent ()) < TOLERANCE_ChainMiterCosLimit)
                    {
                    endTangent.DifferenceOf (currEnd.GetTangent (), nextStart.GetTangent ());

                    if (0.0 != endTangent.Normalize ())
                        pEndTangent = &endTangent;
                    }
                }

            bool    isVisibleCmpn = true;

            context.SetLinestyleTangents (pStartTangent, pEndTangent); // NOTE: This needs to happen before CookElemDisplayParams to setup modifiers!

#if defined (NEEDS_WORK_DGNITEM)
            if (childTemplateEh.IsValid () && setSymbology)
                {
                DisplayHandlerP dHandler = childTemplateEh.GetDisplayHandler ();

                isVisibleCmpn = (dHandler ? dHandler->IsVisible (childTemplateEh, context, false, true, true) : true);
                }

            // Don't advance template iterator for gap segments...
            if (childTemplateEh.IsValid () && !curve->GetMarkerBit (ICurvePrimitive::CURVE_PRIMITIVE_BIT_GapCurve))
                {
                if (setSymbology) // Support style changes/associations for legacy complex types...
                    {
                    context.CookElemDisplayParams (childTemplateEh);
                    context.ActivateOverrideMatSymb ();
                    }
                else if (isComplex && ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line == curve->GetCurvePrimitiveType () && 0.0 == curve->GetLineCP ()->Length ())
                    {
                    // Preserve current tangents and xElemPhaseSet flag when a point is encountered in a complex shape or chain...
                    }
                else
                    {
                    context.CookDisplayParams (); // Set/Clear linestyle start/end tangent modifiers. (constant width so setSymbology is false?)
                    }
                
                childTemplateEh = childTemplateEh.ToNext ();
                }
            else if (isComplex)
                {
                context.CookDisplayParams (); // Set/Clear linestyle start/end tangent modifiers. (constant width so setSymbology is false?)
                }
#endif

            if (isVisibleCmpn)
                {
                switch (curve->GetCurvePrimitiveType ())
                    {
                    case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                        {
                        DSegment3d  segment = *curve->GetLineCP ();

                        if (is3d)
                            {
                            context.DrawStyledLineString3d (2, segment.point, NULL);
                            break;
                            }

                        DPoint2d    points[2];

                        points[0].Init (segment.point[0]);
                        points[1].Init (segment.point[1]);
                    
                        context.DrawStyledLineString2d (2, points, zDepth, NULL);
                        break;
                        }

                    case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                        {
                        bvector<DPoint3d> const* points = curve->GetLineStringCP ();

                        if (is3d)
                            {
                            context.DrawStyledLineString3d ((int) points->size (), &points->front (), NULL, !isComplex && isClosed);
                            break;
                            }

                        int                      nPts = (int) points->size ();
                        std::valarray<DPoint2d>  localPoints2dBuf (nPts);

                        for (int iPt = 0; iPt < nPts; ++iPt)
                            {
                            DPoint3dCP  tmpPt = &points->front ()+iPt;

                            localPoints2dBuf[iPt].x = tmpPt->x;
                            localPoints2dBuf[iPt].y = tmpPt->y;
                            }

                        context.DrawStyledLineString2d (nPts, &localPoints2dBuf[0], zDepth, NULL, !isComplex && isClosed);
                        break;
                        }

                    case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                        {
                        DEllipse3d  ellipse = *curve->GetArcCP ();

                        if (is3d)
                            {
                            context.DrawStyledArc3d (ellipse, !isComplex && isClosed, NULL);
                            break;
                            }

                        context.DrawStyledArc2d (ellipse, !isComplex && isClosed, zDepth, NULL);
                        break;
                        }

                    case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
                    case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
                    case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
                    case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
                        {
                        MSBsplineCurveCP bcurve = curve->GetProxyBsplineCurveCP ();
        
                        if (is3d)
                            {
                            context.DrawStyledBSplineCurve3d (*bcurve);
                            break;
                            }

                        context.DrawStyledBSplineCurve2d (*bcurve, zDepth);
                        break;
                        }

                    default:
                        {
                        BeAssert (false && "Unexpected entry in CurveVector.");
                        break;
                        }
                    }
                }

            prevEnd   = currEnd;
            currStart = nextStart;
            currEnd   = nextEnd;
            }

        context.SetLinestyleTangents (NULL, NULL); // Make sure we clear linestyle tangents...
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
static void DrawStyledCurveVector3d (ViewContextR context, CurveVectorCR curve)
    {
    if (NULL == context.GetCurrLineStyle (NULL))
        {
        if (context.GetIViewDraw ().IsOutputQuickVision ())
            WireframeGeomUtil::DrawOutline (curve, context.GetIDrawGeom ());
        else
            context.GetIDrawGeom ().DrawCurveVector (curve, false);
            
        return;
        }

    DrawStyled (NULL, context, curve, true, 0.0, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
static void DrawStyledCurveVector2d (ViewContextR context, CurveVectorCR curve, double zDepth)
    {
    if (NULL == context.GetCurrLineStyle (NULL))
        {
        if (context.GetIViewDraw ().IsOutputQuickVision ())
            WireframeGeomUtil::DrawOutline2d (curve, context.GetIDrawGeom (), zDepth);
        else
            context.GetIDrawGeom ().DrawCurveVector2d (curve, false, zDepth);
            
        return;
        }

    DrawStyled (NULL, context, curve, false, zDepth, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawStyledCurveVector (ElementHandleCR eh, ViewContextR context, bool setSymbology)
    {
    if (!HaveValidCurves (eh))
        return;
    
    // Only open/closed paths are valid for linestyle display. (ex. exclude point strings)
    if (CurveVector::BOUNDARY_TYPE_None == m_curves->GetBoundaryType ())
        {
        DrawCurveVector (eh, context, false);
        return;
        }

    bool    is3d = DisplayHandler::Is3dElem (eh.GetElementCP ());
#if defined (NEEDS_WORK_DGNITEM)
    bool    isPathHdr = IsCmplxPathHdr (&eh);

    bool    isAssocRegion = isPathHdr && mdlElement_attributePresent (eh.GetElementCP (), LINKAGEID_AssocRegion, NULL);
    // NOTE: We need to skip "extra" complex header for assoc regions that aren't union regions so that template matches up with primitives being drawn...
    if (isAssocRegion && !m_curves->IsUnionRegion ())
        {
        ChildElemIter childTemplateEh = ChildElemIter (eh, ExposeChildrenReason::Count);

        DrawStyled (&childTemplateEh, context, *m_curves, is3d, is3d ? 0.0 : context.GetDisplayPriority (), setSymbology);
        return;
        }
#endif

    DrawStyled (NULL, context, *m_curves, is3d, is3d ? 0.0 : context.GetDisplayPriority (), setSymbology);
    }

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool DirectDrawSimpleLegacyTypes (ElementHandleCR eh, ViewContextR context)
    {
    // If we've already gotten the curve vector to draw thickness/fill use it for outline...
    if (m_curves.IsValid ())
        return false;

    // Optimize single-threaded draw of legacy types to work directly off element data and avoid getting a curve vector...
    switch (eh.GetLegacyType())
        {
        case LINE_ELM:
            {
            DgnElementCP el = eh.GetElementCP ();

            if (el->Is3d())
                {
                context.DrawStyledLineString3d (2, &el->ToLine_3d().start, &el->GetRange().low);
                }
            else
                {
                DPoint2d    range[2];
                DisplayHandler::GetDPRange (range, &el->GetRange());
                context.DrawStyledLineString2d (2, &el->ToLine_2d().start, context.GetDisplayPriority (), range);
                }

            return true;
            }

        case POINT_STRING_ELM:
            {
            DgnElementCP el = eh.GetElementCP ();

            if (el->IsHole())
                {
                if (el->Is3d())
                    {
                    Line_String_3d const*  ls = &el->ToLine_String_3d();

                    if (ls->numverts < 1)
                        return true; // Bad element...ignore...

                    context.GetIDrawGeom().DrawPointString3d (ls->numverts, ls->vertice, &el->GetRange().low);
                    }
                else
                    {
                    Line_String_2d const*  ls = &el->ToLine_String_2d();

                    if (ls->numverts < 1)
                        return true; // Bad element...ignore...

                    DPoint2d    range[2];
                    DisplayHandler::GetDPRange (range, &el->GetRange());
                    context.GetIDrawGeom().DrawPointString2d (ls->numverts, ls->vertice, context.GetDisplayPriority (), range);
                    }

                return true;
                }

            // FALL THROUGH...
            }

        case LINE_STRING_ELM:
        case SHAPE_ELM:
            {
            DgnElementCP el = eh.GetElementCP ();

            if (el->Is3d())
                {
                context.DrawStyledLineString3d (el->ToLine_String_3d().numverts, el->ToLine_String_3d().vertice, &el->GetRange().low, SHAPE_ELM == el->GetLegacyType());
                }
            else
                {
                DPoint2d    range[2];
                DisplayHandler::GetDPRange (range, &el->GetRange());
                context.DrawStyledLineString2d (el->ToLine_String_2d().numverts, el->ToLine_String_2d().vertice, context.GetDisplayPriority (), range, SHAPE_ELM == el->GetLegacyType());
                }

            return true;
            }

        case ARC_ELM:
            {
            DgnElementCP el = eh.GetElementCP ();
            DEllipse3d  ellipse;

            if (el->Is3d())
                {
                Arc_3d const* arc = &el->ToArc_3d();
                ellipse.initFromDGNFields3d (&arc->origin, const_cast <double *> (arc->quat), NULL, NULL, arc->primary, arc->secondary, &arc->startAngle, &arc->sweepAngle);
                context.DrawStyledArc3d (ellipse, false, &el->GetRange().low);
                }
            else
                {
                Arc_2d const*   arc = &el->ToArc_2d();
                DPoint2d        range[2];

                ellipse.initFromDGNFields2d (&arc->origin, &arc->rotationAngle, NULL, arc->primary, arc->secondary, &arc->startAngle, &arc->sweepAngle, 0.0);
                DisplayHandler::GetDPRange (range, &el->GetRange());
                context.DrawStyledArc2d (ellipse, false, context.GetDisplayPriority (), range);
                }

            return true;
            }

        case ELLIPSE_ELM:
            {
            DgnElementCP el = eh.GetElementCP ();
            DEllipse3d  ellipse;

            if (el->Is3d())
                {
                Ellipse_3d const*   arc = &el->ToEllipse_3d();
                double              startRadians = Angle::DegreesToRadians (0.0);

                ellipse.initFromDGNFields3d (&arc->origin, const_cast <double *> (arc->quat), NULL, NULL, arc->primary, arc->secondary, &startRadians, NULL);
                context.DrawStyledArc3d (ellipse, true, &el->GetRange().low);
                }
            else
                {
                Ellipse_2d const*   arc = &el->ToEllipse_2d();
                DPoint2d            range[2];

                ellipse.initFromDGNFields2d (&arc->origin, &arc->rotationAngle, NULL, arc->primary, arc->secondary, NULL, NULL, 0.0);
                DisplayHandler::GetDPRange (range, &el->GetRange());
                context.DrawStyledArc2d (ellipse, true, context.GetDisplayPriority (), range);
                }

            return true;
            }

        default:
            return false;
        }
    }
#endif

}; // CurveVectorOutlineStroker

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  02/12
+===============+===============+===============+===============+===============+======*/
struct CurveVectorThicknessStroker : CurveVectorStroker
{
/*----------------------------------------------------------------------------------*//**
* @bsiclass                                                     Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct ExtrudeLStyleGraphics : IElementGraphicsProcessor
    {
    protected:

    ElementHandleCR             m_eh;
    ViewContextR                m_context;
    Transform                   m_transform;
    CurveVectorOutlineStroker&  m_stroker;

    public:

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Brien.Bastings  11/2009
    +---------------+---------------+---------------+---------------+---------------+------*/
    explicit ExtrudeLStyleGraphics (ElementHandleCR eh, ViewContextR context, CurveVectorOutlineStroker& stroker) : m_eh (eh), m_context (context), m_stroker (stroker)
        {
        m_transform.InitIdentity ();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Brien.Bastings  11/2009
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool _WantClipping () const override {return false;}
    virtual bool _ExpandLineStyles (ILineStyleCP lsStyle) const override {return true;}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Brien.Bastings  11/2009
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void _AnnounceTransform (TransformCP trans) override
        {
        if (trans)
            m_transform = *trans;
        else
            m_transform.initIdentity ();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Brien.Bastings  06/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual BentleyStatus _ProcessTextString (TextStringCR text) override
        {
        return ERROR; // Output glyph geometry...
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Brien.Bastings  11/2009
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual BentleyStatus _ProcessCurveVector (CurveVectorCR curves, bool isFilled) override
        {
        ElemDisplayParamsP  params = m_context.GetCurrentDisplayParams ();
        bool                isCapped = false;
        DVec3dCP            thicknessVector = params->GetThickness (isCapped);
        CurveVectorPtr      tmpCurves = curves.Clone ();
        DgnExtrusionDetail  detail (tmpCurves, thicknessVector ? *thicknessVector : DVec3d::From (0.0, 0.0, 1.0), isCapped && curves.IsAnyRegionType ());

        if (!m_transform.IsIdentity ())
            detail.m_baseCurve->TransformInPlace (m_transform);

        ISolidPrimitivePtr  primitive = ISolidPrimitive::CreateDgnExtrusion (detail);

        m_context.GetIDrawGeom().DrawSolidPrimitive (*primitive);

        return SUCCESS;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Brien.Bastings  02/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void _OutputGraphics (ViewContextR context) override
        {
        context.CookElemDisplayParams (m_eh);
        context.ActivateOverrideMatSymb ();

        m_stroker.DrawStyledCurveVector (m_eh, context, true);
        }

    }; // ExtrudeLStyleGraphics

CurveVectorThicknessStroker (CurveVectorPtr& curves, ICurvePathQuery& query) : CurveVectorStroker (curves, query) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool DrawLineStyleWithThickness (ElementHandleCR eh, ViewContextR context)
    {
    ILineStyleCP lineStyle = context.GetCurrLineStyle (NULL);

    if (NULL == lineStyle)
        return false;

    ILineStyleComponentCP lsComp = lineStyle->_GetComponent ();

    if (lsComp->_IsContinuous () && !lsComp->_HasWidth ())
        return false;

    CurveVectorOutlineStroker  stroker (m_curves, m_query);
    ExtrudeLStyleGraphics      extrude (eh, context, stroker);
    
    ElementGraphicsOutput::Process (extrude, context.GetDgnProject ());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _StrokeForCache (CachedDrawHandleCR dh, ViewContextR context, double pixelSize) override
    {
    ElementHandleCR eh = *dh.GetElementHandleCP();
    if (!HaveValidCurves (eh))
        return;

    if (!DisplayHandler::Is3dElem (eh.GetElementCP ()))
        return;

    if (DrawLineStyleWithThickness (eh, context))
        return;

    ElemDisplayParamsP  params = context.GetCurrentDisplayParams ();
    bool                isCapped = false;
    DVec3dCP            thicknessVector = params->GetThickness (isCapped);

    // SPECIAL CASE: Represent single circle with thickness as a cone...
    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc == m_curves->HasSingleCurvePrimitive ())
        {
        DEllipse3d  ellipse = *m_curves->front ()->GetArcCP ();

        if (ellipse.IsCircular () && ellipse.IsFullEllipse ())
            {
            double      r0, r90, start, sweep;
            DPoint3d    origin, endPt;
            RotMatrix   mtx;

            ellipse.GetScaledRotMatrix (origin, mtx, r0, r90, start, sweep);
            endPt.SumOf (origin, thicknessVector ? *thicknessVector : DVec3d::From (0.0, 0.0, 1.0));

            DgnConeDetail       detail (origin, endPt, mtx, r0, r90, isCapped && m_curves->IsAnyRegionType ());
            ISolidPrimitivePtr  primitive = ISolidPrimitive::CreateDgnCone (detail);

            context.GetIDrawGeom ().DrawSolidPrimitive (*primitive);
            return;
            }
        }

    DgnExtrusionDetail  detail (m_curves, thicknessVector ? *thicknessVector : DVec3d::From (0.0, 0.0, 1.0), isCapped && m_curves->IsAnyRegionType ());
    ISolidPrimitivePtr  primitive = ISolidPrimitive::CreateDgnExtrusion (detail);

    context.GetIDrawGeom ().DrawSolidPrimitive (*primitive);
    }

}; // CurveVectorThicknessStroker

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawCurveVector (ElementHandleCR eh, ICurvePathQueryR query, GeomRepresentations info, bool allowCachedOutline)
    {
    CurveVectorPtr  curves;
    CachedDrawHandle dh(&eh);

    if (0 != (info & DISPLAY_INFO_Thickness))
        {
        CurveVectorThicknessStroker  stroker (curves, query);

        DrawWithThickness (eh, stroker, 2);
        }

    if (0 != (info & (DISPLAY_INFO_Fill | DISPLAY_INFO_Surface)))
        {
        CurveVectorFillStroker  stroker (curves, query);

        DrawCached (eh, stroker, 1);
        }

#if defined (NEEDS_WORK_DGNITEM)
    if (0 != (info & DISPLAY_INFO_Edge))
        {
        CurveVectorOutlineStroker  stroker (curves, query);

        if (allowCachedOutline || !stroker.DirectDrawSimpleLegacyTypes (eh, *this))
            {
            bool    isPathHdr = stroker.IsCmplxPathHdr (&eh);
            bool    hasLegacyStyleChanges = (isPathHdr ? (NULL != GetCurrLineStyle (NULL) || stroker.HasLegacyStyleChanges (eh, allowCachedOutline ? GetQvCacheElem (dh, 0, 0.0) : NULL)) : false);

            // NOTE: Need to push path entries for legacy complex types for pick, old code could be relying on components in hit path...
            if (!hasLegacyStyleChanges && NULL == GetCurrLineStyle (NULL) && (NULL == GetIPickGeom () || !isPathHdr))
                {
                if (allowCachedOutline)
                    DrawCached (eh, stroker, 0);
                else
                    stroker.DrawCurveVector (eh, *this, false);
                }
            else
                {
                stroker.DrawStyledCurveVector (eh, *this, hasLegacyStyleChanges);
                }
            }
        }
#endif

    if (0 != (info & DISPLAY_INFO_Pattern))
        {
        CurveVectorFillStroker           stroker (curves, query);
        ViewContext::ClipStencil         clipStencil (stroker, 1);
        ViewContext::PatternParamSource  patParamSrc;
        
        DrawAreaPattern (eh, clipStencil, patParamSrc); // NOTE: Changes current matsymb...should do this last...
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::DrawStyledCurveVector3d (CurveVectorCR curve)
    {
    CurveVectorOutlineStroker::DrawStyledCurveVector3d (*this, curve);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::DrawStyledCurveVector2d (CurveVectorCR curve, double zDepth)
    {
    CurveVectorOutlineStroker::DrawStyledCurveVector2d (*this, curve, zDepth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static double wireframe_getTolerance (CurveVectorCR curves)
    {
    static double s_minRuleLineTolerance = 1.0e-8;
   
    return  curves.ResolveTolerance (s_minRuleLineTolerance);      // TFS# 24423 - Length calculation can be very slow for B-Curves.  s_defaultLengthRelTol * curves.Length ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void wireframe_addRulePoints (bvector<DPoint3d>& pts, bvector<bool>& interior, DPoint3dCR pt, bool isVertex, double ruleTolerance, bool checkDistance)
    {
    if (checkDistance && 0 != pts.size ())
        {
        if (pt.Distance (pts.back ()) <= ruleTolerance)
            return; // Don't duplicate previous point...
        else if (pt.Distance (pts.front ()) <= ruleTolerance)
            return; // Don't duplicate first point...
        }

    pts.push_back (pt);
    interior.push_back (!isVertex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool wireframe_collectPoints (CurveVectorCR curves, bvector<DPoint3d>& pts, bvector<bool>& interior, double ruleTolerance, bool checkDistance, ViewContextP context)
    {
    if (1 > curves.size ())
        return false;

    if (curves.IsUnionRegion () || curves.IsParityRegion ())
        {
        for (ICurvePrimitivePtr curve: curves)
            {
            if (curve.IsNull () || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType ())
                continue;

            if (context && context->CheckStop ())
                return true;

            wireframe_collectPoints (*curve->GetChildCurveVectorCP (), pts, interior, ruleTolerance, checkDistance, context);
            }
        }
    else
        {
        for (ICurvePrimitivePtr curve: curves)
            {
            if (!curve.IsValid ())
                continue;

            if (context && context->CheckStop ())
                return true;

            switch (curve->GetCurvePrimitiveType ())
                {
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                    {
                    DSegment3dCP  segment = curve->GetLineCP ();

                    wireframe_addRulePoints (pts, interior, segment->point[0], true, ruleTolerance, checkDistance);
                    wireframe_addRulePoints (pts, interior, segment->point[1], true, ruleTolerance, checkDistance);
                    break;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                    {
                    bvector<DPoint3d> const* points = curve->GetLineStringCP ();

                    for (size_t iPt = 0; iPt < points->size (); ++iPt)
                        wireframe_addRulePoints (pts, interior, points->at (iPt), true, ruleTolerance, checkDistance);
                    break;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                    {
                    DEllipse3dCP  ellipse = curve->GetArcCP ();

                    if (ellipse->IsFullEllipse ())
                        {
                        for (size_t iRule = 0; iRule < 4; ++iRule)
                            {
                            double    theta = iRule * Angle::PiOver2 ();
                            DPoint3d  point;

                            ellipse->Evaluate (point, theta);
                            wireframe_addRulePoints (pts, interior, point, false, ruleTolerance, checkDistance);
                            }
                        break;
                        }

                    double      start, sweep;
                    DPoint3d    startPt, endPt;

                    ellipse->GetSweep (start, sweep);
                    ellipse->EvaluateEndPoints (startPt, endPt);

                    int  interiorPts = (int) (fabs (sweep) / (0.5 * Angle::Pi ()));

                    wireframe_addRulePoints (pts, interior, startPt, true, ruleTolerance, checkDistance);

                    if (interiorPts > 0)
                        {
                        int     i;
                        double  theta, delta = sweep / (double) (interiorPts);

                        for (i=0, theta = start + delta; i < interiorPts; i++, theta += delta)
                            {
                            DPoint3d  point;

                            ellipse->Evaluate (point, theta);
                            wireframe_addRulePoints (pts, interior, point, DoubleOps::AlmostEqual (theta, start + sweep), ruleTolerance, checkDistance);
                            }
                        }

                    wireframe_addRulePoints (pts, interior, endPt, true, ruleTolerance, checkDistance);
                    break;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
                    {
                    bvector<DPoint3d> const* points = curve->GetAkimaCurveCP ();

                    for (size_t iPt = 2; iPt < points->size ()-2; ++iPt)
                        wireframe_addRulePoints (pts, interior, points->at (iPt), 2 == iPt || points->size ()-2 == iPt+1, ruleTolerance, checkDistance);
                    break;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
                    {
                    MSBsplineCurveCP bcurve = curve->GetProxyBsplineCurveCP ();
                    int              iPt, numPoints = bcurve->params.numPoles;
                    double           r, delta = 1.0 / (bcurve->params.closed ? bcurve->params.numPoles : bcurve->params.numPoles-1);

                    for (iPt = 0, r = 0.0; iPt < numPoints; r += delta, iPt++)
                        {
                        DPoint3d    point;

                        bcurve->FractionToPoint (point, r);
                        wireframe_addRulePoints (pts, interior, point, bcurve->params.closed ? false : (0 == iPt || numPoints == iPt+1), ruleTolerance, checkDistance);
                        }
                    break;
                    }

                default:
                    break;
                }
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool wireframe_collectArcMidPoints (CurveVectorCR curves, bvector<DPoint3d>& pts, bvector<bool>& interior, double ruleTolerance, bool checkDistance, ViewContextP context)
    {
    if (1 > curves.size ())
        return false;

    if (curves.IsUnionRegion () || curves.IsParityRegion ())
        {
        for (ICurvePrimitivePtr curve: curves)
            {
            if (curve.IsNull () || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType ())
                continue;

            if (context && context->CheckStop ())
                return true;

            wireframe_collectArcMidPoints (*curve->GetChildCurveVectorCP (), pts, interior, ruleTolerance, checkDistance, context);
            }
        }
    else
        {
        for (ICurvePrimitivePtr curve: curves)
            {
            if (!curve.IsValid () || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc != curve->GetCurvePrimitiveType ())
                continue;

            if (context && context->CheckStop ())
                return true;

            DPoint3d    point;

            curve->FractionToPoint (0.5, point);
            wireframe_addRulePoints (pts, interior, point, true, ruleTolerance, checkDistance);
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool wireframe_computeArc (DEllipse3dR ellipse, DPoint3dCR startPt, DPoint3dCR originPt, double sweepAngle, TransformCR transform, RotMatrixCR axes, RotMatrixCR invAxes, double ruleTolerance)
    {
    DPoint3d    endPt, centerPt, tmpPt;

    transform.Multiply (&endPt, &startPt, 1);
    centerPt = originPt;

    tmpPt = startPt;
    axes.Multiply (tmpPt);
    axes.Multiply (centerPt);
    centerPt.z = tmpPt.z;

    DVec3d      xVec, yVec, zVec;
    RotMatrix   rMatrix;

    zVec.Init (0.0, 0.0, 1.0);
    xVec.NormalizedDifference (tmpPt, centerPt);
    yVec.CrossProduct (zVec, xVec);
    rMatrix.InitFromColumnVectors (xVec, yVec, zVec);
    rMatrix.InitProduct (invAxes, rMatrix);
    axes.MultiplyTranspose (centerPt);

    double  radius = centerPt.Distance (startPt);

    if (radius < ruleTolerance)
        return false;

    ellipse.InitFromScaledRotMatrix (centerPt, rMatrix, radius, radius, 0.0, sweepAngle);

    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct StrokeSurfaceCurvesInfo
    {
    ViewContextR        m_context;
    MSBsplineSurfaceCR  m_surface;
    bool                m_includeEdges;
    bool                m_includeFaceIso;

    StrokeSurfaceCurvesInfo (ViewContextR context, MSBsplineSurfaceCR surface, bool includeEdges, bool includeFaceIso) : m_context (context), m_surface (surface), m_includeEdges (includeEdges), m_includeFaceIso (includeFaceIso) {}
    };

#define MAX_CLIPBATCH   200

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
static int wireframe_drawSurfaceCurveCallback (void* userArg, MSBsplineCurveP bcurve, double u0, double u1, double v0, double v1)
    {
    StrokeSurfaceCurvesInfo* info = (StrokeSurfaceCurvesInfo*) userArg;

    // Special case when full boundary isn't displayed...process as strokes...
    if (info->m_includeEdges && !info->m_surface.holeOrigin && (u0 == u1 && u0 == v0 && u0 == v1))
        {
        BsurfBoundary  *boundP = &info->m_surface.boundaries[abs ((int) u0)];
        int            lastEdge, thisEdge, numStrokes;
        DPoint2d       *bP, *endP;
        DPoint3d       strokeBuffer[MAX_CLIPBATCH+1];
        DSegment3d     chord;
 
        numStrokes = 0;
        bP = endP = boundP->points;
        info->m_surface.EvaluatePoint (chord.point[1], bP->x, bP->y);
        thisEdge = bsputil_edgeCode (bP, 0.0);
 
        for (endP += boundP->numPoints, bP++; bP < endP; bP++)
            {
            chord.point[0] = chord.point[1];
            lastEdge = thisEdge;
            info->m_surface.EvaluatePoint (chord.point[1], bP->x, bP->y);
            thisEdge = bsputil_edgeCode (bP, 0.0);
 
            // If both points are on the same edge then do not stroke this segment...
            if (!(lastEdge & thisEdge))
                {
                /* Leave this test in as it supports discontinuity in a B-spline (which arises from the conversion of group holes). */
                if (numStrokes && !LegacyMath::RpntEqual (&chord.point[0], strokeBuffer + numStrokes - 1))
                    {
                    info->m_context.GetIDrawGeom ().DrawLineString3d (numStrokes, strokeBuffer, NULL);
                    numStrokes = 0;
                    }
 
                /* If the buffer is empty ... */
                if (!numStrokes)
                    {
                    strokeBuffer[0] = chord.point[0];
                    numStrokes = 1;
                    }
 
                strokeBuffer[numStrokes] = chord.point[1];
                numStrokes += 1;
 
                if (numStrokes >= MAX_CLIPBATCH-1)
                    {
                    info->m_context.GetIDrawGeom ().DrawLineString3d (numStrokes, strokeBuffer, NULL);
                    strokeBuffer[0] = strokeBuffer[numStrokes - 1];
                    numStrokes = 1;
                    }
                }
            }
 
        info->m_context.GetIDrawGeom ().DrawLineString3d (numStrokes, strokeBuffer, NULL);
 
        return (info->m_context.CheckStop () ? ERROR : SUCCESS);
        }

    bool  showThisRule = true;

    if (!info->m_includeEdges || !info->m_includeFaceIso)
        {
        if ((DoubleOps::AlmostEqual (u0, u1) && (DoubleOps::AlmostEqual (u0, 0.0) || DoubleOps::AlmostEqual (u0, 1.0))) ||
            (DoubleOps::AlmostEqual (v0, v1) && (DoubleOps::AlmostEqual (v0, 0.0) || DoubleOps::AlmostEqual (v0, 1.0))))
            showThisRule = info->m_includeEdges;
        else
            showThisRule = info->m_includeFaceIso;            
        }

    if (showThisRule)
        info->m_context.GetIDrawGeom ().DrawBSplineCurve (*bcurve, false);

    return (info->m_context.CheckStop () ? ERROR : SUCCESS);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool WireframeGeomUtil::CollectRules (DgnExtrusionDetailR detail, bvector<DSegment3d>& rules, bvector<bool>& interior, ViewContextP context)
    {
    bvector<DPoint3d> pts;

    if (wireframe_collectPoints (*detail.m_baseCurve, pts, interior, wireframe_getTolerance (*detail.m_baseCurve), true, context))
        return true;

    for (size_t iPt = 0; iPt < pts.size (); ++iPt)
        {
        DSegment3d  segment = DSegment3d::FromOriginAndDirection (pts[iPt], detail.m_extrusionVector);

        rules.push_back (segment);
        }

    return false;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool WireframeGeomUtil::CollectRules (DgnRotationalSweepDetailR detail, bvector<DEllipse3d>& rules, bvector<bool>& interior, ViewContextP context)
    {
    double ruleTolerance = wireframe_getTolerance (*detail.m_baseCurve);
    bvector<bool> tmpInterior;
    bvector<DPoint3d> pts;

    if (wireframe_collectPoints (*detail.m_baseCurve, pts, tmpInterior, ruleTolerance, true, context))
        return true;

    RotMatrix   axes, invAxes, tmpRMatrix;
    Transform   transform;

    invAxes.InitFrom1Vector (detail.m_axisOfRotation.direction, 2, true);
    axes.TransposeOf (invAxes);

    tmpRMatrix.InitFromPrincipleAxisRotations (axes, 0.0, 0.0, detail.m_sweepAngle);
    tmpRMatrix.InitProduct (invAxes, tmpRMatrix);
    transform.From (tmpRMatrix, detail.m_axisOfRotation.origin);

    for (size_t iRule = 0; iRule < pts.size (); ++iRule)
        {
        DEllipse3d  ellipse;

        if (!wireframe_computeArc (ellipse, pts.at (iRule), detail.m_axisOfRotation.origin, detail.m_sweepAngle, transform, axes, invAxes, ruleTolerance))
            continue;

        rules.push_back (ellipse);
        interior.push_back (tmpInterior.at (iRule));
        }

    if (0 == rules.size ()) // TR#115152 - Problem generating rule arc from arc profile with end point on axis of revolution and small sweep...
        {    
        pts.clear ();
        tmpInterior.clear ();

        if (wireframe_collectArcMidPoints (*detail.m_baseCurve, pts, tmpInterior, ruleTolerance, true, context))
            return true;

        for (size_t iRule = 0; iRule < pts.size (); ++iRule)
            {
            DEllipse3d  ellipse;

            if (!wireframe_computeArc (ellipse, pts.at (iRule), detail.m_axisOfRotation.origin, detail.m_sweepAngle, transform, axes, invAxes, ruleTolerance))
                continue;

            rules.push_back (ellipse);
            interior.push_back (tmpInterior.at (iRule));
            }
        }

    return false;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool WireframeGeomUtil::CollectRules (DgnRuledSweepDetailR detail, bvector<DSegment3d>& rules, bvector<bool>& interior, ViewContextP context)
    {
    for (size_t iProfile = 0; iProfile < detail.m_sectionCurves.size ()-1; ++iProfile)
        {
        bvector<bool> interior1;
        bvector<bool> interior2;
        bvector<DPoint3d> rulePts1;
        bvector<DPoint3d> rulePts2;

        if (wireframe_collectPoints (*detail.m_sectionCurves.at (iProfile), rulePts1, interior1, wireframe_getTolerance (*detail.m_sectionCurves.at (iProfile)), true, context) ||
            wireframe_collectPoints (*detail.m_sectionCurves.at (iProfile+1), rulePts2, interior2, wireframe_getTolerance (*detail.m_sectionCurves.at (iProfile+1)), true, context))
            return true;

        if (rulePts1.size () != rulePts2.size ())
            {
            if (1 == rulePts2.size ())
                {
                // Special case to handle zero scale in both XY...
                rulePts2.insert (rulePts2.end (), rulePts1.size ()-1, rulePts2.front ());
                interior2.insert (interior2.end (), interior1.size ()-1, interior2.front ());
                }
            else
                {
                rulePts1.clear (); rulePts2.clear (); interior1.clear (); interior2.clear ();

                // In case of zero scale in only X or Y...we have no choice but to re-collect without excluding any points...
                if (wireframe_collectPoints (*detail.m_sectionCurves.at (iProfile), rulePts1, interior1, 0.0, false, context) ||
                    wireframe_collectPoints (*detail.m_sectionCurves.at (iProfile+1), rulePts2, interior2, 0.0, false, context))
                    return true;

                if (rulePts1.size () != rulePts2.size ())
                    return true;
                }
            }

        for (size_t iRule = 0; iRule < rulePts1.size (); ++iRule)
            {
            DSegment3d  segment;

            segment.Init (rulePts1.at (iRule), rulePts2.at (iRule));
            rules.push_back (segment);
            interior.push_back (interior1.at (iRule));
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/13
+---------------+---------------+---------------+---------------+---------------+------*/
static void clearCurveVectorIds (CurveVectorCR curveVector)
    {
    for (ICurvePrimitivePtr curve: curveVector)
        {
        curve->SetId (NULL);

        if (curve->GetChildCurveVectorP ().IsValid ())
            clearCurveVectorIds (*curve->GetChildCurveVectorP ());
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Ray.Bentley     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void drawSolidPrimitiveCurveVector (CurveVectorCR curveVector, ViewContextR context, TransformCP transform, CurvePrimitiveId::Type type, CurveTopologyIdCR id, CompoundDrawStateP cds)
    {
    bool    ignoreCurveIds = context.GetIViewDraw ().IsOutputQuickVision () || context.CheckICachedDraw (); // Don't need when called from QvOutput/QvCachedOutput...

    if (!ignoreCurveIds) 
        {
        // Don't need ids when called from QvOutput...
        clearCurveVectorIds (curveVector);
        CurveTopologyId::AddCurveVectorIds (curveVector, type, id, cds);
        }

    if (transform)
        context.PushTransform (*transform);

    // Always output as open profile...
    WireframeGeomUtil::DrawOutline (curveVector, context.GetIDrawGeom ());

    if (!ignoreCurveIds)
        {
        // Best not to leave our curve ids on the curve primitives...
        clearCurveVectorIds (curveVector);
        }

    if (transform)
        context.PopTransformClip ();
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Ray.Bentley     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void drawSolidPrimitiveCurve (ICurvePrimitivePtr primitive, ViewContextR context, CurveTopologyIdCR topologyId, CompoundDrawStateP cds)
    {
    bool    ignoreCurveIds = context.GetIViewDraw ().IsOutputQuickVision () || context.CheckICachedDraw (); // Don't need when called from QvOutput/QvCachedOutput...

    if (!ignoreCurveIds)
        {
        // Don't need ids when called from QvOutput...
        CurvePrimitiveIdPtr newId = CurvePrimitiveId::Create (CurvePrimitiveId::Type_SolidPrimitive, topologyId, cds);

        primitive->SetId (newId.get());
        }

    context.GetIDrawGeom ().DrawCurveVector (*CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open, primitive), false);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void WireframeGeomUtil::Draw (ISolidPrimitiveCR primitive, ViewContextR context, bool includeEdges, bool includeFaceIso)
    {
    CompoundDrawStatePtr    compoundDrawState = context.GetCompoundDrawState ();

    switch (primitive.GetSolidPrimitiveType ())
        {
        case SolidPrimitiveType_DgnTorusPipe:
            {
            DgnTorusPipeDetail  detail;

            if (!primitive.TryGetDgnTorusPipeDetail (detail))
                return;

            int     maxURules = 4;
            bool    showCap0 = (Angle::IsFullCircle (detail.m_sweepAngle) ? includeFaceIso : includeEdges);
            bool    showCap1 = (Angle::IsFullCircle (detail.m_sweepAngle) ? false : includeEdges);

            if (showCap0)
                drawSolidPrimitiveCurve (ICurvePrimitive::CreateArc (detail.VFractionToUSectionDEllipse3d (0.0)), context, CurveTopologyId::FromSweepProfile (0), compoundDrawState.get ());

            if (showCap1)
                drawSolidPrimitiveCurve (ICurvePrimitive::CreateArc (detail.VFractionToUSectionDEllipse3d (1.0)), context, CurveTopologyId::FromSweepProfile (1), compoundDrawState.get ());

            if (!includeFaceIso)
                return;

            size_t  numVRules = DgnRotationalSweepDetail::ComputeVRuleCount (detail.m_sweepAngle);

            for (size_t vRule = 1; vRule < numVRules; ++vRule)
                {
                double  vFraction = (1.0 / numVRules) * vRule;

                drawSolidPrimitiveCurve (ICurvePrimitive::CreateArc (detail.VFractionToUSectionDEllipse3d (vFraction)), context, CurveTopologyId::FromSweepProfile (vRule + 1), compoundDrawState.get ());
                }

            for (int uRule = 0; uRule < maxURules; ++uRule)
                {
                double  uFraction = (1.0 / maxURules) * uRule;

                drawSolidPrimitiveCurve (ICurvePrimitive::CreateArc (detail.UFractionToVSectionDEllipse3d (uFraction)), context, CurveTopologyId::FromSweepLateral (uRule), compoundDrawState.get ());
                }
            return;
            }

        case SolidPrimitiveType_DgnCone:
            {
            DgnConeDetail  detail;

            if (!primitive.TryGetDgnConeDetail (detail))
                return;

            if (detail.m_radiusA > 0.0 && includeEdges)
                {
                DEllipse3d  ellipse;

                ellipse.InitFromDGNFields3d (detail.m_centerA, detail.m_vector0, detail.m_vector90, detail.m_radiusA, detail.m_radiusA, 0.0, msGeomConst_2pi);
                drawSolidPrimitiveCurve (ICurvePrimitive::CreateArc (ellipse), context, CurveTopologyId::FromSweepProfile (0), compoundDrawState.get ());
                }

            if (detail.m_radiusB > 0.0 && includeEdges)
                {
                DEllipse3d  ellipse;
    
                ellipse.InitFromDGNFields3d (detail.m_centerB, detail.m_vector0, detail.m_vector90, detail.m_radiusB, detail.m_radiusB, 0.0, msGeomConst_2pi);
                drawSolidPrimitiveCurve (ICurvePrimitive::CreateArc (ellipse), context, CurveTopologyId::FromSweepProfile (1), compoundDrawState.get ());
                }

            bool    ignoreSilhouettes = context.GetIViewDraw ().IsOutputQuickVision () || context.CheckICachedDraw (); // Don't need when called from QvOutput/QvCachedOutput...

            if (!includeFaceIso || ignoreSilhouettes)
                return; // QVis handles cone silhouette display...

            DSegment3d  silhouettes[2];

            if (!detail.GetSilhouettes (silhouettes[0], silhouettes[1], context.GetViewToLocal ()))
                return; // NOTE: This is expected to fail for TopologyCurveGenerator::CurveByIdCollector, don't allow associations to silhouettes!!!

            drawSolidPrimitiveCurve (ICurvePrimitive::CreateLine (silhouettes[0]), context, CurveTopologyId::FromSweepSilhouette (0), compoundDrawState.get ());
            drawSolidPrimitiveCurve (ICurvePrimitive::CreateLine (silhouettes[1]), context, CurveTopologyId::FromSweepSilhouette (1), compoundDrawState.get ());
            return;
            }

        case SolidPrimitiveType_DgnBox:
            {
            if (!includeEdges)
                return; // No face iso to display...

            DgnBoxDetail  detail;

            if (!primitive.TryGetDgnBoxDetail (detail))
                return;

            bvector<DPoint3d> corners;

            detail.GetCorners (corners);

            DPoint3d  baseRectangle[5];

            baseRectangle[0] = corners[0];
            baseRectangle[1] = corners[1];
            baseRectangle[2] = corners[3];
            baseRectangle[3] = corners[2];
            baseRectangle[4] = corners[0];

            DPoint3d  topRectangle[5];

            topRectangle[0] = corners[4];
            topRectangle[1] = corners[5];
            topRectangle[2] = corners[7];
            topRectangle[3] = corners[6];
            topRectangle[4] = corners[4];

            drawSolidPrimitiveCurve (ICurvePrimitive::CreateLineString (baseRectangle, 5), context, CurveTopologyId::FromSweepProfile (0), compoundDrawState.get ());
            drawSolidPrimitiveCurve (ICurvePrimitive::CreateLineString (topRectangle,  5), context, CurveTopologyId::FromSweepProfile (1), compoundDrawState.get ());

            for (UInt32 iRule = 0; iRule < 4; ++iRule)
                drawSolidPrimitiveCurve (ICurvePrimitive::CreateLine (DSegment3d::From (baseRectangle[iRule], topRectangle[iRule])), context, CurveTopologyId::FromSweepLateral (iRule), compoundDrawState.get ());
            return;
            }

        case SolidPrimitiveType_DgnSphere:
            {
            DgnSphereDetail  detail;

            if (!primitive.TryGetDgnSphereDetail (detail))
                return;

            if (includeFaceIso)
                {
                int     maxURules = 4;
            
                for (int uRule = 0; uRule < maxURules; ++uRule)
                    {
                    double  uFraction = (1.0 / maxURules) * uRule;

                    drawSolidPrimitiveCurve (ICurvePrimitive::CreateArc (detail.UFractionToVSectionDEllipse3d (uFraction)), context, CurveTopologyId::FromSweepLateral (uRule), compoundDrawState.get ());
                    }
                }

            if (!includeEdges)
                return;

            // Draw merdian if latitude sweep > 90 and includes 0.0 sweep latitude...
            if (fabs (detail.m_latitudeSweep) < Angle::PiOver2 ())
                return;

            double  vFraction = detail.LatitudeToVFraction (0.0);

            if (vFraction <= 0.0 || vFraction >= 1.0)
                return;

            drawSolidPrimitiveCurve (ICurvePrimitive::CreateArc (detail.VFractionToUSectionDEllipse3d (vFraction)), context, CurveTopologyId::FromSweepProfile (0), compoundDrawState.get ());
            return;
            }

        case SolidPrimitiveType_DgnExtrusion:
            {
            DgnExtrusionDetail  detail;

            if (!primitive.TryGetDgnExtrusionDetail (detail))
                return;

            if (includeEdges)
                {
                drawSolidPrimitiveCurveVector (*detail.m_baseCurve, context, NULL, CurvePrimitiveId::Type_SolidPrimitive, CurveTopologyId::FromSweepProfile (0), compoundDrawState.get ());

                if (context.CheckStop ())
                    return;

                Transform  transform = Transform::From (detail.m_extrusionVector);

                drawSolidPrimitiveCurveVector (*detail.m_baseCurve, context, &transform, CurvePrimitiveId::Type_SolidPrimitive, CurveTopologyId::FromSweepProfile (1), compoundDrawState.get ());

                if (context.CheckStop ())
                    return;
                }

            bvector<bool> interior;
            bvector<DSegment3d> rules;

            if (CollectRules (detail, rules, interior, &context))
                return;

            for (UInt32 iRule = 0; iRule < rules.size (); ++iRule)
                {
                if (!(interior.at (iRule) ? includeFaceIso : includeEdges))
                    continue;

                drawSolidPrimitiveCurve (ICurvePrimitive::CreateLine (rules.at (iRule)), context, CurveTopologyId::FromSweepLateral (iRule), compoundDrawState.get ());
                }
            return;
            }

        case SolidPrimitiveType_DgnRotationalSweep:
            {
            DgnRotationalSweepDetail  detail;

            if (!primitive.TryGetDgnRotationalSweepDetail (detail))
                return;

            bool    showCap0 = (Angle::IsFullCircle (detail.m_sweepAngle) ? includeFaceIso : includeEdges);
            bool    showCap1 = (Angle::IsFullCircle (detail.m_sweepAngle) ? false : includeEdges);

            if (showCap0)
                {
                drawSolidPrimitiveCurveVector (*detail.m_baseCurve, context, NULL, CurvePrimitiveId::Type_SolidPrimitive, CurveTopologyId::FromSweepProfile (0), compoundDrawState.get ());

                if (context.CheckStop ())
                    return;
                }

            if (showCap1)
                {
                DPoint3d    axisPoint;
                Transform   transform;

                axisPoint.SumOf (detail.m_axisOfRotation.origin, detail.m_axisOfRotation.direction);
                transform.InitFromLineAndRotationAngle (detail.m_axisOfRotation.origin, axisPoint, detail.m_sweepAngle);

                drawSolidPrimitiveCurveVector (*detail.m_baseCurve, context, &transform, CurvePrimitiveId::Type_SolidPrimitive, CurveTopologyId::FromSweepProfile (1), compoundDrawState.get ());

                if (context.CheckStop ())
                    return;
                }

            if (includeFaceIso)
                {
                // Draw V rules (if any needed) in addition to end caps...
                size_t  numVRules = detail.GetVRuleCount ();

                for (size_t vRule = 1; vRule < numVRules; ++vRule)
                    {
                    double      vFraction = (1.0 / numVRules) * vRule;
                    Transform   transform, derivativeTransform;

                    if (!detail.GetVFractionTransform (vFraction, transform, derivativeTransform))
                        continue;

                    drawSolidPrimitiveCurveVector (*detail.m_baseCurve, context, &transform, CurvePrimitiveId::Type_SolidPrimitive, CurveTopologyId::FromSweepProfile (vRule + 1), compoundDrawState.get ());

                    if (context.CheckStop ())
                        return;
                    }
                }

            // Draw U rule arcs based on profile geometry...
            bvector<bool> interior;
            bvector<DEllipse3d> rules;

            if (CollectRules (detail, rules, interior, &context))
                return;

            for (UInt32 uRule = 0; uRule < rules.size (); ++uRule)
                {
                if (!(interior.at (uRule) ? includeFaceIso : includeEdges))
                    continue;

                drawSolidPrimitiveCurve (ICurvePrimitive::CreateArc (rules.at (uRule)), context, CurveTopologyId::FromSweepLateral (uRule), compoundDrawState.get ());
                }
            return;
            }

        case SolidPrimitiveType_DgnRuledSweep:
            {
            DgnRuledSweepDetail detail;
    
            if (!primitive.TryGetDgnRuledSweepDetail (detail))
                return;

            if (includeEdges)
                {
                UInt32  curveIndex = 0;

                for (CurveVectorPtr curves: detail.m_sectionCurves)
                    {
                    drawSolidPrimitiveCurveVector (*curves, context, NULL, CurvePrimitiveId::Type_SolidPrimitive, CurveTopologyId::FromSweepProfile (curveIndex++), compoundDrawState.get ());

                    if (context.CheckStop ())
                        return;
                    }
                }

            bvector<bool> interior;
            bvector<DSegment3d> rules;

            if (CollectRules (detail, rules, interior, &context))
                return;

            for (size_t uRule = 0; uRule < rules.size (); ++uRule)
                {
                if (!(interior.at (uRule) ? includeFaceIso : includeEdges))
                    continue;

                drawSolidPrimitiveCurve (ICurvePrimitive::CreateLine (rules.at (uRule)), context, CurveTopologyId::FromSweepLateral (uRule), compoundDrawState.get ());
                }
            return;
            }

        default:
            return;
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void WireframeGeomUtil::Draw (MSBsplineSurfaceCR surface, ViewContextR context, bool includeEdges, bool includeFaceIso)
    {
    if (includeEdges)
        {
        CurveVectorPtr  curves = surface.GetUnstructuredBoundaryCurves (0.0, true, true);

        if (curves.IsValid ())
            {
            // NOTE: This should be BOUNDARY_TYPE_None with bcurve primitives. Output each curve separately so callers don't have to deal with nesting...
            for (ICurvePrimitivePtr curve : *curves)
                {
                if (!curve.IsValid ())
                    continue;

                context.GetIDrawGeom ().DrawCurveVector (*CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open, curve), false);
                }
            }
        }

    if (includeFaceIso)
        {
        StrokeSurfaceCurvesInfo info (context, surface, false, true);

        bspproc_surfaceWireframeByCurves (&surface, wireframe_drawSurfaceCurveCallback, &info, false);
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void WireframeGeomUtil::Draw (ISolidKernelEntityCR entity, ViewContextR context, IFaceMaterialAttachmentsCP attachments, bool includeEdges, bool includeFaceIso)
    {
    T_HOST.GetSolidsKernelAdmin()._OutputBodyAsWireframe (entity, context, includeEdges, includeFaceIso, attachments);
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RuleCollector : IElementGraphicsProcessor
{
protected:

Transform               m_currentTransform;

MSBsplineSurfaceCP      m_surface;
ISolidPrimitiveCP       m_primitive;
ISolidKernelEntityCP    m_entity;

bool                    m_includeEdges;
bool                    m_includeFaceIso;

CurveVectorPtr          m_curves;

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
explicit RuleCollector (bool includeEdges, bool includeFaceIso)
    {
    m_surface   = NULL;
    m_primitive = NULL;
    m_entity    = NULL;

    m_includeEdges   = includeEdges;
    m_includeFaceIso = includeFaceIso;
    }

virtual ~RuleCollector () {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _WantClipping () const override {return false;}
virtual bool _ProcessAsBody (bool isCurved) const override {return false;}
virtual bool _ProcessAsFacets (bool isPolyface) const override {return false;}
virtual void _AnnounceTransform (TransformCP trans) override {if (trans) m_currentTransform = *trans; else m_currentTransform.InitIdentity ();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _ProcessCurveVector (CurveVectorCR curves, bool isFilled) override
    {
    if (m_curves.IsNull ())
        m_curves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);

    CurveVectorPtr  childCurve = curves.Clone ();

    if (!m_currentTransform.IsIdentity ())
        childCurve->TransformInPlace (m_currentTransform);

    m_curves->Add (childCurve);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _OutputGraphics (ViewContextR context) override
    {
    // NOTE: Can't set DgnProject because we don't have one to use...that's ok, WireframeGeomUtil methods don't need it...
    if (m_surface)
        WireframeGeomUtil::Draw (*m_surface, context, m_includeEdges, m_includeFaceIso);
    else if (m_primitive)
        WireframeGeomUtil::Draw (*m_primitive, context, m_includeEdges, m_includeFaceIso);
    else if (m_entity)
        WireframeGeomUtil::Draw (*m_entity, context, NULL, m_includeEdges, m_includeFaceIso);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void SetBsplineSurface (MSBsplineSurfaceCR surface) {m_surface = &surface;}
void SetSolidPrimitive (ISolidPrimitiveCR primitive) {m_primitive = &primitive;}
void SetSolidEntity (ISolidKernelEntityCR entity) {m_entity = &entity;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr GetCurveVector () {return m_curves;}

}; // RuleCollector

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  03/14
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr WireframeGeomUtil::CollectCurves (ISolidPrimitiveCR primitive, bool includeEdges, bool includeFaceIso)
    {
    RuleCollector   rules (includeEdges, includeFaceIso);

    rules.SetSolidPrimitive (primitive);
    ElementGraphicsOutput::Process (rules);

    return rules.GetCurveVector ();
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  03/14
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr WireframeGeomUtil::CollectCurves (MSBsplineSurfaceCR surface, bool includeEdges, bool includeFaceIso)
    {
    RuleCollector   rules (includeEdges, includeFaceIso);

    rules.SetBsplineSurface (surface);
    ElementGraphicsOutput::Process (rules);

    return rules.GetCurveVector ();
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  03/14
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr WireframeGeomUtil::CollectCurves (ISolidKernelEntityCR entity, bool includeEdges, bool includeFaceIso)
    {
    RuleCollector   rules (includeEdges, includeFaceIso);

    rules.SetSolidEntity (entity);
    ElementGraphicsOutput::Process (rules);

    return rules.GetCurveVector ();
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void WireframeGeomUtil::DrawOutline (CurveVectorCR curves, IDrawGeomR drawGeom)
    {
    if (1 > curves.size ())
        return;

    if (curves.IsUnionRegion () || curves.IsParityRegion ())
        {
        for (ICurvePrimitivePtr curve: curves)
            {
            if (curve.IsNull () || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType ())
                continue;

            DrawOutline (*curve->GetChildCurveVectorCP (), drawGeom);
            }
        }
    else if (curves.IsClosedPath ())
        {
        CurveVector::BoundaryType  saveType = curves.GetBoundaryType ();

        const_cast <CurveVectorR> (curves).SetBoundaryType (CurveVector::BOUNDARY_TYPE_Open);
        drawGeom.DrawCurveVector (curves, false);
        const_cast <CurveVectorR> (curves).SetBoundaryType (saveType);
        }
    else
        {
        // Open and none path types ok...
        drawGeom.DrawCurveVector (curves, false);
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void WireframeGeomUtil::DrawOutline2d (CurveVectorCR curves, IDrawGeomR drawGeom, double zDepth)
    {
    if (1 > curves.size ())
        return;

    if (curves.IsUnionRegion () || curves.IsParityRegion ())
        {
        for (ICurvePrimitivePtr curve: curves)
            {
            if (curve.IsNull () || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType ())
                continue;

            DrawOutline2d (*curve->GetChildCurveVectorCP (), drawGeom, zDepth);
            }
        }
    else if (curves.IsClosedPath ())
        {
        CurveVector::BoundaryType  saveType = curves.GetBoundaryType ();

        const_cast <CurveVectorR> (curves).SetBoundaryType (CurveVector::BOUNDARY_TYPE_Open);
        drawGeom.DrawCurveVector2d (curves, false, zDepth);
        const_cast <CurveVectorR> (curves).SetBoundaryType (saveType);
        }
    else
        {
        // Open and none path types ok...
        drawGeom.DrawCurveVector2d (curves, false, zDepth);
        }
    }

// =======================================================================================
// @bsiclass                                                      Brien.Bastings  01/14
// =======================================================================================
struct TempCurveProvider : ICurvePathQuery
{
protected:

CurveVectorPtr  m_curves;

virtual BentleyStatus _GetCurveVector (ElementHandleCR eh, CurveVectorPtr& curves) {curves = m_curves; return SUCCESS;}

public:

TempCurveProvider (CurveVectorCR curves) {m_curves = curves.Clone ();}

}; // TempCurveProvider

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void WireframeGeomUtil::DrawStyledCurveVector (CurveVectorCR curves, ViewContextR context, bool is3d, GeomRepresentations info)
    {
    if (1 > curves.size ())
        return;
    
    // Ugh...we need to make a temporary element (an extended element avoids having DirectDrawSimpleLegacyTypes ignore our curve provider)...
    TempCurveProvider   provider (curves);   
    EditElementHandle   tmpEeh;

    ExtendedElementHandler::InitializeElement (tmpEeh, NULL, *context.GetDgnProject ().Models ().GetModelById (context.GetDgnProject ().Models ().GetFirstModelId ()), is3d); 
    context.DrawCurveVector (tmpEeh, provider, info, false);
    }
