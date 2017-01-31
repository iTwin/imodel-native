#include "testHarness.h"

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


TEST(Printf,Pound)
    {
    for (double a = 1.0 / 64.0; a < 1000.0; a *= 2.0)
        printf (" (.17g %.17g) (#.17g %#.17g)\n", a, a);
    }



#define N2( x ) ((x)*(x))               // pow( ) is a performance issue
#define N3( x ) ((x)*(x)*(x))           // pow( ) is a performance issue
static int newton_raphson_czechsc
(
double ke,  // end curvature.  (implied straight at start)
double lz,  // distance along curve
double le,  // distance at end
double& xz, // computed x at this distance
double& xe  // comptued x at end
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
    double gamma1;
    double rbar = 2.0 / ke;
    //double re = 1.0 / ke;
    xe = le;
    xesav = 0;
    while (fabs(xe - xesav) > 0.000001)
        {
        xesav = xe;
        thet = xe * ke / 2.;
        lamdax = asin ( thet );         // anglular coordinate of (xe,0) in the rbar circle
        lamda = 1. / cos( lamdax );     // secant (lambdax).   Could rewrite as lamd^2 = rbar^2/ (rbar^2-xe^2)
        t3 = lamda * lamda * N2(ke) / 40.;// sec^2 / (40 ke)= sec^2 / (10 rbar^2)=1/ (10 (rbar cos)^2
        gamma0 = t3;
        gamma1 = 1.0 / (10.0 * (rbar * rbar - xe * xe));
        xe = le - ( gamma0 * N3(xe) );
        }
    thet = xe * ke / 2.;
    lamdax = asin ( thet );
    lamda = 1. / cos( lamdax );
    t3 = lamda * lamda * N2(ke) / 40;;
    gamma0 = t3 / ( N2(xe) );
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



/*-------------------------------------------------------------------------+
|                                                                          |
|   spu - 27 aug 2009                                                      |
|                                                                          |
+-------------------------------------------------------------------------*/
static int newton_raphson_nswsc
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


TEST(CzechSpiral,NewtonStep)
    {
    double re = 1000;
    double le = 100;
    double l  = 0.35 * le;
    double ke = 1.0 / re;
    double x, xe;
    newton_raphson_czechsc (ke, l, le, x, xe);
    printf ("  (l/le %#.17g) (x/xe %#.17g)\n", l/le, x/xe);
    double swx, swxe;
    newton_raphson_nswsc (ke, l, le, swx, swxe);
    printf ("  (l/le %#.17g) (x/xe %#.17g)\n", l/le, swx/swxe);


    }

void TestAngleConstructors (double radians)
    {
    Angle angle0 = Angle::FromRadians (radians);
    Check::Near (radians, angle0.Radians (), "Angle ctor and readback");
    Check::Near (cos(radians), angle0.Cos (), "Angle::Cos");
    Check::Near (sin(radians), angle0.Sin (), "Angle::Sin");
    Check::Near (angle0, Angle::FromDegrees (angle0.Degrees ()));
    Check::Near (angle0, Angle::FromAtan2 (angle0.Sin (), angle0.Cos ()));
    }

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
    size_t      count = 0, failCountYPR = 0, failCountYPR2 = 0, failCountQuat = 0;
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

        } while (++count < 1000000);
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
        double &angleDrift, double &matrixDrift)
    {
    RotMatrix matrix0, matrix;
    YawPitchRollAngles angle1, angle = angle0;
    matrix0 = matrix = angle.ToRotMatrix ();
    // hm... there are always to YPR's with equivalent matrix.
    // find the one the class prefers as baseline ...
    if (!YawPitchRollAngles::TryFromRotMatrix(angle1, matrix))
        return false;
    for (size_t i = 0; i < numTest; i++)
        {
        if (!YawPitchRollAngles::TryFromRotMatrix(angle, matrix))
            return false;
        matrix = angle.ToRotMatrix ();
        }
    angleDrift = angle.MaxDiffRadians (angle1);
    matrixDrift = matrix.MaxDiff (matrix0);
    return true;
    }

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

