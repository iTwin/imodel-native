/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Geom/CurveDetails.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
#ifdef BENTLEY_WIN32
#pragma warning (push)
#pragma warning (default : 4266)	// NEEDS WORK -- warnings about missing virtuals?
#endif
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
// =======================================================================================
//! @addtogroup GeometricQueries
//! Methods used to query elements for common geometric properties.

//! @addtogroup GeometricQueries
//! @beginGroup
// =======================================================================================


//! Detail data for an interval of a parent curve.
struct PartialCurveDetail
{
ICurvePrimitivePtr parentCurve;
double fraction0;
double fraction1;
int64_t userData;

//! construct with all fields.
GEOMDLLIMPEXP PartialCurveDetail (ICurvePrimitiveP _parentCurve, double _fraction0, double _fraction1, int64_t userData);

//! Constructor for null pointer, 01 interval, 0 userdata.
GEOMDLLIMPEXP PartialCurveDetail ();
//! construct with all fields.
GEOMDLLIMPEXP PartialCurveDetail (ICurvePrimitiveP _parentCurve, DSegment1dCR interval, int64_t userData = 0);

//! Construct a subinterval of the input.  f0 and f1 are fractions of the parent's fraction0,fraction1 interval.
GEOMDLLIMPEXP PartialCurveDetail (PartialCurveDetailCR parent, double f0, double f1);

//! (attempt to) map a parent fraction back to the child interval.  This fails if the child is a single point.
GEOMDLLIMPEXP bool ParentFractionToChildFraction (double parentFraction, double &childFraction) const;
//! map a local fraction into the parent fraction.
GEOMDLLIMPEXP double ChildFractionToParentFraction (double f) const;
//! Test if the partial curve fraction range is a single fraction.
GEOMDLLIMPEXP bool IsSingleFraction () const;
//! Return the (directed) interval from fraction0 to fraction1.
GEOMDLLIMPEXP DSegment1d GetInterval () const;

GEOMDLLIMPEXP void UpdateFraction1AndUserData (double f1, int64_t newData)
    {
    fraction1 = f1;
    userData = newData;
    }
};

//!
//! Detail data for a point along a curve, allowing indexing into a subcomponent.
//! 
//!
struct CurveLocationDetail
{
//! Containing curve.
ICurvePrimitiveCP curve;
//! fraction from curve start to end
double          fraction;
//! curve coordinates
DPoint3d        point;
//! index of components within curve
size_t          componentIndex;
//! number of indexed components.
size_t          numComponent;
//! fraction within the indexed part.
double          componentFraction;
//! Context specific
double          a;

//! Construct as null state.
GEOMDLLIMPEXP CurveLocationDetail ();

//! Construct curve data but no coordinate data.
GEOMDLLIMPEXP CurveLocationDetail (ICurvePrimitiveCP _curve, size_t _numComponent = 1);

//! Construct from simple fraction in single component curve.
GEOMDLLIMPEXP CurveLocationDetail (ICurvePrimitiveCP _curve, double _fraction, DPoint3dCR _point);

//! Construct with full indexing
GEOMDLLIMPEXP CurveLocationDetail (ICurvePrimitiveCP _curve, double _fraction, DPoint3dCR _point, size_t _componentIndex, size_t _numComponent, double _componentFraction);

//! Construct with full indexing
GEOMDLLIMPEXP CurveLocationDetail (ICurvePrimitiveCP _curve, double _fraction, DPoint3dCR _point, size_t _componentIndex, size_t _numComponent, double _componentFraction, double _a)
    : curve(_curve),
      fraction(_fraction),
      componentIndex(_componentIndex),
      numComponent(_numComponent),
      componentFraction(_componentFraction),
      point(_point),
      a(_a)
    {
    }

//! conditional replace candidate, with minDistance update if needed.
GEOMDLLIMPEXP bool UpdateIfCloser (CurveLocationDetailCR otherDetail);

//! set xyz distance from point.
GEOMDLLIMPEXP double SetDistanceFrom (DPoint3dCR refPoint);

//! set distance to very large value.
GEOMDLLIMPEXP void SetMaxDistance ();

//! set distance field ({a})
GEOMDLLIMPEXP void SetDistance (double value);

//! copy fraction to componentFraction and set component index and count for single component.
GEOMDLLIMPEXP void SetSingleComponentData ();

//! Set fraction and a fields from parameters. Make the component fraction match.  Set componentIndex and numComponentIndex to 0 and 1.
GEOMDLLIMPEXP void SetSingleComponentFractionAndA (double fraction, double a);

//! Return distance between point fields of this and other.
GEOMDLLIMPEXP double Distance (CurveLocationDetailCR other) const;

//! Use the componentIndex and componentFraction to interpolate doubles.
GEOMDLLIMPEXP bool Interpolate (bvector<double> const &param, double &result) const;

//! Try to evalute the point and unit tangent at the detail's fraction
GEOMDLLIMPEXP ValidatedDRay3d PointAndUnitTangent () const;

//! Try to evaluate the referenced curve at the detail's fraction.
GEOMDLLIMPEXP bool TryFractionToPoint (DPoint3dR xyz, DVec3dR dXdf) const;

//! Try to evaluate the referenced curve at the detail's component fraction.
GEOMDLLIMPEXP bool TryComponentFractionToPoint (DPoint3dR xyz, DVec3dR dXdf) const;


//! test if point coordinates are AlmostEqual
GEOMDLLIMPEXP bool AlmostEqualPoint (CurveLocationDetailCR other) const;

//! construct an interpolated detail.
//! the return has (1) the curve pointer from the first, (2) simple interpolation of fraction, point, and "a" value,
//! (3) component data based on the interpolated fraction and the componentCount from the first.
//! The result is marked valid only if the inputs have matching curve pointers and component counts.
//! This operation makes clear sense if both curved details are from the same curve and (with allowance for start-end matchup)
//!  within the same component (linesegment ?) of that parent.   It may or may not make sense if it spans interiors of
//! two components of the same curve.   It almost certainly does not make sense between curves.
GEOMDLLIMPEXP ValidatedCurveLocationDetail Interpolate (double fraction, CurveLocationDetailCR dataB) const;

//! Set the componentIndex, numComponent, componentFraction, and fraction.   (Global fraction is computed as (componentIndex + componentFraction)/ numComponent.
//! All other data (point, curve pointer, a) is left unchanged.
GEOMDLLIMPEXP void SetFractionFromComponentFraction (double componentFraction, size_t componentIndex, size_t numComponent);

//! Set the componentIndex, numComponent, componentFraction, and fraction.   (Component fraction and index are computed from global fraction and indices)
//! All other data (point, curve pointer, a) is left unchanged.
GEOMDLLIMPEXP void SetComponentFractionFromFraction (double globalFraction, size_t numComponent);

//! Sort to gather CurveLocationDetail's with same curve, and then by fraction within those curves.
GEOMDLLIMPEXP static void SortByCurveAndFraction (bvector<CurveLocationDetail> &detail);
};

