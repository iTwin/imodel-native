/*----------------------------------------------------------------------+
|
|   $Source: DgnHandlers/snapassoc.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/01
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::assoc_getSnapPoint
(
DPoint3dR       snapPoint,
SnapPathCP      snapPathP,
DgnModelP    parentModel_unused,
bool            localCoords_unused
)
    {
    snapPoint = snapPathP->GetSnapPoint ();

#if defined (WIP_V10_ASSOC_POINT)
    if (!localCoords)
        return;

    Transform   fwdTrans, invTrans;

    if (SUCCESS != assoc_getPathTransformToParent (&fwdTrans, snapPathP, parentModel))
        return;

    invTrans.inverseOf (&fwdTrans);
    invTrans.multiply (&snapPoint);
#endif
    }

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  06/09
+===============+===============+===============+===============+===============+======*/
struct SnapAssocHelper
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/01
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus /*SnapAssocHelper::*/SetAndValidateRoots
(
AssocPoint&     assoc,
HitPathCR       path,
DgnModelP    parentModel,
bool            allowFarElm,
int             iRoot = 0,
bool            validate = true
)
    {
    if (SUCCESS != AssociativePoint::SetRoot (assoc, &path, parentModel, allowFarElm, iRoot))
        return ERROR;

    return validate ? AssociativePoint::IsValid (assoc, path.GetEffectiveRoot (), parentModel) : SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus /*SnapAssocHelper::*/CreateKeypoint
(
AssocPoint&     assoc,
UShort          apparentPointCount,
SnapPathCR      snapPath,
int             modifierMask,
bool            allowFarElm,
DgnModelP    parentModel
)
    {
    if (0 == (LINEAR_ASSOC & ~modifierMask))
        return ERROR;

    if (HitGeomType::Point == snapPath.GetGeomDetail().GetEffectiveHitGeomType ())
        {
        AssociativePoint::InitKeypoint (assoc, 0, 1, 1, 1);

        if (SUCCESS != AssociativePoint::SetRoot (assoc, &snapPath, parentModel, allowFarElm))
            return ERROR;

        return AssociativePoint::IsValid (assoc, snapPath.GetEffectiveRoot (), parentModel);
        }

    if (HitGeomType::Segment != snapPath.GetGeomDetail().GetEffectiveHitGeomType ())
        return ERROR;

    UShort     vertex, divisor, numerator;

    vertex    = (UShort)snapPath.GetGeomDetail().GetSegmentNumber ();
    divisor   = (UShort)snapPath.GetSnapDivisor ();
    numerator = (UShort)(snapPath.GetGeomDetail().GetSegmentParam () * divisor + 0.5);

    AssociativePoint::InitKeypoint (assoc, vertex, apparentPointCount, numerator, divisor);

    return /*SnapAssocHelper::*/SetAndValidateRoots (assoc, snapPath, parentModel, allowFarElm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus /*SnapAssocHelper::*/CreateProjection
(
AssocPoint&     assoc,
DPoint3dCP      pointsP,
int             nPoints,
bool            isLine,
SnapPathCR      snapPath,
int             modifierMask,
bool            allowFarElm,
DgnModelP    parentModel
)
    {
    if (0 == (PROJECTION_ASSOC & ~modifierMask))
        return ERROR;

    if (snapPath.GetGeomDetail().GetEffectiveHitGeomType () != HitGeomType::Segment)
        return ERROR;

    int         vertex = (int) snapPath.GetGeomDetail().GetSegmentNumber ();
    DPoint3d    hitPoint;

    // test point needs to be in local coords of element
    assoc_getSnapPoint (hitPoint, &snapPath, parentModel, true);

    if (0 == nPoints || vertex >= nPoints-1)
        return ERROR;

    double      segDist = 0.0, pointDist = 0.0;
    DVec3d      segDir, locateVec;

    segDist = segDir.NormalizedDifference (pointsP[vertex+1], pointsP[vertex]);
    bsiDVec3d_subtractDPoint3dDPoint3d (&locateVec, &hitPoint, &pointsP[vertex]);
    pointDist = bsiDVec3d_dotProduct (&segDir, &locateVec);

    double      ratio = (segDist > mgds_fc_nearZero) ? pointDist / segDist : 0;

    AssociativePoint::InitProjection (assoc, (UShort)vertex, (UShort)(isLine ? (LegacyMath::RpntEqual (&pointsP[0], &pointsP[1]) ? 1 : 2) : nPoints), ratio);

    return /*SnapAssocHelper::*/SetAndValidateRoots (assoc, snapPath, parentModel, allowFarElm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus /*SnapAssocHelper::*/CreateArc
(
AssocPoint&     assoc,
DEllipse3dR     ellipse,
SnapPathCR      snapPath,
int             modifierMask,
bool            allowFarElm,
DgnModelP    parentModel
)
    {
    if (0 == (ARC_ASSOC & ~modifierMask))
        return ERROR;

    switch (snapPath.GetGeomDetail().GetEffectiveHitGeomType ())
        {
        case HitGeomType::Point:
            {
            AssociativePoint::InitArc (assoc, AssociativePoint::ARC_CENTER);

            return /*SnapAssocHelper::*/SetAndValidateRoots (assoc, snapPath, parentModel, allowFarElm);
            }

        case HitGeomType::Arc:
            {
            double  param = snapPath.GetGeomDetail().GetCloseParam ();

            // param is only reliable for checking start/end of arc...
            if (fabs (param) < mgds_fc_epsilon)
                {
                AssociativePoint::InitArc (assoc, AssociativePoint::ARC_START);

                return /*SnapAssocHelper::*/SetAndValidateRoots (assoc, snapPath, parentModel, allowFarElm);
                }
            else if (fabs (1.0 - fabs (param)) < mgds_fc_epsilon)
                {
                AssociativePoint::InitArc (assoc, AssociativePoint::ARC_END);

                return /*SnapAssocHelper::*/SetAndValidateRoots (assoc, snapPath, parentModel, allowFarElm);
                }
            else
                {
                double      r0, r1, start, sweep;
                DPoint3d    center;
                RotMatrix   rMatrix;

                ellipse.GetScaledRotMatrix (center, rMatrix, r0, r1, start, sweep);

                DVec3d      hitDir;
                DPoint3d    hitPoint;

                assoc_getSnapPoint (hitPoint, &snapPath, parentModel, true);
                hitDir.DifferenceOf (hitPoint, center);
                rMatrix.MultiplyTranspose (hitDir);

                double      angle = atan2 (r0 * hitDir.y, r1 * hitDir.x);

                AssociativePoint::InitArc (assoc, AssociativePoint::ARC_ANGLE, angle);

                return /*SnapAssocHelper::*/SetAndValidateRoots (assoc, snapPath, parentModel, allowFarElm);
                }

            break;
            }
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus /*SnapAssocHelper::*/CreateBCurve
(
AssocPoint&         assoc,
MSBsplineCurveCR    curve,
SnapPathCR          snapPath,
int                 modifierMask,
bool                allowFarElm,
DgnModelP        parentModel
)
    {
    GeomDetail const&   detail = snapPath.GetGeomDetail();

    if (BCURVE_ELEMARG_None != detail.GetElemArg ())
        {
        int     nVertex, vertex, numerator, divisor;

        if (BCURVE_ELEMARG_FitCurvePolygon == detail.GetElemArg ())
            {
            nVertex     = curve.params.numPoles;
            vertex      = (int) snapPath.GetGeomDetail().GetCloseVertex ();
            divisor     = 1;
            numerator   = (vertex == nVertex-1) ? 1 : 0;
            }
        else if (BCURVE_ELEMARG_ControlPolygon == detail.GetElemArg ())
            {
            nVertex     = curve.params.numPoles;
            vertex      = (int) snapPath.GetGeomDetail().GetSegmentNumber ();
            divisor     = snapPath.GetSnapDivisor ();
            numerator   = int (snapPath.GetGeomDetail().GetSegmentParam () * divisor + 0.5);
            }
        else
            {
            BeAssert (false);
            return ERROR;
            }

        AssociativePoint::InitKeypoint (assoc, (UShort)vertex, (UShort)nVertex, (UShort)numerator, (UShort)divisor);

        return /*SnapAssocHelper::*/SetAndValidateRoots (assoc, snapPath, parentModel, allowFarElm);
        }

    if (0 == (BCURVE_ASSOC & ~modifierMask))
        return ERROR;

    double      uParam = 0.0, tolerance = mgds_fc_epsilon;
    DPoint3d    hitPoint;

    assoc_getSnapPoint (hitPoint, &snapPath, parentModel, true);

    if (SUCCESS != bsprcurv_minDistToCurve (NULL, NULL, &uParam, &hitPoint, const_cast <MSBsplineCurveP> (&curve), &tolerance, NULL))
        return ERROR;

    AssociativePoint::InitBCurve (assoc, uParam);

    return /*SnapAssocHelper::*/SetAndValidateRoots (assoc, snapPath, parentModel, allowFarElm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus /*SnapAssocHelper::*/CreateBSurface
(
AssocPoint&         assoc,
MSBsplineSurfaceCR  surface,
SnapPathCR          snapPath,
int                 modifierMask,
bool                allowFarElm,
DgnModelP        parentModel
)
    {
    if (0 == (BSURF_ASSOC & ~modifierMask))
        return ERROR;

    DPoint2d    uv;
    DPoint3d    hitPoint;

    memset (&uv, 0, sizeof (uv));
    assoc_getSnapPoint (hitPoint, &snapPath, parentModel, true);
    DPoint3d xyz;
    surface.ClosestPoint (xyz, uv, hitPoint);

    AssociativePoint::InitBSurface (assoc, uv.x, uv.y);

    return /*SnapAssocHelper::*/SetAndValidateRoots (assoc, snapPath, parentModel, allowFarElm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus /*SnapAssocHelper::*/CreateText
(
AssocPoint&     assoc,
SnapPathCR      snapPath,
int             modifierMask,
bool            allowFarElm,
DgnModelP    parentModel
)
    {
    if (0 == (ORIGIN_ASSOC & ~modifierMask))
        return ERROR;

    int         option = 0;
    DPoint3d    rShape[9], hitPoint;
    ElementHandle  eh (snapPath.GetCursorElem ());

    switch (eh.GetLegacyType())
        {
        case TEXT_ELM:
        case TEXT_NODE_ELM:
            {
            DPoint3d    userOrigin;

            if (SUCCESS != ElementUtil::ExtractTextShape (eh, rShape, userOrigin))
                return ERROR;

            break;
            }

        default:
            return ERROR;
        }

    /* Control points:  2 - 6 - 3
                        5 - 9 - 7
                        1 - 8 - 4 (zero is user origin) */

    bsiDPoint3d_interpolate (rShape+4, rShape,   0.5, rShape+1);
    bsiDPoint3d_interpolate (rShape+5, rShape+1, 0.5, rShape+2);
    bsiDPoint3d_interpolate (rShape+6, rShape+2, 0.5, rShape+3);
    bsiDPoint3d_interpolate (rShape+7, rShape+3, 0.5, rShape);
    bsiDPoint3d_interpolate (rShape+8, rShape,   0.5, rShape+2);

    assoc_getSnapPoint (hitPoint, &snapPath, parentModel, true);
    option = LineStringUtil::GetClosestIndex (hitPoint, rShape, 9);

    AssociativePoint::InitOrigin (assoc, static_cast <UShort> (option+1));

    return /*SnapAssocHelper::*/SetAndValidateRoots (assoc, snapPath, parentModel, allowFarElm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus /*SnapAssocHelper::*/CreateMline
(
AssocPoint&     assoc,
SnapPathCR      snapPath,
int             modifierMask,
bool            allowFarElm,
DgnModelP    parentModel
)
    {
    return MultilineHandler::CreateMlineAssoc (ElementHandle (snapPath.GetCursorElem ()), assoc, snapPath, modifierMask, allowFarElm, parentModel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus /*SnapAssocHelper::*/CreateMesh
(
AssocPoint&     assoc,
SnapPathCR      snapPath,
int             modifierMask,
bool            allowFarElm,
DgnModelP    parentModel
)
    {
    return MeshHeaderHandler::CreateMeshAssoc (ElementHandle (snapPath.GetCursorElem ()), assoc, snapPath, modifierMask, allowFarElm, parentModel);
    }

/*---------------------------------------------------------------------------------**//**
    CurveVectorPtr pathCurve = ICurvePathQuery::ElementToCurveVector (ElementHandle (snapPath.GetCursorElem (), snapPath.GetRoot ()));
    ICurvePrimitivePtr& pathMember = pathCurve->front ();
    switch (pathCurve->HasSingleCurvePrimitive ())
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
* @bsimethod                                                    Brien.Bastings  11/04
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus /*SnapAssocHelper::*/CreateCustom
(
AssocPoint&     assocPoint,
SnapPathCR      snapPath,
int             modifierMask,
bool            allowFarElm,
DgnModelP    parentModel
)
    {
#if defined (WIP_V10_CUSTOMASSOC)
    if (0 == (CUSTOM_ASSOC & ~modifierMask))
        return ERROR;

    int         nBytes = 0;
    byte*       dataP = NULL;

    if (!snapPath.GetCustomKeypoint (&nBytes, &dataP))
        return ERROR;

    AssocGeom&  assoc = (AssocGeom&) assocPoint;

    memset (&assoc, 0, sizeof (assoc));
    assoc.customKeypoint.type = CUSTOM_ASSOC;

    LinkageHeader   customKeypointHdr;

    memset (&customKeypointHdr, 0, sizeof (customKeypointHdr));

    customKeypointHdr.primaryID = LINKAGEID_CustomKeypoint;
    customKeypointHdr.user      = true;

    if (SUCCESS != LinkageUtil::SetWords (&customKeypointHdr, LinkageUtil::CalculateSize (sizeof (LinkageHeader) + nBytes)/2))
        return ERROR;

    //  ------------------------------------------------------------------------------------
    //  Create a far reference path element, with the keypoint state as a linkage
    //      In the case of a custom keypoint, we need an element to hold the custom keypoint
    //      state as a payload. We also need a way to capture the path to the target, wherever
    //      it may be. The far reference path element is an element, so it can carry a payload,
    //      and it can capture a path. So, we build on the path element technique to store keypoints.

    //      Note: The far reference path element is designed to hold the path only. That is, the roots in the path
    //      element's dependency linkage point to things like reference attachments and shared cell instances.
    //      The ultimate target of the dependency is not stored in the path element, but in the assocpoint itself.
    //      Now, we always want to create a path element, even if there is no path.
    //      So, in the case where we are referencing an element that does not actually need a path, the path element
    //      will contain zero roots. That's fine. In that case, the path element just serves to hold our keypoint data.

    // NOTE: LinkageUtil::SetWords rounds the size up to a power of 2. Pad data to linkage size to avoid problems w/append.
    byte*   tmpDataP = (byte *) memutil_calloc (LinkageUtil::GetWords (&customKeypointHdr)*2, sizeof (byte), HEAPSIG_SNAP);

    memcpy (tmpDataP, dataP, nBytes);

    BentleyStatus   status = (BentleyStatus) assoc_createFarReference (NULL, &assoc.customKeypoint.targetElementId, &assoc.customKeypoint.pathElementId, &snapPath, true, &customKeypointHdr, tmpDataP, parentModel);

    memutil_free (tmpDataP);

    return status;
#endif
BeAssert (false);
return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus /*SnapAssocHelper::*/CreateFromKeypointSnap
(
AssocPoint&     assoc,
SnapPathCR      snapPath,
int             modifierMask,
bool            allowFarElm,
DgnModelP    parentModel
)
    {
    ElementHandle   eh (snapPath.GetCursorElem ());
    Handler*        handler = &eh.GetHandler ();

    IMultilineQuery*    mlineQuery;

    if (NULL != (mlineQuery = dynamic_cast <IMultilineQuery*> (handler)))
        return /*SnapAssocHelper::*/CreateMline (assoc, snapPath, modifierMask, allowFarElm, parentModel);

    IMeshQuery*         meshQuery;

    if (NULL != (meshQuery = dynamic_cast <IMeshQuery*> (handler)))
        return /*SnapAssocHelper::*/CreateMesh (assoc, snapPath, modifierMask, allowFarElm, parentModel);

    IBsplineSurfaceQuery*   bsurfQuery;

    if (NULL != (bsurfQuery = dynamic_cast <IBsplineSurfaceQuery*> (handler)))
        {
        MSBsplineSurfacePtr surface;

        if (SUCCESS != bsurfQuery->GetBsplineSurface (eh, surface))
            return ERROR;

        BentleyStatus   status = /*SnapAssocHelper::*/CreateBSurface (assoc, *surface, snapPath, modifierMask, allowFarElm, parentModel);

        return status;
        }

    ITextQuery*     textQuery;

    if (NULL != (textQuery = dynamic_cast <ITextQuery*> (handler)) && textQuery->IsTextElement (eh))
        {
        if (0 == (ORIGIN_ASSOC & ~modifierMask))
            return ERROR;

        AssociativePoint::InitOrigin (assoc, 0);

        return /*SnapAssocHelper::*/SetAndValidateRoots (assoc, snapPath, parentModel, allowFarElm);
        }

    CurveVectorPtr pathCurve = ICurvePathQuery::ElementToCurveVector (eh);

    if (pathCurve.IsNull ())
        return ERROR;

    ICurvePrimitivePtr& pathMember = pathCurve->front ();

    switch (pathCurve->HasSingleCurvePrimitive ())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            {
            DSegment3d  segment = *pathMember->GetLineCP ();

            return /*SnapAssocHelper::*/CreateKeypoint (assoc, LegacyMath::RpntEqual (&segment.point[0], &segment.point[1]) ? 1 : 2, snapPath, modifierMask, allowFarElm, parentModel);
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            {
            bvector<DPoint3d> const* points = pathMember->GetLineStringCP ();

            return /*SnapAssocHelper::*/CreateKeypoint (assoc, (UShort) points->size (), snapPath, modifierMask, allowFarElm, parentModel);
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
            {
            bvector<DPoint3d> const* points = pathMember->GetPointStringCP ();

            return /*SnapAssocHelper::*/CreateKeypoint (assoc, (UShort) points->size (), snapPath, modifierMask, allowFarElm, parentModel);
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            {
            DEllipse3d  ellipse = *pathMember->GetArcCP ();

            return /*SnapAssocHelper::*/CreateArc (assoc, ellipse, snapPath, modifierMask, allowFarElm, parentModel);
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
            {
            MSBsplineCurveCP curve = pathMember->GetProxyBsplineCurveCP ();
        
            return /*SnapAssocHelper::*/CreateBCurve (assoc, *curve, snapPath, modifierMask, allowFarElm, parentModel);
            }

        default:
            return ERROR;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus /*SnapAssocHelper::*/CreateFromOriginSnap
(
AssocPoint&     assoc,
SnapPathCR      snapPath,
int             modifierMask,
bool            allowFarElm,
DgnModelP    parentModel
)
    {
    ElementHandle   eh (snapPath.GetCursorElem ());
    Handler*        handler = &eh.GetHandler ();

    IMultilineQuery*    mlineQuery;

    if (NULL != (mlineQuery = dynamic_cast <IMultilineQuery*> (handler)))
        {
        if (0 == (MLINE_ASSOC & ~modifierMask))
            return ERROR;

        int     lineNo = 0;

        if (SUCCESS != snapPath.GetMultilineParameters (NULL, NULL, NULL, &lineNo, NULL, NULL))
            return ERROR;

        AssociativePoint::InitMline (assoc, 0, 0, (UShort)lineNo, 0.0, true);

        return /*SnapAssocHelper::*/SetAndValidateRoots (assoc, snapPath, parentModel, allowFarElm);
        }

#if defined (NEEDS_WORK_DGNITEM)
    ICellQuery*         cellQuery;

    if (NULL != (cellQuery = dynamic_cast <ICellQuery*> (handler)))
        {
        if (0 == (ORIGIN_ASSOC & ~modifierMask))
            return ERROR;

        AssociativePoint::InitOrigin (assoc, 0);

        return /*SnapAssocHelper::*/SetAndValidateRoots (assoc, snapPath, parentModel, allowFarElm);
        }
#endif

    ITextQuery*         textQuery;

    if (NULL != (textQuery = dynamic_cast <ITextQuery*> (handler)) && textQuery->IsTextElement (eh))
        {
        if (0 == (ORIGIN_ASSOC & ~modifierMask))
            return ERROR;

        AssociativePoint::InitOrigin (assoc, 0);

        return /*SnapAssocHelper::*/SetAndValidateRoots (assoc, snapPath, parentModel, allowFarElm);
        }

    CurveVectorPtr pathCurve = ICurvePathQuery::ElementToCurveVector (ElementHandle (snapPath.GetCursorElem ()));

    if (pathCurve.IsNull ())
        return ERROR;

    switch (pathCurve->HasSingleCurvePrimitive ())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
            {
            if (0 == (LINEAR_ASSOC & ~modifierMask))
                return ERROR;

            AssociativePoint::InitKeypoint (assoc, 0, 0, 0, 2);

            return /*SnapAssocHelper::*/SetAndValidateRoots (assoc, snapPath, parentModel, allowFarElm);
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            {
            if (0 == (ARC_ASSOC & ~modifierMask))
                return ERROR;

            AssociativePoint::InitArc (assoc, pathCurve->IsClosedPath () ? AssociativePoint::ARC_CENTER : AssociativePoint::ARC_START);

            return /*SnapAssocHelper::*/SetAndValidateRoots (assoc, snapPath, parentModel, allowFarElm);
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
            {
            if (0 == (BCURVE_ASSOC & ~modifierMask))
                return ERROR;

            AssociativePoint::InitBCurve (assoc, 0.0);

            return /*SnapAssocHelper::*/SetAndValidateRoots (assoc, snapPath, parentModel, allowFarElm);
            }

        default:
            return ERROR;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus /*SnapAssocHelper::*/CreateFromCenterSnap
(
AssocPoint&     assoc,
SnapPathCR      snapPath,
int             modifierMask,
bool            allowFarElm,
DgnModelP    parentModel
)
    {
    ElementHandle   eh (snapPath.GetCursorElem ());
    Handler*        handler = &eh.GetHandler ();

#if defined (NEEDS_WORK_DGNITEM)
    ICellQuery*         cellQuery;

    if (NULL != (cellQuery = dynamic_cast <ICellQuery*> (handler)))
        {
        if (0 == (ORIGIN_ASSOC & ~modifierMask))
            return ERROR;

        AssociativePoint::InitOrigin (assoc, 0);

        return /*SnapAssocHelper::*/SetAndValidateRoots (assoc, snapPath, parentModel, allowFarElm);
        }
#endif

    ITextQuery*         textQuery;

    if (NULL != (textQuery = dynamic_cast <ITextQuery*> (handler)) && textQuery->IsTextElement (eh))
        {
        if (0 == (ORIGIN_ASSOC & ~modifierMask))
            return ERROR;

        AssociativePoint::InitOrigin (assoc, 9);

        return /*SnapAssocHelper::*/SetAndValidateRoots (assoc, snapPath, parentModel, allowFarElm);
        }

    CurveVectorPtr pathCurve = ICurvePathQuery::ElementToCurveVector (ElementHandle (snapPath.GetCursorElem ()));

    if (pathCurve.IsNull ())
        return ERROR;

    switch (pathCurve->HasSingleCurvePrimitive ())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            {
            if (0 == (LINEAR_ASSOC & ~modifierMask))
                return ERROR;

            AssociativePoint::InitKeypoint (assoc, 0, 0, 1, 2);

            return /*SnapAssocHelper::*/SetAndValidateRoots (assoc, snapPath, parentModel, allowFarElm);
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            {
            if (0 == (ARC_ASSOC & ~modifierMask))
                return ERROR;

            AssociativePoint::InitArc (assoc, AssociativePoint::ARC_CENTER);

            return /*SnapAssocHelper::*/SetAndValidateRoots (assoc, snapPath, parentModel, allowFarElm);
            }

        default:
            return ERROR;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus /*SnapAssocHelper::*/CreateFromMidpointSnap
(
AssocPoint&     assoc,
SnapPathCR      snapPath,
int             modifierMask,
bool            allowFarElm,
DgnModelP    parentModel
)
    {
    ElementHandle   eh (snapPath.GetCursorElem ());
    Handler*        handler = &eh.GetHandler ();

#if defined (NEEDS_WORK_DGNITEM)
    ICellQuery*         cellQuery;

    if (NULL != (cellQuery = dynamic_cast <ICellQuery*> (handler)))
        {
        if (0 == (ORIGIN_ASSOC & ~modifierMask))
            return ERROR;

        AssociativePoint::InitOrigin (assoc, 0);

        return /*SnapAssocHelper::*/SetAndValidateRoots (assoc, snapPath, parentModel, allowFarElm);
        }
#endif

    IMultilineQuery*    mlineQuery;

    if (NULL != (mlineQuery = dynamic_cast <IMultilineQuery*> (handler)))
        return /*SnapAssocHelper::*/CreateMline (assoc, snapPath, modifierMask, allowFarElm, parentModel);

    ITextQuery*         textQuery;

    if (NULL != (textQuery = dynamic_cast <ITextQuery*> (handler)) && textQuery->IsTextElement (eh))
        return /*SnapAssocHelper::*/CreateText (assoc, snapPath, modifierMask, allowFarElm, parentModel);

    CurveVectorPtr pathCurve = ICurvePathQuery::ElementToCurveVector (ElementHandle (snapPath.GetCursorElem ()));

    if (pathCurve.IsNull ())
        return ERROR;

    ICurvePrimitivePtr& pathMember = pathCurve->front ();

    switch (pathCurve->HasSingleCurvePrimitive ())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            {
            if (0 == (LINEAR_ASSOC & ~modifierMask))
                return ERROR;

            AssociativePoint::InitKeypoint (assoc, 0, 0, 1, 2);

            return /*SnapAssocHelper::*/SetAndValidateRoots (assoc, snapPath, parentModel, allowFarElm);
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            {
            bvector<DPoint3d> const* points = pathMember->GetLineStringCP ();

            return /*SnapAssocHelper::*/CreateProjection (assoc, &points->front (), (int) points->size (), false, snapPath, modifierMask, allowFarElm, parentModel);
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
            {
            bvector<DPoint3d> const* points = pathMember->GetPointStringCP ();

            return /*SnapAssocHelper::*/CreateProjection (assoc, &points->front (), (int) points->size (), false, snapPath, modifierMask, allowFarElm, parentModel);
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            {
            DEllipse3d  ellipse = *pathMember->GetArcCP ();

            return /*SnapAssocHelper::*/CreateArc (assoc, ellipse, snapPath, modifierMask, allowFarElm, parentModel);
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
            {
            MSBsplineCurveCP curve = pathMember->GetProxyBsplineCurveCP ();
        
            return /*SnapAssocHelper::*/CreateBCurve (assoc, *curve, snapPath, modifierMask, allowFarElm, parentModel);
            }

        default:
            return ERROR;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus /*SnapAssocHelper::*/CreateFromBisectorSnap
(
AssocPoint&     assoc,
SnapPathCR      snapPath,
int             modifierMask,
bool            allowFarElm,
DgnModelP    parentModel
)
    {
    // Currently bisector/midpoint are identical?!?
    return /*SnapAssocHelper::*/CreateFromMidpointSnap (assoc, snapPath, modifierMask, allowFarElm, parentModel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus /*SnapAssocHelper::*/CreateFromNearestSnap
(
AssocPoint&     assoc,
SnapPathCR      snapPath,
int             modifierMask,
bool            allowFarElm,
DgnModelP    parentModel
)
    {
    ElementHandle   eh (snapPath.GetCursorElem ());
    Handler*        handler = &eh.GetHandler ();

    IMultilineQuery*    mlineQuery;

    if (NULL != (mlineQuery = dynamic_cast <IMultilineQuery*> (handler)))
        return /*SnapAssocHelper::*/CreateMline (assoc, snapPath, modifierMask, allowFarElm, parentModel);

    ITextQuery*         textQuery;

    if (NULL != (textQuery = dynamic_cast <ITextQuery*> (handler)) && textQuery->IsTextElement (eh))
        return /*SnapAssocHelper::*/CreateText (assoc, snapPath, modifierMask, allowFarElm, parentModel);

    CurveVectorPtr pathCurve = ICurvePathQuery::ElementToCurveVector (ElementHandle (snapPath.GetCursorElem ()));

    if (pathCurve.IsNull ())
        return ERROR;

    ICurvePrimitivePtr& pathMember = pathCurve->front ();

    switch (pathCurve->HasSingleCurvePrimitive ())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            {
            DSegment3d  segment = *pathMember->GetLineCP ();

            return /*SnapAssocHelper::*/CreateProjection (assoc, segment.point, 2, true, snapPath, modifierMask, allowFarElm, parentModel);
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            {
            bvector<DPoint3d> const* points = pathMember->GetLineStringCP ();

            return /*SnapAssocHelper::*/CreateProjection (assoc, &points->front (), (int) points->size (), false, snapPath, modifierMask, allowFarElm, parentModel);
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
            {
            bvector<DPoint3d> const* points = pathMember->GetPointStringCP ();

            return /*SnapAssocHelper::*/CreateProjection (assoc, &points->front (), (int) points->size (), false, snapPath, modifierMask, allowFarElm, parentModel);
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            {
            DEllipse3d  ellipse = *pathMember->GetArcCP ();

            return /*SnapAssocHelper::*/CreateArc (assoc, ellipse, snapPath, modifierMask, allowFarElm, parentModel);
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
            {
            MSBsplineCurveCP curve = pathMember->GetProxyBsplineCurveCP ();
        
            return /*SnapAssocHelper::*/CreateBCurve (assoc, *curve, snapPath, modifierMask, allowFarElm, parentModel);
            }

        default:
            return ERROR;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus /*SnapAssocHelper::*/CreateFromIntersectionSnap
(
AssocPoint&     assoc,
SnapPathCR      snapPath,
int             modifierMask,
bool            allowFarElm,
DgnModelP    parentModel
)
    {
    HitPath*        otherPathP = NULL;
    IntersectPath*  iSnapPathP = (IntersectPath*) &snapPath;

    if (0 == (INTERSECT2_ASSOC & ~modifierMask))
        return ERROR;

    // Make sure we have a valid intersection path
    if (iSnapPathP->GetPathType () != DisplayPathType::Intersection ||
        NULL == (otherPathP = iSnapPathP->GetSecondPath ()))
        return ERROR;

    EditElementHandle eh1 (iSnapPathP->GetCursorElem ());
    EditElementHandle eh2 (otherPathP->GetCursorElem ());

    if (NULL == eh1.GetElementDescrP () || NULL == eh2.GetElementDescrP ())
        return ERROR;

    int     seg1 = 0, seg2 = 0;

    seg1 = (int) iSnapPathP->GetGeomDetail().GetSegmentNumber ();
    seg2 = (int) otherPathP->GetGeomDetail().GetSegmentNumber ();

    EditElementHandle  segEh1, segEh2;
    
    ElementUtil::GetSegment (segEh1, eh1, seg1);
    ElementUtil::GetSegment (segEh2, eh2, seg2);

    bvector<DPoint3d> isPnts1, isPnts2;

    if (SUCCESS != ElementUtil::GetIntersections (&isPnts1, &isPnts2, segEh1, segEh2, NULL,NULL, true))
        return ERROR;

    size_t  nIntersect = isPnts1.size ();

    if (0 == nIntersect)
        return ERROR;

    size_t      index = 0;
    size_t      nSeg1 = 0, nSeg2 = 0;
    double      distance;
    DPoint3d    hitPoint;

    assoc_getSnapPoint (hitPoint, iSnapPathP, parentModel, false);
    
    DPoint3dOps::ClosestPoint (isPnts1, hitPoint, index, distance);

    nSeg1 = LineStringUtil::GetApparentCount (eh1);
    nSeg2 = LineStringUtil::GetApparentCount (eh2);

    /* Make sure this is a real intersection (not apparent one) */
    if (!LegacyMath::RpntEqual (&isPnts1[index], &isPnts2[index]))
        return ERROR;

    AssociativePoint::InitIntersection (assoc, (byte)index, (UShort)seg1, (UShort)seg2, (int) nSeg1, (int) nSeg2);

    if (SUCCESS != /*SnapAssocHelper::*/SetAndValidateRoots (assoc, *iSnapPathP, parentModel, allowFarElm, 0, false))
        return ERROR;

    return /*SnapAssocHelper::*/SetAndValidateRoots (assoc, *otherPathP, parentModel, allowFarElm, 1, true);
    }

}; // SnapAssocHelper


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       AssociativePoint::CreateFromSnap
(
AssocPoint&     assoc,
SnapPathCP      pathIn,
int             modifierMask,               /* => allowed association type mask */
DgnModelP    parentModel,
CreateOptions   options
)
    {
    SnapPathP   snapPath = pathIn->Clone ();

    if (!snapPath->GetAllowAssociations ())
        return ERROR;

    // TR#213060
    // Usually we don't want to create associations to adjusted points.  Dimensions
    // allow these, since they always project the point to the dim's plane anyway.
    bool        allowAdjustedPoints = (0 != (options & AssociativePoint::CREATE_ASSOC_AllowAdjustedPoints));

    if (!allowAdjustedPoints && snapPath->PointWasAdjusted ())
        return ERROR;

    //  Give the ViewHandler a chance  to tweak the snap, e.g., to change the target element to a section geometry generator helper element
    snapPath->OnCreateAssociationToSnap (parentModel);

    // Can't make association to linestyle/pattern/thickness components...
    if (HIT_DETAIL_None != snapPath->GetGeomDetail().GetDetailType ())
        return ERROR;

    bool    allowFarElm = (0 == (options & AssociativePoint::CREATE_ASSOC_DisallowFarElm));
    bool    allowCustom = (0 == (options & AssociativePoint::CREATE_ASSOC_DisallowCustom));

    if (snapPath->GetCustomKeypoint (NULL, NULL) && allowCustom)
        return SnapAssocHelper::CreateCustom (assoc, *snapPath, modifierMask, allowFarElm, parentModel);

    switch (SnapContext::GetSnapKeypointType (snapPath->GetSnapMode ()))
        {
        case KEYPOINT_TYPE_Keypoint:
            return SnapAssocHelper::CreateFromKeypointSnap (assoc, *snapPath, modifierMask, allowFarElm, parentModel);

        case KEYPOINT_TYPE_Origin:
            return SnapAssocHelper::CreateFromOriginSnap (assoc, *snapPath, modifierMask, allowFarElm, parentModel);

        case KEYPOINT_TYPE_Center:
            return SnapAssocHelper::CreateFromCenterSnap (assoc, *snapPath, modifierMask, allowFarElm, parentModel);

        case KEYPOINT_TYPE_Midpoint:
            return SnapAssocHelper::CreateFromMidpointSnap (assoc, *snapPath, modifierMask, allowFarElm, parentModel);

        case KEYPOINT_TYPE_Bisector:
            return SnapAssocHelper::CreateFromBisectorSnap (assoc, *snapPath, modifierMask, allowFarElm, parentModel);

        case KEYPOINT_TYPE_Nearest:
        case KEYPOINT_TYPE_Tangent:
        case KEYPOINT_TYPE_Tangentpoint:
        case KEYPOINT_TYPE_Perpendicular:
        case KEYPOINT_TYPE_Perpendicularpt:
        case KEYPOINT_TYPE_Point:
        case KEYPOINT_TYPE_PointOn:
            return SnapAssocHelper::CreateFromNearestSnap (assoc, *snapPath, modifierMask, allowFarElm, parentModel);

        case KEYPOINT_TYPE_Intersection:
            return SnapAssocHelper::CreateFromIntersectionSnap (assoc, *snapPath, modifierMask, allowFarElm, parentModel);
        }

    return ERROR;
    }

//  ///////////////////////////////////////////////////////////////////////////////////
//  ///////////////////////////////////////////////////////////////////////////////////
//  ////////////////////////// PersistentSnapPathData /////////////////////////////////
//  ///////////////////////////////////////////////////////////////////////////////////
//  ///////////////////////////////////////////////////////////////////////////////////

/*=================================================================================**//**
*
* Fixed-format data stored at head of PersistentSnapPath state
    See "PersistentSnapPath storage format" below.
*
* @bsiclass                                                     Sam.Wilson      06/2005
+===============+===============+===============+===============+===============+======*/
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct          PersistentSnapPathHeader
{
    enum {FLAGS_HasTwoPaths=0x8000, FLAGS_HasPayload=0x4000, FLAGS_DoDeepCopy=0x2000, FLAGS_AssocTypeMask=0xf};

    UInt16      flags;

    PersistentSnapPathHeader() : flags(0) {;}

    PersistentSnapPathHeader(byte const* buf)
        {
        DataLoader source (buf, sizeof(UInt16));
        source.get (&flags);
        }

    StatusInt   Load (DataInternalizer& source)
        {
        source.get (&flags);
        return SUCCESS;
        }

    void        Store (DataExternalizer& sink) const
        {
        sink.put (flags);
        }

    bool        HasTwoPaths() const {return(flags &  FLAGS_HasTwoPaths) != 0;}
    void        SetHasTwoPaths()    {       flags |= FLAGS_HasTwoPaths;}

    bool        HasPayload() const {return(flags &   FLAGS_HasPayload) != 0;}
    void        ClrHasPayload()    {       flags &= ~FLAGS_HasPayload;}
    void        SetHasPayload()    {       flags |=  FLAGS_HasPayload;}

    bool        GetDoDeepCopy() const {return(flags &   FLAGS_DoDeepCopy) != 0;}
    void        ClrDoDeepCopy()       {       flags &= ~FLAGS_DoDeepCopy;}
    void        SetDoDeepCopy()       {       flags |=  FLAGS_DoDeepCopy;}

    UInt8       GetAssocType() const{return flags &   FLAGS_AssocTypeMask;}
    void        SetAssocType(UInt8 v) {     flags &= ~FLAGS_AssocTypeMask; flags |= v;}

    bool        HasNoData () const {return 0==(flags & ~FLAGS_DoDeepCopy);}

}; // PersistentSnapPathHeader

/*=================================================================================**//**
* Helper class to make it easier to access data stored in a PersistentSnapPath.
* @bsiclass                                                     Sam.Wilson      06/2005
+===============+===============+===============+===============+===============+======*/
struct CustomData
    {
    UInt32      count;
    byte const* data;

    CustomData () : count(0), data(NULL) {;}
    CustomData (byte const* d, UInt32 c) : data(d), count(c) {;}
    };

/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson      12/2007
+===============+===============+===============+===============+===============+======*/
struct          PersistentSnapPathBlobIO
{
    enum Subtype {SUBTYPE_Custom = 0, SUBTYPE_DPoint3d = 1};

    Subtype     m_blobType;
    // union:
    DPoint3d    m_pointData;
    DataLoader  m_custom;

    PersistentSnapPathBlobIO ()
        {
        m_blobType = SUBTYPE_Custom;
        }

    PersistentSnapPathBlobIO (DPoint3d const& pt)
        :
        m_pointData (pt)
        {
        m_blobType = SUBTYPE_DPoint3d;
        }

    PersistentSnapPathBlobIO (void const* data, size_t sz)
        :
        m_custom ((byte*)data, sz)
        {
        m_blobType = SUBTYPE_Custom;
        }

    void     Store (DataExternalizer& sink) const
        {
        sink.put ((UInt8)m_blobType);

        switch (m_blobType)
            {
            default:
                BeAssert (false);
                break;

            case SUBTYPE_Custom:
                sink.put ((UInt32)m_custom.getBytesRead());
                sink.put (m_custom.getBuf(), m_custom.getBytesRead());
                break;

            case SUBTYPE_DPoint3d:
                sink.put (&m_pointData.x, sizeof(m_pointData)/sizeof(m_pointData.x));
                break;
            }
        }

    StatusInt Load (DataLoader& source)
        {
        UInt8 v8;
        source.get (&v8);
        m_blobType = (Subtype) v8;
        switch (m_blobType)
            {
            case SUBTYPE_Custom:
                {
                UInt32 n;
                source.get (&n);
                m_custom.allocate (n);
                source.get (m_custom.getBufRW(), n);
                return SUCCESS;
                }
            case SUBTYPE_DPoint3d:
                source.get (&m_pointData.x, sizeof(m_pointData)/sizeof(m_pointData.x));
                return SUCCESS;
            }
        BeAssert (false);
        return ERROR;
        }

}; // PersistentSnapPathBlobIO

/*=================================================================================**//**
* Helper class to make it easier to access data stored in a PersistentSnapPath.
* @bsiclass                                                     Sam.Wilson      06/2005
+===============+===============+===============+===============+===============+======*/
struct DisplayPathHolder
    {
    DisplayPathPtr    dp;

    DisplayPathP    Get (PersistentElementPath const& pep, DgnModelP homeModel)
        {
        if (NULL == dp.get())
            dp = pep.GetDisplayPath (homeModel);
        return dp.get();
        }
    };

/*=================================================================================**//**
* This class has methods to make it convenient to deserialize a PersistentSnapPath and
* access the objects in it and to create a new PersistentSnapPath.
* This class understands how to parse the storage format of PersistentSnapPath.
* @bsiclass                                                     Sam.Wilson      06/2005
+===============+===============+===============+===============+===============+======*/
struct PersistentSnapPathData
    {
    //  The logical contents of a PersistentSnapPath
    PersistentSnapPathHeader h;
    AssocGeom               assoc;
    PersistentElementPath   pep1, pep2;
    CustomData              custom;
    PersistentSnapPathBlobIO blob;
    CustomData              payload;
    bool                    m_isInvalid;

    //  Transient items that are sometimes created from the objects in a PersistentSnapPath
    DisplayPathHolder       dp1, dp2;

    //  Member functions:

    //! Prepare to read stored PersistentSnapPath
    PersistentSnapPathData () : m_isInvalid(true) {;}
    PersistentSnapPathData (PersistentSnapPath const&);
    //! Prepare to write a new PersistentSnapPath
    PersistentSnapPathData (AssocGeom const& assocIn, DisplayPath const* path1In, DisplayPath const* path2In);
    PersistentSnapPathData (PersistentSnapPathBlobIO const&);
    PersistentSnapPathData (DisplayPath const*, CustomData const&);
    PersistentSnapPathData (DgnModelP homeModel, PersistentElementPath const&);

    StatusInt               CheckStreamSize (DataLoader const&);

    bool                    IsInvalid() const {return m_isInvalid;}

    StatusInt               Parse (DataLoader&);

    DisplayPathCP           GetDisplayPath1 (DgnModelP m) {return dp1.Get(pep1,m);}
    void                    GetDisplayPath1 (DisplayPathPtr& dp, DgnModelP m) {dp = dp1.Get(pep1,m);}
    DisplayPathCP           GetDisplayPath2 (DgnModelP m) {return dp2.Get(pep2,m);}
    void                    GetDisplayPath2 (DisplayPathPtr& dp, DgnModelP m) {dp = dp2.Get(pep1,m);}

    ElementHandle EvaluateElement1 (DgnModelP m) {return pep1.EvaluateElement(m);}
    ElementHandle EvaluateElement2 (DgnModelP m) {return pep2.EvaluateElement(m);}

    StatusInt               Write (::DataExternalizer&);
    StatusInt               WriteTo (PersistentSnapPath&);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentSnapPathData::PersistentSnapPathData (PersistentSnapPath const& psn)
    {
    m_isInvalid = true;
    DataLoader source (psn.m_state, psn.m_stateLen);
    Parse (source);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentSnapPathData::PersistentSnapPathData
(
AssocGeom const&      assocIn,
DisplayPath const*    path1In,
DisplayPath const*    path2In
)   :
    pep1 (path1In)
    {
    if (NULL != path2In)
        {
        h.SetHasTwoPaths();
        pep2 = PersistentElementPath (path2In);
        }

    memcpy (&assoc, &assocIn, sizeof (assoc));
    h.SetAssocType (assoc.type);

    m_isInvalid = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentSnapPathData::PersistentSnapPathData
(
DgnModelP                homeModel,
PersistentElementPath const& pepIn
)   :
    pep1 (pepIn)
    {
    h.SetAssocType (assoc.type = 0);                 // NB: there is no snap detail

    m_isInvalid = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentSnapPathData::PersistentSnapPathData (PersistentSnapPathBlobIO const& bio)
    :
    blob (bio)
    {
    h.SetAssocType (assoc.type = BLOB_ASSOC);
    // Note: both PEPs are nil
    m_isInvalid = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentSnapPathData::PersistentSnapPathData
(
DisplayPath const*  pathIn,
CustomData const&   customIn
)   :
    pep1 (pathIn),
    custom (customIn)
    {
    h.SetAssocType (assoc.type = CUSTOM_ASSOC);
    m_isInvalid = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PersistentSnapPathData::WriteTo (PersistentSnapPath& psn)
    {
    ::DataExternalizer sink;
    if (Write (sink) != SUCCESS)
        return ERROR;

    psn.GrabData (sink);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PersistentSnapPathData::CheckStreamSize (DataLoader const& source)
    {
    if (source.getBuf() == NULL || source.getSize() < sizeof(PersistentSnapPathHeader))
        {
        BeAssert (false && "corrupt PersistentSnapPathData");
        return ERROR;       // invalid
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
*   This is the master parsing function. It understands the storage format of
*   PersistentSnapPath data. Ideally, any function that wants to access
*   the data stored in a PersistentSnapPath should call this.
*   However, see VisitPaths and ModifyPaths for the two other ways that data is accessed.
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       PersistentSnapPathData::Parse (DataLoader& source)
    {
    m_isInvalid = true;

    if (source.getSize() == 0)
        {
        m_isInvalid = false;        // PSP is empty. That's not invalid.
        return SUCCESS;
        }

    if (CheckStreamSize (source) != SUCCESS)
        return ERROR;       // invalid

    StatusInt s;

    //  ---------------------------
    //  Header
    s = h.Load (source);
    if (SUCCESS != s)
        return s;

    //  ---------------------------
    //  Paths
    pep1.Load (source);

    if (h.HasTwoPaths())
        pep2.Load (source);

    //  ----------------------------
    //  Create AssocPoint w/ type-specific detail
    memset (&assoc, 0, sizeof(assoc));

    //      type
    assoc.type = h.GetAssocType();

    //      targets
#if defined (NOT_USEFUL_WITHOUT_FARPATH_ELEMENTS)
    DisplayPathPtr target1  = t1.GetDisplayPath (m_homeModel);
    ElementRefP target1Ref = target1->GetTailElem();
    ElementId   refAtt1   = getRefAttIdForModel (target1->GetRoot());

    if (!h.HasTwoPaths())
        {
        assoc->singleElm.uniqueId = target1Ref->GetElementId();
        assoc->singleElm.refAttachmentId = refAtt1;
        }
    else
        {
        assoc->twoElm.uniqueId1 = target1Ref->GetElementId();

        DisplayPathPtr target2    = t2.GetDisplayPath (m_homeModel);
        assoc->twoElm.uniqueId2 = target2->GetTailElem()->GetElementId();
        assoc->twoElm.refAttachmentId2  = getRefAttIdForModel (target2->GetRoot());
        }
#endif

    switch (assoc.type)
        {
        default:    BeAssert (false); return ERROR;

        case 0:         // no assocpoint is stored.
            break;

        case LINEAR_ASSOC:
            source.get (&assoc.line.divisor);
            source.get (&assoc.line.numerator);
            source.get (&assoc.line.nVertex);
            source.get (&assoc.line.vertex);
            break;

        case ARC_ASSOC:
            source.get (&assoc.arc.angle);
            source.get (&assoc.arc.keyPoint);
            break;

        case MLINE_ASSOC:
            source.get (&assoc.mline.nVertex);
            source.get (&assoc.mline.offsetVal);
            source.get (&assoc.mline.pointNo);
            break;

        case BCURVE_ASSOC:
            source.get (&assoc.bCurve.uParam);
            break;

        case BSURF_ASSOC:
            source.get (&assoc.bSurf.uParam);
            source.get (&assoc.bSurf.vParam);
            break;

        case PROJECTION_ASSOC:
            source.get (&assoc.projection.ratioVal);
            source.get (&assoc.projection.vertex);
            break;

        case ORIGIN_ASSOC:
            break;

        case MESH_VERTEX_ASSOC:
            source.get (&assoc.meshVertex.nVertex);
            source.get (&assoc.meshVertex.vertexIndex);
            break;

        case MESH_EDGE_ASSOC:
            source.get (&assoc.meshEdge.edgeIndex);
            source.get (&assoc.meshEdge.nEdge);
            source.get (&assoc.meshEdge.uParam);
            break;

        case INTERSECT_ASSOC:
            source.get (&assoc.intersect.index);
            break;

        case INTERSECT2_ASSOC:
            source.get (&assoc.intersect2.index);
            source.get (&assoc.intersect2.nSeg1);
            source.get (&assoc.intersect2.nSeg2);
            source.get (&assoc.intersect2.seg1);
            source.get (&assoc.intersect2.seg2);
            break;

        case CUSTOM_ASSOC:
        case BLOB_ASSOC:
            break;  // See below
        }

    //  ---------------------------
    //  Custom snap data
    if (h.GetAssocType () == CUSTOM_ASSOC)
        {
        source.get (&custom.count);
        custom.data = source.getPos ();
        source.skip (custom.count);
        }
    else
    if (h.GetAssocType() == BLOB_ASSOC)
        {
        if (blob.Load (source) != SUCCESS)
            return ERROR;
        }

    //  ---------------------------
    //  Object states
    if (h.HasPayload ())
        {
        source.get (&payload.count);
        payload.data = source.getPos ();
        source.skip (payload.count);
        }

    m_isInvalid = false;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       PersistentSnapPathData::Write (::DataExternalizer& sink)
    {
    //  ***
    //  *** Keep this consistent with PersistentSnapPath::Parse
    //  ***

    //  ----------------------------------------------------------
    //  Make sure header flags are set up to reflect the data that I have.
    h.SetAssocType (assoc.type);

    if (payload.data != NULL && payload.count != 0)
        h.SetHasPayload();
    else
        h.ClrHasPayload();

    BeAssert (!h.HasTwoPaths() || !pep2.IsEmpty());

    //  ----------------------------------------------------------
    //  Don't write anything if I have no data
    if (pep1.IsEmpty() && h.HasNoData())
        {
        BeAssert (pep2.IsEmpty());
        return SUCCESS;
        }

    if (IsInvalid ())
        {
        BeAssert (false);
        return ERROR;
        }

    //  -----------------------------------------------
    //  Store fixed-format header.  (See "PersistentSnapPath storage format")
    h.Store (sink);

    //  -----------------------------------------------
    //  Store path(s)
    pep1.Store (&sink);

    if (h.HasTwoPaths())
        pep2.Store (&sink);

    //  -----------------------------------------------
    //  Store snap-specific detail
    switch (assoc.type)
        {
        default:    BeAssert (false); return ERROR;

        case 0:         // no assocpoint is stored.
            break;

        case LINEAR_ASSOC:
            sink.put (assoc.line.divisor);
            sink.put (assoc.line.numerator);
            sink.put (assoc.line.nVertex);
            sink.put (assoc.line.vertex);
            break;

        case ARC_ASSOC:
            sink.put (assoc.arc.angle);
            sink.put (assoc.arc.keyPoint);
            break;

        case MLINE_ASSOC:
            sink.put (assoc.mline.nVertex);
            sink.put (assoc.mline.offsetVal);
            sink.put (assoc.mline.pointNo);
            break;

        case BCURVE_ASSOC:
            sink.put (assoc.bCurve.uParam);
            break;

        case BSURF_ASSOC:
            sink.put (assoc.bSurf.uParam);
            sink.put (assoc.bSurf.vParam);
            break;

        case PROJECTION_ASSOC:
            sink.put (assoc.projection.ratioVal);
            sink.put (assoc.projection.vertex);
            break;

        case ORIGIN_ASSOC:
            break;

        case MESH_VERTEX_ASSOC:
            sink.put (assoc.meshVertex.nVertex);
            sink.put (assoc.meshVertex.vertexIndex);
            break;

        case MESH_EDGE_ASSOC:
            sink.put (assoc.meshEdge.edgeIndex);
            sink.put (assoc.meshEdge.nEdge);
            sink.put (assoc.meshEdge.uParam);
            break;

        case INTERSECT_ASSOC:
            sink.put (assoc.intersect.index);
            break;

        case INTERSECT2_ASSOC:
            sink.put (assoc.intersect2.index);
            sink.put (assoc.intersect2.nSeg1);
            sink.put (assoc.intersect2.nSeg2);
            sink.put (assoc.intersect2.seg1);
            sink.put (assoc.intersect2.seg2);
            break;

        case CUSTOM_ASSOC:
        case BLOB_ASSOC:
            break;  // See below
        }

    //  ---------------------------
    //  Custom snap data
    if (h.GetAssocType () == CUSTOM_ASSOC)
        {
        sink.put (custom.count);
        sink.put (custom.data, custom.count);
        }
    else
    if (h.GetAssocType() == BLOB_ASSOC)
        {
        blob.Store (sink);
        }

    //  -----------------------------------------------
    //  Store object state IDs
    if (h.HasPayload ())
        {
        sink.put (payload.count);
        sink.put (payload.data, payload.count);
        }

    return SUCCESS;
    }

END_BENTLEY_DGNPLATFORM_NAMESPACE

//  ///////////////////////////////////////////////////////////////////////////////////
//  ///////////////////////////////////////////////////////////////////////////////////
//  ////////////////////////// PersistentSnapPath /////////////////////////////////////
//  ///////////////////////////////////////////////////////////////////////////////////
//  ///////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
void            PersistentSnapPath::FreeMemory ()
    {
    if (NULL != m_state)
        {
        free (m_state);
        m_state = NULL;
        }
    m_stateLen = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
void            PersistentSnapPath::Clear ()
    {
    FreeMemory ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
void            PersistentSnapPath::Copy
(
PersistentSnapPath const& cc
)
    {
    m_homeModel = cc.m_homeModel;
    m_stateLen = cc.m_stateLen;
    m_state = (byte*) malloc (m_stateLen);
    memcpy (m_state, cc.m_state, m_stateLen);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
void            PersistentSnapPath::GrabData
(
::DataExternalizer const& sink
)
    {
    FreeMemory ();

    m_stateLen = (UInt16) sink.getBytesWritten();
    if (0 == m_stateLen)
        {
        m_state = NULL;
        return;
        }
    m_state = (byte*) malloc (m_stateLen);
    memcpy (m_state, sink.getBuf(), m_stateLen);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentSnapPath::~PersistentSnapPath ()
    {
    FreeMemory();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentSnapPath::PersistentSnapPath
(
PersistentSnapPath const& cc
)
    {
    Copy (cc);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentSnapPath& PersistentSnapPath::operator =
(
PersistentSnapPath const& cc
)
    {
    if (&cc == this)
        return *this;

    FreeMemory ();
    Copy (cc);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentSnapPath::PersistentSnapPath (DgnModelP m)
    {
    Init (m);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PersistentSnapPath::IsValid () const
    {
    return m_state != NULL && m_stateLen > sizeof(PersistentSnapPathHeader);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
void            PersistentSnapPath::Store
(
::DataExternalizer&     sink
)   const
    {
    sink.put (m_stateLen);
    sink.put (m_state, m_stateLen);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       PersistentSnapPath::Load
(
::DataInternalizer&      source
)
    {
    source.get (&m_stateLen);
    if (0 == m_stateLen)
        return SUCCESS;

    m_state = (byte*) malloc (m_stateLen);
    source.get (m_state, m_stateLen);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PersistentSnapPath::HasTwoPaths () const
    {
    //  ****
    //  ****    This function doesn't call PersistentSnapPathData::Parse because ...
    //  ****        It seems so simple to get the answer directly from the header.
    //  ****
    DataLoader source (m_state, m_stateLen);

    PersistentSnapPathHeader h;
    if (h.Load (source) != SUCCESS)
/*<*/   return false;

    return h.HasTwoPaths();
    }

#ifdef DGN_IMPORTER_REORG_WIP
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       PersistentSnapPath::OnPreprocessCopy
(
ElementHandle const&   hostElement,
CopyContextP        cc,
PersistentElementPath::CopyOption opt
)
    {
#if defined (WIP_V10_PEP_REMAPPING)
    PersistentSnapPathData spdata (*this);

    spdata.pep1.OnPreprocessCopy (hostElement, cc, opt);
    spdata.pep2.OnPreprocessCopy (hostElement, cc, opt);

    return spdata.WriteTo (*this);
#endif
return SUCCESS;
    }
#endif


// -----------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////// Evaluate/Query PersistentSnapPath //////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
// -----------------------------------------------------------------------------------------
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
WString PersistentSnapPath::GetDescription () const
    {
    PersistentSnapPathData pd (*this);
    if (pd.IsInvalid ())
        {
        BeAssert (false);
        return L"?";
        }

    DgnModelP m (GetHomeDgnModelP());

    WString desc = AssociativePoint::FormatAssocPointString ((AssocPoint&) pd.assoc);

    if (pd.GetDisplayPath1(m))
        {
        WString buf;
        pd.GetDisplayPath1(m)->GetInfoString (buf, DESCRDELIMIT_StdSingleLine);
        desc += L": " + buf;
        }

    if (pd.GetDisplayPath2(m))
        {
        WString buf;
        pd.GetDisplayPath2(m)->GetInfoString (buf, DESCRDELIMIT_StdSingleLine);
        desc += L", " + buf;
        }

    if (pd.assoc.type == CUSTOM_ASSOC)
        {
        WChar buf[32];
        BeStringUtilities::Snwprintf (buf, L"%d", pd.custom.count);
        desc += L" Custom(";
        desc += buf;
        desc += L")";
        }

    if (pd.h.HasPayload())
        {
        WChar buf[32];
        BeStringUtilities::Snwprintf (buf, L"%d", pd.custom.count);
        desc += L" Payload(";
        desc += buf;
        desc += L")";
        }

    return desc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       PersistentSnapPath::GetPaths
(
DisplayPathPtr&   path1,
DisplayPathPtr&   path2,
AssocPoint*     assoc
)   const
    {
    PersistentSnapPathData pd (*this);
    if (pd.IsInvalid ())
        return ERROR;

    DgnModelP m (GetHomeDgnModelP());

    pd.GetDisplayPath1 (path1, m);
    pd.GetDisplayPath2 (path2, m);

    if (NULL != assoc)
        memcpy (assoc, &pd.assoc, sizeof *assoc);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       PersistentSnapPath::EvaluatePoint
(
DPoint3d&       pt
)   const
    {
    PersistentSnapPathData pd (*this);
    if (pd.IsInvalid ())
        return ERROR;

    DgnModelP m (GetHomeDgnModelP());

    if (pd.h.GetAssocType() == BLOB_ASSOC)
        {
        if (pd.blob.m_blobType != PersistentSnapPathBlobIO::SUBTYPE_DPoint3d)
            return ERROR;
        pt = pd.blob.m_pointData;
        return SUCCESS;
        }

    DisplayPathCP dp1 = pd.GetDisplayPath1(m);
    if (dp1 == NULL)
        return ERROR;

    ElementHandle eh (dp1->GetTailElem());
    DisplayHandlerP dispHandler = eh.GetDisplayHandler();

    if (NULL == dispHandler) // See Notes
        return ERROR;

    return dispHandler->EvaluateSnap (eh, pt, dp1, pd.GetDisplayPath2(m), (AssocPoint&)pd.assoc, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      03/07
+---------------+---------------+---------------+---------------+---------------+------*/
// static
StatusInt       PersistentSnapPath::EvaluateAssocPoint
(
DPoint3d&           outPoint,
AssocPoint const&   assocPoint,
DgnModelP        homeModel
)
    {
    int             nRoots;
    DisplayPath     path;

    if (SUCCESS != AssociativePoint::GetRoot (&path, &nRoots, assocPoint, homeModel, 0))
        return ERROR;

    ElementHandle   eh (path.GetTailElem ());
    DisplayHandlerP dispHandler = eh.GetDisplayHandler();

    if (NULL == dispHandler) // See Notes
        return ERROR;

    AssocGeom&  assoc ((AssocGeom&) assocPoint);

    PersistentSnapPath psn (homeModel);

    if (2 == nRoots)
        {
        DisplayPath     path2;

        if (SUCCESS != AssociativePoint::GetRoot (&path2, NULL, assocPoint, homeModel, 1))
            return ERROR;

        psn.FromAssocPointAndPaths (assoc, &path, &path2);

        return dispHandler->EvaluateSnap (eh, outPoint, &path, &path2, assocPoint, psn);
        }

    if (CUSTOM_ASSOC == assoc.type)
        psn.FromCustom (&path, assoc);
    else
        psn.FromAssocPointAndPaths (assoc, &path, NULL);

    return dispHandler->EvaluateSnap (eh, outPoint, &path, NULL, assocPoint, psn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      05/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PersistentSnapPath::DependsOnElementRef
(
ElementRefP      target
)   const
    {
    DisplayPathPtr p1;
    DisplayPathPtr p2;
    if (GetPaths (p1, p2) != SUCCESS)
/*<*/   return false;

    return p1 != NULL && p1->Contains(target) || p2 != NULL && p2->Contains(target);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/03
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandle  PersistentSnapPath::EvaluateElement (bool wantFirst) const
    {
    PersistentSnapPathData pd (*this);
    if (pd.IsInvalid ())
        return ElementHandle();

    DgnModelP m (GetHomeDgnModelP());

    if (wantFirst)
        return pd.EvaluateElement1 (m);

    if (!pd.h.HasTwoPaths ())
        return ElementHandle();

    return pd.EvaluateElement2 (m);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PersistentSnapPath::IsTargetAvailable () const
    {
    PersistentSnapPathData pd (*this);
    if (pd.IsInvalid ())
        return false;

    DgnModelP m (GetHomeDgnModelP());

    if (!pd.EvaluateElement1(m).IsValid ())
        return false;

    if (!pd.h.HasTwoPaths ())
        return true;

    return pd.EvaluateElement2(m).IsValid ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       PersistentSnapPath::GetCustomKeypointData
(
byte const*&    data,
UInt32&         nbytes
)   const
    {
    PersistentSnapPathData pd (*this);
    if (pd.IsInvalid ())
        {
        data = NULL;
        nbytes = 0;
        return ERROR;
        }

    data = pd.custom.data;
    nbytes = pd.custom.count;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void        PersistentSnapPath::SetCustomKeypointData (byte const* data, UInt32 nbytes)
    {
    PersistentSnapPathData pd (*this);
    pd.h.SetAssocType (pd.assoc.type = CUSTOM_ASSOC);
    pd.custom = CustomData (data, nbytes);
    pd.WriteTo (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       PersistentSnapPath::GetPayloadData
(
void const*&    data,
UInt32&         nbytes
)   const
    {
    PersistentSnapPathData pd (*this);
    if (pd.IsInvalid () || !pd.h.HasPayload())
        {
        data = NULL;
        nbytes = 0;
        return ERROR;
        }

    data = pd.payload.data;
    nbytes = pd.payload.count;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void        PersistentSnapPath::SetPayloadData (void const* data, UInt32 nbytes)
    {
    PersistentSnapPathData pd (*this);
    pd.payload = CustomData ((byte*)data, nbytes);
    pd.WriteTo (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2007
+---------------+---------------+---------------+---------------+---------------+------*/
bool PersistentSnapPath::IsStaticDPoint3d () const
    {
    PersistentSnapPathData pd (*this);
    if (pd.IsInvalid ())
        return false;

    return pd.blob.m_blobType == PersistentSnapPathBlobIO::SUBTYPE_DPoint3d;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PersistentSnapPath::GetDoDeepCopy () const
    {
    PersistentSnapPathData pd (*this);
    return pd.h.GetDoDeepCopy ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void        PersistentSnapPath::SetDoDeepCopy (bool doDeepCopy)
    {
    PersistentSnapPathData pd (*this);
    if (doDeepCopy)
        pd.h.SetDoDeepCopy ();
    else
        pd.h.ClrDoDeepCopy ();
    pd.WriteTo (*this);
    }

// -----------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////// Construct PersistentSnapPath ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
// -----------------------------------------------------------------------------------------
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void    PersistentSnapPath::Init (DgnModelP m)
    {
    m_stateLen = 0;
    m_state = NULL;
    m_homeModel = m;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       PersistentSnapPath::FromAssocPointAndPaths
(
AssocGeom const&  assoc,
DisplayPath const*    path1,
DisplayPath const*    path2
)
    {
    BeAssert (assoc.type != CUSTOM_ASSOC);
    PersistentSnapPathData pdata (assoc, path1, path2);
    return pdata.WriteTo (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       PersistentSnapPath::FromBlob (PersistentSnapPathBlobIO const& bio)
    {
    PersistentSnapPathData pdata (bio);
    return pdata.WriteTo (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       PersistentSnapPath::FromCustom
(
DisplayPath const*  path,
byte const*     custom,
UInt32          customSize
)
    {
    PersistentSnapPathData pdata (path, CustomData (custom, customSize));
    return pdata.WriteTo (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       PersistentSnapPath::FromCustom
(
DisplayPath const*  path,
AssocGeom const&    assoc
)
    {
    byte*       data;
    UInt32      size;
    ElementHandle  pathEh (path->GetCursorElem());

    if (SUCCESS != AssociativePoint::CustomGetDataFromPathElement (data, size, pathEh, (AssocPoint const&) assoc))
        return ERROR;

    return FromCustom (path, data, size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       PersistentSnapPath::FromPersistentElementPath
(
DgnModelP                homeModel,
PersistentElementPath const& pep
)
    {
    PersistentSnapPathData pdata (homeModel, pep);
    return pdata.WriteTo (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       PersistentSnapPath::FromPersistentElementPath
(
PersistentElementPath const& pep
)
    {
    return FromPersistentElementPath (NULL, pep);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       PersistentSnapPath::FromAssocPoint
(
AssocPoint const&  assoc
)
    {
    StatusInt       s;
    ElementRefP      ref[2] = {NULL,NULL};
    DgnModelP    model[2] = {NULL,NULL};
    int             nRoots = 0;

    if (SUCCESS != (s = AssociativePoint::GetRoot (&ref[0], &model[0], NULL, &nRoots, const_cast<AssocPoint&>(assoc), m_homeModel, 0)))
        return s;

    if (NULL == ref[0] || nRoots == 0)
        return ERROR;

    DisplayPath dp1 (ref[0], model[0]);

    if (nRoots==1)
        return FromAssocPointAndPaths ((AssocGeom&)assoc, &dp1, NULL);

    s = AssociativePoint::GetRoot (&ref[1], &model[1], NULL, &nRoots, const_cast<AssocPoint&>(assoc), m_homeModel, 1);
    if (SUCCESS != s || NULL == ref[1])
        return ERROR;

    DisplayPath dp2 (ref[1], model[1]);
    return FromAssocPointAndPaths ((AssocGeom&)assoc, &dp1, &dp2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentSnapPath::PersistentSnapPath () {Init(NULL);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentSnapPath::PersistentSnapPath
(
DgnModelP        home,
AssocPoint const&   assoc
)
    {
    Init (home);
    FromAssocPoint (assoc);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2007
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentSnapPath::PersistentSnapPath (DPoint3d const& dp)
    {
    Init (NULL);
    FromBlob (PersistentSnapPathBlobIO (dp));
    }

#ifdef UNUSED_FUNCTION
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/2004
+---------------+---------------+---------------+---------------+---------------+------*/
static ElementId getRefAttIdForModel
(
DgnModelP    model
)
    {
    DgnAttachmentP refP;
    if (NULL ==  model || NULL == (refP = mdlRefFile_getInfo (model)))
        return 0;         // If this is not a reference attachment, then there's nothing to push

    return refP->GetElementId();
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentSnapPath::PersistentSnapPath
(
DgnModelP                 home,
PersistentElementPath const& pep
)
    {
    Init (home);
    FromPersistentElementPath (home, pep);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       PersistentSnapPath::FromSnapPath (SnapPathCP snap)
    {
    if (snap->GetPathType() != DisplayPathType::Snap && snap->GetPathType() != DisplayPathType::Intersection)
        return ERROR;

    if (snap->GetCustomKeypoint (NULL, NULL))
        {
        //  ----------------------------------------
        //  Custom snap
        //  ----------------------------------------

        // Brien wants to remove CAPABILITY_CUSTOM_KEYPOINTS for Vancouver.

        int     nBytes = 0;
        byte*   data = NULL;

        snap->GetCustomKeypoint (&nBytes, &data);

        return FromCustom (snap, data, nBytes);
        }

    //  ----------------------------------------
    //  Regular snap
    //  ----------------------------------------

    AssocGeom   a;
    int         modifierMask = 0;

    if (SUCCESS != AssociativePoint::CreateFromSnap ((AssocPoint&) a, snap, modifierMask, m_homeModel, AssociativePoint::CREATE_ASSOC_DisallowFarElm))
        return ERROR;

    if (DisplayPathType::Intersection == snap->GetPathType ())
        return FromAssocPointAndPaths (a, snap, ((IntersectPath*) snap)->GetSecondPath ());

    return FromAssocPointAndPaths (a, snap, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentSnapPath::PersistentSnapPath
(
DgnModelP    homeModel,
SnapPath const* snap
)
    {
    Init (homeModel);
    FromSnapPath (snap);
    }

/*
    PersistentSnapPath storage format:

    PersistentSnapPathHeader  (UInt16)
    PersistentElementPath     (variable size)
    PersistentElementPath     (variable size)       optional: if hdr.HasTwoPaths() == true
    custom snap data          (UInt32 + nbytes...)  optional: if hdr.GetAssocType() == CUSTOM_ASSOC
    payload                   (UInt32 + nbytes...)  optional: if hdr.HasPayload() == true

    parsePersistentSnapPath reads the stored format and fills out a PersistentSnapPathData structure.
*/

/* Notes
 It's possible to create a snappath to a non-displayable. You can evaluate the element ref(s),
 but you can't evaluate the point. Relationships use SnapPath for all pointers, including pointers to non-displayables.
*/
