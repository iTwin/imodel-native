/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#define _USE_MATH_DEFINES
END_BENTLEY_GEOMETRY_NAMESPACE
#include <math.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static double sDivideRelTol = 1.0e-20;


static double s_defaultStrokeRadians = 0.04;
static int s_defaultMinIntervals = 8;

int DSpiral2dClothoid::GetTransitionTypeCode () const { return TransitionType_Clothoid;}
int DSpiral2dBloss::GetTransitionTypeCode () const { return TransitionType_Bloss;}
int DSpiral2dBiQuadratic::GetTransitionTypeCode () const { return TransitionType_Biquadratic;}
int DSpiral2dCosine::GetTransitionTypeCode () const { return TransitionType_Cosine;}
int DSpiral2dSine::GetTransitionTypeCode () const { return TransitionType_Sine;}

int DSPiral2dWeightedViennese::GetTransitionTypeCode () const { return TransitionType_WeightedViennese;}
int DSPiral2dViennese::GetTransitionTypeCode () const { return TransitionType_Viennese;}
int DSpiral2dWesternAustralian::GetTransitionTypeCode () const { return TransitionType_WesternAustralian;}
int DSpiral2dCzech::GetTransitionTypeCode() const { return TransitionType_Czech; }
int DSpiral2dItalian::GetTransitionTypeCode() const { return TransitionType_Italian; }
int DSpiral2dAustralianRailCorp::GetTransitionTypeCode () const { return TransitionType_AustralianRailCorp;}
int DSpiral2dDirectHalfCosine::GetTransitionTypeCode () const { return TransitionType_DirectHalfCosine;}
int DSpiral2dJapaneseCubic::GetTransitionTypeCode() const { return TransitionType_JapaneseCubic; }
int DSpiral2dChinese::GetTransitionTypeCode () const { return TransitionType_ChineseCubic;}
int DSpiral2dMXCubicAlongArc::GetTransitionTypeCode () const { return TransitionType_MXCubicAlongArc;}
int DSpiral2dArema::GetTransitionTypeCode() const { return TransitionType_Arema; }
int DSpiral2dPolish::GetTransitionTypeCode() const { return TransitionType_PolishCubic; }



DSpiral2dBase::DSpiral2dBase ()
    {
    SetBearingAndCurvatureLimits (0.0, 0.0, 1.0, 1.0);
    }

static int s_transitionTypeOverride = 0;
DSpiral2dBaseP DSpiral2dBase::Create (int transitionType)
    {
    if (s_transitionTypeOverride != 0)
        transitionType = s_transitionTypeOverride;

    if (transitionType == TransitionType_Clothoid)
        return new DSpiral2dClothoid ();
    if (transitionType == TransitionType_Bloss)
        return new DSpiral2dBloss ();
    if (transitionType == TransitionType_Biquadratic)
        return new DSpiral2dBiQuadratic ();
    if (transitionType == TransitionType_Cosine)
        return new DSpiral2dCosine ();
    if (transitionType == TransitionType_Sine)
        return new DSpiral2dSine ();

    // PROBLEM -- dgnplatform asks for nominal length types only here, doesn't know about CreateWithNominalLength.
    // HACK
    // Build with length 0 -- expect it to be fixed up angles and bearings.  Dicey.
    return CreateWithNominalLength (transitionType, 0.0);
    }

DSpiral2dBaseP DSpiral2dBase::Create(int transitionType, bvector<double> const &extraData)
    {
    if (transitionType == TransitionType_Viennese)
        {
        if (extraData.size () < 4 || extraData[3] != (double)TransitionType_Viennese)
            return nullptr;
        return new DSPiral2dViennese(extraData[0], extraData[1], extraData[2]);
        }
    else if (transitionType == TransitionType_WeightedViennese)
        {
        if (extraData.size() < 6 || extraData[5] != (double)TransitionType_WeightedViennese)
            return nullptr;
        return new DSPiral2dWeightedViennese(extraData[0], extraData[1], extraData[2], extraData[3], extraData[4]);
        }
    return Create (transitionType);
    }

void DSpiral2dBase::GetExtraData (bvector<double> &data) const
    {
    data.clear ();
    DSPiral2dViennese const *pViennese = dynamic_cast<const DSPiral2dViennese*> (this);
    if (pViennese != nullptr)
        {
        DSPiral2dViennese::FillExtraDataArray(data, pViennese->mCant, pViennese->mH, pViennese->mE);
        return;
        }
    DSPiral2dWeightedViennese const *pWeightedViennese = dynamic_cast<const DSPiral2dWeightedViennese*>(this);
    if (pWeightedViennese != nullptr)
        {
        DSPiral2dWeightedViennese::FillExtraDataArray(data, pWeightedViennese->mCant, pWeightedViennese->mH, pWeightedViennese->mE,
            pWeightedViennese->mWeight0, pWeightedViennese->mWeight1);
        return;
        }
    return;
    }

void DSPiral2dViennese::FillExtraDataArray (bvector<double> & data, double cant, double h, double e)
    {
    data.clear ();
    data.push_back(cant);
    data.push_back(h);
    data.push_back(e);
    data.push_back((double)TransitionType_Viennese);
    }

void DSPiral2dWeightedViennese::FillExtraDataArray(bvector<double> & data, double cant, double h, double e, double weight0, double weight1)
    {
    data.clear();
    data.push_back(cant);
    data.push_back(h);
    data.push_back(e);
    data.push_back(weight0);
    data.push_back(weight1);
    data.push_back((double)TransitionType_WeightedViennese);
    }

DSpiral2dBaseP DSpiral2dBase::CreateWithNominalLength(int transitionType, double nominalLength)
    {
    if (transitionType == TransitionType_WesternAustralian)
        return new DSpiral2dWesternAustralian (nominalLength);
    if (transitionType == TransitionType_DirectHalfCosine)
        return new DSpiral2dDirectHalfCosine (nominalLength);
    if (transitionType == TransitionType_JapaneseCubic)
        return new DSpiral2dJapaneseCubic(nominalLength);
    if (transitionType == TransitionType_MXCubicAlongArc)
        return new DSpiral2dMXCubicAlongArc (nominalLength);
    if (transitionType == TransitionType_AustralianRailCorp)
        return new DSpiral2dAustralianRailCorp (nominalLength);
    if (transitionType == TransitionType_ChineseCubic)
        return new DSpiral2dChinese (nominalLength);
    if (transitionType == TransitionType_Czech)
        return new DSpiral2dCzech(nominalLength);
    if (transitionType == TransitionType_Italian)
        return new DSpiral2dItalian(nominalLength);
    if (transitionType == TransitionType_Arema)
        return new DSpiral2dArema(nominalLength);
    if (transitionType == TransitionType_PolishCubic)
        return new DSpiral2dPolish(nominalLength);
    return NULL;
    }



struct SpiralTagName {
    int type;
    Utf8CP name;
    };
static SpiralTagName s_spiralNames [] =
    {
        {DSpiral2dBase::TransitionType_Clothoid, "clothoid"},
        {DSpiral2dBase::TransitionType_Bloss, "bloss"},
        {DSpiral2dBase::TransitionType_Biquadratic, "biquadratic"},
        {DSpiral2dBase::TransitionType_Cosine, "cosine"},
        {DSpiral2dBase::TransitionType_Sine, "sine"},
        {DSpiral2dBase::TransitionType_Viennese, "Viennese"},
        {DSpiral2dBase::TransitionType_WeightedViennese, "WeightedViennese"},
        {DSpiral2dBase::TransitionType_WesternAustralian, "WesternAustralian"},
        {DSpiral2dBase::TransitionType_Czech, "Czech"},
        {DSpiral2dBase::TransitionType_AustralianRailCorp, "AustralianRailCorp"},
        {DSpiral2dBase::TransitionType_Italian, "Italian"},
        {DSpiral2dBase::TransitionType_PolishCubic, "PolishCubic"},
        {DSpiral2dBase::TransitionType_MXCubicAlongArc, "MXCubicAlongArc"},
        {DSpiral2dBase::TransitionType_MXCubicAlongTangent, "MXCubicAlongTangent"},
        {DSpiral2dBase::TransitionType_DirectHalfCosine, "HalfCosine"},
        {DSpiral2dBase::TransitionType_ChineseCubic, "ChineseCubic"},
        {DSpiral2dBase::TransitionType_JapaneseCubic, "JapaneseCubic"},
        {DSpiral2dBase::TransitionType_Arema, "Arema"},
    };
int DSpiral2dBase::StringToTransitionType (Utf8CP name)
    {
    for (auto &entry : s_spiralNames)
        if (0 == BeStringUtilities::Stricmp (name, entry.name))
            return entry.type;
    return TransitionType_Unknown;
    }

bool DSpiral2dBase::TransitionTypeToString (int type, Utf8StringR string)
    {
    string.clear ();
    for (auto &entry : s_spiralNames)
        {
        if (entry.type == type)
            {
            string.assign (entry.name);
            return true;
            }
        }
    string.assign ("unknown");
    return false;
    }
//! invoke appropriate concrete class constructor ...
DSpiral2dBaseP DSpiral2dBase::CreateBearingCurvatureBearingCurvature
      (
      int transitionType,
      double startRadians,
      double startCurvature,
      double endRadians,
      double endCurvature,
      bvector<double> const &extraData
      )
    {
    DSpiral2dBaseP data = Create (transitionType, extraData);
    if (data != NULL)
        data->SetBearingAndCurvatureLimits
              (startRadians, startCurvature, endRadians, endCurvature);
    return data;
    }

//! invoke appropriate concrete class constructor ...
DSpiral2dBaseP DSpiral2dBase::CreateBearingCurvatureBearingCurvature
(
    int transitionType,
    double startRadians,
    double startCurvature,
    double endRadians,
    double endCurvature
)
    {
    return CreateBearingCurvatureBearingCurvature (transitionType, startRadians, startCurvature, endRadians, endCurvature, bvector<double> ());
    }

