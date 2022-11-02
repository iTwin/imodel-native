/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <math.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

DCatenaryXY::DCatenaryXY (double a) :m_a (a) {}
DCatenaryXY::DCatenaryXY () :m_a(1.0){}

double DCatenaryXY::YAtX (double x) const
    {
    auto u = DoubleOps::ValidatedDivide (x, m_a);
    if (u.IsValid ())
        return m_a * cosh (u);
    return 0.0; // this hits if a==0.  The divide failed.  But since m_a multiplies that, 0 is a plausible result.
    }

double DCatenaryXY::dYdXAtX (double x) const
    {
    auto u = DoubleOps::ValidatedDivide (x, m_a);
    if (u.IsValid ())
        return sinh (u);
    return 0.0; 
    }

double DCatenaryXY::d2YdX2AtX (double x) const
    {
    auto u = DoubleOps::ValidatedDivide (x, m_a);
    if (u.IsValid ())
        {
        auto dd = DoubleOps::ValidatedDivide (cosh(u), m_a);
        if (dd.IsValid ())
            return dd.Value ();
        }
    return 0.0;
    }




double DCatenaryXY::LengthAtX (double x) const
    {
    auto u = DoubleOps::ValidatedDivide (x, m_a);
    if (u.IsValid ())
        return m_a * sinh (u);
    return 0.0;     // bad...
    }

double DCatenaryXY::RadiansAtX (double x) const
    {
    double dydx = dYdXAtX (x);
    return atan (dydx);
    }

DPoint2d DCatenaryXY::XYAtLength (double s) const
    {
    auto u = DoubleOps::ValidatedDivide (s, m_a);
    return DPoint2d::From (
        m_a * asinh (u),
        sqrt (m_a * m_a + s * s)
        );
    }

DVec2d DCatenaryXY::TangentAtLength (double s) const
    {
    auto q = sqrt (m_a * m_a + s * s);
    if (q == 0.0)
        return DVec2d::From (0,0);
    double divr = 1.0 / q;
    return DVec2d::From (m_a * divr, s * divr);
    }

double DCatenaryXY::RadiansAtLength (double s) const
    {
    return atan2 (s, m_a);
    }

double DCatenaryXY::CurvatureAtLength (double s) const
    {
    return m_a / (m_a * m_a + s * s);
    }

void DCatenaryXY::DerivativesAtLength (double s, DPoint2dR uv, DVec2dR duv, DVec2dR dduv, DVec2dR ddduv) const
    {
    double a2 = m_a * m_a;
    double s2 = s * s;
    auto q = sqrt (a2 + s2);
    if (q == 0.0)
        {
        uv.Zero ();
        duv.Zero ();
        dduv.Zero();
        return;
        }

    auto u = DoubleOps::ValidatedDivide (s, m_a);
    uv = DPoint2d::From (
        m_a * asinh (u),
        q
        );

    double divr = 1.0 / q;
    double divr2 = divr * divr;
    double divr3 = divr * divr2;
    double divr5 = divr3 * divr2;
    duv = DVec2d::From (m_a * divr, s * divr);

    dduv = DVec2d::From (- s * m_a * divr3, a2 * divr3); 
    ddduv = DVec2d::From (-m_a * divr3 + 3.0 * s2 * divr5, -3.0 * s * a2 * divr5 );
    }

bool DCatenaryXY::AlmostEqual (DCatenaryXY const &other, double tolerance) const
    {
    return DoubleOps::AlmostEqual (m_a, other.m_a, tolerance);
    }

#define MIN_STROKE_RADIANS (1.0e-3)
#define MAX_STROKE_POINTS (200)

DCatenary3dPlacement::DCatenary3dPlacement (DCatenaryXY const &xyCurve, DPoint3dDVec3dDVec3dCR basis, double distance0, double distance1)
    : m_xyCatenary (xyCurve), m_basis(basis), m_distanceLimits (distance0, distance1)
    {
    }

DCatenary3dPlacement::DCatenary3dPlacement (double a, DPoint3dDVec3dDVec3dCR basis, double distance0, double distance1)
    : m_xyCatenary (a), m_basis (basis), m_distanceLimits (distance0, distance1)
    {
    }

