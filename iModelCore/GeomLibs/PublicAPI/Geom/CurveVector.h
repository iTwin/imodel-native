/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once


/*__PUBLISH_SECTION_START__*/
//! @file CurveVector.h A collection of curve primitives: CurveVector, CurveCurve, PathLocationDetail, CurveVectorWithDistanceIndex

// @addtogroup BentleyGeom_PolymorphicCurves ICurvePrimitive
    

/**
@page BentleyGeom_Summary_CurveVector CurveVector

A CurveVector is a collection of curve primitives.  

This can represent paths, loops, multiloop parity regions, and multi-region collections.

A CurveVector is a refcounted structure.  At point of creation, a curve vector is addressed via a CurveVectorPtr.   Inputs to methods can be passed as simple references and pointers.

<h3>Example tree structures for CurveVector</h3>
<ul>
<li> open chain:
<pre>
    (CurveVector::BOUNDARY_TYPE_OPEN prim1 prim2 ... primN)
</pre>

<li> single loop:
<pre>
   (CurveVector::BOUNDARY_TYPE_OUTER prim1 prim2 ... primN)
</pre>

<li> multiloop region:
<pre>
 (CurveVector::BOUNDARY_TYPE_UNION_REGION
    (CurveVector::BOUNDARY_TYPE_OUTER prim1 ... primN)
    (CurveVector::BOUNDARY_TYPE_OUTER prim1 ... primN)
    (CurveVector::BOUNDARY_TYPE_INNER prim1 ... primN)
    )
</pre>    

<li> union of regions:
<pre>
    (CurveVector::BOUNDARY_TYPE_UNION_REGION   // the direct children of this CurveVector may be any set of Outer and ParityReigion
        (CurveVector:ParityRegion
            (CurveVector::BOUNDARY_TYPE_OUTER prim1 ... primN)
            (CurveVector::BOUNDARY_TYPE_OUTER prim1 ... primN)
            (CurveVector::BOUNDARY_TYPE_INNER prim1 ... primN)
            )
        (CurveVector::BOUNDARY_TYPE_OUTER prim1 prim2 ... primN)
        )
</pre>    
</ul>

<h3>Parity Regions</h3>
A parity region is a collection of unordered loops.   A point moving in the plane transitions from "in" to "out" (or vice versa) any time it crosses a curve.

This definition of inside and outside is well defined even if ( a) the loops cross each other, ( b) self-intersection within a loop, ( c) any order of presentation of the loops.

In the usual case wheret here is a single outer loop and one or more hole loops, it is customary to store the outer loop first followed by holes, with the outer loop counterclockwise and inner loops clockwise.

Warning: Computational code that requires signed loops (e.g. area calculations) assumes that the largest loop is outer and others are inner.

*/



#ifdef BENTLEY_WIN32
#pragma warning (push)
#pragma warning (default : 4266)	// NEEDS WORK -- warnings about missing virtuals?
#endif
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


