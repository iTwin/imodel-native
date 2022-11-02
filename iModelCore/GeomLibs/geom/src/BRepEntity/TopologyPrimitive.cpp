/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/BRepEntity.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*----------------------------------------------------------------------------------*//**
* @bsimethod
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

        case GeometryType::BRepEntity:
            {
            IBRepEntityCR entity = *GetAsIBRepEntity();

            // The entity transform (after removing scale) can be used for localToWorld (solid kernel to meters)...
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

    // NOTE: Ensure rotation is squared up and normalized (GetEntityTransform is scaled)...
    DPoint3d    origin;
    RotMatrix   rMatrix;

    localToWorld.GetTranslation(origin);
    localToWorld.GetMatrix(rMatrix);
    rMatrix.SquareAndNormalizeColumns(rMatrix, 0, 1);
    localToWorld.InitFrom(rMatrix, origin);

    return true;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool TopologyPrimitive::GetLocalRange(DRange3dR localRange, TransformR localToWorld) const
    {
    if (!GetLocalCoordinateFrame(localToWorld))
        return false;

    if (localToWorld.IsIdentity())
        return GetRange(localRange);

    TopologyPrimitivePtr clone;

    // NOTE: Avoid un-necessary copy of BRep. We just need to change entity transform...
    if (GeometryType::BRepEntity == GetGeometryType())
        clone = new TopologyPrimitive(GetAsIBRepEntity()->CreateInstance());
    else
        clone = Clone();

    Transform worldToLocal;

    worldToLocal.InverseOf(localToWorld);
    clone->TransformInPlace(worldToLocal);

    return clone->GetRange(localRange);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getRange(ICurvePrimitiveCR geom, DRange3dR range, TransformCP transform)
    {
    return (nullptr != transform ? geom.GetRange(range, *transform) : geom.GetRange(range));
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getRange(CurveVectorCR geom, DRange3dR range, TransformCP transform)
    {
    return (nullptr != transform ? geom.GetRange(range, *transform) : geom.GetRange(range));
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getRange(ISolidPrimitiveCR geom, DRange3dR range, TransformCP transform)
    {
    return (nullptr != transform ? geom.GetRange(range, *transform) : geom.GetRange(range));
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getRange(IBRepEntityCR geom, DRange3dR range, TransformCP transform)
    {
    range = geom.GetEntityRange();

    if (nullptr != transform)
        transform->Multiply(range, range);

    return true;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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

        case GeometryType::BRepEntity:
            {
            IBRepEntityCR geom1 = *GetAsIBRepEntity();
            IBRepEntityCR geom2 = *primitive.GetAsIBRepEntity();

            return geom1.IsSameStructureAndGeometry(geom2, tolerance);
            }

        default:
            return false;
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool TopologyPrimitive::IsSolid() const
    {
    switch (GetGeometryType())
        {
        case GeometryType::SolidPrimitive:
            return GetAsISolidPrimitive()->GetCapped();

        case GeometryType::BRepEntity:
            return IBRepEntity::EntityType::Solid == GetAsIBRepEntity()->GetEntityType();

        default:
            return false;
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool TopologyPrimitive::IsSheet() const
    {
    switch (GetGeometryType())
        {
        case GeometryType::CurveVector:
            return GetAsCurveVector()->IsAnyRegionType();

        case GeometryType::SolidPrimitive:
            return !GetAsISolidPrimitive()->GetCapped();

        case GeometryType::BRepEntity:
            return IBRepEntity::EntityType::Sheet == GetAsIBRepEntity()->GetEntityType();

        default:
            return false;
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IBRepEntity::EntityType TopologyPrimitive::GetBRepEntityType() const
    {
    switch (GetGeometryType())
        {
        case GeometryType::CurvePrimitive:
            return IBRepEntity::EntityType::Wire;

        case GeometryType::CurveVector:
            return (GetAsCurveVector()->IsAnyRegionType() ? IBRepEntity::EntityType::Sheet : (GetAsCurveVector()->IsOpenPath() ? IBRepEntity::EntityType::Wire : IBRepEntity::EntityType::Invalid));

        case GeometryType::SolidPrimitive:
            return GetAsISolidPrimitive()->GetCapped() ? IBRepEntity::EntityType::Solid : IBRepEntity::EntityType::Sheet;

        case GeometryType::BRepEntity:
            return GetAsIBRepEntity()->GetEntityType();

        default:
            return IBRepEntity::EntityType::Invalid;
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TopologyPrimitive::TopologyPrimitive(ICurvePrimitivePtr const& source) {m_type = GeometryType::CurvePrimitive; m_data = source;}
TopologyPrimitive::TopologyPrimitive(CurveVectorPtr const& source) {m_type = GeometryType::CurveVector; m_data = source;}
TopologyPrimitive::TopologyPrimitive(ISolidPrimitivePtr const& source) {m_type = GeometryType::SolidPrimitive; m_data = source;}
TopologyPrimitive::TopologyPrimitive(IBRepEntityPtr const& source) {m_type = GeometryType::BRepEntity; m_data = source;}

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TopologyPrimitivePtr TopologyPrimitive::Create(ICurvePrimitivePtr const& source) {return (source.IsValid() ? new TopologyPrimitive(source) : nullptr);}
TopologyPrimitivePtr TopologyPrimitive::Create(CurveVectorPtr const& source) {return (source.IsValid() ? new TopologyPrimitive(source) : nullptr);}
TopologyPrimitivePtr TopologyPrimitive::Create(ISolidPrimitivePtr const& source) {return (source.IsValid() ? new TopologyPrimitive(source) : nullptr);}
TopologyPrimitivePtr TopologyPrimitive::Create(IBRepEntityPtr const& source) {return (source.IsValid() ? new TopologyPrimitive(source) : nullptr);}

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TopologyPrimitivePtr TopologyPrimitive::Create(ICurvePrimitiveCR source) {ICurvePrimitivePtr clone = source.Clone(); return Create(clone);}
TopologyPrimitivePtr TopologyPrimitive::Create(CurveVectorCR source) {CurveVectorPtr clone = source.Clone(); return Create(clone);}
TopologyPrimitivePtr TopologyPrimitive::Create(ISolidPrimitiveCR source) {ISolidPrimitivePtr clone = source.Clone(); return Create(clone);}
TopologyPrimitivePtr TopologyPrimitive::Create(IBRepEntityCR source) {IBRepEntityPtr clone = source.Clone(); return Create(clone);}

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TopologyPrimitive::GeometryType TopologyPrimitive::GetGeometryType() const {return m_type;}
ICurvePrimitivePtr TopologyPrimitive::GetAsICurvePrimitive() const {return (GeometryType::CurvePrimitive == m_type ? static_cast <ICurvePrimitiveP> (m_data.get()) : nullptr);}
CurveVectorPtr TopologyPrimitive::GetAsCurveVector() const {return (GeometryType::CurveVector == m_type ? static_cast <CurveVectorP> (m_data.get()) : nullptr);}
ISolidPrimitivePtr TopologyPrimitive::GetAsISolidPrimitive() const {return (GeometryType::SolidPrimitive == m_type ? static_cast <ISolidPrimitiveP> (m_data.get()) : nullptr);}
IBRepEntityPtr TopologyPrimitive::GetAsIBRepEntity() const {return (GeometryType::BRepEntity == m_type ? static_cast <IBRepEntityP> (m_data.get()) : nullptr);}

END_BENTLEY_GEOMETRY_NAMESPACE