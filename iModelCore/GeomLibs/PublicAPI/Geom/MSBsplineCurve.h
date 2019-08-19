/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once



/*__PUBLISH_SECTION_START__*/
//! @file   MSBsplineCurve.h A Non uniform, rational Bspline curve: MSBsplineCurve, RefCountedMSBsplineCurve

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/**
@brief MSBsplineCurve is a "Non uniform, rational Bspline curve".

<h3>Pole and knot counts</h3>

<TABLE BORDER="1">
<TR><TD>Open curve</TD> <TD>knots = poles + order</TD></TR>
<TR><TD>Closed (periodc) curve</TD> <TD>knots = poles + 2 * order - 1</TD></TR>
</TABLE>

<h3>Parametric position along a bspline curve</h3>

There are two conventions for identifying positions within the splines parameter space:
<TABLE BORDER="1">
<TR><TD>Convention</TD><TD>start value  </TD><TD> end value    </TD><TD> remarks  </TD></TR>
<TR><TD>Fraction position    </TD><TD> always 0.0  </TD><TD> always 1.0    </TD><TD> this is the easiest (prefered) way for any app that is not directly manipulating the knot vector. </TD></TR>
<TR><TD>Knot postion </TD><TD> first active knot value </TD><TD> last active knot value    </TD><TD> "active" knots are the "mid section" knots ignoring (order-1) leading and trailing knots </TD></TR>
</TABLE>

The word Fraction or Knot in a method name indicates how the arguments for that method are interpreted.

<h3>Clamping convention</h3>
Open knot vectors follow the "clamping convention":
<ul>
<li>The first (order) knots are identical and are the start value for the knot range.
<li>The last (order) knots are identical and are the final value for the knot range.
<li>Older code assumes clamping.
<li>Recent code does not assume clamping.
<li>No promises are made for behavior of unclamped knot vectors.
<li>Careful analysis of basis fucntion logic says that the very first and last knot values are never referenced in the basis function logic.  But this extra knot convention is widely used in interchange formats.
</ul>
//! @ingroup BentleyGeom_Bsplines
*/
struct MSBsplineCurve
{
    /// <summary>Type code. Not maintained</summary>
    int32_t             type;
    /// <summary>True if the curve has weights.</summary>
    int32_t             rational;
    /// <summary>Display control parameters.  Not used in computations.  Beware that if a curve is saved into
    ///     a file, a zeroed out display structure may lead to non-displayed curve</summary>
    BsplineDisplay      display;
    /// <summary>count and order data.  Use NumberAllocatedKnots and NumberAllocatedPoles to query.</summary>
    BsplineParam        params;
//#ifdef jmdlgeom_internal
    public:
//#else
//    private:
//#endif    
    /// <summary>array of poles, already multiplied by corresponding weights</summary>
    DPoint3d            *poles;
    /// <summary>Full knot array (all clamping and wrap contents expanded)</summary>
    double              *knots;
    /// <summary>weight array.  Same length as poles.</summary>
    double              *weights;
public:
//flex !! Creating refcounted curves
//flex || Description  || method ||
//flex || Empty curve structure || curvePtr = MSBsplineCurve::CreatePtr ()  ||
//flex || Copy from arrays      || curvePtr = MSBSsplineCurve::CreateFromPolesAndOrder (BVectorDPoint3dCR poles, BVectorDoubleCP weights, BVectorDouble *knots, int order, bool closed, bool inputPolesAlreadyWeighted ||

//flex || Copy bits from instance directly into new refcounted memory.  Instance is zeroed.   RefCounted destructor becomes responsible for free.     || curvePtr = curve::CreateCapture () ||
//flex || Clone contents of instance. Instance is unchanged.  || curvePtr = MSBsplineCurve::CreateCopy () ||
//flex || Clone contents of instance, revising poles and knots to open at specified fraction. Instance is unchanged.  || curvePtr = MSBsplineCurve::CreateCopyOpenAtFraction () ||
//flex || Clone contents of instance, revising poles and knots to open at specified knot. Instance is unchanged.  || curvePtr = MSBsplineCurve::CreateCopyOpenAtKnot () ||
//flex || Clone contents, revising poles to make it a closed curve. || curvePtr = MSBsplineCurve::CreateCopyClosed () ||
//flex || Clone contents of a subset with start and end fractions. || curvePtr = MSBsplineCurve::CreateCopyBetweenFractions (fractionA, fractionB) ||
//flex || Clone contents of a subset with start and end knots. || curvePtr = MSBsplineCurve::CreateCopyBetweenFractions (knotA, knotsB) ||
//flex || Clone contents, reversing direction of curve || curvePtr = MSBsplineCurve::CreateCopyReversed () ||
//flex || Clone contents, applying transform || curvePtr = MSBsplineCurve::CreateCopyTransformed () ||
//flex || Clone contents, saturate all knots || curvePtr = MSBsplineCurve::CreateCopyBezier () ||


    //! Returns a smart pointer to an MSBsplineCurve on the heap.
    GEOMDLLIMPEXP static MSBsplineCurvePtr CreatePtr ();
    //! <summary>Allocate smart pointer target with directly supplied poles, weights, and knots.</summary>
    GEOMDLLIMPEXP static MSBsplineCurvePtr CreateFromPolesAndOrder
            (
            bvector<DPoint3d> const &poles,
            bvector<double> const *weights,
            bvector<double> const *knots,
            int order,
            bool closed,
            bool inputPolesAlreadyWeighted = true
            );

    //! <summary>Allocate smart pointer target with directly supplied poles, weights, and knots.</summary>
    GEOMDLLIMPEXP static MSBsplineCurvePtr CreateFromPolesAndOrder
            (
            DPoint3dCP poles,
            int numPoles,
            int order,
            bool closed = false
            );

    //! <summary>Allocate smart pointer target with directly supplied poles, weights, and knots.</summary>
    GEOMDLLIMPEXP static MSBsplineCurvePtr CreateFromPolesAndOrder
            (
            DPoint2dCP poles,
            int numPoles,
            int order,
            bool closed = false
            );

    /// <summary>Copy bits into smart pointer.  Caller instance zeroed out.<summary>
    GEOMDLLIMPEXP MSBsplineCurvePtr CreateCapture ();
    //! Copy bits into simple structure. Caller instance zeroed.
    GEOMDLLIMPEXP void ExtractTo (MSBsplineCurveR dest);
    /// <summary>Return copy as smart pointer target.</summary>
    GEOMDLLIMPEXP MSBsplineCurvePtr CreateCopy () const;
    /// <summary>Return copy as smart pointer target;  if closed, open it at fraction.</summary>
    GEOMDLLIMPEXP MSBsplineCurvePtr CreateCopyOpenAtFraction (double fraction) const;
    /// <summary>Return copy as smart pointer target;  if closed, open it at knot.</summary>
    GEOMDLLIMPEXP MSBsplineCurvePtr CreateCopyOpenAtKnot (double knot) const;
    /// <summary>Return copy as smart pointer target;  if physically closed revise poles to be a closed bspline</summary>
    GEOMDLLIMPEXP MSBsplineCurvePtr CreateCopyClosed () const;

    /// <summary>Copy the portion between specified fractions.</summary>
    GEOMDLLIMPEXP MSBsplineCurvePtr CreateCopyBetweenFractions (double fraction0, double fraction1) const;
    /// <summary>Copy the portion between specified knots.</summary>
    GEOMDLLIMPEXP MSBsplineCurvePtr CreateCopyBetweenKnots (double knot0, double knot1) const;

    /// <summary>Complete copy with reversed parameterization.</summary>
    GEOMDLLIMPEXP MSBsplineCurvePtr CreateCopyReversed () const;
    /// <summary>Copy with transform applied to poles.</summary>
    GEOMDLLIMPEXP MSBsplineCurvePtr CreateCopyTransformed (TransformCR transform) const;
    /// <summary>Copy with all knots saturated.</summary>
    GEOMDLLIMPEXP MSBsplineCurvePtr CreateCopyBezier () const;
    /// <summary>Copy with offset in XY plane.</summary>
    GEOMDLLIMPEXP MSBsplineCurvePtr CreateCopyOffsetXY (double offset0, double offset1, CurveOffsetOptionsCR options) const;
    
//! <summary>Create a spline curve with Lim's fit conditions or related.
//!<ul>
//!<li>Pole count matches point count.
//!<li>selector = 0 is Lim's condition: interpolation points are matched at knot values for basis function peaks.
//!<li>selector = 1: intepolation points are matched at Greville moving average knot values.
//!</ul>
static GEOMDLLIMPEXP MSBsplineCurvePtr CreateFromInterpolationAtBasisFunctionPeaks
(
bvector<DPoint3d>const &xyz,    //!< [in] xyz interpolation points
size_t order,                   //!< [in] order bspline order.
int selector = 0                //!< [in] selector -- see description.
);
    
//! <summary>Create a spline curve that approximates a CurvePrimitive.
//!<ul>
//!<li>Knots are constructed with uniform or Chebyshev points accordinat to knotSelector.
//!<li>The Greville average knots are then computed.
//!<li>The curve primitive is evaluated at the Greville knots.
//!<li>For example, if the uniform knots for a cubic are [0 0 0 0 1 2 3 4 5 6 7 8 9 10 10 10 10]
//!        the Greville averages are [0 1/3 1 2 3 4 5 6 7 8 9 9+2/3 10]
//!<li>the curve fit interpolates at all those points.
//!<li>If the CurvePrimitives exact parameterization is by true distances, this provides a strong match between bspline knots and true distance.
//!<li>This makes this a strong interpolation for catenary and spiral curves.
//!<li>The only CurvePrimitive method called is FractionToPoint.
//!</ul>
static GEOMDLLIMPEXP MSBsplineCurvePtr CreateFromInterpolationAtGrevilleKnots
(
ICurvePrimitiveCR curve,
size_t numPoles,
int order,
bool normalizeKnots,
int knotSelector = 0       // 0==> uniform, 1==>Chebyshev
);

//! Create a spline with given 
//! <summary>
//!<ul>
//!<li>Returns nullptr if count conditions are not met.
//!<li>Must have (xyz.size () == interpolationKnots.size ()) must have same size.
//!<li>Must have (xyz.size () + order = curveKnots.size ())
//!<li>Curve knots are the usual "full" knot vector (e.g. replication for clamping)
//!</ul>
static GEOMDLLIMPEXP MSBsplineCurvePtr CreateFromInterpolationPointsWithKnots
(
bvector<DPoint3d>const &xyz,                //!< [in] xyz interpolation points
bvector<double> const &interpolationKnots,  //!< [in] knot values where interpolation occurs
bvector<double> curveKnots,                  //!< [in] knots for the curve
int order                                   //! curve order
);