DCatenary3dPlacement::DCatenary3dPlacement (DCatenary3dPlacement  const &other)
    :  m_basis (other.m_basis), m_xyCatenary (other.m_xyCatenary), m_distanceLimits (other.m_distanceLimits)
    {
    }

double DCatenary3dPlacement::StartDistance ()  const {return m_distanceLimits.GetStart ();}
double DCatenary3dPlacement::EndDistance ()  const {return m_distanceLimits.GetEnd ();}

bool DCatenary3dPlacement::AlmostEqual (DCatenary3dPlacement const &other, double tolerance) const
    {
    return m_distanceLimits.AlmostEqual (other.m_distanceLimits, tolerance)
        && DoubleOps::AlmostEqual (1.0 + m_basis.MaxDiff (other.m_basis), 1.0, tolerance)
        && m_xyCatenary.AlmostEqual (other.m_xyCatenary, tolerance);
    }
void DCatenary3dPlacement::ReverseInPlace (){m_distanceLimits.ReverseInPlace ();}

DCatenary3dPlacement DCatenary3dPlacement::CloneBetweenFractions (double fraction0, double fraction1) const
    {
    return DCatenary3dPlacement (
            m_xyCatenary,
            m_basis,
            m_distanceLimits.FractionToPoint (fraction0),
            m_distanceLimits.FractionToPoint (fraction1));
    }

void DCatenary3dPlacement::MultiplyInPlace (TransformCR transform)
    {
    transform.Multiply (m_basis.origin);
    transform.MultiplyMatrixOnly (m_basis.vectorU);
    transform.MultiplyMatrixOnly (m_basis.vectorV);
    }
DPoint3d DCatenary3dPlacement::FractionToPoint (double f) const
    {
    double distanceAlong = m_distanceLimits.FractionToPoint (f);
    DPoint2d uv = m_xyCatenary.XYAtLength (distanceAlong);
    return m_basis.Evaluate (uv);
    }

DRay3d DCatenary3dPlacement::FractionToPointAndTangent (double f) const
    {
    double distanceAlong = m_distanceLimits.FractionToPoint (f);
    DPoint2d uv = m_xyCatenary.XYAtLength (distanceAlong);
    DVec2d duv = m_xyCatenary.TangentAtLength (distanceAlong);
    double delta = m_distanceLimits.Delta ();

    DRay3d ray;
    ray.origin = m_basis.Evaluate (uv);
    ray.direction = delta * m_basis.EvaluateVectorOnly (duv);
    return ray;
    }

Transform DCatenary3dPlacement::FractionToPointAndDerivatives (double f) const
    {
    double distanceAlong = m_distanceLimits.FractionToPoint (f);
    DPoint2d uv = m_xyCatenary.XYAtLength (distanceAlong);
    DVec2d duv, dduv, ddduv;
    m_xyCatenary.DerivativesAtLength (distanceAlong, uv, duv, dduv, ddduv);
    double delta = m_distanceLimits.Delta ();
    return Transform::FromOriginAndVectors 
        (
        m_basis.Evaluate (uv),
        delta * m_basis.EvaluateVectorOnly (duv),
        (delta * delta) * m_basis.EvaluateVectorOnly (dduv),
        (delta * delta * delta) * m_basis.EvaluateVectorOnly (ddduv)
        );
    }


bool DCatenary3dPlacement::AppendPlaneIntersections (DPlane3dCR plane, bvector<double> &xValues, bool bounded) const
    {
    // Catenary is X = A + x U + a*cosh(x/a) * V.
    // Define u = x/a.
    //             X = A + u * (U/a) + cosh(u) * (aV)
    //        H = homogeneous plane equation
    //             0 = A DOT H + u * (U/a) DOT H + cosh(u) * (aV) DOT H.
    // Solve the 1D equation for u values. scale back ot x = u*a.
    DPoint4d hPlane;
    plane.GetDPoint4d (hPlane);
    double divA;
    xValues.clear ();
    double a = m_xyCatenary.m_a;
    if (!DoubleOps::SafeDivide (divA, 1.0, a, 0.0))
        return false;
    double alpha = hPlane.DotProduct (m_basis.origin, 1.0);
    double beta  = hPlane.DotProduct (m_basis.vectorU, 0.0) * divA;
    double gamma = hPlane.DotProduct (m_basis.vectorV, 0.0) * a;
    bool stat = DCatenaryXY::CoshIntersectLine (alpha, beta, gamma, xValues);
    for (auto &x : xValues)
        x *= a;
    return stat;
    }


