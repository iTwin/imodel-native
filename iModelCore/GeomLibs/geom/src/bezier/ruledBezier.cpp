/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/bezier/ruledBezier.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                       |
|   Include Files                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


struct RuledBezier
{
DPoint4d m_coff[2][MAX_BEZIER_CURVE_ORDER];
int m_order;
RuledBezier (DPoint4dCP coffA, DPoint4dCP coffB, int n)
    {
    memcpy (m_coff[0], coffA, n * sizeof (DPoint4d));
    memcpy (m_coff[1], coffB, n * sizeof (DPoint4d));
    m_order = n;
    }

void Evaluate (double u, double v, DPoint4dR X, DPoint4dR dXdu, DPoint4dR dXdv)
    {
    DPoint4d xyzAB[2];
    DPoint4d dxyzAB[2];
    for (int i = 0; i < 2; i++)
        bsiBezier_functionAndDerivative ((double*)&xyzAB[i], (double*)&dxyzAB[i], (double*)m_coff[i], m_order, 4, u);
    X.Interpolate (xyzAB[0], v, xyzAB[1]);
    dXdu.DifferenceOf (xyzAB[1], xyzAB[0]);
    dXdv.Interpolate (dxyzAB[0], v, dxyzAB[1]);
    }
        
bool ClosestPointFunction (DPoint3dCR spacePoint, double u, double v, double &f0, double &f1)
    {
    DPoint4d X[2], dXdu[2];
    DPoint4d Y, dYdu;
    for (int i = 0; i < 2; i++)
        bsiBezier_functionAndDerivative ((double*)&X[i], (double*)&dXdu[i], (double*)&m_coff[i][0], m_order, 4, u);

    Y.Interpolate (X[0], v, X[1]);
    dYdu.Interpolate (dXdu[0], v, dXdu[1]);

    DPoint4d viewDirection, directionU, directionV;
    viewDirection.WeightedDifferenceOf (Y, spacePoint, 1.0);


    // pseudo vectors are in the direction of tangents even though not deweighted ...
    directionU.WeightedDifferenceOf (dYdu, Y);
    directionV.WeightedDifferenceOf (X[1], X[0]);
    f0 = directionU.DotProduct (viewDirection);
    f1 = directionV.DotProduct (viewDirection);
    return true;
    }
};