//! invoke appropriate concrete class constructor ...
DSpiral2dBaseP DSpiral2dBase::CreateBearingCurvatureLengthCurvature
      (
      int transitionType,
      double startRadians,
      double startCurvature,
      double length,
      double endCurvature,
      bvector<double> const &extraData
      )
    {
    DSpiral2dBaseP data = CreateWithNominalLength (transitionType, length);;
    if (data != nullptr)
        data->SetBearingCurvatureLengthCurvature
              (startRadians, startCurvature, length, endCurvature);
    else
        {
        data = Create (transitionType, extraData);
        if (data != nullptr)
            data->SetBearingCurvatureLengthCurvature
              (startRadians, startCurvature, length, endCurvature);
        }
    return data;
    }
//! invoke appropriate concrete class constructor ...
DSpiral2dBaseP DSpiral2dBase::CreateBearingCurvatureLengthCurvature
(
    int transitionType,
    double startRadians,
    double startCurvature,
    double length,
    double endCurvature
)
    {
    return CreateBearingCurvatureLengthCurvature (transitionType, startRadians, startCurvature, length, endCurvature, bvector<double> ());
    }

/*-----------------------------------------------------------------*//**
@description Convert distance-along to fraction.
+---------------+---------------+---------------+---------------+------*/
void DSpiral2dBase::CopyBaseParameters
(
DSpiral2dBaseCP pSource
)
    {
    mTheta0 = pSource->mTheta0;
    mTheta1 = pSource->mTheta1;
    mCurvature0 = pSource->mCurvature0;
    mCurvature1 = pSource->mCurvature1;
    mLength = pSource->mLength;
    }
// THIS IS ONLY RIGHT FOR CLOTHOID !!!
static double AverageAbsoluteCurvature (double curvature0, double curvature1)
    {
    double ak0 = fabs(curvature0);
    double ak1 = fabs(curvature1);
    double averageCurvature;
    if (curvature0 * curvature1 < 0.0)
        {
        // opposite signs -- the spiral crosses the inflection
        // fractions of absolute curvature change fractions are distance fractions to the inflection point.
        double lambda0 = ak0 / (ak0 + ak1);
        double lambda1 = ak1 / (ak0 + ak1);
        // at least 1 is nonzero, so this must be positive
        averageCurvature = 0.5 * (lambda0 * ak0 + lambda1 * ak1);
        }
    else
        {
        averageCurvature = 0.5 * (ak0 + ak1);
        }
    return averageCurvature;
    }

// THIS IS ONLY RIGHT FOR CLOTHOID !!!
static double AverageSignedCurvature(double curvature0, double curvature1)
    {
    return 0.5 * (curvature0 + curvature1);
    }

/*-----------------------------------------------------------------*//**
@description BSIVectorIntegrand query function.
+---------------+---------------+---------------+---------------+------*/
bool DSpiral2dBase::SetBearingAndCurvatureLimits
(
double theta0,
double curvature0,
double theta1,
double curvature1
)
    {
    double angleChange = theta1 - theta0;
    double averageCurvature = AverageAbsoluteCurvature (curvature0, curvature1);
    mTheta0 = theta0;
    mTheta1 = theta1;
    if (curvature0 * curvature1 < 0.0)
        {
        mCurvature0 = curvature0;
        mCurvature1 = curvature1;
        }
    else
        {
        // um ... if curvature does not change sign, its sign must
        // agree with the bearing change.
        double curvatureSign = mTheta1 > mTheta0 ? 1.0 : -1.0;
        mCurvature0 = curvatureSign * fabs(curvature0);
        mCurvature1 = curvatureSign * fabs(curvature1);
        }
    if (fabs (averageCurvature) < sDivideRelTol * fabs (angleChange))
        {
        mLength = 0.0;
        return false;
        }
    mLength = fabs (angleChange / averageCurvature) ;
    auto nominalLengthSpiral = dynamic_cast <DSpiral2dDirectEvaluation*> (this);
    // Problem!! spiral handler does not create approximate spirals correctly.
    // ASSUME/GUESS/PRAY that the curvatures and angle were set up with the transition spiral formulas,
    //   and pulling out the length comes up with the right number ...
    if (nominalLengthSpiral != nullptr && nominalLengthSpiral->m_nominalLength == 0.0)
        nominalLengthSpiral->m_nominalLength = mLength;
    return true;
    }

/*-----------------------------------------------------------------*//**
@description Set start bearing, start curvature, length, and end curvature.
   (Compute end bearing)
@param theta0 IN start bearing
@param curvature0 IN start curvature
@param length IN arc length
@param curvature1 IN end curvature
+---------------+---------------+---------------+---------------+------*/
bool DSpiral2dBase::SetBearingCurvatureLengthCurvature
(
double theta0,
double curvature0,
double length,
double curvature1
)
    {
    mTheta0 = theta0;
    mCurvature0 = curvature0;
    mCurvature1 = curvature1;
    double averageCurvature = AverageAbsoluteCurvature(curvature0, curvature1);
    mLength = length;
    double angleChange = mLength * averageCurvature;
    double angleSign = AverageSignedCurvature (curvature0, curvature1) > 0.0 ? 1.0 : -1.0;
    mTheta1 = mTheta0 + angleSign * angleChange;
    return true;
    }

/*-----------------------------------------------------------------*//**
@description apply a scale factor. (i.e multiply the length, divide the curvatures)
@param a IN factor.
+---------------+---------------+---------------+---------------+------*/
bool DSpiral2dBase::ScaleInPlace (double a)
    {
    auto r = DoubleOps::ValidatedDivide (1.0, a, 1.0);
    if (r.IsValid ())
        {
        mCurvature0 *= r;
        mCurvature1 *= r;
        mLength *= a;
        }
    return r.IsValid ();
    }

/*-----------------------------------------------------------------*//**
@description Set start bearing, start curvature, length, and end curvature.
   (Compute end bearing)
@param theta0 IN start bearing
@param curvature0 IN start curvature
@param length IN arc length
@param theta1 IN end bearing
+---------------+---------------+---------------+---------------+------*/
bool DSpiral2dBase::SetBearingCurvatureLengthBearing
(
double theta0,
double curvature0,
double length,
double theta1
)
    {
    mTheta0 = theta0;
    mCurvature0 = curvature0;
    mTheta1 = theta1;
    double angleChange = theta1 - theta0;
    mLength = length;
    double averageCurvature = angleChange / length;
    double curvature1 = 2.0 * averageCurvature - curvature0;
    mCurvature1 = curvature1;
    return true;
    }



static double sDefaultStrokeAngle = 0.04;
// Return the recommended default stroke angle control 
double DSpiral2dBase::DefaultStrokeAngle ()
    {
    return sDefaultStrokeAngle;
    }

/*-----------------------------------------------------------------*//**
@description Convert distance-along to fraction.
+---------------+---------------+---------------+---------------+------*/
double DSpiral2dBase::DistanceToFraction
(
double distance
) const
    {
    if (mLength == 0.0)
        return 0.0;
    return distance / mLength;
    }

/*-----------------------------------------------------------------*//**
@description Convert fraction to distance-along
+---------------+---------------+---------------+---------------+------*/
double DSpiral2dBase::FractionToDistance
(
double fraction
) const
    {
    return fraction * mLength;
    }

// Return derivatives of the position vector with respect to fraction parameter.
bool DSpiral2dBase::FractionToDerivatives
(
double fraction,
DVec2dR dXdf,
DVec2dR ddXdfdf,
DVec2dR dddXdfdfdf
)
    {
    DSpiral2dDirectEvaluation * fractionalSpiral = dynamic_cast <DSpiral2dDirectEvaluation*> (this);
    if (fractionalSpiral)
        {
        DPoint2d uv;
        return fractionalSpiral->EvaluateAtFraction (fraction, uv, &dXdf, &ddXdfdf, &dddXdfdfdf);
        }
    double scale = mLength;     // each derivative scales by higher power.
    double distance = FractionToDistance (fraction);
    double bearingRadians = DistanceToGlobalAngle (distance);
    double curvature      = DistanceToCurvature (distance);
    double curvatureDerivative = DistanceToCurvatureDerivative (distance);
    double s = sin (bearingRadians);
    double c = cos (bearingRadians);
    double scale2 = scale * scale;
    double scale3 = scale2 * scale;
    // Direct fillin for distance derivatives:
    dXdf.Init (c, s);
    ddXdfdf.Init (-s * curvature, c * curvature);
    double curvature2 = curvature * curvature;
    dddXdfdfdf.Init (-c * curvature2 - s * curvatureDerivative, -s * curvature2 + c * curvatureDerivative);
    // Scale for fraction space
    dXdf.Scale (scale);
    ddXdfdf.Scale (scale2);
    dddXdfdfdf.Scale (scale3);
    return true;
    }

/*-----------------------------------------------------------------*//**
@description BSIVectorIntegrand query function.
+---------------+---------------+---------------+---------------+------*/
int  DSpiral2dBase::GetVectorIntegrandCount () {return 2;}

/*-----------------------------------------------------------------*//**
@description BSIVectorIntegrand query function.
@param distance IN distance parameter
@param pF OUT array of two doubles x,y for integration.
+---------------+---------------+---------------+---------------+------*/
void DSpiral2dBase::EvaluateVectorIntegrand (double distance, double *pF)
    {
    double theta = DistanceToGlobalAngle (distance);
    pF[0] = cos (theta);
    pF[1] = sin (theta);
    }

