/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
namespace Polynomial {

void Bezier::Order2::BasisFunctions (double u, double &b0, double &b1)
    {
    b0 = 1.0 - u;
    b1 = u;
    }

void Bezier::Order3::BasisFunctions (double u, double &b0, double &b1, double &b2)
    {
    double v = 1.0 - u;
    b0 = v * v;
    b1 = 2.0 * u * v;
    b2 = u * u;
    }

void Bezier::Order4::BasisFunctions (double u, double &b0, double &b1, double &b2, double &b3)
    {
    double v = 1.0 - u;
    double uu = u * u;
    double vv = v * v;
    b0 = vv * v;
    b1 = 3.0 * vv * u;
    b2 = 3.0 * v * uu;
    b3 = u * uu;
    }

void Bezier::Order5::BasisFunctions (double u, double &b0, double &b1, double &b2, double &b3, double &b4)
    {
    double v = 1.0 - u;
    double uu = u * u;
    double uuu = uu * u;
    double vv = v * v;
    double vvv = vv * v;
    b0 = vv * vv;
    b1 = 4.0 * vvv * u;
    b2 = 6.0 * vv * uu;
    b3 = 4.0 * v * uuu;
    b4 = uu * uu;
    }



Bezier::Order2::Order2 (){ coffs[0] = coffs[1] = 0.0;}
Bezier::Order2::Order2 (double f0, double f1)
    {
    coffs[0] = f0;
    coffs[1] = f1;
    }

double Bezier::Order2::Evaluate (double u) const
    {
    return (1.0 - u) * coffs[0] + u * coffs[1];
    }

bool Bezier::Order2::Solve
    (double rightHandSide, double &root) const
    {
    double df = coffs[1] - coffs[0];
    return DoubleOps::SafeDivideParameter (root, rightHandSide - coffs[0], df, 0.0);
    }

Bezier::Order3::Order3 (){ coffs[0] = coffs[1] = coffs[2] = 0.0;}



Bezier::Order3::Order3 (double f0, double f1, double f2)
    {
    coffs[0] = f0;
    coffs[1] = f1;
    coffs[2] = f2;
    }

void Bezier::Order3::Scale (double a)
    {
    for (int i = 0; i < 3; i++)
        coffs[i] *= a;
    }

void Bezier::Order3::AddSquaredLinear (double f0, double f1, double a)
    {
    coffs[0] += a * f0 * f0;
    coffs[1] += a * f0 * f1;
    coffs[2] += a * f1 * f1;
    }

int Bezier::Order3::Roots (double targetValue, double *roots, bool restrictTo01) const
    {
    double a0 = coffs[0] - targetValue;
    double a1 = coffs[1] - targetValue;
    double a2 = coffs[2] - targetValue;
    double a01 = a1 - a0;
    double a12 = a2 - a1;
    double a012 = a12 - a01;
    double discriminant = a01 * a01 - a0 * a012;
    if (discriminant < 0.0)
        return 0;
    double du = sqrt (discriminant);
    double zA = -a01 - du;
    double zB = -a01 + du;
    double uA, uB;
    int numRoots = 0;
    if (   DoubleOps::SafeDivide (uA, zA, a012, 0.0)
        && DoubleOps::SafeDivide (uB, zB, a012, 0.0))
        {
        if (fabs (uA - uB) < Angle::SmallAngle ())
            roots[numRoots++] = uA;
        else if (uA < uB)
            {
            roots[numRoots++] = uA;
            roots[numRoots++] = uB;
            }
        else // uB < uA
            {
            roots[numRoots++] = uB;
            roots[numRoots++] = uA;
            }

        }
    else if (DoubleOps::SafeDivide (uA, -0.5 * a0, a01, 0.0))
        {
        roots[numRoots++] = uA;
        }
    if (!restrictTo01)
        return numRoots;

    int numAccept = 0;
    for (int i = 0; i < numRoots; i++)
        {
        if (DoubleOps::IsIn01 (roots[i]))
            roots[numAccept++] = roots[i];
        }
    return numAccept;
    }



double Bezier::Order3::Evaluate (double u) const
    {
    double v = 1.0 - u;
    return coffs[0] * v * v + u * (2.0 * coffs[1] * v + coffs[2] * u);
    }
Bezier::Order3::Order3 (Bezier::Order3 &other, double scale)
    {
    for (int i = 0; i < 3; i++)
        coffs[i] = other.coffs[i] * scale;
    }

Bezier::Order4::Order4 (){ coffs[0] = coffs[1] = coffs[2] = coffs[3] = 0.0;}
Bezier::Order4::Order4 (double f0, double f1, double f2, double f3)
    {
    coffs[0] = f0;
    coffs[1] = f1;
    coffs[2] = f2;
    coffs[3] = f3;
    }

Bezier::Order4::Order4 (Bezier::Order3 const &factorA, Bezier::Order2 const &factorB)
    {
    coffs[0] = factorA.coffs[0] * factorB.coffs[0];
    coffs[1] = (factorA.coffs[0] * factorB.coffs[1] + 2.0 * factorA.coffs[1] * factorB.coffs[0]) / 3.0;
    coffs[2] = (2.0 * factorA.coffs[1] * factorB.coffs[1] + factorA.coffs[2] * factorB.coffs[0]) / 3.0;
    coffs[3] = factorA.coffs[2] * factorB.coffs[1];
    }

double Bezier::Order4::Evaluate (double u) const
    {
    double v1 = 1.0 - u;
    double v2 = v1 * v1;
    double v3 = v2 * v1;
    return coffs[0] * v3
         + u * (3.0 * coffs[1] * v2
               + u * (3.0 * coffs[2] * v1
                        + u * coffs[3]));
    }





Bezier::Order5::Order5 (){ coffs[0] = coffs[1] = coffs[2] = coffs[3] = coffs[4] = 0.0;}
Bezier::Order5::Order5 (double f0, double f1, double f2, double f3, double f4)
    {
    coffs[0] = f0;
    coffs[1] = f1;
    coffs[2] = f2;
    coffs[3] = f3;
    coffs[4] = f4;
    }

Bezier::Order6::Order6 (){ coffs[0] = coffs[1] = coffs[2] = coffs[3] = coffs[4] = coffs[5] = 0.0;}
Bezier::Order6::Order6 (double f0, double f1, double f2, double f3, double f4, double f5)
    {
    coffs[0] = f0;
    coffs[1] = f1;
    coffs[2] = f2;
    coffs[3] = f3;
    coffs[4] = f4;
    coffs[5] = f5;
    }

void Bezier::Order6::AddProduct (Bezier::Order3 const &f, Bezier::Order4 const &g, double a)
    {
    double product[6];
    bsiBezier_univariateProduct (
            product, 0, 1,
            const_cast<double*>(f.coffs), 3, 0, 1,
            const_cast<double*>(g.coffs), 4, 0, 1);
    for (int i = 0; i < 6; i++)
        coffs[i] += a * product[i];
    }

void Bezier::Order5::Scale (double a)
    {
    for (int i = 0; i < 5; i++)
        coffs[i] *= a;
    }

double Bezier::Order5::Evaluate (double u) const
    {
    double v1 = 1.0 - u;
    double v2 = v1 * v1;
    double v3 = v2 * v1;
    double v4 = v2 * v2;
    return coffs[0] * v4
         + u * (4.0 * coffs[1] * v3
               + u * (6.0 * coffs[2] * v2
                      + u * (4.0 * coffs[3] * v1
                            + u * coffs[4])));
    }

static bool FixedDegreeSolver (double roots [], int &numRoots, int maxRoots, double const *coffs, int order)
    {
    double allRoots[MAX_BEZIER_ORDER];
    numRoots = 0;
    int numAllRoots;
    if (bsiBezier_univariateRoots (allRoots, &numAllRoots, const_cast<double*>(coffs), order))
        {
        if (numAllRoots >= order || numAllRoots > maxRoots)   // all zeros   !!!
            return false;
        numRoots = numAllRoots;
        for (int i = 0; i < numRoots; i++)
            roots[i] = allRoots[i];
        return true;
        }
    return false;    
    }

bool Bezier::Order5::Solve (double roots[], int &numRoots, int maxRoots) const
    {
    return FixedDegreeSolver (roots, numRoots, maxRoots, coffs, 5);
    }
bool Bezier::Order6::Solve (double roots[], int &numRoots, int maxRoots) const
    {
    return FixedDegreeSolver (roots, numRoots, maxRoots, coffs, 6);
    }

// add a scaled product of quadratics to a quarter
void Bezier::Order5::AddProduct (Bezier::Order3 const &f, Bezier::Order3 const &g, double a)
    {
    coffs[0] += a * f.coffs[0] * g.coffs[0];
    coffs[1] += a * (f.coffs[0] * g.coffs[1] + f.coffs[1] * g.coffs[0]) * 0.5;
    coffs[2] += a * (f.coffs[0] * g.coffs[2] + 4.0 * f.coffs[1] * g.coffs[1] + f.coffs[2] * g.coffs[0]) / 6.0;
    coffs[3] += a * (f.coffs[1] * g.coffs[2] + f.coffs[2] * g.coffs[1]) * 0.5;
    coffs[4] += a * f.coffs[2] * g.coffs[2];
    }

Power::Degree2 Power::Degree2::FromRootsAndC2 (double root0, double root1, double c2)
    {
    return Degree2
        (
        c2 * root0 * root1,
        - c2 * (root0 + root1),
        c2
        );
    }
Power::Degree2::Degree2 (double c0, double c1, double c2)
    {
    coffs[0] = c0;
    coffs[1] = c1; 
    coffs[2] = c2;
    }

Power::Degree2::Degree2 ()
    {
    coffs[0] = coffs[1] = coffs[2] = 0.0;
    }

void Power::Degree2::AddConstant (double a)
    {
    coffs[0] += a;
    }
// Add (a + b*x)^2 to the quadratic coefficients
void Power::Degree2::AddSquaredLinearTerm (double a, double b)
    {
    coffs[0] += a * a;
    coffs[1] += 2.0 * a * b;
    coffs[2] += b * b;
    }
// Add s * (a + b*x)^2 to the quadratic coefficients
void Power::Degree2::AddSquaredLinearTerm (double a, double b, double s)
    {
    coffs[0] += s * (a * a);
    coffs[1] += s * (2.0 * a * b);
    coffs[2] += s * (b * b);
    }

int Power::Degree2::RealRoots (double ss[2]) const
    {
    int n = bsiMath_solveQuadratic (ss, coffs[2], coffs[1], coffs[0]);
    if (n == 1)
        {
        ss[1] = ss[0];
        }
    else if (n == 2)
        {
        if (ss[0] > ss[1])
            {
            std::swap (ss[0], ss[1]);
            }
        }
    return n;
    }

int Power::Degree2::RealRootsWithSafeDivideCheck (double ss[2]) const
    {
    int n = 0;
    double discriminant = coffs[1] * coffs[1] - 4.0 * coffs[0] * coffs[2];
    if (discriminant >= 0.0)
        {
        double q = sqrt (discriminant);
        double twoa = 2.0 * coffs[2];
        double twoc = 2.0 * coffs[0];
        double bneg = -coffs[1];
        double largeSum = bneg > 0.0 ? bneg + q : bneg - q;
        if (DoubleOps::SafeDivide (ss[n], twoc, largeSum, 0.0))
            n++;
        if (DoubleOps::SafeDivide (ss[n], largeSum, twoa, 0.0))
            n++;
        }
    if (n == 1)
        {
        ss[1] = ss[0];
        }
    else if (n == 2)
        {
        if (ss[0] == ss[1])
            n = 1;
        else if (ss[0] > ss[1])
            {
            std::swap (ss[0], ss[1]);
            }
        }
    return n;
    }


double Power::Degree2::Evaluate (double x) const
    {
    return coffs[0] + x * (coffs[1] + x * coffs[2]);
    }

bool Power::Degree2::TryGetVertexFactorization (double &x0, double &y0, double &c) const
    {
    y0 = 0.0;
    c  = coffs[2];
    if (DoubleOps::SafeDivide (x0, -coffs[1], 2.0 * coffs[2], 0.0))
        {
        y0 = Evaluate (x0);
        return true;
        }
    return false;
    }

Implicit::Torus::Torus (double R, double r, bool reversePhi)
    {
    m_R = R;
    m_r = r;
    m_reversePhi = reversePhi;
    }

// Return size of box (e.g. for use as scale factor)
double Implicit::Torus::BoxSize () const { return fabs (m_R) + fabs (m_r);}
double Implicit::Torus::ImplicitFunctionScale () const
    {
    double a = BoxSize ();
    if (a == 0.0)
        return 1.0;
    return 1.0 / (a * a * a * a);
    }
// Implicit equation for the torus is ...
// (x^2+y^2+z^2+(R^2-r^2))^2 = 4 R^2(x^2+y^2)
// x,y,z are weighted,
// (x^2+y^2+z^2+(R^2-r^2)w^2)^2 = 4 R^2 w^2 (x^2+y^2)
double Implicit::Torus::EvaluateImplicitFunction (double x, double y, double z) const
    {
    double rho2 = x * x + y *y;
    double z2   = z * z;
    double R2 = m_R * m_R;
    double r2 = m_r * m_r;
    double f  = rho2 + z2 + (R2 - r2);
    double g  = 4.0 * R2 * rho2;
    return (f * f - g) * ImplicitFunctionScale ();
    }

double Implicit::Torus::EvaluateImplicitFunction (DPoint3d xyz) const
    {
    return EvaluateImplicitFunction (xyz.x, xyz.y, xyz.z);
    }

double Implicit::Torus::EvaluateImplicitFunction (double x, double y, double z, double w) const
    {
    double rho2 = x * x + y *y;
    double z2   = z * z;
    double w2   = w * w;
    double R2 = m_R * m_R;
    double r2 = m_r * m_r;
    double f  = rho2 + z2 + w2 * (R2 - r2);
    double g  = w2 * 4.0 * R2 * rho2;
    return (f * f - g) * ImplicitFunctionScale ();
    }

int Implicit::Torus::IntersectRay (DRay3dCR ray, double *rayFractions, DPoint3d *points, int maxHit) const
    {
    int numPoints = 0;
    DSegment3d segment;
    DRange1d rayFractionRange;
    DRange3d torusRange;
    torusRange.Init ();
    double b = m_R + m_r;
    torusRange.Extend (b, b, m_r);
    torusRange.Extend (-b, -b, -m_r);
    if (ray.ClipToRange (torusRange, segment, rayFractionRange))
        {
        Bezier::Order3 quadA, quadB;
        quadA.AddSquaredLinear (segment.point[0].x, segment.point[1].x, 1.0);
        quadA.AddSquaredLinear (segment.point[0].y, segment.point[1].y, 1.0);
        quadB = quadA;  // xx + yy
        quadA.AddSquaredLinear (segment.point[0].z, segment.point[1].z, 1.0);
        double RR = m_R * m_R;
        quadA.AddSquaredLinear (1.0, 1.0, RR - m_r * m_r);

        Bezier::Order3 quadRW (RR, RR, RR);
        Bezier::Order5 quartic;
        quartic.AddProduct (quadA, quadA, 1.0);
        quartic.AddProduct (quadRW, quadB, -4.0);
        quartic.Scale (ImplicitFunctionScale ());

        int numRoots;
        double segmentFractions[6];
        if (quartic.Solve (segmentFractions, numRoots, 6) && numRoots <= maxHit)
            {
            for (int i = 0; i < numRoots; i++)
                {
                rayFractionRange.FractionToDouble (segmentFractions[i], rayFractions[i], 0.0);
                points[i] = ray.FractionParameterToPoint (rayFractions[i]);
                }
            numPoints = numRoots;
            }
        }
    return numPoints;
    }

DPoint3d Implicit::Torus::EvaluateThetaPhi (double theta, double phi) const
    {
    double c = cos(theta);
    double s = sin(theta);

    // theta=0 point.
    double x0 = m_R + m_r * cos (phi);
    double z0 = OrientPhiCoordinate (m_r) * sin (phi);
    return DPoint3d::From (c * x0, s * x0, z0);
    }

GEOMDLLIMPEXP void Implicit::Torus::EvaluateDerivativesThetaPhi (double theta, double phi, DVec3dR dXdTheta, DVec3dR dXdPhi) const    
    {
    double cTheta = cos (theta);
    double sTheta = sin (theta);
    double bx = m_r * cos (phi);
    double bz = m_r * sin (phi);

    double x0  = m_R + bx;
    dXdTheta.Init (-x0 * sTheta, x0 * cTheta, 0.0);
    dXdPhi.Init (-cTheta * bz, -sTheta * bz, OrientPhiCoordinate (bx));
    }
DPoint3d Implicit::Torus::EvaluateThetaPhiDistance (double theta, double phi, double distance) const
    {
    double c = cos(theta);
    double s = sin(theta);
    // theta=0 point.
    double x0 = m_R + distance * cos (phi);
    double z0 = OrientPhiCoordinate (distance) * sin (phi);
    return DPoint3d::From (c * x0, s * x0, z0);
    }

bool Implicit::Torus::XYZToThetaPhiDistance (DPoint3dCR xyz, double &theta, double &phi, double &distance, double &rho) const
    {
    rho = xyz.MagnitudeXY ();
    double majorRadiusFactor;
    // hmm... m_R == 0 goes through this safely ...
    bool safeMajor = DoubleOps::SafeDivide (majorRadiusFactor, m_R, rho, 0.0);
    theta = safeMajor ? atan2 (xyz.y, xyz.x) : 0.0;
    DPoint3d majorCirclePoint = DPoint3d::From (majorRadiusFactor * xyz.x,
                majorRadiusFactor * xyz.y, 0.0);
    DVec3d vectorFromMajorCircle = DVec3d::FromStartEnd (majorCirclePoint, xyz);
    distance = vectorFromMajorCircle.Magnitude ();
    double drho = rho - m_R;
    bool safePhi;
    if (xyz.z == 0.0 && drho == 0.0)
        {
        phi = 0.0;
        safePhi = false;
        }
    else
        {
        phi = atan2 (OrientPhiCoordinate (xyz.z), drho);
        safePhi = true;
        }
    return safeMajor && safePhi;
    }

    
double  Implicit::Torus::OrientPhiCoordinate (double z) const {return m_reversePhi ? -z : z;}
    
DEllipse3d Implicit::Torus::MinorCircle (double theta) const
    {
    double c = cos (theta);
    double s = sin (theta);
    return DEllipse3d::From
            (
            c * m_R, s * m_R, 0.0,
            c * m_r, s * m_r, 0.0,
            0.0, 0.0, OrientPhiCoordinate (m_r),
            0.0, Angle::TwoPi ()
            );
    }

DEllipse3d Implicit::Torus::MajorCircle (double phi) const
    {
    double c = cos (phi);
    double s = sin (phi);
    double a = m_R + c * m_r;
    return DEllipse3d::From
            (
            0.0, 0.0, m_r * s,
            a, 0.0, 0.0,
            0.0, a, 0.0,
            0.0, Angle::TwoPi ()
            );
    }





Implicit::Sphere::Sphere (double r)
    : m_r (r)
    {}


//! Evaluate the implicit function at space point (x,y,z)
//! @param [in] x x coordinate
//! @param [in] y y coordinate
//! @param [in] z z coordinate
GEOMDLLIMPEXP double Implicit::Sphere::EvaluateImplicitFunction (double x, double y, double z) const
    {
    return x * x + y * y + z * z - m_r * m_r;
    }

//! Evaluate the implicit function at space point
//! @param [in] xyz coordinates
GEOMDLLIMPEXP double Implicit::Sphere::EvaluateImplicitFunction (DPoint3d xyz) const
    {
    return xyz.x * xyz.x + xyz.y * xyz.y + xyz.z * xyz.z - m_r * m_r;
    }
    
//! Evaluate the implicit function at weighted space point (wx/w, wy/w, wz/w)
//! @param [in] wx (preweighted) x coordinate
//! @param [in] wy (preweighted) y coordinate
//! @param [in] wz (preweighted) z coordinate
//! @param [in] w  weight
GEOMDLLIMPEXP double Implicit::Sphere::EvaluateImplicitFunction (double wx, double wy, double wz, double w) const
    {
    if (w == 0.0)
        return 0.0;
    return (wx * wx + wy * wy + wz * wz) / (w * w) - m_r * m_r;
    }

GEOMDLLIMPEXP bool Implicit::Sphere::XYZToThetaPhiR (DPoint3dCR xyz, double &theta, double &phi, double &r) const
    {
    double rhoSquared = xyz.x * xyz.x + xyz.y * xyz.y;
    double rho = sqrt (rhoSquared);
    r = sqrt (rhoSquared + xyz.z * xyz.z);
    if (r == 0.0)
        {
        theta = phi = 0.0;
        return false;
        }
    else
        {
        phi = atan2 (xyz.z, rho);   // At least one of these is nonzero.
        if (rhoSquared != 0.0)
            {
            theta = atan2 (xyz.y, xyz.x);
            return true;
            }
        else
            {
            theta = 0.0;
            return false;
            }
        }
    }

GEOMDLLIMPEXP int Implicit::Sphere::IntersectRay (DRay3dCR ray, double *rayFractions, DPoint3d *points, int maxHit) const
    {
    Polynomial::Power::Degree2 q;
    // Ray is (origin.x + s * direction.x, etc)
    // squared distance from origin is (origin.x + s*direction.x)^2 + etc
    // sphere radius in local system is 1.
    q.AddSquaredLinearTerm (ray.origin.x, ray.direction.x);
    q.AddSquaredLinearTerm (ray.origin.y, ray.direction.y);
    q.AddSquaredLinearTerm (ray.origin.z, ray.direction.z);
    q.AddConstant (-m_r * m_r);
    double ss[2];
    int n = q.RealRoots (ss);
    if (n > maxHit)
        n = maxHit;
    for (int i = 0; i < n; i++)
        {
        rayFractions[i] = ss[i];
        points[i] = DPoint3d::FromSumOf (ray.origin, ray.direction, ss[i]);
        }
    return n;
    }

//! Compute the point on the surface at specified angles
//! @param [in] theta major circle angle.
//! @param [in] phi minor circle angle.
//! @return point on surface
GEOMDLLIMPEXP DPoint3d Implicit::Sphere::EvaluateThetaPhi (double theta, double phi) const
    {
    double rc = m_r * cos (theta);
    double rs = m_r * sin (theta);
    double cosPhi = cos (phi);
    double sinPhi = sin (phi);
    return DPoint3d::From (rc * cosPhi, rs * cosPhi, m_r * sinPhi);
    }

//! Compute derivatives of the point on the surface at specified angles
//! @param [in] theta major circle angle.
//! @param [in] phi minor circle angle.
//! @param [out] dXdTheta derivative wrt theta
//! @param [out] dXdPhi derivative wrt phi
GEOMDLLIMPEXP void Implicit::Sphere::EvaluateDerivativesThetaPhi (double theta, double phi, DVec3dR dXdTheta, DVec3dR dXdPhi) const
    {
    double rc = m_r * cos (theta);
    double rs = m_r * sin (theta);
    double cosPhi = cos (phi);
    double sinPhi = sin (phi);
    dXdTheta = DVec3d::From (-rs * cosPhi, rc * cosPhi, 0.0);
    dXdPhi   = DVec3d::From (-rc * sinPhi, -rs * sinPhi, m_r * cosPhi);
    }

GEOMDLLIMPEXP DEllipse3d Implicit::Sphere::MeridianCircle (double theta) const
    {
    double rc = m_r * cos (theta);
    double rs = m_r * sin (theta);
    return DEllipse3d::From (
            0,0,0,
            rc, rs, 0,
            0,0,m_r,
            0.0, Angle::TwoPi ()
        );
    }

GEOMDLLIMPEXP DEllipse3d Implicit::Sphere::ParallelCircle (double phi) const
    {
    double cr = m_r * cos (phi);
    double sr = m_r * sin (phi);
    return DEllipse3d::From (
            0,0,sr,
            cr,0,0,
            0,cr,0,
            0.0, Angle::TwoPi ());
    }

}; // (namespace) Polynomial

END_BENTLEY_GEOMETRY_NAMESPACE