//! Vector of ICurvePrimitivePtr, used for collecting curves into paths, loops with area, multiloop areas with parity rules, and unions of areas.
//! See the CurveVector::BoundaryType enumeration for expected deep structure in a CurveVector.
//! @ingroup BentleyGeom_PolymorphicCurves
//! @ingroup GROUP_Geometry
struct CurveVector : RefCountedBase, bvector<ICurvePrimitivePtr>
{
public:
//__PUBLISH_SECTION_END__
//! Select integration rule for areas between bspline and strokes.
//! @param [in] quadratureType (0==>gauss) (1==>uniform)
//! @param [in] numQuadraturePoints number of quadrature points.
//! @param [in] angleTolerance angle tolerance for stroking.
GEOMDLLIMPEXP static void SetMomentIntegrationParameters (int quadratureType, int numQuadraturePoints, double angleTol);
//__PUBLISH_SECTION_START__


//flex !! Boundary type and tree structure
//flex 
//flex Each curve vector is marked with an enumerated value indicating how its contents are to be interpretted.  The enumerated type can be accessed via cv.GetBoundaryType ().
//flex 
//flex || enum name    || represents   || expected children || IsOpenPath ()  || IsClosedPath () || IsAnyRegionType () ||
//flex || BOUNDARY_TYPE_None   || Unstructured collection || any || false || false || false ||
//flex || BOUNDARY_TYPE_Open   || Open chain of curves ||  true curve primitives || true || false || false ||
//flex || BOUNDARY_TYPE_Outer   || Outer loop || true curve primitives || false || true || true ||
//flex || BOUNDARY_TYPE_Inner  || Open chain of curves || true curve primitives|| false || true || true ||
//flex || BOUNDARY_TYPE_ParityRegion   || region boundaries for "exclusive or" among loops || CurveVectors of type BOUNDARY_TYPE_Outer and BOUNDARY_TYPE_Inner || false || false || true ||
//flex || BOUNDARY_TYPE_UnionRegion || region boundaries for "union" of children || CurveVectors of type BOUNDARY_TYPE_Outer, BOUNDARY_TYPE_ParityRegion, BOUNDARY_TYPE_UnionRegion || false || false || false ||
//flex 
//flex 
//flex Mamy queries are simpler using these checks:
//flex 
//flex || Query the boundary type  || boundaryType = cv.GetBoundaryType () ||
//flex || Test for open path || bool cv.IsOpenPath ( ))    .... ||
//flex || Test for closed path || bool cv.IsClosedPath ( ))    .... ||
//flex || Test for parity region || bool cv.IsParityRegion ( ))    .... ||
//flex || Test for union region || bool cv.IsUnionRegion ( ))    .... ||
//flex || Test for parity, union, outer, or inner || bool cv.IsAnyRegionType ( ))    .... ||
//flex 
//flex These calls are used during construction:
//flex 
//flex || cv.SetBoundaryType (boundaryType) || set the boundary type.  Caller is responsible for correctness ||
//flex || cv.SetChildBoundaryType (index, bounaryType) || Confirm that child at [index] is a CurveVector and set the child's boundary type ||
//flex || bool cv.GetChildBoundaryType (index, boundaryType)) || query the boundary type of child at [index].  Returns false if the child is not a cv. ||
//flex || outCurvePrim = cv.FindIndexedLeaf (index) || Deep search to derefence to an indexed leaf, with the index counting through leaves in all subtrees. ||
//flex || bool cv.LeafToIndex (curve, index) || Deep search to find a leaf with same pointer as curve.  Return index if found. ||
//flex || n = cv.CountPrimitivesBelow () || Deep search to count leaf primitives. ||
//flex 


//! Classification of expected structure within a CurveVector.
enum BoundaryType
    {
    //! no specific properties expected for contained curves or points.
    //! @remarks Use of BOUNDARY_TYPE_None is discouraged for representing anything except point strings and RSC font glyphs.
    //!          Do not create a CurveVector of BOUNDARY_TYPE_None and expect it to be treated as anything more than a collection of disjoint ICurvePrimitives.
    //!          A collection of un-related curves are better represented as a bvector of CurveVectorPtr and not as a single CurveVector of BOUNDARY_TYPE_None.
    BOUNDARY_TYPE_None          = 0,
    //! Curves should join head to tail in a single path. The path is not expected to be closed. 
    BOUNDARY_TYPE_Open          = 1, 
    //! Curves should join head to tail in a single closed path; this area expected to be an outer (or only) loop.
    BOUNDARY_TYPE_Outer         = 2, 
    //! Curves should join head to tail in a single closed path; this area is expected to be an inner loop.
    BOUNDARY_TYPE_Inner         = 3, 
    //! Expected to contain (only) multiple CurveVectors, all of which are either BOUNDARY_TYPE_Outer or BOUNDARY_TYPE_inner. (No individual curves or open paths).
    //! These are to be analyzed by partity rules.
    BOUNDARY_TYPE_ParityRegion  = 4, 
    //! Expected to contain (only) multiple CurveVectors, all of which have area. (No individual curves or open paths).
    //! These are to be analyzed by union rules.
    BOUNDARY_TYPE_UnionRegion   = 5, 
    };

//! Classification of a point wrt a closed shape.
enum InOutClassification
    {
    INOUT_Unknown = 0,
    INOUT_In      = 1,
    INOUT_Out     = 2,
    INOUT_On      = 3
    };

friend struct CurvePrimitiveChildCurveVector;

protected:
BoundaryType    m_boundaryType;
friend class RefCountedPtr<ICurvePrimitive>;
public:
GEOMDLLIMPEXP void SetBoundaryType (BoundaryType BoundaryType);
//! Confirm that specified child is a CurveVector and set its boundary type.
GEOMDLLIMPEXP bool SetChildBoundaryType (size_t index, BoundaryType boundaryType);
//! Confirm that specified child is a CurveVector and get its boundary type.
GEOMDLLIMPEXP bool GetChildBoundaryType (size_t index, BoundaryType &boundaryType) const;

//! Deep search for leaf identified by index that counts only at leaf level.
//! @param [in] index leaf index.
//! @return pointer to leaf.
GEOMDLLIMPEXP ICurvePrimitivePtr FindIndexedLeaf (size_t index) const;

//! Deep search for index of specific leaf given by pointer.
//! @param [out] index leaf index. (Number of primitives preceding it in depth first order)
//! @param [in] primitive pointer to leaf.
GEOMDLLIMPEXP bool LeafToIndex (ICurvePrimitiveCP primitive, size_t &index) const;

//! Deep search to count true leaf primitives.
//! @return number of primitive curves accessible by FindIndexeLeaf.
GEOMDLLIMPEXP size_t CountPrimitivesBelow () const;

//! Find Primitive by ID
GEOMDLLIMPEXP ICurvePrimitivePtr FindPrimitiveById (CurvePrimitiveIdCR id) const;

explicit CurveVector (BoundaryType boundaryType) {m_boundaryType = boundaryType;}

public:

//! Return true if the curve vector has a single element and that element is a primitive.
GEOMDLLIMPEXP ICurvePrimitive::CurvePrimitiveType HasSingleCurvePrimitive () const;

//! Return the type code indicating whether the vector is a path, outer boundary, inner boundary, or higher level grouping.
GEOMDLLIMPEXP BoundaryType GetBoundaryType () const;
//! Query: Is this an open path?
GEOMDLLIMPEXP bool IsOpenPath () const;
//! Query: Is this (single) closed path?
GEOMDLLIMPEXP bool IsClosedPath () const;
//! Query: Is this a collection of loops with parity rules?
GEOMDLLIMPEXP bool IsParityRegion () const;
//! Query: is this a collection of areas with union rules?
GEOMDLLIMPEXP bool IsUnionRegion () const;
//! Query: is this any closed area type (single, parity, union)
GEOMDLLIMPEXP bool IsAnyRegionType () const;

//! Query: Does this have start and end points and are the points almost equal?
GEOMDLLIMPEXP bool AreStartEndAlmostEqual () const;

//! Query: Is this an outer loop with an ellipse as its only curve?
GEOMDLLIMPEXP bool IsEllipticDisk (DEllipse3dR ellipse) const;

//! Query: Is this a nominally open path but with matching start and end?
GEOMDLLIMPEXP bool IsPhysicallyClosedPath () const;

//! Query: Is this a rectangle?
//! @param [out] localToWorld transform with origin at start, x and y vectors to adjacent points, z vector unit normal.
//!   (i.e. the x and y vector lengths are the side lengths)
//! @param [out] worldToLocal transorm to map rectangle to 0..1 in each direction.
GEOMDLLIMPEXP bool IsRectangle (TransformR localToWorld, TransformR worldToLocal) const;


//! Swap the entries at given indices.   Return false if either index is out of bounds.
GEOMDLLIMPEXP bool SwapAt (size_t index0, size_t index1);

//! Swap bvectors and type.
GEOMDLLIMPEXP void SwapContents (CurveVectorR other);

//flex !! Construction and Cloning
//flex 
//flex || Create with specified type, no curves     || outCv = CurveVector::Create (boundaryType)  ||
//flex || Create with specified type, single child    || outCv = CurveVector::Create (boundaryType, child)  ||
//flex || Deep copy    || outCv = source.Clone () ||
//flex || Deep copy, expand partial curve references to standalone curves || outCv = source.CloneDereferenced (bool allowExtrapolation, bool recursiveDerference) ||
//flex || Deep search in source for primitives. Add all primitives directly to dest || n = dest.AddPrimitives (source) ||
//flex 
//flex 

//! Create a curve vector with given boundary type and no members.
GEOMDLLIMPEXP static CurveVectorPtr Create (BoundaryType boundaryType);

//! Create a curve vector with a single primitive and given boundary type.
GEOMDLLIMPEXP static CurveVectorPtr Create (BoundaryType boundaryType, ICurvePrimitivePtr primitive);

//! Create a curve vector with multiple initial primitives.
GEOMDLLIMPEXP static CurveVectorPtr Create (BoundaryType boundaryType, bvector<ICurvePrimitivePtr> primitives);


//! Return a "deep copy"
GEOMDLLIMPEXP CurveVectorPtr Clone () const;

//! Return a "deep copy" with transform applied
GEOMDLLIMPEXP CurveVectorPtr Clone (TransformCR transform) const;

//! Return a "deep copy" with transform applied
GEOMDLLIMPEXP CurveVectorPtr Clone (DMatrix4dCR transform) const;

//! Return a "deep copy" with PartialCurve primitives replaced by full curves.
//! @param [in] maximumDeref true to recurse through all steps of PartialCurve chains
//! @param [in] allowExtrapolation true to allow extension before/after endpoints.
GEOMDLLIMPEXP CurveVectorPtr CloneDereferenced (bool allowExtrapolation = false, bool maximumDeref = true) const;

//! return a (deep) clone with fillets inserted between successive curves.
GEOMDLLIMPEXP CurveVectorPtr CloneWithFillets (double radius) const;

//! return a (deep) clone with endpoint gaps closed.
//! When gaps are found larger than gapTolerance, line segments are added.
//! The options that will be used are:
//!  1) options.SetEqualPointTolerance(value):  Gaps smaller than this are acceptable.  This
//!       Suggested value:  around 1e-7 in master units.
//!  2) options.SetMaxDirectAdjustTolerance: gaps this small may be closed by directly moving endopints of lines or linestrings.
//!        SuggestedValue: 10 to 1000 times the equal point tolerance
//!  3) options.SetRemovePriorGapPrimitives(true): primitives marked as gaps are purged. (And the gaps are re-closed)
//!         Suggested value:  true.   (default is true)
//!  4) options.SetMaxAdjustAlongPrimitive: points may move this far if the final point is on the extended element.
//!  
GEOMDLLIMPEXP CurveVectorPtr CloneWithGapsClosed (CurveGapOptionsCR options) const;

//! recurse through source.   append all leaf primitives to this.  Return number added.
GEOMDLLIMPEXP size_t AddPrimitives (CurveVectorCR source);

//! add single curve primitive
GEOMDLLIMPEXP void Add (ICurvePrimitivePtr child);
//! wrap as child curve primitive and add to this vector
GEOMDLLIMPEXP void Add (CurveVectorPtr child);


//! return a (deep) clone with fillets inserted between successive curves.
GEOMDLLIMPEXP CurveVectorPtr CloneWithBlends (BlendType, double radiusA, double radiusB) const;
//! return a (deep) clone with all curves offset by signed distance.
//!  This is a curve operation, and may result in self-intersecting offset curves.
//! @remark A positive offset is to the right of the curve (i.e. CCW outer loop offsets to a larger area)
GEOMDLLIMPEXP CurveVectorPtr CloneOffsetCurvesXY (CurveOffsetOptionsCR options) const;

//! return a (deep) clone with all areas increased or decreased according to the offset distance.
//! @remark A positive offset is to the right of the curve (i.e. CCW outer loop offsets to a larger area)
GEOMDLLIMPEXP CurveVectorPtr AreaOffset (CurveOffsetOptionsCR options) const;

//! generate offset areas to right and left of each curve primitive.
//! @remark Positive offsets are to the named left and right directions.  Negative offsets are possible to create a strip entirely to one side.
GEOMDLLIMPEXP CurveVectorPtr AreaOffsetFromPath(CurveOffsetOptionsCR options, double leftOffset, double rightOffset) const;

//! Count primitives of specified type.
//! @param [in] targetType primitive type to count.
GEOMDLLIMPEXP size_t CountPrimitivesOfType (ICurvePrimitive::CurvePrimitiveType targetType) const;


//! Return a curve vector that is a clone, but with all primitives split at intersections with any splitter curve.
//! Optionally omit tree structure and only copy primitives.
GEOMDLLIMPEXP CurveVectorPtr CloneWithSplits (CurveVectorCR splitterCurves, bool primitivesOnly = false);

//! Return curves (not regions) that are inside, outside, or on a region.
GEOMDLLIMPEXP void AppendSplitCurvesByRegion (CurveVectorCR region, CurveVectorP insideCollector, CurveVectorP outsideCollector, CurveVectorP onCollector);
//! Return curves (not regions) that are below, above, and on a plane
GEOMDLLIMPEXP void AppendSplitCurvesByPlane (DPlane3dCR plane, CurveVectorP belowCollector, CurveVectorP aboveCollector, CurveVectorP onCollector);


//! Return a curve vector that is a clone, but with all polylines split into individual line segments.
GEOMDLLIMPEXP CurveVectorPtr CloneWithExplodedLinestrings () const;


//flex !! Size and orientation
//flex 
//flex || Compute one of several types of local coordinate systems.  Return a deep copy transformed to the system. || outCv = cv.CloneInLocalCoordinates (in frameType, out localToWorld, out worldToLocal, out localRange) ||
//flex || Map a local coordinate (in a 01 coordinate system in the plane of the curves) to global coordinates. || bool cv.TryUVFractionToXYZ (in u, in v, out xyz, out dXdu, out dXdv) ||
//flex || Compute a coordinate frame anywhere within the structure.  || bool cv.GetAnyFrenetFrame (localToWorld)) ....||
//flex || Sum lengths of all primitives || a = cv.Length () || a = cv.FastLength () ||
//flex || Compute range cube   || bool GetRange (out range)) ... ||
//flex || Compute range along a ray || outRange1d = cv.ProjectedParameterRange (ray, fraction0, fraction1)) ||
//flex || Compute range cube after transform || bool GetRange (range, transform) ||
//flex || Test if planar.  If so return transforms and local range || bool cv.IsPlanar (out localToWorld, out worldToLocal, out localRange) ||
//flex || Quick estimate of largest coordinate present || a = cv.FastMaxAbs () || 
//flex || Expand a tolerance to respect the range of coordinates in the curve vector || a = cv.ResolveTolerance () ||
//flex || Compute centroid of the curve as collection of wires || bool cv. WireCentroid (outLength, outPoint)) ... ||
//flex || Compute centroid, normal, and area of the bounded region || bool cv.CentroidNormalArea (outCentroid, outNormal, outArea)) ...||
//flex || Compute products of inertia of bounded region || bool cv.ComputeSecondMomentAreaProducts (outProducts)) ... ||
//flex || Compute products of inertia for a thin wedge of the region being rotated around an axis || bool cv.ComputeSecondMomentDifferentialAreaRotationProducts (axis, outLocalToWorld, outProducts)) ...||
//flex || Compute products of inertia for the curve wire || bool cv.ComputeSecondMomentWireProducts (outProducts) ||
//flex || Compute centroid and area viewing the xy plane || bool cv.CentroidAreaXY (outCentroid, outArea) ||
//flex 


//! Return the centroid of the contained curves, considered as wires.   (Isolated points are not considered.)
//!  (Bounded area centroids are not computed.  The boundary curves are used as wires.)
//! return false if no curves are found.
//! @param [out] length curve length
//! @param [out] centroid curve centroid
GEOMDLLIMPEXP bool  WireCentroid (double &length, DPoint3dR centroid) const;

//! Return the centroid, normal and area of the curve vector.
//! return false if the CurveVector is not one of the area types (union region, parity region, or closed loop)
//! @remark Union region moments are the simple sum of constituents (i.e overlap is not determined)
//! @remark Parity region moments are signed sum per area, assuming largest is outer and all others are inner (subtractive)
//! @remark If curves are non-planar, the centroid and normal are approximations with no particular guarantees.
//! @param [out] centroid curve centroid
//! @param [out] normal   curve normal
//! @param [out] area     area of region.
GEOMDLLIMPEXP bool CentroidNormalArea (DPoint3dR centroid, DVec3dR normal, double &area) const;

//! Return the area, centroid, orientation, and principal moments, treating this as a thin planar sheet.
//! @param [out] products integrated [xx xy xz x; xy yy yz y; xz yz zz z; x y z 1] dA
GEOMDLLIMPEXP bool ComputeSecondMomentAreaProducts (DMatrix4dR products) const;

//! Return the moment products [xx,xy,xz,xw; etc] of the area as a differential rotational slice.
//! @param [in] rotationAxis the origin and z axis of the rotation.
//! @param [out] rotationToWorld transformation from rotation system (origin on rotation axis)
//!     to world.  The products are base don local coordinates in the system.
//! @param [out] products products in the rotation system.
//! @return false if invalid area for rotational sweep.
GEOMDLLIMPEXP bool ComputeSecondMomentDifferentialAreaRotationProducts
                (
                DRay3dCR rotationAxis,
                TransformR rotationToWorld,
                DMatrix4dR products
                ) const;                               

//! Return the moment products [xx,xy,xz,xw; etc] of the wire as a differential rotational contribution
//! @param [in] rotationAxis the origin and z axis of the rotation.
//! @param [out] rotationToWorld transformation from rotation system (origin on rotation axis)
//!     to world.  The products are base don local coordinates in the system.
//! @param [out] products products in the rotation system.
//! @return false if invalid area for rotational sweep.
GEOMDLLIMPEXP bool ComputeSecondMomentDifferentialWireRotationProducts
                (
                DRay3dCR rotationAxis,
                TransformR rotationToWorld,
                DMatrix4dR products
                ) const;                               

//! Return the area, centroid, orientation, and principal moments, treating this as a wire
//! @param [out] products integrated [xx xy xz x; xy yy yz y; xz yz zz z; x y z 1] dA
GEOMDLLIMPEXP bool ComputeSecondMomentWireProducts (DMatrix4dR products) const;

//! Return the centroid and area of the curve vector as viewed in the xy plane.
//! return false if the CurveVector is not one of the area types (union region, parity region, or closed loop)
//! @remark Union region moments are the simple sum of constituents (i.e overlap is not determined)
//! @remark Parity region moments are signed sum per area, assuming largest is outer and all others are inner (subtractive)
//! @param [out] centroid curve centroid
//! @param [out] area     area of region.
GEOMDLLIMPEXP bool  CentroidAreaXY (DPoint3dR centroid, double &area) const;

//! Return the range of ray parameters when contents of the CurveVector are projected to a ray.
//! return DRange1d with range data.
//! @param [in] ray test ray.
//! @remark If the ray's direction vector is a unit vector, the projected parameters are physical distances.
//! @remark If the ray's direction vector is NOT a unit vector, the projected parameters are fractions of the ray's direction vector.
//! @remark If the CurveVector has no curves, the returned range returns true on the DRange1d::IsNull() predicate.
GEOMDLLIMPEXP DRange1d ProjectedParameterRange (DRay3dCR ray) const;




//! Return curve copy and transforms for a local coordinate system related to the curves.
//! The local x and y axes are parallel to the x and y axes of the frenet frame at the curve start.
//! @return curves transformed to local system.
//! @param [out] localToWorld local to world transform
//! @param [out] worldToLocal worldTolocal transform
//! @param [out] localRange curve vector range in the local frame.
//! @param [in] frameType Selects how the geometry size is represented in the scale.
//!<ul>
//!<li>LOCAL_COORDINATE_SCALE_UnitAxesAtStart -- x,y,z columns in the transform are unit vectors.
//!        localRange values are true geometry sizes.
//!         origin is at start point of first primitive.
//!<li>LOCAL_COORDINATE_SCALE_UnitAxesAtLowerLeft -- x,y,z columns in the transform are unit vectors.
//!        localRange values are true geometry sizes.
//!         origin is at lower left of range.
//!<li>LOCAL_COORDINATE_SCALE_01RangeBothAxes -- x column is a vector spanning the x range from min to max.
//!         y column is a vector spanning the y range from min to max.
//!         localRange values are 0..1 (inclusive) in both directions.
//!         origin is at lower left of range.
//!<li>LOCAL_COORDINATE_SCALE_01RangeLargerAxis -- x and y columns have the same length, large enough to 
//!            span the larger direction.
//!         localRange values are 0..1 in the larger direction, 0..f in the smaller direction, where f is that direction's 
//!         size as a fraction of the larger direction.
//!         origin is at lower left of range.
//!</ul>
GEOMDLLIMPEXP CurveVectorPtr CloneInLocalCoordinates
(
LocalCoordinateSelect frameType,
TransformR localToWorld,
TransformR worldToLocal,
DRange3dR localRange
) const;


//! Deep search for any curve primitive that has a well defined coordinate frame.
//! @param [out] frame coordinate frame with origin on a primitive.
//! @return true if a primitive was found.
GEOMDLLIMPEXP bool GetAnyFrenetFrame (TransformR frame) const;

//! Deep search for any curve primitive that has a well defined coordinate frame.
//! @param [out] frame coordinate frame with origin on a primitive.
//! @param [in] searchPreference
//!<pre>
//!<ul>
//!<li>0 to favor use of directions as soon as possible from the begninning.
//!<li>1 to use start point, start tangent, and end tangent and accept that "no matter what" (i.e. accept default fixups if those are parallel)
//!<li>2 to use start point, start tangent, and end tangent but default to searchPreference=0 if they are parallel.
//!</ul>
//!</pre>
//! @return true if a primitive was found.
GEOMDLLIMPEXP bool GetAnyFrenetFrame (TransformR frame, int searchPreference) const;



//! convert u,v fraction to xyz and derivatives.
//! This is an expensive operation.  It has to call CloneInLocalCoordinates before it can multiply uv by the localToWorld transformation.
//! To do this efficiently many times, call CloneInLocalCoordinates once and reuse the localToWorld transform.
//! @param [in] uFraction fractional coordinate in u direction.
//! @param [in] vFraction fractional coordinate in v direction.
//! @param [out] xyz point on face.
//! @param [out] dXdu derivative of face point wrt uFraction.
//! @param [out] dXdv derivative of face point wrt vFraction.
GEOMDLLIMPEXP bool TryUVFractionToXYZ
    (
    double uFraction,
    double vFraction,
    DPoint3dR xyz,
    DVec3dR dXdu,
    DVec3dR dXdv
    ) const;


//flex !! Geometric Construction
//flex 
//flex || Promote a point array to a cv.  If creating loop, optionally force CCW order || outCv = CurveVector::CreateLinear (bvector<DPoint3d> &points, boundaryType, forceXYOrientation)||
//flex || (same) || outCv = CurveVector::CreateLinear (DPoint3d *, pointCount, boundaryType, forceXYOrientation)||
//flex || Create a rectangle from xy corners. (default BOUNDARY_TYPE_Outer) ||  outCv = CurveVector::CreateRectangle (x0, y0, x1, y1, z, boundaryType ) ||
//flex || Create a complete elliptic disk  (default BOUNDARY_TYPE_Outer) || outCv = CurveVector::CreateDisk (DEllipse3d, boundaryType) ||
//flex || Deep copy with fillets inserted where possible || outCv = source.CloneWithFillets (radius) ||
//flex || Deep copy with blends inserted where possible || outCv = source.CloneWithBlends (blendType, distanceA, distanceB) ||
//flex || Deep copy with primitives replaced by strokes. || outCv = source.Stroke (facetOptions) ||
//flex || Deep copy with linestrings exploded to segments || outCv = source.CloneWithExplodedLinestrings () ||
//flex || Deep copy with each primitives replaced by an individual bspline || outCv = source.CloneAsBsplines () ||
//flex || Stroke all primitives.  Append to a single bvector<DPoint3d>, with DISCONNECT seprators || cv.AddStrokePoints (inout points, facetOptions) ||


//! Create a linestring or polygon from xyz data.
//! @param [in] points vertex coordinates points.
//! @param [in] boundaryType is one of
//! <ul>
//! <li> BOUNDARY_TYPE_Outer, or BOUNDARY_TYPE_INNER: Duplication forced on first/last point.  Orientation optionally enforced.
//! <li> BOUNDARY_TYPE_Open, BOUNDARY_TYPE_None: points copied unchanged.
//! <li> BOUNDARY_TYPE_UnionRegion, BOUNDARY_TYPE_ParityRegion create two-level structure with polygonat second level with BOUNDARY_TYPE_Outer.
//! </ul>
//! @param [in] forceXYOrientation true to force outer and inner loops to have correct (CCW/CW) order.
GEOMDLLIMPEXP static CurveVectorPtr CreateLinear (bvector<DPoint3d> const&points,
                    BoundaryType boundaryType = CurveVector::BOUNDARY_TYPE_Open,
                    bool forceXYOrientation = false);

//! Create a linestring or polygon from xyz data.
//! @param [in] points vertex coordinates points.
//! @param [in] numPoints number of coordinates.
//! @param [in] boundaryType is one of
//! <ul>
//! <li> BOUNDARY_TYPE_Outer, or BOUNDARY_TYPE_INNER: Duplication forced on first/last point.  Orientation optionally enforced.
//! <li> BOUNDARY_TYPE_Open, BOUNDARY_TYPE_None: points copied unchanged.
//! <li> BOUNDARY_TYPE_UnionRegion, BOUNDARY_TYPE_ParityRegion create two-level structure with polygonat second level with BOUNDARY_TYPE_Outer.
//! </ul>
//! @param [in] forceXYOrientation true to force outer and inner loops to have correct (CCW/CW) order.
GEOMDLLIMPEXP static CurveVectorPtr CreateLinear (
                    DPoint3dCP points,
                    size_t numPoints,
                    BoundaryType boundaryType = CurveVector::BOUNDARY_TYPE_Open,
                    bool forceXYOrientation = false);

//! Create a linestring or polygon from xyz data.
//! @param [in] points vertex coordinates points.
//! @param [in] numPoints number of coordinates.
//! @param [in] boundaryType is one of
//! <ul>
//! <li> BOUNDARY_TYPE_Outer, or BOUNDARY_TYPE_INNER: Duplication forced on first/last point.  Orientation optionally enforced.
//! <li> BOUNDARY_TYPE_Open, BOUNDARY_TYPE_None: points copied unchanged.
//! <li> BOUNDARY_TYPE_UnionRegion, BOUNDARY_TYPE_ParityRegion create two-level structure with polygonat second level with BOUNDARY_TYPE_Outer.
//! </ul>
//! @param [in] forceXYOrientation true to force outer and inner loops to have correct (CCW/CW) order.
GEOMDLLIMPEXP static CurveVectorPtr CreateLinear (
                    DPoint2dCP points,
                    size_t numPoints,
                    BoundaryType boundaryType = CurveVector::BOUNDARY_TYPE_Open,
                    bool forceXYOrientation = false);

//! Create a rectangle from xy corners.
//! @param [in] x0 start point x coordinate
//! @param [in] y0 start point y coordinate
//! @param [in] x1 opposite corner x coordinate
//! @param [in] y1 opposite corner y coordinate
//! @param [in] z z value for all points.
//! @param [in] boundaryType is one of
//! <ul>
//! <li> BOUNDARY_TYPE_Outer: force to counterclockwise and positive area
//! <li> BOUNDARY_TYPE_Inner: force to clockwise and negative area
//! <li> BOUNDARY_TYPE_Open, BOUNDARY_TYPE_None: point order (x0,y0)(x1,y0),(x1,y1),(x0,y1)
//! <li> BOUNDARY_TYPE_UnionRegion, BOUNDARY_TYPE_ParityRegion create two-level structure with rectangle at second level with BOUNDARY_TYPE_Outer
//! </ul>
GEOMDLLIMPEXP static CurveVectorPtr CreateRectangle(double x0, double y0, double x1, double y1, double z,
                    BoundaryType boundaryType = CurveVector::BOUNDARY_TYPE_Outer);

//! Create a regular polygon parallel to the xy plane.
//! @param [in] center center coordinates.
//! @param [in] xDistance distance from center to x axis crossing point.
//! @param [in] numEdge number of edges (distinct vertices)
//! @param [in] isOuterRadius indicates whether x axis is outer or inner radius.
//!  (i.e. for outer radius (true), the x crossing is a vertex, and for inner radius (false) the x cross is the middle of a vertical edge.
//! @param [in] boundaryType is one of
//! <ul>
//! <li> BOUNDARY_TYPE_Outer: force to counterclockwise and positive area
//! <li> BOUNDARY_TYPE_Inner: force to clockwise and negative area
//! <li> BOUNDARY_TYPE_Open, BOUNDARY_TYPE_None: point order (x0,y0)(x1,y0),(x1,y1),(x0,y1)
//! <li> BOUNDARY_TYPE_UnionRegion, BOUNDARY_TYPE_ParityRegion create two-level structure with rectangle at second level with BOUNDARY_TYPE_Outer
//! </ul>
GEOMDLLIMPEXP static CurveVectorPtr CreateRegularPolygonXY
(
DPoint3dCR center,
double xDistance,
int numEdge,
bool isOuterRadius,
BoundaryType boundaryType = CurveVector::BOUNDARY_TYPE_Outer
);

//! Create a BOUNDARY_TYPE_None with line segments.
GEOMDLLIMPEXP static CurveVectorPtr Create (bvector<DSegment3d> const &segments);



//! Create a (deep) curve vector structure for a complete elliptic (circular) disk.
//! @param [in] arc boundary ellipse
//! @param [in] boundaryType is one of
//! <ul>
//! <li> BOUNDARY_TYPE_Outer: force to counterclockwise and positive area as seen looking at xy plane.
//! <li> BOUNDARY_TYPE_Inner: force to clockwise and negative area as seen looking at xy plane.
//! <li> BOUNDARY_TYPE_Open, BOUNDARY_TYPE_None: ellipse inserted with its own direction.
//! <li> BOUNDARY_TYPE_UnionRegion, BOUNDARY_TYPE_ParityRegion create two-level structure with arc at second level with BOUNDARY_TYPE_Outer
//! </ul>
//! @param [in] forceXYOrientation if true, reverse arc sweep so it acts as requested inner/outer in xy view.
GEOMDLLIMPEXP static CurveVectorPtr CreateDisk (DEllipse3dCR arc, BoundaryType boundaryType = CurveVector::BOUNDARY_TYPE_Outer, bool forceXYOrientation = false);

//! Create a single level curve vector structure with a single primitive.
//! @param [in] child child primitive.
//! @param [in] boundaryType 
GEOMDLLIMPEXP static CurveVectorPtr Create (ICurvePrimitivePtr child, BoundaryType boundaryType = CurveVector::BOUNDARY_TYPE_Open);



//! Return a "deep copy" with primitives replaced by bsplines
//! The tree upper levels of the tree structure are retained -- i.e. the output contains corresponding tree structure
//!     ParityRegion, UnionRegion, OuterLoop, and InnerLoop 
//!<ul>
//!<li>UnionRegion and ParityRegion vectors: Create a new CurveVector of the same type.   Recursively create
//!      children.
//!<li>OuterLoop, InnerLoop, OpenPath: Create a new curve vector of the same type.
//!<li>Primitives: Each primtitive is copied as a bspline curve primitive.
//!</ul>
GEOMDLLIMPEXP CurveVectorPtr CloneAsBsplines () const;


//! Return a "deep copy" with primitives replaced by strokes.
//! The tree upper levels of the tree structure are retained -- i.e. the output contains corresponding tree structure
//!     ParityRegion, UnionRegion, OuterLoop, and InnerLoop 
//!<ul>
//!<li>UnionRegion and ParityRegion vectors: Create a new CurveVector of the same type.   Recursively create
//!      children.
//!<li>OuterLoop, InnerLoop, OpenPath: Create a new curve vector of the same type.
//!     Stroke all curve primitives into a single linestring with the child curves' AddStrokes method.
//!</ul>
GEOMDLLIMPEXP CurveVectorPtr Stroke (IFacetOptionsR options) const;

//! Add stroke points form all children to output.
//! Strokes from all children are concatenated into the same vector, separated only by DISCONNECT points.  Use Stroke() to get structured strokes.
//! @param [in,out] points growing vector of strokes.
//! @param [in] options optiosn for stroke density.  chordTolerance, angleTolerance, and maxEdgeLength will be used.
GEOMDLLIMPEXP void AddStrokePoints (bvector <DPoint3d> &points, IFacetOptionsR options) const;

//! Collect strokes from the structure.
//! Each path or loop (at any level) is converted to points.
//! Loops and paths appear at top level in the two-level bvector.
//! regionsPoints[i][j] is point j of regionsPoints[i].
//! @return false if unexpected structure was -- e.g. UnionRegion which needs more levels of structure.
GEOMDLLIMPEXP void AddStrokePoints
(
bvector <DPoint3dDoubleUVCurveArrays>& points, //!< [out] Collection of collections of loops.
IFacetOptionsP strokeOptions = nullptr              //!< [out] Optional stroke controls.
) const;


//! @description Add all strokes from the  structure.  The output point structure retains structure as complex as "union of multi-loop parity regions"
//!<ul>
//!<li>The regionsPoints[i] is an array of loops.
//!<li>The outer curve vector may be a single loop, parity region, or union region.
//!<li>If the strokeOptions pointer is null, all curved primitives are SKIPPED
//!<li>In a BOUNDARY_TYPE_None, each member is added recursively.
//!</ul>
GEOMDLLIMPEXP bool CollectLinearGeometry
(
bvector <bvector<bvector<DPoint3d>>> &regionsPoint, //!< [out] Collection of collections of loops.
IFacetOptionsP strokeOptions = nullptr              //!< [out] Optional stroke controls.
) const;




//! Collect strokes from the structure.
//! Each path or loop (at any level) is converted to points.
//! Loops and paths appear at top level in the two-level bvector.
//! regionsPoints[i][j] is point j of regionsPoints[i].
//! @return false if unexpected structure was -- e.g. UnionRegion which needs more levels of structure.
GEOMDLLIMPEXP bool CollectLinearGeometry (bvector<bvector<DPoint3d>> &regionsPoints, IFacetOptionsP strokeOptions = nullptr) const;

//! Compute points at (many) specified distances along the (many) curves in the CurveVector.
//! Intervals between successive distances can "jump" from one curve to the next.
//! If curves to not connect head to tail, the gap is NOT filled -- measurement just picks up after the gap.
//! @param [in] distances vector of distances, all measured from the start of the first primitive.
//! @param [out] locations vector of locations.  (This is NOT cleared at start -- points are just added.)
GEOMDLLIMPEXP bool AddSpacedPoints (bvector<double> const &distances, bvector<CurveLocationDetail> &locations) const;

//! Compute point at specified distance along the (many) curves, starting at input detail.
//! This can jump from one primitive to others.
//! If curves to not connect head to tail, the gap is NOT filled -- measurement just picks up after the gap.
//! @param [in] startPoint starting point for measurement.
//! @param [in] signedDistance  distance to move forward or backwars.
GEOMDLLIMPEXP ValidatedCurveLocationDetail PointAtSignedDistanceAlong (CurveLocationDetailCR startPoint, double signedDistance) const;


//flex 
//flex || Convert to single-loop bspline curve || status = source.ToBsplineCurve (out curve) ||
//flex 
//! Represent a curve vector that denotes an open or closed path as a single bspline curve.
GEOMDLLIMPEXP BentleyStatus ToBsplineCurve (MSBsplineCurveR curve) const;
//! Represent a curve vector that denotes an open or closed path as a single bspline curve.
GEOMDLLIMPEXP MSBsplineCurvePtr GetBsplineCurve () const;



//flex !! Parametric Queries
//flex 
//flex || Full 3d closest point search || bool cv.ClosestPointBounded (spacePoint, out location) ||
//flex || XY-only closest point search after optional perspective projection || bool cv.ClosestPointBoundedXY (spacePoint, worldToLocal4d, out location)||
//flex || XY-only closest point search after optional perspective projection, || bool cv.ClosestPointBoundedXY (spacePoint, worldToLocal4d, out location, bool extend0, bool extend1)||
//flex ||    with optional extension beyond endpoints ||
//flex || Get start and end points || bool cv.GetStartEnd (out xyzStart, out xyzEnd) ||
//flex || Get start and end points with unit tangent vectors || bool cv.GetStartEnd (out xyzStart, out xyzEnd, out unitTangentStart, out unitTangentEnd) ||
//flex || Deep search for start point of any primitive. || bool cv.GetStartPoint (out xyzStart) ||
//flex 

//! Search for the closest point on any contained curve.
GEOMDLLIMPEXP bool ClosestPointBounded (DPoint3dCR spacePoint, CurveLocationDetailR location) const;
//! Search for the closest point on any contained curve.
GEOMDLLIMPEXP bool ClosestPointBounded (DPoint3dCR spacePoint, CurveLocationDetailR location, bool extend0, bool extend1) const;

//! Search for the closest point on any contained curve, using only xy (viewed) coordinates.
GEOMDLLIMPEXP bool ClosestPointBoundedXY (DPoint3dCR spacePoint, DMatrix4dCP worldToLocal, CurveLocationDetailR location) const;

//! Search for the closest point on any contained curve, using only xy (viewed) coordinates, optionally allowing extensions from free ends of lines and arcs.
GEOMDLLIMPEXP bool ClosestPointBoundedXY (DPoint3dCR spacePoint, DMatrix4dCP worldToLocal, CurveLocationDetailR location, bool extend0, bool extend1) const;
//! Search for closest point on curve.  If CV is a region type, also look for projection onto interior of the region.
//! @param [in] spacePoint fixed point of search
//! @param [out] curveOrRegionPoint computed point on curve or region interior.
//! @return INOUT_On if the point is a curve point.  INOUT_In if the point is a projection to the region interior.
//!       INOUT_Unknown for empty curve vector.
GEOMDLLIMPEXP CurveVector::InOutClassification ClosestCurveOrRegionPoint (DPoint3dCR spacePoint, DPoint3dR curveOrRegionPoint) const;

//! Search for various keypoints (as requested by the collector)
//! During recursion, extension bits are changed to false for interior points of paths
GEOMDLLIMPEXP void AnnounceKeyPoints
(
DPoint3dCR spacePoint,                       //!< [in] point to project
CurveKeyPointCollector &collector,           //!< [in] object to receive keypoint announcements
bool extend0,                               //!< [in] true to extend from start.
bool extend1                                //!< [in] true to extend from end.
) const;

//! Return first/last among children.
//! @param [out] pointA start point
//! @param [out] pointB end point
GEOMDLLIMPEXP bool GetStartEnd (DPoint3dR pointA, DPoint3dR pointB) const;

//! Return details of start and end.
//! @param [out] pointA start point
//! @param [out] pointB end point
GEOMDLLIMPEXP bool GetStartEnd (CurveLocationDetailR pointA, CurveLocationDetailR pointB) const;


//! Return start and end points and the (normalized!) tangents among children.
//! @param [out] pointA start point
//! @param [out] pointB end point
//! @param [out] unitTangentA normalized tangent (forward) at pointA.
//! @param [out] unitTangentB normalized tangent (forward) at pointB.
GEOMDLLIMPEXP bool GetStartEnd (DPoint3dR pointA, DPoint3dR pointB, DVec3dR unitTangentA, DVec3dR unitTangentB) const;

//! Return start point of the primitive (or first primitive in deep search)
//! @param [out] point start point.
GEOMDLLIMPEXP bool GetStartPoint (DPoint3dR point) const;

//! Sum lengths of contained curves.
GEOMDLLIMPEXP double Length () const;

//! Sum lengths of contained curves with transform applied.
GEOMDLLIMPEXP double Length (RotMatrixCP worldToLocal) const;

//! Sum lengths of contained curves, using fast method that may overestimate the length but is reasonable for setting tolerances.
GEOMDLLIMPEXP double FastLength () const;

//! Maximum gap distance between end of primitive and start of its successor within Open, outer, or Inner loop.
GEOMDLLIMPEXP double MaxGapWithinPath () const;

//! Return a fast estimate of the maximum absoluted value in any coordinate.  This will examine all curves, but is allowed to use safe approximations like bspline pole coordinates instead of exact curve calculations.
GEOMDLLIMPEXP double FastMaxAbs () const;

//! Recursive check for structural match (tree structure and leaf type) with the other curve vector.
GEOMDLLIMPEXP bool IsSameStructure (CurveVectorCR other) const;

//! Recursive check for match (tree structure. leaf type, and geometry) with a peer.
//! <param name="other">peer for comparison</param>
//! <param name="tolerance">distance tolerance.   (See DoubleOps::AlmostEqual ())</param>
GEOMDLLIMPEXP bool IsSameStructureAndGeometry (CurveVectorCR other, double tolerance = 0.0) const;

//! Return the xyz range of contained curves.
GEOMDLLIMPEXP bool GetRange (DRange3dR range) const;
//! Compute range of transformed structure.
//! @param [out] transform transform to target coordinates
//! @param [out] range range of transformed curve.
//! @return true if range computed.
GEOMDLLIMPEXP bool GetRange (DRange3dR range, TransformCR transform) const;

//! Test if the contained curves are planar.  If so, return transforms transforms and local range.
//! @param [out] localToWorld A coordinate frame somewhere on the curves.  The curves are on the xy plane.
//! @param [out] worldToLocal Inverse of localToWorld.
//! @param [out] range range of the curves when worldToLocal is applied.
//! @return true if range is computed and has small z component.
//! @remark returns true (planar!!) for a single line.  Test the local range with IsAlmostZeroY to identify this condition.
GEOMDLLIMPEXP bool IsPlanar (TransformR localToWorld, TransformR worldToLocal, DRange3dR range) const;


//! Test if the contained curves are planar.  If so, return transforms transforms and local range.
//! @param [out] localToWorld A coordinate frame somewhere on the curves.  The curves are on the xy plane.
//! @param [out] worldToLocal Inverse of localToWorld.
//! @param [out] range range of the curves when worldToLocal is applied.
//! @param [in] normal optional normal to resolve ambiguous cases.  If this is NULL, any perpendicular to an ambiguous line will be used.
//! @return true if range is computed and has small z component.
GEOMDLLIMPEXP bool IsPlanarWithDefaultNormal (TransformR localToWorld, TransformR worldToLocal, DRange3dR range, DVec3dCP normal) const; 



//flex !! Non-geometric queries
//flex 
//flex || description || ||
//flex || Get the type of singleton leaf, or CURVE_PRIMITIVE_TYPE_Invalid if not a singleton. || primitiveType = cv.HasSingleCurvePrimitive ()    ||
//flex || Count primitivdes of specific type   || n = source.CountPrimitivesOfType (primitiveType) ||
//flex || Test if there are any primitives other than line segment and linesstring || bool cv.ContainsNonLinearPrimitive ( ))    ... ||
//flex || Convert a cyclic index to the actual range. || i = cv.CyclicIndex(i0) ||
//flex || Access by cyclic index || cp = cv.GetCyclic (index) ||
//flex 


//! return mod of index with vector length, properly corrected for negatives.
GEOMDLLIMPEXP size_t CyclicIndex (int index) const;
//! return child at cyclic index, propertly corrected for negatives.
GEOMDLLIMPEXP ICurvePrimitivePtr GetCyclic (ptrdiff_t index) const;

//! return index of curve location detail in vector (only valid for a vector that is a single open or closed path). 
//! Returns SIZE_MAX if not found.
GEOMDLLIMPEXP size_t CurveLocationDetailIndex (CurveLocationDetail const& location) const;

//! return index of primitive in vector (only valid for a vector that is a single open or closed path). 
//! Returns SIZE_MAX if not found.
GEOMDLLIMPEXP size_t FindIndexOfPrimitive (ICurvePrimitiveCP primitive) const;

//! Search the tree (below the calling instance) for the curve vector which is the immediate parent of given primitive.
GEOMDLLIMPEXP CurveVectorPtr FindParentOfPrimitive (ICurvePrimitiveCP primitive) const;

//! return 0 of locations are equal, -1 if location 0 is less than location 1, and 1 if location 0 > location 1. 
//! This is a lexical comparison using (only) the curve index and the fraction.
GEOMDLLIMPEXP int CurveLocationDetailCompare (CurveLocationDetail const& location0, CurveLocationDetail const& location1) const;

//! return larger of given tolerance and default tolerance based on FastMasAbs of contents ...
GEOMDLLIMPEXP double ResolveTolerance (double tolerance) const;

//! Search for children whose start or end is close to a search point.
//! @param [out] data array recording index, fraction (0 or 1), coordinate, and distance of start or end within tolerance
//! @param [in] xyz search point
//! @param [in] tolerance distance tolerance
GEOMDLLIMPEXP void FindPrimitivesWithNearbyStartEnd (bvector<CurveLocationDetail> &data, DPoint3dCR xyz, double tolerance) const;


//flex !! Selective extraction
//flex 
//flex || Split into parts at parametric positions || outCv = cv.GenerateAllParts (indexA, fractionA, indexB, fractionB) ||
//flex || Copy portion between parametric positions || outCv = CloneBetweenDirectedIndexedFractions (indexA, fractionA, indexB, fractionB) ||
//flex || Copy portion between parametric positions, possibly wrapping cyclically (including negative indices) || CloneBetweenCyclicIndexedFractions (indexA, fractionA, indexB, fractionB) ||
//flex || Copy with all orientations reversed || CloneReversed () ||
//flex || Compute one of several types of local coordinate systems.  Return a deep copy transformed to the system. || outCv = cv.CloneInLocalCoordinates (in frameType, out localToWorld, out worldToLocal, out localRange) ||
//flex 


//! Return a new vector containing curves from index0,fraction0 to index1,fraction1 with the (signed int!!) indices interpretted cyclically.
GEOMDLLIMPEXP CurveVectorPtr CloneBetweenCyclicIndexedFractions (int index0, double fraction0, int index1, double fraction1) const;

//! Return a new vector containing curves from index0,fraction0 to index1,fraction1 with the (signed int!!) indices restricted to array bounds.
GEOMDLLIMPEXP CurveVectorPtr CloneBetweenDirectedFractions (int index0, double fraction0, int index1, double fraction1, bool allowExtrapolation, bool usePartialCurves = false) const;

//! Return a new vector containing curves from index0,fraction0 to index1,fraction1 corresponding to the the CurveLocationDetails.
//! (Note that CurveLocationDetail does not contain an index, so this requires a linear search)
GEOMDLLIMPEXP CurveVectorPtr CloneBetweenDirectedFractions
(
CurveLocationDetailCR location0,        //!< [in] start of cloned portion
CurveLocationDetailCR location1,        //!< [in] end of cloned portion
bool allowExtrapolation = false,        //!< [in] true to allow extrapolation beyond ends
bool usePartialCurves = false           //!< [in] true to create CURVE_PRIMITIVE_TYPE_PartialCurve to clone portions of curves.
) const;




//! Return a new curve vector that has all components reversed.
GEOMDLLIMPEXP CurveVectorPtr CloneReversed () const;

//! Return a CurveVector (BOUNDARY_TYPE_None) which is a collection of open CurveVectors that collectively contain all parts of the input
//! For (indexA,fractionA) prededing (indexB,fractionB) the output traces the input in the forward direction and has the following possibilities (of which null ones are skipped)
//!<ul>
//!<li> BOUNDARY_TYPE_Open - (A B), (B to end), (start to A)
//!<li> BOUNDARY_TYPE_Inner or BOUNDARY_TYPE_Outer - (A B), (B to where A appears in the next period)
//!<li> BOUNDARY_TYPE_ParityRegion, BOUNDARY_TYPE_UnionRegion, BOUNDARY_TYPE_None -- no output.
//!</ul>
//! For (indexA,fractionA) prededing (indexB,fractionB) the output traces the input in the reverse direction and has the following possibilities (of which null ones are skipped)
//!<ul>
//!<li> BOUNDARY_TYPE_Open - (A backwards to B), (B backwards to start), (end backwards to A)
//!<li> BOUNDARY_TYPE_Inner or BOUNDARY_TYPE_Outer - (A backwards to B), (B backwards to where A appears in the previous period.)
//!<li> BOUNDARY_TYPE_ParityRegion, BOUNDARY_TYPE_UnionRegion, BOUNDARY_TYPE_None -- no output.
//!</ul>
GEOMDLLIMPEXP CurveVectorPtr GenerateAllParts (int indexA, double fractionA, int indexB, double fractionB) const;

//! Return true if CurveVector has a component that is not a line or linestring.
GEOMDLLIMPEXP bool ContainsNonLinearPrimitive () const;

//flex !! Intersections, Containment
//flex 
//flex || Find intersection of region with plane. || outCp = cv.PlaneSection (plane, tolerance = 0.0) ||
//flex || Intersection with plane.   return can indicate both a) single point contact and ( b)    "on plane" sections. || cv.AppendPlaneIntersections (plane, bvector<SEE(CurveLocationDetailPair)> & outIntersections) ||
//flex || Inside/outside classification for an xy point. || classification = cv.PointInOnOutXY (xyz) ||
//flex 

//! Compute simple points of intersection of the curve with a plane.
//! Single point intersection appears as a CurveLocationDetailPair with identical locations for both parts of the pair (SameCurveAndFraction)
//! Curve-on-plane appears as CurveLocationDetailPair with curve,fraction data for start and end of on-plane sections.
//! @param [in] plane 
//! @param [out] intersections intersection details
//! @param [in] tolerance for on-plane decisions.  If 0, a tolerance is computed based on the coordinates in the curve.
GEOMDLLIMPEXP void AppendCurvePlaneIntersections (DPlane3dCR plane, bvector<CurveLocationDetailPair> &intersections, double tolerance = 0.0) const;

//! Compute intersections of closed CurveVector with a plane and organize as start end pairs by parity rules.
//! Intersectoins are reported as CurveLocationDetailPairs for start and end of segments.
//! @param [in] plane 
//! @param [out] intersections intersection details
//! @param [in] tolerance for on-plane decisions.  If 0, a tolerance is computed based on the coordinates in the curve.
GEOMDLLIMPEXP bool AppendClosedCurvePlaneIntersections (DPlane3dCR plane, bvector<CurveLocationDetailPair> &intersections, double tolerance = 0.0) const;

//! Compute intersections of closed CurveVector with a plane and organize as start end pairs by parity rules.
//! Return as a single curve primitive (which may be child vector of multiple primitives)
//! If there are no intersections the smart pointer is empty (IsValid () returns false)
//! @param [in] plane 
//! @param [in] tolerance for on-plane decisions.  If 0, a tolerance is computed based on the coordinates in the curve.
GEOMDLLIMPEXP ICurvePrimitivePtr PlaneSection (DPlane3dCR plane, double tolerance = 0.0) const;

//! Test if a point is in, on, or outside when looking at xy plane.
//! return INOUT_Unknown if the CurveVector is not an area. (i.e. type BOUNDARY_TYPE_Outer, BOUNDARY_TYPE_Inner, BOUNDARY_TYPE_ParityRegion, or BOUNDARY_TYPE_Union
//! @param [in] xyz test point
GEOMDLLIMPEXP CurveVector::InOutClassification PointInOnOutXY (DPoint3dCR xyz) const;

//! Test for a ray hit in the curve vector's planar region.
//! return INOUT_Unknown if the CurveVector is not an area. (i.e. type BOUNDARY_TYPE_Outer, BOUNDARY_TYPE_Inner, BOUNDARY_TYPE_ParityRegion, or BOUNDARY_TYPE_Union
GEOMDLLIMPEXP CurveVector::InOutClassification RayPierceInOnOut
(
DRay3dCR ray,                    //!< [in] ray to intersect witht the region
SolidLocationDetailR hitDetail   //!< [out] hit point with parameters relative to the region's local coordinates.
) const;

//flex !! Inplace modification
//flex 
//flex || description || ||
//flex || apply transform || cv.Transform (transform) ||
//flex || Consolidate (in place) adjacent line segments into polyines and compataible arc segments into single arcs || cv.ConsolidateAdjacentPrimitives () ||
//flex || Fixup order, boundary type, and loop direction || bool FixupXYOuterInner (fullGeometryCheckFlag) ||
//flex 


//! Apply a transform to all contained curves.
GEOMDLLIMPEXP bool TransformInPlace (TransformCR transform);

//! Recursively reverse.
//! All leaf curves are reversed.
//! Primitive order within path types (_Outer, _Inner, _Open) is reversed.
//! All others (_Union, _Parity, _None) are unchanged.
GEOMDLLIMPEXP bool ReverseCurvesInPlace ();


//! Inplace update to consolidate contiguous parts.
//! Adjacent lines and linestrings become a single linestring. Interior colinear points of linestrings are eliminated.
//! Adjacent and compatible arcs become single arc.
GEOMDLLIMPEXP void ConsolidateAdjacentPrimitives ();

//! Inplace update to consolidate contiguous parts.
//! Adjacent lines and linestrings become a single linestring.
//! Adjacent and compatible arcs become single arc.
//! @param [in] doSimplifyLinestrings true to eliminate colinear interior points of linestrings.
//! @param [in] xyOnly true to use only xy parts for point comparisons.
GEOMDLLIMPEXP void ConsolidateAdjacentPrimitives (bool doSimplifyLinestrings, bool xyOnly = false);

//! Inplace update to consolidate colinear interior points of linestrings.
//! If distance tolerance is nonpositive a tolerance will be assigned from the range.
GEOMDLLIMPEXP void SimplifyLinestrings (double distanceTol, bool eliminateOverdraw, bool wrap, bool xyOnly = false);

//! Update order, boundary type, and direction of contained loops.
//! Loop A is considered "inside" loop B if (a) loop A has smaller area and (b) the start point of loop A is inside loop B.
//! A loop that is "inside" and ODD number of other loops is considered to be a "hole" (inner loop) within the containing loop that has smallest area.
//! Any other loop is considered an outer loop.
//! <ul>
//! <li>If there is a single outer loop, the (modified) curve vector is marked as a parity region.  The outer loop is moved first and the inner loops follows.
//! <li>If there are mulitple outer loops, the (modified) curve vector is marked as a union region. Within the UnionRegion
//!   <ul>
//!    <li>Outer loops with no contained loops are present as simple Outer loops.
//!    <li>Outer loops with holes are parity regions.
//!   </ul>
//! </ul>
//! @param [in] fullGeometryCheck if true, perform all (expensive) tests for intersections among curves.  When this is enabled, the returned curve vector is typically
//!    a (possibly significantly modfied) clone of the original.
GEOMDLLIMPEXP bool FixupXYOuterInner (bool fullGeometryCheck = false);

//! Reorder curve primitives to produce small head-to-tail gaps.
//! reordering is applied only within boundary types None, Open, and Closed.
//! other types are updated recursively.
//! Return the largest gap.
GEOMDLLIMPEXP double ReorderForSmallGaps ();

//! Join curve primitives head to tail.
//! Return a top level BOUNDARY_TYPE_None containing the various BOUNDARY_TYPE_Open and BOUNDARY_TYPE_Outer
GEOMDLLIMPEXP CurveVectorPtr AssembleChains ();


//flex !! Region booleans
//flex !! Each has an optional bvector to receive pairs (newCurve, oldCurve) so caller can manage relationships
//flex || Union    || outRegion = CurveVector::AreaUnion (regionA, regionB, newToOld) ||
//flex || Intersection || outRegion = CurveVector::AreaIntersection (regionA, regionB, newToOld) ||
//flex || Difference || outRegion = CurveVector::AreaDifference (regionA, regionB, newToOld) ||
//flex || Parity   || outRegion = CurveVector::AreaParity (regionA, regionB, newToOld) ||
//flex 

//! Return a curve vector containing the union of input areas.
//! @param [in] regionA left operand
//! @param [in] regionB right operand
//! @param [in,out] newToOld (optional) pointer to bvector to receive paring of new and old curves.
GEOMDLLIMPEXP static CurveVectorPtr AreaUnion (CurveVectorCR regionA, CurveVectorCR regionB, CurvePrimitivePtrPairVector *newToOld = NULL);

//! Return a curve vector containing the difference of input areas.
//! @param [in] regionA left operand
//! @param [in] regionB right operand
//! @param [in,out] newToOld (optional) pointer to bvector to receive paring of new and old curves.
GEOMDLLIMPEXP static CurveVectorPtr AreaDifference (CurveVectorCR regionA, CurveVectorCR regionB, CurvePrimitivePtrPairVector *newToOld = NULL);

//! Return a curve vector containing the intersection of input areas.
//! @param [in] regionA left operand
//! @param [in] regionB right operand
//! @param [in,out] newToOld (optional) pointer to bvector to receive paring of new and old curves.
GEOMDLLIMPEXP static CurveVectorPtr AreaIntersection (CurveVectorCR regionA, CurveVectorCR regionB, CurvePrimitivePtrPairVector *newToOld = NULL);

//! Return a curve vector containing the parity of input areas.
//! @param [in] regionA left operand
//! @param [in] regionB right operand
//! @param [in,out] newToOld (optional) pointer to bvector to receive paring of new and old curves.
GEOMDLLIMPEXP static CurveVectorPtr AreaParity (CurveVectorCR regionA, CurveVectorCR regionB, CurvePrimitivePtrPairVector *newToOld = NULL);





//! Return a curve vector containing the "inside" areas by various conditions.
//! @param [in] region Region that may have loops back over its area.
//! @param [in] select1 Rule for classifying single area: one of AreaSelect_Parity, AreaSelect_CCWPositiveWindingNumber,  AreaSelect_CCWNonzeroWindingNumber, AreaSelect_CCWNegativeWindingNumber
//! @param [in] select2 Rule for combining leaf left results: One of BoolSelect_Parity, BoolSelect_Union, BoolSelect_Sum_Parity, BoolSelect_CCWPositiveWindingNumber,  BoolSelect_CCWNonzeroWindingNumber, BoolSelect_CCWNegativeWindingNumber
//! @param [in] reverse to return the opposite set of faces.
GEOMDLLIMPEXP static CurveVectorPtr AreaAnalysis
        (CurveVectorCR region, AreaSelect select1, BoolSelect select2, bool reverse);

//! Return a curve vector containing only clockwise areas.
//! Loops within parity regions are fixed first.  Then multiple regions in a union are combined.
//! @param [in] regionA input areas
GEOMDLLIMPEXP static CurveVectorPtr ReduceToCCWAreas (CurveVectorCR regionA);


//! Return a curve vector (of type BOUNDARY_TYPE_None) containing hatch sticks.
GEOMDLLIMPEXP static void CreateXYHatch (
bvector<DSegment3d> &sticks,         //!< [out] computed hatch segments
bvector<HatchSegmentPosition> *segmentPosition, //!< [out] For each stick, description of hatch level and distance along.
CurveVectorCR        boundary,      //!< [in] boundary curves.
DPoint3dCR           startPoint,    //!< [in] Start point for hatch lines
double               angleRadians,  //!< [in] angle from X axis.
double               spacing,       //!< [in] spacing perpendicular to hatch direction
int                  selection = 0  //!< 0 for parity rules, 1 for longest possible strokes (first to last crossings), 2 for leftmsot and rightmost of parity set.
);

//! Return a curve vector (of type BOUNDARY_TYPE_None) containing hatch sticks.

GEOMDLLIMPEXP static void CreateHatch (
bvector<DSegment3d> &sticks,         //!< [out] computed hatch segments
bvector<HatchSegmentPosition> *segmentPosition, //!< [out] For each stick, description of hatch level and distance along.
CurveVectorCR        boundary,      //!< [in] boundary curves.
TransformCR          worldToIntegerZPlanes, //!< [in] Transform to space where each integer Z value is a cut plane.
int                  selection = 0  //!< 0 for parity rules, 1 for longest possible strokes (first to last crossings), 2 for leftmsot and rightmost of parity set.
);

/*---------------------------------------------------------------------------------**//**
* @description Return a scale factor to be applied to the z-axis of the hatch transform so that
* at most maxLines scan planes are defined within specified range.
* @param transform => proposed hatch transform.  xy plane is hatch plane.  z direction
*       is advance vector between successive planes.
* @param worldRange range of data.
* @param maxLines => max number of lines allowed in specified ranges. If 0 or negative, a default
*               is applied.
* @return scale factor to apply to z vector. Any error condition returns 1.0.  Example error
*       conditions are (a) null range, and (b) singular transform.
+---------------+---------------+---------------+---------------+---------------+------*/
GEOMDLLIMPEXP static ValidatedDouble ComputeHatchDensityScale
(
TransformCR transform,
DRange3dCR worldRange,
int                             maxLines
);

//! Return a curveVector with spiral transition between two lines, with shoulder points controlling the spiral max curvatures.
//!<ul>
//!<li>Initial spiral starts at pointA, heading towards shoulderB with zero curvature.
//!<li>initial spiral turns towards shoulderC.
//!<li>initial spiral ends and has curvature-continuous change to a spiral unwinding to a tangency with line shoulderB to shoulderC
//!<li>straight line along shoulderB to shoulderC until
//!<li>second pair of spirals to transition to pointD with tangent to the direction from shoulderC to pointD
//!</ul>
//!
//! If pointA,shoulderB, shoulderC is degnerate (all colinear), the initial spiral pair is omitted and the intermediate line starts at pointA.
//! If shoulderB, shoulderC,pointD is degenerate (all colinear), the final spiral pair is omitted and the intermediate line ends at pointD.
GEOMDLLIMPEXP static CurveVectorPtr CreateSpiralLineToLineShift
(
int transitionType,     //!< [in] transition type
DPoint3dCR pointA,      //!< [in] start point
DPoint3dCR shoulderB,     //!< [in] first target point
DPoint3dCR shoulderC,     //!< [in] second target point
DPoint3dCR pointD     //!< [in] end point
);

//! Return a curveVector with spiral-arc-spiral transtion between two lines, subject to:
//!<ul>
//!<li>specified arcRadiius for central part
//!<li>specified spiralLength for both entry and exit.
//!<li>the spiral-to-line tangency can float along the respective straight line parts
//!</ul>
GEOMDLLIMPEXP static CurveVectorPtr ConstructSpiralArcSpiralTransition
(
DPoint3dCR xyz0,    //!< [in] PI prior to this transition
DPoint3dCR xyz1,    //!< [in] PI for this transition
DPoint3dCR xyz2,    //!< [in] PI after this transition
double arcRadius,   //!< [in] radius of arc portion
double spiralLength //!< [in] length of spiral
);

//! Construct a spiral-arc-spiral transition that is an (approximate) offset of a primary transition.
//!<ul>
//!<li>The offset is a constructSpiralArcSpiralTransition for a simple offset of the 3 point alignments.
//!<li>The radius differs from the primaryRadius by (exactly) offsetDistance.
//!<li>The spiral length is adjusted so that the tangency point for the line-to-spiral is exactly at the offset of
//!     the corresponding line-spiral
//!<li>specified spiralLength for both entry and exit.
//!<li>the spiral-to-line tangency can float along the respective straight line parts
//!<li>offsets at all distances maintain the spiral-arc-spiral structure.   Because the true "rolling ball" offset of a spiral is not
//!    a spiral, the result is only an approximation of a true "rolling ball" offset.
//!</ul>
GEOMDLLIMPEXP static CurveVectorPtr ConstructSpiralArcSpiralTransitionPseudoOffset
(
DPoint3dCR primaryPoint0,   //!< [in] primary path PI prior to this transition
DPoint3dCR primaryPoint1,   //!< [in] primary path PI for this transition
DPoint3dCR primaryPoint2,   //!< [in] primary path PI after this transition
double primaryRadius,       //!< [in] radius of primary path
double primarySpiralLength, //!< [in] primary path spiral length
double offsetDistance       //!< [in] offset distance.  Positive is to the left of the primary path
);

//! Offset the path to both sides, forming an area.
//! <ul>
//! <li> Mixing a positive and negative makes an area completely to one side of the center path
//! <li> Either (but not both) of the offsets can be zero, preserving the center path as a boundary of the area.
//! </ul>
static GEOMDLLIMPEXP CurveVectorPtr  ThickenXYPathToArea
(
    CurveVectorPtr const& path,     //!< [in] original curves.   Expected to be an open, inner, or outer.
    double leftWidth,               //!< [in] offset to left.  Positive is left.
    double rightWidth               //!< [in] offset to right. Positive is right.
);
}; // CurveVector



