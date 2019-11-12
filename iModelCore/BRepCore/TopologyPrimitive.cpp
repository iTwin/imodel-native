/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <BRepCore/SolidKernel.h>
#if defined (BENTLEYCONFIG_PARASOLID)
#include <BRepCore/PSolidUtil.h>
#endif

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool TopologyPrimitive::GetLocalCoordinateFrame(TransformR localToWorld) const
    {
    switch (GetGeometryType())
        {
        case GeometryType::CurvePrimitive:
            {
            ICurvePrimitiveCR curve = *GetAsICurvePrimitive();

            if (!curve.FractionToFrenetFrame(0.0, localToWorld))
                {
                DPoint3d point;

                if (curve.GetStartPoint(point))
                    {
                    localToWorld.InitFrom(point);
                    return true;
                    }

                localToWorld.InitIdentity();
                return false;
                }

            break;
            }

        case GeometryType::CurveVector:
            {
            CurveVectorCR curves = *GetAsCurveVector();

            if (!curves.GetAnyFrenetFrame(localToWorld))
                {
                DPoint3d point;

                if (curves.GetStartPoint(point))
                    {
                    localToWorld.InitFrom(point);
                    return true;
                    }

                localToWorld.InitIdentity();
                return false;
                }

            break;
            }

        case GeometryType::SolidPrimitive:
            {
            Transform         worldToLocal;
            ISolidPrimitiveCR solidPrimitive = *GetAsISolidPrimitive();

            if (!solidPrimitive.TryGetConstructiveFrame(localToWorld, worldToLocal))
                {
                localToWorld.InitIdentity();
                return false;
                }

            break;
            }

        case GeometryType::Polyface:
            {
            PolyfaceHeaderCR polyface = *GetAsPolyfaceHeader();

            double      area;
            DPoint3d    centroid;
            RotMatrix   axes;
            DVec3d      momentXYZ;

            if (!polyface.ComputePrincipalAreaMoments(area, centroid, axes, momentXYZ))
                {
                localToWorld.InitIdentity();
                return false;
                }

            localToWorld.InitFrom(axes, centroid);
            break;
            }

        case GeometryType::BsplineSurface:
            {
            MSBsplineSurfaceCR surface = *GetAsMSBsplineSurface();

            double      area;
            DVec3d      centroid;
            RotMatrix   axes;
            DVec3d      momentXYZ;

            if (surface.ComputePrincipalAreaMoments(area, centroid, axes, momentXYZ))
                {
                localToWorld.InitFrom(axes, centroid);
                break;
                }
            else if (surface.EvaluateNormalizedFrame(localToWorld, 0,0))
                {
                break;
                }

            localToWorld.InitIdentity();
            return false;
            }

        case GeometryType::BRepEntity:
            {
            IBRepEntityCR entity = *GetAsIBRepEntity();

            // The entity transform (after removing SWA scale) can be used for localToWorld (solid kernel to uors)...
            localToWorld = entity.GetEntityTransform();
            break;
            }

        default:
            {
            localToWorld.InitIdentity();
            BeAssert(false);

            return false;
            }
        }

    // NOTE: Ensure rotation is squared up and normalized (ComputePrincipalAreaMoments/GetEntityTransform is scaled)...
    DPoint3d    origin;
    RotMatrix   rMatrix;

    localToWorld.GetTranslation(origin);
    localToWorld.GetMatrix(rMatrix);
    rMatrix.SquareAndNormalizeColumns(rMatrix, 0, 1);
    localToWorld.InitFrom(rMatrix, origin);

    return true;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool TopologyPrimitive::GetLocalRange(DRange3dR localRange, TransformR localToWorld) const
    {
    if (!GetLocalCoordinateFrame(localToWorld))
        return false;

    if (localToWorld.IsIdentity())
        return GetRange(localRange);

#if defined (BENTLEYCONFIG_PARASOLID)
    TopologyPrimitivePtr clone;

    // NOTE: Avoid un-necessary copy of BRep. We just need to change entity transform...
    if (GeometryType::BRepEntity == GetGeometryType())
        clone = new TopologyPrimitive(PSolidUtil::InstanceEntity(*GetAsIBRepEntity()));
    else
        clone = Clone();
#else
    TopologyPrimitivePtr clone = Clone();
#endif
    Transform worldToLocal;

    worldToLocal.InverseOf(localToWorld);
    clone->TransformInPlace(worldToLocal);

    return clone->GetRange(localRange);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getRange(ICurvePrimitiveCR geom, DRange3dR range, TransformCP transform)
    {
    return (nullptr != transform ? geom.GetRange(range, *transform) : geom.GetRange(range));
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getRange(CurveVectorCR geom, DRange3dR range, TransformCP transform)
    {
    return (nullptr != transform ? geom.GetRange(range, *transform) : geom.GetRange(range));
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getRange(ISolidPrimitiveCR geom, DRange3dR range, TransformCP transform)
    {
    return (nullptr != transform ? geom.GetRange(range, *transform) : geom.GetRange(range));
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getRange(PolyfaceQueryCR geom, DRange3dR range, TransformCP transform)
    {
    if (nullptr == transform)
        {
        range = geom.PointRange();
        }
    else
        {
        range.Init();
        range.Extend(*transform, geom.GetPointCP(), (int)geom.GetPointCount());
        }

    return true;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getRange(MSBsplineSurfaceCR geom, DRange3dR range, TransformCP transform)
    {
    // NOTE: MSBsplineSurface::GetPoleRange can result in a very large range and using IPolyfaceConstruction can be slow.
    //       Unfortunately, we don't have a better way to handle trimmed surfaces and getting a large range for a surface has
    //       undesirable consequences for fit view which uses the element aligned box and doesn't look at the geometry.
    if (0 != geom.GetNumBounds())
        {
        IFacetOptionsPtr options = IFacetOptions::CreateForSurfaces();

        options->SetMinPerBezier(3);

        IPolyfaceConstructionPtr builder = IPolyfaceConstruction::New(*options);

        builder->Add(geom);

        return getRange(builder->GetClientMeshR(), range, transform);
        }

    if (nullptr != transform)
        geom.GetPoleRange(range, *transform);
    else
        geom.GetPoleRange(range);

    DRange3d    tightRange;
    Transform   originWithExtentVectors, centroidalLocalToWorld, centroidalWorldToLocal;

    if (geom.TightPrincipalExtents(originWithExtentVectors, centroidalLocalToWorld, centroidalWorldToLocal, tightRange))
        {
        tightRange.ScaleAboutCenter(tightRange, 1.2); // Pad range to make sure surface is fully inside range...
        centroidalLocalToWorld.Multiply(tightRange, tightRange);

        if (nullptr != transform)
            transform->Multiply(tightRange, tightRange);

        range = DRange3d::FromIntersection(range, tightRange, true); // Don't return a range that exceeds pole range in any direction...
        }

    return true;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getRange(IBRepEntityCR geom, DRange3dR range, TransformCP transform)
    {
    range = geom.GetEntityRange();

    if (nullptr != transform)
        transform->Multiply(range, range);

    return true;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool TopologyPrimitive::GetRange(DRange3dR range, TransformCP transform) const
    {
    range.Init();

    switch (GetGeometryType())
        {
        case GeometryType::CurvePrimitive:
            {
            ICurvePrimitiveCR geom = *GetAsICurvePrimitive();

            return getRange(geom, range, transform);
            }

        case GeometryType::CurveVector:
            {
            CurveVectorCR geom = *GetAsCurveVector();

            return getRange(geom, range, transform);
            }

        case GeometryType::SolidPrimitive:
            {
            ISolidPrimitiveCR geom = *GetAsISolidPrimitive();

            return getRange(geom, range, transform);
            }

        case GeometryType::Polyface:
            {
            PolyfaceHeaderCR geom = *GetAsPolyfaceHeader();

            return getRange(geom, range, transform);
            }

        case GeometryType::BsplineSurface:
            {
            MSBsplineSurfaceCR geom = *GetAsMSBsplineSurface();

            return getRange(geom, range, transform);
            }

        case GeometryType::BRepEntity:
            {
            IBRepEntityCR geom = *GetAsIBRepEntity();

            return getRange(geom, range, transform);
            }

        default:
            {
            BeAssert(false);
            return false;
            }
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool TopologyPrimitive::TransformInPlace(TransformCR transform)
    {
    switch (GetGeometryType())
        {
        case GeometryType::CurvePrimitive:
            {
            ICurvePrimitiveR geom = *GetAsICurvePrimitive();

            return geom.TransformInPlace(transform);
            }

        case GeometryType::CurveVector:
            {
            CurveVectorR geom = *GetAsCurveVector();

            return geom.TransformInPlace(transform);
            }

        case GeometryType::SolidPrimitive:
            {
            ISolidPrimitiveR geom = *GetAsISolidPrimitive();

            return geom.TransformInPlace(transform);
            }

        case GeometryType::Polyface:
            {
            PolyfaceHeaderR geom = *GetAsPolyfaceHeader();

            geom.Transform(transform);

            return true;
            }

        case GeometryType::BsplineSurface:
            {
            MSBsplineSurfaceR geom = *GetAsMSBsplineSurface();

            return (SUCCESS == geom.TransformSurface(transform) ? true : false);
            }

        case GeometryType::BRepEntity:
            {
            IBRepEntityR geom = *GetAsIBRepEntity();

            geom.PreMultiplyEntityTransformInPlace(transform); // Just change entity transform...

            return true;
            }

        default:
            {
            BeAssert(false);
            return false;
            }
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  03/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool TopologyPrimitive::IsSameStructureAndGeometry(TopologyPrimitiveCR primitive, double tolerance) const
    {
    if (GetGeometryType() != primitive.GetGeometryType())
        return false;

    switch (GetGeometryType())
        {
        case GeometryType::CurvePrimitive:
            {
            ICurvePrimitiveCR geom1 = *GetAsICurvePrimitive();
            ICurvePrimitiveCR geom2 = *primitive.GetAsICurvePrimitive();

            return geom1.IsSameStructureAndGeometry(geom2, tolerance);
            }

        case GeometryType::CurveVector:
            {
            CurveVectorCR geom1 = *GetAsCurveVector();
            CurveVectorCR geom2 = *primitive.GetAsCurveVector();

            return geom1.IsSameStructureAndGeometry(geom2, tolerance);
            }

        case GeometryType::SolidPrimitive:
            {
            ISolidPrimitiveCR geom1 = *GetAsISolidPrimitive();
            ISolidPrimitiveCR geom2 = *primitive.GetAsISolidPrimitive();

            return geom1.IsSameStructureAndGeometry(geom2, tolerance);
            }

        case GeometryType::Polyface:
            {
            PolyfaceHeaderCR geom1 = *GetAsPolyfaceHeader();
            PolyfaceHeaderCR geom2 = *primitive.GetAsPolyfaceHeader();

            return geom1.IsSameStructureAndGeometry(geom2, tolerance);
            }

        case GeometryType::BsplineSurface:
            {
            MSBsplineSurfaceCR geom1 = *GetAsMSBsplineSurface();
            MSBsplineSurfaceCR geom2 = *primitive.GetAsMSBsplineSurface();

            return geom1.IsSameStructureAndGeometry(geom2, tolerance);
            }

#if defined (BENTLEYCONFIG_PARASOLID)
        case GeometryType::BRepEntity:
            {
            // Can't ignore body to uor transform...post instancing in V8 converter only compares local range for a match which doesn't account for body transform differences...
            if (!GetAsIBRepEntity()->GetEntityTransform().IsEqual(primitive.GetAsIBRepEntity()->GetEntityTransform(), tolerance, tolerance))
                return false;

            double      solidTolerance = tolerance;
            PK_BODY_t   bodyTag1 = PSolidUtil::GetEntityTag(*GetAsIBRepEntity());
            PK_BODY_t   bodyTag2 = PSolidUtil::GetEntityTag(*primitive.GetAsIBRepEntity());

            if (0.0 != solidTolerance)
                {
                Transform uorToSolid;

                uorToSolid.InverseOf(GetAsIBRepEntity()->GetEntityTransform());
                uorToSolid.ScaleDoubleArrayByXColumnMagnitude(&solidTolerance, 1);
                }

            return PSolidUtil::AreBodiesEqual(bodyTag1, bodyTag2, solidTolerance, nullptr);
            }
#endif

        default:
            return false;
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  03/15
+---------------+---------------+---------------+---------------+---------------+------*/
TopologyPrimitivePtr TopologyPrimitive::Clone() const
    {
    switch (GetGeometryType())
        {
        case GeometryType::CurvePrimitive:
            {
            ICurvePrimitivePtr geom = GetAsICurvePrimitive()->Clone();

            return new TopologyPrimitive(geom);
            }

        case GeometryType::CurveVector:
            {
            CurveVectorPtr geom = GetAsCurveVector()->Clone();

            return new TopologyPrimitive(geom);
            }

        case GeometryType::SolidPrimitive:
            {
            ISolidPrimitivePtr geom = GetAsISolidPrimitive()->Clone();

            return new TopologyPrimitive(geom);
            }

        case GeometryType::Polyface:
            {
            PolyfaceHeaderPtr geom = GetAsPolyfaceHeader()->Clone();

            return new TopologyPrimitive(geom);
            }

        case GeometryType::BsplineSurface:
            {
            MSBsplineSurfacePtr geom = GetAsMSBsplineSurface()->Clone();

            return new TopologyPrimitive(geom);
            }

        case GeometryType::BRepEntity:
            {
            IBRepEntityPtr geom = GetAsIBRepEntity()->Clone();

            return new TopologyPrimitive(geom);
            }

        default:
            {
            BeAssert(false);
            return nullptr;
            }
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  07/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool TopologyPrimitive::IsSolid() const
    {
    switch (GetGeometryType())
        {
        case GeometryType::SolidPrimitive:
            return GetAsISolidPrimitive()->GetCapped();

        case GeometryType::Polyface:
            return GetAsPolyfaceHeader()->IsClosedByEdgePairing();

        case GeometryType::BRepEntity:
            return IBRepEntity::EntityType::Solid == GetAsIBRepEntity()->GetEntityType();

        default:
            return false;
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  07/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool TopologyPrimitive::IsSheet() const
    {
    switch (GetGeometryType())
        {
        case GeometryType::CurveVector:
            return GetAsCurveVector()->IsAnyRegionType();

        case GeometryType::BsplineSurface:
            return true;

        case GeometryType::SolidPrimitive:
            return !GetAsISolidPrimitive()->GetCapped();

        case GeometryType::Polyface:
            return !GetAsPolyfaceHeader()->IsClosedByEdgePairing();

        case GeometryType::BRepEntity:
            return IBRepEntity::EntityType::Sheet == GetAsIBRepEntity()->GetEntityType();

        default:
            return false;
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  07/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool TopologyPrimitive::IsWire() const
    {
    switch (GetGeometryType())
        {
        case GeometryType::CurvePrimitive:
            return ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString != GetAsICurvePrimitive()->GetCurvePrimitiveType();

        case GeometryType::CurveVector:
            return GetAsCurveVector()->IsOpenPath();

        case GeometryType::BRepEntity:
            return IBRepEntity::EntityType::Wire == GetAsIBRepEntity()->GetEntityType();

        default:
            return false;
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  07/16
+---------------+---------------+---------------+---------------+---------------+------*/
IBRepEntity::EntityType TopologyPrimitive::GetBRepEntityType() const
    {
    switch (GetGeometryType())
        {
        case GeometryType::CurvePrimitive:
            return IBRepEntity::EntityType::Wire;

        case GeometryType::CurveVector:
            return (GetAsCurveVector()->IsAnyRegionType() ? IBRepEntity::EntityType::Sheet : (GetAsCurveVector()->IsOpenPath() ? IBRepEntity::EntityType::Wire : IBRepEntity::EntityType::Invalid));

        case GeometryType::BsplineSurface:
            return IBRepEntity::EntityType::Sheet;

        case GeometryType::SolidPrimitive:
            return GetAsISolidPrimitive()->GetCapped() ? IBRepEntity::EntityType::Solid : IBRepEntity::EntityType::Sheet;

        case GeometryType::Polyface:
            return GetAsPolyfaceHeader()->IsClosedByEdgePairing() ? IBRepEntity::EntityType::Solid : IBRepEntity::EntityType::Sheet;

        case GeometryType::BRepEntity:
            return GetAsIBRepEntity()->GetEntityType();

        default:
            return IBRepEntity::EntityType::Invalid;
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/15
+---------------+---------------+---------------+---------------+---------------+------*/
TopologyPrimitive::TopologyPrimitive(ICurvePrimitivePtr const& source) {m_type = GeometryType::CurvePrimitive; m_data = source;}
TopologyPrimitive::TopologyPrimitive(CurveVectorPtr const& source) {m_type = GeometryType::CurveVector; m_data = source;}
TopologyPrimitive::TopologyPrimitive(ISolidPrimitivePtr const& source) {m_type = GeometryType::SolidPrimitive; m_data = source;}
TopologyPrimitive::TopologyPrimitive(MSBsplineSurfacePtr const& source) {m_type = GeometryType::BsplineSurface; m_data = source;}
TopologyPrimitive::TopologyPrimitive(PolyfaceHeaderPtr const& source) {m_type = GeometryType::Polyface; m_data = source;}
TopologyPrimitive::TopologyPrimitive(IBRepEntityPtr const& source) {m_type = GeometryType::BRepEntity; m_data = source;}

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/15
+---------------+---------------+---------------+---------------+---------------+------*/
TopologyPrimitivePtr TopologyPrimitive::Create(ICurvePrimitivePtr const& source) {return (source.IsValid() ? new TopologyPrimitive(source) : nullptr);}
TopologyPrimitivePtr TopologyPrimitive::Create(CurveVectorPtr const& source) {return (source.IsValid() ? new TopologyPrimitive(source) : nullptr);}
TopologyPrimitivePtr TopologyPrimitive::Create(ISolidPrimitivePtr const& source) {return (source.IsValid() ? new TopologyPrimitive(source) : nullptr);}
TopologyPrimitivePtr TopologyPrimitive::Create(MSBsplineSurfacePtr const& source) {return (source.IsValid() ? new TopologyPrimitive(source) : nullptr);}
TopologyPrimitivePtr TopologyPrimitive::Create(PolyfaceHeaderPtr const& source) {return (source.IsValid() ? new TopologyPrimitive(source) : nullptr);}
TopologyPrimitivePtr TopologyPrimitive::Create(IBRepEntityPtr const& source) {return (source.IsValid() ? new TopologyPrimitive(source) : nullptr);}

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/15
+---------------+---------------+---------------+---------------+---------------+------*/
TopologyPrimitivePtr TopologyPrimitive::Create(ICurvePrimitiveCR source) {ICurvePrimitivePtr clone = source.Clone(); return Create(clone);}
TopologyPrimitivePtr TopologyPrimitive::Create(CurveVectorCR source) {CurveVectorPtr clone = source.Clone(); return Create(clone);}
TopologyPrimitivePtr TopologyPrimitive::Create(ISolidPrimitiveCR source) {ISolidPrimitivePtr clone = source.Clone(); return Create(clone);}
TopologyPrimitivePtr TopologyPrimitive::Create(MSBsplineSurfaceCR source) {MSBsplineSurfacePtr clone = source.Clone(); return Create(clone);}
TopologyPrimitivePtr TopologyPrimitive::Create(PolyfaceQueryCR source) {PolyfaceHeaderPtr clone = source.Clone(); return Create(clone);}
TopologyPrimitivePtr TopologyPrimitive::Create(IBRepEntityCR source) {IBRepEntityPtr clone = source.Clone(); return Create(clone);}

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Shaun.Sewall    02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TopologyPrimitivePtr TopologyPrimitive::Create(DEllipse3dCR source) {ICurvePrimitivePtr curve = ICurvePrimitive::CreateArc(source); return curve.IsValid() ? Create(curve) : nullptr;}
TopologyPrimitivePtr TopologyPrimitive::Create(DgnBoxDetailCR source) {ISolidPrimitivePtr solid = ISolidPrimitive::CreateDgnBox(source); return solid.IsValid() ? Create(solid) : nullptr;}
TopologyPrimitivePtr TopologyPrimitive::Create(DgnConeDetailCR source) {ISolidPrimitivePtr solid = ISolidPrimitive::CreateDgnCone(source); return solid.IsValid() ? Create(solid) : nullptr;}
TopologyPrimitivePtr TopologyPrimitive::Create(DgnSphereDetailCR source) {ISolidPrimitivePtr solid = ISolidPrimitive::CreateDgnSphere(source); return solid.IsValid() ? Create(solid) : nullptr;}
TopologyPrimitivePtr TopologyPrimitive::Create(DgnTorusPipeDetailCR source) {ISolidPrimitivePtr solid = ISolidPrimitive::CreateDgnTorusPipe(source); return solid.IsValid() ? Create(solid) : nullptr;}

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/15
+---------------+---------------+---------------+---------------+---------------+------*/
TopologyPrimitive::GeometryType TopologyPrimitive::GetGeometryType() const {return m_type;}
ICurvePrimitivePtr TopologyPrimitive::GetAsICurvePrimitive() const {return (GeometryType::CurvePrimitive == m_type ? static_cast <ICurvePrimitiveP> (m_data.get()) : nullptr);}
CurveVectorPtr TopologyPrimitive::GetAsCurveVector() const {return (GeometryType::CurveVector == m_type ? static_cast <CurveVectorP> (m_data.get()) : nullptr);}
ISolidPrimitivePtr TopologyPrimitive::GetAsISolidPrimitive() const {return (GeometryType::SolidPrimitive == m_type ? static_cast <ISolidPrimitiveP> (m_data.get()) : nullptr);}
MSBsplineSurfacePtr TopologyPrimitive::GetAsMSBsplineSurface() const {return (GeometryType::BsplineSurface == m_type ? static_cast <RefCountedMSBsplineSurface*> (m_data.get()) : nullptr);}
PolyfaceHeaderPtr TopologyPrimitive::GetAsPolyfaceHeader() const {return (GeometryType::Polyface == m_type ? static_cast <PolyfaceHeaderP> (m_data.get()) : nullptr);}
IBRepEntityPtr TopologyPrimitive::GetAsIBRepEntity() const {return (GeometryType::BRepEntity == m_type ? static_cast <IBRepEntityP> (m_data.get()) : nullptr);}
