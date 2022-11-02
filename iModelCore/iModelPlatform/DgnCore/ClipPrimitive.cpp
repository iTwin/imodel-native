/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <Vu/VuApi.h>

#define TINY_VALUE                      1.0E-8
#define HUGE_VALUE                      1.0E14
#define POLYFILL_EXTERIOR_EDGE          0x01

// EDL May 6 2018 replace m_gpa by m_curves.
// Delete constructors with gpa.
// Delete GetBCurve -- caller can GetCurves and manipulate as they wish.
// GetCurvesCP always returns the pointer.   old param "only if nonlinear" is gone --
//    caller can do that test easily enough (curves->ContainsNonLinearPrimitive())
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPolygon::ClipPolygon(DPoint2dCP points, size_t nPoints) {Init(points, nPoints);}

void ClipPolygon::Init(DPoint2dCP points, size_t nPoints)
    {
    resize(nPoints);
    memcpy(&front(), points, nPoints * sizeof(DPoint2d));
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ClipPlanesPrimitive : ClipPrimitive
{
    mutable ClipPlaneSetP       m_clipPlanes;
    mutable ClipPlaneSetP       m_maskPlanes;
    uint32_t                    m_flags;
    bool                        m_isMask;

    enum
        {
        Mask_Invisible  = 0x0001,
        Mask_Default = 0
        };

    ClipPlanesPrimitive (bool invisible) { m_clipPlanes = m_maskPlanes = NULL; m_flags = invisible ? Mask_Invisible : 0; m_isMask = false;}
    ClipPlanesPrimitive (ClipPlaneSetCR planeSet, bool invisible) : m_clipPlanes (new ClipPlaneSet (planeSet)), m_maskPlanes (NULL), m_isMask (false) { m_flags = invisible ? Mask_Invisible : 0; }

    ClipPlaneSetCP _GetClipPlanes() const override {return m_clipPlanes;}
    ClipPlaneSetCP _GetMaskPlanes() const override {return m_maskPlanes;}
    ClipPrimitive* _Clone() const override {return new ClipPlanesPrimitive(*this);}
    bool _GetInvisible() const override {return 0 != (m_flags & Mask_Invisible);}

    virtual bool _IsMask () const override { return m_isMask; }
    virtual void _SetIsMask (bool isMask) override
        {
        DELETE_AND_CLEAR(m_maskPlanes);

        m_isMask = isMask;
        m_maskPlanes = isMask ? new ClipPlaneSet(*m_clipPlanes) : NULL;
        }

    void ToJsonPlane(BeJsValue, ClipPlaneCR) const;
    void ToJsonConvexPlaneSet(BeJsValue, ConvexClipPlaneSetCR) const;
    void ToJsonPlaneSet(BeJsValue, ClipPlaneSetCR) const;
    static ClipPlane FromJsonPlane(BeJsConst );
    static ConvexClipPlaneSet FromJsonConvexPlaneSet(BeJsConst);
    static ClipPlaneSetP FromJsonPlaneSet(BeJsConst);

    static ClipPrimitivePtr FromJson(BeJsConst val);
    void _ToJson(BeJsValue) const override;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPlanesPrimitive(ClipPlanesPrimitive const& donor)
    {
    m_clipPlanes = NULL == donor.m_clipPlanes ? NULL : new ClipPlaneSet(*donor.m_clipPlanes);
    m_maskPlanes = NULL == donor.m_maskPlanes ? NULL : new ClipPlaneSet(*donor.m_maskPlanes);
    m_flags      = donor.m_flags;
    m_isMask     = donor.m_isMask;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
~ClipPlanesPrimitive()
    {
   DELETE_AND_CLEAR (m_clipPlanes);
   DELETE_AND_CLEAR (m_maskPlanes);
   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus _TransformInPlace(TransformCR transform) override
    {
    if (NULL != m_clipPlanes)
        m_clipPlanes->TransformInPlace(transform);

    if (NULL != m_maskPlanes)
        m_maskPlanes->TransformInPlace(transform);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus _MultiplyPlanesTimesMatrix(DMatrix4dCR matrix) override
    {
    if (NULL != m_clipPlanes)
        m_clipPlanes->MultiplyPlanesTimesMatrix(matrix);

    if (NULL != m_maskPlanes)
        m_maskPlanes->MultiplyPlanesTimesMatrix(matrix);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool _GetRange(DRange3dR range, TransformCP pTransform, bool returnMaskRange) const override
    {
    return (NULL == m_clipPlanes) ? false : m_clipPlanes->GetRange(range, pTransform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void setPlaneInvisible(ClipPlaneSetCP planeSet, bool invisible)
    {
    if (NULL == planeSet)
        return;

    for (ConvexClipPlaneSet& convexSet: const_cast <ClipPlaneSetR> (*planeSet))
        {
        for (ClipPlane& plane: convexSet)
            plane.SetInvisible(invisible);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void _SetInvisible(bool invisible) override
    {
    if (invisible)
        m_flags |= Mask_Invisible;
    else
        m_flags &= ~Mask_Invisible;

    setPlaneInvisible(_GetClipPlanes(), invisible);
    setPlaneInvisible(_GetMaskPlanes(), invisible);
    }
};   // ClipPlanesPrimitive

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipPlanesPrimitive::ToJsonPlane(BeJsValue val, ClipPlaneCR clipPlane) const
    {
    BeJsGeomUtils::DVec3dToJson(val["normal"], clipPlane.m_normal);
    val["dist"] = clipPlane.m_distance;
    if (clipPlane.GetIsInterior())
        val["interior"] = true;
    if (clipPlane.GetIsInvisible())
        val["invisible"] = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPlane ClipPlanesPrimitive::FromJsonPlane(BeJsConst val)
    {
    ClipPlane clipPlane;
    BeJsGeomUtils::DVec3dFromJson(clipPlane.m_normal, val["normal"]);
    clipPlane.m_distance = val["dist"].asDouble();
    clipPlane.SetFlags(val.isMember("invisible"), val.isMember("interior"));
    return clipPlane;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipPlanesPrimitive::ToJsonConvexPlaneSet(BeJsValue val, ConvexClipPlaneSetCR convexSet) const
    {
    for (auto& plane : convexSet)
        ToJsonPlane(val.appendValue(), plane);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ConvexClipPlaneSet ClipPlanesPrimitive::FromJsonConvexPlaneSet(BeJsConst val)
    {
    ConvexClipPlaneSet set;
    if (val.isArray())
        {
        for (Json::ArrayIndex i=0; i<val.size(); ++i)
            set.push_back(FromJsonPlane(val[(int)i]));
        }

    return set;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipPlanesPrimitive::ToJsonPlaneSet(BeJsValue val, ClipPlaneSetCR planeSet) const
    {
    for (auto& convexSet : planeSet)
        ToJsonConvexPlaneSet(val.appendValue(), convexSet);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPlaneSetP ClipPlanesPrimitive::FromJsonPlaneSet(BeJsConst val)
    {
    ClipPlaneSetP clipPlaneSet = new ClipPlaneSet();
    for (Json::ArrayIndex j = 0; j < val.size(); ++j)
        clipPlaneSet->push_back(FromJsonConvexPlaneSet(val[j]));

    return clipPlaneSet;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipPlanesPrimitive::_ToJson(BeJsValue out) const
    {
    auto planes = out["planes"];

    if (m_clipPlanes)
        ToJsonPlaneSet(planes["clips"], *m_clipPlanes);

    if (_GetInvisible())
        planes["invisible"] = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPrimitivePtr ClipPlanesPrimitive::FromJson(BeJsConst val)
    {
    return new ClipPlanesPrimitive(*FromJsonPlaneSet(val["clips"]), val["invisible"].asBool());
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct   AddPlaneSetParams
    {
    ClipPlaneSetR   m_planeSets;
    double const*   m_zLow;
    double const*   m_zHigh;
    bool            m_outside;
    bool            m_invisible;
    double          m_limitValue;
    DPoint3d        m_localOrigin;
    double*         m_focalLength;

    AddPlaneSetParams(ClipPlaneSetR inPlaneSet, double const* zLow, double const* zHigh, bool outside, bool invisible, double* focalLength) :
                       m_planeSets(inPlaneSet), m_zLow(zLow), m_zHigh(zHigh), m_outside(outside), m_invisible(invisible), m_focalLength(focalLength) {}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void addOutsideZClipSets(ClipPlaneSetR clipPlaneSet,  double const* zLow, double const* zHigh, bool invisible)
    {
    if (NULL != zLow)
        {
        ConvexClipPlaneSet        convexPlaneSet;

        convexPlaneSet.push_back(ClipPlane(DVec3d::From(0.0, 0.0, -1.0), - *zLow, invisible));
        clipPlaneSet.push_back(convexPlaneSet);
        }
    if (NULL != zHigh)
        {
        ConvexClipPlaneSet        convexPlaneSet;

        convexPlaneSet.push_back(ClipPlane(DVec3d::From(0.0, 0.0, 1.0), *zHigh, invisible));
        clipPlaneSet.push_back(convexPlaneSet);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void addZClipPlanes(ConvexClipPlaneSetR planeSet, double const* zLow, double const* zHigh, bool invisible)
    {
    if (NULL != zLow)
        planeSet.push_back(ClipPlane(DVec3d::From(0.0, 0.0, 1.0), *zLow, invisible));

    if (NULL != zHigh)
        planeSet.push_back(ClipPlane(DVec3d::From(0.0, 0.0, -1.0), - *zHigh, invisible));
    }

/*---------------------------------------------------------------------------------**//**
Add an unbounded plane set (a) to the right of the line defined by two points, and (b) "ahead" of
   the start point.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void addPlaneSetOutsideEdge(double x0, double y0, double x1, double y1, AddPlaneSetParams *pParams, bool invisible)
    {
    DVec3d          unit0, unit1;
    DVec3d          vec0;
    DPoint3d        point0;

    ((DVec3dP)&point0)->Init( x0 + pParams->m_localOrigin.x, y0 + pParams->m_localOrigin.y, 0.0);
    vec0.Init( x1 - x0, y1 - y0, 0.0);
    unit0.Normalize(vec0);
    unit1.Init( unit0.y, -unit0.x, 0.0);

    ConvexClipPlaneSet      convexSet;

    convexSet.push_back(ClipPlane(unit1, point0, invisible));
    convexSet.push_back(ClipPlane(unit0, point0, invisible));

    addZClipPlanes(convexSet, pParams->m_zLow, pParams->m_zHigh, invisible);

    pParams->m_planeSets.push_back(convexSet);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isLimitEdge(double limitValue, DPoint3dR point0, DPoint3dR point1)
    {
    double tol = 0.000001 * limitValue;
    // High x limit...
    if (   fabs(point0.x - limitValue) < tol
        && fabs(point1.x - limitValue) < tol)
        return true;
    // low x limit ...
    if (   fabs(point0.x + limitValue) < tol
        && fabs(point1.x + limitValue) < tol)
        return true;

    // high y limit ...
    if (   fabs(point0.y - limitValue) < tol
        && fabs(point1.y - limitValue) < tol)
        return true;
    // low y limit ...
    if (   fabs(point0.y + limitValue) < tol
        && fabs(point1.y + limitValue) < tol)
        return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void addPlaneSet(bvector<DPoint3d> const &points, bvector<bool> const &isBoundary, AddPlaneSetParams &params)
    {
    size_t nPoints = points.size ();
    double                  area = bsiGeom_getXYPolygonArea(points.data (), static_cast<uint32_t>(nPoints));
    DPoint3d                point0, point1, point0Local, point1Local;
    DVec3d                  zVector = DVec3d::From(0.0, 0.0, 1.0);
    DVec3d                  normal, tangent;
    DPoint3d                closurePoint;
    ConvexClipPlaneSet      convexSet;
    bool                    reverse = (area < 0.0);

    for (size_t i = 0;  i < points.size (); i++, point0 = point1, point0Local = point1Local)
        {
        point1Local = points[i % nPoints];
        point1.SumOf(point1Local, params.m_localOrigin);
        if (i == 0)
            closurePoint = point1;

        if (i && !point1.IsEqual(point0, TINY_VALUE))
            {
            bool bIsLimitPlane = isLimitEdge(params.m_limitValue, point0Local, point1Local);
            bool isInterior    =  (!isBoundary[i]) || bIsLimitPlane;

            if (NULL == params.m_focalLength)
                {
                tangent.DifferenceOf(point1, point0);
                normal.NormalizedCrossProduct(zVector, tangent);

                if (reverse)
                    normal.Negate();

                convexSet.push_back(ClipPlane(normal, point0, params.m_invisible, isInterior));
                }
            else
                {
                normal.NormalizedCrossProduct((DVec3dCR) point1, (DVec3dCR) point0);
                if (reverse)
                    normal.Negate();

                convexSet.push_back(ClipPlane(normal, 0.0, params.m_invisible, isInterior));
                }
            }
        }

    addZClipPlanes(convexSet, params.m_zLow, params.m_zHigh, params.m_invisible);        // Note - we add the clip limits even for outside (TR# 236791) to avoid overlapping regions.

    if (!convexSet.empty())
        params.m_planeSets.push_back(convexSet);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void parseConvexPolygonPlanes(ClipPlaneSetR planeSet, DPoint2dCP pVertices, size_t numVerts, double const* zLow, double const* zHigh, bool outside, int directionSign, bool invisible, double* cameraFocalLength)
    {
    struct      PolyEdge
        {
        DPoint3d    m_origin;
        DPoint3d    m_next;
        DVec2d      m_normal;

        PolyEdge(DPoint2dCR origin, DPoint2dCR next, DVec2dCR normal, double z) : m_origin(DPoint3d::From(origin.x, origin.y, z)), m_next(DPoint3d::From(next.x, next.y, z)), m_normal(normal) {}
        };

    static double           s_samePointTolerance = 1.0E-8;
    bvector<PolyEdge>       edges;
    bool                    reverse = (directionSign < 0) != outside;


    for (size_t i=0; i<numVerts-1; i++)
        {
        DVec2d      direction;
        double      z = (NULL == cameraFocalLength) ? 0.0 : - *cameraFocalLength;

        if (direction.NormalizedDifferenceOf(pVertices[i+1], pVertices[i]) > s_samePointTolerance)
            {
            DVec2d      normal = DVec2d::From(reverse ? direction.y : -direction.y, reverse ? -direction.x : direction.x);

            edges.push_back(PolyEdge(pVertices[i], pVertices[i+1], normal, z));
            }
        }
    if (edges.size() < 3)
        {
        BeAssert(false);
        return;
        }

    if (outside)
        {
        for (size_t i=0, last = edges.size()-1; i<edges.size(); i++)
            {
            PolyEdge&               edge     = edges[i];
            PolyEdge&               prevEdge = edges[i ? (i - 1) : last];
            PolyEdge&               nextEdge = edges[(i == last) ? 0 : (i + 1)];
            ConvexClipPlaneSet      convexSet;
            DVec2d                  prevNormal, nextNormal;

            prevNormal.DifferenceOf(edge.m_normal, prevEdge.m_normal);
            nextNormal.DifferenceOf(edge.m_normal, nextEdge.m_normal);

            prevNormal.Normalize();
            nextNormal.Normalize();

            // Create three-sided fans from each edge.   Note we could define the correct region
            // with only two planes for edge, but cannot then designate the "interior" status of the edges accurately.
            convexSet.push_back(ClipPlane(DVec3d::From(prevNormal.x, prevNormal.y), edge.m_origin, invisible, true));
            convexSet.push_back(ClipPlane(DVec3d::From(edge.m_normal.x, edge.m_normal.y), edge.m_origin, invisible, false));
            convexSet.push_back(ClipPlane(DVec3d::From(nextNormal.x, nextNormal.y), nextEdge.m_origin, invisible, true));

            addZClipPlanes(convexSet, zLow, zHigh, invisible);

            planeSet.push_back(convexSet);
            }
        addOutsideZClipSets(planeSet, zLow, zHigh, invisible);
        }
    else
        {
        ConvexClipPlaneSet      convexSet;

        if (NULL == cameraFocalLength)
            {
            for (PolyEdge edge: edges)
                convexSet.push_back(ClipPlane(DVec3d::From(edge.m_normal.x, edge.m_normal.y), edge.m_origin));
            }
        else
            {
            if (reverse)
                for (PolyEdge edge: edges)
                    convexSet.push_back(ClipPlane(DVec3d::FromNormalizedCrossProduct((DVec3dCR) edge.m_origin, (DVec3dCR) edge.m_next), 0.0));
            else
                for (PolyEdge edge: edges)
                    convexSet.push_back(ClipPlane(DVec3d::FromNormalizedCrossProduct((DVec3dCR) edge.m_next, (DVec3dCR) edge.m_origin), 0.0));


            }

        addZClipPlanes(convexSet, zLow, zHigh, invisible);

        planeSet.push_back(convexSet);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void parseConcavePolygonPlanes(ClipPlaneSetR planeSet, DPoint2dCP pVertices, size_t numVerts, double const* zLow, double const* zHigh, bool outside, bool invisible, double* cameraFocalLength)
    {
    size_t                  numTotalVerts;
    AddPlaneSetParams       params(planeSet, zLow, zHigh, outside, invisible, cameraFocalLength);
    double                  largestCoordinate = 0.0;
    double                  aa;

    params.m_localOrigin.x = pVertices[0].x;
    params.m_localOrigin.y = pVertices[0].y;
    params.m_localOrigin.z = (NULL == cameraFocalLength) ? 0.0 : - *cameraFocalLength;
    // If outside clip, add a large rectangle outside the polygon range with the clip shape as an inner loop.

    numTotalVerts = outside ? numVerts + 7 : numVerts;
    bvector <DPoint3d>      point3d(numTotalVerts);
    for (size_t i=0; i<numVerts; i++)
        {
        if (pVertices[i].IsDisconnect())
            {
            point3d[i].InitDisconnect();
            }
        else
            {
            point3d[i].x = pVertices[i].x - params.m_localOrigin.x;
            point3d[i].y = pVertices[i].y - params.m_localOrigin.y;
            point3d[i].z = 0.0;
            aa = fabs(point3d[i].x);
            if (aa > largestCoordinate)
                largestCoordinate = aa;
            aa = fabs(point3d[i].y);
            if (aa > largestCoordinate)
                largestCoordinate = aa;
            }
        }

    if (outside)
        {
        // The limit value just has to be outside the range of the polygon.
        params.m_limitValue = largestCoordinate * 2.0;
        point3d[numVerts].InitDisconnect();
        numVerts++;
        // Points at extrema.
        point3d[numVerts + 0].x = point3d[numVerts + 3].x = point3d[numVerts + 4].x = - params.m_limitValue;
        point3d[numVerts + 1].x = point3d[numVerts + 2].x = params.m_limitValue;
        point3d[numVerts + 0].y = point3d[numVerts + 1].y = point3d[numVerts + 4].y = - params.m_limitValue;
        point3d[numVerts + 2].y = point3d[numVerts + 3].y = params.m_limitValue;
        point3d[numVerts + 5].x = point3d[numVerts + 5].y = DISCONNECT;
        for (int iVertex=0; iVertex < 6; iVertex++)
            point3d[numVerts + iVertex].z = 0.0;
        }
    else
        {
        params.m_limitValue = HUGE_VALUE;
        }

    bvector<bvector<DPoint3d>> loops;
    bvector<bvector<bool>> isBoundary;
    vu_splitToConvexParts(point3d, 1, loops, &isBoundary);
    for (size_t i = 0; i < loops.size (); i++)
        addPlaneSet (loops[i], isBoundary[i], params);

//    vu_splitToConvexParts(&point3d.front(), (int) point3d.size(), &params, addPlaneSet);

    if (params.m_outside)
        {
        // Create plane sets outside each edge ...
        double a = params.m_limitValue;
        addPlaneSetOutsideEdge( a, -a,  a,  a, &params, invisible);
        addPlaneSetOutsideEdge( a,  a, -a,  a, &params, invisible);
        addPlaneSetOutsideEdge(-a,  a, -a, -a, &params, invisible);
        addPlaneSetOutsideEdge(-a, -a,  a, -a, &params, invisible);

        addOutsideZClipSets(planeSet, zLow, zHigh, invisible);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void parseLinearPlanes(ClipPlaneSetR planeSet, DPoint2dCR start, DPoint2dCR end, double const* zLow, double const* zHigh, bool invisible, double* cameraFocalLength)
      // Handles the degenerate case of 2 distinct points (used by select by line).
    {
    DVec2d      normal;

    if (0.0 == normal.NormalizedDifferenceOf(end, start))
        return;

    ConvexClipPlaneSet      convexSet;

    if (NULL == cameraFocalLength)
        {
        DVec2d perpendicular = DVec2d::From(-normal.y, normal.x);

        convexSet.push_back(ClipPlane(DVec3d::From(normal.x, normal.y), DPoint3d::From(start), invisible));
        convexSet.push_back(ClipPlane(DVec3d::From(-normal.x, -normal.y), DPoint3d::From(end), invisible));

        convexSet.push_back(ClipPlane(DVec3d::From(perpendicular.x, perpendicular.y), DPoint3d::From(start), invisible));
        convexSet.push_back(ClipPlane(DVec3d::From(-perpendicular.x, -perpendicular.y), DPoint3d::From(start), invisible));
        }
    else
        {
        DPoint3d        start3d = DPoint3d::From(start.x, start.y, - *cameraFocalLength),
                        end3d   = DPoint3d::From(end.x, end.y, -*cameraFocalLength);

        DVec3d          perpendicular = DVec3d::FromNormalizedCrossProduct((DVec3dCR) end3d, (DVec3dCR) start3d);
        DVec3d          endNormal     = DVec3d::FromNormalizedCrossProduct((DVec3dCR) start3d, perpendicular);

        convexSet.push_back(ClipPlane(perpendicular, 0.0, invisible));
        convexSet.push_back(ClipPlane(endNormal, 0.0, invisible));

        perpendicular.Negate();
        endNormal = DVec3d::FromNormalizedCrossProduct((DVec3dCR) end3d, perpendicular);
        convexSet.push_back(ClipPlane(perpendicular, 0.0, invisible));
        convexSet.push_back(ClipPlane(endNormal, 0.0, invisible));
        }


    addZClipPlanes(convexSet, zLow, zHigh, invisible);

    planeSet.push_back(convexSet);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void parseClipPlanes(ClipPlaneSetR clipPlanes, ClipPolygon const& points, double const *zLow, double const *zHigh, bool isMask, bool invisible, double*  cameraFocalLength)
    {
    bvector<DPoint3d>   shape3d(points.size());
    int                 direction;

    if (points.size() == 3 && !isMask && points.front().IsEqual(points.back()))
        {
        parseLinearPlanes(clipPlanes, points[0], points[1], zLow, zHigh, invisible, cameraFocalLength);
        return;
        }

    bsiDPoint3d_copyDPoint2dArray(&shape3d.front(), &points.front(), (int) points.size());

    if (0 != (direction = bsiGeom_testXYPolygonTurningDirections(&shape3d.front(), (int) shape3d.size())))
        parseConvexPolygonPlanes(clipPlanes,  &points.front(), points.size(), zLow, zHigh, isMask, direction, invisible, cameraFocalLength);
    else
        parseConcavePolygonPlanes(clipPlanes, &points.front(), points.size(), zLow, zHigh, isMask, invisible, cameraFocalLength);
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct  ClipShapePrimitive : ClipPlanesPrimitive
{
    DEFINE_T_SUPER(ClipPlanesPrimitive)
    ClipPolygon                 m_points;
    double                      m_zLow;
    double                      m_zHigh;
    bool                        m_isMask;
    bool                        m_zLowValid;
    bool                        m_zHighValid;
    CurveVectorCPtr              m_curves;
    double m_chordTol;
    double m_angleTol;
    bool                        m_transformValid;
    Transform                   m_transformFromClip;
    Transform                   m_transformToClip;;

    double _GetZLow() const override {return m_zLow;}
    double _GetZHigh() const override {return m_zHigh;}
    bool _ClipZLow() const override {return m_zLowValid;}
    bool _ClipZHigh() const override {return m_zHighValid;}
    bool _IsMask() const override {return m_isMask;}
    ClipPolygonCP _GetPolygon() const override {return &m_points;}
    CurveVectorCP _GetCurvesCP () const override
        {
        return m_curves.get ();
        }

    ClipPrimitive* _Clone() const override {return new ClipShapePrimitive(*this);}
    TransformCP _GetTransformFromClip() const override {return m_transformValid ? &m_transformFromClip : NULL;}
    TransformCP _GetTransformToClip() const override {return m_transformValid ? &m_transformToClip : NULL;}
    void _SetIsMask(bool isMask) override {m_isMask = isMask;}
    void _SetZLow(double zLow) override {m_zLow = zLow; m_zLowValid = true;}
    void _SetZHigh(double zHigh) override {m_zHigh = zHigh; m_zHighValid = true;}
    void _SetClipZLow(bool clip) override {m_zLowValid  = clip;}
    void _SetClipZHigh(bool clip) override {m_zHighValid = clip;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClipShapePrimitive(ClipShapePrimitive const& donor) : ClipPlanesPrimitive(donor)
    {
    m_points            = donor.m_points;
    m_isMask            = donor.m_isMask;
    m_zLow              = donor.m_zLow;
    m_zHigh             = donor.m_zHigh;
    m_zLowValid         = donor.m_zLowValid;
    m_zHighValid        = donor.m_zHighValid;
    m_transformValid    = donor.m_transformValid;
    m_transformToClip   = donor.m_transformToClip;
    m_transformFromClip = donor.m_transformFromClip;

    if (donor.m_curves.IsValid ())
        {
        m_curves = donor.m_curves->Clone ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClipShapePrimitive(DPoint2dCP points, size_t numPoints, bool outside, double const* zLow, double const* zHigh, TransformCP transform, bool invisible) :
    ClipPlanesPrimitive(invisible),
    m_curves(nullptr),
    m_chordTol (0),
    m_angleTol (0)
    {
    BeAssert(NULL != points);
    if (numPoints < 3 || NULL == points)
        {
        BeAssert(false);
        return;
        }

    m_points.Init(points, numPoints);
    if (!points[0].IsEqual(points[numPoints-1]))
        m_points.push_back(points[0]);             // Add stupid closure point.

    Init(outside, zLow, zHigh, transform);

    m_curves = CurveVector::CreateLinear (
            points,
            numPoints,
            CurveVector::BOUNDARY_TYPE_Outer,
            true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClipShapePrimitive
(
CurveVectorCP curves,
double chordTol,
double angleTol,
bool outside,
double const* zLow,
double const* zHigh,
TransformCP transform,
bool invisible
) : ClipPlanesPrimitive (invisible),
    m_curves (curves),
    m_chordTol (chordTol),
    m_angleTol (angleTol)
    {
    bvector<DPoint3d> strokes;
    auto options = IFacetOptions::CreateForCurves ();
    options->SetAngleTolerance (angleTol);
    options->SetChordTolerance (chordTol);
    curves->AddStrokePoints (strokes, *options);
    m_points.clear ();
    for (auto &xyz : strokes)
        m_points.push_back (DPoint2d::From (xyz.x, xyz.y));
    Init (outside, zLow, zHigh, transform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
~ClipShapePrimitive()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ClipPrimitivePtr FromJson(BeJsConst val)
    {
    bvector<DPoint2d> points;
    auto ptsVal = val["points"];
    if (!ptsVal.isArray())
        return nullptr;

    for (Json::ArrayIndex i=0; i<ptsVal.size(); ++i)
        {
        DPoint2d pt;
        BeJsGeomUtils::DPoint2dFromJson(pt, ptsVal[(int)i]);
        points.push_back(pt);
        }

    Transform trans;
    TransformCP transP = nullptr;
    if (val.isMember("trans"))
        {
        transP = &trans;
        BeJsGeomUtils::TransformFromJson(trans, val["trans"]);
        }

    double zLow;
    double const* zlowP = nullptr;
    if (val.isMember("zlow"))
        {
        zlowP = &zLow;
        zLow = val["zlow"].asDouble();
        }

    double zHigh;
    double const* zhighP = nullptr;
    if (val.isMember("zhigh"))
        {
        zhighP = &zHigh;
        zHigh = val["zhigh"].asDouble();
        }

    return new ClipShapePrimitive(points.data(), points.size(), val["mask"].asBool(), zlowP, zhighP, transP, val["invisible"].asBool());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void _ToJson(BeJsValue out) const override
    {
    auto val = out["shape"];

    auto ptsVal = val["points"];
    for (size_t i=0; i<m_points.size(); ++i)
        BeJsGeomUtils::DPoint2dToJson(ptsVal[(Json::ArrayIndex)i], m_points[i]);

    if (!m_transformFromClip.IsIdentity())
        BeJsGeomUtils::TransformToJson(val["trans"], m_transformFromClip);

    if (m_isMask)
        val["mask"] = true;

    if (m_zLow != -HUGE_VALUE)
        val["zlow"] = m_zLow;

    if (m_zHigh != HUGE_VALUE)
        val["zhigh"] = m_zHigh;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Init(bool outside, double const* zLow, double const* zHigh, TransformCP transform)
    {
    m_isMask = outside;

    if (false != (m_zLowValid = (NULL != zLow)))
        m_zLow = *zLow;
    else
        m_zLow = -HUGE_VALUE;

    if (false != (m_zHighValid = (NULL != zHigh)))
        m_zHigh = *zHigh;
    else
        m_zHigh = HUGE_VALUE;

    if (false != (m_transformValid = (NULL != transform)))
        {
        m_transformFromClip = *transform;
        m_transformToClip.InverseOf(m_transformFromClip);
        }
    else
        {
        m_transformFromClip.InitIdentity();
        m_transformToClip = m_transformFromClip;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool _GetRange(DRange3dR range, TransformCP pTransform, bool returnMaskRange) const override
    {
    double          zHigh = 1.0e20, zLow = -1.0e20;
    ClipPolygonCP   clipPolygon;
    Transform       transform = (NULL == pTransform) ? Transform::FromIdentity() : *pTransform;

    if (NULL != GetTransformToClip())
        transform = Transform::FromProduct(transform, *GetTransformFromClip());

    if ((!returnMaskRange && IsMask()) || NULL == (clipPolygon = GetPolygon()))
        return false;

    if (ClipZLow())
        zLow = GetZLow();

    if (ClipZHigh())
        zHigh = GetZHigh();

    range = DRange3d::NullRange();

    for (DPoint2dCR point: *clipPolygon)
        {
        DPoint3d    shapePts[2];

        shapePts[0].Init(point.x, point.y, zLow);
        shapePts[1].Init(point.x, point.y, zHigh);

        transform.Multiply(shapePts, shapePts, 2);

        range.Extend(shapePts, 2);
        }
    return !range.IsEmpty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPlaneSetCP _GetClipPlanes() const override
    {
    if (NULL != m_clipPlanes)
        return  m_clipPlanes;

    m_clipPlanes = new ClipPlaneSet();

    parseClipPlanes(*m_clipPlanes, m_points, m_zLowValid ? &m_zLow : NULL, m_zHighValid ? &m_zHigh : NULL, m_isMask, GetInvisible(), NULL);

    if (m_transformValid)
        m_clipPlanes->TransformInPlace(m_transformFromClip);

    return m_clipPlanes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPlaneSetP _GetMaskPlanes() const override
    {
    if (!m_isMask)
        return NULL;

    if (NULL != m_maskPlanes)
        return  m_maskPlanes;

    m_maskPlanes = new ClipPlaneSet();

    parseClipPlanes(*m_maskPlanes, m_points, m_zLowValid ? &m_zLow : NULL, m_zHighValid ? &m_zHigh : NULL, false, GetInvisible(), NULL);

    if (m_transformValid)
        m_maskPlanes->TransformInPlace(m_transformFromClip);

    return m_maskPlanes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _PointInside(DPoint3dCR point, double onTolerance, bool applyTransform) const
    {
    DPoint3d        testPoint = point;

    if (applyTransform && NULL != _GetTransformToClip())
        _GetTransformToClip()->Multiply(testPoint);

    double      zTolerance = m_isMask ? -onTolerance : onTolerance;

    if ((!_ClipZLow()   || testPoint.z > (_GetZLow()  - zTolerance)) &&
        (!_ClipZHigh() || testPoint.z < (_GetZHigh() + zTolerance)))
        {
        int     parity = PolygonOps::PointPolygonParity (DPoint2d::From (testPoint), m_points, onTolerance);

        if (0 == parity)
            return true;
        else
            return (parity > 0) != m_isMask;
        }
    return m_isMask;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus _TransformInPlace(TransformCR transform) override
    {
    if (transform.IsIdentity())
        return SUCCESS;

    T_Super::_TransformInPlace(transform);

    if (m_transformValid)
        m_transformFromClip = Transform::FromProduct(transform, m_transformFromClip);
    else
        m_transformFromClip = transform;

    m_transformToClip.InverseOf(m_transformFromClip);

    m_transformValid = true;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus _MultiplyPlanesTimesMatrix(DMatrix4dCR matrix) override
    {
    if (m_isMask)
        return ERROR;

    DELETE_AND_CLEAR (m_clipPlanes);
    m_clipPlanes = new ClipPlaneSet();
    parseClipPlanes(*m_clipPlanes, m_points, m_zLowValid ? &m_zLow : NULL, m_zHighValid ? &m_zHigh : NULL, m_isMask, GetInvisible(), nullptr);
    m_clipPlanes->MultiplyPlanesTimesMatrix(matrix);

    return SUCCESS;
    }
};  // ClipShapePrimitive

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClipPrimitive::GetTransforms(TransformP fromClip, TransformP toClip)
    {
    if (NULL != fromClip)
        *fromClip = (NULL == _GetTransformFromClip()) ? Transform::FromIdentity() : *_GetTransformFromClip();

    if (NULL != toClip)
        *toClip = (NULL == _GetTransformToClip()) ? Transform::FromIdentity() : *_GetTransformToClip();

    return (NULL != _GetTransformFromClip());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClipPrimitive::ContainsZClip() const
    {
    ClipPlaneSetCP clipPlanes = GetClipPlanes();

    if (NULL != clipPlanes)
        {
        for (ConvexClipPlaneSetCR convexSet: *clipPlanes)
            {
            for (ClipPlaneCR plane: convexSet)
                {
                if (fabs(plane.GetNormal().z) > 1.0E-6)
                    return true;
                }
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPrimitivePtr ClipPrimitive::CreateCopy(ClipPrimitiveCR primitive)
    {
    return primitive._Clone();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPrimitivePtr ClipPrimitive::CreateFromShape(DPoint2dCP points, size_t numPoints, bool outside, double const* zLow, double const* zHigh, TransformCP transform, bool invisible)
    {
    return new ClipShapePrimitive(points, numPoints, outside, zLow, zHigh, transform, invisible);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPrimitivePtr ClipPrimitive::CreateFromBlock(DPoint3dCR low, DPoint3dCR high, bool outside, ClipMask clipMask, TransformCP transform, bool invisible)
    {
    DPoint2d        blockPoints[5];

    blockPoints[0].x = blockPoints[3].x = blockPoints[4].x = low.x;
    blockPoints[1].x = blockPoints[2].x = high.x;

    blockPoints[0].y = blockPoints[1].y = blockPoints[4].y = low.y;
    blockPoints[2].y = blockPoints[3].y = high.y;

    return CreateFromShape(blockPoints, 5, outside, (ClipMask::None != (clipMask & ClipMask::ZLow)) ? &low.z : NULL, (ClipMask::None != (clipMask & ClipMask::ZHigh)) ? &high.z : NULL, transform, invisible);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPrimitivePtr ClipPrimitive::CreateFromBoundaryCurveVector(CurveVectorCR curveVector, double chordTolerance, double angleTolerance, double const* zLow, double const* zHigh, TransformCP transform, bool invisible)
    {
    if (CurveVector::BOUNDARY_TYPE_Outer != curveVector.GetBoundaryType() &&
        CurveVector::BOUNDARY_TYPE_Inner != curveVector.GetBoundaryType())
        {
        BeAssert(false);
        return ClipPrimitivePtr();
        }

    return new ClipShapePrimitive (
        &curveVector,
        chordTolerance,
        angleTolerance,
        CurveVector::BOUNDARY_TYPE_Inner == curveVector.GetBoundaryType(),
        zLow, zHigh,
        transform,
        invisible);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPrimitivePtr ClipPrimitive::CreateFromClipPlanes(ClipPlaneSetCR planes, bool invisible)
    {
    return new ClipPlanesPrimitive(planes, invisible);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClipPrimitive::IsXYPolygon() const
    {
    if (NULL == GetPolygon())
        return false;

    if (NULL == GetTransformFromClip())
        return true;

    DPoint3d testPoint = {0.0, 0.0, 1.0};
    GetTransformFromClip()->MultiplyMatrixOnly(testPoint);

    return testPoint.MagnitudeXY() < 1.0E-8;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClipPrimitive::PointInside(DPoint3dCR point, double onTolerance) const
    {
    if (NULL != _GetMaskPlanes())
        return ! _GetMaskPlanes()->IsPointOnOrInside(point, onTolerance);

    return NULL == _GetClipPlanes() ? true : _GetClipPlanes()->IsPointOnOrInside(point, onTolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPlaneContainment ClipPrimitive::ClassifyPointContainment(DPoint3dCP points, size_t nPoints, bool ignoreMasks) const
    {
    if (NULL != _GetMaskPlanes())
        {
        if (ignoreMasks)
            return ClipPlaneContainment_StronglyInside;

        switch (_GetMaskPlanes()->ClassifyPointContainment(points, nPoints, true))
            {
            case ClipPlaneContainment_StronglyInside:
                return ClipPlaneContainment_StronglyOutside;

            case ClipPlaneContainment_StronglyOutside:
                return ClipPlaneContainment_StronglyInside;

            case ClipPlaneContainment_Ambiguous:
                return ClipPlaneContainment_Ambiguous;
            }
        }
    return (NULL == _GetClipPlanes()) ? ClipPlaneContainment_StronglyInside : _GetClipPlanes()->ClassifyPointContainment(points, nPoints, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipPrimitive::TransformToClip(DPoint3dR point) const
    {
    TransformCP transform = GetTransformToClip();
    if (nullptr != transform)
        transform->Multiply(point);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipPrimitive::TransformFromClip(DPoint3dR point) const
    {
    TransformCP transform = GetTransformFromClip();
    if (nullptr != transform)
        transform->Multiply(point);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPrimitivePtr ClipPrimitive::FromJson(BeJsConst val)
    {
    if (val.isMember("shape"))
        return ClipShapePrimitive::FromJson(val["shape"]);

    return ClipPlanesPrimitive::FromJson(val["planes"]);
    }