TEST(YPR,RepeatRoundTrip)
{
RadiansGenerator angleSource;
static size_t numRotation = 10000;
static size_t numRoundTrip = 200;
static size_t numTrigRoundTrip = 2000;
double angleDrift, matrixDrift;

static double s_angleTol = 1.0e-14;
static double s_anglePrintTol = 6.0e-15;
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

    TestYPRRoundTrips (ypr, numRoundTrip, angleDrift, matrixDrift);
    if (angleDrift > s_anglePrintTol
        || matrixDrift > s_matrixTol * 0.01 )
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
    Check::True (angleDrift < s_angleTol, "Angle drift in YPR roundtrip");
    Check::True (matrixDrift < s_matrixTol, "Matrix drift in YPR roundtrip");
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

    {
    auto ypr = YawPitchRollAngles::FromDegrees (
            125.84912646501039,
            -28.059548194294162,
            -91.115460149719638
            );


    TestYPRRoundTrips (ypr, numRoundTrip, angleDrift, matrixDrift);
    if (angleDrift > s_angleTol
            || matrixDrift > s_matrixTol
            )
        {
        GEOMAPI_PRINTF (" Drifting YPR: %.17g %.17g %.17g   (drift %.17g)\n",
                ypr.GetYaw ().Degrees (),
                ypr.GetPitch ().Degrees (),
                ypr.GetRoll ().Degrees (),
                angleDrift
                );
        }
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
        sprintf (title, "(converge %d)", (int)i);
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

TEST(FastIntegerPower,Test0)
    {
    char message[2048];
    for (double f : bvector<double>{1.0, 2.0, 23492.2423423, 0.007687612, 1000.0, 10.0, 1000000.0})
        {
        UsageSums sums;
        for (double a0 : bvector<double> {1.0, 2.0, 5.0, 3.2, 0.2342878, 187.345, 1.0/3.0, 1.0/12.0, Angle::Pi ()})
            {
            double a = f * a0;
            for (uint32_t i = 0; i < 15; i++)
                {
                double p = pow (a,i);
                double q = FastIntegerPower (a, i);
                double e = fabs ((p-q)/p);
                sums.Accumulate (e);
                sprintf (message, "FastIntegerPower (a0 %.17lg) (power %d) (p q e %.17lg %.17lg %.17lg)",
                            a0, i, p, q, e);
                Check::Near (p, q, message);
                }
            }
        sprintf (message, "(f %.17lg) FastIntegerPowerMaxError %lg", f, sums.Max ());
        Check::LessThanOrEqual (sums.Max (), 9.0e-16, message);
        }
    }

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
// @bsimethod                                     Farhad.Kabir                    12/16
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
// @bsimethod                                     Farhad.Kabir                    12/16
//---------------------------------------------------------------------------------------
TEST(Angle, PeriodShift)
    {
    double_t periodShift = Angle::PeriodShift(Angle::DegreesToRadians(90), 2);
    Check::Near(Angle::DegreesToRadians(90) + Angle::TwoPi() * 2, periodShift);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    12/16
//---------------------------------------------------------------------------------------
TEST(Angle, ForwardComplement)
    {
    double_t complement = Angle::ForwardComplement(Angle::DegreesToRadians(90));
    Check::Near(Angle::DegreesToRadians(360) - Angle::DegreesToRadians(90), complement);
    complement = Angle::ForwardComplement(Angle::DegreesToRadians(370));
    Check::Near(Angle::DegreesToRadians(360) - Angle::DegreesToRadians(10), complement);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    12/16
//---------------------------------------------------------------------------------------
TEST(Angle, TrigCombination)
    {
    double_t constCoff = 2;
    double_t cosCoff = 2;
    double_t sinCoff = 3;
    auto cosAngle = Angle::FromDegrees(60);
    auto sinAngle = Angle::FromDegrees(60);
    double_t expectedTrig = constCoff + cosCoff * cosAngle.Cos() + sinCoff * sinAngle.Sin();
    double_t resTrig = Angle::EvaluateTrigCombination(constCoff, cosCoff, sinCoff, Angle::DegreesToRadians(60));
    Check::Near(expectedTrig, resTrig);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    12/16
//---------------------------------------------------------------------------------------
TEST(Angle, SmallAngleDegrees)
    {
    AngleInDegrees smallAngle = AngleInDegrees::SmallAngleInDegrees();
    Check::Near(Angle::RadiansToDegrees(1.0e-12), smallAngle.Degrees());
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    12/16
//---------------------------------------------------------------------------------------
TEST(Angle, IsNearZero)
    {
    Check::True(Angle::IsNearZero(0.1e-12));
    Check::True(Angle::IsNearZero(1.0e-12));
    Check::False(Angle::IsNearZero(0.1e-12 + PI));
    Check::False(Angle::IsNearZero(1.1e-12));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    12/16
//---------------------------------------------------------------------------------------
TEST(Angle, YawPitchRoll)
    {
    YawPitchRollAngles anglesYPR = YawPitchRollAngles::FromDegrees(20, 10, -40);
    Check::Near(Angle::DegreesToRadians(40), anglesYPR.MaxAbsRadians());
    Check::False(anglesYPR.IsIdentity());
    anglesYPR = YawPitchRollAngles::FromRadians(1.0e-12, 1.0e-13, 0.1e-12);
    Check::True(anglesYPR.IsIdentity());
    }