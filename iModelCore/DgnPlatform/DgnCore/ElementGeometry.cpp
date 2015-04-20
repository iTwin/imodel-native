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

using namespace flatbuffers;

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometry::GetLocalCoordinateFrame (TransformR localToWorld) const
    {
    switch (GetGeometryType ())
        {
        case GeometryType::CurvePrimitive:
            {
            ICurvePrimitivePtr curve = GetAsICurvePrimitive ();

            if (!curve->FractionToFrenetFrame (0.0, localToWorld))
                {
                DPoint3d point;

                if (curve->GetStartPoint (point))
                    {
                    localToWorld.InitFrom (point);
                    return true;
                    }

                localToWorld.InitIdentity ();
                return false;
                }

            break;
            }

        case GeometryType::CurveVector:
            {
            CurveVectorPtr curves = GetAsCurveVector ();

            if (!curves->GetAnyFrenetFrame (localToWorld))
                {
                DPoint3d point;

                if (curves->GetStartPoint (point))
                    {
                    localToWorld.InitFrom (point);
                    return true;
                    }

                localToWorld.InitIdentity ();
                return false;
                }

            break;
            }

        case GeometryType::SolidPrimitive:
            {
            Transform          worldToLocal;
            ISolidPrimitivePtr solidPrimitive = GetAsISolidPrimitive ();

            if (!solidPrimitive->TryGetConstructiveFrame (localToWorld, worldToLocal))
                {
                localToWorld.InitIdentity ();
                return false;
                }

            break;
            }

        case GeometryType::Polyface:
            {
            PolyfaceHeaderPtr polyface = GetAsPolyfaceHeader ();

            double      area;
            DPoint3d    centroid;
            RotMatrix   axes;
            DVec3d      momentXYZ;

            if (!polyface->ComputePrincipalAreaMoments (area, centroid, axes, momentXYZ))
                {
                localToWorld.InitIdentity ();
                return false;
                }

            localToWorld.InitFrom (axes, centroid);
            break;
            }

        case GeometryType::BsplineSurface:
            {
            MSBsplineSurfacePtr surface = GetAsMSBsplineSurface ();

            double      area;
            DPoint3d    centroid;
            RotMatrix   axes;
            DVec3d      momentXYZ;

            if (!surface->ComputePrincipalAreaMoments (area, (DVec3dR) centroid, axes, momentXYZ))
                {
                localToWorld.InitIdentity ();
                return false;
                }

            localToWorld.InitFrom (axes, centroid);
            break;
            }
        
        case GeometryType::SolidKernelEntity:
            {
            ISolidKernelEntityPtr entity = GetAsISolidKernelEntity ();

            // The entity transform (after removing SWA scale) can be used for localToWorld (solid kernel to uors)...
            localToWorld = entity->GetEntityTransform ();
            break;
            }

        default:
            {
            localToWorld.InitIdentity ();
            BeAssert (false);

            return false;
            }
        }

    // NOTE: Ensure rotation is squared up and normalized (ComputePrincipalAreaMoments/GetEntityTransform is scaled)... 
    DPoint3d    origin;
    RotMatrix   rMatrix;

    localToWorld.GetTranslation (origin);
    localToWorld.GetMatrix (rMatrix);
    rMatrix.SquareAndNormalizeColumns (rMatrix, 0, 1);
    localToWorld.InitFrom (rMatrix, origin);

    return true;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometry::GetLocalRange (DRange3dR localRange, TransformR localToWorld) const
    {
    if (!GetLocalCoordinateFrame (localToWorld))
        return false;

    if (localToWorld.IsIdentity ())
        return GetRange (localRange);
    
    ElementGeometryPtr clone;
    
    // NOTE: Avoid un-necessary copy of BRep. We just need to change entity transform...
    if (GeometryType::SolidKernelEntity == GetGeometryType())
        {
        ISolidKernelEntityPtr geom;

        if (SUCCESS != T_HOST.GetSolidsKernelAdmin()._InstanceEntity (geom, *GetAsISolidKernelEntity()))
            return false;

        clone = new ElementGeometry (geom);
        }
    else
        {
        clone = Clone ();
        }

    Transform   worldToLocal;

    worldToLocal.InverseOf (localToWorld);
    clone->TransformInPlace (worldToLocal);

    return clone->GetRange (localRange);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeometry::GetRange (DRange3dR range, TransformCP transform) const
    {
    range.Init ();

    switch (GetGeometryType ())
        {
        case GeometryType::CurvePrimitive:
            {
            ICurvePrimitivePtr curve = GetAsICurvePrimitive ();

            return (NULL != transform ? curve->GetRange (range, *transform) : curve->GetRange (range));
            }

        case GeometryType::CurveVector:
            {
            CurveVectorPtr curves = GetAsCurveVector ();

            return (NULL != transform ? curves->GetRange (range, *transform) : curves->GetRange (range));
            }

        case GeometryType::SolidPrimitive:
            {
            ISolidPrimitivePtr solidPrimitive = GetAsISolidPrimitive ();

            return (NULL != transform ? solidPrimitive->GetRange (range, *transform) : solidPrimitive->GetRange (range));
            }

        case GeometryType::Polyface:
            {
            PolyfaceHeaderPtr polyface = GetAsPolyfaceHeader ();

            range = polyface->PointRange ();

            if (NULL != transform)
                transform->Multiply (range, range);

            return true;
            }

        case GeometryType::BsplineSurface:
            {
            MSBsplineSurfacePtr surface = GetAsMSBsplineSurface ();

            // NOTE: MSBsplineSurface::GetPoleRange doesn't give a nice fitted box...
            IFacetOptionsPtr          facetOpt = IFacetOptions::Create ();
            IPolyfaceConstructionPtr  builder = IPolyfaceConstruction::Create (*facetOpt);

            builder->Add (*surface);
            range = builder->GetClientMeshR ().PointRange ();

            if (NULL != transform)
                transform->Multiply (range, range);

            return true;
            }
        
        case GeometryType::SolidKernelEntity:
            {
            ISolidKernelEntityPtr entity = GetAsISolidKernelEntity ();

            if (SUCCESS != DgnPlatformLib::QueryHost ()->GetSolidsKernelAdmin ()._GetEntityRange (range, *entity))
                return false;

            entity->GetEntityTransform ().Multiply (range, range);

            if (NULL != transform)
                transform->Multiply (range, range);

            return true;
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
    switch (GetGeometryType ())
        {
        case GeometryType::CurvePrimitive:
            {
            ICurvePrimitivePtr curve = GetAsICurvePrimitive ();

            return curve->TransformInPlace (transform);
            }

        case GeometryType::CurveVector:
            {
            CurveVectorPtr curves = GetAsCurveVector ();

            return curves->TransformInPlace (transform);
            }

        case GeometryType::SolidPrimitive:
            {
            ISolidPrimitivePtr solidPrimitive = GetAsISolidPrimitive ();

            return solidPrimitive->TransformInPlace (transform);
            }

        case GeometryType::Polyface:
            {
            PolyfaceHeaderPtr polyface = GetAsPolyfaceHeader ();

            polyface->Transform (transform);

            return true;
            }

        case GeometryType::BsplineSurface:
            {
            MSBsplineSurfacePtr surface = GetAsMSBsplineSurface ();

            return (SUCCESS == surface->TransformSurface (transform) ? true : false);
            }
        
        case GeometryType::SolidKernelEntity:
            {
            ISolidKernelEntityPtr entity = GetAsISolidKernelEntity ();

            entity->PreMultiplyEntityTransformInPlace (transform); // Just change entity transform...

            return true;
            }

        default:
            {
            BeAssert (false);
            return false;
            }
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  03/15
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryPtr ElementGeometry::Clone () const
    {
    ElementGeometry* clone = nullptr;

    switch (GetGeometryType ())
        {
        case GeometryType::CurvePrimitive:
            {
            ICurvePrimitivePtr geom = GetAsICurvePrimitive()->Clone();

            clone = new ElementGeometry (geom);
            break;
            }

        case GeometryType::CurveVector:
            {
            CurveVectorPtr geom = GetAsCurveVector()->Clone();

            clone = new ElementGeometry (geom);
            break;
            }

        case GeometryType::SolidPrimitive:
            {
            ISolidPrimitivePtr geom = GetAsISolidPrimitive()->Clone();

            clone = new ElementGeometry (geom);
            break;
            }

        case GeometryType::Polyface:
            {
            PolyfaceHeaderPtr geom = BentleyApi::PolyfaceHeader::New();

            geom->CopyFrom (*GetAsPolyfaceHeader());
            clone = new ElementGeometry (geom);
            break;
            }

        case GeometryType::BsplineSurface:
            {
            MSBsplineSurfacePtr geom = BentleyApi::MSBsplineSurface::CreatePtr();

            geom->CopyFrom (*GetAsMSBsplineSurface());
            clone = new ElementGeometry (geom);
            break;
            }
        
        case GeometryType::SolidKernelEntity:
            {
            ISolidKernelEntityPtr geom;

            if (SUCCESS != T_HOST.GetSolidsKernelAdmin()._CopyEntity (geom, *GetAsISolidKernelEntity()))
                return nullptr;

            clone = new ElementGeometry (geom);
            break;
            }

        default:
            {
            BeAssert (false);
            return nullptr;
            }
        }

    return clone;
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

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/15
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryPtr ElementGeometry::Create (ICurvePrimitivePtr const& source) {return (source.IsValid () ? new ElementGeometry (source) : nullptr);}
ElementGeometryPtr ElementGeometry::Create (CurveVectorPtr const& source) {return (source.IsValid () ? new ElementGeometry (source) : nullptr);}
ElementGeometryPtr ElementGeometry::Create (ISolidPrimitivePtr const& source) {return (source.IsValid () ? new ElementGeometry (source) : nullptr);}
ElementGeometryPtr ElementGeometry::Create (MSBsplineSurfacePtr const& source) {return (source.IsValid () ? new ElementGeometry (source) : nullptr);}
ElementGeometryPtr ElementGeometry::Create (PolyfaceHeaderPtr const& source) {return (source.IsValid () ? new ElementGeometry (source) : nullptr);}
ElementGeometryPtr ElementGeometry::Create (ISolidKernelEntityPtr const& source) {return (source.IsValid () ? new ElementGeometry (source) : nullptr);}

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/15
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometry::GeometryType ElementGeometry::GetGeometryType () const {return m_type;}
ICurvePrimitivePtr ElementGeometry::GetAsICurvePrimitive () const {return (GeometryType::CurvePrimitive == m_type ? static_cast <ICurvePrimitiveP> (m_data.get ()) : NULL);}
CurveVectorPtr ElementGeometry::GetAsCurveVector () const {return (GeometryType::CurveVector == m_type ? static_cast <CurveVectorP> (m_data.get ()) : NULL);}
ISolidPrimitivePtr ElementGeometry::GetAsISolidPrimitive () const {return (GeometryType::SolidPrimitive == m_type ? static_cast <ISolidPrimitiveP> (m_data.get ()) : NULL);}
MSBsplineSurfacePtr ElementGeometry::GetAsMSBsplineSurface () const {return (GeometryType::BsplineSurface == m_type ? static_cast <RefCountedMSBsplineSurface*> (m_data.get ()) : NULL);}
PolyfaceHeaderPtr ElementGeometry::GetAsPolyfaceHeader () const {return (GeometryType::Polyface == m_type ? static_cast <PolyfaceHeaderP> (m_data.get ()) : NULL);}
ISolidKernelEntityPtr ElementGeometry::GetAsISolidKernelEntity () const {return (GeometryType::SolidKernelEntity == m_type ? static_cast <ISolidKernelEntityP> (m_data.get ()) : NULL);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Iterator::ToNext()
    {
    if (m_dataOffset >= m_totalDataSize)
        {
        m_data = NULL;
        m_dataOffset = 0;

        return;
        }

    uint32_t        opCode = *((uint32_t *) (m_data));
    uint32_t        dataSize = *((uint32_t *) (m_data + sizeof (opCode)));
    uint8_t const*    data = (0 != dataSize ? (uint8_t const*) (m_data + sizeof (opCode) + sizeof (dataSize)) : NULL);
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
    size_t  egOpSize = sizeof (egOp.m_opCode) + sizeof (egOp.m_dataSize) + paddedDataSize;
    size_t  currSize = m_buffer.size();

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
static bool appendSimpleCurvePrimitive (ElementGeomIO::Writer& writer, ICurvePrimitiveCR curvePrimitive, bool isClosed, bool isFilled)
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

            writer.Append (&points->front(), points->size(), (int8_t) (isClosed ? (isFilled ? FB::BoundaryType_Filled : FB::BoundaryType_Closed) : FB::BoundaryType_Open));

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

            writer.Append (*ellipse, (int8_t) (isClosed ? (isFilled ? FB::BoundaryType_Filled : FB::BoundaryType_Closed) : FB::BoundaryType_Open));

            return true;
            }

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Writer::Append (CurveVectorCR curves, bool isFilled)
    {
    // Special case to avoid having to call new during draw...
    if (m_creatingElement && ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Invalid != curves.HasSingleCurvePrimitive() && appendSimpleCurvePrimitive (*this, *curves.front(), curves.IsClosedPath(), isFilled))
        return;

    bvector<Byte> buffer;

    BentleyGeometryFlatBuffer::GeometryToBytes (curves, buffer);

    if (0 == buffer.size())
        {
        BeAssert (false);
        return;
        }

    Append (Operation (isFilled ? OpCode::CurveVectorFilled : OpCode::CurveVector, (uint32_t) buffer.size(), &buffer.front()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Writer::Append (ICurvePrimitiveCR curvePrimitive, bool isClosed, bool isFilled)
    {
    // Special case to avoid having to call new during draw...
    if (m_creatingElement && appendSimpleCurvePrimitive (*this, curvePrimitive, isClosed, isFilled))
        return;

    OpCode        opCode;
    bvector<Byte> buffer;

    if (isClosed)
        {
        CurveVectorPtr  curve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer, curvePrimitive.Clone());

        BentleyGeometryFlatBuffer::GeometryToBytes (*curve, buffer);
        opCode = (isFilled ? OpCode::CurveVectorFilled : OpCode::CurveVector);
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
void ElementGeomIO::Writer::Append (PolyfaceQueryCR meshData, bool isFilled)
    {
    bvector<Byte> buffer;

    BentleyGeometryFlatBuffer::GeometryToBytes (meshData, buffer);

    if (0 == buffer.size())
        {
        BeAssert (false);
        return;
        }

    Append (Operation (isFilled ? OpCode::PolyfaceFilled : OpCode::Polyface, (uint32_t) buffer.size(), &buffer.front()));
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
void ElementGeomIO::Writer::Append (ISolidKernelEntityCR entity)
    {
    size_t      bufferSize = 0;
    uint8_t*    buffer = NULL;

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
#if defined (NOT_NOW)
    DPoint3d            origin;
    RotMatrix           rMatrix;
    YawPitchRollAngles  angles;

    partToWorld.GetTranslation (origin);
    partToWorld.GetMatrix (rMatrix);

    YawPitchRollAngles::TryFromRotMatrix (angles, rMatrix);

    FlatBufferBuilder fbb;

    auto mloc = FB::CreateBeginPart (fbb, geomPart.GetValueUnchecked(), subCategory.GetValueUnchecked(), (FB::DPoint3d*) &origin, angles.GetYaw().Radians(), angles.GetPitch().Radians(), angles.GetRoll().Radians());

    fbb.Finish (mloc);
    Append (Operation (OpCode::StartGeomPart, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Writer::Append (ElemDisplayParamsCR elParams)
    {
    bool useColor  = !elParams.IsLineColorFromSubCategoryAppearance();
    bool useWeight = !elParams.IsWeightFromSubCategoryAppearance();

    if (useColor || useWeight || 0.0 != elParams.GetTransparency() || 0 != elParams.GetDisplayPriority())
        {
        FlatBufferBuilder fbb;

        auto mloc = FB::CreateBasicSymbology (fbb, useColor ? elParams.GetLineColor().GetValue() : 0, 
                                                   useWeight ? elParams.GetWeight() : 0,
                                                   elParams.GetTransparency(), elParams.GetDisplayPriority(), 
                                                   useColor, useWeight);
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

            // NEEDSWORK: Support independent gradient transparency so it doesn't have to be the same as the element...
            auto mloc = FB::CreateAreaFill (fbb, (FB::FillDisplay) elParams.GetFillDisplay(), 0, 0.0,
                                                      (FB::GradientMode) gradient.GetMode(), gradient.GetFlags(), 
                                                      gradient.GetAngle(), gradient.GetTint(), gradient.GetShift(), 
                                                      colors, values);
            fbb.Finish (mloc);
            }
        else
            {
            // NEEDSWORK: Support independent fill transparency so it doesn't have to be the same as the element...
            auto mloc = FB::CreateAreaFill (fbb, (FB::FillDisplay) elParams.GetFillDisplay(), elParams.GetFillColor().GetValue(), 0.0);

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
            bool                  saveBRep = false, saveEdges = false, saveFaceIso = false;
            ISolidKernelEntityPtr entity = elemGeom.GetAsISolidKernelEntity();

            switch (entity->GetEntityType())
                {
                case ISolidKernelEntity::EntityType_Sheet:
                case ISolidKernelEntity::EntityType_Solid:
                    {
                    saveBRep = true;
                                            
                    if (!DgnPlatformLib::QueryHost()->GetSolidsKernelAdmin()._QueryEntityData (*entity, ISolidKernelEntity::EntityQuery_HasOnlyPlanarFaces))
                        saveFaceIso = true;
                    
                    if (saveFaceIso || DgnPlatformLib::QueryHost()->GetSolidsKernelAdmin()._QueryEntityData (*entity, ISolidKernelEntity::EntityQuery_HasCurvedFaceOrEdge))
                        saveEdges = true;
                    break;
                    }
                    
                case ISolidKernelEntity::EntityType_Wire:
                    {
                    saveEdges = true; // Just save wire body as CurveVector...very in-efficent and un-necessary to persist these as BReps...
                    break;
                    }
                }

            if (saveBRep)
                {
                // Make the parasolid data available for platforms that can support it...MUST GET ADDED FIRST!!!
                Append (*elemGeom.GetAsISolidKernelEntity());

                // Store mesh representation for quick display or when parasolid isn't available...
                IFacetOptionsPtr       facetOpt = IFacetOptions::CreateForCurves();
                IFacetTopologyTablePtr facetsPtr;

                if (SUCCESS == DgnPlatformLib::QueryHost()->GetSolidsKernelAdmin()._FacetBody (facetsPtr, *entity, *facetOpt, NULL))
                    {
                    PolyfaceHeaderPtr polyface = PolyfaceHeader::New();

                    if (SUCCESS == IFacetTopologyTable::ConvertToPolyface (*polyface, *facetsPtr, *facetOpt))
                        {
                        polyface->SetTwoSided (ISolidKernelEntity::EntityType_Solid != entity->GetEntityType());
                        polyface->Transform (entity->GetEntityTransform());

                        bvector<Byte> buffer;

                        BentleyGeometryFlatBuffer::GeometryToBytes (*polyface, buffer);

                        if (0 != buffer.size())
                            Append (Operation (saveEdges ? OpCode::BRepPolyface : OpCode::BRepPolyfaceExact, (uint32_t) buffer.size(), &buffer.front()));
                        }
                    }
                }

            if (saveEdges)
                {
                // When facetted representation is an approximation, we need to store the edge curvcs for snapping...
                // NEEDSWORK: Face attachments affect color...
                CurveVectorPtr edgeCurves = WireframeGeomUtil::CollectCurves (*entity, true, false);

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
                CurveVectorPtr faceCurves = WireframeGeomUtil::CollectCurves (*entity, false, true);

                if (faceCurves.IsValid ())
                    {
                    bvector<Byte> buffer;

                    BentleyGeometryFlatBuffer::GeometryToBytes (*faceCurves, buffer);

                    if (0 != buffer.size())
                        Append (Operation (OpCode::BRepFaceIso, (uint32_t) buffer.size(), &buffer.front()));
                    }
                }

            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Reader::Get (Operation const& egOp, DPoint3dCP& pts, int& nPts, int8_t& boundary)
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
bool ElementGeomIO::Reader::Get (Operation const& egOp, DEllipse3dR arc, int8_t& boundary)
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
bool ElementGeomIO::Reader::Get (Operation const& egOp, ICurvePrimitivePtr& curve)
    {
    if (OpCode::CurvePrimitive != egOp.m_opCode)
        return false;

    curve = BentleyGeometryFlatBuffer::BytesToCurvePrimitive (egOp.m_data);

    return curve.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Reader::Get (Operation const& egOp, CurveVectorPtr& curves, bool& isFilled)
    {
    if (OpCode::CurveVector == egOp.m_opCode)
        isFilled = false;
    else if (OpCode::CurveVectorFilled == egOp.m_opCode)
        isFilled = true;
    else
        return false;

    curves = BentleyGeometryFlatBuffer::BytesToCurveVector (egOp.m_data);

    return curves.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Reader::Get (Operation const& egOp, PolyfaceQueryCarrier& meshData, bool& isFilled)
    {
    if (OpCode::Polyface == egOp.m_opCode)
        isFilled = false;
    else if (OpCode::PolyfaceFilled == egOp.m_opCode)
        isFilled = true;
    else
        return false;

    return BentleyGeometryFlatBuffer::BytesToPolyfaceQueryCarrier (egOp.m_data, meshData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Reader::Get (Operation const& egOp, ISolidPrimitivePtr& solid)
    {
    if (OpCode::SolidPrimitive != egOp.m_opCode)
        return false;

    solid = BentleyGeometryFlatBuffer::BytesToSolidPrimitive (egOp.m_data);

    return solid.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Reader::Get (Operation const& egOp, MSBsplineSurfacePtr& surface)
    {
    if (OpCode::BsplineSurface != egOp.m_opCode)
        return false;

    surface = BentleyGeometryFlatBuffer::BytesToMSBsplineSurface (egOp.m_data);

    return surface.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Reader::Get (Operation const& egOp, ISolidKernelEntityPtr& entity)
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
bool ElementGeomIO::Reader::Get (Operation const& egOp, DgnSubCategoryId& subCategory, TransformR geomToWorld)
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
bool ElementGeomIO::Reader::Get (Operation const& egOp, DgnGeomPartId& geomPart)
    {
    if (OpCode::GeomPartInstance != egOp.m_opCode)
        return false;

#if defined (NOT_NOW)
    auto ppfb = flatbuffers::GetRoot<FB::BeginPart>(egOp.m_data);

    geomPart = DgnGeomPartId (ppfb->geomPartId());
    subCategory = DgnSubCategoryId (ppfb->subCategoryId());

    DPoint3d            origin = *((DPoint3dCP) ppfb->origin());
    YawPitchRollAngles  angles = YawPitchRollAngles::FromRadians (ppfb->yaw(), ppfb->pitch(), ppfb->roll());

    partToWorld = angles.ToTransform (origin);

    return true;
#else
    return false;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Reader::Get (Operation const& egOp, ElemDisplayParamsR elParams)
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
            break;
            }

        case OpCode::AreaFill:
            {
            // NEEDSWORK...
//            auto ppfb = flatbuffers::GetRoot<FB::AreaFill>(egOp.m_data);

            return false;
            }

        default:
            return false;
        }

    return changed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomIO::Reader::Get (Operation const& egOp, ElementGeometryPtr& elemGeom)
    {
    switch (egOp.m_opCode)
        {
        case ElementGeomIO::OpCode::PointPrimitive:
            {
            int         nPts;
            int8_t      boundary;
            DPoint3dCP  pts;
            
            if (!ElementGeomIO::Reader::Get (egOp, pts, nPts, boundary))
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
                case FB::BoundaryType_Filled:
                    elemGeom = ElementGeometry::Create (CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateLineString (pts, nPts)));
                    break;
                }

            return true;
            }

        case ElementGeomIO::OpCode::ArcPrimitive:
            {
            DEllipse3d  arc;
            int8_t      boundary;

            if (!ElementGeomIO::Reader::Get (egOp, arc, boundary))
                break;

            switch (boundary)
                {
                case FB::BoundaryType_None:
                case FB::BoundaryType_Open:
                    elemGeom = ElementGeometry::Create (ICurvePrimitive::CreateArc (arc));
                    break;

                case FB::BoundaryType_Closed:
                case FB::BoundaryType_Filled:
                    elemGeom = ElementGeometry::Create (CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateArc (arc)));
                    break;
                }

            return true;
            }

        case ElementGeomIO::OpCode::CurvePrimitive:
            {
            ICurvePrimitivePtr curvePtr;
                
            if (!ElementGeomIO::Reader::Get (egOp, curvePtr))
                break;

            elemGeom = ElementGeometry::Create (curvePtr);
            return true;
            }

        case ElementGeomIO::OpCode::CurveVector:
        case ElementGeomIO::OpCode::CurveVectorFilled:
            {
            bool           isFilled;
            CurveVectorPtr curvePtr;
                
            if (!ElementGeomIO::Reader::Get (egOp, curvePtr, isFilled))
                break;

            elemGeom = ElementGeometry::Create (curvePtr);
            return true;
            }

        case ElementGeomIO::OpCode::Polyface:
        case ElementGeomIO::OpCode::PolyfaceFilled:
            {
            bool                 isFilled;
            PolyfaceQueryCarrier meshData (0, false, 0, 0, NULL, NULL);

            if (!ElementGeomIO::Reader::Get (egOp, meshData, isFilled))
                break;

            PolyfaceHeaderPtr polyface = PolyfaceHeader::New();                

            polyface->CopyFrom (meshData);
            
            elemGeom = ElementGeometry::Create (polyface);
            return true;
            }

        case ElementGeomIO::OpCode::SolidPrimitive:
            {
            ISolidPrimitivePtr solidPtr;
                
            if (!ElementGeomIO::Reader::Get (egOp, solidPtr))
                break;

            elemGeom = ElementGeometry::Create (solidPtr);
            return true;
            }

        case ElementGeomIO::OpCode::BsplineSurface:
            {
            MSBsplineSurfacePtr surfacePtr;
                
            if (!ElementGeomIO::Reader::Get (egOp, surfacePtr))
                break;

            elemGeom = ElementGeometry::Create (surfacePtr);
            return true;
            }

        case ElementGeomIO::OpCode::ParasolidBRep:
            {
            ISolidKernelEntityPtr entityPtr;

            if (!ElementGeomIO::Reader::Get (egOp, entityPtr))
                break;

            elemGeom = ElementGeometry::Create (entityPtr);
            return true;
            }

        case ElementGeomIO::OpCode::BRepPolyface:
        case ElementGeomIO::OpCode::BRepPolyfaceExact:
            break; // NEEDSWORK: Get when Parasolid isn't available...
        }

    return false;
    }

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  02/2015
+===============+===============+===============+===============+===============+======*/
struct DrawState
{
//DgnGeomPartId       m_geomPart;
Transform           m_geomToWorld;
ViewContextR        m_context;
bool                m_symbologyInitialized;
bool                m_symbologyChanged;
bool                m_geomToWorldPushed;

DrawState (ViewContextR context) : m_context (context) {m_symbologyInitialized = false; m_symbologyChanged = false; m_geomToWorldPushed = false;}
~DrawState() {End();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Begin (DgnSubCategoryId subCategory, TransformCR geomToWorld)
    {
    End(); // Pop state from a previous Begin (if any). Calls aren't required to be paired...

//    m_geomPart    = geomPart;
    m_geomToWorld = geomToWorld;

//    m_context.SetDgnGeomPartId (m_geomPart); // Announce geom part id for picking, etc.

#if defined (NOT_NOW_DRAW_CHANGES)
    // Begin new QvElem...
#else
    if (m_geomToWorldPushed = !m_geomToWorld.IsIdentity())
        m_context.PushTransform (m_geomToWorld);
#endif

    SetElemDisplayParams (subCategory);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void End()
    {
#if defined (NOT_NOW_DRAW_CHANGES)
    // Complete/Save/Draw QvElem pushing/popping partToWorld...
#else
    if (m_geomToWorldPushed)
        m_context.PopTransformClip();
#endif
    m_geomToWorldPushed = false;
//    m_geomPart.Invalidate();

//    m_context.SetDgnGeomPartId (m_geomPart);
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

#if defined (NEEDS_WORK_ELEMENT_REFACTOR)
    // WIP: not sure we should support drawing from PhysicalGeometry any longer
    if (!category.IsValid())
        category = m_context.GetDgnDb().Categories().QueryCategoryId(subCategory);
#endif

    dispParams.Init();
    dispParams.SetCategoryId (category);
    dispParams.SetSubCategoryId (subCategory);

    m_symbologyInitialized = m_symbologyChanged = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ResetElemDisplayParams (DgnSubCategoryId subCategory)
    {
    m_symbologyInitialized = false;
    SetElemDisplayParams (subCategory);
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

    m_context.CookDisplayParams();
    m_symbologyChanged = false;
    }

}; // DrawState

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Collection::Draw (ViewContextR context, DgnCategoryId category, bool isQVis, bool isQVWireframe, bool isPick, bool useBRep) const
    {
    DrawState   state (context);

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

            case ElementGeomIO::OpCode::GeomPartInstance:
                {
#if defined (NEEDSWORK)
                m_context.SetDgnGeomPartId (m_geomPart); // Announce geom part id for picking, etc.
                // DRAW PART...
                m_context.SetDgnGeomPartId (DgnGeomPartId());
#endif
                break;
                }

            case ElementGeomIO::OpCode::BeginSubCategory:
                {
                DgnSubCategoryId subCategory;
                Transform        geomToWorld;

                if (!ElementGeomIO::Reader::Get (egOp, subCategory, geomToWorld))
                    {
                    state.End();
                    break;
                    }

                state.Begin (subCategory, geomToWorld);
                break;
                }

            case ElementGeomIO::OpCode::ResetSubCategory:
                {
                state.ResetElemDisplayParams (context.GetCurrentDisplayParams()->GetSubCategoryId());
                break;
                }

            case ElementGeomIO::OpCode::BasicSymbology:
            case ElementGeomIO::OpCode::LineStyle:
            case ElementGeomIO::OpCode::AreaFill:
            case ElementGeomIO::OpCode::Pattern:
            case ElementGeomIO::OpCode::Material:
                {
                if (!ElementGeomIO::Reader::Get (egOp, *context.GetCurrentDisplayParams()))
                    break;

                state.ChangedElemDisplayParams();
                break;
                }

            case ElementGeomIO::OpCode::PointPrimitive:
                {
                int         nPts;
                int8_t      boundary;
                DPoint3dCP  pts;
                
                if (!ElementGeomIO::Reader::Get (egOp, pts, nPts, boundary))
                    break;

                state.CookElemDisplayParams();

                switch (boundary)
                    {
                    case FB::BoundaryType_None:
                        context.GetIDrawGeom().DrawPointString3d (nPts, pts, NULL);
                        break;

                    case FB::BoundaryType_Open:
                        context.GetIDrawGeom().DrawLineString3d (nPts, pts, NULL);
                        break;

                    case FB::BoundaryType_Closed:
                        context.GetIDrawGeom().DrawShape3d (nPts, pts, false, NULL);
                        break;

                    case FB::BoundaryType_Filled:
                        context.GetIDrawGeom().DrawShape3d (nPts, pts, true, NULL);
                        break;
                    }
                break;
                }

            case ElementGeomIO::OpCode::ArcPrimitive:
                {
                DEllipse3d  arc;
                int8_t      boundary;

                if (!ElementGeomIO::Reader::Get (egOp, arc, boundary))
                    break;

                state.CookElemDisplayParams();

                context.GetIDrawGeom().DrawArc3d (arc, FB::BoundaryType_Closed == boundary, FB::BoundaryType_Filled == boundary, NULL);
                break;
                }

            case ElementGeomIO::OpCode::CurvePrimitive:
                {
                ICurvePrimitivePtr curvePrimitivePtr;
                
                if (!ElementGeomIO::Reader::Get (egOp, curvePrimitivePtr))
                    break;

                state.CookElemDisplayParams();

                CurveVectorPtr  curvePtr = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open, curvePrimitivePtr); // Only open stored as curve primitive for element...

                context.GetIDrawGeom().DrawCurveVector (*curvePtr, false);
                break;
                }

            case ElementGeomIO::OpCode::CurveVector:
            case ElementGeomIO::OpCode::CurveVectorFilled:
                {
                bool           isFilled;
                CurveVectorPtr curvePtr;
                
                if (!ElementGeomIO::Reader::Get (egOp, curvePtr, isFilled))
                    break;

                state.CookElemDisplayParams();

                context.GetIDrawGeom().DrawCurveVector (*curvePtr, isFilled);
                break;
                }

            case ElementGeomIO::OpCode::Polyface:
            case ElementGeomIO::OpCode::PolyfaceFilled:
                {
                bool                 isFilled;
                PolyfaceQueryCarrier meshData (0, false, 0, 0, NULL, NULL);

                if (!ElementGeomIO::Reader::Get (egOp, meshData, isFilled))
                    break;

                state.CookElemDisplayParams();

                context.GetIDrawGeom().DrawPolyface (meshData, isFilled);
                break;
                };

            case ElementGeomIO::OpCode::SolidPrimitive:
                {
                ISolidPrimitivePtr solidPtr;
                
                if (!ElementGeomIO::Reader::Get (egOp, solidPtr))
                    break;

                state.CookElemDisplayParams();

                context.GetIDrawGeom().DrawSolidPrimitive (*solidPtr);
                break;
                }

            case ElementGeomIO::OpCode::BsplineSurface:
                {
                MSBsplineSurfacePtr surfacePtr;
                
                if (!ElementGeomIO::Reader::Get (egOp, surfacePtr))
                    break;

                state.CookElemDisplayParams();

                context.GetIDrawGeom().DrawBSplineSurface (*surfacePtr);
                break;
                }

            case ElementGeomIO::OpCode::ParasolidBRep:
                {
                if (!useBRep)
                    break;

                ISolidKernelEntityPtr entityPtr;

                if (!ElementGeomIO::Reader::Get (egOp, entityPtr))
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

                PolyfaceQueryCarrier meshData (0, false, 0, 0, NULL, NULL);

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

                CurveVectorPtr curvePtr = BentleyGeometryFlatBuffer::BytesToCurveVector (egOp.m_data);

                if (!curvePtr.IsValid())
                    break;

                state.CookElemDisplayParams();

                context.GetIDrawGeom().DrawCurveVector (*curvePtr, false);
                break;
                }

            default:
                break;
            }
        }
    }

#if defined (NEEDS_WORK_ELEMENT_REFACTOR)
// WIP: not sure we should support drawing from PhysicalGeometry any longer
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomIO::Collection::Draw (ViewContextR context, PhysicalGeometryCR geom)
    {
    DrawState   state(context);

    for (PlacedGeomPart part : geom)
        {
        // NOTE: Caller was responsible for pushing itemToWorld on context, ex. RedrawElems::SetTransform...
        state.Begin(part.GetPartPtr()->GetSubCategoryId(), part.GetGeomPartToGeomAspectTransform());
        state.CookElemDisplayParams();

        context.SetDgnGeomPartId(part.GetPartPtr()->GetId()); // Announce geom part id for picking, etc.

        for (ElementGeometryPtr elemGeom : part.GetPartPtr()->GetGeometry())
            {
            if (!elemGeom.IsValid())
                continue;

            //context.CookDisplayParams(); // NEEDSWORK: Setup from sub-category...

            switch (elemGeom->GetGeometryType())
                {
                case ElementGeometry::GeometryType::CurvePrimitive:
                    {
                    ICurvePrimitivePtr curvePrimitivePtr = elemGeom->GetAsICurvePrimitive();
                    CurveVectorPtr curvePtr = CurveVector::Create (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString == curvePrimitivePtr->GetCurvePrimitiveType() ? CurveVector::BOUNDARY_TYPE_None : CurveVector::BOUNDARY_TYPE_Open, curvePrimitivePtr);

                    context.GetIDrawGeom().DrawCurveVector (*curvePtr, false);
                    break;
                    }

                case ElementGeometry::GeometryType::CurveVector:
                    {
                    context.GetIDrawGeom().DrawCurveVector (*elemGeom->GetAsCurveVector(), false);
                    break;
                    }

                case ElementGeometry::GeometryType::SolidPrimitive:
                    {
                    context.GetIDrawGeom().DrawSolidPrimitive (*elemGeom->GetAsISolidPrimitive());
                    break;
                    }

                case ElementGeometry::GeometryType::Polyface:
                    {
                    context.GetIDrawGeom().DrawPolyface (*elemGeom->GetAsPolyfaceHeader(), false);
                    break;
                    }

                case ElementGeometry::GeometryType::BsplineSurface:
                    {
                    context.GetIDrawGeom().DrawBSplineSurface (*elemGeom->GetAsMSBsplineSurface());
                    break;
                    }
            
                case ElementGeometry::GeometryType::SolidKernelEntity:
                    {
                    context.GetIDrawGeom().DrawBody (*elemGeom->GetAsISolidKernelEntity()); // This is a problem if not cached!!!
                    break;
                    }
                }
            }

        context.SetDgnGeomPartId (DgnGeomPartId());
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ElementGeomIO::Collection::ToGeometry (bvector<ElementGeometryPtr>& geometry) const
    {
    BentleyStatus   status = BentleyStatus::SUCCESS;        

    for (auto const& egOp : *this)
        {
        switch (egOp.m_opCode)
            {
            // NOTE: Symbology always comes before geometry...
            case ElementGeomIO::OpCode::BasicSymbology:
                {
                // NEEDSWORK...
                break;
                }

            case ElementGeomIO::OpCode::ResetSubCategory:
                {
                // NEEDSWORK...
                break;
                }
                
            default:
                {
                ElementGeometryPtr  geom;
        
                if (!ElementGeomIO::Reader::Get (egOp, geom))
                    break; // Ignore non-geometry opCode, it's not an error...

                if (!geom.IsValid())
                    {
                    status = BentleyStatus::ERROR; // Failed to create geometry (ex. ISolidKernelEntity).
                    break;
                    }

                geometry.push_back (geom);
                break;
                }
            }
        }

    return status;
    }

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  04/2015
+===============+===============+===============+===============+===============+======*/
struct DrawGeomStream : StrokeElementForCache
{
bool m_isQVis;
bool m_isQVWireframe;
bool m_isPick;
bool m_useBRep;

explicit DrawGeomStream (GeometricElementCR element, bool isQVis, bool isQVWireframe, bool isPick, bool useBRep) : StrokeElementForCache (element)
    {
    m_isQVis = isQVis;
    m_isQVWireframe = isQVWireframe;
    m_isPick = isPick;
    m_useBRep = useBRep;
    }

virtual int32_t _GetQvIndex () const override {return m_isQVWireframe ? 2 : 1;} // NEEDSWORK: Assumes QVElems are per-view...otherwise must setup WF only matsymb like Vancouver... 

virtual void _StrokeForCache (ViewContextR context, double pixelSize) override
    {
    ElementGeomIO::Collection(m_element.GetGeomStream().GetData(), m_element.GetGeomStream().GetSize()).Draw(context, m_element.GetCategoryId(), m_isQVis, m_isQVWireframe, m_isPick, m_useBRep);
    }

}; // DrawGeomStream

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement::Draw (ViewContextR context) const
    {
    bool        isQVis = context.GetIViewDraw().IsOutputQuickVision();
    bool        isQVWireframe = (isQVis && DgnRenderMode::Wireframe == context.GetViewFlags()->GetRenderMode());
    bool        isPick = (nullptr != context.GetIPickGeom());
    bool        useBRep = !(isQVis || isPick);

    // NEEDSWORK: Want separate QvElems per-subCategory...
    DrawGeomStream stroker(*this, isQVis, isQVWireframe, isPick, useBRep);

    context.DrawCached(stroker);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static AxisAlignedBox3d computeElementRange (DRange3dCR localBox, DPoint3dCR origin, YawPitchRollAnglesCR angles, bool is3d)
    {
    static const double s_smallVal = .0005;
    AxisAlignedBox3d range;

    angles.ToTransform(origin).Multiply(range, localBox);

    // low and high are no longer allowed to be equal...
    if (range.low.x == range.high.x)
        {
        range.low.x -= s_smallVal;
        range.high.x += s_smallVal;
        }

    if (range.low.y == range.high.y)
        {
        range.low.y -= s_smallVal;
        range.high.y += s_smallVal;
        }

    if (is3d)
        {
        if (range.low.z == range.high.z)
            {
            range.low.z -= s_smallVal;
            range.high.z += s_smallVal;
            }
        }
    else
        {
        range.low.z = range.high.z = 0.0;
        }

    return range;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ElementItemHandler::SetElementGeom(DgnElementR element, PhysicalGeometryCR geom, DPoint3dCR origin, YawPitchRollAnglesCR angles)
    {
    PhysicalElementP edP = const_cast<PhysicalElementP>(element._ToPhysicalElement());

    if (nullptr == edP)
        return BentleyStatus::ERROR;

    DgnCategoryId categoryId = edP->GetCategoryId();

    if (!categoryId.IsValid())
        return BentleyStatus::ERROR;

    edP->SetItemClassId(GetItemClassId(element.GetDgnDb()));

    ElementGeomIO::Writer writer;

    writer.SetCreatingElement(); // For element graphics we don't store breps and can make other changes for performance...

    for (PlacedGeomPart const& part : geom)
        {
#if defined (NOT_NOW) // Don't store redundant geometry, draw directly from geom part...
        DgnGeomPartId geomPartId = part.GetPartPtr()->GetId();

        if (!geomPartId.IsValid())
            return BentleyStatus::ERROR; // Geom parts must be created before element can be created.

        DgnSubCategoryId subCategoryId = part.GetPartPtr()->GetSubCategoryId();

        if (!subCategoryId.IsValid())
            subCategoryId = DgnCategories::DefaultSubCategoryId(categoryId);
        else if (categoryId != element.GetDgnDb().Categories().QueryCategoryId(subCategoryId))
            return BentleyStatus::ERROR; // All sub-categories must be for element's category.
#else
        DgnSubCategoryId subCategoryId = DgnCategories::DefaultSubCategoryId(categoryId);
#endif

        // NEEDSWORK: Instancing/Stamps?!?
        Transform     aspectToWorld = angles.ToTransform(origin);
        Transform     partToAspect = part.GetGeomPartToGeomAspectTransform();
        Transform     partToWorld = Transform::FromProduct(aspectToWorld, partToAspect);

        // Mark the start of a new geom part/sub-category...geom part id might be useful to communicate to item during picking?
        writer.Append(subCategoryId, partToWorld);
#if defined (NOT_NOW) // Don't store redundant geometry, draw directly from geom part...
        writer.Append(geomPartId);
#else
        ElemDisplayParams elParams;

        elParams.SetSubCategoryId (subCategoryId);
        elParams.SetLineColor (ColorDef::Green());

        for (ElementGeometryPtr elemGeom : part.GetPartPtr()->GetGeometry())
            {
            if (!elemGeom.IsValid())
                continue;

            writer.Append (elParams); // TESTING...
            writer.Append (*elemGeom);
            }
#endif
        }

    if (0 == writer.m_buffer.size())
        return BentleyStatus::ERROR;

    Placement3dR elGeom = edP->GetPlacementR();
    elGeom.GetOriginR() = origin;
    elGeom.GetAnglesR() = angles;
    elGeom.GetElementBoxR() = geom.CalculateBoundingBox();
    elGeom.GetRangeR() = computeElementRange(elGeom.GetElementBox(), origin, angles, element.GetDgnModel().Is3d());

    edP->GetGeomStreamR().SaveData (&writer.m_buffer.front(), (uint32_t) writer.m_buffer.size());

    return BentleyStatus::SUCCESS;

#if defined (WIP_DO_THIS_IN_ADD_REPLACE)
    // Add function to return vector of DgnGeomPartId for graphics stream so we add setup element geom parts...
    if (BentleyStatus::SUCCESS != element.AddToModel())
        return ElementItemKey();

    if (BentleyStatus::SUCCESS != InsertElementGeomUsesParts(model.GetDgnDb(), elementKey.GetElementId(), physicalGeometry))
        return ElementItemKey();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus setElementGeom(DgnElementR element, ElementGeometryR geom, DgnSubCategoryId subCategoryId, DgnClassId itemClassId, DPoint3dCP origin, YawPitchRollAnglesCP angles)
    {
    PhysicalElementP edP = const_cast<PhysicalElementP>(element._ToPhysicalElement());
    if (nullptr == edP)
        return BentleyStatus::ERROR;

    DgnCategoryId categoryId = edP->GetCategoryId();

    if (!categoryId.IsValid())
        return BentleyStatus::ERROR;
    else if (!subCategoryId.IsValid())
        subCategoryId = DgnCategories::DefaultSubCategoryId(categoryId);
    else if (categoryId != element.GetDgnDb().Categories().QueryCategoryId(subCategoryId))
        return BentleyStatus::ERROR;

    edP->SetItemClassId(itemClassId);

    DPoint3d            tmpOrigin;
    YawPitchRollAngles  tmpAngles;
    Transform           localToWorld;

    // NOTE: Assume geometry already in local coords when origin is supplied (angles are optional and ignore if origin isn't specified)...
    if (nullptr != origin)
        {
        tmpOrigin = *origin;

        if (nullptr != angles)
            tmpAngles = *angles;

        localToWorld = tmpAngles.ToTransform(tmpOrigin);
        }
    else
        {
        if (!geom.GetLocalCoordinateFrame(localToWorld))
            return BentleyStatus::ERROR;

        RotMatrix   rMatrix;
        Transform   worldToLocal;

        worldToLocal.InverseOf(localToWorld);

        if (!localToWorld.IsIdentity() && !geom.TransformInPlace(worldToLocal))
            return BentleyStatus::ERROR;

        localToWorld.GetTranslation(tmpOrigin);
        localToWorld.GetMatrix(rMatrix);
        YawPitchRollAngles::TryFromRotMatrix(tmpAngles, rMatrix);
        }

    ElementAlignedBox3d localBox;

    if (!geom.GetRange(localBox))
        return BentleyStatus::ERROR;
 
    Placement3dR elGeom = edP->GetPlacementR();
    elGeom.GetOriginR() = tmpOrigin;
    elGeom.GetAnglesR() = tmpAngles;
    elGeom.GetElementBoxR() = localBox;
    elGeom.GetRangeR() = computeElementRange (localBox, tmpOrigin, tmpAngles, element.GetDgnModel().Is3d());

    ElementGeomIO::Writer writer;

    writer.SetCreatingElement();
    writer.Append(subCategoryId, localToWorld);
    writer.Append(geom);

    edP->GetGeomStreamR().SaveData (&writer.m_buffer.front(), (uint32_t) writer.m_buffer.size());

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ElementItemHandler::SetElementGeom(DgnElementR element, ElementGeometryCR geom, DgnSubCategoryId subCategoryId, DPoint3dCP origin, YawPitchRollAnglesCP angles)
    {
    ElementGeometryPtr geomPtr;
    
    // NOTE: Avoid un-necessary copy of BRep. We just need to change entity transform...
    if (ElementGeometry::GeometryType::SolidKernelEntity == geom.GetGeometryType())
        {
        ISolidKernelEntityPtr clone;

        if (SUCCESS != T_HOST.GetSolidsKernelAdmin()._InstanceEntity (clone, *geom.GetAsISolidKernelEntity()))
            return BentleyStatus::ERROR;

        geomPtr = ElementGeometry::Create (clone);
        }
    else
        {
        geomPtr = geom.Clone ();
        }

    return setElementGeom(element, *geomPtr, subCategoryId, GetItemClassId(element.GetDgnDb()), origin, angles);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ElementItemHandler::SetElementGeom(DgnElementR element, CurveVectorCR geom, DgnSubCategoryId subCategoryId, DPoint3dCP origin, YawPitchRollAnglesCP angles)
    {
    CurveVectorPtr      clone = geom.Clone();
    ElementGeometryPtr  geomPtr = ElementGeometry::Create(clone);

    return setElementGeom(element, *geomPtr, subCategoryId, GetItemClassId(element.GetDgnDb()), origin, angles);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ElementItemHandler::SetElementGeom(DgnElementR element, ICurvePrimitiveCR geom, DgnSubCategoryId subCategoryId, DPoint3dCP origin, YawPitchRollAnglesCP angles)
    {
    ICurvePrimitivePtr  clone = geom.Clone();
    ElementGeometryPtr  geomPtr = ElementGeometry::Create(clone);

    return setElementGeom(element, *geomPtr, subCategoryId, GetItemClassId(element.GetDgnDb()), origin, angles);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ElementItemHandler::SetElementGeom(DgnElementR element, ISolidPrimitiveCR geom, DgnSubCategoryId subCategoryId, DPoint3dCP origin, YawPitchRollAnglesCP angles)
    {
    ISolidPrimitivePtr  clone = geom.Clone();
    ElementGeometryPtr  geomPtr = ElementGeometry::Create(clone);

    return setElementGeom(element, *geomPtr, subCategoryId, GetItemClassId(element.GetDgnDb()), origin, angles);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ElementItemHandler::SetElementGeom(DgnElementR element, MSBsplineSurfaceCR geom, DgnSubCategoryId subCategoryId, DPoint3dCP origin, YawPitchRollAnglesCP angles)
    {
    MSBsplineSurfacePtr clone = MSBsplineSurface::CreatePtr();

    clone->CopyFrom (geom);

    ElementGeometryPtr  geomPtr = ElementGeometry::Create(clone);

    return setElementGeom(element, *geomPtr, subCategoryId, GetItemClassId(element.GetDgnDb()), origin, angles);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ElementItemHandler::SetElementGeom(DgnElementR element, PolyfaceQueryCR geom, DgnSubCategoryId subCategoryId, DPoint3dCP origin, YawPitchRollAnglesCP angles)
    {
    PolyfaceHeaderPtr clone = PolyfaceHeader::New();

    clone->CopyFrom (geom);

    ElementGeometryPtr  geomPtr = ElementGeometry::Create(clone);

    return setElementGeom(element, *geomPtr, subCategoryId, GetItemClassId(element.GetDgnDb()), origin, angles);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ElementItemHandler::SetElementGeom(DgnElementR element, ISolidKernelEntityCR geom, DgnSubCategoryId subCategoryId, DPoint3dCP origin, YawPitchRollAnglesCP angles)
    {
    ISolidKernelEntityPtr clone;

    // NOTE: Avoid un-necessary copy of BRep. We just need to change entity transform...
    if (SUCCESS != T_HOST.GetSolidsKernelAdmin()._InstanceEntity(clone, geom))
        return BentleyStatus::ERROR;

    ElementGeometryPtr  geomPtr = ElementGeometry::Create(clone);

    return setElementGeom(element, *geomPtr, subCategoryId, GetItemClassId(element.GetDgnDb()), origin, angles);
    }
