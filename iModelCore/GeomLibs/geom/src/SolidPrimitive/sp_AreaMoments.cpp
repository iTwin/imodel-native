/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/SolidPrimitive/sp_AreaMoments.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "BoxFaces.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


// moments are passed in a DMatrix4d structured as
// [ xx xy xz x]
// [ xy yy yz y]
// [ xz yz zz z]
// [ x  y  z  a]
// Where each term is an integral of (x,y,z, xx, xy, xz, yy, yz, zz) dA
// Note that for figures entirely in the xy plane all z entries are zero.
//
// Compute all moments and area of the part of unit sphere from latitude phi0 to latitude phi1.
static bool sphereAreaMoments (double phi0, double phi1, DMatrix4dR products)
    {
    double pi = Angle::Pi();
    double div2 = 1.0 / 2.0;
    double div3 = 1.0 / 3.0;
    double r = 1;
    double r2 = r * r;
    double r3 = r2 * r;
    double r4 = r3 * r;
    double tpr2 = 2 * pi * r2;
    double sn1 = sin(phi1);
    double sn12 = sn1*sn1;
    double sn13 = sn12 * sn1;
    double sn0 = sin(phi0);
    double sn02 = sn0*sn0;
    double sn03 = sn02 * sn0;
    double sncd = div3 * (sn13 - sn03);
    
    products = DMatrix4d::FromZero (); 
    
        products.coff[0][0] = products.coff[1][1] = pi*r4*((sn1 - sn0) - sncd);
        products.coff[2][2] = tpr2*r2*sncd;
        products.coff[2][3] = products.coff[3][2] = tpr2*r*div2*(sn12 - sn02);
        products.coff[3][3] = tpr2*(sn1 - sn0);
    
    return true;
    }
// Compute all moments and area of a cone around the z axis with radius rA at zA and rB at zB.
static bool coneAreaMoments (double rA, double rB, double zB, DMatrix4dR integrals)
    {
    double pi = Angle::Pi();
    double zb = zB;
    double zb2 = zb * zb;
    double zb3 = zb2 * zb;
    double zb4 = zb3 * zb;
    double ra = rA;
    double ra2 = ra * ra;
    double ra3 = ra2 * ra;
    double div2 = 1.0 / 2.0;
    double div3 = 1.0 / 3.0;
    double div4 = 1.0 / 4.0;
    double div32 = 3.0 / 2.0;
    double m = (rB - rA) / zb;
    double m2 = m * m;
    double m3 = m2 * m;
    double ct = sqrt(1 + m2)*pi;
    integrals = DMatrix4d::FromZero (); 
    
        integrals.coff[0][0] = integrals.coff[1][1] = ct*(div4*zb4*m3 + zb3*m2*ra + div32*zb2*m*ra2 + zb*ra3);
        integrals.coff[2][2] = ct*2*(div4*zb4*m + div3*zb3*ra);
        integrals.coff[2][3] = integrals.coff[3][2] = ct*2*(div3*zb3*m + div2*zb2*ra);
        integrals.coff[3][3] = ct*2*(div2*zb2*m + zb*ra);
    
    return true;
    }
    
// Compute area moments for the complete unit circle.
static bool unitCircleAreaMoments (DMatrix4dR integrals)
    {
    integrals = DMatrix4d::FromZero ();
    double pi = Angle::Pi ();
    double div4 = 1.0 / 4.0;
    integrals.coff[0][0] = integrals.coff[1][1] = pi*div4;
    integrals.coff[3][3] = pi;
    return true;
    }

static bool unitTorusAreaMoments (double minorRadius,  double majorRadius, double theta0, double sweep, DMatrix4dR integrals)
    {
    double sp = sweep;
    double pi = Angle::Pi();
    double div2 = 1.0 / 2.0;
    double div4 = div2 * div2;
    double a = minorRadius;
    double a2 = a * a;
    double a3 = a2 * a;
    double r = majorRadius;
    double r2 = r * r;
    double r3 = r2 * r;
    double t0 = theta0;
    double t1 = theta0 + sp;
    double hsp = div2 * sp;
    double rterm1 = 2*pi*r*a;
    double rterm2 = 2*pi*r2*a + pi*a3;
    double rterm3 = 2*pi*r3*a + 3*pi*r*a3;
    double sn0 = sin(t0);
    double sn1 = sin(t1);
    double cs0 = cos(t0);
    double cs1 = cos(t1);
    double sindf = div4*(sin(2*t1) - sin(2*t0));
    
    integrals = DMatrix4d::FromZero ();
    
        integrals.coff[0][0] = rterm3 * (hsp + sindf);
        integrals.coff[0][1] = integrals.coff[1][0] = rterm3 * div2 * (sn1*sn1 - sn0*sn0);
        integrals.coff[1][1] = rterm3 * (hsp - sindf);
        integrals.coff[0][3] = integrals.coff[3][0] = rterm2 * (sn1 - sn0);
        integrals.coff[1][3] = integrals.coff[3][1] = rterm2 * (-cs1 + cs0);
        integrals.coff[2][2] = pi*r*a3*sp;
        integrals.coff[3][3] = rterm1*sp;
       
    return true;
    }

