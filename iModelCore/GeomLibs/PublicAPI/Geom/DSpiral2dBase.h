/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#ifndef MinimalRefMethods

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#define DEFAULT_SPIRAL_MAX_STROKE_LENGTH 10.0
typedef struct DSpiral2dBase *DSpiral2dBaseP;
typedef struct DSpiral2dBase const *DSpiral2dBaseCP;

typedef struct DSpiral2dBase &DSpiral2dBaseR;
typedef struct DSpiral2dBase const &DSpiral2dBaseCR;

// Interface class for callbacks ...
struct AnnounceDoubleDPoint2d
{
GEOMAPI_VIRTUAL void Announce (double fraction, DVec2dCR xy) = 0;
};
typedef struct AnnounceDoubleDPoint2d &AnnounceDoubleDPoint2dR;

//!
//! @description Base class for virtuals that implement spiral curve variants.
//!
//! A spiral is governed by its start bearing, start curvature, end bearing,
//!    and end curvature, and a type code which controls how bearing and curvature
//!    vary as a function of distance along the curve.
//!
//! The curve position, measured from a local origin at the start of the curve,
//! is an integral of the spiral functions:
//! <list>
//! <item> x(t1) = x(0) + integral [0..L] dt cos(Q(t)) </item>
//! <item> y(t1) = y(0) + integral [0..L] dt sin(Q(t)) </item>
//! </list>
//!
//! Taking derivatives, the bearing angle of the curve is exactly Q(t), and
//! the curvature is exactly K(t)=dQ/dt.
//!
//! For any intergraion interval, the curve LENGTH is the angle change divided by the average curvature.
//!
//! Usage notes:
//! <list>
//! <item>Derived classes must implement their constructor and two functions
//!   <list>
//!    <item>DistanceToCurvature -- return curvature at specified distance</item>
//!    <item>DistanceToLocalAngle -- return turn angle from start to specified distance</item>
//!   </list>
//! </item>
//! <item>The constructor call installs constants but does NOT install start and end bearing and curvature.</item>
//!
//! <item>Base class constructor sets default 0..1 for both bearing and curvature.</item>
//! <item>An instance becomes useful for computations after a call to SetupBearingAndCurvature.  As a result of this call,
//! <list>
//! <item>This caller's start bearing and curvature and end bearing curvature are installed.</item>
//! <item>The spiral's length is calculated from angle change divided by average curvature.</item>
//! <item>The base class supplies the following additional methods:
//! <list>
//!    <item>DistanceToFracton -- convert distance-along to fractional coordinate 0..1.</item>
//!    <item>DistanceToGlobalAngle -- call DistanceToLocalAngle and add to start angle.</item>
//!    <item>Stroke methods (static methods, takes a spiral instance as an argument) integrate to (multiple) points along the
//!            curve.  Caller supplies a desired angular interval between points.  (Recommended value: 0.08 radians)
//! </list>
//! </item>
//! </list>
//!
struct GEOMDLLIMPEXP DSpiral2dBase : BSIVectorIntegrand
{
public:
// true transition spirals
static const int TransitionType_Unknown           = 0;
static const int TransitionType_Clothoid          = 10;
static const int TransitionType_Bloss             = 11;
static const int TransitionType_Biquadratic       = 12;
static const int TransitionType_Cosine            = 13;
static const int TransitionType_Sine              = 14;
static const int TransitionType_Viennese          = 20;
static const int TransitionType_WeightedViennese  = 21;
// convention: spirals that really have direct evaluations start at 50.
static const int TransitionType_FirstDirectEvaluate = 50;
static const int TransitionType_WesternAustralian       = 50; // IMPLEMENTED 01/18
static const int TransitionType_Czech               = 51; // NOT IMPLEMENTED
static const int TransitionType_AustralianRailCorp          = 52; // IMPLEMENTED 01/18
static const int TransitionType_Italian             = 53; // NOT IMPLEMENTED
static const int TransitionType_PolishCubic         = 54; // NOT IMPLEMENTED
static const int TransitionType_Arema               = 55;
static const int TransitionType_MXCubicAlongArc             = 56; // IMPLEMENTED 01/18
static const int TransitionType_MXCubicAlongTangent          = 57; // NOT IMPLEMENTED
static const int TransitionType_ChineseCubic        = 58;
// convention: spirals with nominal length parameter start at 60
static const int TransitionType_DirectHalfCosine    = 60; // IN DEVELOPMENT 7/18
static const int TransitionType_JapaneseCubic = 61; // Japanese x^3/(6*R1*X1)

//! invoke appropriate concrete class constructor.
//! beware: returned spiral for "direct" types may require special care in bearing/length/curvature setup.
public: static DSpiral2dBaseP Create (int transitionType);

//! invoke appropriate concrete class constructor.
//! beware: returned spiral for "direct" types may require special care in bearing/length/curvature setup.

//! <ul>
//! <li> extraData sequence is type specific.
//! <li> Viennese: {cant,h,e, TransitionType_Viennese}
//! <li> WeightedViennese: {cant,h,e,weight0, weight1, TransitionType_WeightedViennese}
//! <li> Returns nullptr if extraData size and is smaller than needed for the type.  (Excess is ok, to allow type-agnostic storage of larger data block)
//! <li> Note that the final entry repeats the transition type to verify.
//! </ul>
public: static DSpiral2dBaseP Create(int transitionType, bvector<double> const &extraData);

//! Get the extra data for this spiral.
//! REMARK: This should be a virtual, but is implemented as special cases because of API change restriction
public: void GetExtraData(bvector<double> &extraData) const;

//! invoke appropriate concrete class constructor, with type-specific nominal length
public: static DSpiral2dBaseP CreateWithNominalLength (int transitionType, double parameter);

//! return the integer code for the string name.
public: static int StringToTransitionType (Utf8CP name);

//! return the string name of the type
public: static bool TransitionTypeToString (int type, Utf8StringR string);
//! invoke appropriate concrete class constructor ...
public: static DSpiral2dBaseP CreateBearingCurvatureBearingCurvature
      (
      int transitionType,
      double startRadians,
      double startCurvature,
      double endRadians,
      double endCurvature, bvector<double> const &extraData);

//! DEPRECATED -- just adds empty extra data for long form CreateBearingCurvatureBearingCurvature
public: static DSpiral2dBaseP CreateBearingCurvatureBearingCurvature
 (int transitionType, double startRadians, double startCurvature,  double endRadians,  double endCurvature);

//! invoke appropriate concrete class constructor ...
public: static DSpiral2dBaseP CreateBearingCurvatureLengthCurvature
      (
      int transitionType,
      double startRadians,
      double startCurvature,
      double length,
      double endCurvature
      );
//! invoke appropriate concrete class constructor (with extra data)
public: static DSpiral2dBaseP CreateBearingCurvatureLengthCurvature
        (
            int transitionType,
            double startRadians,
            double startCurvature,
            double length,
            double endCurvature,
            bvector<double> const &extraData
        );

public:
    // Primary bearing and curvature data:
    double mTheta0, mTheta1;
    double mCurvature0, mCurvature1;