    //! Create copies of each segment with breaks at disjoint knot points.
    GEOMDLLIMPEXP void GetDisjointCurves (bvector<MSBsplineCurvePtr> &curves) const;
    //! Create copies of each segment with breaks at point or tangent changes.
    GEOMDLLIMPEXP void GetC1DiscontinuousCurves (bvector<double> &fractions, bvector<MSBsplineCurvePtr> &curves) const;


//flex || Interpolate between existing curves || curvePtr = MSBSsplineCurve::CreateInterpolationBetweenCurves (curveA, fraction, curveB) ||



    /// <summary>Create a curve whose poles are interpolated between the poles of two curves.</summary>
    /// <remarks>This will only succeed if the curves have the same pole, knot, and order structure</remarks>
    GEOMDLLIMPEXP static MSBsplineCurvePtr CreateInterpolationBetweenCurves
            (
            MSBsplineCurveCR curveA,
            double fraction,
            MSBsplineCurveCR curveB
            );


    //flex !! Predicates

    //flex || bool MSBsplineCurve::AreCompatible (curveA, curveB) ||Test if counts and knot spaces are matched || 
    //flex || bool IsPhysicallyClosed()     || direct comparison of endpoint coordinates ||
    //flex || bool IsClosed ()  || inspects the params.closed flag. ||


    /// <summary>Test if two curves have compatible knots, order, and pole count</summary>
    GEOMDLLIMPEXP static bool AreCompatible
            (
            MSBsplineCurveCR curveA,
            MSBsplineCurveCR curveB
            );

//! Create copies of all curves, with knots added as needed to make them compatible for surface lofting.
GEOMDLLIMPEXP static bool CloneCompatibleCurves
(
bvector<MSBsplineCurvePtr> &outputCurves,/* <= array of output curves */
bvector<MSBsplineCurvePtr> const &inputCurves,           /* => array of input curves */
bool          enableReverse,         /* => allows reversing output */
bool          openAll                /* => forces opening */
);

//! Create copies of all curves via CloneCompatibleCurves.  In addition, do various further steps
//! to reduce redundant knots.
GEOMDLLIMPEXP static bool CloneAndSimplifyCompatibleCurves
(
bvector<MSBsplineCurvePtr> &compatibleCurves,   //!< [out] curves with compatible knots.
bvector<MSBsplineCurvePtr> const &inputCurves,  //!< [in] existing curves
double          tolerance,      //!< [in] approximation tolerance, set to zero for precise compatibility
int             tangentControl, //!< [in] 0: no tangent control, 1: start point, 2: end point, 3: Both ends
bool            keepMagnitude,  //!< [in] true: derivatives maintained at specified ends, false: only directions
int             derivative,     //!< [in] Highest derivatives maintained. Ignored when keepMagnitude is false
bool            fastMode        //!< [in] true: to remove less data but faster
);

//! Create copies of all curves, with knots added as needed to make them arclength compatible.
GEOMDLLIMPEXP static bool CloneArcLengthCompatibleCurves
(
bvector<MSBsplineCurvePtr> &outputCurves,/* <= array of output curves */
bvector<MSBsplineCurvePtr> const &inputCurves,           /* => array of input curves */
bool          enableReverse,         /* => allows reversing output */
bool          openAll                /* => forces opening */
);


    //! Check whether the B-spline curve is physically closed. A B-spline curve may be non-periodic, but still return 
    //! true if its first and last poles coincide. 
    GEOMDLLIMPEXP bool    IsPhysicallyClosed (double tolerance) const;
    //! Check whether the B-spline curve is periodic.
    GEOMDLLIMPEXP bool    IsClosed () const;
    
    //! Check if the B-spline curve is a single quadratic that is a parabola.
    //! The conditions for a parabola are (a) order 3, (b) only 3 poles, (c) weights all 1.
    //! <remarks>This does NOT detect the "weighted" parabola -- a varying-weight order 3 but with weights that are never zero</remarks>
    //! If so return transforms to and from the coordinate system with the vertex at the origin and parabola equation y = c*x^2
    //! <param name="localToWorld">Coordinate frame with origin at the vertex, x axis tangent to the parabola, parabola opens upward along the y axis.</param>
    //! <param name="worldToLocal">Inverse of the localToWorld transform</param>
    //! <param name="vertexFraction">The bspline fraction parameter of the vertex (origin) of the parabola</param>
    //! <param name="localStart">local start point of the curve</param>
    //! <param name="localEnd">local end point of the curve</param>
    //! <param name="xSquaredCoefficient">coefficient for y=c*x^2</param>
    GEOMDLLIMPEXP bool IsParabola (TransformR localToWorld, TransformR worldToLocal,
                double &vertexFraction, DPoint3dR localStart, DPoint3dR localEnd, double &xSquaredCoefficient) const;
    //! Check whether the B-spline curve has stored weights (This does not check if any are other than 1.0)
    GEOMDLLIMPEXP bool    HasWeights () const;



//flex !! Direct Manipulation of MSBsplineCurve Structure

//flex || Action    || method ||
//flex || zero all bits. Normally used just after declaring structure on heap or after freeing heap pointers.   || curve.Zero ()    ||
//flex || swap all bits between two structures || curveA.SwapContents (curveB) ||
//flex || Allocate heap arrays (poles, knots, weights) to match given counts.  Counts are placed in params structure || curve.Allocate (numPoles, order, bool closed, bool rational) ||
//flex || Allocate heap arrays (poles, knots, weights) to match counts in params structure || curve.Allocate ()
//flex || Free heap memory addressed by pointers within the curve structure.  Pointers are cleared to NULL. || curve.ReleaseMem ()


    ///! <summary>Zero out the curve.  This is customarily applied immediately after allocation on stack or heap.
    ///!   This does NOT free memory from prior contents.
    ///! </summary>
    GEOMDLLIMPEXP void Zero ();
    
    //! Exchange all bits with other.   Usually used to transfer poles etc and leave zeros behind.
    GEOMDLLIMPEXP void SwapContents (MSBsplineCurveR other);

    //! Allocate memory arrays to match the current counts.
    GEOMDLLIMPEXP MSBsplineStatus Allocate   ();
    //! Free memory allocated for the poles, weights and knot vector of a B-spline curve. 
    GEOMDLLIMPEXP void            ReleaseMem ();

    //! Zero all, copy counts to params, allocate:
    GEOMDLLIMPEXP MSBsplineStatus Allocate (int numPoles, int order, bool closed, bool rational);

public:
//flex !!!! Independent allocations
//flex These methods allocate pole, knot, weight arrays as directed.   Data is copied from buffers if buffer pointer is non null.
//flex Warning: These methods do not store counts into the params array.
//flex || curve.AllocatePoles () || Allocate poles only    ||
//flex || curve.AllocateKnots () || Allocate knots only    ||
//flex || curve.AllocateWeights () || Allocate weights only    ||

    //! <summary>Allocate pole pointer to specified count.   (Optionally)copy data from callers buffer</summary>
    GEOMDLLIMPEXP MSBsplineStatus AllocatePoles   (int count, DPoint3dCP data = NULL);
    //! <summary>Allocate knot pointer to specified count.   (Optionally)copy data from callers buffer</summary>
    GEOMDLLIMPEXP MSBsplineStatus AllocateKnots   (int count, double const *data = NULL);
    //! <summary>Allocate weight pointer to specified count.   (Optionally)copy data from callers buffer</summary>
    GEOMDLLIMPEXP MSBsplineStatus AllocateWeights (int count, double const *data = NULL);
    //!
public:
    //flex || Full copy (new heap allocations) from existing curve || curve.CopyFrom (source) ||
    //! Allocate memory for the B-spline curve and copies all data from the input B-spline curve.
    GEOMDLLIMPEXP MSBsplineStatus CopyFrom (MSBsplineCurveCR source);
    

    //! Compute uniformly spaced knots.
    //! This uses counts from params.
    //! @return false if param counts are not set.
    GEOMDLLIMPEXP bool ComputeUniformKnots ();
//flex !! Curve calculations

//flex || method    || Remarks  ||
//flex || curve.FractionToPoint (outXYZ, fraction)  ||
//flex || curve.FractionToPoint (outXYZ, outDerivative1, fraction)  ||
//flex || curve.FractionToPoint (outXYZ, weight, outDerivative1, fraction)  ||
//flex || curve.FractionToPoint (outXYZ, outDerivative2, outDerivative2 fraction)  ||
//flex || curve.FractionToPoints (BVectorDPoint3dR points, numPoints)   || Compute numPoints points with equal parameter spacing ||

