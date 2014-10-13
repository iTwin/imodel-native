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

// ===================================================  ================================
virtual IGeometryPtr Create (CGCoordinateDetail &detail) override
    {
    return IGeometry::Create(ICurvePrimitive::CreatePointString (&detail.xyz, 1));
    }

// ===================================================================================
virtual IGeometryPtr Create (CGLineSegmentDetail &detail) override
    {
    ICurvePrimitivePtr cp = ICurvePrimitive::CreateLine (DSegment3d::From (detail.startPoint, detail.endPoint));
    return IGeometry::Create (cp);
    }

// ===================================================================================
virtual IGeometryPtr Create (CGEllipticArcDetail &detail) override
    {
    DEllipse3d ellipse = detail.placement.AsDEllipse3d
        (
        detail.radiusA,
        detail.radiusB,
        detail.startAngle.Radians (),
        detail.sweepAngle.Radians ()
        );
    ICurvePrimitivePtr cp = ICurvePrimitive::CreateArc (ellipse);
    return IGeometry::Create (cp);
    }

// ===================================================================================
virtual IGeometryPtr Create (CGEllipticDiskDetail &detail) override
    {
    DPoint3d origin;
    RotMatrix axes;
    detail.placement.GetFrame (origin, axes);
    ICurvePrimitivePtr arc = ICurvePrimitive::CreateArc (
                DEllipse3d::FromScaledRotMatrix (
                        origin, axes,
                        detail.radiusA, detail.radiusB,
                        0.0, Angle::TwoPi ()));
    CurveVectorPtr area = CurveVector::Create
                (CurveVector::BOUNDARY_TYPE_Outer);
    area->push_back (arc);
    return IGeometry::Create(ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*area));
    }

// ===================================================================================
virtual IGeometryPtr Create (CGCircularArcDetail &detail) override
    {
    DEllipse3d ellipse = detail.placement.AsDEllipse3d
        (
        detail.radius,
        detail.radius,
        detail.startAngle.Radians (),
        detail.sweepAngle.Radians ()
        );
    ICurvePrimitivePtr cp = ICurvePrimitive::CreateArc (ellipse);
    return IGeometry::Create (cp);
    }

// ===================================================================================
virtual IGeometryPtr Create (CGCircularDiskDetail &detail) override
    {
    DPoint3d origin;
    RotMatrix axes;
    detail.placement.GetFrame (origin, axes);
    ICurvePrimitivePtr arc = ICurvePrimitive::CreateArc (
                DEllipse3d::FromScaledRotMatrix (
                        origin, axes,
                        detail.radius, detail.radius,
                        0.0, Angle::TwoPi ()));
    CurveVectorPtr area = CurveVector::Create
                (CurveVector::BOUNDARY_TYPE_Outer);
    area->push_back (arc);
    return IGeometry::Create(ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*area));
    }

// ===================================================================================
virtual IGeometryPtr Create (CGSkewedConeDetail &detail) override
    {
    RotMatrix axes;
    DPoint3d centerA;
    detail.placement.GetFrame (centerA, axes);
    DgnConeDetail coneDetail (centerA, detail.centerB,
                axes, 
                detail.radiusA, detail.radiusB,
                detail.capped);
    ISolidPrimitivePtr sp = ISolidPrimitive::CreateDgnCone (coneDetail);
    return IGeometry::Create (sp);
    }

// ===================================================================================

/// <summary>
/// factory base class placeholder to create a CircularCone from explicit args.
virtual IGeometryPtr Create (CGCircularConeDetail &detail) override
    {
    RotMatrix axes;
    DVec3d vectorZ;
    DPoint3d centerA;
    detail.placement.GetFrame (centerA, axes);
    axes.GetColumn (vectorZ, 2);
    DPoint3d centerB = DPoint3d::FromSumOf (centerA, vectorZ, detail.height);
    DgnConeDetail coneDetail (centerA, centerB,
                axes, 
                detail.radiusA, detail.radiusB,
                detail.capped);
    ISolidPrimitivePtr sp = ISolidPrimitive::CreateDgnCone (coneDetail);
    return IGeometry::Create (sp);
    }

// ===================================================================================

