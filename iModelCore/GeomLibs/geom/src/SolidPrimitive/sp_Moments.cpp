/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/SolidPrimitive/sp_Moments.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "BoxFaces.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

// moments are passed in a DMatrix4d structured as
// [ xx xy xz x]
// [ xy yy yz y]
// [ xz yz zz z]
// [ x  y  z  v]
// Where each term is an integral of (x,y,z, xx, xy, xz, yy, yz, zz) dV
//

//! Integrate moments for a volume of rotational sweep, given integrals of
//! differential wedge.
//! @param [in] baseWedgeIntegrals integrals of (rho*[xx xy xz x; etc]
//!              for the base differential wedge.
//!      If the wedge is due to swept area on the xz plane, this will normally have 0 in
//!      y row and y column, and the rho term is exactly x.
GEOMDLLIMPEXP DMatrix4d RotateMoments
(
DMatrix4dCR baseWedgeIntegrals,
double theta0,
double theta1
)
    {
    RotMatrix trigIntegrals;
    Angle::TrigIntegrals (theta0, theta1, trigIntegrals);
    DMatrix4d basis[3];
    DMatrix4d basisTranspose[3];
    // c * basis[0] + s * basis[1] + basis[2] = rotation around z axis
    basis[0] = DMatrix4d::FromRowValues  (
            1,0,0,0,
            0,1,0,0,
            0,0,0,0,
            0,0,0,0
            );
    basis[1] = DMatrix4d::FromRowValues (
            0, -1, 0, 0,
            1,  0, 0, 0,
            0,  0, 0, 0,
            0,  0, 0, 0
            );
    basis[2] = DMatrix4d::FromRowValues (
            0,0,0,0,
            0,0,0,0,
            0,0,1,0,
            0,0,0,1
            );
            
    for (int i = 0; i < 3; i++)
        basisTranspose[i].TransposeOf (basis[i]);

    // There is a whole lot of multiplying by 0 in this.  (Look at the basis matrix layouts)
    // But this is reliable code, and think of what you are saving by doing this instead of
    // faceting the sweep and integrating over all those little parameter spaces.
    DMatrix4d result = DMatrix4d::FromZero ();
    DMatrix4d AB, ABC;
    for (int i = 0; i < 3; i++)
        {
        AB.InitProduct (basis[i], baseWedgeIntegrals);
        for (int j = 0; j < 3; j++)
            {
            ABC.InitProduct (AB, basisTranspose[j]);
            result.Add (ABC, trigIntegrals.form3d[i][j]);
            }
        }
    return result;
    }

GEOMDLLIMPEXP DMatrix4d RotateMoments_fast
(
DMatrix4dCR baseMoments,
double theta0,
double theta1
)
    {
    double a = baseMoments.coff[0][0];
    double b = baseMoments.coff[0][1];  // same as [1][0] (symmetry assumed)
    double d = baseMoments.coff[1][1];
    RotMatrix trigIntegrals;
    Angle::TrigIntegrals (theta0, theta1, trigIntegrals);
    double ICC = trigIntegrals.form3d[0][0];
    double ICS = trigIntegrals.form3d[0][1];
    double ISS = trigIntegrals.form3d[1][1];
    double IC  = trigIntegrals.form3d[0][2];
    double IS  = trigIntegrals.form3d[1][2]; 
    double I1  = trigIntegrals.form3d[2][2];
// [c -s][a b][ c s] = [ca-sb  cb-sd][ c s]
// [s  c][b d][-s c]   [sa+cb  sb+cd][-s c]
//
// [  cca-2csb+ssd    csa+(cc-ss)b-csd]
// [csa+(cc-ss)b-scd    ssa+2scb+ccd  ]
//
// = a *  <cc,cs,ss> + b * <-2cs,(cc-ss),-2cs> + d * <ss,cs,cc>  (where <a00,a01,a11> is compact 2x2 symmetric matrix)
    DMatrix4d result = DMatrix4d::FromZero ();
    result.coff[0][0] = ICC *a -2.0 * ICS * b +ISS*d;
    result.coff[0][1] = result.coff[1][0] = ICS*a + (ICC-ISS) * b - ICS*d;
    result.coff[1][1] = ISS*a + 2.0*ICS*b + ICC*d;
    
    for (int j = 2; j < 4; j++)
        {
        result.coff[0][j] = result.coff[j][0]
            = IC * baseMoments.coff[0][j] - IS * baseMoments.coff[1][j];
        result.coff[1][j] = result.coff[j][1]
            = IS * baseMoments.coff[0][j] + IC * baseMoments.coff[1][j];
        }
    result.coff[2][2] = I1 * baseMoments.coff[2][2];
    result.coff[3][3] = I1 * baseMoments.coff[3][3];
    result.coff[2][3] = result.coff[3][2] = I1 * baseMoments.coff[2][3];
    return result;
    }