struct RuledBezier_ClosestPoint : RuledBezier, FunctionRRToRR
{
DPoint3d m_spacePoint;
RuledBezier_ClosestPoint (DPoint4dCP coffA, DPoint4dCP coffB, int n, DPoint3dCR spacePoint)
    : m_spacePoint (spacePoint), RuledBezier (coffA, coffB, n)
    {
    }
bool EvaluateRRToRR (double u, double v, double &f, double &g)
    {
    return ClosestPointFunction (m_spacePoint, u, v, f, g);
    }
};
Public bool bsiBezier_addRuledPatchXYIntersections
(
bvector <SolidLocationDetail> &pickData,
DPoint2dCR target,
DPoint3dCP coffA,
DPoint3dCP coffB,
int order
)
    {
// The patch is points
//   x = Ax + v (Bx-Ax)
//   y = Ay + v (By-Ay)
// (Ax,Ay,Bx,By are bezier polynomials from coffA and coffB, function of u)
// Equating to target.x and target.y
//   v (Bx-Ax) = target.x - Ax
//   v (By-Ay) = target.y - Ay
// Combine and eliminate v
//   (target.x - Ax)(By-Ay) - (target.y - Ay)(Bx - Ax) = 0
    DPoint2d coffP[MAX_BEZIER_CURVE_ORDER], coffQ[MAX_BEZIER_CURVE_ORDER];
    double product01[2 * MAX_BEZIER_CURVE_ORDER];
    double product10[2 * MAX_BEZIER_CURVE_ORDER];
    double resultant[2 * MAX_BEZIER_CURVE_ORDER];
    double roots[2 * MAX_BEZIER_CURVE_ORDER];

    for (int i = 0; i < order; i++)
        {
        coffP[i].x = target.x - coffA[i].x;
        coffP[i].y = target.y - coffA[i].y;
        coffQ[i].x = coffB[i].x - coffA[i].x;
        coffQ[i].y = coffB[i].y - coffA[i].y;
        }
    bsiBezier_univariateProduct (product01, 0, 1, (double*)coffP, order, 0, 2, (double*)coffQ, order, 1, 2);
    bsiBezier_univariateProduct (product10, 0, 1, (double*)coffP, order, 1, 2, (double*)coffQ, order, 0, 2);
    int resultantOrder = 2 * order - 1;
    bsiBezier_subtractPoles (resultant, product01, product10, resultantOrder, 1);
    
    int numRoots;
    if (!bsiBezier_univariateRoots (roots, &numRoots, resultant, resultantOrder))
        return false;

    for (int i = 0; i < numRoots; i++)
        {
        double u = roots[i];
        DPoint3d xyzA, xyzB, xyz;
        DVec3d tangentA, tangentB, dXdu, dXdv;
        double v;
        bsiBezier_functionAndDerivative ((double*)&xyzA, (double*)&tangentA,
            const_cast<double*>((double const*)coffA), order, 3, u);
        bsiBezier_functionAndDerivative ((double*)&xyzB, (double*)&tangentB,
            const_cast<double*>((double const*)coffB), order, 3, u);
        dXdv.DifferenceOf (xyzB, xyzA);
        if (DoubleOps::ChooseSafeDivideParameter (v, target.x - xyzA.x, dXdv.x, target.y - xyzA.y, dXdv.y, 0.0))
            {
            xyz.Interpolate (xyzA, v, xyzB);
            dXdu.Interpolate (tangentA, v, tangentB);
            SolidLocationDetail detail;
            detail.SetXYZ (xyz);
            detail.SetUV (u, v, dXdu, dXdv);
            pickData.push_back (detail);
            }
        }
    return true;
    }