//! Return an interval count for stroking or integration.
//! The returned count is always even.
//! @param [in] spiral spiral being queried.
//! @param [in] startFraction start of interval to stroke.
//! @param [in] endFraction end of interval to stroke.
//! @param [in] maxRadians max turn between strokes.
//! @param [in] minInterval smallest number of intervals.
size_t DSpiral2dBase::GetIntervalCount
(
DSpiral2dBase &spiral,
double startFraction,
double endFraction,
double maxRadians,
int minInterval,
double maxStrokeLength
)
    {
    static double sMaxRadians = atan (1.0);
    double distance0 = startFraction * spiral.mLength;
    double distance1 = endFraction * spiral.mLength;

    double beta0 = spiral.DistanceToGlobalAngle (distance0);
    double beta1 = spiral.DistanceToGlobalAngle (distance1);

    double kurvature0 = spiral.DistanceToCurvature (distance0);
    double kurvature1 = spiral.DistanceToCurvature (distance1);
    double dbeta = fabs (beta1 - beta0);
    if (kurvature0 * kurvature1 < 0.0)
        {
        dbeta = DoubleOps::Max (dbeta, spiral.mLength * (fabs (kurvature0) + fabs (kurvature1)) * 0.5);
        }
    if (maxRadians <= 0.0)
        maxRadians = DSpiral2dBase::DefaultStrokeAngle ();
    else if (maxRadians > sMaxRadians)
        maxRadians = sMaxRadians;
    //double lengthScale = sqrt (fabs (spiral.mC));
    //double maxLengthStep = sMaxLengthStepFactor * lengthScale;

    int numInterval = (int) (0.9999999999 + dbeta / maxRadians);

    int numIntervalByDistance = maxStrokeLength > 0 ? (int)(0.9999999999 + fabs (distance1 - distance0) / maxStrokeLength) : numInterval;
    if (numIntervalByDistance > numInterval)
        numInterval = numIntervalByDistance;
    if (numInterval < minInterval)
        numInterval = minInterval;
// worst case max interval . .
#define MAX_INTERVAL 1000

    if (numInterval > MAX_INTERVAL)
        numInterval = MAX_INTERVAL;

// smaller max interval for reasonable turn angle.
// this protects against metric maxStrokeLength appearing in UOR data ...
#define TYPICAL_ANGLE_LIMIT 0.45
#define MAX_INTERVAL_TYPICAL_ANGLE 100
    if (fabs (beta0 - beta1) < TYPICAL_ANGLE_LIMIT && numInterval > MAX_INTERVAL_TYPICAL_ANGLE)
        numInterval = MAX_INTERVAL_TYPICAL_ANGLE;
    // Ensure that midpoint of biquadratic does not appera mid-interval ...
    if (numInterval & 0x01)
        numInterval += 1;
    if (numInterval == 0)
        numInterval = 1;    // force a little calculation for short interval.
    return numInterval;
    }

/*-----------------------------------------------------------------*//**
@description Integrate the vector displacements of a spiral over a fractional interval.
 This uses the angles, curvatures, and length.
@param startFraction IN fractional coordinate along length.
@param endFraction IN fractional coordinate along length.
@param maxRadians IN maximum bearing change between computed points.
        A default is used if 0.0 is passed.
@param pDXY INOUT buffer to receive points.
@param pFraction INOUT optional buffer to receive distances.
@param numDXY OUT number of points computed.
@param maxDXY IN maximum number of points to compute.  If zero or negative, only the
        final vector is stored (and pDXY is assumed to be valid for one vector).
@param errorBound OUT estimated bound on error.
@returns false if point buffer exceeded.
+---------------+---------------+---------------+---------------+------*/
bool DSpiral2dBase::StrokeToAnnouncer
(
DSpiral2dBase &spiral,
double startFraction,
double endFraction,
double maxRadians,
AnnounceDoubleDPoint2dR announcer,
double &errorBound,
int minInterval,
double maxStrokeLength
)
    {
    BSIQuadraturePoints gauss;
    int numGauss = 4;
    gauss.InitGauss (numGauss);
    size_t numInterval = GetIntervalCount (spiral, startFraction, endFraction, maxRadians, minInterval, maxStrokeLength);
    double rombergFactor = 1.0 / (pow (2.0, gauss.GetConvergencePower ()) - 1.0);
    DVec2d dxy = DVec2d::From (0,0);
    announcer.Announce (startFraction, dxy);
    errorBound = 0.0;

    double distance0 = startFraction * spiral.mLength;
    double distance1 = endFraction * spiral.mLength;


    double distanceStep = numInterval > 0 ? (distance1 - distance0) / numInterval : 0.0;
    for (size_t interval = 0; interval < numInterval; interval++)
        {
        double u0     = distance0 + interval * distanceStep;
        double u1     = distance0 + (interval + 1) * distanceStep;

        //int numSubInterval = (int)(0.99999 + fabs (u1-u0) / maxLengthStep);
        int numSubInterval = 1;

        if (numSubInterval < 1)
            numSubInterval = 1;
        double du =   (u1 - u0) / numSubInterval;
        for (int subInterval = 0; subInterval < numSubInterval; subInterval++)
            {
            double uA = u0 + subInterval * du;
            double uB = u0 + (subInterval + 1) * du;
            DVec2d result1, result2;
            result1.Zero ();
            result2.Zero ();
            gauss.AccumulateWeightedSums (spiral, uA, uB, (double*)&result1, 1);
            gauss.AccumulateWeightedSums (spiral, uA, uB, (double*)&result2, 2);
            DVec2d dr;
            dr.DifferenceOf (result2, result1);
            // Richardson says we can extrapolate by a small fraction of the step ...
            dxy.SumOf (dxy, result2, 1.0, dr, rombergFactor);
            announcer.Announce (uB / spiral.mLength, dxy);
            // and the update is an overestimate of the error ...
            errorBound += rombergFactor * dr.MaxAbs ();
            }
        }

    return true;
    }

struct CaptureLastFractionUV : AnnounceDoubleDPoint2d
{
double m_fraction;
DVec2d m_xy;
size_t m_count;
CaptureLastFractionUV () : m_count (0){}

void Announce (double fraction, DVec2dCR xy)
    {
    m_fraction = fraction;
    m_xy = xy;
    m_count++;
    }
};

struct CaptureAllFractionUV : AnnounceDoubleDPoint2d
{
bvector<double> m_fraction;
bvector<DVec2d> m_xy;
void Announce (double fraction, DVec2dCR xy)
    {
    m_fraction.push_back (fraction);
    m_xy.push_back (xy);
    }
};

struct CaptureAllFractionXYZ : AnnounceDoubleDPoint2d
{
bvector<double> m_fraction;
bvector<DPoint3d> m_xyz;
void Announce (double fraction, DVec2dCR xy)
    {
    m_fraction.push_back (fraction);
    m_xyz.push_back (DPoint3d::From (xy.x, xy.y, 0.0));
    }
};


bool DSpiral2dBase::Stroke
(
DSpiral2dBase &spiral,
double startFraction,
double endFraction,
double maxRadians,
bvector<DVec2d> &uvPoints,
bvector<double> &fractions,
double &errorBound,
double maxStrokeLength
)
    {
    CaptureAllFractionUV capture;
    bool stat = StrokeToAnnouncer (spiral, startFraction, endFraction, maxRadians, capture, errorBound, 0, maxStrokeLength);
    uvPoints.swap (capture.m_xy);
    fractions.swap (capture.m_fraction);
    return stat && fractions.size () > 0;
    }

bool DSpiral2dBase::Stroke
(
DSpiral2dBase &spiral,
double startFraction,
double endFraction,
double maxRadians,
DVec2d &uv,
double &errorBound,
double maxStrokeLength
)
    {
    CaptureLastFractionUV capture;
    bool stat = StrokeToAnnouncer (spiral, startFraction, endFraction, maxRadians, capture, errorBound, 0, maxStrokeLength);
    uv = capture.m_xy;
    return stat && capture.m_count > 0;
    }




struct DSpiral2dBaseClosestPointSearcher : public FunctionRRToRRD
{
DSpiral2dBase & mSpiral;
DPoint3d        mSpacePoint;
Transform       mSpiralToWorld;
double          mDistance;
double          mSpiralFraction;
DPoint3d        mSpiralPoint;

DVec2d IntegrateBetweenFractions
(
double fractionA,
double fractionB
)
    {
    DVec2d uv;
    double error;
    DSpiral2dBase::Stroke (mSpiral, fractionA, fractionB, 0.0, uv, error);
    return uv;
    }

DPoint3d PointAtFraction
(
double fraction
)
    {
    DVec2d uv;
    double error;
    DSpiral2dBase::Stroke (mSpiral, 0.0, fraction, 0.0, uv, error);
    DPoint3d X;
    mSpiralToWorld.Multiply (X, uv.x, uv.y, 0.0);
    return X;
    }

// Virtual function for Newton steps.
// @param u IN curve parameter
// @param v IN dummy 2nd var.
// @param f OUT dot product of curve tangent with point-to-curve vector
// @param g OUT dummy 2nd funtion -- g = v
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
    DPoint3d X;
    DVec3d dX, ddX, dddX;
    DVec2d Y = IntegrateBetweenFractions (0.0, u);
    DVec2d dY, ddY, dddY;
    mSpiral.FractionToDerivatives (u, dY, ddY, dddY);
    mSpiralToWorld.Multiply (X, Y.x, Y.y, 0.0);
    mSpiralToWorld.MultiplyMatrixOnly (dX, dY.x, dY.y, 0.0);
    mSpiralToWorld.MultiplyMatrixOnly (ddX, ddY.x, ddY.y, 0.0);
    mSpiralToWorld.MultiplyMatrixOnly (dddX, dddY.x, dddY.y, 0.0);

    DVec3d U;
    U.DifferenceOf (X, mSpacePoint);
    f = U.DotProduct (dX);
    dfdu = dX.DotProduct (dX) + U.DotProduct (ddX);
    dfdv = 0.0;
    g = v;
    dgdu = 0.0;
    dgdv = 1.0;
    return true;
    }



DSpiral2dBaseClosestPointSearcher
    (
    DSpiral2dBase &spiral,
    TransformCR spiralToWorld,
    DPoint3dCR spacePoint
    )
    : mSpiral (spiral),
      mSpiralToWorld (spiralToWorld),
      mSpacePoint (spacePoint)
    {
    mDistance = DBL_MAX;
    mSpiralFraction = 0.0;
    mSpiralPoint = spacePoint;
    }

void ResetPoint (DPoint3d& xyz, double fraction)
    {
    mDistance = xyz.Distance (mSpacePoint);
    mSpiralPoint = xyz;
    mSpiralFraction = fraction;
    }