    // Derived length:
    double mLength;
public:
    // Return the recommended default stroke angle control
    static double DefaultStrokeAngle ();

public:
    // Default constructor.
    //
    DSpiral2dBase ();
    // Clone all data ...
    GEOMAPI_VIRTUAL DSpiral2dBaseP Clone () const = 0;
    // Set bearing and angle at start and end of the bounded curve.
    // Return true if able to calculate length
    bool SetBearingAndCurvatureLimits
            (
            double theta0,
            double curvature0,
            double theta1,
            double curvature1
            );

    //!
//!    @description Set start bearing, start curvature, length, and end curvature.
//!       (Compute end bearing)
//!    @param [in] theta0  start bearing
//!    @param [in] curvature0  start curvature
//!    @param [in] length  arc length
//!    @param [in] curvature1  end curvature
//!    

    bool SetBearingCurvatureLengthCurvature
    (
    double theta0,
    double curvature0,
    double length,
    double curvature1
    );

//!    @description Set start bearing, start curvature, length, and end curvature.
//!       (Compute end bearing)
//!    @param [in] theta0  start bearing
//!    @param [in] curvature0  start curvature
//!    @param [in] length  arc length
//!    @param [in] theta1 end bearing
    bool SetBearingCurvatureLengthBearing
    (
    double theta0,
    double curvature0,
    double length,
    double theta1
    );

    // Copy all base class parameters from source.
    void CopyBaseParameters (DSpiral2dBaseCP pSource);
    // Convert distance-along the spiral to fractional coordinate.
    double DistanceToFraction (double distance) const;
    // Convert fractional coordiante to distance-along.
    double FractionToDistance (double fraction) const;
    //!
//!    @description BSIVectorIntegrand query function.
//!    

    int  GetVectorIntegrandCount () override;
    //!
//!    @description BSIVectorIntegrand query function.
//!    @param [in] distance  distance parameter
//!    @param [out] pF  array of two doubles x,y for integration.
//!    

    void EvaluateVectorIntegrand (double distance, double *pF) override;

    double DistanceToGlobalAngle (double distance) const;
    
    // Apply a scale factor (e.g. change of units) in place.
    // return true if the scale is nonzero.
    bool ScaleInPlace (double s);

    // Return derivatives of the position vector with respect to fraction parameter.
    bool FractionToDerivatives (double fraction, DVec2dR dXdf, DVec2dR ddXdfdf, DVec2dR dddXdfdfdf);

    // Derived classes must implement 3 virtuals:
    // Return the accumulated turning angle at specified distance, i.e. always 0 at distanece 0;
    GEOMAPI_VIRTUAL double DistanceToLocalAngle  (double distance) const = 0;
    // Return the curvature at specified distance from start ...
    GEOMAPI_VIRTUAL double DistanceToCurvature   (double distance) const = 0;
    // Return the derivative of curvature wrt arc length at specified distance from start ...
    GEOMAPI_VIRTUAL double DistanceToCurvatureDerivative (double distance) const = 0;

