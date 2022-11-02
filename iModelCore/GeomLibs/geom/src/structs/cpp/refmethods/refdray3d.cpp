/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#define OMDLVECTOR_DEFAULT_CLOSEST_APPROACH_MAX_PARAMETER 1.0E10
/*-----------------------------------------------------------------*//**
* Sets pProjection to the projection of pPoint onto the infinite
* ray through pStart in the direction of pDir, and sets lambdaP to the parametric
* coordinate of the projection point.
*
* @param pProjection <= Projection of pPoint on ray
* @param lambdaP <= Parametric coordinates of projection
* @param pPoint => Point to be projected
* @param pStart => Ray start
* @param pDir => Ray direction
* @return true unless direction vector has zero length.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiGeom_projectPointToRay

(
DPoint3dP pProjection,
double      *lambdaP,
DPoint3dCP pPoint,
DPoint3dCP pStart,
DPoint3dCP pDir
)
    {
    DPoint3d    uVector, vVector;
    double      dotUU, dotUV, lambda;

    uVector = *pDir;
    vVector = *pPoint - *pStart;//    bsiDPoint3d_subtractDPoint3dDPoint3d (&vVector, pPoint, pStart);
    dotUU = uVector.DotProduct (uVector);//bsiDPoint3d_dotProduct (&uVector, &uVector);
    dotUV = uVector.DotProduct (vVector); //bsiDPoint3d_dotProduct (&uVector, &vVector);

    if (dotUU == 0.0)
        {
        if (pProjection)
            *pProjection = *pStart;
        if (lambdaP)
            *lambdaP = 0.0;
        return  false;
        }


    lambda = dotUV / dotUU;
    if (pProjection)
        pProjection->SumOf (*pStart, uVector, lambda);//        bsiDPoint3d_addScaledDPoint3d (pProjection, pStart, &uVector, lambda);
    if (lambdaP)
        *lambdaP = lambda;

    return true;
    }

/*-----------------------------------------------------------------*//**
* Compute the line of intersection between two planes.
*
* @param pLineStart <= start point of line
* @param pLineDirection <= direction vector
* @param pOriginA => any point on  plane A
* @param pNormalA => normal vector for plane A
* @param pOriginB => any point on plane B
* @param pNormalB => normal vector for plane B
* @return true if planes have simple intersection
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiGeom_planePlaneIntersection

(
DPoint3dP pLineStart,
DPoint3dP pLineDirection,
DPoint3dCP pOriginA,
DPoint3dCP pNormalA,
DPoint3dCP pOriginB,
DPoint3dCP pNormalB
)
    {
    bool    boolStat;
    DPoint3d intersectionDirection;
    DPoint3d inPlaneA;
    double   param;
    DPoint3d intPoint;
    double sine, cosine;
    intPoint.Zero ();
    double a = intersectionDirection.NormalizedCrossProduct (*pNormalA, *pNormalB);
    double b = 1.0 + pNormalA->MaxAbs () + pNormalB->MaxAbs ();

    inPlaneA.CrossProduct (intersectionDirection, *pNormalA);//    bsiDPoint3d_crossProduct (&inPlaneA, &intersectionDirection, pNormalA);
    sine = intersectionDirection.Magnitude ();
    cosine = fabs (pNormalA->DotProduct (*pNormalB));//bsiDPoint3d_dotProduct (pNormalA, pNormalB));
    double relTol = Angle::SmallAngle ();
    boolStat = ( sine > relTol * cosine
               && a > relTol * b
               )
             ? bsiGeom_rayPlaneIntersection
                    (
                    &param,
                    &intPoint,
                    pOriginA,
                    &inPlaneA,
                    pOriginB,
                    pNormalB
                    )
             : false;

    if (boolStat)
        {
        *pLineStart = intPoint;
        *pLineDirection = intersectionDirection;

#if defined (TEST_PLANE_PLANE_INTERSECT)
            {
            double hA, hB;
            hA = bsiDPoint3d_dotDifference (pLineStart, pOriginA, pNormalA);
            hB = bsiDPoint3d_dotDifference (pLineStart, pOriginB, pNormalA);
            if (fabs(hA) > 0.001 || fabs(hB) > 0.001)
                {
                double dummy;
                dummy = hA;
                }
            }
#endif
        }
    else

        {
        /* fill in originA as the point, intersection direction as best available */
        *pLineStart = *pOriginA;
        *pLineDirection = intersectionDirection;
        }
    return  boolStat;
    }

