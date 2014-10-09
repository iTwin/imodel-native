/*--------------------------------------------------------------------------------------+
|
|  $Source: src/CGNativeFactoryImplementations.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// (To be inclued in BeXmlCGStreamReader.cpp)

#include "nativeCGFactoryH.h"
struct IGeometryCGFactory : ICGFactory
{
public:

// ===================================================================================
/// <summary>
/// factory base class placeholder to create a Coordinate from explicit args.
virtual IGeometryPtr CreateCoordinate
(
InputParamTypeFor_DPoint3d xyz
) override
    {
    return IGeometry::Create(ICurvePrimitive::CreatePointString (&xyz, 1));
    }

// ===================================================================================

virtual IGeometryPtr CreateLineSegment
(
InputParamTypeFor_DPoint3d startPoint,
InputParamTypeFor_DPoint3d endPoint
) override
    {
    ICurvePrimitivePtr cp = ICurvePrimitive::CreateLine (DSegment3d::From (startPoint, endPoint));
    return IGeometry::Create (cp);
    }

/// <summary>
/// factory base class placeholder to create a EllipticArc from explicit args.
virtual IGeometryPtr CreateEllipticArc
(
InputParamTypeFor_PlacementOriginZX placement,
InputParamTypeFor_double radiusA,
InputParamTypeFor_double radiusB,
InputParamTypeFor_Angle startAngle,
InputParamTypeFor_Angle sweepAngle
) override
    {
    DEllipse3d ellipse = placement.AsDEllipse3d
        (
        radiusA,
        radiusB,
        startAngle.Radians (),
        sweepAngle.Radians ()
        );
    ICurvePrimitivePtr cp = ICurvePrimitive::CreateArc (ellipse);
    return IGeometry::Create (cp);
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
    DPoint3d origin;
    RotMatrix axes;
    ICurvePrimitivePtr arc = ICurvePrimitive::CreateArc (
                DEllipse3d::FromScaledRotMatrix (
                        origin, axes,
                        radiusA, radiusB,
                        0.0, Angle::TwoPi ()));
    CurveVectorPtr area = CurveVector::Create
                (CurveVector::BOUNDARY_TYPE_Outer);
    area->push_back (arc);
    return IGeometry::Create(ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*area));
    }

// ===================================================================================

/// <summary>
/// factory base class placeholder to create a EllipticArc from explicit args.
virtual IGeometryPtr CreateCircularArc
(
InputParamTypeFor_PlacementOriginZX placement,
InputParamTypeFor_double radius,
InputParamTypeFor_Angle startAngle,
InputParamTypeFor_Angle sweepAngle
) override
    {
    DEllipse3d ellipse = placement.AsDEllipse3d
        (
        radius,
        radius,
        startAngle.Radians (),
        sweepAngle.Radians ()
        );
    ICurvePrimitivePtr cp = ICurvePrimitive::CreateArc (ellipse);
    return IGeometry::Create (cp);
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
    DPoint3d origin;
    RotMatrix axes;
    ICurvePrimitivePtr arc = ICurvePrimitive::CreateArc (
                DEllipse3d::FromScaledRotMatrix (
                        origin, axes,
                        radius, radius,
                        0.0, Angle::TwoPi ()));
    CurveVectorPtr area = CurveVector::Create
                (CurveVector::BOUNDARY_TYPE_Outer);
    area->push_back (arc);
    return IGeometry::Create(ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*area));
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
) override
    {
    RotMatrix axes;
    DPoint3d centerA;
    placement.GetFrame (centerA, axes);
    DgnConeDetail coneDetail (centerA, centerB,
                axes, 
                radiusA, radiusB,
                capped);
    ISolidPrimitivePtr sp = ISolidPrimitive::CreateDgnCone (coneDetail);
    return IGeometry::Create (sp);
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
) override
    {
    RotMatrix axes;
    DVec3d vectorZ;
    DPoint3d centerA;
    placement.GetFrame (centerA, axes);
    axes.GetColumn (vectorZ, 2);
    DPoint3d centerB = DPoint3d::FromSumOf (centerA, vectorZ, height);
    DgnConeDetail coneDetail (centerA, centerB,
                axes, 
                radiusA, radiusB,
                capped);
    ISolidPrimitivePtr sp = ISolidPrimitive::CreateDgnCone (coneDetail);
    return IGeometry::Create (sp);
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
) override
    {
    RotMatrix axes;
    DVec3d vectorZ;
    DPoint3d centerA;
    placement.GetFrame (centerA, axes);
    axes.GetColumn (vectorZ, 2);
    DPoint3d centerB = DPoint3d::FromSumOf (centerA, vectorZ, height);
    DgnConeDetail coneDetail (centerA, centerB,
                axes, 
                radius, radius,
                capped);
    ISolidPrimitivePtr sp = ISolidPrimitive::CreateDgnCone (coneDetail);
    return IGeometry::Create (sp);
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
) override
    {
    DPoint3d origin;
    RotMatrix axes;
    DVec3d vectorX, vectorY, vectorZ;
    placement.GetFrame (origin, axes);
    axes.GetColumns (vectorX, vectorY, vectorZ);
    DPoint3d baseOrigin = DPoint3d::FromProduct (origin, axes, cornerA.x, cornerA.y, cornerA.z);
    DPoint3d topOrigin  = DPoint3d::FromProduct (origin, axes, cornerA.x, cornerA.y, cornerB.z);
    double dx = cornerB.x - cornerA.x;
    double dy = cornerB.y - cornerA.y;
    DgnBoxDetail detail (baseOrigin, topOrigin, vectorX, vectorY, dx, dy, dx, dy, capped);
    ISolidPrimitivePtr sp = ISolidPrimitive::CreateDgnBox (detail);
    return IGeometry::Create (sp);
    }

// ===================================================================================

/// <summary>
/// factory base class placeholder to create a Sphere from explicit args.
virtual IGeometryPtr CreateSphere
(
InputParamTypeFor_PlacementOriginZX placement,
InputParamTypeFor_double radius
) override
    {
    DPoint3d center;
    RotMatrix axes;
    placement.GetFrame (center, axes);
    DgnSphereDetail detail (center, axes, radius);
    return IGeometry::Create(ISolidPrimitive::CreateDgnSphere (detail));
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
) override
    {
    DPoint3d center;
    RotMatrix axes;
    placement.GetFrame (center, axes);
    DVec3d vectorX, vectorY, vectorZ;
    axes.GetColumns (vectorX, vectorY, vectorZ);
    DVec3d vector0 = vectorX;
    DVec3d vector90 = vectorY;
    double startRadians = startAngle.Radians ();
    double sweepRadians = sweepAngle.Radians ();
    if (startRadians != 0.0)
        {
        double c = cos (startRadians);
        double s = sin (startRadians);
        vector0.SumOf  (vectorX,  c, vectorY, s);
        vector90.SumOf (vectorX, -s, vectorY, c);
        }
    DgnTorusPipeDetail detail (center,
                vector0, vector90, 
                radiusA, radiusB,
                sweepRadians,
                capped);
    return IGeometry::Create(ISolidPrimitive::CreateDgnTorusPipe (detail));
    }

// ===================================================================================
virtual IGeometryPtr CreateLineString
(
bvector<DPoint3d> const &points
) override
    {
    return IGeometry::Create(ICurvePrimitive::CreateLineString(points));
    }
#define CGFactory_CreateLineString
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
) override
    {
    PolyfaceHeaderPtr polyface = PolyfaceHeader::CreateVariableSizeIndexed ();

    CopyToBlockedVector (CoordArray, polyface->Point ());
    CopyToBlockedVector (CoordIndexArray, polyface->PointIndex ());

    CopyToBlockedVector (ParamArray, polyface->Param ());
    CopyToBlockedVector (ParamIndexArray, polyface->ParamIndex ());

    CopyToBlockedVector (NormalArray, polyface->Normal ());
    CopyToBlockedVector (NormalIndexArray, polyface->NormalIndex ());

    // Colors have to be reformatted ...
    if (ColorArray.size () > 0)
        {
        bvector<RgbFactor> dest = polyface->DoubleColor ();
        dest.reserve (ColorArray.size ());
        for (DVec3d const &data : ColorArray)
            {
            dest.push_back (RgbFactor::From (data));
            }
        }
    CopyToBlockedVector (ColorIndexArray, polyface->ColorIndex ());

    return IGeometry::Create (polyface);
    }
};