    // Return the type code for this spiral.
    GEOMAPI_VIRTUAL int GetTransitionTypeCode () const = 0;

#define DECLARE_DSPIRAL2DBASE_OVERRIDES \
    double DistanceToLocalAngle  (double distance) const override;\
    double DistanceToCurvature   (double distance) const override;\
    DSpiral2dBaseP Clone () const override;\
    double DistanceToCurvatureDerivative (double distance) const override;\
    int GetTransitionTypeCode () const override;

#define DECLARE_DSPIRAL2DBASE_DIRECT_EVALUATION_MIDLEVEL_OVERRIDES \
    double DistanceToLocalAngle  (double distance) const override;\
    double DistanceToCurvature   (double distance) const override;\
    double DistanceToCurvatureDerivative (double distance) const override;

#define DECLARE_DSPIRAL2DBASE_DIRECT_EVALUATION_OVERRIDES \
    DSpiral2dBaseP Clone () const override;\
    int GetTransitionTypeCode () const override;


//!
//! @description Integrate the vector displacements of a clothoid between fractional parameters,
//!    returning (only) the displacement between the parameters.
//! @param [in] startFraction  start fraction for integral.
//! @param [in] endFraction  end fraction for integral.
//! @param [in] maxRadians  maximum bearing change between computed points.
//!        A default is used if 0.0 is passed.
//! @param [out] delta  vector from fractional start to fractional end.
//! @param [out] errorBound  estimated bound on error.
//! @param [in] maxStrokeLength recommended 10.0 meters
//! @returns false if point integration failed
//!
static bool Stroke
(
DSpiral2dBase &spiral,
double startFraction,
double endFraction,
double maxRadians,
DVec2dR delta,
double &errorBound,
double maxStrokeLength = DEFAULT_SPIRAL_MAX_STROKE_LENGTH
);

//!
//! @description Integrate the vector displacements of a clothoid over a fractional interval.
//! This uses the angles, curvatures, and length.
//! @param [in] spiral spiral to stroke
//! @param [in] startFraction  start fraction for integral.
//! @param [in] endFraction  end fraction for integral.
//! @param [in] maxRadians  maximum bearing change between computed points.
//!        A default is used if 0.0 is passed.
//! @param [out] uvPoints  array to receive points.  (optional).
//! @param [out] fractions array to receive fractions.
//! @param [out] errorBound  estimated bound on error.
//! @param [in] maxStrokeLength recommended 10.0 meters
//! @returns false if point integration failed
//!
static bool Stroke
(
DSpiral2dBase &spiral,
double startFraction,
double endFraction,
double maxRadians,
bvector<DVec2d> &uvPoints,
bvector<double> &fractions,
double &errorBound,
double maxStrokeLength = DEFAULT_SPIRAL_MAX_STROKE_LENGTH
);


//!
//! @description Integrate the vector displacements of a clothoid over a fractional interval.
//! This uses the angles, curvatures, and length.
//! @param [in] spiral spiral to stroke
//! @param [in] startFraction  start fraction for integral.
//! @param [in] endFraction  end fraction for integral.
//! @param [in] maxRadians  maximum bearing change between computed points.
//!        A default is used if 0.0 is passed.
//! @param [in] F object with a method F->Announce (f, uv) called to announce fraction and coordinate as computed.
//! @param [out] errorBound  estimated bound on error.
//! @param [out] minInterval smallest number of intervals allowed
//! @param [out] maxStrokeLength maximum allowed stroke length
//! @returns false if point integration failed
//!
static bool StrokeToAnnouncer
(
DSpiral2dBase &spiral,
double startFraction,
double endFraction,
double maxRadians,
AnnounceDoubleDPoint2dR F,
double &errorBound,
int minInterval = 0,
double maxStrokeLength = DEFAULT_SPIRAL_MAX_STROKE_LENGTH
);


//! Return an interval count for stroking or integration.
//! Except for degenerate single interval cases, the interval count is always even.  That is the possible values are 
//! @param [in] spiral spiral being queried.
//! @param [in] startFraction start of interval to stroke.
//! @param [in] endFraction end of interval to stroke.
//! @param [in] maxRadians max turn between strokes.
//! @param [in] minInterval smallest number of intervals.
//! @param [in] maxStrokeLength largest stroke size.  Recommended 10 meters
static size_t GetIntervalCount
(
DSpiral2dBase &spiral,
double startFraction,
double endFraction,
double maxRadians,
int minInterval = 0,
double maxStrokeLength = DEFAULT_SPIRAL_MAX_STROKE_LENGTH
);
//!
//! @description Compute the closest spiral point for a given space point.
//! @param [in] spiral  spiral to evaluate.
//! @param [in] startFraction  start fraction of search range
//! @param [in] endFraction  endFraction of search range
//! @param [in] spiralToWorld  transform placing the spiral local coordinates into the world coordinate system
//! @param [in] spacePoint  world coordinates of space point.
//! @param [out] spiralPoint  world coordinates of closest point on spiral
//! @param [out] spiralFraction  fractional coordinates of closest point on spiral
//! @param [out] minDistance  distance from space point to spiralPoint.
//! @returns false if unable to construct
//!
static bool ClosestPoint
(
DSpiral2dBase &spiral,
double startFraction,
double endFraction,
TransformCR spiralToWorld,
DPoint3dCR spacePoint,
DPoint3dR spiralPoint,
double&   spiralFraction,
double&   minDistance
);

// Evaluate clothoid series approximation using TWO terms from each of sine and cosine parts.
// This is used by both ChineseCubic and Arema.
static bool EvaluateTwoTermClothoidSeriesAtDistanceInStandardOrientation
(
    double s, //!< [in] distance for evaluation
    double length,  //!< [in] nominal length.   ASSUMED NONZERO
    double curvature1, //!< [in] exit curvature.  ASSUMED NONZERO
    DPoint2dR xy,          //!< [out] coordinates on spiral
    DVec2dP d1XY,   //!< [out] first derivative wrt distance
    DVec2dP d2XY,   //!< [out] second derivative wrt distance
    DVec2dP d3XY   //!< [out] third derivative wrt distance
);

//!
//! @description compute spirals and arc to make a line-to-line transition.
//! @param [in] lineAPoint  point on line A.
//! @param [in] lineBPoint  point on line B.
//! @param [in] lineLineIntersection  intersection of lines.
//! @param [in] radius  radius for circular part of transition.
//! @param [in] lengthA  length of spiral from line A to circle.
//! @param [in] lengthB  length of spiral from line B to circle.
//! @param [in,out] spiralA  On input, a spiral of the desired type.  On output
//!        all fields are set.
//! @param [in,out] spiralB  On input, a spiral of the desired type.  On output
//!        all fields are set.
//! @param [out] lineToSpiralA  tangency point from line to spiral
//! @param [out] spiralAToArc  tangency point from spiral to arc
//! @param [out] lineToSpiralB  tangency point from line to spiral
//! @param [out] spiralBToArc  tangency point from spiral to arc
//! @param [out] arc  arc between spirals.
//! @returns false if unable to construct
//!
static bool LineSpiralArcSpiralLineTransition
(
DPoint3dCR lineAPoint,
DPoint3dCR lineBPoint,
DPoint3dCR lineLineIntersection,
double     radius,
double      lengthA,
double     lengthB,
DSpiral2dBase &spiralA,
DSpiral2dBase &spiralB,
DPoint3dR  lineToSpiralA,
DPoint3dR  lineToSpiralB,
DPoint3dR  spiralAToArc,
DPoint3dR  spiralBToArc,
DEllipse3dR arc
);

//!
//! @description compute spirals and arc to make a line-to-line transition.
//! @param [in] lineAPoint  point on line A.
//! @param [in] lineBPoint  point on line B.
//! @param [in] lineLineIntersection  intersection of lines.
//! @param [in] length  length of spiral from line A to junction
//! @param [in,out] spiralA  On input, a spiral of the desired type.  On output
//!        all fields are set.
//! @param [in,out] spiralB  On input, a spiral of the desired type.  On output
//!        all fields are set.
//! @param [out] lineToSpiralA  tangency point from line to spiral
//! @param [out] lineToSpiralB  tangency point from line to spiral
//! @param [out] spiralToSpiral  tangency point from spiral to to spiral
//! @param [out] junctionRadius radius at transition between the spirals.
//! @returns false if unable to construct
//!
static bool SymmetricLineSpiralSpiralLineTransition
(
DPoint3dCR lineAPoint,
DPoint3dCR lineBPoint,
DPoint3dCR lineLineIntersection,
double      length,
DSpiral2dBase &spiralA,
DSpiral2dBase &spiralB,
DPoint3dR  lineToSpiralA,
DPoint3dR  lineToSpiralB,
DPoint3dR  spiralToSpiral,
double     &junctionRadius
);
//!
//! @description compute 2 spirals.
//   First spiral begins exactly at the start point and aims at the shoulder
//   Second spiral ends somewhere on the line from shoulder to target.
//! @param [in] startPoint start point
//! @param [in] shoulderPoint target point for first and last tangents
//! @param [in] targetPoint target point for last tangent
//! @param [in,out] spiralA  On input, a spiral of the desired type.  On output
//!        all fields are set.
//! @param [in,out] spiralB  On input, a spiral of the desired type.  On output
//!        all fields are set.
//! @param [out] junctionPoint transition between spirals.  This is the max curvature point.
//! @param [out] endPoint end of second spiral.
//! @returns false if unable to construct
//!
static bool SymmetricPointShoulderTargetTransition
(
DPoint2dCR startPoint,
DPoint2dCR shoulderPoint,
DPoint2dCR targetPoint,
DSpiral2dBase &spiralA,
DSpiral2dBase &spiralB,
DPoint2dR    junctionPoint,
DPoint2dR    endPoint
);

//!
//! @description interface declaration for function to receive announcements of
//!  (multiple) Arc-spiral-line-spiral-arc transltions.
//!
struct ASLSACollector
    {
    //!
//!    @description Announce a single transition.
//!    @param [in] centerA  circle A center.
//!    @param [in] arcToSpiralA  tangency point from arc A to spiral
//!    @param [in] spiralA  spiral from arc A to line.
//!    @param [in] spiralToLineA  tangency point from spiral B to line.
//!    @param [in] centerB  circle B center.
//!    @param [in] arcToSpiralB  tangency point from arc A to spiral
//!    @param [in] spiralB  spiral from arc B to line.
//!    @param [in] spiralToLineB  tangency point from spiral B to line.
//!    

