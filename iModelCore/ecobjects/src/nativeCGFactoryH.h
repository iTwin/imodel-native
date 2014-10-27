struct ICGFactory
{





/// <summary> factory base class placeholder to promote a LineSegment detail structure to IGeometryPtr.</summary>
virtual IGeometryPtr Create (CGLineSegmentDetail &detail)  {return nullptr;}




/// <summary> factory base class placeholder to promote a CircularArc detail structure to IGeometryPtr.</summary>
virtual IGeometryPtr Create (CGCircularArcDetail &detail)  {return nullptr;}




/// <summary> factory base class placeholder to promote a DgnBox detail structure to IGeometryPtr.</summary>
virtual IGeometryPtr Create (CGDgnBoxDetail &detail)  {return nullptr;}




/// <summary> factory base class placeholder to promote a DgnSphere detail structure to IGeometryPtr.</summary>
virtual IGeometryPtr Create (CGDgnSphereDetail &detail)  {return nullptr;}




/// <summary> factory base class placeholder to promote a DgnCone detail structure to IGeometryPtr.</summary>
virtual IGeometryPtr Create (CGDgnConeDetail &detail)  {return nullptr;}




/// <summary> factory base class placeholder to promote a DgnTorusPipe detail structure to IGeometryPtr.</summary>
virtual IGeometryPtr Create (CGDgnTorusPipeDetail &detail)  {return nullptr;}




/// <summary> factory base class placeholder to promote a Block detail structure to IGeometryPtr.</summary>
virtual IGeometryPtr Create (CGBlockDetail &detail)  {return nullptr;}




/// <summary> factory base class placeholder to promote a CircularCone detail structure to IGeometryPtr.</summary>
virtual IGeometryPtr Create (CGCircularConeDetail &detail)  {return nullptr;}




/// <summary> factory base class placeholder to promote a CircularCylinder detail structure to IGeometryPtr.</summary>
virtual IGeometryPtr Create (CGCircularCylinderDetail &detail)  {return nullptr;}




/// <summary> factory base class placeholder to promote a CircularDisk detail structure to IGeometryPtr.</summary>
virtual IGeometryPtr Create (CGCircularDiskDetail &detail)  {return nullptr;}




/// <summary> factory base class placeholder to promote a Coordinate detail structure to IGeometryPtr.</summary>
virtual IGeometryPtr Create (CGCoordinateDetail &detail)  {return nullptr;}




/// <summary> factory base class placeholder to promote a EllipticArc detail structure to IGeometryPtr.</summary>
virtual IGeometryPtr Create (CGEllipticArcDetail &detail)  {return nullptr;}




/// <summary> factory base class placeholder to promote a EllipticDisk detail structure to IGeometryPtr.</summary>
virtual IGeometryPtr Create (CGEllipticDiskDetail &detail)  {return nullptr;}




/// <summary> factory base class placeholder to promote a SingleLineText detail structure to IGeometryPtr.</summary>
virtual IGeometryPtr Create (CGSingleLineTextDetail &detail)  {return nullptr;}




/// <summary> factory base class placeholder to promote a SkewedCone detail structure to IGeometryPtr.</summary>
virtual IGeometryPtr Create (CGSkewedConeDetail &detail)  {return nullptr;}




/// <summary> factory base class placeholder to promote a Sphere detail structure to IGeometryPtr.</summary>
virtual IGeometryPtr Create (CGSphereDetail &detail)  {return nullptr;}




/// <summary> factory base class placeholder to promote a TorusPipe detail structure to IGeometryPtr.</summary>
virtual IGeometryPtr Create (CGTorusPipeDetail &detail)  {return nullptr;}




/// <summary> factory base class placeholder to promote a Vector detail structure to IGeometryPtr.</summary>
virtual IGeometryPtr Create (CGVectorDetail &detail)  {return nullptr;}






virtual IGeometryPtr Create (CGIndexedMeshDetail &detail) {return nullptr;}



virtual IGeometryPtr Create (CGAdjacentSurfacePatchesDetail &detail) {return nullptr;}



virtual IGeometryPtr Create (CGBsplineCurveDetail &detail) {return nullptr;}



virtual IGeometryPtr Create (CGBsplineSurfaceDetail &detail) {return nullptr;}



virtual IGeometryPtr Create (CGCurveChainDetail &detail) {return nullptr;}



virtual IGeometryPtr Create (CGCurveGroupDetail &detail) {return nullptr;}



virtual IGeometryPtr Create (CGCurveReferenceDetail &detail) {return nullptr;}



virtual IGeometryPtr Create (CGGroupDetail &detail) {return nullptr;}



virtual IGeometryPtr Create (CGInterpolatingCurveDetail &detail) {return nullptr;}



virtual IGeometryPtr Create (CGLineStringDetail &detail) {return nullptr;}



virtual IGeometryPtr Create (CGOperationDetail &detail) {return nullptr;}



virtual IGeometryPtr Create (CGParametricSurfacePatchDetail &detail) {return nullptr;}



virtual IGeometryPtr Create (CGPointChainDetail &detail) {return nullptr;}



virtual IGeometryPtr Create (CGPointGroupDetail &detail) {return nullptr;}



virtual IGeometryPtr Create (CGPolygonDetail &detail) {return nullptr;}



virtual IGeometryPtr Create (CGPrimitiveCurveReferenceDetail &detail) {return nullptr;}



virtual IGeometryPtr Create (CGSharedGroupDefDetail &detail) {return nullptr;}



virtual IGeometryPtr Create (CGSharedGroupInstanceDetail &detail) {return nullptr;}



virtual IGeometryPtr Create (CGShelledSolidDetail &detail) {return nullptr;}



virtual IGeometryPtr Create (CGSolidBySweptSurfaceDetail &detail) {return nullptr;}



virtual IGeometryPtr Create (CGSolidByRuledSweepDetail &detail) {return nullptr;}



virtual IGeometryPtr Create (CGSurfaceByRuledSweepDetail &detail) {return nullptr;}



virtual IGeometryPtr Create (CGSolidGroupDetail &detail) {return nullptr;}



virtual IGeometryPtr Create (CGSpiralDetail &detail) {return nullptr;}



virtual IGeometryPtr Create (CGSurfaceBySweptCurveDetail &detail) {return nullptr;}



virtual IGeometryPtr Create (CGSurfaceGroupDetail &detail) {return nullptr;}



virtual IGeometryPtr Create (CGSurfacePatchDetail &detail) {return nullptr;}



virtual IGeometryPtr Create (CGTransformedGeometryDetail &detail) {return nullptr;}



virtual IGeometryPtr Create (CGDgnExtrusionDetail &detail) {return nullptr;}



virtual IGeometryPtr Create (CGDgnRotationalSweepDetail &detail) {return nullptr;}



virtual IGeometryPtr Create (CGDgnRuledSweepDetail &detail) {return nullptr;}



virtual IGeometryPtr Create (CGTransitionSpiralDetail &detail) {return nullptr;}


};