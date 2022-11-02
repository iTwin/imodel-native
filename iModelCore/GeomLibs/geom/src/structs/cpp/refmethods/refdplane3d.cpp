/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*-----------------------------------------------------------------*//**
* @description Store origin and unnormalized vector.
* @param [in] x0 x-coordinate of origin point
* @param [in] y0 y-coordinate of origin point
* @param [in] z0 z-coordinate of origin point
* @param [in] ux x-coordinate of normal vector
* @param [in] uy y-coordinate of normal vector
* @param [in] uz z-coordinate of normal vector
* @group "DPlane3d Initialization"
+----------------------------------------------------------------------*/
DPlane3d DPlane3d::FromOriginAndNormal
(
double          x0,
double          y0,
double          z0,
double          ux,
double          uy,
double          uz
)
    {
    DPlane3d plane;
    plane.InitFromOriginAndNormal (x0, y0, z0, ux, uy, uz);
    return plane;
    }


/*-----------------------------------------------------------------*//**
* @description Store vector and distance. (Normalize the vector before storing)
* @group "DPlane3d Initialization"
+----------------------------------------------------------------------*/
ValidatedDPlane3d DPlane3d::FromNormalAndDistance
(
DVec3dCR normal,
double   distance
)
    {
    auto unitNormal = normal.ValidatedNormalize ();
    if (unitNormal.IsValid ())
        return DPlane3d::FromOriginAndNormal
            (
            DPoint3d::FromScale (unitNormal.Value (), distance),
            unitNormal.Value ()
            );

    return ValidatedDPlane3d (DPlane3d::FromOriginAndNormal (0,0,0,0,0,0));
    }



/*-----------------------------------------------------------------*//**
* @description Store origin and unnormalized vector.
* @param [in] origin origin point
* @param [in] normal normal vector
* @group "DPlane3d Initialization"
+----------------------------------------------------------------------*/
DPlane3d DPlane3d::FromOriginAndNormal
(
DPoint3dCR      origin,
DVec3dCR        normal
)
    {
    DPlane3d plane;
    plane.InitFromOriginAndNormal (origin, normal);
    return plane;
    }

/*-----------------------------------------------------------------*//**
* @description Initialize with first point as origin, normal as unnormalized cross product of vectors
*   to 2nd and 3rd points.
* @param [in] origin origin point
* @param [in] xPoint first point in plane (e.g., x-axis point)
* @param [in] yPoint second point in plane (e.g., y-axis point)
* @group "DPlane3d Initialization"
+----------------------------------------------------------------------*/
DPlane3d DPlane3d::From3Points
(
DPoint3dCR      origin,
DPoint3dCR      xPoint,
DPoint3dCR      yPoint
)
    {
    DPlane3d plane;
    plane.InitFrom3Points (origin, xPoint, yPoint);
    return plane;
    }



/*-----------------------------------------------------------------*//**
* @description Store origin and unnormalized vector.
* @param [out] pPlane initialized plane
* @param [in] x0 x-coordinate of origin point
* @param [in] y0 y-coordinate of origin point
* @param [in] z0 z-coordinate of origin point
* @param [in] ux x-coordinate of normal vector
* @param [in] uy y-coordinate of normal vector
* @param [in] uz z-coordinate of normal vector
* @group "DPlane3d Initialization"
* @bsimethod
+----------------------------------------------------------------------*/
void DPlane3d::InitFromOriginAndNormal
(

        double      x0,
        double      y0,
        double      z0,
        double      ux,
        double      uy,
        double      uz

)
    {
    origin.Init (x0, y0, z0);
    normal.Init (ux, uy, uz);
    }


/*-----------------------------------------------------------------*//**
* @description Store origin and unnormalized vector.
* @param [out] pPlane initialized plane.
* @param [in] origin origin point
* @param [in] normal normal vector
* @group "DPlane3d Initialization"
* @bsimethod
+----------------------------------------------------------------------*/
void DPlane3d::InitFromOriginAndNormal
(
DPoint3dCR _origin,
DVec3dCR _normal
)
    {
    origin = _origin;
    normal = _normal;
    }