    GEOMAPI_VIRTUAL void Collect
        (
        DPoint3dCR     centerA,
        DPoint3dCR      arcToSpiralA,
        DSpiral2dBase   &spiralA,
        DPoint3dCR      spiralToLineA,
        DPoint3dCR      centerB,
        DPoint3dCR      arcToSpiralB,
        DSpiral2dBase   &spiralB,
        DPoint3dCR      spiralToLineB
        ) = 0;
    };
//!
//! @description compute spirals and line segment to make an arc-to-arc transition
//!        with specified length of the spiral parts.
//! @param [in] centerA  circle A center.
//! @param [in] radiusA  circle A radius.
//! @param [in] lengthA  length of spiral leaving A.
//! @param [in] centerB  circle B center.
//! @param [in] radiusB  circle B radius.
//! @param [in] lengthB  length of spiral leaving B.
//! @param [in] spiralA  first spiral.  This is modified during the construction --
//!        intermediate forms are passed to the collector, and there is no particular
//!        significance to the final parameters.
//! @param [in] spiralB  second spiral.  This is modified during the construction --
//!        intermediate forms are passed to the collector, and there is no particular
//!        significance to the final parameters.
//! @param [in] collector  object to receive transition candidates.
//!        This may be called multiple times.
//! @returns number of solutions announced to collector.
//!
static int ArcSpiralLineSpiralArcTransition
(
DPoint3dCR      centerA,
double          radiusA,
double          lengthA,
DPoint3dCR      centerB,
double          radiusB,
double          lengthB,
DSpiral2dBase  &spiralA,
DSpiral2dBase  &spiralB,
ASLSACollector &collector
);

/*-----------------------------------------------------------------*//**
@description test if a length-from-inflection and final radius combination is "small enough" for reasonable use.
Test depends on spiral type:
<ul>
<li> DSpiral2dBase::TransitionType_None indicates apply the strictest test (same as Czech and Italian)
<li> DSpiral2dBase::TransitionType_Czech and DSpiral2dBase::TransitionType_Italian indicate a strict test L < 2*R
<li> other types have no restrictions.
</ul>
@param [in] lengthFromInflection [in] distance along spiral starting at inflection and ending at finalRadius.
@param [in] finalRadius [in] final radius
@param [in] spiralType [in] spiral type.
@param [in] lengthFactor expansion or reduction factor to apply to the allowed length.
+---------------+---------------+---------------+---------------+------*/
bool static IsValidRLCombination (double lengthFromInflection, double radius, int spiralType = DSpiral2dBase::TransitionType_Unknown);


};


// CLOTHOID spiral
// Curvature function is simple linear variation from K0 to K1, i.e.
//         K(u) = K0 + u (K1 - K0)
struct GEOMDLLIMPEXP DSpiral2dClothoid : DSpiral2dBase
{
public:
    DSpiral2dClothoid ();
    DECLARE_DSPIRAL2DBASE_OVERRIDES
};

// BLOSS Spiral
// Curvature function has linear first derivative, constant second derivative ...
//       K(u) = K0 + (3u^2 - 2 u^3)(K1 - K0)
struct GEOMDLLIMPEXP DSpiral2dBloss : DSpiral2dBase
{
public:
    DSpiral2dBloss ();
    DECLARE_DSPIRAL2DBASE_OVERRIDES
};

// BIQUADRATIC Spiral
// Curvature function is a pair of quadratics with continuous K and K' at midpoint,
//           i.e. K(u) = K0 + 2U^2 (K1 - K0) for u in (0,1/2); reverse from 1.
// Second derivative is piecewise constant at 1 and -1 over the first and second half intervals.
struct GEOMDLLIMPEXP DSpiral2dBiQuadratic : DSpiral2dBase
{
public:
    DSpiral2dBiQuadratic ();
    DECLARE_DSPIRAL2DBASE_OVERRIDES
};


// COSINE spiral ..
// Curvature function second derivative is 180 degrees of cosine.
struct GEOMDLLIMPEXP DSpiral2dCosine : DSpiral2dBase
{
public:
    DSpiral2dCosine ();
    DECLARE_DSPIRAL2DBASE_OVERRIDES
};

// SINE spiral ...
// Curvature function second derivative is 360 degrees of sine.
struct GEOMDLLIMPEXP DSpiral2dSine : DSpiral2dBase
{
public:
    DSpiral2dSine ();
    DECLARE_DSPIRAL2DBASE_OVERRIDES
};



// VIENNESE spiral
// Curvature function second derivative is
struct GEOMDLLIMPEXP DSPiral2dViennese : DSpiral2dBase
{
    friend DSpiral2dBase;
private:
// direct parameters -- bundled to mPhi in constructor.
    double mCant;
    double mH;
    double mE;
// The only parameter used in computations  . ...
    double mPhi;
public:
    DSPiral2dViennese
    (
    double cant,
    double h,
    double e
    );

    DECLARE_DSPIRAL2DBASE_OVERRIDES
    //! Return the extra data array for the parameters of a viennese spiral 
    static void FillExtraDataArray(bvector<double> &extraData, double cant, double h, double e);
};

// Specialization for VIENNESE with caller-specified multipliers for primary and roll terms
//   <list>
//    <item>Spiral part second derivative Ks''(u) proportional to u^2 (1-u)^2 (1-2u), i.e. has double roots at 0 and 1, single root at 0.5.</item>
//    <item>Roll part curvature Kr(u) is proportioanl to u^2 (1-u)^2 (1-2u)
//   </list>
struct GEOMDLLIMPEXP DSPiral2dWeightedViennese : DSpiral2dBase
{
friend DSpiral2dBase;
private:
    // direct parameters -- bundled to mPhi in constructor.
    double mCant;
    double mH;
    double mE;
    double mWeight0;
    double mWeight1;
    // The parameters used in computations  . ...
    double mPhi;
    double mF0;
    double mF1;
public:
    DSPiral2dWeightedViennese
    (
    double cant,
    double h,
    double e,
    double weight0,    // multiplier for standard term
    double weight1     // multiplier for roll term.
    );

    DECLARE_DSPIRAL2DBASE_OVERRIDES
    //! Return the extra data array for the parameters of a weighted viennese spiral 
    static void FillExtraDataArray(bvector<double> &extraData, double cant, double h, double e, double weight0, double weight1);
    };