/// <summary>
/// factory base class placeholder to create a CircularCylinder from explicit args.
virtual IGeometryPtr Create(CGCircularCylinderDetail &detail) override
    {
    RotMatrix axes;
    DVec3d vectorZ;
    DPoint3d centerA;
    detail.placement.GetFrame (centerA, axes);
    axes.GetColumn (vectorZ, 2);
    DPoint3d centerB = DPoint3d::FromSumOf (centerA, vectorZ, detail.height);
    DgnConeDetail coneDetail (centerA, centerB,
                axes, 
                detail.radius, detail.radius,
                detail.capped);
    ISolidPrimitivePtr sp = ISolidPrimitive::CreateDgnCone (coneDetail);
    return IGeometry::Create (sp);
    }

// ===================================================================================

/// <summary>
/// factory base class placeholder to create a Block from explicit args.
virtual IGeometryPtr Create (CGBlockDetail &detail) override
    {
    DPoint3d origin;
    RotMatrix axes;
    DVec3d vectorX, vectorY, vectorZ;
    detail.placement.GetFrame (origin, axes);
    axes.GetColumns (vectorX, vectorY, vectorZ);
    DPoint3d baseOrigin = DPoint3d::FromProduct (origin, axes, detail.cornerA.x, detail.cornerA.y, detail.cornerA.z);
    DPoint3d topOrigin  = DPoint3d::FromProduct (origin, axes, detail.cornerA.x, detail.cornerA.y, detail.cornerB.z);
    double dx = detail.cornerB.x - detail.cornerA.x;
    double dy = detail.cornerB.y - detail.cornerA.y;
    DgnBoxDetail boxDetail (baseOrigin, topOrigin, vectorX, vectorY, dx, dy, dx, dy, detail.capped);
    ISolidPrimitivePtr sp = ISolidPrimitive::CreateDgnBox (boxDetail);
    return IGeometry::Create (sp);
    }

// ===================================================================================
/// <summary> factory base class placeholder to create a Sphere from explicit args.</summary>
virtual IGeometryPtr Create (CGSphereDetail &detail) override
    {
    DPoint3d center;
    RotMatrix axes;
    detail.placement.GetFrame (center, axes);
    DgnSphereDetail sphereDetail (center, axes, detail.radius);
    return IGeometry::Create(ISolidPrimitive::CreateDgnSphere (sphereDetail));
    }

// ===================================================================================
/// <summary>
/// factory base class placeholder to create a TorusPipe from explicit args.
virtual IGeometryPtr Create (CGTorusPipeDetail &detail) override
    {
    DPoint3d center;
    RotMatrix axes;
    detail.placement.GetFrame (center, axes);
    DVec3d vectorX, vectorY, vectorZ;
    axes.GetColumns (vectorX, vectorY, vectorZ);
    DVec3d vector0 = vectorX;
    DVec3d vector90 = vectorY;
    double startRadians = detail.startAngle.Radians ();
    double sweepRadians = detail.sweepAngle.Radians ();
    if (startRadians != 0.0)
        {
        double c = cos (startRadians);
        double s = sin (startRadians);
        vector0.SumOf  (vectorX,  c, vectorY, s);
        vector90.SumOf (vectorX, -s, vectorY, c);
        }
    DgnTorusPipeDetail dgnDetail (center,
                vector0, vector90, 
                detail.radiusA, detail.radiusB,
                sweepRadians,
                detail.capped);
    return IGeometry::Create(ISolidPrimitive::CreateDgnTorusPipe (dgnDetail));
    }

// ===================================================================================
virtual IGeometryPtr Create (CGLineStringDetail &cgDetail) override
    {
    return IGeometry::Create(ICurvePrimitive::CreateLineString(cgDetail.PointArray));
    }

// ===================================================================================

