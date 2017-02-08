/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/ClipPlane.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ClipPlane::ClipPlane ()
    {
    m_normal.x = m_normal.y = 0.0;
    m_normal.z = 1.0;
    m_distance = 0.0;
    m_flags = 0;
    }

uint32_t    ClipPlane::GetFlags () const        { return m_flags; }
void        ClipPlane::SetFlags (uint32_t flags)  { m_flags = flags; }
double      ClipPlane::GetDistance () const     { return m_distance; }
DVec3dCR    ClipPlane::GetNormal () const       { return m_normal; }
bool        ClipPlane::GetIsInterior () const   { return 0 != (m_flags & PlaneMask_Interior); }
bool        ClipPlane::GetIsInvisible  () const { return 0 != (m_flags & PlaneMask_Invisible); }
bool        ClipPlane::IsVisible () const       { return 0 == (m_flags & (PlaneMask_Interior | PlaneMask_Invisible)); }  

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPlane::ClipPlane (DPlane3dCR plane, bool invisible, bool interior)
    {
    m_normal.Normalize (plane.normal);
    m_distance = m_normal.DotProduct (plane.origin);
    SetFlags (invisible, interior);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPlane::ClipPlane(DVec3dCR normal, double distance, bool invisible, bool interior)
    {
    m_normal   = normal;
    m_distance = distance;
    SetFlags (invisible, interior);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPlane::ClipPlane(DVec3dCR normal, DPoint3dCR point, bool invisible, bool interior)
    {
    m_normal   = normal;
    m_distance = normal.DotProduct (point);
    SetFlags (invisible, interior);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipPlane::SetFlags (bool invisible, bool interior)
    {
    m_flags = (interior ? PlaneMask_Interior : 0) | (invisible ? PlaneMask_Invisible : 0);
    }
 
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ValidatedClipPlane ClipPlane::FromEdgeAndUpVector
(
DPoint3dCR point0,
DPoint3dCR point1,
DVec3dCR upVector,  //!< [in] vector towards eye.  0-tilt plane normal is the edge vector
Angle tiltAngle     //!< [in] angle to tilt plane
)
    {
    DVec3d edgeVector = point1 - point0;
    auto baseNormal = DVec3d::FromCrossProduct (upVector, edgeVector).ValidatedNormalize ();
    if (baseNormal.IsValid ())
        {
        DVec3d normal = baseNormal.Value ();
        if (!Angle::IsNearZero (tiltAngle.Radians ()))
            {
            auto tiltNormal = DVec3d::FromRotateVectorAroundVector (baseNormal, edgeVector, tiltAngle);
            if (tiltNormal.IsValid ())
                normal = tiltNormal.Value ();
            }
        normal.Negate ();
        return ValidatedClipPlane (ClipPlane (normal, point0, false, false), true);
        }
    return ValidatedClipPlane ();   // and this is invalid
    }

ValidatedClipPlane ClipPlane::FromPointsAndDistanceAlongPlaneNormal
(
bvector<DPoint3d> const &points,    //!< [in] polyline points
DVec3d upVector,                    //!< [in] upward vector (e.g. towards eye at infinity)
double  distance,                    //!< [in] angle for tilt of planes.
bool    pointsInside                //!< [in] true to orient so the points are inside.
)
    {
    auto unitUp = upVector.ValidatedNormalize ();
    if (points.empty () || !unitUp.IsValid ())
        return ValidatedClipPlane ();   // fail

    DRange1d range = DRange1d::From (0);
    DPoint3d xyz0 = points[0];
    DVec3d unit = unitUp.Value ();
    for (size_t i = 1; i < points.size (); i++)
        {
        double d = (points[0] - xyz0).DotProduct (unit);
        range.Extend (d);
        }
    DPoint3d planePoint;
    DVec3d normal = unit;
    if (distance > 0.0)
        {
        planePoint = xyz0 +  (range.High () + distance) * unit;
        normal.Negate ();
        }
    else
        {
        planePoint = xyz0 + (range.Low () + distance) * unit;   // negative distance moves backwards
        }
    if (!pointsInside)
        normal.Negate ();

    return ValidatedClipPlane (ClipPlane (normal, planePoint), true);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DPlane3d ClipPlane::GetDPlane3d () const
    {
    DPlane3d plane;
    plane.normal = m_normal;
    double b = m_normal.Magnitude ();
    if (b == 0.0)
        plane.origin.Zero ();
    else
        plane.origin.Scale (m_normal, m_distance / b);

    return plane;
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DPoint4d ClipPlane::GetDPlane4d () const
    {
    return DPoint4d::From (m_normal.x, m_normal.y, m_normal.z, -m_distance);
    }

void ClipPlane::SetDPlane4d (DPoint4dCR plane)
    {
    double a = sqrt (plane.x * plane.x + plane.y * plane.y + plane.z * plane.z);
    double r = a == 0.0 ? 1.0 : 1.0 / a;
       
    m_normal.x = r * plane.x;
    m_normal.y = r * plane.y;
    m_normal.z = r * plane.z;
    m_distance = -r * plane.w;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double ClipPlane::EvaluatePoint (DPoint3dCR point) const
    {
    return point.x * m_normal.x +  point.y * m_normal.y +  point.z * m_normal.z - m_distance;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double ClipPlane::DotProduct (DVec3dCR vector) const
    {
    return vector.x * m_normal.x +  vector.y * m_normal.y +  vector.z * m_normal.z;
    } 

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double ClipPlane::DotProduct (DPoint3dCR point) const
    {
    return point.x * m_normal.x +  point.y * m_normal.y +  point.z * m_normal.z;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ClipPlane::IsPointOnOrInside (DPoint3dCR point, double tolerance) const
    {
    return EvaluatePoint (point) + tolerance >= 0.0;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ClipPlane::IsPointOn (DPoint3dCR point, double tolerance) const
    {
    return fabs (EvaluatePoint (point)) < tolerance;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ClipPlane::IsPointOnOrInside (DPoint3dCR point) const
    {
    return EvaluatePoint (point) >= 0.0;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2016
+--------------------------------------------------------------------------------------*/
int ClipPlane::SimpleIntersectionFractions(DEllipse3dCR arc, double intersectionFractions[2], bool bounded) const
    {
    double alpha = EvaluatePoint (arc.center);
    double beta = DotProduct (arc.vector0);
    double gamma = DotProduct (arc.vector90);
    double cc[2], ss[2], distanceSquared;
    int numSolution = bsiMath_solveUnitQuadratic (&cc[0], &ss[0], &cc[1], &ss[1], &distanceSquared, alpha, beta, gamma);
    int numAccept = 0;
    for (int i = 0; i < numSolution; i++)
        {
        double theta = atan2 (ss[i], cc[i]);
        double fraction = arc.AngleToFraction(theta);
        if (!bounded || DoubleOps::IsIn01 (fraction))
            intersectionFractions[numAccept++] = fraction;
        }
    return numAccept;
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ClipPlane::BoundedSegmentHasSimpleIntersection (DPoint3dCR pointA, DPoint3dCR pointB, double &fraction) const
    {
    double h0 = EvaluatePoint (pointA);
    double h1 = EvaluatePoint (pointB);
    fraction = 0.0;
    if (h0 * h1 > 0.0)
        return false;
    if (h0 == 0.0 && h1 == 0.0)
        return false;
    fraction = -h0 / (h1 - h0);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipPlane::TransformInPlace (TransformCR transform)
    {
    DPlane3d        plane = GetDPlane3d ();

    transform.Multiply (plane, plane);

    m_normal.Normalize (plane.normal);
    m_distance = m_normal.DotProduct (plane.origin);
    }

void ClipPlane::MultiplyPlaneTimesMatrix(DMatrix4dCR matrix)
    {
    DPoint4d        plane = GetDPlane4d ();

    matrix.MultiplyTranspose (&plane, &plane, 1);
    SetDPlane4d (plane);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void    ClipPlane::SetInvisible (bool invisible)
    {
    if (invisible)
        m_flags |= PlaneMask_Invisible;
    else
        m_flags &= ~PlaneMask_Invisible;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void    ClipPlane::Negate ()
    {
    m_normal.Negate ();
    m_distance = -m_distance;
    }
void ClipPlane::OffsetDistance (double offset) { m_distance += offset; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void    ClipPlane::ConvexPolygonClipInPlace (bvector<DPoint3d> &xyz, bvector<DPoint3d> &work) const
    {
    work.clear ();
    size_t numSplit = 0;
    static double s_fractionTol = 1.0e-8;
    if (xyz.size () > 2)
        {
        DPoint3d xyz0 = xyz.back ();
        double a0 = EvaluatePoint (xyz0);
//        if (a0 >= 0.0)
//            work.push_back (xyz0);
        for (auto &xyz1 : xyz)
            {
            double a1 = EvaluatePoint (xyz1);
            if (a0 * a1 < 0.0)
                {
                // simple crossing ..
                double f = -a0 / (a1 - a0);
                if (f > 1.0 - s_fractionTol && a1 >= 0.0)
                    {
                    // the endpoint will be saved -- avoid the duplicate
                    }
                else
                    work.push_back (DPoint3d::FromInterpolate (xyz0, f, xyz1));
                    numSplit++;
                }
            if (a1 >= 0.0)
                work.push_back (xyz1);
            xyz0 = xyz1;
            a0 = a1;
            }
        }

    if (work.size () <= 2)
        xyz.clear ();
    else if (numSplit > 0)
        work.swap (xyz);
    // If no splits happened, no need to swap
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void    ClipPlane::PolygonCrossings (bvector<DPoint3d> const &xyz, bvector<DPoint3d> &crossings) const
    {
    crossings.clear ();
    if (xyz.size () >= 2)
        {
        DPoint3d xyz0 = xyz.back ();
        double a0 = EvaluatePoint (xyz0);
        for (auto &xyz1 : xyz)
            {
            double a1 = EvaluatePoint (xyz1);
            if (a0 * a1 < 0.0)
                {
                // simple crossing ..
                double f = -a0 / (a1 - a0);
                crossings.push_back (DPoint3d::FromInterpolate (xyz0, f, xyz1));
                }
            if (a1 == 0.0)          // IMPORTANT -- every point is directly tested here
                crossings.push_back (xyz1);
            xyz0 = xyz1;
            a0 = a1;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz 10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void    ClipPlane::ConvexPolygonSplitInsideOutside (
bvector<DPoint3d> const &xyz,         //!< [in] original polygon
bvector<DPoint3d> &xyzIn,             //!< [out] inside part
bvector<DPoint3d> &xyzOut             //!< [out] outside part
) const
    {
    xyzOut.clear ();
    xyzIn.clear ();
    size_t numSplit = 0;
    static double s_fractionTol = 1.0e-8;
    if (xyz.size () > 2)
        {
        DPoint3d xyz0 = xyz.back ();
        double a0 = EvaluatePoint (xyz0);
//        if (a0 >= 0.0)
//            work.push_back (xyz0);
        for (auto &xyz1 : xyz)
            {
            double a1 = EvaluatePoint (xyz1);
            bool nearZero = false;
            if (a0 * a1 < 0.0)
                {
                // simple crossing ..
                double f = -a0 / (a1 - a0);

                if (f > 1.0 - s_fractionTol && a1 >= 0.0)
                    {
                    // the endpoint will be saved -- avoid the duplicate
                    nearZero = true;
                    }
                else
                    {
                    DPoint3d xyzA = DPoint3d::FromInterpolate (xyz0, f, xyz1);
                    xyzIn.push_back (xyzA);
                    xyzOut.push_back (xyzA);
                    }
                    numSplit++;
                }
            if (a1 >= 0.0 || nearZero)
                xyzIn.push_back (xyz1);
            if (a1 <= 0.0 || nearZero)
                xyzOut.push_back (xyz1);
            xyz0 = xyz1;
            a0 = a1;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     02/17
+---------------+---------------+---------------+---------------+---------------+------*/
ValidatedSize ClipPlane::FindPointOnBothPlanes (bvector<DPoint3d> const &data, ClipPlaneCR plane0, ClipPlaneCR plane1, double tolerance)
    {
    for (size_t i = 0; i < data.size (); i++)
        {
        if (plane0.IsPointOn (data[i], tolerance) && plane1.IsPointOn (data[i], tolerance))
            return ValidatedSize (i, true);
        }
    return ValidatedSize (0, false);
    }
END_BENTLEY_GEOMETRY_NAMESPACE