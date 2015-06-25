/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ClipPrimitive.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>


#define TINY_VALUE                      1.0E-8
#define HUGE_VALUE                      1.0E14
#define POLYFILL_EXTERIOR_EDGE          0x01


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPolygon::ClipPolygon (DPoint2dCP points, size_t nPoints) { Init (points, nPoints); }

void ClipPolygon::Init (DPoint2dCP points, size_t nPoints)
    {
    resize (nPoints);
    memcpy (&front(), points, nPoints * sizeof(DPoint2d));
    }

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      04/2013
+===============+===============+===============+===============+===============+======*/
struct  ClipPlanesPrimitive : ClipPrimitive
{
    mutable ClipPlaneSetP       m_clipPlanes;
    mutable ClipPlaneSetP       m_maskPlanes;
    uint32_t                    m_flags;


    enum 
        {
        Mask_Invisible          = 0x0001,
        Mask_Default            = 0
        };


    ClipPlanesPrimitive (bool invisible) { m_clipPlanes = m_maskPlanes = NULL; m_flags = invisible ? Mask_Invisible : 0; }
    ClipPlanesPrimitive (ClipPlaneSetCR planeSet, bool invisible) : m_clipPlanes (new ClipPlaneSet (planeSet)), m_maskPlanes (NULL) { m_flags = invisible ? Mask_Invisible : 0; }