//! Pair of CurveLocationDetail structs, as produced by curve-curve intersection code.
struct CurveLocationDetailPair
{
public:
    CurveLocationDetail detailA;
    CurveLocationDetail detailB;
//! Null constructor
GEOMDLLIMPEXP CurveLocationDetailPair ();
//! Constructor for two given details.
GEOMDLLIMPEXP CurveLocationDetailPair (CurveLocationDetailCR _detailA, CurveLocationDetailCR _detailB);
//! Constructor with single detail copied to both member details.
GEOMDLLIMPEXP CurveLocationDetailPair (CurveLocationDetailCR _detailA);
//! Constructor with single curve, fraction, and point for both details
GEOMDLLIMPEXP CurveLocationDetailPair (ICurvePrimitiveCP curve, double fraction0, DPoint3dCR point0);
//! Constructor with single curve, fraction, and point for both details
GEOMDLLIMPEXP CurveLocationDetailPair (ICurvePrimitiveCP curve, double fraction0, DPoint3dCR point0,
                size_t componentIndex, size_t numComponent, double componentFraction);
//! Constructor for two distinct curve and "a" tags.
GEOMDLLIMPEXP CurveLocationDetailPair
    (
    ICurvePrimitiveCP curve0, double a0, 
    ICurvePrimitiveCP curve1, double a1
    );

//! Constructor for two fraction/point pairs on the same curve.
CurveLocationDetailPair (ICurvePrimitiveCP curve, double fraction0, DPoint3dCR point0, double fraction1, DPoint3dCR point1);

//! Return true if the two details are for (bitwise) identical curve and fraction.
GEOMDLLIMPEXP bool SameCurveAndFraction ();

//! update fractions, points, and a
GEOMDLLIMPEXP void Set (double fraction0, DPoint3dCR point0, double a0, double fraction1, DPoint3dCR point1, double a1);
//! update curve pointers
GEOMDLLIMPEXP void Set (ICurvePrimitiveP curveA, ICurvePrimitiveP curveB);

//! assemble points from detailA, detailB into a DSegment3d.
GEOMDLLIMPEXP DSegment3d GetDSegment3d () const;

//! return squared distance between points.
GEOMDLLIMPEXP double DistanceSquared () const;

//! return signed z change between points.
GEOMDLLIMPEXP double DeltaZ () const;


//! return distance between points.
GEOMDLLIMPEXP double Distance () const;

//! return the index (or SIZE_MAX) of the pair with min and max deltaZ
static GEOMDLLIMPEXP void DeltaZExtremes (bvector<CurveLocationDetailPair> const &pairs, size_t &iMin, size_t &iMax);

//! distribute all pairs with deltaZ less than or equal to split distance in pairA, others in pairB.
static GEOMDLLIMPEXP void SplitByDeltaZ (bvector<CurveLocationDetailPair> const &pairs, double splitDistance,
    bvector<CurveLocationDetailPair> *pairA, bvector<CurveLocationDetailPair> *pairB);

//!! Lexical compare:  curve first, then fration from detailA, then fraction from detailB.
static bool cb_compareCurveFraction (CurveLocationDetailPairCR dataA, CurveLocationDetailPairCR dataB);
};

#ifndef SmallGeomLib


//! Detail data for pick on solid primitive.
//! Data maintained is:
//! <ul>
//! <li>parameter on pick ray.
//! <li>space point at hit
//! <li>two integer ids to distinguish among cap faces and grid of side faces.
//! </ul>
struct SolidLocationDetail
{
friend struct ISolidPrimitive;
public:
//! FaceIndices is a triple of integers identifying a face within a solid primitive.
//! <ul>
//! <li> Index0 is "along" the sweep direction, i.e. one of (a) a special index for each cap or (b) an index of step along the sweep direction.
//! <li> Index1 is a curve primitive index along the curve vector around the end cap.
//! <li> Index2 is an index within that curve primitive, i.e. an edge selector in a linestring.
//! </ul>
struct FaceIndices
    {
    private:
    friend struct SolidLocationDetail;
    ptrdiff_t m_index0;
    ptrdiff_t m_index1;
    ptrdiff_t m_index2;
    public:
    //! constructor with index pair.
    GEOMDLLIMPEXP FaceIndices (ptrdiff_t index0, ptrdiff_t index1, ptrdiff_t index2);
    //! Default constructor (all indices zero)
    GEOMDLLIMPEXP FaceIndices ();
    //! Return the special (constant) face indices for the start cap of swept primitives.
    GEOMDLLIMPEXP static FaceIndices Cap0 ();
    //! Return the special (constant) face indices for the end cap of swept primitives.
    GEOMDLLIMPEXP static FaceIndices Cap1 ();
    //! Set both indices
    GEOMDLLIMPEXP void Set (ptrdiff_t index0, ptrdiff_t index1, ptrdiff_t index2);
    //! Set both indices as the special start cap indices.
    GEOMDLLIMPEXP void SetCap0 ();
    //! set both indices as the special end cap indices
    GEOMDLLIMPEXP void SetCap1 ();

