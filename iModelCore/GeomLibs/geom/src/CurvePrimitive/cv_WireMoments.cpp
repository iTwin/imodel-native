/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/CurvePrimitive/cv_WireMoments.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

void DifferentialStripLineProducts (DMatrix4dR products,  DPoint3dCR pointA, DPoint3dCR pointB, double scale);
static double s_bsplineAngleTolerance = 0.1;
static double s_ellipseAngleTolerance = 0.1;
static int s_numQuadraturePoint = 7;
static int s_quadratureType = 0;

/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct WireProductSums : public ICurvePrimitiveProcessor
{
DMatrix4d m_products;
bvector<DPoint3d> m_strokePoints;   // availalable for use by _process methods.
bvector<double> m_strokeParameters;   // availalable for use by _process methods.

IFacetOptionsPtr m_bcurveStrokeOptions;    
IFacetOptionsPtr m_ellipseStrokeOptions;

WireProductSums ()
    {
    m_products = DMatrix4d::FromZero ();
    
    m_bcurveStrokeOptions = IFacetOptions::CreateForCurves ();
    m_bcurveStrokeOptions->SetParamsRequired (true);
    m_bcurveStrokeOptions->SetAngleTolerance (s_bsplineAngleTolerance);
    m_bcurveStrokeOptions->SetCurveParameterMapping (CURVE_PARAMETER_MAPPING_BezierFraction);

    m_ellipseStrokeOptions = IFacetOptions::CreateForCurves ();
    m_ellipseStrokeOptions->SetParamsRequired (true);
    m_ellipseStrokeOptions->SetAngleTolerance (s_ellipseAngleTolerance);
    m_ellipseStrokeOptions->SetCurveParameterMapping (CURVE_PARAMETER_MAPPING_BezierFraction);

    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool TryGetProducts(DMatrix4dR products)
    {
    products = m_products;
    return true;
    }

void AddLine (DPoint3dCR point0, DPoint3dCR point1)
    {
    DMatrix4d lineProducts;
    DifferentialStripLineProducts (lineProducts, point0, point1, 1.0);
    m_products.Add (lineProducts);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void _ProcessLine(ICurvePrimitiveCR curve, DSegment3dCR source, DSegment1dCP interval) override
    {
    if (interval == NULL)
        AddLine (source.point[0], source.point[1]);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void _ProcessArc(ICurvePrimitiveCR curve, DEllipse3dCR ellipse, DSegment1dCP interval) override
    {
#ifdef circleCase
    double rX, rY;
    RotMatrix axes;
    double theta0, sweep;
    DPoint3d center;
    ellipse.GetScaledRotMatrix (center, axes, rX, rY, theta0, sweep);
#endif
    BSIQuadraturePoints gaussRule;
    gaussRule.InitGauss (s_numQuadraturePoint);
    DPoint3d X;
    DVec3d dXdTheta, ddX;
    double theta, w;
    size_t num = m_ellipseStrokeOptions->EllipseStrokeCount (ellipse);
    double dTheta = ellipse.sweep / (double)num;
    for (size_t i = 0; i < num; i++)
        {
        double theta0 = ellipse.start + i * dTheta;
        double theta1 = ellipse.start + (i + 1) * dTheta;
        for (int i = 0; gaussRule.GetEval (i, theta0, theta1, theta, w); i++)
            {
            ellipse.Evaluate (X, dXdTheta, ddX, theta);
            // we only allow positive weights -- negative is artifact of parameterization.
            m_products.AddSymmetricScaledOuterProduct (X, dXdTheta.Magnitude () * fabs (w));
            }
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
            for (size_t i = 1; i < n; i++, point0 = point1)
                {
                point1 = points[i];
                AddLine (point0, point1);
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
        segment.AddStrokes (m_strokePoints, NULL, &m_strokeParameters, *m_bcurveStrokeOptions, 0.0, 1.0, false, &bcurve);
        DPoint3d xyzCurve;
        double u, w;
        DVec3d curveTangent;
        for (size_t j = 1, numJ = m_strokePoints.size (); j < numJ; j++)
            {
            double parameter0 = m_strokeParameters[j-1];
            double parameter1 = m_strokeParameters[j];
            if (numQuadraturePoints < 1)
                continue;
            for (int k = 0; k < numQuadraturePoints; k++)
                {
                gaussRule.GetEval (k, parameter0, parameter1, u, w);
                segment.FractionToPoint (xyzCurve, curveTangent, u, false);
                products.AddSymmetricScaledOuterProduct (xyzCurve, curveTangent.Magnitude () * w);
                }
            }
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

void go (CurveVectorCR curve)
    {
    }
};


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurveVector::ComputeSecondMomentWireProducts (DMatrix4dR products) const
    {
    WireProductSums sums;
    sums._ProcessCurveVector (*this, NULL);
    return sums.TryGetProducts (products);
    }














static int s_rLineGaussCount = 7;
static int s_rArcGaussCount  = 7;
static int s_rBcurveGaussCount = 7;
/*--------------------------------------------------------------------------------**//**
* integrals of sqrt(xx+yy) * [x y z 1]^[x y z 1]
* i.e. moment products weigthed by distance from z axis
* @bsistruct                                                    EarlinLutz      11/2013
+--------------------------------------------------------------------------------------*/
struct DifferentialRotationWireProductSums : public ICurvePrimitiveProcessor
{
DMatrix4d m_products;
bvector<DPoint3d> m_strokePoints;   // availalable for use by _process methods.
bvector<double> m_strokeParameters;   // availalable for use by _process methods.

IFacetOptionsPtr m_bcurveStrokeOptions;    
IFacetOptionsPtr m_ellipseStrokeOptions;

Transform m_worldToLocal;
BSIQuadraturePoints m_lineGaussRule;
BSIQuadraturePoints m_arcGaussRule;
BSIQuadraturePoints m_bcurveGaussRule;

DifferentialRotationWireProductSums (TransformCR worldToLocal)
    {
    m_worldToLocal = worldToLocal;
    m_products = DMatrix4d::FromZero ();
    
    m_bcurveStrokeOptions = IFacetOptions::CreateForCurves ();
    m_bcurveStrokeOptions->SetParamsRequired (true);
    m_bcurveStrokeOptions->SetAngleTolerance (s_bsplineAngleTolerance);
    m_bcurveStrokeOptions->SetCurveParameterMapping (CURVE_PARAMETER_MAPPING_BezierFraction);

    m_ellipseStrokeOptions = IFacetOptions::CreateForCurves ();
    m_ellipseStrokeOptions->SetParamsRequired (true);
    m_ellipseStrokeOptions->SetAngleTolerance (s_ellipseAngleTolerance);
    m_ellipseStrokeOptions->SetCurveParameterMapping (CURVE_PARAMETER_MAPPING_BezierFraction);

    m_lineGaussRule.InitGauss (s_rLineGaussCount);
    m_arcGaussRule.InitGauss (s_rArcGaussCount);
    m_bcurveGaussRule.InitGauss (s_rBcurveGaussCount);

    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool TryGetProducts(DMatrix4dR products)
    {
    products = m_products;
    return true;
    }

void AddLine (DPoint3dCR point0World, DPoint3dCR point1World)
    {
    double f, w;
    DPoint3d point0, point1;
    m_worldToLocal.Multiply (point0, point0World);
    m_worldToLocal.Multiply (point1, point1World);
    DVec3d dXdf = DVec3d::FromStartEnd (point0, point1);
    DPoint3d X;
    double a = dXdf.Magnitude ();
    for (int i = 0; m_lineGaussRule.GetEval (i, 0.0, 1.0, f, w); i++)
        {
        X.SumOf (point0, dXdf, f);
        m_products.AddSymmetricScaledOuterProduct (X, a * w * X.MagnitudeXY ());
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void _ProcessLine(ICurvePrimitiveCR curve, DSegment3dCR source, DSegment1dCP interval) override
    {
    if (interval == NULL)
        AddLine (source.point[0], source.point[1]);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void _ProcessArc(ICurvePrimitiveCR curve, DEllipse3dCR ellipseWorld, DSegment1dCP interval) override
    {
    DEllipse3d ellipse;
    m_worldToLocal.Multiply (ellipse, ellipseWorld);
    BSIQuadraturePoints gaussRule;
    gaussRule.InitGauss (s_numQuadraturePoint);
    DPoint3d X;
    DVec3d dXdTheta, ddX;
    double theta, w;
    size_t num = m_ellipseStrokeOptions->EllipseStrokeCount (ellipse);
    double dTheta = ellipse.sweep / (double)num;
    for (size_t i = 0; i < num; i++)
        {
        double theta0 = ellipse.start + i * dTheta;
        double theta1 = ellipse.start + (i + 1) * dTheta;
        for (int i = 0; gaussRule.GetEval (i, theta0, theta1, theta, w); i++)
            {
            ellipse.Evaluate (X, dXdTheta, ddX, theta);
            // we only allow positive weights -- negative is artifact of parameterization.
            m_products.AddSymmetricScaledOuterProduct (X, dXdTheta.Magnitude () * fabs (w) * X.MagnitudeXY ());
            }
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
            for (size_t i = 1; i < n; i++, point0 = point1)
                {
                point1 = points[i];
                AddLine (point0, point1);
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
        segment.Multiply (m_worldToLocal);
        segment.AddStrokes (m_strokePoints, NULL, &m_strokeParameters, *m_bcurveStrokeOptions, 0.0, 1.0, false, &bcurve);
        DPoint3d xyzCurve;
        double u, w;
        DVec3d curveTangent;
        for (size_t j = 1, numJ = m_strokePoints.size (); j < numJ; j++)
            {
            double parameter0 = m_strokeParameters[j-1];
            double parameter1 = m_strokeParameters[j];
            if (numQuadraturePoints < 1)
                continue;
            for (int k = 0; k < numQuadraturePoints; k++)
                {
                gaussRule.GetEval (k, parameter0, parameter1, u, w);
                segment.FractionToPoint (xyzCurve, curveTangent, u, false);
                products.AddSymmetricScaledOuterProduct (xyzCurve, curveTangent.Magnitude () * w * xyzCurve.MagnitudeXY ());
                }
            }
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
};





/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurveVector::ComputeSecondMomentDifferentialWireRotationProducts
(
DRay3dCR rotationAxis,
BentleyApi::TransformR localToWorld,
DMatrix4dR products
) const
    {
    Transform worldToLocal;
    DVec3d uVec, vVec, wVec;
    products = DMatrix4d::FromZero ();
    localToWorld.InitIdentity ();
    if (!rotationAxis.direction.GetNormalizedTriad (uVec, vVec, wVec))
        return false;
    localToWorld.InitFromOriginAndVectors (rotationAxis.origin, uVec, vVec, wVec);
    worldToLocal.InverseOf (localToWorld);
    DifferentialRotationWireProductSums sums (worldToLocal);
    sums._ProcessCurveVector (*this, NULL);
    return sums.TryGetProducts (products);
    }

END_BENTLEY_GEOMETRY_NAMESPACE