Public bool bsiBezier_addRuledPatchXYIntersections
(
bvector <SolidLocationDetail> &pickData,
DPoint2dCR target,
DPoint4dCP coffA,
DPoint4dCP coffB,
int order
)
    {
#ifdef NORMALIZE_AT_CURVES
// The patch is points
//
//   x = Ax/Aw + v (Bx/Bw-Ax/Aw)
//   y = Ay/Aw + v (By/Aw-Ay/Aw)
//
// (Ax,Ay,Aw,Bx,By,Bw are bezier polynomials from coffA and coffB, function of u)
//
// Equating to target.x and target.y
//
//   v (Bx/Bw-Ax/Aw) = target.x - Ax/Aw
//   v (By/Bw-Ay/Aw) = target.y - Ay/Aw
//
// Multiply through by weights
//
//   v (Bx Aw - Ax Bw) = target.x Aw Bw - Ax Bw
//   v (By Aw - Ay Bw) = target.y Aw Bw - Ay Bw
//
// Combine and eliminate v
//   (target.x Aw Bw - Ax Bw)(By Aw - Ay Bw) - (target.y Aw Bw - Ay Bw)(Bx Aw - Ax Bw) = 0
//   (target.x Aw - Ax)(By Aw - Ay Bw) - (target.y Aw - Ay)(Bx Aw - Ax Bw) = 0
//        Px * Qy - Py * Qx = 0

    if (order > MAX_BEZIER_CURVE_ORDER)
        return false;

    double AxBw [2 * MAX_BEZIER_CURVE_ORDER];
    double AyBw [2 * MAX_BEZIER_CURVE_ORDER];
    double AwBx [2 * MAX_BEZIER_CURVE_ORDER];
    double AwBy [2 * MAX_BEZIER_CURVE_ORDER];

    double Px   [MAX_BEZIER_CURVE_ORDER];
    double Py   [MAX_BEZIER_CURVE_ORDER];

    double Qx   [2 * MAX_BEZIER_CURVE_ORDER];
    double Qy   [2 * MAX_BEZIER_CURVE_ORDER];
    double PxQy [3 * MAX_BEZIER_CURVE_ORDER];
    double PyQx [3 * MAX_BEZIER_CURVE_ORDER];
    
    double resultant [4 * MAX_BEZIER_CURVE_ORDER];

    bsiBezier_univariateProduct (AxBw, 0, 1, (double*)coffA, order, 0, 4, (double*)coffB, order, 3, 4);
    bsiBezier_univariateProduct (AyBw, 0, 1, (double*)coffA, order, 1, 4, (double*)coffB, order, 3, 4);
    bsiBezier_univariateProduct (AwBx, 0, 1, (double*)coffA, order, 3, 4, (double*)coffB, order, 0, 4);
    bsiBezier_univariateProduct (AwBy, 0, 1, (double*)coffA, order, 3, 4, (double*)coffB, order, 1, 4);

    for (int i = 0; i < order; i++)
        {
        Px[i] = target.x * coffA[i].w - coffA[i].x;
        Py[i] = target.y * coffA[i].w - coffA[i].y;
        }

    int orderQ = 2 * order - 1;
    bsiBezier_subtractPoles (Qx, AwBx, AxBw, orderQ, 1);
    bsiBezier_subtractPoles (Qy, AwBy, AyBw, orderQ, 1);

    bsiBezier_univariateProduct (PxQy, 0, 1, Px, order, 0, 1, Qy, orderQ, 0, 1);
    bsiBezier_univariateProduct (PyQx, 0, 1, Py, order, 0, 1, Qx, orderQ, 0, 1);
    int orderR = order + orderQ - 1;

    bsiBezier_subtractPoles (resultant, PxQy, PyQx, orderR, 1);
    
    int numRoots;
    double roots[4 * MAX_BEZIER_CURVE_ORDER];
    if (!bsiBezier_univariateRoots (roots, &numRoots, resultant, orderR))
        return false;

    for (int i = 0; i < numRoots; i++)
        {
        double u = roots[i];
        DPoint3d xyzA, xyzB, xyz;
        DVec3d tangentA, tangentB, dXdu, dXdv;
        double v;
        bsiBezierDPoint4d_evaluateDPoint3dArray (&xyzA, &tangentA, coffA, order, &u, 1);
        bsiBezierDPoint4d_evaluateDPoint3dArray (&xyzA, &tangentB, coffA, order, &u, 1);
        dXdv.DifferenceOf (xyzB, xyzA);
        if (DoubleOps::ChooseSafeDivideParameter (v, target.x - xyzA.x, dXdv.x, target.y - xyzA.y, dXdv.y, 0.0))
            {
            xyz.Interpolate (xyzA, v, xyzB);
            dXdu.Interpolate (tangentA, v, tangentB);
            SolidLocationDetail detail;
            detail.SetXYZ (xyz);
            detail.SetUV (u, v, dXdu, dXdv);
            pickData.push_back (detail);
            }
        }
    return true;
#else
// The patch is points
//   x = (Ax + v Cx) / (Aw + v Cw)
//   y = (Ay + v Cy) / (Aw + v Cw)
//
// (Ax,Ay,Aw,Bx,By,Bw are bezier polynomials from coffA and coffB, function of u)
// (Cx=Ax-Bx, Cy=By-Ay, Cw=Bw-Aw just differences of A and B.)
//
// Equate to target.x and target.y and multiply by the weights:
//
//   target.x *(Aw + v Cw) = (Ax + v Cx)
//   target.y *(Aw + v Cw) = (Ay + v Cy)
//
//   v (target.x * Cw - Cx) = Ax - target.x Aw
//   v (target.y * Cw - Cy) = Ay - target.y Aw
//
//     (target.x Cw - Cx) (Ay - target.y Aw) - (target.y Cw - Cy) (Ax - target.x Aw) = 0
//     (Cx - target.x Cw) (Ay - target.y Aw) - (Cy - target.y Cw) (Ax - target.x Aw) = 0
//           Px               Qy             -        Py              Qx
    if (order > MAX_BEZIER_CURVE_ORDER)
        return false;
    DPoint4d coffC[MAX_BEZIER_CURVE_ORDER];
    bsiBezier_subtractPoles ((double *)coffC, const_cast<double*>((double const*)coffB), const_cast<double*>((double const*)coffA), order, 4);
    double Px[MAX_BEZIER_CURVE_ORDER];
    double Py[MAX_BEZIER_CURVE_ORDER];
    double Qx[MAX_BEZIER_CURVE_ORDER];
    double Qy[MAX_BEZIER_CURVE_ORDER];

    for (int i = 0; i < order; i++)
        {
        Px[i] = coffC[i].x - target.x * coffC[i].w;
        Py[i] = coffC[i].y - target.y * coffC[i].w;
        Qx[i] = coffA[i].x - target.x * coffA[i].w;
        Qy[i] = coffA[i].y - target.y * coffA[i].w;
        }

    double PxQy[2 * MAX_BEZIER_CURVE_ORDER];
    double PyQx[2 * MAX_BEZIER_CURVE_ORDER];
    double resultant[2 * MAX_BEZIER_CURVE_ORDER];
    bsiBezier_univariateProduct (
            PxQy, 0, 1,
            Px, order, 0, 1,
            Qy, order, 0, 1
            );
    bsiBezier_univariateProduct (
            PyQx, 0, 1,
            Py, order, 0, 1,
            Qx, order, 0, 1
            );

    int orderR = 2 * order - 1;
    bsiBezier_subtractPoles (resultant, PxQy, PyQx, orderR, 1);
    double roots[2 * MAX_BEZIER_CURVE_ORDER];
    int numRoots;
    if (!bsiBezier_univariateRoots (roots, &numRoots, resultant, orderR))
        return false;

    for (int i = 0; i < numRoots; i++)
        {
        double u = roots[i];
        DPoint3d xyzA, xyzB, xyz;
        DVec3d tangentA, tangentB, dXdu, dXdv;
        double v;
        bsiBezierDPoint4d_evaluateDPoint3dArray (&xyzA, &tangentA, coffA, order, &u, 1);
        bsiBezierDPoint4d_evaluateDPoint3dArray (&xyzB, &tangentB, coffB, order, &u, 1);
        dXdv.DifferenceOf (xyzB, xyzA);
        if (DoubleOps::ChooseSafeDivideParameter (v, target.x - xyzA.x, dXdv.x, target.y - xyzA.y, dXdv.y, 0.0))
            {
            xyz.Interpolate (xyzA, v, xyzB);
            dXdu.Interpolate (tangentA, v, tangentB);
            SolidLocationDetail detail;
            detail.SetXYZ (xyz);
            detail.SetUV (u, v, dXdu, dXdv);
            pickData.push_back (detail);
            }
        }
    return true;

#endif
    }