    //! Query the first index.
    GEOMDLLIMPEXP ptrdiff_t Index0 () const;
    //! Query the second index.
    GEOMDLLIMPEXP ptrdiff_t Index1 () const;
    //! Query the third index.
    GEOMDLLIMPEXP ptrdiff_t Index2 () const;

    //! Set the first index.
    GEOMDLLIMPEXP void SetIndex0 (ptrdiff_t value);
    //! Set the second index.
    GEOMDLLIMPEXP void SetIndex1 (ptrdiff_t value);
    //! Set the third index.
    GEOMDLLIMPEXP void SetIndex2 (ptrdiff_t value);


    //! Ask if this is the special index pair for start cap.
    GEOMDLLIMPEXP bool IsCap0 () const;
    //! Ask if this is the special index pair for end cap.
    GEOMDLLIMPEXP bool IsCap1 () const;
    //! Ask if this is either of the start or end caps.
    GEOMDLLIMPEXP bool IsCap () const;
    //! Ask if the indices match a particular triple;
    GEOMDLLIMPEXP bool Is (ptrdiff_t index0, ptrdiff_t index1, ptrdiff_t index2) const ;
    //! Ask if the indices match a particular pair (index3 ignored)
    GEOMDLLIMPEXP bool Is (ptrdiff_t index0, ptrdiff_t index1) const ;
    };


private:
double m_uParameter;
double m_vParameter;
double m_parameter;
DPoint3d m_xyz;
DVec3d   m_uDirection;
DVec3d   m_vDirection;
FaceIndices m_faceIndices;

int m_parentId;

double m_a;

public:

//! Default construtor.
GEOMDLLIMPEXP SolidLocationDetail ();
//! Construct with parameter, no point.
GEOMDLLIMPEXP SolidLocationDetail (int parentId, double s);

//! Constructor with parameter and point.
GEOMDLLIMPEXP SolidLocationDetail (int parentId, double s, DPoint3dCR xyz);

//! Constructor with parameter, point, uv parameters, and uv vectors.
GEOMDLLIMPEXP SolidLocationDetail (int parentId, double s, DPoint3dCR xyz, double u, double v, DVec3dCR uVector, DVec3dCR vVector);

//! Initialize to zeroed state.
GEOMDLLIMPEXP void Init ();

//! Set all face selectors
GEOMDLLIMPEXP void SetFaceIndices (ptrdiff_t _id0, ptrdiff_t _id1, ptrdiff_t _id2);

//! Set all face selectors
GEOMDLLIMPEXP void SetFaceIndices01 (ptrdiff_t _id0, ptrdiff_t _id1);

//! Set face selectors 0 and 1, leave selector 2 unchanged.
GEOMDLLIMPEXP void SetFaceIndices (SolidLocationDetail::FaceIndices const &indices);


//! Special value for primary id of cap faces.
GEOMDLLIMPEXP static const int PrimaryIdCap = -1;

//! Set selectors for cap id (typically 0 or 1)
GEOMDLLIMPEXP void SetCapSelector (int id);

//! Return the parent id.
GEOMDLLIMPEXP int GetParentId () const;
//! Set the parent id.
GEOMDLLIMPEXP void SetParentId (int id);

//! Return the primary selector.
GEOMDLLIMPEXP int GetPrimarySelector () const;
//! Return the secondary selector
GEOMDLLIMPEXP int GetSecondarySelector () const;

//! return a structure with all selector indices.
GEOMDLLIMPEXP SolidLocationDetail::FaceIndices GetFaceIndices () const;
//! Return the parameter along the pick ray.
GEOMDLLIMPEXP double GetPickParameter () const;

//! Return the pick coordinates
GEOMDLLIMPEXP DPoint3d GetXYZ () const;

//! Return u,v parameter information
GEOMDLLIMPEXP DPoint2d GetUV () const;
//! query the u parameter
GEOMDLLIMPEXP double GetU () const;
//! query the v parameter
GEOMDLLIMPEXP double GetV () const;
//! query the a parameter
GEOMDLLIMPEXP double GetA () const;
//! Return u direction vector.
GEOMDLLIMPEXP DVec3d GetUDirection () const;

//! Return v direction vector.
GEOMDLLIMPEXP DVec3d GetVDirection () const;

//! Set the u,v, and direction vectors.
GEOMDLLIMPEXP void SetUV (double u, double v, DVec3dCR uDirection, DVec3dCR vDirection);
//! Set the u,v coordinates
GEOMDLLIMPEXP void SetUV (DPoint2dCR xyz);
//! Set u
GEOMDLLIMPEXP void SetU (double u);
//! Set v.
GEOMDLLIMPEXP void SetV (double v);
//! Set a.
GEOMDLLIMPEXP void SetA (double a);


//! Set the point coordinates.
GEOMDLLIMPEXP void SetXYZ (DPoint3dCR xyz);
//! Set the u direction vector
GEOMDLLIMPEXP void SetUDirection (DVec3d dXdu);
//! Set the v direction vector
GEOMDLLIMPEXP void SetVDirection (DVec3d dXdv);
//! set the pick fraction.
GEOMDLLIMPEXP void SetPickParameter (double f);

//! Ask if the selectors identify a cap
GEOMDLLIMPEXP bool IsCapSelect (int &capId) const;

//! std::sort comparator for sort by parameter ...
GEOMDLLIMPEXP static bool cb_compareLT_parameter
(
SolidLocationDetail const &dataA,
SolidLocationDetail const &dataB
);

typedef struct SolidLocationDetail::FaceIndices const & FaceIndicesCR;
typedef struct SolidLocationDetail::FaceIndices  const * FaceIndicesCP;
typedef struct SolidLocationDetail::FaceIndices  &FaceIndicesR;
typedef struct SolidLocationDetail::FaceIndices  *FaceIndicesP;


//! Ask if a selector pair is the start cap.
GEOMDLLIMPEXP static bool IsCap0 (int selector0, int selector1);
//! Ask if a selector pair is the end cap.
GEOMDLLIMPEXP static bool IsCap1 (int selector0, int selector1);

//! TransformInPlace points and vectors by the transform.
GEOMDLLIMPEXP void TransformInPlace (TransformCR transform);

//! Treat current pick parameter as fractions in new interval.
//! return false (and leave the pick parameter unchanged) if the range is undefined.
GEOMDLLIMPEXP bool MapPickParameterFractionToRange (DRange1dCR range);

GEOMDLLIMPEXP bool UpdateIfSmallerA (SolidLocationDetailCR source);
};


