/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Geom/Angle.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//! The Angle class has static methods for typical angle computations.
struct Angle
{
    friend struct YawPitchRollAngles;
    friend GEOMDLLIMPEXP Angle operator *(Angle const&, double);
    friend GEOMDLLIMPEXP Angle operator *(double, Angle const&);
    friend GEOMDLLIMPEXP Angle operator -(Angle const&);
    friend GEOMDLLIMPEXP Angle operator +(Angle const&, Angle const&);
    friend GEOMDLLIMPEXP Angle operator -(Angle const&, Angle const&);

private: 
    double m_radians;

    // this constructor is PRIVATE so that callers MUST say FromDegrees or FromRadians and cannot accidentally get degrees/radians mixed up.
    Angle(double radians){m_radians = radians;}

public: 
    bool operator ==(Angle const& other) const {return m_radians == other.m_radians;}
    bool operator <(Angle const& other) const {return m_radians < other.m_radians;}
    bool operator >(Angle const& other) const {return m_radians > other.m_radians;}

    //! copy constructor
    Angle(AngleCR other){m_radians = other.m_radians;}

    //! Strongly typed "constructor" (static method)
    static Angle FromDegrees(double degrees) {return Angle(DegreesToRadians(degrees));}

    //! Strongly typed "constructor" (static method)
    static Angle FromRadians(double radians) {return Angle(radians);}

    //! Strongly typed "constructor" (static method)
    static Angle FromAtan2(double sine, double cosine) {return Atan2(sine, cosine);}

    //! Strongly typed "constructor" (static method) for full circle angle
    static Angle FromFullCircle() {return Angle (msGeomConst_2pi);}

    //! return the sine of the angle.
    GEOMDLLIMPEXP double Sin() const;

    //! return the cosine of the angle.
    GEOMDLLIMPEXP double Cos() const;

    //! return the tangent of the angle.
    GEOMDLLIMPEXP double Tan() const;

    //! Zero angle constructor
    Angle() {m_radians = 0.0;}

    //! Return the angle in radians
    double Radians() const {return m_radians;}

    //! Return the angle in degrees
    double Degrees() const {return RadiansToDegrees(m_radians);}

    //! Return true if {fabs(radians)} is within {Angle:SmallAngle} of 2PI.
    GEOMDLLIMPEXP static bool IsFullCircle(double radians);

    //! Test if {radians} is {SmallAngle} or smaller.
    GEOMDLLIMPEXP static bool IsNearZero(double radians);

    //! test if instance is {SmallAngle} or smaller.
	GEOMDLLIMPEXP bool IsNearZero () const;

    //! Test if {radians} is {SmallAngle} or smaller, allowing 2pi shift.
    GEOMDLLIMPEXP static bool IsNearZeroAllowPeriodShift(double radians);

    //! Test if two angles are within {SmallAngle} (NOT allowing 2pi shift!!)
    GEOMDLLIMPEXP static bool NearlyEqual(double radiansA, double radiansB);

    //! Test if two angles are within {SmallAngle}, allowing 2pi shift.
    GEOMDLLIMPEXP static bool NearlyEqualAllowPeriodShift(double radiansA, double radiansB);

    //! Return array of (signed oriented) start-end fraction pairs for overlapping radian sweeps.
    GEOMDLLIMPEXP static void OverlapWrapableIntervals(double startRadiansA, double sweepA, double startRadiansB, double sweepRadiansB,
                bvector<DSegment1d> &startEndFractionA, bvector<DSegment1d> &startEndFractionB);

    //! Small angle used in toleranced angle comparisons.
    GEOMDLLIMPEXP static double SmallAngle();

    //! About 10 times unit roundoff . . .
    GEOMDLLIMPEXP static double TinyAngle();

    //! Medium angle used in toleranced angle comparisons.
    GEOMDLLIMPEXP static double MediumAngle();

    //! Small angle for use with floats . .
    GEOMDLLIMPEXP static double SmallFloatRadians();

    //! constant {PI}
    GEOMDLLIMPEXP static double Pi();
    static Angle AnglePi() {return Angle(Pi());}

    //! constant {2*PI}
    GEOMDLLIMPEXP static double TwoPi();
    static Angle AngleTwoPi() {return Angle(TwoPi());}