    virtual ClipPlaneSetCP  _GetClipPlanes ()           const override { return m_clipPlanes; }
    virtual ClipPlaneSetCP  _GetMaskPlanes ()           const override { return m_maskPlanes; }
    virtual ClipPrimitive*  _Clone ()                   const override { return new ClipPlanesPrimitive (*this); }
    virtual bool            _GetInvisible ()            const override { return 0 != (m_flags & Mask_Invisible); }
      

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPlanesPrimitive (ClipPlanesPrimitive const& donor) 
    { 
    m_clipPlanes = NULL == donor.m_clipPlanes ? NULL : new ClipPlaneSet (*donor.m_clipPlanes); 
    m_maskPlanes = NULL == donor.m_maskPlanes ? NULL : new ClipPlaneSet (*donor.m_maskPlanes); 
    m_flags      = donor.m_flags;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
~ClipPlanesPrimitive () 
    { 
   DELETE_AND_CLEAR (m_clipPlanes); 
   DELETE_AND_CLEAR (m_maskPlanes); 
   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus   _TransformInPlace (TransformCR transform) override 
    { 
    if (NULL != m_clipPlanes)
        m_clipPlanes->TransformInPlace (transform);  

    if (NULL != m_maskPlanes)
        m_maskPlanes->TransformInPlace (transform);  

    return SUCCESS; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool    _GetRange (DRange3dR range, TransformCP pTransform, bool returnMaskRange) const override
    {
    return (NULL == m_clipPlanes) ? false : m_clipPlanes->GetRange (range, pTransform); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
static void setPlaneInvisible (ClipPlaneSetCP planeSet, bool invisible)
    {
    if (NULL != planeSet)
        for (ConvexClipPlaneSet& convexSet: const_cast <ClipPlaneSetR> (*planeSet))
            for (ClipPlane& plane: convexSet)
                plane.SetInvisible (invisible);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _SetInvisible (bool invisible)  override    
    {
    if (invisible)
        m_flags |= Mask_Invisible;
    else
        m_flags &= ~Mask_Invisible;
        
    setPlaneInvisible (_GetClipPlanes(), invisible);
    setPlaneInvisible (_GetMaskPlanes(), invisible);
    }

};   // ClipPlanesPrimitve

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

    AddPlaneSetParams (ClipPlaneSetR inPlaneSet, double const* zLow, double const* zHigh, bool outside, bool invisible, double* focalLength) :
                       m_planeSets (inPlaneSet), m_zLow (zLow), m_zHigh (zHigh), m_outside (outside), m_invisible (invisible), m_focalLength (focalLength)  { }
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
static void addOutsideZClipSets (ClipPlaneSetR clipPlaneSet,  double const* zLow, double const* zHigh, bool invisible)
    {
    if (NULL != zLow)
        {
        ConvexClipPlaneSet        convexPlaneSet;

        convexPlaneSet.push_back (ClipPlane (DVec3d::From (0.0, 0.0, -1.0), - *zLow, invisible));
        clipPlaneSet.push_back (convexPlaneSet);
        }
    if (NULL != zHigh)
        {
        ConvexClipPlaneSet        convexPlaneSet;

        convexPlaneSet.push_back (ClipPlane (DVec3d::From (0.0, 0.0, 1.0), *zHigh, invisible));
        clipPlaneSet.push_back (convexPlaneSet);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    RayBentley                      03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    addZClipPlanes (ConvexClipPlaneSetR planeSet, double const* zLow, double const* zHigh, bool invisible)
    {
    if (NULL != zLow)
        planeSet.push_back (ClipPlane (DVec3d::From (0.0, 0.0, 1.0), *zLow, invisible));

    if (NULL != zHigh)
        planeSet.push_back (ClipPlane (DVec3d::From (0.0, 0.0, -1.0), - *zHigh, invisible));
    }                                                                             


/*---------------------------------------------------------------------------------**//**
Add an unbounded plane set (a) to the right of the line defined by two points, and (b) "ahead" of
   the start point.
* @bsimethod                                    EarlinLutz              12/05
+---------------+---------------+---------------+---------------+---------------+------*/
static void     addPlaneSetOutsideEdge
(
double              x0,
double              y0,
double              x1,
double              y1,
AddPlaneSetParams   *pParams,
bool                invisible
)
    {
    DVec3d          unit0, unit1;
    DVec3d          vec0;
    DPoint3d        point0;

    ((DVec3dP)&point0)->Init ( x0 + pParams->m_localOrigin.x, y0 + pParams->m_localOrigin.y, 0.0);
    vec0.Init ( x1 - x0, y1 - y0, 0.0);
    unit0.Normalize (vec0);
    unit1.Init ( unit0.y, -unit0.x, 0.0);

    ConvexClipPlaneSet      convexSet;

    convexSet.push_back (ClipPlane (unit1, point0, invisible));
    convexSet.push_back (ClipPlane (unit0, point0, invisible));

    addZClipPlanes (convexSet, pParams->m_zLow, pParams->m_zHigh, invisible);
    
    pParams->m_planeSets.push_back (convexSet);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    RayBentley                      03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    isLimitEdge (double limitValue, DPoint3dR point0, DPoint3dR point1)
    {
    double tol = 0.000001 * limitValue;
    // High x limit...
    if (   fabs (point0.x - limitValue) < tol
        && fabs (point1.x - limitValue) < tol)
        return true;
    // low x limit ...
    if (   fabs (point0.x + limitValue) < tol
        && fabs (point1.x + limitValue) < tol)
        return true;

    // high y limit ...
    if (   fabs (point0.y - limitValue) < tol
        && fabs (point1.y - limitValue) < tol)
        return true;
    // low y limit ...
    if (   fabs (point0.y + limitValue) < tol
        && fabs (point1.y + limitValue) < tol)
        return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    RayBentley                      03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    addPlaneSet (DPoint3dP pPoints, int* pFlags, int nPoints, void* pUserArg)
    {
    int                     i;
    double                  area = bsiGeom_getXYPolygonArea (pPoints, nPoints);
    DPoint3d                point0, point1, point0Local, point1Local;
    DVec3d                  zVector = DVec3d::From (0.0, 0.0, 1.0);
    DVec3d                  normal, tangent;
    AddPlaneSetParams       *pParams = static_cast<AddPlaneSetParams *>(pUserArg);
    DPoint3d                closurePoint;
    ConvexClipPlaneSet      convexSet;
    bool                    reverse = (area < 0.0);


    for (i = 0;  i < nPoints; i++, point0 = point1, point0Local = point1Local)
        {
        point1Local = pPoints[i % nPoints];
        point1.SumOf (point1Local, pParams->m_localOrigin);
        if (i == 0)
            closurePoint = point1;

        if (i && !point1.IsEqual (point0, TINY_VALUE))
            {
            bool bIsLimitPlane = isLimitEdge (pParams->m_limitValue, point0Local, point1Local);
            bool isInterior    =  (0 == (pFlags[i-1] & POLYFILL_EXTERIOR_EDGE)) || bIsLimitPlane;

            if (NULL == pParams->m_focalLength)
                {
                tangent.DifferenceOf (point1, point0);
                normal.NormalizedCrossProduct (zVector, tangent);

                if (reverse)
                    normal.Negate();

                convexSet.push_back (ClipPlane (normal, point0, pParams->m_invisible, isInterior));
                }
            else
                {
                normal.NormalizedCrossProduct ((DVec3dCR) point1, (DVec3dCR) point0);
                if (reverse)
                    normal.Negate();

                convexSet.push_back (ClipPlane (normal, 0.0, pParams->m_invisible, isInterior));
                }
            }
        }

    addZClipPlanes (convexSet, pParams->m_zLow, pParams->m_zHigh, pParams->m_invisible);        // Note - we add the clip limits even for outside (TR# 236791) to avoid overlapping regions.

    if (!convexSet.empty())
        pParams->m_planeSets.push_back (convexSet);
    }                                                                     

        

/*---------------------------------------------------------------------------------**//**      
* @bsimethod                                                    RayBentley      03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
static void   parseConvexPolygonPlanes 
(
ClipPlaneSetR   planeSet,
DPoint2dCP      pVertices,
size_t          numVerts,
double const*   zLow,
double const*   zHigh,
bool            outside,
int             direction,
bool            invisible,
double*         cameraFocalLength
)
    {
    struct      PolyEdge
        {
        DPoint3d    m_origin;
        DPoint3d    m_next;
        DVec2d      m_normal;

        PolyEdge (DPoint2dCR origin, DPoint2dCR next, DVec2dCR normal, double z) : m_origin (DPoint3d::From (origin.x, origin.y, z)), m_next (DPoint3d::From (next.x, next.y, z)), m_normal (normal) { }
        };

    static double           s_samePointTolerance = 1.0E-8;
    bvector<PolyEdge>       edges;
    bool                    reverse = (direction < 0) != outside;


    for (size_t i=0; i<numVerts-1; i++)
        {
        DVec2d      direction;
        double      z = (NULL == cameraFocalLength) ? 0.0 : - *cameraFocalLength;

        if (direction.NormalizedDifferenceOf (pVertices[i+1], pVertices[i]) > s_samePointTolerance)
            {
            DVec2d      normal = DVec2d::From (reverse ? direction.y : -direction.y, reverse ? -direction.x : direction.x);
     
            edges.push_back (PolyEdge (pVertices[i], pVertices[i+1], normal, z));
            }
        }
    if (edges.size() < 3)
        {
        BeAssert (false);
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

            prevNormal.DifferenceOf (edge.m_normal, prevEdge.m_normal);
            nextNormal.DifferenceOf (edge.m_normal, nextEdge.m_normal);

            prevNormal.Normalize();
            nextNormal.Normalize();

            // Create three-sided fans from each edge.   Note we could define the correct region
            // with only two planes for edge, but cannot then designate the "interior" status of the edges accurately.
            convexSet.push_back (ClipPlane (DVec3d::From (prevNormal.x, prevNormal.y), edge.m_origin, invisible, true));
            convexSet.push_back (ClipPlane (DVec3d::From (edge.m_normal.x, edge.m_normal.y), edge.m_origin, invisible, false));
            convexSet.push_back (ClipPlane (DVec3d::From (nextNormal.x, nextNormal.y), nextEdge.m_origin, invisible, true));

            addZClipPlanes (convexSet, zLow, zHigh, invisible);

            planeSet.push_back (convexSet);
            }
        addOutsideZClipSets (planeSet, zLow, zHigh, invisible);
        }
    else
        {
        ConvexClipPlaneSet      convexSet;

        if (NULL == cameraFocalLength)
            {
            for (PolyEdge edge: edges)
                convexSet.push_back (ClipPlane (DVec3d::From (edge.m_normal.x, edge.m_normal.y), edge.m_origin));
            }
        else
            {
            if (reverse)
                for (PolyEdge edge: edges)
                    convexSet.push_back (ClipPlane (DVec3d::FromNormalizedCrossProduct ((DVec3dCR) edge.m_origin, (DVec3dCR) edge.m_next), 0.0));
            else
                for (PolyEdge edge: edges)
                    convexSet.push_back (ClipPlane (DVec3d::FromNormalizedCrossProduct ((DVec3dCR) edge.m_next, (DVec3dCR) edge.m_origin), 0.0));

                                                                                        
            }

        addZClipPlanes (convexSet, zLow, zHigh, invisible);

        planeSet.push_back (convexSet);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz 06/06
+---------------+---------------+---------------+---------------+---------------+------*/
static void    parseConcavePolygonPlanes
(
ClipPlaneSetR   planeSet,
DPoint2dCP      pVertices,
size_t          numVerts,
double const*   zLow,
double const*   zHigh,
bool            outside,
bool            invisible,
double*         cameraFocalLength
)
    {
    size_t                  numTotalVerts;
    AddPlaneSetParams       params (planeSet, zLow, zHigh, outside, invisible, cameraFocalLength);
    double                  largestCoordinate = 0.0;
    double                  aa;

    params.m_localOrigin.x = pVertices[0].x;
    params.m_localOrigin.y = pVertices[0].y;
    params.m_localOrigin.z = (NULL == cameraFocalLength) ? 0.0 : - *cameraFocalLength;
    // If outside clip, add a large rectangle outside the polygon range with the clip shape as an inner loop.

    numTotalVerts = outside ? numVerts + 7 : numVerts;
    bvector <DPoint3d>      point3d (numTotalVerts);
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
            aa = fabs (point3d[i].x);
            if (aa > largestCoordinate)
                largestCoordinate = aa;
            aa = fabs (point3d[i].y);
            if (aa > largestCoordinate)
                largestCoordinate = aa;
            }
        }

    if (outside)
        {
        // The limit value just has to be outside the range of the polygon.
        params.m_limitValue = largestCoordinate * 2.0;
        point3d[numVerts].InitDisconnect ();
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

    vu_splitToConvexParts (&point3d.front(), (int) point3d.size(), &params, addPlaneSet);

    if (params.m_outside)
        {
        // Create plane sets outside each edge ...
        double a = params.m_limitValue;
        addPlaneSetOutsideEdge ( a, -a,  a,  a, &params, invisible);
        addPlaneSetOutsideEdge ( a,  a, -a,  a, &params, invisible);
        addPlaneSetOutsideEdge (-a,  a, -a, -a, &params, invisible);
        addPlaneSetOutsideEdge (-a, -a,  a, -a, &params, invisible);

        addOutsideZClipSets (planeSet, zLow, zHigh, invisible);
        }
    }

/*---------------------------------------------------------------------------------**//**      
* @bsimethod                                                    RayBentley      03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
static void   parseLinearPlanes
(
ClipPlaneSetR   planeSet,
DPoint2dCR      start, 
DPoint2dCR      end,
double const*   zLow,
double const*   zHigh,
bool            invisible,
double*         cameraFocalLength
)       // Handles the degenerate case of 2 distinct points (used by select by line).  
    {
    DVec2d      normal;

    if (0.0 == normal.NormalizedDifferenceOf (end, start))
        return;

    ConvexClipPlaneSet      convexSet;

    if (NULL == cameraFocalLength)
        {
        DVec2d                  perpendicular = DVec2d::From (-normal.y, normal.x);

        convexSet.push_back (ClipPlane (DVec3d::From (normal.x, normal.y), DPoint3d::From (start), invisible));
        convexSet.push_back (ClipPlane (DVec3d::From (-normal.x, -normal.y), DPoint3d::From (end), invisible));

        convexSet.push_back (ClipPlane (DVec3d::From (perpendicular.x, perpendicular.y), DPoint3d::From (start), invisible));
        convexSet.push_back (ClipPlane (DVec3d::From (-perpendicular.x, -perpendicular.y), DPoint3d::From (start), invisible));
        }
    else
        {
        DPoint3d        start3d = DPoint3d::From (start.x, start.y, - *cameraFocalLength),
                        end3d   = DPoint3d::From (end.x, end.y, -*cameraFocalLength);

        DVec3d          perpendicular = DVec3d::FromNormalizedCrossProduct ((DVec3dCR) end3d, (DVec3dCR) start3d);
        DVec3d          endNormal     = DVec3d::FromNormalizedCrossProduct ((DVec3dCR) start3d, perpendicular);
                                                                                                                              
        convexSet.push_back (ClipPlane (perpendicular, 0.0, invisible));
        convexSet.push_back (ClipPlane (endNormal, 0.0, invisible));                                                                                                                                                                                              

        perpendicular.Negate ();
        endNormal = DVec3d::FromNormalizedCrossProduct ((DVec3dCR) end3d, perpendicular);
        convexSet.push_back (ClipPlane (perpendicular, 0.0, invisible));
        convexSet.push_back (ClipPlane (endNormal, 0.0, invisible));
        }

        
    addZClipPlanes (convexSet, zLow, zHigh, invisible);

    planeSet.push_back (convexSet);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void parseClipPlanes (ClipPlaneSetR clipPlanes, ClipPolygon const& points, double const *zLow, double const *zHigh, bool isMask, bool invisible, double*  cameraFocalLength)
    {
    bvector<DPoint3d>   shape3d (points.size());
    int                 direction;

    if (points.size() == 3 && !isMask && points.front().IsEqual (points.back()))
        {
        parseLinearPlanes (clipPlanes, points[0], points[1], zLow, zHigh, invisible, cameraFocalLength);
        return;
        }

    bsiDPoint3d_copyDPoint2dArray (&shape3d.front(), &points.front(), (int) points.size());

    if (0 != (direction = bsiGeom_testXYPolygonTurningDirections (&shape3d.front(), (int) shape3d.size())))
        parseConvexPolygonPlanes (clipPlanes,  &points.front(), points.size(), zLow, zHigh, isMask, direction, invisible, cameraFocalLength);
    else
        parseConcavePolygonPlanes (clipPlanes, &points.front(), points.size(), zLow, zHigh, isMask, invisible, cameraFocalLength); 
    }

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      04/2013
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
    GPArrayP                    m_gpa;
    mutable MSBsplineCurvePtr   m_bCurve;  
    bool                        m_transformValid;
    Transform                   m_transformFromClip;
    Transform                   m_transformToClip;;


    virtual     GPArrayCP       _GetGPA (bool onlyIfNonLinear) const override { return (!onlyIfNonLinear || m_gpa->ContainsCurves()) ? m_gpa : NULL; }
    virtual     double          _GetZLow ()               const override  { return m_zLow; }
    virtual     double          _GetZHigh ()              const override  { return m_zHigh; }
    virtual     bool            _ClipZLow ()              const override  { return m_zLowValid; }
    virtual     bool            _ClipZHigh ()             const override  { return m_zHighValid; }
    virtual     bool            _IsMask ()                const override  { return m_isMask; }
    virtual     ClipPolygonCP   _GetPolygon ()            const override  { return &m_points; }
    virtual     ClipPrimitive*  _Clone ()                 const override  { return new ClipShapePrimitive (*this); }
    virtual     TransformCP     _GetTransformFromClip ()  const override  { return m_transformValid ? &m_transformFromClip : NULL; }
    virtual     TransformCP     _GetTransformToClip ()    const override  { return m_transformValid ? &m_transformToClip : NULL; }

    virtual     void            _SetIsMask (bool isMask) override         { m_isMask = isMask; }
    virtual     void            _SetZLow (double zLow)   override         { m_zLow = zLow;    m_zLowValid = true; }        
    virtual     void            _SetZHigh (double zHigh) override         { m_zHigh = zHigh;  m_zHighValid = true; }      
    virtual     void            _SetClipZLow (bool clip)                  { m_zLowValid  = clip; }
    virtual     void            _SetClipHigh (bool clip)                  { m_zHighValid = clip; }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ClipShapePrimitive (ClipShapePrimitive const& donor) : ClipPlanesPrimitive (donor)
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

    if (NULL != donor.m_gpa)
        {
        m_gpa           = GPArray::Grab ();
        m_gpa->CopyContentsOf (donor.m_gpa);
        }

    if (donor.m_bCurve.IsValid())
        m_bCurve = donor.m_bCurve->CreateCopy();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ClipShapePrimitive (DPoint2dCP points, size_t numPoints, bool outside, double const* zLow, double const* zHigh, TransformCP transform, bool invisible) : ClipPlanesPrimitive (invisible), m_gpa (NULL)
    {
    BeAssert (NULL != points);
    if (numPoints < 3 || NULL == points)
        {
        BeAssert (false);
        return;
        }

    m_points.Init (points, numPoints);
    if (!points[0].IsEqual (points[numPoints-1]))
        m_points.push_back (points[0]);             // Add stupid closure point.

    Init (outside, zLow, zHigh, transform);

    m_gpa = GPArray::Grab ();
    m_gpa->Add (points, (int) numPoints);
    }
    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ClipShapePrimitive (GPArrayCR gpa, double chordTolerance, double angleTolerance, bool outside, double const* zLow, double const* zHigh, TransformCP transform, bool invisible) : ClipPlanesPrimitive (invisible)
    {
    m_gpa = GPArray::Grab ();
    m_gpa->CopyContentsOf (&gpa);

    GPArraySmartP   strokedGPA;
    strokedGPA->AddStrokes (gpa, chordTolerance, angleTolerance, 0.0);

    m_points.resize (strokedGPA->GetCount ());

    for (size_t iPt = 0; iPt < m_points.size(); iPt++)
        {
        DPoint3d    pt;

        strokedGPA->GetNormalized (pt, (int) iPt);

        m_points[iPt].x = pt.x;
        m_points[iPt].y = pt.y;
        }

    Init (outside, zLow, zHigh, transform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ClipShapePrimitive (GPArrayCR gpa, ClipPolygonCR points, bool outside, double const* zLow, double const* zHigh, TransformCP transform, bool invisible) : ClipPlanesPrimitive (invisible)
    {
    m_gpa = GPArray::Grab ();
    m_gpa->CopyContentsOf (&gpa);
    m_points = points;

    Init (outside, zLow, zHigh, transform);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
~ClipShapePrimitive ()
    {
    if (NULL != m_gpa)
        m_gpa->Drop ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void    Init (bool outside, double const* zLow, double const* zHigh, TransformCP transform)
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
        m_transformToClip.InverseOf (m_transformFromClip);
        }
    else
        {
        m_transformFromClip.InitIdentity ();
        m_transformToClip = m_transformFromClip;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool    _GetRange (DRange3dR range, TransformCP pTransform, bool returnMaskRange) const override
    {
    double          zHigh = 1.0e20, zLow = -1.0e20;
    ClipPolygonCP   clipPolygon;
    Transform       transform = (NULL == pTransform) ? Transform::FromIdentity() : *pTransform;

    if (NULL != GetTransformToClip())
        transform = Transform::FromProduct (transform, *GetTransformFromClip());

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

        shapePts[0].Init (point.x, point.y, zLow);
        shapePts[1].Init (point.x, point.y, zHigh);
        transform.Multiply (shapePts, shapePts, 2);

        range.Extend (shapePts, 2);
        }
    return !range.IsEmpty();
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ClipPlaneSetCP           _GetClipPlanes () const override  
    {
    if (NULL != m_clipPlanes)
        return  m_clipPlanes;

    m_clipPlanes = new ClipPlaneSet();

    parseClipPlanes (*m_clipPlanes, m_points, m_zLowValid ? &m_zLow : NULL, m_zHighValid ? &m_zHigh : NULL, m_isMask, GetInvisible(), NULL); 

    if (m_transformValid)
        m_clipPlanes->TransformInPlace (m_transformFromClip);

    return m_clipPlanes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _ApplyCameraToPlanes (double focalLength)
    {
    if (m_isMask)
        return ERROR;

    DELETE_AND_CLEAR (m_clipPlanes);
    m_clipPlanes = new ClipPlaneSet();

    parseClipPlanes (*m_clipPlanes, m_points, m_zLowValid ? &m_zLow : NULL, m_zHighValid ? &m_zHigh : NULL, m_isMask, GetInvisible(), &focalLength); 
    if (m_transformValid)
        m_clipPlanes->TransformInPlace (m_transformFromClip);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ClipPlaneSetP           _GetMaskPlanes () const override  
    {
    if (!m_isMask)
        return NULL;

    if (NULL != m_maskPlanes)
        return  m_maskPlanes;

    m_maskPlanes = new ClipPlaneSet();

    parseClipPlanes (*m_maskPlanes, m_points, m_zLowValid ? &m_zLow : NULL, m_zHighValid ? &m_zHigh : NULL, false, GetInvisible(), NULL); 

    if (m_transformValid)
        m_maskPlanes->TransformInPlace (m_transformFromClip);

    return m_maskPlanes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/07
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineCurveCP    _GetBCurve () const
    {
    if (!m_bCurve.IsValid())
        {
        m_bCurve = MSBsplineCurve::CreatePtr ();
        m_gpa->ToBCurve (m_bCurve.get());
        }
    return m_bCurve.get();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _PointInside (DPoint3dCR point, double onTolerance, bool applyTransform) const
    {
    DPoint3d        testPoint = point;

    if (applyTransform && NULL != _GetTransformToClip())
        _GetTransformToClip()->Multiply (testPoint);

    double      zTolerance = m_isMask ? -onTolerance : onTolerance;

    if ((!_ClipZLow()   || testPoint.z > (_GetZLow()  - zTolerance)) &&
        (!_ClipZHigh () || testPoint.z < (_GetZHigh() + zTolerance)))
        {
        int     parity = bsiDPoint2d_PolygonParity ((DPoint2d *) &testPoint, &m_points.front(), (int) m_points.size(), onTolerance);

        if (0 == parity)
            return true;
        else
            return (parity > 0) != m_isMask;
        }
    return m_isMask;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus   _TransformInPlace (TransformCR transform) override
    {
    if (transform.IsIdentity())
        return SUCCESS;
     
    T_Super::_TransformInPlace (transform);

    if (m_transformValid)
        m_transformFromClip = Transform::FromProduct (transform, m_transformFromClip);
    else
        m_transformFromClip = transform;

    m_transformToClip.InverseOf (m_transformFromClip);

    m_transformValid = true;

    return SUCCESS;
    }

};  // ClipShapePrimitive


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ClipPrimitive::GetTransforms (TransformP fromClip, TransformP toClip)
    {
    if (NULL != fromClip)
        *fromClip = (NULL == _GetTransformFromClip ()) ? Transform::FromIdentity() : *_GetTransformFromClip();

    if (NULL != toClip)
        *toClip = (NULL == _GetTransformToClip()) ? Transform::FromIdentity() : *_GetTransformToClip();

    return (NULL != _GetTransformFromClip());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ClipPrimitive::ContainsZClip () const
    {
    ClipPlaneSetCP       clipPlanes = GetClipPlanes();

    if (NULL != clipPlanes)
        {
        for (ConvexClipPlaneSetCR convexSet: *clipPlanes)
            for (ClipPlaneCR plane: convexSet)
                if (fabs (plane.GetNormal().z) > 1.0E-6)
                    return true;
        }

    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPrimitivePtr    ClipPrimitive::CreateCopy (ClipPrimitiveCR primitive)
    {
    return primitive._Clone ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPrimitivePtr    ClipPrimitive::CreateFromShape (DPoint2dCP points, size_t numPoints, bool outside, double const* zLow, double const* zHigh, TransformCP transform, bool invisible)
    {
    return new ClipShapePrimitive (points, numPoints, outside, zLow, zHigh, transform, invisible);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013  
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPrimitivePtr    ClipPrimitive::CreateFromBlock (DPoint3dCR low, DPoint3dCR high, bool outside, ClipMask clipMask, TransformCP transform, bool invisible)
    {
    DPoint2d        blockPoints[5];

    blockPoints[0].x = blockPoints[3].x = blockPoints[4].x = low.x;
    blockPoints[1].x = blockPoints[2].x = high.x;

    blockPoints[0].y = blockPoints[1].y = blockPoints[4].y = low.y;
    blockPoints[2].y = blockPoints[3].y = high.y;

    return CreateFromShape (blockPoints, 5, outside, (ClipMask::None != (clipMask & ClipMask::ZLow)) ? &low.z : NULL, (ClipMask::None != (clipMask & ClipMask::ZHigh)) ? &high.z : NULL, transform, invisible);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPrimitivePtr ClipPrimitive::CreateFromGPA (GPArrayCR gpa, double chordTolerance, double angleTolerance, bool outside, double const* zLow, double const* zHigh, TransformCP transform, bool invisible)
    {
    return new ClipShapePrimitive (gpa, chordTolerance, angleTolerance, outside, zLow, zHigh, transform, invisible);
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPrimitivePtr  ClipPrimitive::CreateFromBoundaryCurveVector (CurveVectorCR curveVector, double chordTolerance, double angleTolerance, double const* zLow, double const* zHigh, TransformCP transform, bool invisible)
    {
    if (CurveVector::BOUNDARY_TYPE_Outer != curveVector.GetBoundaryType() &&
        CurveVector::BOUNDARY_TYPE_Inner != curveVector.GetBoundaryType())
        {
        BeAssert (false);
        return ClipPrimitivePtr ();
        }
    
    GPArraySmartP     loopGpa;
    
    if (SUCCESS != loopGpa->AddCurves (curveVector))
        {
        BeAssert (false);
        return ClipPrimitivePtr ();
        }

    return ClipPrimitive::CreateFromGPA (*loopGpa, chordTolerance, angleTolerance, CurveVector::BOUNDARY_TYPE_Inner == curveVector.GetBoundaryType(), zLow, zHigh, transform, invisible);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPrimitivePtr ClipPrimitive::CreateFromGPA (GPArrayCR gpa, ClipPolygonCR points, bool outside, double const* zLow, double const* zHigh, TransformCP transform, bool invisible)
    {
    return new ClipShapePrimitive (gpa, points, outside, zLow, zHigh, transform, invisible);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPrimitivePtr    ClipPrimitive::CreateFromClipPlanes (ClipPlaneSetCR planes, bool invisible)
    {
    return new ClipPlanesPrimitive (planes, invisible);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ClipPrimitive::IsXYPolygon () const
    {
    if (NULL == GetPolygon())
        return false;

    if (NULL == GetTransformFromClip ())
        return true;


    DPoint3d        testPoint = {0.0, 0.0, 1.0};

    GetTransformFromClip()->MultiplyMatrixOnly (testPoint);

    return testPoint.MagnitudeXY() < 1.0E-8;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool  ClipPrimitive::PointInside (DPoint3dCR point, double onTolerance) const
    {
    if (NULL != _GetMaskPlanes())
        return ! _GetMaskPlanes()->IsPointOnOrInside (point, onTolerance); 

    return NULL == _GetClipPlanes() ? true : _GetClipPlanes()->IsPointOnOrInside (point, onTolerance); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPlaneContainment ClipPrimitive::ClassifyPointContainment (DPoint3dCP points, size_t nPoints, bool ignoreMasks) const
    {
    if (NULL != _GetMaskPlanes())
        {
        if (ignoreMasks)
            return ClipPlaneContainment_StronglyInside;

        switch (_GetMaskPlanes()->ClassifyPointContainment (points, nPoints, true))
            {
            case ClipPlaneContainment_StronglyInside:
                return ClipPlaneContainment_StronglyOutside;

            case ClipPlaneContainment_StronglyOutside:
                return ClipPlaneContainment_StronglyInside;

            case ClipPlaneContainment_Ambiguous:
                return ClipPlaneContainment_Ambiguous;
            }
        }
    return (NULL == _GetClipPlanes()) ? ClipPlaneContainment_StronglyInside : _GetClipPlanes()->ClassifyPointContainment (points, nPoints, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void        ClipPrimitive::TransformToClip (DPoint3dR point) const
    {
    TransformCP     transform;

    if (NULL != (transform = GetTransformToClip()))
        transform->Multiply (point);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void     ClipPrimitive::TransformFromClip (DPoint3dR point) const
    {
    TransformCP     transform;

    if (NULL != (transform = GetTransformFromClip()))
        transform->Multiply (point);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPlaneSetCP      ClipPrimitive::GetClipPlanes () const                                   { return _GetClipPlanes(); }
ClipPlaneSetCP      ClipPrimitive::GetMaskPlanes () const                                   { return _GetMaskPlanes(); }
ClipPlaneSetCP      ClipPrimitive::GetMaskOrClipPlanes () const                             { return NULL == _GetMaskPlanes() ? _GetClipPlanes() : _GetMaskPlanes(); }


GPArrayCP           ClipPrimitive::GetGPA (bool onlyIfNonLinear) const                      { return _GetGPA(onlyIfNonLinear); }
MSBsplineCurveCP    ClipPrimitive::GetBCurve () const                                       { return _GetBCurve (); }
bool                ClipPrimitive::IsMask () const                                          { return _IsMask (); }
bool                ClipPrimitive::ClipZLow () const                                        { return _ClipZLow (); }
bool                ClipPrimitive::ClipZHigh () const                                       { return _ClipZHigh (); }
double              ClipPrimitive::GetZLow () const                                         { return _GetZLow (); }
double              ClipPrimitive::GetZHigh () const                                        { return _GetZHigh (); }
ClipPolygonCP       ClipPrimitive::GetPolygon () const                                      { return _GetPolygon (); }
TransformCP         ClipPrimitive::GetTransformToClip () const                              { return _GetTransformToClip (); }    
TransformCP         ClipPrimitive::GetTransformFromClip () const                            { return _GetTransformFromClip (); } 
bool                ClipPrimitive::GetInvisible () const                                    { return _GetInvisible(); }
bool                ClipPrimitive::GetRange (DRange3dR range, TransformCP transform, bool returnMaskRange) const  { return _GetRange (range, transform, returnMaskRange); }

void                ClipPrimitive::SetIsMask (bool isMask)                   { _SetIsMask (isMask); }
void                ClipPrimitive::SetZLow (double zLow)                     { _SetZLow (zLow); }
void                ClipPrimitive::SetZHigh (double zHigh)                   { _SetZHigh (zHigh); }
void                ClipPrimitive::SetClipZLow (bool clipZLow)               { _SetClipZLow (clipZLow); }
void                ClipPrimitive::SetClipZHigh (bool clipZHigh)             { _SetClipZHigh (clipZHigh); }
void                ClipPrimitive::SetInvisible (bool invisible)             { _SetInvisible (invisible); }
BentleyStatus       ClipPrimitive::TransformInPlace (TransformCR transform)  { return _TransformInPlace (transform); }
BentleyStatus       ClipPrimitive::ApplyCameraToPlanes (double focalLength)  { return _ApplyCameraToPlanes (focalLength); }
void                ClipPrimitive::ParseClipPlanes () const                  { GetClipPlanes(); GetMaskPlanes(); }

void                ClipPrimitive::_SetIsMask (bool isMask)                  { BeAssert (false); }
void                ClipPrimitive::_SetZLow (double zLow)                    { BeAssert (false); } 
void                ClipPrimitive::_SetClipZLow (bool clipZLow)              { BeAssert (!clipZLow); } 
void                ClipPrimitive::_SetZHigh (double zHigh)                  { BeAssert (false); }

void                ClipPrimitive::_SetClipZHigh (bool clipZHigh)            { BeAssert (!clipZHigh); }
BentleyStatus       ClipPrimitive::_TransformInPlace (TransformCR transform) { BeAssert (false); return ERROR; }