//__PUBLISH_SECTION_END__


// Routing interface for type-specific processors.
// Virtual _Process methods receive an optional restricted interval.
// This becomes non-trivial (other than 01) only due to recursion through a partial curve.
// (Passthrough to ChildVector has no clear meaning)
struct ICurvePrimitiveProcessor
{
private:
bool m_aborted;
public:

ICurvePrimitiveProcessor () : m_aborted (false) {}

//! @remark a true return will cancel recursive searches.
bool GetAbortFlag ();
//! Set the abort flag
void SetAbortFlag (bool value);

// Called by default implementations --- useful place for debugger breaks during class development
GEOMDLLIMPEXP GEOMAPI_VIRTUAL void _ProcessDefault (ICurvePrimitiveCR curve, DSegment1dCP interval);

// Visit all contained curves (subject to abort flag)
GEOMDLLIMPEXP GEOMAPI_VIRTUAL void _ProcessCurveVector (CurveVector const &curveVector, DSegment1dCP interval);

// @remark Default action: noop.
GEOMDLLIMPEXP GEOMAPI_VIRTUAL void _ProcessLine (ICurvePrimitiveCR curve, DSegment3dCR segment, DSegment1dCP interval);
// @remark Default action: noop.
GEOMDLLIMPEXP GEOMAPI_VIRTUAL void _ProcessArc (ICurvePrimitiveCR curve, DEllipse3dCR arc, DSegment1dCP interval);
// @remark Default action: noop.
GEOMDLLIMPEXP GEOMAPI_VIRTUAL void _ProcessLineString (ICurvePrimitiveCR curve, bvector<DPoint3d> const &points, DSegment1dCP interval);
// @remark Default action: noop.
GEOMDLLIMPEXP GEOMAPI_VIRTUAL void _ProcessBsplineCurve (ICurvePrimitiveCR curve, MSBsplineCurveCR bcurve, DSegment1dCP interval);
// Default action: noop
GEOMDLLIMPEXP GEOMAPI_VIRTUAL void _ProcessProxyBsplineCurve (ICurvePrimitiveCR curve, MSBsplineCurveCR bcurve, DSegment1dCP interval);
// Default action: noop
GEOMDLLIMPEXP GEOMAPI_VIRTUAL void _ProcessPartialCurve (ICurvePrimitiveCR curve, PartialCurveDetailCR detail, DSegment1dCP interval);
// Default action: invoke _ProcessProxyBsplineCurve
GEOMDLLIMPEXP GEOMAPI_VIRTUAL void _ProcessInterpolationCurve (ICurvePrimitiveCR curve, MSInterpolationCurveCR icurve, DSegment1dCP interval);
// Default action: invoke _ProcessProxyBsplineCurve
GEOMDLLIMPEXP GEOMAPI_VIRTUAL void _ProcessSpiral (ICurvePrimitiveCR curve, DSpiral2dPlacementCR spiral, DSegment1dCP interval);
// Default action: recurse
GEOMDLLIMPEXP GEOMAPI_VIRTUAL void _ProcessChildCurveVector (ICurvePrimitiveCR curve, CurveVectorCR child, DSegment1dCP interval);
// Default action: noop
GEOMDLLIMPEXP GEOMAPI_VIRTUAL void _ProcessPointString (ICurvePrimitiveCR curve, bvector<DPoint3d>const &points, DSegment1dCP interval);
// Default action: invoke _ProcessProxyBsplineCurve
GEOMDLLIMPEXP GEOMAPI_VIRTUAL void _ProcessAkimaCurve (ICurvePrimitiveCR curve, bvector<DPoint3d>const &points, DSegment1dCP interval);

};

//__PUBLISH_SECTION_START__

/*=================================================================================**//**
* Interface implemented to hold additional information about curve primitive.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ICurvePrimitiveInfo : public IRefCounted {};
typedef RefCountedPtr<ICurvePrimitiveInfo> ICurvePrimitiveInfoPtr;

//__PUBLISH_SECTION_END__

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  04/13
+===============+===============+===============+===============+===============+======*/
struct CurveNodeInfo : RefCounted <ICurvePrimitiveInfo>
    {
    bvector<int> m_nodeIds;

    CurveNodeInfo (int nodeId) {m_nodeIds.push_back (nodeId);}

    static ICurvePrimitiveInfoPtr Create (int nodeId) {return new CurveNodeInfo (nodeId);}

    }; // CurveNodeInfo

