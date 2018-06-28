/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Geom/BsplineStructs.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include <Bentley/RefCounted.h>
#ifndef MS_BSPLINE_STRUCTURES_DEFINED
#define MS_BSPLINE_STRUCTURES_DEFINED

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/* B-Spline curve types */
#define     BSCURVE_GENERAL             0
#define     BSCURVE_LINE                1
#define     BSCURVE_CIRCULAR_ARC        2
#define     BSCURVE_CIRCLE              3
#define     BSCURVE_ELLIPTICAL_ARC      4
#define     BSCURVE_ELLIPSE             5
#define     BSCURVE_PARABOLIC_ARC       6
#define     BSCURVE_HYPERBOLIC_ARC      7

/* B-Spline surface types */
#define     BSSURF_GENERAL              0
#define     BSSURF_PLANE                1
#define     BSSURF_RIGHT_CYLINDER       2
#define     BSSURF_CONE                 3
#define     BSSURF_SPHERE               4
#define     BSSURF_TORUS                5
#define     BSSURF_REVOLUTION           6
#define     BSSURF_TAB_CYLINDER         7
#define     BSSURF_RULED_SURFACE        8

/* Constants */
#define BSSURF_U 0
#define BSSURF_V 1

/* Miscellaneous Maxima */
#define MAX_POLES       MAX_VERTICES
#define MAX_ORDER       26
#define MAX_KNOTS       (MAX_POLES + 2*MAX_ORDER - 1)
#define MAX_BNDRY_PTS   MAX_VERTICES
#define MAX_DATA_PTS    MAX_VERTICES
#define MAX_BSORDER     MAX_ORDER
/* Tolerances */
#define KNOT_TOLERANCE_BASIS    1E-10

/* Offset cusp treatments */
#define OFFSET_JUMP_CUSP        0
#define OFFSET_CHAMFER_CUSP     1
#define OFFSET_POINT_CUSP       2
#define OFFSET_PARABOLA_CUSP    3
#define OFFSET_ARC_CUSP         4

/* Continuity degree specifications */
#define     POSITION_CONTINUITY     0
#define     TANGENT_CONTINUITY      1
#define     CURVATURE_CONTINUITY    2
#define     DERIVATIVE_CONTINUITY   3

/* Truncation specifications */
#define     TRUNCATE_BOTH           0
#define     TRUNCATE_NONE           1
#define     TRUNCATE_SINGLE         2

/* Iso display for silhouette curves */
#define     SILHBND_U               0
#define     SILHBND_V               1
#define     SILHBND_NONE            2
#define     SILHBND_BOTH            3

/* Surface edge classification */
#define     NO_EDGE     0x0000
#define     U0_EDGE     0x0001
#define     U1_EDGE     0x0002
#define     V0_EDGE     0x0004
#define     V1_EDGE     0x0008
#define     ANY_EDGE    0x000F

typedef int MSBsplineStatus;
#define MSB_SUCCESS 0
#define MSB_ERROR   0x8000

//! Spline order, closure, and count data.
struct GEOMDLLIMPEXP bSplineParam
    {
	//! Curve order (one more than polynomial degree).
    int32_t             order;
	//! True if the curve uses poles in a periodic sense.
    int32_t             closed;
	//! Number of stored poles.
    int32_t             numPoles;
	//! Knot count.  See NumberAllocatedKnots.
    int32_t             numKnots;               /* # interior knots or zero for uniform */
	//! Not used.
    int32_t             numRules;

    //! Return the number of knots in the allocated knot array.  
	//! Note that this number may be different from the value of the numKnots field of this structure because the numKnots field traditionally indicated only the number of interior knots.
    int NumberAllocatedKnots () const;
	//! Return the number of knots needed for a curve with specified order, closure, and poles.
	//! Note that this number may be different from the value stored in a numKnots field  because that traditionally indicated only the number of interior knots.
    static int NumberAllocatedKnots (int32_t numPoles, int32_t order, int32_t closed);

    //! Return the number of interior knots, as recorded (for legacy reasons) in the params structures
    static int NumberInteriorKnots (int32_t numPoles, int32_t order, int32_t closed);

    };