Public bool bsiBezier_addRayRuledSurfaceIntersections
(
bvector <SolidLocationDetail> &pickData,
DRay3dCR ray,
DPoint3dCP coffA,
DPoint3dCP coffB,
int order
)
    {
    Transform localToWorld, worldToLocal;
    if (!ray.TryGetViewingTransforms (localToWorld, worldToLocal))
        return false;
    if (order > MAX_BEZIER_CURVE_ORDER)
        return false;
    DPoint3d coffA1[MAX_BEZIER_CURVE_ORDER], coffB1[MAX_BEZIER_CURVE_ORDER];
    worldToLocal.Multiply (coffA1, coffA, order);
    worldToLocal.Multiply (coffB1, coffB, order);
    size_t num0 = pickData.size ();

    if (!bsiBezier_addRuledPatchXYIntersections (pickData, DPoint2d::From (0,0), coffA1, coffB1, order))
        return false;
    size_t num1 = pickData.size ();
    for (size_t i = num0; i < num1; i++)
        {
        SolidLocationDetail & pick = pickData.at (i);
        pick.TransformInPlace (localToWorld);
        double rayFraction;
        DPoint3d xyz1, xyz0 = pick.GetXYZ ();
        ray.ProjectPointUnbounded (xyz1, rayFraction, xyz0);
        pick.SetPickParameter (rayFraction);
        }
    return true;
    }

