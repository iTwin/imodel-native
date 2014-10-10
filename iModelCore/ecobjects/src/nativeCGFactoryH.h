struct ICGFactory
{




// ===================================================================================

/// <summary>
/// factory base class placeholder to create a LineSegment from explicit args.
virtual IGeometryPtr CreateLineSegment
(
InputParamTypeFor_DPoint3d startPoint,
InputParamTypeFor_DPoint3d endPoint
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a CircularArc from explicit args.
virtual IGeometryPtr CreateCircularArc
(
InputParamTypeFor_PlacementOriginZX placement,
InputParamTypeFor_double radius,
InputParamTypeFor_Angle startAngle,
InputParamTypeFor_Angle sweepAngle
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a DgnBox from explicit args.
virtual IGeometryPtr CreateDgnBox
(
InputParamTypeFor_DPoint3d baseOrigin,
InputParamTypeFor_DPoint3d topOrigin,
InputParamTypeFor_DVector3d vectorX,
InputParamTypeFor_DVector3d vectorY,
InputParamTypeFor_double baseX,
InputParamTypeFor_double baseY,
InputParamTypeFor_double topX,
InputParamTypeFor_double topY,
InputParamTypeFor_bool capped
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a DgnSphere from explicit args.
virtual IGeometryPtr CreateDgnSphere
(
InputParamTypeFor_DPoint3d center,
InputParamTypeFor_DVector3d vectorX,
InputParamTypeFor_DVector3d vectorZ,
InputParamTypeFor_double radiusXY,
InputParamTypeFor_double radiusZ,
InputParamTypeFor_Angle startLatitude,
InputParamTypeFor_Angle latitudeSweep,
InputParamTypeFor_bool capped
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a DgnCone from explicit args.
virtual IGeometryPtr CreateDgnCone
(
InputParamTypeFor_DPoint3d centerA,
InputParamTypeFor_DPoint3d centerB,
InputParamTypeFor_DVector3d vectorX,
InputParamTypeFor_DVector3d vectorY,
InputParamTypeFor_double radiusA,
InputParamTypeFor_double radiusB,
InputParamTypeFor_bool capped
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a DgnTorusPipe from explicit args.
virtual IGeometryPtr CreateDgnTorusPipe
(
InputParamTypeFor_DPoint3d center,
InputParamTypeFor_DVector3d vectorX,
InputParamTypeFor_DVector3d vectorY,
InputParamTypeFor_double majorRadius,
InputParamTypeFor_double minorRadius,
InputParamTypeFor_Angle sweepAngle,
InputParamTypeFor_bool capped
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a Block from explicit args.
virtual IGeometryPtr CreateBlock
(
InputParamTypeFor_PlacementOriginZX placement,
InputParamTypeFor_DPoint3d cornerA,
InputParamTypeFor_DPoint3d cornerB,
InputParamTypeFor_bool capped
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a CircularCone from explicit args.
virtual IGeometryPtr CreateCircularCone
(
InputParamTypeFor_PlacementOriginZX placement,
InputParamTypeFor_double height,
InputParamTypeFor_double radiusA,
InputParamTypeFor_double radiusB,
InputParamTypeFor_bool capped
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a CircularCylinder from explicit args.
virtual IGeometryPtr CreateCircularCylinder
(
InputParamTypeFor_PlacementOriginZX placement,
InputParamTypeFor_double height,
InputParamTypeFor_double radius,
InputParamTypeFor_bool capped
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a CircularDisk from explicit args.
virtual IGeometryPtr CreateCircularDisk
(
InputParamTypeFor_PlacementOriginZX placement,
InputParamTypeFor_double radius
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a Coordinate from explicit args.
virtual IGeometryPtr CreateCoordinate
(
InputParamTypeFor_DPoint3d xyz
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a EllipticArc from explicit args.
virtual IGeometryPtr CreateEllipticArc
(
InputParamTypeFor_PlacementOriginZX placement,
InputParamTypeFor_double radiusA,
InputParamTypeFor_double radiusB,
InputParamTypeFor_Angle startAngle,
InputParamTypeFor_Angle sweepAngle
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a EllipticDisk from explicit args.
virtual IGeometryPtr CreateEllipticDisk
(
InputParamTypeFor_PlacementOriginZX placement,
InputParamTypeFor_double radiusA,
InputParamTypeFor_double radiusB
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a SingleLineText from explicit args.
virtual IGeometryPtr CreateSingleLineText
(
InputParamTypeFor_PlacementOriginZX placement,
InputParamTypeFor_String textString,
InputParamTypeFor_String fontName,
InputParamTypeFor_double characterXSize,
InputParamTypeFor_double characterYSize,
InputParamTypeFor_int justification
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a SkewedCone from explicit args.
virtual IGeometryPtr CreateSkewedCone
(
InputParamTypeFor_PlacementOriginZX placement,
InputParamTypeFor_DPoint3d centerB,
InputParamTypeFor_double radiusA,
InputParamTypeFor_double radiusB,
InputParamTypeFor_bool capped
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a Sphere from explicit args.
virtual IGeometryPtr CreateSphere
(
InputParamTypeFor_PlacementOriginZX placement,
InputParamTypeFor_double radius
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a TorusPipe from explicit args.
virtual IGeometryPtr CreateTorusPipe
(
InputParamTypeFor_PlacementOriginZX placement,
InputParamTypeFor_double radiusA,
InputParamTypeFor_double radiusB,
InputParamTypeFor_Angle startAngle,
InputParamTypeFor_Angle sweepAngle,
InputParamTypeFor_bool capped
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a Vector from explicit args.
virtual IGeometryPtr CreateVector
(
InputParamTypeFor_DPoint3d xyz,
InputParamTypeFor_DVector3d vector
)
    {
    return nullptr;
    }






// ===================================================================================

/// <summary>
/// factory base class placeholder to create a IndexedMesh from explicit args.
virtual IGeometryPtr CreateIndexedMesh
(
bvector<DPoint3d> const &CoordArray,
bvector<int> const &CoordIndexArray,
bvector<DPoint2d> const &ParamArray,
bvector<int> const &ParamIndexArray,
bvector<DVector3d> const &NormalArray,
bvector<int> const &NormalIndexArray,
bvector<DVector3d> const &ColorArray,
bvector<int> const &ColorIndexArray
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a AdjacentSurfacePatches from explicit args.
virtual IGeometryPtr CreateAdjacentSurfacePatches
(
bvector<IGeometryPtr> const &PatchArray
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a BsplineCurve from explicit args.
virtual IGeometryPtr CreateBsplineCurve
(
InputParamTypeFor_int order,
InputParamTypeFor_bool closed,
bvector<DPoint3d> const &ControlPointArray,
bvector<double> const &WeightArray,
bvector<double> const &KnotArray
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a BsplineSurface from explicit args.
virtual IGeometryPtr CreateBsplineSurface
(
InputParamTypeFor_int orderU,
InputParamTypeFor_bool closedU,
InputParamTypeFor_int numUControlPoint,
InputParamTypeFor_int orderV,
InputParamTypeFor_bool closedV,
InputParamTypeFor_int numVControlPoint,
bvector<DPoint3d> const &ControlPointArray,
bvector<double> const &WeightArray,
bvector<double> const &KnotUArray,
bvector<double> const &KnotVArray
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a CurveChain from explicit args.
virtual IGeometryPtr CreateCurveChain
(
bvector<ICurvePrimitivePtr> const &CurveArray
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a CurveGroup from explicit args.
virtual IGeometryPtr CreateCurveGroup
(
bvector<IGeometryPtr> const &CurveArray
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a CurveReference from explicit args.
virtual IGeometryPtr CreateCurveReference
(
InputParamTypeFor_bool reversed,
InputParamTypeFor_ICurve parentCurve
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a Group from explicit args.
virtual IGeometryPtr CreateGroup
(
bvector<IGeometryPtr> const &MemberArray
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a InterpolatingCurve from explicit args.
virtual IGeometryPtr CreateInterpolatingCurve
(
InputParamTypeFor_int endConditionCode,
InputParamTypeFor_int knotCode,
InputParamTypeFor_DVector3d startVector,
InputParamTypeFor_DVector3d endVector,
bvector<DPoint3d> const &PointArray,
bvector<double> const &KnotArray
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a LineString from explicit args.
virtual IGeometryPtr CreateLineString
(
bvector<DPoint3d> const &PointArray
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a Operation from explicit args.
virtual IGeometryPtr CreateOperation
(
InputParamTypeFor_String name,
bvector<IGeometryPtr> const &MemberArray
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a ParametricSurfacePatch from explicit args.
virtual IGeometryPtr CreateParametricSurfacePatch
(
InputParamTypeFor_LoopType loopType,
InputParamTypeFor_IParametricSurface surface,
bvector<IGeometryPtr> const &CurveChainArray
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a PointChain from explicit args.
virtual IGeometryPtr CreatePointChain
(
bvector<IGeometryPtr> const &PointArray
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a PointGroup from explicit args.
virtual IGeometryPtr CreatePointGroup
(
bvector<IGeometryPtr> const &MemberArray
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a Polygon from explicit args.
virtual IGeometryPtr CreatePolygon
(
bvector<DPoint3d> const &PointArray
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a PrimitiveCurveReference from explicit args.
virtual IGeometryPtr CreatePrimitiveCurveReference
(
InputParamTypeFor_bool reversed,
InputParamTypeFor_IPrimitiveCurve parentCurve
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a SharedGroupDef from explicit args.
virtual IGeometryPtr CreateSharedGroupDef
(
InputParamTypeFor_String name,
InputParamTypeFor_IGeometry geometry
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a SharedGroupInstance from explicit args.
virtual IGeometryPtr CreateSharedGroupInstance
(
InputParamTypeFor_String sharedGroupName,
InputParamTypeFor_Transform transform
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a ShelledSolid from explicit args.
virtual IGeometryPtr CreateShelledSolid
(
InputParamTypeFor_ISurface BoundingSurface
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a SolidBySweptSurface from explicit args.
virtual IGeometryPtr CreateSolidBySweptSurface
(
InputParamTypeFor_ISurface baseGeometry,
InputParamTypeFor_ICurve railCurve
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a SolidByRuledSweep from explicit args.
virtual IGeometryPtr CreateSolidByRuledSweep
(
bvector<IGeometryPtr> const &SectionArray
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a SurfaceByRuledSweep from explicit args.
virtual IGeometryPtr CreateSurfaceByRuledSweep
(
bvector<IGeometryPtr> const &SectionArray
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a SolidGroup from explicit args.
virtual IGeometryPtr CreateSolidGroup
(
bvector<IGeometryPtr> const &SolidArray
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a Spiral from explicit args.
virtual IGeometryPtr CreateSpiral
(
InputParamTypeFor_String spiralType,
InputParamTypeFor_DPoint3d startPoint,
InputParamTypeFor_Angle startBearing,
InputParamTypeFor_double startCurvature,
InputParamTypeFor_DPoint3d endPoint,
InputParamTypeFor_Angle endBearing,
InputParamTypeFor_double endCurvature,
InputParamTypeFor_IGeometry geometry
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a SurfaceBySweptCurve from explicit args.
virtual IGeometryPtr CreateSurfaceBySweptCurve
(
InputParamTypeFor_ICurve baseGeometry,
InputParamTypeFor_ICurve railCurve
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a SurfaceGroup from explicit args.
virtual IGeometryPtr CreateSurfaceGroup
(
bvector<IGeometryPtr> const &SurfaceArray
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a SurfacePatch from explicit args.
virtual IGeometryPtr CreateSurfacePatch
(
InputParamTypeFor_ICurveChain exteriorLoop,
bvector<IGeometryPtr> const &HoleLoopArray
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a TransformedGeometry from explicit args.
virtual IGeometryPtr CreateTransformedGeometry
(
InputParamTypeFor_Transform transform,
InputParamTypeFor_IGeometry geometry
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a DgnExtrusion from explicit args.
virtual IGeometryPtr CreateDgnExtrusion
(
InputParamTypeFor_DVector3d extrusionVector,
InputParamTypeFor_bool capped,
InputParamTypeFor_ISweepable baseGeometry
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a DgnRotationalSweep from explicit args.
virtual IGeometryPtr CreateDgnRotationalSweep
(
InputParamTypeFor_DPoint3d center,
InputParamTypeFor_DVector3d axis,
InputParamTypeFor_bool capped,
InputParamTypeFor_ISweepable baseGeometry
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a DgnRuledSweep from explicit args.
virtual IGeometryPtr CreateDgnRuledSweep
(
InputParamTypeFor_bool capped,
bvector<IGeometryPtr> const &ContourArray
)
    {
    return nullptr;
    }



// ===================================================================================

/// <summary>
/// factory base class placeholder to create a TransitionSpiral from explicit args.
virtual IGeometryPtr CreateTransitionSpiral
(
InputParamTypeFor_String spiralType,
InputParamTypeFor_PlacementOriginZX placement,
InputParamTypeFor_Angle startBearing,
InputParamTypeFor_double startRadius,
InputParamTypeFor_Angle endBearing,
InputParamTypeFor_double endRadius,
InputParamTypeFor_double activeStartFraction,
InputParamTypeFor_double activeEndFraction,
InputParamTypeFor_IGeometry geometry
)
    {
    return nullptr;
    }


};