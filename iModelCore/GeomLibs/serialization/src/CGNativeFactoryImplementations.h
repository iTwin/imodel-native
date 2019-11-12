/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
// (To be inclued in BeXmlCGStreamReader.cpp)

#include "nativeCGFactoryH.h"
struct IGeometryCGFactory : ICGFactory
{
private:
static bool TryGetAsCurveVector (IGeometryPtr &geometry, CurveVectorPtr &cv)
    {
    cv = nullptr;
    if (!geometry.IsValid ())
        return false;
    cv = geometry->GetAsCurveVector ();
    if (cv.IsValid ())
        return true;
    ICurvePrimitivePtr cp = geometry->GetAsICurvePrimitive ();
    if (cp.IsValid ())
        {
        cv = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
        cv->push_back (cp);
        return true;
        }
    return false;
    }
public:

// ===================================================  ================================
IGeometryPtr Create (CGCoordinateDetail &detail) override
    {
    return IGeometry::Create(ICurvePrimitive::CreateLine (DSegment3d::From (detail.xyz, detail.xyz)));
    }

// ===================================================================================
IGeometryPtr Create (CGLineSegmentDetail &detail) override
    {
    ICurvePrimitivePtr cp = ICurvePrimitive::CreateLine (DSegment3d::From (detail.startPoint, detail.endPoint));
    return IGeometry::Create (cp);
    }

IGeometryPtr Create (CGGroupDetail &detail) override
    {
    for (IGeometryPtr const &member : detail.memberArray)
        {
        m_groupMembers.push_back (member);
        }
    CurveVectorPtr curves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    for (IGeometryPtr const &member : detail.memberArray)
        {
        if (member.IsValid ())
            {
            CurveVectorPtr cvMember = member->GetAsCurveVector ();
            if (cvMember.IsValid ())            
                curves->Add (cvMember);

            ICurvePrimitivePtr cpMember = member->GetAsICurvePrimitive ();
            if (cpMember.IsValid ())
                curves->Add (cpMember);
            }
        }
    if (curves->size () > 0)
        {
        return IGeometry::Create (curves);
        }
    return nullptr;
    }
// ===================================================================================
IGeometryPtr Create (CGEllipticArcDetail &detail) override
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
IGeometryPtr Create (CGEllipticDiskDetail &detail) override
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
    return IGeometry::Create(area);
    }

// ===================================================================================
IGeometryPtr Create (CGCircularArcDetail &detail) override
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
IGeometryPtr Create (CGCircularDiskDetail &detail) override
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
    return IGeometry::Create(area);
    }

// ===================================================================================
IGeometryPtr Create (CGSkewedConeDetail &detail) override
    {
    RotMatrix axes;
    DPoint3d centerA;
    detail.placement.GetFrame (centerA, axes);
    DgnConeDetail coneDetail (centerA, detail.centerB,
                axes, 
                detail.radiusA, detail.radiusB,
                detail.bSolidFlag);
    ISolidPrimitivePtr sp = ISolidPrimitive::CreateDgnCone (coneDetail);
    return IGeometry::Create (sp);
    }

// ===================================================================================

/// <summary>
/// factory base class placeholder to create a CircularCone from explicit args.
IGeometryPtr Create (CGCircularConeDetail &detail) override
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
                detail.bSolidFlag);
    ISolidPrimitivePtr sp = ISolidPrimitive::CreateDgnCone (coneDetail);
    return IGeometry::Create (sp);
    }

// ===================================================================================

/// <summary>
/// factory base class placeholder to create a CircularCylinder from explicit args.
IGeometryPtr Create(CGCircularCylinderDetail &detail) override
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
                detail.bSolidFlag);
    ISolidPrimitivePtr sp = ISolidPrimitive::CreateDgnCone (coneDetail);
    return IGeometry::Create (sp);
    }

// ===================================================================================