/**
* intermediate class for "spirals" that really have fraction-to-xyz, with a nominal length for "distance"
* This intermediate class implements DistanceToCurvature, DistanceToLocalAngle, DistanceToCurvatureDerivatives
* based on direct x and y data from EvaluateAtDistance.
*/
struct GEOMDLLIMPEXP DSpiral2dDirectEvaluation: DSpiral2dBase
{
double m_nominalLength;       // arbitrary parameter for evaluation.  (The base spiral data is also available)
DSpiral2dDirectEvaluation (double nominalLength);
public:
//! Evaluate the spiral and derivatives at specified fractional position
//! return true if valid evaluation.
//! DSpiral2dDirectEvaluation default implementation returns false.
virtual bool EvaluateAtFraction
    (
    double fraction, //!< [in] fraction for evaluation
    DPoint2dR xyz,          //!< [out] coordinates on spiral
    DVec2dP d1XYZ,   //!< [out] first derivative wrt fraction
    DVec2dP d2XYZ,   //!< [out] second derivative wrt fraction
    DVec2dP d3XYZ    //!< [out] third derivative wrt fraction
    ) const = 0;

//! Return the true curvature at fractional position.  Implemented in DSpiral2dDirectEvaluation, not individual classes.
//! Assume EvaluateAtFraction.
double FractionToCurvature (double fraction) const;
//! Return the true curvature derivative wrt fraction.  Implemented in DSpiral2dDirectEvaluation, not individual classes.
//! Assume EvaluateAtFraction.
bool FractionToDCurvatureDFraction (double fraction, double &curvature, double &dCurvatureDFraction) const;
//! Return the magnitude of the true derivative of position wrt fraction.  Implemented in DSpiral2dDirectEvaluation, not individual classes.
//! Assume EvaluateAtFraction.
double FractionToVelocity(double fraction) const;
//! Return the tangent angle (in radians) in local coordinates
double FractionToLocalAngle (double fraction) const;

// DistanceToXXX are implemented as pseudo distance:  fraction = distance/m_nominalLength
// Return the accumulated turning angle at specified distance, i.e. always 0 at distanece 0;
GEOMAPI_VIRTUAL double DistanceToLocalAngle  (double distance) const override;
// Return the curvature at specified distance from start ...
GEOMAPI_VIRTUAL double DistanceToCurvature   (double distance) const override;
// Return the derivative of curvature wrt arc length at specified distance from start ...
GEOMAPI_VIRTUAL double DistanceToCurvatureDerivative (double distance) const override;

//! rotate xy and optional derivatives by radians.  (To be called by derived class EvaluateAtDistance when to rotate EvaluateAtDistance results from standard position)
static void ApplyCCWRotation (
    double radians, //!< [in] angle to rotate.
    DPoint2dR xyz,          //! [out] coordinates on spiral
    DVec2dP d1XYZ,   //!< [out] first derivative wrt distance
    DVec2dP d2XYZ,   //!< [out] second derivative wrt distance
    DVec2dP d3XYZ    //!< [out] third derivative wrt distance
    );
};


// WesternAustralian (formerly NEWSOUTHWALES, but that was a name mixup in old civil code) Spiral
// Let a = 1/ (40 R*R*L*L) for exit radius R, spiral length L
// Let b = 1/(6 R L)
//  at distance s along the spiral
//     x =  s *(1-a *s^4)
//     y = b * s^3

struct GEOMDLLIMPEXP DSpiral2dWesternAustralian : DSpiral2dDirectEvaluation
{
    DECLARE_DSPIRAL2DBASE_DIRECT_EVALUATION_OVERRIDES
public:
    DSpiral2dWesternAustralian (double nominalLength);

//! return true if valid evaluation.
bool EvaluateAtFraction
    (
    double fraction, //!< [in] fraction for evaluation
    DPoint2dR xyz,          //!< [out] coordinates on spiral
    DVec2dP d1XYZ,   //!< [out] first derivative wrt fraction
    DVec2dP d2XYZ,   //!< [out] second derivative wrt fraction
    DVec2dP d3XYZ    //!< [out] third derivative wrt fraction
    ) const override;

//! Evaluate at distance a spiral in standard orientation -- zero curvature at origin.
static bool EvaluateAtDistanceInStandardOrientation
    (
    double s,           //!< [in] distance for evaluation
    double length,      //!< [in] strictly nonzero length along spiral.
    double curvature1,  //!< [in] strictly nonzero exit curvature
    DPoint2dR xy,      //!< [out] coordinates on spiral
    DVec2dP d1XY,   //!< [out] first derivative wrt distance
    DVec2dP d2XY,   //!< [out] second derivative wrt distance
    DVec2dP d3XY   //!< [out] third derivative wrt distance
    );
};

// Czech

struct GEOMDLLIMPEXP DSpiral2dCzech : DSpiral2dDirectEvaluation
{
    DECLARE_DSPIRAL2DBASE_DIRECT_EVALUATION_OVERRIDES
public:
    DSpiral2dCzech(double nominalLength);

    //! return true if valid evaluation.
    bool EvaluateAtFraction
    (
        double fraction, //!< [in] fraction for evaluation
        DPoint2dR xyz,          //!< [out] coordinates on spiral
        DVec2dP d1XYZ,   //!< [out] first derivative wrt fraction
        DVec2dP d2XYZ,   //!< [out] second derivative wrt fraction
        DVec2dP d3XYZ    //!< [out] third derivative wrt fraction
    ) const override;

    //! Evaluate at distance a spiral in standard orientation -- zero curvature at origin.
    //@ Remark: Lx and Ls are coupled, but this method does not care which is considered known and which is computed.
    static bool EvaluateAtFractionOfNominalLengthInStandardOrientation
    (
        double fraction,           //!< [in] fraction to be applied to the nominal length-along-curve (which his larger than xLength)
        double Lx,      //! [in] strictly nonzero nominal (x axis) length
        double Ls,      //! [in] strictly nonzero nominal length
        double radius1,  //! [in] strictly nonzero exit radius
        DPoint2dR xy,      //!< [out] coordinates on spiral
        DVec2dP d1XY,   //!< [out] first derivative wrt distance
        DVec2dP d2XY,   //!< [out] second derivative wrt distance
        DVec2dP d3XY   //!< [out] third derivative wrt distance
    );
};
// Polish