    //! Fraction parameter to point on the B-spline curve.
    GEOMDLLIMPEXP void FractionToPoint (DPoint3dR xyz, double f) const;
    //! Fraction parameter to both point and derivative.   Derivative is wrt the fraction parameter.
    GEOMDLLIMPEXP void FractionToPoint (DPoint3dR xyz, DVec3dR tangent, double f) const;
    //! Fraction parameter to both point and derivative.   Derivative is wrt the fraction parameter.
    //! the point and derivative are UNWEIGHTED.
    GEOMDLLIMPEXP void FractionToPoint (DPoint3dR xyz, double &weight, DVec3dR tangent, double f) const;
    //! Fraction parameter to point and 2 derivatives.   Derivatives are wrt the fraction parameter.
    GEOMDLLIMPEXP void FractionToPoint (DPoint3dR xyz, DVec3dR dXYZ, DVec3dR ddXYZ, double f) const;
    //! Evaluate a series of points at numPoints parameters. These parameters are spaced evenly throughout the parameter range.
    GEOMDLLIMPEXP void FractionToPoints (bvector<DPoint3d>& points, size_t numPoints);
    //! Evaluate a series of points at given parametrs.
    GEOMDLLIMPEXP void FractionToPoints (bvector<DPoint3d>& points, bvector<double>& fractions);

    //! Calculate the Frenet frame, radius of curvature, and torsion of the B-spline curve at a particular fractional parameter value. 
    GEOMDLLIMPEXP MSBsplineStatus GetFrenetFrame (DVec3dP frame, DPoint3dR point, double& curvature, double& torsion, double u) const;

    //! Calculate the Frenet frame, radius of curvature, and torsion of the B-spline curve at a particular fractional parameter value. 
    GEOMDLLIMPEXP MSBsplineStatus GetFrenetFrame (TransformR frame, double u) const;

    
    //! Calculate the number of derivatives specified by numDervs of the B-spline curve at a particular fraction.
    GEOMDLLIMPEXP MSBsplineStatus ComputeDerivatives (DVec3dP dervs, int numDervs, double fractionParameter) const;



//flex || curveClosestPoint (outPointOnCurve, outFraction, spacePoint() || search curve for point closest to space point ||
    //! find closest point and its fractional parameter on spline
    GEOMDLLIMPEXP void ClosestPoint (DPoint3dR curvePoint, double &fraction, DPoint3dCR spacePoint) const;
    //! find closest point after (optional) viewing matrix.
    GEOMDLLIMPEXP void ClosestPointXY (DPoint3dR curvePoint, double &fraction, double &xyDistance, DPoint3dCR spacePoint, DMatrix4dCP viewMatrix) const;

//flex || knotValue = curve.FractionToKnot (fraction)   || convert fractional position to knot space ||
//flex || fraction = curve.KnotToFraction (knotValue)   || convert knot value to fractional position ||


    //! Return the knot value at a fractional parameter ...
    GEOMDLLIMPEXP double FractionToKnot (double f) const;
    //! Return the fractional parameter corresponding to a knot value ...
    GEOMDLLIMPEXP double KnotToFraction (double knot) const;

    //! @param [in,out] params on inputs, fractions from a bezier known to map from [knot0,knot1].
    //! @param [in,out] derivatives derivatives to be scaled
    //! @param [in] i0 first index to map
    //! @param [in] knot0 left knot of fraction space.
    //! @param [in] knot1 right knot of fraction space.
    //! @param [in] select selects mapping destination.  One of CURVE_PARAMETER_MAPPING_BezierFraction, CURVE_PARAMETER_MAPPING_CurveKnot, CURVE_PARAMETER_MAPPING_CurveFraction.
    //! @param [in] curve required if if mapping to curve fraction.
    static GEOMDLLIMPEXP void MapFractions (bvector<double> *params, bvector<DVec3d> *derivatives, size_t i0, double knot0, double knot1, CurveParameterMapping select, MSBsplineCurveCP curve);

    //! Return the knot values corresponding to fraction 0 and fraction 1 ...
    GEOMDLLIMPEXP void   GetKnotRange (double &knotA, double &knotB) const;
    //! Return the knot range as DSegment1d..
    GEOMDLLIMPEXP DSegment1d GetKnotRange () const;
    //! Return the knot values and indices corresponding to fraction 0 and fraction 1, and also the equal-knot tolerance ...
    GEOMDLLIMPEXP void   GetKnotRange (double &knotA, double &knotB, int &indexA, int &indexB, double &knotTolerance) const;
    //! return knot by index. returns 0 if out of range.  (Use NumberAllocatedKnots to determine index range).
    GEOMDLLIMPEXP double GetKnot (size_t index) const;
    
    //! Compute blending functions at KNOT value (unnormalized)
    //! knotIndex is the index of the knot to the right of u. (i.e. the leftmost knot of the upper limits of active windows)
    GEOMDLLIMPEXP void KnotToBlendFunctions (double *blend, double *blendDerivative, size_t &knotIndex, double u) const;
    //! return pole by index. returns 0 point if out of range. (Use NumberAllocatedPoles to determine index range).
    //! @DotNetMethodExclude
    GEOMDLLIMPEXP DPoint3d GetPole (size_t index) const;
    GEOMDLLIMPEXP DPoint3d GetPole (int index) const;
    //! return pole by index, counting from the last pole . (i.e. index 0 is the final weight) Returns 0 point if out of range. (Use NumberAllocatedPoles to determine index range).
    //! @DotNetMethodExclude
    GEOMDLLIMPEXP DPoint3d GetReversePole (size_t index) const;
    GEOMDLLIMPEXP DPoint3d GetReversePole (int index) const;
    //! return weight by index, counting from the last weight. (i.e. index 0 is the final weight) Returns 1.0 if out of range. (Use NumberAllocatedPoles to determine index range).
    //! @DotNetMethodExclude
    GEOMDLLIMPEXP double GetReverseWeight (size_t index) const;
    GEOMDLLIMPEXP double GetReverseWeight (int index) const;


    //! return pole by index. returns 0 point if out of range.
    //!  If spline is weighted, the weight is divided out.  If weight zero, no division happens.
    //! (Use NumberAllocatedPoles to determine index range).
    //! @DotNetMethodExclude
    GEOMDLLIMPEXP DPoint3d GetUnWeightedPole (size_t index, bool reverse = false) const;
    GEOMDLLIMPEXP DPoint3d GetUnWeightedPole (int index, bool reverse = false) const;

    //! Apply a transform to a single pole.
    GEOMDLLIMPEXP void TransformPoles (TransformCR transform, size_t index, size_t n);

    //! return pointer to contiguous poles.
    //! @DotNetMethodExclude
    GEOMDLLIMPEXP DPoint3d const *  GetPoleCP () const;
    //! return pointer to contiguous weights
    //! @DotNetMethodExclude
    GEOMDLLIMPEXP double const *    GetWeightCP () const;
    //! return pointer to contiguous knots.
    //! @DotNetMethodExclude
    GEOMDLLIMPEXP double const *    GetKnotCP () const;

    //! return pointer to contiguous poles.
    //! @DotNetMethodExclude
    GEOMDLLIMPEXP DPoint3d *  GetPoleP () const;
    //! return pointer to contiguous weights
    //! @DotNetMethodExclude
    GEOMDLLIMPEXP double *    GetWeightP () const;
    //! return pointer to contiguous knots.
    //! @DotNetMethodExclude
    GEOMDLLIMPEXP double *    GetKnotP () const;

    GEOMDLLIMPEXP DRange1d GetWeightRange () const;
    //! set pole by index. returns false if index out of range.
    //! @DotNetMethodExclude
    GEOMDLLIMPEXP bool SetPole (size_t index, DPoint3dCR value);
    GEOMDLLIMPEXP bool SetPole (int index, DPoint3dCR value);
    //! set pole by index. returns false if index out of range.  If the curve is weighted, the current weight is multiplied
    //! into the input pole.
    //! @DotNetMethodExclude
    GEOMDLLIMPEXP bool SetReWeightedPole (size_t index, DPoint3dCR value, bool reverse = false);
    GEOMDLLIMPEXP bool SetReWeightedPole (int index, DPoint3dCR value, bool reverse = false);


    //! set pole by index. returns false if index out of range.
    //! @DotNetMethodExclude
    GEOMDLLIMPEXP bool SetPole (size_t index, double x, double y, double z);

    //! set weight by index. returns false if index out of range.
    //! @DotNetMethodExclude
    GEOMDLLIMPEXP bool SetWeight (size_t index, double value);
    GEOMDLLIMPEXP bool SetWeight (int index, double value);
    //! set weight by index. returns false if index out of range.
    //! @DotNetMethodExclude
    GEOMDLLIMPEXP bool SetKnot (size_t index, double value);

    //! set pole by index. returns false if index out of range.
    //! @DotNetMethodExclude
    GEOMDLLIMPEXP bool SetPoles (size_t index, DPoint3dCP value, size_t n);
    //! set weight by index. returns false if index out of range.
    //! @DotNetMethodExclude
    GEOMDLLIMPEXP bool SetWeights (size_t index, double const * value, size_t n);
    //! set weight by index. returns false if index out of range.
    //! @DotNetMethodExclude
    GEOMDLLIMPEXP bool SetKnots (size_t index, double const * value, size_t n);

    //! if the curve is rational, divide (wx,wy,wz) style poles by the weights
    GEOMDLLIMPEXP void UnWeightPoles ();
    //! if the curve is rational, multiply (wx,wy,wz) style poles by the weights
    GEOMDLLIMPEXP void WeightPoles ();
    

    //! return pole by index. returns 0 point if out of range. (Use NumberAllocatedPoles to determine index range).
    //! @DotNetMethodExclude
    GEOMDLLIMPEXP DPoint4d GetPoleDPoint4d (size_t index) const;
    //! return weight by index. returns 1 if index out of range or curve is unweighted.  (Use NumberAllocatedPoles to determine index range).
    GEOMDLLIMPEXP double GetWeight (size_t index) const;
    GEOMDLLIMPEXP double GetWeight (int index) const;
    //! Copy all poles out into caller array, dividing each by its weight
    GEOMDLLIMPEXP void GetUnWeightedPoles (bvector<DPoint3d> &outData) const;
    //! Copy all poles out into caller array.
    GEOMDLLIMPEXP void GetPoles (bvector<DPoint3d> &outData) const;
    //! Copy all weights out into caller array.
    GEOMDLLIMPEXP void GetWeights (bvector<double> &outData) const;
    //! Copy all knots out into caller array.
    GEOMDLLIMPEXP void GetKnots (bvector<double> &outData) const;

