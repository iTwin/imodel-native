/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ElementGeometry.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <GeomSerialization/GeomLibsFlatBufferApi.h>
#include <DgnPlatformInternal/DgnCore/ElementGraphics.fb.h>
#include <DgnPlatformInternal/DgnCore/TextStringPersistence.h>
#include "DgnPlatform/Annotations/TextAnnotationDraw.h"

using namespace flatbuffers;

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometry::GetLocalCoordinateFrame(TransformR localToWorld) const
    {
    switch (GetGeometryType())
        {
        case GeometryType::CurvePrimitive:
            {
            ICurvePrimitivePtr curve = GetAsICurvePrimitive();

            if (!curve->FractionToFrenetFrame(0.0, localToWorld))
                {
                DPoint3d point;

                if (curve->GetStartPoint(point))
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
            CurveVectorPtr curves = GetAsCurveVector();

            if (!curves->GetAnyFrenetFrame(localToWorld))
                {
                DPoint3d point;

                if (curves->GetStartPoint(point))
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
            Transform          worldToLocal;
            ISolidPrimitivePtr solidPrimitive = GetAsISolidPrimitive();

            if (!solidPrimitive->TryGetConstructiveFrame(localToWorld, worldToLocal))
                {
                localToWorld.InitIdentity();
                return false;
                }

            break;
            }

        case GeometryType::Polyface:
            {
            PolyfaceHeaderPtr polyface = GetAsPolyfaceHeader();

            double      area;
            DPoint3d    centroid;
            RotMatrix   axes;
            DVec3d      momentXYZ;

            if (!polyface->ComputePrincipalAreaMoments(area, centroid, axes, momentXYZ))
                {
                localToWorld.InitIdentity();
                return false;
                }

            localToWorld.InitFrom(axes, centroid);
            break;
            }

        case GeometryType::BsplineSurface:
            {
            MSBsplineSurfacePtr surface = GetAsMSBsplineSurface();

            double      area;
            DPoint3d    centroid;
            RotMatrix   axes;
            DVec3d      momentXYZ;

            if (!surface->ComputePrincipalAreaMoments(area, (DVec3dR) centroid, axes, momentXYZ))
                {
                localToWorld.InitIdentity();
                return false;
                }

            localToWorld.InitFrom(axes, centroid);
            break;
            }

        case GeometryType::SolidKernelEntity:
            {
            ISolidKernelEntityPtr entity = GetAsISolidKernelEntity();

            // The entity transform (after removing SWA scale) can be used for localToWorld (solid kernel to uors)...
            localToWorld = entity->GetEntityTransform();
            break;
            }

        case GeometryType::TextString:
            {
            TextStringCR text = *GetAsTextString();
            localToWorld.InitFrom(text.GetOrientation(), text.GetOrigin());
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
bool ElementGeometry::GetLocalRange(DRange3dR localRange, TransformR localToWorld) const
    {
    if (!GetLocalCoordinateFrame(localToWorld))
        return false;

    if (localToWorld.IsIdentity())
        return GetRange(localRange);

    ElementGeometryPtr clone;

    // NOTE: Avoid un-necessary copy of BRep. We just need to change entity transform...
    if (GeometryType::SolidKernelEntity == GetGeometryType())
        {
        ISolidKernelEntityPtr geom;

        if (SUCCESS != T_HOST.GetSolidsKernelAdmin()._InstanceEntity(geom, *GetAsISolidKernelEntity()))
            return false;

        clone = new ElementGeometry(geom);
        }
    else
        {
        clone = Clone();
        }

    Transform   worldToLocal;

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
    range = geom.PointRange();

    if (nullptr != transform)
        transform->Multiply(range, range);

    return true;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getRange(MSBsplineSurfaceCR geom, DRange3dR range, TransformCP transform)
    {
    // NOTE: MSBsplineSurface::GetPoleRange doesn't give a nice fitted box...
    IFacetOptionsPtr          facetOpt = IFacetOptions::Create();
    IPolyfaceConstructionPtr  builder = IPolyfaceConstruction::Create(*facetOpt);

    builder->Add(geom);

    return getRange(builder->GetClientMeshR(), range, transform);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getRange(ISolidKernelEntityCR geom, DRange3dR range, TransformCP transform)
    {
    if (SUCCESS != DgnPlatformLib::QueryHost()->GetSolidsKernelAdmin()._GetEntityRange(range, geom))
        return false;

    geom.GetEntityTransform().Multiply(range, range);

    if (nullptr != transform)
        transform->Multiply(range, range);

    return true;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Jeff.Marker     05/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getRange(TextStringCR text, DRange3dR range, TransformCP transform)
    {
    DRange2dCR textRange = text.GetRange();
    range.low.Init(textRange.low);
    range.high.Init(textRange.high);

    Transform textTransform = text.ComputeTransform();
    textTransform.Multiply(&range.low, 2);

    if (nullptr != transform)
        transform->Multiply(range, range);

    return true;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometry::GetRange(DRange3dR range, TransformCP transform) const
    {
    range.Init();

    switch (GetGeometryType())
        {
        case GeometryType::CurvePrimitive:
            {
            ICurvePrimitivePtr geom = GetAsICurvePrimitive();

            return getRange(*geom, range, transform);
            }

        case GeometryType::CurveVector:
            {
            CurveVectorPtr geom = GetAsCurveVector();

            return getRange(*geom, range, transform);
            }

        case GeometryType::SolidPrimitive:
            {
            ISolidPrimitivePtr geom = GetAsISolidPrimitive();

            return getRange(*geom, range, transform);
            }

        case GeometryType::Polyface:
            {
            PolyfaceHeaderPtr geom = GetAsPolyfaceHeader();

            return getRange(*geom, range, transform);
            }

        case GeometryType::BsplineSurface:
            {
            MSBsplineSurfacePtr geom = GetAsMSBsplineSurface();

            return getRange(*geom, range, transform);
            }

        case GeometryType::SolidKernelEntity:
            {
            ISolidKernelEntityPtr geom = GetAsISolidKernelEntity();

            return getRange(*geom, range, transform);
            }

        case GeometryType::TextString:
            {
            TextStringPtr geom = GetAsTextString();

            return getRange(*geom, range, transform);
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
bool ElementGeometry::TransformInPlace(TransformCR transform)
    {
    switch (GetGeometryType())
        {
        case GeometryType::CurvePrimitive:
            {
            ICurvePrimitivePtr geom = GetAsICurvePrimitive();

            return geom->TransformInPlace(transform);
            }

        case GeometryType::CurveVector:
            {
            CurveVectorPtr geom = GetAsCurveVector();

            return geom->TransformInPlace(transform);
            }

        case GeometryType::SolidPrimitive:
            {
            ISolidPrimitivePtr geom = GetAsISolidPrimitive();

            return geom->TransformInPlace(transform);
            }

        case GeometryType::Polyface:
            {
            PolyfaceHeaderPtr geom = GetAsPolyfaceHeader();

            geom->Transform(transform);

            return true;
            }

        case GeometryType::BsplineSurface:
            {
            MSBsplineSurfacePtr geom = GetAsMSBsplineSurface();

            return (SUCCESS == geom->TransformSurface(transform) ? true : false);
            }

        case GeometryType::SolidKernelEntity:
            {
            ISolidKernelEntityPtr geom = GetAsISolidKernelEntity();

            geom->PreMultiplyEntityTransformInPlace(transform); // Just change entity transform...

            return true;
            }

        case GeometryType::TextString:
            {
            TextStringR text = *GetAsTextString();
            text.ApplyTransform(transform);

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
ElementGeometryPtr ElementGeometry::Clone() const
    {
    switch (GetGeometryType())
        {
        case GeometryType::CurvePrimitive:
            {
            ICurvePrimitivePtr geom = GetAsICurvePrimitive()->Clone();

            return new ElementGeometry(geom);
            }

        case GeometryType::CurveVector:
            {
            CurveVectorPtr geom = GetAsCurveVector()->Clone();

            return new ElementGeometry(geom);
            }

        case GeometryType::SolidPrimitive:
            {
            ISolidPrimitivePtr geom = GetAsISolidPrimitive()->Clone();

            return new ElementGeometry(geom);
            }

        case GeometryType::Polyface:
            {
            PolyfaceHeaderPtr geom = GetAsPolyfaceHeader()->Clone();

            return new ElementGeometry(geom);
            }

        case GeometryType::BsplineSurface:
            {
            MSBsplineSurfacePtr geom = GetAsMSBsplineSurface()->Clone();

            return new ElementGeometry(geom);
            }

        case GeometryType::SolidKernelEntity:
            {
            ISolidKernelEntityPtr geom = GetAsISolidKernelEntity()->Clone();

            return new ElementGeometry(geom);
            }

        case GeometryType::TextString:
            {
            TextStringPtr text = GetAsTextString()->Clone();

            return new ElementGeometry(text);
            }

        default:
            {
            BeAssert(false);
            return nullptr;
            }
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeometry::Draw(ViewContextR context) const
    {
    // Do we need to worry about 2d draw (display priority) and fill, etc.?
    switch (GetGeometryType())
        {
        case GeometryType::CurvePrimitive:
            {
            ICurvePrimitivePtr geom = GetAsICurvePrimitive();
            CurveVectorPtr curveGeom = CurveVector::Create(ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString == geom->GetCurvePrimitiveType() ? CurveVector::BOUNDARY_TYPE_None : CurveVector::BOUNDARY_TYPE_Open, geom);

            context.GetIDrawGeom().DrawCurveVector(*curveGeom, false);
            break;
            }

        case GeometryType::CurveVector:
            {
            CurveVectorPtr geom = GetAsCurveVector();

            context.GetIDrawGeom().DrawCurveVector(*geom, false);
            break;
            }

        case GeometryType::SolidPrimitive:
            {
            ISolidPrimitivePtr geom = GetAsISolidPrimitive();

            context.GetIDrawGeom().DrawSolidPrimitive(*geom);
            break;
            }

        case GeometryType::Polyface:
            {
            PolyfaceHeaderPtr geom = GetAsPolyfaceHeader();

            context.GetIDrawGeom().DrawPolyface(*geom, false);
            break;
            }

        case GeometryType::BsplineSurface:
            {
            MSBsplineSurfacePtr geom = GetAsMSBsplineSurface();

            context.GetIDrawGeom().DrawBSplineSurface(*geom);
            break;
            }

        case GeometryType::SolidKernelEntity:
            {
            ISolidKernelEntityPtr geom = GetAsISolidKernelEntity();

            context.GetIDrawGeom().DrawBody(*geom);
            break;
            }

        case GeometryType::TextString:
            {
            TextStringPtr geom = GetAsTextString();

            context.DrawTextString(*geom);
            break;
            }

        default:
            {
            BeAssert(false);
            break;
            }
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/15
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometry::ElementGeometry(ICurvePrimitivePtr const& source) {m_type = GeometryType::CurvePrimitive; m_data = source;}
ElementGeometry::ElementGeometry(CurveVectorPtr const& source) {m_type = GeometryType::CurveVector; m_data = source;}
ElementGeometry::ElementGeometry(ISolidPrimitivePtr const& source) {m_type = GeometryType::SolidPrimitive; m_data = source;}
ElementGeometry::ElementGeometry(MSBsplineSurfacePtr const& source) {m_type = GeometryType::BsplineSurface; m_data = source;}
ElementGeometry::ElementGeometry(PolyfaceHeaderPtr const& source) {m_type = GeometryType::Polyface; m_data = source;}
ElementGeometry::ElementGeometry(ISolidKernelEntityPtr const& source) {m_type = GeometryType::SolidKernelEntity; m_data = source;}
ElementGeometry::ElementGeometry(TextStringPtr const& source) {m_type = GeometryType::TextString; m_data = source;}

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/15
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryPtr ElementGeometry::Create(ICurvePrimitivePtr const& source) {return (source.IsValid() ? new ElementGeometry(source) : nullptr);}
ElementGeometryPtr ElementGeometry::Create(CurveVectorPtr const& source) {return (source.IsValid() ? new ElementGeometry(source) : nullptr);}
ElementGeometryPtr ElementGeometry::Create(ISolidPrimitivePtr const& source) {return (source.IsValid() ? new ElementGeometry(source) : nullptr);}
ElementGeometryPtr ElementGeometry::Create(MSBsplineSurfacePtr const& source) {return (source.IsValid() ? new ElementGeometry(source) : nullptr);}
ElementGeometryPtr ElementGeometry::Create(PolyfaceHeaderPtr const& source) {return (source.IsValid() ? new ElementGeometry(source) : nullptr);}
ElementGeometryPtr ElementGeometry::Create(ISolidKernelEntityPtr const& source) {return (source.IsValid() ? new ElementGeometry(source) : nullptr);}
ElementGeometryPtr ElementGeometry::Create(TextStringPtr const& source) {return (source.IsValid() ? new ElementGeometry(source) : nullptr);}

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/15
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryPtr ElementGeometry::Create(ICurvePrimitiveCR source) {ICurvePrimitivePtr clone = source.Clone(); return Create(clone);}
ElementGeometryPtr ElementGeometry::Create(CurveVectorCR source) {CurveVectorPtr clone = source.Clone(); return Create(clone);}
ElementGeometryPtr ElementGeometry::Create(ISolidPrimitiveCR source) {ISolidPrimitivePtr clone = source.Clone(); return Create(clone);}
ElementGeometryPtr ElementGeometry::Create(MSBsplineSurfaceCR source) {MSBsplineSurfacePtr clone = source.Clone(); return Create(clone);}
ElementGeometryPtr ElementGeometry::Create(PolyfaceQueryCR source) {PolyfaceHeaderPtr clone = source.Clone(); return Create(clone);}
ElementGeometryPtr ElementGeometry::Create(ISolidKernelEntityCR source) {ISolidKernelEntityPtr clone = source.Clone(); return Create(clone);}
ElementGeometryPtr ElementGeometry::Create(TextStringCR source) {TextStringPtr clone = source.Clone(); return Create(clone);}

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/15
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometry::GeometryType ElementGeometry::GetGeometryType() const {return m_type;}
ICurvePrimitivePtr ElementGeometry::GetAsICurvePrimitive() const {return (GeometryType::CurvePrimitive == m_type ? static_cast <ICurvePrimitiveP> (m_data.get()) : nullptr);}
CurveVectorPtr ElementGeometry::GetAsCurveVector() const {return (GeometryType::CurveVector == m_type ? static_cast <CurveVectorP> (m_data.get()) : nullptr);}
ISolidPrimitivePtr ElementGeometry::GetAsISolidPrimitive() const {return (GeometryType::SolidPrimitive == m_type ? static_cast <ISolidPrimitiveP> (m_data.get()) : nullptr);}
MSBsplineSurfacePtr ElementGeometry::GetAsMSBsplineSurface() const {return (GeometryType::BsplineSurface == m_type ? static_cast <RefCountedMSBsplineSurface*> (m_data.get()) : nullptr);}
PolyfaceHeaderPtr ElementGeometry::GetAsPolyfaceHeader() const {return (GeometryType::Polyface == m_type ? static_cast <PolyfaceHeaderP> (m_data.get()) : nullptr);}
ISolidKernelEntityPtr ElementGeometry::GetAsISolidKernelEntity() const { return (GeometryType::SolidKernelEntity == m_type ? static_cast <ISolidKernelEntityP> (m_data.get()) : nullptr); }
TextStringPtr ElementGeometry::GetAsTextString() const { return (GeometryType::TextString == m_type ? static_cast <TextStringP> (m_data.get()) : nullptr); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Operation::IsGeometryOp() const
    {
    switch (m_opCode)
        {
        case OpCode::PointPrimitive:
        case OpCode::PointPrimitive2d:
        case OpCode::ArcPrimitive:
        case OpCode::CurveVector:
        case OpCode::Polyface:
        case OpCode::CurvePrimitive:
        case OpCode::SolidPrimitive:
        case OpCode::BsplineSurface:
        case OpCode::ParasolidBRep:
        case OpCode::BRepPolyface:
        case OpCode::BRepPolyfaceExact:
        case OpCode::BRepEdges:
        case OpCode::BRepFaceIso:
        case OpCode::TextString:
            return true;

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Iterator::ToNext()
    {
    if (m_dataOffset >= m_totalDataSize)
        {
        m_data = nullptr;
        m_dataOffset = 0;

        return;
        }

    uint32_t        opCode = *((uint32_t *) (m_data));
    uint32_t        dataSize = *((uint32_t *) (m_data + sizeof (opCode)));
    uint8_t const*  data = (0 != dataSize ? (uint8_t const*) (m_data + sizeof (opCode) + sizeof (dataSize)) : nullptr);
    size_t          egOpSize = sizeof (opCode) + sizeof (dataSize) + dataSize;

    m_egOp = Operation((OpCode) (opCode), dataSize, data);
    m_data += egOpSize;
    m_dataOffset += egOpSize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Writer::Append(Operation const& egOp)
    {
    uint32_t paddedDataSize = (egOp.m_dataSize + 7) & ~7; // 8 byte aligned...
    size_t   egOpSize = sizeof (egOp.m_opCode) + sizeof (egOp.m_dataSize) + paddedDataSize;
    size_t   currSize = m_buffer.size();

    m_buffer.resize(currSize + egOpSize);

    uint8_t*  currOffset = &(m_buffer.at(currSize));

    memcpy(currOffset, &egOp.m_opCode, sizeof (egOp.m_opCode));
    currOffset += sizeof (egOp.m_opCode);

    memcpy(currOffset, &paddedDataSize, sizeof (paddedDataSize));
    currOffset += sizeof (paddedDataSize);

    if (0 == egOp.m_dataSize)
        return;

    memcpy(currOffset, egOp.m_data, egOp.m_dataSize);
    currOffset += egOp.m_dataSize;

    if (paddedDataSize > egOp.m_dataSize)
        memset(currOffset, 0, paddedDataSize - egOp.m_dataSize); // Pad quietly or also assert?
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Writer::Append(DPoint2dCP pts, size_t nPts, int8_t boundary)
    {
    FlatBufferBuilder fbb;

    auto coords = fbb.CreateVectorOfStructs((FB::DPoint2d*) pts, nPts);

    FB::PointPrimitive2dBuilder builder(fbb);

    builder.add_coords(coords);
    builder.add_boundary((FB::BoundaryType) boundary);

    auto mloc = builder.Finish();

    fbb.Finish(mloc);
    Append(Operation(OpCode::PointPrimitive2d, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Writer::Append(DPoint3dCP pts, size_t nPts, int8_t boundary)
    {
    FlatBufferBuilder fbb;

    auto coords = fbb.CreateVectorOfStructs((FB::DPoint3d*) pts, nPts);

    FB::PointPrimitiveBuilder builder(fbb);

    builder.add_coords(coords);
    builder.add_boundary((FB::BoundaryType) boundary);

    auto mloc = builder.Finish();

    fbb.Finish(mloc);
    Append(Operation(OpCode::PointPrimitive, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Writer::Append(DEllipse3dCR arc, int8_t boundary)
    {
    FlatBufferBuilder fbb;

    auto mloc = FB::CreateArcPrimitive(fbb, (FB::DPoint3d*) &arc.center, (FB::DVec3d*) &arc.vector0, (FB::DVec3d*) &arc.vector90, arc.start, arc.sweep, (FB::BoundaryType) boundary);

    fbb.Finish(mloc);
    Append(Operation(OpCode::ArcPrimitive, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Writer::AppendSimplified(ICurvePrimitiveCR curvePrimitive, bool isClosed, bool is3d)
    {
    // Special case single/simple curve primitives to avoid having to call new during draw...
    switch (curvePrimitive.GetCurvePrimitiveType())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            {
            DSegment3dCP segment = curvePrimitive.GetLineCP();

            if (!is3d)
                {
                DPoint2d localPoints2dBuf[2];

                localPoints2dBuf[0].Init(segment->point[0]);
                localPoints2dBuf[1].Init(segment->point[1]);

                Append(localPoints2dBuf, 2, FB::BoundaryType_Open);

                return true;
                }

            Append(segment->point, 2, FB::BoundaryType_Open);

            return true;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            {
            bvector<DPoint3d> const* points = curvePrimitive.GetLineStringCP();

            if (!is3d)
                {
                int nPts = (int) points->size();
                std::valarray<DPoint2d> localPoints2dBuf(nPts);

                for (int iPt = 0; iPt < nPts; ++iPt)
                    localPoints2dBuf[iPt].Init(points->at(iPt));

                Append(&localPoints2dBuf[0], nPts, (int8_t) (isClosed ? FB::BoundaryType_Closed : FB::BoundaryType_Open));

                return true;
                }

            Append(&points->front(), points->size(), (int8_t) (isClosed ? FB::BoundaryType_Closed : FB::BoundaryType_Open));

            return true;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
            {
            bvector<DPoint3d> const* points = curvePrimitive.GetPointStringCP();

            if (!is3d)
                {
                int nPts = (int) points->size();
                std::valarray<DPoint2d> localPoints2dBuf(nPts);

                for (int iPt = 0; iPt < nPts; ++iPt)
                    localPoints2dBuf[iPt].Init(points->at(iPt));

                Append(&localPoints2dBuf[0], nPts, FB::BoundaryType_None);

                return true;
                }

            Append(&points->front(), points->size(), FB::BoundaryType_None);

            return true;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            {
            DEllipse3dCP  ellipse = curvePrimitive.GetArcCP();

            Append(*ellipse, (int8_t) (isClosed ? FB::BoundaryType_Closed : FB::BoundaryType_Open));

            return true;
            }

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Writer::AppendSimplified(CurveVectorCR curves, bool is3d)
    {
    // Special case to avoid having to call new during draw...
    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Invalid == curves.HasSingleCurvePrimitive())
        return false;

    return AppendSimplified(*curves.front(), curves.IsClosedPath(), is3d);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Writer::AppendSimplified(ElementGeometryCR geom, bool is3d)
    {
    switch (geom.GetGeometryType())
        {
        case ElementGeometry::GeometryType::CurvePrimitive:
            return AppendSimplified(*geom.GetAsICurvePrimitive(), false, is3d);

        case ElementGeometry::GeometryType::CurveVector:
            return AppendSimplified(*geom.GetAsCurveVector(), is3d);

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Writer::Append(CurveVectorCR curves)
    {
    bvector<Byte> buffer;

    BentleyGeometryFlatBuffer::GeometryToBytes(curves, buffer);

    if (0 == buffer.size())
        {
        BeAssert(false);
        return;
        }

    Append(Operation(OpCode::CurveVector, (uint32_t) buffer.size(), &buffer.front()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Writer::Append(ICurvePrimitiveCR curvePrimitive)
    {
    OpCode        opCode;
    bvector<Byte> buffer;

    BentleyGeometryFlatBuffer::GeometryToBytes(curvePrimitive, buffer);
    opCode = OpCode::CurvePrimitive;

    if (0 == buffer.size())
        {
        BeAssert(false);
        return;
        }

    Append(Operation(opCode, (uint32_t) buffer.size(), &buffer.front()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Writer::Append(PolyfaceQueryCR meshData)
    {
    bvector<Byte> buffer;

    BentleyGeometryFlatBuffer::GeometryToBytes(meshData, buffer);

    if (0 == buffer.size())
        {
        BeAssert(false);
        return;
        }

    Append(Operation(OpCode::Polyface, (uint32_t) buffer.size(), &buffer.front()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Writer::Append(ISolidPrimitiveCR solid)
    {
    bvector<Byte> buffer;

    BentleyGeometryFlatBuffer::GeometryToBytes(solid, buffer);

    if (0 == buffer.size())
        {
        BeAssert(false);
        return;
        }

    Append(Operation(OpCode::SolidPrimitive, (uint32_t) buffer.size(), &buffer.front()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Writer::Append(MSBsplineSurfaceCR surface)
    {
    bvector<Byte> buffer;

    BentleyGeometryFlatBuffer::GeometryToBytes(surface, buffer);

    if (0 == buffer.size())
        {
        BeAssert(false);
        return;
        }

    Append(Operation(OpCode::BsplineSurface, (uint32_t) buffer.size(), &buffer.front()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Writer::Append(ISolidKernelEntityCR entity, bool saveBRepOnly)
    {
    bool saveBRep = saveBRepOnly, saveFacets = false, saveEdges = false, saveFaceIso = false;
    IFaceMaterialAttachmentsCP attachments = entity.GetFaceMaterialAttachments();

    if (!saveBRepOnly)
        {
        switch (entity.GetEntityType())
            {
            case ISolidKernelEntity::EntityType_Wire:
                {
                // Save wire body as CurveVector...very in-efficent and un-necessary to persist these as BReps...
                CurveVectorPtr wireGeom = DgnPlatformLib::QueryHost()->GetSolidsKernelAdmin()._WireBodyToCurveVector(entity);

                if (wireGeom.IsValid())
                    Append(*wireGeom);

                return;
                }

            case ISolidKernelEntity::EntityType_Sheet:
                {
                // Save sheet body that is a single planar face as CurveVector...very in-efficent and un-necessary to persist these as BReps...
                CurveVectorPtr faceGeom = DgnPlatformLib::QueryHost()->GetSolidsKernelAdmin()._PlanarSheetBodyToCurveVector(entity);

                if (faceGeom.IsValid())
                    {
                    Append(*faceGeom);
                    return;
                    }

                // Fall through...
                }

            case ISolidKernelEntity::EntityType_Solid:
                {
                saveBRep = saveFacets = true;

                if (!DgnPlatformLib::QueryHost()->GetSolidsKernelAdmin()._QueryEntityData(entity, DgnPlatformLib::Host::SolidsKernelAdmin::EntityQuery_HasOnlyPlanarFaces))
                    saveFaceIso = true;

                // NOTE: Never want OpCode::BRepPolyfaceExact when split by face symbology...
                if (attachments || saveFaceIso || DgnPlatformLib::QueryHost()->GetSolidsKernelAdmin()._QueryEntityData(entity, DgnPlatformLib::Host::SolidsKernelAdmin::EntityQuery_HasCurvedFaceOrEdge))
                    saveEdges = true;
                break;
                }
            }
        }

    // Make the parasolid data available for platforms that can support it...MUST BE ADDED FIRST!!!
    if (saveBRep)
        {
        size_t      bufferSize = 0;
        uint8_t*    buffer = nullptr;

        if (SUCCESS != T_HOST.GetSolidsKernelAdmin()._SaveEntityToMemory(&buffer, bufferSize, entity))
            {
            BeAssert(false);
            return;
            }

        bvector<FB::FaceSymbology> fbSymbVec;
        bvector<FB::FaceSymbologyIndex> fbSymbIndexVec;

        if (nullptr != attachments)
            {
            T_FaceAttachmentsVec const& faceAttachmentsVec = attachments->_GetFaceAttachmentsVec();

            for (FaceAttachment attachment : faceAttachmentsVec)
                {
                // NOTE: First entry is base symbology, it's redundant with GeomStream, storing it makes implementing Get easier/cleaner...
                FB::DPoint2d       uv(0.0, 0.0); // NEEDSWORK_WIP_MATERIAL - Add geometry specific material mappings to ElemDisplayParams/ElemMatSymb...
                ElemDisplayParams  faceParams;

                attachment.ToElemDisplayParams(faceParams);

                FB::FaceSymbology  fbSymb(!faceParams.IsLineColorFromSubCategoryAppearance(), !faceParams.IsMaterialFromSubCategoryAppearance(),
                                          faceParams.IsLineColorFromSubCategoryAppearance() ? 0 : faceParams.GetLineColor().GetValue(),
                                          faceParams.IsMaterialFromSubCategoryAppearance() ? 0 : faceParams.GetMaterial().GetValueUnchecked(),
                                          faceParams.GetTransparency(), uv);

                fbSymbVec.push_back(fbSymb);
                }

            T_FaceToSubElemIdMap const& faceToSubElemIdMap = attachments->_GetFaceToSubElemIdMap();

            for (T_FaceToSubElemIdMap::const_iterator curr = faceToSubElemIdMap.begin(); curr != faceToSubElemIdMap.end(); ++curr)
                {
                FB::FaceSymbologyIndex fbSymbIndex(curr->second.first, (uint32_t) curr->second.second);

                fbSymbIndexVec.push_back(fbSymbIndex);
                }
            }

        FlatBufferBuilder fbb;

        auto entityData = fbb.CreateVector(buffer, bufferSize);
        auto faceSymb = 0 != fbSymbVec.size() ? fbb.CreateVectorOfStructs(&fbSymbVec.front(), fbSymbVec.size()) : 0;
        auto faceSymbIndex = 0 != fbSymbIndexVec.size() ? fbb.CreateVectorOfStructs(&fbSymbIndexVec.front(), fbSymbIndexVec.size()) : 0;

        FB::BRepDataBuilder builder(fbb);

        builder.add_entityTransform((FB::Transform*) &entity.GetEntityTransform());
        builder.add_brepType((FB::BRepType) entity.GetEntityType()); // Allow possibility of checking type w/o expensive restore of brep...
        builder.add_entityData(entityData);

        if (nullptr != attachments)
            {
            builder.add_symbology(faceSymb);
            builder.add_symbologyIndex(faceSymbIndex);
            }

        auto mloc = builder.Finish();

        fbb.Finish(mloc);
        Append(Operation(OpCode::ParasolidBRep, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
        }

    // Store mesh representation for quick display or when parasolid isn't available...
    if (saveFacets)
        {
        IFacetOptionsPtr  facetOpt = IFacetOptions::CreateForCurves();

        if (nullptr != attachments)
            {
            bvector<PolyfaceHeaderPtr> polyfaces;
            bvector<ElemDisplayParams> params;

            WireframeGeomUtil::CollectPolyfaces(entity, m_db, polyfaces, params, *facetOpt);

            for (size_t i=0; i < polyfaces.size(); i++)
                {
                if (0 == polyfaces[i]->GetPointCount())
                    continue;

                bvector<Byte> buffer;

                BentleyGeometryFlatBuffer::GeometryToBytes(*polyfaces[i], buffer);

                if (0 == buffer.size())
                    continue;

                Append(params[i], true);
                Append(Operation(OpCode::BRepPolyface, (uint32_t) buffer.size(), &buffer.front()));
                }
            }
        else
            {
            PolyfaceHeaderPtr polyface = WireframeGeomUtil::CollectPolyface(entity, m_db, *facetOpt);

            if (polyface.IsValid())
                {
                bvector<Byte> buffer;

                BentleyGeometryFlatBuffer::GeometryToBytes(*polyface, buffer);

                if (0 != buffer.size())
                    Append(Operation(saveEdges ? OpCode::BRepPolyface : OpCode::BRepPolyfaceExact, (uint32_t) buffer.size(), &buffer.front()));
                }
            }
        }

    // When facetted representation is an approximation, we need to store the edge curves for snapping...
    if (saveEdges)
        {
        if (nullptr != attachments)
            {
            bvector<CurveVectorPtr> curves;
            bvector<ElemDisplayParams> params;

            WireframeGeomUtil::CollectCurves(entity, m_db, curves, params, true, false);

            for (size_t i=0; i < curves.size(); i++)
                {
                if (0 == curves[i]->size())
                    continue;

                bvector<Byte> buffer;

                BentleyGeometryFlatBuffer::GeometryToBytes(*curves[i], buffer);

                if (0 == buffer.size())
                    continue;

                Append(params[i], true);
                Append(Operation(OpCode::BRepEdges, (uint32_t) buffer.size(), &buffer.front()));
                }
            }
        else
            {
            CurveVectorPtr edgeCurves = WireframeGeomUtil::CollectCurves(entity, m_db, true, false);

            if (edgeCurves.IsValid())
                {
                bvector<Byte> buffer;

                BentleyGeometryFlatBuffer::GeometryToBytes(*edgeCurves, buffer);

                if (0 != buffer.size())
                    Append(Operation(OpCode::BRepEdges, (uint32_t) buffer.size(), &buffer.front()));
                }
            }
        }

    // When facetted representation is an approximation, we need to store the face-iso curves for wireframe display...
    if (saveFaceIso)
        {
        if (nullptr != attachments)
            {
            bvector<CurveVectorPtr> curves;
            bvector<ElemDisplayParams> params;

            WireframeGeomUtil::CollectCurves(entity, m_db, curves, params, false, true);

            for (size_t i=0; i < curves.size(); i++)
                {
                if (0 == curves[i]->size())
                    continue;

                bvector<Byte> buffer;

                BentleyGeometryFlatBuffer::GeometryToBytes(*curves[i], buffer);

                if (0 == buffer.size())
                    continue;

                Append(params[i], true);
                Append(Operation(OpCode::BRepFaceIso, (uint32_t) buffer.size(), &buffer.front()));
                }
            }
        else
            {
            CurveVectorPtr faceCurves = WireframeGeomUtil::CollectCurves(entity, m_db, false, true);

            if (faceCurves.IsValid())
                {
                bvector<Byte> buffer;

                BentleyGeometryFlatBuffer::GeometryToBytes(*faceCurves, buffer);

                if (0 != buffer.size())
                    Append(Operation(OpCode::BRepFaceIso, (uint32_t) buffer.size(), &buffer.front()));
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Writer::Append(DgnGeomPartId geomPart, TransformCP geomToElem)
    {
    if (nullptr == geomToElem || geomToElem->IsIdentity())
        {
        FlatBufferBuilder fbb;

        auto mloc = FB::CreateGeomPart(fbb, geomPart.GetValueUnchecked());

        fbb.Finish(mloc);
        Append(Operation(OpCode::GeomPartInstance, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
        return;
        }

    DPoint3d            origin;
    RotMatrix           rMatrix;
    YawPitchRollAngles  angles;

    geomToElem->GetTranslation(origin);
    geomToElem->GetMatrix(rMatrix);
    YawPitchRollAngles::TryFromRotMatrix(angles, rMatrix);

    FlatBufferBuilder fbb;

    auto mloc = FB::CreateGeomPart(fbb, geomPart.GetValueUnchecked(), (FB::DPoint3d*) &origin, angles.GetYaw().Degrees(), angles.GetPitch().Degrees(), angles.GetRoll().Degrees());

    fbb.Finish(mloc);
    Append(Operation(OpCode::GeomPartInstance, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Writer::Append(ElemDisplayParamsCR elParams, bool ignoreSubCategory)
    {
    bool useColor  = !elParams.IsLineColorFromSubCategoryAppearance();
    bool useWeight = !elParams.IsWeightFromSubCategoryAppearance();
    bool useStyle  = !elParams.IsLineStyleFromSubCategoryAppearance();

    if (useColor || useWeight || useStyle || 0.0 != elParams.GetTransparency() || 0 != elParams.GetDisplayPriority() || DgnGeometryClass::Primary != elParams.GetGeometryClass())
        {
        FlatBufferBuilder fbb;

        auto mloc = FB::CreateBasicSymbology(fbb, ignoreSubCategory ? 0 : elParams.GetSubCategoryId().GetValueUnchecked(),
                                             useColor ? elParams.GetLineColor().GetValue() : 0,
                                             useWeight ? elParams.GetWeight() : 0,
                                             useStyle && nullptr != elParams.GetLineStyle() ? elParams.GetLineStyle()->GetStyleId().GetValueUnchecked() : 0,
                                             elParams.GetTransparency(), elParams.GetDisplayPriority(), (FB::GeometryClass) elParams.GetGeometryClass(),
                                             useColor, useWeight, useStyle);
        fbb.Finish(mloc);
        Append(Operation(OpCode::BasicSymbology, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
        }
    else // NOTE: When ignoreSubCategory is set, "all default values" triggers a sub-category appearance reset for the current sub-category...
        {
        FlatBufferBuilder fbb;

        auto mloc = FB::CreateBasicSymbology(fbb, ignoreSubCategory ? 0 : elParams.GetSubCategoryId().GetValueUnchecked());

        fbb.Finish(mloc);
        Append(Operation(OpCode::BasicSymbology, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
        }

#if LINESTYLES_ENABLED
    if (nullptr != elParams.GetLineStyle() && nullptr != elParams.GetLineStyle()->GetStyleParams())
        {
        FlatBufferBuilder   fbb;
        LineStyleParamsCP   lsParams = elParams.GetLineStyle()->GetStyleParams();
        YawPitchRollAngles  angles;

        YawPitchRollAngles::TryFromRotMatrix(angles, lsParams->rMatrix);
        auto modifiers = FB::CreateLineStyleModifiers(fbb, lsParams->modifiers, lsParams->scale, lsParams->dashScale, lsParams->gapScale, lsParams->startWidth, lsParams->endWidth, lsParams->distPhase, lsParams->fractPhase,
                                                      (FB::DPoint3d*)&lsParams->normal, angles.GetYaw().Degrees(), angles.GetPitch().Degrees(), angles.GetRoll().Degrees());
        fbb.Finish(modifiers);
        Append(Operation(OpCode::LineStyleModifiers, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
        }
#endif

    if (FillDisplay::Never != elParams.GetFillDisplay())
        {
        FlatBufferBuilder fbb;

        if (nullptr != elParams.GetGradient())
            {
            GradientSymbCR    gradient = *elParams.GetGradient();
            bvector<uint32_t> keyColors;
            bvector<double>   keyValues;

            for (int i=0; i < gradient.GetNKeys(); i++)
                {
                double   keyValue;
                ColorDef keyColor;

                gradient.GetKey(keyColor, keyValue, i);

                keyColors.push_back(keyColor.GetValue());
                keyValues.push_back(keyValue);
                }

            auto colors = fbb.CreateVector(keyColors);
            auto values = fbb.CreateVector(keyValues);

            auto mloc = FB::CreateAreaFill(fbb, (FB::FillDisplay) elParams.GetFillDisplay(),
                                           0, 0, 0, elParams.GetFillTransparency(),
                                           (FB::GradientMode) gradient.GetMode(), gradient.GetFlags(),
                                           gradient.GetAngle(), gradient.GetTint(), gradient.GetShift(),
                                           colors, values);
            fbb.Finish(mloc);
            }
        else
            {
            bool isBgFill = elParams.IsFillColorFromViewBackground();
            bool useFillColor = !isBgFill && !elParams.IsFillColorFromSubCategoryAppearance();

            auto mloc = FB::CreateAreaFill(fbb, (FB::FillDisplay) elParams.GetFillDisplay(),
                                           useFillColor ? elParams.GetFillColor().GetValue() : 0, useFillColor, isBgFill,
                                           elParams.GetFillTransparency());
            fbb.Finish(mloc);
            }

        Append(Operation(OpCode::AreaFill, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
        }

    // NEEDSWORK_WIP_MATERIAL - Not sure what we need to store per-geometry...
    //                          I assume we'll still need optional uv settings even when using sub-category material.
    //                          So we need a way to check for that case as we can't call GetMaterial
    //                          when !useMaterial because ElemDisplayParams::Resolve hasn't been called...
    bool useMaterial = !elParams.IsMaterialFromSubCategoryAppearance();

    if (useMaterial)
        {
        FlatBufferBuilder fbb;

        auto mloc = FB::CreateMaterial(fbb, useMaterial, useMaterial && elParams.GetMaterial().IsValid() ? elParams.GetMaterial().GetValueUnchecked() : 0, nullptr, nullptr, 0.0, 0.0, 0.0);
        fbb.Finish(mloc);

        Append(Operation(OpCode::Material, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Writer::Append(ElementGeometryCR elemGeom)
    {
    switch (elemGeom.GetGeometryType())
        {
        case ElementGeometry::GeometryType::CurvePrimitive:
            {
            Append(*elemGeom.GetAsICurvePrimitive());
            break;
            }

        case ElementGeometry::GeometryType::CurveVector:
            {
            Append(*elemGeom.GetAsCurveVector());
            break;
            }

        case ElementGeometry::GeometryType::SolidPrimitive:
            {
            Append(*elemGeom.GetAsISolidPrimitive());
            break;
            }

        case ElementGeometry::GeometryType::Polyface:
            {
            Append(*elemGeom.GetAsPolyfaceHeader());
            break;
            }

        case ElementGeometry::GeometryType::BsplineSurface:
            {
            Append(*elemGeom.GetAsMSBsplineSurface());
            break;
            }

        case ElementGeometry::GeometryType::SolidKernelEntity:
            {
            Append(*elemGeom.GetAsISolidKernelEntity());
            break;
            }

        case ElementGeometry::GeometryType::TextString:
            Append(*elemGeom.GetAsTextString());
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Writer::Append(TextStringCR text)
    {
    bvector<Byte> data;
    if (SUCCESS != TextStringPersistence::EncodeAsFlatBuf(data, text, m_db, TextStringPersistence::FlatBufEncodeOptions::IncludeGlyphLayoutData))
        return;

    Append(Operation(OpCode::TextString, (uint32_t)data.size(), &data[0]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Reader::Get(Operation const& egOp, DPoint2dCP& pts, int& nPts, int8_t& boundary) const
    {
    if (OpCode::PointPrimitive2d != egOp.m_opCode)
        return false;

    auto ppfb = flatbuffers::GetRoot<FB::PointPrimitive2d>(egOp.m_data);

    boundary = (int8_t) ppfb->boundary();
    nPts = (int) ppfb->coords()->Length();
    pts = (DPoint2dCP) ppfb->coords()->Data();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Reader::Get(Operation const& egOp, DPoint3dCP& pts, int& nPts, int8_t& boundary) const
    {
    if (OpCode::PointPrimitive != egOp.m_opCode)
        return false;

    auto ppfb = flatbuffers::GetRoot<FB::PointPrimitive>(egOp.m_data);

    boundary = (int8_t) ppfb->boundary();
    nPts = (int) ppfb->coords()->Length();
    pts = (DPoint3dCP) ppfb->coords()->Data();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Reader::Get(Operation const& egOp, DEllipse3dR arc, int8_t& boundary) const
    {
    if (OpCode::ArcPrimitive != egOp.m_opCode)
        return false;

    auto ppfb = flatbuffers::GetRoot<FB::ArcPrimitive>(egOp.m_data);

    arc.InitFromVectors(*((DPoint3dCP) ppfb->center()), *((DVec3dCP) ppfb->vector0()), *((DVec3dCP) ppfb->vector90()), ppfb->start(), ppfb->sweep());
    boundary = (int8_t) ppfb->boundary();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Reader::Get(Operation const& egOp, ICurvePrimitivePtr& curve) const
    {
    if (OpCode::CurvePrimitive != egOp.m_opCode)
        return false;

    curve = BentleyGeometryFlatBuffer::BytesToCurvePrimitive(egOp.m_data);

    return curve.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Reader::Get(Operation const& egOp, CurveVectorPtr& curves) const
    {
    if (OpCode::CurveVector != egOp.m_opCode)
        return false;

    curves = BentleyGeometryFlatBuffer::BytesToCurveVector(egOp.m_data);

    return curves.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Reader::Get(Operation const& egOp, PolyfaceQueryCarrier& meshData) const
    {
    if (OpCode::Polyface != egOp.m_opCode)
        return false;

    return BentleyGeometryFlatBuffer::BytesToPolyfaceQueryCarrier(egOp.m_data, meshData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Reader::Get(Operation const& egOp, ISolidPrimitivePtr& solid) const
    {
    if (OpCode::SolidPrimitive != egOp.m_opCode)
        return false;

    solid = BentleyGeometryFlatBuffer::BytesToSolidPrimitive(egOp.m_data);

    return solid.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Reader::Get(Operation const& egOp, MSBsplineSurfacePtr& surface) const
    {
    if (OpCode::BsplineSurface != egOp.m_opCode)
        return false;

    surface = BentleyGeometryFlatBuffer::BytesToMSBsplineSurface(egOp.m_data);

    return surface.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Reader::Get(Operation const& egOp, ISolidKernelEntityPtr& entity) const
    {
    if (OpCode::ParasolidBRep != egOp.m_opCode)
        return false;

    auto ppfb = flatbuffers::GetRoot<FB::BRepData>(egOp.m_data);

    // NOTE: It's possible to check ppfb->brepType() to avoid calling restore in order to check type...
    if (SUCCESS != T_HOST.GetSolidsKernelAdmin()._RestoreEntityFromMemory(entity, ppfb->entityData()->Data(), ppfb->entityData()->Length(), *((TransformCP) ppfb->entityTransform())))
        return false;

    if (!ppfb->has_symbology() || !ppfb->has_symbologyIndex())
        return true;

    for (size_t iSymb=0; iSymb < ppfb->symbology()->Length(); iSymb++)
        {
        FB::FaceSymbology const* fbSymb = ((FB::FaceSymbology const*) ppfb->symbology()->Data())+iSymb;

        ElemDisplayParams faceParams;

        if (fbSymb->useColor())
            faceParams.SetLineColor(ColorDef(fbSymb->color()));

        if (fbSymb->useMaterial())
            faceParams.SetMaterial(DgnMaterialId((uint64_t)fbSymb->materialId()));

        faceParams.SetTransparency(fbSymb->transparency());

        if (nullptr == entity->GetFaceMaterialAttachments())
            entity->InitFaceMaterialAttachments(&faceParams);
        else
            const_cast<T_FaceAttachmentsVec&>(entity->GetFaceMaterialAttachments()->_GetFaceAttachmentsVec()).push_back(faceParams);
        }

    if (nullptr == entity->GetFaceMaterialAttachments())
        return true;

    T_FaceToSubElemIdMap const& faceToSubElemIdMap = entity->GetFaceMaterialAttachments()->_GetFaceToSubElemIdMap();
    bmap<int32_t, uint32_t> subElemIdToFaceMap;

    for (T_FaceToSubElemIdMap::const_iterator curr = faceToSubElemIdMap.begin(); curr != faceToSubElemIdMap.end(); ++curr)
        subElemIdToFaceMap[curr->second.first] = curr->first;

    for (size_t iSymbIndex=0; iSymbIndex < ppfb->symbologyIndex()->Length(); iSymbIndex++)
        {
        FB::FaceSymbologyIndex const* fbSymbIndex = ((FB::FaceSymbologyIndex const*) ppfb->symbologyIndex()->Data())+iSymbIndex;
        bmap<int32_t, uint32_t>::const_iterator foundIndex = subElemIdToFaceMap.find(fbSymbIndex->faceIndex());

        if (foundIndex == subElemIdToFaceMap.end())
            continue;

        const_cast<T_FaceToSubElemIdMap&>(faceToSubElemIdMap)[foundIndex->second] = make_bpair(fbSymbIndex->faceIndex(), fbSymbIndex->symbIndex());
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Reader::Get(Operation const& egOp, DgnGeomPartId& geomPart, TransformR geomToElem) const
    {
    if (OpCode::GeomPartInstance != egOp.m_opCode)
        return false;

    auto ppfb = flatbuffers::GetRoot<FB::GeomPart>(egOp.m_data);

    geomPart = DgnGeomPartId((uint64_t)ppfb->geomPartId());

    DPoint3d            origin = (nullptr == ppfb->origin() ? DPoint3d::FromZero() : *((DPoint3dCP) ppfb->origin()));
    YawPitchRollAngles  angles = YawPitchRollAngles::FromDegrees(ppfb->yaw(), ppfb->pitch(), ppfb->roll());

    geomToElem = angles.ToTransform(origin);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Reader::Get(Operation const& egOp, ElemDisplayParamsR elParams) const
    {
    bool changed = false;

    switch (egOp.m_opCode)
        {
        case OpCode::BasicSymbology:
            {
            auto ppfb = flatbuffers::GetRoot<FB::BasicSymbology>(egOp.m_data);

            DgnSubCategoryId subCategoryId((uint64_t)ppfb->subCategoryId());

            if (!subCategoryId.IsValid())
                subCategoryId = elParams.GetSubCategoryId(); // Preserve current sub-category if not explicitly stored (GeomPart, FaceAttachment, etc.)...

            if (subCategoryId.IsValid())
                {
                DgnCategoryId categoryId = elParams.GetCategoryId(); // Preserve current category and reset to sub-category appearance...

                if (!categoryId.IsValid())
                    categoryId = DgnSubCategory::QueryCategoryId(subCategoryId, m_db);

                elParams = ElemDisplayParams();
                elParams.SetCategoryId(categoryId);
                elParams.SetSubCategoryId(subCategoryId);
                changed = true;
                }

            if (ppfb->useColor())
                {
                ColorDef lineColor(ppfb->color());

                if (elParams.IsLineColorFromSubCategoryAppearance() || lineColor != elParams.GetLineColor())
                    {
                    elParams.SetLineColor(lineColor);
                    changed = true;
                    }
                }

            if (ppfb->useWeight())
                {
                uint32_t weight = ppfb->weight();

                if (elParams.IsWeightFromSubCategoryAppearance() || weight != elParams.GetWeight())
                    {
                    elParams.SetWeight(weight);
                    changed = true;
                    }
                }

            if (ppfb->useStyle())
                {
                DgnStyleId  styleId((uint64_t)ppfb->lineStyleId());
                DgnStyleId  currStyleId = (nullptr != elParams.GetLineStyle() ? elParams.GetLineStyle()->GetStyleId() : DgnStyleId());

                if (elParams.IsLineStyleFromSubCategoryAppearance() || styleId != currStyleId)
                    {
                    if (styleId.IsValid())
                        {
                        LineStyleInfoPtr lsInfo = LineStyleInfo::Create(styleId, nullptr);
                        elParams.SetLineStyle(lsInfo.get());
                        }
                    else
                        {
                        elParams.SetLineStyle(nullptr); // Override sub-category appearance to a solid/continuous line...
                        }

                    changed = true;
                    }
                }

            double  transparency = ppfb->transparency();

            if (transparency != elParams.GetTransparency())
                {
                elParams.SetTransparency(transparency);
                changed = true;
                }

            int32_t displayPriority = ppfb->displayPriority();

            if (displayPriority != elParams.GetDisplayPriority())
                {
                elParams.SetDisplayPriority(displayPriority);
                changed = true;
                }

            DgnGeometryClass geomClass = (DgnGeometryClass) ppfb->geomClass();

            if (geomClass != elParams.GetGeometryClass())
                {
                elParams.SetGeometryClass(geomClass);
                changed = true;
                }
            break;
            }

        case OpCode::AreaFill:
            {
            auto ppfb = flatbuffers::GetRoot<FB::AreaFill>(egOp.m_data);

            FillDisplay fillDisplay = (FillDisplay) ppfb->fill();

            if (fillDisplay != elParams.GetFillDisplay())
                {
                elParams.SetFillDisplay(fillDisplay);
                changed = true;
                }

            if (FillDisplay::Never != fillDisplay)
                {
                double        transparency = ppfb->transparency();
                GradientMode  mode = (GradientMode) ppfb->mode();

                if (transparency != elParams.GetFillTransparency())
                    {
                    elParams.SetFillTransparency(transparency);
                    changed = true;
                    }

                if (GradientMode::None == mode)
                    {
                    if (ppfb->useColor())
                        {
                        ColorDef fillColor(ppfb->color());

                        if (elParams.IsFillColorFromSubCategoryAppearance() || fillColor != elParams.GetFillColor())
                            {
                            elParams.SetFillColor(fillColor);
                            changed = true;
                            }
                        }
                    else if (ppfb->isBgColor())
                        {
                        if (!elParams.IsFillColorFromViewBackground())
                            {
                            elParams.SetFillColorToViewBackground();
                            changed = true;
                            }
                        }
                    }
                else
                    {
                    GradientSymbPtr gradientPtr = GradientSymb::Create();

                    gradientPtr->SetMode(mode);
                    gradientPtr->SetFlags(ppfb->flags());
                    gradientPtr->SetShift(ppfb->shift());
                    gradientPtr->SetTint(ppfb->tint());
                    gradientPtr->SetAngle(ppfb->angle());

                    uint32_t nColors = ppfb->colors()->Length();
                    uint32_t* colors = (uint32_t*) ppfb->colors()->Data();
                    bvector<ColorDef> keyColors;

                    for (uint32_t iColor=0; iColor < nColors; iColor++)
                        keyColors.push_back(ColorDef(colors[iColor]));

                    gradientPtr->SetKeys((uint16_t) keyColors.size(), &keyColors.front(), (double*) ppfb->values()->Data());
                    elParams.SetGradient(gradientPtr.get());
                    }
                }
            break;
            }

        case OpCode::Material:
            {
            auto ppfb = flatbuffers::GetRoot<FB::Material>(egOp.m_data);

            // NEEDSWORK_WIP_MATERIAL - Set geometry specific material settings of ElemDisplayParams...
            if (ppfb->useMaterial())
                {
                DgnMaterialId material((uint64_t)ppfb->materialId());

                if (elParams.IsMaterialFromSubCategoryAppearance() || material != elParams.GetMaterial())
                    {
                    elParams.SetMaterial(material);
                    changed = true;
                    }
                }
            break;
            }

        case OpCode::LineStyleModifiers:
            {
            auto ppfb = flatbuffers::GetRoot<FB::LineStyleModifiers>(egOp.m_data);

            DgnStyleId styleId;
            LineStyleInfoCP   currentLsInfo = elParams.GetLineStyle();
            if (currentLsInfo != nullptr)
                styleId = currentLsInfo->GetStyleId();

            LineStyleParams styleParams;
            styleParams.Init();

            styleParams.modifiers = ppfb->modifiers();
            styleParams.scale = ppfb->scale();
            styleParams.dashScale = ppfb->dashScale();
            styleParams.gapScale = ppfb->gapScale();
            styleParams.startWidth = ppfb->startWidth();
            styleParams.endWidth = ppfb->endWidth();
            styleParams.distPhase = ppfb->distPhase();
            styleParams.fractPhase = ppfb->fractPhase();
            styleParams.normal = *(DPoint3d*)ppfb->normal();
            YawPitchRollAngles ypr(AngleInDegrees::FromDegrees(ppfb->yaw()), AngleInDegrees::FromDegrees(ppfb->pitch()), AngleInDegrees::FromDegrees(ppfb->roll()));
            styleParams.rMatrix = ypr.ToRotMatrix();

            LineStyleInfoPtr    lsInfo = LineStyleInfo::Create(styleId, &styleParams);
            elParams.SetLineStyle(lsInfo.get());
            changed = true;
            break;
            }

        default:
            return false;
        }

    return changed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Reader::Get(Operation const& egOp, TextStringR text) const
    {
    if (OpCode::TextString != egOp.m_opCode)
        return false;

    return (SUCCESS == TextStringPersistence::DecodeFromFlatBuf(text, egOp.m_data, egOp.m_dataSize, m_db));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Reader::Get(Operation const& egOp, ElementGeometryPtr& elemGeom) const
    {
    switch (egOp.m_opCode)
        {
        case ElementGeomIO::OpCode::PointPrimitive2d:
            {
            int         nPts;
            int8_t      boundary;
            DPoint2dCP  pts;

            if (!Get(egOp, pts, nPts, boundary))
                break;

            std::valarray<DPoint3d> localPoints3dBuf(nPts);

            for (int iPt = 0; iPt < nPts; ++iPt)
                localPoints3dBuf[iPt].Init(pts[iPt]);

            switch (boundary)
                {
                case FB::BoundaryType_None:
                    elemGeom = ElementGeometry::Create(ICurvePrimitive::CreatePointString(&localPoints3dBuf[0], nPts));
                    break;

                case FB::BoundaryType_Open:
                    elemGeom = ElementGeometry::Create(ICurvePrimitive::CreateLineString(&localPoints3dBuf[0], nPts));
                    break;

                case FB::BoundaryType_Closed:
                    elemGeom = ElementGeometry::Create(CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateLineString(&localPoints3dBuf[0], nPts)));
                    break;
                }

            return true;
            }

        case ElementGeomIO::OpCode::PointPrimitive:
            {
            int         nPts;
            int8_t      boundary;
            DPoint3dCP  pts;

            if (!Get(egOp, pts, nPts, boundary))
                break;

            switch (boundary)
                {
                case FB::BoundaryType_None:
                    elemGeom = ElementGeometry::Create(ICurvePrimitive::CreatePointString(pts, nPts));
                    break;

                case FB::BoundaryType_Open:
                    elemGeom = ElementGeometry::Create(ICurvePrimitive::CreateLineString(pts, nPts));
                    break;

                case FB::BoundaryType_Closed:
                    elemGeom = ElementGeometry::Create(CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateLineString(pts, nPts)));
                    break;
                }

            return true;
            }

        case ElementGeomIO::OpCode::ArcPrimitive:
            {
            DEllipse3d  arc;
            int8_t      boundary;

            if (!Get(egOp, arc, boundary))
                break;

            switch (boundary)
                {
                case FB::BoundaryType_None:
                case FB::BoundaryType_Open:
                    elemGeom = ElementGeometry::Create(ICurvePrimitive::CreateArc(arc));
                    break;

                case FB::BoundaryType_Closed:
                    elemGeom = ElementGeometry::Create(CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateArc(arc)));
                    break;
                }

            return true;
            }

        case ElementGeomIO::OpCode::CurvePrimitive:
            {
            ICurvePrimitivePtr curvePtr;

            if (!Get(egOp, curvePtr))
                break;

            elemGeom = ElementGeometry::Create(curvePtr);
            return true;
            }

        case ElementGeomIO::OpCode::CurveVector:
            {
            CurveVectorPtr curvePtr;

            if (!Get(egOp, curvePtr))
                break;

            elemGeom = ElementGeometry::Create(curvePtr);
            return true;
            }

        case ElementGeomIO::OpCode::Polyface:
            {
            PolyfaceQueryCarrier meshData(0, false, 0, 0, nullptr, nullptr);

            if (!Get(egOp, meshData))
                break;

            elemGeom = ElementGeometry::Create(meshData); // Copy...
            return true;
            }

        case ElementGeomIO::OpCode::SolidPrimitive:
            {
            ISolidPrimitivePtr solidPtr;

            if (!Get(egOp, solidPtr))
                break;

            elemGeom = ElementGeometry::Create(solidPtr);
            return true;
            }

        case ElementGeomIO::OpCode::BsplineSurface:
            {
            MSBsplineSurfacePtr surfacePtr;

            if (!Get(egOp, surfacePtr))
                break;

            elemGeom = ElementGeometry::Create(surfacePtr);
            return true;
            }

        case ElementGeomIO::OpCode::ParasolidBRep:
            {
            ISolidKernelEntityPtr entityPtr;

            if (!Get(egOp, entityPtr))
                break;

            elemGeom = ElementGeometry::Create(entityPtr);
            return true;
            }

        case ElementGeomIO::OpCode::BRepPolyface:
        case ElementGeomIO::OpCode::BRepPolyfaceExact:
            {
            // NOTE: Caller is expected to filter opCode when they don't want these (Parasolid BRep was available)...
            PolyfaceQueryCarrier meshData(0, false, 0, 0, nullptr, nullptr);

            if (!BentleyGeometryFlatBuffer::BytesToPolyfaceQueryCarrier(egOp.m_data, meshData))
                break;

            elemGeom = ElementGeometry::Create(meshData);
            return true;
            }

        case ElementGeomIO::OpCode::BRepEdges:
        case ElementGeomIO::OpCode::BRepFaceIso:
            {
            // NOTE: Caller is expected to filter opCode when they don't want these...
            CurveVectorPtr curvePtr = BentleyGeometryFlatBuffer::BytesToCurveVector(egOp.m_data);

            if (!curvePtr.IsValid())
                break;

            elemGeom = ElementGeometry::Create(curvePtr);
            return true;
            }

        case ElementGeomIO::OpCode::TextString:
            {
            TextStringPtr text = TextString::Create();
            if (SUCCESS != TextStringPersistence::DecodeFromFlatBuf(*text, egOp.m_data, egOp.m_dataSize, m_db))
                break;

            elemGeom = ElementGeometry::Create(text);
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Collection::GetGeomPartIds(IdSet<DgnGeomPartId>& parts, DgnDbR dgnDb) const
    {
    ElementGeomIO::Reader reader(dgnDb);

    for (auto const& egOp : *this)
        {
        if (ElementGeomIO::OpCode::GeomPartInstance != egOp.m_opCode)
            continue;

        DgnGeomPartId geomPartId;
        Transform     geomToElem;

        if (!reader.Get(egOp, geomPartId, geomToElem))
            continue;

        parts.insert(geomPartId);
        }
    }

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  02/2015
+===============+===============+===============+===============+===============+======*/
struct DrawState
{
ViewContextR        m_context;
ViewFlagsCR         m_flags;
bool                m_symbologyChanged;

DrawState(ViewContextR context, ViewFlagsCR flags) : m_context(context), m_flags(flags) {m_symbologyChanged = false;}
~DrawState() {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void InitDisplayParams(DgnCategoryId category)
    {
    ElemDisplayParamsR  dispParams = m_context.GetCurrentDisplayParams();

    dispParams = ElemDisplayParams();
    dispParams.SetCategoryId(category);

    m_symbologyChanged = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SetElemDisplayParams(DgnSubCategoryId subCategory)
    {
    ElemDisplayParamsR  dispParams = m_context.GetCurrentDisplayParams();
    DgnCategoryId       category = dispParams.GetCategoryId(); // Preserve current category...

    dispParams = ElemDisplayParams();
    dispParams.SetCategoryId(category);
    dispParams.SetSubCategoryId(subCategory);

    m_symbologyChanged = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangedElemDisplayParams()
    {
    m_symbologyChanged = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CookElemDisplayParams()
    {
    if (!m_symbologyChanged)
        return;

    // NEEDSWORK: Assumes QVElems will be cached per-view unlike Vancouver and cleared if view settings change...
    if (FillDisplay::ByView == m_context.GetCurrentDisplayParams().GetFillDisplay() && DgnRenderMode::Wireframe == m_flags.GetRenderMode() && !m_flags.fill)
        m_context.GetCurrentDisplayParams().SetFillDisplay(FillDisplay::Never);

    m_context.CookDisplayParams();
    m_symbologyChanged = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsGeometryVisible()
    {
    switch (m_context.GetCurrentDisplayParams().GetGeometryClass())
        {
        case DgnGeometryClass::Construction:
            if (!m_flags.constructions)
                return false;
            break;

        case DgnGeometryClass::Dimension:
            if (!m_flags.dimensions)
                return false;
            break;

        case DgnGeometryClass::Pattern:
            if (!m_flags.patterns)
                return false;
            break;
        }

    if (nullptr == m_context.GetViewport())
        return true;

    DgnSubCategory::Appearance appearance = m_context.GetViewport()->GetViewController().GetSubCategoryAppearance(m_context.GetCurrentDisplayParams().GetSubCategoryId());

    if (appearance.IsInvisible())
        return false;

    switch (m_context.GetDrawPurpose())
        {
        case DrawPurpose::Plot:
            if (appearance.GetDontPlot())
                return false;
            break;

        case DrawPurpose::Pick:
        case DrawPurpose::FenceAccept:
            if (appearance.GetDontLocate()) // NOTE: Don't check GetDontSnap as we still need "not snappable" hit...
                return false;
            break;
        }

    return true;
    }

}; // DrawState

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeomStreamEntryId::operator==(GeomStreamEntryIdCR rhs) const
    {
    if (this == &rhs)
        return true;

    return (m_type == rhs.m_type &&
            m_partId == rhs.m_partId &&
            m_index == rhs.m_index &&
            m_partIndex == rhs.m_partIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeomStreamEntryId::operator!=(GeomStreamEntryIdCR rhs) const
    {
    return !(*this == rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
GeomStreamEntryIdR GeomStreamEntryId::operator=(GeomStreamEntryIdCR rhs)
    {
    m_type = rhs.m_type;
    m_partId = rhs.m_partId;
    m_index = rhs.m_index;
    m_partIndex = rhs.m_partIndex;

    return *this;
    }

//=======================================================================================
//! Helper class for setting GeomStream entry identifier
//=======================================================================================
struct GeomStreamEntryIdHelper
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static bool SetActive(ViewContextR context, bool enable)
    {
    GeomStreamEntryId entryId = context.GetGeomStreamEntryId();

    if (GeomStreamEntryId::Type::Invalid != entryId.GetType() && entryId.GetGeomPartId().IsValid())
        {
        if (!enable)
            {
            entryId.SetGeomPartId(DgnGeomPartId()); // Clear part and remain active...
            context.SetGeomStreamEntryId(entryId);
            }

        return false; // Already active (or remaining active)...
        }

    entryId.Init();

    if (enable)
        entryId.SetType(GeomStreamEntryId::Type::Indexed);

    context.SetGeomStreamEntryId(entryId);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void SetActiveGeomPart(ViewContextR context, DgnGeomPartId partId)
    {
    GeomStreamEntryId entryId = context.GetGeomStreamEntryId();

    if (GeomStreamEntryId::Type::Invalid == entryId.GetType())
        return;

    entryId.SetGeomPartId(partId);
    context.SetGeomStreamEntryId(entryId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void Increment(ViewContextR context)
    {
    GeomStreamEntryId entryId = context.GetGeomStreamEntryId();

    if (GeomStreamEntryId::Type::Indexed != entryId.GetType())
        return;

    if (entryId.GetGeomPartId().IsValid())
        entryId.SetPartIndex(entryId.GetPartIndex()+1);
    else
        entryId.SetIndex(entryId.GetIndex()+1);

    context.SetGeomStreamEntryId(entryId);
    }

}; // GeomStreamEntryIdHelper

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ElementGeomIO::Import(GeomStreamR dest, GeomStreamCR source, DgnImportContext& importer)
    {
    if (!source.HasGeometry())
        return DgnDbStatus::Success; // otherwise we end up writing a header for an otherwise empty stream...

    Writer writer(importer.GetDestinationDb());
    Reader reader(importer.GetSourceDb());
    Collection collection(source.GetData(), source.GetSize());

    for (auto const& egOp : collection)
        {
        switch (egOp.m_opCode)
            {
            case ElementGeomIO::OpCode::BasicSymbology:
                {
                auto ppfb = flatbuffers::GetRoot<FB::BasicSymbology>(egOp.m_data);

                DgnSubCategoryId subCategoryId((uint64_t)ppfb->subCategoryId());
                DgnSubCategoryId remappedSubCategoryId = (subCategoryId.IsValid() ? importer.FindSubCategory(subCategoryId) : DgnSubCategoryId());
                BeAssert((subCategoryId.IsValid() == remappedSubCategoryId.IsValid()) && "Category and all subcategories should have been remapped by the element that owns this geometry");

                DgnStyleId lineStyleId((uint64_t)ppfb->lineStyleId());
                DgnStyleId remappedLineStyleId = (lineStyleId.IsValid() ? importer.RemapLineStyleId(lineStyleId) : DgnStyleId());
                BeAssert((lineStyleId.IsValid() == remappedLineStyleId.IsValid()));

                FlatBufferBuilder remappedfbb;

                auto mloc = FB::CreateBasicSymbology(remappedfbb, remappedSubCategoryId.GetValueUnchecked(),
                                                     ppfb->color(), ppfb->weight(), remappedLineStyleId.GetValueUnchecked(),
                                                     ppfb->transparency(), ppfb->displayPriority(), ppfb->geomClass(),
                                                     ppfb->useColor(), ppfb->useWeight(), ppfb->useStyle());
                remappedfbb.Finish(mloc);
                writer.Append(Operation(OpCode::BasicSymbology, (uint32_t) remappedfbb.GetSize(), remappedfbb.GetBufferPointer()));
                break;
                }

            case ElementGeomIO::OpCode::GeomPartInstance:
                {
                DgnGeomPartId geomPartId;
                Transform     geomToElem;

                if (reader.Get(egOp, geomPartId, geomToElem))
                    {
                    DgnGeomPartId remappedGeomPartId = importer.RemapGeomPartId(geomPartId); // Trigger deep-copy if necessary
                    BeAssert(remappedGeomPartId.IsValid() && "Unable to deep-copy geompart!");
                    writer.Append(remappedGeomPartId, &geomToElem);
                    }
                break;
                }

            case ElementGeomIO::OpCode::Material:
                {
                auto fbSymb = flatbuffers::GetRoot<FB::Material>(egOp.m_data);
                DgnMaterialId materialId((uint64_t)fbSymb->materialId());
                DgnMaterialId remappedMaterialId = (materialId.IsValid() ? importer.RemapMaterialId(materialId) : DgnMaterialId());
                BeAssert((materialId.IsValid() == remappedMaterialId.IsValid()) && "Unable to deep-copy material");

                FlatBufferBuilder remappedfbb;
                auto mloc = FB::CreateMaterial(remappedfbb, fbSymb->useMaterial(), remappedMaterialId.GetValueUnchecked(), fbSymb->origin(), fbSymb->size(), fbSymb->yaw(), fbSymb->pitch(), fbSymb->roll());
                remappedfbb.Finish(mloc);
                writer.Append(Operation(OpCode::Material, (uint32_t) remappedfbb.GetSize(), remappedfbb.GetBufferPointer()));
                break;
                }

            case ElementGeomIO::OpCode::ParasolidBRep:
                {
                auto ppfb = flatbuffers::GetRoot<FB::BRepData>(egOp.m_data);

                if (!ppfb->has_symbology() || !ppfb->has_symbologyIndex())
                    {
                    writer.Append(egOp);
                    break;
                    }

                bvector<FB::FaceSymbology> remappedFaceSymbVec;

                for (size_t iSymb=0; iSymb < ppfb->symbology()->Length(); iSymb++)
                    {
                    FB::FaceSymbology const* fbSymb = ((FB::FaceSymbology const*) ppfb->symbology()->Data())+iSymb;

                    if (fbSymb->useMaterial())
                        {
                        DgnMaterialId materialId((uint64_t)fbSymb->materialId());
                        DgnMaterialId remappedMaterialId = (materialId.IsValid() ? importer.RemapMaterialId(materialId) : DgnMaterialId());
                        BeAssert((materialId.IsValid() == remappedMaterialId.IsValid()) && "Unable to deep-copy material");

                        FB::FaceSymbology  remappedfbSymb(fbSymb->useColor(), fbSymb->useMaterial(),
                                                          fbSymb->color(), remappedMaterialId.GetValueUnchecked(),
                                                          fbSymb->transparency(), fbSymb->uv());

                        remappedFaceSymbVec.push_back(remappedfbSymb);
                        }
                    else
                        {
                        remappedFaceSymbVec.push_back(*fbSymb);
                        }
                    }

                FlatBufferBuilder remappedfbb;
                auto remappedEntityData = remappedfbb.CreateVector(ppfb->entityData()->Data(), ppfb->entityData()->Length());
                auto remappedFaceSymb = remappedfbb.CreateVectorOfStructs(&remappedFaceSymbVec.front(), remappedFaceSymbVec.size());
                auto remappedFaceSymbIndex = remappedfbb.CreateVectorOfStructs((FB::FaceSymbologyIndex const*) ppfb->symbologyIndex()->Data(), ppfb->symbologyIndex()->Length());
                auto mloc = FB::CreateBRepData(remappedfbb, ppfb->entityTransform(), ppfb->brepType(), remappedEntityData, remappedFaceSymb, remappedFaceSymbIndex);
                remappedfbb.Finish(mloc);
                writer.Append(Operation(OpCode::ParasolidBRep, (uint32_t) remappedfbb.GetSize(), remappedfbb.GetBufferPointer()));
                break;
                }

            case ElementGeomIO::OpCode::TextString:
                {
                TextStringPtr text = TextString::Create();
                if (SUCCESS != TextStringPersistence::DecodeFromFlatBuf(*text, egOp.m_data, egOp.m_dataSize, importer.GetSourceDb()))
                    break;

                // What is interesting is that TextString's persistence stores ID, but at runtime it only ever cares about font objects.
                // While you might think it'd be nifty to simply deserialize from the old DB and re-serialize into the new DB (thus getting a new ID based on font type/name), you'd miss out on potentially cloning over embedded face data.
                // Since the TextString came from persistence, assume the ID is valid.
                DgnFontId srcFontId = importer.GetSourceDb().Fonts().FindId(text->GetStyle().GetFont());
                DgnFontId dstFontId = importer.RemapFont(srcFontId);
                DgnFontCP dstFont = importer.GetDestinationDb().Fonts().FindFontById(dstFontId);
                
                if (nullptr == dstFont)
                    { BeDataAssert(nullptr != dstFont); }
                else
                    text->GetStyleR().SetFont(*dstFont);
                
                writer.Append(*text);
                }
            
            default:
                {
                writer.Append(egOp);
                break;
                }
            }
        }

    dest.SaveData(&writer.m_buffer.front(), (uint32_t) writer.m_buffer.size());

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Debug(IDebugOutput& output, GeomStreamCR stream, DgnDbR db, bool isPart)
    {
    Collection  collection(stream.GetData(), stream.GetSize());
    Reader      reader(db);

    IdSet<DgnGeomPartId> parts;

    for (auto const& egOp : collection)
        {
        switch (egOp.m_opCode)
            {
            case ElementGeomIO::OpCode::Header:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::Header\n").c_str());
                break;
                }

            case ElementGeomIO::OpCode::GeomPartInstance:
                {
                auto ppfb = flatbuffers::GetRoot<FB::GeomPart>(egOp.m_data);

                DgnGeomPartId       partId = DgnGeomPartId((uint64_t)ppfb->geomPartId());
                DPoint3d            origin = (nullptr == ppfb->origin() ? DPoint3d::FromZero() : *((DPoint3dCP) ppfb->origin()));
                YawPitchRollAngles  angles = YawPitchRollAngles::FromDegrees(ppfb->yaw(), ppfb->pitch(), ppfb->roll());

                if (output._WantPartGeometry())
                    parts.insert(partId);

                output._DoOutputLine(Utf8PrintfString("OpCode::GeomPartInstance - PartId: %" PRIu64 "\n", partId.GetValue()).c_str());

                if (!output._WantVerbose())
                    break;

                // Transform geomToElem = angles.ToTransform(origin);
                //
                // for (int i=0; i<3; i++)
                //     output._DoOutputLine(Utf8PrintfString("  [%lf, \t%lf, \t%lf, \t%lf]\n", geomToElem.form3d[i][0], geomToElem.form3d[i][1], geomToElem.form3d[i][2], geomToElem.form3d[i][3]).c_str());

                if (!(ppfb->has_origin() || ppfb->has_yaw() || ppfb->has_pitch() || ppfb->has_roll()))
                    break;

                output._DoOutputLine(Utf8PrintfString("  ").c_str());

                if (ppfb->has_origin())
                    output._DoOutputLine(Utf8PrintfString("Origin: [%lf, %lf, %lf] ", origin.x, origin.y, origin.z).c_str());

                if (ppfb->has_yaw())
                    output._DoOutputLine(Utf8PrintfString("Yaw: %lf ", angles.GetYaw().Degrees()).c_str());

                if (ppfb->has_pitch())
                    output._DoOutputLine(Utf8PrintfString("Pitch: %lf ", angles.GetPitch().Degrees()).c_str());

                if (ppfb->has_roll())
                    output._DoOutputLine(Utf8PrintfString("Roll: %lf ", angles.GetRoll().Degrees()).c_str());

                output._DoOutputLine(Utf8PrintfString("\n").c_str());
                break;
                }

            case ElementGeomIO::OpCode::BasicSymbology:
                {
                auto ppfb = flatbuffers::GetRoot<FB::BasicSymbology>(egOp.m_data);

                DgnSubCategoryId   subCategoryId((uint64_t)ppfb->subCategoryId());
                DgnSubCategoryCPtr subCat = (subCategoryId.IsValid() ? DgnSubCategory::QuerySubCategory(subCategoryId, db) : nullptr);

                if (subCat.IsValid() && nullptr != subCat->GetCode().GetValueCP())
                    output._DoOutputLine(Utf8PrintfString("OpCode::BasicSymbology - SubCategory: %s\n", subCat->GetCode().GetValueCP()).c_str());
                else if (subCategoryId.IsValid())
                    output._DoOutputLine(Utf8PrintfString("OpCode::BasicSymbology - SubCategoryId: %" PRIu64 "\n", subCategoryId.GetValue()).c_str());
                else
                    output._DoOutputLine(Utf8PrintfString("OpCode::BasicSymbology\n").c_str());

                if (!output._WantVerbose())
                    break;

                if (!(ppfb->has_color() || ppfb->has_useColor() || 
                      ppfb->has_weight() || ppfb->has_useWeight() || 
                      ppfb->has_lineStyleId() || ppfb->has_useStyle() || 
                      ppfb->has_transparency() || ppfb->has_displayPriority() || ppfb->has_geomClass()))
                    break;

                output._DoOutputLine(Utf8PrintfString("  ").c_str());

                if (ppfb->has_color() || ppfb->has_useColor())
                    {
                    ColorDef color(ppfb->color());
                    output._DoOutputLine(Utf8PrintfString("Color: [Red:%d Green:%d Blue:%d Alpha:%d] ", color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha()).c_str());
                    }

                if (ppfb->has_weight() || ppfb->has_useWeight())
                    output._DoOutputLine(Utf8PrintfString("Weight: %d ", ppfb->weight()).c_str());

                if (ppfb->has_lineStyleId() || ppfb->has_useStyle())
                    output._DoOutputLine(Utf8PrintfString("Style: %" PRIu64 " ", ppfb->lineStyleId()).c_str());

                if (ppfb->has_transparency())
                    output._DoOutputLine(Utf8PrintfString("Transparency: %lf ", ppfb->transparency()).c_str());

                if (ppfb->has_displayPriority())
                    output._DoOutputLine(Utf8PrintfString("Display Priority: %d ", ppfb->displayPriority()).c_str());

                if (ppfb->has_geomClass())
                    {
                    DgnGeometryClass geomClass = (DgnGeometryClass) ppfb->geomClass();
                    Utf8String       classStr;

                    switch (geomClass)
                        {
                        case DgnGeometryClass::Primary:
                            classStr.append("Primary");
                            break;
                        case DgnGeometryClass::Construction:
                            classStr.append("Construction");
                            break;
                        case DgnGeometryClass::Dimension:
                            classStr.append("Dimension");
                            break;
                        case DgnGeometryClass::Pattern:
                            classStr.append("Pattern");
                            break;
                        }

                    output._DoOutputLine(Utf8PrintfString("Geometry Class: %s ", classStr.c_str()).c_str());
                    }

                output._DoOutputLine(Utf8PrintfString("\n").c_str());
                break;
                }

            case ElementGeomIO::OpCode::PointPrimitive:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::PointPrimitive\n").c_str());

                if (!output._WantVerbose())
                    break;

                int         nPts;
                int8_t      boundary;
                DPoint3dCP  pts;
            
                if (!reader.Get(egOp, pts, nPts, boundary))
                    break;

                Utf8String  boundaryStr;

                switch (boundary)
                    {
                    case FB::BoundaryType_None:
                        boundaryStr.append("None (Points)");
                        break;
                    case FB::BoundaryType_Open:
                        boundaryStr.append("Open");
                        break;
                    case FB::BoundaryType_Closed:
                        boundaryStr.append("Closed");
                        break;
                    }

                output._DoOutputLine(Utf8PrintfString("  Point Count: %d - Boundary Type: %s\n", nPts, boundaryStr.c_str()).c_str());
                break;
                }

            case ElementGeomIO::OpCode::PointPrimitive2d:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::PointPrimitive2d\n").c_str());

                if (!output._WantVerbose())
                    break;

                int         nPts;
                int8_t      boundary;
                DPoint2dCP  pts;
            
                if (!reader.Get(egOp, pts, nPts, boundary))
                    break;

                Utf8String  boundaryStr;

                switch (boundary)
                    {
                    case FB::BoundaryType_None:
                        boundaryStr.append("None (Points)");
                        break;
                    case FB::BoundaryType_Open:
                        boundaryStr.append("Open");
                        break;
                    case FB::BoundaryType_Closed:
                        boundaryStr.append("Closed");
                        break;
                    }

                output._DoOutputLine(Utf8PrintfString("  Point Count: %d - Boundary Type: %s\n", nPts, boundaryStr.c_str()).c_str());
                break;
                }

            case ElementGeomIO::OpCode::ArcPrimitive:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::ArcPrimitive\n").c_str());

                if (!output._WantVerbose())
                    break;

                int8_t      boundary;
                DEllipse3d  arc;
            
                if (!reader.Get(egOp, arc, boundary))
                    break;

                Utf8String  boundaryStr;

                switch (boundary)
                    {
                    case FB::BoundaryType_None:
                        boundaryStr.append("None");
                        break;
                    case FB::BoundaryType_Open:
                        boundaryStr.append("Open");
                        break;
                    case FB::BoundaryType_Closed:
                        boundaryStr.append("Closed");
                        break;
                    }

                output._DoOutputLine(Utf8PrintfString("  Start: %f - Sweep: %lf - Boundary Type: %s\n", arc.start, arc.sweep, boundaryStr.c_str()).c_str());
                break;
                }

            case ElementGeomIO::OpCode::CurveVector:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::CurveVector\n").c_str());
                break;
                }

            case ElementGeomIO::OpCode::Polyface:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::Polyface\n").c_str());
                break;
                }

            case ElementGeomIO::OpCode::CurvePrimitive:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::CurvePrimitive\n").c_str());
                break;
                }

            case ElementGeomIO::OpCode::SolidPrimitive:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::SolidPrimitive\n").c_str());
                break;
                }

            case ElementGeomIO::OpCode::BsplineSurface:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::BsplineSurface\n").c_str());
                break;
                }

            case ElementGeomIO::OpCode::ParasolidBRep:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::ParasolidBRep\n").c_str());
                break;
                }

            case ElementGeomIO::OpCode::BRepPolyface:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::BRepPolyface\n").c_str());
                break;
                }

            case ElementGeomIO::OpCode::BRepPolyfaceExact:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::BRepPolyfaceExact\n").c_str());
                break;
                }

            case ElementGeomIO::OpCode::BRepEdges:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::BRepEdges\n").c_str());
                break;
                }

            case ElementGeomIO::OpCode::BRepFaceIso:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::BRepFaceIso\n").c_str());
                break;
                }

            case ElementGeomIO::OpCode::AreaFill:
                {
                auto ppfb = flatbuffers::GetRoot<FB::AreaFill>(egOp.m_data);

                FillDisplay fillDisplay = (FillDisplay) ppfb->fill();
                Utf8String  fillStr;

                switch (fillDisplay)
                    {
                    case FillDisplay::Never:
                        fillStr.append("Never");
                        break;
                    case FillDisplay::ByView:
                        fillStr.append("By View");
                        break;
                    case FillDisplay::Always:
                        fillStr.append("Always");
                        break;
                    case FillDisplay::Blanking:
                        fillStr.append("Blanking");
                        break;
                    }

                output._DoOutputLine(Utf8PrintfString("OpCode::AreaFill - Display: %s\n", fillStr.c_str()).c_str());

                if (!output._WantVerbose())
                    break;

                if (FillDisplay::Never == fillDisplay)
                    break;

                if (ppfb->has_mode())
                    {
                    output._DoOutputLine(Utf8PrintfString("  Gradient: %d ", ppfb->mode()).c_str());

                    if (ppfb->has_transparency())
                        output._DoOutputLine(Utf8PrintfString("Transparency: %lf ", ppfb->transparency()).c_str());

                    output._DoOutputLine(Utf8PrintfString("\n").c_str());
                    break;
                    }

                if (!(ppfb->has_color() || ppfb->has_useColor() || ppfb->has_isBgColor() || ppfb->has_transparency()))
                    break;

                output._DoOutputLine(Utf8PrintfString("  ").c_str());

                if (ppfb->has_color() || ppfb->has_useColor())
                    {
                    ColorDef color(ppfb->color());
                    output._DoOutputLine(Utf8PrintfString("Fill: [Red:%d Green:%d Blue:%d Alpha:%d] ", color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha()).c_str());
                    }
                else if (ppfb->has_isBgColor())
                    output._DoOutputLine(Utf8PrintfString("Fill: View Background ").c_str());
                else
                    output._DoOutputLine(Utf8PrintfString("Fill: Opaque ").c_str());

                if (ppfb->has_transparency())
                    output._DoOutputLine(Utf8PrintfString("Transparency: %lf ", ppfb->transparency()).c_str());

                output._DoOutputLine(Utf8PrintfString("\n").c_str());
                break;
                }

            case ElementGeomIO::OpCode::Pattern:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::Pattern\n").c_str());
                break;
                }

            case ElementGeomIO::OpCode::Material:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::Material\n").c_str());
                break;
                }

            case ElementGeomIO::OpCode::TextString:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::TextString\n").c_str());
                break;
                }

            case ElementGeomIO::OpCode::LineStyleModifiers:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::LineStyleModifiers\n").c_str());
                break;
                }

            default:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode - %d\n", egOp.m_opCode).c_str());
                break;
                }
            }
        }

    if (0 != parts.size())
        {
        for (DgnGeomPartId partId : parts)
            {
            output._DoOutputLine(Utf8PrintfString("\n[--- PartId: %" PRIu64 " ---]\n\n", partId.GetValue()).c_str());

            DgnGeomPartPtr partGeometry = db.GeomParts().LoadGeomPart(partId);

            if (!partGeometry.IsValid())
                continue;

            ElementGeomIO::Debug(output, partGeometry->GetGeomStream(), db, true);
            }
        }

    if (output._WantGeomEntryIds() && !isPart)
        {
        ElementGeometryCollection collection(db, stream);

        output._DoOutputLine(Utf8PrintfString("\n--- GeomStream Entry Ids ---\n\n"));

        for (ElementGeometryPtr geom : collection)
            {
            GeomStreamEntryId geomId = collection.GetGeomStreamEntryId();
            Utf8String        geomType;

            switch (geom->GetGeometryType())
                {
                case ElementGeometry::GeometryType::CurvePrimitive:
                    geomType.assign("CurvePrimitive");
                    break;

                case ElementGeometry::GeometryType::CurveVector:
                    geomType.assign("CurveVector");
                    break;

                case ElementGeometry::GeometryType::SolidPrimitive:
                    geomType.assign("SolidPrimitive");
                    break;

                case ElementGeometry::GeometryType::BsplineSurface:
                    geomType.assign("BsplineSurface");
                    break;

                case ElementGeometry::GeometryType::Polyface:
                    geomType.assign("Polyface");
                    break;

                case ElementGeometry::GeometryType::SolidKernelEntity:
                    geomType.assign("SolidKernelEntity");
                    break;

                case ElementGeometry::GeometryType::TextString:
                    geomType.assign("TextString");
                    break;

                default:
                    geomType.assign("Unknown");
                    break;
                }

            if (!geomId.GetGeomPartId().IsValid())
                output._DoOutputLine(Utf8PrintfString("- GeometryType::%s \t[Index: %d]\n", geomType.c_str(), geomId.GetIndex()).c_str());
            else
                output._DoOutputLine(Utf8PrintfString("- GeometryType::%s \t[Index: %d | PartId: %" PRIu64 " Part Index: %d]\n", geomType.c_str(), geomId.GetIndex(), geomId.GetGeomPartId().GetValue(), geomId.GetPartIndex()).c_str());
            }

        output._DoOutputLine(Utf8PrintfString("\n"));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Collection::Draw(ViewContextR context, DgnCategoryId category, ViewFlagsCR flags) const
    {
    // NEEDSWORK: Assumes QVElems will be cached per-view unlike Vancouver and cleared if view settings change...
    bool        isQVis = context.GetIViewDraw().IsOutputQuickVision() || context.CheckICachedDraw();
    bool        isQVWireframe = (isQVis && DgnRenderMode::Wireframe == flags.GetRenderMode());
    bool        isPick = (nullptr != context.GetIPickGeom());
    bool        useBRep = !(isQVis || isPick);
    DrawState   state(context, flags);

    ElementGeomIO::Reader reader(context.GetDgnDb());

    for (auto const& egOp : *this)
        {
        switch (egOp.m_opCode)
            {
            case ElementGeomIO::OpCode::Header:
                {
                // Current display params is already setup when displaying a geom part...DON'T INITIALIZE!!!
                if (!GeomStreamEntryIdHelper::SetActive(context, true))
                    break;

                Header const*hdr = Reader::GetHeader(egOp);
                if (!hdr->UseCurrentDisplayParams())
                    state.InitDisplayParams(category);
                break;
                }

            case ElementGeomIO::OpCode::BasicSymbology:
            case ElementGeomIO::OpCode::LineStyleModifiers:
            case ElementGeomIO::OpCode::AreaFill:
            case ElementGeomIO::OpCode::Pattern:
            case ElementGeomIO::OpCode::Material:
                {
                if (!reader.Get(egOp, context.GetCurrentDisplayParams()))
                    break;

                state.ChangedElemDisplayParams();
                break;
                }

            case ElementGeomIO::OpCode::GeomPartInstance:
                {
                GeomStreamEntryIdHelper::Increment(context);

                DgnGeomPartId geomPartId;
                Transform     geomToElem;

                if (!reader.Get(egOp, geomPartId, geomToElem))
                    break;

                DgnGeomPartPtr partGeometry = context.GetDgnDb().GeomParts().LoadGeomPart(geomPartId);

                if (!partGeometry.IsValid())
                    break;

                ElementGeomIO::Collection collection(partGeometry->GetGeomStream().GetData(), partGeometry->GetGeomStream().GetSize());

                GeomStreamEntryIdHelper::SetActiveGeomPart(context, geomPartId);
                state.CookElemDisplayParams();

                context.PushTransform(geomToElem);
                collection.Draw(context, category, flags);
                context.PopTransformClip();
                break;
                }

            case ElementGeomIO::OpCode::PointPrimitive2d:
                {
                GeomStreamEntryIdHelper::Increment(context);

                if (!state.IsGeometryVisible())
                    break;

                int         nPts;
                int8_t      boundary;
                DPoint2dCP  pts;

                if (!reader.Get(egOp, pts, nPts, boundary))
                    break;

                state.CookElemDisplayParams();

                switch (boundary)
                    {
                    case FB::BoundaryType_None:
                        context.GetIDrawGeom().DrawPointString2d(nPts, pts, context.GetCurrentDisplayParams().GetNetDisplayPriority(), nullptr);
                        break;

                    case FB::BoundaryType_Open:
                        context.GetIDrawGeom().DrawLineString2d(nPts, pts, context.GetCurrentDisplayParams().GetNetDisplayPriority(), nullptr);
                        break;

                    case FB::BoundaryType_Closed:
                        context.GetIDrawGeom().DrawShape2d(nPts, pts, FillDisplay::Never != context.GetCurrentDisplayParams().GetFillDisplay(), context.GetCurrentDisplayParams().GetNetDisplayPriority(), nullptr);
                        break;
                    }
                break;
                }

            case ElementGeomIO::OpCode::PointPrimitive:
                {
                GeomStreamEntryIdHelper::Increment(context);

                if (!state.IsGeometryVisible())
                    break;

                int         nPts;
                int8_t      boundary;
                DPoint3dCP  pts;

                if (!reader.Get(egOp, pts, nPts, boundary))
                    break;

                state.CookElemDisplayParams();

                switch (boundary)
                    {
                    case FB::BoundaryType_None:
                        context.GetIDrawGeom().DrawPointString3d(nPts, pts, nullptr);
                        break;

                    case FB::BoundaryType_Open:
                        context.GetIDrawGeom().DrawLineString3d(nPts, pts, nullptr);

                        break;

                    case FB::BoundaryType_Closed:
                        context.GetIDrawGeom().DrawShape3d(nPts, pts, FillDisplay::Never != context.GetCurrentDisplayParams().GetFillDisplay(), nullptr);
                        break;
                    }
                break;
                }

            case ElementGeomIO::OpCode::ArcPrimitive:
                {
                GeomStreamEntryIdHelper::Increment(context);

                if (!state.IsGeometryVisible())
                    break;

                DEllipse3d  arc;
                int8_t      boundary;

                if (!reader.Get(egOp, arc, boundary))
                    break;

                state.CookElemDisplayParams();

                if (!context.Is3dView())
                    {
                    if (FB::BoundaryType_Closed != boundary)
                        context.GetIDrawGeom().DrawArc2d(arc, false, false, context.GetCurrentDisplayParams().GetNetDisplayPriority(), nullptr);
                    else
                        context.GetIDrawGeom().DrawArc2d(arc, true, FillDisplay::Never != context.GetCurrentDisplayParams().GetFillDisplay(), context.GetCurrentDisplayParams().GetNetDisplayPriority(), nullptr);
                    break;
                    }

                if (FB::BoundaryType_Closed != boundary)
                    context.GetIDrawGeom().DrawArc3d(arc, false, false, nullptr);
                else
                    context.GetIDrawGeom().DrawArc3d(arc, true, FillDisplay::Never != context.GetCurrentDisplayParams().GetFillDisplay(), nullptr);
                break;
                }

            case ElementGeomIO::OpCode::CurvePrimitive:
                {
                GeomStreamEntryIdHelper::Increment(context);

                if (!state.IsGeometryVisible())
                    break;

                ICurvePrimitivePtr curvePrimitivePtr;

                if (!reader.Get(egOp, curvePrimitivePtr))
                    break;

                state.CookElemDisplayParams();

                // A single curve primitive is always open (or none for a point string)...
                CurveVectorPtr  curvePtr = CurveVector::Create(ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString == curvePrimitivePtr->GetCurvePrimitiveType() ? CurveVector::BOUNDARY_TYPE_None : CurveVector::BOUNDARY_TYPE_Open, curvePrimitivePtr);

                if (!context.Is3dView())
                    {
                    context.GetIDrawGeom().DrawCurveVector2d(*curvePtr, false, context.GetCurrentDisplayParams().GetNetDisplayPriority());
                    break;
                    }

                context.GetIDrawGeom().DrawCurveVector(*curvePtr, false);
                break;
                }

            case ElementGeomIO::OpCode::CurveVector:
                {
                GeomStreamEntryIdHelper::Increment(context);

                if (!state.IsGeometryVisible())
                    break;

                CurveVectorPtr curvePtr;

                if (!reader.Get(egOp, curvePtr))
                    break;

                state.CookElemDisplayParams();

                if (!context.Is3dView())
                    {
                    context.GetIDrawGeom().DrawCurveVector2d(*curvePtr, curvePtr->IsAnyRegionType() && FillDisplay::Never != context.GetCurrentDisplayParams().GetFillDisplay(), context.GetCurrentDisplayParams().GetNetDisplayPriority());
                    break;
                    }

                context.GetIDrawGeom().DrawCurveVector(*curvePtr, curvePtr->IsAnyRegionType() && FillDisplay::Never != context.GetCurrentDisplayParams().GetFillDisplay());
                break;
                }

            case ElementGeomIO::OpCode::Polyface:
                {
                GeomStreamEntryIdHelper::Increment(context);

                if (!state.IsGeometryVisible())
                    break;

                PolyfaceQueryCarrier meshData(0, false, 0, 0, nullptr, nullptr);

                if (!reader.Get(egOp, meshData))
                    break;

                state.CookElemDisplayParams();
                context.GetIDrawGeom().DrawPolyface(meshData, FillDisplay::Never != context.GetCurrentDisplayParams().GetFillDisplay());
                break;
                };

            case ElementGeomIO::OpCode::SolidPrimitive:
                {
                GeomStreamEntryIdHelper::Increment(context);

                if (!state.IsGeometryVisible())
                    break;

                ISolidPrimitivePtr solidPtr;

                if (!reader.Get(egOp, solidPtr))
                    break;

                state.CookElemDisplayParams();
                context.GetIDrawGeom().DrawSolidPrimitive(*solidPtr);
                break;
                }

            case ElementGeomIO::OpCode::BsplineSurface:
                {
                GeomStreamEntryIdHelper::Increment(context);

                if (!state.IsGeometryVisible())
                    break;

                MSBsplineSurfacePtr surfacePtr;

                if (!reader.Get(egOp, surfacePtr))
                    break;

                state.CookElemDisplayParams();
                context.GetIDrawGeom().DrawBSplineSurface(*surfacePtr);
                break;
                }

            case ElementGeomIO::OpCode::ParasolidBRep:
                {
                GeomStreamEntryIdHelper::Increment(context);

                if (!useBRep)
                    break;

                if (!state.IsGeometryVisible())
                    break;

                ISolidKernelEntityPtr entityPtr;

                if (!reader.Get(egOp, entityPtr))
                    {
                    useBRep = false;
                    break;
                    }

                state.CookElemDisplayParams();
                context.GetIDrawGeom().DrawBody(*entityPtr);
                break;
                }

            case ElementGeomIO::OpCode::BRepPolyface:
            case ElementGeomIO::OpCode::BRepPolyfaceExact:
                {
                // Don't increment GeomStreamEntryId...
                if (useBRep)
                    break;

                if (!state.IsGeometryVisible())
                    break;

                PolyfaceQueryCarrier meshData(0, false, 0, 0, nullptr, nullptr);

                if (!BentleyGeometryFlatBuffer::BytesToPolyfaceQueryCarrier(egOp.m_data, meshData))
                    break;

                state.CookElemDisplayParams();

                // NOTE: For this case the exact edge geometry will be supplied by BRepEdges/BRepFaceIso, inhibit facetted edge draw...
                if ((isPick || isQVWireframe) && ElementGeomIO::OpCode::BRepPolyface == egOp.m_opCode)
                    {
                    // NOTE: Don't like doing this...but I can't think of another way.
                    //       We only want to use the mesh for silhouettes and surface locates.
                    //       It's not good for wireframe display or snapping.
                    //       NEEDSWORK_QVIS: Fix mesh silhouette display, erroneously draws non-silhouette edges!!!
                    bvector<int32_t> pointIndex;

                    pointIndex.reserve(meshData.GetPointIndexCount());

                    for (size_t i=0; i<meshData.GetPointIndexCount(); i++)
                        {
                        int32_t thisIndex = *(meshData.GetPointIndexCP()+i);

                        pointIndex.push_back(thisIndex < 0 ? thisIndex : -thisIndex);
                        }

                    PolyfaceQueryCarrier sourceData(meshData.GetNumPerFace(), meshData.GetTwoSided(), meshData.GetPointIndexCount(),
                                                                 meshData.GetPointCount(), meshData.GetPointCP(), &pointIndex.front(),
                                                                 meshData.GetNormalCount(), meshData.GetNormalCP(), meshData.GetNormalIndexCP(),
                                                                 meshData.GetParamCount(), meshData.GetParamCP(), meshData.GetParamIndexCP(),
                                                                 meshData.GetColorCount(), meshData.GetColorIndexCP(),
                                                                 (FloatRgb const*) meshData.GetFloatColorCP(), (RgbFactor const*) meshData.GetDoubleColorCP(), meshData.GetIntColorCP(), meshData.GetColorTableCP(),
                                                                 meshData.GetIlluminationNameCP(), meshData.GetMeshStyle(), meshData.GetNumPerRow());

                    context.GetIDrawGeom().DrawPolyface(sourceData);
                    break;
                    }

                context.GetIDrawGeom().DrawPolyface(meshData);
                break;
                }

            case ElementGeomIO::OpCode::BRepEdges:
                {
                // Don't increment GeomStreamEntryId...
                if (!(isPick || isQVWireframe)) // NEEDSWORK: Assumes QVElems are per-view...otherwise must setup WF only matsymb like Vancouver...
                    break;

                if (!state.IsGeometryVisible())
                    break;

                CurveVectorPtr curvePtr = BentleyGeometryFlatBuffer::BytesToCurveVector(egOp.m_data);

                if (!curvePtr.IsValid())
                    break;

                state.CookElemDisplayParams();
                context.GetIDrawGeom().DrawCurveVector(*curvePtr, false);
                break;
                }

            case ElementGeomIO::OpCode::BRepFaceIso:
                {
                // Don't increment GeomStreamEntryId...
                if (!(isPick || isQVWireframe)) // NEEDSWORK: Assumes QVElems are per-view...otherwise must setup WF only matsymb like Vancouver...
                    break;

                if (!state.IsGeometryVisible())
                    break;

                CurveVectorPtr curvePtr = BentleyGeometryFlatBuffer::BytesToCurveVector(egOp.m_data);

                if (!curvePtr.IsValid())
                    break;

                state.CookElemDisplayParams();
                context.GetIDrawGeom().DrawCurveVector(*curvePtr, false);
                break;
                }

            case ElementGeomIO::OpCode::TextString:
                {
                GeomStreamEntryIdHelper::Increment(context);

                if (!state.IsGeometryVisible())
                    break;

                TextString  text;

                if (SUCCESS != TextStringPersistence::DecodeFromFlatBuf(text, egOp.m_data, egOp.m_dataSize, context.GetDgnDb()))
                    break;

                state.CookElemDisplayParams();
                context.DrawTextString(text);
                break;
                }

            default:
                break;
            }

        if (context.CheckStop())
            break;
        }

    GeomStreamEntryIdHelper::SetActive(context, false);
    }

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  04/2015
+===============+===============+===============+===============+===============+======*/
struct DrawGeomStream : StrokeElementForCache
{
ViewFlags       m_flags;

explicit DrawGeomStream(DgnElementCR element, ViewFlagsCR flags) : StrokeElementForCache(element)
    {
    m_flags = flags; // NEEDSWORK: Assumes QVElems will be cached per-view unlike Vancouver and cleared if view settings change... 
    }

virtual int32_t _GetQvIndex() const override
    {
    // NEEDSWORK: Won't need this when QVElems are per-view...will just have QVElem per sub-category/transform...
    return (DgnRenderMode::Wireframe != m_flags.GetRenderMode() ? 1 : (m_flags.fill ? 2 : 3));
    }

virtual void _StrokeForCache(ViewContextR context, double pixelSize) override
    {
    GeometrySourceCP source = m_element.ToGeometrySource();

    if (nullptr == source)
        return;

    ElementGeomIO::Collection(source->GetGeomStream().GetData(), source->GetGeomStream().GetSize()).Draw(context, source->GetCategoryId(), m_flags);
    }

}; // DrawGeomStream

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometrySource::_Draw(ViewContextR context) const
    {
    DgnElementCP el = ToElement();

    if (nullptr == el)
        {
        // NEEDSWORK: Temporary - Continous rendering DgnGraphic will allow this to be cached...
        Transform   placementTrans = GetPlacementTransform();
        ViewFlags   viewFlags;

        if (nullptr != context.GetViewFlags())
            viewFlags = *context.GetViewFlags();
        else
            viewFlags.InitDefaults();

        context.PushTransform(placementTrans);
        ElementGeomIO::Collection(GetGeomStream().GetData(), GetGeomStream().GetSize()).Draw(context, GetCategoryId(), viewFlags);
        context.PopTransformClip();
        return;
        }

    // NEEDSWORK: Assumes QVElems will be cached per-view unlike Vancouver...
    ViewFlags   viewFlags;

    if (nullptr != context.GetViewFlags())
        viewFlags = *context.GetViewFlags();
    else
        viewFlags.InitDefaults();

    // NEEDSWORK: Want separate QvElems per-subCategory...
    Transform      placementTrans = GetPlacementTransform();
    DrawGeomStream stroker(*el, viewFlags);

    context.PushTransform(placementTrans);
    context.DrawCached(stroker);
    context.PopTransformClip();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometrySource::_DrawHit(HitDetailCR hit, ViewContextR context) const
    {
    if (DrawPurpose::Flash != context.GetDrawPurpose())
        return false;

    if (GeomStreamEntryId::Type::Invalid == hit.GetGeomDetail().GetGeomStreamEntryId().GetType())
        return false;

    switch (hit.GetSubSelectionMode())
        {
        case SubSelectionMode::Part:
            {
            if (hit.GetGeomDetail().GetGeomStreamEntryId().GetGeomPartId().IsValid())
                break;

            return false;
            }

        case SubSelectionMode::Primitive:
            break;

        case SubSelectionMode::Segment:
            {
            if (nullptr != hit.GetGeomDetail().GetCurvePrimitive())
                break;

            return false;
            }

        default:
            return false;
        }

    context.SetCurrentElement(this);

    // Get the ElemDisplayParams for this hit from the GeomStream...
    ElementGeometryCollection collection(*this);

    collection.SetBRepOutput(ElementGeometryCollection::BRepOutput::Mesh);

    for (ElementGeometryPtr geom : collection)
        {
        switch (hit.GetSubSelectionMode())
            {
            case SubSelectionMode::Part:
                {
                if (hit.GetGeomDetail().GetGeomStreamEntryId().GetGeomPartId() == collection.GetGeomStreamEntryId().GetGeomPartId())
                    break;

                continue;
                }

            case SubSelectionMode::Primitive:
            case SubSelectionMode::Segment:
                {
                if (hit.GetGeomDetail().GetGeomStreamEntryId() == collection.GetGeomStreamEntryId())
                    break;

                continue;
                }
            }

        context.GetCurrentDisplayParams() = collection.GetElemDisplayParams();

        if (SubSelectionMode::Segment != hit.GetSubSelectionMode())
            {
            context.CookDisplayParams();
            context.ResetContextOverrides();

            context.PushTransform(collection.GetGeometryToWorld());
            geom->Draw(context);
            context.PopTransformClip();

            continue; // Keep going, want to draw all matching geometry...
            }

        hit.FlashCurveSegment(context);
        break;
        }

    context.SetCurrentElement(nullptr);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
SnapStatus GeometrySource::_OnSnap(SnapContextR context) const
    {
    return context.DoDefaultDisplayableSnap();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeometryCollection::Iterator::ToNext()
    {
    do
        {
        if (m_dataOffset >= m_totalDataSize)
            {
            GeomStreamEntryIdHelper::SetActive(*m_context, false);

            if (!m_partGeometry.IsValid())
                {
                m_data = nullptr;
                m_dataOffset = 0;
                return;
                }

            m_data = m_saveData;
            m_dataOffset = m_saveDataOffset;
            m_totalDataSize = m_saveTotalDataSize;;

            m_partGeometry = nullptr;
            m_context->PopTransformClip(); // Pop part transform...

            if (m_dataOffset >= m_totalDataSize)
                {
                m_data = nullptr;
                m_dataOffset = 0;
                return;
                }
            }

        uint32_t        opCode = *((uint32_t *) (m_data));
        uint32_t        dataSize = *((uint32_t *) (m_data + sizeof (opCode)));
        uint8_t const*  data = (0 != dataSize ? (uint8_t const*) (m_data + sizeof (opCode) + sizeof (dataSize)) : nullptr);
        size_t          egOpSize = sizeof (opCode) + sizeof (dataSize) + dataSize;

        ElementGeomIO::Operation egOp = ElementGeomIO::Operation((ElementGeomIO::OpCode) (opCode), dataSize, data);
        ElementGeomIO::Reader reader(m_context->GetDgnDb());

        m_data += egOpSize;
        m_dataOffset += egOpSize;

        switch (egOp.m_opCode)
            {
            case ElementGeomIO::OpCode::Header:
                {
                GeomStreamEntryIdHelper::SetActive(*m_context, true);
                break;
                }

            case ElementGeomIO::OpCode::BasicSymbology:
            case ElementGeomIO::OpCode::LineStyleModifiers:
            case ElementGeomIO::OpCode::AreaFill:
            case ElementGeomIO::OpCode::Pattern:
            case ElementGeomIO::OpCode::Material:
                {
                reader.Get(egOp, m_context->GetCurrentDisplayParams()); // Update active ElemDisplayParams...
                break;
                }

            case ElementGeomIO::OpCode::GeomPartInstance:
                {
                GeomStreamEntryIdHelper::Increment(*m_context);

                DgnGeomPartId geomPartId;
                Transform     geomToElem;

                if (!reader.Get(egOp, geomPartId, geomToElem))
                    break;

                m_partGeometry = m_context->GetDgnDb().GeomParts().LoadGeomPart(geomPartId);

                if (!m_partGeometry.IsValid())
                    break;

                // Push part transform...
                m_context->PushTransform(geomToElem);

                // Save current data position...
                m_saveData = m_data;
                m_saveDataOffset = m_dataOffset;
                m_saveTotalDataSize = m_totalDataSize;

                // Switch to iterate over part geometry...
                m_data = m_partGeometry->GetGeomStream().GetData();
                m_dataOffset = 0;
                m_totalDataSize = m_partGeometry->GetGeomStream().GetSize();

                GeomStreamEntryIdHelper::SetActiveGeomPart(*m_context, geomPartId);
                break;
                }

            default:
                {
                bool    doOutput = false, doIncrement = false;

                switch (egOp.m_opCode)
                    {
                    case ElementGeomIO::OpCode::ParasolidBRep:
                        {
                        doOutput = (BRepOutput::None != (BRepOutput::BRep & m_bRepOutput));
                        doIncrement = true;
                        break;
                        }

                    case ElementGeomIO::OpCode::BRepEdges:
                        {
                        doOutput = (BRepOutput::None != (BRepOutput::Edges & m_bRepOutput));
                        break;
                        }

                    case ElementGeomIO::OpCode::BRepFaceIso:
                        {
                        doOutput = (BRepOutput::None != (BRepOutput::FaceIso & m_bRepOutput));
                        break;
                        }

                    case ElementGeomIO::OpCode::BRepPolyface:
                    case ElementGeomIO::OpCode::BRepPolyfaceExact:
                        {
                        doOutput = (BRepOutput::None != (BRepOutput::Mesh & m_bRepOutput));
                        break;
                        }

                    default:
                        {
                        doOutput = doIncrement = egOp.IsGeometryOp();
                        break;
                        }
                    }

                if (doIncrement)
                    GeomStreamEntryIdHelper::Increment(*m_context);

                if (!doOutput)
                    break;

                if (!reader.Get(egOp, m_elementGeometry))
                    {
                    if (ElementGeomIO::OpCode::ParasolidBRep == egOp.m_opCode)
                        m_bRepOutput = ((m_bRepOutput & ~BRepOutput::BRep) | BRepOutput::Mesh); // BRep unavailable - start returning BRepPolyface geometry...
                    break;
                    }

                if (m_context->GetCurrentDisplayParams().GetCategoryId().IsValid())
                    m_context->GetCurrentDisplayParams().Resolve(*m_context); // Resolve sub-category appearance...
                return;
                }
            }

        } while (true);
    }

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  04/2015
+===============+===============+===============+===============+===============+======*/
struct ElementGeometryCollectionContext : NullContext
{
    DEFINE_T_SUPER(NullContext)
protected:

SimplifyViewDrawGeom* m_output;

virtual void _SetupOutputs() override {SetIViewDraw(*m_output);}

public:

virtual ~ElementGeometryCollectionContext() {delete (m_output);}

ElementGeometryCollectionContext(DgnDbR db)
    {
    m_output = new SimplifyViewDrawGeom();
    m_purpose = DrawPurpose::CaptureGeometry;
    m_wantMaterials = true; // Setup material in ElemDisplayParams...

    SetBlockAsynchs(true);
    m_output->SetViewContext(this);
    _SetupOutputs();

    m_currDisplayParams = ElemDisplayParams();
    SetDgnDb(db);
    }

}; // ElementGeometryCollectionContext

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ElemDisplayParamsCR ElementGeometryCollection::GetElemDisplayParams()
    {
    return m_context->GetCurrentDisplayParams();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
GeomStreamEntryId ElementGeometryCollection::GetGeomStreamEntryId()
    {
    return m_context->GetGeomStreamEntryId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TransformCR ElementGeometryCollection::GetElementToWorld()
    {
    return m_elemToWorld;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TransformCR ElementGeometryCollection::GetGeometryToWorld()
    {
    TransformCP currentTrans = m_context->GetCurrLocalToWorldTransformCP();

    if (nullptr != currentTrans)
        m_geomToWorld = *currentTrans;
    else
        m_geomToWorld.InitIdentity();

    return m_geomToWorld;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TransformCR ElementGeometryCollection::GetGeometryToElement()
    {
    Transform   worldToElem;

    worldToElem.InverseOf(m_elemToWorld);
    m_geomToElem = Transform::FromProduct(worldToElem, GetGeometryToWorld());

    return m_geomToElem;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryCollection::ElementGeometryCollection(DgnDbR dgnDb, GeomStreamCR geom)
    {
    m_bRepOutput = BRepOutput::BRep;
    m_data = geom.GetData();
    m_dataSize = geom.GetSize();
    m_elemToWorld.InitIdentity();
    m_context = new ElementGeometryCollectionContext(dgnDb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryCollection::ElementGeometryCollection(GeometrySourceCR source)
    {
    m_bRepOutput = BRepOutput::BRep;
    m_data = source.GetGeomStream().GetData();
    m_dataSize = source.GetGeomStream().GetSize();
    m_elemToWorld = source.GetPlacementTransform();
    m_context = new ElementGeometryCollectionContext(source.GetSourceDgnDb());
    m_context->GetCurrentDisplayParams().SetCategoryId(source.GetCategoryId());
    m_context->PushTransform(m_elemToWorld);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryCollection::~ElementGeometryCollection()
    {
    delete((ElementGeometryCollectionContext*) m_context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static bool is3dGeometryType(ElementGeometry::GeometryType geomType)
    {
    switch (geomType)
        {
        case ElementGeometry::GeometryType::SolidPrimitive:
        case ElementGeometry::GeometryType::BsplineSurface:
        case ElementGeometry::GeometryType::Polyface:
        case ElementGeometry::GeometryType::SolidKernelEntity:
            return true;

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
GeomStreamEntryId ElementGeometryBuilder::GetGeomStreamEntryId() const
    {
    GeomStreamEntryId entryId;

    ElementGeomIO::Collection collection(&m_writer.m_buffer.front(), (uint32_t) m_writer.m_buffer.size());

    for (auto const& egOp : collection)
        {
        switch (egOp.m_opCode)
            {
            case ElementGeomIO::OpCode::GeomPartInstance:
                {
                auto ppfb = flatbuffers::GetRoot<FB::GeomPart>(egOp.m_data);

                entryId.SetType(GeomStreamEntryId::Type::Indexed);
                entryId.SetGeomPartId(DgnGeomPartId((uint64_t)ppfb->geomPartId()));
                entryId.SetIndex(entryId.GetIndex()+1);
                break;
                }

            case ElementGeomIO::OpCode::BRepPolyface:
            case ElementGeomIO::OpCode::BRepPolyfaceExact:
            case ElementGeomIO::OpCode::BRepEdges:
            case ElementGeomIO::OpCode::BRepFaceIso:
                break; // These are considered part of OpCode::ParasolidBRep for purposes of GeomStreamEntryId...

            default:
                {
                if (!egOp.IsGeometryOp())
                    break;

                entryId.SetType(GeomStreamEntryId::Type::Indexed);
                entryId.SetGeomPartId(DgnGeomPartId());
                entryId.SetIndex(entryId.GetIndex()+1);
                break;
                }
            }
        }

    return entryId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ElementGeometryBuilder::GetGeomStream(GeomStreamR geom)
    {
    if (0 == m_writer.m_buffer.size())
        return ERROR;

    geom.SaveData(&m_writer.m_buffer.front(), (uint32_t) m_writer.m_buffer.size());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ElementGeometryBuilder::SetGeomStream(DgnGeomPartR part)
    {
    if (!m_isPartCreate)
        return ERROR; // Invalid builder for creating part geometry...

    if (0 == m_writer.m_buffer.size())
        return ERROR;

    part.GetGeomStreamR().SaveData(&m_writer.m_buffer.front(), (uint32_t) m_writer.m_buffer.size());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ElementGeometryBuilder::SetGeomStreamAndPlacement(GeometrySourceR source)
    {
    if (m_isPartCreate)
        return ERROR; // Invalid builder for creating element geometry...

    if (0 == m_writer.m_buffer.size())
        return ERROR;

    if (!m_havePlacement)
        return ERROR;

    if (source.GetCategoryId() != m_elParams.GetCategoryId())
        return ERROR;

    DgnElementCP el = source.ToElement();

    if (nullptr != el && el->GetElementHandler()._IsRestrictedAction(DgnElement::RestrictedAction::SetGeometry))
        return ERROR;

    if (m_is3d)
        {
        if (!m_placement3d.IsValid())
            return ERROR;

        GeometrySource3dP source3d;

        if (nullptr == (source3d = source.ToGeometrySource3dP()))
            return ERROR;

        source3d->SetPlacement(m_placement3d);
        }
    else
        {
        if (!m_placement2d.IsValid())
            return ERROR;

        GeometrySource2dP source2d;

        if (nullptr == (source2d = source.ToGeometrySource2dP()))
            return ERROR;

        source2d->SetPlacement(m_placement2d);
        }

    source.GetGeomStreamR().SaveData(&m_writer.m_buffer.front(), (uint32_t) m_writer.m_buffer.size());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometryBuilder::Append(DgnSubCategoryId subCategoryId)
    {
    ElemDisplayParams elParams;

    elParams.SetCategoryId(m_elParams.GetCategoryId()); // Preserve current category...
    elParams.SetSubCategoryId(subCategoryId);

    return Append(elParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometryBuilder::Append(ElemDisplayParamsCR elParams)
    {
    if (!m_elParams.GetCategoryId().IsValid())
        return false;

    if (elParams.GetCategoryId() != m_elParams.GetCategoryId())
        return false;

    if (elParams.GetCategoryId() != DgnSubCategory::QueryCategoryId(elParams.GetSubCategoryId(), m_dgnDb))
        return false;

    if (m_elParams == elParams)
        return true;

    m_elParams = elParams;
    m_appearanceChanged = true; // Defer append until we actually have some geometry...

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometryBuilder::Append(DgnGeomPartId geomPartId, TransformCR geomToElement)
    {
    if (m_isPartCreate)
        {
        BeAssert(false); // Nested parts are not supported...
        return false;
        }

    if (!m_havePlacement)
        return false; // geomToElement must be relative to an already defined placement (i.e. not computed placement from CreateWorld)...

    DgnGeomPartPtr geomPart = m_dgnDb.GeomParts().LoadGeomPart(geomPartId);

    if (!geomPart.IsValid())
        return false;

    DRange3d localRange = DRange3d::NullRange();
    ElementGeometryCollection collection(m_dgnDb, geomPart->GetGeomStream());

    collection.SetBRepOutput(ElementGeometryCollection::BRepOutput::Mesh); // Can just use the mesh and avoid creating the ISolidKernelEntity...

    for (ElementGeometryPtr geom : collection)
        {
        if (!geom.IsValid())
            continue;

        if (!m_is3d && is3dGeometryType(geom->GetGeometryType()))
            {
            BeAssert(false); // 3d only geometry...
            return false;
            }

        DRange3d range;

        if (!geom->GetRange(range))
            continue;

        localRange.Extend(range);
        }

    if (!geomToElement.IsIdentity())
        geomToElement.Multiply(localRange, localRange);

    OnNewGeom(localRange);
    m_writer.Append(geomPartId, &geomToElement);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeometryBuilder::OnNewGeom(DRange3dCR localRange)
    {
    if (m_isPartCreate)
        {
        // NOTE: Don't need placement or want sub-category to be added, but we do want to
        //       store symbology/material attachments that aren't from the sub-category appearance.
        if (m_appearanceChanged)
            {
            m_writer.Append(m_elParams, m_isPartCreate);
            m_appearanceChanged = false;
            }

        return;
        }

    if (m_is3d)
        m_placement3d.GetElementBoxR().Extend(localRange);
    else
        m_placement2d.GetElementBoxR().Extend(DRange2d::From(DPoint2d::From(localRange.low), DPoint2d::From(localRange.high)));

    if (m_appearanceChanged)
        {
        m_writer.Append(m_elParams, m_isPartCreate);
        m_appearanceChanged = false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometryBuilder::ConvertToLocal(ElementGeometryR geom)
    {
    Transform   localToWorld;

    if (!m_havePlacement)
        {
        if (!geom.GetLocalCoordinateFrame(localToWorld))
            return false;

        DPoint3d            origin;
        RotMatrix           rMatrix;
        YawPitchRollAngles  angles;

        localToWorld.GetTranslation(origin);
        localToWorld.GetMatrix(rMatrix);
        YawPitchRollAngles::TryFromRotMatrix(angles, rMatrix);

        if (m_is3d)
            {
            m_placement3d.GetOriginR() = origin;
            m_placement3d.GetAnglesR() = angles;
            }
        else
            {
            BeAssert(0 == BeNumerical::Compare(origin.z, 0.0));
            if (0.0 != angles.GetPitch().Degrees() || 0.0 != angles.GetRoll().Degrees())
                {
                YawPitchRollAngles tmpAngles(AngleInDegrees(), angles.GetPitch(), angles.GetRoll());
                localToWorld = Transform::FromProduct(localToWorld, tmpAngles.ToTransform(DPoint3d::FromZero()));
                }

            m_placement2d.GetOriginR() = DPoint2d::From(origin);
            m_placement2d.GetAngleR() = angles.GetYaw();
            }

        m_havePlacement = true;
        }
    else if (m_is3d)
        {
        localToWorld = m_placement3d.GetTransform();
        }
    else
        {
        localToWorld = m_placement2d.GetTransform();
        }

    if (localToWorld.IsIdentity())
        return true;

    Transform worldToLocal;

    worldToLocal.InverseOf(localToWorld);

    return geom.TransformInPlace(worldToLocal);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometryBuilder::AppendLocal(ElementGeometryCR geom)
    {
    DRange3d localRange;

    if (!geom.GetRange(localRange))
        return false;

    OnNewGeom(localRange);

    if (!m_writer.AppendSimplified(geom, m_is3d))
        m_writer.Append(geom);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometryBuilder::AppendWorld(ElementGeometryR geom)
    {
    if (!ConvertToLocal(geom))
        return false;

    return AppendLocal(geom);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometryBuilder::Append(ElementGeometryCR geom)
    {
    if (!m_is3d && is3dGeometryType(geom.GetGeometryType()))
        {
        BeAssert(false); // 3d only geometry...
        return false;
        }

    if (m_haveLocalGeom)
        return AppendLocal(geom);

    ElementGeometryPtr geomPtr;

    // NOTE: Avoid un-necessary copy of BRep. We just need to change entity transform...
    if (ElementGeometry::GeometryType::SolidKernelEntity == geom.GetGeometryType())
        {
        ISolidKernelEntityPtr clone;

        if (SUCCESS != T_HOST.GetSolidsKernelAdmin()._InstanceEntity(clone, *geom.GetAsISolidKernelEntity()))
            return false;

        geomPtr = ElementGeometry::Create(clone);
        }
    else
        {
        geomPtr = geom.Clone();
        }

    return AppendWorld(*geomPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometryBuilder::Append(ICurvePrimitiveCR geom)
    {
    if (m_haveLocalGeom)
        {
        DRange3d localRange;

        if (!getRange(geom, localRange, nullptr))
            return false;

        OnNewGeom(localRange);

        if (!m_writer.AppendSimplified(geom, false, m_is3d))
            m_writer.Append(geom);

        return true;
        }

    ElementGeometryPtr geomPtr = ElementGeometry::Create(geom);

    return AppendWorld(*geomPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometryBuilder::Append(CurveVectorCR geom)
    {
    if (m_haveLocalGeom)
        {
        DRange3d localRange;

        if (!getRange(geom, localRange, nullptr))
            return false;

        OnNewGeom(localRange);

        if (!m_writer.AppendSimplified(geom, m_is3d))
            m_writer.Append(geom);

        return true;
        }

    ElementGeometryPtr geomPtr = ElementGeometry::Create(geom);

    return AppendWorld(*geomPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometryBuilder::Append(ISolidPrimitiveCR geom)
    {
    if (!m_is3d)
        {
        BeAssert(false); // 3d only geometry...
        return false;
        }

    if (m_haveLocalGeom)
        {
        DRange3d localRange;

        if (!getRange(geom, localRange, nullptr))
            return false;

        OnNewGeom(localRange);
        m_writer.Append(geom);

        return true;
        }

    ElementGeometryPtr geomPtr = ElementGeometry::Create(geom);

    return AppendWorld(*geomPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometryBuilder::Append(MSBsplineSurfaceCR geom)
    {
    if (!m_is3d)
        {
        BeAssert(false); // 3d only geometry...
        return false;
        }

    if (m_haveLocalGeom)
        {
        DRange3d localRange;

        if (!getRange(geom, localRange, nullptr))
            return false;

        OnNewGeom(localRange);
        m_writer.Append(geom);

        return true;
        }

    ElementGeometryPtr geomPtr = ElementGeometry::Create(geom);

    return AppendWorld(*geomPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometryBuilder::Append(PolyfaceQueryCR geom)
    {
    if (!m_is3d)
        {
        BeAssert(false); // 3d only geometry...
        return false;
        }

    if (m_haveLocalGeom)
        {
        DRange3d localRange;

        if (!getRange(geom, localRange, nullptr))
            return false;

        OnNewGeom(localRange);
        m_writer.Append(geom);

        return true;
        }

    ElementGeometryPtr geomPtr = ElementGeometry::Create(geom);

    return AppendWorld(*geomPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometryBuilder::Append(ISolidKernelEntityCR geom)
    {
    if (!m_is3d)
        {
        BeAssert(false); // 3d only geometry...
        return false;
        }

    if (m_haveLocalGeom)
        {
        DRange3d localRange;

        if (!getRange(geom, localRange, nullptr))
            return false;

        OnNewGeom(localRange);
        m_writer.Append(geom);

        return true;
        }

    ISolidKernelEntityPtr clone;

    // NOTE: Avoid un-necessary copy of BRep. We just need to change entity transform...
    if (SUCCESS != T_HOST.GetSolidsKernelAdmin()._InstanceEntity(clone, geom))
        return false;

    ElementGeometryPtr geomPtr = ElementGeometry::Create(clone);

    return AppendWorld(*geomPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometryBuilder::Append(TextStringCR text)
    {
    if (m_haveLocalGeom)
        {
        DRange3d localRange;
        if (!getRange(text, localRange, nullptr))
            return false;

        OnNewGeom(localRange);
        m_writer.Append(text);

        return true;
        }

    return AppendWorld(*ElementGeometry::Create(text));
    }

BEGIN_UNNAMED_NAMESPACE

//=======================================================================================
//! Allows for the ViewContext-based TextAnnotationDraw to be used with an ElementGeometryBuilder.
// @bsiclass                                                    Jeff.Marker     09/2015
//=======================================================================================
struct TextAnnotationDrawToElementGeometry : IElementGraphicsProcessor
{
private:
    TextAnnotationDrawCR m_annotationDraw;
    ElementGeometryBuilderR m_builder;
    DgnCategoryId m_categoryId;
    Transform m_transform;
    Transform m_geomToElem;

public:
    TextAnnotationDrawToElementGeometry(TextAnnotationDrawCR annotationDraw, TransformCR geomToElem, ElementGeometryBuilderR builder, DgnCategoryId categoryId) :
        m_annotationDraw(annotationDraw), m_builder(builder), m_categoryId(categoryId), m_geomToElem (geomToElem), m_transform(Transform::FromIdentity()) {}

    virtual void _AnnounceTransform(TransformCP transform) override { if (nullptr != transform) { m_transform = *transform; } else { m_transform.InitIdentity(); } }
    virtual void _AnnounceElemDisplayParams(ElemDisplayParamsCR params) override { m_builder.Append(params); }
    virtual BentleyStatus _ProcessTextString(TextStringCR) override;
    virtual BentleyStatus _ProcessCurveVector(CurveVectorCR, bool isFilled) override;
    virtual void _OutputGraphics(ViewContextR) override;
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2015
//---------------------------------------------------------------------------------------
BentleyStatus TextAnnotationDrawToElementGeometry::_ProcessTextString(TextStringCR text)
    {
    if (m_transform.IsIdentity())
        {
        m_builder.Append(text);
        }
    else
        {
        TextString transformedText(text);
        transformedText.ApplyTransform(m_transform);
        m_builder.Append(transformedText);
        }

    return SUCCESS; // SUCCESS means handled
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2015
//---------------------------------------------------------------------------------------
BentleyStatus TextAnnotationDrawToElementGeometry::_ProcessCurveVector(CurveVectorCR curves, bool isFilled)
    {
    if (m_transform.IsIdentity())
        {
        m_builder.Append(curves);
        }
    else
        {
        CurveVector transformedCurves(curves);
        transformedCurves.TransformInPlace(m_transform);
        m_builder.Append(transformedCurves);
        }

    return SUCCESS; // SUCCESS means handled
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2015
//---------------------------------------------------------------------------------------
void TextAnnotationDrawToElementGeometry::_OutputGraphics(ViewContextR context)
    {
    context.PushTransform(m_geomToElem);
    context.GetCurrentDisplayParams().SetCategoryId(m_categoryId);
    m_annotationDraw.Draw(context);
    context.PopTransformClip();
    }

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometryBuilder::Append(TextAnnotationCR text, TransformCR transform)
    {
    TextAnnotationDraw annotationDraw(text);
    TextAnnotationDrawToElementGeometry annotationDrawToGeom(annotationDraw, transform, *this, m_elParams.GetCategoryId());
    ElementGraphicsOutput::Process(annotationDrawToGeom, m_dgnDb);

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2015
//---------------------------------------------------------------------------------------
void ElementGeometryBuilder::SetUseCurrentDisplayParams(bool newValue)
    {
    m_writer.Reset((uint32_t)(newValue ? ElementGeomIO::HeaderFlags::UseCurrentDisplayParams : ElementGeomIO::HeaderFlags::None));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryBuilder::ElementGeometryBuilder(DgnDbR dgnDb, DgnCategoryId categoryId, Placement3dCR placement) : m_dgnDb(dgnDb), m_is3d(true), m_writer(dgnDb)
    {
    m_placement3d = placement;
    m_placement2d.GetElementBoxR().Init(); //throw away pre-existing bounding box...
    m_haveLocalGeom = m_havePlacement = true;
    m_appearanceChanged = false;

    if (!(m_isPartCreate = !categoryId.IsValid())) // Called from CreateGeomPart with invalid category...
        m_elParams.SetCategoryId(categoryId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryBuilder::ElementGeometryBuilder(DgnDbR dgnDb, DgnCategoryId categoryId, Placement2dCR placement) : m_dgnDb(dgnDb), m_is3d(false), m_writer(dgnDb)
    {
    m_placement2d = placement;
    m_placement2d.GetElementBoxR().Init(); //throw away pre-existing bounding box...
    m_haveLocalGeom = m_havePlacement = true;
    m_appearanceChanged = false;

    if (!(m_isPartCreate = !categoryId.IsValid())) // Called from CreateGeomPart with invalid category...
        m_elParams.SetCategoryId(categoryId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryBuilder::ElementGeometryBuilder(DgnDbR dgnDb, DgnCategoryId categoryId, bool is3d) : m_dgnDb(dgnDb), m_is3d(is3d), m_writer(dgnDb)
    {
    m_haveLocalGeom = m_havePlacement = m_isPartCreate = false;
    m_elParams.SetCategoryId(categoryId);
    m_appearanceChanged = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryBuilderPtr ElementGeometryBuilder::CreateGeomPart(GeomStreamCR stream, DgnDbR db, bool ignoreSymbology)
    {
    ElementGeometryBuilderPtr builder = new ElementGeometryBuilder(db, DgnCategoryId(), Placement3d());
    ElementGeomIO::Collection collection(stream.GetData(), stream.GetSize());

    for (auto const& egOp : collection)
        {
        switch (egOp.m_opCode)
            {
            case ElementGeomIO::OpCode::Header:
                break; // Already have header....

            case ElementGeomIO::OpCode::GeomPartInstance:
                return nullptr; // Nested parts aren't supported...

            case ElementGeomIO::OpCode::BasicSymbology:
                {
                if (ignoreSymbology)
                    break;

                // Can't change sub-category in GeomPart's GeomStream, only preserve sub-category appearance overrides.
                auto ppfb = flatbuffers::GetRoot<FB::BasicSymbology>(egOp.m_data);

                if (0 == ppfb->subCategoryId())
                    {
                    builder->m_writer.Append(egOp); // No sub-category, ok to write as-is...
                    break;
                    }

                if (!(ppfb->useColor() || ppfb->useWeight() || ppfb->useStyle() || 0.0 != ppfb->transparency() || 0 != ppfb->displayPriority() || 0 != ppfb->geomClass()))
                    break;

                FlatBufferBuilder fbb;

                auto mloc = FB::CreateBasicSymbology(fbb, 0, ppfb->color(), ppfb->weight(), ppfb->lineStyleId(), ppfb->transparency(), ppfb->displayPriority(), ppfb->geomClass(), ppfb->useColor(), ppfb->useWeight(), ppfb->useStyle());
                fbb.Finish(mloc);
                builder->m_writer.Append(ElementGeomIO::Operation(ElementGeomIO::OpCode::BasicSymbology, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
                break;
                }

            case ElementGeomIO::OpCode::LineStyleModifiers:
            case ElementGeomIO::OpCode::AreaFill:
            case ElementGeomIO::OpCode::Pattern:
            case ElementGeomIO::OpCode::Material:
                {
                if (ignoreSymbology)
                    break;

                builder->m_writer.Append(egOp); // Append raw data...
                break;
                }

            case ElementGeomIO::OpCode::ParasolidBRep:
                {
                if (!ignoreSymbology)
                    {
                    builder->m_writer.Append(egOp); // Append raw data...
                    break;
                    }

                auto ppfb = flatbuffers::GetRoot<FB::BRepData>(egOp.m_data);

                if (!ppfb->has_symbology() || !ppfb->has_symbologyIndex())
                    {
                    builder->m_writer.Append(egOp); // Append raw data...
                    break;
                    }

                FlatBufferBuilder fbb;
                auto tmpEntityData = fbb.CreateVector(ppfb->entityData()->Data(), ppfb->entityData()->Length());
                auto mloc = FB::CreateBRepData(fbb, ppfb->entityTransform(), ppfb->brepType(), tmpEntityData);
                fbb.Finish(mloc);
                builder->m_writer.Append(ElementGeomIO::Operation(ElementGeomIO::OpCode::ParasolidBRep, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
                break;
                }

            default:
                builder->m_writer.Append(egOp); // Append raw data so we don't lose bReps, etc. even when we don't have Parasolid available...
                break;
            }
        }

    return builder;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryBuilderPtr ElementGeometryBuilder::CreateGeomPart(DgnDbR db, bool is3d)
    {
    // NOTE: Part geometry is always specified in local coords, i.e. has identity placement.
    //       Category isn't needed when creating a part, invalid category will be used to set m_isPartCreate.
    if (is3d)
        return new ElementGeometryBuilder(db, DgnCategoryId(), Placement3d());

    return new ElementGeometryBuilder(db, DgnCategoryId(), Placement2d());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryBuilderPtr ElementGeometryBuilder::Create(DgnModelR model, DgnCategoryId categoryId, DPoint3dCR origin, YawPitchRollAngles const& angles)
    {
    if (!categoryId.IsValid())
        return nullptr;

    auto geomModel = model.ToGeometricModel();
    if (nullptr == geomModel || !geomModel->Is3d())
        return nullptr;

    Placement3d placement;

    placement.GetOriginR() = origin;
    placement.GetAnglesR() = angles;

    return new ElementGeometryBuilder(model.GetDgnDb(), categoryId, placement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryBuilderPtr ElementGeometryBuilder::Create(DgnModelR model, DgnCategoryId categoryId, DPoint2dCR origin, AngleInDegrees const& angle)
    {
    if (!categoryId.IsValid())
        return nullptr;

    auto geomModel = model.ToGeometricModel();
    if (nullptr == geomModel || geomModel->Is3d())
        return nullptr;

    Placement2d placement;

    placement.GetOriginR() = origin;
    placement.GetAngleR()  = angle;

    return new ElementGeometryBuilder(model.GetDgnDb(), categoryId, placement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryBuilderPtr ElementGeometryBuilder::CreateWorld(DgnModelR model, DgnCategoryId categoryId)
    {
    if (!categoryId.IsValid())
        return nullptr;

    auto geomModel = model.ToGeometricModel();
    if (nullptr == geomModel)
        return nullptr;

    return new ElementGeometryBuilder(model.GetDgnDb(), categoryId, geomModel->Is3d());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryBuilderPtr ElementGeometryBuilder::Create(GeometrySource3dCR source, DPoint3dCR origin, YawPitchRollAngles const& angles)
    {
    DgnCategoryId categoryId = source.GetCategoryId();

    if (!categoryId.IsValid())
        return nullptr;

    Placement3d placement;

    placement.GetOriginR() = origin;
    placement.GetAnglesR() = angles;

    return new ElementGeometryBuilder(source.GetSourceDgnDb(), categoryId, placement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryBuilderPtr ElementGeometryBuilder::Create(GeometrySource2dCR source, DPoint2dCR origin, AngleInDegrees const& angle)
    {
    DgnCategoryId categoryId = source.GetCategoryId();

    if (!categoryId.IsValid())
        return nullptr;

    Placement2d placement;

    placement.GetOriginR() = origin;
    placement.GetAngleR()  = angle;

    return new ElementGeometryBuilder(source.GetSourceDgnDb(), categoryId, placement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryBuilderPtr ElementGeometryBuilder::Create(GeometrySourceCR source)
    {
    DgnCategoryId categoryId = source.GetCategoryId();

    if (!categoryId.IsValid())
        return nullptr;

    if (nullptr != source.ToGeometrySource3d())
        return new ElementGeometryBuilder(source.GetSourceDgnDb(), categoryId, source.ToGeometrySource3d()->GetPlacement());

    return new ElementGeometryBuilder(source.GetSourceDgnDb(), categoryId, source.ToGeometrySource2d()->GetPlacement());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryBuilderPtr ElementGeometryBuilder::CreateWorld(GeometrySourceCR source)
    {
    DgnCategoryId categoryId = source.GetCategoryId();

    if (!categoryId.IsValid())
        return nullptr;

    return new ElementGeometryBuilder(source.GetSourceDgnDb(), categoryId, nullptr != source.ToGeometrySource3d());
    }