struct GEOMDLLIMPEXP DSpiral2dPolish : DSpiral2dDirectEvaluation
    {
    DECLARE_DSPIRAL2DBASE_DIRECT_EVALUATION_OVERRIDES
    public:
        DSpiral2dPolish(double nominalCurveLength);

        //! return true if valid evaluation.
        bool EvaluateAtFraction
        (
            double fraction, //!< [in] fraction for evaluation
            DPoint2dR xyz,          //!< [out] coordinates on spiral
            DVec2dP d1XYZ,   //!< [out] first derivative wrt fraction
            DVec2dP d2XYZ,   //!< [out] second derivative wrt fraction
            DVec2dP d3XYZ    //!< [out] third derivative wrt fraction
        ) const override;

        //! Evaluate at distance a spiral in standard orientation -- zero curvature at origin.
        //@ Remark: Lx and Ls are coupled, but this method does not care which is considered known and which is computed.
        static bool EvaluateAtFractionOfAxisLengthInStandardOrientation
        (
            double fraction,           //!< [in] fraction to be applied to the nominal length-along-curve (which his larger than xLength)
            double &Lx,      //! [out] x axis length. 
            double Ls,      //! [in] strictly nonzero nominal length
            double radius1,  //! [in] strictly nonzero exit radius
            bool mapDerivativesWRTSeriesDistance, //< [in] if true, map derivatives wrt power series distance.  If false wrt axis distance.   true is unusual
            DPoint2dR xy,      //!< [out] coordinates on spiral
            DVec2dP d1XY,   //!< [out] first derivative wrt distance
            DVec2dP d2XY,   //!< [out] second derivative wrt distance
            DVec2dP d3XY   //!< [out] third derivative wrt distance
        );
    //! Return poles for preferred representation as a bezier curve
    //! These are in the local coordinates of the standard orientation
    //! 
    bool GetBezierPoles (
        bvector<DPoint3d> &poles,   //!< [out] poles
        double startFraction, //!< [in] start fraction for active intervale
        double endFraction //!< [in] end fraction for active interval
        );
    /** Execute unit test of the series inversion logic. */
    static double ValidateSeriesInversion();
    };
// Italian
// y = x^3 / (6 R1 L1)
// L1 is construction-time pseudo distance along.
// x related to s by x = s (1-a s^4) with inversion map shared with czech for s=pseudoDistanceAlong
// fraction is along pseudo distance along

struct GEOMDLLIMPEXP DSpiral2dItalian : DSpiral2dDirectEvaluation
    {
    DECLARE_DSPIRAL2DBASE_DIRECT_EVALUATION_OVERRIDES
    public:
        DSpiral2dItalian(double pseudoLength);

        //! return true if valid evaluation.
        bool EvaluateAtFraction
        (
            double fraction, //!< [in] fraction for evaluation
            DPoint2dR xyz,          //!< [out] coordinates on spiral
            DVec2dP d1XYZ,   //!< [out] first derivative wrt fraction
            DVec2dP d2XYZ,   //!< [out] second derivative wrt fraction
            DVec2dP d3XYZ    //!< [out] third derivative wrt fraction
        ) const override;

    };

// AUSTRALIAN Spiral
// In local coordinates, with specific constants  a1,a2,a3,a4 and m based on length and final radius  . . .
// x = s (1 - a1 m^2 s^4 + a2 m^4 s^8 - a3 m^6 s^12 + a4 m^8 s^16)
// y = m * s^3
struct GEOMDLLIMPEXP DSpiral2dAustralianRailCorp : DSpiral2dDirectEvaluation
{
    DECLARE_DSPIRAL2DBASE_DIRECT_EVALUATION_OVERRIDES
public:
    DSpiral2dAustralianRailCorp (double nominalLength);

//! return true if valid evaluation.
bool EvaluateAtFraction
    (
    double fraction, //!< [in] fraction for evaluation
    DPoint2dR xyz,          //!< [out] coordinates on spiral
    DVec2dP d1XYZ,   //!< [out] first derivative wrt fraction
    DVec2dP d2XYZ,   //!< [out] second derivative wrt fraction
    DVec2dP d3XYZ    //!< [out] third derivative wrt fraction
    ) const override;
//! Evaluate at distance a spiral in standard orientation -- zero curvature at origin.
static bool EvaluateAtDistanceInStandardOrientation
    (
    double s,           //!< [in] distance for evaluation
    double length,      //!< [in] strictly nonzero length along spiral.
    double curvature1,  //!< [in] strictly nonzero exit curvature
    DPoint2dR xy,      //!< [out] coordinates on spiral
    DVec2dP d1XY,   //!< [out] first derivative wrt distance
    DVec2dP d2XY,   //!< [out] second derivative wrt distance
    DVec2dP d3XY   //!< [out] third derivative wrt distance
    );
};

// MX approximate Spiral
// In local coordinates, with specific constants  a1,a2,a3,a4 and m based on length and final radius  . . .
// s = nominal distance along spiral
// x = s (1 - a1 m^2 s^4 + a2 m^4 s^8 - a3 m^6 s^12 + a4 m^8 s^16)
// y = m * s^3
struct GEOMDLLIMPEXP DSpiral2dMXCubicAlongArc : DSpiral2dDirectEvaluation
{
    DECLARE_DSPIRAL2DBASE_DIRECT_EVALUATION_OVERRIDES
public:
    DSpiral2dMXCubicAlongArc (double nominalLength);

//! Evaluate the spiral and derivatives at specified fractional position
//! return true if valid evaluation.
bool EvaluateAtFraction
    (
    double fraction, //!< [in] fraction for evaluation
    DPoint2dR xyz,          //!< [out] coordinates on spiral
    DVec2dP d1XYZ,   //!< [out] first derivative wrt fraction
    DVec2dP d2XYZ,   //!< [out] second derivative wrt fraction
    DVec2dP d3XYZ    //!< [out] third derivative wrt fraction
    ) const override;
//! Evaluate at fraction standard orientation -- zero curvature at origin.
static bool EvaluateAtFractionInStandardOrientation
    (
    double fraction, //!< [in] fraction for evaluation
    double length,  //!< [in] nominal length.   ASSUMED NONZERO
    double curvature1, //!< [in] exit curvature.  ASSUMED NONZERO
    DPoint2dR xy,          //!< [out] coordinates on spiral
    DVec2dP d1XY,   //!< [out] first derivative wrt distance
    DVec2dP d2XY,   //!< [out] second derivative wrt distance
    DVec2dP d3XY   //!< [out] third derivative wrt distance
    );
};


// DIRECTHALFCOSINE
// cosine factor multiplying x^2.
struct GEOMDLLIMPEXP DSpiral2dDirectHalfCosine : DSpiral2dDirectEvaluation
{
    DECLARE_DSPIRAL2DBASE_DIRECT_EVALUATION_OVERRIDES
public:
    DSpiral2dDirectHalfCosine (double axisLength);

//! Evaluate the spiral and optional derivatives at specified distance along.
//! return true if valid evaluation.
bool EvaluateAtFraction
    (
    double fraction, //!< [in] distance for evaluation
    DPoint2dR xyz,          //!< [out] coordinates on spiral
    DVec2dP d1XYZ,   //!< [out] first derivative wrt fraction
    DVec2dP d2XYZ,   //!< [out] second derivative wrt fraction
    DVec2dP d3XYZ   //!< [out] third derivative wrt fraction
    ) const override;

//! Evaluate at distance a spiral in standard orientation -- zero curvature at origin.
static bool EvaluateAtFractionInStandardOrientation
    (
    double s,           //!< [in] distance for evaluation
    double axisLength,      //!< [in] strictly nonzero length along spiral.
    double radius1,  //!< [in] strictly nonzero exit radius
    DPoint2dR xy,      //!< [out] coordinates on spiral
    DVec2dP d1XY,   //!< [out] first derivative wrt fraction
    DVec2dP d2XY,   //!< [out] second derivative wrt fraction
    DVec2dP d3XY   //!< [out] third derivative wrt fraction
    );
};