/*-----------------------------------------------------------------*//**
* @description Compute the intersection point of a ray and a plane.
*
* @param pParam <= intersection parameter within line
* @param pPoint <= intersection point
* @param pLineStart => origin of ray
* @param pLineTangent => direction vector of ray
* @param pOrigin => any point on plane
* @param pNormal => normal vector for plane
* @return true unless the ray is parallel to the plane.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiGeom_rayPlaneIntersection

(
double      *pParam,
DPoint3dP pPoint,
DPoint3dCP pLineStart,
DPoint3dCP pLineTangent,
DPoint3dCP pOrigin,
DPoint3dCP pNormal
)
    {
    bool    boolStat;
    double dot1, dot2, param;
    static double maxFactor = 1.0e12;

    /* Plane: (X - Xorg) dot N = 0
       Line:   X = Xstart + s * T
       Intersection: (Xstart + s * T -Xorg) dot N = 0
                    s * (T dot N) = (Xorg - Xstart) dot N)
                    s = ((Xorg - Xstart) dot N )/ (T dot N)
    */
    dot2 = pLineTangent->DotProduct (*pNormal);// bsiDPoint3d_dotProduct (pLineTangent, (DVec3d const*) pNormal);
    dot1 = pOrigin->DotDifference (*pLineStart, *(DVec3d const*)pNormal);// bsiDPoint3d_dotDifference (pOrigin,  pLineStart, (DVec3d const*) pNormal);

    if (fabs(dot1) < maxFactor * fabs(dot2))
        {
        param = dot1 / dot2;
        boolStat = true;
        }
    else
        {
        param = 0.0;
        boolStat = false;
        }

    if (pParam)
        *pParam = param;
    if (pPoint)
        pPoint->SumOf (*pLineStart, *pLineTangent, param);// bsiDPoint3d_addScaledDPoint3d (pPoint, pLineStart, pLineTangent, param);
    return  boolStat;
    }


/*-----------------------------------------------------------------*//**
* finds the point of closest approach between two (possibly skewed,
* nonintersecting) straight lines in 3 space.  The lines are describe
* by two endpoints.  The return points are described by their parametric
* and cartesian coordinates on the respective lines.
* Input maxParam is the largest parameter value permitted in the output
* points.    If only points within the lines are ever of interest, send
* a smallish maxParam, e.g. 10 or 100.  A zero parameter value indicates
* large parameters are acceptable.
* indicated no point (which might be due to parallel lines or to
* the intersection point being far from the points.)
*
* @param pParamA <= parametric coordinate on line A
* @param pParamB <= parametric coordinate on line B
* @param pPointAP <= closest point on A. May be NULL
* @param pPointBP <= closest point on B. May be NULL
* @param pStartA => start point of line A
* @param pTangentA => direction vector of line A
* @param pStartB => start point of line B
* @param pTangentB => direction vector of line B
* @return true unless rays are parallel
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiGeom_closestApproachOfRays

(
double      *pParamA,
double      *pParamB,
DPoint3dP pPointAP,
DPoint3dP pPointBP,
DPoint3dCP pStartA,
DPoint3dCP pTangentA,
DPoint3dCP pStartB,
DPoint3dCP pTangentB
)
    {
    DPoint3d    deltaAB;
    DPoint3d normalizedTangentA, normalizedTangentB;
    DPoint2d    A,B,C;
    bool        result;
    double      paramA, paramB;

    deltaAB.DifferenceOf (*pStartB, *pStartA);

    /* Define a point parametrically on each line:
        pointA = startA + paramA*tangentA

        pointB = startB + paramB*tangentB
       Join the points by a vector:
        vec = deltaAB + paramB*tangentB - paramA*tangentA
       We want vec to be perpendicular to both tangent vectors:
        (deltaAB + paramB*tangentB - paramA*tangentA) DOT tangentA = 0

(deltaAB + paramB*tangentB - paramA*tangentA) DOT tangentB = 0
       or
        deltaAB DOT tangentA = paramB*(-tangentB DOT tangentA) + paramA*(tangentA DOT tangentA)

        deltaAB DOT tangentB = paramB*(-tangentB DOT tangentB) + paramA*(tangentA DOT tangentB)
       or, as a matrix equation:
        [ A B ] [ paramA ] = [C]
                [ paramB ]
        where A,B,C are two component vectors
        To control condition number, use normalized tangent vectors to the right
            of the DOT.  (Leave unnormalized on left to preserve parameter ranges).
    */
    if (normalizedTangentA.Normalize (*pTangentA) == 0.0
        || normalizedTangentB.Normalize (*pTangentB) == 0.0)
        {
        result = false;
        }
    else
        {
        C.x =  deltaAB.DotProduct (normalizedTangentA);
        C.y =  deltaAB.DotProduct (normalizedTangentB);
        A.x =  pTangentA->DotProduct (normalizedTangentA);
        A.y =  pTangentA->DotProduct (normalizedTangentB);
        B.x = -pTangentB->DotProduct (normalizedTangentA);
        B.y = -pTangentB->DotProduct (normalizedTangentB);

        if (!bsiSVD_solve2x2 (&paramA, &paramB, A.x, B.x, A.y, B.y, C.x, C.y))
            {
            result = false;
            }
        else
            {
            if (pParamA)
                *pParamA = paramA;
            if (pParamB)
                *pParamB = paramB;
            if (pPointAP)
                pPointAP->SumOf (*pStartA, *pTangentA, paramA);
            if (pPointBP)
                pPointBP->SumOf (*pStartB, *pTangentB, paramB);
            result = true;
            }
        }

    return  result;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DRay3d DRay3d::From (DSegment3dCR segment)
    {
    DRay3d ray;
    ray.InitFromOriginAndTarget (segment.point[0], segment.point[1]);
    return ray;
    }