//! Interpolation controls
struct interpolationParam
    {
    int32_t             order;                  /* default 4 */
    int32_t             isPeriodic;
    int32_t             numPoints;              /* including duplicate pt if periodic */
    int32_t             numKnots;               /* # full knots, never zero */
    int32_t             isChordLenKnots;        /* default 1 */
    int32_t             isColinearTangents;     /* default 0 */
    int32_t             isChordLenTangents;     /* default 1 */
    int32_t             isNaturalTangents;      /* default 1 */
    };

//! spline display controls
struct bsplineDisplay
    {
    int32_t             polygonDisplay;
    int32_t             curveDisplay;
    int32_t             rulesByLength;
    };


//! Complete data for a single patch segment extracted from a bspline curve.
//! May be used (in context) for either raw curve poles (with given non-uniform knots) or bezier poles.
//! The size() of the points array is the order of the bezier segment.
struct BCurveSegment
    {
    private:
    // Centroid, length integrals with optional centroid.
    void WireIntegrals (double &length, DPoint3dP centroid, 
            double fractionA, double fractionB);
    public:
    enum
        {
        MaxOrder = 26,
        };
    private:
    DPoint4d m_poles[MaxOrder];
    DPoint4d m_workPoles[MaxOrder];
    double   m_knots[2 * MaxOrder];
    size_t   m_order;
    size_t   m_workOrder;
    size_t   m_numKnots;

    //friend struct MSBsplineCurve;
    friend bool bspcurv_getSupport (MSBsplineCurveCR curve, BCurveSegmentR segment, size_t bezierSelect);
    friend GEOMDLLIMPEXP  int bspproc_prepareCurve (MSBsplineCurveP, int*, int **, MSBsplineCurveP);

    //! segment position in knot intervals
    size_t m_index;
    //! low knot represented by this segment
    double m_uMin;
    //! high knot represented by this segment
    double m_uMax;
    //! flag set to true if this is a null interval (high multiplicity knot)
    bool  m_isNullU;
    public:

    GEOMDLLIMPEXP BCurveSegment ();

    //! Evalute the point at given fraction
    GEOMDLLIMPEXP void FractionToPoint (DPoint3dR xyz, double f) const;

    //! Return the point at given fraction
    GEOMDLLIMPEXP DPoint3d FractionToPoint (double f) const;

    //! Return the knot value at a fractional parameter
    GEOMDLLIMPEXP double FractionToKnot (double f) const;

    //! Evalute both point and derivative at a fractional parameter
    GEOMDLLIMPEXP void FractionToPoint (DPoint3dR xyz, DVec3dR tangent, double f, bool applyKnotScale = true) const;

    //! Copy from {source} and apply a 4d (perspective) matrix.
    GEOMDLLIMPEXP void CopyFrom (BCurveSegmentCR source, DMatrix4dCP matrix = NULL);

    //! Copy from {source} and apply a transformation
    GEOMDLLIMPEXP void CopyFrom (BCurveSegmentCR source, RotMatrixCR matrix);

    //! Copy from {source} and apply a transformation
    GEOMDLLIMPEXP void CopyFrom (BCurveSegmentCR source, TransformCR matrix);
    
    //! Compute length and centroid of curve as wire.
    GEOMDLLIMPEXP void WireCentroid (double &length, DPoint3dR centroid, double fraction0, double fraction1);

    //! Compute length between fractions 
    GEOMDLLIMPEXP void Length (double &length, double fraction0, double fraction1);
    //! Compute length between fractions 
    GEOMDLLIMPEXP void Length (RotMatrixCP worldToLocal, double &length, double fraction0, double fraction1);


    //! Compute polygon length
    GEOMDLLIMPEXP double PolygonLength () const;

    //! Return uMin,uMax packaged as DRange1d....
    GEOMDLLIMPEXP DRange1d KnotRange () const;

    //! On input, knots are arbitrary sorted sequence, with {order-1} leading knots.
    //! On output, knots are collapsed to bezier.
    GEOMDLLIMPEXP void SaturateKnots ();

    //! Add strokes to point and param arrays.
    //! @param [in,out] points receives points.
    //! @param [in,out] params receives parameters.
    //! @param [in,out] derivatives receives derivatives
    //! @param [in] options stroke controls
    //! @param [in] fractionA start fraction
    //! @param [in] fractionB end fraction
    //! @param [in] useWorkPoles true to stroke from work poles, false for primaries.
    //! @param [in] curve curve pointer (for use in parameter mapping, if indicated by facet options)
    GEOMDLLIMPEXP void AddStrokes
        (
        bvector <DPoint3d> &points,
        bvector <DVec3d> *derivatives,
        bvector<double> *params,
        IFacetOptionsCR options,
        double fractionA = 0.0,
        double fractionB = 1.0,
        bool useWorkPoles = false,
        MSBsplineCurveCP curve = NULL
        ) const;

    //GEOMDLLIMPEXP void MapBezierFractions (bvector<double> *params, CurveParameterMapping select) const;


    //! Query if the bezier has weighted control points.  Optionally inspect the work poles rather than the primaries.
    GEOMDLLIMPEXP bool IsRational (bool useWorkPoles = false) const;
    //! Return a work pole by index.
    GEOMDLLIMPEXP DPoint4dP GetWorkPoleP (size_t index);
    //! Query the order of the work poles
    GEOMDLLIMPEXP size_t GetWorkOrder ();
    //! Transform primary poles
    GEOMDLLIMPEXP void Multiply (TransformCR transform);
    
    //! Transform primary poles into work poles.
    GEOMDLLIMPEXP void BuildWorkPoles (TransformCR transform);

    //! Find minmax params (in some or all dimensions)
    //! @param [in,out] params receiver vector.
    //! @param [out] range (optional) range observed at (a) points in the params and (b) endpoints.
    //! @param [in] mapToKnots if true convert bezier params to knots.
    //! @param [in] firstDimension first dimension (0,1,2) to examine.
    //! @param [in] lastDimension last dimension (0,1,2) to examine.
    //! @param [in] includeStartEnd true to force params 0 and 1 into the results.
    GEOMDLLIMPEXP void AddExtrema (bvector<double> &params,
                    DRange3dP range = NULL,
                    bool mapToKnots = false, size_t firstDimension = 0, size_t lastDimension = 2, bool includeStartEnd = true) const;


    //! derference a pole, and drop to xyz.
    GEOMDLLIMPEXP bool TryGetPoleXYZ (size_t index, DPoint3dR xyz) const;
    //! Return (interior) pointer to indexed pole.
    GEOMDLLIMPEXP DPoint4dP GetPoleP (size_t index);
    //! Return(interior) pointer to first pole.
    GEOMDLLIMPEXP DPoint4dP GetPoleP ();
    //! Return the pole array (reference)
    GEOMDLLIMPEXP void GetPoles (bvector<DPoint4d> &poles);
    //! Return pole count.
    GEOMDLLIMPEXP size_t GetOrder ();
    //! Return knot count
    GEOMDLLIMPEXP size_t GetNumKnots ();

    //! Get a pointer to an indexed knot.
    GEOMDLLIMPEXP double *GetKnotP (size_t index);
    //! Get a pointer to all the knots.
    GEOMDLLIMPEXP double *GetKnotP ();

    //! return true if the bezier (just extracted from bspline) has zero-length knot interval.
    GEOMDLLIMPEXP bool IsNullU () const;
    //! Lower knot in parent bspline
    GEOMDLLIMPEXP double UMin () const;
    //! Upper knot in parent bspline
    GEOMDLLIMPEXP double UMax () const;

    //! Set lower knot in parent bspline
    GEOMDLLIMPEXP void SetUMin (double u);
    //! Set upper knot in parent bspline
    GEOMDLLIMPEXP void SetUMax (double u);

    //! Find the intersection of the segment UMin, UMax and the given interval.
    //! If empty, return false.
    //! If not empty, 
    GEOMDLLIMPEXP bool SubdivideToIntersection (DRange1dCR interval);

    //! Index (from start) in parent bspline
    GEOMDLLIMPEXP size_t Index () const;

    //! Run newton iteration to move frctions to closest approach points.
    //! return true if iteration succeeeded.
    //! @param [in] curveA first curve
    //! @param [in] fractionAin initial fraction on curveA
    //! @param [in] curveB second curve
    //! @param [in] fractionBin initial fraction on curveB
    //! @param [in] useWorkPoles true to iterate with the work poles, false for primaries
    //! @param [in] xyOnly true to iterate on xy parts, false for xyz
    //! @param [out] fractionAOut fraction on curveA
    //! @param [out] xyzAOut point on curveA
    //! @param [out] fractionBOut fraction on curveB
    //! @param [out] xyzBOut point on curveB
    static GEOMDLLIMPEXP bool RefineCloseApproach
        (
        BCurveSegmentR curveA,
        double fractionAin,
        BCurveSegmentR curveB,
        double fractionBin,
        bool useWorkPoles,
        bool xyOnly,
        double &fractionAOut,
        DPoint3dR xyzAOut,
        double &fractionBOut,
        DPoint3dR xyzBOut
        );
    };