// JAPANESE CUBIC
// y = x^3 / (6 X1 R1).
struct GEOMDLLIMPEXP DSpiral2dJapaneseCubic: DSpiral2dDirectEvaluation
    {
    DECLARE_DSPIRAL2DBASE_DIRECT_EVALUATION_OVERRIDES
    public:
        DSpiral2dJapaneseCubic(double axisLength);

        //! Evaluate the spiral and optional derivatives at specified distance along.
        //! return true if valid evaluation.
        bool EvaluateAtFraction
        (
            double fraction, //!< [in] distance for evaluation
            DPoint2dR xyz,          //!< [out] coordinates on spiral
            DVec2dP d1XYZ,   //!< [out] first derivative wrt fraction
            DVec2dP d2XYZ,   //!< [out] second derivative wrt fraction
            DVec2dP d3XYZ   //!< [out] third derivative wrt fraction
        ) const override;

        //! Evaluate at distance a spiral in standard orientation -- zero curvature at origin.
        static bool EvaluateAtFractionInStandardOrientation
        (
            double s,           //!< [in] distance for evaluation
            double axisLength,      //!< [in] strictly nonzero length along spiral.
            double radius1,  //!< [in] strictly nonzero exit radius
            DPoint2dR xy,      //!< [out] coordinates on spiral
            DVec2dP d1XY,   //!< [out] first derivative wrt fraction
            DVec2dP d2XY,   //!< [out] second derivative wrt fraction
            DVec2dP d3XY   //!< [out] third derivative wrt fraction
        );
    };
// Chinese uses two term series expansion
struct GEOMDLLIMPEXP DSpiral2dChinese : DSpiral2dDirectEvaluation
    {
    DECLARE_DSPIRAL2DBASE_DIRECT_EVALUATION_OVERRIDES
    public:
        DSpiral2dChinese(double nominalLength);

        //! Evaluate the spiral and optional derivatives at specified fraction along.
        //! return true if valid evaluation.
        bool EvaluateAtFraction
        (
            double fraction, //!< [in] fractional position evaluation
            DPoint2dR xyz,          //!< [out] coordinates on spiral
            DVec2dP d1XYZ,   //!< [out] first derivative wrt fraction
            DVec2dP d2XYZ,   //!< [out] second derivative wrt fraction
            DVec2dP d3XYZ   //!< [out] third derivative wrt fraction
        ) const override;

        //! Evaluate at distance a spiral in standard orientation -- zero curvature at origin.
        //! This is implemented as a call to EvaluateTwoTermClothoidSeriesAtDistanceInStandardOrientation.
        static bool EvaluateAtDistanceInStandardOrientation
        (
            double s,           //!< [in] distance for evaluation
            double length,      //!< [in] strictly nonzero length along spiral.
            double curvature1,  //!< [in] strictly nonzero exit curvature
            DPoint2dR xy,      //!< [out] coordinates on spiral
            DVec2dP d1XY,   //!< [out] first derivative wrt distance
            DVec2dP d2XY,   //!< [out] second derivative wrt distance
            DVec2dP d3XY   //!< [out] third derivative wrt distance
        );
    };
// Arema uses two term series expansion
struct GEOMDLLIMPEXP DSpiral2dArema : DSpiral2dDirectEvaluation
    {
    DECLARE_DSPIRAL2DBASE_DIRECT_EVALUATION_OVERRIDES
    public:
        DSpiral2dArema(double nominalLength);

        //! Evaluate the spiral and optional derivatives at specified fraction along.
        //! return true if valid evaluation.
        bool EvaluateAtFraction
        (
            double fraction, //!< [in] fractional position evaluation
            DPoint2dR xyz,          //!< [out] coordinates on spiral
            DVec2dP d1XYZ,   //!< [out] first derivative wrt fraction
            DVec2dP d2XYZ,   //!< [out] second derivative wrt fraction
            DVec2dP d3XYZ   //!< [out] third derivative wrt fraction
        ) const override;

    };
struct DSpiral2dPlacement;
typedef DSpiral2dPlacement *DSpiral2dPlacementP;
typedef DSpiral2dPlacement const *DSpiral2dPlacementCP;
typedef DSpiral2dPlacement &DSpiral2dPlacementR;
typedef DSpiral2dPlacement const &DSpiral2dPlacementCR;