    //! consant {PI/2}
    GEOMDLLIMPEXP static double PiOver2();
    static Angle AnglePiOver2() {return Angle(PiOver2());}

    //! Convert degrees to radians
    GEOMDLLIMPEXP static double DegreesToRadians(double degrees);

    //! Convert radians to degrees
    GEOMDLLIMPEXP static double RadiansToDegrees(double radians);

    //! Convert radians to degrees
    GEOMDLLIMPEXP static double CircleFractionToRadians(double fraction);

    //! Shift {theta} so it is within one period of {thetaStart} in the direction of {sweep}. Return angle in radians
    GEOMDLLIMPEXP static double AdjustToSweep(double theta, double thetaStart, double sweep);

    //! Shift {theta} so it is within one period of {thetaStart} in the direction of {sweep}.  Return FRACTIONAL position.
    GEOMDLLIMPEXP static double NormalizeToSweep(double theta, double thetaStart, double sweep);

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
    GEOMDLLIMPEXP static double NormalizeToSweep(double theta, double thetaStart, double sweep, bool extend0, bool extend1);

    //! Test if angle is in sweep with no tolerance or period shift
    GEOMDLLIMPEXP static bool InExactSweep(double theta, double thetaStart, double sweep);

    //! Test if theta or any shift by multiple of 2pi is in sweep.
    GEOMDLLIMPEXP static bool InSweepAllowPeriodShift(double theta, double thetaStart, double sweep);

    //! Add a multiple of 2PI to theta...
    GEOMDLLIMPEXP static double PeriodShift(double theta, double periods);

    //! Angle which sweeps in the other direction to the same end angle (modulo 2pi) as the given sweep.
    GEOMDLLIMPEXP static double ReverseComplement(double radians);

    //! Angle which sweeps in the same direction to return to sum of 2pi   Examples
    //! <ul>
    //! <li> ForwardComplement  of {pi/2}  {3pi/2}
    //! <li> ForwardComplement  of {-pi/2}  {-3pi/2}
    //! </ul>
    GEOMDLLIMPEXP static double ForwardComplement(double radians);

    //! Given trig functions (cosine and sine) of some (double) angle 2A, find trig functions for the angle A.
    GEOMDLLIMPEXP static void HalfAngleFuctions(double &cosA, double &sinA, double rCos2A, double rSin2A);

    //! Find a vector that differs from (x0,y0) by a multiple of 90 degrees,
    //! x1 is positive, and y1 is as small as possible in absolute value, i.e. points to the right.
    GEOMDLLIMPEXP static void Rotate90UntilSmallAngle(double &x1, double &y1, double x0, double y0);

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
    GEOMDLLIMPEXP static void TrigCombinationRangeInSweep(double constCoff, double cosCoff, double sinCoff, double thetaA, double sweep, double &thetaMin, double &fMin, double &thetaMax, double &fMax);

    //! Find the min and max values of {f(theta) = constCoff + cosCoff * cos(theta) + sinCoff * sin(theta)} in [-pi,pi]
    //! @param [in] constCoff constant coefficient
    //! @param [in] cosCoff cosine coefficient
    //! @param [in] sinCoff sine coefficient
    //! @param [in] thetaMin angle where min occurs
    //! @param [in] fMin minimum value
    //! @param [in] thetaMax angle where max occurs
    //! @param [in] fMax maximum value
    GEOMDLLIMPEXP static void TrigCombinationRange(double constCoff, double cosCoff, double sinCoff, double &thetaMin, double &fMin, double &thetaMax, double &fMax);

    //! Evaluate {f(theta) = constCoff + cosCoff * cos(theta) + sinCoff * sin(theta)}
    //! @param [in] constCoff constant coefficient
    //! @param [in] cosCoff cosine coefficient
    //! @param [in] sinCoff sine coefficient
    //! @param [in] theta evaluation angle
    GEOMDLLIMPEXP static double EvaluateTrigCombination(double constCoff, double cosCoff, double sinCoff, double theta);

    //! Return the arctan of numerator/denominator, in full -PI to PI range.   0,0 returns 0.
    GEOMDLLIMPEXP static double Atan2(double numerator, double denominator);