bool TestPoint (DPoint3d& xyz, double fraction)
    {
    double distance = xyz.Distance (mSpacePoint);
    if (distance < mDistance)
        {
        mDistance = distance;
        mSpiralPoint = xyz;
        mSpiralFraction = fraction;
        return true;
        }
    return false;
    }

// Simple lookup of search results.
bool GetResults
(
DPoint3dR spiralPoint,
double &spiralFraction,
double &distance
)
    {
    distance = mDistance;
    spiralPoint = mSpiralPoint;
    spiralFraction = mSpiralFraction;
    return mDistance < DBL_MAX;
    }

static bool Compute1dNewtonStep (double f, double df, double &du)
    {
    return DoubleOps::SafeDivide (du, f, df, 0.0);
    }

// Look up the chordal search result.
// If chordal search fraction is within limitA..limitB, run Newton to improve the closest point.
bool GetPolishedResults
(
double limitA,
double limitB,
DPoint3dR spiralPoint,
double &spiralFraction,
double &distance
)
    {
    static double s_endFractionTol = 1.0e-15;  // really tight condition for "are we at an endpoint"
                                            // true zero would probably be ok -- the points were set up by assignment.
    if (     GetResults (spiralPoint, spiralFraction, distance))
        {
        double fraction = spiralFraction;
        DPoint3d X = PointAtFraction (fraction);
        ResetPoint (X, fraction);
        NewtonIterationsRRToRR iterator = NewtonIterationsRRToRR (1.0e-10);
        // if at an endpoint, run the first iteration directly -- don't do more if it
        // leads outward.
        double v = 0;   // artificial variable becasue newton solver wants bivariate
        if (fabs (fraction - limitA) < s_endFractionTol ||fabs (fraction - limitB) < s_endFractionTol)
            {
            double f,g, dfdu, dfdv,dgdu, dgdv, du ;
            if (!EvaluateRRToRRD (fraction, v, f, g, dfdu, dfdv, dgdu, dgdv)
              || !Compute1dNewtonStep(f, dfdu, du))
                return false;
            fraction -= du;
            if ((fraction - limitA) * (fraction - limitB) > 0.0)
                return false; // the iteration is leading outside.
            }
        if (iterator.RunNewton (fraction, v, *this))
            {
            X = PointAtFraction (fraction);
            TestPoint (X, fraction);
            return GetResults (spiralPoint, spiralFraction, distance);
            }
        }
    return false;
    }

bool SearchStrokesInFractionInterval
(
double startFraction,
double endFraction,
DVec2d startUV
)
    {
#define MAX_SEARCH_POINT 5000
    bvector<DVec2d> strokeUV;
    bvector<double> strokeFraction;

    double strokeError;
    bool stat = DSpiral2dBase::Stroke (mSpiral, startFraction, endFraction, 0.0,
                strokeUV, strokeFraction, strokeError);
    if (!stat)
        return false;

    DPoint3d xyzA, xyzB, xyzC;
    mSpiralToWorld.Multiply (xyzA, startUV.x, startUV.y, 0.0);
    TestPoint (xyzA, startFraction);

    for (size_t i = 1; i < strokeUV.size (); i++, xyzA = xyzB)
        {
        mSpiralToWorld.Multiply (xyzB, startUV.x + strokeUV[i].x, startUV.y + strokeUV[i].y, 0.0);
        DVec3d U, V;
        U.DifferenceOf (xyzA, xyzB);
        V.DifferenceOf (mSpacePoint, xyzB);
        TestPoint (xyzB, strokeFraction[i]);
        double UdotV = U.DotProduct (V);
        double UdotU = U.MagnitudeSquared ();
        if (UdotV > 0.0)
            {

            if (UdotV < UdotU)
                {
                double s = UdotV / UdotU;   // strict lessThan test makes this safe
                xyzC.SumOf (xyzB, U, s);
                double fractionC = strokeFraction[i] + s * (strokeFraction[i-1] - strokeFraction[i]);
                TestPoint (xyzC, fractionC);
                }
            }
        }

    return true;
    }

};

bool DSpiral2dBase::ClosestPoint
(
DSpiral2dBase &spiral,
double startFraction,
double endFraction,
TransformCR spiralToWorld,
DPoint3dCR spacePoint,
DPoint3dR spiralPoint,
double&   spiralFraction,
double&   minDistance
)
    {
    DSpiral2dBaseClosestPointSearcher searcher (spiral, spiralToWorld, spacePoint);
    bool stat = false;
    if (startFraction * endFraction < 0.0)
        {
        // fraction0 is within the interval. Search strokes to each end.
        DVec2d uv0;
        uv0.Zero ();
        stat = searcher.SearchStrokesInFractionInterval (0.0, endFraction, uv0)
                && searcher.SearchStrokesInFractionInterval (0.0, startFraction, uv0);
        }
    else
        {
        // fractions have same sign.  Work from the one closer to the origin ...
        if (fabs (startFraction) < fabs (endFraction))
            searcher.SearchStrokesInFractionInterval (startFraction, endFraction, searcher.IntegrateBetweenFractions (0.0, startFraction));
        else
            searcher.SearchStrokesInFractionInterval (endFraction, startFraction, searcher.IntegrateBetweenFractions (0.0, endFraction));
        }
    searcher.GetPolishedResults (startFraction, endFraction, spiralPoint, spiralFraction, minDistance);
    return true;
    }

/*-----------------------------------------------------------------*//**
@description test if a length-from-inflection and final radius combination is "small enough" for reasonable use.
@param 
+---------------+---------------+---------------+---------------+------*/
bool DSpiral2dBase::IsValidRLCombination(double lengthFromInflection, double radius, int spiralType)
    {
    bool doTest = spiralType == DSpiral2dBase::TransitionType_Unknown
                || spiralType == DSpiral2dBase::TransitionType_Czech
                || spiralType == DSpiral2dBase::TransitionType_Italian;
    if (doTest)
        return     fabs(lengthFromInflection) < fabs(2.0 * radius);
    return true;
    }

/*-----------------------------------------------------------------*//**
@description convert distance-along to globla angle.
+---------------+---------------+---------------+---------------+------*/
double DSpiral2dBase::DistanceToGlobalAngle (double distance) const
    {
    return mTheta0 + DistanceToLocalAngle (distance);
    }


// Specialize spiral for CLOTHOID ....
DSpiral2dClothoid::DSpiral2dClothoid () : DSpiral2dBase () {}

DSpiral2dBaseP DSpiral2dClothoid::Clone () const
    {
    DSpiral2dClothoid *pClone = new DSpiral2dClothoid ();
    pClone->CopyBaseParameters (this);
    return pClone;
    }

double DSpiral2dClothoid::DistanceToLocalAngle (double distance) const
    {
    double u = DistanceToFraction (distance);
    return mLength * u * (mCurvature0 + u * 0.5 * (mCurvature1 - mCurvature0));
    }
double DSpiral2dClothoid::DistanceToCurvature (double distance) const
    {
    double f = DistanceToFraction (distance);
    return mCurvature0 + f * (mCurvature1 - mCurvature0);
    }
double DSpiral2dClothoid::DistanceToCurvatureDerivative (double distance) const
    {
    if (mLength == 0.0)
        return 0.0;
    return (mCurvature1 - mCurvature0) / mLength;
    }


// Specialize spiral for BIQUADRATIC ....
DSpiral2dBiQuadratic::DSpiral2dBiQuadratic () : DSpiral2dBase () {}
DSpiral2dBaseP DSpiral2dBiQuadratic::Clone () const
    {
    DSpiral2dBiQuadratic *pClone = new DSpiral2dBiQuadratic ();
    pClone->CopyBaseParameters (this);
    return pClone;
    }
/*-----------------------------------------------------------------*//**
@description convert distance-along to local angle.
+---------------+---------------+---------------+---------------+------*/
double DSpiral2dBiQuadratic::DistanceToLocalAngle (double distance) const
    {
    double u = DistanceToFraction (distance);
    if (u <= 0.5)
        return mLength * u * (mCurvature0 + (2.0 / 3.0) * u * u * (mCurvature1 - mCurvature0));
    else
        {
        double Kbar = 0.5 * (mCurvature0 + mCurvature1);
        double Kdelta = mCurvature1 - mCurvature0;
        double v = 1.0 - u;
        return mLength * (Kbar - v * (mCurvature1 - (2.0 * v * v / 3.0) * Kdelta));
        }
    }

/*-----------------------------------------------------------------*//**
@description convert distance-along to curvature.
+---------------+---------------+---------------+---------------+------*/
double DSpiral2dBiQuadratic::DistanceToCurvature (double distance) const
    {
    double u = DistanceToFraction (distance);
    double f;
    if (u <= 0.5)
        f = 2.0 * u * u;
    else
        {
        double v = 1.0 - u;
        f = 1.0 - 2.0 * v * v;
        }
    return mCurvature0 + f * (mCurvature1 - mCurvature0);
    }

double DSpiral2dBiQuadratic::DistanceToCurvatureDerivative (double distance) const
    {
    if (mLength == 0.0)
        return 0.0;
    double u = DistanceToFraction (distance);
    double dfdu;
    if (u <= 0.5)
        dfdu = 4.0 * u;
    else
        {
        double v = 1.0 - u;
        dfdu = 4.0 * v;
        }
    return dfdu * (mCurvature1 - mCurvature0) / mLength;
    }


// Specialize spiral for BLOSS ....
DSpiral2dBloss::DSpiral2dBloss () : DSpiral2dBase () {}
DSpiral2dBaseP DSpiral2dBloss::Clone () const
    {
    DSpiral2dBloss *pClone = new DSpiral2dBloss ();
    pClone->CopyBaseParameters (this);
    return pClone;
    }
double DSpiral2dBloss::DistanceToLocalAngle (double distance) const
    {
    double u = DistanceToFraction (distance);
    return   mLength * u * (mCurvature0
           + u * u * (1.0 - 0.5 * u) * (mCurvature1 - mCurvature0));
    }