/// <summary>
/// factory base class placeholder to create a IndexedMesh from explicit args.
virtual IGeometryPtr Create (CGIndexedMeshDetail &cgDetail) override
    {
    PolyfaceHeaderPtr polyface = PolyfaceHeader::CreateVariableSizeIndexed ();

    CopyToBlockedVector (cgDetail.xyzArray, polyface->Point ());
    CopyToBlockedVector (cgDetail.coordIndexArray, polyface->PointIndex ());

    CopyToBlockedVector (cgDetail.uvArray, polyface->Param ());
    CopyToBlockedVector (cgDetail.paramIndexArray, polyface->ParamIndex ());

    CopyToBlockedVector (cgDetail.normalArray, polyface->Normal ());
    CopyToBlockedVector (cgDetail.normalIndexArray, polyface->NormalIndex ());

    // Colors have to be reformatted ...
    if (cgDetail.colorArray.size () > 0)
        {
        bvector<RgbFactor> &dest = polyface->DoubleColor ();
        dest.reserve (cgDetail.colorArray.size ());
        for (DVec3d const &data : cgDetail.colorArray)
            {
            dest.push_back (RgbFactor::From (data));
            }
        }
    CopyToBlockedVector (cgDetail.colorIndexArray, polyface->ColorIndex ());

    return IGeometry::Create (polyface);
    }

// ===================================================================================

/// <summary>
/// factory base class placeholder to create a CurveChain from explicit args.
virtual IGeometryPtr Create (CGCurveChainDetail &cgDetail) override
    {
    CurveVectorPtr cv = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    for (ICurvePrimitivePtr const & cp : cgDetail.curveArray)
        cv->push_back (cp);
    return IGeometry::Create (cv);
    }

// ===================================================================================
/// <summary>
/// factory base class placeholder to create a BsplineCurve from explicit args.
virtual IGeometryPtr Create (CGBsplineCurveDetail &cgDetail ) override
    {
    MSBsplineCurvePtr bcurve = MSBsplineCurve::CreateFromPolesAndOrder
          (
          cgDetail.controlPointArray, &cgDetail.weightArray, &cgDetail.knotArray, cgDetail.order, cgDetail.closed, true
          );
    if (bcurve.IsValid ())
        {
        ICurvePrimitivePtr cp = ICurvePrimitive::CreateBsplineCurveSwapFromSource (*bcurve);
        return IGeometry::Create (cp);
        }
    return nullptr;
    }

// ===================================================================================

/// <summary>
/// factory base class placeholder to create a BsplineSurface from explicit args.
virtual IGeometryPtr Create (CGBsplineSurfaceDetail &cgDetail) override
    {
    MSBsplineSurfacePtr bSurface = MSBsplineSurface::CreateFromPolesAndOrder
          (
          cgDetail.controlPointArray, &cgDetail.weightArray,
          &cgDetail.knotUArray, cgDetail.orderU, cgDetail.numUControlPoint, cgDetail.closedU,
          &cgDetail.knotVArray, cgDetail.orderV, cgDetail.numVControlPoint, cgDetail.closedV,
          true);
    if (bSurface.IsValid ())
        {
        return IGeometry::Create (bSurface);
        }
    return nullptr;
    }
// ===================================================================================

/// <summary>
/// factory base class placeholder to create a Polygon from explicit args.
virtual IGeometryPtr Create(CGPolygonDetail &cgDetail) override
    {
    CurveVectorPtr cv = CurveVector::CreateLinear (cgDetail.pointArray,
            CurveVector::BOUNDARY_TYPE_Outer, false);
    return IGeometry::Create (cv);
    }

// ===================================================================================

/// <summary>
/// factory base class placeholder to create a dgnExtrusion from explicit args.
virtual IGeometryPtr Create(CGDgnExtrusionDetail &cgDetail) override
    {
    if (cgDetail.baseGeometry.IsValid())
        {
        DgnExtrusionDetail dgnDetail (cgDetail.baseGeometry, cgDetail.extrusionVector, cgDetail.capped);
        return IGeometry::Create (ISolidPrimitive::CreateDgnExtrusion (dgnDetail));
        }
    return nullptr;
    }

/// <summary>
/// factory base class placeholder to create a DgnBox from explicit args.
virtual IGeometryPtr Create(CGDgnBoxDetail &cgDetail) override
    {
    DgnBoxDetail dgnDetail (
      cgDetail.topOrigin,
      cgDetail.baseOrigin,
      cgDetail.vectorX,
      cgDetail.vectorY,
      cgDetail.baseX,
      cgDetail.baseY,
      cgDetail.topX,
      cgDetail.topY,
      cgDetail.capped
      );
    return IGeometry::Create (ISolidPrimitive::CreateDgnBox (dgnDetail));
    }