    //! Return acos of arg, but cap arg at +- 1
    GEOMDLLIMPEXP static double Acos(double arg);

    //! Return asin of arg, but cap arg at +- 1
    GEOMDLLIMPEXP static double Asin(double arg);

    //! @return i adjusted to [0,1] with wraparound.
    GEOMDLLIMPEXP static int Cyclic2dAxis(int i);

    //! @return i adjusted to [0,1,2] with wraparound.
    GEOMDLLIMPEXP static int Cyclic3dAxis(int i);

    //! @param [out] i0 i adjusted to [0,1,2] with wraparound
    //! @param [out] i1 i+1 adjusted to [0,1,2] with wraparound
    //! @param [out] i2 i+2 adjusted to [0,1,2] with wraparound
    //! @param [in] i initial axis
    GEOMDLLIMPEXP static void Cyclic3dAxes(int &i0, int &i1, int &i2, int i);

    //! @param [out] aOut {(a,b) DOT (cos,sin)}
    //! @param [out] bOut {(cross,sin) DOT (a,b)}
    //! @param [in] a x coordiante
    //! @param [in] b y coordinate
    //! @param [in] cos cosine term of Givens rotation.
    //! @param [in] sin sine term of Givens rotation.
    GEOMDLLIMPEXP static void ApplyGivensWeights(double &aOut, double &bOut, double a, double b, double cos, double sin);

    //! Construct cosine and sine of vector to (a,b).
    //! (Just normalize a and b.)
    //! @param [out] cosine
    //! @param [out]  sine
    //! @param [in] a
    //! @param [in] b
    GEOMDLLIMPEXP static void ConstructGivensWeights(double &cosine, double &sine, double a, double b);

    //! Return integrals of [cc cs c; cs ss s; c s 1] from theta0 to theta1
    //! @param [in] theta0 beginning of integration interval
    //! @param [in] theta1 end of integration interval
    //! @param [out] integrals symmetric matrix of integrals.
    GEOMDLLIMPEXP static void TrigIntegrals(double theta0, double theta1, RotMatrixR integrals);
};

//! Angle object.  The angle is carried in degrees.
//! This is appropriate for angles being read from human-supplied sources where
//! exact integer values like 90,180 etc are expected.
//! The Cos() and Sin() methods use addition/subtraction of 90, 180 as necessary
//! before introducing PI
struct AngleInDegrees
{
private:
    double m_degrees;
    // Private constructor with degrees as input.  public users must specfiy FromDegrees or FromRadians
    // via the static methods.
    AngleInDegrees(double degrees) : m_degrees(degrees) {}

public:
    //! Constructor -- angle == 0
    AngleInDegrees() : m_degrees(0.0) {}

    //! Copy constructor
    AngleInDegrees(AngleInDegrees const& other) : m_degrees(other.m_degrees) {}

    //! Public constructor from the Angle object (which carries radians)
    AngleInDegrees(Angle const& other) : m_degrees(other.Degrees()) {}

    //! The degrees form of the system small angle (for radians, Angle::SmallAngle ())
    GEOMDLLIMPEXP static AngleInDegrees SmallAngleInDegrees();

    //! exact equality test
    bool operator ==(AngleInDegrees const& other) const {return m_degrees == other.m_degrees;}

    //! strict sorting comparison
    bool operator < (AngleInDegrees const& other) const {return m_degrees < other.m_degrees;}

    //! strict sorting comparison
    bool operator <= (AngleInDegrees const& other) const {return m_degrees <= other.m_degrees;}

    //! strict sorting comparison
    bool operator > (AngleInDegrees const& other) const {return m_degrees > other.m_degrees;}

    //! strict sorting comparison
    bool operator >= (AngleInDegrees const& other) const {return m_degrees >= other.m_degrees;}

    //! exact inequality
    bool operator != (AngleInDegrees const& other) const {return m_degrees != other.m_degrees;}

    //! Add two angles
    AngleInDegrees operator+ (AngleInDegrees const& other) const {return AngleInDegrees(m_degrees + other.m_degrees);}

    //! approximate equality test
    GEOMDLLIMPEXP bool AlmostEqual(AngleInDegrees const& other);

