/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Geom/Angle.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//! The Angle class has static methods for typical angle computations.
struct GEOMDLLIMPEXP Angle
{
friend struct YawPitchRollAngles;
friend GEOMDLLIMPEXP Angle operator *(Angle const &, double);
friend GEOMDLLIMPEXP Angle operator *(double, Angle const &);
friend GEOMDLLIMPEXP Angle operator -(Angle const &);
friend GEOMDLLIMPEXP Angle operator +(Angle const &, Angle const &);
friend GEOMDLLIMPEXP Angle operator -(Angle const &, Angle const &);

private: double m_radians;
// the constructor is PRIVATE so that callers MUST say FromDegrees or FromRadians and cannot accidentally get degrees/radians mixed up.
private: Angle (double radians);
//! copy constructor
public: Angle (AngleCR source);
public:
//! Strongly typed "constructor" (static method)
static Angle FromDegrees (double degrees);
public:
//! Strongly typed "constructor" (static method)
static Angle FromRadians (double radians);
public:
//! Strongly typed "constructor" (static method)
static Angle FromAtan2 (double sine, double cosine);

public:
//! return the sine of the angle.
double Sin () const;

public:
//! return the cosine of the angle.
double Cos () const;

public:
    //! return the tangent of the angle.
    double Tan () const;


public:
//! Zero angle constructor
Angle ();


public:
//! Return the angle in radians
double Radians () const;
public:
//! Return the angle in degrees
double Degrees () const;

//! Return true if {fabs(radians)} is within {Angle:SmallAngle} of 2PI.
static bool IsFullCircle (double radians);

//! Test if {radians} is {SmallAngle} or smaller.
static bool IsNearZero (double radians);
//! Test if {radians} is {SmallAngle} or smaller.
static bool IsNearZeroAllowPeriodShift (double radians);

//! Test if two angles are within {SmallAngle} (NOT allowing 2pi shift!!)
static bool NearlyEqual (double radiansA, double radiansB);
//! Test if two angles are within {SmallAngle}, allowing 2pi shift.
static bool NearlyEqualAllowPeriodShift (double radiansA, double radiansB); 

//! Return array of (signed oriented) start-end fraction pairs for overlapping radian sweeps.
static void OverlapWrapableIntervals (double startRadiansA, double sweepA, double startRadiansB, double sweepRadiansB,
            bvector<DSegment1d> &startEndFractionA, bvector<DSegment1d> &startEndFractionB);
//! Small angle used in toleranced angle comparisons.
static double SmallAngle ();

//! About 10 times unit roundoff . . .
static double TinyAngle ();


//! Medium angle used in toleranced angle comparisons.
static double MediumAngle ();

//! constant {PI}
static double Pi ();
//! constant {2*PI}
static double TwoPi ();
//! consant {PI/2}
static double PiOver2 ();

//! Convert degrees to radians
static double DegreesToRadians (double degrees);
//! Convert radians to degrees
static double RadiansToDegrees (double radians);

//! Convert radians to degrees
static double CircleFractionToRadians (double fraction);


//! Shift {theta} so it is within one period of {thetaStart} in the direction of {sweep}. Return angle in radians
static double AdjustToSweep (double theta, double thetaStart, double sweep);

//! Shift {theta} so it is within one period of {thetaStart} in the direction of {sweep}.  Return FRACTIONAL position.
static double NormalizeToSweep (double theta, double thetaStart, double sweep);

//! Shift {theta} so it is within one period of {thetaStart} in the direction of {sweep}.  Convert to FRACTIONAL position.
//!   Adjust the fractional position be conditions for extension:
//!<ul>
//!<li>(false,false) -- No extension.  Pull to nearest endpoint fraction, i.e. 0 or 1
//!<li>(false,true) -- extend forward only.  All "outside" angles become fractions above 1.
//!<li>(true,false) -- extend backward only.  All "outside" angles become negative fractions.
//!<li>(true,true) -- both extensions possible. "outside" angles closer to {thetaStart+sweep} become fractions
//! above 1, otherwise negative fractions.
//!</ul>
//! @param [in] theta angle to fractionalize
//! @param [in] thetaStart beginning of active interval.
//! @param [in] sweep signed extend of active interval.
//! @param [in] extend0 true if negative normalizations are allowed.
//! @param [in] extend1 true if extension above one is allowed.
static double NormalizeToSweep (double theta, double thetaStart, double sweep, bool extend0, bool extend1);

//! Test if angle is in sweep with no tolerance or period shift
static bool InExactSweep (double theta, double thetaStart, double sweep);

//! Test if theta or any shift by multiple of 2pi is in sweep.
static bool InSweepAllowPeriodShift (double theta, double thetaStart, double sweep);

//! Add a multiple of 2PI to theta...
static double PeriodShift (double theta, double periods);

//! Angle which sweeps in the other direction to the same end angle (modulo 2pi) as the given sweep.
static double ReverseComplement (double radians);

//! Angle which sweeps in the same direction to return to sum of 2pi   Examples
//! <ul>
//! <li> ForwardComplement  of {pi/2}  {3pi/2}
//! <li> ForwardComplement  of {-pi/2}  {-3pi/2}
//! </ul>
static double ForwardComplement (double radians);

//! Given trig functions (cosine and sine) of some (double) angle 2A, find trig functions for the angle A.
static void HalfAngleFuctions (double &cosA, double &sinA, double rCos2A, double rSin2A);

//! Find a vector that differs from (x0,y0) by a multiple of 90 degrees,
//! x1 is positive, and y1 is as small as possible in absolute value, i.e. points to the right.
static void Rotate90UntilSmallAngle (double &x1, double &y1, double x0, double y0);

//! Find the min and max values of {f(theta) = constCoff + cosCoff * cos(theta) + sinCoff * sin(theta)} in [thetaA, thetaB].
//! @param [in] constCoff constant coefficient
//! @param [in] cosCoff cosine coefficient
//! @param [in] sinCoff sine coefficient
//! @param [in] thetaA angle range limit
//! @param [in] sweep sweep angle
//! @param [in] thetaMin angle where min occurs
//! @param [in] fMin minimum value
//! @param [in] thetaMax angle where max occurs
//! @param [in] fMax maximum value
static void TrigCombinationRangeInSweep
        (
        double constCoff, double cosCoff, double sinCoff,
        double thetaA, double sweep,
        double &thetaMin, double &fMin,
        double &thetaMax, double &fMax
        );

//! Find the min and max values of {f(theta) = constCoff + cosCoff * cos(theta) + sinCoff * sin(theta)} in [-pi,pi]
//! @param [in] constCoff constant coefficient
//! @param [in] cosCoff cosine coefficient
//! @param [in] sinCoff sine coefficient
//! @param [in] thetaMin angle where min occurs
//! @param [in] fMin minimum value
//! @param [in] thetaMax angle where max occurs
//! @param [in] fMax maximum value
static void TrigCombinationRange
        (
        double constCoff, double cosCoff, double sinCoff,
        double &thetaMin, double &fMin,
        double &thetaMax, double &fMax
        );

//! Evaluate {f(theta) = constCoff + cosCoff * cos(theta) + sinCoff * sin(theta)}
//! @param [in] constCoff constant coefficient
//! @param [in] cosCoff cosine coefficient
//! @param [in] sinCoff sine coefficient
//! @param [in] theta evaluation angle
static double EvaluateTrigCombination (double constCoff, double cosCoff, double sinCoff, double theta);
//! Return the arctan of numerator/denominator, in full -PI to PI range.   0,0 returns 0.
static double Atan2 (double numerator, double denominator);
//! Return acos of arg, but cap arg at +- 1
static double Acos (double arg);
//! Return asin of arg, but cap arg at +- 1
static double Asin (double arg);



//! @return i adjusted to [0,1] with wraparound.
static int Cyclic2dAxis (int i);
//! @return i adjusted to [0,1,2] with wraparound.
static int Cyclic3dAxis (int i);
//! @param [out] i0 i adjusted to [0,1,2] with wraparound
//! @param [out] i1 i+1 adjusted to [0,1,2] with wraparound
//! @param [out] i2 i+2 adjusted to [0,1,2] with wraparound
//! @param [in] i initial axis
static void Cyclic3dAxes (int &i0, int &i1, int &i2, int i);

//! @param [out] aOut {(a,b) DOT (cos,sin)}
//! @param [out] bOut {(cross,sin) DOT (a,b)}
//! @param [in] a x coordiante
//! @param [in] b y coordinate
//! @param [in] cos cosine term of Givens rotation.
//! @param [in] sin sine term of Givens rotation.
static void ApplyGivensWeights
(
double &aOut,
double &bOut,
double a,
double b,
double cos,
double sin
);

//! Construct cosine and sine of vector to (a,b).
//! (Just normalize a and b.)
//! @param [out] cosine
//! @param [out]  sine
//! @param [in] a
//! @param [in] b
static void ConstructGivensWeights
(
double      &cosine,
double      &sine,
double      a,
double      b
);

//! Return integrals of [cc cs c; cs ss s; c s 1] from theta0 to theta1
//! @param [in] theta0 beginning of integration interval
//! @param [in] theta1 end of integration interval
//! @param [out] integrals symmetric matrix of integrals.
static void TrigIntegrals (double theta0, double theta1, RotMatrixR integrals);
};