/*-----------------------------------------------------------------*//**
* @description Normalize the plane vector.
* @param [in,out] pPlane plane to normalize
* @return true if normal vector has nonzero length.
* @group "DPlane3d Modification"
* @bsimethod
+----------------------------------------------------------------------*/
bool DPlane3d::Normalize ()
    {
    return 0.0 != normal.Normalize ();
    }



/*-----------------------------------------------------------------*//**
* @description Initialize with first point as origin, normal as unnormalized cross product of vectors
*   to 2nd and 3rd points.
* @param [out] pPlane initialized plane
* @param [in] origin origin point
* @param [in] xPoint first point in plane (e.g., x-axis point)
* @param [in] yPoint second point in plane (e.g., y-axis point)
* @group "DPlane3d Initialization"
* @bsimethod
+----------------------------------------------------------------------*/
void DPlane3d::InitFrom3Points
(
DPoint3dCR _origin,
DPoint3dCR xPoint,
DPoint3dCR yPoint
)
    {
    origin = _origin;
    normal.CrossProductToPoints (origin, xPoint, yPoint);
    }


/*-----------------------------------------------------------------*//**
* @description Extract origin and normal from 4D plane coefficients.
* @param [out] pPlane plane structure with origin, normal
* @param [in] hPlane 4D plane coefficients
* @return true if plane has a nonzero normal
* @see bsiDPlane3d_getDPoint4d
* @group "DPlane3d Initialization"
* @bsimethod
+----------------------------------------------------------------------*/
bool DPlane3d::Init (DPoint4dCR hPlane)
    {

    double aa = hPlane.x * hPlane.x + hPlane.y * hPlane.y + hPlane.z * hPlane.z;
    double fOrigin, fNormal;
    bool    normalOK = false;
    if (aa != 0.0)
        {
        fOrigin = -hPlane.w / aa;
        fNormal = 1.0 / sqrt (aa);
        origin.Init (
                        hPlane.x * fOrigin,
                        hPlane.y * fOrigin,
                        hPlane.z * fOrigin);
        normal.Init (
                        hPlane.x * fNormal,
                        hPlane.y * fNormal,
                        hPlane.z * fNormal);
        normalOK = true;
        }
    else
        Zero ();
    return normalOK;
    }


/*-----------------------------------------------------------------*//**
* @description Return the plane as a DPoint4d.
* @param [in] pPlane plane structure with origin, normal
* @param [out] hPlane 4D plane coefficients
* @see bsiDPlane3d_initFromDPoint4d
* @group "DPlane3d Queries"
* @bsimethod
+----------------------------------------------------------------------*/
void DPlane3d::GetDPoint4d (DPoint4dR hPlane) const
    {
    GetCoefficients (hPlane.x, hPlane.y, hPlane.z, hPlane.w);
    hPlane.w = -hPlane.w;
    }


/*-----------------------------------------------------------------*//**
* @description Convert the implicit plane ax+by+cz=d to origin-normal form, with a unit normal vector.
* @remarks WARNING: Check your usage.  It is about equally common to write the plane equation with
*       negated d, i.e. ax+by+cz+d=0.  If so, pass in (a,b,c,-d).
* @param [out] pPlane plane structure with origin, normal
* @param [in] a 4D plane x-coefficient
* @param [in] b 4D plane y-coefficient
* @param [in] c 4D plane z-coefficient
* @param [in] d 4D plane constant coefficient
* @return true if plane has a nonzero normal
* @see bsiDPlane3d_getImplicitPlaneCoefficients
* @group "DPlane3d Initialization"
* @bsimethod
+----------------------------------------------------------------------*/
bool DPlane3d::Init
(
double      a,
double      b,
double      c,
double      d
)
    {
    DPoint4d hCoffs;
    hCoffs.Init (a,b,c,-d);
    return Init (hCoffs);
    }


/*-----------------------------------------------------------------*//**
* @description Convert the plane to implicit coeffcients ax+by+cz=d.
* @remarks WARNING: Check your usage.  It is about equally common to write the plane equation with
*       negated d, i.e. ax+by+cz+d=0.  If so, pass in (a,b,c,-d).
* @param [in] pPlane plane structure with origin, normal
* @param [out] coffA 4D plane x-coefficient
* @param [out] coffB 4D plane y-coefficient
* @param [out] coffC 4D plane z-coefficient
* @param [out] coffD 4D plane constant coefficient
* @see bsiDPlane3d_initFromImplicitPlaneCoefficients
* @group "DPlane3d Initialization"
* @bsimethod
+----------------------------------------------------------------------*/
void DPlane3d::GetCoefficients
(
double      &coffA,
double      &coffB,
double      &coffC,
double      &coffD
) const
    {
    coffA = normal.x;
    coffB = normal.y;
    coffC = normal.z;
    coffD = origin.DotProduct (normal);
   }

