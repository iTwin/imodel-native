/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/HitPath.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
SelectionPath::SelectionPath (SelectionPath const* from)
    {
    m_viewport = from->m_viewport;
    SetPath (from);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
int SelectionPath::GetViewNumber() const
    {
    return  m_viewport ? m_viewport->GetViewNumber() : -1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
void GeomDetail::Init ()
    {
    m_primitive     = NULL;
    m_localToWorld.InitIdentity ();
    m_localPoint.Zero ();
    m_geomType      = HitGeomType::None;
    m_detailType    = 0;
    m_hitPriority   = HitPriority::Highest;
    m_patternIndex  = 0;
    m_elemArg       = 0;
    m_nonSnappable  = false;
    m_viewDist      = 0.0;
    m_viewZ         = 0.0;
    m_localNormal.Zero ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
ICurvePrimitiveCP GeomDetail::GetCurvePrimitive () const
    {
    return m_primitive.IsValid () ? m_primitive.get () : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/13
+---------------+---------------+---------------+---------------+---------------+------*/
HitGeomType GeomDetail::GetCurvePrimitiveType () const
    {
    if (!m_primitive.IsValid ())
        return HitGeomType::None;

    switch (m_primitive->GetCurvePrimitiveType ())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            return HitGeomType::Segment;

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            return HitGeomType::Arc;

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
            return HitGeomType::Curve;

        default:
            return HitGeomType::None;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/13
+---------------+---------------+---------------+---------------+---------------+------*/
HitGeomType GeomDetail::GetEffectiveHitGeomType() const
    {
    return (HitGeomType::Surface == m_geomType ? GetCurvePrimitiveType () : m_geomType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void GeomDetail::SetCurvePrimitive (ICurvePrimitiveCP curve, HitGeomType geomType)
    {
    m_primitive = NULL; 
    m_geomType  = HitGeomType::None;

    if (!curve)
        return;

    switch (curve->GetCurvePrimitiveType ())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            {
            DSegment3dCP  segment = curve->GetLineCP ();

            if (segment->point[0].IsEqual (segment->point[1])) // Check for zero length lines and don't store redundant primitive...
                {
                m_geomType = HitGeomType::Point;
                break;
                }

            m_primitive = curve->Clone ();
            m_geomType  = HitGeomType::Segment;
            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            {
            bvector<DPoint3d> const* points = curve->GetLineStringCP ();

            if ((1 == points->size ()) || (2 == points->size () && points->at (0).IsEqual (points->at (1))))
                {
                m_geomType = HitGeomType::Point;
                break;
                }

            m_primitive = curve->Clone ();
            m_geomType  = HitGeomType::Segment;
            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            {
            m_primitive = curve->Clone ();
            m_geomType  = HitGeomType::Arc;
            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
            {
            m_primitive = curve->Clone ();
            m_geomType  = HitGeomType::Curve;
            break;
            }
        }

    // Set geometry type override...
    //  - HitGeomType::Point with CURVE_PRIMITIVE_TYPE_Arc denotes arc center...
    //  - HitGeomType::Surface with any curve primitive denotes an interior hit...
    if (HitGeomType::None != geomType)
        m_geomType = geomType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void GeomDetail::GetClosestPoint (DPoint3dR pt) const
    {
    m_localToWorld.MultiplyAndRenormalize (&pt, &m_localPoint, 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void GeomDetail::SetClosestPoint (DPoint3dCR pt)
    {
    DMatrix4d   worldToLocal;
    
    worldToLocal.TransposeOf (m_localToWorld);
    worldToLocal.MultiplyAndRenormalize (&m_localPoint, &pt, 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeomDetail::GetSegment (DSegment3dR segment) const
    {
    if (!m_primitive.IsValid ())
        return false;

    switch (m_primitive->GetCurvePrimitiveType ())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            segment = *m_primitive->GetLineCP ();
            return true;

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            return m_primitive->TryGetSegmentInLineString (segment, GetSegmentNumber ());

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeomDetail::GetArc (DEllipse3dR ellipse) const
    {
    if (!m_primitive.IsValid ())
        return false;

    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc != m_primitive->GetCurvePrimitiveType ())
        return false;

    ellipse = *m_primitive->GetArcCP ();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeomDetail::FillGPA (GPArrayR gpa, bool worldCoords, bool singleSegment) const
    {
    if (!m_primitive.IsValid ())
        return false;

    DSegment3d  segment;

    if (singleSegment && GetSegment (segment))
        {
        gpa.Add (segment.point, 2);
        }
    else
        {
        switch (m_primitive->GetCurvePrimitiveType ())
            {
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                gpa.Add (m_primitive->GetLineCP ()->point, 2);
                break;

            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                gpa.Add (&m_primitive->GetLineStringCP ()->front (), (int) m_primitive->GetLineStringCP ()->size ());
                break;

            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                gpa.Add (*m_primitive->GetArcCP ());
                break;

            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
                gpa.Add (*m_primitive->GetProxyBsplineCurveCP ());
                break;

            default:
                return false;
            }
        }

    if (worldCoords)
        gpa.Transform (&m_localToWorld);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
size_t GeomDetail::GetSegmentNumber () const
    {
    if (!m_primitive.IsValid () || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString != m_primitive->GetCurvePrimitiveType ())
        return 0;

    bvector<DPoint3d> const* points = m_primitive->GetLineStringCP ();

    if (points->size () < 3)
        return 0;
        
    double  fraction  = GetCloseParam ();
    size_t  nSegments = (points->size ()-1);
    double  uSegRange = (1.0 / nSegments);
    size_t  segmentNo = (size_t) (fraction / uSegRange);

    return (segmentNo >= nSegments ? nSegments-1 : segmentNo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
double GeomDetail::GetSegmentParam () const
    {
    if (!m_primitive.IsValid ())
        return 0.0;
    
    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString != m_primitive->GetCurvePrimitiveType ())
        return GetCloseParam ();

    bvector<DPoint3d> const* points = m_primitive->GetLineStringCP ();

    if (points->size () < 3)
        return GetCloseParam ();
        
    double  fraction  = GetCloseParam ();
    size_t  nSegments = (points->size ()-1);
    double  uSegRange = (1.0 / nSegments);
    size_t  segmentNo = (size_t) (fraction / uSegRange);
    double  segmParam = ((fraction - (uSegRange * (segmentNo >= nSegments ? nSegments-1 : segmentNo))) * nSegments);

    LIMIT_RANGE (0.0, 1.0, segmParam);

    return segmParam;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
double GeomDetail::GetCloseParam () const
    {
    if (!m_primitive.IsValid ())
        return 0.0;

    double      fraction;
    DPoint3d    curvePoint;

    if (!m_primitive->ClosestPointBounded (m_localPoint, fraction, curvePoint))
        return 0.0;

    return fraction;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
size_t GeomDetail::GetCloseVertex () const
    {
    if (!m_primitive.IsValid ())
        return 0;

    size_t  nSegments = 1;

    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString == m_primitive->GetCurvePrimitiveType ())
        {
        bvector<DPoint3d> const* points = m_primitive->GetLineStringCP ();

        nSegments = ((points->size () < 3) ? 1 : (points->size ()-1));
        }

    double  fraction  = GetCloseParam ();
    double  uSegRange = (1.0 / nSegments);
    size_t  closeVertex = (size_t) ((fraction + (uSegRange*.5)) / uSegRange);

    return closeVertex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
size_t GeomDetail::GetPointCount () const
    {
    if (HitGeomType::Point == m_geomType)
        return 1;

    if (!m_primitive.IsValid ())
        return 0;

    switch (m_primitive->GetCurvePrimitiveType ())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            return 2;

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            return m_primitive->GetLineStringCP ()->size ();

        default:
            return 0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeomDetail::IsValidSurfaceHit () const
    {
    return (HitGeomType::Surface == GetGeomType () && 0.0 != GetSurfaceNormal ().Magnitude ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeomDetail::IsValidEdgeHit () const
    {
    switch (GetGeomType ())
        {
        case HitGeomType::Segment:
        case HitGeomType::Curve:
        case HitGeomType::Arc:
            return true;

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
CurvePrimitiveIdCP GeomDetail::GetCurvePrimitiveId() const
    {
    if (!m_primitive.IsValid ())
        return NULL;

    return m_primitive->GetId ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
ICurvePrimitiveInfoPtr MeshSegmentInfo::Create (uint32_t closeVertex, uint32_t segmentVertex)
    {
    return new MeshSegmentInfo (closeVertex, segmentVertex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt HitPath::GetHitLocalToContextLocal (TransformR hitLocalToContextLocalTrans, ViewContextR context) const
    {
    DMatrix4d   contextLocalToWorld;

    hitLocalToContextLocalTrans.InitIdentity ();

    // NOTE: GeomDetail::LocalToWorld may include transforms pushed by _Draw method...
    if (SUCCESS != context.GetCurrLocalToWorldTrans (contextLocalToWorld))
        return ERROR;

    DMatrix4d   worldToContextLocal, hitLocalToContextLocal;

    worldToContextLocal.QrInverseOf (contextLocalToWorld);
    hitLocalToContextLocal.InitProduct (worldToContextLocal, m_geomDetail.GetLocalToWorld ());

    return (hitLocalToContextLocalTrans.InitFrom (hitLocalToContextLocal) ? SUCCESS : ERROR); // hitLocalToContextLocal should never have perspective...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt HitPath::GetContextLocalToHitLocal (TransformR contextLocalToHitLocalTrans, ViewContextR context) const
    {
    Transform   hitLocalToContextLocalTrans;
    StatusInt   status = GetHitLocalToContextLocal (hitLocalToContextLocalTrans, context);

    contextLocalToHitLocalTrans.InverseOf (hitLocalToContextLocalTrans);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/015
+---------------+---------------+---------------+---------------+---------------+------*/
HitPath::HitPath
(
DgnViewportP               viewport,
DisplayPathCR           path,
DPoint3dCR              testPoint,
HitSource               source,
ViewFlagsCR             viewFlags,
GeomDetailCR            geomDetail,
IElemTopologyCP         elemTopo
//IViewHandlerHitInfoCP   viewHandlerHitInfo removed in graphite
) : SelectionPath (viewport)
    {
    SetPath (&path);

    m_locateSource       = source;
    m_testPoint          = testPoint;
    m_viewFlags          = viewFlags;
    m_geomDetail         = geomDetail;
    m_elemTopo           = elemTopo;
    //m_viewHandlerHitInfo = viewHandlerHitInfo; removed in graphite
    m_componentMode      = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/04
+---------------+---------------+---------------+---------------+---------------+------*/
HitPath::HitPath (HitPath const& from) : SelectionPath (&from)
    {
    m_locateSource       = from.m_locateSource;
    m_testPoint          = from.m_testPoint;
    m_viewFlags          = from.m_viewFlags;
    m_geomDetail         = from.m_geomDetail;
    m_elemTopo           = (NULL == from.m_elemTopo) ? NULL : from.m_elemTopo->_Clone ();
    //m_viewHandlerHitInfo = (NULL == from.m_viewHandlerHitInfo) ? NULL : from.m_viewHandlerHitInfo->_Clone (); removed in graphite
    m_componentMode      = from.m_componentMode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/04
+---------------+---------------+---------------+---------------+---------------+------*/
HitPath::~HitPath ()
    {
    ClearElemTopology ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
void HitPath::_GetInfoString (Utf8StringR pathDescr, Utf8CP delimiter) const
    {
    T_Super::_GetInfoString (pathDescr, delimiter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/04
+---------------+---------------+---------------+---------------+---------------+------*/
void            HitPath::ClearElemTopology()
    {
    DELETE_AND_CLEAR (m_elemTopo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool            HitPath::IsSamePath (DisplayPathCP otherPath, bool fullPath) const
    {
    // check base paths
    if (!SelectionPath::IsSamePath (otherPath, fullPath))
        return false;

    // if other path is also hit path allow for elem topo compare...otherwise paths are the same
    if (DisplayPathType::Hit != otherPath->GetPathType())
        return true;

    HitPathP    otherHitPath = (HitPath*) otherPath;

    IElemTopology const *thisTopo, *otherTopo;

    if (NULL == (thisTopo = GetElemTopology()) ||
        NULL == (otherTopo = otherHitPath->GetElemTopology()))
        return true;

    if (0 !=  thisTopo->_Compare (*otherTopo))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
SnapPath::SnapPath (HitPath const* from) : HitPath (*from)
    {
    m_heat   = SNAP_HEAT_None;
    m_sprite = NULL;

    m_customKeypointSize = 0;
    m_customKeypointData = NULL;
    m_allowAssociations  = true;

    m_screenPt.x = m_screenPt.y = 0;
    m_geomDetail.GetClosestPoint (m_snapPoint);
    m_adjustedPt = m_snapPoint;
    m_snapMode = m_originalSnapMode = SnapMode::First;

    if (DisplayPathType::Snap == from->GetPathType())
        {
        SnapPathCP  fromSnap = (SnapPathCP) from;

        m_minScreenDist = fromSnap->m_minScreenDist;
        }
    else
        {
        m_minScreenDist = m_geomDetail.GetScreenDist ();
        m_geomDetail.SetScreenDist (0.0);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
SnapPath::SnapPath (SnapPath const& from) : HitPath (from)
    {
    m_heat              = from.m_heat;
    m_screenPt          = from.m_screenPt;
    m_originalSnapMode  = from.m_originalSnapMode;
    m_divisor           = from.m_divisor;
    m_sprite            = from.m_sprite;
    m_snapMode          = from.m_snapMode;
    m_minScreenDist     = from.m_minScreenDist;
    m_snapPoint         = from.m_snapPoint;
    m_adjustedPt        = from.m_adjustedPt;
    m_allowAssociations = from.m_allowAssociations;

    if (m_sprite)
        m_sprite->AddRef ();

    if (NULL == from.m_customKeypointData || 0 == from.m_customKeypointSize)
        {
        m_customKeypointSize = 0;
        m_customKeypointData = NULL;
        }
    else
        {
        m_customKeypointSize = from.m_customKeypointSize;
        m_customKeypointData = (Byte *) bentleyAllocator_malloc(m_customKeypointSize);

        memcpy (m_customKeypointData, from.m_customKeypointData, m_customKeypointSize);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
SnapPath::~SnapPath ()
    {
    if (m_sprite)
        m_sprite->Release ();

    if (NULL != m_customKeypointData)
        bentleyAllocator_free(m_customKeypointData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ShaunSewall     12/11
+---------------+---------------+---------------+---------------+---------------+------*/
SnapPath* SnapPath::Clone () const 
    {
    return new SnapPath (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @return  the "hit" point for this snapPath. If this path is "hot", this function will return
* the snapped point, otherwise it returns the closest point on the element.
* @bsimethod    HitPath                                         KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
void            SnapPath::GetHitPoint (DPoint3dR pt) const
    {
    if (IsHot ())
        pt = m_snapPoint;
    else
        m_geomDetail.GetClosestPoint (pt);
    }

/*---------------------------------------------------------------------------------**//**
* @return whether point has been adjusted
* @bsimethod    SnapPath                                        BrienBastings   08/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool         SnapPath::PointWasAdjusted () const
    {
    return (!bsiDPoint3d_pointEqualTolerance (&m_snapPoint, &m_adjustedPt, 1.0e-10));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            SnapPath::SetHitPoint (DPoint3dCR hitPoint)
    {
    m_snapPoint  = hitPoint;
    m_adjustedPt = hitPoint;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
IntersectPath::IntersectPath (HitPathCP firstPath, HitPathCP secondPath, DPoint3dCR pt) : SnapPath (firstPath)
    {
    m_secondPath = (HitPath *) secondPath;

    if (m_secondPath)
        m_secondPath->AddRef();

    SetHitPoint (pt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
IntersectPath::IntersectPath (IntersectPath const& i2)
    :
    SnapPath (i2)
    {
    m_secondPath = i2.m_secondPath;
    m_secondPath->AddRef ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
IntersectPath::~IntersectPath ()
    {
    if (m_secondPath)
        m_secondPath->Release();

    m_secondPath = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ShaunSewall     12/11
+---------------+---------------+---------------+---------------+---------------+------*/
SnapPath* IntersectPath::Clone () const 
    {
    return new IntersectPath (*this);
    }

/*---------------------------------------------------------------------------------**//**
* determine whether this is the "same" hit as another one. For InteresectPaths, that means that
* both paths of both hits are the same.
* @bsimethod                                                    KeithBentley    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IntersectPath::IsSamePath
(
DisplayPathCP    otherPath,
bool            fullPath    // fullPath arg is ignored for intersect paths!
) const
    {
    // check base paths
    if (!SnapPath::IsSamePath (otherPath, false))
        return false;

    // for IntersectPaths, it can't be the same hit unless both are of type InteresctPath and BOTH paths match
    if (DisplayPathType::Intersection != otherPath->GetPathType())
        return false;

    // now check the "second" paths
    HitPathCP    o2 = ((IntersectPath*) otherPath)->GetSecondPath();

    return GetSecondPath()->IsSamePath (o2, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
void IntersectPath::SetHilited (DgnElement::Hilited newState) const
    {
    SnapPath::SetHilited (newState);

    // when we're turning on the hilite flag, we need to set the second path to "dashed hilite"
    if (DgnElement::Hilited::Normal == newState)
        newState = DgnElement::Hilited::Dashed;

    m_secondPath->SetHilited (newState);
    }

/*---------------------------------------------------------------------------------**//**
* IntersctPaths override the "DrawInView" method to hilite/unhilte BOTH paths that are part of the
* intersction. The "base" path is drawn using the drawmode of the call, but the "second" path
* is drawn using a dashed symbology.
* @bsimethod                                                    KeithBentley    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            IntersectPath::_DrawInVp (DgnViewportP vp, DgnDrawMode drawMode, DrawPurpose drawPurpose, bool* stopFlag) const
    {
    // start by drawing the first path normally
    T_Super::_DrawInVp (vp, drawMode, drawPurpose, stopFlag);

    SnapPath tmpSnapPath (m_secondPath); // So display handlers know this is from a snap...

    // NOTE: When we're flashing, the hilite flags are not necessarily set on the elementRefs. So to get the second path
    //       drawn with dashed symbology, we need to turn on its "dashed hilite" flag temporarily, and then restore it.
    DgnElement::Hilited currHilite = tmpSnapPath.IsHilited ();

    if (DrawPurpose::Flash == drawPurpose)
        tmpSnapPath.SetHilited (DgnElement::Hilited::Dashed);

    tmpSnapPath.SetComponentMode (GetComponentMode ()); // Set correct component flash mode...
    tmpSnapPath.DrawInVp (vp, drawMode, drawPurpose, stopFlag);

    if (DrawPurpose::Flash == drawPurpose)
        tmpSnapPath.SetHilited (currHilite);
    }

/*=================================================================================**//**
* The result of a "locate" is a sorted list of objects that
* satisfied the search  criteria (a HitList). Earlier hits in the list
* are somehow "better" than those later on.
* @see          IHitPath
* @bsiclass                                                     KeithBentley    12/97
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
HitList::HitList ()
    {
    m_currHit = -1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
HitList::~HitList ()
    {
    clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/2008
+---------------+---------------+---------------+---------------+---------------+------*/
int             HitList::GetCount () const      {return (int) size ();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/2008
+---------------+---------------+---------------+---------------+---------------+------*/
SelectionPath*  HitList::Get (int i)
    {
    if (i < 0)                  // ***NEEDS WORK: the old ObjectArray used to support -1 == END
        i = (int) size ();
    if (i >= GetCount())
        return NULL;
    return at(i).get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void            HitList::Set (int i, SelectionPath*p)
    {
    if (i < 0 || i >= GetCount())
        {
        BeAssert (false);
        return;
        }
    at (i) = p;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void            HitList::Insert (int i, SelectionPath *p)
    {
    if (i < 0 || i == (int)size())
        push_back (p);
    else
        insert (begin()+i, p);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void            HitList::DropNulls ()
    {
    erase (std::remove(begin(), end(), (SelectionPath*)NULL), end());
    }

/*---------------------------------------------------------------------------------**//**
* Drop all entries in the HitList. The reference count of all contained hits is decremented
* before they are dropped. Clears the "current hit" index.
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void    HitList::Empty ()
    {
    // empty the list (also decrements ref counts of entries)
    clear ();

    // we don't have a current hit.
    m_currHit = -1;
    }

/*---------------------------------------------------------------------------------**//**
* remove the first hit in the list.
* @bsimethod    Locate.Hitlist                                  KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
void HitList::RemoveHit (int hitNum)
    {
    if (hitNum < 0)                     // *** NEEDS WORK: The old ObjectArray used to support -1 == END
        hitNum = (int) size() - 1;

    if (hitNum == m_currHit)
        m_currHit = -1;

    if (hitNum >= (int) size ())        // Locate calls GetNextHit, which increments m_currHit, until it goes beyond the end of size of the array.
        return;                         // Then Reset call RemoteCurrentHit, which passes in m_currHit. When it's out of range, we do nothing.

    erase (begin() + hitNum);
    }

/*---------------------------------------------------------------------------------**//**
* get a hit from a particular index into a HitList
* @return       the requested hit from the HitList
* @bsimethod    Locate.Hitlist                                  KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
SelectionPath*    HitList::GetHit
(
int     hitNum
) const
    {
    if (hitNum < 0)                     // *** NEEDS WORK: The old ObjectArray used to support -1 == END
        hitNum = (int) size() - 1;

    if (hitNum >= (int) size ())
        return NULL;

    return  at (hitNum).get ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            HitList::RemoveCurrentHit()
    {
    RemoveHit (m_currHit);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            HitList::SetCurrentHit (HitPathCP hit)
    {
    ResetCurrentHit();

    for (HitPathCP thisHit; NULL != (thisHit=(HitPathCP) GetNextHit()); )
        {
        if (thisHit == hit)
            return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    06/02
+---------------+---------------+---------------+---------------+---------------+------*/
static inline double tenthOfPixel (double inValue) {return ((int) ((inValue * 10.0) + 0.5)) / 10.0;}

#define COMPARE_RELATIVE(a,b) {if (a<b) return -1; if (a>b) return 1;}

static const double s_tooCloseTolerance = 1.0e-5;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/04
+---------------+---------------+---------------+---------------+---------------+------*/
static bool is2dHitCompare (HitPathCR oHit1, HitPathCR oHit2)
    {
    return (!oHit1.GetCursorElem()->GetDgnModel().Is3d() && !oHit2.GetCursorElem()->GetDgnModel().Is3d());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/12
+---------------+---------------+---------------+---------------+---------------+------*/
static int doZCompareOfSurfaceAndEdge (HitPathCR oHitSurf, HitPathCR oHitEdge)
    {
    DgnViewportP   vp = oHitSurf.GetViewport ();
    DPoint4d    homogeneousPlane;
    
    if (!vp || !homogeneousPlane.PlaneFromOriginAndNormal (oHitSurf.GetGeomDetail ().GetClosestPointLocal (), oHitSurf.GetGeomDetail ().GetSurfaceNormal ()))
        return 0;

    DMap4dCP    worldToViewMap = vp->GetWorldToViewMap ();
    DMatrix4d   worldToLocal;
    DMap4d      localToWorldMap, localToViewMap;

    worldToLocal.QrInverseOf (oHitSurf.GetGeomDetail ().GetLocalToWorld ());
    localToWorldMap.InitFrom (oHitSurf.GetGeomDetail ().GetLocalToWorld (), worldToLocal);
    localToViewMap.InitProduct (*worldToViewMap, localToWorldMap);

    DPoint4d    eyePointLocal;

    localToViewMap.M1.GetColumn (eyePointLocal, 2);

    DPoint3d    testPointLocal = oHitEdge.GetGeomDetail ().GetClosestPointLocal ();
    DMatrix4d   localEdgeToLocalSurf;

    localEdgeToLocalSurf.InitProduct (worldToLocal, oHitEdge.GetGeomDetail ().GetLocalToWorld ());
    localEdgeToLocalSurf.MultiplyAndRenormalize (&testPointLocal, &testPointLocal, 1);

    double  a0 = homogeneousPlane.DotProduct (eyePointLocal);
    double  a1 = homogeneousPlane.DotProduct (testPointLocal, 1.0);
    double  tol = s_tooCloseTolerance * (1.0 + fabs (a0) + fabs (a1) + fabs (homogeneousPlane.w));

#if defined (NOT_NOT_DUMP)
    if (fabs (a1) < tol)
        printf ("Draw\n\n");
    else if (a0 * a1 > 0)
        printf ("Edge wins\n\n");
    else
        printf ("Surface wins\n\n");
#endif

    if (fabs (a1) < tol)
        return 0;

    return ((a0 * a1 > 0) ? 1 : -1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
static int doZCompare (HitPathCR oHit1, HitPathCR oHit2)
    {
    double  z1 = oHit1.GetGeomDetail ().GetZValue ();
    double  z2 = oHit2.GetGeomDetail ().GetZValue ();

    // For 2d hits z reflects display priority which should be checked before locate priority, etc. when a fill/surface hit is involved...
    if (is2dHitCompare (oHit1, oHit2))
        {
        // screen z values are sorted descending
        COMPARE_RELATIVE (z2, z1);

        return 0;
        }

    // Point clouds already output only a single best z for a screen location...only compare using screen distance, not z...
    if (HIT_DETAIL_PointCloud == oHit1.GetGeomDetail ().GetDetailType () && HIT_DETAIL_PointCloud == oHit2.GetGeomDetail ().GetDetailType ())
        return 0;

    // Always prioritize sprites (ex. HUD markers) over surface hits...
    if (HIT_DETAIL_Sprite == oHit1.GetGeomDetail ().GetDetailType () || HIT_DETAIL_Sprite == oHit2.GetGeomDetail ().GetDetailType ())
        return 0;

    DVec3d  normal1 = oHit1.GetGeomDetail ().GetSurfaceNormal ();
    DVec3d  normal2 = oHit2.GetGeomDetail ().GetSurfaceNormal ();

    if (0.0 != normal1.Magnitude () && 0.0 != normal2.Magnitude ())
        {
        // Both surface hits...if close let other criteria determine order...
        if (DoubleOps::WithinTolerance (z1, z2, s_tooCloseTolerance))
            return 0;
        }
    else if (0.0 != normal1.Magnitude ())
        {
        // 1st is surface hit...project 2nd hit into plane defined by surface normal...
        int compareResult = doZCompareOfSurfaceAndEdge (oHit1, oHit2);

        return (0 == compareResult ? 0 : compareResult);
        }
    else if (0.0 != normal2.Magnitude ())
        {
        // 2nd is surface hit...project 1st hit into plane defined by surface normal...
        int compareResult = doZCompareOfSurfaceAndEdge (oHit2, oHit1);

        return (0 == compareResult ? 0 : -compareResult);
        }
    else
        {
        bool isQvWireHit1 = (HitGeomType::Surface == oHit1.GetGeomDetail ().GetGeomType () && NULL == oHit1.GetGeomDetail ().GetCurvePrimitive ());
        bool isQvWireHit2 = (HitGeomType::Surface == oHit2.GetGeomDetail ().GetGeomType () && NULL == oHit2.GetGeomDetail ().GetCurvePrimitive ());

        // NOTE: QV wireframe hits are only needed to locate silhouettes, make sure they always lose to a real edge hit since a robust z compare isn't possible...
        if (isQvWireHit1 && !isQvWireHit2)
            return 1;
        else if (isQvWireHit2 && !isQvWireHit1)
            return -1;
        else if (DoubleOps::WithinTolerance (z1, z2, s_tooCloseTolerance))
            return 0; // Both QV or real edge hits...if close let other criteria determine order...
        }

    // screen z values are sorted descending
    COMPARE_RELATIVE (z2, z1);

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* compare two hits for insertion into list. Hits are compared by
* calling GetLocatePriority() and then GetLocateDistance() on each.
* @bsimethod    Locate.Hitlist                                  KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
int HitList::Compare (HitPathCP oHit1, HitPathCP oHit2, bool comparePriority, bool compareElemClass, bool compareZ) const
    {
    if (NULL == oHit1 || NULL == oHit2)
        return 0;

    if (compareZ)
        {
        int zCompareValue = doZCompare (*oHit1, *oHit2);

        if (0 != zCompareValue)
            return zCompareValue;
        }

    if (comparePriority)
        {
        int p1 = static_cast<int>(oHit1->GetGeomDetail ().GetLocatePriority ());
        int p2 = static_cast<int>(oHit2->GetGeomDetail ().GetLocatePriority ());

        COMPARE_RELATIVE (p1, p2);
        }

    double dist1 = tenthOfPixel (oHit1->GetGeomDetail ().GetScreenDist ());
    double dist2 = tenthOfPixel (oHit2->GetGeomDetail ().GetScreenDist ());

    COMPARE_RELATIVE (dist1, dist2);

    // Linestyle/pattern/thickness hits have lower priority...
    COMPARE_RELATIVE (oHit1->GetGeomDetail ().GetDetailType (), oHit2->GetGeomDetail ().GetDetailType ());

    //// everything else is the same, higher filepos wins because w/file order display (2d) it's the one on top always.        removed in graphite
    //if (oHit1->GetRoot () == oHit2->GetRoot ())                                                                              removed in graphite
    //    COMPARE_RELATIVE (oHit2->GetHeadElem ()->GetFilePos (), oHit1->GetHeadElem ()->GetFilePos ());                       removed in graphite

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* Add a new hit to the list. Hits are sorted according to their priority and distance.
* @bsimethod    Locate.Hitlist                                  KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
int             HitList::AddHit
(
HitPath*        newHit,
bool            allowDuplicates,
bool            comparePriority,
bool            compareElemClass
)
    {
    HitList::iterator currHit = begin ();

    HitPath*    oldHit;
    int         count = GetCount();
    int         index = 0;
    int         comparison;

    while (index < count)
        {
        oldHit = (HitPath*) currHit->get ();

        comparison = Compare (newHit, oldHit, comparePriority, compareElemClass, true);

        // Caller can establish a policy to only ever allow one hit for a given path. However, we want to make sure
        // that the one hit we do save is the "best" hit for that path. Therefore, every time we get another hit
        // for a path, we drop the one with the lower value based on the comparison, and save the other one.
        if (!allowDuplicates && newHit->IsSamePath (oldHit, true))
            {
            // replace with new hit if it's better (otherwise just ignore it).
            if (comparison < 0)
                *currHit = newHit;

            return count;
            }

        if (comparison < 0)
            break;

        currHit++;
        index++;
        }

    // this increments ref count of newHit
    insert (begin()+index, newHit);

#if defined (NOT_NOT_DUMP)
    printf ("HIT LIST COUNT: %d\n", GetCount ());

    HitPathP    thisPath;

    for (int i=0; NULL != (thisPath = (HitPathP) GetHit (i)); i++)
        printf ("(%d) Elem: %I64d, GeomType: %d Z: %lf Normal: (%lf %lf %lf)\n", i, thisPath->GetHeadElem ()->GetElementId (), thisPath->GetGeomDetail ().GetGeomType (), thisPath->GetGeomDetail ().GetZValue (), thisPath->GetGeomDetail ().GetSurfaceNormal ().x, thisPath->GetGeomDetail ().GetSurfaceNormal ().y, thisPath->GetGeomDetail ().GetSurfaceNormal ().z);

    printf ("\n\n");
#endif

    return index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
SelectionPath*  HitList::GetCurrentHit () const
    {
    if (-1 == m_currHit)
        return NULL;

    return (HitPath*) GetHit (m_currHit);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
SelectionPath*  HitList::GetNextHit ()
    {
    m_currHit++;

    return GetCurrentHit();
    }

/*---------------------------------------------------------------------------------**//**
* search through hitlist and remove any hits that match a specified path.
* @bsimethod                                                    KeithBentley    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     HitList::RemoveHitsMatchingPath
(
DisplayPathCP    path
)
    {
    SelectionPath*  thisPath;
    bool         removedOne = false;

    // walk backwards through list so we don't have to worry about what happens on remove
    for (int i=GetCount()-1; i>=0; i--)
        {
        if ((NULL != (thisPath = GetHit (i))) && thisPath->IsSamePath (path, true))
            {
            removedOne = true;
            RemoveHit (i);
            }
        }
    return  removedOne;
    }

/*---------------------------------------------------------------------------------**//**
* search through hitlist and remove any hits that contain a specified elementRef.
* @bsimethod                                                    KeithBentley    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool HitList::RemoveHitsContaining (DgnElementP element)
    {
    SelectionPath*  thisPath;
    bool         removedOne = false;

    // walk backwards through list so we don't have to worry about what happens on remove
    for (int i=GetCount()-1; i>=0; i--)
        {
        if ((NULL != (thisPath = GetHit (i))) && thisPath->Contains (element))
            {
            removedOne = true;
            RemoveHit (i);
            }
        }
    return  removedOne;
    }

/*---------------------------------------------------------------------------------**//**
* search through hitlist and remove any hits from the specified modelRef
* @bsimethod                                                    KeithBentley    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     HitList::RemoveHitsFrom (DgnModelP modelRef)
    {
    SelectionPath*  thisPath;
    bool         removedOne = false;

    // walk backwards through list so we don't have to worry about what happens on remove
    for (int i=GetCount()-1; i>=0; i--)
        {
        if ((NULL != (thisPath = GetHit (i))) && modelRef == thisPath->GetRoot())
            {
            removedOne = true;
            RemoveHit (i);
            }
        }
    return  removedOne;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod    Locate.Hitlist                                  KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
void HitList::Dump (WCharCP label) const
    {
    printf ("%ls %d", label, GetCount());

    SelectionPath*  thisPath;
    for (int i=0; NULL != (thisPath = GetHit(i)); i++)
        thisPath->Dump(L"\n");

    printf ("\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       HitPath::GetLinearParameters (DSegment3dP hitSeg, int* vertex, int* segmentNumber) const
    {
    GeomDetail const&  detail = GetGeomDetail ();

    if (hitSeg)
        {
        if (!detail.GetSegment (*hitSeg))
            return ERROR;

        // Return segment in world coords...
        detail.GetLocalToWorld ().MultiplyAndRenormalize (hitSeg->point, hitSeg->point, 2);
        }

    if (vertex)
        *vertex = (int) detail.GetCloseVertex ();

    if (segmentNumber)
        *segmentNumber = (int) detail.GetSegmentNumber ();

    return SUCCESS;
    }

#ifdef WIP_VANCOUVER_MERGE // DgnActionItem
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DgnActionItemContext::DgnActionItemContext (HitPathCP hitPath, DPoint3dCP point, DgnViewportP view)
    :m_hitPathPtr(NULL), m_pointPtr (NULL), m_view (view)
    {
    if (NULL != point)
        m_pointPtr = std::shared_ptr<DPoint3d>(new DPoint3d(*point));

    if (NULL != hitPath)
        m_hitPathPtr = new HitPath(*hitPath);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct NullDgnActionItemContext: public DgnActionItemContext
    {
    private:
    ECN::IAUIDataContextCP m_context;
    
    virtual ContextType _GetContextType() const override {return m_context->GetContextType();}
    virtual ECN::IECInstanceP       GetInstance () const override {return m_context->GetInstance();}
    virtual void*                   GetCustomData() const override {return m_context->GetCustomData();}
    virtual ECN::ECInstanceIterableCP    GetInstanceIterable () const override {return m_context->GetInstanceIterable();}
    virtual WString                 GetMoniker () const override {return m_context->GetMoniker();}
    

    public:
        NullDgnActionItemContext (ECN::IAUIDataContextCR context)
            :m_context(&context), DgnActionItemContext(NULL, NULL, NULL)
            {}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void    IEditActionSource::_GetCommand (bvector<ECN::IUICommandPtr>& cmds, ECN::IAUIDataContextCR instance, int purpose)
    {
    bvector<DgnPlatform::IEditActionPtr> dgnPlatformCmds;
    DgnActionItemContext const* info = dynamic_cast<DgnActionItemContext const*> (&instance);
    if (NULL != info)
        _GetCommand(dgnPlatformCmds, *info, purpose);
    else
        {
        NullDgnActionItemContext context(instance);
        _GetCommand(dgnPlatformCmds, context, purpose);
        }

    std::copy(dgnPlatformCmds.begin(), dgnPlatformCmds.end(), std::back_inserter(cmds));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HitPathCP       DgnActionItemContext::GetHitPath() const {return  m_hitPathPtr.get();}
DgnViewportP       DgnActionItemContext::GetView() const 
    {
    return (NULL == m_hitPathPtr.get()) ? m_view : m_hitPathPtr->GetViewport();
    }

DPoint3dCP      DgnActionItemContext::GetPoint() const {return  m_pointPtr.get();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandle   DgnActionItemContext::GetRootElement() const 
    {
    if (m_hitPathPtr.IsNull())
        return ElementHandle();

    DgnElementP     element = m_hitPathPtr->GetPathElem(0);
    DgnModelP    modelRef = m_hitPathPtr->GetRoot ();

    return ElementHandle(element, modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnUISeperator : public IEditAction
    {
    WString m_id;
    DgnUISeperator (WCharCP id)
        :m_id(id)
        {}

    virtual WCharCP             _GetCommandId () const override { return m_id.c_str(); }
    virtual WString             _GetLabel ()  const override { return L""; }
    virtual WString             _GetDescription ()  const override { return L""; }
    virtual ECN::ECImageKeyCP   _GetImageId ()  const override { return NULL; }
    virtual bool                _IsSeparator ()  const override { return true; }
    
    virtual EditActionMenuMark  _GetMenuMark() const override {return MENUMARK_None;}
    virtual bool                _GetIsEnabled () const override {return true;}
    virtual EditActionPriority  _GetPriority () const override {return EDITACTION_PRIORITY_UserCommon;}
    virtual BentleyStatus   _ExecuteCmd (ECN::IAUIDataContextCP instance) override
        {
        BeAssert(false);
        return ERROR;
        }
    public:
    static IEditAction* CreateSeparator(WCharCP id) {return new DgnUISeperator(id);}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IEditActionPtr  DgnEditAction::CreateSeparator (WCharCP id)
    {
    return DgnUISeperator::CreateSeparator(id);
    }

struct ECInstanceToDgnECInstanceCollectionAdapter;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECInstanceToDgnECInstanceCollectionAdapterIter : public IDgnECInstanceCollectionIteratorAdapter
    {
    private:
        ECN::ECInstanceIterable::const_iterator                  m_iter;
        DgnECInstancePtr                                    m_val;
        bool                                                m_valFetched;
        ECInstanceToDgnECInstanceCollectionAdapter const&   m_parent;

        
    public:
    ECInstanceToDgnECInstanceCollectionAdapterIter (ECInstanceToDgnECInstanceCollectionAdapter const& parent, bool begin);

    virtual void                MoveToNext  () override
        {
        ++m_iter;
        m_val = NULL;
        m_valFetched = false;
        }

    virtual bool                IsDifferent (ECN::IInstanceCollectionIteratorAdapter<DgnECInstanceP const> const & rhs) const override
        {
        ECInstanceToDgnECInstanceCollectionAdapterIter const* rhsIter = static_cast<ECInstanceToDgnECInstanceCollectionAdapterIter const*>(&rhs);
        return m_iter != rhsIter->m_iter;
        }
    
    virtual reference           GetCurrent () override;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECInstanceToDgnECInstanceCollectionAdapter : public IDgnECInstanceCollectionAdapter
    {
    friend ECInstanceToDgnECInstanceCollectionAdapterIter;
    
    private:
        ECN::ECInstanceIterableCP       m_iterable;
    public:
        ECInstanceToDgnECInstanceCollectionAdapter (ECN::ECInstanceIterableCP const& iterable)
            :m_iterable(iterable)
            {}

    virtual const_iterator begin() const override
        {
        return new ECInstanceToDgnECInstanceCollectionAdapterIter(*this, true);
        }
    virtual const_iterator end() const override
        {
        return new ECInstanceToDgnECInstanceCollectionAdapterIter(*this, false);
        }

    static ECInstanceToDgnECInstanceCollectionAdapter* Create(ECN::ECInstanceIterableCP const& iterable) {return new ECInstanceToDgnECInstanceCollectionAdapter(iterable);}
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceToDgnECInstanceCollectionAdapterIter::ECInstanceToDgnECInstanceCollectionAdapterIter (ECInstanceToDgnECInstanceCollectionAdapter const& parent, bool begin)
:m_iter(begin ? parent.m_iterable->begin() : parent.m_iterable->end()), m_valFetched(false), m_parent(parent)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceToDgnECInstanceCollectionAdapterIter::reference ECInstanceToDgnECInstanceCollectionAdapterIter::GetCurrent ()
    {
    if (m_valFetched)
        return m_val.GetCR();

    m_valFetched = true;
    DgnECInstanceP instance = dynamic_cast<DgnECInstanceP> (*m_iter);
    if (NULL == instance)
        return m_val.GetCR();
    
    m_val = instance;
    return m_val.GetCR();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECInstanceIterable   DgnActionItemContext::_GetDgnInstanceIterable() const
    {
    ECN::ECInstanceIterableCP ecIterable = GetInstanceIterable();
    if (NULL != ecIterable)
        return DgnECInstanceIterable(ECInstanceToDgnECInstanceCollectionAdapter::Create(ecIterable));

    ElementHandle element = GetRootElement();
    if (element.IsValid())
        {
        FindInstancesScopePtr scope = FindInstancesScope::CreateScope(element, false);
        ECQueryPtr query = ECQuery::CreateQuery(ECQUERY_PROCESS_SearchAllClasses);
        return Bentley::DgnPlatform::DgnECManager::GetManager().FindInstances(*scope, *query);
        }


    return DgnECInstanceIterable::CreateEmpty();
    }
#endif