//! static operations on curves.
//! @ingroup BentleyGeom_PolymorphicCurves

struct CurveCurve
{
private: CurveCurve (); // static class, no instances allowed.
public:
//! Collect apparent intersections between curves as viewed in XY plane, optionally after a
//!   transform.
//! Each intersection is recorded as a "partial curve" that has fraction data indicating
//!   its position on the parent curve.
//! If the intersection is a single point, the "partial curve" has (bitwise) identical start
//!   and end fractions.
//! If the intersection is a coincident portion of the curves, the "partial curve" indicates the
//!   fractional range of the intersection.
static GEOMDLLIMPEXP void IntersectionsXY
(
CurveVectorR intersectionA,
CurveVectorR intersectionB,
CurveVectorR curveA, 
CurveVectorR curveB,
DMatrix4dCP    pWorldToLocal
);

public:
//! Collect apparent intersections between curves as viewed in XY plane, optionally after a
//!   transform.
//! Each intersection is recorded as a "partial curve" that has fraction data indicating
//!   its position on the parent curve.
//! If the intersection is a single point, the "partial curve" has (bitwise) identical start
//!   and end fractions.
//! If the intersection is a coincident portion of the curves, the "partial curve" indicates the
//!   fractional range of the intersection.
static GEOMDLLIMPEXP void SelfIntersectionsXY
(
CurveVectorR intersectionA,
CurveVectorR intersectionB,
CurveVectorR curve, 
DMatrix4dCP    pWorldToLocal
);

//! Collect apparent intersections between curves as viewed in XY plane, optionally after a
//!   transform.
//! Each intersection is recorded as a "partial curve" that has fraction data indicating
//!   its position on the parent curve.
//! If the intersection is a single point, the "partial curve" has (bitwise) identical start
//!   and end fractions.
//! If the intersection is a coincident portion of the curves, the "partial curve" indicates the
//!   fractional range of the intersection.
static GEOMDLLIMPEXP void IntersectionsXY
(
CurveVectorR intersectionA,     //!< intersection points on curveA
CurveVectorR intersectionB,     //!< intersection points on curveB
ICurvePrimitiveP curveA,       //!< first curves
ICurvePrimitiveP curveB,       //!< second curves
DMatrix4dCP    pWorldToLocal,   //!< optional viewing transformation
bool         extend = false     //!< extend the two curves?
);

//! Collect apparent intersections between curves as viewed in XY plane, optionally after a
//!   transform.
//! Each intersection is recorded as a "partial curve" that has fraction data indicating
//!   its position on the parent curve.
//! If the intersection is a single point, the "partial curve" has (bitwise) identical start
//!   and end fractions.
//! If the intersection is a coincident portion of the curves, the "partial curve" indicates the
//!   fractional range of the intersection.
static GEOMDLLIMPEXP void IntersectionsXY
(
CurveVectorR intersectionA,     //!< intersection points on curveA
CurveVectorR intersectionB,     //!< intersection points on curveB
ICurvePrimitiveR curveA,       //!< first curves
CurveVectorR     curveB,       //!< second curves
DMatrix4dCP    pWorldToLocal   //!< optional viewing transformation
);

//! Collect apparent intersections between rotating curves and a ray. (Only simple intersections are reported)
//! The curve and ray are both transformed to the local system and allowed to rotate
//! around the local system z axis.
static GEOMDLLIMPEXP void IntersectionRotatedRay
(
bvector<CurveLocationDetail>&intersectionA, //!< intersection parameters on curves
bvector<CurveLocationDetail>&intersectionB, //!< intersection parameters on ray.
ICurvePrimitiveCP curveA,       //!< curves
TransformCR  worldToLocal,      //!< world to local.
DRay3dCR            rayB        //!< ray to intersect with surface swept by rotating curves
);

//! Find points (or intervals!!!) where curveA and curveB have close approach.
//! @param [out] pointsOnA "partial curve" data for points or intervals on curve A
//! @param [out] pointsOnB "partial curve" data for points or intervals on curve B
//! @param [in] curveA first curve
//! @param [in] curveB second curve
//! @param maxDist [in] largest distance to consider.
static GEOMDLLIMPEXP void CloseApproach
(
CurveVectorR pointsOnA,
CurveVectorR pointsOnB,
ICurvePrimitiveP curveA, 
ICurvePrimitiveP curveB,
double maxDist = DBL_MAX
);

//! Run a Newton iteration from given start positions to search for a close approach point.
//! This is not guaranteed to converge.
//! This is not guaranteed to converge to the closest of mutliple results.
//! The Newton iteration uses extended curves that may be evaluated beyond the usual 0..1 fraction range.
static GEOMDLLIMPEXP bool ClosestApproachNewton (
    ICurvePrimitiveCR curveA,   //!< [in] First curve to search
    ICurvePrimitiveCR curveB,   //!< [in] Second curve to search
    double &fractionA,          //!< fractional position to be updated.
    double &fractionB,          //!< fractional position to be updated.
    DPoint3dR xyzA,              //!< [out] final point on curveA.
    DPoint3dR xyzB               //!< [out] final point on curveB.
    );
    
//! Find points (or intervals!!!) where (chains) curveA and curveB have close approach.
//! @param [out] pointsOnA "partial curve" data for points or intervals on curve A
//! @param [out] pointsOnB "partial curve" data for points or intervals on curve B
//! @param [in] chainA first curve
//! @param [in] chainB second curve
//! @param maxDist [in] largest distance to consider.
void CloseApproach
(
CurveVectorR pointsOnA,
CurveVectorR pointsOnB,
CurveVectorCR chainA, 
CurveVectorCR chainB,
double maxDist
);

//! Find a single point where curveA and curveB have closest approach.
//! return false if one or both curves have no geometry.
//! @param [out] pointOnA point on curve A
//! @param [out] pointOnB point on curve B
//! @param [in] curveA first curve
//! @param [in] curveB second curve
static GEOMDLLIMPEXP bool ClosestApproach
(
CurveLocationDetailR pointOnA,
CurveLocationDetailR pointOnB,
ICurvePrimitiveP curveA, 
ICurvePrimitiveP curveB
);

//! Find a single point where two chains have closest approach.
//! return false if one or both curves have no geometry.
//! @param [out] pointOnA point on curve A
//! @param [out] pointOnB point on curve B
//! @param [in] chainA first chain
//! @param [in] chainB second chain
static GEOMDLLIMPEXP bool ClosestApproach
(
CurveLocationDetailR pointOnA,
CurveLocationDetailR pointOnB,
CurveVectorCR chainA, 
CurveVectorCR chainB
);


//! Convenience function for extracting single point intersection from the output of CurveCurveIntersectionsXY.
//! Specifically, test if the specified index in the CurveVectors are both PartialCurve with a degenerate
//! (single point) fraction interval.  If so return the fractions and the evaluated points.
//! (Use GetPartialCurveDetailPair to get the full detail with curve pointer.)
//! return false if index out of range, index addresses something other than PartialCurveData, or
//!    partial curve data is anything other than a single point.
//! @param [in] intersectionA first source vector
//! @param [in] intersectionB second source vector
//! @param [in] i index to acccess in source vectors.
//! @param [out] fractionA fraction from data at intersectionA[i]
//! @param [out] pointA point from data at intersectionA[i]
//! @param [out] fractionB fraction from data at intersectionB[i]
//! @param [out] pointB point from data at intersectionB[i]
static GEOMDLLIMPEXP bool IsSinglePointPair
(
CurveVectorR intersectionA,
CurveVectorR intersectionB,
size_t i,
double &fractionA,
DPoint3dR pointA,
double &fractionB,
DPoint3dR pointB
);

//! Convenience function for extracting single point intersection from the output of CurveCurveIntersectionsXY.
//! Specifically, test if the specified index in the CurveVectors are both PartialCurve with a degenerate
//! (single point) fraction interval.  If so return the points and tangent vectors.
//! (Use GetPartialCurveDetailPair to get the full detail with curve pointer.)
//! return false if index out of range, index addresses something other than PartialCurveData, or
//!    partial curve data is anything other than a single point, or the points do not have well-defined tangent.
//! @param [in] intersectionA first source vector
//! @param [in] intersectionB second source vector
//! @param [in] i index to acccess in source vectors.
//! @param [out] pointAndUnitTangentA
//! @param [out] pointAndUnitTangentB
static GEOMDLLIMPEXP bool IsSinglePointPair
(
CurveVectorR intersectionA,
CurveVectorR intersectionB,
size_t i,
DRay3dR pointAndUnitTangentA,
DRay3dR pointAndUnitTangentB
);

//! Convenience function for extracting single point intersection from the output of CurveCurveIntersectionsXY.
//! Specifically, test if the specified index in the CurveVectors are both PartialCurve with a degenerate
//! (single point) fraction interval.  If so return the fractions and the evaluated points.
//! (Use GetPartialCurveDetailPair to get the full detail with curve pointer.)
//! return false if index out of range, index addresses something other than PartialCurveData, or
//!    partial curve data is anything other than a single point.
//! @param [in] intersectionA first source vector
//! @param [in] intersectionB second source vector
//! @param [in] i index to acccess in source vectors.
//! @param [out] detailA point at intersectionA[i]
//! @param [out] detailB oint at intersectionB[i]
static GEOMDLLIMPEXP bool IsSinglePointPair
(
CurveVectorR intersectionA,
CurveVectorR intersectionB,
size_t i,
CurveLocationDetailR detailA,
CurveLocationDetailR detailB
);

//! Test if interval i is contained in interval j.
static GEOMDLLIMPEXP bool IsContained
(
CurveVectorR intersectionA, //!< intersection intervals (partial curves)
CurveVectorR intersectionB, //!< intersection intervals (partial curves)
size_t i,                   //!< index of first intersection interval
size_t j,                   //!< index of second intersection interval
bool considerSinglePoints,    //!< true to consider j if it is a point
bool considerIntervals        //!< true to consider j if it an interval
);



//! Convenience function for extracting PartialCurveDetail data from two arrays
//! returned by CurveCurve::IntersectionsXY
//! return false if index out of range or addresses something other than PartialCurveData.
//! @param [in] intersectionA first source vector
//! @param [in] intersectionB second source vector
//! @param [in] i index to acccess in source vectors.
//! @param [out] detailA data from intersectionA
//! @param [out] detailB data from intersectionB
static GEOMDLLIMPEXP bool GetPartialCurveDetailPair
(
CurveVectorR intersectionA,
CurveVectorR intersectionB,
size_t i,
PartialCurveDetailR detailA,
PartialCurveDetailR detailB
);


//! Detail about a computed fillet.
//! detailA, detailB refer to the curves the fillet joins.
struct FilletDetail
{
CurveLocationDetail detailA, detailB;
DEllipse3d arc;
};

//! compute all possible fillet arcs between primitives.
//! @param [in] curveA first primitive
//! @param [in] curveB second primitive
//! @param [in] radius fillet radius.
//! @param [in] extend true to allow fillet to touch on extended geometry
//! @param [out] arcs collected arcs.
static GEOMDLLIMPEXP void CollectFilletArcs
(
ICurvePrimitiveR curveA, 
ICurvePrimitiveR curveB,
double radius,
bool extend,
bvector<FilletDetail> &arcs
);

//! compute all possible fillet arcs between primitives.
//! @param [in] chainA first set of primitives
//! @param [in] chainB second set of primitives
//! @param [in] radius fillet radius.
//! @param [in] extend true to allow fillet to touch on extended geometry
//! @param [out] arcs collected arcs.
static GEOMDLLIMPEXP void CollectFilletArcs
(
CurveVectorCR chainA, 
CurveVectorCR chainB,
double radius,
bool extend,
bvector<FilletDetail> &arcs
);

//! Compute all possible blend curves between pairs of curves from two sources.
static GEOMDLLIMPEXP void CollectBlends
(
ICurvePrimitiveR curveA,    //!< First source set
ICurvePrimitiveR curveB,    //!< second source set
BlendType blendType,        //!< Select blend type
double distanceA,           //!< distance applicable to curveA
double distanceB,           //!< distance applicable to curveB
bool extend,                //!< true to allow blends to extended lines and arcs
bvector<BlendDetail> &blendCurves  //!< computed blend curves.  Each detail has a curve location detail for its position on each curve, and a primitive for the blend itself.
);

//! Compute all possible blend curves between pairs of curves from two sources.
static GEOMDLLIMPEXP void CollectBlends
(
CurveVectorCR chainA,       //!< first source set
CurveVectorCR chainB,       //!< second source set
BlendType blendType,        //!< Select blend type
double distanceA,           //!< distance applicable to curveA
double distanceB,           //!< distance applicable to curveB
bool extend,                //!< true to allow blends to extended lines and arcs
bvector<BlendDetail> &blendCurves  //!< computed blend curves.  Each detail has a curve location detail for its position on each curve, and a primitive for the blend itself.
);

//! Return false if the curve vectors are not individually planar or are not closed
static GEOMDLLIMPEXP bool TransverseRegionIntersectionSegments
(
CurveVectorCR regionA,     //!< first source set
CurveVectorCR regionB,     //!< second source set
bvector<DSegment3d> &segments  //!< Destination for segments.
);

//! Construct a mutiple-radius fillet in a corner.
//! The turn angle is distributed evenly among the radii.
static GEOMDLLIMPEXP CurveVectorPtr ConstructMultiRadiusBlend
(
DPoint3dCR corner,          //!< [in] corner of nominal sharp turn.
DVec3dCR   vectorA,         //!< [in] outbound vector on A side.
DVec3dCR   vectorB,         //!< [in] outbound vector on B side.
bvector<double> radii,      //!< [in] vector of successive radii on the transition.
bool reverse = false         //!<  [in] true to do reverse blend (e.g in a right angle, construct 270 turn initially heading away from the corner)
);

//! Construct a mutiple-radius fillet in a corner.
//! The turn angle is distributed evenly among the radii.
static GEOMDLLIMPEXP CurveVectorPtr ConstructMultiRadiusBlend
(
DPoint3dCR corner,          //!< [in] corner of nominal sharp turn.
DVec3dCR   vectorA,         //!< [in] outbound vector on A side.
DVec3dCR   vectorB,         //!< [in] outbound vector on B side.
Angle hardAngleAtStart,     //!< hard angle to turn from start tangent onto start line
double startDistance,       //!< distance to move on start line
bvector<double> radii,      //!< [in] radii for fillets
double endDistance,         //!< distance to move beyond tangent from final arc
Angle hardAngleAtEnd,       //!< hard angle to turn from end tangent to end line
bool reverse = false         //!<  [in] true to do reverse blend (e.g in a right angle, construct 270 turn initially heading away from the corner)
);


//! Construct a sequence of arcs that have given radii and sweeps and join each other with tangent continuity.
static GEOMDLLIMPEXP CurveVectorPtr ConstructTangentArcChain
(
DPoint3dCR startPoint,          //!< [in] corner of nominal sharp turn.
DVec3dCR   startTangent,         //!< [in] outbound vector on A side.
DVec3dCR   planeNormal,     //!< [in] normal vector for plane of arc.
bvector<double> radii,       //!< [in] vector of arc radii
bvector<Angle> angles       //!< [in] angles for arc sweeps
);

//! Search for a multi-radius blend near given start fractions.
static GEOMDLLIMPEXP CurveVectorPtr ConstructMultiRadiusBlend
(
ICurvePrimitiveR curveA,    //!< First source set
ICurvePrimitiveR curveB,    //!< second source set
bvector<double> radii,      //!< [in] radii for fillets
double &fractionA,          //!< [in,out] fraction on curveA
double &fractionB,          //!< [in,out] fraction on curveB
bool reverse = false        //!<  [in] true to do reverse blend (e.g in a right angle, construct 270 turn initially heading away from the corner)
);

//! Search for a multi-radius blend near given start fractions.
//! The blend optionally begins and ends with hard turn and straight line move from curve, and ends with straight line continuation of the
//! final arc, leading to hard turn at end.
static GEOMDLLIMPEXP CurveVectorPtr ConstructMultiRadiusBlend
(
ICurvePrimitiveR curveA,    //!< First source set
ICurvePrimitiveR curveB,    //!< second source set
Angle hardAngleAtStart,     //!< hard angle to turn from start tangent onto start line
double startDistance,       //!< distance to move on start line
bvector<double> radii,      //!< [in] radii for fillets
double endDistance,         //!< distance to move beyond tangent from final arc
Angle hardAngleAtEnd,       //!< hard angle to turn from end tangent to end line
double &fractionA,          //!< [in,out] fraction on curveA
double &fractionB,           //!< [in,out] fraction on curveB
bool reverse                  //!<  [in] true to do reverse blend (e.g in a right angle, construct 270 turn initially heading away from the corner)
);


//! Compute taperFilletTaper transition in the smaller sector between the vectors.
//! Optional offset are shifts that are positive "into" the sector of construction.
static GEOMDLLIMPEXP CurveVectorPtr ConstructTaperFilletTaper
(
DPoint3dCR corner,          //!< [in] corner of nominal sharp turn.
DVec3dCR   vectorA,         //!< [in] vector to inbound side of blend
DVec3dCR   vectorB,         //!< [in] vector to outbound side of blend
double setbackA,            //!< [in] setback distance from vectorA
double taperA,              //!< [in] taper distance along vectorA
double filletRadius,        //!< [in] fillet radius
double setbackB,            //!< [in] setback distance from vectorB
double taperB,              //!< [in] taper distance along vectorB
double offsetA = 0.0,       //!< [in] optional offset from curveA.
double offsetB = 0.0        //!< [in] optional offset from curveB.
);

//! Search for a symmetric taper and fillet blend near given start fractions
//! Optional offset are shifts that are positive "into" the sector of construction.
static GEOMDLLIMPEXP CurveVectorPtr ConstructTaperFilletTaper
(
ICurvePrimitiveR curveA,    //!< First source set
ICurvePrimitiveR curveB,    //!< second source set
double setbackA,            //!< [in] setback distance from vectorA
double taperA,              //!< [in] taper distance along vectorA
double filletRadius,        //!< [in] fillet radius
double setbackB,            //!< [in] setback distance from vectorB
double taperB,             //!< [in] taper distance along vectorB
double &fractionA,          //!< [in,out] fraction on curveA
double &fractionB,           //!< [in,out] fraction on curveB
double offsetA = 0.0,       //!< [in] optional offset from curveA.
double offsetB = 0.0        //!< [in] optional offset from curveB.
);

//! Search for a symmetric taper and fillet blend near given start distances
static GEOMDLLIMPEXP CurveVectorPtr ConstructTaperFilletTaper
(
CurveVectorWithDistanceIndexR curveA,    //!< First source set
CurveVectorWithDistanceIndexR curveB,    //!< second source set
double setbackA,            //!< [in] setback distance from vectorA
double taperA,              //!< [in] taper distance along vectorA
double filletRadius,        //!< [in] fillet radius
double setbackB,            //!< [in] setback distance from vectorB
double taperB,              //!< [in] taper distance along vectorB
double &distanceA,          //!< [in,out] distance on curveA
double &distanceB,           //!< [in,out] distance on curveB
double offsetA = 0.0,       //!< [in] optional offset from curveA.
double offsetB = 0.0        //!< [in] optional offset from curveB.
);
//! Search for locations where there is a local min or max in the signed Z distance between
//! curves that are montone increasing in X.
static GEOMDLLIMPEXP void CollectLocalZApproachExtremaOrderedX
(
CurveVectorCR curveA,                           //!< [in] curve A
CurveVectorCR curveB,                           //!< [in] curve B
bvector<CurveLocationDetailPair> &localMin,   //!< [out] local minima
bvector<CurveLocationDetailPair> &localMiax   //!< [out] local maxima
);

//! Collect "vertical strokes" between corresponding points on two "vertical alignment" curvevectors.
static GEOMDLLIMPEXP void CollectOrderedXVerticalStrokes
(
CurveVectorCR curveA,
CurveVectorCR curveB,
bvector<CurveLocationDetailPair> &allPairs
);
//! Compute intersections of (a) a curve rotated around an axis and (b) a space curve.
static GEOMDLLIMPEXP void IntersectRotatedCurveSpaceCurve
(
TransformCR worldToLocal,               //!< [in] transform into system where rotation is around the Z axis
CurveVectorCR rotatedCurve,             //!< [in] curve to be rotated
ICurvePrimitiveCR spaceCurve,           //!< [in] space curve
bvector<CurveLocationDetail> &detailA,   //!< [out] intersection points on the rotated curve
bvector<CurveLocationDetail> &detailB    //!< [out] intersection points on the space curve
);

//! Construct (possibly many) arcs that have:
//!<ul>
//!<li>pointA is the start
//!<li>tangentA is the tangent from the start point
//!<li>endpoint tangent to a CurvePrimitive in curves.
//!</ul>
//! In each FilletDetail:
//! <ul>
//! <li> arc is the arc
//! <li> detailA.point is pointA
//! <li> detailB is the curve detail from at the tangency with the curve
//! </ul>
static GEOMDLLIMPEXP void ConstructArcs_PointTangentCurveTangent
(
bvector<FilletDetail> &arcs,
DPoint3dCR pointA,
DVec3dCR tangentA,
CurveVectorCR curves
);
//! Within the curveStrokes vector, for each adjacent pair that share curve pointer search for a tangent circle construction
//! within the fraction interval of the pair.  
//!<ul>
//!<li>See ConstructArcs_PointTangentCurveTangent variant with the last parameter as a curvevector for full description of the arc construction.
//!<li>only the fraction and curve pointer arrays are referenced.
//!<li>Note that this method adds points to the output but does not initially clear it.
//!</ul>
static GEOMDLLIMPEXP void ConstructArcs_Add_PointTangentCurveTangent
(
bvector<FilletDetail> &arcs,
DPoint3dCR pointA,
DVec3dCR tangentA,
DPoint3dDoubleUVCurveArrays &curveStrokes
);

//! Search for curve points that can be the endpoint tangency of an arc starting at pointA with direction tangentA
//!<ul>
//!<li>See ConstructArcs_PointTangentCurveTangent variant with the last parameter as a curvevector for full description of the arc construction.
//!<li>only the fraction and curve pointer arrays are referenced.
//!</ul>
static GEOMDLLIMPEXP void ConstructArcs_PointTangentCurveTangent
(
bvector<FilletDetail> &arcs,
DPoint3dCR pointA,
DVec3dCR tangentA,
ICurvePrimitiveCR curve
);

};