//! Spiral packaged with placement frame and fractional interval
//! Note that the stored spiral is a pointer.
//! The constructor allocates a new spiral as clone of its input.
//! The destructor frees the spiral.
struct GEOMDLLIMPEXP DSpiral2dPlacement
{
DSpiral2dBaseP spiral;
Transform frame;
double fractionA;
double fractionB;
//! Null spiral, identity transform, zero fractions;
DSpiral2dPlacement ();
//! Constructor -- clones the source spiral.
DSpiral2dPlacement (DSpiral2dBaseCR source, TransformCR frame, double fractionA, double fractionB);

//! Constructor -- copies the pointer.
DSpiral2dPlacement (DSpiral2dBaseP _spiral, TransformCR frame, double fractionA, double fractionB);

//! Constructor -- copies the pointer.
void InitCapturePointer (DSpiral2dBaseP _spiral, TransformCR frame, double fractionA, double fractionB);

//! Return a clone.
DSpiral2dPlacementP Clone (DSpiral2dPlacementCR source) const;
//! Free the spiral pointer.
~DSpiral2dPlacement ();
//! Free the current spiral pointer and replace by (possibly NULL) arg.
void ReplaceSpiral (DSpiral2dBaseP callerSpiral);
//! Reverse the fractions !!
bool ReverseInPlace ();
//! Apply AlmostEqual to all components
bool AlmostEqual (DSpiral2dPlacement const &other, double tolerance = 0.0) const;
//! Apply AlmostEqual to all components except fractions
bool AlmostEqual01 (DSpiral2dPlacement const &other, double tolerance = 0.0) const;
//! Apply AlmostEqual to various points and parameters to determine if the same paths are visited.
int AlmostEqualByActiveIntervalPoints(DSpiral2dPlacement const &other, double tolerance) const;
DSegment1d FractionInterval () const;

DPoint3d FractionToPoint (double fraction) const;
DRay3d FractionToPointAndDerivative (double fraction) const;
DRay3d FractionToPointAndUnitTangent (double fraction) const;

DPoint3d ActiveFractionToPoint (double fraction) const;
DRay3d ActiveFractionToPointAndDerivative (double fraction) const;
DRay3d ActiveFractionToPointAndUnitTangent (double fraction) const;

double ActiveFractionToGlobalFraction (double activeFraction) const;
double GlobalFractionToActiveFraction (double globalFraction) const;

//! return xyz first, second, third derivatives as columns.
RotMatrix FractionToDerivatives (double fraction) const;
//! return xyz first, second, third derivatives as columns, scaled for active interval. 
RotMatrix ActiveFractionToDerivatives (double fraction) const;


Transform FractionToFrenetFrame (double fraction) const;
Transform ActiveFractionToFrenetFrame (double fraction) const;
//! absolute length of the reference spiral (between 0 and 1)
double SpiralLength01 () const;
//! absolute length of the partial spiral (between start and end fractions)
double SpiralLengthActiveInterval () const;
//! Length of the spiral after mapping by matrix*frame.matrix
double MappedSpiralLengthActiveInterval (RotMatrixCR matrix) const;

//! return the displacement between fractions.  Having a close f0 is much faster than FractionToPoint with no starter fraction.
DVec3d DisplacementBetweenFractions (double f0, double f1) const;
DVec3d DisplacementBetweenActiveFractions (double g0, double g1) const;
};
#ifndef SmallGeomLib
//! Unbounded catenary (x,y)(s) = (a arcsinh (s/a), sqrt (a*a + s*s)
//! which also satisfies     y = (1/a) cosh (x/a)
//! This is normally used as a member of an object that provides transform and x limits.
//! Within this class all the easy x-y-distance relationships are well defined.
struct DCatenaryXY
{
double m_a;
GEOMDLLIMPEXP DCatenaryXY (double a);
GEOMDLLIMPEXP DCatenaryXY ();

//! Return curve y value at x
GEOMDLLIMPEXP double YAtX (double x) const;
//! Return slope at x
GEOMDLLIMPEXP double dYdXAtX (double x) const;
//! Return 2nd derivative at x.
GEOMDLLIMPEXP double d2YdX2AtX (double x) const;

//! Return curve angle (from X axis) at x.
GEOMDLLIMPEXP double RadiansAtX (double x) const;
//! Return curve length from origin to x.  This is signed.
GEOMDLLIMPEXP double LengthAtX (double x) const;
//! return x,y at arc length from the min point.
GEOMDLLIMPEXP DPoint2d XYAtLength (double s) const;
//! Return the tangent vector (unit), derivative of XY wrt s.
GEOMDLLIMPEXP DVec2d TangentAtLength (double s) const;
//! Return the tangent angle at distance
GEOMDLLIMPEXP double RadiansAtLength (double s) const;
//! Return the point and 2 derivatives at distance from min point
GEOMDLLIMPEXP void DerivativesAtLength (double s, DPoint2dR uv, DVec2dR duv, DVec2dR dduv, DVec2dR ddduv) const;
//! return curvature at (signed) distance along.
GEOMDLLIMPEXP double CurvatureAtLength(double s) const;

GEOMDLLIMPEXP bool AlmostEqual (DCatenaryXY const &other, double tolerance) const;

/**
Solve for roots of alpha + beta*x + gamma * cosh(x) = 0
<ul>
<li> Note that the cosh(x) is raw x -- NOT divided by the "a" parameter used in the catenary.
<li> At most 2 roots are possible.
<li> A false return is detection of failed iteration.
<li> A true return with 0,1,or 2 roots is normal.
<li> The false case is probably due to very large numbers.   cosh(x) gets large very quickly.
</ul>
*/
GEOMDLLIMPEXP static bool CoshIntersectLine (double alpha, double beta, double gamma, bvector<double> &roots);
 
/**
Solve for simultaneous roots of
<ul>
<li>y = cosh(x)
<li>x*hLine.x + y*hLine.y + hLine.z = 0
<ul>
<li> Note that the cosh(x) is raw x -- NOT divided by the "a" parameter used in the catenary.
<li> At most 2 roots are possible.
<li> A false return is detection of failed iteration.
<li> A true return with 0,1,or 2 roots is normal.
<li> The false case is probably due to very large numbers.   cosh(x) gets large very quickly.
</ul>
*/
GEOMDLLIMPEXP static bool CoshIntersectHomogeneousLine (DVec3dCR hLine, bvector<double> &roots);


};

#define MIN_STROKE_RADIANS (1.0e-3)
#define MAX_STROKE_POINTS (200)
//! Bounded catenary placed into a coordinate system.
//! The catenary bounds are (signed) distances from the low point.
struct DCatenary3dPlacement
{
friend struct CurvePrimitiveCatenaryCurve;
private:
DPoint3dDVec3dDVec3d m_basis;
DCatenaryXY m_xyCatenary;
DSegment1d m_distanceLimits;
public:
//! Constructor.
GEOMDLLIMPEXP DCatenary3dPlacement (double a, DPoint3dDVec3dDVec3dCR basis, double distance0, double distance1);

GEOMDLLIMPEXP DCatenary3dPlacement () {}


GEOMDLLIMPEXP DCatenary3dPlacement (DCatenaryXY const &xyCurve, DPoint3dDVec3dDVec3dCR basis, double x0, double x1);

GEOMDLLIMPEXP DCatenary3dPlacement (DCatenary3dPlacement  const &other);
//! Return a clone over a fractional interval within the existing curve.
GEOMDLLIMPEXP DCatenary3dPlacement CloneBetweenFractions (double fraction0, double fraction1) const;

//! Get explicit contents ...
GEOMDLLIMPEXP void Get (double &a, DPoint3dDVec3dDVec3dR basis, DSegment1dR segment) const;
//! Return the local coordinate (parameter) of the catenary start.
GEOMDLLIMPEXP double StartDistance () const;
//! Return the local coordinate (parameter) of the catenary end.
GEOMDLLIMPEXP double EndDistance () const;

GEOMDLLIMPEXP bool AlmostEqual (DCatenary3dPlacement const &other, double tolerance) const;

GEOMDLLIMPEXP void ReverseInPlace ();

GEOMDLLIMPEXP void MultiplyInPlace (TransformCR transform);
GEOMDLLIMPEXP DPoint3d FractionToPoint (double f) const;
GEOMDLLIMPEXP DRay3d FractionToPointAndTangent (double f) const;
GEOMDLLIMPEXP Transform FractionToPointAndDerivatives (double f) const;

GEOMDLLIMPEXP void Stroke (bvector<DPoint3d> &xyz, bvector<double> &fraction, double fraction0, double fraction1,
double chordTolerance,
double angleTolerance,
double maxEdgeLength
) const;
//! compute intersections with a plane. Return x values
GEOMDLLIMPEXP bool AppendPlaneIntersections (DPlane3dCR plane, bvector<double> &xValues, bool bounded) const;

};
#endif
END_BENTLEY_GEOMETRY_NAMESPACE
#endif