// Compute all moments and volume of the part of unit sphere from latitude phi0 to latitude phi1.
static bool sphereMoments (double phi0, double phi1, DMatrix4dR products)
    {
    double pi = Angle::Pi();
    double s1 = sin(phi1);
    double c1 = cos(phi1);
    double s0 = sin(phi0);
    double c0 = cos(phi0);
    double div3 = 1.0 / 3.0;
    double div23 = 2.0 / 3.0;
    double div4 = 1.0 / 4.0;
    double div5 = 1.0 / 5.0;
    products = DMatrix4d::FromZero ();
    products.coff[0][0] = div4*pi*((s1 - s0) - div23*(pow(s1,3) - pow(s0,3)) + div5*(pow(s1,5) - pow(s0,5)));
    products.coff[1][1] = products.coff[0][0];
    products.coff[2][2] = pi * (div3 * (pow(s1,3) - pow(s0,3)) - div5*(pow(s1,5) - pow(s0,5)));
    products.coff[3][2] = products.coff[2][3] = -div4*pi*(pow(c1,4) - pow(c0,4));
    products.coff[3][3] = pi*(s1 - s0 - div3*(pow(s1,3) - pow(s0,3)));
    return true;
    }
// Compute all moments and volume of a cone around the z axis with radius rA at zA and rB at zB.
static bool coneMoments (double rA, double rB, double zB, DMatrix4dR integrals)
    {
    double pi = Angle::Pi();
    //double m = (rB - rA)/(zB - zA);
    double m = (rB - rA)/zB;
    //double b = (rA*zB - zA*rB)/(zB - zA);
    double b = rA;
    double div20 = 1 / 20.0;
    double div3 = 1 / 3.0;
    double div2 = 1.0 / 2.0;
    double div23 = 2.0 / 3.0;
    double div4 = 1.0 / 4.0;
    double div5 = 1.0 / 5.0;
    double rB2 = rB*rB;
    double rB3 = rB2*rB;
    double rB4 = rB3*rB;
    double rA2 = rA*rA;
    double rA3 = rA2*rA;
    double rA4 = rA3*rA; 
    double zB2 = zB*zB;
    double zB3 = zB2*zB;
    double zB4 = zB3*zB;
    double zB5 = zB4*zB;
    double m2  = m * m;
    double b2  = b * b;
    integrals = DMatrix4d::FromZero (); 
    
    //For zA zero
    integrals.coff[0][0] = pi*div20*zB*(rB4 + rB3*rA + rB2*rA2 + rB*rA3 + rA4);
    integrals.coff[1][1] = integrals.coff[0][0];
    integrals.coff[2][2] = pi*(div5*zB5*m2 + div2*zB4*m*b + div3*zB3*b2);
    integrals.coff[3][2] = integrals.coff[2][3] = pi*(div4*zB4*m2 + div23*zB3*m*b + div2*zB2*b2);
    integrals.coff[3][3] = pi*div3*zB*(rB2 + rB*rA + rA2);
    
    //For nonzero zA
    /*integrals.coff[0][0] = pi*div20*(zB - zA)*(pow(rB,4) + pow(rB,3)*pow(rA,1) + pow(rB,2)*pow(rA,2) + pow(rB,1)*pow(rA,3) + pow(rA,4));
    integrals.coff[1][1] = integrals.coff[0][0];
    integrals.coff[2][2] = pi*(div5*(pow(zB,5)*pow(m,2) - pow(zA,5)*pow(m,2)) + div2*(pow(zB,4)*m*b - pow(zA,4)*m*b) + div3*(pow(zB,3)*pow(b,2) - pow(zA,3)*pow(b,2)));
    integrals.coff[3][2] = integrals.coff[2][3] = pi*(div4*(pow(zB,4)*pow(m,2) - pow(zA,4)*pow(m,2)) + div23*(pow(zB,3)*m*b - pow(zA,3)*m*b) + div2*(pow(zB,2)*pow(b,2) - pow(zA,2)*pow(b,2)));
    integrals.coff[3][3] = pi*div3*(zB-zA)*(pow(rB,2) + rB*rA + pow(rA,2));*/
   /* if (rA = rB)
        {
        integrals.coff[0][0] = pi*pow(rA,4)*div4 * (zB - zA);
        integrals.coff[1][1] = integrals.coff[0][0];
        integrals.coff[2][2] = pi*pow(rA,2)*div3 * (pow(zB,3) - pow(zA,3));
        integrals.coff[3][2] = integrals.coff[2][3] = pi*pow(rA,2)*div2 * (pow(zB,2) - pow(zA,2));
        integrals.coff[3][3] = pi*pow(rA,2) * (zB - zA);
        }*/
    return true;
    }