//! Angle object.  The angle is carried in degrees.
//! This is appropriate for angles being read from human-supplied sources where
//! exact integer values like 90,180 etc are expected.
//! The Cos() and Sin() methods use addition/subtraction of 90, 180 as necessary
//! before introducing PI
 struct GEOMDLLIMPEXP AngleInDegrees
{
private:
double m_degrees;
// Private constructor with degrees as input.  public users must specfiy FromDegrees or FromRadians
// via the static methods.
AngleInDegrees (double degrees);

public:
//! Constructor -- angle == 0
AngleInDegrees ();
//! Copy constructor
AngleInDegrees (AngleInDegrees const &other);
//! Public constructor from the Angle object (which carries radians)
AngleInDegrees (Angle const &other);

//! The degrees form of the system small angle (for radians, Angle::SmallAngle ())
static AngleInDegrees SmallAngleInDegrees ();

//! exact equality test
bool operator == (AngleInDegrees const &other) const;
//! strict sorting comparison
bool operator < (AngleInDegrees const &other) const;
//! strict sorting comparison
bool operator <= (AngleInDegrees const &other) const;
//! strict sorting comparison
bool operator > (AngleInDegrees const &other) const;
//! strict sorting comparison
bool operator >= (AngleInDegrees const &other) const;
//! exact inequality
bool operator != (AngleInDegrees const &other) const;

//! Add two angles
AngleInDegrees operator+ (AngleInDegrees const &other) const {return AngleInDegrees (m_degrees + other.m_degrees);}

//! approximate equality test
bool AlmostEqual (AngleInDegrees const &other);
//! Construct angle in degrees from input in radians
static AngleInDegrees FromRadians (double radians);
//! Construct angle in degrees from input in degrees.
static AngleInDegrees FromDegrees (double degrees);

//! Return the angle in degrees as simple double
double Degrees ()const ;
//! Return the angle in radians as a simple double.
double Radians () const;
//! Return the cosine of the angle
double Cos () const;
//! Return the sine of the angle
double Sin () const;
//! Construct from xy vector components (with y first as usual for atan2)
static AngleInDegrees FromAtan2 (double y, double x);
};

