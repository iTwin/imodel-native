/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


struct AbstractPrimitiveAreaIntegrator
{
GEOMAPI_VIRTUAL void _DifferentialStripLineProducts (DMatrix4dR products,  DPoint3dCR pointA, DPoint3dCR pointB, double scale) = 0;
GEOMAPI_VIRTUAL void _TriangleProducts (DPoint3dCR point0, DPoint3dCR pointA, DPoint3dCR pointB, DMatrix4dR products) = 0;
GEOMAPI_VIRTUAL bool _EllipseCapProducts (DEllipse3dCR ellipse, DMatrix4dR products) = 0;
};


static double s_angleTolerance = 0.1;
static int s_numQuadraturePoint = 7;
static int s_quadratureType = 0;
void CurveVector::SetMomentIntegrationParameters (int quadratureType, int numQuadraturePoints, double angleTol)
    {
    s_angleTolerance = angleTol;
    s_numQuadraturePoint = numQuadraturePoints;
    s_quadratureType = quadratureType;
    }
/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct InertiaProductsSums : public ICurvePrimitiveProcessor
{
DMatrix4d m_products;
Transform m_worldToLocal;
Transform m_localToWorld;
AbstractPrimitiveAreaIntegrator &m_integrator;

bvector<DPoint3d> m_strokePoints;   // availalable for use by _process methods.
bvector<double> m_strokeParameters;   // availalable for use by _process methods.

IFacetOptionsPtr m_bcurveStrokeOptions;    
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

InertiaProductsSums (AbstractPrimitiveAreaIntegrator &integrator, TransformCR localToWorld, TransformCR worldToLocal)
    : m_integrator (integrator)
    {
    m_products = DMatrix4d::FromZero ();
    m_worldToLocal = worldToLocal;
    m_localToWorld = localToWorld;
    
    m_bcurveStrokeOptions = IFacetOptions::CreateForCurves ();
    m_bcurveStrokeOptions->SetParamsRequired (true);
    m_bcurveStrokeOptions->SetAngleTolerance (s_angleTolerance);
    m_bcurveStrokeOptions->SetCurveParameterMapping (CURVE_PARAMETER_MAPPING_BezierFraction);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool TryGetProducts(DMatrix4dR products, bool returnWorldProducts)
    {
    RotMatrix Q = RotMatrix::From (m_localToWorld);
    RotMatrix Q0;
    double scale;
    if (Q.IsRigidScale (Q0, scale))    // area integrals do not map commute with nonuniform scaling!!!
        {
        if (returnWorldProducts)
            products = DMatrix4d::FromSandwichProduct (m_localToWorld, m_products, scale * scale);
        else
            {
            products = m_products;
            }
        return true;
        }
    products = DMatrix4d::FromZero ();
    return false;
    }

// Sweep a triangle from the origin to pointA, pointB.
// 
void AddLocalTriangle(DPoint3dCR pointA, DPoint3dCR pointB)
    {
    DMatrix4d products;
    DPoint3d zero;
    zero.Zero ();
    m_integrator._TriangleProducts (zero, pointA, pointB, products);
    m_products.Add (products);
    }

void AddLocalEllipseSweep_byQuadratureOfLineStrips (DEllipse3dCR ellipse)
    {
    // sweep from the origin to the ellipse.
    // Stroke the curve.
    // Accumulate moments in triangles from the origin to the strokes.
    // This leaves slivers between each stroke and the nearby curve.
    // Integrate each sliver by 1D quadrature.
    // The quadrature variable is the ellipse angle.
    BCurveSegment segment;
    BSIQuadraturePoints gaussRule;

    if (s_quadratureType == 1)
        gaussRule.InitUniform (s_numQuadraturePoint);
    //else if (s_quadratureType == 2)
    //    gaussRule.InitClenshawCurtis (s_numQuadraturePoint);
    else // default to gauss rules
        gaussRule.InitGauss (s_numQuadraturePoint);
        
    int numQuadraturePoints = gaussRule.GetNumEval ();
    DMatrix4d products = DMatrix4d::FromZero ();

    size_t numStroke = m_bcurveStrokeOptions->EllipseStrokeCount (ellipse);
    if (numStroke < 1)
        numStroke = 1;
    DPoint3d point0, point1, xyzCurve, xyzChord;
    ellipse.FractionParameterToPoint (point0, 0.0);
    ellipse.FractionParameterToPoint (point1, 1.0);
    double dTheta = ellipse.sweep / (double)numStroke;
    double theta0, theta1, thetaStart;
    thetaStart = theta0 = ellipse.start;
    DVec3d chordVector, perpVector;
    double theta, w, wdx;
    DVec3d curveD1, curveD2, point0ToCurve;
    DMatrix4d lineProducts;
    for (size_t j = 1; j <= numStroke; j++, point0 = point1, theta0 = theta1)
        {
        theta1 = thetaStart + j * dTheta;
        ellipse.Evaluate (point1, theta1);
        chordVector.NormalizedDifference (point1, point0);
        // expect chordVector has z=0?
        perpVector.Init (chordVector.y, -chordVector.x, 0.0);
        AddLocalTriangle (point0, point1);
        if (numQuadraturePoints < 1)
            continue;
        for (int k = 0; k < numQuadraturePoints; k++)
            {
            gaussRule.GetEval (k, theta0, theta1, theta, w);
            ellipse.Evaluate (xyzCurve, curveD1, curveD2, theta);
            wdx = w * curveD1.DotProduct (chordVector);
            point0ToCurve.DifferenceOf (xyzCurve, point0);
            if (point0ToCurve.DotProduct (perpVector) < 0.0)
                wdx = -wdx;
            xyzChord.SumOf (point0, chordVector, point0ToCurve.DotProduct (chordVector));
            m_integrator._DifferentialStripLineProducts (lineProducts, xyzChord, xyzCurve, wdx);
            products.Add (lineProducts);
            }
        }
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
        AddLocalTriangle (segment.point[0], segment.point[1]);
    else
        {
        AddLocalTriangle (DPoint3d::FromInterpolate (
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

    if (NULL != interval)
        myEllipse = DEllipse3d::FromFractionInterval (ellipse, interval->GetStart (), interval->GetEnd ());

    ApplyWorldToLocal (myEllipse.center);
    ApplyWorldToLocal (myEllipse.vector0);
    ApplyWorldToLocal (myEllipse.vector90);
    DPoint3d pointA, pointB;
    myEllipse.FractionParameterToPoint (pointA, 0.0);
    myEllipse.FractionParameterToPoint (pointB, 1.0);
    DMatrix4d products;
    if (m_integrator._EllipseCapProducts (myEllipse, products))
        {
        AddLocalTriangle (pointA, pointB);
        m_products.Add (products);
        }
    else
        {
        AddLocalEllipseSweep_byQuadratureOfLineStrips (myEllipse);
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void _ProcessLineString(ICurvePrimitiveCR curve, bvector<DPoint3d> const &points, DSegment1dCP interval) override
    {
   if (NULL == interval)
        {
        size_t n = points.size ();
        if (n > 1)
            {
            DPoint3d point0, point1;
            point0 = points[0];
            ApplyWorldToLocal (point0);
            for (size_t i = 1; i < n; i++, point0 = point1)
                {
                point1 = points.at(i);
                ApplyWorldToLocal (point1);
                AddLocalTriangle (point0, point1);
                }
            }
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
    // Stroke the curve.
    // Accumulate moments in triangles from the origin to the strokes.
    // This leaves slivers between each stroke and the nearby curve.
    // Integrate each sliver by 1D quadrature.
    // The quadrature variable is the curve parameter.
    //    At each curve parameter required by the quadrature, the 1D integrand is the integral along the line from the
    //     curve point back to the stroke.
    //    The width of the strip with the line at its center is the 1D integration variable times the part of the 
    //     curve tangent along the stroke direction -- i.e. curveTangent.DotProduct(lineTangent)
    // Accuracy experience:
    // Really fine stroking -- 0.02 radians -- gives about 4 digits.
    // Stroking at 0.1 radians (about 5 degrees), 7 gauss points gives machine precision.
    // Math is right.
    BCurveSegment segment;
    BSIQuadraturePoints gaussRule;

    if (s_quadratureType == 1)
        gaussRule.InitUniform (s_numQuadraturePoint);
    //else if (s_quadratureType == 2)
    //    gaussRule.InitClenshawCurtis (s_numQuadraturePoint);
    else // default to gauss rules
        gaussRule.InitGauss (s_numQuadraturePoint);
        
    int numQuadraturePoints = gaussRule.GetNumEval ();
    DMatrix4d products = DMatrix4d::FromZero ();
    for (size_t i = 0; bcurve.AdvanceToBezier  (segment, i, true);)
        {
        m_strokePoints.clear ();
        m_strokeParameters.clear ();       
        ApplyWorldToLocal (segment);
        segment.AddStrokes (m_strokePoints, NULL, &m_strokeParameters, *m_bcurveStrokeOptions, 0.0, 1.0, false, &bcurve);
        DPoint3d point0, point1, xyzCurve, xyzChord;
        point0 = m_strokePoints[0];
        double parameter0 = m_strokeParameters[0];
        double parameter1;
        DVec3d chordVector, perpVector;
        double u, w, wdx;
        DVec3d curveTangent, point0ToCurve;
        DMatrix4d lineProducts;
        for (size_t j = 1, numJ = m_strokePoints.size (); j < numJ; j++, point0 = point1, parameter0 = parameter1)
            {
            point1 = m_strokePoints[j];
            parameter1 = m_strokeParameters[j];
            // Are parameters in bezier space or segment space?
            chordVector.NormalizedDifference (point1, point0);
            // expect chordVector has z=0?
            perpVector.Init (chordVector.y, -chordVector.x, 0.0);
            AddLocalTriangle (point0, point1);
            if (numQuadraturePoints < 1)
                continue;
            for (int k = 0; k < numQuadraturePoints; k++)
                {
                gaussRule.GetEval (k, parameter0, parameter1, u, w);
                segment.FractionToPoint (xyzCurve, curveTangent, u, false);
                wdx = w * curveTangent.DotProduct (chordVector);
                point0ToCurve.DifferenceOf (xyzCurve, point0);
                if (point0ToCurve.DotProduct (perpVector) < 0.0)
                    wdx = -wdx;
                xyzChord.SumOf (point0, chordVector, point0ToCurve.DotProduct (chordVector));
                m_integrator._DifferentialStripLineProducts (lineProducts, xyzChord, xyzCurve, wdx);
                products.Add (lineProducts);
                }
            }
//        _ProcessLineString (curve, m_strokePoints, NULL);
        }
    m_products.Add (products);
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
The sign of the returned area (products[3][3]) is the sign of the largest child.

* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool PrincipalMoments_goAnySign
(
AbstractPrimitiveAreaIntegrator &integrator,
CurveVectorCR curves,
TransformCR localToWorld,
TransformCR worldToLocal,
DMatrix4dR products,
bool returnWorldProducts
)
    {
    DPoint3d referencePoint;
    products = DMatrix4d::FromZero ();
    if (!curves.GetStartPoint (referencePoint))
        return false;

    // Really hate to trust this ....
    //bool parity = curves.GetBoundaryType () == CurveVector::BOUNDARY_TYPE_ParityRegion;
    CurveVector::BoundaryType boundaryType = curves.GetBoundaryType ();
    switch (boundaryType)
        {
        case CurveVector::BOUNDARY_TYPE_UnionRegion:
        case CurveVector::BOUNDARY_TYPE_ParityRegion:
            {
            bvector<DMatrix4d> childProducts;
            // ASSUME: Individual members have inconsistent order.
            // For UNION: Flip all so normals align with largest.
            // For PARITY: Flip all so smaller ones have normals opposite largest.
            size_t maxAreaIndex = 0;
            double maxAbsArea = -DBL_MAX;
            double referenceSign = 0.0;
            for (size_t index = 0; index < curves.size (); index++)
                {
                ICurvePrimitivePtr child = curves[index];
                CurveVectorCP childVector = child->GetChildCurveVectorCP ();
                if (childVector == NULL)
                    return false;
                DMatrix4d thisChildProducts;
                if (!PrincipalMoments_goAnySign (integrator, *childVector, localToWorld, worldToLocal, thisChildProducts, returnWorldProducts))
                    return false;
                childProducts.push_back (thisChildProducts);
                double area = thisChildProducts.coff[3][3];
                if (fabs(area) > maxAbsArea)
                    {
                    maxAreaIndex = childProducts.size () - 1;
                    maxAbsArea = fabs (area);
                    referenceSign = area >= 0.0 ? 1.0 : -1.0;
                    }
                }

            if (childProducts.size () == 0)
                return false;

            products = DMatrix4d::FromZero ();
            
            for (size_t i = 0; i < childProducts.size (); i++)
                {
                double targetSign = referenceSign;
                double thisArea = childProducts[i].coff[3][3];
                if (i != maxAreaIndex)
                    {
                    if (boundaryType == CurveVector::BOUNDARY_TYPE_ParityRegion)
                        targetSign *= -1.0;                    
                    }
                double factor = 1.0;
                if (thisArea * targetSign < 0.0)
                    factor = -1.0;
                products.Add (childProducts[i], factor);
                }
            }
            return true;
            
        case CurveVector::BOUNDARY_TYPE_Inner:
        case CurveVector::BOUNDARY_TYPE_Outer:
            {
            InertiaProductsSums sums (integrator, localToWorld, worldToLocal);
            sums._ProcessCurveVector (curves, NULL);
            return sums.TryGetProducts (products, returnWorldProducts);
            }

        case CurveVector::BOUNDARY_TYPE_None:
        case CurveVector::BOUNDARY_TYPE_Open:
        default:
            return false;
        }
    }

static bool PrincipalMoments_goPositiveArea
(
AbstractPrimitiveAreaIntegrator &integrator,
CurveVectorCR curves,
TransformCR localToWorld,
TransformCR worldToLocal,
DMatrix4dR products,
bool returnWorldProducts
)
    {
    if (PrincipalMoments_goAnySign (integrator, curves, localToWorld, worldToLocal, products, returnWorldProducts))
        {
        if (products.coff[3][3] < 0.0)
            products.Scale (-1.0);
        return true;
        }
    return false;
    }

// Accumulate scale * integral (0<=s<=1) ds (((1-s)A + s B)((1-s)A + s B)^)  A.Distance(B)
//                  =>> (AA^ + BB^)/3 + (AB^+BA^)/6
// for use as a strip integral the scale includes (from caller) stripWidth*stripLength
//    stripWidth = T dot U      (T=curve tangent, U=nominal x axis unit vector)
void DifferentialStripLineProducts (DMatrix4dR products,  DPoint3dCR pointA, DPoint3dCR pointB, double scale)
    {
    double distance = pointA.Distance (pointB);
    double A[4] = {pointA.x, pointA.y, pointA.z, 1.0};
    double B[4] = {pointB.x, pointB.y, pointB.z, 1.0};
    double scale1 = distance * scale / 3.0;
    double scale2 = distance * scale / 6.0;
    for (int i = 0; i < 4; i++)
        {
        for (int j = 0; j < 4; j++)
            {
            products.coff[i][j] =
                  scale1 * (A[i] * A[j] + B[i] * B[j])
                + scale2 * (A[i] * B[j] + B[i] * A[j]);
            }
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool unitTriangleAreaProducts (DMatrix4dR integrals)
    {
    integrals = DMatrix4d::FromZero ();
    
        integrals.coff[0][0] = 1.0 / 12.0;
        integrals.coff[1][0] = integrals.coff[0][1] = 1.0 / 24.0;
        integrals.coff[1][1] = 1.0 / 12.0;
        integrals.coff[0][3] = integrals.coff[3][0] = 1.0 / 6.0;
        integrals.coff[1][3] = integrals.coff[3][1] = 1.0 /6.0;
        integrals.coff[3][3] = 1.0 / 2.0;
    
    return true;
    }

void TriangleAreaProducts (DPoint3dCR point0, DPoint3dCR pointA, DPoint3dCR pointB, DMatrix4dR products)
    {
    DMatrix4d local = DMatrix4d::FromZero ();
    unitTriangleAreaProducts ( local);
    
    double ux = pointA.x - point0.x;
    double uy = pointA.y - point0.y;
    double uz = pointA.z - point0.z;
    double vx = pointB.x - point0.x;
    double vy = pointB.y - point0.y;
    double vz = pointB.z - point0.z;
    
    DVec3d vecA = DVec3d::From (pointA);
    DVec3d vecB = DVec3d::From (pointB);
    
    DVec3d J = DVec3d::FromCrossProduct (vecA, vecB);
    double detJ = J.z;  //  effectively view from above?
    
    Transform localtoworld = Transform::FromRowValues
    (ux, vx, 0, point0.x,
     uy, vy, 0, point0.y,
     uz, vz, 0, point0.z);
    
    products = DMatrix4d::FromSandwichProduct (localtoworld, local, detJ);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool unitCircleSectorAreaProducts (double theta0, double theta1, DMatrix4dR integrals)
    {
    integrals = DMatrix4d::FromZero ();
    double div3 = 1.0 / 3.0;


    double deltaTheta = theta1 - theta0;
    double s0 = sin (theta0);
    double s1 = sin (theta1);
    double c0 = cos (theta0);
    double c1 = cos (theta1);
    double halfdiff_deltaSin2Theta = 0.5 * (sin (2.0 * theta1) - sin (2.0 * theta0));
    
    integrals.coff[0][0] = 0.125 * (deltaTheta + halfdiff_deltaSin2Theta);
    integrals.coff[0][1] = integrals.coff[1][0] = 0.125 * (s1 * s1 - s0 * s0);
    integrals.coff[1][1] = 0.125 * (deltaTheta - halfdiff_deltaSin2Theta);
    integrals.coff[0][3] = integrals.coff[3][0] =  div3 * (s1 - s0);
    integrals.coff[1][3] = integrals.coff[3][1] = -div3 * (c1 - c0);
    integrals.coff[3][3] = 0.5 * deltaTheta;
        
    
    // (Do the integrals with given limits, but multiply all by sweepSign to make it act like
    //   theta0+sweep back to theta0 if sweep is negative)
    return true;
    }
static bool unitChordArcAreaProducts ( double theta0, double sweep,DMatrix4dR integrals)
    {
    double theta1;
    if (sweep > 0.0)
        {
        theta1 = theta0 + sweep;
        }
    else
        {
        theta1 = theta0;
        theta0 = theta1 + sweep;
        }
    DMatrix4d sector, unitTriangle, triangle;
    unitCircleSectorAreaProducts (theta0, theta1, sector);
    unitTriangleAreaProducts (unitTriangle);
    double c0 = cos (theta0), c1 = cos (theta1);
    double s0 = sin (theta0), s1 = sin (theta1);
    Transform placement = Transform::FromRowValues
        (
        c0, c1, 0, 0,
        s0, s1, 0, 0,
        0,  0,  0, 0
        );
     double detJ = c0 * s1 - c1 * s0;
     triangle = DMatrix4d::FromSandwichProduct (placement, unitTriangle, detJ);
     integrals.DifferenceOf (sector, triangle);
     if (sweep < 0.0)
        integrals.Scale (-1.0);
    return true;
    }

void EllipseCapProducts (DEllipse3dCR ellipse, DMatrix4dR products)
    {
    DMatrix4d local = DMatrix4d::FromZero ();
    unitChordArcAreaProducts (ellipse.start, ellipse.sweep, local);
    
    double xc = ellipse.center.x;
    double yc = ellipse.center.y;
    double zc = ellipse.center.z;
    double ux = ellipse.vector0.x;
    double uy = ellipse.vector0.y;
    double uz = ellipse.vector0.z;
    double vx = ellipse.vector90.x;
    double vy = ellipse.vector90.y;
    double vz = ellipse.vector90.z;
    
    DVec3d J = DVec3d::FromCrossProduct (ellipse.vector0, ellipse.vector90);
    double detJ = J.z;
    
    Transform localtoworld = Transform::FromRowValues 
                        (
                        ux, vx, 0, xc,
                        uy, vy, 0, yc,
                        uz, vz, 0, zc
                        );

    products = DMatrix4d::FromSandwichProduct (localtoworld, local, detJ);    
    }

struct FlatPrimitiveAreaIntegrator : AbstractPrimitiveAreaIntegrator
{
void _DifferentialStripLineProducts (DMatrix4dR products,  DPoint3dCR pointA, DPoint3dCR pointB, double scale) override 
    {
    DifferentialStripLineProducts (products, pointA, pointB, scale);
    }
void _TriangleProducts (DPoint3dCR point0, DPoint3dCR pointA, DPoint3dCR pointB, DMatrix4dR products) override 
    {
    TriangleAreaProducts (point0, pointA, pointB, products);
    }

bool _EllipseCapProducts (DEllipse3dCR ellipse, DMatrix4dR products) override 
    {
    EllipseCapProducts (ellipse, products);
    return true;
    }
};

// Expected orientation is in the xZ plane. (NOT XY !!!)
// Differential volume is proportional to X.
struct ZWedgePrimitiveAreaIntegrator : AbstractPrimitiveAreaIntegrator
{
BSITriangleQuadraturePoints m_triangleGauss;
BSIQuadraturePoints m_lineGauss;
ZWedgePrimitiveAreaIntegrator ()
    {
    m_triangleGauss.InitStrang (8);
    m_lineGauss.InitGauss (4);  // integrating x^3 etc.  Maybe 2 would be enough?
    }
void _DifferentialStripLineProducts (DMatrix4dR products,  DPoint3dCR pointA, DPoint3dCR pointB, double scale) override 
    {
    DVec3d U = DVec3d::FromStartEnd (pointA, pointB);
    double a = U.Magnitude ();     // ugh.  maybe should be xz for consistency with triangle
    double u, w;
    DPoint3d xyz;
    products = DMatrix4d::FromZero ();
    // accumulate only in upper triangle ..
    for (int i = 0; m_lineGauss.GetEval (i,  0.0, 1.0, u, w); i++)
        {
        xyz.SumOf (pointA, U, u);
        products.AddSymmetricScaledOuterProduct (xyz, a * w * xyz.x);
        }
    }

void _TriangleProducts (DPoint3dCR point0, DPoint3dCR pointA, DPoint3dCR pointB, DMatrix4dR products) override 
    {
    DVec3d U = DVec3d::FromStartEnd (point0, pointA);
    DVec3d V = DVec3d::FromStartEnd (point0, pointB);
    DVec3d J = DVec3d::FromCrossProduct (U,V);
    DVec3d Z = DVec3d::From (0,0,1);
    double u, v, w;
    DPoint3d xyz;
    products = DMatrix4d::FromZero ();
    // accumulate only in upper triangle ..
    for (int i = 0; m_triangleGauss.GetEval (i, u, v, w); i++)
        {
        xyz.SumOf (point0, U, u, V, v);
        double detJ = J.TripleProduct (*(DVec3d*)&xyz, Z);
        products.AddSymmetricScaledOuterProduct (xyz, w * detJ);
        }
    }

bool _EllipseCapProducts (DEllipse3dCR ellipse, DMatrix4dR products) override 
    {
    return false;
    }    
};





/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurveVector::ComputeSecondMomentAreaProducts (DMatrix4dR products) const
    {
    BentleyApi::Transform worldToLocal, localToWorld;
    DRange3d localRange;
    FlatPrimitiveAreaIntegrator integrator;
    CurveVectorPtr planarCurves = CloneInLocalCoordinates (LOCAL_COORDINATE_SCALE_UnitAxesAtStart,
                localToWorld, worldToLocal, localRange);
    if (planarCurves.IsValid ()
        && localRange.IsAlmostZeroZ ()
        )
        {
        Transform identity = Transform::FromIdentity ();
        double maxGap = planarCurves->MaxGapWithinPath ();
        double diagonal = localRange.low.DistanceXY (localRange.high);
        static double s_relTol = 1.0e-8;
        static double s_shiftFactor = 100.0;
        double absTol = s_relTol * diagonal;
        static bool s_fixAll = true;
        if (s_fixAll || maxGap > absTol)
            {
            CurveGapOptions options;
            options.SetMaxDirectAdjustTolerance (absTol);
            options.SetRemovePriorGapPrimitives (false);    // ???
            options.SetMaxAdjustAlongCurve (absTol * s_shiftFactor);
            planarCurves = planarCurves->CloneWithGapsClosed(options);
            }
        planarCurves->FixupXYOuterInner (true);
        DMatrix4d localProducts;
        if (PrincipalMoments_goPositiveArea (integrator, *planarCurves, identity, identity, localProducts, true))
            {
            products = DMatrix4d::FromSandwichProduct (localToWorld, localProducts, 1.0);
            return true;
            }
        }
    products = DMatrix4d::FromZero ();
    return false;
    }

bool CurveVector::ComputeSecondMomentDifferentialAreaRotationProducts
(
DRay3dCR rotationAxis,
BentleyApi::TransformR rotationToWorld,
DMatrix4dR products
) const
    {
    BentleyApi::Transform worldToLocal, localToWorld;
    DRange3d range;
    ZWedgePrimitiveAreaIntegrator integrator;
    static double s_axisZeroRelTol = 1.0e-8;   // hmm.. how to tell if ray is in local xy plane.  Don't want to be really strict here ...
    if (IsPlanar (localToWorld, worldToLocal, range))
        {
        DRay3d localAxis;
        DVec3d planeNormal;
        localToWorld.GetMatrixColumn (planeNormal, 2);
        worldToLocal.Multiply (localAxis, rotationAxis);
        if (planeNormal.IsPerpendicularTo (rotationAxis.direction)
            && fabs (localAxis.origin.z) < s_axisZeroRelTol * range.LargestCoordinate ()
            )
            {
            DVec3d rotX, rotY, rotZ;   // axes of rotation system.
            rotZ.Normalize (rotationAxis.direction);
            rotY = planeNormal;
            rotX.CrossProduct (rotY, rotZ);
            rotationToWorld.InitFromOriginAndVectors (rotationAxis.origin, rotX, rotY, rotZ);
            BentleyApi::Transform worldToRotation;
            worldToRotation.InverseOf (rotationToWorld);        // Phew.  This moves the (planar !!!) area onto the xz plane of the rotation system.
            DRange3d rotatedRange;
            GetRange (rotatedRange, worldToRotation);
            double xbar = (rotatedRange.low.x + rotatedRange.high.x) * 0.5;
            if (xbar < 0.0)
                {
                // rebuild with x, y negated ...
                rotX.Negate ();
                rotY.Negate ();
                rotationToWorld.InitFromOriginAndVectors (rotationAxis.origin, rotX, rotY, rotZ);
                worldToRotation.InverseOf (rotationToWorld);
                }
            return PrincipalMoments_goPositiveArea (integrator, *this, rotationToWorld, worldToRotation, products, false);
            }
        }
    products = DMatrix4d::FromZero ();
    return false;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
