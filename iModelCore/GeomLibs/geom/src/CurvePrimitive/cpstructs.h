/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/CurvePrimitive/cpstructs.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  02/10
+===============+===============+===============+===============+===============+======*/
struct CurvePrimitiveLine : public ICurvePrimitive
{
protected:
explicit CurvePrimitiveLine(DSegment3dCR segment);
DSegment3d      m_segment;
ICurvePrimitivePtr _Clone() const override;
ICurvePrimitivePtr _CloneBetweenFractions ( double fractionA, double fractionB, bool allowExtrapolation ) const override;
ICurvePrimitivePtr _CloneAsSingleOffsetPrimitiveXY (CurveOffsetOptionsCR options) const override;
CurvePrimitiveType _GetCurvePrimitiveType() const override;
DSegment3dCP _GetLineCP() const override;
void _Process(ICurvePrimitiveProcessor &processor, DSegment1dCP interval) const override;
bool _IsExtensibleFractionSpace() const override;
bool _IsMappableFractionSpace() const override;
bool _IsFractionSpace() const override;
bool _FractionToPoint(double fraction, DPoint3dR point) const override;
bool _TrySetStart (DPoint3dCR xyz) override;
bool _TrySetEnd (DPoint3dCR xyz) override;
bool _FractionToPoint(double fraction, DPoint3dR point, DVec3dR tangent) const override;
bool _FractionToPoint(double fraction, DPoint3dR point, DVec3dR tangent, DVec3dR derivative2) const override;
bool _FractionToPoint(double fraction, DPoint3dR point, DVec3dR tangent, DVec3dR derivative2, DVec3dR derivative3) const override;
bool _FractionToFrenetFrame(double f, TransformR frame) const override;
bool _Length(double &length) const override;
bool _Length (RotMatrixCP worldToLocal, double &length) const override;
bool _GetRange(DRange3dR range) const override;
bool _GetRange(DRange3dR range, TransformCR transform) const override;
double _FastMaxAbs() const override;
bool _IsSameStructureAndGeometry (ICurvePrimitiveCR other, double tolerance) const override;
size_t _NumComponent () const override;
DRange1d _ProjectedParameterRange(DRay3dCR ray) const override;
DRange1d _ProjectedParameterRange(DRay3dCR ray, double fractionA, double fractionB) const override;
bool _GetBreakFraction(size_t breakFractionIndex, double &fraction) const override;
bool _AdjustFractionToBreakFraction(double fraction, Rounding::RoundingMode mode, size_t &breakIndex, double &adjustedFraction) const override;
bool _GetMSBsplineCurve(MSBsplineCurveR curve, double fractionA, double fractionB) const override;
bool _AddStrokes(bvector <DPoint3d> &points, IFacetOptionsCR options, bool includeStartPoint, double startFraction, double endFraction ) const override;
bool _AddStrokes(bvector <PathLocationDetail> &points, IFacetOptionsCR options, double startFraction, double endFraction ) const override;
size_t _GetStrokeCount(IFacetOptionsCR options, double startFraction, double endFraction) const override;
bool _SignedDistanceBetweenFractions(double startFraction, double endFraction, double &signedDistance) const override;
bool _SignedDistanceBetweenFractions(RotMatrixCP worldToLocal, double startFraction, double endFraction, double &signedDistance) const override;
bool _PointAtSignedDistanceFromFraction(double startFraction, double signedDistance, bool allowExtension, CurveLocationDetailR location) const override;
bool _PointAtSignedDistanceFromFraction (RotMatrixCP worldToLocal, double startFraction, double signedLength, bool allowExtension, CurveLocationDetailR location) const override;
bool _ClosestPointBounded (DPoint3dCR spacePoint, double &fraction, DPoint3dR curvePoint, bool extend0, bool extend1) const override;
bool _ClosestPointBoundedXY(DPoint3dCR spacePoint, DMatrix4dCP worldToLocal, CurveLocationDetailR location, bool extend0, bool extend1) const override;
void _AnnounceKeyPoints
(
DPoint3dCR spacePoint,                       //!< [in] point to project
CurveKeyPointCollector &collector,           //!< [in] object to receive keypoint announcements
bool extend0,                               //!< true to extend at start of primitives.
bool extend1                                //!< true to extend at end of primitives.
) const override;
bool _TransformInPlace (TransformCR transform) override;
bool _ReverseCurvesInPlace () override;
void _AppendCurvePlaneIntersections(DPlane3dCR plane, bvector<CurveLocationDetailPair> &intersections, double tol) const override;
void _AppendCurveRangeIntersections (LocalRangeCR range, bvector<PartialCurveDetail> &intersections) const override;
void  _AppendCurvePlaneIntersections (DPoint3dDVec3dDVec3dCR triangle, UVBoundarySelect bounded, bvector<CurveAndSolidLocationDetail> &intersections) const override;
bool _WireCentroid(double &length, DPoint3dR centroid, double fraction0, double fraction1) const override;
public:
static GEOMDLLIMPEXP  ICurvePrimitive* Create (DSegment3dCR segment);

};


