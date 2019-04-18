/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
struct ICGFactory
{

public:
    // scream.   IGeoemtryPtr cannot handle trees.   
    // ad hoc selected parse implementations can stash their deep members here ...
    bvector<IGeometryPtr> m_groupMembers;
    



/// <summary> factory base class placeholder to promote a LineSegment detail structure to IGeometryPtr.</summary>
GEOMAPI_VIRTUAL IGeometryPtr Create(CGLineSegmentDetail &detail){return nullptr;}




/// <summary> factory base class placeholder to promote a CircularArc detail structure to IGeometryPtr.</summary>
GEOMAPI_VIRTUAL IGeometryPtr Create(CGCircularArcDetail &detail){return nullptr;}




/// <summary> factory base class placeholder to promote a DgnBox detail structure to IGeometryPtr.</summary>
GEOMAPI_VIRTUAL IGeometryPtr Create(CGDgnBoxDetail &detail){return nullptr;}




/// <summary> factory base class placeholder to promote a DgnSphere detail structure to IGeometryPtr.</summary>
GEOMAPI_VIRTUAL IGeometryPtr Create(CGDgnSphereDetail &detail){return nullptr;}




/// <summary> factory base class placeholder to promote a DgnCone detail structure to IGeometryPtr.</summary>
GEOMAPI_VIRTUAL IGeometryPtr Create(CGDgnConeDetail &detail){return nullptr;}




/// <summary> factory base class placeholder to promote a DgnTorusPipe detail structure to IGeometryPtr.</summary>
GEOMAPI_VIRTUAL IGeometryPtr Create(CGDgnTorusPipeDetail &detail){return nullptr;}




/// <summary> factory base class placeholder to promote a Block detail structure to IGeometryPtr.</summary>
GEOMAPI_VIRTUAL IGeometryPtr Create(CGBlockDetail &detail){return nullptr;}




/// <summary> factory base class placeholder to promote a CircularCone detail structure to IGeometryPtr.</summary>
GEOMAPI_VIRTUAL IGeometryPtr Create(CGCircularConeDetail &detail){return nullptr;}




/// <summary> factory base class placeholder to promote a CircularCylinder detail structure to IGeometryPtr.</summary>
GEOMAPI_VIRTUAL IGeometryPtr Create(CGCircularCylinderDetail &detail){return nullptr;}




/// <summary> factory base class placeholder to promote a CircularDisk detail structure to IGeometryPtr.</summary>
GEOMAPI_VIRTUAL IGeometryPtr Create(CGCircularDiskDetail &detail){return nullptr;}




/// <summary> factory base class placeholder to promote a Coordinate detail structure to IGeometryPtr.</summary>
GEOMAPI_VIRTUAL IGeometryPtr Create(CGCoordinateDetail &detail){return nullptr;}




/// <summary> factory base class placeholder to promote a EllipticArc detail structure to IGeometryPtr.</summary>
GEOMAPI_VIRTUAL IGeometryPtr Create(CGEllipticArcDetail &detail){return nullptr;}




/// <summary> factory base class placeholder to promote a EllipticDisk detail structure to IGeometryPtr.</summary>
GEOMAPI_VIRTUAL IGeometryPtr Create(CGEllipticDiskDetail &detail){return nullptr;}




/// <summary> factory base class placeholder to promote a SingleLineText detail structure to IGeometryPtr.</summary>
GEOMAPI_VIRTUAL IGeometryPtr Create(CGSingleLineTextDetail &detail){return nullptr;}




/// <summary> factory base class placeholder to promote a SkewedCone detail structure to IGeometryPtr.</summary>
GEOMAPI_VIRTUAL IGeometryPtr Create(CGSkewedConeDetail &detail){return nullptr;}




/// <summary> factory base class placeholder to promote a Sphere detail structure to IGeometryPtr.</summary>
GEOMAPI_VIRTUAL IGeometryPtr Create(CGSphereDetail &detail){return nullptr;}




/// <summary> factory base class placeholder to promote a TorusPipe detail structure to IGeometryPtr.</summary>
GEOMAPI_VIRTUAL IGeometryPtr Create(CGTorusPipeDetail &detail){return nullptr;}




/// <summary> factory base class placeholder to promote a Vector detail structure to IGeometryPtr.</summary>
GEOMAPI_VIRTUAL IGeometryPtr Create(CGVectorDetail &detail){return nullptr;}






GEOMAPI_VIRTUAL IGeometryPtr Create(CGIndexedMeshDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGAdjacentSurfacePatchesDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGBsplineCurveDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGBsplineSurfaceDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGCurveChainDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGCurveGroupDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGCurveReferenceDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGGroupDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGInterpolatingCurveDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGLineStringDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGOperationDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGParametricSurfacePatchDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGPointChainDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGPointGroupDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGPolygonDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGPrimitiveCurveReferenceDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGPartialCurveDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGSharedGroupDefDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGSharedGroupInstanceDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGShelledSolidDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGSolidBySweptSurfaceDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGSolidByRuledSweepDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGSurfaceByRuledSweepDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGSolidGroupDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGSpiralDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGSurfaceBySweptCurveDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGSurfaceGroupDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGSurfacePatchDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGTransformedGeometryDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGDgnExtrusionDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGDgnRotationalSweepDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGDgnRuledSweepDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGTransitionSpiralDetail &detail){return nullptr;}



GEOMAPI_VIRTUAL IGeometryPtr Create(CGDgnCurveVectorDetail &detail){return nullptr;}


};