/// <summary>
/// factory base class placeholder to create a Block from explicit args.
IGeometryPtr Create (CGBlockDetail &detail) override
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
    DgnBoxDetail boxDetail (baseOrigin, topOrigin, vectorX, vectorY, dx, dy, dx, dy, detail.bSolidFlag);
    ISolidPrimitivePtr sp = ISolidPrimitive::CreateDgnBox (boxDetail);
    return IGeometry::Create (sp);
    }

// ===================================================================================
/// <summary> factory base class placeholder to create a Sphere from explicit args.</summary>
IGeometryPtr Create (CGSphereDetail &detail) override
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
IGeometryPtr Create (CGTorusPipeDetail &detail) override
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
                detail.bSolidFlag);
    return IGeometry::Create(ISolidPrimitive::CreateDgnTorusPipe (dgnDetail));
    }

// ===================================================================================
IGeometryPtr Create (CGLineStringDetail &cgDetail) override
    {
    return IGeometry::Create(ICurvePrimitive::CreateLineString(cgDetail.PointArray));
    }

// ===================================================================================

/// <summary>
/// factory base class placeholder to create a IndexedMesh from explicit args.
IGeometryPtr Create (CGIndexedMeshDetail &cgDetail) override
    {
    PolyfaceHeaderPtr polyface = PolyfaceHeader::CreateVariableSizeIndexed ();

    CopyToBlockedVector (cgDetail.xyzArray, polyface->Point ());
    CopyToBlockedVector (cgDetail.coordIndexArray, polyface->PointIndex ());

    CopyToBlockedVector (cgDetail.uvArray, polyface->Param ());
    CopyToBlockedVector (cgDetail.paramIndexArray, polyface->ParamIndex ());

    CopyToBlockedVector (cgDetail.normalArray, polyface->Normal ());
    CopyToBlockedVector (cgDetail.normalIndexArray, polyface->NormalIndex ());

#if defined (NOT_NOW_MESH_COLOR)
    // Colors have to be reformatted ...
    if (cgDetail.colorArray.size () > 0)
        {
        BlockedVectorRgbFactorR &dest = polyface->DoubleColor ();
        dest.reserve (cgDetail.colorArray.size ());
        if (cgDetail.colorArray.size () > 0)
            dest.SetActive (true);
        for (DVec3d const &data : cgDetail.colorArray)
            {
            dest.push_back (RgbFactor::From (data));
            }
        }
    CopyToBlockedVector (cgDetail.colorIndexArray, polyface->ColorIndex ());
#endif

    return IGeometry::Create (polyface);
    }

// ===================================================================================

/// <summary>
/// factory base class placeholder to create a CurveChain from explicit args.
IGeometryPtr Create (CGCurveChainDetail &cgDetail) override
    {
    CurveVectorPtr cv = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    for (ICurvePrimitivePtr const & cp : cgDetail.curveArray)
        cv->push_back (cp);
    return IGeometry::Create (cv);
    }

// ===================================================================================
/// <summary>
/// factory base class placeholder to create a BsplineCurve from explicit args.
IGeometryPtr Create (CGBsplineCurveDetail &cgDetail ) override
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
IGeometryPtr Create (CGBsplineSurfaceDetail &cgDetail) override
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

IGeometryPtr Create (CGParametricSurfacePatchDetail &detail) override
    {
    if (detail.surface.IsValid () && detail.surface->GetGeometryType () == IGeometry::GeometryType::BsplineSurface)
        {
        MSBsplineSurfacePtr bsurf = detail.surface->GetAsMSBsplineSurface ();
        if (detail.loopArray.size () > 0)
            {
            size_t numCV = 0;
            for (size_t i = 0; i < detail.loopArray.size(); i++)
                {
                if (detail.loopArray[i]->GetGeometryType () == IGeometry::GeometryType::CurveVector)
                    numCV++;
                }
            if (numCV == detail.loopArray.size ())
                {
                if (numCV == 1)
                    {
                    CurveVectorPtr cv = detail.loopArray[0]->GetAsCurveVector ();
                    bsurf->SetTrim (*cv);
                    }
                else
                    {
                    CurveVectorPtr paritySet = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
                    for (size_t i = 0; i < detail.loopArray.size(); i++)
                        paritySet->Add (detail.loopArray[i]->GetAsCurveVector ());
                    bsurf->SetTrim (*paritySet);
                    }
                return IGeometry::Create (bsurf);
                }
            }
        }
    return nullptr;
    }