void AddBilinearQuadAreaMoments (DBilinearPatch3dCR patch, BSIQuadraturePointsCR quadraturePoints, DMatrix4dR products)
    {
    DPoint3d xyz;
    DVec3d dXdu, dXdv, J;
    double u, v, wu, wv;
    for (int i = 0; quadraturePoints.GetEval (i, 0.0, 1.0,  u, wu); i++)
        {
        for (int j = 0; quadraturePoints.GetEval (j, 0.0, 1.0, v, wv); j++)
            {
            patch.Evaluate (u, v, xyz, dXdu, dXdv);
            J.CrossProduct (dXdu, dXdv);
            products.AddSymmetricScaledOuterProduct (xyz, wu * wv * J.Magnitude ());
            }
        }
    }
// get area moments for unit circle or sector of unit circle.  Map out to placement in 3D
static bool accumulateFullDiskAreaIntegrals (DEllipse3dCR ellipse, DMatrix4dR sums)
    {
    bool stat = false;
    DMatrix4d localIntegrals, globalIntegrals;
    stat = unitCircleAreaMoments (localIntegrals);
    if (stat)
        {
        DVec3d unitW;
        double detJ = unitW.NormalizedCrossProduct (ellipse.vector0, ellipse.vector90);
        Transform placement = Transform::FromOriginAndVectors (ellipse.center, ellipse.vector0, ellipse.vector90, unitW);
        globalIntegrals = DMatrix4d::FromSandwichProduct (placement, localIntegrals, detJ);
        sums.Add (globalIntegrals);
        }
    return stat;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnSphereDetail::ComputeSecondMomentAreaProducts (TransformR localToWorld, DMatrix4dR localProducts) const
                              
    {
    localToWorld = m_localToWorld;
    if (sphereAreaMoments (m_startLatitude, m_startLatitude + m_latitudeSweep, localProducts))
        {
        if (IsRealCap (0))
            {
            DEllipse3d cap0;
            double r0 = cos (m_startLatitude);
            double z0 = sin (m_startLatitude);
            cap0.Init (0,0,z0,    r0, 0,0,    0, r0, 0,   0,Angle::TwoPi ());
            accumulateFullDiskAreaIntegrals (cap0, localProducts);
            }
        if (IsRealCap (1))
            {
            DEllipse3d cap1;
            double phi1 = m_startLatitude + m_latitudeSweep;
            double r1 = cos (phi1);
            double z1 = sin (phi1);
            cap1.Init (0,0,z1, r1, 0,0,   0, r1, 0,    0, Angle::TwoPi ());
            accumulateFullDiskAreaIntegrals (cap1, localProducts);
            }   
        return true;
        }
    localProducts = DMatrix4d::FromZero ();
    localToWorld = Transform::FromIdentity ();        
    return false;
    }


// Succesful output if uniform transform has same XY vectors as skewable, z axis of same length, and all are mutually
//  perpendicular.   Then the original z=1 point is at z=zB in this system.
bool ConvertConeLocalToWorldToRigidScale (TransformCR skewable, TransformR uniform, double &zB)
    {
    DPoint3d origin;
    DVec3d vectorU, vectorV, vectorW;
    skewable.GetOriginAndVectors (origin, vectorU, vectorV, vectorW);
    DVec3d vectorW1;
    vectorW1.GeometricMeanCrossProduct (vectorU, vectorV);
    uniform.InitFromOriginAndVectors (origin, vectorU, vectorV, vectorW1);
    RotMatrix Q;
    uniform.GetMatrix (Q);
    double scale;
    RotMatrix unitDirections;
    return DoubleOps::SafeDivide (zB, vectorW.Magnitude (), vectorW1.Magnitude (), 1.0)
        && Q.IsRigidScale (unitDirections, scale);
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnConeDetail::ComputeSecondMomentAreaProducts (TransformR localToWorld, DMatrix4dR localProducts) const
    {
    double radiusA, radiusB;
    Transform worldToLocal, skewLocalToWorld;
    GetTransforms (skewLocalToWorld, worldToLocal, radiusA, radiusB);
    double zB;
    
    if (ConvertConeLocalToWorldToRigidScale (skewLocalToWorld, localToWorld, zB)
        && coneAreaMoments (radiusA, radiusB, zB, localProducts))
        {
        DEllipse3d cap;          
        if (IsRealCap (0))
            {
            cap.Init (0,0,0, radiusA, 0,0,  0,radiusA,0, 0.0, Angle::TwoPi ());
            accumulateFullDiskAreaIntegrals (cap, localProducts);
            }
        if (IsRealCap (1))
            {
            cap.Init (0,0,zB, radiusB, 0,0,  0,radiusB,0, 0.0, Angle::TwoPi ());
            accumulateFullDiskAreaIntegrals (cap, localProducts);
            }
        return true;
        }
    localProducts = DMatrix4d::FromZero ();
    localToWorld = Transform::FromIdentity ();
    return false;
    }

DEllipse3d unitTorusSectionEllipse (double minorRadius, double majorRadius, double theta)
    {
    double c = cos (theta);
    double s = sin (theta);
    return DEllipse3d::From (majorRadius * c, majorRadius * s, 0.0,
                  minorRadius * c, minorRadius * s, 0.0,
                  0,               0,               minorRadius,
                  0.0, Angle::TwoPi ());
    
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnTorusPipeDetail::ComputeSecondMomentAreaProducts (TransformR localToWorld, DMatrix4dR localProducts) const
    {
    DPoint3d center;
    RotMatrix matrix, unitMatrix;
    double scale;
    double minorRadius, majorRadius, sweep;
    if  (   TryGetFrame (center, matrix, minorRadius, majorRadius, sweep)
        &&  matrix.IsRigidScale (unitMatrix, scale)
        &&  unitTorusAreaMoments (m_minorRadius, m_majorRadius, 0.0, m_sweepAngle, localProducts)
        )
        {
        if (!Angle::IsFullCircle (m_sweepAngle) && m_capped)
            {
            accumulateFullDiskAreaIntegrals (unitTorusSectionEllipse (majorRadius, minorRadius, 0.0), localProducts);
            accumulateFullDiskAreaIntegrals (unitTorusSectionEllipse (majorRadius, minorRadius, m_sweepAngle), localProducts);
            }
        localToWorld = Transform::From (matrix, center);
        return true;
        }
    localProducts = DMatrix4d::FromZero ();
    localToWorld = Transform::FromIdentity ();        
    return false;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnBoxDetail::ComputeSecondMomentAreaProducts (TransformR localToWorld, DMatrix4dR localProducts) const
    {
    static int s_optimizeParallelogram = true;
    static int s_numGaussPoint = 5;
    BoxFaces cornerData;
    cornerData.Load (*this);
    
    // setup machinery for both (a) a gauss rule for curved patch and (b) a reference unit square
    //   for parallelograms.
    BSIQuadraturePoints gaussRule;
    gaussRule.InitGauss (s_numGaussPoint);
    double Iaa = 1.0 / 3.0; // unit square squared product in either coordinate.
    double Iab = 1.0 / 4.0; // unit square mixed product
    double Ia  = 1.0 / 2.0; // unit square centroid in either coordinate.
    DMatrix4d unitSquareMoments = DMatrix4d::FromRowValues (
            Iaa, Iab, 0, Ia,
            Iab, Iaa, 0, Ia,
            0,   0,   0,  0,
            Ia,  Ia,  0,  1
            );
    DVec3d zero;
    zero.Zero ();
    bool capped = m_capped;
    localProducts = DMatrix4d::FromZero ();
    for (int i = 0; i < 6; i++)
        {
        if (!capped && cornerData.IsCapFace (i))
            continue;
        DBilinearPatch3d patch = cornerData.GetFace (i);
        if (s_optimizeParallelogram && patch.IsParallelogram ())
            {
            Transform placement;
            DVec3d U = patch.GetUEdgeVector (0);
            DVec3d V = patch.GetVEdgeVector (0);
            placement.InitFromOriginAndVectors (patch.point[0][0], U, V, zero);
            DVec3d J;
            J.CrossProduct (U, V);
            double detJ = J.Magnitude ();
            DMatrix4d parallelogramProducts = DMatrix4d::FromSandwichProduct (placement, unitSquareMoments, detJ);
            localProducts.Add (parallelogramProducts);
            }
        else
            AddBilinearQuadAreaMoments (patch, gaussRule, localProducts);
        }
    localToWorld = Transform::FromIdentity ();
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnExtrusionDetail::ComputeSecondMomentAreaProducts (TransformR localToWorld, DMatrix4dR localProducts) const
    {
    localToWorld = Transform::FromIdentity ();  
    localProducts = DMatrix4d::FromZero ();

    DMatrix4d baseAreaProducts = DMatrix4d::FromZero ();
    DMatrix4d baseWireProducts = DMatrix4d::FromZero ();
    Transform transform = Transform::From (m_extrusionVector);
    
    if (!m_baseCurve->ComputeSecondMomentWireProducts (baseWireProducts))
        return false;
    localProducts = DMatrix4d::SweepMomentProducts (baseWireProducts, m_extrusionVector);
    if (m_capped && m_baseCurve->IsAnyRegionType ())
        {
        if (!m_baseCurve->ComputeSecondMomentAreaProducts (baseAreaProducts))
            return false;
        localProducts.Add (baseAreaProducts);
        
        DMatrix4d topAreaProducts = DMatrix4d::FromSandwichProduct (transform, baseAreaProducts, 1.0);
        localProducts.Add (topAreaProducts);
        }
   return true;
   }
static double s_arcAngleTolerance = 0.2;
static double s_bcurveAngleTolerance = 0.15;
struct AreaMomentAccumulator
{
DMatrix4d m_products;
BSIQuadraturePoints m_lineGauss;
BSIQuadraturePoints m_arcGauss;
IFacetOptionsPtr m_arcStrokeOptions;
IFacetOptionsPtr m_bcurveStrokeOptions;

bool GetPrincipalMoments (double &area, DVec3dR centroid, RotMatrixR axes, DVec3dR momentxyz)
    {
    return m_products.ConvertInertiaProductsToPrincipalAreaMoments (Transform::FromIdentity (),
            area, centroid, axes, momentxyz);
    }
bool GetProducts (TransformR localToWorld, DMatrix4dR localProducts)
    {
    localProducts = m_products;
    localToWorld = Transform::FromIdentity ();
    return true;
    }    


AreaMomentAccumulator ()
    {
    m_products = DMatrix4d::FromZero ();
    m_lineGauss.InitGauss (5);
    m_arcGauss.InitGauss (7);
    m_arcStrokeOptions = IFacetOptions::CreateForCurves ();
    m_arcStrokeOptions->SetAngleTolerance (s_arcAngleTolerance);

    m_bcurveStrokeOptions = IFacetOptions::CreateForCurves ();
    m_bcurveStrokeOptions->SetParamsRequired (true);
    m_bcurveStrokeOptions->SetAngleTolerance (s_bcurveAngleTolerance);
    m_bcurveStrokeOptions->SetCurveParameterMapping (CURVE_PARAMETER_MAPPING_BezierFraction);    
    }

    
bool AddClosed (CurveVectorCR curves)
    {
    if (!curves.IsAnyRegionType ())
        return true;    // There is no area to add.  This is not an error !!!
    DMatrix4d regionProducts;
    if (!curves.ComputeSecondMomentAreaProducts (regionProducts))
        return false;
    m_products.Add (regionProducts);
    return true;
    }

// integrate in a grid between the two ellipses.
bool AddBetween (DEllipse3d ellipseA, DEllipse3d ellipseB, double f0, double f1)
    {
    DPoint3d XA, XB, X;
    DVec3d  dXAdu, dXBdu, dXdv, dXdu, ddXA, ddXB;
    double checksum = 0.0;
    double u, v, wu, wv, detJ;
    DMatrix4d products = DMatrix4d::FromZero ();
    for (int i = 0; m_lineGauss.GetEval (i, f0, f1, u, wu); i++)
        {
        ellipseA.FractionParameterToDerivatives (XA, dXAdu, ddXA, u);
        ellipseB.FractionParameterToDerivatives (XB, dXBdu, ddXB, u);
        dXdv.DifferenceOf (XB, XA);
        for (int j = 0; m_arcGauss.GetEval (j, 0.0, 1.0, v, wv); j++)
            {
            X.Interpolate (XA, v, XB);
            dXdu.Interpolate (dXAdu, v, dXBdu);
            detJ = dXdu.CrossProductMagnitude (dXdv);
            products.AddSymmetricScaledOuterProduct (X, detJ * wu * wv);
            checksum += wu * wv;
            }
        }
    assert (DoubleOps::AlmostEqual (checksum, f1 - f0));
    m_products.Add (products);        
    return true;
    
    }

bool AddBetween (DEllipse3d ellipseA, DEllipse3d ellipseB)
    {
    size_t numStrokeA = m_arcStrokeOptions->EllipseStrokeCount (ellipseA);
    size_t numStrokeB = m_arcStrokeOptions->EllipseStrokeCount (ellipseB);
    size_t numStroke = std::max (numStrokeA, numStrokeB);
    if (numStroke < 1)
        numStroke = 1;  // what would cause this?
    double df = 1.0 / numStroke;
    for (size_t i = 0; i < numStroke; i++)
        {
        double f0 = i * df;
        double f1 = (i+1) * df;
        AddBetween (ellipseA, ellipseB, f0, f1);
        }
    return true;
    }

    
bool AddBetween (DSegment3dCR segmentA, DSegment3d segmentB)
    {
    DBilinearPatch3d patch (segmentA, segmentB);
    DMatrix4d products = DMatrix4d::FromZero ();
    AddBilinearQuadAreaMoments (patch, m_lineGauss, products);
    m_products.Add (products);        
    return true;
    }

bool AddBetween (CurveVectorCR curveA, CurveVectorCR curveB);   // for recursive calls ...
bool AddBetween (ICurvePrimitiveCR curveA, ICurvePrimitiveCR curveB)
    {
    
    ICurvePrimitive::CurvePrimitiveType typeA = curveA.GetCurvePrimitiveType ();
    ICurvePrimitive::CurvePrimitiveType typeB = curveB.GetCurvePrimitiveType ();
    if (typeA != typeB)
        return false;
        

    DSegment3d segmentA, segmentB;
    DEllipse3d ellipseA, ellipseB;
    size_t numLinestringPoints;
    bvector<DPoint3d> const *linestringA;
    bvector<DPoint3d> const *linestringB;
    if (typeA == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector)
        {
        return AddBetween (
                    *curveA.GetChildCurveVectorCP (),
                    *curveB.GetChildCurveVectorCP ()
                    );
        }
    else if (typeA == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString
            && NULL != (linestringA = curveA.GetLineStringCP ())
            && NULL != (linestringB = curveB.GetLineStringCP ())
            && (numLinestringPoints = linestringA->size ()) == linestringB->size ()
            )
        {
        for (size_t i = 1; i < numLinestringPoints; i++)
            {
            segmentA.Init (linestringA->at(i-1), linestringA->at(i));
            segmentB.Init (linestringB->at(i-1), linestringB->at(i));
            if (!AddBetween (segmentA, segmentB))
                return false;
            }
        return true;
        }
    else if (typeA == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line
        && curveA.TryGetLine (segmentA)
        && curveA.TryGetLine (segmentB)
        )
        {
        return AddBetween (segmentA, segmentB);
        }
    else if (typeA == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc
        && curveA.TryGetArc (ellipseA)
        && curveB.TryGetArc (ellipseB)
        )
        {
        return AddBetween (ellipseA, ellipseB)        ;
        }
    return false;
    }
};

bool AreaMomentAccumulator::AddBetween (CurveVectorCR curveA, CurveVectorCR curveB)
    {
    CurveVector::BoundaryType typeA = curveA.GetBoundaryType ();
    CurveVector::BoundaryType typeB = curveB.GetBoundaryType ();
    size_t numA = curveA.size ();
    size_t numB = curveB.size ();
    if (typeA != typeB)
        return false;
    if (numA != numB)
        return false;
    // this is "just" an area sweep.  Recursion is pretty simple..
    for (size_t i = 0; i < numA; i++)
        {
        if (!AddBetween (*curveA[i], *curveB[i]))
            return false;
        }
    return true;
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnRuledSweepDetail::ComputeSecondMomentAreaProducts (TransformR localToWorld, DMatrix4dR localProducts) const
    {
    localProducts = DMatrix4d::FromZero ();
    localToWorld = Transform::FromIdentity ();
        
    AreaMomentAccumulator accumulator;
    size_t numSection = m_sectionCurves.size ();
    if (numSection > 0 && m_capped)
        if (!accumulator.AddClosed (*m_sectionCurves[0]))
            return false;
    if (numSection > 1 && m_capped)
        if (!accumulator.AddClosed (*m_sectionCurves[numSection - 1]))
            return false;
    for (size_t i = 1; i < numSection;i ++)
        {
        if (!accumulator.AddBetween (*m_sectionCurves[i-1], *m_sectionCurves[i]))
            return false;
        }
        
    return accumulator.GetProducts  (localToWorld, localProducts);
    }
GEOMDLLIMPEXP DMatrix4d RotateMoments (DMatrix4dCR baseWedgeIntegrals, double theta0, double theta1);
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnRotationalSweepDetail::ComputeSecondMomentAreaProducts (TransformR localToWorld, DMatrix4dR localProducts) const
    {
    DMatrix4d localDifferentialWireProducts;
    DRay3d myAxis = m_axisOfRotation;
    double myAngle = m_sweepAngle;
    if (myAngle < 0.0)
        {
        myAngle = fabs (myAngle);
        myAxis.direction.Negate ();
        }
    if (m_baseCurve->ComputeSecondMomentDifferentialWireRotationProducts (myAxis, localToWorld, localDifferentialWireProducts))
        {
        localProducts = RotateMoments (localDifferentialWireProducts, 0.0, myAngle);
        return true;
        }
    return false;
    }

static bool InitNullMoments
(
double &area,
DVec3dR centroid,
RotMatrixR axes,
DVec3dR momentxyz
)
    {
    area = 0.0;
    centroid.Zero ();
    axes.InitIdentity ();
    momentxyz.Zero ();
    return false;
    }



bool ISolidPrimitive::ComputePrincipalAreaMoments
(
double &area,
DVec3dR centroid,
RotMatrixR axes,
DVec3dR momentxyz
) const
    {
    DMatrix4d products;
    Transform localToWorld;
    if (_ComputeSecondMomentAreaProducts (localToWorld, products)
        && products.ConvertInertiaProductsToPrincipalAreaMoments (localToWorld,
            area, centroid, axes, momentxyz))
        return true;
    InitNullMoments (area, centroid, axes, momentxyz);
    return false;
    }


//! Return the various integrated products for area moment calculations.   The primitive is treated as a thin shell.
//! @param [out] products integrated [xx xy xz x; xy yy yz y; xz yz zz z; x y z 1] dA
//! @return false if unable to compute.
bool ISolidPrimitive::ComputeSecondMomentAreaProducts (TransformR localToWorld, DMatrix4dR localProducts) const
    {
    if (_ComputeSecondMomentAreaProducts (localToWorld, localProducts))
        return true;
    localProducts = DMatrix4d::FromZero ();
    localToWorld = Transform::FromIdentity ();
    return false;
    }
    
//! Return the various integrated products for area moment calculations.   The primitive is treated as a thin shell.
//! @param [out] products integrated [xx xy xz x; xy yy yz y; xz yz zz z; x y z 1] dA
//! @return false if unable to compute.
bool ISolidPrimitive::ComputeSecondMomentAreaProducts (DMatrix4dR worldProducts) const
    {
    Transform localToWorld;
    DMatrix4d localProducts;
    if (_ComputeSecondMomentAreaProducts (localToWorld, localProducts))
        {
        RotMatrix Q, Q0;
        localToWorld.GetMatrix (Q);
        double scale;
        if (Q.IsRigidScale (Q0, scale))  // It really better be !! The area intgegrals don't map otherwise!!
            {
            worldProducts = DMatrix4d::FromSandwichProduct (localToWorld, localProducts, scale * scale);
            return true;
            }
        }
    worldProducts = DMatrix4d::FromZero ();
    return false;
    }

GEOMDLLIMPEXP bool ISolidPrimitive::ComputeFacetedPrincipalAreaMoments
(
IFacetOptionsP facetOptions,
double &area,
DVec3dR centroid,
RotMatrixR axes,
DVec3dR momentxyz
) const
    {
    static double s_defaultAngleTolerance = 0.10;
    IFacetOptionsPtr optionsPtr = NULL;
    if (NULL == facetOptions)
        {
        optionsPtr = IFacetOptions::New ();
        optionsPtr->SetAngleTolerance (s_defaultAngleTolerance);
        facetOptions = optionsPtr.get ();
        }
    // use significantly tighter angle tolerance than appropriate for
    // display.   We know there are no horrible bspline surfaces that would
    // make turn the tight angle tolernace into trillions of facets ...
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::New (*facetOptions);
    builder->AddSolidPrimitive (*this);
    return builder->GetClientMeshR ().ComputePrincipalAreaMoments (area, centroid, axes, momentxyz);
    }




END_BENTLEY_GEOMETRY_NAMESPACE