    //! compress knot values and mark active range.
    //! @param [in] inKnot input knot values.
    //! @param [in] order curve order, used for analyzing active interval.
    //! @param [out] outKnot compressed knot values
    //! @param [out] multiplicities number of occurrences
    //! @param [out] lowActiveIndex index (in outKnots) of left end of active range.
    //! @param [out] highActiveIndex index (in outKnots) of right end of active range.
    //! @remark A "Clamped and open" knot vector will have trivial indices 0 and {outKnots.size ()-1}.  Periodic or unclamped will have other indices.
    //! @remark This uses the MSBsplineCurve::AreSameKnots for tolerancing.
    //! @remark This metohd does not try to "correct" issues like excess multiplicity and clamping.  It just counts the multiplicity according to AreSameKnots.
    //! @remark The input and output knot vectors must be distinct arrays !!!
    static GEOMDLLIMPEXP void CompressKnots (
            bvector <double> const &inKnot,
            int order,
            bvector<double> &outKnot,
            bvector<size_t>&multiplicities,
            size_t &lowActiveIndex,
            size_t &highActiveIndex
            );

    
    //! Copy all poles out into caller array.
    GEOMDLLIMPEXP void GetPoles4d (bvector<DPoint4d> &outData) const;


//flex !!!! Spline queries

//flex || Query || Remarks ||
//flex || int GetOrder ()   || Order is one more than the degree of the curve.  ||
//flex || int NumberAllocatedKnots ()   || This is the number of knots as required by the pole count, order, and closure.  (Do not use the params.numKnots value) ||
//flex || int NumberAllocatedPoles ()   || (matches params.numPoles) ||
//flex || bool AreKnotsValid () || check counts and knot values ||
//
    //! return the curve order.
    GEOMDLLIMPEXP int GetIntOrder () const;
    GEOMDLLIMPEXP size_t GetOrder () const;

    //! (Deprecated --- use GetNumKnots or GetIntNumKnots) Return the true allocated size of the knot array....
    GEOMDLLIMPEXP int    NumberAllocatedKnots () const;
    //! (Deprecated --- use GetNumPoles or GetIntNumPoles)
    //! Return the true allocated size of the pole array....
    GEOMDLLIMPEXP int    NumberAllocatedPoles () const;

    GEOMDLLIMPEXP size_t  GetNumPoles () const;
    GEOMDLLIMPEXP size_t  GetNumKnots () const;

    GEOMDLLIMPEXP int     GetIntNumPoles () const;
    GEOMDLLIMPEXP int     GetIntNumKnots () const;

    //! Return false if knot counts or values are invalid.
    GEOMDLLIMPEXP bool AreKnotsValid (bool clampingRequired = true) const;

    //! Return curve display flag.
    GEOMDLLIMPEXP bool GetCurveDisplay () const;
    //! Return polygon display flag.
    GEOMDLLIMPEXP bool GetPolygonDisplay () const;

    //! Set the curve display flag.
    GEOMDLLIMPEXP void SetCurveDisplay (bool value);
    //!  Set the polygon display flag.
    GEOMDLLIMPEXP void SetPolygonDisplay (bool value);



//flex !!!! Knot modifications
    //! rewrite knot values in a..b.  Return true if a,b and current start,end define a valid scale factor
    GEOMDLLIMPEXP bool MapKnots (double a, double b);
    //! Add a given knot value to the B-spline curve. that newMultiplicity is the desired final multiplicity of a knot 
    //! that may already exist. 
    GEOMDLLIMPEXP MSBsplineStatus AddKnot (double unnormalizedKnotValue, int newMultiplicity);
    //! Normalize knots to 01
    GEOMDLLIMPEXP void NormalizeKnots ();
    GEOMDLLIMPEXP void ComputeGrevilleAbscissa (bvector<double> &averageKnots)  const;
    // Compute the Greville knots (moving average of (order-1) consecutive knots) in a uniform knot sequence from 0 to 1
    static GEOMDLLIMPEXP void ComputeUniformKnotGrevilleAbscissa (bvector<double> &averageKnots, size_t numInterval, int order);
    
//flex !!!! Modifications as copy operations

//flex These work in the C-style -- the instance is assumed uninitialized on call.   (1) Declare an uninitialized MSBsplineCurve on the stack (2) call a function that makes its instance the target.
//flex Caller is responsible for (1) freeing prior contents (ReleaseMem) of the struct if it was already in use and (2) final ReleaseMem for the curve returned here.

//flex || method || remarks ||
//flex || destCurve.CopyOpen (source, unnormalizedSplitKnot)    || copy, converting to open from specified start knot if possible ||
//flex || destCurve.CopyClosed (source) || copy, converting to periodic if possible ||
//flex || destCurve.CopyReversed (source) || copy, reversing the knots and poles space ||
//flex || destCurve.CopySegment (source, knotA, knotB)  || copy portion specified in knot values ||
//flex || destCurve.CopyFractionSegment (source, fractionA, fractionB)  || copy portion specified in fractional knot values ||
//flex || destCurve.CopyTransformed || copy with linear transform ||

    //! Create B-spline curve by opening a closed B-spline curve.
    GEOMDLLIMPEXP MSBsplineStatus CopyOpen (MSBsplineCurveCR source, double unnormalizedKnot);
    //! Create B-spline curve by closing a open B-spline curve.
    GEOMDLLIMPEXP MSBsplineStatus CopyClosed (MSBsplineCurveCR source);
    //! Create B-spline curve by reserving the direction of a B-spline curve.
    GEOMDLLIMPEXP MSBsplineStatus CopyReversed (MSBsplineCurveCR source);
    //! Create B-spline curve by extracting a part of a B-spline curve.
    GEOMDLLIMPEXP MSBsplineStatus CopySegment (MSBsplineCurveCR source, double unnormalizedKnotA, double unnormalizedKnotB);

    //! Create B-spline curve by extracting a part of a B-spline curve.
    GEOMDLLIMPEXP MSBsplineStatus CopyFractionSegment (MSBsplineCurveCR source, double fractionA, double fractionB);

    //! Create B-spline curve by transforming a B-spline curve.
    GEOMDLLIMPEXP MSBsplineStatus CopyTransformed (MSBsplineCurveCR source, TransformCR trans);


//flex !!! Bezier segments

//flex  The ith bezier segment is is affected by by (order) poles starting at index (order+i) and (2*order-1) knots.
//flex

//flex A BezierSegment (named segment etc) is a structure that carries poles, weights, knots, order, and indices back into its spline.

//flex || method    || remarks ||
//flex || bool curve.GetSupport (bvector<DPoint4d> ""& outPoles, outKnots, bezierSelect)    || Get support to arrays.||
//flex || bool GetSupport (outSegment, bezierSelect) || Get support to working structure.  ||
//flex || bool AdvanceToBezier (outSegment, inoutBezierSelect, bool saturateKnots)  || Starting at inoutBezierSelect, look for a bezier with nonzero knot length. ||
//flex || bool AdvanceToBezierInKnotInterval (outSegment, inoutBezierSelect, knotRange)  || Starting at inoutBezierSelect, look for (and saturate) a bezier with nonzero knot length and overlapping specified knot interval. ||
//flex || size_t CountDistinctBeziers ()    || Count how many locally supported beziers have nonzero knot interval. ||
//flex || int FindKnotInterval (t) || return knot index to left of parameter. ||

    //! Extract the {order} poles and {2*(order-1)} knots that support a single bezier interval ...
    //! @param [out] outPoles poles
    //! @param [out] outKnots knots
    //! @param [in] bezierSelect index of bezier to extract.
    //! @DotNetMethodExclude
    GEOMDLLIMPEXP bool GetSupport (bvector<DPoint4d>&outPoles, bvector<double>&outKnots, size_t bezierSelect) const;
    
    //! Extract the {order} poles and {2*(order-1)} knots that support a single bezier interval ...
    //! @param [out] segment extracted data.
    //! @param [in] bezierSelect index of bezier to extract.
    //! @DotNetMethodExclude
    GEOMDLLIMPEXP bool GetSupport (BCurveSegmentR segment, size_t bezierSelect) const;
    //! Get poles for a single bezier poles from the curve.
    //! return false if invalid bezierSelect.  Note that the bezierSelect for a high multiplicity knot returns true for the function
    //!    but marks the interval as null.   Normal usage is to loop over all beziers in a bspline but skip processing the null intervals.
    //! @param [out] segment a filled BCurveSegment.
    //! @param [in] bezierSelect selects a bezier interval within the bspline
    //! @DotNetMethodExclude
    GEOMDLLIMPEXP bool GetBezier (BCurveSegment& segment, size_t bezierSelect) const;
    //! Count the number of beziers that have non-null knot intervals.
    GEOMDLLIMPEXP size_t CountDistinctBeziers () const;
    //! Return the index of the knot at the left of the interval containing specified knot.
    //! When knotValue exactly matches a knot, the returned index is of the knot to the left -
    //!   i.e. knotValue appears at the RIGHT of the returned interval.
    //! (favor  knot[index] < knotValue <= knot[index + 1])
    GEOMDLLIMPEXP size_t FindKnotInterval (double knotValue) const;