// ===================================================================================

/// <summary>
/// factory base class placeholder to create a Polygon from explicit args.
IGeometryPtr Create(CGPolygonDetail &cgDetail) override
    {
    CurveVectorPtr cv = CurveVector::CreateLinear (cgDetail.pointArray,
            CurveVector::BOUNDARY_TYPE_Outer, false);
    return IGeometry::Create (cv);
    }

// ===================================================================================

/// <summary>
/// factory base class placeholder to create a dgnExtrusion from explicit args.
IGeometryPtr Create(CGDgnExtrusionDetail &cgDetail) override
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
IGeometryPtr Create(CGDgnBoxDetail &cgDetail) override
    {
    DgnBoxDetail dgnDetail (
      cgDetail.baseOrigin,
      cgDetail.topOrigin,
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
IGeometryPtr Create(CGDgnSphereDetail &cgDetail) override
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
IGeometryPtr Create(CGDgnConeDetail &cgDetail) override
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
IGeometryPtr Create(CGDgnTorusPipeDetail &cgDetail) override
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



IGeometryPtr Create(CGDgnRotationalSweepDetail &cgDetail) override
    {
    DgnRotationalSweepDetail dgnDetail
            (cgDetail.baseGeometry,
            cgDetail.center, cgDetail.axis,
            cgDetail.sweepAngle.Radians (), cgDetail.capped
            );
    return IGeometry::Create (ISolidPrimitive::CreateDgnRotationalSweep (dgnDetail));
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
        CurveVectorPtr cv;
        if (!TryGetAsCurveVector (baseGeometry, cv))
            {
            }
        else if (GetSweepVectorFromRailCurve (railCurve, vector))
            {
            DgnExtrusionDetail dgnDetail (cv, vector, capped);
            return IGeometry::Create (ISolidPrimitive::CreateDgnExtrusion (dgnDetail));
            }
        else if (GetRotationAxisFromRailCurve (railCurve, axis, sweepRadians))
            {
            DgnRotationalSweepDetail dgnDetail (cv, axis.origin, axis.direction, sweepRadians, capped);
            return IGeometry::Create (ISolidPrimitive::CreateDgnRotationalSweep (dgnDetail));
            }
        }
    return nullptr;
    }

IGeometryPtr Create(CGSolidBySweptSurfaceDetail &cgDetail) override {return Create (cgDetail.baseGeometry, cgDetail.railCurve, true);}
IGeometryPtr Create(CGSurfaceBySweptCurveDetail &cgDetail) override {return Create (cgDetail.baseGeometry, cgDetail.railCurve, false);}

/// <summary> Create DgnRotationalSweep.</summary>
static IGeometryPtr CreateRuled(bvector<CurveVectorPtr> &sections, bool capped)
    {

    DgnRuledSweepDetail dgnDetail;
    dgnDetail.m_capped = capped;
    dgnDetail.m_sectionCurves = sections;
    if (dgnDetail.m_sectionCurves.size () > 0)
        {
        return IGeometry::Create (ISolidPrimitive::CreateDgnRuledSweep (dgnDetail));
        }
    return nullptr;
    }

IGeometryPtr Create(CGSurfaceByRuledSweepDetail &cgDetail) override {return CreateRuled (cgDetail.SectionArray, false);}
IGeometryPtr Create(CGSolidByRuledSweepDetail &cgDetail) override {return CreateRuled (cgDetail.SectionArray, true);}

IGeometryPtr Create(CGDgnRuledSweepDetail &cgDetail) override
    {
    DgnRuledSweepDetail dgnDetail (cgDetail.contourArray, cgDetail.capped);
    return IGeometry::Create (ISolidPrimitive::CreateDgnRuledSweep (dgnDetail));
    }


IGeometryPtr Create (CGSurfacePatchDetail &detail) override
    {
    CurveVectorPtr cv;
    if (TryGetAsCurveVector (detail.exteriorLoop, cv))
        {
        if (detail.holeLoopArray.size () == 0)
            {
            cv->SetBoundaryType (CurveVector::BOUNDARY_TYPE_Outer);
            return IGeometry::Create (cv);
            }
        else
            {
            auto parityRegion = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
            cv->SetBoundaryType (CurveVector::BOUNDARY_TYPE_Outer);
            parityRegion->Add (cv);
            for (IGeometryPtr &g : detail.holeLoopArray)
                {
                CurveVectorPtr hole;
                if (TryGetAsCurveVector (g, hole))
                    {
                    hole->SetBoundaryType (CurveVector::BOUNDARY_TYPE_Inner);
                    parityRegion->Add (hole);
                    }
                }
            return IGeometry::Create (parityRegion);
            }
        }
    return nullptr;
    }

// ===================================================================================
IGeometryPtr Create (CGDgnCurveVectorDetail &detail) override
    {
    CurveVectorPtr cv = CurveVector::Create ((CurveVector::BoundaryType)detail.boundaryType);
    for (ICurvePrimitivePtr child : detail.memberArray)
        cv->Add (child);
    return IGeometry::Create (cv);
    }

// ===================================================================================
IGeometryPtr Create (CGPointChainDetail &detail) override
    {
    bvector<DPoint3d> points;
    for (size_t i = 0; i < detail.PointArray.size (); i++)
        {
        ICurvePrimitivePtr cp = detail.PointArray[i]->GetAsICurvePrimitive ();
        DPoint3d xyz;
        if (cp.IsValid () && cp->GetStartPoint (xyz))
            points.push_back (xyz);
        }
    
    return IGeometry::Create (ICurvePrimitive::CreatePointString (points));
    }

// ===================================================================================
IGeometryPtr Create (CGTransitionSpiralDetail &detail) override
    {
    DPoint3d origin;
    RotMatrix axes;
    detail.placement.GetFrame (origin, axes);
    Transform transform = Transform::From (axes, origin);
    ICurvePrimitivePtr cp;
    if (detail.length != 0.0)
        {
        cp = ICurvePrimitive::CreateSpiralBearingRadiusLengthRadius (
                DSpiral2dBase::StringToTransitionType (detail.spiralType.c_str ()),
                detail.startBearing.Radians (),
                detail.startRadius,
                detail.length,
                detail.endRadius,
                transform,
                detail.activeStartFraction,
                detail.activeEndFraction
                );
        }
    else
        {
        cp = ICurvePrimitive::CreateSpiralBearingRadiusBearingRadius (
                DSpiral2dBase::StringToTransitionType (detail.spiralType.c_str ()),
                detail.startBearing.Radians (),
                detail.startRadius,
                detail.endBearing.Radians (),
                detail.endRadius,
                transform,
                detail.activeStartFraction,
                detail.activeEndFraction
                );
        }
    if (!cp.IsValid ())
        return nullptr;
    return IGeometry::Create (cp);
    }

IGeometryPtr Create (CGPartialCurveDetail &detail) override
    {
    ICurvePrimitivePtr cp = ICurvePrimitive::CreatePartialCurve (
            detail.parentCurve.get (),
            detail.fraction0, detail.fraction1
            );
    if (!cp.IsValid ())
        return nullptr;
    return IGeometry::Create (cp);
    }
};