#ifdef abc
/*-----------------------------------------------------------------*//**
* @description Apply a transformation to the source plane.
* @param [out] pDest transformed plane
* @param [in] transform transformation to apply
* @param [in] source source plane
* @group "DPlane3d Transform"
* @return false if the matrix part of the transform is singular.
* @bsimethod
+----------------------------------------------------------------------*/
bool DPlane3d::InitProduct
(

TransformCR transform,
DPlane3dCR source

)
    {
    RotMatrix matrix;
    /* Origin transforms as a point */
    transform.Multiply (origin, source.origin);
    /* Normal transforms as the inverse transpose of the matrix part. */
    /* BTW: If the matrix is orthogonal, this is a long way to multiply by the
         matrix part.  UGH. */
    transform.GetMatrix (matrix);
    return matrix.SolveTranspose (normal, source.normal);
    }
#endif

/*-----------------------------------------------------------------*//**
* @description Fill the plane data with zeros.
* @param [out] pDest initialized plane
* @group "DPlane3d Initialization"
* @bsimethod
+----------------------------------------------------------------------*/
void DPlane3d::Zero ()
    {
    origin.Zero ();
    normal.Zero ();
    }

ValidatedDPoint3d DPlane3d::Intersect3Planes
(
DVec3dCR normalA,
double distanceA,
DVec3dCR normalB,
double distanceB,
DVec3dCR normalC,
double distanceC
)
    {
    // normalA DOT X = distanceA
    RotMatrix matrix = RotMatrix::FromRowVectors (normalA, normalB, normalC);
    auto rightHandSide = DVec3d::From (distanceA, distanceB, distanceC);
    DVec3d solution;
    if (matrix.Solve (solution, rightHandSide))
        return ValidatedDPoint3d (solution, true);
    return ValidatedDPoint3d (DPoint3d::From (distanceA * normalA.x, distanceA * normalA.y, distanceA * normalA.z), false);
    }

ValidatedDPoint3d DPlane3d::Intersect3Planes
(
DPlane3dCR planeA,
DPlane3dCR planeB,
DPlane3dCR planeC
)
    {
    // (X-originA) DOT normalA = 0, i.e.     normalA DOT X = normalA dot originA
    RotMatrix matrix = RotMatrix::FromRowVectors (planeA.normal, planeB.normal, planeC.normal);
    auto rightHandSide = DVec3d::From
        (
        planeA.normal.DotProduct (planeA.origin),
        planeB.normal.DotProduct (planeB.origin),
        planeC.normal.DotProduct (planeC.origin)
        );
    DVec3d solution;
    if (matrix.Solve (solution, rightHandSide))
        return ValidatedDPoint3d (solution, true);
    return ValidatedDPoint3d (planeA.origin, false);
    }

/*-----------------------------------------------------------------*//**
* @description Test if the numeric entries in the plane are all absolutely zero (no tolerances).
* @param [in] pPlane plane to query
* @return true if the plane contains only zero coordinates.
* @group "DPlane3d Queries"
* @bsimethod
+----------------------------------------------------------------------*/
bool DPlane3d::IsZero () const
    {
    return
            origin.x == 0.0
        &&  origin.y == 0.0
        &&  origin.z == 0.0
        &&  normal.x == 0.0
        &&  normal.y == 0.0
        &&  normal.z == 0.0;
    }