    //! Move forward looking for a bezier segment of the curve.  Skip over nulls.
    //! return false if no bezier found.
    //! @param segment returned bezier
    //! @param [in,out] bezierSelect on input, first bezier to examine.
    //!     On output, this is advanced to the "next" bezier to examine.
    //! @param [in] saturateKnots true to saturate knots to proper bezier, false to leave raw support and knots.
    bool AdvanceToBezier(BCurveSegment& segment, size_t &bezierSelect, bool saturateKnots = true) const;

    //! Move forward looking for a bezier segment that overlaps a knot interval.  Skip over nulls and
    //!  "earlier" knot intervals.
    //! return false if no bezier found.
    //! @param segment returned saturated bezier, subdivided to match a portion of the knot interval.
    //! @param [in,out] bezierSelect on input, first bezier to examine.
    //!     On output, this is advanced to the "next" bezier to examine.
    //! @param [in] interval live knot interval.
    GEOMDLLIMPEXP bool AdvanceToBezierInKnotInterval (
            BCurveSegment& segment,
            size_t &bezierSelect,
            DRange1dCR interval
            ) const;

    //! Move forward looking for a bezier segment that overlaps a knot interval.  Skip over nulls and
    //!  "earlier" knot intervals.
    //! return false if no bezier found.
    //! @param segment returned saturated bezier, subdivided to match a portion of the knot interval.
    //! @param [in,out] bezierSelect on input, first bezier to examine.
    //!     On output, this is advanced to the "next" bezier to examine.
    //! @param [in] interval live knot interval.
    GEOMDLLIMPEXP bool AdvanceToBezierInFractionInterval (
            BCurveSegment& segment,
            size_t &bezierSelect,
            DRange1dCR interval
            ) const;

    //! return an index to use to start "Retreat" order bezier access.
    GEOMDLLIMPEXP size_t GetTailBezierSelect () const;
    //! @param[in] bezierSelect "retreat" order bezier index.  Initialie with GetTailBezierSelect ()
    //! @param [in] interval knot interval that must be overlapped.
    //! @param [out] segment saturated bezier for segment.
    GEOMDLLIMPEXP bool RetreatToBezierInKnotInterval (
            BCurveSegment& segment,
            size_t &bezierSelect,
            DRange1dCR interval
            ) const;

    //! enumeration of possible classifications of a knot value wrt to the knot array.
    typedef enum {
        KNOTPOS_BEFORE_START = 0,
        KNOTPOS_START,
        KNOTPOS_INTERVAL,
        KNOTPOS_INTERIOR,
        KNOTPOS_FINAL,
        KNOTPOS_AFTER_FINAL
        } KnotPosition;
    //! Search for a knot value.
    //!  If the value is strictly between knots (KNOTPOS_INTERVAL), return the immediate surrounding knot indices.
    //!  In all other cases (KNOTPOS_BEFORE_START, KNOTPOS_START, KNOTPOS_INTERIOR, KNOTPOS_FINAl, KNOTPOS_AFTER_FINAL)
    //!      return first and last indices of identical knots.   Hence k1-k0+1 is the multiplicity of the matched knot.
    GEOMDLLIMPEXP KnotPosition SearchKnot (double unnormalizedKnotValue, int &k0, int &k1, double &correctedKnotValue) const;

//flex !! Measurements

//flex || Method || Remarks ||
//flex || a = curve.Length ()   || Measure length.   This is near-machine precision.   Expensive. ||
//flex || a = curve.LengthBetweenFractions (startFraction, endFraction) || Length between fraction positions ||
//flex || bool curve.FractionAtSignedDistance (startFraction, signedDistance, outEndFraction, outDistanceMoved) || May be incomplete due to end of curve ||
//flex 
    //! compute the length of the B-spline curve.
    GEOMDLLIMPEXP double Length () const;

    //! compute the length of the transformed bspline curve
    GEOMDLLIMPEXP double Length (RotMatrixCP worldToLocal) const;

    //! compute the length of the B-spline curve at a given knot interval [startKnot, endKnot].
    GEOMDLLIMPEXP double LengthBetweenKnots (double startKnot, double endKnot) const;

    //! compute the length of the B-spline curve at a given knot interval [startKnot, endKnot].
    GEOMDLLIMPEXP double LengthBetweenKnots (RotMatrixCP worldTolocal, double startKnot, double endKnot) const;

    //! compute the length of the B-spline curve at a given fraction interval [startFraction, endFraction].
    GEOMDLLIMPEXP double LengthBetweenFractions (double startFraction, double endFraction) const;

    //! compute the length of the B-spline curve at a given fraction interval [startFraction, endFraction].
    GEOMDLLIMPEXP double LengthBetweenFractions (RotMatrixCP worldTolocal, double startFraction, double endFraction) const;

    
    //! Move by (up to !!) signedDistance along the curve.  Stop at endpoint if encountered before the distance.
    //! return true if full movement.
    //! @param [in] startParam starting position (in 0..1 parameter space)
    //! @param [in] signedDistance requested distance (sign indictes direction relative to parameter space)
    //! @param [out] endParam parameter where movement stopped.
    //! @param [out] actualSignedDistance distance to actual stopping place.
    GEOMDLLIMPEXP bool FractionAtSignedDistance (double startParam, double signedDistance, double &endParam, double &actualSignedDistance) const;

    //! Move by (up to !!) signedDistance along the curve.  Stop at endpoint if encountered before the distance.
    //! return true if full movement.
    //! @param [in] worldToLocal transformation for tangent vectors.
    //! @param [in] startParam starting position (in 0..1 parameter space)
    //! @param [in] signedDistance requested distance (sign indictes direction relative to parameter space)
    //! @param [out] endParam parameter where movement stopped.
    //! @param [out] actualSignedDistance distance to actual stopping place.
    GEOMDLLIMPEXP bool FractionAtSignedDistance (RotMatrixCP worldToLocal, double startParam, double signedDistance, double &endParam, double &actualSignedDistance) const;
    
    
    //! Calculate the parameters and location of the all inflection points of a B-spline curve.
    GEOMDLLIMPEXP MSBsplineStatus ComputeInflectionPoints (bvector<DPoint3d>& points, bvector<double>& params);

    //! Calculate the parameters and location of the all inflection points of a B-spline curve.
    GEOMDLLIMPEXP MSBsplineStatus ComputeInflectionPointsXY (bvector<DPoint3d>& points, bvector<double>& params, RotMatrixCP transform);
    
    //! Compute intersections with plane.
    //! @param [out] points optional array to receive points.
    //! @param [out] fractionParameters optional array to receive fractional params
    //! @param [in] plane
    GEOMDLLIMPEXP void AddPlaneIntersections (bvector<DPoint3d>* points, bvector<double> *fractionParameters, DPlane3dCR plane) const;

    //! Compute intersections with plane (plane in homogeneous form)
    //! @param [out] points optional array to receive points.
    //! @param [out] fractionParameters optional array to receive fractional params
    //! @param [in] planeCoffs homogeneous plane coefficients
    GEOMDLLIMPEXP void AddPlaneIntersections (bvector<DPoint3d>* points, bvector<double> *fractionParameters, DPoint4dCR planeCoffs) const;

    //! Compute intersections with line segment, as viewed in xy plane.
    //! @param [out] curvePoints points on curve
    //! @param [out] curveFractions fractions on curve
    //! @param [out] linePoints points on line
    //! @param [out] lineFractions fractions on line.
    //! @param [in] segment line segment to intersect
    //! @param [in] extendSegment true to consider the extended line.
    //! @param [in] matrix (optional) matrix to apply to both the curve and segment to get to as-viewed coordinates for xy calculations.
    GEOMDLLIMPEXP void AddLineIntersectionsXY (bvector<DPoint3d> *curvePoints, bvector<double> *curveFractions,
                                 bvector<DPoint3d> *linePoints, bvector<double> *lineFractions,
                                 DSegment3dCR segment, bool extendSegment, DMatrix4dCP matrix) const;


    //! Compute intersections with a linestring, as viewed in xy plane.
    //! @param [out] curveAPoints points on curve
    //! @param [out] curveAFractions fractions on curve
    //! @param [out] curveBPoints points on linestring
    //! @param [out] curveBFractions fractions on linestring.
    //! @param [in] linestring LineString to intersect
    //! @param [in] matrix (optional) matrix to apply to both the curve and linestring to get to as-viewed coordinates for xy calculations.
    GEOMDLLIMPEXP void AddLinestringIntersectionsXY (
                    bvector<DPoint3d> *curveAPoints,  bvector<double> *curveAFractions,
                    bvector<DPoint3d> *curveBPoints,  bvector<double> *curveBFractions,
                    bvector<DPoint3d> const &linestring,
                    DMatrix4dCP matrix
                    ) const;

    //! Compute intersections with a linestring, as viewed in xy plane.
    //! @param [out] curveAPoints points on curve
    //! @param [out] curveAFractions fractions on curve
    //! @param [out] curveBPoints points on linestring
    //! @param [out] curveBFractions fractions on linestring.
    //! @param [in] linestring LineString to intersect
    //! @param [in] extendLinestring true to extend ends of the linestring.
    //! @param [in] matrix (optional) matrix to apply to both the curve and linestring to get to as-viewed coordinates for xy calculations.
    GEOMDLLIMPEXP void AddLinestringIntersectionsXY (
                    bvector<DPoint3d> *curveAPoints,  bvector<double> *curveAFractions,
                    bvector<DPoint3d> *curveBPoints,  bvector<double> *curveBFractions,
                    bvector<DPoint3d> const &linestring,
                    bool extendLinestring,
                    DMatrix4dCP matrix
                    ) const;


