/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once



/*__PUBLISH_SECTION_START__*/
#ifdef BENTLEY_WIN32
#pragma warning (push)
#pragma warning (default : 4266)	// NEEDS WORK -- warnings about missing virtuals?
#endif
BEGIN_BENTLEY_GEOMETRY_NAMESPACE




/**
An ICurvePrimitive is a refcounted structure.  At point of creation, an instance is addressed via a ICurvePrimitivePtr.   Inputs to methods can be passed as simple references and pointers.

<h3> Primitive type and queries</h3>
 
 Each curve vector is marked with an enumerated value indicating how its contents are to be interpretted.  The enumerated type can be accessed via cv.GetCurvePrimitiveType ().
 
<TABLE BORDER="1">
 <TR><TD>enum name                                 </TD> <TD> represents   </TD> <TD> Remarks </TD> </TR>
 <TR><TD> CURVE_PRIMITIVE_TYPE_Invalid             </TD> <TD> error type.  Exists only at point of construction </TD> <TD> </TD> </TR>
 <TR><TD> CURVE_PRIMITIVE_TYPE_Line                </TD> <TD> line segment    </TD> <TD> Detail data is DSegment3d</TD> </TR>
 <TR><TD> CURVE_PRIMITIVE_TYPE_LineString          </TD> <TD> polyline   </TD> <TD> Detail data is bvector<DPoint3d> </TD> </TR>
 <TR><TD> CURVE_PRIMITIVE_TYPE_Arc                 </TD> <TD> elliptic arc   </TD> <TD> Detail data is a DEllipse3d </TD> </TR>
 <TR><TD> CURVE_PRIMITIVE_TYPE_BsplineCurve        </TD> <TD> bspline curve  </TD> <TD> Detail data is MSBsplienCurve </TD> </TR>
 <TR><TD> CURVE_PRIMITIVE_TYPE_InterpolationCurve  </TD> <TD> bspline curve with original interpolation points </TD> <TD> </TD> </TR>
 <TR><TD> CURVE_PRIMITIVE_TYPE_AkimaCurve          </TD> <TD> C1 curve through points. </TD> <TD>  This is a 1970s curve type with very poor smoothness properties.   </TD> </TR>
 <TR><TD> CURVE_PRIMITIVE_TYPE_PointString         </TD> <TD> unconnected points </TD> <TD>    </TD> </TR>
 <TR><TD> CURVE_PRIMITIVE_TYPE_CurveVector         </TD> <TD> placeholder for CurveVector </TD> <TD>  </TD> </TR>
 <TR><TD> CURVE_PRIMITIVE_TYPE_Spiral              </TD> <TD> Clothoid or other plane spiral section </TD> <TD>    </TD> </TR>
 <TR><TD> CURVE_PRIMITIVE_TYPE_PartialCurve        </TD> <TD> fractional portion of another curve </TD> <TD>   </TD> </TR>
 <TR><TD> CURVE_PRIMITIVE_TYPE_NotClassified       </TD> <TD>  </TD> <TD>  </TD> </TR>
 </TABLE>
 <h3> Fractional position along a primitive</h3>

 Every primitive (except CURVE_PRIMITIVE_TYPE_CurveVector and CURVE_PRIMITIVE_TYPE_PointString) supports a "fractional" parametric form.
 
 <ul>
 <li>The fractional coordinate 0.0 is always the start point.
 <li>The fractional coordinate 1.0 is always the end point.
 <li>increasing fractions "from 0 towards 1" moves forward along the curve.
 <li>methods    curve->FractionToPoint (fraction, xyz ...)   can be freely called for fractions between 0 and 1.
 <li>fractions outside of 0 to 1 may be treated as errors if the equations for the primitive do not allow extended evaluation.
 <li>the mapping from fraction to point is determined by the equations of the curve.
     <ul>
     <li>fraction changes are not required to be proportional to distance change.
     <li>fractional changes ARE proportional to distance change for a limited subset of curve primtivies.  Specifically, these types are 
         <ul>
         <li>line segment
         <li>circular arcs
         <li>transition spirals
         </ul>
     <li>fractional changes are NOT typically proportional to distance change for other types:
         <ul>
         <li>LineString
         <li>elliptic (non-circular)arcs
         <li>bspline curve
         <li>interpolation curves
         <li>partial curves
         </ul>
     </ul>
<li>A linestring with (N+1) points (i.e. N edges) has this parameterization
    <ul>
    <li>fraction 0 is the start point
    <li>fraction (1/N), (2/N) through ((N-1)/N) are internal point
    <li>fraction 1 is the final point.
    <li> within each edge, there is a "componentFraction" that various from 0 to 1.
    <li> within edge k (k ranging from 0 through N-1 inclusive) the global fraction for componentFration within the edge is (k + componentFraction)/N
    <li> When a position "along a linestring" is captured in a CurveLocationDetail, the detail carries both
        <ul>
        <li>fraction = the global fraction
        <li>(componentIndex, componentFraction) to identify the particular edge and fractional position along the edge
        </ul>
    </ul>
 <li> To convert between fractional position and distance along, use these methods:
    <ul>
    <li>curvePrimitive.PointAtSignedDistanceFromFraction (startFraction, signedDistance, allowExtension, [out]curveLocationDetail) --- starts at startFraction and moves forward or backwards by given distance.
    <li>curvePrimitive.SignedDistanceBetweenFractions (startFraction, endFraction, [out]distance) -- returns distance "along the curve" between two fractional positions.
    </ul>
 <li> To make queries based on "distance along" within a CurveVector with multiple primitives, bundle the CurveVector into a CurveVectorWithDistanceIndex.
 <li> Question: Why aren't distances used everywhere?
     <ul>
     <li>Answer 1: For many types, computing with true distance along is <em>significantly</em> more expensive than computations involving the natural parameterization of the curve type.
     <li>Answer 2: The "0 to 1" fraction convention gives absolute certainty of what to pass to ask for start and end, without needing to check the length of the instance.
     </ul>
 </ul>
 */
struct ICurvePrimitive :  public RefCountedBase
{
//__PUBLISH_SECTION_END__
private:

    uint32_t                            m_markerBits;
    ICurvePrimitiveInfoPtr              m_info;
    mutable CurvePrimitiveIdPtr         m_id;
    int64_t                             m_tag;
    

//__PUBLISH_SECTION_START__
public:
//! Enumeration of possible curve primitive types.
enum CurvePrimitiveType
    {
    CURVE_PRIMITIVE_TYPE_Invalid            =  0,
    CURVE_PRIMITIVE_TYPE_Line               =  1,
    CURVE_PRIMITIVE_TYPE_LineString         =  2,
    CURVE_PRIMITIVE_TYPE_Arc                =  3,
    CURVE_PRIMITIVE_TYPE_BsplineCurve       =  4,
    CURVE_PRIMITIVE_TYPE_InterpolationCurve =  5,
    CURVE_PRIMITIVE_TYPE_AkimaCurve         =  6,
    CURVE_PRIMITIVE_TYPE_PointString        =  7,
    CURVE_PRIMITIVE_TYPE_CurveVector        =  8,
    CURVE_PRIMITIVE_TYPE_Spiral             =  9,
    CURVE_PRIMITIVE_TYPE_PartialCurve       = 10,
    CURVE_PRIMITIVE_TYPE_Catenary           = 11,
    CURVE_PRIMITIVE_TYPE_TrackingCurve  = 12,
    CURVE_PRIMITIVE_TYPE_NotClassified      = -1,
    };


//flex!! Marker bits
//flex
//flex Each primitive has a bit mask field (32 bits).  The low order 16 bits are available for arbitrary use by applications.  The high 16 bits are reserved for library use.
//flex!!!!Marker mask names
//flex The CurvePrimitiveMarkerBit enumeration indicates these uses:
//flex
//flex || Mask name (enumerated type)            || Means ||
//flex || CURVE_PRIMITIVE_BIT_GapCurve           || Line segment created to fill a gap. ||
//flex || CURVE_PRIMITIVE_BIT_AllApplicationBits || bits available for application use. ||
//flex!!!!Marker queries
//flex || Query a bit || bool curvePrim.GetMarkerBit (bitSelector) ||
//flex || Set a bit    || curvePrim.SetMarkerBit () ||



//! Bit mask for access to selective parts of an integer tag in each curve primitive.
enum CurvePrimitiveMarkerBit
    {
    CURVE_PRIMITIVE_BIT_GapCurve = 0x00010000,
    CURVE_PRIMITIVE_BIT_AllApplicationBits = 0x0000FFFF,
    CURVE_PRIMITIVE_BIT_ApplicationBit0 = 0x00000001,
    CURVE_PRIMITIVE_BIT_ApplicationBit1 = (0x00000001 << 1),
    CURVE_PRIMITIVE_BIT_ApplicationBit2 = (0x00000001 << 2),
    CURVE_PRIMITIVE_BIT_ApplicationBit3 = (0x00000001 << 3),
    CURVE_PRIMITIVE_BIT_ApplicationBit4 = (0x00000001 << 4),
    CURVE_PRIMITIVE_BIT_ApplicationBit5 = (0x00000001 << 5),
    CURVE_PRIMITIVE_BIT_ApplicationBit6 = (0x00000001 << 6),
    CURVE_PRIMITIVE_BIT_ApplicationBit7 = (0x00000001 << 7),
    };

//! Ask if specified marker bit is on or off
GEOMDLLIMPEXP bool GetMarkerBit (CurvePrimitiveMarkerBit selector) const;
//! Set the specified marker bit on or off
GEOMDLLIMPEXP void SetMarkerBit (CurvePrimitiveMarkerBit selector, bool value);


//flex!! Int64 tag
//flex
//flex Each curve primitive has in Int64 tag.
//flex The access functions have int variants, but the tag itself is always 64 bits.
//! Get the Int64 tag ...
GEOMDLLIMPEXP int64_t GetTag () const;
//! Get the Int64 tag (cast to int)
GEOMDLLIMPEXP int GetIntTag () const;
//! Set the Int64 tag
GEOMDLLIMPEXP void SetTag (int64_t tag);
//! Set the Int64 tag from an int
GEOMDLLIMPEXP void SetTag (int tag);

//__PUBLISH_SECTION_END__
GEOMDLLIMPEXP ICurvePrimitiveInfoPtr const& GetCurvePrimitiveInfo () const;
GEOMDLLIMPEXP ICurvePrimitiveInfoPtr& GetCurvePrimitiveInfoW ();
// Access the CurvePrimitiveInfo AND cast to special type FacetEdgeLocationDetailVectorPtr.
GEOMDLLIMPEXP FacetEdgeLocationDetailVectorPtr GetFacetEdgeLocationDetailVectorPtr () const;
GEOMDLLIMPEXP void SetCurvePrimitiveInfo (ICurvePrimitiveInfoPtr data);

/*__PUBLISH_SECTION_START__*/
GEOMDLLIMPEXP CurvePrimitiveIdCP  GetId() const;

GEOMDLLIMPEXP void                SetId(CurvePrimitiveIdP id) const;

GEOMDLLIMPEXP CurveVectorPtr GetChildCurveVectorP ();

protected:

ICurvePrimitive (); // constructor accessed from derived classes to clear marker bits.
// Final steps (marker bit transfer)
void FinishClone (ICurvePrimitiveCR parent);

//! Return a copy.
GEOMAPI_VIRTUAL ICurvePrimitivePtr _Clone () const = 0; // Base class can't help.
//! Return a copy.
GEOMAPI_VIRTUAL ICurvePrimitivePtr _CloneComponent (ptrdiff_t componentIndex) const;     // Base class invokes full _Clone.  Only linestring overrides.

//! BASE CLASS APPLIES FRACTION LIMITS -- IMPLEMENTATIONS CAN IGNORE allowExtension
GEOMAPI_VIRTUAL ICurvePrimitivePtr _CloneBetweenFractions (double fractionA, double FractionB, bool allowExtrapolation) const = 0; // Base class can't help.

GEOMAPI_VIRTUAL ICurvePrimitivePtr _CloneAsSingleOffsetPrimitiveXY (CurveOffsetOptionsCR options) const;

GEOMAPI_VIRTUAL CurvePrimitiveType        _GetCurvePrimitiveType () const;
GEOMAPI_VIRTUAL DSegment3dCP              _GetLineCP () const;
GEOMAPI_VIRTUAL bvector<DPoint3d> const*  _GetLineStringCP () const;
GEOMAPI_VIRTUAL bvector<DPoint3d>*        _GetLineStringP ();
GEOMAPI_VIRTUAL DEllipse3dCP              _GetArcCP () const;
GEOMAPI_VIRTUAL MSBsplineCurveCP          _GetBsplineCurveCP () const;
GEOMAPI_VIRTUAL MSBsplineCurvePtr         _GetBsplineCurvePtr () const;
GEOMAPI_VIRTUAL MSBsplineCurveCP          _GetProxyBsplineCurveCP () const;
GEOMAPI_VIRTUAL MSBsplineCurvePtr         _GetProxyBsplineCurvePtr () const;
GEOMAPI_VIRTUAL MSInterpolationCurveCP    _GetInterpolationCurveCP () const;
GEOMAPI_VIRTUAL bvector<DPoint3d> const*  _GetAkimaCurveCP () const;
GEOMAPI_VIRTUAL bvector<DPoint3d> const*  _GetPointStringCP () const;
GEOMAPI_VIRTUAL CurveVector const*        _GetChildCurveVectorCP () const;
GEOMAPI_VIRTUAL CurveVectorPtr            _GetChildCurveVectorP ();

GEOMAPI_VIRTUAL DSpiral2dPlacementCP      _GetSpiralPlacementCP () const;
GEOMAPI_VIRTUAL bool _TryGetCatenary (DCatenary3dPlacementR) const;
GEOMAPI_VIRTUAL PartialCurveDetailCP      _GetPartialCurveDetailCP () const;
GEOMAPI_VIRTUAL bool                      _TryGetPartialCurveData (double &fractionA, double &fractionB, ICurvePrimitivePtr &parent, int64_t &tag) const;
GEOMAPI_VIRTUAL bool _FractionToPoint (double f, DPoint3dR point) const;
GEOMAPI_VIRTUAL bool _ComponentFractionToPoint (ptrdiff_t componentIndex, double f, DPoint3dR point) const;
GEOMAPI_VIRTUAL bool _FractionToPoint (double f, DPoint3dR point, DVec3dR tangent) const;
GEOMAPI_VIRTUAL bool _FractionToPoint (double f, CurveLocationDetail &point) const;

GEOMAPI_VIRTUAL bool _ComponentFractionToPoint (ptrdiff_t componentIndex, double f, DPoint3dR point, DVec3dR tangent) const;
GEOMAPI_VIRTUAL bool _FractionToPoint (double f, DPoint3dR point, DVec3dR tangent, DVec3dR derivative2) const;
GEOMAPI_VIRTUAL bool _FractionToPoint (double f, DPoint3dR point, DVec3dR tangent, DVec3dR derivative2, DVec3dR derivative3) const;
GEOMAPI_VIRTUAL bool _FractionToPointWithTwoSidedDerivative (double f, DPoint3dR point, DVec3dR tangentA, DVec3dR tangentB) const;

GEOMAPI_VIRTUAL bool _FractionToFrenetFrame (double f, TransformR frame) const;
GEOMAPI_VIRTUAL bool _FractionToFrenetFrame (double f, TransformR frame, double &curvature, double &torsion) const;
GEOMAPI_VIRTUAL bool _Length (double &length) const;
GEOMAPI_VIRTUAL bool _Length (RotMatrixCP worldToLocal, double &length) const;
GEOMAPI_VIRTUAL bool _FastLength (double &length) const;
GEOMAPI_VIRTUAL bool _GetRange (DRange3dR range) const;
GEOMAPI_VIRTUAL bool _GetRange (DRange3dR range, TransformCR transform) const;
GEOMAPI_VIRTUAL DRange1d _ProjectedParameterRange (DRay3dCR ray, double fraction0, double fraction1) const;
GEOMAPI_VIRTUAL DRange1d _ProjectedParameterRange (DRay3dCR ray) const;
GEOMAPI_VIRTUAL double _FastMaxAbs () const;
GEOMAPI_VIRTUAL bool _IsSameStructureAndGeometry (ICurvePrimitiveCR other, double tolerance) const = 0;
GEOMAPI_VIRTUAL size_t _NumComponent () const = 0;
GEOMAPI_VIRTUAL bool _GetBreakFraction (size_t breakFractioniIndex, double &fraction) const;
GEOMAPI_VIRTUAL bool _AdjustFractionToBreakFraction (double fraction, Rounding::RoundingMode mode, size_t &breakIndex, double &adjustedFraction) const;
GEOMAPI_VIRTUAL bool _GetMSBsplineCurve (MSBsplineCurveR curve, double fractionA, double fractionB) const;
GEOMAPI_VIRTUAL bool _SignedDistanceBetweenFractions (double startFraction, double endFraction, double &signedDistance) const;
GEOMAPI_VIRTUAL bool _SignedDistanceBetweenFractions (RotMatrixCP worldToLocal, double startFraction, double endFraction, double &signedDistance) const;
// REMARK -- The nonvirtual dispatcher enforces clamping conditions.   Subclasses should only implement extension
//   logic if there is significant performance benefit.
GEOMAPI_VIRTUAL bool _PointAtSignedDistanceFromFraction (double startFraction, double signedLength, bool allowExtension, CurveLocationDetailR location) const;
GEOMAPI_VIRTUAL bool _PointAtSignedDistanceFromFraction (RotMatrixCP worldToLocal, double startFraction, double signedLength, bool allowExtension, CurveLocationDetailR location) const;
GEOMAPI_VIRTUAL bool _ClosestPointBounded (DPoint3dCR spacePoint, double &fraction, DPoint3dR curvePoint, bool extend0, bool extend1) const;
GEOMAPI_VIRTUAL bool _ClosestPointBounded (DPoint3dCR spacePoint, CurveLocationDetailR location, bool extend0, bool extend1) const;
GEOMAPI_VIRTUAL bool _ClosestPointBoundedXY (DPoint3dCR spacePoint, DMatrix4dCP worldToLocal, CurveLocationDetailR location, bool extend0, bool extend1) const;
// REMARK:  The non-GEOMAPI_VIRTUAL dispatcher queries and sends start and end points as needed.
// Primitives implement perp, tan, breakpoints.
GEOMAPI_VIRTUAL void _AnnounceKeyPoints
(
DPoint3dCR spacePoint,                       //!< [in] point to project
CurveKeyPointCollector &collector,           //!< [in] object to receive keypoint announcements
bool extend0,                               //!< true to extend at start of primitives.
bool extend1                                //!< true to extend at end of primitives.
) const;

GEOMAPI_VIRTUAL bool _GetStartEnd (DPoint3dR pointA, DPoint3dR pointB) const;
GEOMAPI_VIRTUAL bool _GetStartEnd (DPoint3dR pointA, DPoint3dR pointB, DVec3dR unitTangentA, DVec3dR unitTangentB) const;
GEOMAPI_VIRTUAL bool _TrySetStart (DPoint3dCR xyz);
GEOMAPI_VIRTUAL bool _TrySetEnd   (DPoint3dCR xyz);
GEOMAPI_VIRTUAL bool _IsExtensibleFractionSpace () const;
GEOMAPI_VIRTUAL bool _IsMappableFractionSpace () const;
GEOMAPI_VIRTUAL bool _IsFractionSpace () const;
GEOMAPI_VIRTUAL bool _IsPeriodicFractionSpace (double &period) const;
GEOMAPI_VIRTUAL bool _TransformInPlace (TransformCR transform) = 0;
GEOMAPI_VIRTUAL bool _ReverseCurvesInPlace () = 0;
GEOMAPI_VIRTUAL void  _AppendCurvePlaneIntersections (DPlane3dCR plane, bvector<CurveLocationDetailPair> &intersections, double tolerance = 0.0) const;
GEOMAPI_VIRTUAL void  _AppendCurveRangeIntersections (LocalRangeCR range, bvector<PartialCurveDetail> &intersections) const;
GEOMAPI_VIRTUAL void  _AppendCurveBilinearPatchIntersections (DBilinearPatch3dCR range, bvector<CurveAndSolidLocationDetail> &intersections) const;
GEOMAPI_VIRTUAL void  _AppendCurvePlaneIntersections (DPoint3dDVec3dDVec3dCR triangle, UVBoundarySelect bounded, bvector<CurveAndSolidLocationDetail> &intersections) const;
GEOMAPI_VIRTUAL bool _WireCentroid (double &length, DPoint3dR centroid, double fraction0, double fraction1) const;
GEOMAPI_VIRTUAL bool _AddStrokes (bvector <DPoint3d> &points, IFacetOptionsCR options, bool includeStartPoint, double startFraction, double endFraction) const;
GEOMAPI_VIRTUAL bool _AddStrokes (bvector <PathLocationDetail> &points, IFacetOptionsCR options, double startFraction, double endFraction) const;
GEOMAPI_VIRTUAL bool _AddStrokes (DPoint3dDoubleUVCurveArrays &strokes, IFacetOptionsCR options, double startFraction, double endFraction) const;

GEOMAPI_VIRTUAL size_t _GetStrokeCount (IFacetOptionsCR options, double startFraction, double endFraction) const;

GEOMAPI_VIRTUAL bool _TryGetTrackingCurveData (ICurvePrimitivePtr &parent, RotMatrixR matrix, int64_t &tag) const;


// shared implementation, allows nullptr for worldToLocal
bool PointAtSignedDistanceFromFraction_go (RotMatrixCP worldToLocal, double startFraction, double signedDistance, bool allowExtension, CurveLocationDetailR location) const;


public:

GEOMAPI_VIRTUAL void _Process (struct ICurvePrimitiveProcessor & processor, DSegment1dCP interval) const = 0;
// Dispatch the curve to a detail processor.
GEOMDLLIMPEXP void Process (struct ICurvePrimitiveProcessor & processor) const;

friend struct CurveVector;
friend struct PolyfaceQuery;
friend struct SectionGraph;
//! If this is a linestring primitive, return pointer to its coordinate data array.  Otherwise return null.
//! This is kept in the nonpublished area and shared with CurveVector.
GEOMDLLIMPEXP bvector<DPoint3d> * GetLineStringP ();

public:

//! Return the integer type code for the curve primitive type.
GEOMDLLIMPEXP CurvePrimitiveType           GetCurvePrimitiveType () const;


//flex !!!! Type-specific content queries
//flex|| Return a primitives type || primitiveType = curvePrim.GetCurvePrimitiveType ()   ||
//flex|| Ask if this is a line, and if so return the coordinates || bool curvePrim.TryGetLine (outSegment)  ||
//flex|| Ask if this is an arc, and if so return the coordinates || bool curvePrim.TryGetArc (outArc) ||
//flex|| Ask if this is a linestring, and if so return a pointer to its points || bvector<DPoint3d> const * points = curvePrim.GetLineStringCP () ||
//flex|| Ask if this is a bspline curve, and if so return a pointer to its details || MSBsplineCurveCP curve = curvePrim.GetBsplineCurveCP () ||
//flex|| Ask if this is an interpolating curve, and if so return a pointer to its details || MSInterpolationCurveCP curve = curvePrim.GetInterpolationCurveCP () ||
//flex|| Ask if this is a curve type that has a proxy bspline curve || MSBsplineCurveCP curve = curvePrim.GetProxyBsplineCurveCP () ||
//flex|| || MSBsplineCurveCP curve = curvePrim.GetProxyBsplineCurvePtr () ||
//flex|| Ask if this is an Akima curve, and if so return a pointer to its points || bvector<DPoint3d> const * points = curvePrim.GetAkimaCurveCP () ||
//flex|| Ask if this is a point string, and if so return a pointer to its points || bvector<DPoint3d> const * points = curvePrim.GetPointStringCP () ||
//flex|| Ask if this is an placeholder for a CurveVector || CurveVectorCP curves = curvePrim.GetChildCurveVectorCP () ||
//flex|| Ask if this is a spiral curve, and if so return a pointer to its details || DSpiral2dPlacementCP curve = curvePrim.GetSpiralPlaceemntCP () ||
//flex|| copy a fraction portion of (any type of) primitive into an MSBsplineCurve || bool curvePrim.GetMSBsplineCurve (outCurve, fraction0, fraction1) ||

//! If this is a line primitive with zero length, copy its coordinate to a DPoint3d.
GEOMDLLIMPEXP bool TryGetPoint (DPoint3dR xyz) const;

//! If this is a line primitive, copy its coordinate data to a DSegment3d.
GEOMDLLIMPEXP bool TryGetLine (DSegment3dR segment) const;
//! If this is an arc primitive, copy its coordinate data to a DEllipse3d.
GEOMDLLIMPEXP bool TryGetArc  (DEllipse3dR arc) const;

//! If this is a catenary primitive, copy its coordinate data to a DCatenary3dPlacement
GEOMDLLIMPEXP bool TryGetCatenary  (DCatenary3dPlacementR arc) const;
//! If this is a mapped curve, return its data.
GEOMDLLIMPEXP bool TryGetTrackingCurveData  (ICurvePrimitivePtr &parent, RotMatrixR matrix, int64_t &tag) const;


//! If this is a line primitive, return const pointer to its coordinate data.  Otherwise return NULL.
GEOMDLLIMPEXP DSegment3dCP                 GetLineCP () const;
//! If this is a linestring primitive, return const pointer to its coordinate data array.  Otherwise return null.
GEOMDLLIMPEXP bvector<DPoint3d> const* GetLineStringCP () const;
//! If this is a linestring and index is valid, get the (single) indexed segment.
GEOMDLLIMPEXP bool TryGetSegmentInLineString (DSegment3dR segment, size_t startPointIndex) const;

//! If this is a linestring, add a point.
GEOMDLLIMPEXP bool TryAddLineStringPoint (DPoint3dCR xyz);

//! If this is an arc primitive, return const pointer to its coordinate data.  Otherwise return NULL.
GEOMDLLIMPEXP DEllipse3dCP                 GetArcCP () const;
//! If this is a bspline primtiive, return const pointer to its curve.  Otherwise return NULL.
GEOMDLLIMPEXP MSBsplineCurveCP             GetBsplineCurveCP () const;
//! If this is a bspline primtiive, return ref counted pointer to its curve.  Otherwise return NULL.
GEOMDLLIMPEXP MSBsplineCurvePtr             GetBsplineCurvePtr () const;
//! If available, return a representative bspline curve.
GEOMDLLIMPEXP MSBsplineCurveCP             GetProxyBsplineCurveCP () const;
//! If available, return a(refcounted pointer to) a proxy bspline curve.
GEOMDLLIMPEXP MSBsplineCurvePtr            GetProxyBsplineCurvePtr () const;
//! If this is an Interpolation curve, return const pointer to its coordinate data.   Otherwise return NULL.
GEOMDLLIMPEXP MSInterpolationCurveCP       GetInterpolationCurveCP () const;
//! If this is an Akima curve, return const pointer to its array of fit points (with extra points at each end to control slope).
GEOMDLLIMPEXP bvector<DPoint3d> const* GetAkimaCurveCP () const;
//! If this is a point string, return const pointer to its point array.  Otherwise return NULL.
GEOMDLLIMPEXP bvector<DPoint3d> const* GetPointStringCP () const;
//! If this is a carrier for a curve vector, return const pointer to the vector.  Otherwise return NULL.
GEOMDLLIMPEXP CurveVectorCP            GetChildCurveVectorCP () const;
//! If this is s a spiral placement return const pointer to its coordinate data.  Otherwise return NULL.
GEOMDLLIMPEXP DSpiral2dPlacementCP         GetSpiralPlacementCP () const;
//! If this is a reference to portion of another curve, return detail data.  Otherwise return NULL.
GEOMDLLIMPEXP PartialCurveDetailCP         GetPartialCurveDetailCP () const;
//! If this is a reference to portion of another curve, return detail data.  Otherwise return NULL.
GEOMDLLIMPEXP bool  TryGetPartialCurveData (double &fractionA, double &fractionB, ICurvePrimitivePtr &parentCurve, int64_t &tag) const;




//flex!! Geometric Construction

//flex || Create a line segment || outPrim = CurvePrimitive::CreateLine (DSegment3dCR inSegment) || outPrim = CurvePrimitive::CreateLine (DPoint3dCR point0, DPoint3dCR point1) 
//flex || Create an elliptic arc || outPrim = CurvePrimtiive::CreateArc (DEllipse3DCR inArc) ||
//flex || Create a polyline || outPrim = CurvePrimitive::CreateLineString (bvector<DPoint3d> &points) ||
//flex ||  || outPrim = CurvePrimitive::CreateLineString (DPoint3dCP points, int n) ||
//flex || Create COPY of bspline curve.  Caller still responsible for disposal of original || outPrim = CurvePrimitive::CreateBsplineCurve (MSBsplineCurveCR curve) ||
//flex || Create COPY of interpolation curve.  Caller still responsible for disposal of original || outPrim = CurvePrimitive::CreateInterpolationCurve (MSInterpolationCurveCR curve) ||
//flex || Create an Akima curve || outPrim = CurvePrimitive::CreateAkimaCurve (DPoint3dCP points, int n) ||
//flex || Create a linestring for a rectangle, optionally fix orientation for positive or negative area || outPrim = CurvePrimitive::CreateRectangle (x0, y0, x1, y1, z, areaSign) ||
//flex || Create a spiral curve || outPrim = CurvePrimitive::CreateSpiral (DSpiral2dBaseCR planeSpiral, localToWorld, fractionA, fractionB) ||
//flex || Create a catenary curve || outPrim = CurvePrimitive::CreateCatenary (a, localToWorld, x0, x1) ||
//flex || Create a curve shifted by linear transform of parent curve derivative || outPrim = CurvePrimitive::CreateCatenary (parentCurve, matrix, userData) ||
//flex || Create a curve that references a fractional part of a parent curve || outPrim = CurvePrimitive::CreatePartialCurve (ICurvePrimitiveP parentCurve, fraction0, fraction1) ||
//flex || Create a curve that is fractionally interpolated between existing curves || outPrim = CurvePrimitive::CreateInterpolationBetweenCurves (curvePrimA, fraction, curvePrimB) ||

//! Allocate and fill a new line segemnt.
//! @param [in] segment source segment.
GEOMDLLIMPEXP static ICurvePrimitivePtr CreateLine (DSegment3dCR segment);

//! Allocate and fill a new line segemnt.
//! @param [in] point0 start point.
//! @param [in] point1 end point.
GEOMDLLIMPEXP static ICurvePrimitivePtr CreateLine (DPoint3dCR point0, DPoint3dCR point1);


//! Allocate and fill a new linestring
//! @param [in] points source coordinates.
//! @param [in] nPoints point count.
GEOMDLLIMPEXP static ICurvePrimitivePtr CreateLineString (DPoint3dCP points, size_t nPoints);

//! Allocate and fill a new linestring
//! @param [in] points source coordinates.
GEOMDLLIMPEXP static ICurvePrimitivePtr CreateLineString (bvector<DPoint3d> const &points);

//! Allocate and fill a new linestring
//! @param [in] points source coordinates.
//! @param [in] z z coordinate to apply when DPoint2d is copied to DPoint3d
GEOMDLLIMPEXP static ICurvePrimitivePtr CreateLineString (bvector<DPoint2d> const &points, double z = 0.0);

//! Allocate and fill a new elliptic arc
//! @param [in] ellipse source ellipse.
GEOMDLLIMPEXP static ICurvePrimitivePtr CreateArc (DEllipse3dCR ellipse);
//! Allocate and fill a new bspline curve
//! @param [in] curve source curve.  A copy (clone, repeat allocation of memory) of the curve is placed into the new object.  Caller is still responsible for freeing the input curve.
GEOMDLLIMPEXP static ICurvePrimitivePtr CreateBsplineCurve (MSBsplineCurveCR curve);
//! Allocate and fill a new bspline curve
//! @param [in] curve source curve.  Bits (including pointers) are copied to the CurvePrimtive.  source curve is zeroed -- caller has no "free" responsibilities.
GEOMDLLIMPEXP static ICurvePrimitivePtr CreateBsplineCurveSwapFromSource (MSBsplineCurveR curve);


//! Create curve primitive pointing to the same curve as the input.
//! @param [in] curve source curve.  The ICurvePrimitive will point at the same data.  Caller should NOT free the curve when it leaves scope.
GEOMDLLIMPEXP static ICurvePrimitivePtr CreateBsplineCurve (MSBsplineCurvePtr curve);


//! Allocate and fill a new interpolation curve
//! @param [in] fitCurve source curve.   Data is COPIED into the new object.   Caller is still responsible for freeing the input fitCurve.
GEOMDLLIMPEXP static ICurvePrimitivePtr CreateInterpolationCurve (MSInterpolationCurveCR fitCurve);

//! Allocate and fill a new interpolation curve
//! @param [in] fitCurve source curve.   Data is swapped into the curve.  input fitCurve is zeroed.
GEOMDLLIMPEXP static ICurvePrimitivePtr CreateInterpolationCurveSwapFromSource (MSInterpolationCurveR fitCurve);

//! Allocate and fill a new akima curve
//! @param [in] points akima control points to copy into the primitive.
//!   First two and final two points are slope end condition controls.  Others are pass-through.
//! @param [in] nPoints control point count.
GEOMDLLIMPEXP static ICurvePrimitivePtr CreateAkimaCurve (DPoint3dCP points, size_t nPoints);
//! Allocate and fill a new point string
//! @param [in] points coordinates to copy into the primitive
//! @param [in] nPoints number of points
GEOMDLLIMPEXP static ICurvePrimitivePtr CreatePointString (DPoint3dCP points, size_t nPoints);
//! Allocate and fill a new point string
//! @param [in] points coordinates to copy into the primitive
GEOMDLLIMPEXP static ICurvePrimitivePtr CreatePointString (bvector<DPoint3d> &points);


//! Create a rectangle from xy corners.
//! @param [in] x0 start point x coordinate
//! @param [in] y0 start point y coordinate
//! @param [in] x1 opposite corner x coordinate
//! @param [in] y1 opposite corner y coordinate
//! @param [in] z z value for all points.
//! @param [in] areaSignPreference is one of
//! <ul>
//! <li> any positive integer to force positive xy area.
//! <li> 0 to take order (x0,y0)(x1,y0),(x1,y1),(x0,y1)
//! <li> any negative integer to force negative xy area.
//! </ul>
GEOMDLLIMPEXP static ICurvePrimitivePtr CreateRectangle(double x0, double y0, double x1, double y1, double z, int areaSignPreference = 0);

//! Create a regular polygon parallel to the xy plane.
//! @param [in] center center coordinates.
//! @param [in] xDistance distance from center to x axis crossing point.
//! @param [in] numEdge number of edges (distinct vertices)
//! @param [in] isOuterRadius indicates whether x axis is outer or inner radius.
//!  (i.e. for outer radius (true), the x crossing is a vertex, and for inner radius (false) the x cross is the middle of a vertical edge.
//! @param [in] areaSignPreference is one of
//! <ul>
//! <li> any positive integer to force positive xy area.
//! <li> 0 to take order (x0,y0)(x1,y0),(x1,y1),(x0,y1)
//! <li> any negative integer to force negative xy area.
//! </ul>
GEOMDLLIMPEXP static ICurvePrimitivePtr CreateRegularPolygonXY
(
DPoint3dCR center,
double xDistance,
int numEdge,
bool isOuterRadius,
int areaSignPreference = 0
);

//! Allocate and fill a spiral curve.
//! @param [in] spiral spiral structure (to be cloned -- caller still responsible for deallocation)
//! @param [in] frame placement frame
//! @param [in] fractionA start fraction for active portion of curve
//! @param [in] fractionB end fraction for active portion of curve
//! @param [in] maxStrokeLength reasonable estimate of small stroke length.  10 meters recommended.
GEOMDLLIMPEXP static ICurvePrimitivePtr CreateSpiral
(
DSpiral2dBaseCR spiral,
TransformCR frame,
double fractionA,
double fractionB,
double maxStrokeLength = 10.0
);

//! Create a catenary curve
//! @param [in] a catenary constant for standard catenary equation {y = a * cosh(x/a)}
//! @param [in] basis placement frame (origin, x axis, y axis)
//! @param [in] x0 x coordinate for start of bounded curve
//! @param [in] x1 x coordinate for end of bounded curve
GEOMDLLIMPEXP static ICurvePrimitivePtr CreateCatenary (double a, DPoint3dDVec3dDVec3dCR basis, double x0, double x1);

//! Create a curve shifted by linear transform of parent curve derivative.
//! @param [in] parentCurve
//! @param [in] matrix to apply to parent derivative
//! @param [in] userData arbitrary tag
GEOMDLLIMPEXP static ICurvePrimitivePtr CreateTrackingCurve (ICurvePrimitiveP parentCurve, RotMatrixCR matrix, int64_t userData);

//! Create a curve with given xy offset at its start.
//! If this is an xy plane curve whose parameterization is strictly proportional to distance (i.e. constant magnitude of first derivative), this will be an offset of that distance everywhere.
//! (i.e. this is a constant offset only for line, circular arc, and spiral)
//! @param [in] parentCurve
//! @param [in] signedOffset offset to right at curve start.   Positive is to the right.
GEOMDLLIMPEXP static ICurvePrimitivePtr CreateTrackingCurveXY (ICurvePrimitiveP parentCurve, double signedOffset);


//! Allocate and fill a spiral curve.
//! @param [in] startRadians bearing at start
//! @param [in] startCurvature curvature at start (or 0 if flat)
//! @param [in] endRadians bearing at end
//! @param [in] endCurvature curvature at end (or 0 if flat)
//! @param [in] transitionType (see DSpiral2dBase)
//! @param [in] frame placement frame
//! @param [in] fractionA start fraction for active portion of curve
//! @param [in] fractionB end fraction for active portion of curve
GEOMDLLIMPEXP static ICurvePrimitivePtr CreateSpiralBearingCurvatureBearingCurvature
      (
      int transitionType,
      double startRadians,
      double startCurvature,
      double endRadians,
      double endCurvature,
      TransformCR frame,
      double fractionA,
      double fractionB
      );

//! Allocate and fill a spiral curve.
//! @param [in] startRadians bearing at start
//! @param [in] startRadius radius at start (or 0 if flat)
//! @param [in] endRadians bearing at end
//! @param [in] endRadius radius at end (or 0 if flat)
//! @param [in] transitionType (see DSpiral2dBase)
//! @param [in] frame placement frame
//! @param [in] fractionA start fraction for active portion of curve
//! @param [in] fractionB end fraction for active portion of curve
GEOMDLLIMPEXP static ICurvePrimitivePtr CreateSpiralBearingRadiusBearingRadius
      (
      int transitionType,
      double startRadians,
      double startRadius,
      double endRadians,
      double endRadius,
      TransformCR frame,
      double fractionA,
      double fractionB
      );

//! Allocate and fill a spiral curve.
//! @param [in] startRadians bearing at start
//! @param [in] startRadius radius at start (or 0 if flat)
//! @param [in] length length along spiral
//! @param [in] endRadius radius at end (or 0 if flat)
//! @param [in] transitionType (see DSpiral2dBase)
//! @param [in] frame placement frame
//! @param [in] fractionA start fraction for active portion of curve
//! @param [in] fractionB end fraction for active portion of curve
GEOMDLLIMPEXP static ICurvePrimitivePtr CreateSpiralBearingRadiusLengthRadius
      (
      int transitionType,
      double startRadians,
      double startRadius,
      double length,
      double endRadius,
      TransformCR frame,
      double fractionA,
      double fractionB
      );

//! Allocate and fill a spiral curve.
//! @param [in] startRadians bearing at start
//! @param [in] startCurvature curvature at start (or 0 if flat)
//! @param [in] length length along spiral
//! @param [in] endCurvature curvature at end (or 0 if flat)
//! @param [in] transitionType (see DSpiral2dBase)
//! @param [in] frame placement frame
//! @param [in] fractionA start fraction for active portion of curve
//! @param [in] fractionB end fraction for active portion of curve
GEOMDLLIMPEXP static ICurvePrimitivePtr CreateSpiralBearingCurvatureLengthCurvature
      (
      int transitionType,
      double startRadians,
      double startCurvature,
      double length,
      double endCurvature,
      TransformCR frame,
      double fractionA,
      double fractionB
      );
//! Create a pair of spirals that act like a "fillet" of a corner defined by start, shoulder and target.
//!<ul>
//!<li>Start exactly at pointA.
//!<li>Depart towards pointB with zero curvature
//!<li>turn towards pointC, reachnig maximum curvature near pointB.
//!<li>flatten out to tangency with the line pointB..pointC.
//!</ul>
//! 
GEOMDLLIMPEXP static bool CreateSpiralsStartShoulderTarget
(
int transitionType,     //!< [in] transition type
DPoint3dCR pointA,     //!< [in] start point
DPoint3dCR pointB,      //!< [in] target point for (both) tangencies
DPoint3dCR pointC,      //!< [in] final target
ICurvePrimitivePtr &primitiveA, //!< [in] first spiral
ICurvePrimitivePtr &primitiveB  //!< [in]  second spiral
);

//! Construct a spiral with start radius, spiral length, and end radius.
//!<ul>
//!<li> The spiral is paralllel to the xy plane.
//!<li> This is a special construction for "cubic" approximations.
//!<li> The constructed spiral is a fractional subset of another spiral that includes its inflection point (which may be outside
//!           the active fractional subset).
//!</ul>
GEOMDLLIMPEXP static ICurvePrimitivePtr CreatePseudoSpiralPointBearingRadiusLengthRadius
(
int typeCode,               //!< [in] transition type.  This method is intended to work with "cubic" approximations (New South Wales, Australian etc)
DPoint3dCR startPoint,      //!< [in] start point of spiral.
double startRadians,        //!< [in] start bearing angle in xy plane.
double radiusA,             //!< [in] (signed) radius (or 0 for line) at start.
double lengthAB,            //!< [in] length of spiral between radiusA and radiusB.
double radiusB              //!< [in] (signed) radius (or 0 for line) at end.
);

//! Construct a spiral with start radius, spiral length, and end radius.
//!<ul>
//!<li> The spiral is paralllel to the xy plane.
//!<li> This is a special construction for "cubic" approximations.
//!<li> The constructed spiral is a fractional subset of another spiral that includes its inflection point (which may be outside
//!           the active fractional subset).
//!</ul>
GEOMDLLIMPEXP static ICurvePrimitivePtr CreatePseudoSpiralPointBearingRadiusLengthRadius
(
int typeCode,               //!< [in] transition type.  This method is intended to work with "cubic" approximations (New South Wales, Australian etc)
DPoint3dCR startPoint,      //!< [in] start point of spiral.
double startRadians,        //!< [in] start bearing angle in xy plane.
double radiusA,             //!< [in] (signed) radius (or 0 for line) at start.
double lengthAB,            //!< [in] length of spiral between radiusA and radiusB.
double radiusB,              //!< [in] (signed) radius (or 0 for line) at end.
double startFraction,       //!< [in] active interval start fraction
double endFraction          //!< [in] active interval end fraction
);


//! Allocate and fill a reference to a portion of a parent curve.
//! @param [in] parentCurve pointer to another curve.
//! @param [in] fraction0 start of active portion of parent.
//! @param [in] fraction1 end of active portion of parent.
//! @param [in] index application data.
//! @remark fraction0 and fraction1 may be in forward or reverse relationship.
GEOMDLLIMPEXP static ICurvePrimitivePtr  CreatePartialCurve (ICurvePrimitiveP parentCurve, double fraction0, double fraction1, int64_t index = 0);

//! Create a curve that is interpolated between parents.
//! @param [in] curveA first curve
//! @param [in] curveB second curve
//! @param [in] fraction interpolation position.
//! @return null curve pointer if curves are not compatible for interpolation.
GEOMDLLIMPEXP static ICurvePrimitivePtr  CreateInterpolationBetweenCurves
(
ICurvePrimitiveCR curveA,
double fraction,
ICurvePrimitiveCR curveB
);

//flex!! Construction and Cloning
//flex
//flex || Deep clone || outPrim = curvePrim.Clone () ||
//flex || Clone component. || outPrim = curvePrim.CloneComponent (ptrdiff_t componentIndex) ||
//flex || Copy fractional portion || outPrim = curvePrimitive.CloneBetweenFractions (fractionA, fractionB, bool allowExtrapolation) ||
//flex || Deep clone with partial curves dereferenced || outPrim = curvePrimtive.CloneDereferenced (bool allowExtrapolation, bool recusiveDereference) ||
//flex || Create a child curve vector for preexisting refcounted curve vector || outPrim = CurvePrimitive::CreateChidlCurveVector (curveVectorPtr) ||
//flex || Create child curve vector by deep copy of input (passed by reference) || outPrim =CurvePrimitive::CretaeChildCurveVector_copyFromSource (source) ||
//flex || Create child curve vector swapping contents from input (passed by reference) || outPrim =CurvePrimitive::CretaeChildCurveVector_swapFromSource (source) ||

//! Return a deep copy
GEOMDLLIMPEXP ICurvePrimitivePtr Clone () const;

//! Return a deep copy with transform applied
GEOMDLLIMPEXP ICurvePrimitivePtr Clone (TransformCR transform) const;

//! Return a deep copy with transform applied.   If the transform has perspective effects, the geometry is reprojected to Cartesian form. (This can lose parameterization)
GEOMDLLIMPEXP ICurvePrimitivePtr Clone (DMatrix4dCR transform) const;

//! Return a copy of a component.
//! Component index only applies to linestring. Any invalid index clones entire linestring.
//! All others are complete Clone.
GEOMDLLIMPEXP ICurvePrimitivePtr CloneComponent (ptrdiff_t componentIndex) const;

//! Return a copy of a subset.  The fraction interval may be high to low;  fraction clamping respects the interval direction.
//! @return null pointer if fraction clamping resulted in zero-length interval.
//! @param [in] fractionA start of returned invterval.
//! @param [in] fractionB end of returned interval.
//! @param [in] allowExtrapolation   If false, out of bounds values are clamped.
GEOMDLLIMPEXP ICurvePrimitivePtr CloneBetweenFractions (double fractionA, double fractionB, bool allowExtrapolation) const;

//! Return an offset primitive.  This operates only on single primtives that can offset to another single primitive.
//! @param [in] options contains tolerance for bspline and ellipse offset.
GEOMDLLIMPEXP ICurvePrimitivePtr CloneAsSingleOffsetPrimitiveXY (CurveOffsetOptionsCR options) const;

//! Return a (deep) clone with all partial curves replaced by complete curves.
//! @param [in] maximumDeref true to recurse through all steps of PartialCurve chains
//! @param [in] allowExtrapolation true to allow extension before/after endpoints.
GEOMDLLIMPEXP ICurvePrimitivePtr CloneDereferenced (bool allowExtrapolation = false, bool maximumDeref = true) const;

//! Create a curve primitive with (ref coutnted pointer to preexisting refcounted child.
GEOMDLLIMPEXP static ICurvePrimitivePtr CreateChildCurveVector (CurveVectorPtr source);
//! Create child vector, making deep copy of the source.
GEOMDLLIMPEXP static ICurvePrimitivePtr CreateChildCurveVector_CopyFromSource (CurveVectorCR source);
//! Create a child vector; contents of source are taken, source itself is cleared.
GEOMDLLIMPEXP static ICurvePrimitivePtr CreateChildCurveVector_SwapFromSource (CurveVectorR source);

//flex!! Parametric Queries

//flex || Fraction to point || bool curvePrim.FractionToPoint (fraction, outPoint) ||
//flex ||                  || bool curvePrim.FractionToPoint (fraction, outPoint, outDerivative1) ||
//flex ||                  || bool curvePrim.FractionToPointWithTwoSidedDerivative (fraction, outPoint, derivativeA, derivativeB) ||
//flex ||                  || bool curvePrim.FractionToPoint (fraction, outPoint, outDerivative, outDerivative2) ||
//flex ||                  || bool curvePrim.FractionToPoint (fraction, outPoint, outDerivative, outDerivative2, outDerivative3) ||
//flex ||                  || bool FractionToFrenetFrame (fraction, outTransform) ||
//flex ||  Distance from one fractional position to another || bool curvePrimitive.SignedDistanceBetweenFractions (fraction0, fraction1, outDistance)||
//flex || Find position (fraction and coordinates) a distance away from a start fraction || bool curvePrimitive.PointAtSignedDistanceAndFraction (fraction0, signedDistance, bool allowExtension, SEE(CurveLocationDetail) outPosition) ||
//flex || Ask if fractional coordintaes make sense within this curve || bool curvePrim.IsFractionSpace () ||
//flex || Ask if fractional coordinates outside 0..1 make sense on this curve type || bool curvePrim.IsExtensibleFractionSpace () ||
//flex || Ask if fractional coordinates become periodic, and if so return the (fraction space) period || bool curvePrim.IsPeriodicFractionSpace () ||
//flex || Ask if fraction coordinates map cleanly to fractional-interval partial curves.   (linestrings and child curve primitives do not) || bool curvePrim.IsMappableFractionSpace () ||

//! Evaluate curve fractional position within its parameter space.
//! @return false if this is not a parameterized curve (EXAMPLE: A ChildCurveVector)
//! @param [in] f evaluation fraction.
//! @param [out] point point on curve.
GEOMDLLIMPEXP bool FractionToPoint (double f, DPoint3dR point) const;

//! Evaluate curve fractional position within its parameter space.
//! @return false if this is not a parameterized curve (EXAMPLE: A ChildCurveVector)
//! @param [in] f evaluation fraction.
//! @param [out] point point on curve.
//! @param [out] tangent first derivative with respect to the fractional coordinate.  (NOT normalized)
GEOMDLLIMPEXP bool FractionToPoint (double f, DPoint3dR point, DVec3dR tangent) const;

//! Evaluate curve fractional position within its parameter space.
//! The returned ValidatedDRay3d is is IsValid () false if this is not a parameterized curve (EXAMPLE: A ChildCurveVector)
//! @param [in] f evaluation fraction.
//! @return ray with whose origin and direction fields are the point and unit tangent.
GEOMDLLIMPEXP ValidatedDRay3d FractionToPointAndUnitTangent (double f) const;

//! Evaluate curve fractional position within its parameter space.
//! <ul>
//! <li>On a smooth curve, incoming and outgoing derivatives will be identical.
//! <li>Incoming and outgoing derivatives can differ at linestring interior points and bspline knots.
//! </ul>
//! return false if this is not a parameterized curve (EXAMPLE: A ChildCurveVector)
//! @param [in] f evaluation fraction.
//! @param [out] point point on curve.
//! @param [out] derivativeA  first derivative with respect to the fractional coordinate, taken on the inbound (lower parameter) side of the parameter.
//! @param [out] derivativeB  first derivative with respect to the fractional coordinate, taken on the outbound (higher parameter) side of the parameter.
GEOMDLLIMPEXP bool FractionToPointWithTwoSidedDerivative (double f, DPoint3dR point, DVec3dR derivativeA, DVec3dR derivativeB) const;

//! Evaluate curve fractional position within the parameter space of a component.
//!   (For anything except a linestring, componentIndex is ignored and this is equivalent to FractionToPoint)
//! @return false if this is not a parameterized curve (EXAMPLE: A ChildCurveVector)
//! @param [in] componentIndex index of the component to evaluate.
//! @param [in] f evaluation fraction.
//! @param [out] point point on curve.
GEOMDLLIMPEXP bool ComponentFractionToPoint (ptrdiff_t componentIndex, double f, DPoint3dR point) const;

//! Evaluate curve fractional position within the parameter space of a component.
//!   (For anything except a linestring, componentIndex is ignored and this is equivalent to FractionToPoint)
//! @return false if this is not a parameterized curve (EXAMPLE: A ChildCurveVector)
//! @param [in] componentIndex index of the component to evaluate.
//! @param [in] f evaluation fraction.
//! @param [out] point point on curve.
//! @param [out] tangent first derivative with respect to the fractional coordinate.  (NOT normalized)
GEOMDLLIMPEXP bool ComponentFractionToPoint (ptrdiff_t componentIndex, double f, DPoint3dR point, DVec3dR tangent) const;

//! Evaluate curve fractional position within its parameter space.
//! @return false if this is not a parameterized curve (EXAMPLE: A ChildCurveVector)
//! @param [in] f evaluation fraction.
//! @param [out] ray point and derivative packaged as a ray.
GEOMDLLIMPEXP bool FractionToPoint (double f, DRay3dR ray) const;

//! Evaluate curve fractional position, and return as a CurveLocationDetail.
//! @return false if this is not a parameterized curve (EXAMPLE: A ChildCurveVector)
//! @param [in] f evaluation fraction.
//! @param [out] point evaluated point, with fraction and component markup.
GEOMDLLIMPEXP bool FractionToPoint (double f, CurveLocationDetail &point) const;

//! Evaluate curve point at fractional position within its parameter space.
//! @return false if this is not a parameterized curve (EXAMPLE: A ChildCurveVector)
//! @param [in] f evaluation fraction.
//! @param [out] point point on curve.
//! @param [out] tangent first deriviatve with respect to the fractional coordinate.  (NOT normalized)
//! @param [out] derivative2 second deriviatve with respect to the fractional coordinate.  (NOT normalized)
GEOMDLLIMPEXP bool FractionToPoint (double f, DPoint3dR point, DVec3dR tangent, DVec3dR derivative2) const;

//! Evaluate curve point at fractional position within its parameter space.
//! @return false if this is not a parameterized curve (EXAMPLE: A ChildCurveVector)
//! @param [in] f evaluation fraction.
//! @param [out] point point on curve.
//! @param [out] tangent first deriviatve with respect to the fractional coordinate.  (NOT normalized)
//! @param [out] derivative2 second deriviatve with respect to the fractional coordinate.  (NOT normalized)
//! @param [out] derivative3 third deriviatve with respect to the fractional coordinate.  (NOT normalized)
GEOMDLLIMPEXP bool FractionToPoint (double f, DPoint3dR point, DVec3dR tangent, DVec3dR derivative2, DVec3dR derivative3) const;

//! Evaluate curve point at fractional position within its parameter space.
//! @return false if this is not a parameterized curve (EXAMPLE: A ChildCurveVector)
//! @param [in] f evaluation fraction.
//! @param [out] frame Coordinate frame with origin on curve, x direction along curve, y direction in curvature plane, z direction perpendicular.
GEOMDLLIMPEXP bool FractionToFrenetFrame (double f, TransformR frame) const;
//! Evaluate curve point at fractional position within its parameter space.
//! @return false if this is not a parameterized curve (EXAMPLE: A ChildCurveVector)
//! @param [in] f evaluation fraction.
//! @param [out] frame Coordinate frame with origin on curve, x direction along curve, y direction in curvature plane, z direction perpendicular.
//! @param [out] curvature curvature (in the xy plane of the frenet frame)
//! @param [out] torsion 
GEOMDLLIMPEXP bool FractionToFrenetFrame (double f, TransformR frame, double &curvature, double &torsion) const;


//! Evaluate curve point at fractional position within its parameter space.
//! @return Coordinate frame with origin on curve, x direction along curve, y direction in curvature plane, z direction perpendicular.
//!     The return is invalid if this is not a parameterized curve (e.g. a ChildCurveVector)
//! @param [in] f evaluation fraction.
GEOMDLLIMPEXP ValidatedTransform FractionToFrenetFrame (double f) const;

//flex!! Size and orientation
//flex|| Compute length || bool curvePrim.Length (outLength ) ||
//flex|| Compute range cube   || bool curvePrim.GetRange (out range) ||
//flex|| Compute range cube after transform || bool curvePrim.GetRange (range, transform) ||
//flex|| range of projection in parameter space of a ray. || range1d = curvePrim.ProjectedParameterRange (ray, fraction0, fraction1) ||
//flex|| Quick estimate of largest coordinate present || a = curvePrim.FastMaxAbs () || 
//flex|| Compute centroid of the curve as a wire || bool curvePrim. WireCentroid (outLength, outPoint fraction0, fraction1)||

//! Compute curve length.
//! @return false if no measurable curves.
//! @param [out] length curve length.   For ChildCurveVector, length of contained curves is summed.
GEOMDLLIMPEXP bool Length (double &length) const;

//! Compute curve length under effect of a RotMatrix.
//! Note that if you want xy area, the transform must include scaling z parts to zero.  Just passing
//! a viewing matrix is not sufficient -- the z depth is still present after the multiplication.
//! return false if no measurable curves.
//! @param [in] worldToLocal transform to apply to vectors.
//! @param [out] length curve length.   For ChildCurveVector, length of contained curves is summed.
GEOMDLLIMPEXP bool Length (RotMatrixCP worldToLocal, double &length) const;



//! Compute a fast approximation of curve length.
//! @return false if no measurable curves.
//! @param [out] length curve length.   For ChildCurveVector, length of contained curves is summed.
GEOMDLLIMPEXP bool FastLength (double &length) const;

//! Return range of the primitive.
GEOMDLLIMPEXP bool GetRange (DRange3dR range) const;
//! Return range of the primitive under a transform
GEOMDLLIMPEXP bool GetRange (DRange3dR range, TransformCR transform) const;

//! Return the range the primitive projected onto a ray.
//! @return DRange1d with range data.
//! @param [in] ray test ray.
//! @param [in] fraction0 start fraction of active part.
//! @param [in] fraction1 end fraction of active part.
//! @remark If the ray's direction vector is a unit vector, the projected parameters are physical distances.
//! @remark If the ray's direction vector is NOT a unit vector, the projected parameters are fractions of the ray's direction vector.
//! @remark If the primitive has no curves, the returned range returns true on the DRange1d::IsNull() predicate.
GEOMDLLIMPEXP DRange1d ProjectedParameterRange (DRay3dCR ray, double fraction0, double fraction1) const;

//! Return the range the primitive projected onto a ray.
//! @return DRange1d with range data.
//! @param [in] ray test ray.
//! @remark If the ray's direction vector is a unit vector, the projected parameters are physical distances.
//! @remark If the ray's direction vector is NOT a unit vector, the projected parameters are fractions of the ray's direction vector.
//! @remark If the primitive has no curves, the returned range returns true on the DRange1d::IsNull() predicate.
GEOMDLLIMPEXP DRange1d ProjectedParameterRange (DRay3dCR ray) const;

//! Return a representative large coordinate.
//! This is not required to be a true range limit.
//! For instance, a max abs of a bspline pole range is acceptable.
GEOMDLLIMPEXP double FastMaxAbs () const;

//! Recursive check for structural match (tree structure and leaf type) with the other curve primitive.
GEOMDLLIMPEXP bool IsSameStructure (ICurvePrimitiveCR other) const;

//! Recursive check for match (tree structure. leaf type, and geometry) with a peer.
//! <param name="other">peer for comparison</param>
//! <param name="tolerance">distance tolerance. (See DoubleOps::AlmostEqual ())</param>
GEOMDLLIMPEXP bool IsSameStructureAndGeometry (ICurvePrimitiveCR other, double tolerance = 0.0) const;

//! Return the number of components that can be parameterized.
//! This is
//!<ul>
//!<li> 1 for all curves except linestrings.
//!<li> 0 for child curve vector
//!<li> the number of edges in a linestring.
//!</ul>
GEOMDLLIMPEXP size_t NumComponent() const;

//flex!! Break fractions
//flex All primitives have a "break" at start and end (fractions 0 and 1).   Polylines have additional breaks at interior vertices.  Bspline, Interpolation, and Akima curves have breaks at interior knot values.
//flex || Ask for fractional coordinate of breakpoint by index || bool curvePrim.GetBreakFraction (index, outFraction) ||
//flex || Ask for index and fraction of breakpoint closest to a fraction || bool AdjustFractionToBreakFraction (inFraction, roundingMode, outBreakIndex, outBreakFraction) ||


//! Return the fractional postion of a point where the curve's continuity has a break.
GEOMDLLIMPEXP bool GetBreakFraction (size_t breakFractioniIndex, double &fraction) const;
//! Move a fraction to the nearest break fraction.
GEOMDLLIMPEXP bool AdjustFractionToBreakFraction (double fraction, Rounding::RoundingMode mode, size_t &breakIndex, double &adjustedFraction) const;
//! Initialize an MSBsplineCurve form of the curve.
GEOMDLLIMPEXP bool GetMSBsplineCurve (MSBsplineCurveR curve, double fraction0 = 0.0, double fraction1 = 1.0) const;
//! Clone as bspline.  Return nullptr if unable to clone as curve.
GEOMDLLIMPEXP ICurvePrimitivePtr CloneAsBspline (double fraction0 = 0.0, double fraction1 = 1.0) const;
//! Initialize a (ref coutned) MSBsplineCurve form of the curve.
GEOMDLLIMPEXP MSBsplineCurvePtr GetMSBsplineCurvePtr (double fraction0 = 0.0, double fraction1 = 1.0) const;


//! Return distance between fractions.
//! Primitives that do not have extensible fraction spaces will SILENTLY clamp the fractions to 0..1.
//! @return false if this is not a parameterized curve (EXAMPLE: A ChildCurveVector)
//! @param [in] startFraction starting position for partial curve.
//! @param [in] endFraction end position for partial curve.
//! @param [out] signedDistance distance along curve, negative if fraction direction is reversed.
GEOMDLLIMPEXP bool SignedDistanceBetweenFractions (double startFraction, double endFraction, double &signedDistance) const;

//! Return distance between fractions.
//! Primitives that do not have extensible fraction spaces will SILENTLY clamp the fractions to 0..1.
//! return false if this is not a parameterized curve (EXAMPLE: A ChildCurveVector)
//! @param [in] startFraction starting position for partial curve.
//! @param [in] endFraction end position for partial curve.
//! @param [in] worldToLocal matrix to apply to vectors.
//! @param [out] signedDistance distance along curve, negative if fraction direction is reversed.
GEOMDLLIMPEXP bool SignedDistanceBetweenFractions (RotMatrixCP worldToLocal, double startFraction, double endFraction, double &signedDistance) const;


//! Attempt to move a specified distance from given fraction.
//! @param [in] startFraction starting position.
//! @param [in] signedDistance distance for attempted move.
//! @param [in] allowExtension controls extended behaior of line, arc, and linestring.
//! @param [out] location fraction and point after move.  {a} field indicates actual signed distance moved (which may be less than request!!)
//! @remark If extension is not allowed, there are fussy rules for both the input and output.
//! (1) The startFraction is clamped to {0..1}
//! (2) Movement stops at the endpoint in the indicated direction.
GEOMDLLIMPEXP bool PointAtSignedDistanceFromFraction (double startFraction, double signedDistance, bool allowExtension, CurveLocationDetailR location) const;


//! Attempt to move a specified distance from given fraction.
//! @param [in] worldToView optional transformation.  To get "in XY Plane" behavior, the matrix must flatten.
//! @param [in] startFraction starting position.
//! @param [in] signedDistance distance for attempted move.
//! @param [in] allowExtension controls extended behaior of line, arc, and linestring.
//! @param [out] location fraction and point after move.  {a} field indicates actual signed distance moved (which may be less than request!!)
//! @remark If extension is not allowed, there are fussy rules for both the input and output.
//! (1) The startFraction is clamped to {0..1}
//! (2) Movement stops at the endpoint in the indicated direction.
GEOMDLLIMPEXP bool PointAtSignedDistanceFromFraction (RotMatrixCP worldToView, double startFraction, double signedDistance, bool allowExtension, CurveLocationDetailR location) const;

//! Find the closest point to a space point.
//! @return false if not a simple curve. (EXAMPLE: ChildCurveVector)
//! @param [in] spacePoint reference point.
//! @param [out] fraction fractional position at closest point.
//! @param [out] curvePoint coordinates on curve
GEOMDLLIMPEXP bool ClosestPointBounded (DPoint3dCR spacePoint, double &fraction, DPoint3dR curvePoint) const;

//! Search for various keypoints (as requested by the collector)
//! During recursion, extension bits are changed to false for interior points of paths
GEOMDLLIMPEXP void AnnounceKeyPoints
(
DPoint3dCR spacePoint,                       //!< [in] point to project
CurveKeyPointCollector &collector,           //!< [in] object to receive keypoint announcements
bool extend0,                               //!< true to extend at start of primitives.
bool extend1                                //!< true to extend at end of primitives.
) const;


//! Find the closest point to a space point.
//! @return false if not a simple curve. (EXAMPLE: ChildCurveVector)
//! @param [in] spacePoint reference point.
//! @param [out] location details of hit:
//! <ul>
//! <li> location.curve = curve (e.g. from search within ChildCurveVector)
//! <li> location.fraction = fractional coordinate on location.curve
//! <li> location.point = coordinates on location.curve
//! <li> location.a = distance from space point
//! <li> componentIndex = if location.curve is a linestring, the segment index within the linestring
//! <li> numComponent = if location.curve is a linestring, the number of edges.
//! <li> componentFraction = if location.curve is a linestring, the fractional position within the edge.
//! </ul>
//! @param [in] extend0 true to allow line, linestring, ellipse to extend backwards before start.
//! @param [in] extend1 true to allow line, linestring, ellipse to extend beyond end.
GEOMDLLIMPEXP bool ClosestPointBounded (DPoint3dCR spacePoint, CurveLocationDetailR location, bool extend0, bool extend1) const;

//! Find the closest point to a space point.
//! @return false if not a simple curve. (EXAMPLE: ChildCurveVector)
//! @param [in] spacePoint reference point.
//! @param [out] location details of hit:
//! <ul>
//! <li> location.curve = curve (e.g. from search within ChildCurveVector)
//! <li> location.fraction = fractional coordinate on location.curve
//! <li> location.point = coordinates on location.curve
//! <li> location.a = distance from space point
//! <li> componentIndex = if location.curve is a linestring, the segment index within the linestring
//! <li> numComponent = if location.curve is a linestring, the number of edges.
//! <li> componentFraction = if location.curve is a linestring, the fractional position within the edge.
//! </ul>
GEOMDLLIMPEXP bool ClosestPointBounded (DPoint3dCR spacePoint, CurveLocationDetailR location) const;



//! Find closest point, measuring in xy plane after applying a (possibly perspective) transform to both the curve and space point.
//! @param [in] spacePoint reference point.
//! @param [in] worldToLocal optional transform.
//! @param [out] location details of closest point.   See ClosestPointBounded.
GEOMDLLIMPEXP bool ClosestPointBoundedXY (DPoint3dCR spacePoint, DMatrix4dCP worldToLocal, CurveLocationDetailR location) const;


//! Find closest point, measuring in xy plane after applying a (possibly perspective) transform to both the curve and space point.
//! @param [in] spacePoint reference point.
//! @param [in] worldToLocal optional transform.
//! @param [out] location details of closest point.
//! @param [in] extend0 true to extend at beginning
//! @param [in] extend1 true to extend at end
GEOMDLLIMPEXP bool ClosestPointBoundedXY (DPoint3dCR spacePoint, DMatrix4dCP worldToLocal, CurveLocationDetailR location, bool extend0, bool extend1) const;

//! @return the start and end points of the curve
//! @param [out] pointA start point
//! @param [out] pointB end point
//! @remark For ChildCurveVector, the first and last component endpoints are determined.
GEOMDLLIMPEXP bool GetStartEnd (DPoint3dR pointA, DPoint3dR pointB) const;
//! @return start and end points and the (normalized!) tangents.
//! @param [out] pointA start point
//! @param [out] pointB end point
//! @param [out] unitTangentA normalized tangent (forward) at pointA.
//! @param [out] unitTangentB normalized tangent (forward) at pointB.
//! @remark For ChildCurveVector, the first and last component endpoints are determined.
GEOMDLLIMPEXP bool GetStartEnd (DPoint3dR pointA, DPoint3dR pointB, DVec3dR unitTangentA, DVec3dR unitTangentB) const;

//! Return first primitive in a deep search.
//! @param [out] point start point.
GEOMDLLIMPEXP bool GetStartPoint (DPoint3dR point) const;

//! Modify the start point if possible
GEOMDLLIMPEXP bool TrySetStart (DPoint3dCR xyz);
//! Modify the end point if possible
GEOMDLLIMPEXP bool TrySetEnd (DPoint3dCR xyz);

//! Test if fractional queries allow extension.  When this is true, the CurvePrimitive recognizes fractions outside of 0..1.
GEOMDLLIMPEXP bool IsExtensibleFractionSpace () const;

//! Test if cloned fractional intervals map linearly back to parent fractions.
//! (Not true for linestrings and child curve vectors.
GEOMDLLIMPEXP bool IsMappableFractionSpace () const;

//! Test if this is a curve (i.e. not a child vector or point string)
GEOMDLLIMPEXP bool IsFractionSpace () const;


//! Return true if the curve is part of a (possibly larger) periodic curve.
//! @param [out] period period as a multiple of the bounded curve's fraction space.  For example, a quarter arc has a period of 4.
GEOMDLLIMPEXP bool IsPeriodicFractionSpace(double &period) const;

//flex !! Inplace modification
//flex 
//flex || description || ||
//flex || apply transform || curvePrim.TransformInPlace (transform) ||

//! Transform in place.
GEOMDLLIMPEXP bool TransformInPlace (TransformCR transform);

//! reverse the parameterization in place.
GEOMDLLIMPEXP bool ReverseCurvesInPlace ();


//flex !! Intersections, Containment
//flex 
//flex || Intersection with plane.   return can indicate both a) single point contact and ( b)    "on plane" sections. || cp.AppendCurvePlaneIntersections (plane, bvector<SEE(CurveLocationDetailPair)> & outIntersections, double tolerance) ||


//! Return vector of intersections with a plane.
//! Single point intersection appears as a CurveLocationDetailPair with identical locations for both parts of the pair (SameCurveAndFraction)
//! Curve-on-plane appears as CurveLocationDetailPair with curve,fraction data for start and end of on-plane sections.
//! @param [in] plane 
//! @param [out] intersections intersection details
//! @param [in] tolerance for on-plane decisions.  If 0, a tolerance is computed based on the coordinates in the curve.
GEOMDLLIMPEXP void AppendCurvePlaneIntersections (DPlane3dCR plane, bvector<CurveLocationDetailPair> &intersections, double tolerance = 0.0) const;

//! Return intesections of curve primitive with a range box.
GEOMDLLIMPEXP void  AppendCurveRangeIntersections
(
LocalRangeCR range,                          //!< [in] range box
bvector<PartialCurveDetail> &intersections   //!< [out] intersections 
) const;

//! Return intesections of curve primitive with a bilinear patch
GEOMDLLIMPEXP void  AppendCurveBilinearPatchIntersections
(
DBilinearPatch3dCR patch,                      //!< [in] patch to intersect
bvector<CurveAndSolidLocationDetail> &intersections   //!< [out] intersections 
) const;

//! Return intesections of curve primitive with (the plane of) a triangle.
GEOMDLLIMPEXP void  AppendCurvePlaneIntersections
(
DPoint3dDVec3dDVec3dCR plane,  //!< [in] plane to intersect
UVBoundarySelect   bounded,         //!< [in] selects Unbounded, Triangle, or Parallelogram boundaries.
bvector<CurveAndSolidLocationDetail> &intersections   //!< [out] intersections 
) const;

//! Return the centroid of (a portion of) the curve.
//! @param [out] length curve length
//! @param [out] centroid curve centroid
//! @param [in] fraction0 start fraction of active part of sweep.
//! @param [in] fraction1 end fraction of active part of sweep.
GEOMDLLIMPEXP bool WireCentroid (double &length, DPoint3dR centroid, double fraction0 = 0.0, double fraction1 = 1.0) const;


//flex !! Stroking
//flex 
//flex || Add stroked approximation to a bvector. || curvePrim.AddStrokes (bvector<DPoint3d> &points, SEE(IFacetOptions) facetOptions, bool includeStart, fraction0, fraction1) ||
//flex || Ask how many strokes sill be needed || n = curvePrim.GetStrokeCount (SEE(IFacetOptions) facetOptions, fraction0, fraction1) ||

//! Stroke the curve and add points to the bvector.
//! @return true if this is a strokable primitive -- Line, Arc, Bspline, Spiral, Akima, or partial curve.
//!        false for non-strokable -- i.e. child vector or point vector.
//! @param [in,out] points growing vector of strokes.
//! @param [in] options stroke tolerance.
//! @param [in] includeStartPoint if false, do NOT put start point in the output.
//! @param [in] startFraction start of partial curve interval.
//! @param [in] endFraction end of partial curve interval.
GEOMDLLIMPEXP bool AddStrokes (bvector <DPoint3d> &points, IFacetOptionsCR options,
                bool includeStartPoint = true,
                double startFraction = 0.0,
                double endFraction = 1.0
                ) const;

//! Stroke the curve and add points to the arrays.
//!<ul>
//!<li>At each point, the computed output is:  [point X, fraction, dXdf, curve]
//!<li>Note that because the curve pointer changes from end of one curve to start of another, there is no option to omit start point.
//!</li>
//! @return true if this is a strokable primitive -- Line, Arc, Bspline, Spiral, Akima, or partial curve.
//!        false for non-strokable -- i.e. child vector or point vector.
//! @param [in,out] points growing vector of strokes.
//! @param [in] options stroke tolerance.
//! @param [in] startFraction start of partial curve interval.
//! @param [in] endFraction end of partial curve interval.
GEOMDLLIMPEXP bool AddStrokes (DPoint3dDoubleUVCurveArrays &points, IFacetOptionsCR options,
                double startFraction = 0.0,
                double endFraction = 1.0
                ) const;


//! Stroke the curve and add points to the bvector.  This includes distanceAlongCurve as an output -- hence may be significantly
//! more expensive than just the points.
//! @return true if this is a strokable primitive -- Line, Arc, Bspline, Spiral, Akima, or partial curve.
//!        false for non-strokable -- i.e. child vector or point vector.
//! @param [in,out] points growing vector of strokes, annotated with distance along.  When prior points are present in the
//!       points array, the final prior distance is used as start for new distances.
//! @param [in] options stroke tolerance.
//! @param [in] startFraction start of partial curve interval.
//! @param [in] endFraction end of partial curve interval.
GEOMDLLIMPEXP bool AddStrokes (
    bvector <PathLocationDetail> &points,
    IFacetOptionsCR options,
    double startFraction = 0.0,
    double endFraction = 1.0
    ) const;

//! Return the number of strokes needed to approximate this curve primitive.
GEOMDLLIMPEXP size_t GetStrokeCount (IFacetOptionsCR options, double startFraction = 0.0, double endFraction = 1.0) const;

//! Compute intersections of a ray with a ruled surface between two primitves.
//! @return false if primitives are not compatible
//! @param [out] pickData array to receive picks
//! @param [in] curveA first curve
//! @param [in] curveB second curve
//! @param [in] ray ray
GEOMDLLIMPEXP static bool AddRuledSurfaceRayIntersections
    (
    bvector<struct SolidLocationDetail> &pickData,
    ICurvePrimitiveCR curveA,
    ICurvePrimitiveCR curveB,
    DRay3dCR ray
    );


}; // ICurvePrimitive


END_BENTLEY_GEOMETRY_NAMESPACE

#ifdef BENTLEY_WIN32
#pragma warning (pop)
#endif