    //! Construct angle in degrees from input in radians
    static AngleInDegrees FromRadians(double radians) {return AngleInDegrees(Angle::RadiansToDegrees(radians));}

    //! Construct angle in degrees from input in degrees.
    static AngleInDegrees FromDegrees(double degrees) {return AngleInDegrees(degrees);}

    //! Return the angle in degrees as simple double
    double Degrees()const {return m_degrees;}

    //! Return the angle in radians as a simple double.
    double Radians() const {return Angle::DegreesToRadians(m_degrees);}

    //! Return the cosine of the angle
    GEOMDLLIMPEXP double Cos() const;

    //! Return the sine of the angle
    GEOMDLLIMPEXP double Sin() const;

    //! Construct from xy vector components (with y first as usual for atan2)
    GEOMDLLIMPEXP static AngleInDegrees FromAtan2(double y, double x);
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
struct YawPitchRollAngles
{
private:
    AngleInDegrees m_yaw, m_pitch, m_roll;

public:
    //! minimal constructor -- zero angles.
    YawPitchRollAngles() {m_roll = m_pitch = m_yaw = AngleInDegrees::FromDegrees(0.0);}

    //! constructor from all angles
    YawPitchRollAngles(AngleInDegrees yaw, AngleInDegrees pitch, AngleInDegrees roll) : m_yaw(yaw), m_pitch(pitch), m_roll(roll) {}

    //! constructor from all angles in degrees.
    YawPitchRollAngles(Angle yaw, Angle pitch, Angle roll) : m_yaw(yaw), m_pitch(pitch), m_roll(roll) {}

    AngleInDegrees GetYaw() const {return m_yaw;}
    AngleInDegrees GetPitch() const {return m_pitch;}
    AngleInDegrees GetRoll() const {return m_roll;}
    void SetYaw(AngleInDegrees angle) {m_yaw = angle;}
    void SetPitch(AngleInDegrees angle) {m_pitch = angle;}
    void SetRoll(AngleInDegrees angle) {m_roll = angle;}

    //! Add an offset to the Yaw angle
    void AddYaw(AngleInDegrees const& a2) {m_yaw = m_yaw + a2;}

    //! constructor from all angles in radians
    static YawPitchRollAngles FromRadians(double yawRadians, double pitchRadians, double rollRadians) 
        {return YawPitchRollAngles(AngleInDegrees::FromRadians(yawRadians), AngleInDegrees::FromRadians(pitchRadians), AngleInDegrees::FromRadians(rollRadians));}

    //! constructor from all angles in degrees
    static YawPitchRollAngles FromDegrees(double yawDegrees, double pitchDegrees, double rollDegrees) 
        {return YawPitchRollAngles(AngleInDegrees::FromDegrees(yawDegrees), AngleInDegrees::FromDegrees(pitchDegrees), AngleInDegrees::FromDegrees(rollDegrees));}

    //! Try to extract angles from a RotMatrix
    GEOMDLLIMPEXP static bool TryFromRotMatrix(YawPitchRollAnglesR angles, RotMatrixCR matrix);

    //! Convert the angles to a RotMatrix
    GEOMDLLIMPEXP RotMatrix ToRotMatrix() const;

    //! Convert the angles and an origin to a Transform.
    Transform ToTransform(DPoint3dCR origin) const {return Transform::From(ToRotMatrix(), origin);}

    GEOMDLLIMPEXP static bool TryFromTransform(DPoint3dR origin, YawPitchRollAnglesR angles, TransformCR transform);

    //! test for near-zero angles.
    GEOMDLLIMPEXP bool IsIdentity() const;

    //! return the maximum absolute radians among the angles.
    GEOMDLLIMPEXP double MaxAbsRadians() const;

    //! return the maximum absolute difference in radians among the angles.
    GEOMDLLIMPEXP double MaxDiffRadians(YawPitchRollAngles const& other) const;

    //! return the maximum absolute degrees among the angles.
    GEOMDLLIMPEXP double MaxAbsDegrees() const;

    //! return the maximum absolute difference in degrees among the angles.
    GEOMDLLIMPEXP double MaxDiffDegrees(YawPitchRollAngles const& other) const;
};
END_BENTLEY_GEOMETRY_NAMESPACE