struct TorusMoments
{
public:
static bool ComputeProducts (double R, double a, double sweepangle, DMatrix4dR products)
    {
    double pi = Angle::Pi();
    double a2 = a * a;
    double a3 = a2 * a;
    double a4 = a3 * a;
    double R2 = R * R;
    double R3 = R2 * R;
    double div2 = 1.0 / 2.0;
    double div4 = 1.0 / 4.0;
    double term1 = pi*R3*a2 + 3*pi*div4*R*a4;
    double term2 = pi*R2*a2 + pi*div4*a4;
    double theta = sweepangle;
    products = DMatrix4d::FromZero (); 
  
        products.coff[0][0] = term1 * (div2*theta + div4*sin(2*theta));
        products.coff[0][1] = products.coff[1][0] = term1*div2*sin(theta)*sin(theta);
        products.coff[0][3] = products.coff[3][0] = term2*sin(theta);
        products.coff[1][1] = term1 * (div2*theta - div4*sin(2*theta));
        products.coff[1][3] = products.coff[3][1] = term2 * (1 - cos(theta));
        products.coff[2][2] = pi*div4*R*a4*theta;
        products.coff[3][3] = pi*R*a2*theta;
    
    return true;
    }

};

static bool boxMoments (double ax, double ay, double bx, double by, double zb, DMatrix4dR integrals)
    {
    double div2 = 1.0 / 2.0;
    double div3 = 1.0 / 3.0;
    double div4 = 1.0 / 4.0;
    double div5 = 1.0 / 5.0;
    double z2 = zb * zb;
    double z3 = z2 * zb;
    double z4 = z3 * zb;
    double z5 = z4 * zb;
    double mx = (bx - ax) / zb;
    double mx2 = mx * mx;
    double mx3 = mx2 * mx;
    double my = (by - ay) / zb;
    double my2 = my * my;
    double my3 = my2 * my;
    double ax2 = ax * ax;
    double ax3 = ax2 * ax;
    double ay2 = ay * ay;
    double ay3 = ay2 * ay;
    integrals = DMatrix4d::FromZero (); 
    
        integrals.coff[0][0] = div3*(div5*z5*mx3*my + div4*z4*(3*mx2*my*ax + mx3*ay) + div3*z3*(3*mx*my*ax2 + 3*mx2*ax*ay) + div2*z2*(ax3*my + 3*mx*ax2*ay) + zb*ax3*ay);
        integrals.coff[0][1] = integrals.coff[1][0] = div4 * (div5*z5*mx2*my2 + div4*z4*(2*mx2*my*ay + 2*my2*mx*ax) + div3*z3*(mx2*ay2 + my2*ax2 + 4*mx*my*ax*ay) + div2*z2*(2*mx*ax*ay2 + 2*my*ay*ax2) + zb*ax2*ay2);
        integrals.coff[0][2] = integrals.coff[2][0] = div2*(div5*z5*mx2*my + div4*z4*(mx2*ay + 2*mx*my*ax) + div3*z3*(2*mx*ax*ay + ax2*my) + div2*z2*ax2*ay);
        integrals.coff[0][3] = integrals.coff[3][0] = div2*(div4*z4*mx2*my + div3*z3*(mx2*ay + 2*mx*my*ax) + div2*z2*(2*mx*ax*ay + ax2*my) + zb*ax2*ay);
        integrals.coff[1][1] = div3*(div5*z5*my3*mx + div4*z4*(3*my2*mx*ay + my3*ax) + div3*z3*(3*mx*my*ay2 + 3*my2*ax*ay) + div2*z2*(ay3*mx + 3*my*ay2*ax) + zb*ay3*ax);
        integrals.coff[1][2] = integrals.coff[2][1] = div2*(div5*z5*my2*mx + div4*z4*(my2*ax + 2*mx*my*ay) + div3*z3*(2*my*ax*ay + ay2*mx) + div2*z2*ay2*ax);
        integrals.coff[1][3] = integrals.coff[3][1] = div2*(div4*z4*my2*mx + div3*z3*(my2*ax + 2*mx*my*ay) + div2*z2*(2*my*ax*ay + ay2*mx) + zb*ay2*ax);
        integrals.coff[2][2] = div5*z5*mx*my + div4*z4*(mx*ay + my*ax) + div3*z3*ax*ay;
        integrals.coff[2][3] = integrals.coff[3][2] = div4*z4*mx*my + div3*z3*(mx*ay + my*ax) + div2*z2*ax*ay;
        integrals.coff[3][3] = div3*z3*mx*my + div2*z2*(mx*ay + my*ax) + zb*ax*ay;
        
    return true;
    }