/*__PUBLISH_SECTION_END__*/
GEOMDLLIMPEXP bool ImprovePlaneCurveIntersection (DPlane3dCR plane, ICurvePrimitiveCP curve, CurveLocationDetailPair& pair);
GEOMDLLIMPEXP void ImprovePlaneCurveIntersections (DPlane3dCR plane, ICurvePrimitiveCP curve, bvector<CurveLocationDetailPair> &intersections);
GEOMDLLIMPEXP bool ImprovePerpendicularProjection (ICurvePrimitiveCP curve, DPoint3dCR spacePoint, double &fraction, DPoint3dR xyz);
GEOMDLLIMPEXP bool ImprovePerpendicularProjectionXY (ICurvePrimitiveCP curve, DPoint3dCR spacePoint, double &fraction, DPoint3dR xyz, DMatrix4dCP matrix);
GEOMDLLIMPEXP bool ImprovePlaneCurveCurveTransverseIntersectionXY
(
ICurvePrimitiveCR curveA,
ICurvePrimitiveCR curveB,
DMatrix4dCP worldToLocal,
double &fractionA,
double &fractionB
);

GEOMDLLIMPEXP void AppendTolerancedPlaneIntersections (DPlane3dCR plane, ICurvePrimitiveCP curve, DEllipse3d ellipse, bvector<CurveLocationDetailPair> &intersections, double tol);
GEOMDLLIMPEXP void AppendTolerancedPlaneIntersections (DPlane3dCR plane, ICurvePrimitiveCP curve, DSegment3dCR segment, bvector<CurveLocationDetailPair> &intersections, double tol);
GEOMDLLIMPEXP void AppendTolerancedPlaneIntersections (DPlane3dCR plane, ICurvePrimitiveCP curve, MSBsplineCurveCR bcurve, bvector<CurveLocationDetailPair> &intersections, double tol);
GEOMDLLIMPEXP void AppendTolerancedPlaneIntersections (DPlane3dCR plane, ICurvePrimitiveCP curve, bvector<DPoint3d>const &points, bvector<CurveLocationDetailPair> &intersections, double tol);
GEOMDLLIMPEXP void AppendTolerancedPlaneIntersections (DPlane3dCR plane, ICurvePrimitiveCP curve, CurveVectorCR curves, bvector<CurveLocationDetailPair> &intersections, double tol);
GEOMDLLIMPEXP void AppendTolerancedPlaneIntersections (DPlane3dCR plane, ICurvePrimitiveCP curve, DCatenary3dPlacementCR catenary, bvector<CurveLocationDetailPair> &intersections, double tol);