double DSpiral2dBloss::DistanceToCurvature (double distance) const
    {
    double u = DistanceToFraction (distance);
    double f = u * u * (3.0 - 2.0 * u);
    return mCurvature0 + f * (mCurvature1 - mCurvature0);
    }

double DSpiral2dBloss::DistanceToCurvatureDerivative (double distance) const
    {
    if (mLength == 0.0)
        return 0.0;
    double u = DistanceToFraction (distance);
    double dfdu = 6.0 * u * (1.0 - u);
    return dfdu * (mCurvature1 - mCurvature0) / mLength;
    }

// Specialize spiral for SINE ....
DSpiral2dSine::DSpiral2dSine () : DSpiral2dBase () {}
DSpiral2dBaseP DSpiral2dSine::Clone () const
    {
    DSpiral2dSine *pClone = new DSpiral2dSine ();
    pClone->CopyBaseParameters (this);
    return pClone;
    }

double DSpiral2dSine::DistanceToLocalAngle (double distance) const
    {
    double u = DistanceToFraction (distance);
    double B = 1.0 / (4.0 * M_PI * M_PI);
    return mLength *
        (mCurvature0 * u  + (mCurvature1 - mCurvature0) * (u * u * 0.5 + B * (cos (2.0 * M_PI * u) - 1.0)));
    }

double DSpiral2dSine::DistanceToCurvature (double distance) const
    {
    double u = DistanceToFraction (distance);
    double A = 1.0 / (2.0 * M_PI);

    return mCurvature0 + (mCurvature1 - mCurvature0) * (u - A * sin (2.0 * M_PI * u));
    }

double DSpiral2dSine::DistanceToCurvatureDerivative (double distance) const
    {
    if (mLength == 0.0)
        return 0.0;
    double u = DistanceToFraction (distance);
    double dfdu = 1.0 - cos (2.0 * M_PI * u);
    return dfdu * (mCurvature1 - mCurvature0) / mLength;
    }

// Specialize spiral for COSINE ....
DSpiral2dCosine::DSpiral2dCosine () : DSpiral2dBase () {}

DSpiral2dBaseP DSpiral2dCosine::Clone () const
    {
    DSpiral2dCosine *pClone = new DSpiral2dCosine ();
    pClone->CopyBaseParameters (this);
    return pClone;
    }

double DSpiral2dCosine::DistanceToLocalAngle (double distance) const
    {
    double u = DistanceToFraction (distance);
    double B = 1.0 / (2.0 * M_PI);
    return mLength *
        ((mCurvature0 + mCurvature1) * 0.5 * u   - B * (mCurvature1 - mCurvature0) * sin (M_PI * u));
    }

double DSpiral2dCosine::DistanceToCurvature (double distance) const
    {
    double u = DistanceToFraction (distance);
    return 0.5 * ((mCurvature0 + mCurvature1)  - (mCurvature1 - mCurvature0) * cos (M_PI * u));
    }

double DSpiral2dCosine::DistanceToCurvatureDerivative (double distance) const
    {
    if (mLength == 0.0)
        return 0.0;
    double u = DistanceToFraction (distance);
    double dfdu = 0.5 * M_PI * sin ( M_PI * u);
    return dfdu * (mCurvature1 - mCurvature0) / mLength;
    }
// Specialize spiral for VIENNESE .....
DSPiral2dViennese::DSPiral2dViennese
    (
    double cant,
    double h,
    double e
    ) : DSpiral2dBase ()
    {
    mPhi = cant * h / e;
    mCant = cant;
    mH = h;
    mE = e;
    }

DSpiral2dBaseP DSPiral2dViennese::Clone () const
    {
    DSPiral2dViennese *pClone = new DSPiral2dViennese (mCant, mH, mE);
    pClone->CopyBaseParameters (this);
    return pClone;
    }


double DSPiral2dViennese::DistanceToLocalAngle (double distance) const
    {
    double u = DistanceToFraction (distance);
    //double B = 1.0 / M_PI;
    double u2 = u * u;
    double u3 = u2 * u;
    double u5 = u2 * u3;
    double v = 1.0 - u;
    double v3 = v * v * v;
    double L2 = mLength * mLength;
    return mLength *
            (  mCurvature0 * u
            + 140.0 * mPhi * u3 * v3 / L2
            + 0.5 * (mCurvature1 - mCurvature0) * u5 * (14.0 - u * (28.0 - u * (20.0 - 5.0 * u)))
            );
    }

double DSPiral2dViennese::DistanceToCurvature (double distance) const
    {
    double u = DistanceToFraction (distance);
    double L2 = mLength * mLength;
    double u2 = u * u;
    double u4 = u2 * u2;
    return mCurvature0
          + 420.0 * (mPhi / L2) * u2 * (1.0 - u * (4.0 - u * (5.0 - u * 2.0)))
          + (mCurvature1 - mCurvature0)
                * u4 * (35.0 - u * (84.0 - u * (70.0 - u * 20.0)));
    }

double DSPiral2dViennese::DistanceToCurvatureDerivative (double distance) const
    {
    if (mLength == 0.0)
        return 0.0;
    double u = DistanceToFraction (distance);
    double L2 = mLength * mLength;
    double u3 = u * u * u;
    double df1du = 420.0 * (mPhi / L2) *  u * (2.0 - u * (12.0 - u * (20.0 - u * 10.0)));
    double df2du = u3 * (140.0 - u * (420.0 - u * (420.0 - u * 140.0)));
    return (df1du + df2du * (mCurvature1 - mCurvature0)) / mLength;
    }

// Specialize spiral for VIENNESE with caller-supplied scale factors for spiral and roll terms ....
DSpiral2dBaseP DSPiral2dWeightedViennese::Clone () const
    {
    DSPiral2dWeightedViennese *pClone = new DSPiral2dWeightedViennese (mCant, mH, mE, mF0, mF1);
    pClone->CopyBaseParameters (this);
    return pClone;
    }

DSPiral2dWeightedViennese::DSPiral2dWeightedViennese
    (
    double cant,
    double h,
    double e,
    double f0,
    double f1
    ) : DSpiral2dBase ()
    {
    mPhi = cant * h / e;
    mCant = cant;
    mH = h;
    mE = e;
    mF0 = f0;
    mF1 = f1;
    }

double DSPiral2dWeightedViennese::DistanceToLocalAngle (double distance) const
    {
    double u = DistanceToFraction (distance);
    //double B = 1.0 / M_PI;
    double u2 = u * u;
    double u3 = u2 * u;
    double u5 = u2 * u3;
    double v = 1.0 - u;
    double v3 = v * v * v;
    double L2 = mLength * mLength;
    return mLength *
            (  mCurvature0 * u
            + mF1 * 140.0 * mPhi * u3 * v3 / L2
            + mF0 * 0.5 * (mCurvature1 - mCurvature0) * u5 * (14.0 - u * (28.0 - u * (20.0 - 5.0 * u)))
            );
    }

double DSPiral2dWeightedViennese::DistanceToCurvature (double distance) const
    {
    double u = DistanceToFraction (distance);
    double L2 = mLength * mLength;
    double u2 = u * u;
    double u4 = u2 * u2;
    return mCurvature0
          + mF1 * 420 * (mPhi / L2) * u2 * (1.0 - u * (4.0 - u * (5.0 - u * 2.0)))
          + mF0 * (mCurvature1 - mCurvature0)
                * u4 * (35.0 - u * (84.0 - u * (70.0 - u * 20)));
    }

double DSPiral2dWeightedViennese::DistanceToCurvatureDerivative (double distance) const
    {
    if (mLength == 0.0)
        return 0.0;
    double u = DistanceToFraction (distance);
    double L2 = mLength * mLength;
    double u3 = u * u * u;
    double df1du = 420.0 * (mPhi / L2) *  u * (2.0 - u * (12.0 - u * (20.0 - u * 10.0)));
    double df2du = u3 * (140.0 - u * (420.0 - u * (420.0 - u * 140.0)));
    return (mF1 * df1du + mF0 * df2du * (mCurvature1 - mCurvature0)) / mLength;
    }