/*__PUBLISH_SECTION_END__*/

struct BCurveSegment1d
    {
    //! support values
    bvector<double> poles;
    //! knots from parent curve
    bvector<double> knots;
    //! segment position in knot intervals
    size_t index;
    //! low knot represented by this segment
    double uMin;
    //! high knot represented by this segment
    double uMax;
    //! flag set to true if this is a null interval (high multiplicity knot)
    bool  isNullU;
    
    GEOMDLLIMPEXP BCurveSegment1d ();

    //! Evalute the value at given fraction
    GEOMDLLIMPEXP void FractionToValue (double& value, double f) const;

    //! Return the value at given fraction
    GEOMDLLIMPEXP double FractionToValue (double f) const;

    //! Return the knot value at a fractional parameter
    GEOMDLLIMPEXP double FractionToKnot (double f) const;

    //! Evalute both value and derivative at a fractional parameter
    GEOMDLLIMPEXP void FractionToValue (double& value, double& der, double f, bool applyKnotScale = true) const;

    //! Return uMin,uMax packaged as DRange1d....
    GEOMDLLIMPEXP DRange1d KnotRange () const;
    };

struct  Bspline1d
    {
private:
    //! support poles
    bvector<double> poles;
    //! full knots from Bspline curve
    bvector<double> knots;
public:
    GEOMDLLIMPEXP Bspline1d ();

    GEOMDLLIMPEXP bvector<double> &GetPolesR ();
    GEOMDLLIMPEXP bvector<double> &GetKnotsR ();
    //! Return the order
    GEOMDLLIMPEXP int GetOrder () const;

    //! Extract the {order} poles and {2*(order-1)} knots that support a single bezier interval ...
    //! @param [out] outPoles poles
    //! @param [out] outKnots knots
    //! @param [in] bezierSelect index of bezier to extract.
    //! @DotNetMethodExclude
    GEOMDLLIMPEXP bool GetSupport (bvector<double>& outPoles, bvector<double>& outKnots, size_t bezierSelect) const;

    //! Get poles for a single bezier poles from the 1D bspline.
    //! return false if invalide bezierSelect.  Note that the bezierSelect for a high multiplicity knot returns true for the function
    //!    but marks the interval as null.   Normal usage is to loop over all beziers in a bspline but skip processing the null intervals.
    //! @param [out] segment a filled BCurveSegment1d.
    //! @param [in] bezierSelect selects a bezier interval within the bspline
    //! @DotNetMethodExclude
    GEOMDLLIMPEXP bool GetBezier (BCurveSegment1d& segment, size_t bezierSelect) const;

    // Calculate the roots of Bspline1d curve at given value
    GEOMDLLIMPEXP bool FractionRoots (bvector<double> &rootFractions, double value) const;

    //! Populate the Bspline1d curve with the given parameters.
    //! @DotNetMethodExclude
    GEOMDLLIMPEXP void Populate (bvector<double> &poleVector, bvector<double> &knotVector);

    //! Populate the Bspline1d curve with the given parameters.
    //! @DotNetMethodParameterIsInputArray{pPoints,numPoints}
    //! @DotNetMethodParameterIsInputArray{pKnots,numKnots}
    //! @DotNetMethodExclude
    GEOMDLLIMPEXP void Populate (double const * pVales, size_t numVales, double const * pKnots, size_t numKnots);

    //! Create the Bspline1d curve from point array and order, with clamped uniform knots in 0..1
    GEOMDLLIMPEXP void Populate (double const * pVales, size_t numVales, size_t order);

    //! Return the knot values corresponding to fraction 0 and fraction 1 ...
    GEOMDLLIMPEXP DRange1d KnotRange () const;

    //! Return the knot value at a fractional parameter ...
    GEOMDLLIMPEXP double FractionToKnot (double f) const;

    //! Return the fractional parameter corresponding to a knot value ...
    GEOMDLLIMPEXP double KnotToFraction (double knot) const;

    //! Fraction parameter to value on the Bspline1d curve.
    GEOMDLLIMPEXP double FractionToValue (double f);
    //! Fraction parameter to both point and derivative. Derivative is wrt the fraction parameter.
    GEOMDLLIMPEXP void FractionToValue (double& value, double& der, double f);

    //! Knot parameter to value on the Bspline1d curve.
    GEOMDLLIMPEXP double KnotToValue (double knot);
    //! Knot parameter to both point and derivative. Derivative is wrt the fraction parameter.
    GEOMDLLIMPEXP void KnotToValue (double& value, double& der, double knot);
};