//__PUBLISH_SECTION_START__
/*=================================================================================**//**
*  readIndex and fraction for a position along an edge of a facet in a mesh.
* @bsiclass                                                     Earlin.Lutz  09/12
+===============+===============+===============+===============+===============+======*/
struct FacetEdgeLocationDetail
    {
    size_t m_readIndex;
    double m_fraction;
    GEOMDLLIMPEXP FacetEdgeLocationDetail (size_t readIndex, double fraction);
    GEOMDLLIMPEXP FacetEdgeLocationDetail ();
    };
//__PUBLISH_SECTION_END__

struct FacetEdgeLocationDetailVector;
typedef RefCountedPtr<FacetEdgeLocationDetailVector> FacetEdgeLocationDetailVectorPtr;
//__PUBLISH_SECTION_START__

/*=================================================================================**//**
*  options to control joints among offset curves
* @bsiclass                                                     Earlin.Lutz  12/12
+===============+===============+===============+===============+===============+======*/
struct CurveOffsetOptions
{
private:
double m_tolerance;
double m_arcAngle;  // (if this is positive) turns larger than this become arcs.
double m_chamferAngle; // (if this is positive) "outer chamfers" are created with this max angle.
double m_offsetDistance;
bool   m_forceClosure;
bool   m_unusedBool[8];
double m_unusedDouble[8];
int    m_bCurvePointsPerKnot;   // When offsetting bspline, number of points requested per knot interval.
int    m_bCurveMethod;      // 0=default=MX spline fit.  1=greville absisae with current knots.
int    m_unusedInt[6];
public:
GEOMDLLIMPEXP CurveOffsetOptions (double offsetDistance);  // construct with default options.

GEOMDLLIMPEXP void SetTolerance (double tol);
GEOMDLLIMPEXP void SetOffsetDistance (double distance);
GEOMDLLIMPEXP void SetArcAngle (double valueRadians);
GEOMDLLIMPEXP void SetChamferAngle (double value);
GEOMDLLIMPEXP void SetForceClosure (bool value);
GEOMDLLIMPEXP void SetBCurvePointsPerKnot (int n);
//! Select the method to be used to offset bspline curves in MSBsplineCurve::CreateCopyOffsetXY and ICurvePrimitive::CloneAsSingleOffsetPrimitiveXY
//!<ul>
//!<li>1 -- offset with fit points at offsets of the Greville knots.  (Moving averages of (order-1) knots)  This is appropriate (and strongly preferred) when the input curve is smooth.
//!<li>0 -- (and other) offset to multiple fit points within each bezier span.   Use SetBCurfvePointsPerKnot () to control fit point counts.  This method significantly increases number of knots.  This
//!         may be more accurate for a single offset, but leads to overly dense curves if offset is repeated.
//!</ul>
GEOMDLLIMPEXP void SetBCurveMethod (int n);
GEOMDLLIMPEXP double GetTolerance () const;
GEOMDLLIMPEXP double GetOffsetDistance () const;
GEOMDLLIMPEXP double GetArcAngle () const;
GEOMDLLIMPEXP double GetChamferAngle () const;
GEOMDLLIMPEXP bool  GetForceClosure () const;
GEOMDLLIMPEXP int GetBCurvePointsPerKnot () const;
GEOMDLLIMPEXP int GetBCurveMethod () const;
};


/*=================================================================================**//**
*  options to gap closure
* @bsiclass                                                     Earlin.Lutz  12/12
+===============+===============+===============+===============+===============+======*/
struct CurveGapOptions
{
private:
double m_equalPointTolerance;
double m_maxDirectAdjust;
double m_maxAdjustAlongCurve;
bool   m_removePriorGapPrimitives;
double m_unusedDouble[10];
int    m_unusedInt[10];
bool   m_unusedBool[10];
public:
GEOMDLLIMPEXP CurveGapOptions ();   // default options (1e-7, 1e-4,1e-3)
GEOMDLLIMPEXP CurveGapOptions (double m_equalPointTolerance, double maxDirectAdjust, double maxAdjustAlongCurve); 

//! Set gap size that does not need to be corrected.
GEOMDLLIMPEXP void SetEqualPointTolerance (double value); 
//! Set max allowable motion of line and linestring endpoints.
GEOMDLLIMPEXP void SetMaxDirectAdjustTolerance (double value);
//! Set max motion along a curve.
GEOMDLLIMPEXP void SetMaxAdjustAlongCurve (double value);
//! Set max motion along a curve.
GEOMDLLIMPEXP void SetRemovePriorGapPrimitives (bool value);

//! @return tolerance for gaps that do not have to be closed at all.
GEOMDLLIMPEXP double GetEqualPointTolerance () const;    
//! @return max allowable motion of line and linestring endpoints.
GEOMDLLIMPEXP double GetMaxDirectAdjustTolerance () const; 
//! @return max allowable motion along a curve.
GEOMDLLIMPEXP double GetMaxAdjustAlongCurve () const;
//! @return flag to remove prior gap primitives
GEOMDLLIMPEXP bool GetRemovePriorGapPrimitives () const;


};