bool DSpiral2dBase::LineSpiralArcSpiralLineTransition
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
)
    {
    DVec3d tangentA, tangentB;
    tangentA.DifferenceOf (lineLineIntersection, lineAPoint);
    tangentA.z = 0.0;
    tangentB.DifferenceOf (lineLineIntersection, lineBPoint);
    tangentB.z = 0.0;
    double thetaA = Angle::Atan2 (tangentA.y, tangentA.x);
    double thetaB = Angle::Atan2 (tangentB.y, tangentB.x);

    DVec3d unitAX, unitBX, unitAY, unitBY;
    unitAX.Normalize (tangentA);
    unitBX.Normalize (tangentB);
    unitAY.UnitPerpendicularXY (unitAX);
    unitBY.UnitPerpendicularXY (unitBX);

    double thetaAB = tangentA.AngleToXY (tangentB);
    double sideA = thetaAB < 0.0 ? 1.0 : -1.0;
    double sideB = - sideA;
    double radiusA = sideA * fabs (radius);
    double radiusB = sideB * fabs (radius);
    double curvatureA = 1.0 / radiusA;
    double curvatureB = 1.0 / radiusB;
    // In local coordiantes, follow each spiral staring in +X direction ...
    spiralA.SetBearingCurvatureLengthCurvature (0.0, 0.0, lengthA, curvatureA);
    spiralB.SetBearingCurvatureLengthCurvature (0.0, 0.0, lengthB, curvatureB);
    DVec2d spiralChordA, spiralChordB;
    double error0, error1;
    DSpiral2dBase::Stroke (spiralA, 0.0, 1.0, DSpiral2dBase::DefaultStrokeAngle (), spiralChordA, error0);
    DSpiral2dBase::Stroke (spiralB, 0.0, 1.0, DSpiral2dBase::DefaultStrokeAngle (), spiralChordB, error1);

    // From the end of spiral, step away to arc center ...
    double sA = spiralChordA.x - radiusA * sin (spiralA.mTheta1);
    double tA = spiralChordA.y + radiusA * cos (spiralA.mTheta1);

    double sB = spiralChordB.x - radiusB * sin (spiralB.mTheta1);
    double tB = spiralChordB.y + radiusB * cos (spiralB.mTheta1);

    double uA, uB;  // unknown distances from circle intersection to line-spiral tangency points,
                    // each measured from line intersection in respective direction (unitA, unitB).
    // In world space, starting from intersection point Q, we move to the center indicated by each spiral
    // by stepping along its line to the tangency point, then by distances s and t tangent and perpendicular
    // circleCenter = lineIntersection + (uA + sA) unitAX + tA unitAY = lineIntersection + (uB + sB) unitBX + tB unitBY
    //  [unitAX.x   -unitBX.x][uA] = sB*untiBX.x +tB*unitBY.x - sA*unitAX.x -tA*unitAY.x
    //  [unitAX.y   -unitBX.y][uB] =  (sim with y for x)
    DVec3d vectorA, vectorB;
    vectorA.SumOf (unitAX, sA, unitAY, tA);
    vectorB.SumOf (unitBX, sB, unitBY, tB);

    bool stat = bsiSVD_solve2x2 (&uA, &uB,
                unitAX.x, -unitBX.x,
                unitAX.y, -unitBX.y,
                vectorB.x - vectorA.x,
                vectorB.y - vectorA.y) ? true : false;
    if (stat)
        {
        lineToSpiralA.SumOf (lineLineIntersection, unitAX, uA);
        lineToSpiralB.SumOf (lineLineIntersection, unitBX, uB);
        spiralAToArc.SumOf (lineToSpiralA, unitAX, spiralChordA.x, unitAY, spiralChordA.y);
        spiralBToArc.SumOf (lineToSpiralB, unitBX, spiralChordB.x, unitBY, spiralChordB.y);
        spiralA.mTheta0 += thetaA;
        spiralA.mTheta1 += thetaA;
        spiralB.mTheta0 += thetaB;
        spiralB.mTheta1 += thetaB;
        arc.center.SumOf (lineLineIntersection, unitAX, uA + sA, unitAY, tA);
        arc.vector0.DifferenceOf (spiralAToArc, arc.center);
        arc.vector90.Init (-arc.vector0.y, arc.vector0.x, 0.0);
        DVec3d vectorC;
        vectorC.DifferenceOf (spiralBToArc, arc.center);
        arc.start = 0.0;
        arc.sweep = Angle::Atan2 (arc.vector90.DotProduct (vectorC), arc.vector0.DotProduct (vectorC));
        }
    return stat;
    }

//!
//! @description compute 2 spirals.  (Only xy parts of inputs are used)
//   First spiral begins exactly at the start point and aims at the shoulder
//   Second spiral ends somewhere on the line from shoulder to target.
//! @param [in] startPoint start point
//! @param [in] shoulderPoint target point for first and last tangents
//! @param [in] targetPoint target point for last tangent
//! @param [in,out] pSpiralA  On input, a spiral of the desired type.  On output
//!        all fields are set.
//! @param [in,out] pSpiralB  On input, a spiral of the desired type.  On output
//!        all fields are set.
//! @returns false if unable to construct
//!
bool DSpiral2dBase::SymmetricPointShoulderTargetTransition
(
DPoint2dCR startPoint,
DPoint2dCR shoulderPoint,
DPoint2dCR targetPoint,
DSpiral2dBase &spiralA,
DSpiral2dBase &spiralB,
DPoint2dR    junctionPoint,
DPoint2dR    endPoint
)
    {
    auto vectorAB = DVec2d::FromStartEnd (startPoint, shoulderPoint);
    auto vectorBC = DVec2d::FromStartEnd (shoulderPoint, targetPoint);
    double refLength = vectorAB.Magnitude ();

    auto bearingAB = Angle::Atan2 (vectorAB.y, vectorAB.x);
    auto turnRadians = vectorAB.AngleTo (vectorBC);
    auto bearingB = bearingAB + 0.5 * turnRadians;
    auto bearingBC    = bearingAB + turnRadians;
    DVec2d jointTangent = DVec2d::From (cos (bearingB), sin(bearingB));
    // Make a spiral of known length.  Appropriate scaling will make it end on the bisector line.
    spiralA.SetBearingCurvatureLengthBearing (bearingAB, 0.0, refLength, bearingB);
    DVec2d spiralChord;
    double error;
    DSpiral2dBase::Stroke (spiralA, 0.0, 1.0, DSpiral2dBase::DefaultStrokeAngle (), spiralChord, error);
    // (spiralChord *s - vectorAB) dot jointTangent = 0
    auto s = DoubleOps::ValidatedDivideParameter (vectorAB.DotProduct (jointTangent), spiralChord.DotProduct (jointTangent));
    if (s.IsValid ())
        {
        double length = refLength * s;
        spiralA.SetBearingCurvatureLengthBearing (bearingAB, 0.0, length, bearingB);
        spiralB.SetBearingAndCurvatureLimits (bearingB, spiralA.mCurvature1, bearingBC, 0.0);
        DVec2d chordAB, chordBC;
        double errorAB, errorBC;
        DSpiral2dBase::Stroke (spiralA, 0.0, 1.0, DSpiral2dBase::DefaultStrokeAngle (), chordAB, errorAB);
        DSpiral2dBase::Stroke (spiralB, 0.0, 1.0, DSpiral2dBase::DefaultStrokeAngle (), chordBC, errorBC);
        junctionPoint = startPoint + DVec2d::From (chordAB);
        endPoint = junctionPoint + DVec2d::From (chordBC);
        return true;
        }
    return false;

    }
bool DSpiral2dBase::SymmetricLineSpiralSpiralLineTransition
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
)
    {
    DVec3d tangentA, tangentB;  // outward tangents from lineLineIntersection . . .
    DVec3d forwardB;
    forwardB.DifferenceOf (lineBPoint, lineLineIntersection);
    forwardB.z = 0.0;
    tangentA.DifferenceOf (lineLineIntersection, lineAPoint);
    tangentA.z = 0.0;
    tangentB.DifferenceOf (lineLineIntersection, lineBPoint);
    tangentB.z = 0.0;
    double dTheta = 0.5 * tangentA.AngleToXY (forwardB);    // Angle change by each spiral piece alone
    double thetaA = Angle::Atan2 (tangentA.y, tangentA.x);
    double thetaB = Angle::Atan2 (tangentB.y, tangentB.x);

    DVec3d unitAX, unitBX, unitAY, unitBY;
    unitAX.Normalize (tangentA);
    unitBX.Normalize (tangentB);
    unitAY.UnitPerpendicularXY (unitAX);
    unitBY.UnitPerpendicularXY (unitBX);

    // In local coordiantes, follow each spiral staring in +X direction ...
    spiralA.SetBearingCurvatureLengthBearing (0.0, 0.0, length, dTheta);
    spiralB.SetBearingCurvatureLengthBearing (0.0, 0.0, length, -dTheta);
    if (length <= 0.0)
        return false;
    DVec2d spiralChord;
    double error;
    DSpiral2dBase::Stroke (spiralA, 0.0, 1.0, DSpiral2dBase::DefaultStrokeAngle (), spiralChord, error);
    double ex = spiralChord.y * tan (dTheta);
    lineToSpiralA.SumOf (lineLineIntersection, unitAX, -(spiralChord.x + ex));
    lineToSpiralB.SumOf (lineLineIntersection, unitBX, -(spiralChord.x + ex));
    spiralToSpiral.SumOf (lineLineIntersection, unitAX, -ex, unitAY, spiralChord.y);
    
    spiralA.mTheta0 += thetaA;
    spiralA.mTheta1 += thetaA;
    spiralB.mTheta0 += thetaB;
    spiralB.mTheta1 += thetaB;
    junctionRadius = DoubleOps::ValidatedDivide (1.0, spiralA.mCurvature1, 0.0);
    return true;
    }

// Critical measurements on a spiral relative to its initial osculating circle.
// circle has center at (0,baseRadius)
// spiral starts along x axis, transitions to straight line
// polar coordinates are measured from circle center with polar angle zero in negative y direction.
// Spiral departs from circle at polar angle 0.
// Spiral ends at some other distance from center.
// Draw tangent line at spiral end.
// Projection of circle center on this line is the tangency point.
struct PolarizedSpiral
{
double mBaseRadius;         // osculating circle radius
double mSpiralLength;       // measured along spiral
double mPolarEndRadius;     // circle center to spiral end
double mPolarEndAngle;      // in circle polar system.
double mPolarTangencyAngle; // in circle polar system.
double mTangencyRadius;     // distance from
double mSpiralSweep;        // angle turned by spiral.
double mSpiralCurvature0;
double mSpiralCurvature1;

DPoint3d mCenter;
DVec3d mLocalX;
DVec3d mLocalY;
double mLocalXAngle;


PolarizedSpiral
(
double baseRadius,
double spiralLength,
DSpiral2dBase &spiral,
DPoint3d circleCenter,
DVec3d &localX,
DVec3d &localY
)
    {
    mCenter     = circleCenter;
    mBaseRadius = baseRadius;
    mSpiralLength = spiralLength;

    // Make a spiral starting at origin with bearing 0...
    DVec2d chord;
    double error;

    DoubleOps::SafeDivide (mSpiralCurvature0, 1.0, baseRadius, 0.0);
    mSpiralCurvature1 = 0.0;

    spiral.SetBearingCurvatureLengthCurvature ( 0.0, mSpiralCurvature0, spiralLength, mSpiralCurvature1);
    DSpiral2dBase::Stroke (spiral, 0.0, 1.0, DSpiral2dBase::DefaultStrokeAngle (), chord, error);
    DRay3d ray;
    ray.origin.Init (chord.x, chord.y, 0.0);
    ray.direction.Init (cos (spiral.mTheta1), sin (spiral.mTheta1), 0.0);
    DPoint3d center;
    center.Init (0.0, baseRadius, 0.0);
    DPoint3d projection;
    double parameter;
    DVec3d tangencyVector;
    ray.ProjectPointUnbounded (projection, parameter, center);
    tangencyVector.DifferenceOf (projection, center);
    mPolarTangencyAngle = Angle::Atan2 (tangencyVector.x, -tangencyVector.y);
    mTangencyRadius = projection.Distance (center);
    // vector from circle center to spiral end ...
    DVec2d radialVector;
    radialVector.Init (chord.x, baseRadius - chord.y);
    mPolarEndRadius  = radialVector.Magnitude ();
    mPolarEndAngle   = Angle::Atan2 (radialVector.x, radialVector.y);
    mSpiralSweep     = spiral.mTheta1 - spiral.mTheta0;
    mLocalX = localX;
    mLocalY = localY;
    mLocalXAngle = Angle::Atan2 (localX.y, localX.x);
    }

DPoint3d LocalPolarToGlobal (double r, double theta)
    {
    DPoint3d xyz;
    xyz.SumOf (mCenter, mLocalX, r * cos (theta), mLocalY, r * sin (theta));
    return xyz;
    }

// Assign start point, spiral parameters, and end point for spiral with given tangency refernece and direction (plus or minus 1)
void PlaceAtTangency
(
double thetaT,      // positive X to tangency line.
double direction,
DPoint3dR xyz0,
DSpiral2dBase &spiral,
DPoint3dR xyz1
)
    {
    double theta0 = thetaT - direction * mPolarTangencyAngle;
    double theta1 = theta0 + direction * mPolarEndAngle;
    xyz0 = LocalPolarToGlobal (mBaseRadius, theta0);
    xyz1 = LocalPolarToGlobal (mPolarEndRadius, theta1);
    double bearing0 = theta0 + mLocalXAngle + direction * msGeomConst_piOver2;
    spiral.SetBearingCurvatureLengthCurvature (bearing0, direction * mSpiralCurvature0, mSpiralLength, mSpiralCurvature1);
    }
};


