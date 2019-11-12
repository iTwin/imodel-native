/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


#define NUM_PATH_CENTROID_INTEGRALS 12

// Rectangular strip of (infintessimal) width (dw) from A to B.
// Strip length is L = Distance (A, B)
// Point on strip at fraction u is
//    X = (1-u)A + uB
//    <f> denotes {integral_{u=0 to u=1} f(u) du}
//  second moment integrals {xx xy xz;yx yy yz;zx zy zz} are
//  <X X^ wL>= <((1-u)A + uB)((1-u)A^ + uB^) wL>
//        = (dw)L((AA^/3 + (BA^+AB^)/6 + BB^/3)
// With B = A + U:
//        dw L (AA^/3 + (A+U)A^+A(A+U)^/6 + (A+U)(A+U)^/3
//      = dw L (AA^ + (UA^+AU^)/2 + UU^/3)
//
// Triangular sliver from A to B, with B end along (infintessimal) vector (Vdv)
// At fraction u, the area differential is dA = u du Magnitude ((B-A) cross V) dV = u du J dV
//    where J = Magnituce ((B-A) cross V)
// Second moment integrals are
//   <XX^ u du> J dv
//          = <((1-u)A + uB)((1-u)A^ + uB^) u>
//          = J dv (AA^/12 + (BA^+AB^)/12 + BB^/4)
//  Integrate dv from v=0 to v=1, with B = C + v V, and D = C+V/2 = A + U + V/2
//      J(AA^/12 + (AD^ + DA^)/12 + CC^/4 + (CV^+VC^)/4 + VV^/12)
// or with B-A=U
// J(AA^/2 + (AU^+UA^)/3 + (AV^+VA^)/6 + UU^/4 + (UV^+VU^)/8 + VV^/12)
//
#ifdef CompileAllMomentCode

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static RotMatrix LineProducts
(
DPoint3dCR pointA,
DPoint3dCR pointB
)
    {
    static const double div3 = 1.0 / 3.0;
    DVec3d A = DVec3d::From (pointA);
    DVec3d U = DVec3d::FromStartEnd (pointA, pointB);

    RotMatrix E;
    E.Zero ();
    E.AddScaledOuterProductInPlace (A, A, 1.0);

    E.AddScaledOuterProductInPlace (A, U, 0.5);
    E.AddScaledOuterProductInPlace (U, A, 0.5);

    E.AddScaledOuterProductInPlace (U, U, div3);
    double a = U.Magnitude ();
    E.ScaleColumns (E, a, a, a);
    return E;    
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static RotMatrix TriangleProducts
(
DPoint3dCR pointA,
DPoint3dCR pointB,
DPoint3dCR pointC
)
    {
    static const double div3 = 1.0 / 3.0;
    static const double div4 = 0.25;
    static const double div6 = 1.0 / 6.0;
    static const double div8 = 0.125;
    static const double div12 = 1.0 / 12.0;
    DVec3d A = DVec3d::From (pointA);
    DVec3d U = DVec3d::FromStartEnd (pointA, pointB);
    DVec3d V = DVec3d::FromStartEnd (pointB, pointC);
    RotMatrix E;
    E.Zero ();
    E.AddScaledOuterProductInPlace (A, A, 0.5);

    E.AddScaledOuterProductInPlace (A, U, div3);    // div4
    E.AddScaledOuterProductInPlace (U, A, div3);

    E.AddScaledOuterProductInPlace (A, V, div6);
    E.AddScaledOuterProductInPlace (V, A, div6);

    E.AddScaledOuterProductInPlace (U, U, div4);    // div6?

    E.AddScaledOuterProductInPlace (U, V, div8);    // div9 ?
    E.AddScaledOuterProductInPlace (V, U, div8);

    E.AddScaledOuterProductInPlace (V, V, div12);
    DVec3d J;
    double a = J.NormalizedCrossProduct (U, V);
    E.ScaleColumns (E, a, a, a);
    return E;
    }
#endif
typedef struct
    {
    bool    bFixedZ;     // If true, force geometry to origin z.
    bool    bEnableProducts;
    double sum[NUM_PATH_CENTROID_INTEGRALS];
    DPoint3d origin;
    } PathIntegralContext;
#ifdef CompileAll

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static double maxAbsDiff(double *dataA, double *dataB, size_t count)
    {
    double eMax = 0.0;
    for (size_t i = 0; i < count; i++)
        {
        double e = fabs (dataA[i] - dataB[i]);
        if (e > eMax)
            eMax = e;
        }
    return eMax;
    }
#endif

/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct BezierSweptMomentEvaluator : BSIVectorIntegrand
{
DPoint3d m_origin;
DPoint3d m_xyz0;
DPoint3d m_xyz1;
DVec3d   m_edgeVector;
DVec3d   m_unitEdgeVector;
BCurveSegment &m_bezierCurve;

BezierSweptMomentEvaluator (BCurveSegment &bezier,
DPoint3dCR origin,
DPoint3dCR xyz0,
DPoint3dCR xyz1
)
    :
    m_bezierCurve (bezier),
    m_origin (origin),
    m_xyz0 (xyz0),
    m_xyz1 (xyz1)
    {
    m_edgeVector = DVec3d::FromStartEnd (xyz0, xyz1);
    //double uu = m_edgeVector.DotProduct(m_edgeVector);
    m_unitEdgeVector.Normalize (m_edgeVector);
    }

/*---------------------------------------------------------------------------------**//**
* Compute path integrands at specified angle on a conic.
* @param
* @return
* @bsimethod                                                    EarlinLutz      05/99
+--------------------------------------------------------------------------------------*/
void   EvaluateCentroidIntegrands
(
double  *pF,
double  theta,
int     numFunc
)
    {
    DPoint3d point;
    DVec3d   tangent;

    bsiBezierDPoint4d_evaluateDPoint3dArray
                (
                &point, &tangent,
                m_bezierCurve.GetPoleP (),
                (int)m_bezierCurve.GetOrder (),
                &theta, 1);
    
    DVec3d vectorV = DVec3d::FromStartEnd (m_xyz0, point);
    double distanceAlong = vectorV.DotProduct (m_unitEdgeVector);
    DPoint3d edgePoint;
    edgePoint.SumOf (m_xyz0, m_unitEdgeVector, distanceAlong);
    double a = tangent.DotProduct (m_unitEdgeVector);
    DVec3d averageTangent, perp;
    averageTangent.SumOf (tangent, 0.5, m_unitEdgeVector, 0.5 * a);
    perp.SumOf (vectorV, m_unitEdgeVector, -distanceAlong);
    DVec3d normal  = DVec3d::FromCrossProduct (perp, averageTangent);  // normal * dx is area of differential bar from point back to edge.
    DPoint3d centroid;
    DVec3d centroidVector;
    centroid.Interpolate (edgePoint, 0.5, point);
    centroidVector.DifferenceOf (centroid, m_origin);    

    RotMatrix products;
    products.Zero ();
    products.AddScaledOuterProductInPlace (centroidVector, normal, 1.0);
    pF[0] = normal.x;
    pF[1] = normal.y;
    pF[2] = normal.z;

    products.GetRowValues
            (
            pF[3], pF[ 4], pF[ 5],
            pF[6], pF[ 7], pF[ 8],
            pF[9], pF[10], pF[11]
            );
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void EvaluateVectorIntegrand(double u,double *pF) override
    {
    EvaluateCentroidIntegrands (pF, u, 12);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
int GetVectorIntegrandCount() override
    {
    return 12;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void Integrate(double u0, double u1, double *pF, int numIntegral, int numGauss)
    {
    BSIQuadraturePoints quadPoints;
    quadPoints.InitGauss(numGauss);
    double a, w;
    double ff[20];
    double F0[20];
    double F1[20];
    double v0 = u0;
    double v1 = 0.5 * (u0 + u1);
    double w0 = v1;
    double w1 = u1;
    memset (F0, 0, numIntegral * sizeof (double));
    memset (F1, 0, numIntegral * sizeof (double));
    for (int i = 0; quadPoints.GetEval (i, u0, u1, a, w); i++)
        {
        EvaluateCentroidIntegrands (ff, a, numIntegral);
        for (int k = 0; k < numIntegral; k++)
            F0[k] += ff[k] * w;
        }
    for (int i = 0; quadPoints.GetEval (i, v0, v1, a, w); i++)
        {
        EvaluateCentroidIntegrands (ff, a, numIntegral);
        for (int k = 0; k < numIntegral; k++)
            F1[k] += ff[k] * w;
        }
    for (int i = 0; quadPoints.GetEval (i, w0, w1, a, w); i++)
        {
        EvaluateCentroidIntegrands (ff, a, numIntegral);
        for (int k = 0; k < numIntegral; k++)
            F1[k] += ff[k] * w;
        }
    // REMARK: Here we should say pF[k] = F1[i] + e * (F1[k]-f0[k]) where e is the Richardson extrapolation factor.
    // That is for another day.
    for (int k = 0; k < numIntegral; k++)
        {
        pF[k] = F1[k];
        }
    //double eMax = maxAbsDiff (pF, F0, numIntegral);    
    }
};
#ifdef CompileAll

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void cb_bezierPathIntegralDispatcher
(
double  *pF,
double  theta,
BezierSweptMomentEvaluator *pBezier,
int     numFunc
)
    {
    pBezier->EvaluateCentroidIntegrands (pF, theta, numFunc);
    }
#endif

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void computeMomentIntegrals
(
DVec3dR                 normalSum,
RotMatrixR              centroidTensor,
double                  u0,
double                  u1,
DPoint3dCR              origin,
BCurveSegment           &bezier,
double                  normalTol,
double                  centroidTol
)
    {
    double integral[NUM_PATH_CENTROID_INTEGRALS];

    int numFunc = 12;
    DPoint3d xyz0, xyz1;
    bezier.FractionToPoint (xyz0, u0);
    bezier.FractionToPoint (xyz1, u1);

    // (Hopefull small and easily integrable) area swept from chord to curve.
    BezierSweptMomentEvaluator evaluator (bezier, origin, xyz0, xyz1);
    evaluator.Integrate (u0, u1, integral, numFunc, 5);
#ifdef NCCheck
    double integralNC[20], error[20];
    int count;
    double absTol = centroidTol;
    static double s_relTol = 1.0e-12;
    bsiMath_recursiveNewtonCotes5Vector (integralNC, error, &count,
                    u0, u1, absTol, s_relTol,
                    (PFVectorIntegrand)cb_bezierPathIntegralDispatcher, &evaluator, numFunc);
    double integralA[NUM_PATH_CENTROID_INTEGRALS];
    evaluator.Integrate (u0, u1, integralA, numFunc, 4);
    double eMax = maxAbsDiff (integral, integralNC, numFunc);
#endif
    normalSum.Init (integral[0], integral[1], integral[2]);
    centroidTensor = RotMatrix::FromRowValues
        (
        integral[3], integral[ 4], integral[ 5],
        integral[6], integral[ 7], integral[ 8],
        integral[9], integral[10], integral[11]
        );

    // triangle from origin to xyz0,xyz1
    DVec3d vector0 = DVec3d::FromStartEnd (origin, xyz0);
    DVec3d vector1 = DVec3d::FromStartEnd (origin, xyz1);
    DVec3d centroidVector;
    double a = 1.0 / 3.0;
    centroidVector.SumOf (vector0, a, vector1, a);
    DVec3d cross;
    cross.CrossProduct (vector0, vector1);
    normalSum.SumOf (normalSum, cross, 0.5);
    centroidTensor.AddScaledOuterProductInPlace (centroidVector, cross, 0.5);
    }


GEOMDLLIMPEXP bool CurveVector::GetStartPoint (DPoint3dR point) const
    {
    for (size_t i = 0,  n = size (); i < n; i++)
        {
        if (at(i)->GetStartPoint (point))
            return true;
        }
    return false;
    }

GEOMDLLIMPEXP bool  CurveVector::WireCentroid
(
double &length,
DPoint3dR centroid
) const
    {
    DPoint3d weightedCentroidSum;
    size_t numComponent = 0;
    centroid.Zero ();
    weightedCentroidSum.Zero ();
    length = 0.0;
    //double a = 0.0;
    for (size_t i = 0, n = size (); i < n; i++)
        {
        double componentLength;
        DVec3d componentCentroid;
        if (at(i)->WireCentroid (componentLength, componentCentroid))
            {
            length += componentLength;
            weightedCentroidSum.SumOf (weightedCentroidSum, componentCentroid, componentLength);
            numComponent++;
            }
        }
    if (numComponent > 0.0
        && centroid.SafeDivide (weightedCentroidSum, length))
        return true;
    return false;
    }

GEOMDLLIMPEXP DRange1d CurveVector::ProjectedParameterRange (DRay3dCR ray) const
    {
    DRange1d dotRange = DRange1d ();        // For lines etc, record range of simple dot products -- needs to be divided by 
                                            // rayVectorMagnitudeSquared () at end.
    DRange1d parameterRange = DRange1d ();  // The real result.
    double rayVectorMagnitudeSquared = ray.direction.MagnitudeSquared ();

    for (size_t i = 0, n = size (); i < n; i++)
        {
        ICurvePrimitive::CurvePrimitiveType curveType = at(i)->GetCurvePrimitiveType ();
        switch (curveType)
            {
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                {
                DSegment3dCP segment = at(i)->GetLineCP ();
                dotRange.Extend (ray.DirectionDotVectorToTarget (segment->point[0]));
                dotRange.Extend (ray.DirectionDotVectorToTarget (segment->point[1]));
                break;
                }
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                {
                bvector<DPoint3d>const *points = at(i)->GetLineStringCP ();
                for (size_t i = 0, n = points->size (); i < n; i++)
                    dotRange.Extend (points->at(i).DotDifference (ray.origin, ray.direction));
                break;
                }
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                {
                DEllipse3dCP ellipse = at(i)->GetArcCP ();
                DPoint3d arcPts[2];
                ellipse->EvaluateEndPoints(arcPts[0], arcPts[1]);
                double a = ray.DirectionDotVectorToTarget (ellipse->center);
                double u = ray.DirectionDotVector (ellipse->vector0);
                double v = ray.DirectionDotVector (ellipse->vector90);
                double s0 = ray.DirectionDotVectorToTarget(arcPts[0]);
                double s1 = s0;
                bsiEllipse_componentRange (&s0, &s1, a, u, v, ellipse->start, ellipse->sweep);
                dotRange.Extend (s0);
                dotRange.Extend (s1);
                dotRange.Extend(ray.DirectionDotVectorToTarget(arcPts[1]));
                break;
                }
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector:
                {
                CurveVectorCP       childCurveVector = at(i)->GetChildCurveVectorCP();
                
                if (NULL != childCurveVector)
                    {
                    DRange1d            curveVectorRange = childCurveVector->ProjectedParameterRange (ray);

                    if (!curveVectorRange.IsNull())
                        parameterRange.Extend (curveVectorRange);
                    }
                break;
                }

            default:
                {
                MSBsplineCurveCP bcurve = at(i)->GetProxyBsplineCurveCP ();
                if (NULL != bcurve)
                    {
                    DRange1d curveRange = bcurve->GetRangeOfProjectionOnRay (ray, 0.0, 1.0);
                    if (!curveRange.IsNull ())
                        parameterRange.Extend (curveRange);
                    }
                break;
                }
            }
        }

    // line, linestring, ellipse have been accumulating range of simple dot products (not yet divided by ray vector dot)
    // Do the division and extend the parameterRange.
    double a0, a1;
    if (!dotRange.IsNull ()
        && DoubleOps::SafeDivideParameter (a0, dotRange.low, rayVectorMagnitudeSquared, 0.0)
        && DoubleOps::SafeDivideParameter (a1, dotRange.high, rayVectorMagnitudeSquared, 0.0)
        )
        {
        parameterRange.Extend (a0);
        parameterRange.Extend (a1);
        }

    return parameterRange;
    }



/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct CentroidSums : public ICurvePrimitiveProcessor
{
double m_absAreaSum;
RotMatrix m_centroidTensor;
DVec3d m_normalSum;
DPoint3d m_referencePoint;
Transform m_worldToLocal;


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ApplyWorldToLocal(DPoint3dR point)
    {
    m_worldToLocal.Multiply (point);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ApplyWorldToLocal(DVec3dR vector)
    {
    m_worldToLocal.MultiplyMatrixOnly (vector);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ApplyWorldToLocal(BCurveSegment &bezier)
    {
    DPoint4dP poles = bezier.GetPoleP ();
    for (size_t i = 0, n = bezier.GetOrder ();
            i < n; i++)
        {
        m_worldToLocal.Multiply (poles[i], poles[i]);
        }
    }

CentroidSums (DPoint3d referencePoint, TransformCR worldToLocal)
    {
    m_referencePoint = referencePoint;
    m_centroidTensor.Zero ();
    m_absAreaSum = 0.0;
    m_normalSum.Zero ();
    m_worldToLocal = worldToLocal;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool GetCentroidNormalArea(DPoint3dR centroid, DVec3dR unitNormal, double &area)
    {
    area = unitNormal.Normalize (m_normalSum);
    if (area <= Angle::SmallAngle () * m_absAreaSum)
        {
        centroid = m_referencePoint;
        unitNormal.Zero ();
        area = 0.0;
        return false;
        }
    DVec3d centroidVector;
    m_centroidTensor.Multiply (centroidVector, unitNormal);
    centroid.SumOf (m_referencePoint, centroidVector, 1.0 / area);
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void AddAreaMoment(DVec3dCR centroidVector, DVec3dCR normalVector, double normalScale, double momentScale)
    {
    m_normalSum.SumOf (m_normalSum, normalVector, normalScale);
    m_centroidTensor.AddScaledOuterProductInPlace (centroidVector, normalVector, momentScale);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void AddAreaMoment(DVec3dCR normalVector, RotMatrixR centroidTensor)
    {
    m_normalSum.SumOf (m_normalSum, normalVector);
    m_centroidTensor.Add (centroidTensor);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void AddSegment(DPoint3dCR pointA, DPoint3dCR pointB)
    {
    DVec3d vectorA, vectorB, crossVector, momentVector;
    vectorA.DifferenceOf (pointA, m_referencePoint);
    vectorB.DifferenceOf (pointB, m_referencePoint);
    crossVector.CrossProduct (vectorA, vectorB);
    double f = 1.0 / 3.0;
    momentVector.SumOf (vectorA, f, vectorB, f);
    AddAreaMoment (momentVector, crossVector, 0.5, 0.5);
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void _ProcessLine(ICurvePrimitiveCR curve, DSegment3dCR source, DSegment1dCP interval) override
    {
    DSegment3d segment = source;
    ApplyWorldToLocal (segment.point[0]);
    ApplyWorldToLocal (segment.point[1]);

    if (NULL == interval)
        AddSegment (segment.point[0], segment.point[1]);
    else
        {
        AddSegment (DPoint3d::FromInterpolate (
                        segment.point[0], interval->GetStart (), segment.point[1]),
                    DPoint3d::FromInterpolate (
                        segment.point[0], interval->GetEnd (), segment.point[1])
                    );
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void _ProcessArc(ICurvePrimitiveCR curve, DEllipse3dCR ellipse, DSegment1dCP interval) override
    {
    DEllipse3d myEllipse = ellipse;
    ApplyWorldToLocal (myEllipse.center);
    ApplyWorldToLocal (myEllipse.vector0);
    ApplyWorldToLocal (myEllipse.vector90);

    if (NULL != interval)
        myEllipse = DEllipse3d::FromFractionInterval (ellipse, interval->GetStart (), interval->GetEnd ());
    DPoint3d startPoint, endPoint;
    myEllipse.EvaluateEndPoints (startPoint, endPoint);
    AddSegment (startPoint, myEllipse.center);
    AddSegment (myEllipse.center, endPoint);
    DVec3d crossVector = DVec3d::FromCrossProduct (myEllipse.vector0, myEllipse.vector90);

    double alpha = myEllipse.sweep * 0.5;
    double radiusFraction = 2.0 * sin (alpha) / (3.0 * alpha); // fractional move towards ellipse along middle angle
    double unitArea = alpha;  // sector of unit circle, will be scaled by our vector cross product.
    double midAngle = myEllipse.start + 0.5 * myEllipse.sweep;
    DPoint3d centroidPoint;
    centroidPoint.SumOf (myEllipse.center,
                myEllipse.vector0,  radiusFraction * cos (midAngle),
                myEllipse.vector90, radiusFraction * sin (midAngle)
                );
    DVec3d centroidVector = DVec3d::FromStartEnd (m_referencePoint, centroidPoint);
    AddAreaMoment (centroidVector, crossVector, unitArea, unitArea);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void _ProcessLineString(ICurvePrimitiveCR curve, bvector<DPoint3d> const &points, DSegment1dCP interval) override
    {
   if (NULL == interval)
        {
        size_t n = points.size ();
        for (size_t i = 1; i < n; i++)
            AddSegment (points.at(i-1), points.at(i));
        }
    else
        {
        // NEEDS WORK
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void _ProcessBsplineCurve(ICurvePrimitiveCR curve, MSBsplineCurveCR bcurve, DSegment1dCP interval) override
    {
    BCurveSegment segment;
    DVec3d normalSum;
    RotMatrix tensorSum;
    DRange3d range;
    static double s_angleTol = 0.3;
    bcurve.GetPoleRange (range);
    double s_relTol = 1.0e-14;
    double diagonal = range.low.Distance (range.high);
    double normalTol = s_relTol * diagonal * diagonal;
    double centroidTol = s_relTol * diagonal * diagonal * diagonal;
    if (NULL == interval)
        {
        for (size_t i = 0; bcurve.GetBezier (segment, i); i++)
            {
            if (!segment.IsNullU ())
                {
                ApplyWorldToLocal (segment);
                int numEdge = bsiBezierDPoint4d_estimateEdgeCount (segment.GetPoleP (), (int)segment.GetOrder (), 0.0, s_angleTol, 0.0, bcurve.rational? false : true);
                double df = 1.0 / (double)numEdge;
                for (int i = 0; i < numEdge; i++)
                    {
                    computeMomentIntegrals (normalSum, tensorSum,
                            i * df, (i + 1) * df,
                            m_referencePoint, segment,
                            normalTol, centroidTol);
                    AddAreaMoment (normalSum, tensorSum);
                    }
                }
            }
        }
    else
        {
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void _ProcessProxyBsplineCurve(ICurvePrimitiveCR curve, MSBsplineCurveCR bcurve, DSegment1dCP interval) override
    {
    _ProcessBsplineCurve (curve, bcurve, interval);
    }
#ifdef CentroidHandleAllProcessMethods

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void _ProcessPartialCurve(ICurvePrimitiveCR curve, PartialCurveDetailCR detail, DSegment1dCP interval) override
    {
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void _ProcessInterpolationCurve(ICurvePrimitiveCR curve, MSInterpolationCurveCR icurve, DSegment1dCP interval) override
    {
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void _ProcessSpiral(ICurvePrimitiveCR curve, DSpiral2dPlacementCR spiral, DSegment1dCP interval) override
    {
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void _ProcessChildCurveVector(ICurvePrimitiveCR curve, CurveVector const &child, DSegment1dCP interval) override
    {
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void _ProcessPointString(ICurvePrimitiveCR curve, bvector<DPoint3d>const &points, DSegment1dCP interval) override
    {
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void _ProcessAkimaCurve(ICurvePrimitiveCR curve, bvector<DPoint3d>const &points, DSegment1dCP interval) override
    {
    }
#endif
};


/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct Moment1Data
{
double m_mass;
DVec3d m_centroidVector;
DVec3d m_normal;
Moment1Data (double mass, DVec3d centroidVector, DVec3d normal)
    {
    m_mass = mass;
    m_centroidVector = centroidVector;
    m_normal = normal;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double NormalDot(DVec3dCR normal) {return m_normal.DotProduct (normal);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void Reverse(){m_normal.Negate ();}
};


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool CentroidNormalArea_go
(
CurveVectorCR curves,
DPoint3dR   centroid,
DVec3dR     normal,
double      &area,
TransformCR worldToLocal
)
    {
    centroid.Zero ();
    normal.Zero ();
    area = 0.0;
    DPoint3d referencePoint;
    
    if (!curves.GetStartPoint (referencePoint))
        return false;

    // Really hate to trust this ....
    bool parity = curves.GetBoundaryType () == CurveVector::BOUNDARY_TYPE_ParityRegion;
    switch (curves.GetBoundaryType ())
        {
        case CurveVector::BOUNDARY_TYPE_UnionRegion:
        case CurveVector::BOUNDARY_TYPE_ParityRegion:
            {
            // ASSUME: Individual members have inconsistent order.
            // For UNION: Flip all so normals align with largest.
            // For PARITY: Flip all so smaller ones have normals opposite largest.
            bvector <Moment1Data> momentData;
            double maxArea = 0.0;
            size_t maxAreaIndex = 0;
            for (size_t index = 0; index < curves.size (); index++)
                {
                ICurvePrimitivePtr child = curves[index];
                DVec3d childCentroid;
                DVec3d   childNormal;
                double   childArea;
                CurveVectorCP childVector = child->GetChildCurveVectorCP ();
                if (childVector == NULL)
                    return false;
                if (!childVector->CentroidNormalArea (childCentroid, childNormal, childArea))
                    return false;
                momentData.push_back (Moment1Data (childArea, childCentroid, childNormal));
                double absArea = fabs (childArea);
                if (absArea > maxArea)
                    {
                    maxAreaIndex = index;
                    maxArea = absArea;
                    }
                }
            DPoint3d multiRegionReferencePoint;
            multiRegionReferencePoint.Zero ();
            CentroidSums sums (multiRegionReferencePoint, worldToLocal);
            DVec3d positiveNormal = momentData[maxAreaIndex].m_normal;
            //double holeArea = 0.0;
            for (size_t index = 0; index < momentData.size (); index++)
                {
                double s = momentData[index].m_normal.DotProduct (positiveNormal);
                bool reverse = false;
                if (index == maxAreaIndex)
                    {
                    }
                else
                    {
                    if (parity)
                        reverse = s > 0.0;
                    else
                        reverse = s < 0.0;
                    }
                if (reverse)
                    momentData[index].Reverse ();

                sums.AddAreaMoment (
                            momentData[index].m_centroidVector,
                            momentData[index].m_normal,
                            momentData[index].m_mass,
                            momentData[index].m_mass
                            );
                }
            return sums.GetCentroidNormalArea (centroid, normal, area);
            }


        case CurveVector::BOUNDARY_TYPE_Inner:
        case CurveVector::BOUNDARY_TYPE_Outer:
            {
            CentroidSums sums (referencePoint, worldToLocal);
            sums._ProcessCurveVector (curves, NULL);
            return sums.GetCentroidNormalArea (centroid, normal, area);
            }

        case CurveVector::BOUNDARY_TYPE_None:
        case CurveVector::BOUNDARY_TYPE_Open:
        default:
            return false;
        }
    }

GEOMDLLIMPEXP bool  CurveVector::CentroidNormalArea
(
DPoint3dR   centroid,
DVec3dR     normal,
double      &area
) const
    {
    BentleyApi::Transform worldToLocal;
    worldToLocal.InitIdentity ();
    return CentroidNormalArea_go (*this, centroid, normal, area, worldToLocal);
    }

GEOMDLLIMPEXP bool  CurveVector::CentroidAreaXY
(
DPoint3dR   centroid,
double      &area
) const
    {
    DVec3d normal;
    BentleyApi::Transform worldToLocal = Transform::FromRowValues
        (
        1,0,0,0,
        0,1,0,0,
        0,0,0,0
        );
    bool stat = CentroidNormalArea_go (*this, centroid, normal, area, worldToLocal);
    if (normal.z < 0.0)
        area = - area;
    return stat;
    }



END_BENTLEY_GEOMETRY_NAMESPACE