/// <summary> factory base class placeholder to create a DgnSphere from explicit args.</summary>
virtual IGeometryPtr Create(CGDgnSphereDetail &cgDetail) override
    {
    DgnSphereDetail dgnDetail (
      cgDetail.center,
      cgDetail.vectorX,
      cgDetail.vectorZ,
      cgDetail.radiusXY,
      cgDetail.radiusZ,
      cgDetail.startLatitude.Radians (),
      cgDetail.latitudeSweep.Radians (),
      cgDetail.capped
      );
    return IGeometry::Create (ISolidPrimitive::CreateDgnSphere (dgnDetail));
    }

/// <summary> factory base class placeholder to create a DgnCone from explicit args.</summary>
virtual IGeometryPtr Create(CGDgnConeDetail &cgDetail) override
    {
    DgnConeDetail dgnDetail (
      cgDetail.centerA,
      cgDetail.centerB,
      cgDetail.vectorX,
      cgDetail.vectorY,
      cgDetail.radiusA,
      cgDetail.radiusB,
      cgDetail.capped
      );
    return IGeometry::Create (ISolidPrimitive::CreateDgnCone (dgnDetail));
    }

/// <summary> factory base class placeholder to create a DgnTorusPipe from explicit args.</summary>
virtual IGeometryPtr Create(CGDgnTorusPipeDetail &cgDetail) override
    {
    DgnTorusPipeDetail dgnDetail (
      cgDetail.center,
      cgDetail.vectorX,
      cgDetail.vectorY,
      cgDetail.majorRadius,
      cgDetail.minorRadius,
      cgDetail.sweepAngle.Radians (),
      cgDetail.capped
      );
    return IGeometry::Create (ISolidPrimitive::CreateDgnTorusPipe (dgnDetail));
    }

static bool GetSweepVectorFromRailCurve (IGeometryPtr &source, DVec3dR vector)
    {
    ICurvePrimitivePtr cp;
    if (source.IsValid ())
        {
        cp = source->GetAsICurvePrimitive ();
        DSegment3d segment;
        if (cp.IsValid ()
            && cp->TryGetLine (segment))
            {
            vector = DVec3d::FromStartEnd (segment.point[0], segment.point[1]);
            return true;
            }
        }
    return false;
    }

static bool GetRotationAxisFromRailCurve (IGeometryPtr &source, DRay3dR axis, double &sweepRadians)
    {
    ICurvePrimitivePtr cp;
    if (source.IsValid ())
        {
        cp = source->GetAsICurvePrimitive ();
        DEllipse3d arc;
        if (cp.IsValid ()
            && cp->TryGetArc (arc)
            && arc.IsCircular()
            )
            {
            axis.origin = arc.center;
            axis.direction = DVec3d::FromNormalizedCrossProduct (
                  arc.vector0, arc.vector90);
            sweepRadians = arc.sweep;
            return true;
            }
        }
    return false;
    }


/// <summary> Create DgnRotationalSweep or DgnExtrusion from explicit args.</summary>
static IGeometryPtr Create(IGeometryPtr &baseGeometry, IGeometryPtr &railCurve, bool capped)
    {
    DVec3d vector;
    DRay3d axis;
    double sweepRadians;
    if (baseGeometry.IsValid ()
        && railCurve.IsValid ()
        )
        {
        CurveVectorPtr cv = baseGeometry->GetAsCurveVector ();
        if (!cv.IsValid ())
            {
            }
        else if (GetSweepVectorFromRailCurve (railCurve, vector))
            {
            DgnExtrusionDetail dgnDetail (cv, vector, true);
            return IGeometry::Create (ISolidPrimitive::CreateDgnExtrusion (dgnDetail));
            }
        else if (GetRotationAxisFromRailCurve (railCurve, axis, sweepRadians))
            {
            DgnRotationalSweepDetail dgnDetail (cv, axis.origin, axis.direction, sweepRadians, true);
            return IGeometry::Create (ISolidPrimitive::CreateDgnRotationalSweep (dgnDetail));
            }
        }
    return nullptr;
    }

virtual IGeometryPtr Create(CGSolidBySweptSurfaceDetail &cgDetail) override {return Create (cgDetail.baseGeometry, cgDetail.railCurve, true);}
virtual IGeometryPtr Create(CGSurfaceBySweptCurveDetail &cgDetail) override {return Create (cgDetail.baseGeometry, cgDetail.railCurve, false);}

};