void sweepRotationalMoments (DMatrix4dCR P, double sweepAngle, DMatrix4dR integrals)
    {
    double t = sweepAngle;
    double tw = 2*t;
    double div2 = 1.0 / 2.0;
    double div4 = div2 * div2;
    double csqtrm = div2*t + div4*sin(tw);
    double ssqtrm  = div2*t - div4*sin(tw);
    double cstrm = div2*sin(t)*sin(t);
    double ctrm = sin(t);
    double strm = (1 - cos(t));
    
    double xx = P.coff[0][0];
    double xy = P.coff[0][1];
    double xz = P.coff[0][2];
    double x = P.coff[0][3];
    double yy = P.coff[1][1];
    double yz = P.coff[1][2];
    double y = P.coff[1][3];
    double zz = P.coff[2][2];
    double z = P.coff[2][3];
    double v = P.coff[3][3];
    
        integrals.coff[0][0] = xx*csqtrm - 2*xy*cstrm + yy*ssqtrm;
        integrals.coff[1][0] = xy*csqtrm + (xx - yy)*cstrm - xy*ssqtrm;
        integrals.coff[2][0] = xz*ctrm - yz*strm;
        integrals.coff[3][0] = x*ctrm - y*strm;
        integrals.coff[0][1] = xy*csqtrm - (xx - yy)*cstrm - xy*ssqtrm;
        integrals.coff[1][1] = yy*csqtrm + 2*xy*cstrm + xx*ssqtrm;
        integrals.coff[2][1] = xz*strm + yz*ctrm;
        integrals.coff[3][1] = x*strm + y*ctrm;
        integrals.coff[0][2] = xz*ctrm - yz*strm;
        integrals.coff[1][2] = xz*strm + yz*ctrm;
        integrals.coff[2][2] = zz*t;
        integrals.coff[3][2] = z*t;
        integrals.coff[0][3] = x*ctrm - y*strm;
        integrals.coff[1][3] = x*strm + y*ctrm;
        integrals.coff[2][3] = z*t;
        integrals.coff[3][3] = v*t;
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnSphereDetail::ComputeSecondMomentVolumeProducts (TransformR localToWorld, DMatrix4dR products) const
    {
    localToWorld = m_localToWorld;
    return sphereMoments (m_startLatitude, m_startLatitude + m_latitudeSweep, products);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnConeDetail::ComputeSecondMomentVolumeProducts (TransformR localToWorld, DMatrix4dR products) const
    {
    double radiusA, radiusB;
    Transform worldToLocal;
    GetTransforms (localToWorld, worldToLocal, radiusA, radiusB);
    return coneMoments (radiusA, radiusB, 1.0, products);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnTorusPipeDetail::ComputeSecondMomentVolumeProducts (TransformR localToWorld, DMatrix4dR products) const
    {
    products = DMatrix4d::FromZero ();
    double radiusA = 0.0, radiusB = 0.0, sweepRadians = 0.0;
    RotMatrix spinAxes;
    DPoint3d center;
    if (TryGetFrame (center, spinAxes, radiusA, radiusB, sweepRadians)
        & TorusMoments::ComputeProducts (radiusA, radiusB, sweepRadians, products)
        )
        {
        localToWorld = Transform::From (spinAxes, center);
        return true;
        }
    products = DMatrix4d::FromZero ();
    localToWorld = Transform::FromIdentity ();
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnBoxDetail::ComputeSecondMomentVolumeProducts (TransformR localToWorld, DMatrix4dR products) const
    {
    products = DMatrix4d::FromZero ();
    double ax, ay, bx, by;
    GetNonUniformTransform (localToWorld, ax, ay, bx, by);
    return boxMoments (ax, ay, bx, by, 1.0, products);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnExtrusionDetail::ComputeSecondMomentVolumeProducts (TransformR localToWorld, DMatrix4dR products) const
    {
    DMatrix4d baseProducts;
    if (m_baseCurve->ComputeSecondMomentAreaProducts (baseProducts))
        {
        products = DMatrix4d::SweepMomentProducts (baseProducts, m_extrusionVector);
        localToWorld = Transform::FromIdentity ();
        return true;
        }
    products = DMatrix4d::FromZero ();
    localToWorld = Transform::FromIdentity ();
    return false;
    }
    
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnRuledSweepDetail::ComputeSecondMomentVolumeProducts (TransformR localToWorld, DMatrix4dR products) const
    {
    products = DMatrix4d::FromZero ();
    localToWorld = Transform::FromIdentity ();
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnRotationalSweepDetail::ComputeSecondMomentVolumeProducts (TransformR localToWorld, DMatrix4dR products) const
    {
    DMatrix4d differentialWedgeProducts = DMatrix4d::FromZero ();
    Transform wedgeToWorld;
    if (m_baseCurve->ComputeSecondMomentDifferentialAreaRotationProducts (m_axisOfRotation, wedgeToWorld, differentialWedgeProducts))
        {
        products = RotateMoments (differentialWedgeProducts, 0.0, m_sweepAngle);
        localToWorld = wedgeToWorld;
        return true;
        }
    products = DMatrix4d::FromZero ();
    localToWorld = Transform::FromIdentity ();
    return false;
    }

static bool InitNullMoments
(
double &volume,
DVec3dR centroid,
RotMatrixR axes,
DVec3dR momentxyz
)
    {
    volume = 0.0;
    centroid.Zero ();
    axes.InitIdentity ();
    momentxyz.Zero ();
    return false;
    }

bool ISolidPrimitive::ComputePrincipalMoments
    (
    double &volume,
    DVec3dR centroid,
    RotMatrixR axes,
    DVec3dR momentxyz
    ) const
    {
    if (!IsClosedVolume ())
        return InitNullMoments (volume, centroid, axes, momentxyz);
    DMatrix4d products;
    Transform localToWorld;
    if (_ComputeSecondMomentVolumeProducts (localToWorld, products)
        && products.ConvertInertiaProductsToPrincipalMoments (localToWorld,
            volume, centroid, axes, momentxyz))
        return true;
    InitNullMoments (volume, centroid, axes, momentxyz);
    return false;    
    }

//! Return the various integrated products for moment calculations.  The primitive is treated as a volume
//! @param [out] products integrated [xx xy xz x; xy yy yz y; xz yz zz z; x y z 1] dA
//! @return false if unable to compute.
bool ISolidPrimitive::ComputeSecondMomentVolumeProducts (TransformR localToWorld, DMatrix4dR products) const
    {
    if (_ComputeSecondMomentVolumeProducts (localToWorld, products))
        return true;
    products = DMatrix4d::FromZero ();
    localToWorld = Transform::FromIdentity ();
    return false;
    }

//! Return the various integrated products for area moment calculations.   The primitive is treated as a thin shell.
//! @param [out] products integrated [xx xy xz x; xy yy yz y; xz yz zz z; x y z 1] dA
//! @return false if unable to compute.
bool ISolidPrimitive::ComputeSecondMomentVolumeProducts (DMatrix4dR worldProducts) const
    {
    Transform localToWorld;
    DMatrix4d localProducts;
    if (_ComputeSecondMomentVolumeProducts (localToWorld, localProducts))
        {
        RotMatrix Q;
        localToWorld.GetMatrix (Q);
        worldProducts = DMatrix4d::FromSandwichProduct (localToWorld, localProducts, Q.Determinant ());
        return true;
        }
    worldProducts = DMatrix4d::FromZero ();
    return false;
    }


GEOMDLLIMPEXP bool ISolidPrimitive::ComputeFacetedPrincipalMoments
    (
    IFacetOptions *facetOptions,
    double &volume,
    DVec3dR centroid,
    RotMatrixR axes,
    DVec3dR momentxyz
    ) const
    {
    static double s_defaultAngleTolerance = 0.10;
    if (!IsClosedVolume ())
        return InitNullMoments (volume, centroid, axes, momentxyz);
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
    return builder->GetClientMeshR ().ComputePrincipalMoments (volume, centroid, axes, momentxyz);
    }


END_BENTLEY_GEOMETRY_NAMESPACE