/*=================================================================================**//**
* DSegment3d with additional tagging:
* <pre>
* <ul>
* <li> a facet read index (applicable to the whole segment)
* <li> the double (fractional) coordinate where the drape point was taken from the drape segment.
* <li> the distance swept from the drape to the hit. (WARNING: not supported by some constructions)
* <li> An edge location detail where the (possibly extended) drape line crossed a facet edge.
* </ul>
* </pre>
* Note that if the drape segment endpoint falls within the facet:
* <pre>
* <ul>
*   <li> the segment coordinates are where the segment endpoint falls in the interior of the facet.
*   <li> the corresponding facet edge location is a facet edge point where the extended line left the facet.
* </ul>
* </pre>
* @bsiclass                                                     Earlin.Lutz  10/15
+===============+===============+===============+===============+===============+======*/
struct DrapeSegment
{
DSegment3d m_segment;
double m_drapeEdgeFraction[2];
double m_drapeDistance[2];
FacetEdgeLocationDetail m_edgeDetail[2];
size_t m_facetReadIndex;
size_t m_drapeEdgeIndex;

/// Defalt ctor...
GEOMDLLIMPEXP DrapeSegment ();

/// Construct with a facetReadIndex, setting drape edge fractions to reversed infinite (null interval) values.
GEOMDLLIMPEXP DrapeSegment (size_t facetReadIndex, size_t drapeEdgeIndex = 0);
/// std::sort comparison based on drapeEdgeIndex and then drape edge fraction
GEOMDLLIMPEXP bool operator < (DrapeSegment const &other) const;

/// Test if the interval is empty.
GEOMDLLIMPEXP bool IsEmptyInterval () const;
/// Install data for the start (index 0) point.
GEOMDLLIMPEXP void SetPoint0 (double edgeFractionA, DPoint3dCR xyzA, FacetEdgeLocationDetail edgeDetail);

/// Install data for the end(index 1) point.
GEOMDLLIMPEXP void SetPoint1 (double edgeFractionA, DPoint3dCR xyzA, FacetEdgeLocationDetail edgeDetail);

/// Save a point as start and/or end point according to its drape fraction
GEOMDLLIMPEXP void ExpandInterval (double segmentFraction, DPoint3dCR xyzA, FacetEdgeLocationDetail edgeDetail);

/// restrict the drape segment to fractions 0..1 on the drape edge.  This may adjust the xyz coordinates of one or both endpoints
/// (but leave the facet edge locations unchanged.)
GEOMDLLIMPEXP bool RestrictTo01 ();

/// Set m_drapeDistance by multiplying transform "y" row times segment points
GEOMDLLIMPEXP void SetDrapeDistancesFromLocalY (TransformCR worldToLocal);

/// Return (if possible) the part of this drape segment that overlaps edgeFraction0..edgeFraction1.
/// In invalid returns, the DrapeSegment part matches the input.
GEOMDLLIMPEXP ValidatedDrapeSegment Clip (double edgeFraction0, double edgeFraction1) const;
/// return interpolated distance at specified fraction (start is 0, end is 1).
/// No test for empty interval.
GEOMDLLIMPEXP double FractionToDistance (double fraction) const;

// If the instance and candidate have matching indices and the candidate fractions extend those of the instance,
// extend the instance fractions and coordinates and return true.  Otherwise return false.
GEOMDLLIMPEXP bool Extend (DrapeSegment const &candidate);

// test if the instance endpoint and candidate startpoint have almost equal coordinates. (No comparison of other annotation)
GEOMDLLIMPEXP bool AlmostEqualXYZEndToStart (DrapeSegment const &candidate) const;


// If compress is requested (true) and the candidate is an extension of back (), extend back ().
// otherwise push_back (segment).
GEOMDLLIMPEXP static void ExtendOrAppend (bvector<DrapeSegment> &data, DrapeSegment const &segment, bool compress = true);

// SORT (in place) the allDrape array (by segment index and fraction)
// Within each segment, compare drape segments (considering all splits due to jump from one facet to another)
// Extract the min and max depth segments.
// If compress is false, the minDrape and maxDrape vectors will have directly corresponding entries.
// if compress is true, their counts will differ due to different consolidation of drape segments on the min and max depth paths.
GEOMDLLIMPEXP static void SelectMinMaxVisibleDrape (bvector<DrapeSegment> &allDrape, bvector<DrapeSegment> &minDrape, bvector<DrapeSegment> &maxDrape, bool compress);

/// compute the area of the quadrilateral produced by sweeping the line segment to a plane.
///  The plane is assumed to have a unit normal.
GEOMDLLIMPEXP double SignedSweptAreaSegmentToPlaneWithUnitNormal (DPlane3dCR plane) const;

/// sum the areas sweeping segments to a plane.
///  The plane is assumed to have a unit normal.
GEOMDLLIMPEXP static double SumSignedSweptAreaToPlaneWithUnitNormal (bvector<DrapeSegment> const &data, DPlane3dCR plane);

/// Copy segment data into linestrings.
GEOMDLLIMPEXP static void ExtractLinestrings (bvector<DrapeSegment> const &data, bvector<bvector<DPoint3d>> &linestrings);


};



//__PUBLISH_SECTION_END__


/*=================================================================================**//**
*  bvector of FacetEdgeLocationDetail, with RefCount for use as ICurvePrimitiveInfo ...
* public interface allows only add, size(), and TryGetAt with distinct args.
* @bsiclass                                                     Earlin.Lutz  09/12
+===============+===============+===============+===============+===============+======*/
struct FacetEdgeLocationDetailVector : RefCounted <ICurvePrimitiveInfo>
    {
    private:
        FacetEdgeLocationDetailVector ();
    bvector<FacetEdgeLocationDetail> m_data;
    public:
    GEOMDLLIMPEXP static FacetEdgeLocationDetailVectorPtr Create ();

    GEOMDLLIMPEXP size_t size () const;

    GEOMDLLIMPEXP bool TryGet (size_t i, size_t &readIndex, double &fraction) const;
    GEOMDLLIMPEXP bool TryGet (size_t i, FacetEdgeLocationDetailR data) const;

    GEOMDLLIMPEXP void Add(FacetEdgeLocationDetailCR data);
    GEOMDLLIMPEXP void Add(size_t readIndex, double fraction);
    };

//__PUBLISH_SECTION_START__


//! Enumeration of variants of blend calculations
enum BlendType
    {
    CURVE_CURVE_BLEND_BisectorParabola,        // for parabola with axis that bisects the angle between the defining lines
    CURVE_CURVE_BLEND_VerticalAxisParabola,    // parabola with vertical axis
    };