Public bool bsiBezier_addRayRuledSurfaceIntersections
(
bvector <SolidLocationDetail> &pickData,
DRay3dCR ray,
DPoint4dCP coffA,
DPoint4dCP coffB,
int order
)
    {
    Transform localToWorld, worldToLocal;
    if (!ray.TryGetViewingTransforms (localToWorld, worldToLocal))
        return false;
    Transform product;
    product.InitProduct (localToWorld, worldToLocal);
    if (order > MAX_BEZIER_CURVE_ORDER)
        return false;
    DPoint4d coffA1[MAX_BEZIER_CURVE_ORDER], coffB1[MAX_BEZIER_CURVE_ORDER];
    worldToLocal.Multiply (coffA1, coffA, order);
    worldToLocal.Multiply (coffB1, coffB, order);
    size_t num0 = pickData.size ();

    if (!bsiBezier_addRuledPatchXYIntersections (pickData,
            DPoint2d::From (0,0), coffA1, coffB1, order))
        return false;
    size_t num1 = pickData.size ();
    for (size_t i = num0; i < num1; i++)
        {
        pickData[i].TransformInPlace (localToWorld);
        double rayFraction;
        DPoint3d xyz1, xyz0 = pickData[i].GetXYZ ();
        ray.ProjectPointUnbounded (xyz1, rayFraction, xyz0);
        pickData[i].SetPickParameter (rayFraction);
        }
    return true;
    }




void bsiBezier_evaluateRuled
(
double u,
double v,
DPoint3dCP coffA,
DPoint3dCP coffB,
int order,
DPoint3dR xyz,
DVec3dR dXdu,
DVec3dR dXdv
)
    {
    DPoint3d xyzA, xyzB;
    DVec3d tangentA, tangentB;
    bsiBezier_functionAndDerivative ((double*)&xyzA, (double*)&tangentA,
            const_cast<double*>((double const*)coffA), order, 3, u);
    bsiBezier_functionAndDerivative ((double*)&xyzB, (double*)&tangentB,
            const_cast<double*>((double const*)coffB), order, 3, u);
    xyz.Interpolate (xyzA, v, xyzB);
    dXdv.DifferenceOf (xyzB, xyzA);
    dXdu.Interpolate (tangentA, v, tangentB);
    }
    
struct ClosestPointSearcher
{
DPoint3d m_spacePoint;

DPoint3d m_xyz;
double m_u, m_v;
double m_distance;
ClosestPointSearcher (DPoint3dCR spacePoint)
    {
    m_spacePoint = spacePoint;
    m_distance = DBL_MAX;
    m_u = m_v = 0.0;
    }

void Update (DPoint3dCR xyz, double u, double v)
    {
    double distance = xyz.Distance (m_spacePoint);
    if (distance < m_distance)
        {
        m_xyz = xyz;
        m_u = u;
        m_v = v;
        m_distance = distance;
        }
    }

void Update (DPoint4dCR xyzw, double u, double v)
    {
    DPoint3d xyz;
    if (xyzw.GetProjectedXYZ (xyz))
        Update (xyz, u, v);
    }


bool IsValid () { return m_distance < DBL_MAX;}

void UpdateByProjectionOnLine (DPoint4dCR pointA, DPoint4dCR pointB, double u)
    {
    DPoint3d xyz;
    double v;
    DSegment4d segment = DSegment4d::From (pointA, pointB);
    if (segment.ProjectPointBounded (xyz, v, m_spacePoint))
        Update (xyz, u, v);
    }
    
void UpdateByProjectionOnLine (DPoint3dCR pointA, DPoint3dCR pointB, double u)
    {
    DPoint3d xyz;
    double v;
    DSegment3d segment = DSegment3d::From (pointA, pointB);
    if (segment.ProjectPointBounded (xyz, v, m_spacePoint))
        Update (xyz, u, v);
    }    

bool TransferToDetail (SolidLocationDetailR detail)
    {
    DVec3d ddU, ddV;
    ddU.Zero ();
    ddV.Zero ();
    detail = SolidLocationDetail (0, m_distance, m_xyz, m_u, m_v, ddU, ddV);
    return IsValid ();
    }
};
static double s_maxDelta = 0.1;
Public bool bsiBezier_ruledPatchClosestPoint
(
SolidLocationDetailR pickData,
DPoint3dCR spacePoint,
DPoint4dCP coffA,
DPoint4dCP coffB,
int order
)
    {
    ClosestPointSearcher searcher (spacePoint);
    int numEdge = 4 * order;
    double du = 1.0 / numEdge;
    DPoint4d pointA, pointB;
    for (int i = 0; i <= numEdge; i++)
        {
        double u = i * du;
        bsiBezier_evaluate ((double *)&pointA, const_cast<double*>((double const*)coffA), order, 4, u);
        bsiBezier_evaluate ((double *)&pointB, const_cast<double*>((double const*)coffB), order, 4, u);
        searcher.UpdateByProjectionOnLine (pointA, pointB, u);
        }
    if (searcher.IsValid ())
        {
        RuledBezier_ClosestPoint evaluator(coffA, coffB, order, spacePoint);
        NewtonIterationsRRToRR newton (1.0e-14);
        double u = searcher.m_u;
        double v = searcher.m_v;
        // hm.. This iteration can run outside 01 easily -- bezier evaluation extends.
        if (newton.RunApproximateNewton (u, v, evaluator, s_maxDelta, s_maxDelta))
            {            
            u = DoubleOps::ClampFraction (u);
            v = DoubleOps::ClampFraction (v);
            DPoint4d X;
            DPoint4d dXdu, dXdv;
            evaluator.Evaluate (u, v, X, dXdu, dXdv);
            searcher.Update (X, u, v);
            }
        }

    return searcher.TransferToDetail (pickData);
    }