//! Compact (3 angles in degrees) description of rigid body rotation.
//! This is "popular" near the user interface level, but "must" be converted to/and from Transform for "real" operations.
//!<ul>
//!<li>YPR with mutliple non-zero angles is infuriatingly difficult to describe intuitively.
//!<li>The coordinate system orientation has X forward, Y to the left, and Z up.
//!<li>If only one is nonzero, the meanings are:
//!  <ul>
//!  <li>YAW is rotation around Z (X towards Y) (turn left)
//!  <li>PITCH is rotation around negative Y (X towards Z)  (nose up)
//!  <li>ROLL is rotation around X (Y towards Z)    (tip right)
//!  </ul>
//!<li> The composite RotMatrix is YAW * PITCH * ROLL
//!<li>A physical sequence for this is:
//!   <ul>
//!   <li>Place data in a local coordinate system with x forward, y left, and z up.
//!   <li>Work throught Y*P*R sequence from the right, applying all rotations around the global axes.
//!   <li>apply roll around the positive global X axis.
//!   <li>apply pitch around the negative global Y axis.
//!   <li>apply yaw around the positive global Z axis.
//!   </ul>
//!</ul>
 struct GEOMDLLIMPEXP YawPitchRollAngles
{
private:
AngleInDegrees m_yaw, m_pitch, m_roll;
public:
//! minimal constructor -- zero angles.
YawPitchRollAngles ();

//! constructor from all angles
YawPitchRollAngles (AngleInDegrees Yaw, AngleInDegrees pitch, AngleInDegrees roll);
//! constructor from all angles in degrees ..
YawPitchRollAngles (Angle yaw, Angle pitch, Angle roll);

AngleInDegrees GetYaw   () const;
AngleInDegrees GetPitch () const;
AngleInDegrees GetRoll  () const;

//! Add an offset to the Yaw angle
void AddYaw(AngleInDegrees const& a2) {m_yaw = m_yaw + a2;}

void SetYaw (AngleInDegrees a);
void SetPitch(AngleInDegrees a);
void SetRoll (AngleInDegrees a);


//! constructor from all angles in radians
static YawPitchRollAngles FromRadians (double yawRadians, double pitchRadians, double rollRadians);

//! constructor from all angles in degrees
static YawPitchRollAngles FromDegrees (double yawDegrees, double pitchDegrees, double rollDegrees);

//! Try to extract angles from a RotMatrix
static bool TryFromRotMatrix (YawPitchRollAnglesR angles, RotMatrixCR matrix);
//! Convert the angles to a RotMatrix
RotMatrix ToRotMatrix () const;
//! Convert the angles and an origin to a Transform.
Transform ToTransform (DPoint3dCR origin) const;

static bool TryFromTransform (DPoint3dR origin, YawPitchRollAnglesR angles, TransformCR transform);
//! test for near-zero angles.
bool IsIdentity () const;
//! return the maximum absolute radians among the angles.
double MaxAbsRadians () const;

//! return the maximum absolute difference among radians among the angles.
double MaxDiffRadians (YawPitchRollAngles const &other) const;

//! return the maximum absolute radians among the angles.
double MaxAbsDegrees () const;

//! return the maximum absolute difference among radians among the angles.
double MaxDiffDegrees (YawPitchRollAngles const &other) const;

};
END_BENTLEY_GEOMETRY_NAMESPACE
