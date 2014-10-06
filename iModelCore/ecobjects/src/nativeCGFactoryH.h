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
InputParamTypeFor_IPlacement placement,
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
InputParamTypeFor_IPlacement placement,
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
InputParamTypeFor_IPlacement placement,
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
InputParamTypeFor_IPlacement placement,
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
InputParamTypeFor_IPlacement placement,
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
InputParamTypeFor_IPlacement placement,
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
InputParamTypeFor_IPlacement placement,
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
InputParamTypeFor_IPlacement placement,
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
InputParamTypeFor_IPlacement placement,
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
InputParamTypeFor_IPlacement placement,
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
InputParamTypeFor_IPlacement placement,
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



//before expandAllNodeTypes
//<expandFileLater>in\expandAllNodeTypes.in</expandFileLater>
//after expandAllNodeTypes
};