//! A KnotData structure carries "fully analyzed" description of knot vectors.
//! The knots are carried in bvectors.
//! The usage patter is
//! 1) Allocate and load from a curve or surface:
//!           KnotData knotData;
//!           knotData.LoadCurve (bcurve);
//! 2) Use the data
//! 3) Allow destructor to release heap arrays.
//!
struct KnotData
{
//! Complete knot vector
bvector<double>allKnots;
//! Order of the bspline basis functions.
size_t order;
//! closure flag.
bool closed;
//! Distinct knot values
bvector<double>compressedKnots;
//! Corresponding multiplicities
bvector<size_t>multiplicities;
//! left index of the active knot interval: compressedKnots[leftIndex] is the lower limit of the knot range.
//! (In a clampled open knot vector this is zero.   It can be nonzero in unclamped and periodic knot vectors)
size_t leftIndex;
//! right index of the active knot interval: compressedKnots[rightIndex] is the upper limit of the knot range.
//! (In a clampled open knot vector this is {compressedKnots.size () - 1}.   It can be a lower index in unclamped and periodic knot vectors)
size_t rightIndex;

//! Clear all data
GEOMDLLIMPEXP void Clear ();

//! Test ordering conditions:
//! 1) order > 0
//! 2) numKnots >= 2 * order
//! 3) allKnots is nondecreasing
//! 4) sum of multiplicities == numKnots
//! 5) leftIndex,rightIndex must be within range and increasing.
GEOMDLLIMPEXP bool IsWellOrdered () const;

//! Load knot data from a curve.
GEOMDLLIMPEXP bool LoadCurveKnots (MSBsplineCurveCR curve);

//! Load knot data from the u direction of a surface
GEOMDLLIMPEXP bool LoadSurfaceUKnots (MSBsplineSurfaceCR surface);

//! Load knot data from the v direction of a surface
GEOMDLLIMPEXP bool LoadSurfaceVKnots (MSBsplineSurfaceCR surface);

//! query if u is strictly interior to the knot range.
GEOMDLLIMPEXP bool IsStrictInteriorKnot (double u) const;

//! return the multiplicity of a knotValue (and its corrected actual knot if it is "clsoe" to a knot) 
GEOMDLLIMPEXP void FindKnotMultiplicity (double knotValue, double &correctedKnotValue, size_t &multiplicityOut) const;

//! access a knot and its multplicity, identified by index from the left of the active interval.
//! @return false if index is beyond the right end.
GEOMDLLIMPEXP bool GetKnotByActiveIndex (size_t i, double &a, size_t &multiplicityA) const;

//! access a knot interval, identified by index from the left of the active interval.
//! @return false if index is beyond the right end.
GEOMDLLIMPEXP bool GetKnotIntervalByActiveIndex (size_t i, double &a0, double &a1) const;

//! access a knot interval, identified by index from the left of the active interval.
//! @return false if index is beyond the right end.
GEOMDLLIMPEXP bool GetLongestActiveKnotInterval (size_t &i, double &a0, double &a1) const;


//! Return the number of active knots (including last)
GEOMDLLIMPEXP size_t GetNumActiveKnots () const;

//! Return start and of active knot range. (one full period if periodic)
GEOMDLLIMPEXP bool GetActiveKnotRange (double &knot0, double &knot1) const;

//! Collect knots with multiplicity of targetMultiplicity or greater.
GEOMDLLIMPEXP void CollectHighMultiplicityActiveKnots (size_t targetMultiplicity, bool includeEnds, bvector <double> &knots);
};

 
/*__PUBLISH_SECTION_START__*/

END_BENTLEY_GEOMETRY_NAMESPACE
#endif