struct RuledPatchBetweenDEllipse3d
{
DEllipse3d m_ellipse[2];
int m_order;
RuledPatchBetweenDEllipse3d (DEllipse3dCR ellipseA, DEllipse3dCR ellipseB)
    {
    m_ellipse[0] = ellipseA;
    m_ellipse[1] = ellipseB;
    }

void Evaluate (double u, double v, DPoint3dR X, DPoint3dR dXdu, DPoint3dR dXdv)
    {
    DPoint3d xyzAB[2];
    DVec3d dAB[2];
    DVec3d ddAB[2];
    for (int i = 0; i < 2; i++)
        m_ellipse[i].FractionParameterToDerivatives (xyzAB[i], dAB[i], ddAB[i], u);
    X.Interpolate (xyzAB[0], v, xyzAB[1]);
    dXdv.DifferenceOf (xyzAB[1], xyzAB[0]);
    dXdu.Interpolate (dAB[0], v, dAB[1]);
    }
        
bool ClosestPointFunction (DPoint3dCR spacePoint, double u, double v, double &f0, double &f1)
    {
    DPoint3d X;
    DVec3d dXdu, dXdv, E;
    Evaluate (u, v, X, dXdu, dXdv);
    E.DifferenceOf (X, spacePoint);
    f0 = dXdu.DotProduct (E);
    f1 = dXdv.DotProduct (E);
    return true;
    }
};


struct RuledPatchBetweenDEllipse3d_ClosestPoint : RuledPatchBetweenDEllipse3d, FunctionRRToRR
{
DPoint3d m_spacePoint;
RuledPatchBetweenDEllipse3d_ClosestPoint (DEllipse3dCR ellipseA, DEllipse3dCR ellipseB, DPoint3dCR spacePoint)
    : m_spacePoint (spacePoint), RuledPatchBetweenDEllipse3d (ellipseA, ellipseB)
    {
    }
bool EvaluateRRToRR (double u, double v, double &f, double &g)
    {
    return ClosestPointFunction (m_spacePoint, u, v, f, g);
    }
};