enum AreaSelect
    {
    AreaSelect_Parity                      = 0,
    AreaSelect_CCWPositiveWindingNumber    = 1,
    AreaSelect_CCWNonzeroWindingNumber     = 2,
    AreaSelect_CCWNegativeWindingNumber    = 3,
    };

// Area analysis generates (a) an integer property at each leaf level face, (b) a true/false from that int.
// At higher level, BoolSelect gives choices of interpretation of the collection.
enum BoolSelect
    {
    BoolSelect_Parity = 0,    // XOR of leaf-level bools
    BoolSelect_Union  = 2,    // UNION of leaf level bools
    BoolSelect_Summed_Parity   = 3,   // XOR of leaf level integers
    BoolSelect_Summed_Positive = 4,   // Positive sum of leaf level integers 
    BoolSelect_Summed_NonZero  = 5,   // Nonzero sum of leaf level integers
    BoolSelect_Summed_Negative = 6,    // Negative sum of leaf level integers.
    BoolSelect_FromStructure = 1000   // Dictated by structure of supplied data.
    };

//! Detail about a computed blend.
//! detailA, detailB refer to the curves the fillet joins.
struct BlendDetail
{
CurveLocationDetail detailA, detailB;
ICurvePrimitivePtr geometry;
};

// Pair of curve primitives (e.g. for use in bvector<CurvePrimitivePtrPair>)
struct CurvePrimitivePtrPair
    {
    ICurvePrimitivePtr curveA;
    ICurvePrimitivePtr curveB;
    CurvePrimitivePtrPair (ICurvePrimitivePtr curveA, ICurvePrimitivePtr curveB);
    };
typedef bvector<CurvePrimitivePtrPair> CurvePrimitivePtrPairVector;

/** @endGroup */

//! A range with transforms between local and world coordinates.
//!<ul>
//!<li>The localToWorld and worldToLocalTransforms are inverses of each other.
//!<li>The localRange is "primary".
//!<li>WorldRange is the world space range of the 8 localRange corners transformed to world.
//!<li>since the transforms may have rotations, the world range is generally "larger".
//!</ul>
struct LocalRange
{
Transform m_localToWorld;
Transform m_worldToLocal;
DRange3d  m_localRange;
DRange3d  m_worldRange;
//! Return the local coordinates of a point given by fractional position in the range.
GEOMDLLIMPEXP DPoint3d RangeFractionToLocal (double x, double y, double z) const;
//! Return the world coordinates of a point given by fractional position in the range.
GEOMDLLIMPEXP DPoint3d RangeFractionToWorld (double x, double y, double z) const;//! Initialize with identity transforms and an empty range.
GEOMDLLIMPEXP void InitNullRange ();
//! Constructor with transforms and range.
GEOMDLLIMPEXP LocalRange (TransformCR localToWorld, TransformCR worldToLocal, DRange3dCR localRange);
//! Constructor with transforms and range.
GEOMDLLIMPEXP LocalRange (TransformCR localToWorld, TransformCR worldToLocal, DRange3dCR localRange, DRange3dCR worldRange);
// Initialize with identity transforms and empty range
GEOMDLLIMPEXP LocalRange ();
GEOMDLLIMPEXP bool InitFromPrincipalAxesOfPoints (bvector<DPoint3d> const &xyz);
GEOMDLLIMPEXP bool InitFromPrincipalAxesOfPoints (bvector<DPoint4d> const &xyzw);
// Return 0 if space point is in or on the range.
// Otherwise return shortest distanced to any point of the range.
GEOMDLLIMPEXP double DistanceOutside (DPoint3dCR spacePoint) const;
};

//! LocalRange with size_t and double tags (e.g. for bsurface patch)
//! The constructors do NOT attempt to pass args to the LocalRange base class -- use the base class InitXXXX methods for that.
struct TaggedLocalRange : public LocalRange
{
size_t m_indexA, m_indexB;
double m_a;
GEOMDLLIMPEXP TaggedLocalRange (size_t indexA, size_t indexB, double a = 0.0);
// sort an array of TaggedLocalRanges by increasing "a" value.    
static GEOMDLLIMPEXP void SortByA (bvector<TaggedLocalRange> &data);
// Compute and save distance from the range to space point.
void GEOMDLLIMPEXP SetDistanceOutside (DPoint3dCR spacePoint);
};

//! Detail structure for combined data on a curve and a surface or solid.
struct CurveAndSolidLocationDetail
{
CurveLocationDetail m_curveDetail;
SolidLocationDetail m_solidDetail;
CurveAndSolidLocationDetail
    (
    CurveLocationDetailCR curveDetail,
    SolidLocationDetailCR solidDetail
    )
    : m_curveDetail (curveDetail), m_solidDetail(solidDetail)
    {}
};



//! Collector object for various searches.
struct CurveKeyPointCollector
{
enum KeyPointType
{
EndPoint = 0,
Perpendicular = 1,
Tangency = 2,
BreakPoint = 3,
NumTypes = 4
};
protected:
bool m_needKeyPointType [10];
bool m_xyOnly;
DMatrix4d m_worldToLocal;

public:
//! Initialize with EndPoint, Perpendicular, and BreakPoint. KeyPoint types and no projection.
GEOMDLLIMPEXP CurveKeyPointCollector ();
//! Indicate keypoint preference
GEOMDLLIMPEXP void EnableKeyPointType (KeyPointType selector, bool value);

//! Indicate keypoint preference, and suppress all others.
GEOMDLLIMPEXP void EnableSingleKeyPointType (KeyPointType selector);

//! Request projection to XY plane.
GEOMDLLIMPEXP void SetXYOnly (DMatrix4dCR worldToLocal);
//! Ask if a particular KeyPointType is requested.
GEOMDLLIMPEXP bool NeedKeyPointType (KeyPointType selector) const;
//! Query the worldTolocal matrix.
GEOMDLLIMPEXP bool GetWorldToLocal (DMatrix4dR worldToLocal) const;

//! Announce a keypoint in world form.  Derived class override captures as needed.
GEOMDLLIMPEXP GEOMAPI_VIRTUAL void AnnouncePoint (CurveLocationDetailCR worldDetail, KeyPointType selector);

};