//return a 2d curve offset in the XY plane
GEOMDLLIMPEXP CurveVectorPtr FullOffset (CurveVectorPtr curve, double const& distanceOffset);

/*__PUBLISH_SECTION_START__*/

//! A PathLocationDetail is a detailed description of where a point is along multi-curve path.
//!  "Detailed description" means it has
//!<ul>
//!<li>An index of a primitive within the entire path.
//!<li>The "along path" distance from the start of the entire path.
//!<li>A CurveLocationDetail giving detailed postion within a particular curve.
//!       The "a" field of CurveLocationDetail is used for distance from the start of the curve.
//!</ul>
//!
//! In normal use, all fields are present and self-consistent.  Certain computations may leave fields undefined.
//!  (e.g FractionToPathLocationDetail offers the option of skipping the distance calculation)
//! @ingroup BentleyGeom_ReturnedData
struct PathLocationDetail
{
friend struct CurveVectorWithDistanceIndex;
friend struct CurveVectorWithXIndex;
friend struct PathLocationDetailPair;
private:
CurveLocationDetail m_curveDetail;
int32_t m_pathIndex;    // index to the parent path.
double m_pathDistance;  // distance along path.
public:
//! Constructor -- all zeros except pathIndex is -1
GEOMDLLIMPEXP PathLocationDetail ();

//! Copy constructor
GEOMDLLIMPEXP PathLocationDetail (PathLocationDetail const &other);

//! Constructor with supplied curve detail. pathIndex as int
GEOMDLLIMPEXP PathLocationDetail (CurveLocationDetailCR curveDetail, int pathIndex = -1, double distance = DBL_MAX);
//! Constructor with supplied curve detail, pathIndex as size_t
GEOMDLLIMPEXP PathLocationDetail (CurveLocationDetailCR curveDetail, size_t pathIndex, double distance = DBL_MAX);

//! Constructor with just distance -- e.g. to create a key for IsLessThan_ByPathDistance.
GEOMDLLIMPEXP PathLocationDetail (double distance);

GEOMDLLIMPEXP PathLocationDetail FractionToPathLocationDetail (double f, bool evaluateDistance = true) const;
//! Query xyz coordinates
GEOMDLLIMPEXP DPoint3d Point () const;
//! Query xyz coordinates and normalized tangent vector. (This reevaluates the curve)
GEOMDLLIMPEXP DPoint3d PointAndUnitTangent (DVec3dR unitTangent) const;
//! Query CurveLocationDetail with fractional position for queries
GEOMDLLIMPEXP CurveLocationDetail GetCurveLocationDetail () const;

//! Query the curve primitive index within the path
GEOMDLLIMPEXP size_t PathIndex () const;
//! Set the curve primitive index within the path
GEOMDLLIMPEXP void SetPathIndex (int32_t i);
//! Set the curve primitive index within the path
GEOMDLLIMPEXP void SetPathIndex (size_t i);


//! Query the curve primitive fraction
GEOMDLLIMPEXP double CurveFraction () const;

//! Ask if two details reference the same curve.  If so return the fraction limits.
GEOMDLLIMPEXP bool IsSameCurve (PathLocationDetail const &other, DSegment1d &fractionInterval, ICurvePrimitive::CurvePrimitiveType &curveType) const;

//! Test if there is a curve.
GEOMDLLIMPEXP bool HasCurve () const;
//! distance between points of this and other detail.
GEOMDLLIMPEXP double DistanceToPoint (PathLocationDetail const &other) const;
//! distance to given point.
GEOMDLLIMPEXP double DistanceToPoint (DPoint3dCR xyz) const;
//! distance squared to given point.
GEOMDLLIMPEXP double DistanceSquaredToPoint (DPoint3dCR xyz) const;
//! Queryt the stored distance to path start.  Note that this is not a recompute -- just a member access.
GEOMDLLIMPEXP double DistanceFromPathStart () const;
//! Comparison using only the stored distance.
static GEOMDLLIMPEXP bool IsLessThan_ByPathDistance   (PathLocationDetail const &dataA, PathLocationDetail const &dataB);

//! Test if two details have the same curve pointer.
GEOMDLLIMPEXP bool IsSameCurve (PathLocationDetail const &other) const;

static GEOMDLLIMPEXP bool AlmostEquallPoint (PathLocationDetailCR dataA, PathLocationDetailCR dataB);
static GEOMDLLIMPEXP bool AlmostEquallPointDistance (PathLocationDetailCR dataA, PathLocationDetailCR dataB);
static GEOMDLLIMPEXP bool AlmostEquallPointDistanceCurve (PathLocationDetailCR dataA, PathLocationDetailCR dataB);

//! Extract all the distances
static GEOMDLLIMPEXP void GetDistances (bvector<PathLocationDetail> const &locations, bvector<double> &distances);

//! Return a path location detail with (if possible) index that agrees with the pointer for a particular curvevector.
//! If the instance pointer and index match the curve vector, return it as is.
//! If not, but pointer can be found in the curve vector, and mark the returned PathLocationDetail valid.
//! If the pointer is not found in the curve vector, return the instance dtail marked not valid.
ValidatedPathLocationDetail CorrectIndex (CurveVectorCR path) const;
};