/*-----------------------------------------------------------------*//**
@description compute spirals and line segment to make an arc-to-arc transition
        with specified length of the spiral parts.
@param centerA IN circle A center.
@param radiusA IN circle A radius.
@param lengthA IN length of spiral leaving A.
@Param selectALeftOfAB IN true to
@param centerB IN circle B center.
@param radiusB IN circle B radius.
@param lengthB IN length of spiral leaving B.
@param collector IN object to receive transition candidates.
        This may be called multiple times.
@returns number of solutions announced to collector.
+---------------+---------------+---------------+---------------+------*/
int DSpiral2dBase::ArcSpiralLineSpiralArcTransition
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
)
    {
    DVec3d localX, localY;

    localX.NormalizedDifference (centerB, centerA);
    localY.Init (-localX.y, localX.x, 0.0);
    double distanceAB = centerA.Distance (centerB);
    DPoint3d xyz0A, xyz1A, xyz0B, xyz1B;
    radiusA = fabs (radiusA);
    radiusB = fabs (radiusB);
    lengthA = fabs (lengthA);
    lengthB = fabs (lengthB);
    PolarizedSpiral polarDataA (radiusA, lengthA, spiralA, centerA, localX, localY);
    PolarizedSpiral polarDataB (radiusB, lengthB, spiralB, centerB, localX, localY);

    int numSolution = 0;
    double lambda;  // radiusB/radiusA = nondimensional radius
    double delta;   // distanceAB / radiusA = nondimensional distance

    if (   !DoubleOps::SafeDivide (lambda, polarDataB.mTangencyRadius, polarDataA.mTangencyRadius, 0.0)
        || !DoubleOps::SafeDivide (delta,  distanceAB, polarDataA.mTangencyRadius, 0.0))
        return false;
    // unsigned altitudes for tangency points for "inside" tangents.
    double c0, s0;
    if (DoubleOps::SafeDivide (c0, 1.0 + lambda, delta, 0.0)
        && fabs (c0) < 1.0)
        {
        s0 = sqrt (1.0 - c0 * c0);
        double alphaB = Angle::Atan2 (s0, -c0);
        double alphaA = Angle::Atan2 (-s0, c0);
        polarDataA.PlaceAtTangency (alphaA, 1.0, xyz0A, spiralA, xyz1A);
        polarDataB.PlaceAtTangency (alphaB, 1.0, xyz0B, spiralB, xyz1B);
        collector.Collect
                    (
                    centerA, xyz0A, spiralA, xyz1A,
                    centerB, xyz0B, spiralB, xyz1B
                    );
        numSolution++;

        polarDataA.PlaceAtTangency (-alphaA, -1.0, xyz0A, spiralA, xyz1A);
        polarDataB.PlaceAtTangency (-alphaB, -1.0, xyz0B, spiralB, xyz1B);
        collector.Collect
                    (
                    centerA, xyz0A, spiralA, xyz1A,
                    centerB, xyz0B, spiralB, xyz1B
                    );

        numSolution++;
        }

    double s1, cc1;
    if (DoubleOps::SafeDivide (s1, lambda - 1.0, delta, 0.0)
        && (cc1 = 1.0 - s1 * s1) > 0.0)
        {
        double c1 = sqrt (cc1);
        double alpha = Angle::Atan2 (c1, -s1);
        polarDataA.PlaceAtTangency (alpha,-1.0, xyz0A, spiralA, xyz1A);
        polarDataB.PlaceAtTangency (alpha, 1.0, xyz0B, spiralB, xyz1B);
        collector.Collect
                    (
                    centerA, xyz0A, spiralA, xyz1A,
                    centerB, xyz0B, spiralB, xyz1B
                    );
        numSolution++;

        polarDataA.PlaceAtTangency (-alpha,  1.0, xyz0A, spiralA, xyz1A);
        polarDataB.PlaceAtTangency (-alpha, -1.0, xyz0B, spiralB, xyz1B);
        collector.Collect
                    (
                    centerA, xyz0A, spiralA, xyz1A,
                    centerB, xyz0B, spiralB, xyz1B
                    );
        }
    return numSolution;
    }

DSpiral2dPlacement::DSpiral2dPlacement ()
    {
    spiral = NULL;
    frame  = Transform::FromIdentity ();
    fractionA = 0.0;
    fractionB = 1.0;
    }


DSpiral2dPlacement::DSpiral2dPlacement (DSpiral2dBaseCR source, TransformCR _frame, double _fractionA, double _fractionB)
    {
    spiral = source.Clone ();
    frame  = _frame;
    fractionA = _fractionA;
    fractionB = _fractionB;
    }

DSpiral2dPlacement::DSpiral2dPlacement (DSpiral2dBaseP _spiral, TransformCR _frame, double _fractionA, double _fractionB)
    {
    spiral = _spiral;
    frame  = _frame;
    fractionA = _fractionA;
    fractionB = _fractionB;
    }

void DSpiral2dPlacement::InitCapturePointer (DSpiral2dBaseP _spiral, TransformCR _frame, double _fractionA, double _fractionB)
    {
    spiral = _spiral;
    frame  = _frame;
    fractionA = _fractionA;
    fractionB = _fractionB;
    }
//! Return a clone.
DSpiral2dPlacementP DSpiral2dPlacement::Clone (DSpiral2dPlacementCR source) const
    {
    return new DSpiral2dPlacement (NULL != spiral ? spiral->Clone () : NULL, frame, fractionA, fractionB);
    }

//! Free the spiral pointer.
DSpiral2dPlacement::~DSpiral2dPlacement ()
    {
    if (NULL != spiral)
        delete spiral;
    }
//! Free the current spiral pointer and replace by (possibly NULL) arg.
void DSpiral2dPlacement::ReplaceSpiral (DSpiral2dBaseP callerSpiral)
    {
    if (NULL != spiral)
        delete spiral;
    spiral = callerSpiral;
    }

//! Reverse fractions
bool DSpiral2dPlacement::ReverseInPlace ()
    {
    std::swap (fractionA, fractionB);
    return true;
    }
// Return a shift by a multiple of 2PI so thetaB is within 180 degrees of thetaA
double RadianPeriodShift (double thetaA, double thetaB)
    {
    double pi = Angle::Pi ();
    double twoPi = Angle::TwoPi ();
    double delta = thetaB - thetaA;
    if (delta > pi)
        {
        return Angle::TwoPi () * (int)(0.5 + delta/twoPi);
        }
    else if (delta < - pi)
        {
        return Angle::TwoPi () * (int)(0.5 - delta/twoPi);
        }
    return 0.0;
    }
static bool FluffyAlmostEqualBearing (double bearingA, double bearingB)
    {
    return fabs (bearingA - bearingB) < 1.0e-8;
    }

//! Apply AlmostEqual to all components except fractions
bool DSpiral2dPlacement::AlmostEqual01 (DSpiral2dPlacement const &other, double tolerance) const
    {
    if (spiral == nullptr || other.spiral == nullptr)
        return false;
    if (spiral->GetTransitionTypeCode () != other.spiral->GetTransitionTypeCode ())
        return false;
    // Tolerance problems abound.
    // Caller tolerance is implicitly for coordinates.
    // We have some angles.  We have curvatures.  The frame origin might be coordinate, but the frame matrix is probably orthogonal.
    // EDL May 2017 allow bearings to float by 360?
    double delta = RadianPeriodShift (spiral->mTheta0, other.spiral->mTheta0);
    if (!FluffyAlmostEqualBearing (spiral->mTheta0, other.spiral->mTheta0 + delta))
        return false;
    if (!FluffyAlmostEqualBearing (spiral->mTheta1, other.spiral->mTheta1 + delta))
        return false;
    if (!DoubleOps::AlmostEqual(spiral->mCurvature0, other.spiral->mCurvature0))
        return false;
    if (!DoubleOps::AlmostEqual(spiral->mCurvature1, other.spiral->mCurvature1))
        return false;
    if (!frame.IsEqual (other.frame, DoubleOps::SmallCoordinateRelTol (), tolerance))
        return false;
    return true;
    }