//! Special case of CurveKeyPointCollector -- save only the closest point to the bias point.
//! 
struct CurveKeyPoint_ClosestPointCollector : CurveKeyPointCollector
{
private:
CurveLocationDetail m_closestPoint; //! closest point so far.  "a" field is used for distance
DPoint3d m_worldBiasPoint;
CurveKeyPointCollector::KeyPointType m_keyPointType;
public:
//! Constructor.   Initialize as invalid closest point.
GEOMDLLIMPEXP CurveKeyPoint_ClosestPointCollector
(
DPoint3dCR biasPoint //!< [in] world point for distance calculations.
);
//! Announce a keypoint in world form.  Derived class override captures as needed.
GEOMDLLIMPEXP void AnnouncePoint (CurveLocationDetailCR worldDetail, KeyPointType selector) override;
//! Access the saved detail
GEOMDLLIMPEXP bool GetResult (CurveLocationDetailR detail, KeyPointType &selector) const;
};

//! structure to carry adjacency information extracted from graph structures
//! Use of fields is documented in methods that return NeighborIndices.
struct NeighborIndices
{
   struct NeighborEntry
    {
    size_t siteIndex;
    size_t neighborIndex;
    NeighborEntry (size_t site, size_t neighbor) : siteIndex (site), neighborIndex(neighbor) {}
    };
private:
size_t m_siteIndex;
size_t m_auxIndex;
bvector<NeighborEntry> m_neighbor;
public:
NeighborIndices (size_t siteIndex) : m_siteIndex (siteIndex){}
void AddNeighbor (size_t siteIndex, size_t neighborIndex) {m_neighbor.push_back (NeighborEntry (siteIndex, neighborIndex));}
void SetAuxIndex (size_t index) {m_auxIndex = index;}
size_t GetSiteIndex (){return m_siteIndex;}
size_t GetAuxIndex (){return m_auxIndex;}
bvector<NeighborEntry> &Neighbors (){return m_neighbor;}
};
#endif

//! Description of where a hatch line is along its containing infinite line (with known origin)
struct HatchSegmentPosition
{
//! Distance to segment start
double startDistance;
//! Distance to segment end
double endDistance;
//! line identifier. (origin is 0, first hatch line is 1, etc)
double hatchLine;       
};
#define DEFAULT_MAX_DASH 100000
//! Array of (signed) dash lengths, with computed total of all lengths.
//! Negative lengths are gaps.
struct DashData
{
// PRIMARY
private: bvector<double> m_dashLengths;
private: size_t m_maxDash;
// DERIVED
private: double m_period;

public: GEOMDLLIMPEXP DashData (size_t maxDash = DEFAULT_MAX_DASH);
//! Clear the length array.   Replace by new data and period.
public: GEOMDLLIMPEXP void SetDashLengths (double const *lengths, uint32_t count);
//! Clear the length array.   Replace by new data and period.
public: GEOMDLLIMPEXP void SetDashLengths (bvector<double> const &lengths);
//! Add one dash to the array.  Update the summed period.
public: GEOMDLLIMPEXP void AddDash (double length);

//! Compute dashes for the segment positioned within the cyclic dash pattern.
public: GEOMDLLIMPEXP void AppendDashes
(
DSegment3dCR segment,               //!< [in] segment to be expanded
double startDistance,               //!< [in] distance (along containing line, scaled for dash lengths) to start of this segment.
double endDistance,                 //!< [in] distance (along containing line, scaled for dash lengths) to end of this segment.
bvector<DSegment3d> &segments       //!< [out] segments
);
//! For each segmentA, create dashes under control of dashLengths its position data
GEOMDLLIMPEXP void AppendDashes
(
bvector<DSegment3d> const &segmentA,               //!< [in] segments to be expanded
bvector<HatchSegmentPosition> const &positionA,     //!< [in] dash position data.  Assumed compatible with segmentA
bvector<DSegment3d> &segmentB       //!< [out] segments
);

};

//! Coordinate and location data for a facet edge, as produced by CollectTopologicalBoundaries
struct FacetEdgeDetail
{
DSegment3d segment;     // simple coordinates of the endpoints
size_t readIndex;       // the facet read index.
uint32_t clusterIndex;      // an index shared by all edges within a cluster
uint32_t numInCluster;  // number of edges in the cluster.
GEOMDLLIMPEXP FacetEdgeDetail (
    DSegment3dCR _segment,
    size_t readIndex = SIZE_MAX,
    uint32_t _clusterIndex = UINT_MAX,
    uint32_t _numInCluster = 0
    );
//! Apply transform to the segment coordintes
void TransformInPlace (TransformCR transform);
//! Apply transtform to all segment coordinates.
static void TransformInPlace (bvector<FacetEdgeDetail> &edges, TransformCR transform);
//! Append (push_back) all from source to dest.
static void Append (bvector<FacetEdgeDetail> &dest, bvector<FacetEdgeDetail> const &source);
//! Return any point in the array.  (000 if empty array)
static DPoint3d GetAnyPoint (bvector<FacetEdgeDetail> const &source)
    {
    return source.size () > 0
        ? source.front ().segment.point[0]
        : DPoint3d::From (0,0,0);
    }
};
END_BENTLEY_GEOMETRY_NAMESPACE

#ifdef BENTLEY_WIN32
#pragma warning (pop)
#endif