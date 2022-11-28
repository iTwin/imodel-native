/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <Bentley/BeNumerical.h>

void TestFunc (double a, double b)
    {
    }

void Callers (double a)
    {
    TestFunc (a, true);
    }

void TestSweeps (double thetaStart, double sweep)
    {
    double residual = Angle::TwoPi () - sweep;
    if (sweep < 0.0)
        residual = -residual;
    double residualFraction = residual / sweep;
    double e = 0.1 * residualFraction;
    double f1 = 1.0 + e;    // steps to slightly positive position.
    double g1 = e - residualFraction;   // slightly positive position addressed as negative.
    double f0 = -e; // steps to slightly negative position.
    double g0 = 1.0 + residualFraction - e; // slightly negative position addressed as positive.
    double theta1 = thetaStart + (1.0 + e) * sweep;   // much closer to 1 end than 0 end.
    Check::Near (1.0,
            Angle::NormalizeToSweep (theta1, thetaStart, sweep, false, false), "expect pullback to 1");
    Check::Near (f1,
            Angle::NormalizeToSweep (theta1, thetaStart, sweep, false, true), "expect short extension from 1");
    Check::Near (g1,
            Angle::NormalizeToSweep (theta1, thetaStart, sweep, true, false), "expect long extension from 0");
    Check::Near (f1,
            Angle::NormalizeToSweep (theta1, thetaStart, sweep, true, true), "expect short extension from 1");

    Check::Near (f1,
            Angle::NormalizeToSweep (theta1, thetaStart, sweep, false, true), "expect short extension from 1");
        

    double theta0 = thetaStart + f0 * sweep;    // much closer to 0 end than 1 end.
    Check::Near (0.0,
            Angle::NormalizeToSweep (theta0, thetaStart, sweep, false, false), "expect pullback to 0");
    Check::Near (g0,
            Angle::NormalizeToSweep (theta0, thetaStart, sweep, false, true), "expect long extension from 1");
    Check::Near (f0,
            Angle::NormalizeToSweep (theta0, thetaStart, sweep, true, false), "expect short extension from 0");
    Check::Near (f0,
            Angle::NormalizeToSweep (theta0, thetaStart, sweep, true, true), "expect short extension from 0");

    Check::Near (g0,
            Angle::NormalizeToSweep (theta0, thetaStart, sweep, false, true), "expect long extension from 1");

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Angle,NormalizeToSweep)
    {
    for (double sweep = 5.0; sweep < Angle::TwoPi (); sweep += 2.0)
        {
        for (double thetaStart = -10.0; thetaStart < 11.0; thetaStart += 1.0)
            {
            TestSweeps (thetaStart, sweep);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Printf,Pound)
    {
    for (double a = 1.0 / 64.0; a < 1000.0; a *= 2.0)
        printf (" (.17g %.17g) (#.17g %#.17g)\n", a, a);
    }

#define N2( x ) ((x)*(x))               // pow( ) is a performance issue
#define N3( x ) ((x)*(x)*(x))           // pow( ) is a performance issue

// newton iteration to find x so that     f (x,a) ~= target
template<typename T>
ValidatedDouble RunNewton (double target, bool (*function)(double x, T &a, double &f, double &dfdx), T a)
    {
    double x = target;
    double dx = 10000.0;
    uint32_t iterations = 0;
    uint32_t numConverged = 0;
    static double s_absTol = 1.0e-10;
    static double s_relTol = 1.0e-10;
    double tolerance = s_absTol + s_relTol * fabs (target);
    while (iterations < 10)
        {
        double f, dfdx;
        if (!function (x, a, f, dfdx)
            || dfdx == 0.0)
            return ValidatedDouble (x, false);
        dx = (f - target) / dfdx;
        x = x - dx;
        if (fabs (dx) < tolerance)
            {
            numConverged++;
            if (numConverged >= 2)
                return ValidatedDouble (x, true);
            }
        else
            numConverged = 0;
        iterations++;
        }
    return ValidatedDouble (x, false);
    }

bool cb_XPlus5GammaX5 (double x, double &gamma, double &f, double &dfdx)
    {
    double x2 = x * x;
    double x4 = x2 * x2;
    f = x * (1.0 + gamma * x4);
    dfdx = 1.0 + 5.0 * gamma * x4;
    return true;
    }
struct CubicSpiralVirtuals {
virtual ~CubicSpiralVirtuals() {}
virtual bool evaluateProjectedCoordinate
(
double xIn,
double &x,      //
double &y,
double &t,
double &r,
double &directCurvature
) = 0;
};

struct CzechCubicSpiral : CubicSpiralVirtuals {
#define LENGTH_TOLERANCE           0.00000001
#define RADIUS_TOLERANCE           0.000000001
#define CENTER_TO_CENTER_TOLERANCE 0.000000001
#define LIKE_SIGNS(a,b) (a * b > 0.0)
static int aecAlg_computeCzechTangentFromLength( double R, double Lp, double Xo, double *X )
    {
    int cycles = 0, idx;
    double lamda, gamma, x[ 3 ], xo[ 3 ], dif[ 3 ];

    lamda = asin( Lp / ( 2.0 * R ) );
    gamma = 1.0 / cos( lamda );
    double gamma1 = gamma * gamma / (40.0 * R * R * Lp * Lp);
    // EDL: gamma = 4RR / (4RR-LL)
    static int s_select = 1;
    if (s_select == 1)
        {
        *X = RunNewton (Xo, cb_XPlus5GammaX5, gamma1);
        return SUCCESS;
        }

    // classic binary search.
    // for input Xo=0, this does produces the middle of an interval with its left at origin -- does NOT return the obvious 0.0!
    x[ 0 ] = 0.0;
    xo[ 0 ] = x[ 0 ] + pow( gamma, 2 ) * pow( x[ 0 ], 5 ) / ( 40.0 * pow( R, 2 ) * pow( Lp, 2 ) );
    dif[ 0 ] = xo[ 0 ] - Xo;

    x[ 2 ] = Lp;
    xo[ 2 ] = x[ 2 ] + pow( gamma, 2 ) * pow( x[ 2 ], 5 ) / ( 40.0 * pow( R, 2 ) * pow( Lp, 2 ) );
    dif[ 2 ] = xo[ 2 ] - Xo;
    
    while( ( fabs( xo[ 0 ] - xo[ 2 ] ) > LENGTH_TOLERANCE ) && ( ++cycles < 100 ) )
    {
        x[ 1 ] = .5 * ( x[ 0 ] + x[ 2 ] );
        xo[ 1 ] = x[ 1 ] + pow( gamma, 2 ) * pow( x[ 1 ], 5 ) / ( 40.0 * pow( R, 2 ) * pow( Lp, 2 ) );
        dif[ 1 ] = xo[ 1 ] - Xo;

        if( LIKE_SIGNS( dif[ 0 ], dif[ 2 ] ) )
            idx = ( fabs( dif[ 0 ] ) < fabs( dif[ 2 ] ) ) ? 2 : 0;
        else
            idx = ( LIKE_SIGNS( dif[ 1 ], dif[ 2 ] ) ) ? 2 : 0;

        dif[ idx ] = dif[ 1 ];
        x[ idx ] = x[ 1 ];
        xo[ idx ] = xo[ 1 ];
    }

    *X = .5 * ( x[ 0 ] + x[ 2 ] );
    return( SUCCESS );
    }

static int aecAlg_computeCzechLoFromRLp( double R, double lp, double *lo, double *lamda, double *gamma )
    {
    *lamda = asin( lp / ( 2. * R ) );
    *gamma = 1. / cos( *lamda );
    *lo = lp + *gamma * *gamma * ( ( lp * lp * lp ) / ( 40. * R * R ) );

    return( SUCCESS );
    }

static void DistanceAlongToXYTR (double R, double spiralLength, double pseudoLength, double partialLength, double &x, double &y, double &t, double &r, double &directCurvature)
    {
    double signage = 1.0;
    double quark, lambda, gamma;
    double totalTangent;
    aecAlg_computeCzechTangentFromLength( R, spiralLength, pseudoLength, &totalTangent);
    aecAlg_computeCzechLoFromRLp (R, totalTangent, &quark, &lambda, &gamma);
    aecAlg_computeCzechTangentFromLength( R, spiralLength, partialLength, &x);
    static int select = 1;
    double u = x;
    if (select == 1)
        u = partialLength;
    double xxx = u * u * u;
    double a = gamma / (6.0 * R * totalTangent);
//    y = gamma * xxx / (6.0 * R * totalTangent);
    y = a * xxx;

    // direct radius of curvature.
    // y = a * xxx
    //double dydx = 3.0 * a * u * u;
    double ddydxdx = 6.0 * a * u;
    double q = 1.0 + ddydxdx * ddydxdx;
    directCurvature = ddydxdx / sqrt (q * q * q);

    double xFraction = x / totalTangent;
    t = atan (tan (lambda) * xFraction * xFraction);
    if (x == 0.0)
        r = 0.0;
    else
        r = 1.0 / (signage * x / (R * totalTangent));
    }
// INSTANCE SECTION
double m_R;     // exit radius
double m_spiralArcLength;  // length along true spiral
double m_projectedLength;   // length along axis

CzechCubicSpiral (double endRadius, double projectedLength, double arcLength) :
    m_R (endRadius), m_spiralArcLength (arcLength), m_projectedLength (projectedLength)
    {
    }
bool evaluateProjectedCoordinate (
double xIn,
double &x,      //
double &y,
double &t,
double &r,
double &directCurvature
) override
    {
    DistanceAlongToXYTR (m_R, m_projectedLength, m_spiralArcLength, xIn, x, y, t, r, directCurvature);
    return true;
    }
};

struct NSWCubicSpiral : CubicSpiralVirtuals {
// INSTANCE SECTION
double m_R;     // exit radius
double m_spiralArcLength;  // length along true spiral
double m_projectedLength;   // length along axis
// This is the APPROXIMATE mapping from "distance along the curve" back to the x axis
double DistanceAlongToX (double s)
    {
    return s * (1.0 - s*s*s*s / (40.0 * m_R * m_R * m_spiralArcLength * m_spiralArcLength));
    }
double SToY (double s)
    {
    return s * s * s / (6.0 * m_R * m_spiralArcLength);
    }
double SToR (double s)
    {
    double curvature0 = 0.0;
    double curvature1 = 1.0 / (m_R);
    double curvatureX = DoubleOps::Interpolate (curvature0, s / m_spiralArcLength, curvature1);
    return curvatureX == 0.0 ? 0.0 : 1.0 / curvatureX;
    }
NSWCubicSpiral (double endRadius, double arcLength) :
    m_R (endRadius), m_spiralArcLength (arcLength)
    {
    m_projectedLength = DistanceAlongToX (arcLength);
    }

bool evaluateProjectedCoordinate (
double s,
double &x,      //
double &y,
double &t,
double &r,
double &directCurvature
) override
    {
    x = DistanceAlongToX (s);
    y = SToY (s);
    t = s * s / ( 2.0 * m_R * m_spiralArcLength);
    r = SToR (s);    // should that go through gamma correction for fraction?
    return true;
    }
};


struct AremaSpiral : CubicSpiralVirtuals {
// INSTANCE SECTION
double m_R;     // exit radius
double m_spiralArcLength;  // length along true spiral

AremaSpiral (double endRadius, double arcLength) :
    m_R (endRadius), m_spiralArcLength (arcLength)
    {
    }

bool evaluateProjectedCoordinate (
double partialLength,
double &x,      //
double &y,
double &t,
double &r,
double &directCurvature
) override
    {
    double curvature;
    DoubleOps::SafeDivide (curvature, 1.0, m_R, 0.0);
    static double s_factorD = 100.0;

    double D = s_factorD * Angle::RadiansToDegrees(curvature);

    double l = partialLength;
    double s = l / 100.0;
    double k = D / ( m_spiralArcLength / 100.0 );
    double theta = 0.5 * k * s * s;

    x = 100.0 * s - 0.000762 * k * k * pow( s, 5 );
    y = 0.291 * k * s * s * s - 0.00000158 * k * k * k * pow( s, 7 );
    t = Angle::DegreesToRadians (theta);
    r = 0.0;//aecAlg_computeRadiusFromPartialLength( spi, partialLength );
    directCurvature = 0.0;
    return true;
    }
};


/*-------------------------------------------------------------------------+
|                                                                          |
|   spu - 27 aug 2009                                                      |
|                                                                          |
+-------------------------------------------------------------------------*/
int newton_raphson_nswsc
(
double ke,
double lz,
double le,
double& xz,
double& xe
)
    {
    int sts = SUCCESS;

    double x;
    double xsav;
    double xesav;
    double lamdax;
    double lamda;
    double gamma0;
    double fx;
    double fdashx;
    double thet;
    double t3;
    double sint;
    double sintsav;
    xe = le;
    xesav = 0;
    while (fabs(xe - xesav) > 0.000001)
        {
        xesav = xe;
        thet = xe * ke / 2.;
        sintsav = 0;
        sint = thet;
        while (fabs(sint - sintsav) > 0.000001)
            {
            sintsav = sint;
            sint = N3(sint) + thet;
            }
        lamdax = asin ( sint );
        lamda = 1. / cos( lamdax );
        t3 = N3(lamda) * N3(lamda) * N2(ke) / 40;;
        gamma0 = t3;
        xe = le - ( gamma0 * N3(xe) );
        }
    thet = xe * ke / 2.;
    sintsav = 0;
    sint = thet;
    while (fabs(sint - sintsav) > 0.000001)
        {
        sintsav = sint;
        sint = N3(sint) + thet;
        }
    lamdax = asin ( sint );
    lamda = 1. / cos( lamdax );
    t3 = N3(lamda) * N3(lamda) * N2(ke) / 40;;
    gamma0 = t3 / ( xe * xe );
    x = lz;
    xsav = 0;
    while (fabs(x - xsav) > 0.000001 )
        {
        xsav = x;
        fx = gamma0 * N3(x) * N2(x) + x - lz;
        fdashx = 5 * gamma0 * N3(x) * x + 1.;
        x = x - (fx / fdashx);
        }
    xz = x;
    return( sts );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CzechSpiral,NewtonStep)
    {
    static double s_uorsPerMeter = 1.0; //10000.0;
    static double s_smallCurvature = 1.0e-14;
    double flagHeight = 2.0 * s_uorsPerMeter;
    // simulate uor computations:
    //    convert metric sizes to uors
    //    use the Check:: transform to scale back to meters for output to dgnjs file
    //    do all origin shifts within this method (rather than via Check::Shift)
    auto transform0 = Check::GetTransform ();
    auto transform1 = Transform::From (RotMatrix::FromScale (1.0 / s_uorsPerMeter), DPoint3d::From (0,0,0));
    Check::SetTransform (transform1);
    double spiralLength = 100 * s_uorsPerMeter;
    double yShift = 0.0;
    bvector<double> radiusMeters {1000, 400, 200 };
    bvector<double> pseudoLengthMeters {100.02506262460328, 100.15873011778872, 100.66666663992606};
    bvector<double> edgeCount {15, 25, 35};
    for (int select : {0,1, 2})
        {
        yShift = 0.0;
        for (size_t i = 0; i < radiusMeters.size (); i++)
            {
            double radius1 = radiusMeters[i] * s_uorsPerMeter;
            double pseudoLength = pseudoLengthMeters[i] * s_uorsPerMeter;
            CubicSpiralVirtuals *spiral = nullptr;
            if (select == 1)
                spiral = new NSWCubicSpiral (radius1, spiralLength);
            else if (select == 2)
                spiral = new AremaSpiral (radius1, spiralLength);
            else
                spiral = new CzechCubicSpiral (radius1, spiralLength, pseudoLength);

            double distanceStep = spiralLength / edgeCount[i];
            //double curvature1 = 1.0 / radius1;
            // unused - double averageRadius = 2.0 * radius1;
            //double averageCircleRadians = spiralLength / averageRadius;
            double directCurvature = 0;
            bvector<DPoint3d>xy;
            double xB = 0, yB = 0, tB = 0, rB = 0;

            for (double distanceAlong = 0.0; distanceAlong <= spiralLength * 1.007; distanceAlong += distanceStep)
                {
                //double xB1, yB1, tB1, rB1, directCurvature1;
                //spiral->DistanceAlongToXYTR (radius1, spiralLength, pseudoLength, distanceAlong, xB1, yB1, tB1, rB1, directCurvature1);
                spiral->evaluateProjectedCoordinate (distanceAlong, xB, yB, tB, rB, directCurvature);
                xy.push_back (DPoint3d::From (
                    select == 0 ? distanceAlong : xB, yB + yShift));
                }
            GEOMAPI_PRINTF(" final x,y,t,r %20.15le %20.15le   %le %le  r1 %le\n", xB, yB, tB, rB, directCurvature < s_smallCurvature ? 0.0 : 1.0 / directCurvature);
            auto last = xy.back ();
            auto xyFlag = xy.back () + DVec3d::FromXYAngleAndMagnitude (select * Angle::DegreesToRadians (15), flagHeight);
            xy.push_back (xyFlag);
            Check::SaveTransformed (xy);
    
            Check::SaveTransformed (DSegment3d::From (0,yShift,0, last.x, yShift,0));
#ifdef saveArc
        auto arc = DEllipse3d::From (
                0, yShift + averageRadius, 0,
                0, -averageRadius, 0,
                averageRadius, 0, 0,
                0.0, averageCircleRadians);
        Check::SaveTransformed (arc);
#endif
            yShift += 10.0 * s_uorsPerMeter;
            delete spiral;
            }
        }
    Check::ClearGeometry ("CzechSpiral.NewtonStep");
    Check::SetTransform (transform0);
//    printf ("  (l/le %#.17g) (x/xe %#.17g)\n", l/le, x/xe);
#ifdef NSW
    double swx, swxe;
    newton_raphson_nswsc (ke, l, le, swx, swxe);
    printf ("NSW  (l/le %#.17g) (x/xe %#.17g)\n", l/le, swx/swxe);
#endif
    }

#ifdef CompileOldNSWTest
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PseudoSpiral,NewSouthWales)
    {
    static double s_uorsPerMeter = 1.0; //10000.0;
    static double s_smallCurvature = 1.0e-14;
    double flagHeight = 2.0 * s_uorsPerMeter;
    // simulate uor computations:
    //    convert metric sizes to uors
    //    use the Check:: transform to scale back to meters for output to dgnjs file
    //    do all origin shifts within this method (rather than via Check::Shift)
    auto transform0 = Check::GetTransform ();
    auto transform1 = Transform::From (RotMatrix::FromScale (1.0 / s_uorsPerMeter), DPoint3d::From (0,0,0));
    Check::SetTransform (transform1);
    double spiralLength = 100 * s_uorsPerMeter;
    double yShift = 0.0;
    bvector<double> radiusMeters {1000, 400, 200 };
    //bvector<double> pseudoLengthMeters {100.02506262460328, 100.15873011778872, 100.66666663992606};
    //bvector<double> edgeCount {15, 25, 35};
    for (int select : {2})
        {
        yShift = 0.0;
        for (size_t i = 0; i < radiusMeters.size (); i++)
            {
            double radius1 = radiusMeters[i] * s_uorsPerMeter;
            //double pseudoLength = pseudoLengthMeters[i] * s_uorsPerMeter;
            auto frame = Transform::From (0, yShift, 0);
            ICurvePrimitivePtr spiral = nullptr;
            spiral = ICurvePrimitive::CreateSpiralBearingCurvatureLengthCurvature (
                DSpiral2dBase::TransitionType_WesternAustralian,
                0.0, 0.0, spiralLength, 1.0 / radius1,
                frame,
                0.0, 1.0
                );
            auto bcurve = spiral->GetProxyBsplineCurvePtr ();
            auto bprim = ICurvePrimitive::CreateBsplineCurve (bcurve);
            //Check::SaveTransformed (*spiral);
            Check::SaveTransformed (*bprim);
            yShift += 10.0 * s_uorsPerMeter;
            }
        }
    Check::ClearGeometry ("PseudoSpiral.NewSouthWales");
    }
#endif
void TestAngleConstructors (double radians)
    {
    Angle angle0 = Angle::FromRadians (radians);
    Check::Near (radians, angle0.Radians (), "Angle ctor and readback");
    Check::Near (cos(radians), angle0.Cos (), "Angle::Cos");
    Check::Near (sin(radians), angle0.Sin (), "Angle::Sin");
    Check::Near (angle0, Angle::FromDegrees (angle0.Degrees ()));
    Check::Near (angle0, Angle::FromAtan2 (angle0.Sin (), angle0.Cos ()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(AngleCR,CTORS)
    {
    TestAngleConstructors (0.0);
    TestAngleConstructors (0.4);
    }

void AddPoints (bvector <DPoint3d> &points, int i0, int i1, double dx, int j0, int j1, double dy, int k0, int k1, double dz)
    {
    for (int i = i0; i <= i1; i++)
        {
        for (int j = j0; j <= j1; j++)
            {
            for (int k = k0; k <= k1; k++)
                {
                double x = i * dx;
                double y = j * dy;
                double z = k * dz;
                points.push_back (DPoint3d::From (x,y,z));
                }
            }
        }
    }

static void AddPoints (bvector<DPoint3d> &points, int i0, int i1, double dx)
    {
    AddPoints (points, i0, i1, dx, i0, i1, dx, i0, i1, dx);
    }

static bool isSimple (AngleCR angle)
    {
    return angle.Degrees () >= 0.0 && angle.Degrees () < 90.0;
    }

static void testYPR (AngleCR yaw, AngleCR pitch, AngleCR roll, bool checkBoth)
    {
    YawPitchRollAngles yprA (yaw, pitch, roll), yprB;

    RotMatrix matrixA = yprA.ToRotMatrix ();
    if (Check::True (YawPitchRollAngles::TryFromRotMatrix (yprB, matrixA), "first inversion"))
        {
        RotMatrix matrixB = yprB.ToRotMatrix ();

        Check::Near (matrixA, matrixB, "ypr matrix round trip");

        if (isSimple(yaw) && isSimple (roll) && isSimple (pitch))
            {
            Angle yawAngleA = Angle::FromDegrees (yprA.GetYaw().Degrees());
            Angle rollAngleA = Angle::FromDegrees (yprA.GetRoll().Degrees());
            Angle pitchAngleA = Angle::FromDegrees (yprA.GetPitch().Degrees());
            Angle yawAngleB = Angle::FromDegrees (yprB.GetYaw().Degrees());
            Angle rollAngleB = Angle::FromDegrees (yprB.GetRoll().Degrees());
            Angle pitchAngleB = Angle::FromDegrees (yprB.GetPitch().Degrees());

            Check::Near (yawAngleA, yawAngleB, "yaw");
            Check::Near (rollAngleA, rollAngleB, "roll");
            Check::Near (pitchAngleA, pitchAngleB, "pitch");
            }
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(YPRAngles, ShouldFail)
    {
    RotMatrix matrixA;
    matrixA.Zero ();
    YawPitchRollAngles yprB;
    Check::False(YawPitchRollAngles::TryFromRotMatrix(yprB, matrixA), "inversion of zero fails");
    double myNan= nan("1");
    auto angle1 = Angle::FromRadians (myNan);
    auto badCos1 = angle1.Cos ();
    Check::True (isnan (badCos1));
    auto angle2 = Angle::FromDegrees(myNan);
    auto badCos2 = angle2.Cos();
    Check::True(isnan(badCos2));

    Check::False(Angle::IsNearZeroAllowPeriodShift(myNan)); // previously caused infinite loop

    for (double sign : {1.0, -1.0})
        {
        for (double degrees1 : {0.2, 1.0, 10.0, 90.0, 365.0, 500.0, 710.0, 730.0, 1.0e10, 1.0e20, 1.0e40, 1.0e100, 1.0e200, myNan})
            {
            double degrees = degrees1 * sign;
            auto angleZ = AngleInDegrees::FromDegrees(degrees);
            auto badCosZ = angleZ.Cos();
            auto badSinZ = angleZ.Sin ();

            if (!Check::Near (1.0, badCosZ * badCosZ + badSinZ * badSinZ))
                printf("unit trig failure (%le  %le, %le)\n", degrees, badCosZ, badSinZ);
            }
        }
    RotMatrix badMatrix;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            badMatrix.form3d[i][j] = myNan;
    YawPitchRollAngles ypr;
    Check::False (YawPitchRollAngles::TryFromRotMatrix(ypr, badMatrix), "YPR from nan matrix");

    auto transform = Transform::FromIdentity ();
    YawPitchRollAngles ypr1;
    DPoint3d origin1;
    Check::True(YawPitchRollAngles::TryFromTransform(origin1, ypr1, transform), "YPR from transform with good matrix.");
    Check::False (ypr1.IsNan ());
    Check::False (origin1.IsNan ());

    Check::False (transform.IsNan (), "good transform");
    auto badOrigin = DPoint3d::From(myNan, myNan, myNan);
    Check::True(badOrigin.IsNan(), "bad point");
    transform.SetTranslation (badOrigin);
    Check::True(transform.IsNan(), "bad transform");
    DPoint3d origin;
    Check::False(YawPitchRollAngles::TryFromTransform(origin, ypr, transform), "YPR from transform with nan origin.");
    transform = Transform::From (badMatrix, DPoint3d::FromZero ());
    Check::True (transform.IsNan ());
    Check::False(YawPitchRollAngles::TryFromTransform(origin, ypr, transform), "YPR from transform with nan matrix.");

    Check::False (DPoint3d::From (1,2,3).IsNan (), "normal DPoint3d.IsNan ");
    Check::True(DPoint3d::From(myNan, 2, 3).IsNan(), "DPoint3d.IsNan x");
    Check::True(DPoint3d::From(1, myNan, 3).IsNan(), "DPoint3d.IsNan y");
    Check::True(DPoint3d::From(1, 2, myNan).IsNan(), "DPoint3d.IsNan z");

    Check::False(DPoint3d::From(1, 2).IsNan(), "normal DPoint2d.IsNan ");
    Check::True(DPoint3d::From(myNan, 2).IsNan(), "DPoint2d.IsNan x");
    Check::True(DPoint3d::From(1, myNan).IsNan(), "DPoint2d.IsNan y");
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(YPRAngles,RotMatrixConversions)
    {
    double aa = -110.0;
    double bb = -150.0;
    double cc = 0.0;
    bool runBoth = true;
    double a = 30.0;
    bvector<DPoint3d> degreePoints;
    degreePoints.push_back (DPoint3d::From (aa, bb, cc));
    AddPoints (degreePoints, -2, 2, 0.1);
    AddPoints (degreePoints, -1, 1, 3.0 * a);
    AddPoints (degreePoints, -6, 6, a);
    // 182 is bad for old code.
    for (size_t i = 0; i < degreePoints.size (); i++)
      testYPR (
            Angle::FromDegrees (degreePoints[i].x),
            Angle::FromDegrees (degreePoints[i].y),
            Angle::FromDegrees (degreePoints[i].z),
            runBoth
            );
    }

void VerifyRotationSense (RotMatrix matrix, Angle expectedAngle, int idPerp, double expectedSign)
    {
    RotMatrix identity = RotMatrix::FromIdentity ();
    int id0 = Angle::Cyclic3dAxis (idPerp + 1);
    int id1 = Angle::Cyclic3dAxis (id0 + 1);
    DVec3d target0 = DVec3d::FromColumn (matrix, id0);
    DVec3d target1 = DVec3d::FromColumn (matrix, id1);

    DVec3d start0  = DVec3d::FromColumn (identity, id0);
    DVec3d start1  = DVec3d::FromColumn (identity , id1);
    DVec3d perp    = DVec3d::FromColumn (identity, idPerp);

    double radians0 = expectedSign * start0.SignedAngleTo (target0, perp);
    double radians1 = expectedSign * start1.SignedAngleTo (target1, perp);

    Check::Perpendicular (target0, perp, "target0 perp");
    Check::Perpendicular (target1, perp, "target1 perp");

    Check::Near (radians0, expectedAngle.Radians (), "id0 rotation");
    Check::Near (radians1, expectedAngle.Radians (), "id1 rotation");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(YPR,AngleSense)
    {
    Angle theta = Angle::FromDegrees (10.0);
    Angle zero = Angle::FromDegrees (0.0);
    YawPitchRollAngles rollOnly (zero, zero, theta);
    YawPitchRollAngles pitchOnly (zero, theta, zero);
    YawPitchRollAngles yawOnly (theta, zero, zero);

    RotMatrix rollMatrix  = rollOnly.ToRotMatrix ();
    RotMatrix pitchMatrix = pitchOnly.ToRotMatrix ();
    RotMatrix yawMatrix   = yawOnly.ToRotMatrix ();

    VerifyRotationSense (rollMatrix, theta, 0, 1.0);
    VerifyRotationSense (pitchMatrix, theta, 1, -1.0);
    VerifyRotationSense (yawMatrix, theta, 2, 1.0);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(YPR,NegativeYZ)
    {
    RotMatrix matrix = RotMatrix::FromRowValues
          (
          1,0,0,
          0,-1,0,
          0,0,-1);
    YawPitchRollAngles ypr;
    YawPitchRollAngles::TryFromRotMatrix (ypr, matrix);
    RotMatrix matrixB = ypr.ToRotMatrix ();
    Check::Near (matrix, matrixB);
    double rollA = ypr.GetRoll ().Radians ();
    double rollB = Angle::Pi ();
    double rollC = PI;
    Check::True (rollA == rollB);
    Check::True (rollC == rollB);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(YPR,AngleReflections)
    {
    bvector<double> referenceAngle;
    referenceAngle.push_back (0.0);
    referenceAngle.push_back (Angle::PiOver2 ());
    referenceAngle.push_back (Angle::Pi());

    referenceAngle.push_back (-Angle::PiOver2 ());
    referenceAngle.push_back (-Angle::Pi());

    referenceAngle.push_back (0.25 * Angle::Pi ());
    referenceAngle.push_back (0.75 * Angle::Pi ());

    referenceAngle.push_back (-0.25 * Angle::Pi ());
    referenceAngle.push_back (-0.75 * Angle::Pi ());

    referenceAngle.push_back ( -1000.0);

    bvector<double> shiftAngle;
    shiftAngle.push_back (0.0);
    double e = 1.0e-8;
    for (size_t i = 0; i < 8; i++, e /= 10.0)
        {
        shiftAngle.push_back (e);
        shiftAngle.push_back (-e);
        }

    // The .Sin () and .Cos() methods on Angle do tests for near-multiples of 90 and do range reduction.
    // This is supposed to correct really small errors . .. make sure its deviations really are small . . .
    double maxError = 0.0;
    for (double ref : referenceAngle)
        {
        for (double delta : shiftAngle)
            {
            double q = ref + delta;
            Angle theta = Angle::FromRadians (q);
            double s0 = sin(theta.Radians());
            double s1 = theta.Sin();

            double c0 = cos(theta.Radians());
            double c1 = theta.Cos();

            double ds = fabs (s0 - s1);
            double dc = fabs (c0 - c1);
            maxError  = DoubleOps::Max (ds, maxError);
            maxError  = DoubleOps::Max (dc, maxError);
            }
        }
    static double s_trigTolerance = 8.0e-16;
    Check::True (maxError < s_trigTolerance);

    double q90 = atan2 (1.0, 0.0);
    double c0 = cos(Angle::PiOver2 ());
    double c1 = cos (q90);
    double q180 = atan2 (0.0, -1.0);
    double s0 = sin(Angle::Pi ());
    double s1 = sin (q180);
    Check::Near (c1, c0);
    Check::Near (s1,s0);
    }

struct DoubleGenerator
{
double m_a, m_b;
uint64_t m_xmin, m_xmax;
UsageSums m_fractionSums;
UsageSums m_angleSums;
#define NUM_BIT_CHECK 5
uint64_t m_bits[NUM_BIT_CHECK];
DoubleGenerator (double a = 0.0, double b = 1.0)
  : m_a (a), m_b (b), m_fractionSums(), m_angleSums ()
  {
  m_xmin = SIZE_MAX;
  m_xmax = 0;
  for (int i = 0; i < NUM_BIT_CHECK; i++)
      m_bits[i] = 0;
  }

double Next ()
    {
    // we know that rand () only gives 15 bit values ....
    uint64_t u0 = rand ();
    uint64_t u1 = rand ();
    uint64_t u2 = rand ();
    u1 = u1 << 15;
    u2 = u2 << 30;
    m_bits[0] |= u0;
    m_bits[1] |= u1;
    m_bits[2] |= u2;
    uint64_t u  = u0 + u1 + u2;
    m_bits[3] |= 0;
    if (u < m_xmin)
        m_xmin = u;
    if (u > m_xmax)
        m_xmax = u;
    double f = (double) u / (((uint64_t)1) << 45);
    m_fractionSums.Accumulate (f);
    double theta = m_a + f * (m_b - m_a);
    m_angleSums.Accumulate (theta);
    return theta;
    }
};

struct RandomGeometryGenerator
{

DoubleGenerator m_angleSource;

double m_maxFixup;

RandomGeometryGenerator ()
    : m_angleSource (-Angle::Pi (), Angle::Pi ())
    {
    m_maxFixup = 0.0;
    }

RotMatrix RandomRotMatrix (bool renormalizeInitialMatrix)
    {
    double x, y, z;

    x = m_angleSource.Next ();
    y = m_angleSource.Next ();
    z = m_angleSource.Next ();
    RotMatrix matrix;
    matrix.InitFromPrincipleAxisRotations(RotMatrix::FromIdentity (), x, y, z);
    if (renormalizeInitialMatrix)
        {
        RotMatrix original = matrix;
        matrix.SquareAndNormalizeColumns (matrix, 0, 1);
        double fixup = matrix.MaxDiff (original);
        if (fixup > m_maxFixup)
            m_maxFixup = fixup;
        }
    return matrix;
    }
};

struct RadiansGenerator : DoubleGenerator
{
RadiansGenerator () :DoubleGenerator (
        -Angle::Pi (),
        Angle::Pi ()
        )
    {
    }

Angle NextAngle () {return Angle::FromRadians (Next ());}
};

void AppendCheckedAngle (bvector<AngleInDegrees> &angles, double q)
    {
    if (q > -180.0 && q <= 180.0)
        angles.push_back (AngleInDegrees::FromDegrees (q));
    }
// For 0<=u<= 1/2:
//   make an angle 0..45 (but with quintic adjust to cluster at 0 and 45)
//   Use as +- around all multiples of 45.
void LoadMirroredAngles (int numPerQuadrant, bvector<AngleInDegrees> &angles)
    {
    AppendCheckedAngle (angles, 0.0);
    AppendCheckedAngle (angles, 90.0);
    AppendCheckedAngle (angles, -90.0);
    AppendCheckedAngle (angles, 180.0);

    double du = 1.0 / numPerQuadrant;
    for (int i = 1; i < numPerQuadrant; i++)
        {
        double u = i * du;
        double v = 1.0 - u;
        // bezier quintic with control values (0,0,0,9090,90)
        //double q = (3.0 * v * u * u + u*u*u) * 90.0;
        double q = u * u * (10.0 * v * v * u + 5.0 * v * u * u + u * u * u) * 90;
        for (double refAngle = -180.0; refAngle < 180.0; refAngle += 90.0)
            {
            AppendCheckedAngle (angles, refAngle + q);
            }
        }
    }

void TestYPRRoundTrip_AllQuadrants (int numPerQuadrant)
    {
    size_t      failCountYPR = 0, failCountYPR2 = 0, failCountQuat = 0;
    double      maxAbsYPR = 0.0, maxAbsYPR2 = 0.0, maxAbsQuat = 0.0, maxProductFuzz = 0.0;
    double dQuat, dYPR, dYPR2;
    static double tolerance = 5.0e-15;
    RotMatrix identity = RotMatrix::FromIdentity ();
    RotMatrix product;
    RandomGeometryGenerator source;
    size_t numYPRBetter = 0;
    size_t numYPR2Better = 0;
    bvector<AngleInDegrees> testDegrees;
    LoadMirroredAngles (numPerQuadrant, testDegrees);
    double numTest = 0;
    for (AngleInDegrees yawDegrees : testDegrees)
    for (AngleInDegrees pitchDegrees : testDegrees)
    for (AngleInDegrees rollDegrees : testDegrees)
        {
        numTest++;
        RotMatrix           yprMatrix, ypr2Matrix, quatMatrix;
        DPoint4d            quat;
        YawPitchRollAngles  angles;
        YawPitchRollAngles  angles2;

        auto baseAngles = YawPitchRollAngles (yawDegrees, pitchDegrees, rollDegrees);
        RotMatrix baseMatrix = baseAngles.ToRotMatrix ();

        product.InitProductRotMatrixTransposeRotMatrix(baseMatrix, baseMatrix);
        double productFuzz = identity.MaxDiff (product);
        if (productFuzz > maxProductFuzz)
            maxProductFuzz = productFuzz;

        YawPitchRollAngles::TryFromRotMatrix(angles, baseMatrix);
        yprMatrix = angles.ToRotMatrix();

        angles2 = YawPitchRollAngles::FromDegrees (angles.GetYaw().Degrees(), angles.GetPitch().Degrees(), angles.GetRoll().Degrees());
        ypr2Matrix = angles2.ToRotMatrix();

        baseMatrix.GetQuaternion(quat, false);
        quatMatrix = RotMatrix::FromQuaternion(quat);

        if ((dYPR  = baseMatrix.MaxDiff(yprMatrix)) > maxAbsYPR)
            maxAbsYPR = dYPR;

        if ((dYPR2 = baseMatrix.MaxDiff(ypr2Matrix)) > maxAbsYPR2)
            maxAbsYPR2 = dYPR2;

        if ((dQuat = baseMatrix.MaxDiff(quatMatrix)) > maxAbsQuat)
            maxAbsQuat = dQuat;

        if (dYPR2 < dYPR)
            numYPR2Better++;
        else
            numYPRBetter++;

        if (!baseMatrix.IsEqual(yprMatrix, tolerance))
            failCountYPR++;

        if (!baseMatrix.IsEqual(ypr2Matrix, tolerance))
            failCountYPR2++;

        if (!baseMatrix.IsEqual(quatMatrix, tolerance))
            failCountQuat++;

        }

    printf (">>> YPR and Quat %g roundtrips. \n", numTest);
    printf (">>> Failed YPR: %" PRIu64 " (%.4lg) YPR2: %" PRIu64 " (%.4lg) Quat: %" PRIu64 " (%.4lg)\n", (uint64_t)failCountYPR, maxAbsYPR, (uint64_t)failCountYPR2, maxAbsYPR2, (uint64_t)failCountQuat, maxAbsQuat);
    printf (" (maxProductFuzz %.4lg)\n", maxProductFuzz);
    printf(" Better matrix drift (Radians %d) (Degrees %d)\n", (int)numYPRBetter, (int)numYPR2Better);
    }


// cycle through trig function round trips. Return the drift.
double TestTrigRoundTrips (double radians0, size_t n,
      UsageSums &stableDeltas,
      UsageSums &stableCounts,
      UsageSums &failureDeltas,
      bvector<DPoint3d> &hardTrig
      )
    {
    static double s_trigRoundTripTol = 1.0e-16;
    static size_t s_hardTrigTrigger = 20;
    static size_t s_numCall = 0;
    static size_t s_numDelta = 0;
    double radians = radians0;
    s_numCall++;
    for (size_t i = 0; i < n; i++)
        {
        double c = cos (radians);
        double s = sin(radians);
        double radians2 = radians;
        radians = atan2 (s, c);
        if (fabs(radians - radians2) < s_trigRoundTripTol)
            {
            double d = fabs (radians - radians0);
            stableDeltas.Accumulate (d);
            stableCounts.Accumulate (log ((double)(1 + i)));
            if (i > s_hardTrigTrigger)
                hardTrig.push_back (DPoint3d::From
                      (radians0, d, (double)i));
            return d;
            }
        }

    s_numDelta++;
    double d = fabs (radians - radians0);
    failureDeltas.Accumulate (d);
    hardTrig.push_back (DPoint3d::From
          (radians0, d, (double)n));
    return d;
    }

bool TestYPRRoundTrips (YawPitchRollAngles angle0, size_t numTest,
        double &angleDrift, double &matrixDrift, size_t &numTestBeforeStable)
    {
    RotMatrix matrix0, matrix;
    YawPitchRollAngles angle1, angle = angle0;
    matrix0 = matrix = angle.ToRotMatrix ();
    // hm... there are always to YPR's with equivalent matrix.
    // find the one the class prefers as baseline ...
    numTestBeforeStable = 0;
    if (!YawPitchRollAngles::TryFromRotMatrix(angle1, matrix))
        return false;
    for (size_t i = 0; i < numTest; i++)
        {
        numTestBeforeStable++;
        if (!YawPitchRollAngles::TryFromRotMatrix(angle, matrix))
            return false;
        RotMatrix matrix2 = matrix;
        matrix = angle.ToRotMatrix ();
        if (matrix.MaxDiff (matrix2) == 0.0)
            break;
        }
    angleDrift = angle.MaxDiffRadians (angle1);
    matrixDrift = matrix.MaxDiff (matrix0);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(YPR,RoundTripRadiansAndDegrees)
{
TestYPRRoundTrip_AllQuadrants (15);
}

void PrintUsageSums
    (
    UsageSums &data,
    const char *title,
    bool fullPrecision
    )
    {
    if (fullPrecision)
        printf ("(%s (n %g)  (range %.16g %.16g) (avg %.16g) (sdv %.16g)\n",
              title, data.Count (),
              data.m_min,
              data.m_max,
              data.Mean (),
              data.StandardDeviation ()
              );
    else
        printf ("(%s (n %g)  (range %.4g %.4g) (avg %.4g) (sdv %.4g)\n",
              title, data.Count (),
              data.m_min,
              data.m_max,
              data.Mean (),
              data.StandardDeviation ()
              );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(YPR,RepeatRoundTrip)
{
RadiansGenerator angleSource;
static size_t numRotation = 10000;
static size_t numRoundTrip = 200;
static size_t numTrigRoundTrip = 2000;
double angleDrift, matrixDrift;

static double s_angleTol = 1.0e-14;
static double s_anglePrintTol = 6.0e-15;
static double s_matrixDriftWarningFraction = 0.1;
static double s_matrixTol = 8.0e-14;
UsageSums angleDiffs;
UsageSums matrixDiffs;
UsageSums trigDiffs;
UsageSums stableTrig, failedTrig, stableTrigLogCount;
bvector<DPoint3d> hardTrigList;
GEOMAPI_PRINTF ("(numRotation %g) (numRoundTrip %g)\n", (double)numRotation, (double)numRoundTrip);
uint32_t numSuspect = 0;
for (size_t i = 0; i < numRotation; i++)
    {

    YawPitchRollAngles ypr (angleSource.NextAngle (),
                  angleSource.NextAngle (),
                  angleSource.NextAngle ()
          );


    double e0 = TestTrigRoundTrips (ypr.GetYaw ().Radians (), numTrigRoundTrip, stableTrig, stableTrigLogCount, failedTrig, hardTrigList);
    double e1 = TestTrigRoundTrips (ypr.GetPitch ().Radians (), numTrigRoundTrip, stableTrig, stableTrigLogCount, failedTrig, hardTrigList);
    double e2 = TestTrigRoundTrips (ypr.GetRoll ().Radians (), numTrigRoundTrip, stableTrig, stableTrigLogCount, failedTrig, hardTrigList);
    trigDiffs.Accumulate (e0);
    trigDiffs.Accumulate (e1);
    trigDiffs.Accumulate (e2);
    size_t numBeforeFailure = 0;
    TestYPRRoundTrips (ypr, numRoundTrip, angleDrift, matrixDrift, numBeforeFailure);
    if (angleDrift > s_anglePrintTol
        || matrixDrift > s_matrixTol * s_matrixDriftWarningFraction )
        {
        GEOMAPI_PRINTF (" Drifting YPR: %.17g %.17g %.17g   (angle drift %.17g)  (matrix drift %.17g)\n",
                ypr.GetYaw ().Degrees (),
                ypr.GetPitch ().Degrees (),
                ypr.GetRoll ().Degrees (),
                angleDrift,
                matrixDrift
                );
        numSuspect ++;
        }
    Check::LessThanOrEqual (angleDrift, s_angleTol, "Angle drift in YPR roundtrip");
    Check::LessThanOrEqual (matrixDrift, s_matrixTol, "Matrix drift in YPR roundtrip");
    angleDiffs.Accumulate (angleDrift);
    matrixDiffs.Accumulate (matrixDrift);
    }
GEOMAPI_PRINTF (" (angleDrift  :max %.4lg :mean %.4lg :sdv %.4lg)\n",
        angleDiffs.m_max,
        angleDiffs.Mean (),
        angleDiffs.StandardDeviation ());
GEOMAPI_PRINTF (" (numSuspect %d\n", numSuspect);
GEOMAPI_PRINTF (" (matrixDrift :max %.4lg :mean %.4lg :sdv %.4lg)\n",
        matrixDiffs.m_max,
        matrixDiffs.Mean (),
        matrixDiffs.StandardDeviation ());
GEOMAPI_PRINTF (" (trigDrift :max %.4lg :mean %.4lg :sdv %.4lg)\n",
        trigDiffs.m_max,  
        trigDiffs.Mean (),
        trigDiffs.StandardDeviation ());
GEOMAPI_PRINTF (" (stableTrig :max %.4lg :mean %.4lg :sdv %.4lg :n %g)\n",
        stableTrig.m_max,  
        stableTrig.Mean (),
        stableTrig.StandardDeviation (),
        stableTrig.Count ()
        );
GEOMAPI_PRINTF (" (stableTrigCount :max %.4lg :mean %.4lg :sdv %.4lg :n %g)\n",
        stableTrigLogCount.m_max,  
        stableTrigLogCount.Mean (),
        stableTrigLogCount.StandardDeviation (),
        stableTrigLogCount.Count ()
        );
GEOMAPI_PRINTF (" (divergedTrig :max %.4lg :mean %.4lg :sdv %.4lg :n %g)\n",
        failedTrig.m_max,  
        failedTrig.Mean (),
        failedTrig.StandardDeviation (),
        failedTrig.Count ()
        );

    printf (" (numberHardTrigs %d\n", (int) hardTrigList.size ());
    for (DPoint3d xyz : hardTrigList)
        {
        printf ("(:n %.10g :radians %.16g :d %.4g)\n",
              xyz.z, xyz.x, xyz.y);
        }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(YPR,DriftSuspects)
    {
    bvector<YawPitchRollAngles> candidates
        {
        YawPitchRollAngles::FromDegrees (125.84912646501039, -28.059548194294162, -91.115460149719638),
        YawPitchRollAngles::FromDegrees (-133.02302268659332, -4.7723197644609154, 72.573836118745035),
        YawPitchRollAngles::FromDegrees (-88.007082666831593, -53.115407922932704, -93.134314064195678),
        YawPitchRollAngles::FromDegrees (-103.19095628007744, -175.28868466299173, -126.37517965389067),
        YawPitchRollAngles::FromDegrees (-133.54650112974241, -1.9423659932863306, 77.724764566984263),
        YawPitchRollAngles::FromDegrees (30,60,45),
        YawPitchRollAngles::FromDegrees (-30,60,45),
        YawPitchRollAngles::FromDegrees (30,-60,45),
        YawPitchRollAngles::FromDegrees (30,60,-45),
        YawPitchRollAngles::FromDegrees (-30,-60,45),
        YawPitchRollAngles::FromDegrees (-30,60,-45),
        YawPitchRollAngles::FromDegrees (30,-60,-45),
        YawPitchRollAngles::FromDegrees (-30,-60,-45),
        YawPitchRollAngles::FromDegrees (-3.546501, -1.9423, 7.724),
        YawPitchRollAngles::FromDegrees (3.1,6.1,4.2897),
        YawPitchRollAngles::FromDegrees (-3.1,6.1,4.2897),
        YawPitchRollAngles::FromDegrees (3.1,-6.1,4.2897),
        YawPitchRollAngles::FromDegrees (3.1,6.1,-4.2897),
        YawPitchRollAngles::FromDegrees (-3.1,-6.1,4.2897),
        YawPitchRollAngles::FromDegrees (-3.1,6.1,-4.2897),
        YawPitchRollAngles::FromDegrees (3.1,-6.1,-4.2897),
        YawPitchRollAngles::FromDegrees (-3.1,-6.1,-4.2897)
        };
    static size_t numRoundTrip = 200;
    for (auto ypr : candidates)
        {
        double angleDrift, matrixDrift;
        size_t numTestsRun;
        TestYPRRoundTrips (ypr, numRoundTrip, angleDrift, matrixDrift, numTestsRun);
        GEOMAPI_PRINTF (" Drifting YPR: %.17g %.17g %.17g   (angle drift %.17g) (matrix drift %.17g) (numTestsRun %d)\n",
                ypr.GetYaw ().Degrees (),
                ypr.GetPitch ().Degrees (),
                ypr.GetRoll ().Degrees (),
                angleDrift,
                matrixDrift,
                (int)numTestsRun
                );
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(YPR,DriftSuspectsSingleAxis)
    {
    static size_t numTheta = 180 * 257; // hit integer degrees exactly (within IEEE precision), and divide into a prime number of parts.
    static size_t numRoundTrip = 200;
    UsageSums num1, num2, num3;
    UsageSums angleDrift1, angleDrift2, angleDrift3;
    
    for (size_t i = 0; i <= numTheta; i++)
        {
        double f = i / (double)numTheta;
        double degrees = DoubleOps::Interpolate (-180.0, f, 180.0);
        auto ypr0 = YawPitchRollAngles::FromDegrees (degrees, 0, 0);
        auto ypr1 = YawPitchRollAngles::FromDegrees (0, degrees, 0);
        auto ypr2 = YawPitchRollAngles::FromDegrees (0, 0, degrees);
        double angleDrift, matrixDrift;
        size_t numTestsRun;
        TestYPRRoundTrips (ypr0, numRoundTrip, angleDrift, matrixDrift, numTestsRun);
        num1.Accumulate (numTestsRun);
        angleDrift1.Accumulate (angleDrift);
        TestYPRRoundTrips (ypr1, numRoundTrip, angleDrift, matrixDrift, numTestsRun);
        num2.Accumulate (numTestsRun);
        angleDrift2.Accumulate (angleDrift);
        TestYPRRoundTrips (ypr2, numRoundTrip, angleDrift, matrixDrift, numTestsRun);
        num3.Accumulate (numTestsRun);
        angleDrift3.Accumulate (angleDrift);
        }
    }
void TestYPROrder (double yawDegrees, double pitchDegrees, double rollDegrees)
    {
    RotMatrix Y = RotMatrix::FromVectorAndRotationAngle (DVec3d::From (0,0,1), Angle::DegreesToRadians (yawDegrees));
    RotMatrix P = RotMatrix::FromVectorAndRotationAngle (DVec3d::From (0,-1,0), Angle::DegreesToRadians (pitchDegrees));
    RotMatrix R = RotMatrix::FromVectorAndRotationAngle (DVec3d::From (1,0,0), Angle::DegreesToRadians (rollDegrees));
    RotMatrix YPR = Y * P * R;
    //RotMatrix RPY = R * P * Y;
    YawPitchRollAngles ypr = YawPitchRollAngles::FromDegrees (yawDegrees, pitchDegrees, rollDegrees);
    RotMatrix A = ypr.ToRotMatrix ();
    Check::Near (A, YPR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(YPROrder,Test0)
    {
    TestYPROrder (0,0,0);
    TestYPROrder (0,0,90);
    TestYPROrder (0,90,0);
    TestYPROrder (90,0,0);
    TestYPROrder (10,20,0);
    TestYPROrder (10,0,20);
    TestYPROrder (0,10,20);
    TestYPROrder (10,20,30);
    }
struct TrigRoundTripTests
{
// EasyTrig[i] counts (and averages) angles that converge in [i] steps.
bvector<UsageSums> easyTrig;
// count and average of all angles tested.
UsageSums startAngles;

// explicit list of angles that take more than easyTrig.size () iterations
bvector<DPoint3d>hardTrig;
// record in hard trig for more than 
size_t m_recordAsHard;
size_t m_maxIterations;

void Dump ()
    {
    PrintUsageSums (startAngles, "StartAngles", true);
    for (size_t i = 0; i < easyTrig.size (); i++)
        {
        if (easyTrig[i].Count () == 0)
            continue;
        char title[120];
        snprintf (title, sizeof(title), "(converge %d)", (int)i);
        PrintUsageSums (easyTrig[i], title, false);
        }
    printf (" (slow or non converged (n %d))\n",(int) hardTrig.size ());
    for (DPoint3d xyz : hardTrig)
        {
        printf ("(:n %4g :radians %.16g :d %.4g)\n",
              xyz.z, xyz.x, xyz.y);
        }
    }

TrigRoundTripTests
(
size_t maxIterations,
size_t numFixed,    // collect per-iteration-count stats up to this many
size_t recordAsHard  // if exceed this number of iterations, record the start angle.
)
  : m_maxIterations (maxIterations),
    m_recordAsHard (recordAsHard)
  {
  easyTrig.resize (numFixed);
  }

void RunTests (double radians0)
    {
    static double s_sinSwitch = 0.0;  // 0.125;
    static double s_trigRoundTripTol = 1.0e-16;
    double radians = radians0;
    startAngles.Accumulate (radians);
    BoolCounter positiveDelta;
    for (size_t i = 0; i < m_maxIterations; i++)
        {
        double c = cos (radians);
        double s = sin(radians);
        double radians2 = radians;
        if (fabs (s) < s_sinSwitch)
            radians = asin (s); // sqrt (c*c+s*s));
        else
            radians = atan2 (s, c);
        positiveDelta.Count (radians > radians2);
        if (fabs(radians - radians2) < s_trigRoundTripTol)
            {
            double d = fabs (radians - radians0);
            if (i < easyTrig.size ())
                easyTrig[i].Accumulate (d);
            if (i >= m_recordAsHard || i >= easyTrig.size ())
                hardTrig.push_back (DPoint3d::From
                      (radians0, d, (double)i));
            return;
            }
        }
    double d = fabs (radians - radians0);
    hardTrig.push_back (DPoint3d::From
          (radians0, d, (double)m_maxIterations));
    return;
    }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(YPR,StableTrigFunctions)
    {
    static size_t n = 1000000;
    TrigRoundTripTests tester (1000, 60, 60);
    double delta = Angle::Pi () / n;
    for (size_t i = 0; i < n; i++)
        {
        tester.RunTests (i * delta);
        }
    tester.Dump ();
    }

bool TestAngleInDegrees (double degrees)
    {
    AngleInDegrees qD = AngleInDegrees::FromDegrees (degrees);
    Angle qR = Angle::FromDegrees (degrees);
    double radians = Angle::DegreesToRadians (degrees);
    Check::StartScope ("TestAngleInDegrees", degrees);
    Check::PushTolerance (
            (fabs (degrees) < 45.0)
              ? ToleranceSelect_NearMachine
              : ToleranceSelect_Tight
            );
    Check::Near (qD.Degrees (), qR.Degrees (), "Degrees");
    Check::Near (qD.Radians (), qR.Radians (), "Radians");

    Check::Near (qD.Sin (), sin(radians), "Sin");
    Check::Near (qD.Cos (), cos(radians), "Cos");

    double c = qD.Cos ();
    double s = qD.Sin ();
    AngleInDegrees qA = AngleInDegrees::FromAtan2 (s, c);
    if (fabs (degrees) < 180.0)
        Check::Near (degrees, qA.Degrees (), "Atan2");
    else
        {
        // Compare sine and cosine -- angle may have wrapped . ..
        Check::Near (c, qA.Cos (), "Cos(Atan2))");
        Check::Near (s, qA.Sin (), "Sin(Atan2))");
        }

    Check::Near (qD.Cos (), qR.Cos (), "Cos");
    Check::Near (qD.Sin (), qR.Sin (), "Sin");
    Check::PopTolerance ();

    auto qD1 = AngleInDegrees::FromDegrees (degrees + 1);
    Check::True (qD < qD1, "<");
    Check::False (qD < qD, "<");
    Check::False (qD1 < qD, "<");

    Check::False (qD == qD1, "==");
    Check::True (qD == qD, "==");
    Check::False (qD1 == qD, "==");

    Check::True (qD <= qD1, "<=");
    Check::True (qD <= qD, "<=");
    Check::False (qD1 <= qD, "<=");

    Check::False (qD > qD1, ">");
    Check::False (qD > qD, ">");
    Check::True  (qD1 > qD, ">");

    Check::False (qD >= qD1, ">=");
    Check::True (qD >= qD, ">=");
    Check::True (qD1 >= qD, ">=");

    Check::True (qD != qD1, "!=");
    Check::False (qD != qD, "!=");
    Check::True (qD1 != qD, "!=");

    Check::True (qD.AlmostEqual (qD), "AlmostEqual");
    Check::False (qD.AlmostEqual (qD1), "AlmostEqual");
    static double s_smallDegreeRelFactor = 1.0e-15;
    Check::True (qD.AlmostEqual (AngleInDegrees::FromDegrees (degrees + s_smallDegreeRelFactor * 360.0)), "AlmostEqual");

    AngleInDegrees qDCopy = qD;
    AngleInDegrees qDCopyAngle = qR;
    Check::True (qDCopy == qD, "Copy");
    Check::Near (qD.Degrees (), qDCopyAngle.Degrees (), "Copy(Angle)");
    Check::EndScope ();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(AngleInDegrees,HelloWorld)
    {
    double shift = 5.0;
    Check::True (AngleInDegrees ().Degrees () == 0.0, "Default ctor");
    // Touch angles at and near all multiples of 45 degrees for 6 full round trips in both directions.
    // Remark: trig values show problems (to the "tight" 1e-14 level) above 7300 degrees.
    static double s_largeDegrees = 2100.0;
    for (double baseDegrees = 0.0; baseDegrees < s_largeDegrees; baseDegrees += 45.0)
        {
        TestAngleInDegrees (baseDegrees);
        TestAngleInDegrees (baseDegrees + shift);
        TestAngleInDegrees (baseDegrees - shift);
        if (baseDegrees != 0.0)
            {
            TestAngleInDegrees (-baseDegrees);
            TestAngleInDegrees (-baseDegrees + shift);
            TestAngleInDegrees (-baseDegrees - shift);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(OBM,PolygonIndexing)
    {
    // verify LEAP code triangle indexing for 5-point polygon:
    
    int npts = 5;
    int nOffset = 0;
    GEOMAPI_PRINTF(" TriangleIndexing, !hasVoid\n");
	for (int i=1; i<npts-1; i++)
		GEOMAPI_PRINTF("  %d %d %d\n",  nOffset + 0, nOffset + i, nOffset + i+1);
		
    GEOMAPI_PRINTF(" TriangleIndexing, hasVoid true\n");
    for (int i=0; i<npts-2; i++)
		GEOMAPI_PRINTF("  %d %d %d\n",  nOffset + i+1, nOffset + i+2, nOffset + i);
    
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(stdvector, binarySearchBehavior)
{
    bool doPrint = Check::PrintDeepStructs ();
  std::vector<double> v;
  v.push_back (0);
  v.push_back (10);
  v.push_back (20);
  v.push_back (30);
  v.push_back (40);
  
  std::vector<double>::iterator low,up;
    if (doPrint)
        {
        for  (double d : v)
        printf (" %g", d);
        printf ("\n");
        printf ("Search in begin..end\n");
        }
  for (double i = -5; i <= 45; i += 5)
    {
    low=std::lower_bound (v.begin(), v.end(), i); //          ^
    up= std::upper_bound (v.begin(), v.end(), i); //                   ^
    if (doPrint)
        printf (" %g %d %d \n", i, (int)(low- v.begin()), (int)(up - v.begin ()));
    }

  printf ("Search in begin..end-1\n");
  for (double searchValue = -5; searchValue <= 45; searchValue += 5)
    {
    low=std::lower_bound (v.begin(), v.end() - 1, searchValue);
    up= std::upper_bound (v.begin(), v.end() - 1, searchValue);
    if (doPrint)
        printf (" searchValue %g   lower_bound %d   (upper_bound %d)\n",
                searchValue,
                (int)(low - v.begin ()),
                (int)(low - v.begin ())
                );
    auto limitA = up - 1;
    auto limitB = up;
    if (searchValue < v.front ())
        {
        Check::Ptrdiff (0, up - v.begin (), "left data resolves to left interval");
        if (doPrint)
            printf (" left extrapolation %g [%g,%g]\n",
              searchValue, *up, *(up + 1));

        }
    else if (searchValue >= v.back ())
        {
        Check::Ptrdiff (2, v.end () - limitA, "right data resolves to right interval");
        if (doPrint)
            printf (" right extrapolation %g [%g,%g]\n",
              *limitA, *limitB, searchValue);
        }
    else
        {
        Check::True (searchValue >= *limitA, "interior resolve A");
        Check::True (searchValue < *limitB, "interior resolve B");
        if (doPrint)
            printf (" bounds of resolved interval [%g,%g,%g]\n",
              *limitA, searchValue, *limitB);
        }
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(bvector, binarySearchBehavior)
{
    bool doPrint = Check::PrintDeepStructs ();
    bvector<double> v;
    v.push_back (0);
    v.push_back (10);
    v.push_back (20);
    v.push_back (30);
    v.push_back (40);

    bvector<double>::iterator low,up;
    if (doPrint)
        {
        for  (double d : v)
            printf (" %g", d);
        printf ("\n");
        printf ("Search in begin..end\n");
        }
  for (double i = -5; i <= 45; i += 5)
    {
    low=std::lower_bound (v.begin(), v.end(), i); //          ^
    up= std::upper_bound (v.begin(), v.end(), i); //                   ^
    if (doPrint)
        printf (" %g %d %d \n", i, (int)(low- v.begin()), (int)(up - v.begin ()));
    }

  if (doPrint)
        printf ("Search in begin..end-1\n");
  for (double searchValue = -5; searchValue <= 45; searchValue += 5)
    {
    low=std::lower_bound (v.begin(), v.end() - 1, searchValue);
    up= std::upper_bound (v.begin(), v.end() - 1, searchValue);
    if (doPrint)
        printf (" searchValue %g   lower_bound %d   (upper_bound %d)\n",
                searchValue,
                (int)(low - v.begin ()),
                (int)(low - v.begin ())
                );
    auto limitA = up - 1;
    auto limitB = up;
    if (searchValue < v.front ())
        {
        Check::Ptrdiff (0, up - v.begin (), "left data resolves to left interval");
        if (doPrint)
            printf (" left extrapolation %g [%g,%g]\n",
              searchValue, *up, *(up + 1));

        }
    else if (searchValue >= v.back ())
        {
        Check::Ptrdiff (2, v.end () - limitA, "right data resolves to right interval");
        if (doPrint)
            printf (" right extrapolation %g [%g,%g]\n",
              *limitA, *limitB, searchValue);
        }
    else
        {
        Check::True (searchValue >= *limitA, "interior resolve A");
        Check::True (searchValue < *limitB, "interior resolve B");
        if (doPrint)
            printf (" bounds of resolved interval [%g,%g,%g]\n",
              *limitA, searchValue, *limitB);
        }
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Angle,OverlappableIntervals_CrudeCounts)
    {
    bvector<DSegment1d> intervalA, intervalB;
    bvector<double> sweeps  {90, 180, 290, -80, -190};
    bvector<double> starts  {0, 90, 152, -31, 401};
    for (double sweepA : sweeps)
        {
        for (double sweepB : sweeps)
            {
            for (double startA : sweeps)
                {
                for (double startB : sweeps)
                    {
                    Angle::OverlapWrapableIntervals (
                                Angle::DegreesToRadians (startA),
                                Angle::DegreesToRadians (sweepA),
                                Angle::DegreesToRadians (startB),
                                Angle::DegreesToRadians (sweepB),
                                intervalA, intervalB
                                );
                    Check::Size (intervalA.size (), intervalB.size ());
                    Check::True (intervalA.size () <= 2);
                    if (fabs (sweepA) + fabs (sweepB) < 360.0)
                        Check::True (intervalA.size () <= 1);
                    else
                        Check::True (intervalA.size () >= 1);
                    }
                }
            }
        }
    } 

bool CheckDSegment (DSegment1dCR segment, double a0, double a1)
    {
    return Check::Near (a0, segment.GetStart ())
        && Check::Near (a1, segment.GetEnd ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Angle,OverlappableIntervalsFixedFractions)
    {
    bvector<DSegment1d> intervalA, intervalB;
    bvector<double> sweeps  {90, 180, 290, -80, -190};
    bvector<double> starts  {0, 90, 152, -31, 401};
    bvector<double> fractions {0, 0.25, 0.50, 0.75, 1.0};
    size_t numTest = 0;
    for (double startA : sweeps)
        {
        for (double sweepA : sweeps)
            {
            for (double fB0 : fractions)
                {
                for (double fB1 : fractions)
                    {
                    if (fB0 != fB1)
                        {
                        // Simple interior, single intersection.
                        double startB = startA + fB0 * sweepA;
                        double sweepB = (fB1-fB0) * sweepA;
                        Angle::OverlapWrapableIntervals (
                                    Angle::DegreesToRadians (startA),
                                    Angle::DegreesToRadians (sweepA),
                                    Angle::DegreesToRadians (startB),
                                    Angle::DegreesToRadians (sweepB),
                                    intervalA, intervalB
                                    );
                        if (Check::Size (1, intervalA.size ()))
                            {
                            if (fB1 > fB0)
                                CheckDSegment (intervalA[0], fB0, fB1);
                            else
                                CheckDSegment (intervalA[0], fB1, fB0);
                            if (sweepA * sweepB > 0.0)
                                CheckDSegment (intervalB[0], 0.0, 1.0);
                            else
                                CheckDSegment (intervalB[0], 1.0, 0.0);
                            }
                        }

                    if (fB1 > fB0
                        && fB0 > 0.0 && fB1 < 1.0)
                        {
                        // Forward wrap with nonzero intersection at each end.
                        double startB = startA + fB1 * sweepA;
                        double sweepB01 = (fB1-fB0) * sweepA;
                        double sweepB =
                            sweepB01 > 0
                                ? 360 - sweepB01 
                                : -360 - sweepB01;
                        Angle::OverlapWrapableIntervals (
                                    Angle::DegreesToRadians (startA),
                                    Angle::DegreesToRadians (sweepA),
                                    Angle::DegreesToRadians (startB),
                                    Angle::DegreesToRadians (sweepB),
                                    intervalA, intervalB
                                    );
                        if (Check::Size (2, intervalA.size ()))
                            {
                            double endB = startA + fB0 * sweepA;// but that's in primary period
                            CheckDSegment (intervalA[0], fB1, 1.0);
                            CheckDSegment (intervalA[1], 0.0, fB0);
                            CheckDSegment (intervalB[0], 0.0,
                                        (startA + sweepA - startB) / sweepB);
                            CheckDSegment (intervalB[1],
                                        1.0 - (endB- startA) / sweepB,
                                        1.0);
                            }
                        }
                    numTest++;
                    }
                }
            }
        }
    } 



double FastIntegerPower (double a, uint32_t n)
    {
    double q = a;
    double product = (n & 0x01) ? a : 1.0;
    n = n >> 1;
    while (n > 0)
        {
        q = q * q;
        if (n & 0x01)
            product *= q;
        n = n >> 1;
        }
    return product;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FastIntegerPower,Test0)
    {
    for (double f : bvector<double>{1.0, 2.0, 23492.2423423, 0.007687612, 1000.0, 10.0, 1000000.0})
             //LOG.infov("\n value of f :  %f \n", f);
        {
        UsageSums sums;
        for (double a0 : bvector<double> {1.0, 2.0, 5.0, 3.2, 0.2342878, 187.345, 1.0/3.0, 1.0/12.0, Angle::Pi ()})
            {
            double a = f * a0;
            for (uint32_t i = 0; i < 15; i++)
                {
                double p = pow(a, i);
                double q = FastIntegerPower(a, i);
                double e = fabs((p - q) / p);
                sums.Accumulate(e);
                if (e > 1.0e-15)
                     {
                     LOG.infov("(a %.17g) (i %d) (p %.17g) (q %.17g) (e %.2g)\n", a, i, p, q, e);
                     }
                }
              }
             LOG.infov(" Factor %f Max power Error %.17g\n", f, sums.Max());
        Check::LessThanOrEqual (sums.Max (), 1.0e-14);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Angle, Operators)
    {
    auto alpha = Angle::FromDegrees (10);
    auto beta = Angle::FromDegrees (20);
    auto sum = alpha + beta;
    auto diff = alpha - beta;
    auto neg = -alpha;
    double f = 2.3;
    auto fa = f * alpha;
    auto af = alpha * f;
    Check::Near (sum.Radians (), alpha.Radians () + beta.Radians ());
    Check::Near (diff.Radians (), alpha.Radians () - beta.Radians ());
    Check::Near (neg.Radians (), -alpha.Radians ());
    Check::Near (fa.Radians (), f * alpha.Radians ());
    Check::Near (af.Radians (), alpha.Radians ()* f);
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(Angle, AcosAsin)
    {
    Angle angle1 = Angle::FromDegrees(60);
    Check::Near(angle1.Radians(), Angle::Acos(angle1.Cos()));
    angle1 = Angle::FromDegrees(15.8);
    Check::Near(angle1.Radians(), Angle::Acos(angle1.Cos()));
    Angle angle2 = Angle::FromDegrees(90);
    Check::Near(angle2.Radians(), Angle::Asin(angle2.Sin()));
    angle2 = Angle::FromDegrees(32.5);
    Check::Near(angle2.Radians(), Angle::Asin(angle2.Sin()));
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(Angle, PeriodShift)
    {
    double periodShift = Angle::PeriodShift(Angle::DegreesToRadians(90), 2);
    Check::Near(Angle::DegreesToRadians(90) + Angle::TwoPi() * 2, periodShift);
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(Angle, ForwardComplement)
    {
    double complement = Angle::ForwardComplement(Angle::DegreesToRadians(90));
    Check::Near(Angle::DegreesToRadians(360) - Angle::DegreesToRadians(90), complement);
    complement = Angle::ForwardComplement(Angle::DegreesToRadians(370));
    Check::Near(Angle::DegreesToRadians(360) - Angle::DegreesToRadians(10), complement);
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(Angle, TrigCombination)
    {
    double constCoff = 2;
    double cosCoff = 2;
    double sinCoff = 3;
    auto cosAngle = Angle::FromDegrees(60);
    auto sinAngle = Angle::FromDegrees(60);
    double expectedTrig = constCoff + cosCoff * cosAngle.Cos() + sinCoff * sinAngle.Sin();
    double resTrig = Angle::EvaluateTrigCombination(constCoff, cosCoff, sinCoff, Angle::DegreesToRadians(60));
    Check::Near(expectedTrig, resTrig);
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(Angle, SmallAngleDegrees)
    {
    AngleInDegrees smallAngle = AngleInDegrees::SmallAngleInDegrees();
    Check::Near(Angle::RadiansToDegrees(1.0e-12), smallAngle.Degrees());
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(Angle, IsNearZero)
    {
    // static method ..
    Check::True(Angle::IsNearZero(0.1e-12));
    Check::True(Angle::IsNearZero(1.0e-12));
    Check::False(Angle::IsNearZero(0.1e-12 + PI));
    Check::False(Angle::IsNearZero(1.1e-12));
    // instance method
    Check::True(Angle::FromRadians(0.1e-12).IsNearZero());
    Check::True(Angle::FromRadians(1.0e-12).IsNearZero());
    Check::False(Angle::FromRadians(0.1e-12 + PI).IsNearZero());
    Check::False(Angle::FromRadians(1.1e-12).IsNearZero());
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(Angle, YawPitchRoll)
    {
    YawPitchRollAngles anglesYPR = YawPitchRollAngles::FromDegrees(20, 10, -40);
    Check::Near(Angle::DegreesToRadians(40), anglesYPR.MaxAbsRadians());
    Check::False(anglesYPR.IsIdentity());
    anglesYPR = YawPitchRollAngles::FromRadians(1.0e-12, 1.0e-13, 0.1e-12);
    Check::True(anglesYPR.IsIdentity());
    }
static bvector<uint32_t> windowsResult
{
  0,   3,   5,   8,  10,  13,  15,  18,  20,  23, 
 26,  28,  31,  33,  36,  38,  41,  43,  46,  48, 
 51,  54,  56,  59,  61,  64,  66,  69,  71,  74, 
 77,  79,  82,  84,  87,  89,  92,  94,  97,  99, 
102, 105, 107, 110, 112, 115, 117, 120, 122, 125, 
128, 130, 133, 135, 138, 140, 143, 145, 148, 150, 
153, 156, 158, 161, 163, 166, 168, 171, 173, 176, 
179, 181, 184, 186, 189, 191, 194, 196, 199, 201, 
204, 207, 209, 212, 214, 217, 219, 222, 224, 227, 
230, 232, 235, 237, 240, 242, 245, 247, 250, 252, 
255
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(HSV,RoundOff)
    {
#define MAXFACTOR 100
    bvector<uint32_t> computed;
    static double s_roundingEpsilon = 6.0e-14;  // At 256, the granularity above is 5.6e-14, below is 2.8e-14.
    double roundingValue = 0.5 + s_roundingEpsilon;
    for (uint32_t value = 0; value <= MAXFACTOR; value++)
        {
        computed.push_back ((int)( (roundingValue + 255.0 * value / MAXFACTOR)));
#ifdef TestStdRound
        int computed1 = (int)std::round (255.0 * value / MAXFACTOR);
        EXPECT_EQ (computed1, (int)computed.back ()) << "hsv std::round versus (int)";
#endif
        }
#ifdef PRINT_HSV_PERCENTAGE_TABLE
    for (size_t i = 0; i < computed.size (); i++)
        {
        printf("%3d, ", computed[i]);
        if (((i+1) % 10) == 0)
            printf ("\n");
        }
#endif
    for (size_t i = 0; i <= MAXFACTOR; i++)
        {
        char s[200];
        snprintf (s, sizeof(s), "HSV Rounding (i %d) (windows %d) (computed %d)", (int)i, (int)windowsResult[i], (int)computed[i]);
        EXPECT_EQ ((int)windowsResult[i], computed[i]) << s;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(HSV,PrintFloatingPointGranularity)
    {
    for (int i = 1; i <= 256; i *= 2)
        {
        double d = (double)i;
        printf (" (i %d) (before %8.2e)  (after %8.2e)\n",
                    i,
                    d - BeNumerical::BeNextafter (d, -DBL_MAX),
                    BeNumerical::BeNextafter (d, DBL_MAX) - d
                    );
        }
    }
// This tests if the current CPU settings and atan2 function treat atan2(-0.0,-1) differently from atan2 (+0,-1)
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Angle,Atan2Zero)
    {
    double thetaPlus = atan2(0.0, -1.0);
    double thetaMinus = atan2(-0.0, -1.0);
    double betaPlus = Angle::Atan2 (0.0, -1.0);
    double betaMinus = Angle::Atan2 (-0.0, -1.0);
    printf(" atan2(+0,-1) %.17lg\n", thetaPlus);
    printf(" atan2(-0,-1) %.17lg\n", thetaMinus);
    printf(" Angle::Atan2(+0,-1) %.17lg\n", betaPlus);
    printf(" Angle::Atan2(-0,-1) %.17lg\n", betaMinus);
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PowerFunction,Hello)
    {
    double f = sqrt (10.0);
    double aMax = 1.0e4;
    uint32_t maxPower = 6;
    for (double a = 1.0; a < aMax; a *= f)
        {
        double b = a;
        double maxDelta = 0;
        for (uint32_t p = 1; p <= maxPower; p++, b *= a)
            {
            double c = pow (a, p);
            maxDelta = DoubleOps::MaxAbs (maxDelta, (c - b) / b);
            Check::Near (b, pow (a, p), "repeated multiplication a^p = pow (a,p)");
            }
        Check::True (maxDelta < 1.0e-13, "Power function versus inline multiplication");
        printf (" a= %15.2g maxDeltarelative  %.2le\n", a, maxDelta);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PowerFunction,Series)
    {
    double L = 120.0;   // typical spiral length
    double R = 400.0;   // typical track radius
    double alpha = 1.0 / (40.0 * R * R * L *L);
    double dMax = 0.0;
    for (double d = 0.0; d <= L; d += 10.0)
        {
        double d2 = d * d;
        double d4 = d2 * d2;
        double f0 = d * (1.0 - alpha * d4);
        double f1 = d - alpha * pow (d, 5);
        double delta = fabs (f1 - f0);
        dMax = DoubleOps::Max (delta, dMax);
        }
    Check::True (dMax < 1.0e-13, "Power function versus inline multiplication");
    }
#include <Geom/internal/capi/trigfuncs_capi.h>
#ifdef TestBsiTrig
TEST(bsiTrig, AngleInSweep)
    {
    double theta;
    for (double startRadians : { 0.0, 2.0, -1.4})
        {
        for (double sweepRadians : {-0.3, 5.9, 0.5})
            {
            theta = startRadians + 0.4 * sweepRadians;
            Check::True (bsiTrig_angleInSweep (theta, startRadians, sweepRadians));
            theta = startRadians + 1.03 * sweepRadians;
            Check::False(bsiTrig_angleInSweep(theta, startRadians, sweepRadians));
            theta = startRadians - 0.03 * sweepRadians;
            Check::False(bsiTrig_angleInSweep(theta, startRadians, sweepRadians));
            }
        }
    }
TEST(bsiTrig, Spherical)
    {
    DPoint3d xyzA = DPoint3d::From (0.3, 0.8, 0.9);
    DPoint3d uvwB;
    DPoint3d xyzC;
    bsiVector_cartesianToSpherical (&uvwB, &xyzA);
    bsiVector_sphericalToCartesian (&xyzC, &uvwB);
    Check::Near (xyzA, xyzC, "cartesian to spherical to cartesian round trip");
    }
#endif
