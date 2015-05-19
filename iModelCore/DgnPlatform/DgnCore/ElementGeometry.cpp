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

using namespace flatbuffers;

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometry::GetLocalCoordinateFrame (TransformR localToWorld) const
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
bool ElementGeometry::GetLocalRange (DRange3dR localRange, TransformR localToWorld) const
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
static bool getRange (ICurvePrimitiveCR geom, DRange3dR range, TransformCP transform)
    {
    return (nullptr != transform ? geom.GetRange(range, *transform) : geom.GetRange(range));
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getRange (CurveVectorCR geom, DRange3dR range, TransformCP transform)
    {
    return (nullptr != transform ? geom.GetRange(range, *transform) : geom.GetRange(range));
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getRange (ISolidPrimitiveCR geom, DRange3dR range, TransformCP transform)
    {
    return (nullptr != transform ? geom.GetRange(range, *transform) : geom.GetRange(range));
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getRange (PolyfaceQueryCR geom, DRange3dR range, TransformCP transform)
    {
    range = geom.PointRange();

    if (nullptr != transform)
        transform->Multiply(range, range);

    return true;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getRange (MSBsplineSurfaceCR geom, DRange3dR range, TransformCP transform)
    {
    // NOTE: MSBsplineSurface::GetPoleRange doesn't give a nice fitted box...
    IFacetOptionsPtr          facetOpt = IFacetOptions::Create();
    IPolyfaceConstructionPtr  builder = IPolyfaceConstruction::Create(*facetOpt);

    builder->Add (geom);

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
static bool getRange (TextStringCR text, DRange3dR range, TransformCP transform)
    {
    DRange2dCR textRange = text.GetRange();
    range.low.Init(textRange.low);
    range.high.Init(textRange.high);
    
    if (nullptr != transform)
        transform->Multiply(range, range);

    return true;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometry::GetRange (DRange3dR range, TransformCP transform) const
    {
    range.Init ();

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
            BeAssert (false);
            return false;
            }
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometry::TransformInPlace (TransformCR transform)
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
            
            DPoint3d newOrigin = text.GetOrigin();
            transform.Multiply(newOrigin);

            RotMatrix newOrientation = text.GetOrientation();
            DPoint2d scaleFactor;
            TextString::TransformOrientationAndExtractScale(scaleFactor, newOrientation, transform);

            DPoint2d newSize = text.GetStyle().GetSize();
            newSize.x *= scaleFactor.x;
            newSize.y *= scaleFactor.y;

            text.SetOrigin(newOrigin);
            text.SetOrientation(newOrientation);
            text.GetStyleR().SetSize(newSize);

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
ElementGeometryPtr ElementGeometry::Clone () const
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
            PolyfaceHeaderPtr geom = BentleyApi::PolyfaceHeader::New();

            geom->CopyFrom (*GetAsPolyfaceHeader());

            return new ElementGeometry(geom);
            }

        case GeometryType::BsplineSurface:
            {
            MSBsplineSurfacePtr geom = BentleyApi::MSBsplineSurface::CreatePtr();

            geom->CopyFrom (*GetAsMSBsplineSurface());

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
* @bsimethod                                                    Brien.Bastings  02/15
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometry::ElementGeometry (ICurvePrimitivePtr const& source) {m_type = GeometryType::CurvePrimitive; m_data = source;}
ElementGeometry::ElementGeometry (CurveVectorPtr const& source) {m_type = GeometryType::CurveVector; m_data = source;}
ElementGeometry::ElementGeometry (ISolidPrimitivePtr const& source) {m_type = GeometryType::SolidPrimitive; m_data = source;}
ElementGeometry::ElementGeometry (MSBsplineSurfacePtr const& source) {m_type = GeometryType::BsplineSurface; m_data = source;}
ElementGeometry::ElementGeometry (PolyfaceHeaderPtr const& source) {m_type = GeometryType::Polyface; m_data = source;}
ElementGeometry::ElementGeometry (ISolidKernelEntityPtr const& source) {m_type = GeometryType::SolidKernelEntity; m_data = source;}
ElementGeometry::ElementGeometry (TextStringPtr const& source) {m_type = GeometryType::TextString; m_data = source;}

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/15
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryPtr ElementGeometry::Create (ICurvePrimitivePtr const& source) {return (source.IsValid() ? new ElementGeometry(source) : nullptr);}
ElementGeometryPtr ElementGeometry::Create (CurveVectorPtr const& source) {return (source.IsValid() ? new ElementGeometry(source) : nullptr);}
ElementGeometryPtr ElementGeometry::Create (ISolidPrimitivePtr const& source) {return (source.IsValid() ? new ElementGeometry(source) : nullptr);}
ElementGeometryPtr ElementGeometry::Create (MSBsplineSurfacePtr const& source) {return (source.IsValid() ? new ElementGeometry(source) : nullptr);}
ElementGeometryPtr ElementGeometry::Create (PolyfaceHeaderPtr const& source) {return (source.IsValid() ? new ElementGeometry(source) : nullptr);}
ElementGeometryPtr ElementGeometry::Create (ISolidKernelEntityPtr const& source) {return (source.IsValid() ? new ElementGeometry(source) : nullptr);}
ElementGeometryPtr ElementGeometry::Create (TextStringPtr const& source) {return (source.IsValid() ? new ElementGeometry(source) : nullptr);}

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/15
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryPtr ElementGeometry::Create (ICurvePrimitiveCR source) {ICurvePrimitivePtr clone = source.Clone(); return Create(clone);}
ElementGeometryPtr ElementGeometry::Create (CurveVectorCR source) {CurveVectorPtr clone = source.Clone(); return Create(clone);}
ElementGeometryPtr ElementGeometry::Create (ISolidPrimitiveCR source) {ISolidPrimitivePtr clone = source.Clone(); return Create(clone);}
ElementGeometryPtr ElementGeometry::Create (MSBsplineSurfaceCR source) {MSBsplineSurfacePtr clone = MSBsplineSurface::CreatePtr(); clone->CopyFrom(source); return Create(clone);}
ElementGeometryPtr ElementGeometry::Create (PolyfaceQueryCR source) {PolyfaceHeaderPtr clone = PolyfaceHeader::New(); clone->CopyFrom(source); return Create(clone);}
ElementGeometryPtr ElementGeometry::Create (ISolidKernelEntityCR source) {ISolidKernelEntityPtr clone = source.Clone(); return Create(clone);}
ElementGeometryPtr ElementGeometry::Create (TextStringCR source) {TextStringPtr clone = source.Clone(); return Create(clone);}

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/15
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometry::GeometryType ElementGeometry::GetGeometryType () const {return m_type;}
ICurvePrimitivePtr ElementGeometry::GetAsICurvePrimitive () const {return (GeometryType::CurvePrimitive == m_type ? static_cast <ICurvePrimitiveP> (m_data.get ()) : nullptr);}
CurveVectorPtr ElementGeometry::GetAsCurveVector () const {return (GeometryType::CurveVector == m_type ? static_cast <CurveVectorP> (m_data.get ()) : nullptr);}
ISolidPrimitivePtr ElementGeometry::GetAsISolidPrimitive () const {return (GeometryType::SolidPrimitive == m_type ? static_cast <ISolidPrimitiveP> (m_data.get ()) : nullptr);}
MSBsplineSurfacePtr ElementGeometry::GetAsMSBsplineSurface () const {return (GeometryType::BsplineSurface == m_type ? static_cast <RefCountedMSBsplineSurface*> (m_data.get ()) : nullptr);}
PolyfaceHeaderPtr ElementGeometry::GetAsPolyfaceHeader () const {return (GeometryType::Polyface == m_type ? static_cast <PolyfaceHeaderP> (m_data.get ()) : nullptr);}
ISolidKernelEntityPtr ElementGeometry::GetAsISolidKernelEntity() const { return (GeometryType::SolidKernelEntity == m_type ? static_cast <ISolidKernelEntityP> (m_data.get()) : nullptr); }
TextStringPtr ElementGeometry::GetAsTextString() const { return (GeometryType::TextString == m_type ? static_cast <TextStringP> (m_data.get()) : nullptr); }

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

    m_egOp = Operation ((OpCode) (opCode), dataSize, data);
    m_data += egOpSize;
    m_dataOffset += egOpSize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Writer::Append (Operation const& egOp)
    {
    uint32_t paddedDataSize = (egOp.m_dataSize + 7) & ~7; // 8 byte aligned...
    size_t   egOpSize = sizeof (egOp.m_opCode) + sizeof (egOp.m_dataSize) + paddedDataSize;
    size_t   currSize = m_buffer.size();

    m_buffer.resize (currSize + egOpSize);

    uint8_t*  currOffset = &(m_buffer.at (currSize));

    memcpy (currOffset, &egOp.m_opCode, sizeof (egOp.m_opCode));
    currOffset += sizeof (egOp.m_opCode);

    memcpy (currOffset, &paddedDataSize, sizeof (paddedDataSize));
    currOffset += sizeof (paddedDataSize);

    if (0 == egOp.m_dataSize)
        return;
            
    memcpy (currOffset, egOp.m_data, egOp.m_dataSize);
    currOffset += egOp.m_dataSize;

    if (paddedDataSize > egOp.m_dataSize)
        memset (currOffset, 0, paddedDataSize - egOp.m_dataSize); // Pad quietly or also assert?
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Writer::Append (DPoint3dCP pts, size_t nPts, int8_t boundary)
    {
    FlatBufferBuilder fbb;

    auto coords = fbb.CreateVectorOfStructs ((FB::DPoint3d*) pts, nPts);

    FB::PointPrimitiveBuilder builder (fbb);

    builder.add_coords (coords);
    builder.add_boundary ((FB::BoundaryType) boundary);

    auto mloc = builder.Finish();

    fbb.Finish (mloc);
    Append (Operation (OpCode::PointPrimitive, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Writer::Append (DEllipse3dCR arc, int8_t boundary)
    {
    FlatBufferBuilder fbb;

    auto mloc = FB::CreateArcPrimitive (fbb, (FB::DPoint3d*) &arc.center, (FB::DVec3d*) &arc.vector0, (FB::DVec3d*) &arc.vector90, arc.start, arc.sweep, (FB::BoundaryType) boundary);

    fbb.Finish (mloc);
    Append (Operation (OpCode::ArcPrimitive, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static bool appendSimpleCurvePrimitive (ElementGeomIO::Writer& writer, ICurvePrimitiveCR curvePrimitive, bool isClosed)
    {
    // Special case single/simple curve primitives to avoid having to call new during draw...
    switch (curvePrimitive.GetCurvePrimitiveType())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            {
            DSegment3dCP  segment = curvePrimitive.GetLineCP();

            writer.Append (segment->point, 2, FB::BoundaryType_Open);

            return true;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            {
            bvector<DPoint3d> const*  points = curvePrimitive.GetLineStringCP();

            writer.Append (&points->front(), points->size(), (int8_t) (isClosed ? FB::BoundaryType_Closed : FB::BoundaryType_Open));

            return true;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
            {
            bvector<DPoint3d> const*  points = curvePrimitive.GetPointStringCP();

            writer.Append (&points->front(), points->size(), FB::BoundaryType_None);

            return true;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            {
            DEllipse3dCP  ellipse = curvePrimitive.GetArcCP();

            writer.Append (*ellipse, (int8_t) (isClosed ? FB::BoundaryType_Closed : FB::BoundaryType_Open));

            return true;
            }

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Writer::Append (CurveVectorCR curves)
    {
    // Special case to avoid having to call new during draw...
    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Invalid != curves.HasSingleCurvePrimitive() && appendSimpleCurvePrimitive (*this, *curves.front(), curves.IsClosedPath()))
        return;

    bvector<Byte> buffer;

    BentleyGeometryFlatBuffer::GeometryToBytes (curves, buffer);

    if (0 == buffer.size())
        {
        BeAssert (false);
        return;
        }

    Append (Operation (OpCode::CurveVector, (uint32_t) buffer.size(), &buffer.front()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Writer::Append (ICurvePrimitiveCR curvePrimitive, bool isClosed)
    {
    // Special case to avoid having to call new during draw...
    if (appendSimpleCurvePrimitive (*this, curvePrimitive, isClosed))
        return;

    OpCode        opCode;
    bvector<Byte> buffer;

    if (isClosed)
        {
        CurveVectorPtr  curve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer, curvePrimitive.Clone());

        BentleyGeometryFlatBuffer::GeometryToBytes (*curve, buffer);
        opCode = OpCode::CurveVector;
        }
    else
        {
        BentleyGeometryFlatBuffer::GeometryToBytes (curvePrimitive, buffer);
        opCode = OpCode::CurvePrimitive;
        }

    if (0 == buffer.size())
        {
        BeAssert (false);
        return;
        }

    Append (Operation (opCode, (uint32_t) buffer.size(), &buffer.front()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Writer::Append (PolyfaceQueryCR meshData)
    {
    bvector<Byte> buffer;

    BentleyGeometryFlatBuffer::GeometryToBytes (meshData, buffer);

    if (0 == buffer.size())
        {
        BeAssert (false);
        return;
        }

    Append (Operation (OpCode::Polyface, (uint32_t) buffer.size(), &buffer.front()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Writer::Append (ISolidPrimitiveCR solid)
    {
    bvector<Byte> buffer;

    BentleyGeometryFlatBuffer::GeometryToBytes (solid, buffer);

    if (0 == buffer.size())
        {
        BeAssert (false);
        return;
        }

    Append (Operation (OpCode::SolidPrimitive, (uint32_t) buffer.size(), &buffer.front()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Writer::Append (MSBsplineSurfaceCR surface)
    {
    bvector<Byte> buffer;

    BentleyGeometryFlatBuffer::GeometryToBytes (surface, buffer);

    if (0 == buffer.size())
        {
        BeAssert (false);
        return;
        }

    Append (Operation (OpCode::BsplineSurface, (uint32_t) buffer.size(), &buffer.front()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Writer::Append (ISolidKernelEntityCR entity, IFaceMaterialAttachmentsCP attachments, bool saveBRepOnly)
    {
    bool        saveBRep = saveBRepOnly, saveFacets = false, saveEdges = false, saveFaceIso = false;

    if (!saveBRepOnly)
        {
        switch (entity.GetEntityType())
            {
            case ISolidKernelEntity::EntityType_Sheet: // NEEDWORK: Save single planar face as CurveVector? See PSolidUtil::CreateElement in Vancouver...
            case ISolidKernelEntity::EntityType_Solid:
                {
                saveBRep = saveFacets = true;
                                            
                if (!DgnPlatformLib::QueryHost()->GetSolidsKernelAdmin()._QueryEntityData (entity, ISolidKernelEntity::EntityQuery_HasOnlyPlanarFaces))
                    saveFaceIso = true;
                    
                if (saveFaceIso || DgnPlatformLib::QueryHost()->GetSolidsKernelAdmin()._QueryEntityData (entity, ISolidKernelEntity::EntityQuery_HasCurvedFaceOrEdge))
                    saveEdges = true;
                break;
                }
                    
            case ISolidKernelEntity::EntityType_Wire:
                {
                saveEdges = true; // Just save wire body as CurveVector...very in-efficent and un-necessary to persist these as BReps...
                break;
                }
            }
        }

    if (saveBRep)
        {
        // Make the parasolid data available for platforms that can support it...MUST BE ADDED FIRST!!!
        size_t      bufferSize = 0;
        uint8_t*    buffer = nullptr;

        if (SUCCESS != T_HOST.GetSolidsKernelAdmin()._SaveEntityToMemory (&buffer, bufferSize, entity))
            {
            BeAssert (false);
            return;
            }

        FlatBufferBuilder fbb;

    //    auto faceAttachments = fbb.CreateVectorOfStructs ((FB::FaceSymbology*) ?, n?); // NEEDSWORK
        auto entityData = fbb.CreateVector (buffer, bufferSize);

        FB::BRepDataBuilder builder (fbb);

        builder.add_entityTransform ((FB::Transform*) &entity.GetEntityTransform());
        builder.add_entityData (entityData);
    //    builder.add_faceAttachments (faceAttachments);

        auto mloc = builder.Finish();

        fbb.Finish (mloc);
        Append (Operation (OpCode::ParasolidBRep, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
        }

    if (saveFacets)
        {
        // Store mesh representation for quick display or when parasolid isn't available...
        IFacetOptionsPtr       facetOpt = IFacetOptions::CreateForCurves();
        IFacetTopologyTablePtr facetsPtr;

        if (SUCCESS == DgnPlatformLib::QueryHost()->GetSolidsKernelAdmin()._FacetBody (facetsPtr, entity, *facetOpt, nullptr))
            {
            PolyfaceHeaderPtr polyface = PolyfaceHeader::New();

            if (SUCCESS == IFacetTopologyTable::ConvertToPolyface (*polyface, *facetsPtr, *facetOpt))
                {
                polyface->SetTwoSided (ISolidKernelEntity::EntityType_Solid != entity.GetEntityType());
                polyface->Transform (entity.GetEntityTransform());

                bvector<Byte> buffer;

                BentleyGeometryFlatBuffer::GeometryToBytes (*polyface, buffer);

                if (0 != buffer.size())
                    Append (Operation (saveEdges ? OpCode::BRepPolyface : OpCode::BRepPolyfaceExact, (uint32_t) buffer.size(), &buffer.front()));
                }
            }
        }

    if (saveEdges)
        {
        // When facetted representation is an approximation, we need to store the edge curves for snapping...
        // NEEDSWORK: Face attachments affect color...
        CurveVectorPtr edgeCurves = WireframeGeomUtil::CollectCurves (entity, true, false);

        if (edgeCurves.IsValid ())
            {
            bvector<Byte> buffer;

            BentleyGeometryFlatBuffer::GeometryToBytes (*edgeCurves, buffer);

            if (0 != buffer.size())
                Append (Operation (OpCode::BRepEdges, (uint32_t) buffer.size(), &buffer.front()));
            }
        }

    if (saveFaceIso)
        {
        // When facetted representation is an approximation, we need to store the face-iso curves for wireframe display...
        // NEEDSWORK: Face attachments affect color...
        CurveVectorPtr faceCurves = WireframeGeomUtil::CollectCurves (entity, false, true);

        if (faceCurves.IsValid ())
            {
            bvector<Byte> buffer;

            BentleyGeometryFlatBuffer::GeometryToBytes (*faceCurves, buffer);

            if (0 != buffer.size())
                Append (Operation (OpCode::BRepFaceIso, (uint32_t) buffer.size(), &buffer.front()));
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Writer::Append (DgnSubCategoryId subCategory, TransformCR geomToWorld)
    {
    DPoint3d            origin;
    RotMatrix           rMatrix;
    YawPitchRollAngles  angles;

    geomToWorld.GetTranslation (origin);
    geomToWorld.GetMatrix (rMatrix);

    YawPitchRollAngles::TryFromRotMatrix (angles, rMatrix);

    FlatBufferBuilder fbb;

    auto mloc = FB::CreateBeginSubCategory (fbb, subCategory.GetValueUnchecked(), (FB::DPoint3d*) &origin, angles.GetYaw().Radians(), angles.GetPitch().Radians(), angles.GetRoll().Radians());

    fbb.Finish (mloc);
    Append (Operation (OpCode::BeginSubCategory, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Writer::Append (DgnGeomPartId geomPart)
    {
    FlatBufferBuilder fbb;

    auto mloc = FB::CreateGeomPart (fbb, geomPart.GetValueUnchecked());

    fbb.Finish (mloc);
    Append (Operation (OpCode::GeomPartInstance, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Writer::Append (ElemDisplayParamsCR elParams)
    {
    bool useColor  = !elParams.IsLineColorFromSubCategoryAppearance();
    bool useWeight = !elParams.IsWeightFromSubCategoryAppearance();

    if (useColor || useWeight || 0.0 != elParams.GetTransparency() || 0 != elParams.GetDisplayPriority() || DgnGeometryClass::Primary != elParams.GetGeometryClass())
        {
        FlatBufferBuilder fbb;

        auto mloc = FB::CreateBasicSymbology (fbb, useColor ? elParams.GetLineColor().GetValue() : 0, 
                                                   useWeight ? elParams.GetWeight() : 0,
                                                   elParams.GetTransparency(), elParams.GetDisplayPriority(), 
                                                   useColor, useWeight, (FB::GeometryClass) elParams.GetGeometryClass());
        fbb.Finish (mloc);
        Append (Operation (OpCode::BasicSymbology, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
        }

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

                gradient.GetKey (keyColor, keyValue, i);

                keyColors.push_back (keyColor.GetValue());
                keyValues.push_back (keyValue);
                }

            auto colors = fbb.CreateVector (keyColors);
            auto values = fbb.CreateVector (keyValues);

            auto mloc = FB::CreateAreaFill (fbb, (FB::FillDisplay) elParams.GetFillDisplay(), 0, elParams.GetFillTransparency(),
                                                      (FB::GradientMode) gradient.GetMode(), gradient.GetFlags(), 
                                                      gradient.GetAngle(), gradient.GetTint(), gradient.GetShift(), 
                                                      colors, values);
            fbb.Finish (mloc);
            }
        else
            {
            bool useFillColor = !elParams.IsFillColorFromSubCategoryAppearance();

            auto mloc = FB::CreateAreaFill (fbb, (FB::FillDisplay) elParams.GetFillDisplay(), useFillColor ? elParams.GetFillColor().GetValue() : 0, elParams.GetFillTransparency());

            fbb.Finish (mloc);
            }

        Append (Operation (OpCode::AreaFill, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Writer::Append (ElementGeometryCR elemGeom)
    {
    switch (elemGeom.GetGeometryType())
        {
        case ElementGeometry::GeometryType::CurvePrimitive:
            {
            Append (*elemGeom.GetAsICurvePrimitive());
            break;
            }

        case ElementGeometry::GeometryType::CurveVector:
            {
            Append (*elemGeom.GetAsCurveVector());
            break;
            }

        case ElementGeometry::GeometryType::SolidPrimitive:
            {
            Append (*elemGeom.GetAsISolidPrimitive());
            break;
            }

        case ElementGeometry::GeometryType::Polyface:
            {
            Append (*elemGeom.GetAsPolyfaceHeader());
            break;
            }

        case ElementGeometry::GeometryType::BsplineSurface:
            {
            Append (*elemGeom.GetAsMSBsplineSurface());
            break;
            }

        case ElementGeometry::GeometryType::SolidKernelEntity:
            {
            Append (*elemGeom.GetAsISolidKernelEntity());
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
    if (SUCCESS != TextStringPersistence::EncodeAsFlatBuf(data, text, *m_db))
        return;

    Append(Operation(OpCode::TextString, (uint32_t)data.size(), &data[0]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Reader::Get (Operation const& egOp, DPoint3dCP& pts, int& nPts, int8_t& boundary) const
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
bool ElementGeomIO::Reader::Get (Operation const& egOp, DEllipse3dR arc, int8_t& boundary) const
    {
    if (OpCode::ArcPrimitive != egOp.m_opCode)
        return false;

    auto ppfb = flatbuffers::GetRoot<FB::ArcPrimitive>(egOp.m_data);

    arc.InitFromVectors (*((DPoint3dCP) ppfb->center()), *((DVec3dCP) ppfb->vector0()), *((DVec3dCP) ppfb->vector90()), ppfb->start(), ppfb->sweep());
    boundary = (int8_t) ppfb->boundary();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Reader::Get (Operation const& egOp, ICurvePrimitivePtr& curve) const
    {
    if (OpCode::CurvePrimitive != egOp.m_opCode)
        return false;

    curve = BentleyGeometryFlatBuffer::BytesToCurvePrimitive (egOp.m_data);

    return curve.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Reader::Get (Operation const& egOp, CurveVectorPtr& curves) const
    {
    if (OpCode::CurveVector != egOp.m_opCode)
        return false;

    curves = BentleyGeometryFlatBuffer::BytesToCurveVector (egOp.m_data);

    return curves.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Reader::Get (Operation const& egOp, PolyfaceQueryCarrier& meshData) const
    {
    if (OpCode::Polyface != egOp.m_opCode)
        return false;

    return BentleyGeometryFlatBuffer::BytesToPolyfaceQueryCarrier (egOp.m_data, meshData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Reader::Get (Operation const& egOp, ISolidPrimitivePtr& solid) const
    {
    if (OpCode::SolidPrimitive != egOp.m_opCode)
        return false;

    solid = BentleyGeometryFlatBuffer::BytesToSolidPrimitive (egOp.m_data);

    return solid.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Reader::Get (Operation const& egOp, MSBsplineSurfacePtr& surface) const
    {
    if (OpCode::BsplineSurface != egOp.m_opCode)
        return false;

    surface = BentleyGeometryFlatBuffer::BytesToMSBsplineSurface (egOp.m_data);

    return surface.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Reader::Get (Operation const& egOp, ISolidKernelEntityPtr& entity) const
    {
    if (OpCode::ParasolidBRep != egOp.m_opCode)
        return false;

    auto ppfb = flatbuffers::GetRoot<FB::BRepData>(egOp.m_data);

    if (SUCCESS != T_HOST.GetSolidsKernelAdmin()._RestoreEntityFromMemory (entity, ppfb->entityData()->Data(), ppfb->entityData()->Length(), ISolidKernelEntity::SolidKernel_PSolid, *((TransformCP) ppfb->entityTransform())))
        return false;

    return entity.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Reader::Get (Operation const& egOp, DgnSubCategoryId& subCategory, TransformR geomToWorld) const
    {
    if (OpCode::BeginSubCategory != egOp.m_opCode)
        return false;

    auto ppfb = flatbuffers::GetRoot<FB::BeginSubCategory>(egOp.m_data);

    subCategory = DgnSubCategoryId (ppfb->subCategoryId());

    DPoint3d            origin = *((DPoint3dCP) ppfb->origin());
    YawPitchRollAngles  angles = YawPitchRollAngles::FromRadians (ppfb->yaw(), ppfb->pitch(), ppfb->roll());

    geomToWorld = angles.ToTransform (origin);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Reader::Get (Operation const& egOp, DgnGeomPartId& geomPart) const
    {
    if (OpCode::GeomPartInstance != egOp.m_opCode)
        return false;

    auto ppfb = flatbuffers::GetRoot<FB::GeomPart>(egOp.m_data);

    geomPart = DgnGeomPartId (ppfb->geomPartId());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Reader::Get (Operation const& egOp, ElemDisplayParamsR elParams) const
    {
    bool changed = false;

    switch (egOp.m_opCode)
        {
        case OpCode::BasicSymbology:
            {
            auto ppfb = flatbuffers::GetRoot<FB::BasicSymbology>(egOp.m_data);

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
                    ColorDef fillColor(ppfb->color());

                    if (elParams.IsFillColorFromSubCategoryAppearance() || fillColor != elParams.GetFillColor())
                        {
                        elParams.SetFillColor(fillColor);
                        changed = true;
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
                        keyColors.push_back (ColorDef(colors[iColor]));

                    gradientPtr->SetKeys ((uint16_t) keyColors.size(), &keyColors.front(), (double*) ppfb->values()->Data());
                    elParams.SetGradient(gradientPtr.get());
                    }
                }
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

    return (SUCCESS == TextStringPersistence::DecodeFromFlatBuf(text, egOp.m_data, egOp.m_dataSize, *m_db));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Reader::Get (Operation const& egOp, ElementGeometryPtr& elemGeom) const
    {
    switch (egOp.m_opCode)
        {
        case ElementGeomIO::OpCode::PointPrimitive:
            {
            int         nPts;
            int8_t      boundary;
            DPoint3dCP  pts;
            
            if (!Get (egOp, pts, nPts, boundary))
                break;

            switch (boundary)
                {
                case FB::BoundaryType_None:
                    elemGeom = ElementGeometry::Create (ICurvePrimitive::CreatePointString (pts, nPts));
                    break;

                case FB::BoundaryType_Open:
                    elemGeom = ElementGeometry::Create (ICurvePrimitive::CreateLineString (pts, nPts));
                    break;

                case FB::BoundaryType_Closed:
                    elemGeom = ElementGeometry::Create (CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateLineString (pts, nPts)));
                    break;
                }

            return true;
            }

        case ElementGeomIO::OpCode::ArcPrimitive:
            {
            DEllipse3d  arc;
            int8_t      boundary;

            if (!Get (egOp, arc, boundary))
                break;

            switch (boundary)
                {
                case FB::BoundaryType_None:
                case FB::BoundaryType_Open:
                    elemGeom = ElementGeometry::Create (ICurvePrimitive::CreateArc (arc));
                    break;

                case FB::BoundaryType_Closed:
                    elemGeom = ElementGeometry::Create (CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateArc (arc)));
                    break;
                }

            return true;
            }

        case ElementGeomIO::OpCode::CurvePrimitive:
            {
            ICurvePrimitivePtr curvePtr;
                
            if (!Get (egOp, curvePtr))
                break;

            elemGeom = ElementGeometry::Create (curvePtr);
            return true;
            }

        case ElementGeomIO::OpCode::CurveVector:
            {
            CurveVectorPtr curvePtr;
                
            if (!Get (egOp, curvePtr))
                break;

            elemGeom = ElementGeometry::Create (curvePtr);
            return true;
            }

        case ElementGeomIO::OpCode::Polyface:
            {
            PolyfaceQueryCarrier meshData (0, false, 0, 0, nullptr, nullptr);

            if (!Get (egOp, meshData))
                break;

            elemGeom = ElementGeometry::Create (meshData); // Copy...
            return true;
            }

        case ElementGeomIO::OpCode::SolidPrimitive:
            {
            ISolidPrimitivePtr solidPtr;
                
            if (!Get (egOp, solidPtr))
                break;

            elemGeom = ElementGeometry::Create (solidPtr);
            return true;
            }

        case ElementGeomIO::OpCode::BsplineSurface:
            {
            MSBsplineSurfacePtr surfacePtr;
                
            if (!Get (egOp, surfacePtr))
                break;

            elemGeom = ElementGeometry::Create (surfacePtr);
            return true;
            }

        case ElementGeomIO::OpCode::ParasolidBRep:
            {
            ISolidKernelEntityPtr entityPtr;

            if (!Get (egOp, entityPtr))
                break;

            elemGeom = ElementGeometry::Create (entityPtr);
            return true;
            }

        case ElementGeomIO::OpCode::BRepPolyface:
        case ElementGeomIO::OpCode::BRepPolyfaceExact:
            {
            // NOTE: Caller is expected to filter opCode when they don't want these (Parasolid BRep was available)...
            PolyfaceQueryCarrier meshData (0, false, 0, 0, nullptr, nullptr);

            if (!BentleyGeometryFlatBuffer::BytesToPolyfaceQueryCarrier (egOp.m_data, meshData))
                break;

            elemGeom = ElementGeometry::Create (meshData);
            return true;
            }

        case ElementGeomIO::OpCode::BRepEdges:
        case ElementGeomIO::OpCode::BRepFaceIso:
            {
            // NOTE: Caller is expected to filter opCode when they don't want these...
            CurveVectorPtr curvePtr = BentleyGeometryFlatBuffer::BytesToCurveVector (egOp.m_data);

            if (!curvePtr.IsValid())
                break;

            elemGeom = ElementGeometry::Create (curvePtr);
            return true;
            }
        
        case ElementGeomIO::OpCode::TextString:
            {
            TextStringPtr text = TextString::Create();
            if (SUCCESS != TextStringPersistence::DecodeFromFlatBuf(*text, egOp.m_data, egOp.m_dataSize, *m_db))
                break;
            
            elemGeom = ElementGeometry::Create(text);
            return true;
            }
        }

    return false;
    }

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  02/2015
+===============+===============+===============+===============+===============+======*/
struct DrawState
{
Transform           m_geomToWorld;
ViewContextR        m_context;
ViewFlagsCR         m_flags;
bool                m_symbologyInitialized;
bool                m_symbologyChanged;
bool                m_geomToWorldPushed;

DrawState (ViewContextR context, ViewFlagsCR flags) : m_context(context), m_flags(flags) {m_symbologyInitialized = false; m_symbologyChanged = false; m_geomToWorldPushed = false;}
~DrawState() {End();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Begin (DgnSubCategoryId subCategory, TransformCR geomToWorld)
    {
    End(); // Pop state from a previous Begin (if any). Calls aren't required to be paired...

    m_geomToWorld = geomToWorld;

    // NEEDSWORK: Draw changes - Begin new QvElem...
    if (m_geomToWorldPushed = !m_geomToWorld.IsIdentity())
        m_context.PushTransform (m_geomToWorld);

    SetElemDisplayParams (subCategory);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void End()
    {
    // NEEDSWORK: Draw changes - Complete/Save/Draw QvElem pushing/popping partToWorld...
    if (m_geomToWorldPushed)
        m_context.PopTransformClip();

    m_geomToWorldPushed = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void InitDisplayParams (DgnCategoryId category)
    {
    ElemDisplayParamsR  dispParams = *m_context.GetCurrentDisplayParams();

    dispParams.Init();
    dispParams.SetCategoryId (category);
    
    m_symbologyInitialized = m_symbologyChanged = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SetElemDisplayParams (DgnSubCategoryId subCategory)
    {
    ElemDisplayParamsR  dispParams = *m_context.GetCurrentDisplayParams();

    if (m_symbologyInitialized && subCategory == dispParams.GetSubCategoryId())
        return;

    DgnCategoryId  category = dispParams.GetCategoryId(); // Preserve current category...

    dispParams.Init();
    dispParams.SetCategoryId (category);
    dispParams.SetSubCategoryId (subCategory);

    m_symbologyInitialized = m_symbologyChanged = true;
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
    if (FillDisplay::ByView == m_context.GetCurrentDisplayParams()->GetFillDisplay() && DgnRenderMode::Wireframe == m_flags.GetRenderMode() && !m_flags.fill)
        m_context.GetCurrentDisplayParams()->SetFillDisplay(FillDisplay::Never);

    m_context.CookDisplayParams();
    m_symbologyChanged = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsGeometryVisible()
    {
    switch (m_context.GetCurrentDisplayParams()->GetGeometryClass())
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

    DgnCategories::SubCategory::Appearance appearance = m_context.GetViewport()->GetViewController().GetSubCategoryAppearance(m_context.GetCurrentDisplayParams()->GetSubCategoryId());

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
* @bsimethod                                                    Brien.Bastings  11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Collection::Draw (ViewContextR context, DgnCategoryId category, ViewFlagsCR flags) const
    {
    // NEEDSWORK: Assumes QVElems will be cached per-view unlike Vancouver and cleared if view settings change... 
    bool        isQVis = context.GetIViewDraw().IsOutputQuickVision() || context.CheckICachedDraw();
    bool        isQVWireframe = (isQVis && DgnRenderMode::Wireframe == flags.GetRenderMode());
    bool        isPick = (nullptr != context.GetIPickGeom());
    bool        useBRep = !(isQVis || isPick);
    DrawState   state (context, flags);
    ElementGeomIO::Reader reader(context.GetDgnDb());

    for (auto const& egOp : *this)
        {
        switch (egOp.m_opCode)
            {
            case ElementGeomIO::OpCode::Header:
                {
                // Verify that this is first opcode or something...
                state.InitDisplayParams (category);
                break;
                }

            case ElementGeomIO::OpCode::BeginSubCategory:
                {
                DgnSubCategoryId subCategory;
                Transform        geomToWorld;

                if (!reader.Get (egOp, subCategory, geomToWorld))
                    {
                    state.End();
                    break;
                    }

                state.Begin (subCategory, geomToWorld);
                break;
                }

            case ElementGeomIO::OpCode::BasicSymbology:
            case ElementGeomIO::OpCode::LineStyle:
            case ElementGeomIO::OpCode::AreaFill:
            case ElementGeomIO::OpCode::Pattern:
            case ElementGeomIO::OpCode::Material:
                {
                if (!reader.Get (egOp, *context.GetCurrentDisplayParams()))
                    break;

                state.ChangedElemDisplayParams();
                break;
                }

            case ElementGeomIO::OpCode::GeomPartInstance:
                {
                DgnGeomPartId geomPartId;

                if (!reader.Get (egOp, geomPartId))
                    break;

                DgnGeomParts::Draw(geomPartId, context, category, flags);
                break;
                }

            case ElementGeomIO::OpCode::PointPrimitive:
                {
                if (!state.IsGeometryVisible())
                    break;

                int         nPts;
                int8_t      boundary;
                DPoint3dCP  pts;
                
                if (!reader.Get (egOp, pts, nPts, boundary))
                    break;

                state.CookElemDisplayParams();

                switch (boundary)
                    {
                    case FB::BoundaryType_None:
                        context.GetIDrawGeom().DrawPointString3d (nPts, pts, nullptr);
                        break;

                    case FB::BoundaryType_Open:
                        context.GetIDrawGeom().DrawLineString3d (nPts, pts, nullptr);
                        break;

                    case FB::BoundaryType_Closed:
                        context.GetIDrawGeom().DrawShape3d (nPts, pts, FillDisplay::Never != context.GetCurrentDisplayParams()->GetFillDisplay(), nullptr);
                        break;
                    }
                break;
                }

            case ElementGeomIO::OpCode::ArcPrimitive:
                {
                if (!state.IsGeometryVisible())
                    break;

                DEllipse3d  arc;
                int8_t      boundary;

                if (!reader.Get (egOp, arc, boundary))
                    break;

                state.CookElemDisplayParams();

                if (FB::BoundaryType_Closed != boundary)
                    context.GetIDrawGeom().DrawArc3d (arc, false, false, nullptr);
                else
                    context.GetIDrawGeom().DrawArc3d (arc, true, FillDisplay::Never != context.GetCurrentDisplayParams()->GetFillDisplay(), nullptr);
                break;
                }

            case ElementGeomIO::OpCode::CurvePrimitive:
                {
                if (!state.IsGeometryVisible())
                    break;

                ICurvePrimitivePtr curvePrimitivePtr;
                
                if (!reader.Get (egOp, curvePrimitivePtr))
                    break;

                state.CookElemDisplayParams();

                CurveVectorPtr  curvePtr = CurveVector::Create (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString == curvePrimitivePtr->GetCurvePrimitiveType() ? CurveVector::BOUNDARY_TYPE_None : CurveVector::BOUNDARY_TYPE_Open, curvePrimitivePtr); // Only open stored as curve primitive for element...

                context.GetIDrawGeom().DrawCurveVector (*curvePtr, false);
                break;
                }

            case ElementGeomIO::OpCode::CurveVector:
                {
                if (!state.IsGeometryVisible())
                    break;

                CurveVectorPtr curvePtr;
                
                if (!reader.Get (egOp, curvePtr))
                    break;

                state.CookElemDisplayParams();

                context.GetIDrawGeom().DrawCurveVector (*curvePtr, curvePtr->IsAnyRegionType() && FillDisplay::Never != context.GetCurrentDisplayParams()->GetFillDisplay());
                break;
                }

            case ElementGeomIO::OpCode::Polyface:
                {
                if (!state.IsGeometryVisible())
                    break;

                PolyfaceQueryCarrier meshData (0, false, 0, 0, nullptr, nullptr);

                if (!reader.Get (egOp, meshData))
                    break;

                state.CookElemDisplayParams();

                context.GetIDrawGeom().DrawPolyface (meshData, FillDisplay::Never != context.GetCurrentDisplayParams()->GetFillDisplay());
                break;
                };

            case ElementGeomIO::OpCode::SolidPrimitive:
                {
                if (!state.IsGeometryVisible())
                    break;

                ISolidPrimitivePtr solidPtr;
                
                if (!reader.Get (egOp, solidPtr))
                    break;

                state.CookElemDisplayParams();

                context.GetIDrawGeom().DrawSolidPrimitive (*solidPtr);
                break;
                }

            case ElementGeomIO::OpCode::BsplineSurface:
                {
                if (!state.IsGeometryVisible())
                    break;

                MSBsplineSurfacePtr surfacePtr;
                
                if (!reader.Get (egOp, surfacePtr))
                    break;

                state.CookElemDisplayParams();

                context.GetIDrawGeom().DrawBSplineSurface (*surfacePtr);
                break;
                }

            case ElementGeomIO::OpCode::ParasolidBRep:
                {
                if (!useBRep)
                    break;

                if (!state.IsGeometryVisible())
                    break;

                ISolidKernelEntityPtr entityPtr;

                if (!reader.Get (egOp, entityPtr))
                    {
                    useBRep = false;
                    break;
                    }

                state.CookElemDisplayParams();

                context.GetIDrawGeom().DrawBody (*entityPtr);
                break;
                }

            case ElementGeomIO::OpCode::BRepPolyface:
            case ElementGeomIO::OpCode::BRepPolyfaceExact:
                {
                if (useBRep)
                    break;

                if (!state.IsGeometryVisible())
                    break;

                PolyfaceQueryCarrier meshData (0, false, 0, 0, nullptr, nullptr);

                if (!BentleyGeometryFlatBuffer::BytesToPolyfaceQueryCarrier (egOp.m_data, meshData))
                    break;

                state.CookElemDisplayParams();

                // NOTE: For this case the exact edge geometry will be supplied by BRepEdges/BRepFaceIso, inhibit facetted edge draw...
                if ((isPick || isQVWireframe) && ElementGeomIO::OpCode::BRepPolyface == egOp.m_opCode)
                    {
                    PolyfaceHeaderPtr clone = BentleyApi::PolyfaceHeader::New();

                    clone->CopyFrom (meshData);
                    clone->MarkInvisibleEdges (10.0); // This is "clever" and hopefully temporary...

                    context.GetIDrawGeom().DrawPolyface (*clone);
                    }
                else
                    {
                    context.GetIDrawGeom().DrawPolyface (meshData);
                    }
                break;
                }

            case ElementGeomIO::OpCode::BRepEdges:
                {
                if (!(isPick || isQVWireframe)) // NEEDSWORK: Assumes QVElems are per-view...otherwise must setup WF only matsymb like Vancouver...
                    break;

                if (!state.IsGeometryVisible())
                    break;

                CurveVectorPtr curvePtr = BentleyGeometryFlatBuffer::BytesToCurveVector (egOp.m_data);

                if (!curvePtr.IsValid())
                    break;

                state.CookElemDisplayParams();

                context.GetIDrawGeom().DrawCurveVector (*curvePtr, false);
                break;
                }

            case ElementGeomIO::OpCode::BRepFaceIso:
                {
                if (!(isPick || isQVWireframe)) // NEEDSWORK: Assumes QVElems are per-view...otherwise must setup WF only matsymb like Vancouver...
                    break;

                if (!state.IsGeometryVisible())
                    break;

                CurveVectorPtr curvePtr = BentleyGeometryFlatBuffer::BytesToCurveVector (egOp.m_data);

                if (!curvePtr.IsValid())
                    break;

                state.CookElemDisplayParams();

                context.GetIDrawGeom().DrawCurveVector (*curvePtr, false);
                break;
                }
            
            case ElementGeomIO::OpCode::TextString:
                {
                if (!state.IsGeometryVisible())
                    break;

                TextString text;

                if (SUCCESS != TextStringPersistence::DecodeFromFlatBuf(text, egOp.m_data, egOp.m_dataSize, context.GetDgnDb()))
                    break;
                
                state.CookElemDisplayParams();

                context.GetIDrawGeom().DrawTextString(text);                
                break;
                }
            
            default:
                break;
            }
        }
    }

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  04/2015
+===============+===============+===============+===============+===============+======*/
struct DrawGeomStream : StrokeElementForCache
{
ViewFlags       m_flags;

explicit DrawGeomStream (GeometricElementCR element, ViewFlagsCR flags) : StrokeElementForCache (element)
    {
    m_flags = flags; // NEEDSWORK: Assumes QVElems will be cached per-view unlike Vancouver and cleared if view settings change... 
    }

virtual int32_t _GetQvIndex () const override
    {
    // NEEDSWORK: Won't need this when QVElems are per-view...will just have QVElem per sub-category/transform...
    return (DgnRenderMode::Wireframe != m_flags.GetRenderMode() ? 1 : (m_flags.fill ? 2 : 3));
    }

virtual void _StrokeForCache (ViewContextR context, double pixelSize) override
    {
    ElementGeomIO::Collection(m_element.GetGeomStream().GetData(), m_element.GetGeomStream().GetSize()).Draw(context, m_element.GetCategoryId(), m_flags);
    }

}; // DrawGeomStream

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement::_Draw (ViewContextR context) const
    {
    // NEEDSWORK: Assumes QVElems will be cached per-view unlike Vancouver... 
    ViewFlags   viewFlags;

    if (nullptr != context.GetViewFlags())
        viewFlags = *context.GetViewFlags();
    else
        viewFlags.InitDefaults();

    // NEEDSWORK: Want separate QvElems per-subCategory...
    DrawGeomStream stroker(*this, viewFlags);

    context.DrawCached(stroker);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometricElement::_DrawHit (HitPathCR hit, ViewContextR context) const
    {
    if (DrawPurpose::Flash != context.GetDrawPurpose())
        return false;

    if (!hit.GetComponentMode())
        return false;

    ICurvePrimitiveCP primitive = hit.GetGeomDetail().GetCurvePrimitive();

    if (nullptr == primitive)
        return false;

    GeometricElementCP element = (nullptr != hit.GetHeadElem() ? hit.GetHeadElem()->ToGeometricElement() : nullptr);

    if (nullptr == element)
        return false;

    context.SetCurrentElement(element);

    bool        pushedtrans = false;
    Transform   hitLocalToContextLocal;

    // NOTE: GeomDetail::LocalToWorld includes pushed transforms...
    if (SUCCESS == hit.GetHitLocalToContextLocal(hitLocalToContextLocal, context) && !hitLocalToContextLocal.IsIdentity())
        {
        context.PushTransform(hitLocalToContextLocal);
        pushedtrans = true;
        }

    // NEEDSWORK: Store curve symbology in hit detail when cleaning up DisplayPath/HitPath...
    ElemDisplayParamsR  dispParams = *context.GetCurrentDisplayParams();

    dispParams.Init();
    dispParams.SetCategoryId(element->GetCategoryId());

    context.CookDisplayParams();
    context.ResetContextOverrides();

    DSegment3d      segment;
    CurveVectorPtr  curve;
    bool            doSegmentFlash = (hit.GetPathType() < DisplayPathType::Snap);

    if (!doSegmentFlash)
        {
        switch (static_cast<SnapPathCR>(hit).GetSnapMode())
            {
            case SnapMode::Center:
            case SnapMode::Origin:
            case SnapMode::Bisector:
                break; // Snap point for these is computed using entire linestring, not just the hit segment...

            default:
                doSegmentFlash = true;
                break;
            }
        }

    // Flash only the selected segment of linestrings/shapes based on snap mode...
    if (doSegmentFlash && hit.GetGeomDetail().GetSegment(segment))
        curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateLine(segment));
    else
        curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, primitive->Clone());

    if (element->Is3d())
        context.GetIDrawGeom().DrawCurveVector(*curve, false);
    else
        context.GetIDrawGeom().DrawCurveVector2d(*curve, false, context.GetCurrentDisplayParams()->GetNetDisplayPriority());

    if (pushedtrans)
        context.PopTransformClip();

    context.SetCurrentElement(nullptr);

    return true;
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
            m_data = nullptr;
            m_dataOffset = 0;

            return;
            }

        if (m_partGeometry.IsValid())
            {
            for (ElementGeometryPtr elemGeom : m_partGeometry->GetGeometry())
                {
                if (!elemGeom.IsValid())
                    continue;

                if (!m_elementGeometry.IsValid())
                    {
                    m_elementGeometry = elemGeom;
                    break;
                    }
                
                if (elemGeom.get() == m_elementGeometry.get())
                    m_elementGeometry = nullptr;
                }

            if (m_elementGeometry.IsValid())
                return;

            m_context->SetDgnGeomPartId(DgnGeomPartId());
            m_partGeometry = nullptr;
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
                break;

            case ElementGeomIO::OpCode::BeginSubCategory:
                {
                Transform        geomToWorld;
                DgnSubCategoryId subCategory;

                if (!reader.Get(egOp, subCategory, geomToWorld))
                    break;

                if (nullptr != m_context->GetCurrLocalToFrustumTransformCP())
                    m_context->PopTransformClip(); // Pop previous sub-category/transform...

                m_context->PushTransform(geomToWorld);

                ElemDisplayParamsR  elParams = *m_context->GetCurrentDisplayParams();
                DgnCategoryId       category = elParams.GetCategoryId();

                if (!category.IsValid())
                    category = m_context->GetDgnDb().Categories().QueryCategoryId(subCategory);

                elParams.Init();
                elParams.SetCategoryId(category);
                elParams.SetSubCategoryId(subCategory);
                break;
                }

            case ElementGeomIO::OpCode::BasicSymbology:
            case ElementGeomIO::OpCode::LineStyle:
            case ElementGeomIO::OpCode::AreaFill:
            case ElementGeomIO::OpCode::Pattern:
            case ElementGeomIO::OpCode::Material:
                {
                reader.Get(egOp, *m_context->GetCurrentDisplayParams()); // Update active ElemDisplayParams...
                break;
                }

            case ElementGeomIO::OpCode::GeomPartInstance:
                {
                DgnGeomPartId geomPartId;

                if (!reader.Get(egOp, geomPartId))
                    break;

                DgnGeomPartPtr partGeometry = m_context->GetDgnDb().GeomParts().LoadGeomPart(geomPartId);

                if (!partGeometry.IsValid())
                    break;

                m_elementGeometry = partGeometry->GetGeometry().front();

                if (!m_elementGeometry.IsValid())
                    break; // Ignore failure...

                m_context->GetCurrentDisplayParams()->Resolve(*m_context); // Resolve sub-category appearance...
                m_context->SetDgnGeomPartId(geomPartId);
                m_partGeometry = partGeometry;
                return;
                }

            case ElementGeomIO::OpCode::ParasolidBRep:
                {
                if (!m_useBRep)
                    break;

                if (!reader.Get(egOp, m_elementGeometry) || !m_elementGeometry.IsValid())
                    {
                    m_useBRep = false; // BRep unavailable - start returning BRepPolyface geometry...
                    break;
                    }

                m_context->GetCurrentDisplayParams()->Resolve(*m_context); // Resolve sub-category appearance...
                return;
                }

            case ElementGeomIO::OpCode::BRepEdges:
            case ElementGeomIO::OpCode::BRepFaceIso:
                break; // Skip - Iterator should never return wireframe geometry cache...

            case ElementGeomIO::OpCode::BRepPolyface:
            case ElementGeomIO::OpCode::BRepPolyfaceExact:
                {
                if (m_useBRep)
                    break;

                // Fall through...
                }

            default:
                {
                if (!reader.Get(egOp, m_elementGeometry) || !m_elementGeometry.IsValid())
                    break; // Ignore non-geometry opCode (or failures)...

                m_context->GetCurrentDisplayParams()->Resolve(*m_context); // Resolve sub-category appearance...
                return;
                }
            }

        } while (true);
    }

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  04/2015
+===============+===============+===============+===============+===============+======*/
struct ElementGeometryCollectionContext : public NullContext
{
    DEFINE_T_SUPER(NullContext)
protected:

SimplifyViewDrawGeom* m_output;

virtual void _SetupOutputs() override {SetIViewDraw (*m_output);}

public:

virtual ~ElementGeometryCollectionContext () {delete (m_output);}

ElementGeometryCollectionContext (DgnDbR db)
    {
    m_output = new SimplifyViewDrawGeom();
    m_purpose = DrawPurpose::CaptureGeometry;
    m_wantMaterials = true; // Setup material in ElemDisplayParams...

    SetBlockAsynchs (true);
    m_output->SetViewContext (this);
    _SetupOutputs();

    m_currDisplayParams.Init();
    SetDgnDb (db);
    }

}; // ElementGeometryCollectionContext

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ElemDisplayParamsCR ElementGeometryCollection::GetElemDisplayParams()
    {
    return *m_context->GetCurrentDisplayParams();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGeomPartId ElementGeometryCollection::GetDgnGeomPartId()
    {
    return m_context->GetDgnGeomPartId();
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
    TransformCP currentTrans = m_context->GetCurrLocalToFrustumTransformCP();

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
ElementGeometryCollection::ElementGeometryCollection (DgnDbR dgnDb, uint8_t const* data, size_t dataSize)
    {
    m_data = data;
    m_dataSize = dataSize;
    m_elemToWorld.InitIdentity();
    m_context = new ElementGeometryCollectionContext(dgnDb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryCollection::ElementGeometryCollection (GeometricElementCR element)
    {
    m_data = element.GetGeomStream().GetData();
    m_dataSize = element.GetGeomStream().GetSize();
    m_elemToWorld = (element.Is3d() ? element.ToElement3d()->GetPlacement().GetTransform() : element.ToElement2d()->GetPlacement().GetTransform());
    m_context = new ElementGeometryCollectionContext(element.GetDgnDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryCollection::~ElementGeometryCollection ()
    {
    delete((ElementGeometryCollectionContext*) m_context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ElementGeometryBuilder::SetGeomStreamAndPlacement (GeometricElementR element)
    {
    if (0 == m_writer.m_buffer.size())
        return ERROR;

    if (!m_havePlacement)
        return ERROR;

    if (element.GetCategoryId() != m_elParams.GetCategoryId())
        return ERROR;

    if (m_model.Is3d())
        {
        DgnElement3dP element3d;

        if (nullptr == (element3d = element.ToElement3dP()))
            return ERROR;

        element3d->SetPlacement(m_placement3d);
        }
    else
        {
        DgnElement2dP element2d;

        if (nullptr == (element2d = element.ToElement2dP()))
            return ERROR;

        element2d->SetPlacement(m_placement2d);
        }

    element.GetGeomStreamR().SaveData (&m_writer.m_buffer.front(), (uint32_t) m_writer.m_buffer.size());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometryBuilder::Append (DgnSubCategoryId subCategoryId)
    {
    ElemDisplayParams elParams;

    elParams.SetCategoryId(m_elParams.GetCategoryId()); // Preserve current category...
    elParams.SetSubCategoryId(subCategoryId);

    return Append (elParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometryBuilder::Append (ElemDisplayParamsCR elParams)
    {
    if (!m_elParams.GetCategoryId().IsValid())
        return false;

    if (elParams.GetCategoryId() != m_elParams.GetCategoryId())
        return false;

    if (elParams.GetCategoryId() != m_model.GetDgnDb().Categories().QueryCategoryId(elParams.GetSubCategoryId()))
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
bool ElementGeometryBuilder::Append (DgnGeomPartId geomPartId, TransformCR geomToElement)
    {
    if (!m_havePlacement)
        return false; // geomToElement must be relative to an already defined placement (i.e. not computed placement from CreateWorld)...

    DgnGeomPartPtr geomPart = m_model.GetDgnDb().GeomParts().LoadGeomPart(geomPartId);

    if (!geomPart.IsValid())
        return false;

    DRange3d localRange;

    for (ElementGeometryPtr geom : geomPart->GetGeometry())
        {
        if (!geom.IsValid())
            continue;

        DRange3d range;

        if (!geom->GetRange(range))
            continue;

        localRange.Extend(range);
        }

    OnNewGeom (localRange, &geomToElement);
    m_writer.Append(geomPartId);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeometryBuilder::OnNewGeom (DRange3dCR localRange, TransformCP geomToElement)
    {
    Transform elementToWorld;
    
    if (m_model.Is3d())
        {
        m_placement3d.GetElementBoxR().Extend(localRange);
        elementToWorld = m_placement3d.GetTransform();
        }
    else
        {
        m_placement2d.GetElementBoxR().Extend(DRange2d::From(DPoint2d::From(localRange.low), DPoint2d::From(localRange.high)));
        elementToWorld = m_placement2d.GetTransform();
        }

    Transform geomToWorld = (nullptr == geomToElement ? elementToWorld : Transform::FromProduct(elementToWorld, *geomToElement));

    // Only establish "geometry group" boundaries at sub-category or transform changes...
    if (!m_prevSubCategory.IsValid() || (m_prevSubCategory != m_elParams.GetSubCategoryId() || !m_prevGeomToWorld.IsEqual(geomToWorld)))
        {
        m_writer.Append(m_elParams.GetSubCategoryId(), geomToWorld);

        m_prevSubCategory = m_elParams.GetSubCategoryId();
        m_prevGeomToWorld = geomToWorld;
        }

    if (m_appearanceChanged)
        {
        m_writer.Append(m_elParams);
        m_appearanceChanged = false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometryBuilder::ConvertToLocal (ElementGeometryR geom)
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

        if (m_model.Is3d())
            {
            m_placement3d.GetOriginR() = origin;
            m_placement3d.GetAnglesR() = angles;
            }
        else
            {
            m_placement2d.GetOriginR() = DPoint2d::From(origin);
            m_placement2d.GetAngleR() = angles.GetRoll().Degrees();
            }
    
        m_havePlacement = true;
        }
    else if (m_model.Is3d())
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
bool ElementGeometryBuilder::AppendLocal (ElementGeometryCR geom, TransformCP geomToElement, bool deferWrite)
    {
    DRange3d localRange;

    if (!geom.GetRange(localRange, geomToElement))
        return false;

    OnNewGeom(localRange, geomToElement);

    if (!deferWrite)
        m_writer.Append(geom);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometryBuilder::AppendWorld (ElementGeometryR geom, bool deferWrite)
    {
    if (!ConvertToLocal(geom))
        return false;

    return AppendLocal(geom, nullptr, deferWrite);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometryBuilder::Append (ElementGeometryCR geom)
    {
    if (m_haveLocalGeom)
        return AppendLocal(geom, nullptr);

    ElementGeometryPtr geomPtr;

    // NOTE: Avoid un-necessary copy of BRep. We just need to change entity transform...
    if (ElementGeometry::GeometryType::SolidKernelEntity == geom.GetGeometryType())
        {
        ISolidKernelEntityPtr clone;

        if (SUCCESS != T_HOST.GetSolidsKernelAdmin()._InstanceEntity (clone, *geom.GetAsISolidKernelEntity()))
            return false;

        geomPtr = ElementGeometry::Create (clone);
        }
    else
        {
        geomPtr = geom.Clone ();
        }

    return AppendWorld (*geomPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometryBuilder::Append (ICurvePrimitiveCR geom)
    {
    if (m_haveLocalGeom)
        {
        DRange3d localRange;

        if (!getRange(geom, localRange, nullptr))
            return false;

        OnNewGeom (localRange, nullptr);
        m_writer.Append(geom);

        return true;
        }

    ElementGeometryPtr geomPtr = ElementGeometry::Create(geom);

    return AppendWorld (*geomPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometryBuilder::Append (CurveVectorCR geom)
    {
    if (m_haveLocalGeom)
        {
        DRange3d localRange;

        if (!getRange(geom, localRange, nullptr))
            return false;

        OnNewGeom (localRange, nullptr);
        m_writer.Append(geom);

        return true;
        }

    ElementGeometryPtr geomPtr = ElementGeometry::Create(geom);

    return AppendWorld (*geomPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometryBuilder::Append (ISolidPrimitiveCR geom)
    {
    if (m_haveLocalGeom)
        {
        DRange3d localRange;

        if (!getRange(geom, localRange, nullptr))
            return false;

        OnNewGeom (localRange, nullptr);
        m_writer.Append(geom);

        return true;
        }

    ElementGeometryPtr geomPtr = ElementGeometry::Create(geom);

    return AppendWorld (*geomPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometryBuilder::Append (MSBsplineSurfaceCR geom)
    {
    if (m_haveLocalGeom)
        {
        DRange3d localRange;

        if (!getRange(geom, localRange, nullptr))
            return false;

        OnNewGeom (localRange, nullptr);
        m_writer.Append(geom);

        return true;
        }

    ElementGeometryPtr geomPtr = ElementGeometry::Create(geom);

    return AppendWorld (*geomPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometryBuilder::Append (PolyfaceQueryCR geom)
    {
    if (m_haveLocalGeom)
        {
        DRange3d localRange;

        if (!getRange(geom, localRange, nullptr))
            return false;

        OnNewGeom (localRange, nullptr);
        m_writer.Append(geom);

        return true;
        }

    ElementGeometryPtr geomPtr = ElementGeometry::Create(geom);

    return AppendWorld (*geomPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometryBuilder::Append (ISolidKernelEntityCR geom, IFaceMaterialAttachmentsCP attachments)
    {
    if (m_haveLocalGeom)
        {
        DRange3d localRange;

        if (!getRange(geom, localRange, nullptr))
            return false;

        OnNewGeom (localRange, nullptr);
        m_writer.Append(geom, attachments);

        return true;
        }

    ISolidKernelEntityPtr clone;

    // NOTE: Avoid un-necessary copy of BRep. We just need to change entity transform...
    if (SUCCESS != T_HOST.GetSolidsKernelAdmin()._InstanceEntity(clone, geom))
        return false;

    ElementGeometryPtr geomPtr = ElementGeometry::Create(clone);

    if (!AppendWorld (*geomPtr, true)) // Don't Append as ElementGeomentry for faceAttachments...
        return false;

    m_writer.Append(*geomPtr->GetAsISolidKernelEntity (), attachments);

    return true;
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

        OnNewGeom(localRange, nullptr);
        m_writer.Append(text);

        return true;
        }

    return AppendWorld(*ElementGeometry::Create(text));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryBuilder::ElementGeometryBuilder(DgnModelR model, DgnCategoryId categoryId, Placement3dCR placement) : m_model(model), m_writer(model.GetDgnDb())
    {
    m_elParams.SetCategoryId(categoryId);
    m_appearanceChanged = false;
    m_placement3d = placement;
    m_haveLocalGeom = m_havePlacement = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryBuilder::ElementGeometryBuilder (DgnModelR model, DgnCategoryId categoryId, Placement2dCR placement) : m_model (model), m_writer(model.GetDgnDb())
    {
    m_elParams.SetCategoryId(categoryId);
    m_appearanceChanged = false;
    m_placement2d = placement;
    m_haveLocalGeom = m_havePlacement = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryBuilder::ElementGeometryBuilder (DgnModelR model, DgnCategoryId categoryId) : m_model (model), m_writer(model.GetDgnDb())
    {
    m_elParams.SetCategoryId(categoryId);
    m_appearanceChanged = false;
    m_haveLocalGeom = m_havePlacement = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryBuilderPtr ElementGeometryBuilder::Create (DgnModelR model, DgnCategoryId categoryId, DPoint3dCR origin, YawPitchRollAngles angles)
    {
    if (!categoryId.IsValid() || !model.Is3d())
        return nullptr;

    Placement3d placement;

    placement.GetOriginR() = origin;
    placement.GetAnglesR() = angles;

    return new ElementGeometryBuilder(model, categoryId, placement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryBuilderPtr ElementGeometryBuilder::Create (DgnModelR model, DgnCategoryId categoryId, DPoint2dCR origin, double angle)
    {
    if (!categoryId.IsValid() || model.Is3d())
        return nullptr;

    Placement2d placement;

    placement.GetOriginR() = origin;
    placement.GetAngleR()  = angle;

    return new ElementGeometryBuilder(model, categoryId, placement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryBuilderPtr ElementGeometryBuilder::CreateWorld (DgnModelR model, DgnCategoryId categoryId)
    {
    if (!categoryId.IsValid())
        return nullptr;

    return new ElementGeometryBuilder(model, categoryId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryBuilderPtr ElementGeometryBuilder::Create (DgnElement3dCR element, DPoint3dCR origin, YawPitchRollAngles angles)
    {
    return ElementGeometryBuilder::Create(element.GetDgnModel(), element.GetCategoryId(), origin, angles);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryBuilderPtr ElementGeometryBuilder::Create (DgnElement2dCR element, DPoint2dCR origin, double angle)
    {
    return ElementGeometryBuilder::Create(element.GetDgnModel(), element.GetCategoryId(), origin, angle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryBuilderPtr ElementGeometryBuilder::CreateWorld (GeometricElementCR element)
    {
    return ElementGeometryBuilder::CreateWorld(element.GetDgnModel(), element.GetCategoryId());
    }