//! Return a ray with origin interpolated between points.   The ray direction is the point0 to point1 vector scaled by vectorScale
DRay3d DRay3d::FromIinterpolateWithScaledDifference (DPoint3dCR point0, double fraction, DPoint3dCR point1, double vectorScale)
    {
    DRay3d ray;
    ray.origin.Interpolate (point0, fraction, point1);
    ray.direction = (point1 - point0) * vectorScale;
    return ray;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DRay3d DRay3d::FromOriginAndVector (DPoint3dCR origin, DVec3dCR vector)
    {
    DRay3d ray;
    ray.InitFromOriginAndVector (origin, vector);
    return ray;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DRay3d DRay3d::FromOriginAndTarget (DPoint2dCR point0, DPoint2dCR point1)
    {
    DRay3d ray;
    ray.InitFromOriginAndTarget (point0, point1);
    return ray;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DRay3d DRay3d::FromOriginAndTarget (DPoint3dCR point0, DPoint3dCR point1)
    {
    DRay3d ray;
    ray.InitFromOriginAndTarget (point0, point1);
    return ray;
    }


//! Return dot product of (unnormalized) ray vector with vector from ray origin to given point.
double DRay3d::DirectionDotVectorToTarget (DPoint3dCR target) const
    {
    return  (target.x - origin.x) * direction.x
          + (target.y - origin.y) * direction.y
          + (target.z - origin.z) * direction.z;
    }

//! Return dot product of (unnormalized) ray vector with input vector.
double DRay3d::DirectionDotVector (DVec3d vector) const
    {
    return  vector.x * direction.x
          + vector.y * direction.y
          + vector.z * direction.z;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DRay3d::InitFrom (DSegment3dCR segment)
    {
    origin = segment.point[0];
    direction.DifferenceOf (segment.point[1], segment.point[0]);
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DRay3d::InitFromOriginAndTarget (DPoint2dCR point0, DPoint2dCR point1)
    {
    this->origin.x = point0.x;
    this->origin.y = point0.y;
    this->direction.x = point1.x - point0.x;
    this->direction.y = point1.y - point0.y;
    this->origin.z = this->direction.z = 0.0;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DRay3d::InitFromOriginAndTarget (DPoint3dCR point0, DPoint3dCR point1)
    {
    origin = point0;
    direction.DifferenceOf (point1, point0);
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DRay3d::InitFromOriginAndVector (DPoint3dCR _origin, DVec3dCR _vector)
    {
    origin = _origin;
    direction = _vector;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DRay3d::InitFromPlanePlaneIntersection (DPlane3dCR planeA, DPlane3dCR planeB)
    {
    return bsiGeom_planePlaneIntersection (
            &origin, &direction,
            &planeA.origin, &planeA.normal,
            &planeB.origin, &planeB.normal
            );
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DRay3d::EvaluateEndPoints
(
DPoint3dR point0,
DPoint3dR point1
) const
    {
    point0 = origin;
    point1.SumOf (origin, direction);
    }



//! @param [in] param fractional parameter
//! @return evaluated point.
DPoint3d DRay3d::FractionParameterToPoint (double  param) const
    {
    return DPoint3d::FromSumOf (origin, direction, param);
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DRay3d::ProjectPointBounded
(

DPoint3dR closestPoint,
double   &closestParam,
DPoint3dCR spacePoint

) const
    {
    bool stat = ProjectPointUnbounded (closestPoint, closestParam, spacePoint);
    if (closestParam < 0.0)
        {
        closestPoint = origin;
        closestParam = 0.0;
        }
    else if (closestParam > 1.0)
        {
        closestParam = 1.0;
        closestPoint = DPoint3d::FromSumOf (origin, direction, 1.0);
        }
    return stat;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DRay3d::ProjectPointUnbounded
(
DPoint3dR closestPoint,
double   &closestParam,
DPoint3dCR spacePoint
) const
    {
    bool    result;
    result = DoubleOps::SafeDivide
                (
                closestParam,
                spacePoint.DotDifference (origin, direction),
                direction.DotProduct (direction), 0.0
                );
    closestPoint.SumOf (origin, direction, closestParam);
    return result;
    }




#ifdef abc
/*-----------------------------------------------------------------*//**
*
* Apply a transformation to the source ray.
* @param [in] transform transformation to apply.
* @param [in] source source ray
* @indexVerb transform
* @bsimethod
+----------------------------------------------------------------------*/
bool DRay3d::InitProduct
(

TransformCR transform,
DRay3dCR source

)
    {
    transform.Multiply (origin, source.origin);
    transform.MultiplyMatrixOnly (direction, source.direction);
    return true;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* Return the intersection of the (unbounded) ray with a plane.
* @param [out] intPoint intersection point
* @param [out] intParam parameter along the ray
* @param [in] plane plane (origin and normal)
* @return false if ray, plane are parallel.
* @indexVerb
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DRay3d::Intersect
(
DPoint3dR intPoint,
double          &intParam,
DPlane3dCR plane
) const
    {
    DVec3d vectorA = DVec3d::FromStartEnd (plane.origin, origin);
    double UdotN = direction.DotProduct(plane.normal);
    double AdotN = vectorA.DotProduct (plane.normal);
    bool result = DoubleOps::SafeDivide (intParam, -AdotN, UdotN, 0.0);
    intPoint.SumOf (origin, direction, intParam);
    return result;
    }



//! Return the intersection of this ray with a specified z plane of a coordinate frame.
//! @param [in] frame coordinate frame, e.g. often called localWorldFrame for an object
//! @param [in] localZ z coordinate in local frame (e.g. 0 for local xy plane pierce point.)
//! @return true if the ray is not parallel to the plane
//! @param [out] uvw local uvw coordinates.  (w matches input z)
//! @param [out] t parameter along ray.
bool DRay3d::IntersectZPlane
(
TransformR frame,
double     localZ,
DPoint3dR  uvw,
double     &t
) const
    {
    DPoint3d frameOrigin;
    DVec3d frameU, frameV, frameW;
    frame.GetOriginAndVectors (frameOrigin, frameU, frameV, frameW);
    DPoint3d rightHandSide, uvNegativeT;
    // To Solve:
    //    frameOrigin + u*frame.U + v*frame.V + localZ*frame.W = origin + t * direction
    //    [frame.U  frame.V direction][u,v,-t]^ = origin - frameOrigin - localZ * frame.W
    rightHandSide.SumOf (origin, frameOrigin, -1.0, frameW, -localZ);
    RotMatrix system = RotMatrix::FromColumnVectors (frameU, frameV, direction);
    if (system.Solve (uvNegativeT, rightHandSide))
        {
        uvw.x = uvNegativeT.x;
        uvw.y = uvNegativeT.y;
        uvw.z = localZ;
        t     = - uvNegativeT.z;
        return true;
        }
    uvw.Zero ();
    t = 0.0;
    return false;
    }



/*---------------------------------------------------------------------------------**//**
* Return the intersection of the (unbounded) ray with a circle, using only
* xy coordinates.
* @param [out] intPoint 0, 1, or 2 intersection points.
* @param [out] intParam 0, 1, or 2 intersection parameters.
* @param [in] center circle center.
* @param [in] radius circle radius.
* @return   number of intersections.
* @indexVerb
* @bsimethod
+--------------------------------------------------------------------------------------*/
int DRay3d::IntersectCircleXY
(
DPoint3dP intPoint,
double          *intParam,
DPoint3dCR center,
double          radius
) const
    {
    DVec3d V;
    double a, b, c;
    int numSolution;
    double param[2];
    int i;
    V.DifferenceOf (origin, center);
    a = direction.DotProductXY (direction);
    b = 2.0 * direction.DotProductXY (V);
    c = V.MagnitudeSquared () - radius * radius;

    numSolution = bsiMath_solveQuadratic (param, a, b, c);
    if (numSolution == 2 && param[0] > param[1])
        std::swap (param[0], param[1]);
    for (i = 0; i < numSolution; i++)
        {
        if (intParam)
            intParam[i] = param[i];
        if (intPoint)
            intPoint[i].SumOf (origin, direction, param[i]);
        }
    return numSolution;
    }


/*---------------------------------------------------------------------------------**//**
* Return the intersection of the (unbounded) ray with a circle, using only
* xy coordinates.
* @param [out] intPoint 0, 1, or 2 intersection points.
* @param [out] pRayParam 0, 1, or 2 intersection parameters on line
* @param [out] pPatchParam 0, 1, or 2 s,t patch parameter pairs.
* @return   number of intersections.
* @indexVerb
* @bsimethod
+--------------------------------------------------------------------------------------*/
int DRay3d::IntersectHyperbolicParaboloid
(
DPoint3dP intPoint,
double          *pRayParameter,
DPoint2dP pPatchParameter,
DPoint3dCR point00,
DPoint3dCR point10,
DPoint3dCR point01,
DPoint3dCR point11
) const
    {
    /* The RAY is X = A + lambda * R
       The PATCH is X = P00 + s U + t V + st W
       The two points are equal when
            -lambda R + (P00 - A) + sU + tV + st W = 0
            -lambda R + C + sU + tV + st W = 0
        Do a rotation so Ry = Rz = 0.
        Then the y, z equations are a bilinear equation in s,t.
        Solve it.
        With the known s,t, back out lambda from the x equation.
                lambda = (C.x + s U.x + t V.x + st W.x) / R.x
    */
    double cc, ss;
    int i;
    int numRoot = 0;
    DVec3d U, V, W, R, C;
    DPoint3d D; 
    double sbuf[2], tbuf[2];
    U.DifferenceOf (point10, point00);
    V.DifferenceOf (point01, point00);
    D.SumOf (point10, V);
    W.DifferenceOf (point11, D);
    R = this->direction;
    C.DifferenceOf (point00, this->origin);
    /* Rotate Ry into Rx, and apply to all others. */
    Angle::ConstructGivensWeights (cc, ss, R.x, R.y);
        Angle::ApplyGivensWeights (R.x, R.y, R.x, R.y, cc, ss);
        Angle::ApplyGivensWeights (C.x, C.y, C.x, C.y, cc, ss);
        Angle::ApplyGivensWeights (U.x, U.y, U.x, U.y, cc, ss);
        Angle::ApplyGivensWeights (V.x, V.y, V.x, V.y, cc, ss);
        Angle::ApplyGivensWeights (W.x, W.y, W.x, W.y, cc, ss);
    /* Rotate Ry into Rx, and apply to all others. */
    Angle::ConstructGivensWeights (cc, ss, R.x, R.z);
        Angle::ApplyGivensWeights (R.x, R.z, R.x, R.z, cc, ss);
        Angle::ApplyGivensWeights (C.x, C.z, C.x, C.z, cc, ss);
        Angle::ApplyGivensWeights (U.x, U.z, U.x, U.z, cc, ss);
        Angle::ApplyGivensWeights (V.x, V.z, V.x, V.z, cc, ss);
        Angle::ApplyGivensWeights (W.x, W.z, W.x, W.z, cc, ss);

    bsiMath_solveBilinear
                    (
                    sbuf, tbuf, &numRoot,
                    W.y, U.y, V.y, C.y,
                    W.z, U.z, V.z, C.z
                    );

    for (i = 0; i < numRoot; i++)
        {
        double lambda, s, t;
        s = sbuf[i];
        t = tbuf[i];
        if (!DoubleOps::SafeDivide (lambda,
                            C.x + s * U.x + t * V.x + s * t * W.x,
                            R.x,
                            0.0))
            return 0;
        if (pRayParameter)
            pRayParameter[i] = lambda;
        if (pPatchParameter)
            {
            pPatchParameter[i].x = s;
            pPatchParameter[i].y = t;
            }
        if (intPoint)
            {
            intPoint[i].SumOf (this->origin, this->direction, lambda);
            }
        }
    return numRoot;
    }

/* PatchCoffs is coefficients for "Ray intersect hyperbolic paraboloid".
This is a matrix with 4 rows and 6 columns.
*/
#define RIHPNumRow 4
#define RIHPNumCol 6
struct PatchCoffs
    {
#ifdef DeclareFillArrays
#define NumFill 6
    double fill0[NumFill];
#endif
    double coffs[RIHPNumRow][RIHPNumCol];
#ifdef DeclareFillArrays
    double fill1[NumFill];
#endif
    void Zero ()
        {
#ifdef DeclareFillArrays
        for (int i = 0; i < NumFill; i++)
            {
            fill0[i] = 0.0;
            fill1[i] = 0.0;
            }
#endif
        for (int i = 0; i < RIHPNumRow; i++)
            for (int j = 0; j < RIHPNumCol; j++)
                coffs[i][j] = 0.0;
        }
    PatchCoffs ()
        {
        Zero ();
        }
    };
typedef enum
    {
    ColA   = 0,
    ColR   = 1,
    ColP00 = 2,
    ColU   = 3,
    ColV   = 4,
    ColW   = 5
    } RIHPColumn;



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
static void rihp_loadCoffs(
PatchCoffs *pCoffs,
DPoint3dCP pXYZ,
double                  weight,
RIHPColumn              column
)
    {
    if (0 <= column && column < RIHPNumCol)
        {
        pCoffs->coffs[0][column] = weight;
        pCoffs->coffs[1][column] = pXYZ->x;
        pCoffs->coffs[2][column] = pXYZ->y;
        pCoffs->coffs[3][column] = pXYZ->z;
        }
    }

/*---------------------------------------------------------------------------------*//*
* In col0, compute rotations for rows from pivot+1 to end into (pivot,pivot).
* Do the same rotations in column range pivot to end.
*----------------------------------------------------------------------------------*/
static void rihp_rotateToPivot

(
PatchCoffs *pCoffs,
uint32_t    pivot
)
    {
    uint32_t row, col;
    double cc, ss;
    if (   pivot + 1 < RIHPNumRow
        && pivot < RIHPNumRow // security checker needs both of these
        )
        {
        for (row = pivot + 1; row < RIHPNumRow; row++)
            {
            Angle::ConstructGivensWeights
                            (
                            cc, ss,
                            pCoffs->coffs[pivot][pivot],
                            pCoffs->coffs[row][pivot]
                            );

            for (col = pivot; col < RIHPNumCol; col++)
                Angle::ApplyGivensWeights
                            (
                            pCoffs->coffs[pivot][col],
                            pCoffs->coffs[row][col],
                            pCoffs->coffs[pivot][col],
                            pCoffs->coffs[row][col],
                            cc, ss);
            }
        }
    }

#ifndef MinimalRefMethods


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DRay3d::IntersectPlane
(
DEllipse3d      ellipse,
DPoint3dR       intersectionPoint,
double &        rayParameter,
DPoint2dR        ellipsePlaneCoordinates
) const
    {
    // Ray: X = origin + s * direction
    // Plane: X = center + u * vector0 + v * vector90
    // [vector0 vector90 direction] (u,v,-s)^ = origin - center
    DVec3d rhs = DVec3d::FromStartEnd (ellipse.center, origin);
    RotMatrix matrix = RotMatrix::FromColumnVectors (
                ellipse.vector0, ellipse.vector90, direction);
    DVec3d uvNegS;
    if (matrix.Solve (uvNegS, rhs))
        {
        rayParameter = -uvNegS.z;
        intersectionPoint.SumOf (origin, direction, rayParameter);
        ellipsePlaneCoordinates = DPoint2d::From (uvNegS.x, uvNegS.y);
        return true;
        }
    rayParameter = 0.0;
    DPoint3d uvw;
    ellipse.PointToXYLocal (uvw, origin);
    ellipsePlaneCoordinates.x = uvw.x;
    ellipsePlaneCoordinates.y = uvw.y;
    intersectionPoint.SumOf (ellipse.center,
                ellipse.vector0, ellipsePlaneCoordinates.x,
                ellipse.vector90, ellipsePlaneCoordinates.y);
                
    return false;
    }

#endif



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
int DRay3d::IntersectHyperbolicParaboloid
(
DPoint3dP intPoint,
double    *rayParameter,
DPoint2dP patchParameter,
DPoint3dCR point00,
double          w00,
DPoint3dCR point10,
double          w10,
DPoint3dCR point01,
double          w01,
DPoint3dCR point11,
double          w11
) const
    {
    /* The RAY is X = A + lambda * R
       The PATCH is X = P00 + s U + t V + st W
       These has 3 unknowns and 4 equations (weighted!!!).
       We want a set of lambda, s, t where some MULTIPLE alpha times the ray point
        is identical to the patch in all 4 dimensions, hence
        alpha A + alpha lambda R = P00 + s U + t V + s t W
        Do three rotations to zero out A.x, A.y, A.z, throwing the magnitude of the A
            components into A.w.  (Note that this makes R.w nonzero.  I'm not sure I like
            the numerical implications of this, even if the rotations are orthogonal.)
        Do more rotations so R.y = R.z = 0.
        We now have
        alpha A.w + alpha lambda R.w = P00.w + s U.w + t V.w + s t W.w
                    alpha lambda R.x = P00.x + s U.x + t V.x + s t W.x
                             0       = P00.y + s U.y + t V.y + s t W.y
                             0       = P00.z + s U.z + t V.z + s t W.z
        Solve the y,z equation for s and t.  Bilinear equation solver does the details.
        Solve the x equation for product mu = alpha lambda
        Solve the w equation for alpha
        Finally lambda = mu / alpha
    */
    int i;
    int numOut  = 0;
    int numRoot = 0;
    PatchCoffs coffs;
    double sbuf[2], tbuf[2];
    DVec3d U, V, W;
    DPoint3d D;
    double dw10 = w10 - w00;
    double dw01 = w01 - w00;
    double dw11 = (w11 - w00) - (dw10 + dw01);
    U.DifferenceOf (point10, point00);
    V.DifferenceOf (point01, point00);
    D.SumOf (point10, V);
    W.DifferenceOf (point11, D);

    rihp_loadCoffs (&coffs, &this->origin, 1.0, ColA);
    rihp_loadCoffs (&coffs, &this->direction, 0.0, ColR);
    rihp_loadCoffs (&coffs, &point00, w00, ColP00);
    rihp_loadCoffs (&coffs, &U,  dw10, ColU);
    rihp_loadCoffs (&coffs, &V,  dw01, ColV);
    rihp_loadCoffs (&coffs, &W, dw11, ColW);

    rihp_rotateToPivot (&coffs, 0);
    rihp_rotateToPivot (&coffs, 1);

    bsiMath_solveBilinear
                    (
                    sbuf, tbuf, &numRoot,
                    coffs.coffs[2][ColW], coffs.coffs[2][ColU], coffs.coffs[2][ColV], coffs.coffs[2][ColP00],
                    coffs.coffs[3][ColW], coffs.coffs[3][ColU], coffs.coffs[3][ColV], coffs.coffs[3][ColP00]
                    );

    for (i = 0; i < numRoot; i++)
        {
        double lambda, alpha, mu, s, t;
        s = sbuf[i];
        t = tbuf[i];
        if  (  DoubleOps::SafeDivide (mu,
                    coffs.coffs[1][ColP00]
                        + s * coffs.coffs[1][ColU]
                        + t * coffs.coffs[1][ColV]
                        + s * t * coffs.coffs[1][ColW],
                    coffs.coffs[1][ColR],
                    0.0)
            && DoubleOps::SafeDivide (alpha,
                    coffs.coffs[0][ColP00]
                        + s * coffs.coffs[0][ColU]
                        + t * coffs.coffs[0][ColV]
                        + s * t * coffs.coffs[0][ColW]
                        - mu * coffs.coffs[0][ColR],
                    coffs.coffs[0][ColA],
                    0.0)
            && DoubleOps::SafeDivide (lambda, mu, alpha, 0.0)
            )
            {
            if (rayParameter)
                rayParameter[numOut] = lambda;
            if (patchParameter)
                {
                patchParameter[numOut].x = s;
                patchParameter[numOut].y = t;
                }
            if (intPoint)
                {
                intPoint[numOut].SumOf (this->origin, this->direction, lambda);
                }
            numOut++;
            }
        }
    return numOut;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DRay3d::ClosestApproachUnboundedRayBoundedSegment
(
double &fractionRay,
double &fractionSegment,
DPoint3dR pointRay,
DPoint3dR pointSegment,
DRay3dCR ray,
DSegment3dCR segment
)
    {
    DRay3d rayB;
    rayB.origin = segment.point[0];
    rayB.direction = DVec3d::FromStartEnd (segment.point[0], segment.point[1]);
    if (ClosestApproachUnboundedRayUnboundedRay (fractionRay, fractionSegment,
                    pointRay, pointSegment,
                    ray, rayB)
        && fractionSegment >= 0.0
        && fractionSegment <= 1.0
        )
        {
        // Take it!!
        }
    else if (fractionSegment >= 1.0)
        {
        fractionSegment = 1.0;
        pointSegment = segment.point[1];
        }
    else
        {
        fractionSegment = 0.0;
        pointSegment = segment.point[0];
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
ValidatedDRay3d DRay3d::ValidatedNormalize () const
    {
    ValidatedDVec3d unitTangent = direction.ValidatedNormalize();
    DRay3d ray = *this;
    if (unitTangent.IsValid ())
        {
        ray.direction = unitTangent;
        return ValidatedDRay3d (ray, true);
        }
    return ValidatedDRay3d (*this, false);
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DRay3d::ClosestApproachUnboundedRayUnboundedRay
(
double &fractionA,
double &fractionB,
DPoint3dR pointA,
DPoint3dR pointB,
DRay3dCR rayA,
DRay3dCR rayB
)
    {
    DPoint3d    deltaAB;
    DPoint3d    normalizedTangentA, normalizedTangentB;
    DPoint2d    A,B,C;
    bool        result = false;
    fractionA = 0.0;
    fractionB = 0.0;
    deltaAB.DifferenceOf (rayB.origin, rayA.origin);

    /* Define a point parametrically on each line:
        pointA = startA + paramA*tangentA

        pointB = startB + paramB*tangentB
       Join the points by a vector:
        vec = deltaAB + paramB*tangentB - paramA*tangentA
       We want vec to be perpendicular to both tangent vectors:
        (deltaAB + paramB*tangentB - paramA*tangentA) DOT tangentA = 0

(deltaAB + paramB*tangentB - paramA*tangentA) DOT tangentB = 0
       or
        deltaAB DOT tangentA = paramB*(-tangentB DOT tangentA) + paramA*(tangentA DOT tangentA)

        deltaAB DOT tangentB = paramB*(-tangentB DOT tangentB) + paramA*(tangentA DOT tangentB)
       or, as a matrix equation:
        [ A B ] [ paramA ] = [C]
                [ paramB ]
        where A,B,C are two component vectors
        To control condition number, use normalized tangent vectors to the right
            of the DOT.  (Leave unnormalized on left to preserve parameter ranges).
    */
    if  (  normalizedTangentA.Normalize (rayA.direction) != 0.0
        && normalizedTangentB.Normalize (rayB.direction) != 0.0
        )
        {
        C.x =  deltaAB.DotProduct (normalizedTangentA);
        C.y =  deltaAB.DotProduct (normalizedTangentB);
        A.x =  rayA.direction.DotProduct (normalizedTangentA);
        A.y =  rayA.direction.DotProduct (normalizedTangentB);
        B.x = -rayB.direction.DotProduct (normalizedTangentA);
        B.y = -rayB.direction.DotProduct (normalizedTangentB);

        result = bsiSVD_solve2x2 (&fractionA, &fractionB, A.x, B.x, A.y, B.y, C.x, C.y);
        }

    if (result)
        {
        pointA.SumOf (rayA.origin, rayA.direction, fractionA);
        pointB.SumOf (rayB.origin, rayB.direction, fractionB);
        }
    else
        {
        pointA = rayA.origin;
        fractionA = 0.0;
        rayB.ProjectPointUnbounded (pointB, fractionB, pointA);    
        }
    return  result;
    }

// Find interval of t for which x = a + bt hits [x0, x1]
// Reduce the validRange.
static void Clip1d (double a, double b, double x0, double x1, DRange1dR validRange)
    {
    double rb;
    if (validRange.high < validRange.low)
        return;
    if (!DoubleOps::SafeDivide (rb, 1.0 , b, 0.0))
        {
        // Never crosses x0, x1.  Maybe it's between ..
        if ((a - x0) * (a - x1) <= 0.0)
            return;
        // else all outside ..
        validRange.InitNull ();
        return;
        }
    // simple crossings ...
    double t0 = rb * (x0 - a);
    double t1 = rb * (x1 - a);
    if (t0 > t1)
        {
        double aa = t0;
        t0 = t1;
        t1 = aa;
        }
    if (t0 > validRange.low)
        validRange.low = t0;
    if (t1 < validRange.high)
        validRange.high = t1;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DRay3d::ClipToRange
(
DRange3dCR range,
DSegment3dR segment, // clipped segment.
DRange1dR  rayFractionRange
) const
    {
    rayFractionRange = DRange1d::InfiniteRange ();
    Clip1d (origin.x, direction.x, range.low.x, range.high.x, rayFractionRange);
    Clip1d (origin.y, direction.y, range.low.y, range.high.y, rayFractionRange);
    Clip1d (origin.z, direction.z, range.low.z, range.high.z, rayFractionRange);
    if (rayFractionRange.IsEmpty ())
        return false;
    segment.point[0] = FractionParameterToPoint (rayFractionRange.low);
    segment.point[1] = FractionParameterToPoint (rayFractionRange.high);
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DRay3d::TryGetViewingTransforms
(
TransformR localToWorld,
TransformR worldToLocal
) const
    {
    DVec3d xVector, yVector, zVector;
    if (direction.GetNormalizedTriad (xVector, yVector, zVector))
        {
        localToWorld.InitFromOriginAndVectors (origin, xVector, yVector, zVector);
        worldToLocal.InvertRigidBodyTransformation (localToWorld);
        return true;
        }
    return false;
    }
#ifdef BuildIntersectUnboundedXY
// EDL -- coded Marcy 2017, not needed (or tested)
/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
ValidatedDPoint3d DRay3d::IntesectUnboundedXY
(
DRay3dCR rayA
DRay3dCR rayB
)
    {
    double fractionA, fractionB;
    if (bsiSVD_solve2x2 (&fractionA, &fractionB,
            rayA.direction.x, rayB.direction.x,
            rayA.direction.y, rayB.direction.y,
            rayB.origin.x - rayA.origin.x,    rayA.origin.y - rayB.origin.y))
        return ValidatedDPoint3d (rayA.origin + fractionA * rayA.direction);
    return ValidatedDPoint3d (rayA.origin, false);
    }
#endif
END_BENTLEY_GEOMETRY_NAMESPACE