    //! Compute intersections with ellipse, as viewed in xy plane.
    //! @param [out] curvePoints points on curve
    //! @param [out] curveFractions fractions on curve
    //! @param [out] ellipsePoints points on ellipse
    //! @param [out] ellipseFractions fractions on ellipse.
    //! @param [in] arc arc to intersect
    //! @param [in] extendConic true to consider the extended ellipse.
    //! @param [in] matrix (optional) matrix to apply to both the curve and arc to get to as-viewed coordinates for xy calculations.
    GEOMDLLIMPEXP void AddArcIntersectionsXY (bvector<DPoint3d> *curvePoints, bvector<double> *curveFractions,
                                 bvector<DPoint3d> *ellipsePoints, bvector<double> *ellipseFractions,
                                 DEllipse3dCR arc, bool extendConic, DMatrix4dCP matrix) const;

    //! Compute intersections with ellipse, as viewed in xy plane.
    //! @param [out] curveAPoints points on instances curve
    //! @param [out] curveAFractions fractions on instance curve
    //! @param [out] curveBPoints points on second curve
    //! @param [out] curveBFractions fractions on second curve
    //! @param [in] curveB second bspline curve
    //! @param [in] matrix (optional) matrix to apply to both the two curves to get to as-viewed coordinates for xy calculations.
    GEOMDLLIMPEXP void AddCurveIntersectionsXY (bvector<DPoint3d> *curveAPoints, bvector<double> *curveAFractions,
                                 bvector<DPoint3d> *curveBPoints, bvector<double> *curveBFractions,
                                 MSBsplineCurveCR curveB, DMatrix4dCP matrix) const;


    //! Compute intersections with ellipse, as viewed in xy plane.
    //! @param [out] curveAPoints points on instances curve
    //! @param [out] curveAFractions fractions on instance curve
    //! @param [out] curveAOverlapFractions intervals of fractional overlap on instance curve
    //! @param [out] curveBPoints points on second curve
    //! @param [out] curveBFractions fractions on second curve
    //! @param [out] curveBOverlapFractions intervals of fractional overlap on second curve
    //! @param [in] curveB second bspline curve
    //! @param [in] matrix (optional) matrix to apply to both the two curves to get to as-viewed coordinates for xy calculations.
    GEOMDLLIMPEXP void AddCurveIntersectionsXY (
            bvector<DPoint3d> *curveAPoints, bvector<double> *curveAFractions, bvector<DSegment1d> *curveAOverlapFractions,
            bvector<DPoint3d> *curveBPoints, bvector<double> *curveBFractions, bvector<DSegment1d> *curveBOverlapFractions,
            MSBsplineCurveCR curveB, DMatrix4dCP matrix) const;


    //! Find full 3d cusps.
    //! @param [out] points array to receive xyz of cusps.
    //! @param [out] fractionParameters array to receive fraction parameters of cusps.
    GEOMDLLIMPEXP void AddCusps (bvector<DPoint3d>*points, bvector<double> *fractionParameters) const;

    //! Find full cusps as viewed in xy.
    //! @param [out] points array to receive xyz of cusps.
    //! @param [out] fractionParameters array to receive fraction parameters of cusps.
    //! @param [in] matrix optional transformation into viewing space.
    GEOMDLLIMPEXP void AddCuspsXY (bvector<DPoint3d>*points, bvector<double> *fractionParameters, DMatrix4dCP matrix) const;

    //! Compute points along the bspline, spaced to have equal point-to-point distance (chordLength)
    //! Stroke length is measured along the stroke, NOT along the arc.
    //! The first point will be the start point of the curve.
    //! The last point is usually NOT the end point of the curve.
    GEOMDLLIMPEXP bool StrokeWithFixedChordLength
    (
    bvector<DPoint3d> &points,  //!< [out] stroke points
    bvector<double> &params,    //!< [out] fraction parameters for the strokes
    double          chordLength //!< [in] stroke length.
    );

    //! Compute points along the bspline, spaced to have equal point-to-point distance (chordLength)
    //! Stroke length is measured along the stroke, NOT along the arc.
    //! Stroke length adapts to the count and shape.
    //! The first point will be the start point of the curve.
    //! The last point is the end of the curve.
    GEOMDLLIMPEXP bool StrokeFixedNumberWithEqualChordLength
    (
    bvector<DPoint3d> &points,  //!< [out] stroke points
    bvector<double> &params,    //!< [out] fraction parameters for the strokes
    size_t          numSeg      //!< [in] segment count
    );

    //! Compute points along the bspline, spaced to have equal chord error (true perpendicular from chord to curve)
    //! Stroke length adapts to the count and shape.
    //! The first point will be the start point of the curve.
    //! The last point is the end of the curve.
    GEOMDLLIMPEXP bool StrokeFixedNumberWithEqualChordError
    (
    bvector<DPoint3d> &points,  //!< [out] stroke points
    bvector<double> &params,    //!< [out] fraction parameters for the strokes
    size_t          numSeg      //!< [in] segment count
    );

    //! Compute points along the bspline, spaced to have equal fraction spacing.
    //! The first point will be the start point of the curve.
    //! The last point is the end of the curve.
    GEOMDLLIMPEXP void StrokeFixedNumberWithEqualFractionLength
    (
    bvector<DPoint3d> &points,  //!< [out] stroke points
    bvector<double> &params,    //!< [out] fraction parameters for the strokes
    size_t          numSeg      //!< [in] segment count
    );

    //! Open the closed B-spline curve.
    GEOMDLLIMPEXP MSBsplineStatus MakeOpen (double u);
    //! Close the open B-spline curve.
    GEOMDLLIMPEXP MSBsplineStatus MakeClosed ();
    //! Reverse the direction of the B-spline curve.
    GEOMDLLIMPEXP MSBsplineStatus MakeReversed ();
    //! Make an equivalent rational B-spline curve.
    GEOMDLLIMPEXP MSBsplineStatus MakeRational ();
    
    //! Exteact the curve from the B-spline curve at the interval [unnormalizedKnotA, unnormalizedKnotB].
    GEOMDLLIMPEXP MSBsplineStatus ExtractSegmentBetweenKnots (MSBsplineCurveR target, double unnormalizedKnotA, double unnormalizedKnotB);
    //! Extract the start or end point of the B-spline curve.
    GEOMDLLIMPEXP void ExtractEndPoints (DPoint3dR start, DPoint3dR end) const;
    //! Calculate the normal and center of a plane containing the B-spline curve.
    GEOMDLLIMPEXP MSBsplineStatus ExtractCurveNormal (DVec3dR normal, DPoint3dR position, double &planarDeviation) const;

    //! Compute stroke approximation
    //! @param [out] points array to receive points.
    //! @param [in] chordTol target distance from stokes to curve
    //! @param [in] angleTol target angle between strokes
    //! @param [in] maxEdgeLength target edge length
    //! @param [in] includeStartPoint false to skip the start point, e.g. when this spline is chained after another curve.
    GEOMDLLIMPEXP void AddStrokes (bvector <DPoint3d> &points,
                double chordTol = 0.0, double angleTol = 0.20, double maxEdgeLength = 0.0, bool includeStartPoint = true) const;

    //! Compute strokes at uniform fraction step.
    //! @param [out] points array to receive points.
    //! @param [out] fractions array to receive fractions.
    //! @param [in] numPoints number of points.
    GEOMDLLIMPEXP void PointsAtUniformFractions (bvector <DPoint3d> &points, bvector<double> &fractions, size_t numPoints) const;

    //! Compute points at uniform arclength steps.
    //! @param [out] points array to receive points.
    //! @param [out] fractions array to receive fractions.
    //! @param [in] numPoints number of points.
    GEOMDLLIMPEXP bool PointsAtUniformArcLength (bvector <DPoint3d> &points, bvector<double> &fractions, size_t numPoints) const;

    //! Compute stroke approximation
    //! @param [out] points array to receive points.
    //! @param [out] derivatives optional array to receive derivative vectors
    //! @param [out] params optional array to receive parameters
    //! @param [in] parameterSelect parameter mapping control
    //! @param [in] chordTol target distance from stokes to curve
    //! @param [in] angleTol target angle between strokes
    //! @param [in] maxEdgeLength target edge length
    //! @param [in] includeStartPoint false to skip the start point, e.g. when this spline is chained after another curve.
    GEOMDLLIMPEXP void AddStrokes
                (
                bvector <DPoint3d> &points,
                bvector <DVec3d> *derivatives = NULL,
                bvector <double> *params = NULL,
                double chordTol = 0.0,
                double angleTol = 0.20,
                double maxEdgeLength = 0.0,
                bool includeStartPoint = true,
                CurveParameterMapping parameterSelect = CURVE_PARAMETER_MAPPING_CurveKnot
                ) const;

    //! Compute stroke approximation
    //! @param [out] points array to receive points.
    //! @param [out] derivatives optional array to receive derivative vectors
    //! @param [out] params optional array to receive parameters
    //! @param [in] options tolerance options.
    //! @param [in] includeStart false to omit start point.
    GEOMDLLIMPEXP void AddStrokes
                (
                IFacetOptionsCR options,
                bvector <DPoint3d> &points,
                bvector <DVec3d> *derivatives = NULL,
                bvector <double> *params = NULL,
                bool includeStart = true
                ) const;



    //! Compute a fixed number of points at regularly spaced fractional parameters
    GEOMDLLIMPEXP void AddStrokes
                (
                size_t numPoints,
                bvector <DPoint3d> &points,
                bvector <DVec3d> *derivatives = NULL,
                bvector <double> *params = NULL,
                bool includeStartPoint = true,
                CurveParameterMapping parameterSelect = CURVE_PARAMETER_MAPPING_CurveKnot
                ) const;


    //! Compute stroke count
    GEOMDLLIMPEXP size_t GetStrokeCount (double chordTol = 0.0, double angleTol = 0.20, double maxEdgeLength = 0.0) const;

    //! For space point Q (spacePoint), find all curve points X where line XQ is tangent to the curve.
    //! @param [out] points array to receive points.
    //! @param [out] fractions array to receive fraction parameters.
    //! @param [in] spacePoint space point.
    //! @param [in] matrix optional transformation into viewing space.
    GEOMDLLIMPEXP void AllTangentsXY (bvector<DPoint3d>& points, bvector<double>& fractions, DPoint3dCR spacePoint, DMatrix4dCP matrix) const;

