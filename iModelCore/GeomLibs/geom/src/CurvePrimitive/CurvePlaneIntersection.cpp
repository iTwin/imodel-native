/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void AppendTolerancedPlaneIntersections(DPlane3dCR plane, ICurvePrimitiveCP curve, DSegment3dCR segment, bvector<CurveLocationDetailPair> &intersections, double tol)
    {
    double h0 = plane.Evaluate (segment.point[0]);
    double h1 = plane.Evaluate (segment.point[1]);
    if (fabs (h0) <= tol && fabs (h1) <= tol)
        {
        intersections.push_back (CurveLocationDetailPair (curve, 0.0, segment.point[0], 1.0, segment.point[1]));
        }
    else if (fabs (h0) <= tol)
        {
        intersections.push_back (CurveLocationDetailPair (curve, 0.0, segment.point[0]));
        }
    else if (fabs (h1) <= tol)
        {
        intersections.push_back (CurveLocationDetailPair (curve, 1.0, segment.point[1]));
        }
    else if (h0 * h1 < 0.0)
        {
        double s = -h0 / (h1 - h0);
        DPoint3d point;
        segment.FractionParameterToPoint (point, s);
        intersections.push_back (CurveLocationDetailPair (curve, s, point));
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void TestAndAppend(
            ICurvePrimitiveCP curve,
            DEllipse3dR ellipse,
            double theta,
            bvector<CurveLocationDetailPair> &intersections)
    {
    if (ellipse.IsAngleInSweep (theta))
        {
        DPoint3d point;
        ellipse.Evaluate (point, theta);
        intersections.push_back (CurveLocationDetailPair (curve, ellipse.AngleToFraction (theta), point));
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void AppendTolerancedPlaneIntersections(DPlane3dCR plane, ICurvePrimitiveCP curve, DEllipse3d ellipse, bvector<CurveLocationDetailPair> &intersections, double tol)
    {
    double hCenter  = plane.Evaluate (ellipse.center);
    double dh0      = plane.EvaluateVector (ellipse.vector0);
    double dh90     = plane.EvaluateVector (ellipse.vector90);
    double thetaC = atan2 (dh90, dh0);    // Angle where ellipse tangent is parallel to plane.
    double q0 =  dh0  * cos (thetaC);
    double q90 = dh90 * sin (thetaC);
    double hC = hCenter + q0 + q90;
    double hD = hCenter - q0 - q90;

    if ((hC > tol && hD > tol) || (hC < -tol && hD < -tol))
        {
        // no intersections
        }
    else if (fabs (hC) < tol && fabs (hD) < tol)
        {
        // Entire ellipse is ON
        DPoint3d pointA, pointB;
        ellipse.Evaluate (pointA, ellipse.start);
        ellipse.Evaluate (pointB, ellipse.start + ellipse.sweep);
        intersections.push_back (CurveLocationDetailPair (curve, 0.0, pointA, 1.0, pointB));
        }
    else if (fabs (hC) < tol)
        {
        TestAndAppend (curve, ellipse, thetaC, intersections);
        }
    else if (fabs (hD) < tol)
        {
        TestAndAppend (curve, ellipse, thetaC + msGeomConst_pi, intersections);
        }
    else if (hC * hD < 0.0) // amplitude is nonzero, tol cases are eliminated
        {
        double hBar = sqrt (dh0 * dh0 + dh90 * dh90);   // amplitide of cosine wave around thetaC
        double lambda = -hCenter / hBar;
        if (fabs (lambda) <= 1.0)
            {
            double delta = acos (lambda);
            TestAndAppend (curve, ellipse, thetaC + delta, intersections);
            TestAndAppend (curve, ellipse, thetaC - delta, intersections);
            }
        }

    }

struct CurvePlaneIntersectionFunction : FunctionRToRD
{
ICurvePrimitiveCP m_curve;
DPlane3d m_plane;

CurvePlaneIntersectionFunction (ICurvePrimitiveCP curve, DPlane3dCR plane)
    : m_curve (curve), m_plane(plane)
    {
    }

bool EvaluateRToRD (double fraction, double &f, double &df) override 
    {
    DPoint3d xyz;
    DVec3d tangent;
    if (m_curve->FractionToPoint (fraction, xyz, tangent))
        {
        DVec3d vector = DVec3d::FromStartEnd (m_plane.origin, xyz);
        f = vector.DotProduct (m_plane.normal);
        df = tangent.DotProduct (m_plane.normal);
        return true;
        }
    return false;
    }
};


bool ImprovePlaneCurveIntersection (DPlane3dCR plane, ICurvePrimitiveCP curve, CurveLocationDetailPair& pair)
    {
    if (pair.SameCurveAndFraction ())
        {
        CurvePlaneIntersectionFunction functionObject (curve, plane);
        double fraction = pair.detailA.fraction;
        double a = curve->FastMaxAbs ();

        NewtonIterationsRRToRR newton (Angle::SmallAngle () * a);
        if (newton.RunNewton (fraction, functionObject))
            {
            CurveLocationDetail detail;
            if (curve->FractionToPoint (fraction, detail))
                {
                pair.detailA = pair.detailB = detail;
                return true;
                }
            }
        }
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2016
+--------------------------------------------------------------------------------------*/
bool TransformDerivatives
(
DMatrix4dCP worldToLocal,
DPoint3dR xyz,
DVec3dR d1xyz,
DVec3dR d2xyz
)
    {
    if (nullptr == worldToLocal)
        return true;
    DPoint4d X, dX, ddX;
    X = worldToLocal->Multiply (xyz, 1.0);
    dX = worldToLocal->Multiply (d1xyz, 0.0);
    ddX = worldToLocal->Multiply (d2xyz, 0.0);
    auto normalizedDerivatives = DPoint4d::TryNormalizePointAndDerivatives (X, dX, ddX);
    if (normalizedDerivatives.IsValid ())
        {
        xyz = normalizedDerivatives.Value().origin;
        d1xyz = normalizedDerivatives.Value().vectorU;
        d2xyz = normalizedDerivatives.Value().vectorV;
        return true;
        }
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2016
+--------------------------------------------------------------------------------------*/
struct CurveCurveIntersectionFunction : FunctionRRToRRD
{
ICurvePrimitiveCR m_curveA, m_curveB;
DMatrix4dCP m_worldToLocal;

CurveCurveIntersectionFunction (ICurvePrimitiveCR curveA, ICurvePrimitiveCR curveB, DMatrix4dCP worldToLocal)
    : m_curveA (curveA), m_curveB (curveB), m_worldToLocal (worldToLocal)
    {
    }

bool EvaluateRRToRRD (
double u,
double v,
double &f,
double &g,
double &dfdu,
double &dfdv,
double &dgdu,
double &dgdv
) override 
    {
    DPoint3d xyzA, xyzB;
    DVec3d dxyzA, dxyzB;
    DVec3d derivative2A, derivative2B;
    if (   m_curveA.FractionToPoint (u, xyzA, dxyzA, derivative2A)
        && m_curveB.FractionToPoint (v, xyzB, dxyzB, derivative2B)
        )
        {
        if (TransformDerivatives (m_worldToLocal, xyzA, dxyzA, derivative2A)
            && TransformDerivatives (m_worldToLocal, xyzB, dxyzB, derivative2B))
            {
            f = xyzA.x - xyzB.x;
            dfdu = dxyzA.x; dfdv = -dxyzB.x;
            g = xyzA.y - xyzB.y;
            dgdu = dxyzA.y; dgdv = -dxyzB.y;
            return true;
            }
        }
    return false;
    }
};

bool ImprovePlaneCurveCurveTransverseIntersectionXY
(
ICurvePrimitiveCR curveA,
ICurvePrimitiveCR curveB,
DMatrix4dCP worldToLocal,
double &fractionA,
double &fractionB
)
    {
    CurveCurveIntersectionFunction functionObject (curveA, curveB, worldToLocal);
    double a = curveA.FastMaxAbs ();

    NewtonIterationsRRToRR newton (Angle::SmallAngle () * a);
    double fA = fractionA, fB = fractionB;
    if (newton.RunNewton (fA, fB, functionObject))
        {
        fractionA = fA;
        fractionB = fB;
        return true;
        }
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2016
+--------------------------------------------------------------------------------------*/


struct PointToCurvePerpendicularProjectionFunction : FunctionRToRD
{
ICurvePrimitiveCP m_curve;
DPoint3d m_spacePoint;

PointToCurvePerpendicularProjectionFunction (ICurvePrimitiveCP curve, DPoint3dCR spacePoint)
    : m_curve (curve), m_spacePoint(spacePoint)
    {
    }

bool EvaluateRToRD (double fraction, double &f, double &df) override 
    {
    DPoint3d xyz;
    DVec3d tangent;
    DVec3d derivative2;
    if (m_curve->FractionToPoint (fraction, xyz, tangent, derivative2))
        {
        DVec3d vector = DVec3d::FromStartEnd (m_spacePoint, xyz);
        f = vector.DotProduct (tangent);
        df = tangent.DotProduct (tangent) + vector.DotProduct (derivative2);
        return true;
        }
    return false;
    }
};

bool ImprovePerpendicularProjection (ICurvePrimitiveCP curve, DPoint3dCR spacePoint, double &fraction, DPoint3dR xyz)
    {
    PointToCurvePerpendicularProjectionFunction functionObject (curve, spacePoint);
    double a = curve->FastMaxAbs ();
    NewtonIterationsRRToRR newton (Angle::SmallAngle () * a);
    if (newton.RunNewton (fraction, functionObject))
        {
        CurveLocationDetail detail;
        if (curve->FractionToPoint (fraction, detail))
            {
            fraction = detail.fraction;
            xyz = detail.point;
            return true;
            }
        }
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2016
+--------------------------------------------------------------------------------------*/

struct PointToCurvePerpendicularProjectionFunctionXY : FunctionRToRD
{
ICurvePrimitiveCP m_curve;
DPoint3d m_spacePoint;
DPoint3d m_transformedSpacePoint;
DMatrix4d m_matrix;
RotMatrix m_Q;
// 0=> identity
// 1=> cartesian
// 2=> full homogeneous
int m_transformComplexity; 
PointToCurvePerpendicularProjectionFunctionXY (ICurvePrimitiveCP curve, DPoint3dCR spacePoint, DMatrix4dCP matrix)
    : m_curve (curve), m_spacePoint(spacePoint)
    {
    if (nullptr == matrix)
        {
        m_transformComplexity = 0;
        m_matrix.InitIdentity ();
        m_Q.InitIdentity ();
        }
    else
        {
        m_matrix = *matrix;
        DVec3d A,B;
        double c;
        matrix->ExtractAroundPivot (m_Q, A, B, c, 3);
        matrix->MultiplyAndRenormalize (&m_transformedSpacePoint, &m_spacePoint, 1);
        if (m_matrix.IsIdentity ())
            m_transformComplexity = 0;
        else if (m_matrix.IsAffine ())
            m_transformComplexity = 1;  // only Q matters!!
        else
            m_transformComplexity = 2;  // full homogeneous !!!!
        }
    }

bool EvaluateRToRD (double fraction, double &f, double &df) override 
    {
    DPoint3d xyz;
    DVec3d tangent;
    DVec3d derivative2;
    if (m_curve->FractionToPoint (fraction, xyz, tangent, derivative2))
        {

        if (m_transformComplexity == 2)
            {
            DPoint4d X, dX, ddX;
            X = m_matrix.Multiply (xyz, 1.0);
            dX = m_matrix.Multiply (tangent, 0.0);
            ddX = m_matrix.Multiply (derivative2, 0.0);
            auto normalizedDerivatives = DPoint4d::TryNormalizePointAndDerivatives (X, dX, ddX);
            if (normalizedDerivatives.IsValid ())
                {
                DVec3d vector = DVec3d::FromStartEnd (m_transformedSpacePoint, normalizedDerivatives.Value ().origin);
                tangent = normalizedDerivatives.Value ().vectorU;
                derivative2 = normalizedDerivatives.Value ().vectorV;
                f = vector.DotProductXY (tangent);
                df = tangent.DotProductXY (tangent) + vector.DotProductXY (derivative2);
                return true;
                }
            }
        else
            {
            DVec3d vector = DVec3d::FromStartEnd (m_spacePoint, xyz);
            if (m_transformComplexity == 1)
                {
                m_Q.Multiply (vector);
                m_Q.Multiply (tangent);
                m_Q.Multiply (derivative2);
                }
            f = vector.DotProductXY (tangent);
            df = tangent.DotProductXY (tangent) + vector.DotProductXY (derivative2);
            return true;
            }
        }
    return false;
    }
};


bool ImprovePerpendicularProjectionXY (ICurvePrimitiveCP curve, DPoint3dCR spacePoint, double &fraction, DPoint3dR xyz, DMatrix4dCP matrix)
    {
    PointToCurvePerpendicularProjectionFunctionXY functionObject (curve, spacePoint, matrix);
    double a = curve->FastMaxAbs ();
    NewtonIterationsRRToRR newton (Angle::SmallAngle () * a);
    if (newton.RunNewton (fraction, functionObject))
        {
        CurveLocationDetail detail;
        if (curve->FractionToPoint (fraction, detail))
            {
            fraction = detail.fraction;
            xyz = detail.point;
            return true;
            }
        }
    return false;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ImprovePlaneCurveIntersections(DPlane3dCR plane, ICurvePrimitiveCP curve, bvector<CurveLocationDetailPair> &intersections)
    {
    for (auto &pair : intersections)
        {
        ImprovePlaneCurveIntersection (plane, curve, pair);
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void AppendTolerancedPlaneIntersections(DPlane3dCR plane, ICurvePrimitiveCP curve, MSBsplineCurveCR bcurve,
            bvector<CurveLocationDetailPair> &intersections, double tol)
    {
    bvector<double>fractions;
    bvector<DPoint3d>points;
    bcurve.AddPlaneIntersections (&points, &fractions, plane);
    for (size_t i = 0, n = fractions.size (); i < n; i++)
        {
        intersections.push_back (CurveLocationDetailPair (curve, fractions[i], points[i]));        
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct PolylineCrossingData
    {
    size_t index;
    double h;
    bool on;
    DPoint3d xyz;
    // If i is a valid index, setup all data.
    // If i is invalid, save i but leave other data unchanged.
    bool Setup (size_t i, bvector<DPoint3d>const &points, DPlane3dCR plane, double tol)
        {
        index = i;
        if (i >= points.size ())
            return false;
        xyz = points[i];
        h = plane.Evaluate (xyz);
        on = fabs (h) <= tol;
        return true;
        }
    };

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void AppendTolerancedPlaneIntersections(DPlane3dCR plane, ICurvePrimitiveCP curve, bvector<DPoint3d>const &points,
            bvector<CurveLocationDetailPair> &intersections, double tol)
    {
    size_t n = points.size ();
    if (n < 1)
        return;
    PolylineCrossingData crossing0, crossing1, crossing2;

    DPoint3d xyz;
    crossing0.Setup (0, points, plane, tol);
    double fractionStep = 1.0 / (n - 1);
    // Process from crossing0 forward ...
    while (crossing0.index < n)
        {
        if (crossing0.on)
            {
            size_t numOn = 1;
            // crossing0..crossing1 are all ON.
            // crossing2 pushes ahead (and becomes the first one to look at next time around the outer loop.)
            crossing2 = crossing1 = crossing0;
            while (crossing2.Setup (crossing2.index + 1, points, plane, tol) && crossing2.on)
                {
                crossing1 = crossing2;
                numOn++;
                }
            if (numOn == 1)
                {
                // single point on
                intersections.push_back (CurveLocationDetailPair (curve, crossing0.index * fractionStep, crossing0.xyz,
                                    crossing0.index, n - 1, 0.0));
                }
            else
                {
                // interval on
                intersections.push_back (CurveLocationDetailPair (curve,
                            crossing0.index * fractionStep, crossing0.xyz,
                            (1 + crossing1.index) * fractionStep, crossing1.xyz
                            ));
                }
            crossing0 = crossing2;
            }
        else
            {
            // crossing0 is OFF.
            if (   crossing1.Setup (crossing0.index + 1, points, plane, tol)
                && !crossing1.on
                && crossing0.h * crossing1.h < 0.0)
                {
                double edgeFraction = -crossing0.h / (crossing1.h - crossing0.h);
                xyz.Interpolate (crossing0.xyz, edgeFraction, crossing1.xyz);
                intersections.push_back (CurveLocationDetailPair (curve,
                                    (crossing0.index + edgeFraction) * fractionStep, xyz,
                                    crossing0.index , n - 1, edgeFraction));
                }
            crossing0 = crossing1;
            }
        }
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void AppendTolerancedPlaneIntersections(DPlane3dCR plane, ICurvePrimitiveCP curve, CurveVectorCR vector,
            bvector<CurveLocationDetailPair> &intersections, double tol)
    {
    for (size_t i = 0, n = vector.size (); i < n; i++)
        vector[i]->AppendCurvePlaneIntersections (plane, intersections, tol);
    }


END_BENTLEY_GEOMETRY_NAMESPACE