/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  02/10
+===============+===============+===============+===============+===============+======*/
struct CurvePrimitiveArc : public ICurvePrimitive
{
protected:
DEllipse3d      m_ellipse;
explicit CurvePrimitiveArc(DEllipse3dCR ellipse);
ICurvePrimitivePtr _Clone() const override;
bool _IsExtensibleFractionSpace() const override;
bool _IsMappableFractionSpace() const override;
bool _IsFractionSpace() const override;
bool _IsPeriodicFractionSpace(double &period) const override;
ICurvePrimitivePtr _CloneBetweenFractions ( double fractionA, double fractionB, bool allowExtrapolation ) const override;
ICurvePrimitivePtr _CloneAsSingleOffsetPrimitiveXY (CurveOffsetOptionsCR options) const override;
CurvePrimitiveType _GetCurvePrimitiveType() const override;
DEllipse3dCP _GetArcCP() const override;
void _Process(ICurvePrimitiveProcessor &processor, DSegment1dCP interval) const override;
bool _FractionToPoint(double fraction, DPoint3dR point) const override;
bool _FractionToPoint(double fraction, DPoint3dR point, DVec3dR vector) const override;
bool _FractionToPoint(double fraction, DPoint3dR point, DVec3dR vector, DVec3dR derivative2) const override;
bool _FractionToPoint(double fraction, DPoint3dR point, DVec3dR vector, DVec3dR derivative2, DVec3dR derivative3) const override;
bool _Length(double &length) const override;
bool _Length (RotMatrixCP worldToLocal, double &length) const override;
bool _GetRange(DRange3dR range) const override;
bool _GetRange(DRange3dR range, TransformCR transform) const override;
double _FastMaxAbs() const override;
bool _IsSameStructureAndGeometry (ICurvePrimitiveCR other, double tolerance) const override;
size_t _NumComponent () const override;
DRange1d _ProjectedParameterRange(DRay3dCR ray, double fractionA, double fractionB) const override;
DRange1d _ProjectedParameterRange(DRay3dCR ray) const override;
bool _GetBreakFraction(size_t breakFractionIndex, double &fraction) const override;
bool _AdjustFractionToBreakFraction(double fraction, Rounding::RoundingMode mode, size_t &breakIndex, double &adjustedFraction) const override;
bool _GetMSBsplineCurve(MSBsplineCurveR curve, double fractionA, double fractionB) const override;
bool _AddStrokes(bvector <DPoint3d> &points, IFacetOptionsCR options, bool includeStartPoint, double startFraction, double endFraction ) const override;
bool _AddStrokes(bvector <PathLocationDetail> &points, IFacetOptionsCR options, double startFraction, double endFraction ) const override;
size_t _GetStrokeCount(IFacetOptionsCR options, double startFraction, double endFraction) const override;
bool _SignedDistanceBetweenFractions(double startFraction, double endFraction, double &signedDistance) const override;
bool _SignedDistanceBetweenFractions(RotMatrixCP worldToLocal, double startFraction, double endFraction, double &signedDistance) const override;
bool _PointAtSignedDistanceFromFraction(double startFraction, double signedDistance, bool allowExtension, CurveLocationDetailR location) const override;
bool _PointAtSignedDistanceFromFraction (RotMatrixCP worldToLocal, double startFraction, double signedLength, bool allowExtension, CurveLocationDetailR location) const override;
bool _ClosestPointBounded(DPoint3dCR spacePoint, double &fraction, DPoint3dR curvePoint, bool extend0, bool extend1) const override;
bool _ClosestPointBoundedXY(DPoint3dCR spacePoint, DMatrix4dCP worldToLocal, CurveLocationDetailR location, bool extend0, bool extend1) const override;
void _AnnounceKeyPoints
(
DPoint3dCR spacePoint,                       //!< [in] point to project
CurveKeyPointCollector &collector,           //!< [in] object to receive keypoint announcements
bool extend0,                               //!< true to extend at start of primitives.
bool extend1                                //!< true to extend at end of primitives.
) const override;
bool _TransformInPlace(TransformCR transform) override;
bool _ReverseCurvesInPlace () override;
void _AppendCurvePlaneIntersections(DPlane3dCR plane, bvector<CurveLocationDetailPair> &intersections, double tol) const override;
bool _WireCentroid(double &length, DPoint3dR centroid, double fraction0, double fraction1) const override;

public:
static ICurvePrimitive* Create (DEllipse3dCR ellipse);
};