    //! For space point Q, find all curve points X where line XQ is tangent to the curve.
    //! @param [out] points array to receive points.
    //! @param [out] fractions array to receive fraction parameters.
    //! @param [in] spacePoint space point.
    GEOMDLLIMPEXP void AllTangents (bvector<DPoint3d>& points, bvector<double>& fractions, DPoint3dCR spacePoint) const;

    //! For space point Q (spacePoint), find a curve point X where line XQ is tangent to the curve. Point X is close to biasPoint.
    //! @param [out] curvePoint tangential point.
    //! @param [out] curveFraction fraction parameter of tangential point.
    //! @param [in] spacePoint space point.
    //! @param [in] biasPoint bias point.
    //! @param [in] matrix optional transformation into viewing space.
    GEOMDLLIMPEXP bool ClosestTangentXY (DPoint3dR curvePoint, double &curveFraction, DPoint3dCR spacePoint, DPoint3dCR biasPoint, DMatrix4dCP matrix) const;
    
     //! For space point Q (spacePoint), find a curve point X where line XQ is tangent to the curve. Point X is close to biasPoint.
    //! @param [out] curvePoint tangential point.
    //! @param [out] curveFraction fraction parameter of tangential point.
    //! @param [in] spacePoint space point.
    //! @param [in] biasPoint bias point.
    GEOMDLLIMPEXP bool ClosestTangent (DPoint3dR curvePoint, double &curveFraction, DPoint3dCR spacePoint, DPoint3dCR biasPoint) const;

    //! Find all curve points X where the tangents is parallel to the given vector.
    //! @param [out] points array to receive points.
    //! @param [out] fractions array to receive fraction parameters.
    //! @param [in] vector given direction.
    GEOMDLLIMPEXP void AllParallellTangentsXY (bvector<DPoint3d>& points, bvector<double>& fractions, DVec3d vector) const;

    //! Create the B-spline curve from point array and order.
    GEOMDLLIMPEXP MSBsplineStatus CreateFromPointsAndOrder (DPoint3dCP pPoints, int numPoints, int order, bool closed = false);

    //! Populate the B-spline curve with the given parameters.
    //! @DotNetMethodExclude
    GEOMDLLIMPEXP MSBsplineStatus Populate
        (
        bvector<DPoint3d> const &pointVector,
        bvector<double> const *weightVector,
        bvector<double> const *knotVector,
        int order,
        bool closed,
        bool inputPolesAlreadyWeighted
        );

    //! Populate the B-spline curve with the given parameters.
    //! @DotNetMethodParameterIsInputArray{pPoints,numPoints}
    //! @DotNetMethodParameterIsInputArray{pWeights,numPoints}
    //! @DotNetMethodParameterIsInputArray{pKnots,numKnots}
    //! @DotNetMethodExclude
    GEOMDLLIMPEXP MSBsplineStatus Populate
        (
        DPoint3dCP pPoints,
        double const * pWeights,
        int numPoints,
        double const * pKnots,
        int numKnots,
        int order,
        bool closed,
        bool inputPolesAlreadyWeighted
        );

    //! Change the B-spline curve by appending a given curve.
    GEOMDLLIMPEXP MSBsplineStatus AppendCurve (MSBsplineCurveCR inCurve);
    //! Check if the B-spline curves has same parameters with the given curve.
    GEOMDLLIMPEXP bool IsSameGeometry (MSBsplineCurveCR other)const;

    //! Elevate the degree (increases the order) of the B-spline curve.
    GEOMDLLIMPEXP MSBsplineStatus ElevateDegree (int newDegree);
    //! Create a series of Bézier curve for the B-spline curve.
    //! @DotNetMethodExclude
    GEOMDLLIMPEXP void CopyAsBeziers (bvector<MSBsplineCurvePtr>& beziers) const;


    //! Get all C1 fractional Discontinuities.
    //! This inspects xyz and tangent at each knot break.
    //!  (It does not look for intraknot cusps)
    //! @DotNetMethodExclude
    GEOMDLLIMPEXP void GetC1DiscontinuousFractions (bvector<double> &fractions) const;

    //! Compute the length of the control polygon of the B-spline curve.
    GEOMDLLIMPEXP double PolygonLength () const;
    //! Get a (fairly tight) tolerance for the B-spline curve.
    GEOMDLLIMPEXP double Resolution () const;
    //! Get a tolerance of the B-spline curve.
    GEOMDLLIMPEXP double Resolution (double abstol, double reltol) const;


    //! Get the range of the poles of the B-spline curve.
    GEOMDLLIMPEXP void GetPoleRange (DRange3dR range) const;
    //! Get the range of the B-spline curve.
    GEOMDLLIMPEXP DRange3d GetRange () const;

    //! Get the range of parameters of the projection of (a fractional portion of the curve onto a ray
    //! return range whose low and high values are the extreme parameters (in ray fractions) of the projection of the
    //!     curve onto the ray.
    //! @param [in] ray ray to project to
    //! @param [in] fraction0 start of active part of the curve
    //! @param [in] fraction1 end of active part of the curve
    GEOMDLLIMPEXP DRange1d GetRangeOfProjectionOnRay (DRay3dCR ray, double fraction0 = 0.0, double fraction1 = 1.0) const;

    //! Rotate the B-spline curve.
    GEOMDLLIMPEXP MSBsplineStatus RotateCurve (RotMatrixCR rMatrix);
    //! Transform the B-spline curve.
    GEOMDLLIMPEXP MSBsplineStatus TransformCurve (TransformCR transform);
    //! Transform the B-spline curve using a 4d transformation.
    GEOMDLLIMPEXP MSBsplineStatus TransformCurve4d (DMatrix4dCR transform4d);
    
    //! convert to a weighted curve whose normalized points fall on a focal plane
    GEOMDLLIMPEXP void ProjectToZFocalPlane (double focalLength);
    //! Remove all removable knots with the tolerance and end condition constraints.
    GEOMDLLIMPEXP MSBsplineStatus RemoveKnotsBounded (double tol, int startPreservation, int endPreservation);
    //! Compute the bound of remove r-th knot s times
    GEOMDLLIMPEXP static double GetRemovalKnotBound (MSBsplineCurveCP pCurve, int r, int s);


    //! Clean all unnecessary knots.
    GEOMDLLIMPEXP MSBsplineStatus CleanKnots ();
    
/*__PUBLISH_SECTION_END__*/
    //! @DotNetMethodExclude
    static void SetAllocationFunctions (
            int  (*AllocateCurve)(MSBsplineCurve *),
            void (*FreeCurve)(MSBsplineCurve *)
            );
/*__PUBLISH_SECTION_START__*/
    
    //! Create the B-spline curve by appending two input curves with continuity and reparameterization constraints.
    GEOMDLLIMPEXP MSBsplineStatus AppendCurves (MSBsplineCurveCR inCurve1, MSBsplineCurveCR inCurve2, bool forceContinuity, bool reparam);

    //! Return the centroid of the (uniformly distributed) wire mass.
    GEOMDLLIMPEXP void WireCentroid (double &length, DPoint3dR centroid, double fraction0, double fraction1) const;

    //! Create the B-spline curve from the parameters of an elliptic arc.
    GEOMDLLIMPEXP MSBsplineStatus InitEllipticArc (DPoint3d &center, double rX, double rY,
            double startRadians = 0.0,
            double sweepRadians = msGeomConst_2pi,
            RotMatrixP axes = NULL
            );

    //! Initialize the B-spline curve for an ellipse.
    GEOMDLLIMPEXP MSBsplineStatus InitFromDEllipse3d (DEllipse3dCR ellipse);
    //! Initialize the B-spline curve for a conic
    GEOMDLLIMPEXP static MSBsplineCurvePtr CreateFromDConic4d (DConic4dCR conic);

    //! Create the B-spline curve from point array and order.
    GEOMDLLIMPEXP MSBsplineStatus InitFromPoints (DPoint3dCP points, int nPoints);

    //! Create bspline equivalent of Akima curve.
    GEOMDLLIMPEXP MSBsplineStatus InitAkima (DPoint3dCP points, size_t nPoints);
    //! Create bspline equivalent of Akima curve.
    GEOMDLLIMPEXP MSBsplineStatus InitAkima (bvector<DPoint3d> const &points);

    //! Create a B-spline curve from a series of Bézier curve.
    GEOMDLLIMPEXP static MSBsplineCurvePtr CreateFromBeziers (bvector<MSBsplineCurvePtr> const &beziers);
    //! Create a B-spline curve from an array of DPoint4d ponts.
    GEOMDLLIMPEXP MSBsplineStatus InitFromDPoint4dArray (DPoint4dCP pPoleArray, int numPoles, int order);

    //! This routine computes a B-spline curve approximated the old one.
    //! return ERROR if no results.
    //! @param [in]  pIn Input G1 curve.
    //! @param [in]  tolerance Geometric tolerance, this should be in general the chord height tol.
    //! @param [in]  order Desired degree of the pOut, 4 is recommended.
    //! @param [in]  parametrization CHORDLENGTH = 2, CENTRIPETAL = 3.
    //! @param [in]  bMaintainEndTangents true to maintain the end tangents.
    GEOMDLLIMPEXP MSBsplineStatus ApproximateAnyCurve (MSBsplineCurveCP pIn, double tolerance, int order, int parametrization, bool bMaintainEndTangents);