// NEEDS WORK: apply scale factors from placement !!!
void DCatenary3dPlacement::Stroke
(
bvector<DPoint3d> &xyz,
bvector<double> &fraction,
double fraction0,
double fraction1,
double maxChord,
double maxAngle,
double maxEdgeLength
) const
    {
    xyz.clear ();
    fraction.clear ();
    double s0 = m_distanceLimits.FractionToPoint (fraction0);
    double s1 = m_distanceLimits.FractionToPoint (fraction1);
    double dTheta =fabs ( m_xyCatenary.RadiansAtLength(s1) - m_xyCatenary.RadiansAtLength(s0));
    // max curvature appears closest to s = 0 ..
    double minDistance = s0 * s1 < 0.0 ? 0.0 : DoubleOps::Min (fabs (s0), fabs (s1));
    double maxCurvature = m_xyCatenary.CurvatureAtLength (minDistance);

    size_t n = IFacetOptions::DistanceAndTurnStrokeCount (
            m_distanceLimits.Length (), dTheta, maxCurvature,
            maxChord, maxAngle, maxEdgeLength
            );

    double df = 1.0 / (double)n;
    for (size_t i = 0; i <= n; i++)
        {
        double intervalFraction = i * df;
        double s = DoubleOps::Interpolate (s0, intervalFraction, s1);
        DPoint2d uv = m_xyCatenary.XYAtLength (s);
        fraction.push_back (DoubleOps::Interpolate (fraction0, intervalFraction, fraction1));
        xyz.push_back (m_basis.Evaluate (uv.x, uv.y));
        }
    }

void DCatenary3dPlacement::Get (double &a, DPoint3dDVec3dDVec3dR basis, DSegment1dR segment) const
    {
    a = m_xyCatenary.m_a;
    basis = m_basis;
    segment = m_distanceLimits;
    }

// Interface for an object that can evaluate a 1d to 1d function and derivative.
class CoshIntersectLineFunction : FunctionRToRD
{
double m_alpha;
double m_beta;
double m_gamma;
public:
CoshIntersectLineFunction (double alpha, double beta, double gamma)
    : m_alpha (alpha), m_beta(beta), m_gamma(gamma)
    {
    }

// Virtual function
// @param [in] u  variable
// @param [out]f  function value
// @param [out]dfdu  derivative of f wrt u
// @return true if function was evaluated.
bool EvaluateRToRD
(
double u,
double &f,
double &dfdu
) override
    {
    f = m_alpha + m_beta * u + m_gamma * cosh(u);
    dfdu = m_beta + m_gamma * sinh(u);
    return true;
    }
double EvaluateLinePart (double u){return m_alpha + m_beta * u;}
double EvaluateCoshPart (double u){return m_gamma * cosh(u);}

bool RefineRoot (double &u)
    {
    // Allow more iterations than usual ... we know large x is a problem ...
    NewtonIterationsRRToRR newton (Angle::SmallAngle (), Angle::SmallAngle (), 40);
    if (newton.RunNewton (u, *this))
        {
        double yA = EvaluateLinePart (u);
        double yB = EvaluateCoshPart (u);
        double delta = yA + yB;
        double relTol = 1.0e-10;
        return fabs (delta) < relTol * (fabs (yA) + fabs (yB));
        }
    return false;
    }
};

 // Solve for roots of alpha + beta*x + gamma * cosh(x) = 0