static double s_sweepAngleStep = 0.1;
Public bool bsiDEllipse3d_ruledPatchClosestPoint
(
SolidLocationDetailR pickData,
DPoint3dCR spacePoint,
DEllipse3dCR ellipseA,
DEllipse3dCR ellipseB
)
    {
    ClosestPointSearcher searcher (spacePoint);
    DPoint3d pointA, pointB;
    double maxSweep = DoubleOps::MaxAbs (ellipseA.sweep, ellipseB.sweep);
    int numEdge = (int) ceil (maxSweep / s_sweepAngleStep);     // NEEDSWORK: Is this (int) correct? VS2012 complains without it.
    if (numEdge < 3) numEdge = 3;
    double du = 1.0 / numEdge;
    
    for (int i = 0; i <= numEdge; i++)
        {
        double u = i * du;
        ellipseA.FractionParameterToPoint (pointA, u);
        ellipseB.FractionParameterToPoint (pointB, u);
        searcher.UpdateByProjectionOnLine (pointA, pointB, u);
        }
    if (searcher.IsValid ())
        {
        RuledPatchBetweenDEllipse3d_ClosestPoint evaluator(ellipseA, ellipseB, spacePoint);
        NewtonIterationsRRToRR newton (1.0e-14);
        double u = searcher.m_u;
        double v = searcher.m_v;
        // hm.. This iteration can run outside 01 easily -- bezier evaluation extends.
        if (newton.RunApproximateNewton (u, v, evaluator, s_maxDelta, s_maxDelta))
            {            
            u = DoubleOps::ClampFraction (u);
            v = DoubleOps::ClampFraction (v);
            DPoint3d X;
            DVec3d dXdu, dXdv;
            evaluator.Evaluate (u, v, X, dXdu, dXdv);
            searcher.Update (X, u, v);
            }
        }

    return searcher.TransferToDetail (pickData);
    }    

#ifdef EXPERIMENT_DETERMINANT_RAY_PATCH    
// Indices for 6 products A[i]*B[j]*(C[k]*D[l]-C[l]*D[k])
// These are added to from determinant -- the ordering of k and l provides sign effects.
struct Determinant4Indices
    {
    int i;
    int j;
    int k;
    int l;
    };    
Public bool bsiBezier_addRuledPatchRayIntersections
(
bvector <SolidLocationDetail> &pickData,
DPoint4dCR pointE,
DPoint4dCR pointW,
DPoint4dCR coffA,
int orderA,
DPoint4dCR coffB,
int orderB
)
    {
// The patch is points
//   X = (1-v)A(u) + vB(u)
// The ray is points
//   E + wW
// The ray pierces along the ruling at u if A(u), B(u), E, W are coplanar, i.e. the determinant of those 4 4d points is zero.
//
//  | Ax  Bx  Ex  Wx |
//  | Ay  By  Ey  Wy |
//  | Az  Bz  Ez  Wz |
//  | Aw  Bw  Ew  Ww |
//
    const int numPositivePermutations = 12;
    static Determinant4Indices indices[numPositivePermutations] =
        {
        {0,1,2,3},
        {0,2,3,1},
        {0,3,1,2},
        
        {1,2,0,3},
        {1,3,2,0},
        {1,0,3,2},
        
        {2,3,0,1},
        {2,0,1,3},
        {2,1,3,0},
        
        {3,0,2,1},
        {3,2,1,0},
        {3,1,0,2},        
        };
    double *pE = &pointE.x;     // So it acts like an array of 0,1,2,3...
    double *pW = &pointE.x;
    double term[MAX_BEZIER_ORDER];
    double sum[MAX_BEZIER_ORDER];
    int orderAB = orderA + orderB - 1;
    for (int i = 0; i < orderAB; i++)
        sum[i] = 0.0;
    for (int p = 0; p < numPositivePermutations; p++)
        {
        int i = indices[p].i;
        int j = indices[p].j;
        int k = indices[p].k;
        int l = indices[p].l;
        // 2x2 determinant of the kl positions ....
        double ew = pE[k] * pW[l] - pE[l] * pW[k];
        // ah, at this point I see the ray-patch function above.  It "looks down the ray" and asks where the patch hits 00 in that xy
        // coordinate system.
        // so we'll leave this one hanging ...
        }
    return true;
    }
#endif    
END_BENTLEY_GEOMETRY_NAMESPACE