    //! This routine computes a B-spline curve approximated the give points set.
    //! return ERROR if no results.
    //! @param [in]  points data to be approximated.
    //! @param [in]  numPoints Number of points.
    //! @param [in]  endControl true: pass both ends, false: do not require both end points.
    //! @param [in]  sTangent Start tangent (optional).
    //! @param [in]  eTangent End tangent (optional).
    //! @param [in]  keepTanMag Keep end tangent's magnitude
    //! @param [in]  iterDegree Start iteration degree
    //! @param [in]  reqDegree Required degree of output curve
    //! @param [in]  singleKnot Use single interior knots
    //! @param [in]  tolerance Fitting tolerance
    GEOMDLLIMPEXP MSBsplineStatus InitFromLeastSquaresFit (DPoint3dCP points, int numPoints, bool endControl, DVec3dCP sTangent, DVec3dCP eTangent, 
                                        bool keepTanMag, int iterDegree, int reqDegree, bool singleKnot, double tolerance);

    //! This routine computes a B-spline curve with given numPoles and order approximated the give points set.
    //! return ERROR if no results.
    //! @param [out] avgDistance average error, or NULL.
    //! @param [out] maxDistance maximum error, or NULL.
    //! @param [in]  info the params of resulted curve.
    //! @param [in]  knts the knot vector of curve (optional).
    //! @param [in]  pnts data to be approximated.
    //! @param [in]  uValues the parameterization of data (optional).
    //! @param [in]  numPnts Number of points.
    GEOMDLLIMPEXP MSBsplineStatus InitFromGeneralLeastSquares (double *avgDistance, double *maxDistance, BsplineParam info, bvector<double>* knts, DPoint3d *pnts, double *uValues, int numPnts);

    //! This routine computes a B-spline curve interpolated the give points set.
    //! return ERROR if no results.
    //! @param [in]  points data to be Interpolated.
    //! @param [in]  numPoints Number of points.
    //! @param [in]  parametrization CHORDLENGTH = 2, CENTRIPETAL = 3.
    //! @param [in]  endControl true: pass both ends, false: do not require both end points.
    //! @param [in]  sTangent Start tangent (optional).
    //! @param [in]  eTangent End tangent (optional).
    //! @param [in]  keepTanMag Keep end tangent's magnitude
    //! @param [in]  order The order of resulted curve.
    GEOMDLLIMPEXP MSBsplineStatus InitFromInterpolatePoints (DPoint3dCP points, int numPoints, int parametrization,
                                                      bool endControl, DVec3dCP sTangent, DVec3dCP eTangent, bool keepTanMag, int order);

    //! This approximation routine computes a set of sample points of a G1 continuous B-spline curve.
    //! return ERROR if no results.
    //! @param [out] P Points computed on curve.
    //! @param [out] up Parameters of curve corresponding to the points in vector P.
    //! @param [out] uq If par = 2 or par = 3 , then uq is  the array of parameters corresponding to the points in P. Otherwise, uq is not allocated.
    //! @param [in]  pCurve The base curve.
    //! @param [in]  par Flag: 2: chordlength parameterization wanted, 3: centripetal parameterization wanted
    //! @param [in]  Eg Geometric error tolerance. The perpendicular distance from curve to corresponding point isn't greater than Eg.
    //! @param [in]  ptol Point coincidence tolerance.  Two points are considered to be equal if the  distance between them is less than or equal to ptol.  ptol < Eg  should hold
    GEOMDLLIMPEXP static MSBsplineStatus SampleG1CurveByPoints (bvector<DPoint3d>& P, bvector<double>& up, 
                                        bvector<double>& uq, MSBsplineCurveCP pCurve, int par, double Eg, double ptol);

    //! This routine computes a least square B-spline curve to the sample points.
    //! return ERROR if no results.
    //! @param [out] pOut Resulted curve.
    //! @param [in]  Q Data points.
    //! @param [in]  u Parameters corresponding to data points.
    //! @param [in]  knots Knot vector of approximating curve.
    //! @param [in]  numPoles The number of resulted curve.
    //! @param [in]  order The order of resulted curve.
    GEOMDLLIMPEXP static MSBsplineStatus GeneralLeastSquaresApproximation (MSBsplineCurveP pOut, bvector<DPoint3d> const &Q, bvector<double> const &u,
                                        bvector<double> const &knots, int numPoles, int order);

    //! This routine computes a least square B-spline curve to the sample points.
    //! return ERROR if no results.
    //! @param [out] pOut Resulted curve.
    //! @param [in]  Q Data points.
    //! @param [in]  u Parameters corresponding to data points.
    //! @param [in]  endControl true: pass both ends, false: do not require both end points.
    //! @param [in]  sTangent Start tangent (optional).
    //! @param [in]  eTangent End tangent (optional).
    //! @param [in]  numPoles The number of resulted curve.
    //! @param [in]  order The order of resulted curve.

    GEOMDLLIMPEXP static MSBsplineStatus WeightedLeastSquaresFit (MSBsplineCurveP pOut, bvector<DPoint3d> const Q, bvector<double> const u, bool endControl,
                                        DVec3dCP sTangent, DVec3dCP eTangent, int numPoles, int order);
    //! Insert a set of knot to given B-spline curve.
    //! return ERROR if insertion fails.
    //! @param [out] pCurve The curve after knots insertion.
    //! @param [in]  pCurve The curve before knots insertion.
    //! @param [in]  X A vector of knots to be insert into pCurve.

    GEOMDLLIMPEXP static MSBsplineStatus KnotRefinement (bvector<double> const &X, MSBsplineCurveP pCurve);

    //! This routine computes a non-rational B-spline curve approximated the old one.
    //! return ERROR if no results.
    //! @param [out] pOut Non-rational curve created (may be the same as pIn).
    //! @param [in]  pIn Input G1 curve.
    //! @param [in]  degree Desired degree of the pOut, 3 is recommended.
    //! @param [in]  keepTangent true to maintain the end tangents.
    //! @param [in]  parametrization CHORDLENGTH = 2, CENTRIPETAL = 3.
    //! @param [in]  geomTol Geometric tolerance, this should be in general the chord height tol.
    //! @param [in]  paramTol Parametric tolerance, recommand set to 10.0*geomTol.
    //! @param [in]  pointTol Point equal tolerance, recommand set to 0.01*geomTol.
    GEOMDLLIMPEXP static MSBsplineStatus ApproximateG1Curve (MSBsplineCurveP pOut, MSBsplineCurveCP pIn, int degree,bool keepTangent, int parametrization, double geomTol,
                                        double paramTol, double pointTol);

    //! This routine computes a non-rational B-spline curve approximated the old one.
    //! return ERROR if no results.
    //! @param [out] pOut Non-rational curve created (may be the same as pIn).
    //! @param [in]  pIn Input G1 curve.
    //! @param [in]  degree Desired degree of the pOut, 3 is recommended.
    //! @param [in]  keepTangent true to maintain the end tangents.
    //! @param [in]  parametrization CHORDLENGTH = 2, CENTRIPETAL = 3.
    //! @param [in]  tol Geometric tolerance, this should be in general the chord height tol.
    GEOMDLLIMPEXP static MSBsplineStatus ApproximateNurbsCurve (MSBsplineCurveP pOut, MSBsplineCurveCP pIn, int degree, bool keepTangent, int parametrization, double tol);

    //! Compare weights with arbitrary but consistent tolerance.
    GEOMDLLIMPEXP static bool AreSameWeights (double w0, double w1);
    //! Compare knots.   Absolute tolerance 1e-8 for knots in -1..1.
    //! Relative tolerance 1e-8 outside.
    GEOMDLLIMPEXP static bool AreSameKnots (double knot0, double knot1);

    //! Compare curves.
    GEOMDLLIMPEXP bool AlmostEqual (MSBsplineCurveCR other) const;
    //! Compare curves.
    GEOMDLLIMPEXP bool AlmostEqual (MSBsplineCurveCR other, double tolerance) const;


    //! Compute intersections of a ray with a ruled surface between two curves.
    //! @return false if curves are not compatible.
    //! @param [in,out] pickData accumulating intersection data.
    //! @param [in] curveA base curve of ruled surface.
    //! @param [in] curveB top curve of ruled surface.
    //! @param [in] ray ray to intersect
    GEOMDLLIMPEXP static bool AddRuleSurfaceRayIntersections (
            bvector<struct SolidLocationDetail> &pickData, MSBsplineCurveCR curveA, MSBsplineCurveCR curveB, DRay3dCR ray);
    
    //! Compute the closest point on the ruled surface between two curves.
    //! @return false if curves are not compatible.
    //! @param [out] pickData accumulating intersection data.
    //! @param [in] curveA base curve of ruled surface.
    //! @param [in] curveB top curve of ruled surface.
    //! @param [in] spacePoint 
    GEOMDLLIMPEXP static bool RuledSurfaceClosestPoint (
            SolidLocationDetail &pickData,
            MSBsplineCurveCR curveA,
            MSBsplineCurveCR curveB,
            DPoint3dCR spacePoint
            );

    GEOMDLLIMPEXP bool HasValidPoleAllocation () const;
    GEOMDLLIMPEXP bool HasValidWeightAllocation () const;
    GEOMDLLIMPEXP bool HasValidKnotAllocation () const;
    GEOMDLLIMPEXP bool HasValidOrder () const;
    GEOMDLLIMPEXP bool HasValidPoleCounts () const;
    GEOMDLLIMPEXP bool HasValidCountsAndAllocations () const;
}; // MSBsplineCurve

//! MSBsplineCurve with IRefCounted support for smart pointers.
//! Create via MSBsplineCurve::CreatePtr ();
//! @ingroup BentleyGeom_BSplines
struct RefCountedMSBsplineCurve : public MSBsplineCurve, RefCountedBase
    {
/*__PUBLISH_SECTION_END__*/
friend struct MSBsplineCurve;
/*__PUBLISH_SECTION_START__*/ 
    protected:
    RefCountedMSBsplineCurve ();
    ~RefCountedMSBsplineCurve ();
    };

END_BENTLEY_GEOMETRY_NAMESPACE
