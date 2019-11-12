/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

namespace Polynomial {
namespace Power {
struct Degree2
{
double coffs[3];
//! Construct with zero coefficients
GEOMDLLIMPEXP Degree2 ();

//! Construct from coefficients
//! @parma [in] c0 constant coefficient
//! @param [in] c1 linear coefficient
//! @parma [in] c2 quadratic coefficient
GEOMDLLIMPEXP Degree2 (double c0, double c1, double c2);

//! @param [in] a constant to add
GEOMDLLIMPEXP void AddConstant (double a);

//! Return a quadratic with specified roots and squared term coefficient.
static GEOMDLLIMPEXP Degree2 FromRootsAndC2 (double root0, double root1, double c2);
//! Evalaute at x
GEOMDLLIMPEXP double Evaluate (double x) const ;
//! Add (a0+a1*x)^2
//! @param [in] a0 constant coefficient
//! @param [in] a1 linear coefficient
GEOMDLLIMPEXP void AddSquaredLinearTerm (double a0, double a1);

//! Add scale*(a0+a1*x)^2
//! @param [in] a0 constant coefficient
//! @param [in] a1 linear coefficient
//! @parma [in] scale scale factor
GEOMDLLIMPEXP void AddSquaredLinearTerm (double a0, double a1, double scale);
//! Compute real solutions.  Solutions are sorted.
//! @param [out] ss (caller allocated) array of 2 solutions.
//! @return number of solutions.
GEOMDLLIMPEXP int RealRoots (double ss[2]) const;

//! Compute real solutions.  Solutions are sorted.
//! Toleranceing is based on success of DoubleOps::SafeDivide.
//!  (This is usually appropriate for results that are physical distances)
//! @param [out] ss (caller allocated) array of 2 solutions.
//! @return number of solutions.
GEOMDLLIMPEXP int RealRootsWithSafeDivideCheck (double ss[2]) const;

//! Try to factor as a vertex y value plus squared term.
//! The factored form is y = y0 + c * (x-x0)^2
//!
GEOMDLLIMPEXP bool TryGetVertexFactorization (double &x0, double &y0, double &c) const;
};
};


namespace Bezier {
//! Degree 2 (quadratic) bezier polynomial.
struct Order2;
struct Order3;
struct Order4;
struct Order5;
struct Order6;

struct Order3
{
double coffs[3];

GEOMDLLIMPEXP Order3 ();
GEOMDLLIMPEXP Order3 (double f0, double f1, double f2);

//! Return basis function values at u.
//! @param [in] u parameter
//! @param [out] b0 basis function value.
//! @param [out] b1 basis function value.
//! @parma [out] b2 basis function value
static GEOMDLLIMPEXP void BasisFunctions (double u, double &b0, double &b1, double &b2);


//! Scaled copy of other ..
GEOMDLLIMPEXP Order3 (Order3 &other, double scale);
//! Add a term {a * (f0*(1-u) + v1*u)) to the quadratic
//! @param [in] f0 linear term value at u=0.
//! @parma [in] f1 linear term value at u=1.
GEOMDLLIMPEXP void AddSquaredLinear (double f0, double f1, double a);

//! Scale the polynomial.
//! @param [in] a scale factor
GEOMDLLIMPEXP void Scale (double a);

//! Evaluate the polynomial.
//! @param [in] u parameter for evaluation
GEOMDLLIMPEXP double Evaluate (double u) const;

//! Solve for roots in [0,1] closed.
//! Ignore other roots.
//! Caller must dimension array to at least 2
//! All zero coffs returns 0.
GEOMDLLIMPEXP int Roots (double targetValue, double *roots, bool restrictTo01 = false) const;
};






struct Order2
{
double coffs[2];
GEOMDLLIMPEXP Order2 ();
GEOMDLLIMPEXP Order2 (double f0, double f1);
//! Return basis function values at u.
//! @param [in] u parameter
//! @param [out] b0 basis function value.
//! @param [out] b1 basis function value.
static GEOMDLLIMPEXP void BasisFunctions (double u, double &b0, double &b1);


//! Evaluate the polynomial.
//! @param [in] u parameter for evaluation
GEOMDLLIMPEXP double Evaluate (double u) const;

//! Solve p(u) = rightHandSide
GEOMDLLIMPEXP bool Solve (double rightHandSide, double &root) const;
};
// Degree 3 (cubic) bezier polynomial.
struct Order4
{
double coffs[4];
GEOMDLLIMPEXP Order4 ();
GEOMDLLIMPEXP Order4 (double f0, double f1, double f2, double f3);
// Product of quadratic and linear factors.
GEOMDLLIMPEXP Order4 (Order3 const &factorA, Order2 const &factorB);
GEOMDLLIMPEXP double Evaluate (double u) const;

//! Return basis function values at u.
//! @param [in] u parameter
//! @param [out] b0 basis function value.
//! @param [out] b1 basis function value.
//! @parma [out] b2 basis function value
//! @parma [out] b3 basis function value
static GEOMDLLIMPEXP void BasisFunctions (double u, double &b0, double &b1, double &b2, double &b3);
};

struct Order5
{
double coffs[5];
//! Constructor with zero coefficients.
GEOMDLLIMPEXP Order5 ();
//! Constructor with complete list of coefficients.
GEOMDLLIMPEXP Order5 (double f0, double f1, double f2, double f3, double f4);

//! Scale the polynomial.
//! @param [in] a scale factor
GEOMDLLIMPEXP void Scale (double a);

//! Evaluate the polynomial.
//! @param [in] u parameter for evaluation
GEOMDLLIMPEXP double Evaluate (double u) const;

//! add a scaled product of quadratics to quartic
//! @param [in] f first polynomial
//! @param [in] g second polynomial
//! @param [in] a scale factor
GEOMDLLIMPEXP void AddProduct (Bezier::Order3 const &f, Bezier::Order3 const &g, double a);

//! @return true if valid solution with no more than maxRoots.
//! @param [out] roots (caller allocated) array of roots.
//! @param [out] numRoots number of roots.
//! @param [in] maxRoots caller's size of roots[]
GEOMDLLIMPEXP bool Solve (double roots[], int &numRoots, int maxRoots) const;


//! Return basis function values at u.
//! @param [in] u parameter
//! @param [out] b0 basis function value.
//! @param [out] b1 basis function value.
//! @parma [out] b2 basis function value
//! @parma [out] b3 basis function value
//! @parma [out] b4 basis function value
static GEOMDLLIMPEXP void BasisFunctions (double u, double &b0, double &b1, double &b2, double &b3, double &b4);

};  // struct Degree4

struct Order6
{
double coffs[6];
//! Constructor with zero coefficients.
GEOMDLLIMPEXP Order6 ();
//! Constructor with complete list of coefficients.
GEOMDLLIMPEXP Order6 (double f0, double f1, double f2, double f3, double f4, double f5);

//! add a scaled product of quadratic and cubic
//! @param [in] f first polynomial
//! @param [in] g second polynomial
//! @param [in] a scale factor
GEOMDLLIMPEXP void AddProduct (Bezier::Order3 const &f, Bezier::Order4 const &g, double a);


//! @return true if valid solution with no more than maxRoots.
//! @param [out] roots (caller allocated) array of roots.
//! @param [out] numRoots number of roots.
//! @param [in] maxRoots caller's size of roots[]
GEOMDLLIMPEXP bool Solve (double roots[], int &numRoots, int maxRoots) const;

};


}; // namespace bezier

namespace Implicit {

struct Torus
{
double m_R; // xy circle radius
double m_r; // pipe diameter radius
bool m_reversePhi; // false for phi circle (+X, +Z), true for phi circle (+X,-Z)
//! Constructor from major and minor radius
GEOMDLLIMPEXP Torus (double R, double r, bool reversePhi);

//! @return size of box (e.g. for use as scale factor)
GEOMDLLIMPEXP double BoxSize () const;
//! @return the scale factor applied to the implicit function evaluations to control their range. (1/(R+r)^4)
GEOMDLLIMPEXP double ImplicitFunctionScale () const;

//! Evaluate the implicit function at space point (x,y,z)
//! @param [in] x x coordinate
//! @param [in] y y coordinate
//! @param [in] z z coordinate
GEOMDLLIMPEXP double EvaluateImplicitFunction (double x, double y, double z) const;

//! Evaluate the implicit function at space point
//! @param [in] xyz coordinates
GEOMDLLIMPEXP double EvaluateImplicitFunction (DPoint3d xyz) const;
    
//! Evaluate the implicit function at weighted space point (wx/w, wy/w, wz/w)
//! @param [in] wx (preweighted) x coordinate
//! @param [in] wy (preweighted) y coordinate
//! @param [in] wz (preweighted) z coordinate
//! @param [in] w  weight
GEOMDLLIMPEXP double EvaluateImplicitFunction (double wx, double wy, double wz, double w) const;

GEOMDLLIMPEXP int IntersectRay (DRay3dCR ray, double *rayFractions, DPoint3d *points, int maxHit) const;
//! Compute the point on the surface at specified angles
//! @param [in] theta major circle angle.
//! @param [in] phi minor circle angle.
//! @return point on surface
GEOMDLLIMPEXP DPoint3d EvaluateThetaPhi (double theta, double phi) const;

//! Compute derivatives of the point on the surface at specified angles
//! @param [in] theta major circle angle.
//! @param [in] phi minor circle angle.
//! @param [out] dXdTheta derivative wrt theta
//! @param [out] dXdPhi derivative wrt phi
GEOMDLLIMPEXP void EvaluateDerivativesThetaPhi (double theta, double phi, DVec3dR dXdtheta, DVec3dR dXdPhi) const;

//! Compute the point on the surface at specified angles and multiple of minor radius
//! @param [in] theta major circle angle.
//! @param [in] phi minor circle angle.
//! @param [in] majorCircleDistance distance from major circle.
//! @return point on surface
GEOMDLLIMPEXP DPoint3d EvaluateThetaPhiDistance (double theta, double phi, double majorCircleDistance) const;

//! Inverse of EvaluateThetaPhiLambda.
//! @param [in] xyz space point
//! @param [out] theta major circle angle
//! @param [out] phi minor circle angle
//! @param [out] majorCircleDistance distance from major circle
//! @param [out] rho major plane distance.  (z distance is xyz.z)
//! @return false if any of the inversions are multiple valued.
//!   (i.e. xyz on z axis or xyz on major circle)
//!   BUT ... theta, phi, distance are always valid to return to the same xyz)
//! @remark This does not consider special issues when the minor radius is larger than major radius.
GEOMDLLIMPEXP bool XYZToThetaPhiDistance (DPoint3dCR xyz, double &theta, double &phi, double &majorCircleDistance, double &rho) const;

//! @return minor section circle at specified major circle angle.
//! @param [in] theta major circle angle.
GEOMDLLIMPEXP DEllipse3d MinorCircle (double theta) const;

//! @return minor section circle at specified major circle angle.
//! @param [in] phi minor circle angle.
GEOMDLLIMPEXP DEllipse3d MajorCircle (double phi) const;

//! @return z multiplied by plus or minus 1 per the m_reversePhi flag.
GEOMDLLIMPEXP double OrientPhiCoordinate (double z) const;

};

//! Perfect sphere of radius r centered at origin.
//! Parameterization uses
//!<ul>
//!<li>theta longitude
//!<li>phi latitude
//!</ul>
struct Sphere
{
double m_r;
GEOMDLLIMPEXP Sphere (double r);

//! Evaluate the implicit function at space point (x,y,z)
//! @param [in] x x coordinate
//! @param [in] y y coordinate
//! @param [in] z z coordinate
GEOMDLLIMPEXP double EvaluateImplicitFunction (double x, double y, double z) const;

//! Evaluate the implicit function at space point
//! @param [in] xyz coordinates
GEOMDLLIMPEXP double EvaluateImplicitFunction (DPoint3d xyz) const;
    
//! Evaluate the implicit function at weighted space point (wx/w, wy/w, wz/w)
//! @param [in] wx (preweighted) x coordinate
//! @param [in] wy (preweighted) y coordinate
//! @param [in] wz (preweighted) z coordinate
//! @param [in] w  weight
GEOMDLLIMPEXP double EvaluateImplicitFunction (double wx, double wy, double wz, double w) const;

//! Invert xyz to spherical coordinates.
//! @param [in] xyz cartesian point.
//! @param [out] theta longitude
//! @param [out] phi latitude
//! @param [out] r radius (measured from center, not from nominal surface)
GEOMDLLIMPEXP bool XYZToThetaPhiR (DPoint3dCR xyz, double &theta, double &phi, double &r) const;
GEOMDLLIMPEXP int IntersectRay (DRay3dCR ray, double *rayFractions, DPoint3d *points, int maxHit) const;
//! Compute the point on the surface at specified angles
//! @param [in] theta major circle angle.
//! @param [in] phi minor circle angle.
//! @return point on surface
GEOMDLLIMPEXP DPoint3d EvaluateThetaPhi (double theta, double phi) const;

//! Compute derivatives of the point on the surface at specified angles
//! @param [in] theta major circle angle.
//! @param [in] phi minor circle angle.
//! @param [out] dXdTheta derivative wrt theta
//! @param [out] dXdPhi derivative wrt phi
GEOMDLLIMPEXP void EvaluateDerivativesThetaPhi (double theta, double phi, DVec3dR dXdtheta, DVec3dR dXdPhi) const;

//! @return meridian
//! @param [in] theta longitude angle
GEOMDLLIMPEXP DEllipse3d MeridianCircle (double theta) const;

//! @return Parallel of latitude
//! @param [in] phi latitude
GEOMDLLIMPEXP DEllipse3d ParallelCircle (double phi) const;
};


};
};

END_BENTLEY_GEOMETRY_NAMESPACE