//! Two PathLocationDetail structures paired
struct PathLocationDetailPair
{
ptrdiff_t m_tagA;
ptrdiff_t m_tagB;
PathLocationDetail m_detailA;
PathLocationDetail m_detailB;

GEOMDLLIMPEXP PathLocationDetail DetailA () const;
GEOMDLLIMPEXP PathLocationDetail DetailB () const;
GEOMDLLIMPEXP PathLocationDetail Detail (size_t index) const;

GEOMDLLIMPEXP void SetDetailA (PathLocationDetail const &detail);
GEOMDLLIMPEXP void SetDetailB (PathLocationDetail const &detail);


GEOMDLLIMPEXP ptrdiff_t GetTagA () const;
GEOMDLLIMPEXP ptrdiff_t GetTagB () const;
GEOMDLLIMPEXP ptrdiff_t GetTag (size_t index) const;
GEOMDLLIMPEXP void SetTagA (ptrdiff_t tag);
GEOMDLLIMPEXP void SetTagB (ptrdiff_t tag);

GEOMDLLIMPEXP PathLocationDetailPair (PathLocationDetail const &detailA, PathLocationDetail const &detailB, ptrdiff_t tagA = 0, ptrdiff_t tagB = 0);

GEOMDLLIMPEXP PathLocationDetailPair ();

static GEOMDLLIMPEXP bool IsLessThanByPathDistance_lexicalAB (PathLocationDetailPair const &dataA, PathLocationDetailPair const &dataB);
static GEOMDLLIMPEXP bool IsLessThanByPathDistance_lexicalBA (PathLocationDetailPair const &dataA, PathLocationDetailPair const &dataB);

//! merge data from the DetailA and DetailB in pairs.
static GEOMDLLIMPEXP void Merge
(
bvector<PathLocationDetailPair> const &pairs,   //!< [in] pairs array of point pairs.
bvector<DPoint3d> *mergedXYZ,   //!< [out] mergedXYZ optional vector of points with xy from DetailA(), z from DetailB().
bvector<double>   *distanceA = nullptr    //!< [out] distanceA optional vector of distances.
);
};

