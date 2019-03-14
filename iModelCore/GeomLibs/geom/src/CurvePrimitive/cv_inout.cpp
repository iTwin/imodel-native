/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/CurvePrimitive/cv_inout.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct SimpleCrossingCounter
{
size_t numLeft;
size_t numRight;
size_t numOnEdge;
size_t numOnPlane;

DPlane3d splitPlane;
DPlane3d sortPlane;

DPoint3d chainStartPoint;
double   chainStartAltitude;
DPoint3d chainEndPoint;
double   chainEndAltitude;
size_t numProcessed;
double tol;
public: SimpleCrossingCounter (double _tol, DPlane3d _splitPlane, DPlane3d _sortPlane)
    {
    numOnPlane = numLeft = numRight = numProcessed = numOnEdge = 0;
    tol = _tol;
    splitPlane = _splitPlane;
    sortPlane  = _sortPlane;
    }

CurveVector::InOutClassification GetClassification ()
    {
    // any single edge hit gives final ON
    if (numOnEdge > 0)
        return CurveVector::INOUT_On;
    // Any single vertexOnPlane gives Unknown
    if (numOnPlane > 0)
        return CurveVector::INOUT_Unknown;
    // Nothing "on", so parity counts should be clear ...
    bool oddLeft = (1 == (numLeft & 0x01));
    bool oddRight = (1 == (numRight & 0x01));
    if (oddLeft && oddRight)
        return CurveVector::INOUT_In;
    else if (!oddLeft && !oddRight)
        return CurveVector::INOUT_Out;
    return CurveVector::INOUT_Unknown;
    }

protected:
// Record an edge crossing as left, right, or on.
void CheckPoint_leftRight (DPoint3dCR point)
    {
    double sortCoordinate = sortPlane.Evaluate (point);
    if (sortCoordinate > tol)
        numRight++;
    else if (sortCoordinate < -tol)
        numLeft++;
    else
        numOnEdge++;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool IsClearEndPoint(DPoint3d point, double h)
    {
    if (fabs (h) > tol)
        return true;
    double sortCoordinate = sortPlane.Evaluate (point);
    if (fabs (sortCoordinate) < tol)
        numOnEdge++;
    else
        numOnPlane++;
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void CheckEdge_go(DPoint3dCR point0, double h0, DPoint3dCR point1, double h1)
    {
    if (IsClearEndPoint (point0, h0) && IsClearEndPoint (point1, h1))
        {
        if (h0 * h1 < 0.0)
            {
            double s = -h0 / (h1 - h0);
            CheckPoint_leftRight (DPoint3d::FromInterpolate (point0, s, point1));
            }
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void RegisterStartPoint(DPoint3dCR point, double h)
    {
    if (numProcessed == 0)
        {
        chainStartPoint = point;
        chainStartAltitude = h;
        }
    else
        CheckEdge_go (chainEndPoint, chainEndAltitude, point, h);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void RegisterEndPoint(DPoint3dCR point, double h)
    {
    chainEndPoint = point;
    chainEndAltitude = h;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void RegisterON()
    {
    numOnPlane++;
    }
public: 

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void CheckEdge(DPoint3dCR point0, DPoint3dCR point1)
    {
    double h0 = splitPlane.Evaluate (point0);
    double h1 = splitPlane.Evaluate (point1);
    RegisterStartPoint (point0, h0);
    CheckEdge_go (point0, h0, point1, h1);
    RegisterEndPoint (point1, h1);
    }

public:

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void CheckLineString(bvector<DPoint3d> const &points)
    {
    size_t n = points.size ();
    if (n == 0)
        return;
    DPoint3d point0, point1;
    double h0, h1;
    point0 = points[0];
    h0 = splitPlane.Evaluate (point0);
    RegisterStartPoint (point0, h0);
    for (size_t i = 1; i < n; i++, point0 = point1, h0 = h1)
        {
        point1 = points[i];
        h1 = splitPlane.Evaluate (point1);
        CheckEdge_go (point1, h1, point0, h0);
        }
    RegisterEndPoint (point0, h0);
    }

public:

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void CheckArc(DEllipse3dCR ellipse)
    {
    double uC = splitPlane.Evaluate (ellipse.center);
    double u0 = splitPlane.normal.DotProduct (ellipse.vector0);
    double u90 = splitPlane.normal.DotProduct (ellipse.vector90);
    // The ellipse altitude is uC+u0 * cos(theta) + u90 * sin(theta)
    DPoint3d xyzA, xyzB;
    double hA, hB;
    double angles[2];

    if (fabs (u0) <= tol && fabs (u90) <= tol)
        {
        // ellpse is parallel to the plane.
        if (fabs (uC) <= tol)
            {
            RegisterON ();
            }
        }
    else
        {
        double thetaMax, thetaMin;
        double amplitude;
        DPoint3d xyzC;
        size_t numCrossing;
        ellipse.Evaluate (xyzA, ellipse.start);
        ellipse.Evaluate (xyzB, ellipse.start + ellipse.sweep);
        hA = splitPlane.Evaluate (xyzA);
        hB = splitPlane.Evaluate (xyzB);
        RegisterStartPoint (xyzA, hA);
        if (!IsClearEndPoint (xyzA, hA) || !IsClearEndPoint (xyzB, hB))
            {
            // Don't try to sort out scanline intersections with endpoints 
            }
        else if (AnalyticRoots::LinearTrigFormRoots (uC, u0, u90, angles, numCrossing, thetaMax, thetaMin, amplitude))
            {
            if (numCrossing == 2)
                {
                ellipse.Evaluate (xyzC, angles[0]);
                if (ellipse.IsAngleInSweep (angles[0]))
                    CheckPoint_leftRight (xyzC);
                ellipse.Evaluate (xyzC, angles[1]);
                if (ellipse.IsAngleInSweep (angles[1]))
                    CheckPoint_leftRight (xyzC);
                }
            }
        else if (u0 > 0.0 && u0 - amplitude < tol)
            {
            // Report min/max points twice -- if near test point it will register "ON", if not the crossings will cancel.
            if (ellipse.IsAngleInSweep (thetaMin))
                {
                ellipse.Evaluate (xyzC, thetaMin);
                CheckPoint_leftRight (xyzC);
                CheckPoint_leftRight (xyzC);
                }
            }
        else if (u0 < 0.0 && u0 + amplitude > -tol)
            {
            if (ellipse.IsAngleInSweep (thetaMax))
                {
                ellipse.Evaluate (xyzC, thetaMax);
                CheckPoint_leftRight (xyzC);
                CheckPoint_leftRight (xyzC);
                }
            }
        RegisterEndPoint (xyzB, hB);
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void CheckBCurve(MSBsplineCurveCR bcurve)
    {
    bvector<DPoint3d>points;
    bvector<double>params;

    DPoint3d xyzA, xyzB;
    DPoint3d point01, point12;
    double h01, h12;
    bcurve.AddPlaneIntersections (&points, &params, splitPlane);
    bcurve.ExtractEndPoints (xyzA, xyzB);

    double hA = splitPlane.Evaluate (xyzA);
    double hB = splitPlane.Evaluate (xyzB);
    static double s_paramTol = 1.0e-8;
    if (IsClearEndPoint (xyzA, hA) && IsClearEndPoint (xyzB, hB))
        {
        // hmm.. 0 and 1 should not be intersections, but we'll filter them anyway ...
        RegisterStartPoint (xyzA, hA);
        double param0, param1, param2;
        DPoint3d point1;
        size_t n = params.size ();
        while (n > 0 && params[n-1] > 1.0 - s_paramTol)
            n--;
        size_t i = 0;
        while (i < n && params[i] < s_paramTol)
            i++;
            
        if (n > 0 && i < n)
            {
            param0 = 0.0;
            for (; i < n; param0 = param1)
                {
                param1 = params[i];
                point1 = points[i];
                i++;
                while (i < n && params[i] < param1 + s_paramTol)
                    i++;
                if (i < n)
                    param2 = params[i];
                else
                    param2 = 1.0;
                // param1, point1 is a crossing.
                // find altitudes at the middle of the surrounding curve intervals
                bcurve.FractionToPoint (point01, 0.5 * (param0 + param1));
                bcurve.FractionToPoint (point12, 0.5 * (param1 + param2));
                h01 = splitPlane.Evaluate (point01);
                h12 = splitPlane.Evaluate (point12);
                // crossing or tangency?
                if (h01 * h12 < 0.0)
                    {
                    CheckPoint_leftRight (point1);
                    }
                else
                    {
                    CheckPoint_leftRight (point1);
                    CheckPoint_leftRight (point1);
                    }                
                }
            }
        RegisterEndPoint (xyzB, hB);
        }
    }

};



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static CurveVector::InOutClassification ClassifySingleLoopInOnOut_go
(
CurveVectorCR curves,
DPoint3dCR xyz,
DVec3dCR scanDirection,
CurveVector::InOutClassification &classification
)
    {
    if (  curves.GetBoundaryType () != CurveVector::BOUNDARY_TYPE_Outer
       && curves.GetBoundaryType () != CurveVector::BOUNDARY_TYPE_Inner)
        return CurveVector::INOUT_Unknown;

    DVec3d perpVector = DVec3d::From (-scanDirection.y, scanDirection.x, 0.0);
    DPlane3d sortPlane = DPlane3d::FromOriginAndNormal (xyz, scanDirection);
    DPlane3d perpPlane = DPlane3d::FromOriginAndNormal (xyz, perpVector);
    double bigCoordinate = 1.0;
    double tol = Angle::SmallAngle () * bigCoordinate;
    SimpleCrossingCounter counter (tol, perpPlane, sortPlane);
    size_t numCurves = curves.size ();
    for (size_t i = 0; i < numCurves; i++)
        {
        switch (curves[i]->GetCurvePrimitiveType ())
            {
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                {
                DSegment3dCP segment = curves[i]->GetLineCP ();
                counter.CheckEdge (segment->point[0], segment->point[1]);
                }
                break;
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                {
                DEllipse3dCP arc = curves[i]->GetArcCP ();
                counter.CheckArc (*arc);
                }
                break;
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                {
                bvector<DPoint3d>const *points = curves[i]->GetLineStringCP ();
                counter.CheckLineString (*points);
                }
                break;
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
                {
                MSBsplineCurveCP bcurve = curves[i]->GetProxyBsplineCurveCP ();
                if (NULL == bcurve)
                    return CurveVector::INOUT_Unknown;
                counter.CheckBCurve (*bcurve);
                }
                break;
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector:
            default:
                return CurveVector::INOUT_Unknown;
            }
        }
    return counter.GetClassification ();
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static CurveVector::InOutClassification ClassifySingleLoopInOnOut_go
(
CurveVectorCR curves,
DPoint3dCR xyz
)
    {
    CurveVector::InOutClassification classification;
    classification = ClassifySingleLoopInOnOut_go (curves, xyz, DVec3d::From (1.0, 0.0, 0.0), classification);
    if (CurveVector::INOUT_Unknown != classification)
        return classification;

    classification = ClassifySingleLoopInOnOut_go (curves, xyz, DVec3d::From (0.0, 1.0, 0.0), classification);
    if (CurveVector::INOUT_Unknown != classification)
        return classification;

    // Uh oh.   Simple classification ran into troubles in both x and y directions.
    // Try some uncommon angles...
    double dTheta = 0.142322999;
    for (size_t i = 0; i < 10; i++)
        {
        double theta = i * dTheta;
        double c = cos (theta);
        double s = sin (theta);
        classification = ClassifySingleLoopInOnOut_go (curves, xyz, DVec3d::From (c, s, 0.0), classification);
        if (CurveVector::INOUT_Unknown != classification)
            return classification;
        }
    return CurveVector::INOUT_Unknown;
    }

CurveVector::InOutClassification CurveVector::PointInOnOutXY (DPoint3dCR xyz) const
    {
    size_t n = size ();
    if (IsUnionRegion ())
        {
        size_t numOut = 0;
        for (size_t i = 0; i < n; i++)
            {
            CurveVectorCP child = at(i)->GetChildCurveVectorCP ();
            if (NULL != child)
                {
                CurveVector::InOutClassification childClassification = child->PointInOnOutXY (xyz);
                if (childClassification == INOUT_In
                    || childClassification == INOUT_On)
                    return childClassification;
                else if (childClassification == INOUT_Out)
                    numOut++;
                }
            }
        if (numOut > 0)
            return INOUT_Out;
        }
    else if (IsParityRegion ())
        {
        int numIn = 0;
        for (size_t i = 0; i < n; i++)
            {
            CurveVectorCP child = at(i)->GetChildCurveVectorCP ();
            if (NULL != child)
                {
                CurveVector::InOutClassification childClassification = child->PointInOnOutXY (xyz);
                if (childClassification == INOUT_On)
                    return INOUT_On;
                if (childClassification == INOUT_In)
                    numIn++;
                }
            }
        return (numIn & 0x01) == 1 ? INOUT_In : INOUT_Out;
        }
    else if (IsClosedPath ())
        {
        return ClassifySingleLoopInOnOut_go (*this, xyz);
        }
        
    return INOUT_Unknown;
    }


CurveVector::InOutClassification CurveVector::RayPierceInOnOut
(
DRay3dCR ray,
SolidLocationDetailR hitDetail
) const
    {
    Transform localToWorld, worldToLocal;
    DRange3d localRange;
    CurveVectorPtr localCurve = CloneInLocalCoordinates
            (
            LOCAL_COORDINATE_SCALE_01RangeBothAxes,
            localToWorld, worldToLocal,
            localRange
            );
    if (localCurve.IsValid ())
        {        
        double rayFraction;
        DPoint3d uvw;
        
        if (ray.IntersectZPlane (localToWorld, 0.0, uvw, rayFraction))
            {
            DVec3d uVector, vVector;
            localToWorld.GetMatrixColumn (uVector, 0);
            localToWorld.GetMatrixColumn (vVector, 1);
            hitDetail = SolidLocationDetail (0,
                rayFraction,
                ray.FractionParameterToPoint (rayFraction),
                uvw.x, uvw.y,
                uVector, vVector
                );
            
            if (!localRange.IsContainedXY (uvw))
                return CurveVector::INOUT_Out;
            else
                return localCurve->PointInOnOutXY (uvw);
            }
        }
    hitDetail = SolidLocationDetail ();
    return CurveVector::INOUT_Unknown;    
    }

CurveVector::InOutClassification CurveVector::ClosestCurveOrRegionPoint
(
DPoint3dCR spacePoint,
DPoint3dR  curveOrRegionPoint
) const
    {
    CurveLocationDetail curveLocation;
    if (!ClosestPointBounded (spacePoint, curveLocation))
        return CurveVector::INOUT_Unknown;
    curveOrRegionPoint = curveLocation.point;
    CurveVector::InOutClassification classification = CurveVector::INOUT_On;
    if (IsAnyRegionType ())
        {
        Transform localToWorld, worldToLocal;
        DRange3d localRange;
        CurveVectorPtr localCurve = CloneInLocalCoordinates
                (
                LOCAL_COORDINATE_SCALE_UnitAxesAtStart,
                localToWorld, worldToLocal,
                localRange
                );
        if (localCurve.IsValid ())
            {        
            DPoint3d uvw = DPoint3d::FromProduct (worldToLocal, spacePoint);
            if (localRange.IsContainedXY (uvw))
                {
                uvw.z = 0.0;
                InOutClassification inout = localCurve->PointInOnOutXY (uvw);
                if (inout == CurveVector::INOUT_In)
                    {
                    curveOrRegionPoint = DPoint3d::FromProduct (localToWorld, uvw);
                    classification = inout;
                    }
                }
            }
        }
    return classification;
    }



END_BENTLEY_GEOMETRY_NAMESPACE