/*-----------------------------------------------------------------*//**
* @description Project a (generally off-plane) point onto the plane.
* @param [in] pPlane plane to query
* @param [out] projection projection of point onto the plane
* @param [in] point point to project to plane
* @return true if the plane has a well defined normal.
* @group "DPlane3d Projection"
* @bsimethod
+----------------------------------------------------------------------*/
bool DPlane3d::ProjectPoint
(
DPoint3dR projection,
DPoint3dCR point
) const
    {
    DPoint3d V;
    double UdotU, UdotV, s = 0;
    bool    result;
    V.DifferenceOf (point, origin);
    UdotU = normal.MagnitudeSquared ();
    UdotV = normal.DotProduct (V);

    result = DoubleOps::SafeDivide (s, UdotV, UdotU, 0.0);
    projection.SumOf (point, normal, -s);
    return result;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double DPlane3d::Evaluate
(
DPoint3dCR point
) const
    {
    return point.DotDifference (origin, normal);
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DRange1d DPlane3d::EvaluateRange
(
DPoint3dCP points,
size_t     n,
size_t   &minIndex,
size_t   &maxIndex
) const
    {
    minIndex = maxIndex = 0;
    if (n <= 0)
        return DRange1d::NullRange ();
    double aMin = points[0].DotDifference (origin, normal);
    double aMax = aMin;
    for (size_t i = 1; i < n; i++)
        {
        double a = points[i].DotDifference (origin, normal);
        if (a > aMax)
            {
            maxIndex = i;
            aMax = a;
            }
        else if (a < aMin)
            {
            minIndex = i;
            aMin = a;
            }
        }
    return DRange1d::From (aMin, aMax);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DRange1d DPlane3d::EvaluateRange
(
bvector<DPoint3d> const &points,
size_t   &minIndex,
size_t   &maxIndex
) const
    {
    if (points.size () == 0)
        return DRange1d::NullRange ();
    return EvaluateRange (&points[0], points.size (), minIndex, maxIndex);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double DPlane3d::EvaluateMaxAbs
(
DPoint3dCP points,
size_t     n
) const
    {
    if (n <= 0)
        return 0.0;
    double aMax = fabs (points[0].DotDifference (origin, normal));
    for (size_t i = 1; i < n; i++)
        {
        aMax = DoubleOps::MaxAbs (points[i].DotDifference (origin, normal), aMax);
        }
    return aMax;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double DPlane3d::EvaluateMaxAbs (bvector <DPoint3d> const &points) const
    {
    if (points.size () == 0)
        return 0.0;
    return EvaluateMaxAbs (&points[0], points.size ());
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double DPlane3d::EvaluateVector (DVec3dCR vector) const
    {
    return vector.DotProduct (normal);
    }



/*-----------------------------------------------------------------*//**
* @description Approximate a plane through a set of points.
* @remarks The method used is:
    <ul>
    <li>Find the bounding box.</li>
    <li>Choose the axis with greatest range.</li>
    <li>Take two points that are on the min and max of this axis.</li>
    <li>Also take as a third point the point that is most distant from the line connecting the two extremal points.</li>
    <li>Form plane through these 3 points.</li>
    </ul>
* @param [out] plane computed plane 
* @param pPoint [out] point array
* @param numPoint [in] number of points
* @param toleranceIn [in] requested tolerance for consdering points too close to define vectors.
* @param toleranceOut [out] tolerance actually used --- the larger of toleranceIn and a relative tolerance from coordinate magnitudes.
* @return true if the points define a clear plane; false if every point lies on the line (within tolerance) joining the two extremal points.
* @group "DPoint3d Queries"
* @bsimethod
+----------------------------------------------------------------------*/
static bool     planeThroughPointsTol
(
DPlane3dR plane,
DPoint3dCP pPoint,
int             numPoint,
double          toleranceIn,
double          &toleranceOut
)
    {
    static double s_defaultRelTol = 1.0e-12;
    plane.Zero ();
    toleranceOut = toleranceIn;
    if (numPoint > 2)
        {
        plane.origin = pPoint[0];

        size_t mostDistantFrom0 = 1 + DPoint3dOps::MostDistantIndex (pPoint + 1, (size_t)(numPoint - 1), pPoint[0]);
        double maxCoordinate = DPoint3dOps::LargestCoordinate (pPoint, (size_t)numPoint);
        if (maxCoordinate < Angle::SmallAngle ())
            return false;
        double myTol = s_defaultRelTol * maxCoordinate;
        if (toleranceOut < myTol)
            toleranceOut = myTol;
        DVec3d vectorX = DVec3d::FromStartEnd (pPoint[0], pPoint[mostDistantFrom0]);

        double magX = vectorX.Magnitude ();
        if (magX < toleranceOut)
            return false;
        DVec3d vectorYMax, vectorY, vectorZ;
        vectorYMax.Zero ();
        size_t iMax = 0;
        double a, aMax = 0.0;
        for (size_t i = 1; i < (size_t)numPoint; i++)
            {
            vectorY.DifferenceOf (pPoint[i], pPoint[0]);
            vectorZ.CrossProduct (vectorX, vectorY);
            a = vectorZ.MagnitudeSquared ();
            if (a > aMax)
                {
                iMax = i;
                aMax = a;
                vectorYMax = vectorY;
                }
            }

        double magXmagYSine = plane.normal.NormalizedCrossProduct (vectorX, vectorYMax);
        double perpendicularDistance;
        if (DoubleOps::SafeDivide (perpendicularDistance, magXmagYSine, magX, 0.0)
           && perpendicularDistance > toleranceOut)
            return true;
        }
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DPlane3d::InitFromArray (DPoint3dCP pointArray, int numPoint, double &maxAbsDistance)
    {
    double effectiveTolerance;
    if (planeThroughPointsTol (*this, pointArray, numPoint, 0.0, effectiveTolerance))
        {
        maxAbsDistance = EvaluateMaxAbs (pointArray, numPoint);
        return true;
        }
    
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DPlane3d::InitFromArray (bvector<DPoint3d> const &in, double &maxAbsDistance)
    {
    if (in.size () == 0)
        return false;
    return InitFromArray (&in[0], (int)in.size (), maxAbsDistance);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DPlane3d::InitFromArray (bvector<DPoint3d> const &in)
    {
    if (in.size () == 0)
        return false;
    double distance;
    return InitFromArray (&in[0], (int)in.size (), distance);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DPlane3d::InitFromArray (DPoint3dCP pointArray, int numPoint)
    {
    double effectiveTolerance;
    if (planeThroughPointsTol (*this, pointArray, numPoint, 0.0, effectiveTolerance))
        {
        double maxAbsDistance = EvaluateMaxAbs (pointArray, numPoint);
        return maxAbsDistance <= effectiveTolerance;
        }
    
    return false;
    }

ValidatedDPlane3d DPlane3d::VoronoiSplitPlane (DPoint3dCR xyzA, double rA, DPoint3dCR xyzB, double rB, int voronoiMetric)
    {
    auto  unitAB = (xyzB-xyzA).ValidatedNormalize ();
    if (unitAB.IsValid ())
        {
        DPoint3d origin;
        if (voronoiMetric == 1)
            {
            DPoint3d shiftA = xyzA + rA * unitAB.Value ();
            DPoint3d shiftB = xyzB - rB * unitAB.Value ();
            origin = DPoint3d::FromInterpolate (shiftA, 0.5, shiftB);
            }
        else if (voronoiMetric == 2)
            {
            origin = DPoint3d::FromWeightedAverage (xyzA, rB, xyzB, rA);    // yes, switch the radii to make large one get more space.
            }
        else if (voronoiMetric == 3)
            {
            // distance = distance to tangency = sqrt (euclideandistance^2 - radius^2)
            // equal distance split of chord is
            //    x = distance from A
            //    c-x = distance from B (c == distance A to B)
            //  x^2 - rA^2 = (c-x)^2 - rB^2
            //  x = (c^2-rB^2+rA^2)/(2c)
            // fractional    x=x/c = (c^2-rB^2+rA^2)/(2c^2)
            double cc = xyzA.DistanceSquared (xyzB);
            //double c = sqrt (cc);
            double numerator = (cc + rA * rA - rB * rB);
            auto f = DoubleOps::ValidatedDivide (numerator, 2.0 * cc, 0.5);
            origin = DPoint3d::FromInterpolate (xyzA, f.Value (), xyzB);
            }
        else
            origin = DPoint3d::FromInterpolate (xyzA, 0.5, xyzB);
        return ValidatedDPlane3d (DPlane3d::FromOriginAndNormal (origin, unitAB.Value ()), true);
        }
    return ValidatedDPlane3d (DPlane3d::FromOriginAndNormal (xyzA, DVec3d::From (0,0,0)), false);
    }
END_BENTLEY_GEOMETRY_NAMESPACE