//! Context for searching a path by "distance along"
//! Position along the path is described by a CurveLocationDetail.
//! The CurveLocationDetail fields that are used are:
//!   curve -- a primitive
//!   point -- xyz coordinates
//!   fraction -- fractional position within the primitive
//!   a -- total distance since beginning of the whole set of curves.
//! (i.e. the componentIndex and numComponent fields are not used.)
struct CurveVectorWithDistanceIndex : RefCountedBase
{
private:

struct PathEntry : PathLocationDetail
{
double             m_projectedDistance;
DRange3d           m_range;
PathEntry (PathLocationDetail const &pathDetail, double projectedDistance, DRange3dCR range)
    : PathLocationDetail (pathDetail),
    m_projectedDistance (projectedDistance),
    m_range(range)
    {
    }
PathEntry (double distance) : PathLocationDetail (distance), m_projectedDistance (distance) {}
// @param projected true for m_projectedDistance, false for m_pathDistance;    
double GetDistance (bool projected) const;    
//! comparison using ONLY the projected path distance.    
static GEOMDLLIMPEXP bool IsLessThan_ByPathDistanceXY (PathEntry const &dataA, PathEntry const &dataB);
static GEOMDLLIMPEXP bool IsLessThan_ByPathDistance   (PathEntry const &dataA, PathEntry const &dataB);
GEOMDLLIMPEXP PathLocationDetail GetPathLocationDetail () const;
GEOMDLLIMPEXP bool RangeIntersectsXY (PathEntry const &other) const;
};

CurveVectorCPtr m_curveVector; // must be referenced to be sure curvePrimitiveCP in detail data is valid.
bvector<PathEntry> m_locations;
CurveVectorPtr m_savedPrimitives; // Ptr's to extension primitives.

RotMatrix m_worldToViewRotMatrix;
RotMatrix m_viewToWorldRotMatrix;
RotMatrix m_flattenToView;          // Scale (1,0,0) * m_worldToViewRotMatrix;
DMatrix4d m_worldToView;

//! append a single primitive (and create 0-distance start locatiopn if needed)
bool AppendPrimitive (ICurvePrimitivePtr const &source);

//! recursively append primitives
void AppendPrimitives (CurveVectorCR source);

//! Clear all curve and index data.
void ClearCurves ();

//! initialize transforms (and ClearCurves())
void Init (RotMatrixCR worldToView);
//! Constructor for specified viewing direction.
//! Viewing direction is saved.
//! Follow by installing the path with SetPath
explicit CurveVectorWithDistanceIndex (RotMatrix worldToView);
//! Constructor for xy viewing.
//! Follow by installing the path with SetPath.
CurveVectorWithDistanceIndex ();

//! Return detailed curve location for the position at targetDistance along the curve -- optional flattening
bool SearchByDistanceFromPathStart
(
RotMatrixCP flatten,
double targetDistance,
PathLocationDetail &detail
) const;


// clone a fragment of an indexed curve.
// if non-null, push it to dest.
// (Create dest if needed)
void CloneAndAppenedPartialCurve(CurveVectorPtr &dest, int32_t locationIndex, double fraction0, double fraction1, bool allowExtrapolation) const;

ValidatedValue <PathLocationDetail> IndexedFractionToPathLocationDetail (size_t index, double fraction, bool computeDistance);

public:

//! Constructor for specified viewing direction.
//! Viewing direction is saved.
//! Follow by installing the path with SetPath
static GEOMDLLIMPEXP CurveVectorWithDistanceIndexPtr Create (RotMatrix worldToView);
//! Constructor for xy viewing.
//! Follow by installing the path with SetPath.
static GEOMDLLIMPEXP CurveVectorWithDistanceIndexPtr Create ();

//! Return true if there are no curves.
GEOMDLLIMPEXP bool IsEmpty () const;


//! Announce the path to be indexed.
//! This will:
//! 1) save a pointer to the path.
//! 2) create an index of <curve,distance> to support fast search and navigation by distance.
GEOMDLLIMPEXP void SetPath (CurveVectorPtr &path);
GEOMDLLIMPEXP void SetPath (CurveVectorCR path);
//! Get the CurveVector being used (possbily a clone or modification of what sent to SetPath)
GEOMDLLIMPEXP CurveVectorCPtr GetCurveVector () const;

//! Announce the path to be indexed.
//! This will
//! 1) Construct line segments to extend in the directions of the start and end tangents.
//! 2) create the <curve,distance> index
//! 3) return PositionLocationDetails for the limits of the bounded path.
GEOMDLLIMPEXP bool SetExtendedPath
(
CurveVectorPtr &path,       //!< [in] path to save
double extensionDistance,   //!< [in] distance to extend
PathLocationDetail &boundedStart, //!< [out] indexed start position for the unextended curve
PathLocationDetail &boundedEnd,    //!< [out] indexed end position for the unextendded curve
bool measureExtensionInView = false,//!< [in] true to have extension measured in the projected plane.
double maxExtensionFactor = 4.0     //!< [in] multiplier giving the longest true extension due to projection.
);
//! Return complete path length.
GEOMDLLIMPEXP double TotalPathLength () const;
// xy methods are deprecated.  they can be kept as internal by choice of macro here...
#define GEOMDLLIMPEXP_XY GEOMDLLIMPEXP
//#define GEOMDLLIMPEXP_XY
//! Return complete path length as flattened into the view.
GEOMDLLIMPEXP_XY double TotalPathLengthXY () const;

//! Return the start location.
GEOMDLLIMPEXP PathLocationDetail AtStart () const;
//! Return the final location.
GEOMDLLIMPEXP PathLocationDetail AtEnd () const;

//! projected distance between points
GEOMDLLIMPEXP_XY double DistanceBetweenPointsXY (DPoint3dCR xyzA, DPoint3dCR xyzB) const;

    
//! Return detailed curve location for the position at targetDistance along the curve.
GEOMDLLIMPEXP bool SearchByDistanceFromPathStart
(
double targetDistance,
PathLocationDetail &detail
) const;

//! Return a ray with point on the curve, unit vector in curve tangent direction.
//!<ul>
//!<li>If extrapolate is false, points out of range return with the end point and tangent marked invalid.
//!<li>If extrapolate is true, poitns out of range return as linear extrapolation of the end point.
//!</ul>
GEOMDLLIMPEXP ValidatedDRay3d DistanceAlongToPointAndUnitTangent
(
double distanceAlong,       //!< [in] distance along path computed ponit.
bool extrapolate = true     //!< [in] true to allow extropolation of end tangent.
) const;

//! Return detailed curve location for the position at targetDistance along the curve, measuring in path distance.
//! WARNING: The distance entry in the PositionLocationDetail is true distance (with z variation)
GEOMDLLIMPEXP_XY bool SearchByDistanceFromPathStartXY
(
double targetDistanceXY,
PathLocationDetail &detail
) const;

//! Search for curve point closest to given space point.
GEOMDLLIMPEXP PathLocationDetail SearchClosestPointBounded
(
DPoint3dCR spacePoint,
bool computeDistanceAlong
) const;

//! Start at specified location.
//! Make a circle of given radius.
//! Look for the "first" intersection of the circle with the path, moving only forward if the radius is positive and only backward if negative.
GEOMDLLIMPEXP_XY bool SearchFirstIntersectionWithCircleXY
(
PathLocationDetail const &startLocation,
double signedRadius,
PathLocationDetail &intersectionLocation
) const;

//! Given a path location, find the projected curve distance from the path start.
GEOMDLLIMPEXP_XY bool DistanceXYFromPathStart
(
PathLocationDetail const &detail,   //! @param[in] detail that ties to a curve within the path.
double &distance                    //! @param [out] distance along the projected curve.
) const;

//! Clone a subset of the curve.
GEOMDLLIMPEXP CurveVectorPtr CloneBetweenPathLocations (PathLocationDetail const &location0, PathLocationDetail const &location1) const;

//! Get start and end points for all breaks between locations.
GEOMDLLIMPEXP_XY void GetBreakPointsBetweenDistancesXY (double distanceA, double distanceB, bvector<PathLocationDetail> &locations) const;

//! Clone a subset of the curve.
GEOMDLLIMPEXP CurveVectorPtr CloneBetweenDistances (double distance0, double distance1) const;
//! Clone a subset of the curve, measuring only in xy.
GEOMDLLIMPEXP_XY CurveVectorPtr CloneBetweenDistancesXY (double distance0, double distance1) const;

//! Get a complete list of important points along the curve, with distance data.
GEOMDLLIMPEXP void GetBreakPoints (bvector<PathLocationDetail> &locations, bool replicateEndStart);

//! Classify overlapping and disjoint subsets.
//! Paths are assumed to be reasonably correlated -- common sections will move forward on both, then separate and rejoin moving forward again.
//! interval pairs have GetTagA values indicating overlap (0==> disjoing, 1==>overlap)
static GEOMDLLIMPEXP void FindCommonSubPaths (
CurveVectorWithDistanceIndex    &pathA,             //!< [in] first path
CurveVectorWithDistanceIndex    &pathB,             //!< [in] second path
bvector<PathLocationDetailPair> &pathAIntervals,    //!< [out]  intervals of pathA
bvector<PathLocationDetailPair> &pathBIntervals,     //!< [out]  corresponding intervals of pathB
bool                            includeGaps,         //!< [in]  true to create gap intervals between common parts
bool                            compressIntervals    //!< [in]  true to return only start end of common paths -- no internal break locations (e.g. between curves)
);

//! Compute strokes along the curves.
GEOMDLLIMPEXP void Stroke
(
bvector<PathLocationDetail> &locations, //!< [out] stroked locations
IFacetOptionsCR options                 //!< [in] density controls
) const;

//==================================================================================
// Special methods that are valid only if the map has monontone X as its sort distance.
GEOMDLLIMPEXP bool SearchByElevationMapXPlane
(
double targetDistance,
PathLocationDetail &data
) const;


//! Create a stroked, heavily annotated points.
//! In each PathLocationDetail in the locations vector, 
//! <pre>
//! <ul>
//! <li> DetailA () (point, curve pointer, fractional data, and distance) is from the xyCurve
//! <li> DetailB () (point, curve pointer, fractional data, and idstance) is from the zCurve
//! <li> distances in DetailA and DetailB match.
//! </ul>
//! </pre>
static GEOMDLLIMPEXP bool StrokeHorizontalAndVerticalCurves
(
IFacetOptionsCR xyOptions,  //!< [in] required options for xy stroke.
IFacetOptionsCR zOptions,   //!< [in] optional options for z stroke.
CurveVectorWithDistanceIndex  &xyCurve, //!< [in] space curve, typically with z identically 0, indexed by distance along curve
CurveVectorWithDistanceIndex  &zCurve,  //!< [in] z curve, with x being distance along curve and elevation in z.
bvector<PathLocationDetailPair> &locations  //!> [out] complete details of locations on xyCurve, zCurve
);

//! Create a stroked, heavily annotated points.
//! In each PathLocationDetail in the locations vector, 
//! <pre>
//! <ul>
//! <li> DetailA () (point, curve pointer, fractional data, and distance) is from the xyCurve
//! <li> DetailB () (point, curve pointer, fractional data, and idstance) is from the zCurve
//! <li> distances in DetailA and DetailB match.
//! </ul>
//! </pre>
static GEOMDLLIMPEXP bool StrokeHorizontalAndVerticalCurves
(
IFacetOptionsCR xyOptions,  //!< [in] required options for xy stroke.
IFacetOptionsCR zOptions,   //!< [in] optional options for z stroke.
CurveVectorWithDistanceIndex  &xyCurve, //!< [in] space curve, typically with z identically 0, indexed by distance along curve
CurveVectorWithXIndex &zCurve,  //!< [in] z curve, with x being distance along curve and elevation in z.
bvector<PathLocationDetailPair> &locations  //!> [out] complete details of locations on xyCurve, zCurve
);

};