/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  02/10
+===============+===============+===============+===============+===============+======*/
struct CurvePrimitiveLineString : public ICurvePrimitive
{
protected:

bvector<DPoint3d>   m_points;
explicit CurvePrimitiveLineString(DPoint3dCP points, size_t nPoints);
explicit CurvePrimitiveLineString(bvector<DPoint3d> const &points);
explicit CurvePrimitiveLineString(bvector<DPoint2d> const &points, double z);
explicit CurvePrimitiveLineString();

ICurvePrimitivePtr _Clone() const override;
ICurvePrimitivePtr _CloneComponent (ptrdiff_t componentIndex) const override;
ICurvePrimitivePtr _CloneBetweenFractions ( double fractionA, double fractionB, bool allowExtrapolation ) const override;
CurvePrimitiveType _GetCurvePrimitiveType() const override;
bvector<DPoint3d>* _GetLineStringP () override;
bvector<DPoint3d> const* _GetLineStringCP () const override;
void _Process(ICurvePrimitiveProcessor &processor, DSegment1dCP interval) const override;
bool _IsExtensibleFractionSpace() const override;
bool _IsMappableFractionSpace() const override;
bool _IsFractionSpace() const override;
bool _FractionToPoint(double fraction, DPoint3dR point) const override;
bool _ComponentFractionToPoint(ptrdiff_t componentIndex, double fraction, DPoint3dR point) const override;
bool _FractionToPoint(double fraction, DPoint3dR point, DVec3dR tangent) const override;
bool _FractionToPoint (double f, CurveLocationDetail &point) const override;
bool _ComponentFractionToPoint(ptrdiff_t componentIndex, double fraction, DPoint3dR point, DVec3dR tangent) const override;
bool _FractionToPoint(double fraction, DPoint3dR point, DVec3dR tangent, DVec3dR derivative2) const override;
bool _FractionToPoint(double fraction, DPoint3dR point, DVec3dR tangent, DVec3dR derivative2, DVec3dR derivative3) const override;
bool _FractionToPointWithTwoSidedDerivative (double f, DPoint3dR point, DVec3dR tangentA, DVec3dR tangentB) const override;

bool _FractionToFrenetFrame(double f, TransformR frame) const override;
bool _Length(double &length) const override;
bool _Length (RotMatrixCP worldToLocal, double &length) const override;
bool _GetRange(DRange3dR range) const override;
bool _GetRange(DRange3dR range, TransformCR transform) const override;
double _FastMaxAbs() const override;
bool _IsSameStructureAndGeometry (ICurvePrimitiveCR other, double tolerance) const override;
size_t _NumComponent () const override;
DRange1d _ProjectedParameterRange(DRay3dCR ray) const override;
DRange1d _ProjectedParameterRange(DRay3dCR ray, double fractionA, double fractionB) const override;
bool _GetBreakFraction(size_t breakFractionIndex, double &fraction) const override;
bool _AdjustFractionToBreakFraction(double fraction, Rounding::RoundingMode mode, size_t &breakIndex, double &adjustedFraction) const override;
bool _GetMSBsplineCurve(MSBsplineCurveR curve, double fractionA, double fractionB) const override;
bool _AddStrokes(bvector <DPoint3d> &points, IFacetOptionsCR options, bool includeStartPoint, double startFraction, double endFraction ) const override;
bool _AddStrokes(bvector <PathLocationDetail> &points, IFacetOptionsCR options, double startFraction, double endFraction ) const override;
bool _AddStrokes (DPoint3dDoubleUVCurveArrays &strokes, IFacetOptionsCR options, double startFraction, double endFraction) const override;
size_t _GetStrokeCount(IFacetOptionsCR options, double startFraction, double endFraction) const override;
bool _SignedDistanceBetweenFractions(double startFraction, double endFraction, double &signedDistance) const override;
bool _SignedDistanceBetweenFractions(RotMatrixCP worldToLocal, double startFraction, double endFraction, double &signedDistance) const override;
bool _PointAtSignedDistanceFromFraction(double startFraction, double signedDistance, bool allowExtension, CurveLocationDetailR location) const override;
bool _PointAtSignedDistanceFromFraction (RotMatrixCP worldToLocal, double startFraction, double signedLength, bool allowExtension, CurveLocationDetailR location) const override;
bool _ClosestPointBounded(DPoint3dCR spacePoint, double &fraction, DPoint3dR curvePoint, bool extend0, bool extend1) const override;
bool _ClosestPointBounded(DPoint3dCR spacePoint, CurveLocationDetailR detail, bool extend0, bool extend1) const override;
bool _ClosestPointBoundedXY ( DPoint3dCR spacePoint, DMatrix4dCP worldToLocal, CurveLocationDetailR detail, bool extend0, bool extend1 ) const override;
void _AnnounceKeyPoints
(
DPoint3dCR spacePoint,                       //!< [in] point to project
CurveKeyPointCollector &collector,           //!< [in] object to receive keypoint announcements
bool extend0,                               //!< true to extend at start of primitives.
bool extend1                                //!< true to extend at end of primitives.
) const override;
bool _TransformInPlace (TransformCR transform) override;
bool _ReverseCurvesInPlace () override;
bool _WireCentroid(double &length, DPoint3dR centroid, double fraction0, double fraction1) const override;
void _AppendCurvePlaneIntersections(DPlane3dCR plane, bvector<CurveLocationDetailPair> &intersections, double tol) const override;
void _AppendCurveRangeIntersections (LocalRangeCR range, bvector<PartialCurveDetail> &intersections) const override;
bool _TrySetStart (DPoint3dCR xyz) override;
bool _TrySetEnd (DPoint3dCR xyz) override;
public:
GEOMDLLIMPEXP static ICurvePrimitive* Create (DPoint3dCP points, size_t nPoints);
GEOMDLLIMPEXP static ICurvePrimitive* Create (bvector<DPoint2d> const &points, double z = 0.0);
}; // CurvePrimitiveLineString


