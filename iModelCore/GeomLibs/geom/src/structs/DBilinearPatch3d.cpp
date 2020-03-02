/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     08/12
+--------------------------------------------------------------------------------------*/
DBilinearPatch3d::DBilinearPatch3d
(
DPoint3dCR xyz00,
DPoint3dCR xyz10,
DPoint3dCR xyz01,
DPoint3dCR xyz11
)
    {
    point[0][0] = xyz00;
    point[1][0] = xyz10;
    point[0][1] = xyz01;
    point[1][1] = xyz11;
    }


DBilinearPatch3d::DBilinearPatch3d
(
DPoint2dCR xyz00,
DPoint2dCR xyz10,
DPoint2dCR xyz01,
DPoint2dCR xyz11
)
    {
    point[0][0].Init (xyz00);
    point[1][0].Init (xyz10);
    point[0][1].Init (xyz01);
    point[1][1].Init (xyz11);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     08/12
+--------------------------------------------------------------------------------------*/
DBilinearPatch3d::DBilinearPatch3d ()
    {
    point[0][0].Zero ();
    point[1][0].Zero ();
    point[0][1].Zero ();
    point[1][1].Zero ();
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     08/12
+--------------------------------------------------------------------------------------*/
DBilinearPatch3d::DBilinearPatch3d
(
DSegment3dCR lowerEdge, DSegment3dCR upperEdge
)
    {
    point[0][0] = lowerEdge.point[0];
    point[1][0] = lowerEdge.point[1];
    point[0][1] = upperEdge.point[0];
    point[1][1] = upperEdge.point[1];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     08/12
+--------------------------------------------------------------------------------------*/
void DBilinearPatch3d::Evaluate (double u, double v, DPoint3dR xyz, DVec3dR dXdu, DVec3dR dXdv) const
    {
    DVec3d U0, U1, V0, V1;
    U0.DifferenceOf (point[1][0], point[0][0]);
    U1.DifferenceOf (point[1][1], point[0][1]);
    V0.DifferenceOf (point[0][1], point[0][0]);
    V1.DifferenceOf (point[1][1], point[1][0]);
    dXdu.SumOf (U0, 1.0 - v, U1, v);
    dXdv.SumOf (V0, 1.0 - u, V1, u);
    xyz.SumOf (point[0][0], U0, u, dXdv, v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     08/12
+--------------------------------------------------------------------------------------*/
void DBilinearPatch3d::EvaluateNormal (double u, double v, DPoint3dR xyz, DVec3dR unitNormal) const
    {
    DVec3d dXdu, dXdv;
    DVec3d unusedVector;
    DPoint3d unusedPoint;
    Evaluate (u, v, xyz, dXdu, dXdv);
    double tolerance = DoubleOps::SmallMetricDistance ();
    // if local parameter space is collapsed, move transversely in parameter space to a nearby parallel  . .
    if (dXdu.Magnitude () < tolerance)
        Evaluate (u, DoubleOps::Interpolate (0.5, 0.95, v), unusedPoint, dXdu, unusedVector);
    if (dXdv.Magnitude() < tolerance)
        Evaluate(DoubleOps::Interpolate(0.5, 0.95, u), v, unusedPoint, unusedVector, dXdv);
    unitNormal.NormalizedCrossProduct (dXdu, dXdv);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     08/12
+--------------------------------------------------------------------------------------*/
DPoint3d DBilinearPatch3d::Evaluate (double u, double v) const
    {
    DPoint3d xyz;
    double v0 = 1.0 - v;
    double u0 = 1.0 - u;
    xyz.x =  u0 * (v0 * point[0][0].x + v * point[0][1].x)
          +  u  * (v0 * point[1][0].x + v * point[1][1].x);
    xyz.y =  u0 * (v0 * point[0][0].y + v * point[0][1].y)
          +  u  * (v0 * point[1][0].y + v * point[1][1].y);
    xyz.z =  u0 * (v0 * point[0][0].z + v * point[0][1].z)
          +  u  * (v0 * point[1][0].z + v * point[1][1].z);
    return xyz;
    }



//! return up to 5 (!?!) uv coordinates of projections of xyz onto the (bounded) patch.
bool DBilinearPatch3d::PerpendicularsOnBoundedPatch
(
DPoint3dCR xyz,
bvector<DPoint2d> &uv
) const
    {
    uv.clear ();
    // Y = X - C = (1-s)(1-t)V00 + (1-s)t V01 + s(t-t)V10 + st V11
    // dYds = (1-t)U0 + t U1        U0,U1 are vectors along lower and upper edges
    // dYdt = (1-s)V0 + s V1        V0,V1 are vectors along left and right edges.
    // Y.dYds = 0 ===>  (1-s)q0 + s q1 = 0
    //      where q0, q1 are quadratics in t, built in bezier form below.
    // hence   s   = -q0/(q1-q0)
    //  and  (1-s) = q1/(q1-q0)
    DVec3d cornerVector00 = DVec3d::FromStartEnd (point[0][0], xyz);
    DVec3d cornerVector10 = DVec3d::FromStartEnd (point[1][0], xyz);
    DVec3d cornerVector01 = DVec3d::FromStartEnd (point[0][1], xyz);
    DVec3d cornerVector11 = DVec3d::FromStartEnd (point[1][1], xyz);

    DVec3d edgeU0 = DVec3d::FromStartEnd (point[0][0], point[1][0]);
    DVec3d edgeU1 = DVec3d::FromStartEnd (point[0][1], point[1][1]);

    Polynomial::Bezier::Order3 q0 (edgeU0.DotProduct (cornerVector00),
                                 0.5 * (edgeU1.DotProduct (cornerVector00)
                                       +edgeU0.DotProduct (cornerVector01)),
                                  edgeU1.DotProduct (cornerVector01));
    Polynomial::Bezier::Order3 q1 (edgeU0.DotProduct (cornerVector10),
                                 0.5 * (edgeU1.DotProduct (cornerVector10)
                                       +edgeU0.DotProduct (cornerVector11 )),
                                  edgeU1.DotProduct (cornerVector11));
    // Now (!!) s = -q0(t)/(q1(t)-q0(t))
    // and   (1-s) = q1(t) /(q1(t)-q0(t))
    // When you plug this in to the (homogenous) bilinears and derivatives, the denominator
    //  appears everywhere and can be ignored in the (Y-X(s,t))DOT dX(s,t)/ds = 0 conditions.
    // rotated q's as subsitutes for (1-s), s
    Polynomial::Bezier::Order3 r[2] =
        {
        Polynomial::Bezier::Order3 (q1,  1.0),
        Polynomial::Bezier::Order3 (q0, -1.0),        
        };
    Polynomial::Bezier::Order4 cubic[4] =
        {
        Polynomial::Bezier::Order4 (r[0], Polynomial::Bezier::Order2 (1,0)),
        Polynomial::Bezier::Order4 (r[0], Polynomial::Bezier::Order2 (0,1)),
        Polynomial::Bezier::Order4 (r[1], Polynomial::Bezier::Order2 (1,0)),
        Polynomial::Bezier::Order4 (r[1], Polynomial::Bezier::Order2 (0,1))
        };

    DVec3d                      vecA[4] =
        {
        cornerVector00,
        cornerVector01,
        cornerVector10,
        cornerVector11
        };

        
    DVec3d edgeV[2] = 
        {
        DVec3d::FromStartEnd (point[0][0], point[0][1]),
        DVec3d::FromStartEnd (point[1][0], point[1][1])
        };
        
    // accumulate cubic[i]*r[j] * vecA[i] DOT edgeV[j];
    Polynomial::Bezier::Order6 resultant;   // all zeros
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 2; j++)
            resultant.AddProduct (r[j], cubic[i], vecA[i].DotProduct (edgeV[j]));
    double roots[MAX_BEZIER_ORDER];
    int numRoots;
    if (resultant.Solve (roots, numRoots, MAX_BEZIER_ORDER))
        {
        for (int i = 0; i < numRoots; i++)
            {
            double t = roots[i];
            double a0 = q0.Evaluate (t);
            double a1 = q1.Evaluate (t);
            double s;
            if (DoubleOps::SafeDivide (s, -a0, a1 - a0, 0.0))
                {
                if (s >= 0.0 && s <= 1.0)
                    {
                    uv.push_back (DPoint2d::From (s,t));
                    }
                }
            }
        }
    return uv.size () > 0;
    }

DVec3d DBilinearPatch3d::GetVEdgeVector (int i) const
    {
    DVec3d vector;
    i =  (i &0x01);
    vector.DifferenceOf (point[i][1], point[i][0]);
    return vector;
    }

DVec3d DBilinearPatch3d::GetUEdgeVector (int i) const
    {
    DVec3d vector;
    i =  (i &0x01);
    vector.DifferenceOf (point[1][i], point[0][i]);
    return vector;
    }

DVec3d DBilinearPatch3d::GetDiagonalFrom00 () const
    {
    return DVec3d::FromStartEnd (point[0][0], point[1][1]);
    }

DVec3d DBilinearPatch3d::GetDiagonalFrom01 () const
    {
    return DVec3d::FromStartEnd (point[0][1], point[1][0]);
    }


bool DBilinearPatch3d::IsParallelogram () const
    {
    DVec3d vectorA = GetVEdgeVector (0);
    DVec3d vectorB = GetVEdgeVector (1);
    return DVec3dOps::AlmostEqual (vectorA, vectorB);
    }

static bool ShortOrPerpendicularToUnitVector (DVec3dCR U, double minSquaredLength, DVec3dCR V, double tolerance)
    {
    double UdotU = U.MagnitudeSquared ();
    if (UdotU < minSquaredLength)
        return true;
    double UdotV = U.DotProduct (V);
    return UdotV * UdotV < tolerance * tolerance * UdotU;
    }
bool DBilinearPatch3d::IsPlanar (double angleTol) const
    {
    // The patch is planar if the U0, V0, and W vectors have zero triple product.  (W = vector from P00+U0+U1 to P11)
    // but testing for zero is a scaling puzzle.
    // Try vectors:
    // Naive logic: normalA = U0.Cross.V1 edge vector.  normalB = V0.Cross.U1.  These should be parallel.
    //  But if one of the edges is collapsed there is a zero vector in there.  Don't know how the parallel tests handle that.
    //  So get the average normal by U, V edges....
    DVec3d U0 = GetUEdgeVector (0);
    DVec3d U1 = GetUEdgeVector (1);

    DVec3d V0 = GetVEdgeVector (0);
    DVec3d V1 = GetVEdgeVector (1);

    // Compute twice the average U and v vectors.   If at most one edge is degenerate, these will be nonzero.
    DVec3d U, V;
    U.SumOf (U0, U1);
    V.SumOf (V0, V1);
    static double s_shortVectorSquaredFactor = 1.0e-20;
    double aU = s_shortVectorSquaredFactor * U.MagnitudeSquared ();
    double aV = s_shortVectorSquaredFactor * V.MagnitudeSquared ();
    DVec3d refNormal;
    refNormal.NormalizedCrossProduct (U, V);
    // Require that each edge that is plainly not zero (compared to its partner) is perpendicular.
    // Mutter and grumble: 4 tests.  Why not just 1 or 2?
    return  ShortOrPerpendicularToUnitVector (U0, aU, refNormal, angleTol)
         && ShortOrPerpendicularToUnitVector (U1, aU, refNormal, angleTol)
         && ShortOrPerpendicularToUnitVector (V0, aV, refNormal, angleTol)
         && ShortOrPerpendicularToUnitVector (V1, aV, refNormal, angleTol);
    }

bool DBilinearPatch3d::IsPlanar () const
    {
    return IsPlanar (Angle::SmallAngle ());
    }


DSegment3d DBilinearPatch3d::GetCCWEdge (int i) const
    {
    i =  (i & 0x03);    // only 00,01,10,11 bits left
    switch (i)
        {
        case 0: return DSegment3d::From (point[0][0], point[1][0]);
        case 1: return DSegment3d::From (point[1][0], point[1][1]);
        case 2: return DSegment3d::From (point[1][1], point[0][1]);
        case 3: return DSegment3d::From (point[0][1], point[0][0]);
        }
    // unreachable after masking !!!
    return DSegment3d::From (0,0,0,0,0,0);
    }

void DBilinearPatch3d::EvaluateGrid (int numUPoint, int numVPoint, bvector<DPoint3d> &gridPoints) const
    {
    if (numUPoint < 2)
        numUPoint = 2;
    if (numVPoint < 2)
        numVPoint = 2;
    gridPoints.clear ();
    double du = 1.0 / (double)(numUPoint - 1);
    double dv = 1.0 / (double)(numVPoint - 1);
    for (int j = 0; j < numVPoint; j++)
        {
        double v = j * dv;
        for (int i = 0; i < numUPoint; i++)
            {
            gridPoints.push_back (Evaluate (i * du, v));
            }
        }
    }



END_BENTLEY_GEOMETRY_NAMESPACE