//! Fast search structure for a curve vector whose x coordinate is distance along a vertical alignment, and
//! z is the elevation. 
struct CurveVectorWithXIndex : RefCountedBase
{
private:
    CurveVectorCPtr m_curves;
    bvector<PathLocationDetail> m_locations;
    CurveVectorWithXIndex (CurveVectorCR curve);

bool AddIndexEntry (size_t index, double fraction);

//! Build the x-direction index (using the curve vector given to the constructor).
bool BuildIndex ();

public:
//! Create a searchable vertical alignment from its curves.
//! Returns nullptr if the curves are not monotone in x.
static GEOMDLLIMPEXP CurveVectorWithXIndexPtr Create (CurveVectorR curves);

//! Detailed data for start of the curve.
GEOMDLLIMPEXP ValidatedPathLocationDetail AtStart (bool valid) const;
//! Detailed data for end of the curve.
GEOMDLLIMPEXP ValidatedPathLocationDetail AtEnd (bool valid) const;
//! Search the curve for the fragment at given x value.
GEOMDLLIMPEXP ValidatedPathLocationDetail XToPathLocationDetail (double x) const;
GEOMDLLIMPEXP CurveVectorCPtr GetCurveVectorPtr () const;

//! Convert fractional position to x coordinate.
GEOMDLLIMPEXP double FractionToX (double fraction) const;
//! Convert x coordinate to fractinoal position.
GEOMDLLIMPEXP double XToFraction (double x) const;
//! Query the range of the x search index.
GEOMDLLIMPEXP DRange1d XRange () const;
//! Query the starting x coordinate. (return 0 if no curves)
GEOMDLLIMPEXP double GetStartX () const;
//! Query the final x coordinate. (return 0 if no curves)
GEOMDLLIMPEXP double GetEndX () const;

//! Extract a subcurve by x limits . . 
GEOMDLLIMPEXP CurveVectorPtr CloneDirectedXInterval (double xStart, double xEnd) const;
//! Extract a subcurve by detailed locations . . 
GEOMDLLIMPEXP CurveVectorPtr CloneDirectedInterval (PathLocationDetailCR detailA, PathLocationDetailCR detailB) const;

};






END_BENTLEY_GEOMETRY_NAMESPACE

#ifdef BENTLEY_WIN32
#pragma warning (pop)
#endif