// compare point, direction, and curvature at fractional positions
bool AlmostEqualAtFractions(
DSpiral2dPlacement const &placementA,
double activeFractionA,
DSpiral2dPlacement const &placementB,
double activeFractionB,
double tolerance) 
    {
    double fractionA = placementA.ActiveFractionToGlobalFraction(activeFractionA);
    double fractionB = placementB.ActiveFractionToGlobalFraction(activeFractionB);
    double scaleA = placementA.fractionB - placementA.fractionA;
    double scaleB = placementB.fractionB - placementB.fractionA;
    auto frameA = placementA.FractionToFrenetFrame(fractionA);
    auto frameB = placementB.FractionToFrenetFrame(fractionB);
    if (!frameA.Origin ().AlmostEqual (frameB.Origin(), tolerance))
        return false;
    auto kA = placementA.spiral->DistanceToCurvature (placementA.spiral->FractionToDistance (fractionA));
    auto kB = placementB.spiral->DistanceToCurvature(placementB.spiral->FractionToDistance(fractionB));
    if (!DoubleOps::AlmostEqual(fabs (kA), fabs (kB)))
        return false;
    DVec3d vectorXA, vectorXB, vectorYA, vectorYB;
    frameA.GetMatrixColumn(vectorXA, 0);
    frameB.GetMatrixColumn(vectorXB, 0);
    vectorXA = scaleA * vectorXA;
    vectorXB = scaleB * vectorXB;
    if (!vectorXA.AlmostEqual(vectorXB))
        return false;
    frameA.GetMatrixColumn(vectorYA, 1);
    frameB.GetMatrixColumn(vectorYB, 1);
    vectorYA = kA * vectorYA;
    vectorYB = kB * vectorYB;
    if (!vectorYA.AlmostEqual(vectorYB))
        return false;
    return true;
    }

//! Apply AlmostEqual to evaluated start and end points, bearings and curvatures
int DSpiral2dPlacement::AlmostEqualByActiveIntervalPoints(DSpiral2dPlacement const &other, double tolerance) const
    {
    auto spiralA = spiral;
    auto spiralB = other.spiral;
    if (spiralA == nullptr || spiralB== nullptr)
        return false;
    if (spiralA->GetTransitionTypeCode() != spiralB->GetTransitionTypeCode())
        return false;
    if (DoubleOps::AlmostEqual(spiralA->mLength, spiralB->mLength))
        {
        // hmm .. Any 4 properties (Q0, Q1, K0, K1) are really enough due to symmetry of spiral snap functions.
        // We will also test at midpoint.
        for (double f = 0.0; f <= 1.0; f += 0.5)
            {
            if (!AlmostEqualAtFractions(*this, f, other, f, tolerance))
                return false;
            }
        return true;
        }
    return false;
    }

//! Apply AlmostEqual to all components.
bool DSpiral2dPlacement::AlmostEqual(DSpiral2dPlacement const &other, double tolerance) const
    {
    return AlmostEqual01 (other, tolerance)
        && DoubleOps::AlmostEqual (fractionA, other.fractionA)
        && DoubleOps::AlmostEqual (fractionB, other.fractionB);
    }

DSegment1d DSpiral2dPlacement::FractionInterval () const { return DSegment1d (fractionA, fractionB);}
DPoint3d DSpiral2dPlacement::FractionToPoint (double fraction) const
    {
    DVec2d uv;
    double error;
    DSpiral2dBase::Stroke (*spiral, 0.0, fraction, 0.0, uv, error);
    return frame * DPoint3d::From (uv.x, uv.y, 0.0);
    }

DPoint3d DSpiral2dPlacement::ActiveFractionToPoint (double g) const
    {
    return FractionToPoint (ActiveFractionToGlobalFraction (g));
    }

DVec3d DSpiral2dPlacement::DisplacementBetweenFractions (double f0, double f1) const
    {
    DVec2d uv;
    double error;
    DSpiral2dBase::Stroke (*spiral, f0, f1, 0.0, uv, error);
    return frame * DVec3d::From (uv.x, uv.y, 0.0);
    }

DVec3d DSpiral2dPlacement::DisplacementBetweenActiveFractions (double g0, double g1) const
    {
    return DisplacementBetweenFractions (
            ActiveFractionToGlobalFraction (g0),
            ActiveFractionToGlobalFraction (g1)
            );
    }

double DSpiral2dPlacement::GlobalFractionToActiveFraction (double globalFraction) const
    {
    auto result = DoubleOps::InverseInterpolate (fractionA, globalFraction, fractionB);
    return result.Value ();
    }

double DSpiral2dPlacement::ActiveFractionToGlobalFraction (double activeFraction) const
    {
    return DoubleOps::Interpolate (fractionA, activeFraction, fractionB);
    }

Transform DSpiral2dPlacement::FractionToFrenetFrame (double fraction) const
    {
    DVec2d uv;
    double error;
    DSpiral2dBase::Stroke (*spiral, 0.0, fraction, 0.0, uv, error);
    DVec2d dU, ddU, dddU;
    spiral->FractionToDerivatives (fraction, dU, ddU, dddU);
    DPoint3d origin = frame * DPoint3d::From (uv);
    DVec3d dX = frame * DVec3d::From (dU);
    DVec3d dZ = DVec3d::From (0,0,1);
    RotMatrix axes = RotMatrix::FromColumnVectors (dX, dX, dZ);/// the Y axis will be recomputed shortly ...
    axes.SquareAndNormalizeColumns (axes, 0,2, true);
    return Transform::From (axes, origin);
    }
Transform DSpiral2dPlacement::ActiveFractionToFrenetFrame (double activeFraction) const
    {
    return FractionToFrenetFrame (ActiveFractionToGlobalFraction (activeFraction));
    }

RotMatrix DSpiral2dPlacement::FractionToDerivatives (double fraction) const
    {
    DVec2d dU, ddU, dddU;
    spiral->FractionToDerivatives (fraction, dU, ddU, dddU);
    DVec3d dX = frame * DVec3d::From (dU);
    DVec3d ddX = frame * DVec3d::From (ddU);
    DVec3d dddX = frame * DVec3d::From (dddU);
    return RotMatrix::FromColumnVectors (dX, ddX, dddX);
    }

RotMatrix DSpiral2dPlacement::ActiveFractionToDerivatives (double activeFraction) const
    {
    auto matrix = FractionToDerivatives (ActiveFractionToGlobalFraction (activeFraction));
    double a = fractionB - fractionA;
    if (a != 0.0 && a != 1.0)
        {
#ifdef ScaleDiv
        double diva = 1.0 / a;
        matrix.ScaleColumns (diva, diva * diva, diva * diva * diva);
#else
        matrix.ScaleColumns (a, a * a, a * a * a);
#endif
        }
    return matrix;
    }



DRay3d DSpiral2dPlacement::FractionToPointAndUnitTangent (double fraction) const
    {
    auto ray = FractionToPointAndDerivative (fraction);
    ray.direction.Normalize ();
    return ray;
    }

DRay3d DSpiral2dPlacement::FractionToPointAndDerivative (double fraction) const
    {
    DVec2d uv;
    double error;
    DSpiral2dBase::Stroke (*spiral, 0.0, fraction, 0.0, uv, error);
    DVec2d dU, ddU, dddU;
    spiral->FractionToDerivatives (fraction, dU, ddU, dddU);
    DPoint3d origin = frame * DPoint3d::From (uv);   // be sure to cast as POINT
    DVec3d dX = frame * DVec3d::From (dU);          // be sure to cast as VECTOR ..
    return DRay3d::FromOriginAndVector (origin, dX);
    }

double DSpiral2dPlacement::SpiralLengthActiveInterval () const
    {
    return spiral->mLength * fabs (fractionB - fractionA);
    }

double DSpiral2dPlacement::SpiralLength01 () const
    {
    return spiral->mLength;
    }
// context for
// 1) capture a placement and additional mapping matrix (view matrix)
// 2) present integrands for evaluation by gauss quadrature queries
// 3) run the integration
class MappedLengthIntegrator : public BSIIncrementalVectorIntegrand
{
DVec3d m_uVector, m_vVector;
DSpiral2dPlacementCR m_placement;
double m_lastDistance;
public:
MappedLengthIntegrator (RotMatrixCR matrix, DSpiral2dPlacementCR placement)
    : m_placement(placement)
    {
    RotMatrix matrixB;
    m_placement.frame.GetMatrix (matrixB);
    DVec3d wVector;
    RotMatrix matrixAB = matrix * matrixB;
    matrixAB.GetColumns (m_uVector, m_vVector, wVector);
    m_lastDistance = 0.0;
    }

void EvaluateVectorIntegrand (double t, double *pF) override 
    {
    double dUV[10];
    m_placement.spiral->EvaluateVectorIntegrand (t, dUV);
    DVec3d vector = m_uVector * dUV[0] + m_vVector * dUV[1];
    pF[0] = vector.Magnitude ();
    }
int  GetVectorIntegrandCount () override { return 1;}

bool AnnounceIntermediateIntegral (double t, double *pIntegral) override 
    {
    m_lastDistance = pIntegral[0];
    return true;
    }

double IntegrateMappedLengthBetweenPrimaryFractions (double startFraction, double endFraction)
    {
    BSIQuadraturePoints gauss;
    int numGauss = 4;
    gauss.InitGauss (numGauss);     
    m_lastDistance = 0.0;
    double nominalLength = m_placement.SpiralLength01 ();
    double d0 = startFraction * nominalLength;
    double d1 = endFraction * nominalLength;
    double totalErrorBound;
    size_t numInterval = DSpiral2dBase::GetIntervalCount (*m_placement.spiral, startFraction, endFraction, s_defaultStrokeRadians, s_defaultMinIntervals);
    gauss.IntegrateWithRombergExtrapolation (*this, d0, d1, (uint32_t)numInterval, totalErrorBound);
    return m_lastDistance;
    }
};

double DSpiral2dPlacement::MappedSpiralLengthActiveInterval (RotMatrixCR matrix) const
    {
    MappedLengthIntegrator integrator (matrix, *this);
    return fabs (integrator.IntegrateMappedLengthBetweenPrimaryFractions (fractionA, fractionB));
    }


END_BENTLEY_GEOMETRY_NAMESPACE

#include "directEvaluationSpiral.h"