/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  02/10
+===============+===============+===============+===============+===============+======*/
struct CurvePrimitivePointString : public CurvePrimitiveLineString
{
protected:


explicit CurvePrimitivePointString(DPoint3dCP points, size_t nPoints); 
explicit CurvePrimitivePointString(bvector<DPoint3d> points);



CurvePrimitiveType _GetCurvePrimitiveType() const override;
bvector<DPoint3d> const* _GetPointStringCP () const override;
bool _GetMSBsplineCurve(MSBsplineCurveR curve, double fractionA, double fractionB) const override;
bvector<DPoint3d> const* _GetLineStringCP () const override;
bvector<DPoint3d> * _GetLineStringP () override;
ICurvePrimitivePtr _Clone() const override;
bool _ClosestPointBounded(DPoint3dCR spacePoint, CurveLocationDetailR detail, bool extend0, bool extend1) const override;
bool _ClosestPointBoundedXY(DPoint3dCR spacePoint, DMatrix4dCP worldToLocal, CurveLocationDetailR detail, bool extend0, bool extend1) const override;
bool _IsSameStructureAndGeometry (ICurvePrimitiveCR other, double tolerance) const override;

public:

GEOMDLLIMPEXP static ICurvePrimitive* Create (DPoint3dCP points, size_t nPoints);
GEOMDLLIMPEXP static ICurvePrimitive* Create (bvector<DPoint3d> &points);

}; // CurvePrimitivePointString