bool DCatenaryXY::CoshIntersectLine (double alpha, double beta, double gamma, bvector<double> &roots)
    {
    CoshIntersectLineFunction F (alpha, beta, gamma);
    // f(x) = alpha + beta*x + gamma * cosh(x)
    // f'(x) = beta + gamma * sinh (x)
    // Find x where alpha + beta*x is parallel to cosh (x) . . 
    double xA = asinh (-beta/gamma);
    double yA = F.EvaluateLinePart (xA);
    double yB = F.EvaluateCoshPart (xA);

    // Bad things happen for large x values.
    //    cosh(x) gets huge.
    //     slope of the line gets huge.
    //     intercept of the line gets huge.
    //
    roots.clear ();
    if (DoubleOps::AlmostEqual (yA, yB))
        {
        roots.push_back (xA);
        return true;
        }
    if ((yB - yA) * gamma < 0.0)
        {
        // The cosh is above the line
        return true;
        }
    double step = DoubleOps::MaxAbs (xA, 1.0);
    double xx[2];
    xx[0] = xA - step;
    xx[1] = xA + step;
    int errors = 0;
    for (size_t i = 0; i < 2; i++)
        {
        double x = xx[i];
        if (F.RefineRoot (x))
            {
            roots.push_back (x);
            }
        else
            errors++;
        }
    return errors == 0;
    }







// Interface for an object that can evaluate a 1d to 1d function and derivative.
class CoshIntersectHomogeneousLineFunction : FunctionRRToRRD
{
DVec3d m_hLine;

public:
CoshIntersectHomogeneousLineFunction (DVec3dCR hLine)
    : m_hLine(hLine)
    {
    }

// Virtual function
// @param [in] u  variable
// @param [out]f  function value
// @param [out]dfdu  derivative of f wrt u
// @return true if function was evaluated.
bool EvaluateRRToRRD
(
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
    f = m_hLine.x * u + m_hLine.y * v + m_hLine.z;
    dfdu = m_hLine.x;
    dfdv = m_hLine.y;
    g = v - cosh (u);
    dgdu = -sinh (u);
    dgdv = 1.0;
    return true;
    }

double EvaluateCoshPart (double u) const { return cosh(u);}
double EvaluateImplicitLine (double u, double v) const { return m_hLine.x * u + m_hLine.y * v + m_hLine.z;}
bool RefineRoot (double &u, double &v)
    {
    // Allow more iterations than usual ... we know large x is a problem ...
    double relTol = Angle::SmallAngle ();
    NewtonIterationsRRToRR newton (Angle::SmallAngle (), relTol, 40);
    if (newton.RunNewton (u, v, *this))
        {
        double f, g;
        double dfdu, dfdv, dgdu, dgdv;
        EvaluateRRToRRD (u, v, f,g, dfdu, dfdv, dgdu, dgdv);
        double delta = fabs (f) + fabs (g);
        return fabs (delta) < relTol * (fabs (u) + fabs (v));
        }
    return false;
    }
};

 // Solve for roots of (x,y,1) dot hLine = 0    and y = cosh(x)
bool DCatenaryXY::CoshIntersectHomogeneousLine (DVec3dCR hLine, bvector<double> &roots)
    {
    roots.clear ();
    CoshIntersectHomogeneousLineFunction F (hLine);
    // cosh tangent vector is (1.0, sin (x))
    // line normal is (hx, hy)
    double beta;
    if (!DoubleOps::SafeDivide (beta, -hLine.x, hLine.y, 0.0))
        {
        double x;
        if (DoubleOps::SafeDivide (x, -hLine.z, hLine.x, 0.0))
            {
            roots.push_back (x);
            return true;
            }
        return false;
        }
    double xA = asinh (beta);
    double yA = F.EvaluateCoshPart (xA);
    double fA = F.EvaluateImplicitLine (xA, yA);

    // Bad things happen for large x values.
    //    cosh(x) gets huge.
    //     slope of the line gets huge.
    //     intercept of the line gets huge.
    //
    roots.clear ();
    if (DoubleOps::AlmostEqual (yA, yA + fA))
        {
        roots.push_back (xA);
        return true;
        }
    if (fA * hLine.y > 0.0)
        {
        // The cosh is above the line
        return true;
        }
    double step = DoubleOps::MaxAbs (xA, 1.0);
    double xx[2];
    xx[0] = xA - step;
    xx[1] = xA + step;
    int errors = 0;
    for (size_t i = 0; i < 2; i++)
        {
        double x = xx[i];
        double y = F.EvaluateCoshPart (x);
        if (F.RefineRoot (x, y))
            {
            roots.push_back (x);
            }
        else
            errors++;
        }
    return errors == 0;
    }


















END_BENTLEY_GEOMETRY_NAMESPACE
